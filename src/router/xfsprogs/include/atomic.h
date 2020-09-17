// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2011 RedHat, Inc.
 * All Rights Reserved.
 */
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

/*
 * Warning: These are not really atomic at all. They are wrappers around the
 * kernel atomic variable interface. If we do need these variables to be atomic
 * (due to multithreading of the code that uses them) we need to add some
 * pthreads magic here.
 */
typedef	int32_t	atomic_t;
typedef	int64_t	atomic64_t;

#define atomic_inc_return(x)	(++(*(x)))
#define atomic_dec_return(x)	(--(*(x)))

#define atomic64_read(x)	*(x)
#define atomic64_set(x, v)	(*(x) = v)

#endif /* __ATOMIC_H__ */
