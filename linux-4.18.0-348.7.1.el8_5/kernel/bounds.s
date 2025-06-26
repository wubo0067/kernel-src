	.file	"bounds.c"
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
# -D CONFIG_TPAUSE=1 -D CC_USING_FENTRY -D KBUILD_BASENAME="bounds"
# -D KBUILD_MODNAME="bounds"
# -isystem /usr/lib/gcc/x86_64-redhat-linux/8/include
# -include ./include/linux/kconfig.h
# -include ./include/linux/compiler_types.h -MD kernel/.bounds.s.d
# kernel/bounds.c -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -m64
# -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3
# -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel
# -mindirect-branch=thunk-extern -mindirect-branch-register -mrecord-mcount
# -mfentry -march=x86-64 -auxbase-strip kernel/bounds.s -g -gdwarf-4 -O2
# -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs
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
	.p2align 4,,15
	.globl	foo
	.type	foo, @function
foo:
.LFB163:
	.file 1 "kernel/bounds.c"
	.loc 1 17 1 view -0
	.cfi_startproc
1:	call	__fentry__
	.section __mcount_loc, "a",@progbits
	.quad 1b
	.previous
	.loc 1 19 2 view .LVU1
#APP
# 19 "kernel/bounds.c" 1
	
.ascii "->NR_PAGEFLAGS $26 __NR_PAGEFLAGS"	#
# 0 "" 2
	.loc 1 20 2 view .LVU2
# 20 "kernel/bounds.c" 1
	
.ascii "->MAX_NR_ZONES $5 __MAX_NR_ZONES"	#
# 0 "" 2
	.loc 1 22 2 view .LVU3
# 22 "kernel/bounds.c" 1
	
.ascii "->NR_CPUS_BITS $13 ilog2(CONFIG_NR_CPUS)"	#
# 0 "" 2
	.loc 1 24 2 view .LVU4
# 24 "kernel/bounds.c" 1
	
.ascii "->SPINLOCK_SIZE $4 sizeof(spinlock_t)"	#
# 0 "" 2
# kernel/bounds.c:26: }
	.loc 1 26 1 is_stmt 0 view .LVU5
#NO_APP
	ret	
	.cfi_endproc
.LFE163:
	.size	foo, .-foo
