/*
 * Generic waiting primitives.
 *
 * (C) 2004 Nadia Yvette Chambers, Oracle
 */
#include "sched.h"

void __init_waitqueue_head(struct wait_queue_head *wq_head, const char *name,
			   struct lock_class_key *key)
{
	spin_lock_init(&wq_head->lock);
	lockdep_set_class_and_name(&wq_head->lock, key, name);
	INIT_LIST_HEAD(&wq_head->head);
}

EXPORT_SYMBOL(__init_waitqueue_head);

void add_wait_queue(struct wait_queue_head *wq_head,
		    struct wait_queue_entry *wq_entry)
{
	unsigned long flags;

	wq_entry->flags &= ~WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave(&wq_head->lock, flags);
	__add_wait_queue(wq_head, wq_entry);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}
EXPORT_SYMBOL(add_wait_queue);

void add_wait_queue_exclusive(struct wait_queue_head *wq_head,
			      struct wait_queue_entry *wq_entry)
{
	unsigned long flags;

	wq_entry->flags |= WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave(&wq_head->lock, flags);
	__add_wait_queue_entry_tail(wq_head, wq_entry);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}
EXPORT_SYMBOL(add_wait_queue_exclusive);

void add_wait_queue_priority(struct wait_queue_head *wq_head,
			     struct wait_queue_entry *wq_entry)
{
	unsigned long flags;

	wq_entry->flags |= WQ_FLAG_EXCLUSIVE | WQ_FLAG_PRIORITY;
	spin_lock_irqsave(&wq_head->lock, flags);
	__add_wait_queue(wq_head, wq_entry);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}
EXPORT_SYMBOL_GPL(add_wait_queue_priority);

void remove_wait_queue(struct wait_queue_head *wq_head,
		       struct wait_queue_entry *wq_entry)
{
	unsigned long flags;

	spin_lock_irqsave(&wq_head->lock, flags);
	__remove_wait_queue(wq_head, wq_entry);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}
EXPORT_SYMBOL(remove_wait_queue);

/*
 * Scan threshold to break wait queue walk.
 * This allows a waker to take a break from holding the
 * wait queue lock during the wait queue walk.
 */
#define WAITQUEUE_WALK_BREAK_CNT 64

/*
 * The core wakeup function. Non-exclusive wakeups (nr_exclusive == 0) just
 * wake everything up. If it's an exclusive wakeup (nr_exclusive == small +ve
 * number) then we wake that number of exclusive tasks, and potentially all
 * the non-exclusive tasks. Normally, exclusive tasks will be at the end of
 * the list and any non-exclusive tasks will be woken first. A priority task
 * may be at the head of the list, and can consume the event without any other
 * tasks being woken.
 *
 * There are circumstances in which we can try to wake a task which has already
 * started to run but is not in state TASK_RUNNING. try_to_wake_up() returns
 * zero in this (rare) case, and we handle it by continuing to scan the queue.
 */
