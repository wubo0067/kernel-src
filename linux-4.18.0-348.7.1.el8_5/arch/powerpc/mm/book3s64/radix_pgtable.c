/*
 * Page table handling routines for radix page table.
 *
 * Copyright 2015-2016, Aneesh Kumar K.V, IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#define pr_fmt(fmt) "radix-mmu: " fmt

#include <linux/kernel.h>
#include <linux/sched/mm.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <linux/mm.h>
#include <linux/string_helpers.h>
#include <linux/memory.h>

#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/mmu_context.h>
#include <asm/dma.h>
#include <asm/machdep.h>
#include <asm/mmu.h>
#include <asm/firmware.h>
#include <asm/powernv.h>
#include <asm/sections.h>
#include <asm/trace.h>
#include <asm/ultravisor.h>

#include <trace/events/thp.h>

unsigned int mmu_pid_bits;
unsigned int mmu_base_pid;
unsigned long radix_mem_block_size __ro_after_init;

static __ref void *early_alloc_pgtable(unsigned long size, int nid,
			unsigned long region_start, unsigned long region_end)
{
	unsigned long pa = 0;
	void *pt;

	if (region_start || region_end) /* has region hint */
		pa = memblock_alloc_range(size, size, region_start, region_end,
						MEMBLOCK_NONE);
	else if (nid != -1) /* has node hint */
		pa = memblock_alloc_base_nid(size, size,
						MEMBLOCK_ALLOC_ANYWHERE,
						nid, MEMBLOCK_NONE);

	if (!pa)
		pa = memblock_alloc_base(size, size, MEMBLOCK_ALLOC_ANYWHERE);

	BUG_ON(!pa);

	pt = __va(pa);
	memset(pt, 0, size);

	return pt;
}

/*
 * When allocating pud or pmd pointers, we allocate a complete page
 * of PAGE_SIZE rather than PUD_TABLE_SIZE or PMD_TABLE_SIZE. This
 * is to ensure that the page obtained from the memblock allocator
 * can be completely used as page table page and can be freed
 * correctly when the page table entries are removed.
 */
static int early_map_kernel_page(unsigned long ea, unsigned long pa,
			  pgprot_t flags,
			  unsigned int map_page_size,
			  int nid,
			  unsigned long region_start, unsigned long region_end)
{
	unsigned long pfn = pa >> PAGE_SHIFT;
	pgd_t *pgdp;
	pud_t *pudp;
	pmd_t *pmdp;
	pte_t *ptep;

	pgdp = pgd_offset_k(ea);
	if (pgd_none(*pgdp)) {
		pudp = early_alloc_pgtable(PAGE_SIZE, nid,
					   region_start, region_end);
		pgd_populate(&init_mm, pgdp, pudp);
	}
	pudp = pud_offset(pgdp, ea);
	if (map_page_size == PUD_SIZE) {
		ptep = (pte_t *)pudp;
		goto set_the_pte;
	}
	if (pud_none(*pudp)) {
		pmdp = early_alloc_pgtable(PAGE_SIZE, nid, region_start,
					   region_end);
		pud_populate(&init_mm, pudp, pmdp);
	}
	pmdp = pmd_offset(pudp, ea);
	if (map_page_size == PMD_SIZE) {
		ptep = pmdp_ptep(pmdp);
		goto set_the_pte;
	}
	if (!pmd_present(*pmdp)) {
		ptep = early_alloc_pgtable(PAGE_SIZE, nid,
						region_start, region_end);
		pmd_populate_kernel(&init_mm, pmdp, ptep);
	}
	ptep = pte_offset_kernel(pmdp, ea);

set_the_pte:
	set_pte_at(&init_mm, ea, ptep, pfn_pte(pfn, flags));
	smp_wmb();
	return 0;
}

/*
 * nid, region_start, and region_end are hints to try to place the page
 * table memory in the same node or region.
 */
