// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2011 RedHat, Inc.
 * All Rights Reserved.
 */
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

/*
 * Atomics are provided by liburcu.
 *
 * API and guidelines for which operations provide memory barriers is here:
 *
 * https://github.com/urcu/userspace-rcu/blob/master/doc/uatomic-api.md
 *
 * Unlike the kernel, the same interface supports 32 and 64 bit atomic integers.
 */
#include <urcu/uatomic.h>
#include "spinlock.h"

typedef	int32_t	atomic_t;
typedef	int64_t	atomic64_t;

#define atomic_read(a)		uatomic_read(a)
#define atomic_set(a, v)	uatomic_set(a, v)
#define atomic_add(v, a)	uatomic_add(a, v)
#define atomic_sub(v, a)	uatomic_sub(a, v)
#define atomic_inc(a)		uatomic_inc(a)
#define atomic_dec(a)		uatomic_dec(a)
#define atomic_inc_return(a)	uatomic_add_return(a, 1)
#define atomic_dec_return(a)	uatomic_sub_return(a, 1)
#define atomic_dec_and_test(a)	(atomic_dec_return(a) == 0)
#define cmpxchg(a, o, n)	uatomic_cmpxchg(a, o, n);

static inline bool atomic_add_unless(atomic_t *a, int v, int u)
{
	int r = atomic_read(a);
	int n, o;

	do {
		o = r;
		if (o == u)
			break;
		n = o + v;
		r = uatomic_cmpxchg(a, o, n);
	} while (r != o);

	return o != u;
}

static inline bool atomic_inc_not_zero(atomic_t *a)
{
	return atomic_add_unless(a, 1, 0);
}

static inline bool atomic_dec_and_lock(atomic_t *a, spinlock_t *lock)
{
	if (atomic_add_unless(a, -1, 1))
		return 0;

	spin_lock(lock);
	if (atomic_dec_and_test(a))
		return 1;
	spin_unlock(lock);
	return 0;
}

#ifdef HAVE_LIBURCU_ATOMIC64
/*
 * On most (64-bit) platforms, liburcu can handle 64-bit atomic counter
 * updates, so we preferentially use that.
 */
#define atomic64_read(a)	uatomic_read(a)
#define atomic64_set(a, v)	uatomic_set(a, v)
#define atomic64_add(v, a)	uatomic_add(a, v)
#define atomic64_sub(v, a)	uatomic_sub(a, v)
#define atomic64_inc(a)		uatomic_inc(a)
#define atomic64_dec(a)		uatomic_dec(a)
#else
/*
 * If we don't detect support for that, emulate it with a lock.  Currently
 * there are only three atomic64_t counters in userspace and none of them are
 * performance critical, so we serialize them all with a single mutex since
 * the kernel atomic64_t API doesn't have an _init call.
 */
extern pthread_mutex_t	atomic64_lock;

static inline int64_t
atomic64_read(atomic64_t *a)
{
	int64_t	ret;

	pthread_mutex_lock(&atomic64_lock);
	ret = *a;
	pthread_mutex_unlock(&atomic64_lock);
	return ret;
}

static inline void
atomic64_add(int64_t v, atomic64_t *a)
{
	pthread_mutex_lock(&atomic64_lock);
	(*a) += v;
	pthread_mutex_unlock(&atomic64_lock);
}

static inline void
atomic64_set(atomic64_t *a, int64_t v)
{
	pthread_mutex_lock(&atomic64_lock);
	(*a) = v;
	pthread_mutex_unlock(&atomic64_lock);
}

#define atomic64_inc(a)		atomic64_add(1, (a))
#define atomic64_dec(a)		atomic64_add(-1, (a))
#define atomic64_sub(v, a)	atomic64_add(-(v), (a))

#endif /* HAVE_URCU_ATOMIC64 */

#define __smp_mb()		cmm_smp_mb()

/* from compiler_types.h */
/*
 * __unqual_scalar_typeof(x) - Declare an unqualified scalar type, leaving
 *			       non-scalar types unchanged.
 */
/*
 * Prefer C11 _Generic for better compile-times and simpler code. Note 'char'
 * is not type-compatible with 'signed char', and we define a separate case.
 */
#define __scalar_type_to_expr_cases(type)				\
		unsigned type:	(unsigned type)0,			\
		signed type:	(signed type)0

#define __unqual_scalar_typeof(x) typeof(				\
		_Generic((x),						\
			char:	(char)0,				\
			__scalar_type_to_expr_cases(char),		\
			__scalar_type_to_expr_cases(short),		\
			__scalar_type_to_expr_cases(int),		\
			__scalar_type_to_expr_cases(long),		\
			__scalar_type_to_expr_cases(long long),		\
			default: (x)))

/* Is this type a native word size -- useful for atomic operations */
#define __native_word(t) \
	(sizeof(t) == sizeof(char) || sizeof(t) == sizeof(short) || \
	 sizeof(t) == sizeof(int) || sizeof(t) == sizeof(long))

#define compiletime_assert(a, s)	BUILD_BUG_ON(!(a))

#define compiletime_assert_atomic_type(t)				\
		compiletime_assert(__native_word(t),			\
			"Need native word sized stores/loads for atomicity.")

/* from rwonce.h */
/*
 * Yes, this permits 64-bit accesses on 32-bit architectures. These will
 * actually be atomic in some cases (namely Armv7 + LPAE), but for others we
 * rely on the access being split into 2x32-bit accesses for a 32-bit quantity
 * (e.g. a virtual address) and a strong prevailing wind.
 */
#define compiletime_assert_rwonce_type(t)					\
	compiletime_assert(__native_word(t) || sizeof(t) == sizeof(long long),	\
		"Unsupported access size for {READ,WRITE}_ONCE().")

/*
 * Use __READ_ONCE() instead of READ_ONCE() if you do not require any
 * atomicity. Note that this may result in tears!
 */
#ifndef __READ_ONCE
#define __READ_ONCE(x)	(*(const volatile __unqual_scalar_typeof(x) *)&(x))
#endif

#define READ_ONCE(x)								\
({										\
	compiletime_assert_rwonce_type(x);					\
	__READ_ONCE(x);								\
})

#define __WRITE_ONCE(x, val)							\
do {										\
	*(volatile typeof(x) *)&(x) = (val);					\
} while (0)

#define WRITE_ONCE(x, val)							\
do {										\
	compiletime_assert_rwonce_type(x);					\
	__WRITE_ONCE(x, val);							\
} while (0)

/* from barrier.h */
#ifndef __smp_store_release
#define __smp_store_release(p, v)					\
do {									\
	compiletime_assert_atomic_type(*p);				\
	__smp_mb();							\
	WRITE_ONCE(*p, v);						\
} while (0)
#endif

#ifndef __smp_load_acquire
#define __smp_load_acquire(p)						\
({									\
	__unqual_scalar_typeof(*p) ___p1 = READ_ONCE(*p);		\
	compiletime_assert_atomic_type(*p);				\
	__smp_mb();							\
	(typeof(*p))___p1;						\
})
#endif

#ifndef smp_store_release
#define smp_store_release(p, v) __smp_store_release((p), (v))
#endif

#ifndef smp_load_acquire
#define smp_load_acquire(p) __smp_load_acquire(p)
#endif

#endif /* __ATOMIC_H__ */
