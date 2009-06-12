#ifndef _FRIO_SEMAPHORE_H
#define _FRIO_SEMAPHORE_H

#define RW_LOCK_BIAS		 0x01000000

#ifndef __ASSEMBLY__

#include <linux/linkage.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>

#include <asm/system.h>
#include <asm/atomic.h>

/*
 * Interrupt-safe semaphores..
 *
 * (C) Copyright 1996 Linus Torvalds
 *
 * FRIO version by akbar hussain Lineo Inc  April 2001
 *
 */

struct semaphore {
	atomic_t count;
	atomic_t waking;
	wait_queue_head_t wait;
#if WAITQUEUE_DEBUG
	long __magic;
#endif
};

#if WAITQUEUE_DEBUG
#define __SEM_DEBUG_INIT(name) \
		, (long)&(name).__magic
#else
#define __SEM_DEBUG_INIT(name)
#endif

#define __SEMAPHORE_INITIALIZER(name,count) \
{ ATOMIC_INIT(count), ATOMIC_INIT(0), __WAIT_QUEUE_HEAD_INITIALIZER((name).wait) \
	__SEM_DEBUG_INIT(name) }

#define __MUTEX_INITIALIZER(name) \
	__SEMAPHORE_INITIALIZER(name,1)

#define __DECLARE_SEMAPHORE_GENERIC(name,count) \
	struct semaphore name = __SEMAPHORE_INITIALIZER(name,count)

#define DECLARE_MUTEX(name) __DECLARE_SEMAPHORE_GENERIC(name,1)
#define DECLARE_MUTEX_LOCKED(name) __DECLARE_SEMAPHORE_GENERIC(name,0)

static inline void sema_init (struct semaphore *sem, int val)
{
	*sem = (struct semaphore)__SEMAPHORE_INITIALIZER(*sem, val);
}

static inline void init_MUTEX (struct semaphore *sem)
{
	sema_init(sem, 1);
}

static inline void init_MUTEX_LOCKED (struct semaphore *sem)
{
	sema_init(sem, 0);
}

#if 0
asmlinkage void __down_failed(void /* special register calling convention */);
asmlinkage int  __down_failed_interruptible(void  /* params in registers */);
asmlinkage int  __down_failed_trylock(void  /* params in registers */);
asmlinkage void __up_wakeup(void /* special register calling convention */);
#endif

asmlinkage void __down(struct semaphore * sem);
asmlinkage int  __down_interruptible(struct semaphore * sem);
asmlinkage int  __down_trylock(struct semaphore * sem);
asmlinkage void __up(struct semaphore * sem);

extern spinlock_t semaphore_wake_lock;

/*
 * This is ugly, but we want the default case to fall through.
 * "down_failed" is a special asm handler that calls the C
 * routine that actually waits. See arch/m68k/lib/semaphore.S
 */
static inline void down(struct semaphore * sem)
{
#if WAITQUEUE_DEBUG
	CHECK_MAGIC(sem->__magic);
#endif
 
        if (atomic_dec_return(&sem->count) < 0)
                __down(sem);  
}

static inline int down_interruptible(struct semaphore * sem)
{
	int ret = 0;

#if WAITQUEUE_DEBUG
	CHECK_MAGIC(sem->__magic);
#endif
 
        if (atomic_dec_return(&sem->count) < 0)
                ret = __down_interruptible(sem); 
	return(ret);
}

static inline int down_trylock(struct semaphore * sem)
{
	int ret = 0;
#if WAITQUEUE_DEBUG
	CHECK_MAGIC(sem->__magic);
#endif
 
        if (atomic_dec_return(&sem->count) < 0)
                ret = __down_trylock(sem);  
	return ret;
}

/*
 * Note! This is subtle. We jump to wake people up only if
 * the semaphore was negative (== somebody was waiting on it).
 * The default case (no contention) will result in NO
 * jumps for both down() and up().
 */
static inline void up(struct semaphore * sem)
{
#if WAITQUEUE_DEBUG
	CHECK_MAGIC(sem->__magic);
#endif
        if (atomic_inc_return(&sem->count) <= 0)
                __up(sem);  
}


#endif /* __ASSEMBLY__ */

#endif /* _FRIO_SEMAPHORE_H */