static int __map_kernel_page(unsigned long ea, unsigned long pa,
			  pgprot_t flags,
			  unsigned int map_page_size,
			  int nid,
			  unsigned long region_start, unsigned long region_end)
{
	unsigned long pfn = pa >> PAGE_SHIFT;
	pgd_t *pgdp;
	pud_t *pudp;
	pmd_t *pmdp;
	pte_t *ptep;
	/*
	 * Make sure task size is correct as per the max adddr
	 */
	BUILD_BUG_ON(TASK_SIZE_USER64 > RADIX_PGTABLE_RANGE);

	if (unlikely(!slab_is_available()))
		return early_map_kernel_page(ea, pa, flags, map_page_size,
						nid, region_start, region_end);

	/*
	 * Should make page table allocation functions be able to take a
	 * node, so we can place kernel page tables on the right nodes after
	 * boot.
	 */
	pgdp = pgd_offset_k(ea);
	pudp = pud_alloc(&init_mm, pgdp, ea);
	if (!pudp)
		return -ENOMEM;
	if (map_page_size == PUD_SIZE) {
		ptep = (pte_t *)pudp;
		goto set_the_pte;
	}
	pmdp = pmd_alloc(&init_mm, pudp, ea);
	if (!pmdp)
		return -ENOMEM;
	if (map_page_size == PMD_SIZE) {
		ptep = pmdp_ptep(pmdp);
		goto set_the_pte;
	}
	ptep = pte_alloc_kernel(pmdp, ea);
	if (!ptep)
		return -ENOMEM;

set_the_pte:
	set_pte_at(&init_mm, ea, ptep, pfn_pte(pfn, flags));
	smp_wmb();
	return 0;
}

int radix__map_kernel_page(unsigned long ea, unsigned long pa,
			  pgprot_t flags,
			  unsigned int map_page_size)
{
	return __map_kernel_page(ea, pa, flags, map_page_size, -1, 0, 0);
}

#ifdef CONFIG_STRICT_KERNEL_RWX
void radix__change_memory_range(unsigned long start, unsigned long end,
				unsigned long clear)
{
	unsigned long idx;
	pgd_t *pgdp;
	pud_t *pudp;
	pmd_t *pmdp;
	pte_t *ptep;

	start = ALIGN_DOWN(start, PAGE_SIZE);
	end = PAGE_ALIGN(end); // aligns up

	pr_debug("Changing flags on range %lx-%lx removing 0x%lx\n",
		 start, end, clear);

	for (idx = start; idx < end; idx += PAGE_SIZE) {
		pgdp = pgd_offset_k(idx);
		pudp = pud_alloc(&init_mm, pgdp, idx);
		if (!pudp)
			continue;
		if (pud_is_leaf(*pudp)) {
			ptep = (pte_t *)pudp;
			goto update_the_pte;
		}
		pmdp = pmd_alloc(&init_mm, pudp, idx);
		if (!pmdp)
			continue;
		if (pmd_is_leaf(*pmdp)) {
			ptep = pmdp_ptep(pmdp);
			goto update_the_pte;
		}
		ptep = pte_alloc_kernel(pmdp, idx);
		if (!ptep)
			continue;
update_the_pte:
		radix__pte_update(&init_mm, idx, ptep, clear, 0, 0);
	}

	radix__flush_tlb_kernel_range(start, end);
}

void radix__mark_rodata_ro(void)
{
	unsigned long start, end;

	start = (unsigned long)_stext;
	end = (unsigned long)__init_begin;

	radix__change_memory_range(start, end, _PAGE_WRITE);
}

void radix__mark_initmem_nx(void)
{
	unsigned long start = (unsigned long)__init_begin;
	unsigned long end = (unsigned long)__init_end;

	radix__change_memory_range(start, end, _PAGE_EXEC);
}
#endif /* CONFIG_STRICT_KERNEL_RWX */

static inline void __meminit print_mapping(unsigned long start,
					   unsigned long end,
					   unsigned long size,
					   bool exec)
{
	char buf[10];

	if (end <= start)
		return;

	string_get_size(size, 1, STRING_UNITS_2, buf, sizeof(buf));

	pr_info("Mapped 0x%016lx-0x%016lx with %s pages%s\n", start, end, buf,
		exec ? " (exec)" : "");
}

