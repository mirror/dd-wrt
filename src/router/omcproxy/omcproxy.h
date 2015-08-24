/*
 * Author: Steven Barth <steven at midlink.org>
 *
 * Copyright 2015 Deutsche Telekom AG
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef OMGPROXY_H_
#define OMGPROXY_H_

#define OMGPROXY_DEFAULT_L_LEVEL 7

#ifndef L_LEVEL
#define L_LEVEL OMGPROXY_DEFAULT_L_LEVEL
#endif /* !L_LEVEL */

#ifndef L_PREFIX
#define L_PREFIX ""
#endif /* !L_PREFIX */

#ifdef __APPLE__

#define __APPLE_USE_RFC_3542
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP

#include <sys/queue.h>
#ifdef LIST_HEAD
#undef LIST_HEAD
#endif /* LIST_HEAD */

#endif /* __APPLE__ */

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <libubox/utils.h>

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

typedef int64_t omgp_time_t;
#define OMGP_TIME_MAX INT64_MAX
#define OMGP_TIME_PER_SECOND INT64_C(1000)

static inline omgp_time_t omgp_time(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((omgp_time_t)ts.tv_sec * OMGP_TIME_PER_SECOND) +
			((omgp_time_t)ts.tv_nsec / (1000000000 / OMGP_TIME_PER_SECOND));
}

extern int log_level;

// Logging macros

#define L_INTERNAL(level, ...)                  \
do {                                            \
  if (log_level >= level)                       \
    syslog(level, L_PREFIX __VA_ARGS__);        \
 } while(0)

#if L_LEVEL >= LOG_ERR
#define L_ERR(...) L_INTERNAL(LOG_ERR, __VA_ARGS__)
#else
#define L_ERR(...) do {} while(0)
#endif

#if L_LEVEL >= LOG_WARNING
#define L_WARN(...) L_INTERNAL(LOG_WARNING, __VA_ARGS__)
#else
#define L_WARN(...) do {} while(0)
#endif

#if L_LEVEL >= LOG_NOTICE
#define L_NOTICE(...) L_INTERNAL(LOG_NOTICE, __VA_ARGS__)
#else
#define L_NOTICE(...) do {} while(0)
#endif

#if L_LEVEL >= LOG_INFO
#define L_INFO(...) L_INTERNAL(LOG_INFO, __VA_ARGS__)
#else
#define L_INFO(...) do {} while(0)
#endif

#if L_LEVEL >= LOG_DEBUG
#define L_DEBUG(...) L_INTERNAL(LOG_DEBUG, __VA_ARGS__)
#else
#define L_DEBUG(...) do {} while(0)
#endif


// Some C99 compatibility
#ifndef typeof
#define typeof __typeof
#endif

#ifndef container_of
#define container_of(ptr, type, member) (           \
    (type *)( (char *)ptr - offsetof(type,member) ))
#endif

#ifndef __unused
#define __unused __attribute__((unused))
#endif

#endif /* PIMBD_H_ */
