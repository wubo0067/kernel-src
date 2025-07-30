/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_PAGE_64_DEFS_H
#define _ASM_X86_PAGE_64_DEFS_H

#ifndef __ASSEMBLY__
#include <asm/kaslr.h>
#endif

#ifdef CONFIG_KASAN
#define KASAN_STACK_ORDER 1
#else
#define KASAN_STACK_ORDER 0
#endif

#define THREAD_SIZE_ORDER (2 + KASAN_STACK_ORDER)
#define THREAD_SIZE (PAGE_SIZE << THREAD_SIZE_ORDER)

#define EXCEPTION_STACK_ORDER (0 + KASAN_STACK_ORDER)
#define EXCEPTION_STKSZ (PAGE_SIZE << EXCEPTION_STACK_ORDER)

#define IRQ_STACK_ORDER (2 + KASAN_STACK_ORDER)
#define IRQ_STACK_SIZE (PAGE_SIZE << IRQ_STACK_ORDER)

/*
 * The index for the tss.ist[] array. The hardware limit is 7 entries.
 */
#define IST_INDEX_DF 0
#define IST_INDEX_NMI 1
#define IST_INDEX_DB 2
#define IST_INDEX_MCE 3
#define IST_INDEX_VC 4

/*
 * Set __PAGE_OFFSET to the most negative possible address +
 * PGDIR_SIZE*17 (pgd slot 273).
 *
 * The gap is to allow a space for LDT remap for PTI (1 pgd slot) and space for
 * a hypervisor (16 slots). Choosing 16 slots for a hypervisor is arbitrary,
 * but it's what Xen requires.
 */
#define __PAGE_OFFSET_BASE_L5 _AC(0xff11000000000000, UL)
#define __PAGE_OFFSET_BASE_L4 _AC(0xffff888000000000, UL)

/*
    它定义了内核虚拟地址空间中“直接内存映射区域”的起始地址

    直接内存映射（Direct Memory Mapping）：内核需要一种高效的方式来访问系统中的所有物理内存。
        最简单的方法就是将全部（或大部分）物理内存连续地映射到内核虚拟地址空间的一个区域。
        这个区域就叫“直接内存映射区”或“线性映射区”。

    在这个区域里，虚拟地址和物理地址之间存在一个固定的偏移量（offset）。
        这个固定的偏移量就是 __PAGE_OFFSET。

        转换关系非常简单：
        虚拟地址 = 物理地址 + __PAGE_OFFSET
        物理地址 = 虚拟地址 - __PAGE_OFFSET
    内核中著名的 __pa() (physical address) 和 __va() (virtual address) 宏就是利用
        __PAGE_OFFSET 来进行这种快速转换的。
    所以，__PAGE_OFFSET 的值，就标志着内核空间中这片巨大、重要的直接映射区域从哪里开始。
        任何高于 __PAGE_OFFSET 的地址都被认为是内核地址。
*/
#ifdef CONFIG_DYNAMIC_MEMORY_LAYOUT
#define __PAGE_OFFSET page_offset_base
#else
#define __PAGE_OFFSET __PAGE_OFFSET_BASE_L4
#endif /* CONFIG_DYNAMIC_MEMORY_LAYOUT */

#define __START_KERNEL_map _AC(0xffffffff80000000, UL)

/* See Documentation/x86/x86_64/mm.txt for a description of the memory map. */

#define __PHYSICAL_MASK_SHIFT 52

#ifdef CONFIG_X86_5LEVEL
#define __VIRTUAL_MASK_SHIFT (pgtable_l5_enabled() ? 56 : 47)
#else
#define __VIRTUAL_MASK_SHIFT 47
#endif

/*
 * Kernel image size is limited to 1GiB due to the fixmap living in the
 * next 1GiB (see level2_kernel_pgt in arch/x86/kernel/head_64.S). Use
 * 512MiB by default, leaving 1.5GiB for modules once the page tables
 * are fully set up. If kernel ASLR is configured, it can extend the
 * kernel page table mapping, reducing the size of the modules area.
 */
#if defined(CONFIG_RANDOMIZE_BASE)
#define KERNEL_IMAGE_SIZE (1024 * 1024 * 1024)
#else
#define KERNEL_IMAGE_SIZE (512 * 1024 * 1024)
#endif

#endif /* _ASM_X86_PAGE_64_DEFS_H */
