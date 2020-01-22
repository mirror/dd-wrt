/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 */

#ifndef __KSMBD_TIME_WRAPPERS_H
#define __KSMBD_TIME_WRAPPERS_H

/*
 * A bunch of ugly hacks to workaoround all the API differences
 * between different kernel versions.
 */

#define NTFS_TIME_OFFSET	((u64)(369*365 + 89) * 24 * 3600 * 10000000)

/* Convert the Unix UTC into NT UTC. */
static inline u64 ksmbd_UnixTimeToNT(struct timespec t)
{
	/* Convert to 100ns intervals and then add the NTFS time offset. */
	return (u64) t.tv_sec * 10000000 + t.tv_nsec / 100 + NTFS_TIME_OFFSET;
}

struct timespec ksmbd_NTtimeToUnix(__le64 ntutc);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
static inline struct timespec64 to_kern_timespec(struct timespec ts)
{
	return timespec_to_timespec64(ts);
}

static inline struct timespec from_kern_timespec(struct timespec64 ts)
{
	return timespec64_to_timespec(ts);
}
#else
#define to_kern_timespec(ts) (ts)
#define from_kern_timespec(ts) (ts)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
#define KSMBD_TIME_TO_TM	time64_to_tm
#else
#define KSMBD_TIME_TO_TM	time_to_tm
#endif

static inline long long ksmbd_systime(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
	struct timespec		ts;

	getnstimeofday(&ts);
	return ksmbd_UnixTimeToNT(ts);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
	struct timespec64	ts;

	getnstimeofday64(&ts);
	return ksmbd_UnixTimeToNT(timespec64_to_timespec(ts));
#else
	struct timespec		ts;

	getnstimeofday(&ts);
	return ksmbd_UnixTimeToNT(ts);
#endif
}
#endif /* __KSMBD_TIME_WRAPPERS_H */