.Letext0:
	.file 2 "./include/uapi/asm-generic/int-ll64.h"
	.file 3 "./include/linux/types.h"
	.file 4 "./arch/x86/include/asm/alternative.h"
	.file 5 "./arch/x86/include/asm/nops.h"
	.file 6 "./include/linux/init.h"
	.file 7 "./include/linux/printk.h"
	.file 8 "./include/linux/jump_label.h"
	.file 9 "./arch/x86/include/asm/jump_label.h"
	.file 10 "./include/linux/kernel.h"
	.file 11 "./include/linux/page-flags.h"
	.file 12 "./include/linux/mmzone.h"
	.file 13 "./include/asm-generic/int-ll64.h"
	.file 14 "./arch/x86/include/asm/asm.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x5c9
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF126
	.byte	0x1
	.long	.LASF127
	.long	.LASF128
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF1
	.uleb128 0x3
	.long	0x34
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF4
	.uleb128 0x3
	.long	0x4e
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF6
	.uleb128 0x4
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF7
	.uleb128 0x5
	.long	.LASF9
	.byte	0x2
	.byte	0x1f
	.byte	0x2a
	.long	0x82
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF8
	.uleb128 0x6
	.string	"u64"
	.byte	0xd
	.byte	0x17
	.byte	0xf
	.long	0x76
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF10
	.uleb128 0x7
	.byte	0x8
	.long	0x34
	.uleb128 0x5
	.long	.LASF11
	.byte	0x3
	.byte	0x1e
	.byte	0x11
	.long	0xae
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.long	.LASF12
	.uleb128 0x8
	.byte	0x4
	.byte	0x3
	.byte	0xb1
	.byte	0x9
	.long	0xcc
	.uleb128 0x9
	.long	.LASF37
	.byte	0x3
	.byte	0xb2
	.byte	0x6
	.long	0x68
	.byte	0
	.byte	0
	.uleb128 0x5
	.long	.LASF13
	.byte	0x3
	.byte	0xb3
	.byte	0x3
	.long	0xb5
	.uleb128 0xa
	.long	.LASF129
	.byte	0xe
	.byte	0xba
	.byte	0x18
	.long	0x2d
	.uleb128 0xb
	.long	.LASF14
	.byte	0x4
	.byte	0x3d
	.byte	0xc
	.long	0x68
	.uleb128 0xb
	.long	.LASF15
	.byte	0x4
	.byte	0x4b
	.byte	0xd
	.long	0xa2
	.uleb128 0xb
	.long	.LASF16
	.byte	0x5
	.byte	0x8f
	.byte	0x25
	.long	0x108
	.uleb128 0x7
	.byte	0x8
	.long	0x114
	.uleb128 0x7
	.byte	0x8
	.long	0x55
	.uleb128 0x3
	.long	0x10e
	.uleb128 0x5
	.long	.LASF17
	.byte	0x6
	.byte	0x74
	.byte	0xf
	.long	0x125
	.uleb128 0x7
	.byte	0x8
	.long	0x12b
	.uleb128 0xc
	.long	0x68
	.uleb128 0x7
	.byte	0x8
	.long	0x136
	.uleb128 0xd
	.uleb128 0x5
	.long	.LASF18
	.byte	0x6
	.byte	0x7f
	.byte	0x14
	.long	0x119
	.uleb128 0xe
	.long	0x137
	.long	0x14e
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.long	.LASF19
	.byte	0x6
	.byte	0x87
	.byte	0x19
	.long	0x143
	.uleb128 0xb
	.long	.LASF20
	.byte	0x6
	.byte	0x87
	.byte	0x31
	.long	0x143
	.uleb128 0xe
	.long	0x34
	.long	0x171
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.long	.LASF21
	.byte	0x6
	.byte	0x90
	.byte	0x18
	.long	0x166
	.uleb128 0xb
	.long	.LASF22
	.byte	0x6
	.byte	0x91
	.byte	0xe
	.long	0x9c
	.uleb128 0xb
	.long	.LASF23
	.byte	0x6
	.byte	0x92
	.byte	0x15
	.long	0x40
	.uleb128 0x10
	.long	.LASF43
	.uleb128 0xb
	.long	.LASF24
	.byte	0x6
	.byte	0x98
	.byte	0x20
	.long	0x195
	.uleb128 0xb
	.long	.LASF25
	.byte	0x6
	.byte	0x9b
	.byte	0xd
	.long	0xa2
	.uleb128 0xb
	.long	.LASF26
	.byte	0x6
	.byte	0xa1
	.byte	0xf
	.long	0x130
	.uleb128 0xb
	.long	.LASF27
	.byte	0x6
	.byte	0xa3
	.byte	0xd
	.long	0xa2
	.uleb128 0xe
	.long	0x3b
	.long	0x1d5
	.uleb128 0xf
	.byte	0
	.uleb128 0x3
	.long	0x1ca
	.uleb128 0xb
	.long	.LASF28
	.byte	0x7
	.byte	0xb
	.byte	0x13
	.long	0x1d5
	.uleb128 0xb
	.long	.LASF29
	.byte	0x7
	.byte	0xc
	.byte	0x13
	.long	0x1d5
	.uleb128 0xe
	.long	0x68
	.long	0x1fd
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.long	.LASF30
	.byte	0x7
	.byte	0x3e
	.byte	0xc
	.long	0x1f2
	.uleb128 0xb
	.long	.LASF31
	.byte	0x7
	.byte	0x52
	.byte	0xd
	.long	0x166
	.uleb128 0xb
	.long	.LASF32
	.byte	0x7
	.byte	0x55
	.byte	0xc
	.long	0x68
	.uleb128 0xb
	.long	.LASF33
	.byte	0x7
	.byte	0xbc
	.byte	0xc
	.long	0x68
	.uleb128 0xb
	.long	.LASF34
	.byte	0x7
	.byte	0xbd
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF35
	.byte	0x7
	.value	0x118
	.byte	0xc
	.long	0x68
	.uleb128 0xb
	.long	.LASF36
	.byte	0x8
	.byte	0x53
	.byte	0xd
	.long	0xa2
	.uleb128 0x12
	.long	.LASF72
	.byte	0x18
	.byte	0x9
	.byte	0x49
	.byte	0x8
	.long	0x287
	.uleb128 0x9
	.long	.LASF38
	.byte	0x9
	.byte	0x4a
	.byte	0xf
	.long	0x287
	.byte	0
	.uleb128 0x9
	.long	.LASF39
	.byte	0x9
	.byte	0x4b
	.byte	0xf
	.long	0x287
	.byte	0x8
	.uleb128 0x13
	.string	"key"
	.byte	0x9
	.byte	0x4c
	.byte	0xf
	.long	0x287
	.byte	0x10
	.byte	0
	.uleb128 0x5
	.long	.LASF40
	.byte	0x9
	.byte	0x44
	.byte	0xd
	.long	0x89
	.uleb128 0xe
	.long	0x252
	.long	0x29e
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.long	.LASF41
	.byte	0x8
	.byte	0xd4
	.byte	0x1a
	.long	0x293
	.uleb128 0xb
	.long	.LASF42
	.byte	0x8
	.byte	0xd5
	.byte	0x1a
	.long	0x293
	.uleb128 0x10
	.long	.LASF44
	.uleb128 0x3
	.long	0x2b6
	.uleb128 0x11
	.long	.LASF45
	.byte	0x7
	.value	0x1d8
	.byte	0x25
	.long	0x2bb
	.uleb128 0x10
	.long	.LASF46
	.uleb128 0x11
	.long	.LASF47
	.byte	0xa
	.value	0x11d
	.byte	0x24
	.long	0x2cd
	.uleb128 0x14
	.long	0x95
	.long	0x2ee
	.uleb128 0x15
	.long	0x68
	.byte	0
	.uleb128 0x11
	.long	.LASF48
	.byte	0xa
	.value	0x11e
	.byte	0xf
	.long	0x2fb
	.uleb128 0x7
	.byte	0x8
	.long	0x2df
	.uleb128 0x11
	.long	.LASF49
	.byte	0xa
	.value	0x209
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF50
	.byte	0xa
	.value	0x20a
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF51
	.byte	0xa
	.value	0x20b
	.byte	0x16
	.long	0x2d
	.uleb128 0x11
	.long	.LASF52
	.byte	0xa
	.value	0x20c
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF53
	.byte	0xa
	.value	0x20d
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF54
	.byte	0xa
	.value	0x20e
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF55
	.byte	0xa
	.value	0x20f
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF56
	.byte	0xa
	.value	0x210
	.byte	0x16
	.long	0x2d
	.uleb128 0x11
	.long	.LASF57
	.byte	0xa
	.value	0x211
	.byte	0xd
	.long	0xa2
	.uleb128 0x11
	.long	.LASF58
	.byte	0xa
	.value	0x212
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF59
	.byte	0xa
	.value	0x213
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF60
	.byte	0xa
	.value	0x215
	.byte	0xd
	.long	0xa2
	.uleb128 0x11
	.long	.LASF61
	.byte	0xa
	.value	0x21c
	.byte	0x11
	.long	0xcc
	.uleb128 0x11
	.long	.LASF62
	.byte	0xa
	.value	0x230
	.byte	0xc
	.long	0x68
	.uleb128 0x11
	.long	.LASF63
	.byte	0xa
	.value	0x232
	.byte	0xd
	.long	0xa2
	.uleb128 0x16
	.long	.LASF80
	.byte	0x7
	.byte	0x4
	.long	0x40
	.byte	0xa
	.value	0x238
	.byte	0xd
	.long	0x402
	.uleb128 0x17
	.long	.LASF64
	.byte	0
	.uleb128 0x17
	.long	.LASF65
	.byte	0x1
	.uleb128 0x17
	.long	.LASF66
	.byte	0x2
	.uleb128 0x17
	.long	.LASF67
	.byte	0x3
	.uleb128 0x17
	.long	.LASF68
	.byte	0x4
	.uleb128 0x17
	.long	.LASF69
	.byte	0x5
	.uleb128 0x17
	.long	.LASF70
	.byte	0x6
	.byte	0
	.uleb128 0x11
	.long	.LASF71
	.byte	0xa
	.value	0x240
	.byte	0x3
	.long	0x3c4
	.uleb128 0x18
	.long	.LASF73
	.byte	0x3
	.byte	0xa
	.value	0x268
	.byte	0x8
	.long	0x448
	.uleb128 0x19
	.long	.LASF74
	.byte	0xa
	.value	0x269
	.byte	0x7
	.long	0x34
	.byte	0
	.uleb128 0x19
	.long	.LASF75
	.byte	0xa
	.value	0x26a
	.byte	0x7
	.long	0x34
	.byte	0x1
	.uleb128 0x19
	.long	.LASF76
	.byte	0xa
	.value	0x26b
	.byte	0x7
	.long	0xa2
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.long	0x40f
	.uleb128 0xe
	.long	0x448
	.long	0x45d
	.uleb128 0x1a
	.long	0x2d
	.byte	0x1f
	.byte	0
	.uleb128 0x3
	.long	0x44d
	.uleb128 0x11
	.long	.LASF77
	.byte	0xa
	.value	0x26e
	.byte	0x20
	.long	0x45d
	.uleb128 0x11
	.long	.LASF78
	.byte	0xa
	.value	0x270
	.byte	0x13
	.long	0x1d5
	.uleb128 0x11
	.long	.LASF79
	.byte	0xa
	.value	0x27b
	.byte	0x13
	.long	0x1d5
	.uleb128 0x1b
	.long	.LASF81
	.byte	0x7
	.byte	0x4
	.long	0x40
	.byte	0xb
	.byte	0x4b
	.byte	0x6
	.long	0x57a
	.uleb128 0x17
	.long	.LASF82
	.byte	0
	.uleb128 0x17
	.long	.LASF83
	.byte	0x1
	.uleb128 0x17
	.long	.LASF84
	.byte	0x2
	.uleb128 0x17
	.long	.LASF85
	.byte	0x3
	.uleb128 0x17
	.long	.LASF86
	.byte	0x4
	.uleb128 0x17
	.long	.LASF87
	.byte	0x5
	.uleb128 0x17
	.long	.LASF88
	.byte	0x6
	.uleb128 0x17
	.long	.LASF89
	.byte	0x7
	.uleb128 0x17
	.long	.LASF90
	.byte	0x8
	.uleb128 0x17
	.long	.LASF91
	.byte	0x9
	.uleb128 0x17
	.long	.LASF92
	.byte	0xa
	.uleb128 0x17
	.long	.LASF93
	.byte	0xb
	.uleb128 0x17
	.long	.LASF94
	.byte	0xc
	.uleb128 0x17
	.long	.LASF95
	.byte	0xd
	.uleb128 0x17
	.long	.LASF96
	.byte	0xe
	.uleb128 0x17
	.long	.LASF97
	.byte	0xf
	.uleb128 0x17
	.long	.LASF98
	.byte	0x10
	.uleb128 0x17
	.long	.LASF99
	.byte	0x11
	.uleb128 0x17
	.long	.LASF100
	.byte	0x12
	.uleb128 0x17
	.long	.LASF101
	.byte	0x13
	.uleb128 0x17
	.long	.LASF102
	.byte	0x14
	.uleb128 0x17
	.long	.LASF103
	.byte	0x15
	.uleb128 0x17
	.long	.LASF104
	.byte	0x16
	.uleb128 0x17
	.long	.LASF105
	.byte	0x17
	.uleb128 0x17
	.long	.LASF106
	.byte	0x18
	.uleb128 0x17
	.long	.LASF107
	.byte	0x19
	.uleb128 0x17
	.long	.LASF108
	.byte	0x1a
	.uleb128 0x17
	.long	.LASF109
	.byte	0x9
	.uleb128 0x17
	.long	.LASF110
	.byte	0x9
	.uleb128 0x17
	.long	.LASF111
	.byte	0xd
	.uleb128 0x17
	.long	.LASF112
	.byte	0x9
	.uleb128 0x17
	.long	.LASF113
	.byte	0x4
	.uleb128 0x17
	.long	.LASF114
	.byte	0x9
	.uleb128 0x17
	.long	.LASF115
	.byte	0xc
	.uleb128 0x17
	.long	.LASF116
	.byte	0xd
	.uleb128 0x17
	.long	.LASF117
	.byte	0x11
	.uleb128 0x17
	.long	.LASF118
	.byte	0x3
	.byte	0
	.uleb128 0x16
	.long	.LASF119
	.byte	0x7
	.byte	0x4
	.long	0x40
	.byte	0xc
	.value	0x18b
	.byte	0x6
	.long	0x5b2
	.uleb128 0x17
	.long	.LASF120
	.byte	0
	.uleb128 0x17
	.long	.LASF121
	.byte	0x1
	.uleb128 0x17
	.long	.LASF122
	.byte	0x2
	.uleb128 0x17
	.long	.LASF123
	.byte	0x3
	.uleb128 0x17
	.long	.LASF124
	.byte	0x4
	.uleb128 0x17
	.long	.LASF125
	.byte	0x5
	.byte	0
	.uleb128 0x1c
	.string	"foo"
	.byte	0x1
	.byte	0x10
	.byte	0x6
	.quad	.LFB163
	.quad	.LFE163-.LFB163
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
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
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
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
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
	.uleb128 0x5
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
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
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
	.uleb128 0x9
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
	.uleb128 0xa
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
	.uleb128 0xb
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
	.uleb128 0xc
	.uleb128 0x15
	.byte	0
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x15
	.byte	0
	.uleb128 0x27
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x13
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
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
	.uleb128 0x12
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
	.uleb128 0x13
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
	.uleb128 0x14
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
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
	.uleb128 0x17
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x18
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
	.uleb128 0x19
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
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
	.uleb128 0x1a
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
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
	.uleb128 0x1c
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
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
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF21:
	.string	"boot_command_line"
