/*
 * Copyright (C) 2006 Rackable Systems All rights reserved.
 * Copyright (C) 2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Abstract out the time methods used so they can be adjusted later -
 * the results of these routines should stay in-core.  
 */

#include "libdm/misc/dmlib.h"

#include <stdlib.h>

#define NSEC_PER_USEC	UINT64_C(1000)
#define NSEC_PER_MSEC	UINT64_C(1000000)
#define NSEC_PER_SEC	UINT64_C(1000000000)

/*
 * The realtime section uses clock_gettime with the CLOCK_MONOTONIC
 * parameter to prevent issues with time warps
 * This implementation requires librt.
 */
#ifdef HAVE_REALTIME

#include <time.h>

struct dm_timestamp {
	struct timespec t;
};

static uint64_t _timestamp_to_uint64(struct dm_timestamp *ts)
{
	uint64_t stamp = 0;

	stamp += (uint64_t) ts->t.tv_sec * NSEC_PER_SEC;
	stamp += (uint64_t) ts->t.tv_nsec;

	return stamp;
}

struct dm_timestamp *dm_timestamp_alloc(void)
{
	struct dm_timestamp *ts = NULL;

	if (!(ts = dm_zalloc(sizeof(*ts))))
		stack;

	return ts;
}

int dm_timestamp_get(struct dm_timestamp *ts)
{
	if (!ts)
		return 0;

	if (clock_gettime(CLOCK_MONOTONIC, &ts->t)) {
		log_sys_error("clock_gettime", "get_timestamp");
		ts->t.tv_sec = 0;
		ts->t.tv_nsec = 0;
		return 0;
	}

	return 1;
}

#else /* ! HAVE_REALTIME */

/*
 * The !realtime section just uses gettimeofday and is therefore subject
 * to ntp-type time warps - not sure if should allow that.
 */

#include <sys/time.h>

struct dm_timestamp {
	struct timeval t;
};

static uint64_t _timestamp_to_uint64(struct dm_timestamp *ts)
{
	uint64_t stamp = 0;

	stamp += ts->t.tv_sec * NSEC_PER_SEC;
	stamp += ts->t.tv_usec * NSEC_PER_USEC;

	return stamp;
}

struct dm_timestamp *dm_timestamp_alloc(void)
{
	struct dm_timestamp *ts;

	if (!(ts = dm_malloc(sizeof(*ts))))
		stack;

	return ts;
}

int dm_timestamp_get(struct dm_timestamp *ts)
{
	if (!ts)
		return 0;

	if (gettimeofday(&ts->t, NULL)) {
		log_sys_error("gettimeofday", "get_timestamp");
		ts->t.tv_sec = 0;
		ts->t.tv_usec = 0;
		return 0;
	}

	return 1;
}

#endif /* HAVE_REALTIME */

/*
 * Compare two timestamps.
 *
 * Return: -1 if ts1 is less than ts2
 *          0 if ts1 is equal to ts2
 *          1 if ts1 is greater than ts2
 */
int dm_timestamp_compare(struct dm_timestamp *ts1, struct dm_timestamp *ts2)
{
	uint64_t t1, t2;

	t1 = _timestamp_to_uint64(ts1);
	t2 = _timestamp_to_uint64(ts2);

	if (t2 < t1)
		return 1;

	if (t1 < t2)
		return -1;

	return 0;
}

/*
 * Return the absolute difference in nanoseconds between
 * the dm_timestamp objects ts1 and ts2.
 *
 * Callers that need to know whether ts1 is before, equal to, or after ts2
 * in addition to the magnitude should use dm_timestamp_compare.
 */
uint64_t dm_timestamp_delta(struct dm_timestamp *ts1, struct dm_timestamp *ts2)
{
	uint64_t t1, t2;

	t1 = _timestamp_to_uint64(ts1);
	t2 = _timestamp_to_uint64(ts2);

	if (t1 > t2)
		return t1 - t2;

	return t2 - t1;
}

void dm_timestamp_copy(struct dm_timestamp *ts_new, struct dm_timestamp *ts_old)
{
	*ts_new = *ts_old;
}

void dm_timestamp_destroy(struct dm_timestamp *ts)
{
	dm_free(ts);
}
