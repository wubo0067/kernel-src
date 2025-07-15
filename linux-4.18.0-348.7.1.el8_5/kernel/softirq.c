/*
 *	linux/kernel/softirq.c
 *
 *	Copyright (C) 1992 Linus Torvalds
 *
 *	Distribute under GPLv2.
 *
 *	Rewritten. Old one was good in 2.2, but in 2.3 it was immoral. --ANK (990903)
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/export.h>
#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/notifier.h>
#include <linux/percpu.h>
#include <linux/cpu.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/rcupdate.h>
#include <linux/ftrace.h>
#include <linux/smp.h>
#include <linux/smpboot.h>
#include <linux/tick.h>
#include <linux/irq.h>

#define CREATE_TRACE_POINTS
#include <trace/events/irq.h>

/*
   - No shared variables, all the data are CPU local.
   - If a softirq needs serialization, let it serialize itself
     by its own spinlocks.
   - Even if softirq is serialized, only local cpu is marked for
     execution. Hence, we get something sort of weak cpu binding.
     Though it is still not clear, will it result in better locality
     or will not.

   Examples:
   - NET RX softirq. It is multithreaded and does not require
     any global serialization.
   - NET TX softirq. It kicks software netdevice queues, hence
     it is logically serialized per device, but this serialization
     is invisible to common code.
   - Tasklets: serialized wrt itself.
 */

#ifndef __ARCH_IRQ_STAT
DEFINE_PER_CPU_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);
#endif

static struct softirq_action softirq_vec[NR_SOFTIRQS] __cacheline_aligned_in_smp;

DEFINE_PER_CPU(struct task_struct *, ksoftirqd);

const char *const softirq_to_name[NR_SOFTIRQS] = {
	"HI",	    "TIMER",   "NET_TX", "NET_RX",  "BLOCK",
	"IRQ_POLL", "TASKLET", "SCHED",	 "HRTIMER", "RCU"
};

/*
 * we cannot loop indefinitely here to avoid userspace starvation,
 * but we also don't want to introduce a worst case 1/HZ latency
 * to the pending events, so lets the scheduler to balance
 * the softirq load for us.
 */
static void wakeup_softirqd(void)
{
	/* Interrupts are disabled: no need to stop preemption */
	struct task_struct *tsk = __this_cpu_read(ksoftirqd);

	if (tsk && tsk->state != TASK_RUNNING)
		wake_up_process(tsk);
}

/*
 * If ksoftirqd is scheduled, we do not want to process pending softirqs
 * right now. Let ksoftirqd handle this at its own rate, to get fairness,
 * unless we're doing some of the synchronous softirqs.
 */
#define SOFTIRQ_NOW_MASK ((1 << HI_SOFTIRQ) | (1 << TASKLET_SOFTIRQ))
static bool ksoftirqd_running(unsigned long pending)
{
	struct task_struct *tsk = __this_cpu_read(ksoftirqd);

	if (pending & SOFTIRQ_NOW_MASK)
		return false;
	return tsk && (tsk->state == TASK_RUNNING);
}

/*
 * preempt_count and SOFTIRQ_OFFSET usage:
 * - preempt_count is changed by SOFTIRQ_OFFSET on entering or leaving
 *   softirq processing.
 * - preempt_count is changed by SOFTIRQ_DISABLE_OFFSET (= 2 * SOFTIRQ_OFFSET)
 *   on local_bh_disable or local_bh_enable.
 * This lets us distinguish between whether we are currently processing
 * softirq and whether we just have bh disabled.
 */

/*
 * This one is for softirq.c-internal use,
 * where hardirqs are disabled legitimately:
 */
#ifdef CONFIG_TRACE_IRQFLAGS

DEFINE_PER_CPU(int, hardirqs_enabled);
DEFINE_PER_CPU(int, hardirq_context);
EXPORT_PER_CPU_SYMBOL_GPL(hardirqs_enabled);
EXPORT_PER_CPU_SYMBOL_GPL(hardirq_context);