static int __meminit create_physical_mapping(unsigned long start,
					     unsigned long end,
					     unsigned long max_mapping_size,
					     int nid, pgprot_t _prot)
{
	unsigned long vaddr, addr, mapping_size = 0;
	bool prev_exec, exec = false;
	pgprot_t prot;
#ifdef CONFIG_STRICT_KERNEL_RWX
	int split_text_mapping = 1;
#else
	int split_text_mapping = 0;
#endif

	start = ALIGN(start, PAGE_SIZE);
	for (addr = start; addr < end; addr += mapping_size) {
		unsigned long gap, previous_size;
		int rc;

		gap = end - addr;
		previous_size = mapping_size;
		prev_exec = exec;
		if (gap > max_mapping_size)
			gap = max_mapping_size;

retry:
		if (IS_ALIGNED(addr, PUD_SIZE) && gap >= PUD_SIZE &&
		    mmu_psize_defs[MMU_PAGE_1G].shift &&
		    PUD_SIZE <= max_mapping_size)
			mapping_size = PUD_SIZE;
		else if (IS_ALIGNED(addr, PMD_SIZE) && gap >= PMD_SIZE &&
			 mmu_psize_defs[MMU_PAGE_2M].shift)
			mapping_size = PMD_SIZE;
		else
			mapping_size = PAGE_SIZE;

		if (split_text_mapping && (mapping_size == PUD_SIZE) &&
			(addr <= __pa_symbol(__init_begin)) &&
			(addr + mapping_size) >= __pa_symbol(_stext)) {
			max_mapping_size = PMD_SIZE;
			goto retry;
		}

		if (split_text_mapping && (mapping_size == PMD_SIZE) &&
		    (addr <= __pa_symbol(__init_begin)) &&
		    (addr + mapping_size) >= __pa_symbol(_stext))
			mapping_size = PAGE_SIZE;

		if (mapping_size != previous_size || exec != prev_exec) {
			print_mapping(start, addr, previous_size, prev_exec);
			start = addr;
		}

		vaddr = (unsigned long)__va(addr);

		if (overlaps_kernel_text(vaddr, vaddr + mapping_size) ||
		    overlaps_interrupt_vector_text(vaddr, vaddr +
		    mapping_size)) {
			prot = PAGE_KERNEL_X;
			exec = true;
		} else {
			prot = _prot;
			exec = false;
		}

		if (mapping_size != previous_size || exec != prev_exec) {
			print_mapping(start, addr, previous_size, prev_exec);
			start = addr;
		}

		rc = __map_kernel_page(vaddr, addr, prot, mapping_size, nid, start, end);
		if (rc)
			return rc;
	}

	print_mapping(start, addr, mapping_size, exec);
	return 0;
}

void __init radix_init_pgtable(void)
{
	unsigned long rts_field;
	struct memblock_region *reg;

	/* We don't support slb for radix */
	mmu_slb_size = 0;

	/*
	 * Create the linear mapping
	 */
	for_each_memblock(memory, reg) {
		/*
		 * The memblock allocator  is up at this point, so the
		 * page tables will be allocated within the range. No
		 * need or a node (which we don't have yet).
		 */
		WARN_ON(create_physical_mapping(reg->base,
						reg->base + reg->size,
						radix_mem_block_size,
						-1, PAGE_KERNEL));
	}

	/* Find out how many PID bits are supported */
	if (!cpu_has_feature(CPU_FTR_P9_RADIX_PREFETCH_BUG)) {
		if (!mmu_pid_bits)
			mmu_pid_bits = 20;
		mmu_base_pid = 1;
	} else if (cpu_has_feature(CPU_FTR_HVMODE)) {
		if (!mmu_pid_bits)
			mmu_pid_bits = 20;
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
		/*
		 * When KVM is possible, we only use the top half of the
		 * PID space to avoid collisions between host and guest PIDs
		 * which can cause problems due to prefetch when exiting the
		 * guest with AIL=3
		 */
		mmu_base_pid = 1 << (mmu_pid_bits - 1);
#else
		mmu_base_pid = 1;
#endif
	} else {
		/* The guest uses the bottom half of the PID space */
		if (!mmu_pid_bits)
			mmu_pid_bits = 19;
		mmu_base_pid = 1;
	}

	/*
	 * Allocate Partition table and process table for the
	 * host.
	 */
	BUG_ON(PRTB_SIZE_SHIFT > 36);
	process_tb = early_alloc_pgtable(1UL << PRTB_SIZE_SHIFT, -1, 0, 0);
	/*
	 * Fill in the process table.
	 */
	rts_field = radix__get_tree_size();
	process_tb->prtb0 = cpu_to_be64(rts_field | __pa(init_mm.pgd) | RADIX_PGD_INDEX_SIZE);

	pr_info("Process table %p and radix root for kernel: %p\n", process_tb, init_mm.pgd);

	/*
	 * The init_mm context is given the first available (non-zero) PID,
	 * which is the "guard PID" and contains no page table. PIDR should
	 * never be set to zero because that duplicates the kernel address
	 * space at the 0x0... offset (quadrant 0)!
	 *
	 * An arbitrary PID that may later be allocated by the PID allocator
	 * for userspace processes must not be used either, because that
	 * would cause stale user mappings for that PID on CPUs outside of
	 * the TLB invalidation scheme (because it won't be in mm_cpumask).
	 *
	 * So permanently carve out one PID for the purpose of a guard PID.
	 */
	init_mm.context.id = mmu_base_pid;
	mmu_base_pid++;
}

