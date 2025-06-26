	.file	"devicetable-offsets.c"
# GNU C89 (GCC) version 8.5.0 20210514 (Red Hat 8.5.0-26.0.1) (x86_64-redhat-linux)
#	compiled by GNU C version 8.5.0 20210514 (Red Hat 8.5.0-26.0.1), GMP version 6.1.2, MPFR version 3.1.6-p2, MPC version 1.1.0, isl version none
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  -nostdinc -I ./arch/x86/include
# -I ./arch/x86/include/generated -I ./include/drm-backport -I ./include
# -I ./arch/x86/include/uapi -I ./arch/x86/include/generated/uapi
# -I ./include/uapi -I ./include/generated/uapi -D __KERNEL__
# -D CC_HAVE_ASM_GOTO -D CONFIG_AS_CFI=1 -D CONFIG_AS_CFI_SIGNAL_FRAME=1
# -D CONFIG_AS_CFI_SECTIONS=1 -D CONFIG_AS_FXSAVEQ=1 -D CONFIG_AS_SSSE3=1
# -D CONFIG_AS_CRC32=1 -D CONFIG_AS_AVX=1 -D CONFIG_AS_AVX2=1
# -D CONFIG_AS_AVX512=1 -D CONFIG_AS_SHA1_NI=1 -D CONFIG_AS_SHA256_NI=1
# -D CONFIG_TPAUSE=1 -D CC_USING_FENTRY
# -D KBUILD_BASENAME="devicetable_offsets"
# -D KBUILD_MODNAME="devicetable_offsets"
# -isystem /usr/lib/gcc/x86_64-redhat-linux/8/include
# -include ./include/linux/kconfig.h
# -include ./include/linux/compiler_types.h
# -MD scripts/mod/.devicetable-offsets.s.d
# scripts/mod/devicetable-offsets.c -mno-sse -mno-mmx -mno-sse2 -mno-3dnow
# -mno-avx -m64 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3
# -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel
# -mindirect-branch=thunk-extern -mindirect-branch-register -mrecord-mcount
# -mfentry -march=x86-64 -auxbase-strip scripts/mod/devicetable-offsets.s
# -g -gdwarf-4 -O2 -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs
# -Werror=implicit-function-declaration -Wno-format-security
# -Wno-sign-compare -Wno-frame-address -Wformat-truncation=0
# -Wformat-overflow=0 -Wno-int-in-bool-context -Werror
# -Wframe-larger-than=2048 -Wno-unused-but-set-variable
# -Wunused-const-variable=0 -Wdeclaration-after-statement -Wno-pointer-sign
# -Wno-stringop-truncation -Werror=implicit-int -Werror=strict-prototypes
# -Werror=date-time -Werror=incompatible-pointer-types
# -Werror=designated-init -Wno-packed-not-aligned -std=gnu90 -p
# -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -falign-jumps=1
# -falign-loops=1 -funit-at-a-time -fno-asynchronous-unwind-tables
# -fno-jump-tables -fno-delete-null-pointer-checks -fno-reorder-blocks
# -fno-ipa-cp-clone -fno-partial-inlining -fstack-protector-strong
# -fno-inline-functions-called-once -fno-strict-overflow
# -fno-merge-all-constants -fmerge-constants -fstack-check=no
# -fconserve-stack -fmacro-prefix-map=./= -fverbose-asm
# --param allow-store-data-races=0
# options enabled:  -faggressive-loop-optimizations -falign-labels
# -fauto-inc-dec -fbranch-count-reg -fcaller-saves
# -fchkp-check-incomplete-type -fchkp-check-read -fchkp-check-write
# -fchkp-instrument-calls -fchkp-narrow-bounds -fchkp-optimize
# -fchkp-store-bounds -fchkp-use-static-bounds
# -fchkp-use-static-const-bounds -fchkp-use-wrappers -fcode-hoisting
# -fcombine-stack-adjustments -fcompare-elim -fcprop-registers
# -fcrossjumping -fcse-follow-jumps -fdefer-pop -fdevirtualize
# -fdevirtualize-speculatively -fdwarf2-cfi-asm -fearly-inlining
# -feliminate-unused-debug-types -fexpensive-optimizations
# -fforward-propagate -ffp-int-builtin-inexact -ffunction-cse -fgcse
# -fgcse-lm -fgnu-runtime -fgnu-unique -fguess-branch-probability
# -fhoist-adjacent-loads -fident -fif-conversion -fif-conversion2
# -findirect-inlining -finline -finline-atomics -finline-small-functions
# -fipa-bit-cp -fipa-cp -fipa-icf -fipa-icf-functions -fipa-icf-variables
# -fipa-profile -fipa-pure-const -fipa-reference
# -fipa-reference-addressable -fipa-sra -fipa-stack-alignment -fipa-vrp
# -fira-hoist-pressure -fira-share-save-slots -fira-share-spill-slots
# -fisolate-erroneous-paths-dereference -fivopts -fkeep-static-consts
# -fleading-underscore -flifetime-dse -flra-remat -flto-odr-type-merging
# -fmath-errno -fmerge-constants -fmerge-debug-strings
# -fmove-loop-invariants -fomit-frame-pointer -foptimize-sibling-calls
# -foptimize-strlen -fpeephole -fpeephole2 -fplt -fprefetch-loop-arrays
# -fprofile -free -freg-struct-return -freorder-blocks-and-partition
# -freorder-functions -frerun-cse-after-loop
# -fsched-critical-path-heuristic -fsched-dep-count-heuristic
# -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
# -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
# -fsched-stalled-insns-dep -fschedule-fusion -fschedule-insns2
# -fsemantic-interposition -fshow-column -fshrink-wrap
# -fshrink-wrap-separate -fsigned-zeros -fsplit-ivs-in-unroller
# -fsplit-wide-types -fssa-backprop -fssa-phiopt -fstack-protector-strong
# -fstdarg-opt -fstore-merging -fstrict-volatile-bitfields -fsync-libcalls
# -fthread-jumps -ftoplevel-reorder -ftrapping-math -ftree-bit-ccp
# -ftree-builtin-call-dce -ftree-ccp -ftree-ch -ftree-coalesce-vars
# -ftree-copy-prop -ftree-cselim -ftree-dce -ftree-dominator-opts
# -ftree-dse -ftree-forwprop -ftree-fre -ftree-loop-if-convert
# -ftree-loop-im -ftree-loop-ivcanon -ftree-loop-optimize
# -ftree-parallelize-loops= -ftree-phiprop -ftree-pre -ftree-pta
# -ftree-reassoc -ftree-scev-cprop -ftree-sink -ftree-slsr -ftree-sra
# -ftree-switch-conversion -ftree-tail-merge -ftree-ter -ftree-vrp
# -funit-at-a-time -fvar-tracking -fvar-tracking-assignments -fverbose-asm
# -fwrapv -fwrapv-pointer -fzero-initialized-in-bss -m128bit-long-double
# -m64 -malign-stringops -mavx256-split-unaligned-load
# -mavx256-split-unaligned-store -mfentry -mfxsr -mglibc -mieee-fp
# -mindirect-branch-register -mlong-double-80 -mno-fancy-math-387
# -mno-red-zone -mno-sse4 -mpush-args -mrecord-mcount -mskip-rax-setup
# -mtls-direct-seg-refs -mvzeroupper

	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB71:
	.file 1 "scripts/mod/devicetable-offsets.c"
	.loc 1 10 1 view -0
	.cfi_startproc
