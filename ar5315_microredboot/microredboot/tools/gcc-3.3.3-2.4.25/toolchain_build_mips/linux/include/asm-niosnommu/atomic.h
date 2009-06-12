//vic - add 'atomic_add/sub_return', 'atomic_add_negative'
//vic     from v850 architecture

/* atomic.h: 
 *
 * Copyright (C) 2001 Vic Phillips (vic@microtronix.com)
 *
 * Copyright (C) 1996 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef __ARCH_NIOS_ATOMIC__
#define __ARCH_NIOS_ATOMIC__

#include <asm/system.h>

typedef struct { int counter; } atomic_t;
#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	(((v)->counter) = i)


extern __inline__ void atomic_add(int i, atomic_t *v)
{
	_disable_interrupts();
	v->counter += i;
	_enable_interrupts();
}

extern __inline__ void atomic_sub(int i, atomic_t *v)
{
	_disable_interrupts();
	v->counter -= i;
	_enable_interrupts();
}

extern __inline__ int atomic_sub_and_test(int i, atomic_t *v)
{
	int result;
	_disable_interrupts();
	v->counter -= i;
	result = (v->counter == 0);
	_enable_interrupts();
	return result;
}

extern __inline__ void atomic_inc(atomic_t *v)
{
	_disable_interrupts();
	v->counter += 1;
	_enable_interrupts();
}

extern __inline__ void atomic_dec(atomic_t *v)
{
	int i = 1;					/* the compiler optimizes better this way */
	_disable_interrupts();
	v->counter -= i;
	_enable_interrupts();
}

extern __inline__ int atomic_dec_and_test(atomic_t *v)
{
	int result;
	int i = 1;					/* the compiler optimizes better this way */
	_disable_interrupts();
	v->counter -= i;
	result = (v->counter == 0);
	_enable_interrupts();
	return result;
}

extern __inline__ int atomic_inc_return(atomic_t *v)
{
	int result;
	_disable_interrupts();
	result = ++v->counter;
	_enable_interrupts();
	return result;
}

extern __inline__ int atomic_dec_return(atomic_t *v)
{
	int result;
	int i = 1;					/* the compiler optimizes better this way */
	_disable_interrupts();
	v->counter -= i;
	result = v->counter;
	_enable_interrupts();
	return result;
}

extern __inline__ int atomic_add_return (int i, volatile atomic_t *v)
{
	int res;

	_disable_interrupts();
	res = v->counter + i;
	v->counter = res;
	_enable_interrupts();

	return res;
}

static __inline__ int atomic_sub_return (int i, volatile atomic_t *v)
{
	int res;

	_disable_interrupts();
	res = v->counter - i;
	v->counter = res;
	_enable_interrupts();

	return res;
}

#define atomic_add_negative(i,v)	(atomic_add_return ((i), (v)) < 0)

/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()


#endif /* !(__ARCH_NIOS_ATOMIC__) */