.LASF46:
	.string	"atomic_notifier_head"
.LASF45:
	.string	"kmsg_fops"
.LASF63:
	.string	"early_boot_irqs_disabled"
.LASF84:
	.string	"PG_referenced"
.LASF65:
	.string	"SYSTEM_SCHEDULING"
.LASF15:
	.string	"skip_smp_alternatives"
.LASF20:
	.string	"__con_initcall_end"
.LASF30:
	.string	"console_printk"
.LASF101:
	.string	"PG_unevictable"
.LASF2:
	.string	"unsigned int"
.LASF75:
	.string	"c_false"
.LASF77:
	.string	"taint_flags"
.LASF81:
	.string	"pageflags"
.LASF71:
	.string	"system_state"
.LASF76:
	.string	"module"
.LASF68:
	.string	"SYSTEM_POWER_OFF"
.LASF103:
	.string	"PG_uncached"
.LASF89:
	.string	"PG_waiters"
.LASF64:
	.string	"SYSTEM_BOOTING"
.LASF17:
	.string	"initcall_t"
.LASF99:
	.string	"PG_reclaim"
.LASF56:
	.string	"panic_on_taint"
.LASF105:
	.string	"PG_young"
.LASF111:
	.string	"PG_fscache"
.LASF128:
	.string	"/home/calmwu/Program/kernel-src/linux-4.18.0-348.7.1.el8_5"
