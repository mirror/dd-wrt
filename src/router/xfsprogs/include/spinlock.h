// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019-20 RedHat, Inc.
 * All Rights Reserved.
 */
#ifndef __LIBXFS_SPINLOCK_H__
#define __LIBXFS_SPINLOCK_H__

/*
 * This implements kernel compatible spinlock exclusion semantics. These,
 * however, are not spinlocks, as spinlocks cannot be reliably implemented in
 * userspace without using realtime scheduling task contexts. Hence this
 * interface is implemented with pthread mutexes and so can block, but this is
 * no different to the kernel RT build which replaces spinlocks with mutexes.
 * Hence we know it works.
 */

typedef pthread_mutex_t	spinlock_t;

#define spin_lock_init(l)	pthread_mutex_init(l, NULL)
#define spin_lock(l)		pthread_mutex_lock(l)
#define spin_trylock(l)		(pthread_mutex_trylock(l) != EBUSY)
#define spin_unlock(l)		pthread_mutex_unlock(l)

#endif /* __LIBXFS_SPINLOCK_H__ */
