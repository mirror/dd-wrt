/**
 * @file compat.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief compatibility functions
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _POSIX_C_SOURCE 1 /* fdopen, _POSIX_PATH_MAX */
#define _ISOC99_SOURCE /* vsnprintf */
#define _XOPEN_SOURCE 500 /* strdup */

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "compat.h"

#ifndef HAVE_VDPRINTF
int
vdprintf(int fd, const char *format, va_list ap)
{
    FILE *stream;
    int count;

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

    if (l < 0 || !(*strp = malloc(l + 1U))) {
        return -1;
    }

    return vsnprintf(*strp, l + 1U, fmt, ap);
}
#endif

#ifndef HAVE_STRNDUP
char *
strndup(const char *s, size_t n)
{
    char *buf;
    size_t len = 0;

    /* strnlen */
    for (; (len < n) && (s[len] != '\0'); ++len);

    if (!(buf = malloc(len + 1U))) {
        return NULL;
    }

    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}
#endif

#ifndef HAVE_GETLINE
ssize_t
getline(char **lineptr, size_t *n, FILE *stream)
{
    static char line[256];
    char *ptr;
    unsigned int len;

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