1:	call	__fentry__
	.section __mcount_loc, "a",@progbits
	.quad 1b
	.previous
	.loc 1 11 2 view .LVU1
#APP
# 11 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_usb_device_id $32 sizeof(struct usb_device_id)"	#
# 0 "" 2
	.loc 1 12 2 view .LVU2
# 12 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_match_flags $0 offsetof(struct usb_device_id, match_flags)"	#
# 0 "" 2
	.loc 1 13 2 view .LVU3
# 13 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_idVendor $2 offsetof(struct usb_device_id, idVendor)"	#
# 0 "" 2
	.loc 1 14 2 view .LVU4
# 14 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_idProduct $4 offsetof(struct usb_device_id, idProduct)"	#
# 0 "" 2
	.loc 1 15 2 view .LVU5
# 15 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bcdDevice_lo $6 offsetof(struct usb_device_id, bcdDevice_lo)"	#
# 0 "" 2
	.loc 1 16 2 view .LVU6
# 16 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bcdDevice_hi $8 offsetof(struct usb_device_id, bcdDevice_hi)"	#
# 0 "" 2
	.loc 1 17 2 view .LVU7
# 17 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bDeviceClass $10 offsetof(struct usb_device_id, bDeviceClass)"	#
# 0 "" 2
	.loc 1 18 2 view .LVU8
# 18 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bDeviceSubClass $11 offsetof(struct usb_device_id, bDeviceSubClass)"	#
# 0 "" 2
	.loc 1 19 2 view .LVU9
# 19 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bDeviceProtocol $12 offsetof(struct usb_device_id, bDeviceProtocol)"	#
# 0 "" 2
	.loc 1 20 2 view .LVU10
# 20 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bInterfaceClass $13 offsetof(struct usb_device_id, bInterfaceClass)"	#
# 0 "" 2
	.loc 1 21 2 view .LVU11
# 21 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bInterfaceSubClass $14 offsetof(struct usb_device_id, bInterfaceSubClass)"	#
# 0 "" 2
	.loc 1 22 2 view .LVU12
# 22 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bInterfaceProtocol $15 offsetof(struct usb_device_id, bInterfaceProtocol)"	#
# 0 "" 2
	.loc 1 23 2 view .LVU13
# 23 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_usb_device_id_bInterfaceNumber $16 offsetof(struct usb_device_id, bInterfaceNumber)"	#
# 0 "" 2
	.loc 1 25 2 view .LVU14
# 25 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_hid_device_id $24 sizeof(struct hid_device_id)"	#
# 0 "" 2
	.loc 1 26 2 view .LVU15
# 26 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hid_device_id_bus $0 offsetof(struct hid_device_id, bus)"	#
# 0 "" 2
	.loc 1 27 2 view .LVU16
# 27 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hid_device_id_group $2 offsetof(struct hid_device_id, group)"	#
# 0 "" 2
	.loc 1 28 2 view .LVU17
# 28 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hid_device_id_vendor $4 offsetof(struct hid_device_id, vendor)"	#
# 0 "" 2
	.loc 1 29 2 view .LVU18
# 29 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hid_device_id_product $8 offsetof(struct hid_device_id, product)"	#
# 0 "" 2
	.loc 1 31 2 view .LVU19
# 31 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_ieee1394_device_id $32 sizeof(struct ieee1394_device_id)"	#
# 0 "" 2
	.loc 1 32 2 view .LVU20
# 32 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ieee1394_device_id_match_flags $0 offsetof(struct ieee1394_device_id, match_flags)"	#
# 0 "" 2
	.loc 1 33 2 view .LVU21
# 33 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ieee1394_device_id_vendor_id $4 offsetof(struct ieee1394_device_id, vendor_id)"	#
# 0 "" 2
	.loc 1 34 2 view .LVU22
# 34 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ieee1394_device_id_model_id $8 offsetof(struct ieee1394_device_id, model_id)"	#
# 0 "" 2
	.loc 1 35 2 view .LVU23
# 35 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ieee1394_device_id_specifier_id $12 offsetof(struct ieee1394_device_id, specifier_id)"	#
# 0 "" 2
	.loc 1 36 2 view .LVU24
# 36 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ieee1394_device_id_version $16 offsetof(struct ieee1394_device_id, version)"	#
# 0 "" 2
	.loc 1 38 2 view .LVU25
# 38 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_pci_device_id $32 sizeof(struct pci_device_id)"	#
# 0 "" 2
	.loc 1 39 2 view .LVU26
# 39 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pci_device_id_vendor $0 offsetof(struct pci_device_id, vendor)"	#
# 0 "" 2
	.loc 1 40 2 view .LVU27
# 40 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pci_device_id_device $4 offsetof(struct pci_device_id, device)"	#
# 0 "" 2
	.loc 1 41 2 view .LVU28
# 41 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pci_device_id_subvendor $8 offsetof(struct pci_device_id, subvendor)"	#
# 0 "" 2
	.loc 1 42 2 view .LVU29
