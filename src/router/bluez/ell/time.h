/*
 * Embedded Linux library
 * Copyright (C) 2019  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_TIME_H
#define __ELL_TIME_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define L_USEC_PER_SEC 1000000ULL
#define L_MSEC_PER_SEC 1000ULL
#define L_USEC_PER_MSEC 1000ULL
#define L_NSEC_PER_SEC  1000000000ULL
#define L_NSEC_PER_MSEC 1000000ULL
#define L_NSEC_PER_USEC 1000ULL
#define L_TIME_INVALID ((uint64_t) -1)

uint64_t l_time_now(void);

static inline bool l_time_after(uint64_t a, uint64_t b)
{
	return a > b;
}

static inline bool l_time_before(uint64_t a, uint64_t b)
{
	return l_time_after(b, a);
}

static inline uint64_t l_time_offset(uint64_t time, uint64_t offset)
{
	/* check overflow */
	if (offset > UINT64_MAX - time)
		return UINT64_MAX;

	return time + offset;
}

static inline uint64_t l_time_diff(uint64_t a, uint64_t b)
{
	return (a < b) ? b - a : a - b;
}

static inline uint64_t l_time_to_secs(uint64_t time)
{
	return time / L_USEC_PER_SEC;
}

static inline uint64_t l_time_to_msecs(uint64_t time)
{
	return time / L_USEC_PER_MSEC;
}

#ifdef __cplusplus
}
#endif

#endif /* __ELL_TIME_H */
