/*
 * Detect Hung Task
 *
 * kernel/hung_task.c - kernel thread for detecting tasks stuck in D state
 *
 */

#include <linux/mm.h>
#include <linux/cpu.h>
#include <linux/nmi.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/lockdep.h>
#include <linux/export.h>
#include <linux/sysctl.h>
#include <linux/suspend.h>
#include <linux/utsname.h>
#include <linux/sched/signal.h>
#include <linux/sched/debug.h>

#include <trace/events/sched.h>

/*
 * The number of tasks checked:
 */
int __read_mostly sysctl_hung_task_check_count = PID_MAX_LIMIT;

/*
 * Limit number of tasks checked in a batch.
 *
 * This value controls the preemptibility of khungtaskd since preemption
 * is disabled during the critical section. It also controls the size of
 * the RCU grace period. So it needs to be upper-bound.
 */
#define HUNG_TASK_BATCHING 1024

/*
 * Zero means infinite timeout - no checking done:
 */
unsigned long __read_mostly sysctl_hung_task_timeout_secs =
	CONFIG_DEFAULT_HUNG_TASK_TIMEOUT;

int __read_mostly sysctl_hung_task_warnings = 10;

static int __read_mostly did_panic;
static bool hung_task_show_lock;
static bool hung_task_call_panic;

static struct task_struct *watchdog_task;

/*
 * Should we panic (and reboot, if panic_timeout= is set) when a
 * hung task is detected:
 */
unsigned int __read_mostly sysctl_hung_task_panic =
	CONFIG_BOOTPARAM_HUNG_TASK_PANIC_VALUE;

static int __init hung_task_panic_setup(char *str)
{
	int rc = kstrtouint(str, 0, &sysctl_hung_task_panic);

	if (rc)
		return rc;
	return 1;
}
__setup("hung_task_panic=", hung_task_panic_setup);

static int hung_task_panic(struct notifier_block *this, unsigned long event,
			   void *ptr)
{
	did_panic = 1;

	return NOTIFY_DONE;
}

static struct notifier_block panic_block = {
	.notifier_call = hung_task_panic,
};

static void check_hung_task(struct task_struct *t, unsigned long timeout)
{
	// 表示线程总的切换次数，包括主动和被动的
	unsigned long switch_count = t->nvcsw + t->nivcsw;

	/*
	 * Ensure the task is not frozen.
	 * Also, skip vfork and any other user process that freezer should skip.
	 */
	if (unlikely(t->flags & (PF_FROZEN | PF_FREEZER_SKIP)))
		return;

	/*
	 * When a freshly created task is scheduled once, changes its state to
	 * TASK_UNINTERRUPTIBLE without having ever been switched out once, it
	 * musn't be checked.
	 */
	if (unlikely(!switch_count))
		return;

	if (switch_count != t->last_switch_count) {
		// 如果总切换次数和 last_switch_count 不等，表示在上次 khungtaskd 更新 last_switch_count 之后就发生了进程切换；
		// 反之，相等则表示 120s 时间内没有发生切换。
		t->last_switch_count = switch_count;
		return;
	}

	trace_sched_process_hang(t);

	if (!sysctl_hung_task_warnings && !sysctl_hung_task_panic)
		return;

	/*
	 * Ok, the task did not get scheduled for more than 2 minutes,
	 * complain:
	 */
	if (sysctl_hung_task_warnings) {
		// hung task 错误打印次数限制，默认为 10 次，整个系统运行期间最多打印 10 次
		if (sysctl_hung_task_warnings > 0)
			sysctl_hung_task_warnings--;
		pr_err("INFO: task %s:%d blocked for more than %ld seconds.\n",
		       t->comm, t->pid, timeout);
		pr_err("      %s %s %.*s\n", print_tainted(),
		       init_utsname()->release,
		       (int)strcspn(init_utsname()->version, " "),
		       init_utsname()->version);
		pr_err("\"echo 0 > /proc/sys/kernel/hung_task_timeout_secs\""
		       " disables this message.\n");
		// 显示进程 ID、名称、状态以及栈等信息。
		sched_show_task(t);
		// 如果使能 debug_locks，则打印进程持有的锁。
		hung_task_show_lock = true;
	}

	touch_nmi_watchdog();

	if (sysctl_hung_task_panic) {
		hung_task_show_lock = true;
		hung_task_call_panic = true;
	}
}

/*
 * To avoid extending the RCU grace period for an unbounded amount of time,
 * periodically exit the critical section and enter a new one.
 *
 * For preemptible RCU it is sufficient to call rcu_read_unlock in order
 * to exit the grace period. For classic RCU, a reschedule is required.
 */
static bool rcu_lock_break(struct task_struct *g, struct task_struct *t)
{
	bool can_cont;

	get_task_struct(g);
	get_task_struct(t);
	rcu_read_unlock();
	cond_resched();
	rcu_read_lock();
	can_cont = pid_alive(g) && pid_alive(t);
	put_task_struct(t);
	put_task_struct(g);

	return can_cont;
}

/*
 * Check whether a TASK_UNINTERRUPTIBLE does not get woken up for
 * a really long time (120 seconds). If that happens, print out
 * a warning.
 */
