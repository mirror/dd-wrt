/*
 * Embedded Linux library
 * Copyright (C) 2019  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <time.h>
#include <sys/time.h>

#include "time.h"
#include "time-private.h"
#include "random.h"
#include "private.h"

uint64_t _time_from_timespec(const struct timespec *ts)
{
	return ts->tv_sec * L_USEC_PER_SEC + ts->tv_nsec / L_NSEC_PER_USEC;
}

static uint64_t _time_from_timeval(const struct timeval *tv)
{
	return tv->tv_sec * L_USEC_PER_SEC + tv->tv_usec;
}

/**
 * l_time_now:
 *
 * Get the running clocktime in microseconds
 *
 * Returns: Current clock time in microseconds
 **/
LIB_EXPORT uint64_t l_time_now(void)
{
	struct timespec now;

	clock_gettime(CLOCK_BOOTTIME, &now);
	return _time_from_timespec(&now);
}

uint64_t time_realtime_now(void)
{
	struct timespec now;

	clock_gettime(CLOCK_REALTIME, &now);
	return _time_from_timespec(&now);
}

/**
 * l_time_after
 *
 * Returns: True if time a is after time b
 **/

/**
 * l_time_before
 *
 * Returns: True if time a is before time b
 **/

/**
 * l_time_offset
 *
 * @time: Start time to calculate offset
 * @offset: Amount of time to add to 'time'
 *
 * Adds an offset to a time value. This checks for overflow, and if detected
 * returns UINT64_MAX.
 *
 * Returns: A time value 'time' + 'offset'. Or UINT64_MAX if time + offset
 * exceeds UINT64_MAX.
 **/

/* Compute ms + RAND*ms where RAND is in range -0.1 .. 0.1 */
uint64_t _time_fuzz_msecs(uint64_t ms)
{
	/* We do this by subtracting 0.1ms and adding 0.1ms * rand[0 .. 2] */
	return ms - ms / 10 +
			(l_getrandom_uint32() % (2 * L_MSEC_PER_SEC)) *
						ms / 10 / L_MSEC_PER_SEC;
}

uint64_t _time_pick_interval_secs(uint32_t min_secs, uint32_t max_secs)
{
	uint64_t min_ms = min_secs * L_MSEC_PER_SEC;
	uint64_t max_ms = max_secs * L_MSEC_PER_SEC;

	return l_getrandom_uint32() % (max_ms + 1 - min_ms) + min_ms;
}

/* Compute a time in ms based on seconds + max_offset * [-1.0 .. 1.0] */
uint64_t _time_fuzz_secs(uint32_t secs, uint32_t max_offset)
{
	uint64_t ms = secs * L_MSEC_PER_SEC;
	uint64_t r = l_getrandom_uint32();

	max_offset *= L_MSEC_PER_SEC;

	if (r & 0x80000000)
		ms += (r & 0x7fffffff) % max_offset;
	else
		ms -= (r & 0x7fffffff) % max_offset;

	return ms;
}

/*
 * Convert a *recent* CLOCK_REALTIME-based timestamp to a
 * CLOCK_BOOTTIME-based usec count consistent with l_time functions.
 * The longer the time since the input timestamp the higher the
 * probability of the two clocks having diverged and the higher the
 * expected error magnitude.
 */
uint64_t _time_realtime_to_boottime(const struct timeval *ts)
{
	uint64_t now_realtime;
	uint64_t now_boottime = l_time_now();
	struct timespec timespec;
	uint64_t ts_realtime;
	uint64_t offset;

	clock_gettime(CLOCK_REALTIME, &timespec);
	now_realtime = _time_from_timespec(&timespec);
	ts_realtime = _time_from_timeval(ts);
	offset = l_time_diff(ts_realtime, now_realtime);

	/* Most likely case, timestamp in the past */
	if (l_time_before(ts_realtime, now_realtime)) {
		if (offset > now_boottime)
			return 0;

		return now_boottime - offset;
	}

	return l_time_offset(now_boottime, offset);
}
