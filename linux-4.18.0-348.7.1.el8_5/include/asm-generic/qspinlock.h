/*
 * Queued spinlock
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * (C) Copyright 2013-2015 Hewlett-Packard Development Company, L.P.
 * (C) Copyright 2015 Hewlett-Packard Enterprise Development LP
 *
 * Authors: Waiman Long <waiman.long@hpe.com>
 */
#ifndef __ASM_GENERIC_QSPINLOCK_H
#define __ASM_GENERIC_QSPINLOCK_H

#include <asm-generic/qspinlock_types.h>
#include <linux/atomic.h>

/**
 * queued_spin_is_locked - is the spinlock locked?
 * @lock: Pointer to queued spinlock structure
 * Return: 1 if it is locked, 0 otherwise
 */
static __always_inline int queued_spin_is_locked(struct qspinlock *lock)
{
	/*
	 * Any !0 state indicates it is locked, even if _Q_LOCKED_VAL
	 * isn't immediately observable.
	 */
	return atomic_read(&lock->val);
}

/**
 * queued_spin_value_unlocked - is the spinlock structure unlocked?
 * @lock: queued spinlock structure
 * Return: 1 if it is unlocked, 0 otherwise
 *
 * N.B. Whenever there are tasks waiting for the lock, it is considered
 *      locked wrt the lockref code to avoid lock stealing by the lockref
 *      code and change things underneath the lock. This also allows some
 *      optimizations to be applied without conflict with lockref.
 */
static __always_inline int queued_spin_value_unlocked(struct qspinlock lock)
{
	return !atomic_read(&lock.val);
}

/**
 * queued_spin_is_contended - check if the lock is contended
 * @lock : Pointer to queued spinlock structure
 * Return: 1 if lock contended, 0 otherwise
 */
static __always_inline int queued_spin_is_contended(struct qspinlock *lock)
{
	return atomic_read(&lock->val) & ~_Q_LOCKED_MASK;
}
/**
 * queued_spin_trylock - try to acquire the queued spinlock
 * @lock : Pointer to queued spinlock structure
 * Return: 1 if lock acquired, 0 if failed
 */
static __always_inline int queued_spin_trylock(struct qspinlock *lock)
{
	if (!atomic_read(&lock->val) &&
	    (atomic_cmpxchg_acquire(&lock->val, 0, _Q_LOCKED_VAL) == 0))
		return 1;
	return 0;
}

extern void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val);

/**
 * queued_spin_lock - acquire a queued spinlock
 * @lock: Pointer to queued spinlock structure
 */
static __always_inline void queued_spin_lock(struct qspinlock *lock)
{
	u32 val;
	//和当前 lock->val 的值进行比较，如果是 0，则将其设置为_Q_LOCKED_VAL，表示获取锁成功
	val = atomic_cmpxchg_acquire(&lock->val, 0, _Q_LOCKED_VAL);
	// 判断原始值是否为 0，如果是 0，则表示获取锁成功
	if (likely(val == 0))
		return;
	// 否则，调用 queued_spin_lock_slowpath 函数进行获取锁
	queued_spin_lock_slowpath(lock, val);
}

#ifndef queued_spin_unlock
/**
 * queued_spin_unlock - release a queued spinlock
 * @lock : Pointer to queued spinlock structure
 */
static __always_inline void queued_spin_unlock(struct qspinlock *lock)
{
	/*
	 * unlock() needs release semantics:
	 */
	smp_store_release(&lock->locked, 0);
}
#endif

#ifndef virt_spin_lock
static __always_inline bool virt_spin_lock(struct qspinlock *lock)
{
	return false;
}
#endif

/*
 * Remapping spinlock architecture specific functions to the corresponding
 * queued spinlock functions.
 */
#define arch_spin_is_locked(l) queued_spin_is_locked(l)
#define arch_spin_is_contended(l) queued_spin_is_contended(l)
#define arch_spin_value_unlocked(l) queued_spin_value_unlocked(l)
#define arch_spin_lock(l) queued_spin_lock(l)
#define arch_spin_trylock(l) queued_spin_trylock(l)
#define arch_spin_unlock(l) queued_spin_unlock(l)

#endif /* __ASM_GENERIC_QSPINLOCK_H */