.LASF31:
	.string	"devkmsg_log_str"
.LASF114:
	.string	"PG_foreign"
.LASF57:
	.string	"panic_on_taint_nousertaint"
.LASF79:
	.string	"hex_asc_upper"
.LASF74:
	.string	"c_true"
.LASF8:
	.string	"long long unsigned int"
.LASF62:
	.string	"root_mountflags"
.LASF67:
	.string	"SYSTEM_HALT"
.LASF96:
	.string	"PG_writeback"
.LASF110:
	.string	"PG_swapcache"
.LASF100:
	.string	"PG_swapbacked"
.LASF94:
	.string	"PG_private"
.LASF122:
	.string	"ZONE_NORMAL"
.LASF23:
	.string	"reset_devices"
.LASF73:
	.string	"taint_flag"
.LASF108:
	.string	"__NR_PAGEFLAGS"
.LASF37:
	.string	"counter"
.LASF39:
	.string	"target"
.LASF61:
	.string	"panic_cpu"
.LASF86:
	.string	"PG_dirty"
.LASF129:
	.string	"current_stack_pointer"
.LASF91:
	.string	"PG_owner_priv_1"
.LASF12:
	.string	"_Bool"
.LASF72:
	.string	"jump_entry"
.LASF49:
	.string	"oops_in_progress"
