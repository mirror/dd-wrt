#ifndef __ARCH_M68KNOMMU_ATOMIC__
#define __ARCH_M68KNOMMU_ATOMIC__

#include <asm/system.h>	/* save_and_cli() */

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

/*
 * We do not have SMP m68k systems, so we don't have to deal with that.
 */

typedef struct { int counter; } atomic_t;
#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	(((v)->counter) = i)

#ifdef CONFIG_COLDFIRE

static __inline__ void atomic_add(int i, atomic_t *v)
{
	__asm__ __volatile__(
		"movel %1,%%d0\t\n"
		"addl %%d0,%0"
		:
		: "m" (*v), "id" (i)
		: "d0");
}

static __inline__ void atomic_sub(int i, atomic_t *v)
{
	__asm__ __volatile__(
		"movel %1,%%d0\t\n"
		"subl %%d0,%0"
		:
		: "m" (*v), "id" (i)
		: "d0");
}

#else

static __inline__ void atomic_add(int i, atomic_t *v)
{
	__asm__ __volatile__("addl %1,%0" : "=m" (*v) : "d" (i), "0" (*v));
}

static __inline__ void atomic_sub(int i, atomic_t *v)
{
	__asm__ __volatile__("subl %1,%0" : "=m" (*v) : "d" (i), "0" (*v));
}

#endif /* COLDFIRE */

static __inline__ void atomic_inc(volatile atomic_t *v)
{
	__asm__ __volatile__("addql #1,%0" : "=m" (*v): "0" (*v));
}

static __inline__ void atomic_dec(volatile atomic_t *v)
{
	__asm__ __volatile__("subql #1,%0" : "=m" (*v): "0" (*v));
}

static __inline__ int atomic_dec_and_test(volatile atomic_t *v)
{
	char c;
	__asm__ __volatile__("subql #1,%1; seq %0" : "=d" (c), "=m" (*v): "1" (*v));
	return c != 0;
}

#define atomic_clear_mask(mask, v) \
	__asm__ __volatile__("andl %1,%0" : "=m" (*v) : "id" (~(mask)),"0"(*v))

#define atomic_set_mask(mask, v) \
	__asm__ __volatile__("orl %1,%0" : "=m" (*v) : "id" (mask),"0"(*v))

/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()    barrier()
#define smp_mb__after_atomic_dec() barrier()
#define smp_mb__before_atomic_inc()    barrier()
#define smp_mb__after_atomic_inc() barrier()

extern __inline__ int atomic_add_return(int i, atomic_t * v)
{
	unsigned long temp, flags;

	save_and_cli(flags);
	temp = *(long *)v;
	temp += i;
	*(long *)v = temp;
	restore_flags(flags);

	return temp;
}

extern __inline__ int atomic_sub_return(int i, atomic_t * v)
{
	unsigned long temp, flags;

	save_and_cli(flags);
	temp = *(long *)v;
	temp -= i;
	*(long *)v = temp;
	restore_flags(flags);

	return temp;
}

#define atomic_dec_return(v) atomic_sub_return(1,(v))
#define atomic_inc_return(v) atomic_add_return(1,(v))

#define atomic_sub_and_test(i,v) (atomic_sub_return((i), (v)) == 0)
#define atomic_dec_and_test(v) (atomic_sub_return(1, (v)) == 0)

#endif /* __ARCH_M68KNOMMU_ATOMIC __ */