# 42 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pci_device_id_subdevice $12 offsetof(struct pci_device_id, subdevice)"	#
# 0 "" 2
	.loc 1 43 2 view .LVU30
# 43 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pci_device_id_class $16 offsetof(struct pci_device_id, class)"	#
# 0 "" 2
	.loc 1 44 2 view .LVU31
# 44 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pci_device_id_class_mask $20 offsetof(struct pci_device_id, class_mask)"	#
# 0 "" 2
	.loc 1 46 2 view .LVU32
# 46 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_ccw_device_id $16 sizeof(struct ccw_device_id)"	#
# 0 "" 2
	.loc 1 47 2 view .LVU33
# 47 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ccw_device_id_match_flags $0 offsetof(struct ccw_device_id, match_flags)"	#
# 0 "" 2
	.loc 1 48 2 view .LVU34
# 48 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ccw_device_id_cu_type $2 offsetof(struct ccw_device_id, cu_type)"	#
# 0 "" 2
	.loc 1 49 2 view .LVU35
# 49 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ccw_device_id_cu_model $6 offsetof(struct ccw_device_id, cu_model)"	#
# 0 "" 2
	.loc 1 50 2 view .LVU36
# 50 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ccw_device_id_dev_type $4 offsetof(struct ccw_device_id, dev_type)"	#
# 0 "" 2
	.loc 1 51 2 view .LVU37
# 51 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ccw_device_id_dev_model $7 offsetof(struct ccw_device_id, dev_model)"	#
# 0 "" 2
	.loc 1 53 2 view .LVU38
# 53 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_ap_device_id $16 sizeof(struct ap_device_id)"	#
# 0 "" 2
	.loc 1 54 2 view .LVU39
# 54 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ap_device_id_dev_type $2 offsetof(struct ap_device_id, dev_type)"	#
# 0 "" 2
	.loc 1 56 2 view .LVU40
# 56 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_css_device_id $16 sizeof(struct css_device_id)"	#
# 0 "" 2
	.loc 1 57 2 view .LVU41
# 57 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_css_device_id_type $1 offsetof(struct css_device_id, type)"	#
# 0 "" 2
	.loc 1 59 2 view .LVU42
# 59 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_serio_device_id $4 sizeof(struct serio_device_id)"	#
# 0 "" 2
	.loc 1 60 2 view .LVU43
# 60 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_serio_device_id_type $0 offsetof(struct serio_device_id, type)"	#
# 0 "" 2
	.loc 1 61 2 view .LVU44
# 61 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_serio_device_id_proto $3 offsetof(struct serio_device_id, proto)"	#
# 0 "" 2
	.loc 1 62 2 view .LVU45
# 62 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_serio_device_id_id $2 offsetof(struct serio_device_id, id)"	#
# 0 "" 2
	.loc 1 63 2 view .LVU46
# 63 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_serio_device_id_extra $1 offsetof(struct serio_device_id, extra)"	#
# 0 "" 2
	.loc 1 65 2 view .LVU47
# 65 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_acpi_device_id $32 sizeof(struct acpi_device_id)"	#
# 0 "" 2
	.loc 1 66 2 view .LVU48
# 66 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_acpi_device_id_id $0 offsetof(struct acpi_device_id, id)"	#
# 0 "" 2
	.loc 1 67 2 view .LVU49
# 67 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_acpi_device_id_cls $24 offsetof(struct acpi_device_id, cls)"	#
# 0 "" 2
	.loc 1 68 2 view .LVU50
# 68 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_acpi_device_id_cls_msk $28 offsetof(struct acpi_device_id, cls_msk)"	#
# 0 "" 2
	.loc 1 70 2 view .LVU51
# 70 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_pnp_device_id $16 sizeof(struct pnp_device_id)"	#
# 0 "" 2
	.loc 1 71 2 view .LVU52
# 71 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pnp_device_id_id $0 offsetof(struct pnp_device_id, id)"	#
# 0 "" 2
	.loc 1 73 2 view .LVU53
# 73 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_pnp_card_device_id $80 sizeof(struct pnp_card_device_id)"	#
# 0 "" 2
	.loc 1 74 2 view .LVU54
# 74 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pnp_card_device_id_devs $16 offsetof(struct pnp_card_device_id, devs)"	#
# 0 "" 2
	.loc 1 76 2 view .LVU55
# 76 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_pcmcia_device_id $80 sizeof(struct pcmcia_device_id)"	#
# 0 "" 2
	.loc 1 77 2 view .LVU56
# 77 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_match_flags $0 offsetof(struct pcmcia_device_id, match_flags)"	#
# 0 "" 2
	.loc 1 78 2 view .LVU57
# 78 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_manf_id $2 offsetof(struct pcmcia_device_id, manf_id)"	#
# 0 "" 2
	.loc 1 79 2 view .LVU58
# 79 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_card_id $4 offsetof(struct pcmcia_device_id, card_id)"	#
# 0 "" 2
	.loc 1 80 2 view .LVU59
# 80 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_func_id $6 offsetof(struct pcmcia_device_id, func_id)"	#
# 0 "" 2
	.loc 1 81 2 view .LVU60
# 81 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_function $7 offsetof(struct pcmcia_device_id, function)"	#
# 0 "" 2
	.loc 1 82 2 view .LVU61
# 82 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_device_no $8 offsetof(struct pcmcia_device_id, device_no)"	#
# 0 "" 2
	.loc 1 83 2 view .LVU62
# 83 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_pcmcia_device_id_prod_id_hash $12 offsetof(struct pcmcia_device_id, prod_id_hash)"	#
# 0 "" 2
	.loc 1 85 2 view .LVU63
# 85 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_of_device_id $200 sizeof(struct of_device_id)"	#
# 0 "" 2
	.loc 1 86 2 view .LVU64
# 86 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_of_device_id_name $0 offsetof(struct of_device_id, name)"	#
# 0 "" 2
	.loc 1 87 2 view .LVU65
# 87 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_of_device_id_type $32 offsetof(struct of_device_id, type)"	#
# 0 "" 2
	.loc 1 88 2 view .LVU66
# 88 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_of_device_id_compatible $64 offsetof(struct of_device_id, compatible)"	#
# 0 "" 2
	.loc 1 90 2 view .LVU67
