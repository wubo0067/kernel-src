// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on arch/arm/kernel/signal.c
 *
 * Copyright (C) 1995-2009 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */

#include <linux/cache.h>
#include <linux/compat.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/freezer.h>
#include <linux/stddef.h>
#include <linux/uaccess.h>
#include <linux/sizes.h>
#include <linux/string.h>
#include <linux/resume_user_mode.h>
#include <linux/ratelimit.h>
#include <linux/syscalls.h>

#include <asm/daifflags.h>
#include <asm/debug-monitors.h>
#include <asm/elf.h>
#include <asm/exception.h>
#include <asm/cacheflush.h>
#include <asm/ucontext.h>
#include <asm/unistd.h>
#include <asm/fpsimd.h>
#include <asm/ptrace.h>
#include <asm/syscall.h>
#include <asm/signal32.h>
#include <asm/traps.h>
#include <asm/vdso.h>

/*
 * Do a signal return; undo the signal stack. These are aligned to 128-bit.
 */
struct rt_sigframe {
	struct siginfo info;
	struct ucontext uc;
};

struct frame_record {
	u64 fp;
	u64 lr;
};

struct rt_sigframe_user_layout {
	struct rt_sigframe __user *sigframe;
	struct frame_record __user *next_frame;

	unsigned long size;	/* size of allocated sigframe data */
	unsigned long limit;	/* largest allowed size */

	unsigned long fpsimd_offset;
	unsigned long esr_offset;
	unsigned long sve_offset;
	unsigned long tpidr2_offset;
	unsigned long za_offset;
	unsigned long zt_offset;
	unsigned long extra_offset;
	unsigned long end_offset;
};

#define BASE_SIGFRAME_SIZE round_up(sizeof(struct rt_sigframe), 16)
#define TERMINATOR_SIZE round_up(sizeof(struct _aarch64_ctx), 16)
#define EXTRA_CONTEXT_SIZE round_up(sizeof(struct extra_context), 16)

static void init_user_layout(struct rt_sigframe_user_layout *user)
{
	const size_t reserved_size =
		sizeof(user->sigframe->uc.uc_mcontext.__reserved);

	memset(user, 0, sizeof(*user));
	user->size = offsetof(struct rt_sigframe, uc.uc_mcontext.__reserved);

	user->limit = user->size + reserved_size;

	user->limit -= TERMINATOR_SIZE;
	user->limit -= EXTRA_CONTEXT_SIZE;
	/* Reserve space for extension and terminator ^ */
}

static size_t sigframe_size(struct rt_sigframe_user_layout const *user)
{
	return round_up(max(user->size, sizeof(struct rt_sigframe)), 16);
}

/*
 * Sanity limit on the approximate maximum size of signal frame we'll
 * try to generate.  Stack alignment padding and the frame record are
 * not taken into account.  This limit is not a guarantee and is
 * NOT ABI.
 */
#define SIGFRAME_MAXSZ SZ_256K

static int __sigframe_alloc(struct rt_sigframe_user_layout *user,
			    unsigned long *offset, size_t size, bool extend)
{
	size_t padded_size = round_up(size, 16);

	if (padded_size > user->limit - user->size &&
	    !user->extra_offset &&
	    extend) {
		int ret;

		user->limit += EXTRA_CONTEXT_SIZE;
		ret = __sigframe_alloc(user, &user->extra_offset,
				       sizeof(struct extra_context), false);
		if (ret) {
			user->limit -= EXTRA_CONTEXT_SIZE;
			return ret;
		}

		/* Reserve space for the __reserved[] terminator */
		user->size += TERMINATOR_SIZE;

		/*
		 * Allow expansion up to SIGFRAME_MAXSZ, ensuring space for
		 * the terminator:
		 */
		user->limit = SIGFRAME_MAXSZ - TERMINATOR_SIZE;
	}

	/* Still not enough space?  Bad luck! */
	if (padded_size > user->limit - user->size)
		return -ENOMEM;

	*offset = user->size;
	user->size += padded_size;

	return 0;
}

/*
 * Allocate space for an optional record of <size> bytes in the user
 * signal frame.  The offset from the signal frame base address to the
 * allocated block is assigned to *offset.
 */
static int sigframe_alloc(struct rt_sigframe_user_layout *user,
			  unsigned long *offset, size_t size)
{
	return __sigframe_alloc(user, offset, size, true);
}

/* Allocate the null terminator record and prevent further allocations */
static int sigframe_alloc_end(struct rt_sigframe_user_layout *user)
{
	int ret;

	/* Un-reserve the space reserved for the terminator: */
	user->limit += TERMINATOR_SIZE;

	ret = sigframe_alloc(user, &user->end_offset,
			     sizeof(struct _aarch64_ctx));
	if (ret)
		return ret;

	/* Prevent further allocation: */
	user->limit = user->size;
	return 0;
}

static void __user *apply_user_offset(
	struct rt_sigframe_user_layout const *user, unsigned long offset)
{
	char __user *base = (char __user *)user->sigframe;

	return base + offset;
}

struct user_ctxs {
	struct fpsimd_context __user *fpsimd;
	u32 fpsimd_size;
	struct sve_context __user *sve;
	u32 sve_size;
	struct tpidr2_context __user *tpidr2;
	u32 tpidr2_size;
	struct za_context __user *za;
	u32 za_size;
	struct zt_context __user *zt;
	u32 zt_size;
};

