/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_OSQ_LOCK_H
#define __LINUX_OSQ_LOCK_H

/*
 * An MCS like lock especially tailored for optimistic spinning for sleeping
 * lock implementations (mutex, rwsem, etc).
 */
struct optimistic_spin_node {
	struct optimistic_spin_node *next,
		*prev; // next 和 prev 指针可以组成一个双向链表
	int locked; /* 1 if lock acquired 表示加锁状态。 */
	int cpu; /* encoded CPU # + 1 value 用于重新编码CPU编号，表示该node在那个CPU上。 */
};

struct optimistic_spin_queue {
	/*
	 * Stores an encoded value of the CPU # of the tail node in the queue.
	 * If the queue is empty, then it's set to OSQ_UNLOCKED_VAL.
	 */
	atomic_t tail;
};

#define OSQ_UNLOCKED_VAL (0)

/* Init macro and function. */
#define OSQ_LOCK_UNLOCKED                                                      \
	{                                                                      \
		ATOMIC_INIT(OSQ_UNLOCKED_VAL)                                  \
	}

static inline void osq_lock_init(struct optimistic_spin_queue *lock)
{
	// 把队列 tail 设置为 OSQ_UNLOCKED_VAL，即 0。
	atomic_set(&lock->tail, OSQ_UNLOCKED_VAL);
}

extern bool osq_lock(struct optimistic_spin_queue *lock);
extern void osq_unlock(struct optimistic_spin_queue *lock);

static inline bool osq_is_locked(struct optimistic_spin_queue *lock)
{
	return atomic_read(&lock->tail) != OSQ_UNLOCKED_VAL;
}

#endif
