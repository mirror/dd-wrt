#ifndef _MORSE_OPERATIONS_H_
#define _MORSE_OPERATIONS_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/bitmap.h>
#include "morse.h"
#include "utils.h"

/**
 * enum morse_ops_flags - Features in operation on the morse device.
 */
enum morse_ops_flags {
	MORSE_OPS_DTIM_CTS_TO_SELF = 0,
	MORSE_OPS_LEGACY_AMSDU,

	MORSE_OPS_LAST = MORSE_OPS_LEGACY_AMSDU,
};

#define OPERATIONS_FLAGS_WIDTH   (MORSE_INT_CEIL(MORSE_OPS_LAST, 32))

struct morse_ops {
	unsigned long flags[OPERATIONS_FLAGS_WIDTH];
};

#define MORSE_OPS_IN_USE(MORSE_OPS, OPERATION)\
	morse_ops_in_use(MORSE_OPS, \
			   MORSE_OPS_##OPERATION)

#define MORSE_OPS_SET(MORSE_OPS, OPERATION)\
	morse_ops_set(MORSE_OPS, \
			   MORSE_OPS_##OPERATION)

#define MORSE_OPS_CLEAR(MORSE_OPS, OPERATION)\
	morse_ops_clear(MORSE_OPS, \
			   MORSE_OPS_##OPERATION)

/**
 * @brief Check if an operational feature is in use.
 *
 * @param mors The target morse operations.
 * @param flag The operation flag to check.
 * @return true if the operation is in use, false if otherwise.
 */
static inline bool morse_ops_in_use(struct morse_ops *ops, enum morse_ops_flags flag)
{
	const unsigned long *flags_ptr = ops->flags;

	return test_bit(flag, flags_ptr);
}

/**
 * @brief Set an operational flag.
 *
 * @param ops The ops struct to set.
 * @param flag The flag to set.
 */
static inline void morse_ops_set(struct morse_ops *ops, enum morse_ops_flags flag)
{
	unsigned long *flags_ptr = ops->flags;

	set_bit(flag, flags_ptr);
}

/**
 * @brief Clear an operational flag.
 *
 * @param ops The ops struct to clear.
 * @param flag The flag to clear.
 */
static inline void morse_ops_clear(struct morse_ops *ops, enum morse_ops_flags flag)
{
	unsigned long *flags_ptr = ops->flags;

	clear_bit(flag, flags_ptr);
}

#endif /* !_MORSE_OPERATIONS_H_ */
