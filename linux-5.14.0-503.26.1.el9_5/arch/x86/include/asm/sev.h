/* SPDX-License-Identifier: GPL-2.0 */
/*
 * AMD Encrypted Register State Support
 *
 * Author: Joerg Roedel <jroedel@suse.de>
 */

#ifndef __ASM_ENCRYPTED_STATE_H
#define __ASM_ENCRYPTED_STATE_H

#include <linux/types.h>
#include <linux/sev-guest.h>

#include <asm/insn.h>
#include <asm/sev-common.h>
#include <asm/bootparam.h>
#include <asm/coco.h>

#define GHCB_PROTOCOL_MIN	1ULL
#define GHCB_PROTOCOL_MAX	2ULL
#define GHCB_DEFAULT_USAGE	0ULL

#define	VMGEXIT()			{ asm volatile("rep; vmmcall\n\r"); }

enum es_result {
	ES_OK,			/* All good */
	ES_UNSUPPORTED,		/* Requested operation not supported */
	ES_VMM_ERROR,		/* Unexpected state from the VMM */
	ES_DECODE_FAILED,	/* Instruction decoding failed */
	ES_EXCEPTION,		/* Instruction caused exception */
	ES_RETRY,		/* Retry instruction emulation */
};

struct es_fault_info {
	unsigned long vector;
	unsigned long error_code;
	unsigned long cr2;
};

struct pt_regs;

/* ES instruction emulation context */
struct es_em_ctxt {
	struct pt_regs *regs;
	struct insn insn;
	struct es_fault_info fi;
};

/*
 * AMD SEV Confidential computing blob structure. The structure is
 * defined in OVMF UEFI firmware header:
 * https://github.com/tianocore/edk2/blob/master/OvmfPkg/Include/Guid/ConfidentialComputingSevSnpBlob.h
 */
#define CC_BLOB_SEV_HDR_MAGIC	0x45444d41
struct cc_blob_sev_info {
	u32 magic;
	u16 version;
	u16 reserved;
	u64 secrets_phys;
	u32 secrets_len;
	u32 rsvd1;
	u64 cpuid_phys;
	u32 cpuid_len;
	u32 rsvd2;
} __packed;

void do_vc_no_ghcb(struct pt_regs *regs, unsigned long exit_code);

static inline u64 lower_bits(u64 val, unsigned int bits)
{
	u64 mask = (1ULL << bits) - 1;

	return (val & mask);
}

struct real_mode_header;
enum stack_type;

/* Early IDT entry points for #VC handler */
extern void vc_no_ghcb(void);
extern void vc_boot_ghcb(void);
extern bool handle_vc_boot_ghcb(struct pt_regs *regs);

/* PVALIDATE return codes */
#define PVALIDATE_FAIL_SIZEMISMATCH	6

/* Software defined (when rFlags.CF = 1) */
#define PVALIDATE_FAIL_NOUPDATE		255

/* RMUPDATE detected 4K page and 2MB page overlap. */
#define RMPUPDATE_FAIL_OVERLAP		4

/* PSMASH failed due to concurrent access by another CPU */
#define PSMASH_FAIL_INUSE		3

/* RMP page size */
#define RMP_PG_SIZE_4K			0
#define RMP_PG_SIZE_2M			1
#define RMP_TO_PG_LEVEL(level)		(((level) == RMP_PG_SIZE_4K) ? PG_LEVEL_4K : PG_LEVEL_2M)
#define PG_LEVEL_TO_RMP(level)		(((level) == PG_LEVEL_4K) ? RMP_PG_SIZE_4K : RMP_PG_SIZE_2M)

struct rmp_state {
	u64 gpa;
	u8 assigned;
	u8 pagesize;
	u8 immutable;
	u8 rsvd;
	u32 asid;
} __packed;

#define RMPADJUST_VMSA_PAGE_BIT		BIT(16)

/* SNP Guest message request */
struct snp_req_data {
	unsigned long req_gpa;
	unsigned long resp_gpa;
	unsigned long data_gpa;
	unsigned int data_npages;
};

#define MAX_AUTHTAG_LEN		32

