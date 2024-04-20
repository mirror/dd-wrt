/**
 * @file compat.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief compatibility functions
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _POSIX_C_SOURCE 200809L /* fdopen, _POSIX_PATH_MAX, strdup */
#define _ISOC99_SOURCE /* vsnprintf */

#include "compat.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifndef HAVE_VDPRINTF
int
vdprintf(int fd, const char *format, va_list ap)
{
    FILE *stream;
    int count = 0;

    stream = fdopen(dup(fd), "a+");
    if (stream) {
        count = vfprintf(stream, format, ap);
        fclose(stream);
    }
    return count;
}

#endif

#ifndef HAVE_ASPRINTF
int
asprintf(char **strp, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vasprintf(strp, fmt, ap);
    va_end(ap);
    return ret;
}

#endif

#ifndef HAVE_VASPRINTF
int
vasprintf(char **strp, const char *fmt, va_list ap)
{
    va_list ap2;

    va_copy(ap2, ap);
    int l = vsnprintf(0, 0, fmt, ap2);

    va_end(ap2);

    if ((l < 0) || !(*strp = malloc(l + 1U))) {
        return -1;
    }

    return vsnprintf(*strp, l + 1U, fmt, ap);
}

#endif

#ifndef HAVE_GETLINE
ssize_t
getline(char **lineptr, size_t *n, FILE *stream)
{
    static char line[256];
    char *ptr;
    ssize_t len;

    if (!lineptr || !n) {
        errno = EINVAL;
        return -1;
    }

    if (ferror(stream) || feof(stream)) {
        return -1;
    }

    if (!fgets(line, 256, stream)) {
        return -1;
    }

    ptr = strchr(line, '\n');
    if (ptr) {
        *ptr = '\0';
    }

    len = strlen(line);

    if (len + 1 < 256) {
        ptr = realloc(*lineptr, 256);
        if (!ptr) {
            return -1;
        }
        *lineptr = ptr;
        *n = 256;
    }

    strcpy(*lineptr, line);
    return len;
}

#endif

#ifndef HAVE_STRNDUP
char *
strndup(const char *s, size_t n)
{
    char *buf;
    size_t len = 0;

    /* strnlen */
    for ( ; (len < n) && (s[len] != '\0'); ++len) {}

    if (!(buf = malloc(len + 1U))) {
        return NULL;
    }

    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}

#endif

#ifndef HAVE_STRNSTR
char *
strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if ((slen-- < 1) || ((sc = *s++) == '\0')) {
                    return NULL;
                }
            } while (sc != c);
            if (len > slen) {
                return NULL;
            }
        } while (strncmp(s, find, len));
        s--;
    }
    return (char *)s;
}

#endif

#ifndef HAVE_STRCHRNUL
char *
strchrnul(const char *s, int c)
{
    char *p = strchr(s, c);

    return p ? p : (char *)s + strlen(s);
}

#endif

#ifndef HAVE_GET_CURRENT_DIR_NAME
char *
get_current_dir_name(void)
{
    char tmp[_POSIX_PATH_MAX];
    char *retval = NULL;

    if (getcwd(tmp, sizeof(tmp))) {
        retval = strdup(tmp);
        if (!retval) {
            errno = ENOMEM;
        }
    }

    return retval;
}

#endif

#ifndef HAVE_PTHREAD_MUTEX_TIMEDLOCK
int
pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
    int64_t nsec_diff;
    int32_t diff;
    struct timespec cur, dur;
    int rc;

    /* try to acquire the lock and, if we fail, sleep for 5ms. */
    while ((rc = pthread_mutex_trylock(mutex)) == EBUSY) {
        /* get real time */
#ifdef CLOCK_REALTIME
        clock_gettime(CLOCK_REALTIME, &cur);
#else
        struct timeval tv;

        gettimeofday(&tv, NULL);
        cur.tv_sec = (time_t)tv.tv_sec;
        cur.tv_nsec = 1000L * (long)tv.tv_usec;
#endif

        /* get time diff */
        nsec_diff = 0;
        nsec_diff += (((int64_t)abstime->tv_sec) - ((int64_t)cur.tv_sec)) * 1000000000L;
        nsec_diff += ((int64_t)abstime->tv_nsec) - ((int64_t)cur.tv_nsec);
        diff = (nsec_diff ? nsec_diff / 1000000L : 0);

        if (diff < 1) {
            /* timeout */
            break;
        } else if (diff < 5) {
            /* sleep until timeout */
            dur.tv_sec = 0;
            dur.tv_nsec = (long)diff * 1000000;
        } else {
            /* sleep 5 ms */
            dur.tv_sec = 0;
            dur.tv_nsec = 5000000;
        }

        nanosleep(&dur, NULL);
    }

    return rc;
}

#endif