void __local_bh_disable_ip(unsigned long ip, unsigned int cnt)
{
	unsigned long flags;

	WARN_ON_ONCE(in_irq());

	raw_local_irq_save(flags);
	/*
	 * The preempt tracer hooks into preempt_count_add and will break
	 * lockdep because it calls back into lockdep after SOFTIRQ_OFFSET
	 * is set and before current->softirq_enabled is cleared.
	 * We must manually increment preempt_count here and manually
	 * call the trace_preempt_off later.
	 */
	__preempt_count_add(cnt);
	/*
	 * Were softirqs turned off above:
	 */
	if (softirq_count() == (cnt & SOFTIRQ_MASK))
		lockdep_softirqs_off(ip);
	raw_local_irq_restore(flags);

	if (preempt_count() == cnt) {
#ifdef CONFIG_DEBUG_PREEMPT
		current->preempt_disable_ip = get_lock_parent_ip();
#endif
		trace_preempt_off(CALLER_ADDR0, get_lock_parent_ip());
	}
}
EXPORT_SYMBOL(__local_bh_disable_ip);
#endif /* CONFIG_TRACE_IRQFLAGS */

static void __local_bh_enable(unsigned int cnt)
{
	lockdep_assert_irqs_disabled();

	if (preempt_count() == cnt)
		trace_preempt_on(CALLER_ADDR0, get_lock_parent_ip());

	if (softirq_count() == (cnt & SOFTIRQ_MASK))
		lockdep_softirqs_on(_RET_IP_);

	__preempt_count_sub(cnt);
}

/*
 * Special-case - softirqs can safely be enabled by __do_softirq(),
 * without processing still-pending softirqs:
 */
void _local_bh_enable(void)
{
	WARN_ON_ONCE(in_irq());
	__local_bh_enable(SOFTIRQ_DISABLE_OFFSET);
}
EXPORT_SYMBOL(_local_bh_enable);

void __local_bh_enable_ip(unsigned long ip, unsigned int cnt)
{
	WARN_ON_ONCE(in_irq());
	lockdep_assert_irqs_enabled();
#ifdef CONFIG_TRACE_IRQFLAGS
	local_irq_disable();
#endif
	/*
	 * Are softirqs going to be turned on now:
	 */
	if (softirq_count() == SOFTIRQ_DISABLE_OFFSET)
		lockdep_softirqs_on(ip);
	/*
	 * Keep preemption disabled until we are done with
	 * softirq processing:
	 */
	preempt_count_sub(cnt - 1);

	if (unlikely(!in_interrupt() && local_softirq_pending())) {
		/*
		 * Run softirq if any pending. And do it in its own stack
		 * as we may be calling this deep in a task call stack already.
		 */
		do_softirq();
	}

	preempt_count_dec();
#ifdef CONFIG_TRACE_IRQFLAGS
	local_irq_enable();
#endif
	preempt_check_resched();
}
EXPORT_SYMBOL(__local_bh_enable_ip);

/*
 * We restart softirq processing for at most MAX_SOFTIRQ_RESTART times,
 * but break the loop if need_resched() is set or after 2 ms.
 * The MAX_SOFTIRQ_TIME provides a nice upper bound in most cases, but in
 * certain cases, such as stop_machine(), jiffies may cease to
 * increment and so we need the MAX_SOFTIRQ_RESTART limit as
 * well to make sure we eventually return from this method.
 *
 * These limits have been established via experimentation.
 * The two things to balance is latency against fairness -
 * we want to handle softirqs as soon as possible, but they
 * should not be able to lock up the box.
 */
#define MAX_SOFTIRQ_TIME msecs_to_jiffies(2)
#define MAX_SOFTIRQ_RESTART 10

#ifdef CONFIG_TRACE_IRQFLAGS
/*
 * When we run softirqs from irq_exit() and thus on the hardirq stack we need
 * to keep the lockdep irq context tracking as tight as possible in order to
 * not miss-qualify lock contexts and miss possible deadlocks.
 */

static inline bool lockdep_softirq_start(void)
{
	bool in_hardirq = false;

	if (lockdep_hardirq_context()) {
		in_hardirq = true;
		lockdep_hardirq_exit();
	}

	lockdep_softirq_enter();

	return in_hardirq;
}

static inline void lockdep_softirq_end(bool in_hardirq)
{
	lockdep_softirq_exit();

	if (in_hardirq)
		lockdep_hardirq_enter();
}
#else
static inline bool lockdep_softirq_start(void)
{
	return false;
}
static inline void lockdep_softirq_end(bool in_hardirq)
{
}
#endif

