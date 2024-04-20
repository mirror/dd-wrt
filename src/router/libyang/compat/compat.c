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
#ifndef _MSC_VER
#include <sys/time.h>
#endif
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
    static char chunk[256];
    char *ptr;
    ssize_t len, written;

    if (!lineptr || !n) {
        errno = EINVAL;
        return -1;
    }

    if (ferror(stream) || feof(stream)) {
        return -1;
    }

    *n = *lineptr ? *n : 0;
    written = 0;
    while (fgets(chunk, sizeof(chunk), stream)) {
        len = strlen(chunk);
        if ((size_t)(written + len) > *n) {
            ptr = realloc(*lineptr, *n + sizeof(chunk));
            if (!ptr) {
                return -1;
            }
            *lineptr = ptr;
            *n = *n + sizeof(chunk);
        }
        memcpy(*lineptr + written, &chunk, len);
        written += len;
        if ((*lineptr)[written - 1] == '\n') {
            break;
        }
    }
    if (written) {
        (*lineptr)[written] = '\0';
    } else {
        written = -1;
    }

    return written;
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

#ifndef _MSC_VER
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
#endif

#ifndef HAVE_REALPATH
#ifdef _WIN32
char *
realpath(const char *path, char *resolved_path)
{
    char *resolved = _fullpath(resolved_path, path, PATH_MAX);

    if ((_access(resolved, 0) == -1) && (errno == ENOENT)) {
        return NULL;
    }
    return resolved;
}

#elif defined (__NetBSD__)
char *
realpath(const char *path, char *resolved_path)
{
    ssize_t nbytes;

    nbytes = readlink(path, resolved_path, PATH_MAX);
    if (nbytes == -1) {
        return NULL;
    }
    return resolved_path;
}

#else
#error No realpath() implementation for this platform is available.
#endif
#endif

#ifndef HAVE_LOCALTIME_R
#ifdef _WIN32
struct tm *
localtime_r(const time_t *timep, struct tm *result)
{
    errno_t res = localtime_s(result, timep);

    if (res) {
        return NULL;
    } else {
        return result;
    }
}

#else
#error No localtime_r() implementation for this platform is available.
#endif
#endif

#ifndef HAVE_GMTIME_R
#ifdef _WIN32
struct tm *
gmtime_r(const time_t *timep, struct tm *result)
{
    errno_t res = gmtime_s(result, timep);

    if (res) {
        return NULL;
    } else {
        return result;
    }
}

#else
#error No gmtime_r() implementation for this platform is available.
#endif
#endif

#ifndef HAVE_TIMEGM
time_t
timegm(struct tm *tm)
{
    pthread_mutex_t tz_lock = PTHREAD_MUTEX_INITIALIZER;
    time_t ret;
    char *tz;

    pthread_mutex_lock(&tz_lock);

    tz = getenv("TZ");
    if (tz) {
        tz = strdup(tz);
    }
    setenv("TZ", "", 1);
    tzset();

    ret = mktime(tm);

    if (tz) {
        setenv("TZ", tz, 1);
        free(tz);
    } else {
        unsetenv("TZ");
    }
    tzset();

    pthread_mutex_unlock(&tz_lock);

    return ret;
}

#endif

#ifndef HAVE_SETENV
#ifdef _WIN32
int
setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;

    if (!overwrite) {
        size_t envsize = 0;

        errcode = getenv_s(&envsize, NULL, 0, name);
        if (errcode || envsize) {
            return errcode;
        }
    }
    return _putenv_s(name, value);
}

#else
#error No setenv() implementation for this platform is available.
#endif
#endif
