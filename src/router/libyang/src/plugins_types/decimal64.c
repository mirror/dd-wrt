/**
 * @file decimal64.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in decimal64 type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <stdint.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesDecimal64 decimal64 (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 8        | yes | `int64_t *` | little-endian value represented without floating point |
 */

static LY_ERR lyplg_type_validate_decimal64(const struct ly_ctx *UNUSED(ctx), const struct lysc_type *type, const struct lyd_node *UNUSED(ctx_node), const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err);

/**
 * @brief Convert decimal64 number to canonical string.
 *
 * @param[in] num Decimal64 number stored in int64.
 * @param[in] type Decimal64 type with fraction digits.
 * @param[out] str Canonical string value.
 * @return LY_ERR value.
 */
static LY_ERR
decimal64_num2str(int64_t num, struct lysc_type_dec *type, char **str)
{
    char *ret;

    /* allocate the value */
    ret = calloc(1, LY_NUMBER_MAXLEN);
    LY_CHECK_RET(!ret, LY_EMEM);

    if (num) {
        int count = sprintf(ret, "%" PRId64 " ", num);

        if (((num > 0) && ((count - 1) <= type->fraction_digits)) || ((count - 2) <= type->fraction_digits)) {
            /* we have 0. value, print the value with the leading zeros
             * (one for 0. and also keep the correct with of num according
             * to fraction-digits value)
             * for (num < 0) - extra character for '-' sign */
            count = sprintf(ret, "%0*" PRId64 " ", (num > 0) ? (type->fraction_digits + 1) : (type->fraction_digits + 2), num);
        }
        for (uint8_t i = type->fraction_digits, j = 1; i > 0; i--) {
            if (j && (i > 1) && (ret[count - 2] == '0')) {
                /* we have trailing zero to skip */
                ret[count - 1] = '\0';
            } else {
                j = 0;
                ret[count - 1] = ret[count - 2];
            }
            count--;
        }
        ret[count - 1] = '.';
    } else {
        /* zero */
        sprintf(ret, "0.0");
    }

    *str = ret;
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_store_decimal64(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    struct lysc_type_dec *type_dec = (struct lysc_type_dec *)type;
    LY_ERR ret = LY_SUCCESS;
    int64_t num;
    char *canon;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != 8) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB decimal64 value size %zu (expected 8).",
                    value_len);
            goto cleanup;
        }

        /* we have the decimal64 number, in host byte order */
        memcpy(&num, value, value_len);
        num = le64toh(num);
    } else {
        /* check hints */
        ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse decimal64 value */
        ret = lyplg_type_parse_dec64(type_dec->fraction_digits, value, value_len, &num, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* store value */
    storage->dec64 = num;

    /* we need canonical value for the range check */
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
    } else {
        /* generate canonical value */
        ret = decimal64_num2str(num, type_dec, &canon);
        LY_CHECK_GOTO(ret, cleanup);

        /* store it */
        ret = lydict_insert_zc(ctx, canon, (const char **)&storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!(options & LYPLG_TYPE_STORE_ONLY)) {
        /* validate value */
        ret = lyplg_type_validate_decimal64(ctx, type, NULL, NULL, storage, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_validate_clb for the built-in decimal64 type.
 */
static LY_ERR
lyplg_type_validate_decimal64(const struct ly_ctx *UNUSED(ctx), const struct lysc_type *type, const struct lyd_node *UNUSED(ctx_node),
        const struct lyd_node *UNUSED(tree), struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret;
    struct lysc_type_dec *type_dec = (struct lysc_type_dec *)type;
    int64_t num;

    LY_CHECK_ARG_RET(NULL, type, storage, err, LY_EINVAL);
    *err = NULL;
    num = storage->dec64;

    if (type_dec->range) {
        /* check range of the number */
        ret = lyplg_type_validate_range(type->basetype, type_dec->range, num, storage->_canonical,
                strlen(storage->_canonical), err);
        LY_CHECK_RET(ret);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_compare_decimal64(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    /* if type is the same, the fraction digits are, too */
    if (val1->dec64 != val2->dec64) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF int
lyplg_type_sort_decimal64(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->dec64 > val2->dec64) {
        return 1;
    } else if (val1->dec64 < val2->dec64) {
        return -1;
    } else {
        return 0;
    }
}

LIBYANG_API_DEF const void *
lyplg_type_print_decimal64(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    int64_t num = 0;
    void *buf;

    if (format == LY_VALUE_LYB) {
        num = htole64(value->dec64);
        if (num == value->dec64) {
            /* values are equal, little-endian */
            *dynamic = 0;
            if (value_len) {
                *value_len = sizeof value->dec64;
            }
            return &value->dec64;
        } else {
            /* values differ, big-endian */
            buf = calloc(1, sizeof value->dec64);
            LY_CHECK_RET(!buf, NULL);

            *dynamic = 1;
            if (value_len) {
                *value_len = sizeof value->dec64;
            }
            memcpy(buf, &num, sizeof value->dec64);
            return buf;
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
 * @brief Plugin information for decimal64 type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_decimal64[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_DEC64_STR,

        .plugin.id = "libyang 2 - decimal64, version 1",
        .plugin.store = lyplg_type_store_decimal64,
        .plugin.validate = lyplg_type_validate_decimal64,
        .plugin.compare = lyplg_type_compare_decimal64,
        .plugin.sort = lyplg_type_sort_decimal64,
        .plugin.print = lyplg_type_print_decimal64,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
        .plugin.lyb_data_len = 8,
    },
    {0}
};
