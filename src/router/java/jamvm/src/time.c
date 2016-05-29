/*
 * Copyright (C) 2008 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sys/time.h>
#include <time.h>
#include <limits.h>

void getTimeoutAbsolute(struct timespec *ts, long long millis,
                        long long nanos) {

    /* Calculate seconds (long long prevents overflow) */
    long long seconds = millis / 1000 + nanos / 1000000000;

    /* Calculate nanoseconds */
    nanos %= 1000000000;
    nanos += (millis % 1000) * 1000000;

    /* Adjust values so that nanos is less than 1 second */
    if(nanos > 999999999) {
        seconds++;
        nanos -= 1000000000;
    }

    /* If seconds is too big to fit into the timespec use the
       maximum value (year 2038) */
    ts->tv_sec = seconds > LONG_MAX ? LONG_MAX : seconds;
    ts->tv_nsec = nanos;
}

void getTimeoutRelative(struct timespec *ts, long long millis,
                        long long nanos) {
    struct timeval tv;
    long long seconds;

    /* Get the current time */
    gettimeofday(&tv, NULL);

    /* Calculate seconds (long long prevents overflow) */
    seconds = tv.tv_sec + millis / 1000 + nanos / 1000000000;

    /* Calculate nanoseconds */
    nanos %= 1000000000;
    nanos += (tv.tv_usec + ((millis % 1000) * 1000)) * 1000;

    /* Adjust values so that nanos is less than 1 second.
       This also prevents overflowing the timespec, as the
       value may be larger than tv_nsec (signed int) */
    seconds += nanos / 1000000000;
    nanos %= 1000000000;

    /* If seconds is too big to fit into the timespec use the
       maximum value (year 2038) */
    ts->tv_sec = seconds > LONG_MAX ? LONG_MAX : seconds;
    ts->tv_nsec = nanos;
}