static int preserve_fpsimd_context(struct fpsimd_context __user *ctx)
{
	struct user_fpsimd_state const *fpsimd =
		&current->thread.uw.fpsimd_state;
	int err;

	/* copy the FP and status/control registers */
	err = __copy_to_user(ctx->vregs, fpsimd->vregs, sizeof(fpsimd->vregs));
	__put_user_error(fpsimd->fpsr, &ctx->fpsr, err);
	__put_user_error(fpsimd->fpcr, &ctx->fpcr, err);

	/* copy the magic/size information */
	__put_user_error(FPSIMD_MAGIC, &ctx->head.magic, err);
	__put_user_error(sizeof(struct fpsimd_context), &ctx->head.size, err);

	return err ? -EFAULT : 0;
}

static int restore_fpsimd_context(struct user_ctxs *user)
{
	struct user_fpsimd_state fpsimd;
	int err = 0;

	/* check the size information */
	if (user->fpsimd_size != sizeof(struct fpsimd_context))
		return -EINVAL;

	/* copy the FP and status/control registers */
	err = __copy_from_user(fpsimd.vregs, &(user->fpsimd->vregs),
			       sizeof(fpsimd.vregs));
	__get_user_error(fpsimd.fpsr, &(user->fpsimd->fpsr), err);
	__get_user_error(fpsimd.fpcr, &(user->fpsimd->fpcr), err);

	clear_thread_flag(TIF_SVE);
	current->thread.fp_type = FP_STATE_FPSIMD;

	/* load the hardware registers from the fpsimd_state structure */
	if (!err)
		fpsimd_update_current_state(&fpsimd);

	return err ? -EFAULT : 0;
}


#ifdef CONFIG_ARM64_SVE

static int preserve_sve_context(struct sve_context __user *ctx)
{
	int err = 0;
	u16 reserved[ARRAY_SIZE(ctx->__reserved)];
	u16 flags = 0;
	unsigned int vl = task_get_sve_vl(current);
	unsigned int vq = 0;

	if (thread_sm_enabled(&current->thread)) {
		vl = task_get_sme_vl(current);
		vq = sve_vq_from_vl(vl);
		flags |= SVE_SIG_FLAG_SM;
	} else if (test_thread_flag(TIF_SVE)) {
		vq = sve_vq_from_vl(vl);
	}

	memset(reserved, 0, sizeof(reserved));

	__put_user_error(SVE_MAGIC, &ctx->head.magic, err);
	__put_user_error(round_up(SVE_SIG_CONTEXT_SIZE(vq), 16),
			 &ctx->head.size, err);
	__put_user_error(vl, &ctx->vl, err);
	__put_user_error(flags, &ctx->flags, err);
	BUILD_BUG_ON(sizeof(ctx->__reserved) != sizeof(reserved));
	err |= __copy_to_user(&ctx->__reserved, reserved, sizeof(reserved));

	if (vq) {
		/*
		 * This assumes that the SVE state has already been saved to
		 * the task struct by calling the function
		 * fpsimd_signal_preserve_current_state().
		 */
		err |= __copy_to_user((char __user *)ctx + SVE_SIG_REGS_OFFSET,
				      current->thread.sve_state,
				      SVE_SIG_REGS_SIZE(vq));
	}

	return err ? -EFAULT : 0;
}

static int restore_sve_fpsimd_context(struct user_ctxs *user)
{
	int err = 0;
	unsigned int vl, vq;
	struct user_fpsimd_state fpsimd;
	u16 user_vl, flags;

	if (user->sve_size < sizeof(*user->sve))
		return -EINVAL;

	__get_user_error(user_vl, &(user->sve->vl), err);
	__get_user_error(flags, &(user->sve->flags), err);
	if (err)
		return err;

	if (flags & SVE_SIG_FLAG_SM) {
		if (!system_supports_sme())
			return -EINVAL;

		vl = task_get_sme_vl(current);
	} else {
		/*
		 * A SME only system use SVE for streaming mode so can
		 * have a SVE formatted context with a zero VL and no
		 * payload data.
		 */
		if (!system_supports_sve() && !system_supports_sme())
			return -EINVAL;

		vl = task_get_sve_vl(current);
	}

	if (user_vl != vl)
		return -EINVAL;

	if (user->sve_size == sizeof(*user->sve)) {
		clear_thread_flag(TIF_SVE);
		current->thread.svcr &= ~SVCR_SM_MASK;
		current->thread.fp_type = FP_STATE_FPSIMD;
		goto fpsimd_only;
	}

	vq = sve_vq_from_vl(vl);

	if (user->sve_size < SVE_SIG_CONTEXT_SIZE(vq))
		return -EINVAL;

	/*
	 * Careful: we are about __copy_from_user() directly into
	 * thread.sve_state with preemption enabled, so protection is
	 * needed to prevent a racing context switch from writing stale
	 * registers back over the new data.
	 */

	fpsimd_flush_task_state(current);
	/* From now, fpsimd_thread_switch() won't touch thread.sve_state */

	sve_alloc(current, true);
	if (!current->thread.sve_state) {
		clear_thread_flag(TIF_SVE);
		return -ENOMEM;
	}

	err = __copy_from_user(current->thread.sve_state,
			       (char __user const *)user->sve +
					SVE_SIG_REGS_OFFSET,
			       SVE_SIG_REGS_SIZE(vq));
	if (err)
		return -EFAULT;

	if (flags & SVE_SIG_FLAG_SM)
		current->thread.svcr |= SVCR_SM_MASK;
	else
		set_thread_flag(TIF_SVE);
	current->thread.fp_type = FP_STATE_SVE;

fpsimd_only:
	/* copy the FP and status/control registers */
	/* restore_sigframe() already checked that user->fpsimd != NULL. */
	err = __copy_from_user(fpsimd.vregs, user->fpsimd->vregs,
			       sizeof(fpsimd.vregs));
	__get_user_error(fpsimd.fpsr, &user->fpsimd->fpsr, err);
	__get_user_error(fpsimd.fpcr, &user->fpsimd->fpcr, err);

	/* load the hardware registers from the fpsimd_state structure */
	if (!err)
		fpsimd_update_current_state(&fpsimd);

	return err ? -EFAULT : 0;
}

