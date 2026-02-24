/*
 * Copyright 2022-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _MORSE_UTILS_H_
#define _MORSE_UTILS_H_

/** Integer ceiling function */
#define MORSE_INT_CEIL(_num, _div)	(((_num) + (_div) - 1) / (_div))

/** Convert from a time in us to time units (1024us) */
#define MORSE_US_TO_TU(x)		((x) / 1024)

/** Convert from a time in time units (1024us) to us */
#define MORSE_TU_TO_US(x)		((x) * 1024UL)

/** Convert from a time in time units (1024us) to ms */
#define MORSE_TU_TO_MS(x)		(MORSE_TU_TO_US(x) / 1000UL)

/** Convert seconds to milliseconds */
#define MORSE_SECS_TO_MSECS(x)	((x) * 1000)

#define LOWER_32_BITS(x) ((x) & GENMASK(31, 0))
#define UPPER_32_BITS(x) (((x) >> 32) & GENMASK(31, 0))

#define NSS_IDX_TO_NSS(x) ((x) + 1)
#define NSS_TO_NSS_IDX(x) ((x) - 1)

/**
 * @brief Align ptr to alignment downwards (ie. to a lower value)
 * eg. Aligning to 4 - 0x80000003 -> 0x80000000
 * The kernel provides a macro for this after v5.10
 *
 * @param ptr Pointer to align
 * @param alignment byte boundary to align to
 * @return void* Aligned pointer, adjusted downward (towards zero)
 */
static inline void *align_down(void *ptr, uint alignment)
{
	return (void *)((uintptr_t)ptr & -(uintptr_t)(alignment));
}

/**
 * Make sure val is a power of 2
 */
#define IS_POWER_OF_TWO(val) (!((val) & ((val) - 1)))

/**
 * Some kernels won't have spectre mitigations. To get around this, we just define this
 * as a macro to pass the value through
 */
#ifndef array_index_nospec
#define array_index_nospec(x, y) (x)
#endif

/**
 * Older kernels don't have this define, so copy it in.
 */
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
#ifndef DECLARE_FLEX_ARRAY
	/**
	 * __DECLARE_FLEX_ARRAY() - Declare a flexible array usable in a union
	 *
	 * @TYPE: The type of each flexible array element
	 * @NAME: The name of the flexible array member
	 *
	 * In order to have a flexible array member in a union or alone in a
	 * struct, it needs to be wrapped in an anonymous struct with at least 1
	 * named member, but that member can be empty.
	 */
#define DECLARE_FLEX_ARRAY(TYPE, NAME)	\
		struct { \
			struct { } __empty_ ## NAME; \
			TYPE NAME[]; \
		}
#endif
#endif

#endif /* !_MORSE_UTILS_H_ */