/* See SNP spec SNP_GUEST_REQUEST section for the structure */
enum msg_type {
	SNP_MSG_TYPE_INVALID = 0,
	SNP_MSG_CPUID_REQ,
	SNP_MSG_CPUID_RSP,
	SNP_MSG_KEY_REQ,
	SNP_MSG_KEY_RSP,
	SNP_MSG_REPORT_REQ,
	SNP_MSG_REPORT_RSP,
	SNP_MSG_EXPORT_REQ,
	SNP_MSG_EXPORT_RSP,
	SNP_MSG_IMPORT_REQ,
	SNP_MSG_IMPORT_RSP,
	SNP_MSG_ABSORB_REQ,
	SNP_MSG_ABSORB_RSP,
	SNP_MSG_VMRK_REQ,
	SNP_MSG_VMRK_RSP,

	SNP_MSG_TYPE_MAX
};

enum aead_algo {
	SNP_AEAD_INVALID,
	SNP_AEAD_AES_256_GCM,
};

struct snp_guest_msg_hdr {
	u8 authtag[MAX_AUTHTAG_LEN];
	u64 msg_seqno;
	u8 rsvd1[8];
	u8 algo;
	u8 hdr_version;
	u16 hdr_sz;
	u8 msg_type;
	u8 msg_version;
	u16 msg_sz;
	u32 rsvd2;
	u8 msg_vmpck;
	u8 rsvd3[35];
} __packed;

struct snp_guest_msg {
	struct snp_guest_msg_hdr hdr;
	u8 payload[4000];
} __packed;

struct sev_guest_platform_data {
	u64 secrets_gpa;
};

/*
 * The secrets page contains 96-bytes of reserved field that can be used by
 * the guest OS. The guest OS uses the area to save the message sequence
 * number for each VMPCK.
 *
 * See the GHCB spec section Secret page layout for the format for this area.
 */
struct secrets_os_area {
	u32 msg_seqno_0;
	u32 msg_seqno_1;
	u32 msg_seqno_2;
	u32 msg_seqno_3;
	u64 ap_jump_table_pa;
	u8 rsvd[40];
	u8 guest_usage[32];
} __packed;

#define VMPCK_KEY_LEN		32

/* See the SNP spec version 0.9 for secrets page format */
struct snp_secrets_page_layout {
	u32 version;
	u32 imien	: 1,
	    rsvd1	: 31;
	u32 fms;
	u32 rsvd2;
	u8 gosvw[16];
	u8 vmpck0[VMPCK_KEY_LEN];
	u8 vmpck1[VMPCK_KEY_LEN];
	u8 vmpck2[VMPCK_KEY_LEN];
	u8 vmpck3[VMPCK_KEY_LEN];
	struct secrets_os_area os_area;
	u8 rsvd3[3840];
} __packed;

#ifdef CONFIG_AMD_MEM_ENCRYPT
extern void __sev_es_ist_enter(struct pt_regs *regs);
extern void __sev_es_ist_exit(void);
static __always_inline void sev_es_ist_enter(struct pt_regs *regs)
{
	if (cc_vendor == CC_VENDOR_AMD &&
	    cc_platform_has(CC_ATTR_GUEST_STATE_ENCRYPT))
		__sev_es_ist_enter(regs);
}
static __always_inline void sev_es_ist_exit(void)
{
	if (cc_vendor == CC_VENDOR_AMD &&
	    cc_platform_has(CC_ATTR_GUEST_STATE_ENCRYPT))
		__sev_es_ist_exit();
}
extern int sev_es_setup_ap_jump_table(struct real_mode_header *rmh);
extern void __sev_es_nmi_complete(void);
static __always_inline void sev_es_nmi_complete(void)
{
	if (cc_vendor == CC_VENDOR_AMD &&
	    cc_platform_has(CC_ATTR_GUEST_STATE_ENCRYPT))
		__sev_es_nmi_complete();
}
extern int __init sev_es_efi_map_ghcbs(pgd_t *pgd);
extern void sev_enable(struct boot_params *bp);

static inline int rmpadjust(unsigned long vaddr, bool rmp_psize, unsigned long attrs)
{
	int rc;

	/* "rmpadjust" mnemonic support in binutils 2.36 and newer */
	asm volatile(".byte 0xF3,0x0F,0x01,0xFE\n\t"
		     : "=a"(rc)
		     : "a"(vaddr), "c"(rmp_psize), "d"(attrs)
		     : "memory", "cc");

	return rc;
}
static inline int pvalidate(unsigned long vaddr, bool rmp_psize, bool validate)
{
	bool no_rmpupdate;
	int rc;

	/* "pvalidate" mnemonic support in binutils 2.36 and newer */
	asm volatile(".byte 0xF2, 0x0F, 0x01, 0xFF\n\t"
		     CC_SET(c)
		     : CC_OUT(c) (no_rmpupdate), "=a"(rc)
		     : "a"(vaddr), "c"(rmp_psize), "d"(validate)
		     : "memory", "cc");

	if (no_rmpupdate)
		return PVALIDATE_FAIL_NOUPDATE;

	return rc;
}