#else /* ! CONFIG_ARM64_SVE */

static int restore_sve_fpsimd_context(struct user_ctxs *user)
{
	WARN_ON_ONCE(1);
	return -EINVAL;
}

/* Turn any non-optimised out attempts to use this into a link error: */
extern int preserve_sve_context(void __user *ctx);

#endif /* ! CONFIG_ARM64_SVE */

#ifdef CONFIG_ARM64_SME

static int preserve_tpidr2_context(struct tpidr2_context __user *ctx)
{
	int err = 0;

	current->thread.tpidr2_el0 = read_sysreg_s(SYS_TPIDR2_EL0);

	__put_user_error(TPIDR2_MAGIC, &ctx->head.magic, err);
	__put_user_error(sizeof(*ctx), &ctx->head.size, err);
	__put_user_error(current->thread.tpidr2_el0, &ctx->tpidr2, err);

	return err;
}

static int restore_tpidr2_context(struct user_ctxs *user)
{
	u64 tpidr2_el0;
	int err = 0;

	if (user->tpidr2_size != sizeof(*user->tpidr2))
		return -EINVAL;

	__get_user_error(tpidr2_el0, &user->tpidr2->tpidr2, err);
	if (!err)
		write_sysreg_s(tpidr2_el0, SYS_TPIDR2_EL0);

	return err;
}

static int preserve_za_context(struct za_context __user *ctx)
{
	int err = 0;
	u16 reserved[ARRAY_SIZE(ctx->__reserved)];
	unsigned int vl = task_get_sme_vl(current);
	unsigned int vq;

	if (thread_za_enabled(&current->thread))
		vq = sve_vq_from_vl(vl);
	else
		vq = 0;

	memset(reserved, 0, sizeof(reserved));

	__put_user_error(ZA_MAGIC, &ctx->head.magic, err);
	__put_user_error(round_up(ZA_SIG_CONTEXT_SIZE(vq), 16),
			 &ctx->head.size, err);
	__put_user_error(vl, &ctx->vl, err);
	BUILD_BUG_ON(sizeof(ctx->__reserved) != sizeof(reserved));
	err |= __copy_to_user(&ctx->__reserved, reserved, sizeof(reserved));

	if (vq) {
		/*
		 * This assumes that the ZA state has already been saved to
		 * the task struct by calling the function
		 * fpsimd_signal_preserve_current_state().
		 */
		err |= __copy_to_user((char __user *)ctx + ZA_SIG_REGS_OFFSET,
				      current->thread.sme_state,
				      ZA_SIG_REGS_SIZE(vq));
	}

	return err ? -EFAULT : 0;
}

static int restore_za_context(struct user_ctxs *user)
{
	int err = 0;
	unsigned int vq;
	u16 user_vl;

	if (user->za_size < sizeof(*user->za))
		return -EINVAL;

	__get_user_error(user_vl, &(user->za->vl), err);
	if (err)
		return err;

	if (user_vl != task_get_sme_vl(current))
		return -EINVAL;

	if (user->za_size == sizeof(*user->za)) {
		current->thread.svcr &= ~SVCR_ZA_MASK;
		return 0;
	}

	vq = sve_vq_from_vl(user_vl);

	if (user->za_size < ZA_SIG_CONTEXT_SIZE(vq))
		return -EINVAL;

	/*
	 * Careful: we are about __copy_from_user() directly into
	 * thread.sme_state with preemption enabled, so protection is
	 * needed to prevent a racing context switch from writing stale
	 * registers back over the new data.
	 */

	fpsimd_flush_task_state(current);
	/* From now, fpsimd_thread_switch() won't touch thread.sve_state */

	sme_alloc(current, true);
	if (!current->thread.sme_state) {
		current->thread.svcr &= ~SVCR_ZA_MASK;
		clear_thread_flag(TIF_SME);
		return -ENOMEM;
	}

	err = __copy_from_user(current->thread.sme_state,
			       (char __user const *)user->za +
					ZA_SIG_REGS_OFFSET,
			       ZA_SIG_REGS_SIZE(vq));
	if (err)
		return -EFAULT;

	set_thread_flag(TIF_SME);
	current->thread.svcr |= SVCR_ZA_MASK;

	return 0;
}

static int preserve_zt_context(struct zt_context __user *ctx)
{
	int err = 0;
	u16 reserved[ARRAY_SIZE(ctx->__reserved)];

	if (WARN_ON(!thread_za_enabled(&current->thread)))
		return -EINVAL;

	memset(reserved, 0, sizeof(reserved));

	__put_user_error(ZT_MAGIC, &ctx->head.magic, err);
	__put_user_error(round_up(ZT_SIG_CONTEXT_SIZE(1), 16),
			 &ctx->head.size, err);
	__put_user_error(1, &ctx->nregs, err);
	BUILD_BUG_ON(sizeof(ctx->__reserved) != sizeof(reserved));
	err |= __copy_to_user(&ctx->__reserved, reserved, sizeof(reserved));

	/*
	 * This assumes that the ZT state has already been saved to
	 * the task struct by calling the function
	 * fpsimd_signal_preserve_current_state().
	 */
	err |= __copy_to_user((char __user *)ctx + ZT_SIG_REGS_OFFSET,
			      thread_zt_state(&current->thread),
			      ZT_SIG_REGS_SIZE(1));

	return err ? -EFAULT : 0;
}