static void check_hung_uninterruptible_tasks(unsigned long timeout)
{
	int max_count =
		sysctl_hung_task_check_count; // 检测最大进程数，默认为最大进程号
	int batch_count = HUNG_TASK_BATCHING;
	struct task_struct *g, *t;

	/*
	 * If the system crashed already then all bets are off,
	 * do not report extra hung tasks:
	 */
	if (test_taint(TAINT_DIE) || did_panic)
		return;

	hung_task_show_lock = false;
	rcu_read_lock();
	for_each_process_thread (g, t) {
		if (!max_count--)
			goto unlock;
		if (!--batch_count) {
			batch_count = HUNG_TASK_BATCHING;
			// 防止 rcu_read_lock 占用过长时间。释放 rcu，并主动调度。调度回来后检查响应进程是否还在，不在则退出遍历，否则继续。
			if (!rcu_lock_break(g, t))
				goto unlock;
		}
		/* use "==" to skip the TASK_KILLABLE tasks waiting on NFS */
		if (t->state == TASK_UNINTERRUPTIBLE)
			check_hung_task(t, timeout);
	}
unlock:
	rcu_read_unlock();
	if (hung_task_show_lock)
		debug_show_all_locks();
	if (hung_task_call_panic) {
		trigger_all_cpu_backtrace();
		panic("hung_task: blocked tasks");
	}
}

static long hung_timeout_jiffies(unsigned long last_checked,
				 unsigned long timeout)
{
	/* timeout of 0 will disable the watchdog */
	// last_checked 是上次 watchdog 检查的时间，用 jiffies 计数表示
	// timeout 是 watchdog 的超时阈值，单位是秒
	// 如果传入的 timeout 为 0，说明要禁用 watchdog 机制。函数会返回 MAX_SCHEDULE_TIMEOUT，这是一个很大的数值，理论上会导致永远不会超时。
	// last_checked - jiffies 计算自上次检查后已经过去的 jiffies 数 (时间单位)。
	// timeout * HZ 将以秒为单位的超时阈值转换为 jiffies 数 (HZ 是一个宏，表示每秒对应的 jiffies 数)。
	return timeout ? last_checked - jiffies + timeout * HZ :
			 MAX_SCHEDULE_TIMEOUT;
}

/*
 * Process updating of timeout sysctl
 */
int proc_dohung_task_timeout_secs(struct ctl_table *table, int write,
				  void __user *buffer, size_t *lenp,
				  loff_t *ppos)
{
	int ret;

	ret = proc_doulongvec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		goto out;

	wake_up_process(watchdog_task);

out:
	return ret;
}

static atomic_t reset_hung_task = ATOMIC_INIT(0);

void reset_hung_task_detector(void)
{
	atomic_set(&reset_hung_task, 1);
}
EXPORT_SYMBOL_GPL(reset_hung_task_detector);

static bool hung_detector_suspended;

static int hungtask_pm_notify(struct notifier_block *self, unsigned long action,
			      void *hcpu)
{
	switch (action) {
	case PM_SUSPEND_PREPARE:
	case PM_HIBERNATION_PREPARE:
	case PM_RESTORE_PREPARE:
		hung_detector_suspended = true;
		break;
	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		hung_detector_suspended = false;
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

/*
 * kthread which checks for tasks stuck in D state
 */
static int watchdog(void *dummy)
{
	unsigned long hung_last_checked = jiffies;

	set_user_nice(current, 0);

	for (;;) {
		unsigned long timeout = sysctl_hung_task_timeout_secs;
		// 计算下次 watchdog 检查的超时时间
		long t = hung_timeout_jiffies(hung_last_checked, timeout);

		if (t <= 0) {
			// 如果小于 0，说明再度调度的时间 (jeffies > hung_last_checked + timeout)，已经过了一个检测周期
			// atomic_xchg(&reset_hung_task, 0) 会将其值设置为 0，并返回原来的值
			// 如果 hung_detector_suspended 为 0，那么说明 hung task 检测器没有被挂起，可以进行检查
			if (!atomic_xchg(&reset_hung_task, 0) &&
			    !hung_detector_suspended)
				// 检查是否有任务长时间无法被中断。这个函数会遍历所有的任务，如果有任务的状态为不可中断，
				// 并且已经运行了超过 timeout 秒，那么就会打印警告信息
				check_hung_uninterruptible_tasks(timeout);
			hung_last_checked = jiffies;
			continue;
		}
		// 如果 t 大于 0，那么说明还没有到下一次检查的时间，会调用 schedule_timeout_interruptible(t) 函数，将当前任务挂起，等待 t 个 jiffies 后再次被调度
		schedule_timeout_interruptible(t);
		// 挂起当前任务一段时间后，会返回到调用它的地方，然后从那里继续执行
	}

	return 0;
}

static int __init hung_task_init(void)
{
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);

	/* Disable hung task detector on suspend */
	pm_notifier(hungtask_pm_notify, 0);

	watchdog_task = kthread_run(watchdog, NULL, "khungtaskd");

	return 0;
}
subsys_initcall(hung_task_init);
