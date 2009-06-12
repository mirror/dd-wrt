/*
 * include/asm-microblaze/atomic.h -- Atomic operations
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_ATOMIC_H__
#define __MICROBLAZE_ATOMIC_H__

#include <linux/config.h>

#include <asm/system.h>

#ifdef CONFIG_SMP
#error SMP not supported
#endif

typedef struct { int counter; } atomic_t;

#define ATOMIC_INIT(i)	{ (i) }

#ifdef __KERNEL__

#define atomic_read(v)		((v)->counter)
#define atomic_set(v,i)		(((v)->counter) = (i))

extern __inline__ int atomic_add_return (int i, volatile atomic_t *v)
{
	unsigned long flags;
	int res;

	__save_flags_cli (flags);
	res = v->counter + i;
	v->counter = res;
	__restore_flags (flags);

	return res;
}

static __inline__ int atomic_sub_return (int i, volatile atomic_t *v)
{
	unsigned long flags;
	int res;

	__save_flags_cli (flags);
	res = v->counter - i;
	v->counter = res;
	__restore_flags (flags);

	return res;
}

static __inline__ void atomic_clear_mask (unsigned long mask, unsigned long *addr)
{
	unsigned long flags;

	__save_flags_cli (flags);
	*addr &= ~mask;
	__restore_flags (flags);
}

#endif

#define atomic_add(i, v)	atomic_add_return ((i), (v))
#define atomic_sub(i, v)	atomic_sub_return ((i), (v))

#define atomic_dec_return(v)	atomic_sub_return (1, (v))
#define atomic_inc_return(v)	atomic_add_return (1, (v))
#define atomic_inc(v) 		atomic_inc_return (v)
#define atomic_dec(v) 		atomic_dec_return (v)

#define atomic_sub_and_test(i,v)	(atomic_sub_return ((i), (v)) == 0)
#define atomic_dec_and_test(v)		(atomic_sub_return (1, (v)) == 0)
#define atomic_add_negative(i,v)	(atomic_add_return ((i), (v)) < 0)

/* Atomic operations are already serializing on ARM */
#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()

#endif /* __MICROBLAZE_ATOMIC_H__ */