struct snp_guest_request_ioctl;

void setup_ghcb(void);
void early_snp_set_memory_private(unsigned long vaddr, unsigned long paddr,
				  unsigned long npages);
void early_snp_set_memory_shared(unsigned long vaddr, unsigned long paddr,
				 unsigned long npages);
void snp_set_memory_shared(unsigned long vaddr, unsigned long npages);
void snp_set_memory_private(unsigned long vaddr, unsigned long npages);
void snp_set_wakeup_secondary_cpu(void);
bool snp_init(struct boot_params *bp);
void __noreturn snp_abort(void);
void snp_dmi_setup(void);
int snp_issue_guest_request(u64 exit_code, struct snp_req_data *input, struct snp_guest_request_ioctl *rio);
void snp_accept_memory(phys_addr_t start, phys_addr_t end);
u64 snp_get_unsupported_features(u64 status);
u64 sev_get_status(void);
void sev_show_status(void);
#else
static inline void sev_es_ist_enter(struct pt_regs *regs) { }
static inline void sev_es_ist_exit(void) { }
static inline int sev_es_setup_ap_jump_table(struct real_mode_header *rmh) { return 0; }
static inline void sev_es_nmi_complete(void) { }
static inline int sev_es_efi_map_ghcbs(pgd_t *pgd) { return 0; }
static inline void sev_enable(struct boot_params *bp) { }
static inline int pvalidate(unsigned long vaddr, bool rmp_psize, bool validate) { return 0; }
static inline int rmpadjust(unsigned long vaddr, bool rmp_psize, unsigned long attrs) { return 0; }
static inline void setup_ghcb(void) { }
static inline void __init
early_snp_set_memory_private(unsigned long vaddr, unsigned long paddr, unsigned long npages) { }
static inline void __init
early_snp_set_memory_shared(unsigned long vaddr, unsigned long paddr, unsigned long npages) { }
static inline void snp_set_memory_shared(unsigned long vaddr, unsigned long npages) { }
static inline void snp_set_memory_private(unsigned long vaddr, unsigned long npages) { }
static inline void snp_set_wakeup_secondary_cpu(void) { }
static inline bool snp_init(struct boot_params *bp) { return false; }
static inline void snp_abort(void) { }
static inline void snp_dmi_setup(void) { }
static inline int snp_issue_guest_request(u64 exit_code, struct snp_req_data *input, struct snp_guest_request_ioctl *rio)
{
	return -ENOTTY;
}

static inline void snp_accept_memory(phys_addr_t start, phys_addr_t end) { }
static inline u64 snp_get_unsupported_features(u64 status) { return 0; }
static inline u64 sev_get_status(void) { return 0; }
static inline void sev_show_status(void) { }
#endif

#ifdef CONFIG_KVM_AMD_SEV
bool snp_probe_rmptable_info(void);
int snp_lookup_rmpentry(u64 pfn, bool *assigned, int *level);
void snp_dump_hva_rmpentry(unsigned long address);
int psmash(u64 pfn);
int rmp_make_private(u64 pfn, u64 gpa, enum pg_level level, u32 asid, bool immutable);
int rmp_make_shared(u64 pfn, enum pg_level level);
void snp_leak_pages(u64 pfn, unsigned int npages);
void kdump_sev_callback(void);
void snp_fixup_e820_tables(void);
#else
static inline bool snp_probe_rmptable_info(void) { return false; }
static inline int snp_lookup_rmpentry(u64 pfn, bool *assigned, int *level) { return -ENODEV; }
static inline void snp_dump_hva_rmpentry(unsigned long address) {}
static inline int psmash(u64 pfn) { return -ENODEV; }
static inline int rmp_make_private(u64 pfn, u64 gpa, enum pg_level level, u32 asid,
				   bool immutable)
{
	return -ENODEV;
}
static inline int rmp_make_shared(u64 pfn, enum pg_level level) { return -ENODEV; }
static inline void snp_leak_pages(u64 pfn, unsigned int npages) {}
static inline void kdump_sev_callback(void) { }
static inline void snp_fixup_e820_tables(void) {}
#endif

#endif