static int restore_zt_context(struct user_ctxs *user)
{
	int err;
	u16 nregs;

	/* ZA must be restored first for this check to be valid */
	if (!thread_za_enabled(&current->thread))
		return -EINVAL;

	if (user->zt_size != ZT_SIG_CONTEXT_SIZE(1))
		return -EINVAL;

	if (__copy_from_user(&nregs, &(user->zt->nregs), sizeof(nregs)))
		return -EFAULT;

	if (nregs != 1)
		return -EINVAL;

	/*
	 * Careful: we are about __copy_from_user() directly into
	 * thread.zt_state with preemption enabled, so protection is
	 * needed to prevent a racing context switch from writing stale
	 * registers back over the new data.
	 */

	fpsimd_flush_task_state(current);
	/* From now, fpsimd_thread_switch() won't touch ZT in thread state */

	err = __copy_from_user(thread_zt_state(&current->thread),
			       (char __user const *)user->zt +
					ZT_SIG_REGS_OFFSET,
			       ZT_SIG_REGS_SIZE(1));
	if (err)
		return -EFAULT;

	return 0;
}

#else /* ! CONFIG_ARM64_SME */

/* Turn any non-optimised out attempts to use these into a link error: */
extern int preserve_tpidr2_context(void __user *ctx);
extern int restore_tpidr2_context(struct user_ctxs *user);
extern int preserve_za_context(void __user *ctx);
extern int restore_za_context(struct user_ctxs *user);
extern int preserve_zt_context(void __user *ctx);
extern int restore_zt_context(struct user_ctxs *user);

#endif /* ! CONFIG_ARM64_SME */

static int parse_user_sigframe(struct user_ctxs *user,
			       struct rt_sigframe __user *sf)
{
	struct sigcontext __user *const sc = &sf->uc.uc_mcontext;
	struct _aarch64_ctx __user *head;
	char __user *base = (char __user *)&sc->__reserved;
	size_t offset = 0;
	size_t limit = sizeof(sc->__reserved);
	bool have_extra_context = false;
	char const __user *const sfp = (char const __user *)sf;

	user->fpsimd = NULL;
	user->sve = NULL;
	user->tpidr2 = NULL;
	user->za = NULL;
	user->zt = NULL;

	if (!IS_ALIGNED((unsigned long)base, 16))
		goto invalid;

	while (1) {
		int err = 0;
		u32 magic, size;
		char const __user *userp;
		struct extra_context const __user *extra;
		u64 extra_datap;
		u32 extra_size;
		struct _aarch64_ctx const __user *end;
		u32 end_magic, end_size;

		if (limit - offset < sizeof(*head))
			goto invalid;

		if (!IS_ALIGNED(offset, 16))
			goto invalid;

		head = (struct _aarch64_ctx __user *)(base + offset);
		__get_user_error(magic, &head->magic, err);
		__get_user_error(size, &head->size, err);
		if (err)
			return err;

		if (limit - offset < size)
			goto invalid;

		switch (magic) {
		case 0:
			if (size)
				goto invalid;

			goto done;

		case FPSIMD_MAGIC:
			if (!system_supports_fpsimd())
				goto invalid;
			if (user->fpsimd)
				goto invalid;

			user->fpsimd = (struct fpsimd_context __user *)head;
			user->fpsimd_size = size;
			break;

		case ESR_MAGIC:
			/* ignore */
			break;

		case SVE_MAGIC:
			if (!system_supports_sve() && !system_supports_sme())
				goto invalid;

			if (user->sve)
				goto invalid;

			user->sve = (struct sve_context __user *)head;
			user->sve_size = size;
			break;

		case TPIDR2_MAGIC:
			if (!system_supports_tpidr2())
				goto invalid;

			if (user->tpidr2)
				goto invalid;

			user->tpidr2 = (struct tpidr2_context __user *)head;
			user->tpidr2_size = size;
			break;

		case ZA_MAGIC:
			if (!system_supports_sme())
				goto invalid;

			if (user->za)
				goto invalid;

			user->za = (struct za_context __user *)head;
			user->za_size = size;
			break;

		case ZT_MAGIC:
			if (!system_supports_sme2())
				goto invalid;

			if (user->zt)
				goto invalid;

			user->zt = (struct zt_context __user *)head;
			user->zt_size = size;
			break;

		case EXTRA_MAGIC:
			if (have_extra_context)
				goto invalid;

			if (size < sizeof(*extra))
				goto invalid;

			userp = (char const __user *)head;

			extra = (struct extra_context const __user *)userp;
			userp += size;

			__get_user_error(extra_datap, &extra->datap, err);
			__get_user_error(extra_size, &extra->size, err);
			if (err)
				return err;

			/* Check for the dummy terminator in __reserved[]: */

			if (limit - offset - size < TERMINATOR_SIZE)
				goto invalid;

			end = (struct _aarch64_ctx const __user *)userp;
			userp += TERMINATOR_SIZE;

			__get_user_error(end_magic, &end->magic, err);
			__get_user_error(end_size, &end->size, err);
			if (err)
				return err;

			if (end_magic || end_size)
				goto invalid;

			/* Prevent looping/repeated parsing of extra_context */
			have_extra_context = true;

			base = (__force void __user *)extra_datap;
			if (!IS_ALIGNED((unsigned long)base, 16))
				goto invalid;

			if (!IS_ALIGNED(extra_size, 16))
				goto invalid;

			if (base != userp)
				goto invalid;

			/* Reject "unreasonably large" frames: */
			if (extra_size > sfp + SIGFRAME_MAXSZ - userp)
				goto invalid;

			/*
			 * Ignore trailing terminator in __reserved[]
			 * and start parsing extra data:
			 */
			offset = 0;
			limit = extra_size;

			if (!access_ok(base, limit))
				goto invalid;

			continue;

		default:
			goto invalid;
		}

		if (size < sizeof(*head))
			goto invalid;

		if (limit - offset < size)
			goto invalid;

		offset += size;
	}

done:
	return 0;

invalid:
	return -EINVAL;
}