.LASF47:
	.string	"panic_notifier_list"
.LASF52:
	.string	"panic_on_oops"
.LASF38:
	.string	"code"
.LASF109:
	.string	"PG_checked"
.LASF1:
	.string	"char"
.LASF44:
	.string	"file_operations"
.LASF118:
	.string	"PG_reported"
.LASF24:
	.string	"rootfs_fs_type"
.LASF88:
	.string	"PG_active"
.LASF92:
	.string	"PG_arch_1"
.LASF87:
	.string	"PG_lru"
.LASF106:
	.string	"PG_idle"
.LASF19:
	.string	"__con_initcall_start"
.LASF14:
	.string	"alternatives_patched"
.LASF28:
	.string	"linux_banner"
.LASF119:
	.string	"zone_type"
.LASF42:
	.string	"__stop___jump_table"
.LASF85:
	.string	"PG_uptodate"
.LASF95:
	.string	"PG_private_2"
.LASF121:
	.string	"ZONE_DMA32"
.LASF34:
	.string	"dmesg_restrict"
.LASF123:
	.string	"ZONE_MOVABLE"
.LASF7:
	.string	"long long int"
.LASF59:
	.string	"sysctl_panic_on_stackoverflow"
.LASF93:
	.string	"PG_reserved"
.LASF70:
	.string	"SYSTEM_SUSPEND"
