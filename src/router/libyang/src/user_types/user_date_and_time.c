/**
 * @file user_date_and_time.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Example implementation of an date-and-time as a user type, only validation is performed
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
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "../user_types.h"

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

static const char *gmt_offsets[] = {
    "+00:00",
    "+00:20",
    "+00:30",
    "+01:00",
    "+01:24",
    "+01:30",
    "+02:00",
    "+02:30",
    "+03:00",
    "+03:30",
    "+04:00",
    "+04:30",
    "+04:51",
    "+05:00",
    "+05:30",
    "+05:40",
    "+05:45",
    "+06:00",
    "+06:30",
    "+07:00",
    "+07:20",
    "+07:30",
    "+08:00",
    "+08:30",
    "+08:45",
    "+09:00",
    "+09:30",
    "+09:45",
    "+10:00",
    "+10:30",
    "+11:00",
    "+11:30",
    "+12:00",
    "+12:45",
    "+13:00",
    "+13:45",
    "+14:00",
    "-00:00",
    "-00:44",
    "-01:00",
    "-02:00",
    "-02:30",
    "-03:00",
    "-03:30",
    "-04:00",
    "-04:30",
    "-05:00",
    "-06:00",
    "-07:00",
    "-08:00",
    "-08:30",
    "-09:00",
    "-09:30",
    "-10:00",
    "-10:30",
    "-11:00",
    "-12:00",
};

static int
date_and_time_store_clb(const char *UNUSED(type_name), const char *value_str, lyd_val *UNUSED(value), char **err_msg)
{
    struct tm tm, tm2;
    uint32_t i, j, k;
    int ret;

    /* \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[\+\-]\d{2}:\d{2})
     * 2018-03-21T09:11:05(.55785...)(Z|+02:00) */
    memset(&tm, 0, sizeof tm);
    i = 0;

    /* year */
    tm.tm_year = atoi(value_str + i);
    /* if there was some invalid number, it will either be discovered in the loop below or by mktime() */
    tm.tm_year -= 1900;
    for (j = i + 4; i < j; ++i) {
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
    }
    if (value_str[i] != '-') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", '-' expected.", value_str[i], i, value_str);
        goto error;
    }
    ++i;

    /* month */
    tm.tm_mon = atoi(value_str + i);
    tm.tm_mon -= 1;
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
    }
    if (value_str[i] != '-') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", '-' expected.", value_str[i], i, value_str);
        goto error;
    }
    ++i;

    /* day */
    tm.tm_mday = atoi(value_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
    }
    if (value_str[i] != 'T') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", 'T' expected.", value_str[i], i, value_str);
        goto error;
    }
    ++i;

    /* hours */
    tm.tm_hour = atoi(value_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
    }
    if (value_str[i] != ':') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", ':' expected.", value_str[i], i, value_str);
        goto error;
    }
    ++i;

    /* minutes */
    tm.tm_min = atoi(value_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
    }
    if (value_str[i] != ':') {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", ':' expected.", value_str[i], i, value_str);
        goto error;
    }
    ++i;

    /* seconds */
    tm.tm_sec = atoi(value_str + i);
    for (j = i + 2; i < j; ++i) {
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
    }
    if ((value_str[i] != '.') && (value_str[i] != 'Z') && (value_str[i] != '+') && (value_str[i] != '-')) {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", '.', 'Z', '+', or '-' expected.",
                       value_str[i], i, value_str);
        goto error;
    }

    /* validate using mktime() */
    tm2 = tm;
    if (mktime(&tm) == -1) {
        ret = asprintf(err_msg, "Checking date-and-time value \"%s\" failed (%s).", value_str, strerror(errno));
        goto error;
    }
    /* we now have correctly filled the remaining values, use them */
    memcpy(((char *)&tm2) + (6 * sizeof(int)), ((char *)&tm) + (6 * sizeof(int)), sizeof(struct tm) - (6 * sizeof(int)));
    /* back it up again */
    tm = tm2;
    /* let mktime() correct date & time with having the other values correct now */
    if (mktime(&tm) == -1) {
        ret = asprintf(err_msg, "Checking date-and-time value \"%s\" failed (%s).", value_str, strerror(errno));
        goto error;
    }
    /* detect changes in the filled values */
    if (memcmp(&tm, &tm2, 6 * sizeof(int))) {
        ret = asprintf(err_msg, "Checking date-and-time value \"%s\" failed, canonical date and time is \"%04d-%02d-%02dT%02d:%02d:%02d\".",
                       value_str, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        goto error;
    }

    /* tenth of a second */
    if (value_str[i] == '.') {
        ++i;
        if (!isdigit(value_str[i])) {
            ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", a digit expected.", value_str[i], i, value_str);
            goto error;
        }
        do {
            ++i;
        } while (isdigit(value_str[i]));
    }

    switch (value_str[i]) {
    case 'Z':
        /* done */
        break;
    case '+':
    case '-':
        /* timezone shift */
        k = sizeof gmt_offsets / sizeof *gmt_offsets;
        for (j = 0; j < k ; ++j) {
            if (!strncmp(value_str + i, gmt_offsets[j], 6)) {
                break;
            }
        }
        if (j == k) {
            ret = asprintf(err_msg, "Invalid timezone \"%.6s\" in date-and-time value \"%s\".", value_str + i, value_str);
            goto error;
        }
        i += 5;
        break;
    default:
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", 'Z', '+', or '-' expected.", value_str[i], i, value_str);
        goto error;
    }

    /* no other characters expected */
    ++i;
    if (value_str[i]) {
        ret = asprintf(err_msg, "Invalid character '%c'[%d] in date-and-time value \"%s\", no characters expected.", value_str[i], i, value_str);
        goto error;
    }

    /* validation succeeded and we do not want to change how it is stored */
    return 0;

error:
    if (ret == -1) {
        err_msg = NULL;
    }
    return 1;
}

/* Name of this array must match the file name! */
struct lytype_plugin_list user_date_and_time[] = {
    {"ietf-yang-types", "2013-07-15", "date-and-time", date_and_time_store_clb, NULL},
    {NULL, NULL, NULL, NULL, NULL} /* terminating item */
};