static int __wake_up_common(struct wait_queue_head *wq_head, unsigned int mode,
			    int nr_exclusive, int wake_flags, void *key,
			    wait_queue_entry_t *bookmark)
{
	wait_queue_entry_t *curr, *next;
	int cnt = 0;

	if (bookmark && (bookmark->flags & WQ_FLAG_BOOKMARK)) {
		curr = list_next_entry(bookmark, entry);

		list_del(&bookmark->entry);
		bookmark->flags = 0;
	} else
		// 获得等待队列的第一个条目
		// 如果 list 为空，list_first_entry 实际上会返回 list 表头本身
		// 这是因为在空列表中，头节点的 next 指针指向自己。
		curr = list_first_entry(&wq_head->head, wait_queue_entry_t,
					entry);
	// 如果等待队列为空
	if (&curr->entry == &wq_head->head)
		// 直接返回 nr_exclusive（可能是剩余的独占唤醒次数）
		return nr_exclusive;

	list_for_each_entry_safe_from (curr, next, &wq_head->head, entry) {
		unsigned flags = curr->flags;
		int ret;

		if (flags & WQ_FLAG_BOOKMARK)
			continue;
		// default_wake_function，用来唤醒
		ret = curr->func(curr, mode, wake_flags, key);
		if (ret < 0)
			break;
		if (ret && (flags & WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
			// 对于独占任务，使用 wake_up_all 或者队列中只有独占任务时，才会被唤醒
			// 而且只能唤醒一个独占任务
			break;

		if (bookmark && (++cnt > WAITQUEUE_WALK_BREAK_CNT) &&
		    (&next->entry != &wq_head->head)) {
			bookmark->flags = WQ_FLAG_BOOKMARK;
			list_add_tail(&bookmark->entry, &next->entry);
			break;
		}
	}

	return nr_exclusive;
}

static void __wake_up_common_lock(struct wait_queue_head *wq_head,
				  unsigned int mode, int nr_exclusive,
				  int wake_flags, void *key)
{
	unsigned long flags;
	wait_queue_entry_t bookmark;

	bookmark.flags = 0;
	bookmark.private = NULL;
	bookmark.func = NULL;
	INIT_LIST_HEAD(&bookmark.entry);

	spin_lock_irqsave(&wq_head->lock, flags);
	nr_exclusive = __wake_up_common(wq_head, mode, nr_exclusive, wake_flags,
					key, &bookmark);
	spin_unlock_irqrestore(&wq_head->lock, flags);

	while (bookmark.flags & WQ_FLAG_BOOKMARK) {
		spin_lock_irqsave(&wq_head->lock, flags);
		nr_exclusive = __wake_up_common(wq_head, mode, nr_exclusive,
						wake_flags, key, &bookmark);
		spin_unlock_irqrestore(&wq_head->lock, flags);
	}
}

/**
 * __wake_up - wake up threads blocked on a waitqueue.
 * @wq_head: the waitqueue，等待队列的头部
 *
 * @mode: which threads，指定了唤醒的进程的状态模式。它决定了哪些类型的线程或进程会被唤醒
 * 常用的模式包括：
TASK_INTERRUPTIBLE：仅唤醒那些处于可被信号中断的睡眠状态的任务。
TASK_UNINTERRUPTIBLE：唤醒那些不可被信号中断的睡眠状态的任务。
TASK_KILLABLE：唤醒那些可以被特定信号（如 SIGKILL）唤醒的任务。

 * @nr_exclusive: how many wake-one or wake-many threads to wake up，独占唤醒数量
 如果 nr_exclusive 为 0，则意味着要唤醒队列中的所有线程
 如果 nr_exclusive 大于 0，则会唤醒指定数量的独占等待的线程（唤醒一次或多次）。

 * @key: is directly passed to the wakeup function
 key 是传递给唤醒函数的用户定义数据。这通常是一个指向特定资源或事件的指针，该事件与等待队列相关联。
 *
 * If this function wakes up a task, it executes a full memory barrier before
 * accessing the task state.
 */
void __wake_up(struct wait_queue_head *wq_head, unsigned int mode,
	       int nr_exclusive, void *key)
{
	__wake_up_common_lock(wq_head, mode, nr_exclusive, 0, key);
}
EXPORT_SYMBOL(__wake_up);

/*
 * Same as __wake_up but called with the spinlock in wait_queue_head_t held.
 */
void __wake_up_locked(struct wait_queue_head *wq_head, unsigned int mode,
		      int nr)
{
	__wake_up_common(wq_head, mode, nr, 0, NULL, NULL);
}
EXPORT_SYMBOL_GPL(__wake_up_locked);

void __wake_up_locked_key(struct wait_queue_head *wq_head, unsigned int mode,
			  void *key)
{
	__wake_up_common(wq_head, mode, 1, 0, key, NULL);
}
EXPORT_SYMBOL_GPL(__wake_up_locked_key);

void __wake_up_locked_key_bookmark(struct wait_queue_head *wq_head,
				   unsigned int mode, void *key,
				   wait_queue_entry_t *bookmark)
{
	__wake_up_common(wq_head, mode, 1, 0, key, bookmark);
}
EXPORT_SYMBOL_GPL(__wake_up_locked_key_bookmark);

/**
 * __wake_up_sync_key - wake up threads blocked on a waitqueue.
 * @wq_head: the waitqueue
 * @mode: which threads
 * @key: opaque value to be passed to wakeup targets
 *
 * The sync wakeup differs that the waker knows that it will schedule
 * away soon, so while the target thread will be woken up, it will not
 * be migrated to another CPU - ie. the two threads are 'synchronized'
 * with each other. This can prevent needless bouncing between CPUs.
 *
 * On UP it can prevent extra preemption.
 *
 * If this function wakes up a task, it executes a full memory barrier before
 * accessing the task state.
 */
void __wake_up_sync_key(struct wait_queue_head *wq_head, unsigned int mode,
			void *key)
{
	if (unlikely(!wq_head))
		return;

	__wake_up_common_lock(wq_head, mode, 1, WF_SYNC, key);
}
EXPORT_SYMBOL_GPL(__wake_up_sync_key);

/**
 * __wake_up_locked_sync_key - wake up a thread blocked on a locked waitqueue.
 * @wq_head: the waitqueue
 * @mode: which threads
 * @key: opaque value to be passed to wakeup targets
 *
 * The sync wakeup differs in that the waker knows that it will schedule
 * away soon, so while the target thread will be woken up, it will not
 * be migrated to another CPU - ie. the two threads are 'synchronized'
 * with each other. This can prevent needless bouncing between CPUs.
 *
 * On UP it can prevent extra preemption.
 *
 * If this function wakes up a task, it executes a full memory barrier before
 * accessing the task state.
 */
void __wake_up_locked_sync_key(struct wait_queue_head *wq_head,
			       unsigned int mode, void *key)
{
	__wake_up_common(wq_head, mode, 1, WF_SYNC, key, NULL);
}
EXPORT_SYMBOL_GPL(__wake_up_locked_sync_key);

/*
 * __wake_up_sync - see __wake_up_sync_key()
 */
void __wake_up_sync(struct wait_queue_head *wq_head, unsigned int mode)
{
	__wake_up_sync_key(wq_head, mode, NULL);
}
EXPORT_SYMBOL_GPL(__wake_up_sync); /* For internal use only */

/*
 * Note: we use "set_current_state()" _after_ the wait-queue add,
 * because we need a memory barrier there on SMP, so that any
 * wake-function that tests for the wait-queue being active
 * will be guaranteed to see waitqueue addition _or_ subsequent
 * tests in this thread will see the wakeup having taken place.
 *
 * The spin_unlock() itself is semi-permeable and only protects
 * one way (it only protects stuff inside the critical region and
 * stops them from bleeding out - it would still allow subsequent
 * loads to move into the critical region).
 */
void prepare_to_wait(struct wait_queue_head *wq_head,
		     struct wait_queue_entry *wq_entry, int state)
{
	unsigned long flags;

	wq_entry->flags &= ~WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave(&wq_head->lock, flags);
	if (list_empty(&wq_entry->entry))
		__add_wait_queue(wq_head, wq_entry);
	set_current_state(state);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}
EXPORT_SYMBOL(prepare_to_wait);

/* This API is RH only */
/* Returns true if we are the first waiter in the queue, false otherwise. */
bool prepare_to_wait_exclusive_return(struct wait_queue_head *wq_head,
				      struct wait_queue_entry *wq_entry,
				      int state)
{
	unsigned long flags;
	bool was_empty = false;

	wq_entry->flags |= WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave(&wq_head->lock, flags);
	if (list_empty(&wq_entry->entry)) {
		was_empty = list_empty(&wq_head->head);
		__add_wait_queue_entry_tail(wq_head, wq_entry);
	}
	set_current_state(state);
	spin_unlock_irqrestore(&wq_head->lock, flags);
	return was_empty;
}
EXPORT_SYMBOL(prepare_to_wait_exclusive_return);

void prepare_to_wait_exclusive(struct wait_queue_head *wq_head,
			       struct wait_queue_entry *wq_entry, int state)
{
	prepare_to_wait_exclusive_return(wq_head, wq_entry, state);
}
EXPORT_SYMBOL(prepare_to_wait_exclusive);

void init_wait_entry(struct wait_queue_entry *wq_entry, int flags)
{
	wq_entry->flags = flags;
	wq_entry->private = current;
	wq_entry->func = autoremove_wake_function;
	INIT_LIST_HEAD(&wq_entry->entry);
}
EXPORT_SYMBOL(init_wait_entry);

long prepare_to_wait_event(struct wait_queue_head *wq_head,
			   struct wait_queue_entry *wq_entry, int state)
{
	unsigned long flags;
	long ret = 0;

	spin_lock_irqsave(&wq_head->lock, flags);
	if (signal_pending_state(state, current)) {
		/*
		 * Exclusive waiter must not fail if it was selected by wakeup,
		 * it should "consume" the condition we were waiting for.
		 *
		 * The caller will recheck the condition and return success if
		 * we were already woken up, we can not miss the event because
		 * wakeup locks/unlocks the same wq_head->lock.
		 *
		 * But we need to ensure that set-condition + wakeup after that
		 * can't see us, it should wake up another exclusive waiter if
		 * we fail.
		 */
		list_del_init(&wq_entry->entry);
		ret = -ERESTARTSYS;
	} else {
		if (list_empty(&wq_entry->entry)) {
			if (wq_entry->flags & WQ_FLAG_EXCLUSIVE)
				__add_wait_queue_entry_tail(wq_head, wq_entry);
			else
				__add_wait_queue(wq_head, wq_entry);
		}
		set_current_state(state);
	}
	spin_unlock_irqrestore(&wq_head->lock, flags);

	return ret;
}
EXPORT_SYMBOL(prepare_to_wait_event);

/*
 * Note! These two wait functions are entered with the
 * wait-queue lock held (and interrupts off in the _irq
 * case), so there is no race with testing the wakeup
 * condition in the caller before they add the wait
 * entry to the wake queue.
 */
int do_wait_intr(wait_queue_head_t *wq, wait_queue_entry_t *wait)
{
	if (likely(list_empty(&wait->entry)))
		__add_wait_queue_entry_tail(wq, wait);

	set_current_state(TASK_INTERRUPTIBLE);
	if (signal_pending(current))
		return -ERESTARTSYS;

	spin_unlock(&wq->lock);
	schedule();
	spin_lock(&wq->lock);

	return 0;
}
EXPORT_SYMBOL(do_wait_intr);

int do_wait_intr_irq(wait_queue_head_t *wq, wait_queue_entry_t *wait)
{
	if (likely(list_empty(&wait->entry)))
		__add_wait_queue_entry_tail(wq, wait);

	set_current_state(TASK_INTERRUPTIBLE);
	if (signal_pending(current))
		return -ERESTARTSYS;

	spin_unlock_irq(&wq->lock);
	schedule();
	spin_lock_irq(&wq->lock);

	return 0;
}
EXPORT_SYMBOL(do_wait_intr_irq);

/**
 * finish_wait - clean up after waiting in a queue
 * @wq_head: waitqueue waited on
 * @wq_entry: wait descriptor
 *
 * Sets current thread back to running state and removes
 * the wait descriptor from the given waitqueue if still
 * queued.
 */
void finish_wait(struct wait_queue_head *wq_head,
		 struct wait_queue_entry *wq_entry)
{
	unsigned long flags;

	__set_current_state(TASK_RUNNING);
	/*
	 * We can check for list emptiness outside the lock
	 * IFF:
	 *  - we use the "careful" check that verifies both
	 *    the next and prev pointers, so that there cannot
	 *    be any half-pending updates in progress on other
	 *    CPU's that we haven't seen yet (and that might
	 *    still change the stack area.
	 * and
	 *  - all other users take the lock (ie we can only
	 *    have _one_ other CPU that looks at or modifies
	 *    the list).
	 */
	if (!list_empty_careful(&wq_entry->entry)) {
		spin_lock_irqsave(&wq_head->lock, flags);
		list_del_init(&wq_entry->entry);
		spin_unlock_irqrestore(&wq_head->lock, flags);
	}
}
EXPORT_SYMBOL(finish_wait);

int autoremove_wake_function(struct wait_queue_entry *wq_entry, unsigned mode,
			     int sync, void *key)
{
	int ret = default_wake_function(wq_entry, mode, sync, key);

	if (ret)
		list_del_init(&wq_entry->entry);

	return ret;
}
EXPORT_SYMBOL(autoremove_wake_function);

static inline bool is_kthread_should_stop(void)
{
	return (current->flags & PF_KTHREAD) && kthread_should_stop();
}

/*
 * DEFINE_WAIT_FUNC(wait, woken_wake_func);
 *
 * add_wait_queue(&wq_head, &wait);
 * for (;;) {
 *     if (condition)
 *         break;
 *
 *     // in wait_woken()			// in woken_wake_function()
 *
 *     p->state = mode;				wq_entry->flags |= WQ_FLAG_WOKEN;
 *     smp_mb(); // A				try_to_wake_up():
 *     if (!(wq_entry->flags & WQ_FLAG_WOKEN))	   <full barrier>
 *         schedule()				   if (p->state & mode)
 *     p->state = TASK_RUNNING;			      p->state = TASK_RUNNING;
 *     wq_entry->flags &= ~WQ_FLAG_WOKEN;	~~~~~~~~~~~~~~~~~~
 *     smp_mb(); // B				condition = true;
 * }						smp_mb(); // C
 * remove_wait_queue(&wq_head, &wait);		wq_entry->flags |= WQ_FLAG_WOKEN;
 */
long wait_woken(struct wait_queue_entry *wq_entry, unsigned mode, long timeout)
{
	/*
	 * The below executes an smp_mb(), which matches with the full barrier
	 * executed by the try_to_wake_up() in woken_wake_function() such that
	 * either we see the store to wq_entry->flags in woken_wake_function()
	 * or woken_wake_function() sees our store to current->state.
	 */
	set_current_state(mode); /* A */
	if (!(wq_entry->flags & WQ_FLAG_WOKEN) && !is_kthread_should_stop())
		timeout = schedule_timeout(timeout);
	__set_current_state(TASK_RUNNING);

	/*
	 * The below executes an smp_mb(), which matches with the smp_mb() (C)
	 * in woken_wake_function() such that either we see the wait condition
	 * being true or the store to wq_entry->flags in woken_wake_function()
	 * follows ours in the coherence order.
	 */
	smp_store_mb(wq_entry->flags, wq_entry->flags & ~WQ_FLAG_WOKEN); /* B */

	return timeout;
}
EXPORT_SYMBOL(wait_woken);

int woken_wake_function(struct wait_queue_entry *wq_entry, unsigned mode,
			int sync, void *key)
{
	/* Pairs with the smp_store_mb() in wait_woken(). */
	smp_mb(); /* C */
	wq_entry->flags |= WQ_FLAG_WOKEN;

	return default_wake_function(wq_entry, mode, sync, key);
}
EXPORT_SYMBOL(woken_wake_function);