/*
它也可以在其他上下文中被调用
a) 从硬中断退出时（在 irq_exit 函数中）。irq_exit
b) 在启用之前被禁用的软中断时（如 local_bh_enable 函数中）。
*/

asmlinkage __visible void __softirq_entry __do_softirq(void)
{
	// #define MAX_SOFTIRQ_TIME msecs_to_jiffies(2)
	unsigned long end = jiffies + MAX_SOFTIRQ_TIME;
	unsigned long old_flags = current->flags;
	int max_restart = MAX_SOFTIRQ_RESTART;
	struct softirq_action *h;
	bool in_hardirq;
	__u32 pending;
	int softirq_bit;

	/*
	 * Mask out PF_MEMALLOC s current task context is borrowed for the
	 * softirq. A softirq handled such as network RX might set PF_MEMALLOC
	 * again if the socket is related to swap
	 */
	current->flags &= ~PF_MEMALLOC;
	// 这个时候硬件中断是关闭的
	pending = local_softirq_pending();
	account_irq_enter_time(current);
	// 这个调用会将 SOFTIRQ_OFFSET 加到 preempt_count 上，设置第 8 位，
	// in_serving_softirq() 会判断是否在软中断处理程序中
	// __preempt_count + 1 << 8
	// __local_bh_disable_ip禁用本地软中断
	__local_bh_disable_ip(_RET_IP_, SOFTIRQ_OFFSET);
	in_hardirq = lockdep_softirq_start();

restart:
	/* Reset the pending bitmask before enabling irqs */
	set_softirq_pending(0);
	// 开启了中断，允许硬中断打断软中断处理
	/*
	** 硬中断优先级高于软中断。
	** 软中断本质上是“延迟处理”，可以被硬中断抢占。
	** 只有在软中断框架的关键控制流程（如 pending 标志位清零、状态切换等）才需要临时关闭硬中断，保证一致性。
	*/
	local_irq_enable();

	h = softirq_vec;

	while ((softirq_bit = ffs(pending))) {
		// 循环未决软中断 bit 位
		unsigned int vec_nr;
		int prev_count;

		h += softirq_bit - 1;

		vec_nr = h - softirq_vec;
		prev_count = preempt_count();

		kstat_incr_softirqs_this_cpu(vec_nr);

		// 可以跟踪软中断的入口和出口，来判断中断处理的延迟 perf record -g -e irq:softirq_entry
		trace_softirq_entry(vec_nr);
		// 调用例如 NET_RX_ACTION(net_rx_action), NET_TX_ACTION 函数
		/*
		- 这里会调用 tasklet_action 和 tasklet_hi_action等中断处理函数。
		- Linux内核的设计哲学是：既然已经开始处理这一批软中断，就尽量一次性处理完，避免频繁的上下文切换开销。
		  所以没有在这里判断jiffies是否超过end时间。
		*/
		h->action(h);
		trace_softirq_exit(vec_nr);

		/*
		- preempt_count记录了
          硬中断嵌套层数
		  软中断嵌套层数
		  禁用抢占的次数
		  自旋锁持有计数

		- 这是一个抢占计数一致性检查，用于确保软中断处理函数在执行前后的抢占状态保持一致。
		  进入时：可能会增加 preempt_count（禁用抢占、获取锁等）
          退出时：必须恢复到进入前的状态（释放锁、启用抢占等）

		- 防止内核状态混乱，如果preempt_count不匹配
		  可能导致调度器异常
		  可能引起死锁
		  可能造成抢占失效
		*/
		if (unlikely(prev_count != preempt_count())) {
			pr_err("huh, entered softirq %u %s %p with preempt_count %08x, exited with %08x?\n",
			       vec_nr, softirq_to_name[vec_nr], h->action,
			       prev_count, preempt_count());
			// 自动修复
			preempt_count_set(prev_count);
		}
		h++;
		pending >>= softirq_bit;
	}

	if (__this_cpu_read(ksoftirqd) == current)
		rcu_softirq_qs();
	local_irq_disable();

	pending = local_softirq_pending();
	// 限制单次 __do_softirq() 调用的执行时间，防止它长时间占用 CPU。
	if (pending) {
		// 如果还有软中断未处理，且当前时间在预设结束时间之前，且不需要重新调度，重启次数不为 0，则可以继续处理
		if (time_before(jiffies, end) && !need_resched() &&
		    --max_restart)
			goto restart;
		// 如果不满足，唤醒 ksoftirqd 内核线程来处理剩余的软中断
		// 如果 __do_softirq 在非 ksoftirqd 上下文中运行（例如，从硬中断退出），
		// 并且没有完成所有软中断的处理，那么唤醒 ksoftirqd 是有意义的。
		wakeup_softirqd();
	}

	lockdep_softirq_end(in_hardirq);
	account_irq_exit_time(current);
	__local_bh_enable(SOFTIRQ_OFFSET);
	WARN_ON_ONCE(in_interrupt());
	current_restore_flags(old_flags, PF_MEMALLOC);
}

