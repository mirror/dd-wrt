#ifndef __BACKPORT_RANDOM_H
#define __BACKPORT_RANDOM_H
#include_next <linux/random.h>
#include <linux/version.h>

#if (LINUX_VERSION_IS_GEQ(3,3,0) && LINUX_VERSION_IS_LESS(3,4,10)) || \
    (LINUX_VERSION_IS_GEQ(3,1,0) && LINUX_VERSION_IS_LESS(3,2,27)) || \
    LINUX_VERSION_IS_LESS(3,0,41)
#define add_device_randomness LINUX_BACKPORT(add_device_randomness)
static inline void add_device_randomness(const void *buf, unsigned int size)
{
}
#endif

#if LINUX_VERSION_IS_LESS(3,8,0)
/* backports 496f2f9 */
#define prandom_seed(_seed)		srandom32(_seed)
#define prandom_u32()			random32()
#define prandom_u32_state(_state)	prandom32(_state)
/* backport 6582c665d6b882dad8329e05749fbcf119f1ab88 */
#define prandom_bytes LINUX_BACKPORT(prandom_bytes)
void prandom_bytes(void *buf, int bytes);
#endif

#if LINUX_VERSION_IS_LESS(3,14,0)
/**
 * prandom_u32_max - returns a pseudo-random number in interval [0, ep_ro)
 * @ep_ro: right open interval endpoint
 *
 * Returns a pseudo-random number that is in interval [0, ep_ro). Note
 * that the result depends on PRNG being well distributed in [0, ~0U]
 * u32 space. Here we use maximally equidistributed combined Tausworthe
 * generator, that is, prandom_u32(). This is useful when requesting a
 * random index of an array containing ep_ro elements, for example.
 *
 * Returns: pseudo-random number in interval [0, ep_ro)
 */
#define prandom_u32_max LINUX_BACKPORT(prandom_u32_max)
static inline u32 prandom_u32_max(u32 ep_ro)
{
	return (u32)(((u64) prandom_u32() * ep_ro) >> 32);
}
#endif /* LINUX_VERSION_IS_LESS(3,14,0) */

#if LINUX_VERSION_IS_LESS(4,9,0)
static inline u32 get_random_u32(void)
{
	return get_random_int();
}
#endif

#if LINUX_VERSION_IS_LESS(6,1,0)
static inline u8 get_random_u8(void)
{
	return get_random_u32() & 0xff;
}

static inline u16 get_random_u16(void)
{
	return get_random_u32() & 0xffff;
}

static inline u32 __get_random_u32_below(u32 ceil)
{
	/*
	 * This is the slow path for variable ceil. It is still fast, most of
	 * the time, by doing traditional reciprocal multiplication and
	 * opportunistically comparing the lower half to ceil itself, before
	 * falling back to computing a larger bound, and then rejecting samples
	 * whose lower half would indicate a range indivisible by ceil. The use
	 * of `-ceil % ceil` is analogous to `2^32 % ceil`, but is computable
	 * in 32-bits.
	 */
	u32 rand = get_random_u32();
	u64 mult;

	/*
	 * This function is technically undefined for ceil == 0, and in fact
	 * for the non-underscored constant version in the header, we build bug
	 * on that. But for the non-constant case, it's convenient to have that
	 * evaluate to being a straight call to get_random_u32(), so that
	 * get_random_u32_inclusive() can work over its whole range without
	 * undefined behavior.
	 */
	if (unlikely(!ceil))
		return rand;

	mult = (u64)ceil * rand;
	if (unlikely((u32)mult < ceil)) {
		u32 bound = -ceil % ceil;
		while (unlikely((u32)mult < bound))
			mult = (u64)ceil * get_random_u32();
	}
	return mult >> 32;
}

/*
 * Returns a random integer in the interval [0, ceil), with uniform
 * distribution, suitable for all uses. Fastest when ceil is a constant, but
 * still fast for variable ceil as well.
 */
static inline u32 get_random_u32_below(u32 ceil)
{
	if (!__builtin_constant_p(ceil))
		return __get_random_u32_below(ceil);

	/*
	 * For the fast path, below, all operations on ceil are precomputed by
	 * the compiler, so this incurs no overhead for checking pow2, doing
	 * divisions, or branching based on integer size. The resultant
	 * algorithm does traditional reciprocal multiplication (typically
	 * optimized by the compiler into shifts and adds), rejecting samples
	 * whose lower half would indicate a range indivisible by ceil.
	 */
	BUILD_BUG_ON_MSG(!ceil, "get_random_u32_below() must take ceil > 0");
	if (ceil <= 1)
		return 0;
	for (;;) {
		if (ceil <= 1U << 8) {
			u32 mult = ceil * get_random_u8();
			if (likely(is_power_of_2(ceil) || (u8)mult >= (1U << 8) % ceil))
				return mult >> 8;
		} else if (ceil <= 1U << 16) {
			u32 mult = ceil * get_random_u16();
			if (likely(is_power_of_2(ceil) || (u16)mult >= (1U << 16) % ceil))
				return mult >> 16;
		} else {
			u64 mult = (u64)ceil * get_random_u32();
			if (likely(is_power_of_2(ceil) || (u32)mult >= -ceil % ceil))
				return mult >> 32;
		}
	}
}

/*
 * Returns a random integer in the interval (floor, U32_MAX], with uniform
 * distribution, suitable for all uses. Fastest when floor is a constant, but
 * still fast for variable floor as well.
 */
static inline u32 get_random_u32_above(u32 floor)
{
	BUILD_BUG_ON_MSG(__builtin_constant_p(floor) && floor == U32_MAX,
			 "get_random_u32_above() must take floor < U32_MAX");
	return floor + 1 + get_random_u32_below(U32_MAX - floor);
}

/*
 * Returns a random integer in the interval [floor, ceil], with uniform
 * distribution, suitable for all uses. Fastest when floor and ceil are
 * constant, but still fast for variable floor and ceil as well.
 */
static inline u32 get_random_u32_inclusive(u32 floor, u32 ceil)
{
	BUILD_BUG_ON_MSG(__builtin_constant_p(floor) && __builtin_constant_p(ceil) &&
			 (floor > ceil || ceil - floor == U32_MAX),
			 "get_random_u32_inclusive() must take floor <= ceil");
	return floor + get_random_u32_below(ceil - floor + 1);
}
#endif /* <6.2 */

#endif /* __BACKPORT_RANDOM_H */