# 90 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_vio_device_id $64 sizeof(struct vio_device_id)"	#
# 0 "" 2
	.loc 1 91 2 view .LVU68
# 91 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_vio_device_id_type $0 offsetof(struct vio_device_id, type)"	#
# 0 "" 2
	.loc 1 92 2 view .LVU69
# 92 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_vio_device_id_compat $32 offsetof(struct vio_device_id, compat)"	#
# 0 "" 2
	.loc 1 94 2 view .LVU70
# 94 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_input_device_id $200 sizeof(struct input_device_id)"	#
# 0 "" 2
	.loc 1 95 2 view .LVU71
# 95 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_flags $0 offsetof(struct input_device_id, flags)"	#
# 0 "" 2
	.loc 1 96 2 view .LVU72
# 96 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_bustype $8 offsetof(struct input_device_id, bustype)"	#
# 0 "" 2
	.loc 1 97 2 view .LVU73
# 97 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_vendor $10 offsetof(struct input_device_id, vendor)"	#
# 0 "" 2
	.loc 1 98 2 view .LVU74
# 98 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_product $12 offsetof(struct input_device_id, product)"	#
# 0 "" 2
	.loc 1 99 2 view .LVU75
# 99 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_version $14 offsetof(struct input_device_id, version)"	#
# 0 "" 2
	.loc 1 100 2 view .LVU76
# 100 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_evbit $16 offsetof(struct input_device_id, evbit)"	#
# 0 "" 2
	.loc 1 101 2 view .LVU77
# 101 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_keybit $24 offsetof(struct input_device_id, keybit)"	#
# 0 "" 2
	.loc 1 102 2 view .LVU78
# 102 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_relbit $120 offsetof(struct input_device_id, relbit)"	#
# 0 "" 2
	.loc 1 103 2 view .LVU79
# 103 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_absbit $128 offsetof(struct input_device_id, absbit)"	#
# 0 "" 2
	.loc 1 104 2 view .LVU80
# 104 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_mscbit $136 offsetof(struct input_device_id, mscbit)"	#
# 0 "" 2
	.loc 1 105 2 view .LVU81
# 105 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_ledbit $144 offsetof(struct input_device_id, ledbit)"	#
# 0 "" 2
	.loc 1 106 2 view .LVU82
# 106 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_sndbit $152 offsetof(struct input_device_id, sndbit)"	#
# 0 "" 2
	.loc 1 107 2 view .LVU83
# 107 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_ffbit $160 offsetof(struct input_device_id, ffbit)"	#
# 0 "" 2
	.loc 1 108 2 view .LVU84
# 108 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_input_device_id_swbit $176 offsetof(struct input_device_id, swbit)"	#
# 0 "" 2
	.loc 1 110 2 view .LVU85
# 110 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_eisa_device_id $16 sizeof(struct eisa_device_id)"	#
# 0 "" 2
	.loc 1 111 2 view .LVU86
# 111 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_eisa_device_id_sig $0 offsetof(struct eisa_device_id, sig)"	#
# 0 "" 2
	.loc 1 113 2 view .LVU87
# 113 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_parisc_device_id $8 sizeof(struct parisc_device_id)"	#
# 0 "" 2
	.loc 1 114 2 view .LVU88
# 114 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_parisc_device_id_hw_type $0 offsetof(struct parisc_device_id, hw_type)"	#
# 0 "" 2
	.loc 1 115 2 view .LVU89
# 115 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_parisc_device_id_hversion $2 offsetof(struct parisc_device_id, hversion)"	#
# 0 "" 2
	.loc 1 116 2 view .LVU90
# 116 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_parisc_device_id_hversion_rev $1 offsetof(struct parisc_device_id, hversion_rev)"	#
# 0 "" 2
	.loc 1 117 2 view .LVU91
# 117 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_parisc_device_id_sversion $4 offsetof(struct parisc_device_id, sversion)"	#
# 0 "" 2
	.loc 1 119 2 view .LVU92
# 119 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_sdio_device_id $16 sizeof(struct sdio_device_id)"	#
# 0 "" 2
	.loc 1 120 2 view .LVU93
# 120 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdio_device_id_class $0 offsetof(struct sdio_device_id, class)"	#
# 0 "" 2
	.loc 1 121 2 view .LVU94
# 121 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdio_device_id_vendor $2 offsetof(struct sdio_device_id, vendor)"	#
# 0 "" 2
	.loc 1 122 2 view .LVU95
# 122 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdio_device_id_device $4 offsetof(struct sdio_device_id, device)"	#
# 0 "" 2
	.loc 1 124 2 view .LVU96
# 124 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_ssb_device_id $6 sizeof(struct ssb_device_id)"	#
# 0 "" 2
	.loc 1 125 2 view .LVU97
# 125 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ssb_device_id_vendor $0 offsetof(struct ssb_device_id, vendor)"	#
# 0 "" 2
	.loc 1 126 2 view .LVU98
# 126 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ssb_device_id_coreid $2 offsetof(struct ssb_device_id, coreid)"	#
# 0 "" 2
	.loc 1 127 2 view .LVU99
# 127 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ssb_device_id_revision $4 offsetof(struct ssb_device_id, revision)"	#
# 0 "" 2
	.loc 1 129 2 view .LVU100
# 129 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_bcma_device_id $6 sizeof(struct bcma_device_id)"	#
# 0 "" 2
	.loc 1 130 2 view .LVU101
# 130 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_bcma_device_id_manuf $0 offsetof(struct bcma_device_id, manuf)"	#
# 0 "" 2
	.loc 1 131 2 view .LVU102
# 131 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_bcma_device_id_id $2 offsetof(struct bcma_device_id, id)"	#
# 0 "" 2
	.loc 1 132 2 view .LVU103
# 132 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_bcma_device_id_rev $4 offsetof(struct bcma_device_id, rev)"	#
# 0 "" 2
	.loc 1 133 2 view .LVU104
# 133 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_bcma_device_id_class $5 offsetof(struct bcma_device_id, class)"	#
# 0 "" 2
	.loc 1 135 2 view .LVU105
# 135 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_virtio_device_id $8 sizeof(struct virtio_device_id)"	#
# 0 "" 2
	.loc 1 136 2 view .LVU106