static void __init radix_init_partition_table(void)
{
	unsigned long rts_field, dw0, dw1;

	mmu_partition_table_init();
	rts_field = radix__get_tree_size();
	dw0 = rts_field | __pa(init_mm.pgd) | RADIX_PGD_INDEX_SIZE | PATB_HR;
	dw1 = __pa(process_tb) | (PRTB_SIZE_SHIFT - 12) | PATB_GR;
	mmu_partition_table_set_entry(0, dw0, dw1, false);

	pr_info("Initializing Radix MMU\n");
	pr_info("Partition table %p\n", partition_tb);
}

static int __init get_idx_from_shift(unsigned int shift)
{
	int idx = -1;

	switch (shift) {
	case 0xc:
		idx = MMU_PAGE_4K;
		break;
	case 0x10:
		idx = MMU_PAGE_64K;
		break;
	case 0x15:
		idx = MMU_PAGE_2M;
		break;
	case 0x1e:
		idx = MMU_PAGE_1G;
		break;
	}
	return idx;
}

static int __init radix_dt_scan_page_sizes(unsigned long node,
					   const char *uname, int depth,
					   void *data)
{
	int size = 0;
	int shift, idx;
	unsigned int ap;
	const __be32 *prop;
	const char *type = of_get_flat_dt_prop(node, "device_type", NULL);

	/* We are scanning "cpu" nodes only */
	if (type == NULL || strcmp(type, "cpu") != 0)
		return 0;

	/* Find MMU PID size */
	prop = of_get_flat_dt_prop(node, "ibm,mmu-pid-bits", &size);
	if (prop && size == 4)
		mmu_pid_bits = be32_to_cpup(prop);

	/* Grab page size encodings */
	prop = of_get_flat_dt_prop(node, "ibm,processor-radix-AP-encodings", &size);
	if (!prop)
		return 0;

	pr_info("Page sizes from device-tree:\n");
	for (; size >= 4; size -= 4, ++prop) {

		struct mmu_psize_def *def;

		/* top 3 bit is AP encoding */
		shift = be32_to_cpu(prop[0]) & ~(0xe << 28);
		ap = be32_to_cpu(prop[0]) >> 29;
		pr_info("Page size shift = %d AP=0x%x\n", shift, ap);

		idx = get_idx_from_shift(shift);
		if (idx < 0)
			continue;

		def = &mmu_psize_defs[idx];
		def->shift = shift;
		def->ap  = ap;
	}

	/* needed ? */
	cur_cpu_spec->mmu_features &= ~MMU_FTR_NO_SLBIE_B;
	return 1;
}

#ifdef CONFIG_MEMORY_HOTPLUG
static int __init probe_memory_block_size(unsigned long node, const char *uname, int
					  depth, void *data)
{
	unsigned long *mem_block_size = (unsigned long *)data;
	const __be64 *prop;
	int len;

	if (depth != 1)
		return 0;

	if (strcmp(uname, "ibm,dynamic-reconfiguration-memory"))
		return 0;

	prop = of_get_flat_dt_prop(node, "ibm,lmb-size", &len);
	if (!prop || len < sizeof(__be64))
		/*
		 * Nothing in the device tree
		 */
		*mem_block_size = MIN_MEMORY_BLOCK_SIZE;
	else
		*mem_block_size = be64_to_cpup(prop);
	return 1;
}

static unsigned long radix_memory_block_size(void)
{
	unsigned long mem_block_size = MIN_MEMORY_BLOCK_SIZE;

	/*
	 * OPAL firmware feature is set by now. Hence we are ok
	 * to test OPAL feature.
	 */
	if (firmware_has_feature(FW_FEATURE_OPAL))
		mem_block_size = 1UL * 1024 * 1024 * 1024;
	else
		of_scan_flat_dt(probe_memory_block_size, &mem_block_size);

	return mem_block_size;
}

#else   /* CONFIG_MEMORY_HOTPLUG */

static unsigned long radix_memory_block_size(void)
{
	return 1UL * 1024 * 1024 * 1024;
}

#endif /* CONFIG_MEMORY_HOTPLUG */


