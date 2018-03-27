/*
 * clock_gettime.h - clock_gettime() support with fallback to gettimeofday()
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Authors: Alexandru Ardelean <ardeleanalex@gmail.com>
 */

#ifndef __CLOCK_GETTIME_H__
#define __CLOCK_GETTIME_H__

#include <config.h>
#include <time.h>
#include <sys/time.h>

#if HAVE_STRUCT_TIMESPEC
// Nothing
#else
struct timespec
{
    long tv_sec;        /* Seconds.  */
    long tv_nsec;       /* Nanoseconds.  */
}
#endif /* HAVE_STRUCT_TIMESPEC */

#if HAVE_CLOCK_GETTIME
// Nothing
#else

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC	1
#endif

static inline int clock_gettime (clockid_t clock, struct timespec *ts)
{
    struct timeval tv;

    if (gettimeofday (&tv, NULL))
        return -1;
        
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
    return 0;
}

#endif /* HAVE_CLOCK_GETTIME */

#endif /* __CLOCK_GETTIME_H__ */
