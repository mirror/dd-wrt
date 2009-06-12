#ifndef _HYPERSTONE_NOMMU_ATOMIC_H
#define _HYPERSTONE_NOMMU_ATOMIC_H

#include <asm/system.h>
/*
 * We do not have SMP Hyperstone systems, so we don't have to deal with that.
 */

typedef struct { int counter; } atomic_t;
#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	(((v)->counter) = i)

static __inline__ int atomic_add(int i, atomic_t *v)
{
	int newval;
	cli();
	newval = v->counter += i;
	sti();
	return newval;
}

static __inline__ int atomic_add_negative(int i, atomic_t *v)
{
	int newval;
	cli();
	newval = v->counter += i;
	sti();
	return newval < 0;
}

static __inline__ void atomic_sub(int i, atomic_t *v)
{
	cli();
	v->counter -= i;
	sti();
}

static __inline__ void atomic_inc(volatile atomic_t *v)
{
	cli();
	v->counter += 1;
	sti();
}

static __inline__ void atomic_dec(volatile atomic_t *v)
{
	cli();
	v->counter -= 1;
	sti();
}

static __inline__ int atomic_dec_and_test(volatile atomic_t *v)
{
	int result;

	cli();
	v->counter -= 1;
	result = (v->counter == 0);
	sti();
	
	return result;
}

static __inline__ void atomic_clear_mask(unsigned long mask, volatile atomic_t *v)
{
	cli();
	v->counter &= mask;
	sti();
}

static __inline__ void atomic_set_mask(unsigned long mask, volatile atomic_t *v)
{
	cli();
	v->counter |= mask;
	sti();
}

extern __inline__ int atomic_add_return(int i, atomic_t * v)
{
	unsigned long temp;

	cli();
	temp = v->counter;
	temp += i;
	v->counter = temp;
	sti();

	return temp;
}

extern __inline__ int atomic_sub_return(int i, atomic_t * v)
{
	unsigned long temp;

	cli();
	temp = v->counter;
	temp -= i;
	v->counter = temp;
	sti();

	return temp;
}

#define atomic_dec_return(v) atomic_sub_return(1,(v))
#define atomic_inc_return(v) atomic_add_return(1,(v))

#define atomic_sub_and_test(i,v) (atomic_sub_return((i), (v)) == 0)
#define atomic_dec_and_test(v) (atomic_sub_return(1, (v)) == 0)

/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()    barrier()
#define smp_mb__after_atomic_dec() barrier()
#define smp_mb__before_atomic_inc()    barrier()
#define smp_mb__after_atomic_inc() barrier()


#endif /* _HYPERSTONE_NOMMU_ATOMIC_H */
