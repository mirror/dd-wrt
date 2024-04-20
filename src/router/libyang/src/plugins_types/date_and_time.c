/**
 * @file date_and_time.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types date-and-time type plugin.
 *
 * Copyright (c) 2019-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strdup */

#include "plugins_types.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesDateAndTime date-and-time (ietf-yang-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 8        | yes | `time_t *` | UNIX timestamp |
 * | 1        | no | `int8_t *` | flag whether the value is in the special -00:00 unknown timezone or not |
 * | string length | no | `char *` | string with the fraction digits of a second |
 */

static void lyplg_type_free_date_and_time(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Implementation of ::lyplg_type_store_clb for ietf-yang-types date-and-time type.
 */
static LY_ERR
lyplg_type_store_date_and_time(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_dat = (struct lysc_type_str *)type;
    struct lyd_value_date_and_time *val;
    uint32_t i;
    char c;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len < 8) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB date-and-time value size %zu "
                    "(expected at least 8).", value_len);
            goto cleanup;
        }
        for (i = 9; i < value_len; ++i) {
            c = ((char *)value)[i];
            if (!isdigit(c)) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB date-and-time character '%c' "
                        "(expected a digit).", c);
                goto cleanup;
            }
        }

        /* store timestamp */
        memcpy(&val->time, value, sizeof val->time);

        /* store fractions of second */
        if (value_len > 9) {
            val->fractions_s = strndup(((char *)value) + 9, value_len - 9);
            LY_CHECK_ERR_GOTO(!val->fractions_s, ret = LY_EMEM, cleanup);
        }

        /* store unknown timezone */
        if (value_len > 8) {
            val->unknown_tz = *(((int8_t *)value) + 8) ? 1 : 0;
        }

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* length restriction, there can be only ASCII chars */
    if (type_dat->length) {
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_dat->length, value_len, value, value_len, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* date-and-time pattern */
    ret = lyplg_type_validate_patterns(type_dat->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* convert to UNIX time and fractions of second */
    ret = ly_time_str2time(value, &val->time, &val->fractions_s);
    if (ret) {
        ret = ly_err_new(err, ret, 0, NULL, NULL, "%s", ly_last_errmsg());
        goto cleanup;
    }

    if (!strncmp(((char *)value + value_len) - 6, "-00:00", 6)) {
        /* unknown timezone */
        val->unknown_tz = 1;
    }

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value, value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_date_and_time(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for ietf-yang-types date-and-time type.
 */
static LY_ERR
lyplg_type_compare_date_and_time(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_date_and_time *v1, *v2;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    /* compare timestamp and unknown tz */
    if ((v1->time != v2->time) || (v1->unknown_tz != v2->unknown_tz)) {
        return LY_ENOT;
    }

    /* compare second fractions */
    if ((!v1->fractions_s && !v2->fractions_s) ||
            (v1->fractions_s && v2->fractions_s && !strcmp(v1->fractions_s, v2->fractions_s))) {
        return LY_SUCCESS;
    }
    return LY_ENOT;
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for ietf-yang-types date-and-time type.
 */
static const void *
lyplg_type_print_date_and_time(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_date_and_time *val;
    struct tm tm;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        if (val->unknown_tz || val->fractions_s) {
            ret = malloc(8 + 1 + (val->fractions_s ? strlen(val->fractions_s) : 0));
            LY_CHECK_ERR_RET(!ret, LOGMEM(ctx), NULL);

            *dynamic = 1;
            if (value_len) {
                *value_len = 8 + 1 + (val->fractions_s ? strlen(val->fractions_s) : 0);
            }
            memcpy(ret, &val->time, sizeof val->time);
            memcpy(ret + 8, &val->unknown_tz, sizeof val->unknown_tz);
            if (val->fractions_s) {
                memcpy(ret + 9, val->fractions_s, strlen(val->fractions_s));
            }
        } else {
            *dynamic = 0;
            if (value_len) {
                *value_len = 8;
            }
            ret = (char *)&val->time;
        }
        return ret;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        if (val->unknown_tz) {
            /* ly_time_time2str but always using GMT */
            if (!gmtime_r(&val->time, &tm)) {
                return NULL;
            }
            if (asprintf(&ret, "%04d-%02d-%02dT%02d:%02d:%02d%s%s-00:00",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                    val->fractions_s ? "." : "", val->fractions_s ? val->fractions_s : "") == -1) {
                return NULL;
            }
        } else {
            if (ly_time_time2str(val->time, val->fractions_s, &ret)) {
                return NULL;
            }
        }

        /* store it */
        if (lydict_insert_zc(ctx, ret, (const char **)&value->_canonical)) {
            LOGMEM(ctx);
            return NULL;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = strlen(value->_canonical);
    }
    return value->_canonical;
}

/**
 * @brief Implementation of ::lyplg_type_dup_clb for ietf-yang-types date-and-time type.
 */
static LY_ERR
lyplg_type_dup_date_and_time(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_date_and_time *orig_val, *dup_val;

    memset(dup, 0, sizeof *dup);

    /* optional canonical value */
    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    /* allocate value */
    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

    LYD_VALUE_GET(original, orig_val);

    /* copy timestamp and unknown tz */
    dup_val->time = orig_val->time;
    dup_val->unknown_tz = orig_val->unknown_tz;

    /* duplicate second fractions */
    if (orig_val->fractions_s) {
        dup_val->fractions_s = strdup(orig_val->fractions_s);
        LY_CHECK_ERR_GOTO(!dup_val->fractions_s, ret = LY_EMEM, error);
    }

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_date_and_time(ctx, dup);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_free_clb for ietf-yang-types date-and-time type.
 */
static void
lyplg_type_free_date_and_time(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_date_and_time *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
    LYD_VALUE_GET(value, val);
    if (val) {
        free(val->fractions_s);
        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for date-and-time type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_date_and_time[] = {
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "date-and-time",

        .plugin.id = "libyang 2 - date-and-time, version 1",
        .plugin.store = lyplg_type_store_date_and_time,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_date_and_time,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_date_and_time,
        .plugin.duplicate = lyplg_type_dup_date_and_time,
        .plugin.free = lyplg_type_free_date_and_time,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
