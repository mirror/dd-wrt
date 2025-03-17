/* iso_alloc_random.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

/* Contributed by Oscar Reparaz (@oreparaz)
 * https://github.com/struct/isoalloc/pull/5 */

#include "iso_alloc_internal.h"

#define OLD_GLIBC (__GLIBC__ == 2 && __GLIBC_MINOR__ <= 24)

#if OLD_GLIBC
#include <linux/random.h>
#include <sys/syscall.h>
#elif __APPLE__
#include <Security/SecRandom.h>
#elif __FreeBSD__ || __DragonFly__ || __linux__ || __ANDROID__
#include <sys/random.h>
#elif __NetBSD__
#include <stdlib.h>
#elif __sun
#include <sys/random.h>
#else
#error "unknown OS"
#endif

/* Adapted from Daniel Lemire (@lemire). The code can be found at
 * https://github.com/lemire/testingRNG/blob/master/source/wyhash.h
 * Which is adapted from wyhash from @wangyi-fudan
 *
 * This is not suitable for anything cryptographic. We use it in
 * IsoAlloc for generating pointer masks, shuffling arrays etc,
 * and the initial seed is refreshed with rand_uint64() each time
 * we create a new zone.
 *
 * The magic values below are just random 64 bit primes. These values
 * are combined with our seed and then run through the mum function.
 * See the original wyhash paper for more details.
 * https://github.com/wangyi-fudan/wyhash/ */
INTERNAL_HIDDEN INLINE uint64_t us_rand_uint64(uint64_t *seed) {
    *seed += 0x5d6447c8df9375b5;
    __uint128_t tmp = (__uint128_t) *seed * 0x3c5829f2fb3c3eef;
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp = (__uint128_t) m1 * 0x4f38e4ccc1b0ea59;
    return (tmp >> 64) ^ tmp;
}

INTERNAL_HIDDEN INLINE uint64_t rand_uint64(void) {
    uint64_t val = 0;
    int ret = 0;

/* In modern versions of glibc (>=2.25) we can call getrandom(),
 * but older versions of glibc are still in use as of writing this.
 * Use the raw system call as a lower common denominator.
 * We give up on checking the return value. The alternative would be
 * to crash. We prefer here to keep going with degraded randomness. */
#if OLD_GLIBC
    ret = syscall(SYS_getrandom, &val, sizeof(val), GRND_NONBLOCK) != sizeof(val);
#elif __APPLE__
    ret = SecRandomCopyBytes(kSecRandomDefault, sizeof(val), &val);
#elif __FreeBSD__ || __DragonFly__ || __linux__ || __ANDROID__ || __sun
    ret = getrandom(&val, sizeof(val), GRND_NONBLOCK) != sizeof(val);
#elif __NetBSD__
    /* Temporary solution until NetBSD 10 released with getrandom support */
    arc4random_buf(&val, sizeof(val));
#endif

#if ABORT_NO_ENTROPY
    if(ret != 0) {
        LOG_AND_ABORT("Unable to gather enough entropy");
    }
#endif

    return val;
}