void __init radix__early_init_devtree(void)
{
	int rc;

	/*
	 * Try to find the available page sizes in the device-tree
	 */
	rc = of_scan_flat_dt(radix_dt_scan_page_sizes, NULL);
	if (!rc) {
		/*
		 * No page size details found in device tree.
		 * Let's assume we have page 4k and 64k support
		 */
		mmu_psize_defs[MMU_PAGE_4K].shift = 12;
		mmu_psize_defs[MMU_PAGE_4K].ap = 0x0;

		mmu_psize_defs[MMU_PAGE_64K].shift = 16;
		mmu_psize_defs[MMU_PAGE_64K].ap = 0x5;
	}

	/*
	 * Max mapping size used when mapping pages. We don't use
	 * ppc_md.memory_block_size() here because this get called
	 * early and we don't have machine probe called yet. Also
	 * the pseries implementation only check for ibm,lmb-size.
	 * All hypervisor supporting radix do expose that device
	 * tree node.
	 */
	radix_mem_block_size = radix_memory_block_size();
	return;
}

static void radix_init_amor(void)
{
	/*
	* In HV mode, we init AMOR (Authority Mask Override Register) so that
	* the hypervisor and guest can setup IAMR (Instruction Authority Mask
	* Register), enable key 0 and set it to 1.
	*
	* AMOR = 0b1100 .... 0000 (Mask for key 0 is 11)
	*/
	mtspr(SPRN_AMOR, (3ul << 62));
}

static void radix_init_iamr(void)
{
	/*
	 * Radix always uses key0 of the IAMR to determine if an access is
	 * allowed. We set bit 0 (IBM bit 1) of key0, to prevent instruction
	 * fetch.
	 */
	mtspr(SPRN_IAMR, (1ul << 62));
}

void __init radix__early_init_mmu(void)
{
	unsigned long lpcr;

#ifdef CONFIG_PPC_64K_PAGES
	/* PAGE_SIZE mappings */
	mmu_virtual_psize = MMU_PAGE_64K;
#else
	mmu_virtual_psize = MMU_PAGE_4K;
#endif

#ifdef CONFIG_SPARSEMEM_VMEMMAP
	/* vmemmap mapping */
	if (mmu_psize_defs[MMU_PAGE_2M].shift) {
		/*
		 * map vmemmap using 2M if available
		 */
		mmu_vmemmap_psize = MMU_PAGE_2M;
	} else
		mmu_vmemmap_psize = mmu_virtual_psize;
#endif
	/*
	 * initialize page table size
	 */
	__pte_index_size = RADIX_PTE_INDEX_SIZE;
	__pmd_index_size = RADIX_PMD_INDEX_SIZE;
	__pud_index_size = RADIX_PUD_INDEX_SIZE;
	__pgd_index_size = RADIX_PGD_INDEX_SIZE;
	__pud_cache_index = RADIX_PUD_INDEX_SIZE;
	__pte_table_size = RADIX_PTE_TABLE_SIZE;
	__pmd_table_size = RADIX_PMD_TABLE_SIZE;
	__pud_table_size = RADIX_PUD_TABLE_SIZE;
	__pgd_table_size = RADIX_PGD_TABLE_SIZE;

	__pmd_val_bits = RADIX_PMD_VAL_BITS;
	__pud_val_bits = RADIX_PUD_VAL_BITS;
	__pgd_val_bits = RADIX_PGD_VAL_BITS;

	__kernel_virt_start = RADIX_KERN_VIRT_START;
	__kernel_virt_size = RADIX_KERN_VIRT_SIZE;
	__vmalloc_start = RADIX_VMALLOC_START;
	__vmalloc_end = RADIX_VMALLOC_END;
	__kernel_io_start = RADIX_KERN_IO_START;
	vmemmap = (struct page *)RADIX_VMEMMAP_BASE;
	ioremap_bot = IOREMAP_BASE;

#ifdef CONFIG_PCI
	pci_io_base = ISA_IO_BASE;
#endif
	__pte_frag_nr = RADIX_PTE_FRAG_NR;
	__pte_frag_size_shift = RADIX_PTE_FRAG_SIZE_SHIFT;
	__pmd_frag_nr = RADIX_PMD_FRAG_NR;
	__pmd_frag_size_shift = RADIX_PMD_FRAG_SIZE_SHIFT;

	radix_init_pgtable();

	if (!firmware_has_feature(FW_FEATURE_LPAR)) {
		lpcr = mfspr(SPRN_LPCR);
		mtspr(SPRN_LPCR, lpcr | LPCR_UPRT | LPCR_HR);
		radix_init_partition_table();
		radix_init_amor();
	} else {
		radix_init_pseries();
	}

	memblock_set_current_limit(MEMBLOCK_ALLOC_ANYWHERE);

	radix_init_iamr();
	/* Switch to the guard PID before turning on MMU */
	radix__switch_mmu_context(NULL, &init_mm);
	tlbiel_all();
}

