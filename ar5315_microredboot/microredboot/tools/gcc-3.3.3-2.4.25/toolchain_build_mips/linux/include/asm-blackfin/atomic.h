#ifndef __ARCH_FRIONOMMU_ATOMIC__
#define __ARCH_FRIONOMMU_ATOMIC__

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 *
 * Generally we do not concern about SMP FRIO systems, so we don't have
 * to deal with that.
 *
 * Tony Kou (tonyko@lineo.ca)   Lineo Inc.   2001
 */

typedef struct { int counter; } atomic_t;
#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	(((v)->counter) = i)


static __inline__ void atomic_add(int i, atomic_t *v)
{	
	int __temp = 0;
	__asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 = %0 + %2;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "d" (i), "m" (v->counter), "0" (__temp)
		: "R3");
}

static __inline__ void atomic_sub(int i, atomic_t *v)
{
	int __temp = 0;
	__asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 = %0 - %2;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "d" (i), "m" (v->counter), "0" (__temp)
		: "R3");
}

static __inline__ int atomic_add_return(int i, atomic_t * v)
{
	int __temp = 0;
	__asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 = %0 + %2;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "d" (i), "m" (v->counter), "0" (__temp)
		: "R3");

	return __temp;
}

static __inline__ int atomic_sub_return(int i, atomic_t * v)
{
	int __temp = 0;
	__asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 = %0 - %2;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "d" (i), "m" (v->counter), "0" (__temp)
		: "R3");

	return __temp;
}


static __inline__ void atomic_inc(volatile atomic_t *v)
{
	int __temp = 0;
	__asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 += 1;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "m" (v->counter), "0" (__temp)
		: "R3");
}

static __inline__ void atomic_dec(volatile atomic_t *v)
{ 
	int __temp = 0;
	__asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 += -1;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "m" (v->counter), "0" (__temp)
		: "R3");
}

static __inline__ void atomic_clear_mask(unsigned int mask, atomic_t *v)
{
	int __temp = 0;
        __asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 = %0 & %2;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "d" (~(mask)), "m" (v->counter), "0" (__temp)
		: "R3");
}

static __inline__ void atomic_set_mask(unsigned int mask, atomic_t *v)
{
	int __temp = 0;
        __asm__ __volatile__(
		"cli R3;\n\t"
		"%0 = %1;\n\t"
		"%0 = %0 | %2;\n\t"
		"%1 = %0;\n\t"
		"sti R3;\n\t"
		: "=d" (__temp), "=m" (v->counter)
		: "d" (mask), "m" (v->counter), "0" (__temp)
		: "R3");
}

#define atomic_dec_return(v) atomic_sub_return(1,(v))
#define atomic_inc_return(v) atomic_add_return(1,(v))

#define atomic_sub_and_test(i,v) (atomic_sub_return((i), (v)) == 0)
#define atomic_dec_and_test(v) (atomic_sub_return(1, (v)) == 0)

/*
 * XXX: Defined here because needed in put_bh()
 */
/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()    barrier()
#define smp_mb__after_atomic_dec() barrier()
#define smp_mb__before_atomic_inc()    barrier()
#define smp_mb__after_atomic_inc() barrier()

#endif /* __ARCH_FRIONOMMU_ATOMIC __ */