# 136 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_virtio_device_id_device $0 offsetof(struct virtio_device_id, device)"	#
# 0 "" 2
	.loc 1 137 2 view .LVU107
# 137 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_virtio_device_id_vendor $4 offsetof(struct virtio_device_id, vendor)"	#
# 0 "" 2
	.loc 1 139 2 view .LVU108
# 139 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_hv_vmbus_device_id $24 sizeof(struct hv_vmbus_device_id)"	#
# 0 "" 2
	.loc 1 140 2 view .LVU109
# 140 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hv_vmbus_device_id_guid $0 offsetof(struct hv_vmbus_device_id, guid)"	#
# 0 "" 2
	.loc 1 142 2 view .LVU110
# 142 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_rpmsg_device_id $32 sizeof(struct rpmsg_device_id)"	#
# 0 "" 2
	.loc 1 143 2 view .LVU111
# 143 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_rpmsg_device_id_name $0 offsetof(struct rpmsg_device_id, name)"	#
# 0 "" 2
	.loc 1 145 2 view .LVU112
# 145 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_i2c_device_id $32 sizeof(struct i2c_device_id)"	#
# 0 "" 2
	.loc 1 146 2 view .LVU113
# 146 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_i2c_device_id_name $0 offsetof(struct i2c_device_id, name)"	#
# 0 "" 2
	.loc 1 148 2 view .LVU114
# 148 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_spi_device_id $40 sizeof(struct spi_device_id)"	#
# 0 "" 2
	.loc 1 149 2 view .LVU115
# 149 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_spi_device_id_name $0 offsetof(struct spi_device_id, name)"	#
# 0 "" 2
	.loc 1 151 2 view .LVU116
# 151 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_dmi_system_id $344 sizeof(struct dmi_system_id)"	#
# 0 "" 2
	.loc 1 152 2 view .LVU117
# 152 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_dmi_system_id_matches $16 offsetof(struct dmi_system_id, matches)"	#
# 0 "" 2
	.loc 1 154 2 view .LVU118
# 154 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_platform_device_id $32 sizeof(struct platform_device_id)"	#
# 0 "" 2
	.loc 1 155 2 view .LVU119
# 155 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_platform_device_id_name $0 offsetof(struct platform_device_id, name)"	#
# 0 "" 2
	.loc 1 157 2 view .LVU120
# 157 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_mdio_device_id $8 sizeof(struct mdio_device_id)"	#
# 0 "" 2
	.loc 1 158 2 view .LVU121
# 158 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mdio_device_id_phy_id $0 offsetof(struct mdio_device_id, phy_id)"	#
# 0 "" 2
	.loc 1 159 2 view .LVU122
# 159 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mdio_device_id_phy_id_mask $4 offsetof(struct mdio_device_id, phy_id_mask)"	#
# 0 "" 2
	.loc 1 161 2 view .LVU123
# 161 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_zorro_device_id $16 sizeof(struct zorro_device_id)"	#
# 0 "" 2
	.loc 1 162 2 view .LVU124
# 162 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_zorro_device_id_id $0 offsetof(struct zorro_device_id, id)"	#
# 0 "" 2
	.loc 1 164 2 view .LVU125
# 164 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_isapnp_device_id $16 sizeof(struct isapnp_device_id)"	#
# 0 "" 2
	.loc 1 165 2 view .LVU126
# 165 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_isapnp_device_id_vendor $4 offsetof(struct isapnp_device_id, vendor)"	#
# 0 "" 2
	.loc 1 166 2 view .LVU127
# 166 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_isapnp_device_id_function $6 offsetof(struct isapnp_device_id, function)"	#
# 0 "" 2
	.loc 1 168 2 view .LVU128
# 168 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_ipack_device_id $12 sizeof(struct ipack_device_id)"	#
# 0 "" 2
	.loc 1 169 2 view .LVU129
# 169 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ipack_device_id_format $0 offsetof(struct ipack_device_id, format)"	#
# 0 "" 2
	.loc 1 170 2 view .LVU130
# 170 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ipack_device_id_vendor $4 offsetof(struct ipack_device_id, vendor)"	#
# 0 "" 2
	.loc 1 171 2 view .LVU131
# 171 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ipack_device_id_device $8 offsetof(struct ipack_device_id, device)"	#
# 0 "" 2
	.loc 1 173 2 view .LVU132
# 173 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_amba_id $16 sizeof(struct amba_id)"	#
# 0 "" 2
	.loc 1 174 2 view .LVU133
# 174 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_amba_id_id $0 offsetof(struct amba_id, id)"	#
# 0 "" 2
	.loc 1 175 2 view .LVU134
# 175 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_amba_id_mask $4 offsetof(struct amba_id, mask)"	#
# 0 "" 2
	.loc 1 177 2 view .LVU135
# 177 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_mips_cdmm_device_id $1 sizeof(struct mips_cdmm_device_id)"	#
# 0 "" 2
	.loc 1 178 2 view .LVU136
# 178 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mips_cdmm_device_id_type $0 offsetof(struct mips_cdmm_device_id, type)"	#
# 0 "" 2
	.loc 1 180 2 view .LVU137
# 180 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_x86_cpu_id $16 sizeof(struct x86_cpu_id)"	#
# 0 "" 2
	.loc 1 181 2 view .LVU138
# 181 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_x86_cpu_id_feature $6 offsetof(struct x86_cpu_id, feature)"	#
# 0 "" 2
	.loc 1 182 2 view .LVU139
# 182 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_x86_cpu_id_family $2 offsetof(struct x86_cpu_id, family)"	#
# 0 "" 2
	.loc 1 183 2 view .LVU140
# 183 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_x86_cpu_id_model $4 offsetof(struct x86_cpu_id, model)"	#
# 0 "" 2
	.loc 1 184 2 view .LVU141
# 184 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_x86_cpu_id_vendor $0 offsetof(struct x86_cpu_id, vendor)"	#
# 0 "" 2
	.loc 1 186 2 view .LVU142
# 186 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_cpu_feature $2 sizeof(struct cpu_feature)"	#
# 0 "" 2
	.loc 1 187 2 view .LVU143