.LASF97:
	.string	"PG_head"
.LASF25:
	.string	"rodata_enabled"
.LASF11:
	.string	"bool"
.LASF51:
	.string	"panic_print"
.LASF55:
	.string	"panic_on_warn"
.LASF124:
	.string	"ZONE_DEVICE"
.LASF66:
	.string	"SYSTEM_RUNNING"
.LASF26:
	.string	"late_time_init"
.LASF107:
	.string	"PG_workingset"
.LASF54:
	.string	"panic_on_io_nmi"
.LASF41:
	.string	"__start___jump_table"
.LASF29:
	.string	"linux_proc_banner"
.LASF18:
	.string	"initcall_entry_t"
.LASF90:
	.string	"PG_slab"
.LASF33:
	.string	"printk_delay_msec"
.LASF117:
	.string	"PG_isolated"
.LASF9:
	.string	"__u64"
.LASF115:
	.string	"PG_slob_free"
.LASF35:
	.string	"kptr_restrict"
.LASF32:
	.string	"suppress_printk"
.LASF5:
	.string	"short int"
.LASF10:
	.string	"long int"
.LASF113:
	.string	"PG_savepinned"
.LASF69:
	.string	"SYSTEM_RESTART"
.LASF126:
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
.LASF82:
	.string	"PG_locked"
.LASF127:
	.string	"kernel/bounds.c"
.LASF78:
	.string	"hex_asc"
.LASF53:
	.string	"panic_on_unrecovered_nmi"
.LASF116:
	.string	"PG_double_map"
.LASF58:
	.string	"sysctl_panic_on_rcu_stall"
.LASF13:
	.string	"atomic_t"
.LASF50:
	.string	"panic_timeout"
.LASF0:
	.string	"long unsigned int"
.LASF112:
	.string	"PG_pinned"
.LASF80:
	.string	"system_states"
.LASF40:
	.string	"jump_label_t"
.LASF4:
	.string	"unsigned char"
.LASF83:
	.string	"PG_error"
.LASF16:
	.string	"ideal_nops"
.LASF22:
	.string	"saved_command_line"
.LASF43:
	.string	"file_system_type"
.LASF102:
	.string	"PG_mlocked"
.LASF98:
	.string	"PG_mappedtodisk"
.LASF60:
	.string	"crash_kexec_post_notifiers"
.LASF104:
	.string	"PG_hwpoison"
.LASF3:
	.string	"signed char"
.LASF6:
	.string	"short unsigned int"
.LASF27:
	.string	"initcall_debug"
.LASF125:
	.string	"__MAX_NR_ZONES"
.LASF120:
	.string	"ZONE_DMA"
.LASF48:
	.string	"panic_blink"
.LASF36:
	.string	"static_key_initialized"
	.ident	"GCC: (GNU) 8.5.0 20210514 (Red Hat 8.5.0-26.0.1)"
	.section	.note.GNU-stack,"",@progbits