asmlinkage __visible void do_softirq(void)
{
	// 存储待处理的软中断标志
	__u32 pending;
	// 保存当前的中断状态
	unsigned long flags;

	if (in_interrupt())
		// 检查是否在中断上下文中，如果是，则直接返回，不处理软中断。
		// 如果 cpu 正在服务硬件中断
		return;

	// 用于保存当前的中断状态到 flags 变量中，并关闭中断，以防止在处理软中断期间发生其他中断。
	local_irq_save(flags);

	// 函数用于获取当前待处理的软中断标志。
	// 它是待处理的软中断的 32 位位图----如果第 n 位被设置位 1，那么第 n 位对应的软中断等待被处理
	// 每一次一种软 IRQ 类型受到服务时，其位就会从活跃的软 IRQ 的本地副本 pending 中清掉。
	// 没有未决的软 IRQ 需要处理，该函数返回 0
	// __raise_softirq_irqoff 会置中断位图位 1
	pending = local_softirq_pending();

	// 这段代码首先检查是否有待处理的软中断（pending 是否非零）。
	// 如果有，并且当前没有正在运行的软中断守护线程（ksoftirqd_running(pending) 返回假），
	// 则调用 do_softirq_own_stack() 函数来处理软中断。
	if (pending && !ksoftirqd_running(pending))
		// 有未决的软中断，且软中断守护线程没有运行，则调用 do_softirq_own_stack() 处理软中断
		// 这个函数在中断关闭情况下执行，需要执行效率非常快
		do_softirq_own_stack();

	// 恢复之前保存的中断状态：
	local_irq_restore(flags);
}

/*
 * Enter an interrupt context.
 */
void irq_enter(void)
{
	rcu_irq_enter();
	if (is_idle_task(current) && !in_interrupt()) {
		/*
		 * Prevent raise_softirq from needlessly waking up ksoftirqd
		 * here, as softirq will be serviced on return from interrupt.
		 */
		local_bh_disable();
		tick_irq_enter();
		_local_bh_enable();
	}

	__irq_enter();
}

static inline void invoke_softirq(void)
{
	if (ksoftirqd_running(local_softirq_pending()))
		// 是否有软中断内核线程（ksoftirqd）正在处理当前待执行的软中断。
		// 如果有，函数会立即返回，避免重复处理。
		return;

	if (!force_irqthreads) {
#ifdef CONFIG_HAVE_IRQ_EXIT_ON_IRQ_STACK
		/*
		 * We can safely execute softirq on the current stack if
		 * it is the irq stack, because it should be near empty
		 * at this stage.
		 */
		__do_softirq();
#else
		/*
		 * Otherwise, irq_exit() is called on the task stack that can
		 * be potentially deep already. So call softirq in its own stack
		 * to prevent from any overrun.
		 */
		do_softirq_own_stack();
#endif
	} else {
		wakeup_softirqd();
	}
}

static inline void tick_irq_exit(void)
{
#ifdef CONFIG_NO_HZ_COMMON
	int cpu = smp_processor_id();

	/* Make sure that timer wheel updates are propagated */
	if ((idle_cpu(cpu) && !need_resched()) || tick_nohz_full_cpu(cpu)) {
		if (!in_irq())
			tick_nohz_irq_exit();
	}
#endif
}

/*
 * Exit an interrupt context. Process softirqs if needed and possible:
 */