void radix__early_init_mmu_secondary(void)
{
	unsigned long lpcr;
	/*
	 * update partition table control register and UPRT
	 */
	if (!firmware_has_feature(FW_FEATURE_LPAR)) {
		lpcr = mfspr(SPRN_LPCR);
		mtspr(SPRN_LPCR, lpcr | LPCR_UPRT | LPCR_HR);

		set_ptcr_when_no_uv(__pa(partition_tb) |
				    (PATB_SIZE_SHIFT - 12));

		radix_init_amor();
	}
	radix_init_iamr();

	radix__switch_mmu_context(NULL, &init_mm);
	tlbiel_all();
}

/* Called during kexec sequence with MMU off */
notrace void radix__mmu_cleanup_all(void)
{
	unsigned long lpcr;

	if (!firmware_has_feature(FW_FEATURE_LPAR)) {
		lpcr = mfspr(SPRN_LPCR);
		mtspr(SPRN_LPCR, lpcr & ~LPCR_UPRT);
		set_ptcr_when_no_uv(0);
		powernv_set_nmmu_ptcr(0);
		radix__flush_tlb_all();
	}
}

#ifdef CONFIG_MEMORY_HOTPLUG
static void free_pte_table(pte_t *pte_start, pmd_t *pmd)
{
	pte_t *pte;
	int i;

	for (i = 0; i < PTRS_PER_PTE; i++) {
		pte = pte_start + i;
		if (!pte_none(*pte))
			return;
	}

	pte_free_kernel(&init_mm, pte_start);
	pmd_clear(pmd);
}

static void free_pmd_table(pmd_t *pmd_start, pud_t *pud)
{
	pmd_t *pmd;
	int i;

	for (i = 0; i < PTRS_PER_PMD; i++) {
		pmd = pmd_start + i;
		if (!pmd_none(*pmd))
			return;
	}

	pmd_free(&init_mm, pmd_start);
	pud_clear(pud);
}

static void free_pud_table(pud_t *pud_start, pgd_t *pgd)
{
	pud_t *pud;
	int i;

	for (i = 0; i < PTRS_PER_PUD; i++) {
		pud = pud_start + i;
		if (!pud_none(*pud))
			return;
	}

	pud_free(&init_mm, pud_start);
	pgd_clear(pgd);
}

static void remove_pte_table(pte_t *pte_start, unsigned long addr,
			     unsigned long end)
{
	unsigned long next;
	pte_t *pte;

	pte = pte_start + pte_index(addr);
	for (; addr < end; addr = next, pte++) {
		next = (addr + PAGE_SIZE) & PAGE_MASK;
		if (next > end)
			next = end;

		if (!pte_present(*pte))
			continue;

		if (!PAGE_ALIGNED(addr) || !PAGE_ALIGNED(next)) {
			/*
			 * The vmemmap_free() and remove_section_mapping()
			 * codepaths call us with aligned addresses.
			 */
			WARN_ONCE(1, "%s: unaligned range\n", __func__);
			continue;
		}

		pte_clear(&init_mm, addr, pte);
	}
}

static void remove_pmd_table(pmd_t *pmd_start, unsigned long addr,
			     unsigned long end)
{
	unsigned long next;
	pte_t *pte_base;
	pmd_t *pmd;

	pmd = pmd_start + pmd_index(addr);
	for (; addr < end; addr = next, pmd++) {
		next = pmd_addr_end(addr, end);

		if (!pmd_present(*pmd))
			continue;

		if (pmd_is_leaf(*pmd)) {
			if (!IS_ALIGNED(addr, PMD_SIZE) ||
			    !IS_ALIGNED(next, PMD_SIZE)) {
				WARN_ONCE(1, "%s: unaligned range\n", __func__);
				continue;
			}
			pte_clear(&init_mm, addr, (pte_t *)pmd);
			continue;
		}

		pte_base = (pte_t *)pmd_page_vaddr(*pmd);
		remove_pte_table(pte_base, addr, next);
		free_pte_table(pte_base, pmd);
	}
}

