/***
  This file is part of eudev, forked from systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#pragma once

#include <inttypes.h>

typedef uint64_t usec_t;
typedef uint64_t nsec_t;

#define USEC_FMT "%" PRIu64

#include "macro.h"

typedef struct dual_timestamp {
        usec_t realtime;
        usec_t monotonic;
} dual_timestamp;

#define USEC_INFINITY ((usec_t) -1)
#define NSEC_INFINITY ((nsec_t) -1)

#define MSEC_PER_SEC  1000ULL
#define USEC_PER_SEC  ((usec_t) 1000000ULL)
#define USEC_PER_MSEC ((usec_t) 1000ULL)
#define NSEC_PER_SEC  ((nsec_t) 1000000000ULL)
#define NSEC_PER_MSEC ((nsec_t) 1000000ULL)
#define NSEC_PER_USEC ((nsec_t) 1000ULL)

#define USEC_PER_MINUTE ((usec_t) (60ULL*USEC_PER_SEC))
#define NSEC_PER_MINUTE ((nsec_t) (60ULL*NSEC_PER_SEC))
#define USEC_PER_HOUR ((usec_t) (60ULL*USEC_PER_MINUTE))
#define NSEC_PER_HOUR ((nsec_t) (60ULL*NSEC_PER_MINUTE))
#define USEC_PER_DAY ((usec_t) (24ULL*USEC_PER_HOUR))
#define NSEC_PER_DAY ((nsec_t) (24ULL*NSEC_PER_HOUR))
#define USEC_PER_WEEK ((usec_t) (7ULL*USEC_PER_DAY))
#define NSEC_PER_WEEK ((nsec_t) (7ULL*NSEC_PER_DAY))
#define USEC_PER_MONTH ((usec_t) (2629800ULL*USEC_PER_SEC))
#define NSEC_PER_MONTH ((nsec_t) (2629800ULL*NSEC_PER_SEC))
#define USEC_PER_YEAR ((usec_t) (31557600ULL*USEC_PER_SEC))
#define NSEC_PER_YEAR ((nsec_t) (31557600ULL*NSEC_PER_SEC))

#define FORMAT_TIMESPAN_MAX 64

usec_t now(clockid_t clock);

usec_t timespec_load(const struct timespec *ts) _pure_;
struct timespec *timespec_store(struct timespec *ts, usec_t u);

struct timeval *timeval_store(struct timeval *tv, usec_t u);
char *format_timespan(char *buf, size_t l, usec_t t, usec_t accuracy);