void irq_exit(void)
{
#ifndef __ARCH_IRQ_EXIT_IRQS_DISABLED
	local_irq_disable();
#else
	lockdep_assert_irqs_disabled();
#endif
	account_irq_exit_time(current);
	// * preempt_count_sub(HARDIRQ_OFFSET) 减少硬中断嵌套计数，表示硬中断上下文已退出。
	preempt_count_sub(HARDIRQ_OFFSET);
	//  检查当前 CPU 是否有待处理的软中断（包括 tasklet）。
	if (!in_interrupt() && local_softirq_pending())
		invoke_softirq();

	// 在中断退出的时候处理与时钟tick相关的操作
	// 还是在非硬中断上下文，准确的说是硬中断退出的尾部处理阶段。
	// 但此时 preempt_count 中的 HARDIRQ 位已经被清除了
	tick_irq_exit();
	rcu_irq_exit();
	/* must be last! */
	lockdep_hardirq_exit();
}

/*
 * This function must run with irqs disabled!，在中断关闭情况下执行
 */
inline void raise_softirq_irqoff(unsigned int nr)
{
	// 设置标志位
	__raise_softirq_irqoff(nr);

	/*
	 * If we're in an interrupt or softirq, we're done
	 * (this also catches softirq-disabled code). We will
	 * actually run the softirq once we return from
	 * the irq or softirq.
	 *
	 * Otherwise we wake up ksoftirqd to make sure we
	 * schedule the softirq soon. 如果在中断上下文中，包括上下半部分，则返回非 0；
	 */
	if (!in_interrupt())
		// 如果该函数在 y 硬中断 isr 中调用，in_interrupt 是会返回 true 的。
		wakeup_softirqd();
}

void raise_softirq(unsigned int nr)
{
	unsigned long flags;
	// 将状态存放在 flags 中，并且关闭中断
	local_irq_save(flags);
	raise_softirq_irqoff(nr);
	// 恢复保存的 flags(中断开关，视保存前的状况而定)
	local_irq_restore(flags);
}

void __raise_softirq_irqoff(unsigned int nr)
{
	trace_softirq_raise(nr);
	// 标注被延迟处理的软中断类型
	// 用来置标志位，每个 CPU 都有一个 softirq_pending 位图，每种软中断类型对应这个位图中的一个位
	// 这里，nr 是软中断的编号，(1UL << nr) 创建一个只有对应位被设置的掩码
	// __raise_softirq_irqoff 函数不会累计计数，多次调用这个函数只会将对应的软中断标记为未决，而不会增加计数
	or_softirq_pending(1UL << nr);
}

void open_softirq(int nr, void (*action)(struct softirq_action *))
{
	softirq_vec[nr].action = action;
}

/*
 * Tasklets
 */
struct tasklet_head {
	struct tasklet_struct *head;
	struct tasklet_struct **tail;
};

static DEFINE_PER_CPU(struct tasklet_head, tasklet_vec);
static DEFINE_PER_CPU(struct tasklet_head, tasklet_hi_vec);

static void __tasklet_schedule_common(struct tasklet_struct *t,
				      struct tasklet_head __percpu *headp,
				      unsigned int softirq_nr)
{
	struct tasklet_head *head;
	unsigned long flags;
	// 保存当前中断状态并禁用本地中断。这是为了防止多个 CPU 同时访问 tasklet 队列而导致竞争条件。
	local_irq_save(flags);
	// 获取当前 CPU 对应的 tasklet 队列头部。this_cpu_ptr 是一个宏，用于访问 per-CPU 变量。
	head = this_cpu_ptr(headp);
	t->next = NULL;
	// 将 tasklet 添加到队列的末尾
	*head->tail = t;
	// 更新队列的尾指针，使其指向新添加的 tasklet 的 next 指针
	head->tail = &(t->next);
	// 这是一个Linux内核中用于触发软中断的函数调用，专门设计在中断已被禁用的上下文中使用
	// _irqoff后缀明确表示假设调用这个函数时中断已经被禁用。这是一个重要前提，
	// *意味调用者必须在禁用中断的上下文中调用这个函数，以确保软中断的处理不会被其他中断打断。
	/*
	于函数假设中断已被禁用，它可以安全地修改软中断的状态位而无需额外的同步机制。
	这种设计在高频率的中断处理场景中特别重要，因为它减少了不必要的中断控制开销，
	同时保证了操作的原子性和数据一致性。
	*/
	raise_softirq_irqoff(softirq_nr);
	local_irq_restore(flags);
}

void __tasklet_schedule(struct tasklet_struct *t)
{
	__tasklet_schedule_common(t, &tasklet_vec, TASKLET_SOFTIRQ);
}
EXPORT_SYMBOL(__tasklet_schedule);