static void remove_pud_table(pud_t *pud_start, unsigned long addr,
			     unsigned long end)
{
	unsigned long next;
	pmd_t *pmd_base;
	pud_t *pud;

	pud = pud_start + pud_index(addr);
	for (; addr < end; addr = next, pud++) {
		next = pud_addr_end(addr, end);

		if (!pud_present(*pud))
			continue;

		if (pud_is_leaf(*pud)) {
			if (!IS_ALIGNED(addr, PUD_SIZE) ||
			    !IS_ALIGNED(next, PUD_SIZE)) {
				WARN_ONCE(1, "%s: unaligned range\n", __func__);
				continue;
			}
			pte_clear(&init_mm, addr, (pte_t *)pud);
			continue;
		}

		pmd_base = (pmd_t *)pud_page_vaddr(*pud);
		remove_pmd_table(pmd_base, addr, next);
		free_pmd_table(pmd_base, pud);
	}
}

static void __meminit remove_pagetable(unsigned long start, unsigned long end)
{
	unsigned long addr, next;
	pud_t *pud_base;
	pgd_t *pgd;

	spin_lock(&init_mm.page_table_lock);

	for (addr = start; addr < end; addr = next) {
		next = pgd_addr_end(addr, end);

		pgd = pgd_offset_k(addr);
		if (!pgd_present(*pgd))
			continue;

		if (pgd_is_leaf(*pgd)) {
			if (!IS_ALIGNED(addr, PGDIR_SIZE) ||
			    !IS_ALIGNED(next, PGDIR_SIZE)) {
				WARN_ONCE(1, "%s: unaligned range\n", __func__);
				continue;
			}

			pte_clear(&init_mm, addr, (pte_t *)pgd);
			continue;
		}

		pud_base = (pud_t *)pgd_page_vaddr(*pgd);
		remove_pud_table(pud_base, addr, next);
		free_pud_table(pud_base, pgd);
	}

	spin_unlock(&init_mm.page_table_lock);
	radix__flush_tlb_kernel_range(start, end);
}

int __meminit radix__create_section_mapping(unsigned long start,
					    unsigned long end, int nid,
					    pgprot_t prot)
{
	if (end >= RADIX_VMALLOC_START) {
		pr_warn("Outside the supported range\n");
		return -1;
	}

	return create_physical_mapping(__pa(start), __pa(end),
				       radix_mem_block_size, nid, prot);
}

int __meminit radix__remove_section_mapping(unsigned long start, unsigned long end)
{
	remove_pagetable(start, end);
	return 0;
}
#endif /* CONFIG_MEMORY_HOTPLUG */

#ifdef CONFIG_SPARSEMEM_VMEMMAP
static int __map_kernel_page_nid(unsigned long ea, unsigned long pa,
				 pgprot_t flags, unsigned int map_page_size,
				 int nid)
{
	return __map_kernel_page(ea, pa, flags, map_page_size, nid, 0, 0);
}

int __meminit radix__vmemmap_create_mapping(unsigned long start,
				      unsigned long page_size,
				      unsigned long phys)
{
	/* Create a PTE encoding */
	unsigned long flags = _PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_KERNEL_RW;
	int nid = early_pfn_to_nid(phys >> PAGE_SHIFT);
	int ret;

	ret = __map_kernel_page_nid(start, phys, __pgprot(flags), page_size, nid);
	BUG_ON(ret);

	return 0;
}

#ifdef CONFIG_MEMORY_HOTPLUG
void __meminit radix__vmemmap_remove_mapping(unsigned long start, unsigned long page_size)
{
	remove_pagetable(start, start + page_size);
}
#endif
#endif

#ifdef CONFIG_TRANSPARENT_HUGEPAGE

unsigned long radix__pmd_hugepage_update(struct mm_struct *mm, unsigned long addr,
				  pmd_t *pmdp, unsigned long clr,
				  unsigned long set)
{
	unsigned long old;

#ifdef CONFIG_DEBUG_VM
	WARN_ON(!radix__pmd_trans_huge(*pmdp) && !pmd_devmap(*pmdp));
	assert_spin_locked(pmd_lockptr(mm, pmdp));
#endif

	old = radix__pte_update(mm, addr, (pte_t *)pmdp, clr, set, 1);
	trace_hugepage_update(addr, old, clr, set);

	return old;
}

pmd_t radix__pmdp_collapse_flush(struct vm_area_struct *vma, unsigned long address,
			pmd_t *pmdp)

