/**
 * @file user_yang_types.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types typedef validation and conversion to canonical format
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "compat.h"
#include "../user_types.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 */
LYTYPE_VERSION_CHECK

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

static int
date_and_time_store_clb(struct ly_ctx *UNUSED(ctx), const char *UNUSED(type_name), const char **value_str,
                        lyd_val *UNUSED(value), char **err_msg)
{
    struct tm tm, tm2;
    uint32_t i, j;
    const char *val_str = *value_str;
    int ret;

    /* \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[\+\-]\d{2}:\d{2})
     * 2018-03-21T09:11:05(.55785...)(Z|+02:00) */
    memset(&tm, 0, sizeof tm);
    i = 0;

    /* year */
    tm.tm_year = atoi(val_str + i);
    /* if there was some invalid number, it will either be discovered in the loop below or by mktime() */
    tm.tm_year -= 1900;
    for (j = i + 4; i < j; ++i) {
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
    }
    if (val_str[i] != '-') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", '-' expected.", val_str[i], i, val_str);
        goto error;
    }
    ++i;

    /* month */
    tm.tm_mon = atoi(val_str + i);
    tm.tm_mon -= 1;
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
    }
    if (val_str[i] != '-') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", '-' expected.", val_str[i], i, val_str);
        goto error;
    }
    ++i;

    /* day */
    tm.tm_mday = atoi(val_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
    }
    if (val_str[i] != 'T') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", 'T' expected.", val_str[i], i, val_str);
        goto error;
    }
    ++i;

    /* hours */
    tm.tm_hour = atoi(val_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
    }
    if (val_str[i] != ':') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", ':' expected.", val_str[i], i, val_str);
        goto error;
    }
    ++i;

    /* minutes */
    tm.tm_min = atoi(val_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
    }
    if (val_str[i] != ':') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", ':' expected.", val_str[i], i, val_str);
        goto error;
    }
    ++i;

    /* seconds */
    tm.tm_sec = atoi(val_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
    }
    if ((val_str[i] != '.') && (val_str[i] != 'Z') && (val_str[i] != '+') && (val_str[i] != '-')) {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", '.', 'Z', '+', or '-' expected.",
                       val_str[i], i, val_str);
        goto error;
    }

    /* validate using mktime() */
    tm2 = tm;
    errno = 0;
    mktime(&tm);
    /* ENOENT is set when "/etc/localtime" is missing but the function suceeeds */
    if (errno && (errno != ENOENT)) {
        ret = asprintf(err_msg, "Checking date-and-time value \"%s\" failed (%s).", val_str, strerror(errno));
        goto error;
    }
    /* we now have correctly filled the remaining values, use them */
    memcpy(((char *)&tm2) + (6 * sizeof(int)), ((char *)&tm) + (6 * sizeof(int)), sizeof(struct tm) - (6 * sizeof(int)));
    /* back it up again */
    tm = tm2;
    /* let mktime() correct date & time with having the other values correct now */
    errno = 0;
    mktime(&tm);
    if (errno && (errno != ENOENT)) {
        ret = asprintf(err_msg, "Checking date-and-time value \"%s\" failed (%s).", val_str, strerror(errno));
        goto error;
    }
    /* detect changes in the filled values */
    if (memcmp(&tm, &tm2, 6 * sizeof(int))) {
        ret = asprintf(err_msg, "Checking date-and-time value \"%s\" failed, canonical date and time is \"%04d-%02d-%02dT%02d:%02d:%02d\".",
                       val_str, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        goto error;
    }

    /* tenth of a second */
    if (val_str[i] == '.') {
        ++i;
        if (!isdigit(val_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", val_str[i], i, val_str);
            goto error;
        }
        do {
            ++i;
        } while (isdigit(val_str[i]));
    }

    switch (val_str[i]) {
    case 'Z':
        /* done */
        break;
    case '+':
    case '-':
        /* timezone shift */
        if ((val_str[i + 1] < '0') || (val_str[i + 1] > '2')) {
            ret = asprintf(err_msg, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".", val_str + i, val_str);
            goto error;
        }
        if ((val_str[i + 2] < '0') || ((val_str[i + 1] == '2') && (val_str[i + 2] > '3')) || (val_str[i + 2] > '9')) {
            ret = asprintf(err_msg, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".", val_str + i, val_str);
            goto error;
        }

        if (val_str[i + 3] != ':') {
            ret = asprintf(err_msg, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".", val_str + i, val_str);
            goto error;
        }

        if ((val_str[i + 4] < '0') || (val_str[i + 4] > '5')) {
            ret = asprintf(err_msg, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".", val_str + i, val_str);
            goto error;
        }
        if ((val_str[i + 5] < '0') || (val_str[i + 5] > '9')) {
            ret = asprintf(err_msg, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".", val_str + i, val_str);
            goto error;
        }

        i += 5;
        break;
    default:
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", 'Z', '+', or '-' expected.", val_str[i], i, val_str);
        goto error;
    }

    /* no other characters expected */
    ++i;
    if (val_str[i]) {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", no characters expected.", val_str[i], i, val_str);
        goto error;
    }

    /* validation succeeded and we do not want to change how it is stored */
    return 0;

error:
    if (ret == -1) {
        *err_msg = NULL;
    }
    return 1;
}

static int
hex_string_store_clb(struct ly_ctx *ctx, const char *UNUSED(type_name), const char **value_str, lyd_val *value, char **err_msg)
{
    char *str;
    uint32_t i, len;

    str = strdup(*value_str);
    if (!str) {
        /* we can hardly allocate an error message */
        *err_msg = NULL;
        return 1;
    }

    len = strlen(str);
    for (i = 0; i < len; ++i) {
        if ((str[i] >= 'A') && (str[i] <= 'Z')) {
            /* make it lowercase (canonical format) */
            str[i] += 32;
        }
    }

    /* update the value correctly */
    lydict_remove(ctx, *value_str);
    *value_str = lydict_insert_zc(ctx, str);
    value->string = *value_str;
    return 0;
}

/* Name of this array must match the file name! */
struct lytype_plugin_list user_yang_types[] = {
    {"ietf-yang-types", "2013-07-15", "date-and-time", date_and_time_store_clb, NULL},
    {"ietf-yang-types", "2013-07-15", "phys-address", hex_string_store_clb, NULL},
    {"ietf-yang-types", "2013-07-15", "mac-address", hex_string_store_clb, NULL},
    {"ietf-yang-types", "2013-07-15", "hex-string", hex_string_store_clb, NULL},
    {"ietf-yang-types", "2013-07-15", "uuid", hex_string_store_clb, NULL},
    {NULL, NULL, NULL, NULL, NULL} /* terminating item */
};