void __tasklet_hi_schedule(struct tasklet_struct *t)
{
	__tasklet_schedule_common(t, &tasklet_hi_vec, HI_SOFTIRQ);
}
EXPORT_SYMBOL(__tasklet_hi_schedule);

static void tasklet_action_common(struct softirq_action *a,
				  struct tasklet_head *tl_head,
				  unsigned int softirq_nr)
{
	struct tasklet_struct *list;

	local_irq_disable();
	list = tl_head->head;
	tl_head->head = NULL;
	tl_head->tail = &tl_head->head;
	local_irq_enable();

	while (list) {
		struct tasklet_struct *t = list;

		list = list->next;

		if (tasklet_trylock(t)) {
			if (!atomic_read(&t->count)) {
				if (!test_and_clear_bit(TASKLET_STATE_SCHED,
							&t->state))
					BUG();
				t->func(t->data);
				tasklet_unlock(t);
				continue;
			}
			tasklet_unlock(t);
		}
		/*
		没有成功锁定，禁用中断，将该tasklet添加回队列末尾，并重新触发软中断。
		这确保了该 tasklet 在下次软中断处理时会被再次处理。
		*/
		local_irq_disable();
		t->next = NULL;
		*tl_head->tail = t;
		tl_head->tail = &t->next;
		__raise_softirq_irqoff(softirq_nr);
		local_irq_enable();
	}
}

static __latent_entropy void tasklet_action(struct softirq_action *a)
{
	tasklet_action_common(a, this_cpu_ptr(&tasklet_vec), TASKLET_SOFTIRQ);
}

static __latent_entropy void tasklet_hi_action(struct softirq_action *a)
{
	tasklet_action_common(a, this_cpu_ptr(&tasklet_hi_vec), HI_SOFTIRQ);
}

void tasklet_init(struct tasklet_struct *t, void (*func)(unsigned long),
		  unsigned long data)
{
	t->next = NULL;
	t->state = 0;
	atomic_set(&t->count, 0);
	t->func = func;
	t->data = data;
}
EXPORT_SYMBOL(tasklet_init);

void tasklet_kill(struct tasklet_struct *t)
{
	if (in_interrupt())
		pr_notice("Attempt to kill tasklet from interrupt\n");

	while (test_and_set_bit(TASKLET_STATE_SCHED, &t->state)) {
		do {
			yield();
		} while (test_bit(TASKLET_STATE_SCHED, &t->state));
	}
	tasklet_unlock_wait(t);
	clear_bit(TASKLET_STATE_SCHED, &t->state);
}
EXPORT_SYMBOL(tasklet_kill);

void __init softirq_init(void)
{
	int cpu;

	for_each_possible_cpu (cpu) {
		per_cpu(tasklet_vec, cpu).tail =
			&per_cpu(tasklet_vec, cpu).head;
		per_cpu(tasklet_hi_vec, cpu).tail =
			&per_cpu(tasklet_hi_vec, cpu).head;
	}
	// 注册普通tasklet处理函数
	open_softirq(TASKLET_SOFTIRQ, tasklet_action);
	// 注册高优先级tasklet处理函数
	open_softirq(HI_SOFTIRQ, tasklet_hi_action);
}

static int ksoftirqd_should_run(unsigned int cpu)
{
	return local_softirq_pending();
}
/*
为什么要屏蔽硬中断：
a. 避免嵌套中断：
软中断处理函数可能会比较耗时。
如果在处理软中断时允许硬中断，可能会导致新的硬中断触发新的软中断，造成嵌套。
b. 保证一致性：
禁用硬中断确保软中断处理过程中不会被打断，保持数据的一致性。
c. 防止优先级反转：
软中断通常优先级低于硬中断。禁用硬中断可以防止低优先级的软中断处理被高优先级的硬中断反复打断。
d. 性能考虑：
在某些情况下，禁用硬中断可以减少上下文切换，提高处理效率。
e. 简化同步：
禁用硬中断简化了软中断处理函数中的同步机制。

3. 潜在风险和平衡：
长时间禁用硬中断可能会增加系统延迟。
__do_softirq() 内部通常有机制限制执行时间，防止长时间占用 CPU。
*/
static void run_ksoftirqd(unsigned int cpu)
{
	// 首先禁用本地中断
	local_irq_disable();
	// 检查是否有待处理的软中断
	if (local_softirq_pending()) {
		/*
		 * We can safely run softirq on inline stack, as we are not deep
		 * in the task stack here.
		 */
		// 如果有待处理的软中断，执行
		__do_softirq();
		// 处理完后，重新启用中断
		local_irq_enable();
		// 让出 cpu，给其他任务运行的机会
		cond_resched();
		return;
	}
	local_irq_enable();
}