static int restore_sigframe(struct pt_regs *regs,
			    struct rt_sigframe __user *sf)
{
	sigset_t set;
	int i, err;
	struct user_ctxs user;

	err = __copy_from_user(&set, &sf->uc.uc_sigmask, sizeof(set));
	if (err == 0)
		set_current_blocked(&set);

	for (i = 0; i < 31; i++)
		__get_user_error(regs->regs[i], &sf->uc.uc_mcontext.regs[i],
				 err);
	__get_user_error(regs->sp, &sf->uc.uc_mcontext.sp, err);
	__get_user_error(regs->pc, &sf->uc.uc_mcontext.pc, err);
	__get_user_error(regs->pstate, &sf->uc.uc_mcontext.pstate, err);

	/*
	 * Avoid sys_rt_sigreturn() restarting.
	 */
	forget_syscall(regs);

	err |= !valid_user_regs(&regs->user_regs, current);
	if (err == 0)
		err = parse_user_sigframe(&user, sf);

	if (err == 0 && system_supports_fpsimd()) {
		if (!user.fpsimd)
			return -EINVAL;

		if (user.sve)
			err = restore_sve_fpsimd_context(&user);
		else
			err = restore_fpsimd_context(&user);
	}

	if (err == 0 && system_supports_tpidr2() && user.tpidr2)
		err = restore_tpidr2_context(&user);

	if (err == 0 && system_supports_sme() && user.za)
		err = restore_za_context(&user);

	if (err == 0 && system_supports_sme2() && user.zt)
		err = restore_zt_context(&user);

	return err;
}

