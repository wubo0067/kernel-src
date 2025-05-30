/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_PREEMPT_H
#define __ASM_PREEMPT_H

#include <asm/rmwcc.h>
#include <asm/percpu.h>
#include <linux/thread_info.h>

DECLARE_PER_CPU(int, __preempt_count);

/* We use the MSB mostly because its available */
#define PREEMPT_NEED_RESCHED 0x80000000

/*
 * We use the PREEMPT_NEED_RESCHED bit as an inverted NEED_RESCHED such
 * that a decrement hitting 0 means we can and should reschedule.
 */
#define PREEMPT_ENABLED (0 + PREEMPT_NEED_RESCHED)

/*
 * We mask the PREEMPT_NEED_RESCHED bit so as not to confuse all current users
 * that think a non-zero value indicates we cannot preempt.
 */
static __always_inline int preempt_count(void)
{
	/*
	这是一个底层的 per-CPU 变量读取操作,4 表示 4 字节数据
	bit 0-7  : preempt count, 抢占计数器的低 8 位用于记录抢占深度。
	bit 8: 第 8 位（PREEMPT_NEED_RESCHED）用于标记是否需要重新调度。

	举例：
	preempt_count = 0x103  // 二进制：0001 0000 0011
	~PREEMPT_NEED_RESCHED = 0x7fffffff

	清除重调度标志
	*/
	return raw_cpu_read_4(__preempt_count) & ~PREEMPT_NEED_RESCHED;
}

static __always_inline void preempt_count_set(int pc)
{
	int old, new;

	do {
		old = raw_cpu_read_4(__preempt_count);
		new = (old & PREEMPT_NEED_RESCHED) |
		      (pc & ~PREEMPT_NEED_RESCHED);
	} while (raw_cpu_cmpxchg_4(__preempt_count, old, new) != old);
}

/*
 * must be macros to avoid header recursion hell
 */
#define init_task_preempt_count(p)                                             \
	do {                                                                   \
	} while (0)

#define init_idle_preempt_count(p, cpu)                                        \
	do {                                                                   \
		per_cpu(__preempt_count, (cpu)) = PREEMPT_ENABLED;             \
	} while (0)

/*
 * We fold the NEED_RESCHED bit into the preempt count such that
 * preempt_enable() can decrement and test for needing to reschedule with a
 * single instruction.
 *
 * We invert the actual bit, so that when the decrement hits 0 we know we both
 * need to resched (the bit is cleared) and can resched (no preempt count).
 */

static __always_inline void set_preempt_need_resched(void)
{
	raw_cpu_and_4(__preempt_count, ~PREEMPT_NEED_RESCHED);
}

static __always_inline void clear_preempt_need_resched(void)
{
	// raw_cpu_or_4() 是一个底层操作，对 __preempt_count 执行按位或操作
	// 通过对 __preempt_count 进行按位或操作，该函数实际上是设置了 PREEMPT_NEED_RESCHED 标志
	// 这看起来可能有点反直觉（函数名是 "clear"，但操作是设置标志），但这是因为 PREEMPT_NEED_RESCHED 标志在
	// __preempt_count 中的实现方式。

	// **和标志 TIF_NEET_RESCHED 容易混淆，这个是调度机制，标识任务是否需要重新被调度，PREEMPT_NEED_RESCHED 指示是否可以
	// **安全的进行抢占
	raw_cpu_or_4(__preempt_count, PREEMPT_NEED_RESCHED);
}

static __always_inline bool test_preempt_need_resched(void)
{
	return !(raw_cpu_read_4(__preempt_count) & PREEMPT_NEED_RESCHED);
}

/*
 * The various preempt_count add/sub methods
 */

static __always_inline void __preempt_count_add(int val)
{
	raw_cpu_add_4(__preempt_count, val);
}

static __always_inline void __preempt_count_sub(int val)
{
	raw_cpu_add_4(__preempt_count, -val);
}

/*
 * Because we keep PREEMPT_NEED_RESCHED set when we do _not_ need to reschedule
 * a decrement which hits zero means we have no preempt_count and should
 * reschedule.
 */
static __always_inline bool __preempt_count_dec_and_test(void)
{
	return GEN_UNARY_RMWcc("decl", __preempt_count, e, __percpu_arg([var]));
}

/*
 * Returns true when we need to resched and can (barring IRQ state).
 */
static __always_inline bool should_resched(int preempt_offset)
{
	return unlikely(raw_cpu_read_4(__preempt_count) == preempt_offset);
}

#ifdef CONFIG_PREEMPTION
extern asmlinkage void ___preempt_schedule(void);
#define __preempt_schedule()                                                   \
	asm volatile("call ___preempt_schedule" : ASM_CALL_CONSTRAINT)

extern asmlinkage void preempt_schedule(void);
extern asmlinkage void ___preempt_schedule_notrace(void);
#define __preempt_schedule_notrace()                                           \
	asm volatile("call ___preempt_schedule_notrace" : ASM_CALL_CONSTRAINT)

extern asmlinkage void preempt_schedule_notrace(void);
#endif

#endif /* __ASM_PREEMPT_H */