#ifdef CONFIG_HOTPLUG_CPU
/*
 * tasklet_kill_immediate is called to remove a tasklet which can already be
 * scheduled for execution on @cpu.
 *
 * Unlike tasklet_kill, this function removes the tasklet
 * _immediately_, even if the tasklet is in TASKLET_STATE_SCHED state.
 *
 * When this function is called, @cpu must be in the CPU_DEAD state.
 */
void tasklet_kill_immediate(struct tasklet_struct *t, unsigned int cpu)
{
	struct tasklet_struct **i;

	BUG_ON(cpu_online(cpu));
	BUG_ON(test_bit(TASKLET_STATE_RUN, &t->state));

	if (!test_bit(TASKLET_STATE_SCHED, &t->state))
		return;

	/* CPU is dead, so no lock needed. */
	for (i = &per_cpu(tasklet_vec, cpu).head; *i; i = &(*i)->next) {
		if (*i == t) {
			*i = t->next;
			/* If this was the tail element, move the tail ptr */
			if (*i == NULL)
				per_cpu(tasklet_vec, cpu).tail = i;
			return;
		}
	}
	BUG();
}

static int takeover_tasklets(unsigned int cpu)
{
	/* CPU is dead, so no lock needed. */
	local_irq_disable();

	/* Find end, append list for that CPU. */
	if (&per_cpu(tasklet_vec, cpu).head != per_cpu(tasklet_vec, cpu).tail) {
		*__this_cpu_read(tasklet_vec.tail) =
			per_cpu(tasklet_vec, cpu).head;
		this_cpu_write(tasklet_vec.tail,
			       per_cpu(tasklet_vec, cpu).tail);
		per_cpu(tasklet_vec, cpu).head = NULL;
		per_cpu(tasklet_vec, cpu).tail =
			&per_cpu(tasklet_vec, cpu).head;
	}
	raise_softirq_irqoff(TASKLET_SOFTIRQ);

	if (&per_cpu(tasklet_hi_vec, cpu).head !=
	    per_cpu(tasklet_hi_vec, cpu).tail) {
		*__this_cpu_read(tasklet_hi_vec.tail) =
			per_cpu(tasklet_hi_vec, cpu).head;
		__this_cpu_write(tasklet_hi_vec.tail,
				 per_cpu(tasklet_hi_vec, cpu).tail);
		per_cpu(tasklet_hi_vec, cpu).head = NULL;
		per_cpu(tasklet_hi_vec, cpu).tail =
			&per_cpu(tasklet_hi_vec, cpu).head;
	}
	raise_softirq_irqoff(HI_SOFTIRQ);

	local_irq_enable();
	return 0;
}
#else
#define takeover_tasklets NULL
#endif /* CONFIG_HOTPLUG_CPU */

static struct smp_hotplug_thread softirq_threads = {
	.store = &ksoftirqd,
	.thread_should_run = ksoftirqd_should_run,
	.thread_fn = run_ksoftirqd,
	.thread_comm = "ksoftirqd/%u",
};

static __init int spawn_ksoftirqd(void)
{
	cpuhp_setup_state_nocalls(CPUHP_SOFTIRQ_DEAD, "softirq:dead", NULL,
				  takeover_tasklets);
	BUG_ON(smpboot_register_percpu_thread(&softirq_threads));

	return 0;
}
early_initcall(spawn_ksoftirqd);

/*
 * [ These __weak aliases are kept in a separate compilation unit, so that
 *   GCC does not inline them incorrectly. ]
 */

int __init __weak early_irq_init(void)
{
	return 0;
}

int __init __weak arch_probe_nr_irqs(void)
{
	return NR_IRQS_LEGACY;
}

int __init __weak arch_early_irq_init(void)
{
	return 0;
}

unsigned int __weak arch_dynirq_lower_bound(unsigned int from)
{
	return from;
}