# 187 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_cpu_feature_feature $0 offsetof(struct cpu_feature, feature)"	#
# 0 "" 2
	.loc 1 189 2 view .LVU144
# 189 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_mei_cl_device_id $64 sizeof(struct mei_cl_device_id)"	#
# 0 "" 2
	.loc 1 190 2 view .LVU145
# 190 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mei_cl_device_id_name $0 offsetof(struct mei_cl_device_id, name)"	#
# 0 "" 2
	.loc 1 191 2 view .LVU146
# 191 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mei_cl_device_id_uuid $32 offsetof(struct mei_cl_device_id, uuid)"	#
# 0 "" 2
	.loc 1 192 2 view .LVU147
# 192 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mei_cl_device_id_version $48 offsetof(struct mei_cl_device_id, version)"	#
# 0 "" 2
	.loc 1 194 2 view .LVU148
# 194 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_rio_device_id $8 sizeof(struct rio_device_id)"	#
# 0 "" 2
	.loc 1 195 2 view .LVU149
# 195 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_rio_device_id_did $0 offsetof(struct rio_device_id, did)"	#
# 0 "" 2
	.loc 1 196 2 view .LVU150
# 196 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_rio_device_id_vid $2 offsetof(struct rio_device_id, vid)"	#
# 0 "" 2
	.loc 1 197 2 view .LVU151
# 197 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_rio_device_id_asm_did $4 offsetof(struct rio_device_id, asm_did)"	#
# 0 "" 2
	.loc 1 198 2 view .LVU152
# 198 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_rio_device_id_asm_vid $6 offsetof(struct rio_device_id, asm_vid)"	#
# 0 "" 2
	.loc 1 200 2 view .LVU153
# 200 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_ulpi_device_id $16 sizeof(struct ulpi_device_id)"	#
# 0 "" 2
	.loc 1 201 2 view .LVU154
# 201 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ulpi_device_id_vendor $0 offsetof(struct ulpi_device_id, vendor)"	#
# 0 "" 2
	.loc 1 202 2 view .LVU155
# 202 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_ulpi_device_id_product $2 offsetof(struct ulpi_device_id, product)"	#
# 0 "" 2
	.loc 1 204 2 view .LVU156
# 204 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_hda_device_id $32 sizeof(struct hda_device_id)"	#
# 0 "" 2
	.loc 1 205 2 view .LVU157
# 205 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hda_device_id_vendor_id $0 offsetof(struct hda_device_id, vendor_id)"	#
# 0 "" 2
	.loc 1 206 2 view .LVU158
# 206 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hda_device_id_rev_id $4 offsetof(struct hda_device_id, rev_id)"	#
# 0 "" 2
	.loc 1 207 2 view .LVU159
# 207 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_hda_device_id_api_version $8 offsetof(struct hda_device_id, api_version)"	#
# 0 "" 2
	.loc 1 209 2 view .LVU160
# 209 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_sdw_device_id $16 sizeof(struct sdw_device_id)"	#
# 0 "" 2
	.loc 1 210 2 view .LVU161
# 210 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdw_device_id_mfg_id $0 offsetof(struct sdw_device_id, mfg_id)"	#
# 0 "" 2
	.loc 1 211 2 view .LVU162
# 211 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdw_device_id_part_id $2 offsetof(struct sdw_device_id, part_id)"	#
# 0 "" 2
	.loc 1 212 2 view .LVU163
# 212 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdw_device_id_sdw_version $4 offsetof(struct sdw_device_id, sdw_version)"	#
# 0 "" 2
	.loc 1 213 2 view .LVU164
# 213 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_sdw_device_id_class_id $5 offsetof(struct sdw_device_id, class_id)"	#
# 0 "" 2
	.loc 1 215 2 view .LVU165
# 215 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_fsl_mc_device_id $18 sizeof(struct fsl_mc_device_id)"	#
# 0 "" 2
	.loc 1 216 2 view .LVU166
# 216 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_fsl_mc_device_id_vendor $0 offsetof(struct fsl_mc_device_id, vendor)"	#
# 0 "" 2
	.loc 1 217 2 view .LVU167
# 217 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_fsl_mc_device_id_obj_type $2 offsetof(struct fsl_mc_device_id, obj_type)"	#
# 0 "" 2
	.loc 1 219 2 view .LVU168
# 219 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_tb_service_id $40 sizeof(struct tb_service_id)"	#
# 0 "" 2
	.loc 1 220 2 view .LVU169
# 220 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_tb_service_id_match_flags $0 offsetof(struct tb_service_id, match_flags)"	#
# 0 "" 2
	.loc 1 221 2 view .LVU170
# 221 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_tb_service_id_protocol_key $4 offsetof(struct tb_service_id, protocol_key)"	#
# 0 "" 2
	.loc 1 222 2 view .LVU171
# 222 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_tb_service_id_protocol_id $16 offsetof(struct tb_service_id, protocol_id)"	#
# 0 "" 2
	.loc 1 223 2 view .LVU172
# 223 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_tb_service_id_protocol_version $20 offsetof(struct tb_service_id, protocol_version)"	#
# 0 "" 2
	.loc 1 224 2 view .LVU173
# 224 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_tb_service_id_protocol_revision $24 offsetof(struct tb_service_id, protocol_revision)"	#
# 0 "" 2
	.loc 1 226 2 view .LVU174
# 226 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_typec_device_id $16 sizeof(struct typec_device_id)"	#
# 0 "" 2
	.loc 1 227 2 view .LVU175
# 227 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_typec_device_id_svid $0 offsetof(struct typec_device_id, svid)"	#
# 0 "" 2
	.loc 1 228 2 view .LVU176
# 228 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_typec_device_id_mode $2 offsetof(struct typec_device_id, mode)"	#
# 0 "" 2
	.loc 1 230 2 view .LVU177
# 230 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_mhi_device_id $40 sizeof(struct mhi_device_id)"	#
# 0 "" 2
	.loc 1 231 2 view .LVU178
# 231 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_mhi_device_id_chan $0 offsetof(struct mhi_device_id, chan)"	#
# 0 "" 2
	.loc 1 233 2 view .LVU179
# 233 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->SIZE_auxiliary_device_id $40 sizeof(struct auxiliary_device_id)"	#
# 0 "" 2
	.loc 1 234 2 view .LVU180