SYSCALL_DEFINE0(rt_sigreturn)
{
	struct pt_regs *regs = current_pt_regs();
	struct rt_sigframe __user *frame;

	/* Always make any pending restarted system calls return -EINTR */
	current->restart_block.fn = do_no_restart_syscall;

	/*
	 * Since we stacked the signal on a 128-bit boundary, then 'sp' should
	 * be word aligned here.
	 */
	if (regs->sp & 15)
		goto badframe;

	frame = (struct rt_sigframe __user *)regs->sp;

	if (!access_ok(frame, sizeof (*frame)))
		goto badframe;

	if (restore_sigframe(regs, frame))
		goto badframe;

	if (restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	return regs->regs[0];

badframe:
	arm64_notify_segfault(regs->sp);
	return 0;
}

/*
 * Determine the layout of optional records in the signal frame
 *
 * add_all: if true, lays out the biggest possible signal frame for
 *	this task; otherwise, generates a layout for the current state
 *	of the task.
 */
static int setup_sigframe_layout(struct rt_sigframe_user_layout *user,
				 bool add_all)
{
	int err;

	if (system_supports_fpsimd()) {
		err = sigframe_alloc(user, &user->fpsimd_offset,
				     sizeof(struct fpsimd_context));
		if (err)
			return err;
	}

	/* fault information, if valid */
	if (add_all || current->thread.fault_code) {
		err = sigframe_alloc(user, &user->esr_offset,
				     sizeof(struct esr_context));
		if (err)
			return err;
	}

	if (system_supports_sve() || system_supports_sme()) {
		unsigned int vq = 0;

		if (add_all || test_thread_flag(TIF_SVE) ||
		    thread_sm_enabled(&current->thread)) {
			int vl = max(sve_max_vl(), sme_max_vl());

			if (!add_all)
				vl = thread_get_cur_vl(&current->thread);

			vq = sve_vq_from_vl(vl);
		}

		err = sigframe_alloc(user, &user->sve_offset,
				     SVE_SIG_CONTEXT_SIZE(vq));
		if (err)
			return err;
	}

	if (system_supports_tpidr2()) {
		err = sigframe_alloc(user, &user->tpidr2_offset,
				     sizeof(struct tpidr2_context));
		if (err)
			return err;
	}

	if (system_supports_sme()) {
		unsigned int vl;
		unsigned int vq = 0;

		if (add_all)
			vl = sme_max_vl();
		else
			vl = task_get_sme_vl(current);

		if (thread_za_enabled(&current->thread))
			vq = sve_vq_from_vl(vl);

		err = sigframe_alloc(user, &user->za_offset,
				     ZA_SIG_CONTEXT_SIZE(vq));
		if (err)
			return err;
	}

	if (system_supports_sme2()) {
		if (add_all || thread_za_enabled(&current->thread)) {
			err = sigframe_alloc(user, &user->zt_offset,
					     ZT_SIG_CONTEXT_SIZE(1));
			if (err)
				return err;
		}
	}

	return sigframe_alloc_end(user);
}

static int setup_sigframe(struct rt_sigframe_user_layout *user,
			  struct pt_regs *regs, sigset_t *set)
{
	int i, err = 0;
	struct rt_sigframe __user *sf = user->sigframe;

	/* set up the stack frame for unwinding */
	__put_user_error(regs->regs[29], &user->next_frame->fp, err);
	__put_user_error(regs->regs[30], &user->next_frame->lr, err);

	for (i = 0; i < 31; i++)
		__put_user_error(regs->regs[i], &sf->uc.uc_mcontext.regs[i],
				 err);
	__put_user_error(regs->sp, &sf->uc.uc_mcontext.sp, err);
	__put_user_error(regs->pc, &sf->uc.uc_mcontext.pc, err);
	__put_user_error(regs->pstate, &sf->uc.uc_mcontext.pstate, err);

	__put_user_error(current->thread.fault_address, &sf->uc.uc_mcontext.fault_address, err);

	err |= __copy_to_user(&sf->uc.uc_sigmask, set, sizeof(*set));

	if (err == 0 && system_supports_fpsimd()) {
		struct fpsimd_context __user *fpsimd_ctx =
			apply_user_offset(user, user->fpsimd_offset);
		err |= preserve_fpsimd_context(fpsimd_ctx);
	}

	/* fault information, if valid */
	if (err == 0 && user->esr_offset) {
		struct esr_context __user *esr_ctx =
			apply_user_offset(user, user->esr_offset);

		__put_user_error(ESR_MAGIC, &esr_ctx->head.magic, err);
		__put_user_error(sizeof(*esr_ctx), &esr_ctx->head.size, err);
		__put_user_error(current->thread.fault_code, &esr_ctx->esr, err);
	}

	/* Scalable Vector Extension state (including streaming), if present */
	if ((system_supports_sve() || system_supports_sme()) &&
	    err == 0 && user->sve_offset) {
		struct sve_context __user *sve_ctx =
			apply_user_offset(user, user->sve_offset);
		err |= preserve_sve_context(sve_ctx);
	}

	/* TPIDR2 if supported */
	if (system_supports_tpidr2() && err == 0) {
		struct tpidr2_context __user *tpidr2_ctx =
			apply_user_offset(user, user->tpidr2_offset);
		err |= preserve_tpidr2_context(tpidr2_ctx);
	}

	/* ZA state if present */
	if (system_supports_sme() && err == 0 && user->za_offset) {
		struct za_context __user *za_ctx =
			apply_user_offset(user, user->za_offset);
		err |= preserve_za_context(za_ctx);
	}

	/* ZT state if present */
	if (system_supports_sme2() && err == 0 && user->zt_offset) {
		struct zt_context __user *zt_ctx =
			apply_user_offset(user, user->zt_offset);
		err |= preserve_zt_context(zt_ctx);
	}

	if (err == 0 && user->extra_offset) {
		char __user *sfp = (char __user *)user->sigframe;
		char __user *userp =
			apply_user_offset(user, user->extra_offset);

		struct extra_context __user *extra;
		struct _aarch64_ctx __user *end;
		u64 extra_datap;
		u32 extra_size;

		extra = (struct extra_context __user *)userp;
		userp += EXTRA_CONTEXT_SIZE;

		end = (struct _aarch64_ctx __user *)userp;
		userp += TERMINATOR_SIZE;

		/*
		 * extra_datap is just written to the signal frame.
		 * The value gets cast back to a void __user *
		 * during sigreturn.
		 */
		extra_datap = (__force u64)userp;
		extra_size = sfp + round_up(user->size, 16) - userp;

		__put_user_error(EXTRA_MAGIC, &extra->head.magic, err);
		__put_user_error(EXTRA_CONTEXT_SIZE, &extra->head.size, err);
		__put_user_error(extra_datap, &extra->datap, err);
		__put_user_error(extra_size, &extra->size, err);

		/* Add the terminator */
		__put_user_error(0, &end->magic, err);
		__put_user_error(0, &end->size, err);
	}

	/* set the "end" magic */
	if (err == 0) {
		struct _aarch64_ctx __user *end =
			apply_user_offset(user, user->end_offset);

		__put_user_error(0, &end->magic, err);
		__put_user_error(0, &end->size, err);
	}

	return err;
}

static int get_sigframe(struct rt_sigframe_user_layout *user,
			 struct ksignal *ksig, struct pt_regs *regs)
{
	unsigned long sp, sp_top;
	int err;

	init_user_layout(user);
	err = setup_sigframe_layout(user, false);
	if (err)
		return err;

	sp = sp_top = sigsp(regs->sp, ksig);

	sp = round_down(sp - sizeof(struct frame_record), 16);
	user->next_frame = (struct frame_record __user *)sp;

	sp = round_down(sp, 16) - sigframe_size(user);
	user->sigframe = (struct rt_sigframe __user *)sp;

	/*
	 * Check that we can actually write to the signal frame.
	 */
	if (!access_ok(user->sigframe, sp_top - sp))
		return -EFAULT;

	return 0;
}

static void setup_return(struct pt_regs *regs, struct k_sigaction *ka,
			 struct rt_sigframe_user_layout *user, int usig)
{
	__sigrestore_t sigtramp;

	regs->regs[0] = usig;
	regs->sp = (unsigned long)user->sigframe;
	regs->regs[29] = (unsigned long)&user->next_frame->fp;
	regs->pc = (unsigned long)ka->sa.sa_handler;

	/*
	 * Signal delivery is a (wacky) indirect function call in
	 * userspace, so simulate the same setting of BTYPE as a BLR
	 * <register containing the signal handler entry point>.
	 * Signal delivery to a location in a PROT_BTI guarded page
	 * that is not a function entry point will now trigger a
	 * SIGILL in userspace.
	 *
	 * If the signal handler entry point is not in a PROT_BTI
	 * guarded page, this is harmless.
	 */
	if (system_supports_bti()) {
		regs->pstate &= ~PSR_BTYPE_MASK;
		regs->pstate |= PSR_BTYPE_C;
	}

	/* TCO (Tag Check Override) always cleared for signal handlers */
	regs->pstate &= ~PSR_TCO_BIT;

	/* Signal handlers are invoked with ZA and streaming mode disabled */
	if (system_supports_sme()) {
		/*
		 * If we were in streaming mode the saved register
		 * state was SVE but we will exit SM and use the
		 * FPSIMD register state - flush the saved FPSIMD
		 * register state in case it gets loaded.
		 */
		if (current->thread.svcr & SVCR_SM_MASK) {
			memset(&current->thread.uw.fpsimd_state, 0,
			       sizeof(current->thread.uw.fpsimd_state));
			current->thread.fp_type = FP_STATE_FPSIMD;
		}

		current->thread.svcr &= ~(SVCR_ZA_MASK |
					  SVCR_SM_MASK);
		sme_smstop();
	}

	if (ka->sa.sa_flags & SA_RESTORER)
		sigtramp = ka->sa.sa_restorer;
	else
		sigtramp = VDSO_SYMBOL(current->mm->context.vdso, sigtramp);

	regs->regs[30] = (unsigned long)sigtramp;
}

static int setup_rt_frame(int usig, struct ksignal *ksig, sigset_t *set,
			  struct pt_regs *regs)
{
	struct rt_sigframe_user_layout user;
	struct rt_sigframe __user *frame;
	int err = 0;

	fpsimd_signal_preserve_current_state();

	if (get_sigframe(&user, ksig, regs))
		return 1;

	frame = user.sigframe;

	__put_user_error(0, &frame->uc.uc_flags, err);
	__put_user_error(NULL, &frame->uc.uc_link, err);

	err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
	err |= setup_sigframe(&user, regs, set);
	if (err == 0) {
		setup_return(regs, &ksig->ka, &user, usig);
		if (ksig->ka.sa.sa_flags & SA_SIGINFO) {
			err |= copy_siginfo_to_user(&frame->info, &ksig->info);
			regs->regs[1] = (unsigned long)&frame->info;
			regs->regs[2] = (unsigned long)&frame->uc;
		}
	}

	return err;
}

static void setup_restart_syscall(struct pt_regs *regs)
{
	if (is_compat_task())
		compat_setup_restart_syscall(regs);
	else
		regs->regs[8] = __NR_restart_syscall;
}

/*
 * OK, we're invoking a handler
 */
static void handle_signal(struct ksignal *ksig, struct pt_regs *regs)
{
	sigset_t *oldset = sigmask_to_save();
	int usig = ksig->sig;
	int ret;

	rseq_signal_deliver(ksig, regs);

	/*
	 * Set up the stack frame
	 */
	if (is_compat_task()) {
		if (ksig->ka.sa.sa_flags & SA_SIGINFO)
			ret = compat_setup_rt_frame(usig, ksig, oldset, regs);
		else
			ret = compat_setup_frame(usig, ksig, oldset, regs);
	} else {
		ret = setup_rt_frame(usig, ksig, oldset, regs);
	}

	/*
	 * Check that the resulting registers are actually sane.
	 */
	ret |= !valid_user_regs(&regs->user_regs, current);

	/* Step into the signal handler if we are stepping */
	signal_setup_done(ret, ksig, test_thread_flag(TIF_SINGLESTEP));
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 *
 * Note that we go through the signals twice: once to check the signals that
 * the kernel can handle, and then we build all the user-level signal handling
 * stack-frames in one go after that.
 */
static void do_signal(struct pt_regs *regs)
{
	unsigned long continue_addr = 0, restart_addr = 0;
	int retval = 0;
	struct ksignal ksig;
	bool syscall = in_syscall(regs);

	/*
	 * If we were from a system call, check for system call restarting...
	 */
	if (syscall) {
		continue_addr = regs->pc;
		restart_addr = continue_addr - (compat_thumb_mode(regs) ? 2 : 4);
		retval = regs->regs[0];

		/*
		 * Avoid additional syscall restarting via ret_to_user.
		 */
		forget_syscall(regs);

		/*
		 * Prepare for system call restart. We do this here so that a
		 * debugger will see the already changed PC.
		 */
		switch (retval) {
		case -ERESTARTNOHAND:
		case -ERESTARTSYS:
		case -ERESTARTNOINTR:
		case -ERESTART_RESTARTBLOCK:
			regs->regs[0] = regs->orig_x0;
			regs->pc = restart_addr;
			break;
		}
	}

	/*
	 * Get the signal to deliver. When running under ptrace, at this point
	 * the debugger may change all of our registers.
	 */
	if (get_signal(&ksig)) {
		/*
		 * Depending on the signal settings, we may need to revert the
		 * decision to restart the system call, but skip this if a
		 * debugger has chosen to restart at a different PC.
		 */
		if (regs->pc == restart_addr &&
		    (retval == -ERESTARTNOHAND ||
		     retval == -ERESTART_RESTARTBLOCK ||
		     (retval == -ERESTARTSYS &&
		      !(ksig.ka.sa.sa_flags & SA_RESTART)))) {
			syscall_set_return_value(current, regs, -EINTR, 0);
			regs->pc = continue_addr;
		}

		handle_signal(&ksig, regs);
		return;
	}

	/*
	 * Handle restarting a different system call. As above, if a debugger
	 * has chosen to restart at a different PC, ignore the restart.
	 */
	if (syscall && regs->pc == restart_addr) {
		if (retval == -ERESTART_RESTARTBLOCK)
			setup_restart_syscall(regs);
		user_rewind_single_step(current);
	}

	restore_saved_sigmask();
}

void do_notify_resume(struct pt_regs *regs, unsigned long thread_flags)
{
	do {
		if (thread_flags & _TIF_NEED_RESCHED_MASK) {
			/* Unmask Debug and SError for the next task */
			local_daif_restore(DAIF_PROCCTX_NOIRQ);

			schedule();
		} else {
			local_daif_restore(DAIF_PROCCTX);

			if (thread_flags & _TIF_UPROBE)
				uprobe_notify_resume(regs);

			if (thread_flags & _TIF_MTE_ASYNC_FAULT) {
				clear_thread_flag(TIF_MTE_ASYNC_FAULT);
				send_sig_fault(SIGSEGV, SEGV_MTEAERR,
					       (void __user *)NULL, current);
			}

			if (thread_flags & (_TIF_SIGPENDING | _TIF_NOTIFY_SIGNAL))
				do_signal(regs);

			if (thread_flags & _TIF_NOTIFY_RESUME)
				resume_user_mode_work(regs);

			if (thread_flags & _TIF_FOREIGN_FPSTATE)
				fpsimd_restore_current_state();
		}

		local_daif_mask();
		thread_flags = read_thread_flags();
	} while (thread_flags & _TIF_WORK_MASK);
}

unsigned long __ro_after_init signal_minsigstksz;

/*
 * Determine the stack space required for guaranteed signal devliery.
 * This function is used to populate AT_MINSIGSTKSZ at process startup.
 * cpufeatures setup is assumed to be complete.
 */
void __init minsigstksz_setup(void)
{
	struct rt_sigframe_user_layout user;

	init_user_layout(&user);

	/*
	 * If this fails, SIGFRAME_MAXSZ needs to be enlarged.  It won't
	 * be big enough, but it's our best guess:
	 */
	if (WARN_ON(setup_sigframe_layout(&user, true)))
		return;

	signal_minsigstksz = sigframe_size(&user) +
		round_up(sizeof(struct frame_record), 16) +
		16; /* max alignment padding */
}

/*
 * Compile-time assertions for siginfo_t offsets. Check NSIG* as well, as
 * changes likely come with new fields that should be added below.
 */
static_assert(NSIGILL	== 11);
static_assert(NSIGFPE	== 15);
static_assert(NSIGSEGV	== 9);
static_assert(NSIGBUS	== 5);
static_assert(NSIGTRAP	== 6);
static_assert(NSIGCHLD	== 6);
static_assert(NSIGSYS	== 2);
static_assert(sizeof(siginfo_t) == 128);
static_assert(__alignof__(siginfo_t) == 8);
static_assert(offsetof(siginfo_t, si_signo)	== 0x00);
static_assert(offsetof(siginfo_t, si_errno)	== 0x04);
static_assert(offsetof(siginfo_t, si_code)	== 0x08);
static_assert(offsetof(siginfo_t, si_pid)	== 0x10);
static_assert(offsetof(siginfo_t, si_uid)	== 0x14);
static_assert(offsetof(siginfo_t, si_tid)	== 0x10);
static_assert(offsetof(siginfo_t, si_overrun)	== 0x14);
static_assert(offsetof(siginfo_t, si_status)	== 0x18);
static_assert(offsetof(siginfo_t, si_utime)	== 0x20);
static_assert(offsetof(siginfo_t, si_stime)	== 0x28);
static_assert(offsetof(siginfo_t, si_value)	== 0x18);
static_assert(offsetof(siginfo_t, si_int)	== 0x18);
static_assert(offsetof(siginfo_t, si_ptr)	== 0x18);
static_assert(offsetof(siginfo_t, si_addr)	== 0x10);
static_assert(offsetof(siginfo_t, si_addr_lsb)	== 0x18);
static_assert(offsetof(siginfo_t, si_lower)	== 0x20);
static_assert(offsetof(siginfo_t, si_upper)	== 0x28);
static_assert(offsetof(siginfo_t, si_pkey)	== 0x20);
static_assert(offsetof(siginfo_t, si_perf_data)	== 0x18);
static_assert(offsetof(siginfo_t, si_perf_type)	== 0x20);
static_assert(offsetof(siginfo_t, si_perf_flags) == 0x24);
static_assert(offsetof(siginfo_t, si_band)	== 0x10);
static_assert(offsetof(siginfo_t, si_fd)	== 0x18);
static_assert(offsetof(siginfo_t, si_call_addr)	== 0x10);
static_assert(offsetof(siginfo_t, si_syscall)	== 0x18);
static_assert(offsetof(siginfo_t, si_arch)	== 0x1c);
