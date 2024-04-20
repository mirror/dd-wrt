/**
 * @file string.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in string type plugin.
 *
 * Copyright (c) 2019 - 2023 CESNET, z.s.p.o.
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
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesString string (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | string itself |
 */

/**
 * @brief Check string value for invalid characters.
 *
 * @param[in] value String to check.
 * @param[in] value_len Length of @p value.
 * @param[out] err Generated error on error.
 * @return LY_ERR value.
 */
static LY_ERR
string_check_chars(const char *value, size_t value_len, struct ly_err_item **err)
{
    size_t len, parsed = 0;

    while (value_len - parsed) {
        if (ly_checkutf8(value + parsed, value_len - parsed, &len)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid character 0x%hhx.", value[parsed]);
        }
        parsed += len;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_store_string(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT UNUSED(format), void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    if (!(options & LYPLG_TYPE_STORE_IS_UTF8)) {
        /* check the UTF-8 encoding */
        ret = string_check_chars(value, value_len, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        /* value_len is in bytes, but we need number of characters here */
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, ly_utf8len(value, value_len), value, value_len, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* pattern restrictions */
    ret = lyplg_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* store canonical value */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
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
 * @brief Plugin information for string type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_string[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_STRING_STR,

        .plugin.id = "libyang 2 - string, version 1",
        .plugin.store = lyplg_type_store_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