# 234 "scripts/mod/devicetable-offsets.c" 1
	
.ascii "->OFF_auxiliary_device_id_name $0 offsetof(struct auxiliary_device_id, name)"	#
# 0 "" 2
	.loc 1 236 2 view .LVU181
# scripts/mod/devicetable-offsets.c:237: }
	.loc 1 237 1 is_stmt 0 view .LVU182
#NO_APP
	xorl	%eax, %eax	#
	ret	
	.cfi_endproc
.LFE71:
	.size	main, .-main
	.text
.Letext0:
	.file 2 "./include/uapi/asm-generic/int-ll64.h"
	.file 3 "./include/asm-generic/int-ll64.h"
	.file 4 "./include/linux/types.h"
	.file 5 "./include/uapi/linux/uuid.h"
	.file 6 "./arch/x86/include/asm/alternative.h"
	.file 7 "./arch/x86/include/asm/nops.h"
	.file 8 "./include/linux/jump_label.h"
	.file 9 "./arch/x86/include/asm/jump_label.h"
	.file 10 "./arch/x86/include/asm/string_64.h"
	.file 11 "./include/linux/uuid.h"
	.file 12 "./arch/x86/include/asm/asm.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x2db
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF39
	.byte	0x1
	.long	.LASF40
	.long	.LASF41
	.long	.Ldebug_ranges0+0
	.quad	0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF1
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF3
	.uleb128 0x3
	.long	.LASF8
	.byte	0x2
	.byte	0x15
	.byte	0x17
	.long	0x51
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF4
	.uleb128 0x4
	.long	0x51
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF6
	.uleb128 0x5
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF7
	.uleb128 0x3
	.long	.LASF9
	.byte	0x2
	.byte	0x1f
	.byte	0x2a
	.long	0x85
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF10
	.uleb128 0x6
	.string	"u8"
	.byte	0x3
	.byte	0x11
	.byte	0xf
	.long	0x45
	.uleb128 0x4
	.long	0x8c
	.uleb128 0x6
	.string	"u64"
	.byte	0x3
	.byte	0x17
	.byte	0xf
	.long	0x79
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF11
	.uleb128 0x3
	.long	.LASF12
	.byte	0x4
	.byte	0x1e
	.byte	0x11
	.long	0xbb
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.long	.LASF13
	.uleb128 0x7
	.byte	0x4
	.byte	0x4
	.byte	0xb1
	.byte	0x9
	.long	0xd9
	.uleb128 0x8
	.long	.LASF15
	.byte	0x4
	.byte	0xb2
	.byte	0x6
	.long	0x6b
	.byte	0
	.byte	0
	.uleb128 0x3
	.long	.LASF14
	.byte	0x4
	.byte	0xb3
	.byte	0x3
	.long	0xc2
	.uleb128 0x7
	.byte	0x10
	.byte	0x5
	.byte	0x17
	.byte	0x9
	.long	0xfa
	.uleb128 0x9
	.string	"b"
	.byte	0x5
	.byte	0x18
	.byte	0x7
	.long	0xfa
	.byte	0
	.byte	0
	.uleb128 0xa
	.long	0x45
	.long	0x10a
	.uleb128 0xb
	.long	0x29
	.byte	0xf
	.byte	0
	.uleb128 0x3
	.long	.LASF16
	.byte	0x5
	.byte	0x19
	.byte	0x3
	.long	0xe5
	.uleb128 0x4
	.long	0x10a
	.uleb128 0xc
	.long	.LASF42
	.byte	0xc
	.byte	0xba
	.byte	0x18
	.long	0x29
	.uleb128 0xd
	.long	.LASF17
	.byte	0x6
	.byte	0x3d
	.byte	0xc
	.long	0x6b
	.uleb128 0xd
	.long	.LASF18
	.byte	0x6
	.byte	0x4b
	.byte	0xd
	.long	0xaf
	.uleb128 0xd
	.long	.LASF19
	.byte	0x7
	.byte	0x8f
	.byte	0x25
	.long	0x14b
	.uleb128 0xe
	.byte	0x8
	.long	0x157
	.uleb128 0xe
	.byte	0x8
	.long	0x58
	.uleb128 0x4
	.long	0x151
	.uleb128 0xd
	.long	.LASF20
	.byte	0x8
	.byte	0x53
	.byte	0xd
	.long	0xaf
	.uleb128 0xf
	.byte	0x8
	.byte	0x8
	.byte	0x6a
	.byte	0x2
	.long	0x196
	.uleb128 0x10
	.long	.LASF21
	.byte	0x8
	.byte	0x6b
	.byte	0x11
	.long	0x29
	.uleb128 0x10
	.long	.LASF22
	.byte	0x8
	.byte	0x6c
	.byte	0x16
	.long	0x1cb
	.uleb128 0x10
	.long	.LASF23
	.byte	0x8
	.byte	0x6d
	.byte	0x1a
	.long	0x1d6
	.byte	0
	.uleb128 0x11
	.long	.LASF26
	.byte	0x18
	.byte	0x9
	.byte	0x49
	.byte	0x8
	.long	0x1cb
	.uleb128 0x8
	.long	.LASF24
	.byte	0x9
	.byte	0x4a
	.byte	0xf
	.long	0x1fd
	.byte	0
	.uleb128 0x8
	.long	.LASF25
	.byte	0x9
	.byte	0x4b
	.byte	0xf
	.long	0x1fd
	.byte	0x8
	.uleb128 0x9
	.string	"key"
	.byte	0x9
	.byte	0x4c
	.byte	0xf
	.long	0x1fd
	.byte	0x10
	.byte	0
	.uleb128 0xe
	.byte	0x8
	.long	0x196
	.uleb128 0x12
	.long	.LASF43
	.uleb128 0xe
	.byte	0x8
	.long	0x1d1
	.uleb128 0x11
	.long	.LASF27
	.byte	0x10
	.byte	0x8
	.byte	0x5b
	.byte	0x8
	.long	0x1fd
	.uleb128 0x8
	.long	.LASF28
	.byte	0x8
	.byte	0x5c
	.byte	0xb
	.long	0xd9
	.byte	0
	.uleb128 0x13
	.long	0x168
	.byte	0x8
	.byte	0
	.uleb128 0x3
	.long	.LASF29
	.byte	0x9
	.byte	0x44
	.byte	0xd
	.long	0x9c
	.uleb128 0xa
	.long	0x196
	.long	0x214
	.uleb128 0x14
	.byte	0
	.uleb128 0xd
	.long	.LASF30
	.byte	0x8
	.byte	0xd4
	.byte	0x1a
	.long	0x209
	.uleb128 0xd
	.long	.LASF31
	.byte	0x8
	.byte	0xd5
	.byte	0x1a
	.long	0x209
	.uleb128 0x15
	.long	.LASF32
	.byte	0x10
	.byte	0x8
	.value	0x162
	.byte	0x8
	.long	0x249
	.uleb128 0x16
	.string	"key"
	.byte	0x8
	.value	0x163
	.byte	0x14
	.long	0x1dc
	.byte	0
	.byte	0
	.uleb128 0xd
	.long	.LASF33
	.byte	0xa
	.byte	0x79
	.byte	0x1
	.long	0x22c
	.uleb128 0x7
	.byte	0x10
	.byte	0xb
	.byte	0x18
	.byte	0x9
	.long	0x26a
	.uleb128 0x9
	.string	"b"
	.byte	0xb
	.byte	0x19
	.byte	0x7
	.long	0xfa
	.byte	0
	.byte	0
	.uleb128 0x3
	.long	.LASF34
	.byte	0xb
	.byte	0x1a
	.byte	0x3
	.long	0x255
	.uleb128 0x4
	.long	0x26a
	.uleb128 0xd
	.long	.LASF35
	.byte	0xb
	.byte	0x29
	.byte	0x15
	.long	0x116
	.uleb128 0xd
	.long	.LASF36
	.byte	0xb
	.byte	0x2a
	.byte	0x15
	.long	0x276
	.uleb128 0xa
	.long	0x97
	.long	0x2a3
	.uleb128 0xb
	.long	0x29
	.byte	0xf
	.byte	0
	.uleb128 0x4
	.long	0x293
	.uleb128 0xd
	.long	.LASF37
	.byte	0xb
	.byte	0x65
	.byte	0x11
	.long	0x2a3
	.uleb128 0xd
	.long	.LASF38
	.byte	0xb
	.byte	0x66
	.byte	0x11
	.long	0x2a3
	.uleb128 0x17
	.long	.LASF44
	.byte	0x1
	.byte	0x9
	.byte	0x5
	.long	0x6b
	.quad	.LFB71
	.quad	.LFE71-.LFB71
	.uleb128 0x1
	.byte	0x9c
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x17
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x13
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0xd
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.LFB71
	.quad	.LFE71-.LFB71
	.quad	0
	.quad	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.quad	.LFB71
	.quad	.LFE71
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF8:
	.string	"__u8"