{
	pmd_t pmd;

	VM_BUG_ON(address & ~HPAGE_PMD_MASK);
	VM_BUG_ON(radix__pmd_trans_huge(*pmdp));
	VM_BUG_ON(pmd_devmap(*pmdp));
	/*
	 * khugepaged calls this for normal pmd
	 */
	pmd = *pmdp;
	pmd_clear(pmdp);

	/*
	 * pmdp collapse_flush need to ensure that there are no parallel gup
	 * walk after this call. This is needed so that we can have stable
	 * page ref count when collapsing a page. We don't allow a collapse page
	 * if we have gup taken on the page. We can ensure that by sending IPI
	 * because gup walk happens with IRQ disabled.
	 */
	serialize_against_pte_lookup(vma->vm_mm);

	radix__flush_tlb_collapsed_pmd(vma->vm_mm, address);

	return pmd;
}

/*
 * For us pgtable_t is pte_t *. Inorder to save the deposisted
 * page table, we consider the allocated page table as a list
 * head. On withdraw we need to make sure we zero out the used
 * list_head memory area.
 */
void radix__pgtable_trans_huge_deposit(struct mm_struct *mm, pmd_t *pmdp,
				 pgtable_t pgtable)
{
	struct list_head *lh = (struct list_head *) pgtable;

	assert_spin_locked(pmd_lockptr(mm, pmdp));

	/* FIFO */
	if (!pmd_huge_pte(mm, pmdp))
		INIT_LIST_HEAD(lh);
	else
		list_add(lh, (struct list_head *) pmd_huge_pte(mm, pmdp));
	pmd_huge_pte(mm, pmdp) = pgtable;
}

pgtable_t radix__pgtable_trans_huge_withdraw(struct mm_struct *mm, pmd_t *pmdp)
{
	pte_t *ptep;
	pgtable_t pgtable;
	struct list_head *lh;

	assert_spin_locked(pmd_lockptr(mm, pmdp));

	/* FIFO */
	pgtable = pmd_huge_pte(mm, pmdp);
	lh = (struct list_head *) pgtable;
	if (list_empty(lh))
		pmd_huge_pte(mm, pmdp) = NULL;
	else {
		pmd_huge_pte(mm, pmdp) = (pgtable_t) lh->next;
		list_del(lh);
	}
	ptep = (pte_t *) pgtable;
	*ptep = __pte(0);
	ptep++;
	*ptep = __pte(0);
	return pgtable;
}

pmd_t radix__pmdp_huge_get_and_clear(struct mm_struct *mm,
				     unsigned long addr, pmd_t *pmdp)
{
	pmd_t old_pmd;
	unsigned long old;

	old = radix__pmd_hugepage_update(mm, addr, pmdp, ~0UL, 0);
	old_pmd = __pmd(old);
	return old_pmd;
}

int radix__has_transparent_hugepage(void)
{
	/* For radix 2M at PMD level means thp */
	if (mmu_psize_defs[MMU_PAGE_2M].shift == PMD_SHIFT)
		return 1;
	return 0;
}
#endif /* CONFIG_TRANSPARENT_HUGEPAGE */

void radix__ptep_set_access_flags(struct vm_area_struct *vma, pte_t *ptep,
				  pte_t entry, unsigned long address, int psize)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long set = pte_val(entry) & (_PAGE_DIRTY | _PAGE_ACCESSED |
					      _PAGE_RW | _PAGE_EXEC);

	unsigned long change = pte_val(entry) ^ pte_val(*ptep);
	/*
	 * To avoid NMMU hang while relaxing access, we need mark
	 * the pte invalid in between.
	 */
	if ((change & _PAGE_RW) && atomic_read(&mm->context.copros) > 0) {
		unsigned long old_pte, new_pte;

		old_pte = __radix_pte_update(ptep, _PAGE_PRESENT, _PAGE_INVALID);
		/*
		 * new value of pte
		 */
		new_pte = old_pte | set;
		radix__flush_tlb_page_psize(mm, address, psize);
		__radix_pte_update(ptep, _PAGE_INVALID, new_pte);
	} else {
		__radix_pte_update(ptep, 0, set);
		/*
		 * Book3S does not require a TLB flush when relaxing access
		 * restrictions when the address space is not attached to a
		 * NMMU, because the core MMU will reload the pte after taking
		 * an access fault, which is defined by the architectue.
		 */
	}
	/* See ptesync comment in radix__set_pte_at */
}
