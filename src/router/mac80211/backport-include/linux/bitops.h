#ifndef __BACKPORT_BITOPS_H
#define __BACKPORT_BITOPS_H
#include_next <linux/bitops.h>
#include <linux/version.h>
#include <generated/utsrelease.h>

#ifndef GENMASK

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l) \
	(((~0ULL) - (1ULL << (l)) + 1) & \
	 (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

#endif

#ifndef BIT_ULL
#define BIT_ULL(nr) (1ULL << (nr))
#endif

#ifndef BITS_PER_TYPE
#define BITS_PER_TYPE(type) (sizeof(type) * BITS_PER_BYTE)
#endif

#endif /* __BACKPORT_BITOPS_H */