.LASF30:
	.string	"__start___jump_table"
.LASF23:
	.string	"next"
.LASF20:
	.string	"static_key_initialized"
.LASF27:
	.string	"static_key"
.LASF12:
	.string	"bool"
.LASF41:
	.string	"/home/calmwu/Program/kernel-src/linux-4.18.0-348.7.1.el8_5"
.LASF19:
	.string	"ideal_nops"
.LASF25:
	.string	"target"
.LASF16:
	.string	"guid_t"
.LASF15:
	.string	"counter"
.LASF4:
	.string	"unsigned char"
.LASF28:
	.string	"enabled"
.LASF0:
	.string	"long unsigned int"
.LASF38:
	.string	"uuid_index"
.LASF32:
	.string	"static_key_false"
.LASF18:
	.string	"skip_smp_alternatives"
.LASF35:
	.string	"guid_null"
.LASF29:
	.string	"jump_label_t"
.LASF42:
	.string	"current_stack_pointer"
.LASF31:
	.string	"__stop___jump_table"
.LASF24:
	.string	"code"
.LASF43:
	.string	"static_key_mod"
.LASF44:
	.string	"main"
.LASF2:
	.string	"unsigned int"
.LASF40:
	.string	"scripts/mod/devicetable-offsets.c"
.LASF34:
	.string	"uuid_t"
.LASF10:
	.string	"long long unsigned int"
.LASF26:
	.string	"jump_entry"
.LASF3:
	.string	"signed char"
.LASF9:
	.string	"__u64"
.LASF7:
	.string	"long long int"
.LASF14:
	.string	"atomic_t"
.LASF1:
	.string	"char"
.LASF39:
	.ascii	"GNU C89 8.5.0 20210514 (Red Hat 8.5.0-26.0.1) -mno-sse -mno-"
	.ascii	"mmx -mno-sse2 -mno-3dnow -mno-avx -m64 -mno-80387 -mno-fp-re"
	.ascii	"t-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtun"
	.ascii	"e=generic -mno-red-zone -mcmodel=kernel -mindirect-branch=th"
	.ascii	"unk-extern -mindirect-branch-register -mrecord-mcount -mfent"
	.ascii	"ry -march=x86-64 -g -gdwarf-4 -O2 -std=gnu90 -p -fno-strict-"
	.ascii	"aliasing -fno-common -fshort-wchar -fno-PIE -falign-jumps=1 "
	.ascii	"-falign-loops=1 -funit-at-a-time -fno-asynchronous-unwind-ta"
	.ascii	"bles -fno-jump-tables -fno-delete-null-pointer-"
	.string	"checks -fno-reorder-blocks -fno-ipa-cp-clone -fno-partial-inlining -fstack-protector-strong -fno-inline-functions-called-once -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fstack-check=no -fconserve-stack --param allow-store-data-races=0"
.LASF21:
	.string	"type"
.LASF36:
	.string	"uuid_null"
.LASF5:
	.string	"short int"
.LASF17:
	.string	"alternatives_patched"
.LASF33:
	.string	"mcsafe_key"
.LASF11:
	.string	"long int"
.LASF37:
	.string	"guid_index"
.LASF22:
	.string	"entries"
.LASF6:
	.string	"short unsigned int"
.LASF13:
	.string	"_Bool"
	.ident	"GCC: (GNU) 8.5.0 20210514 (Red Hat 8.5.0-26.0.1)"
	.section	.note.GNU-stack,"",@progbits
