/**
 * @file boolean.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in boolean type plugin.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesBoolean boolean (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 1 | yes | `int8_t *` | 0 for false, otherwise true |
 */

API LY_ERR
lyplg_type_store_boolean(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    int8_t i;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != 1) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB boolean value size %zu (expected 1).",
                    value_len);
            goto cleanup;
        }

        /* store value */
        i = *(int8_t *)value;
        storage->boolean = i ? 1 : 0;

        /* store canonical value */
        ret = lydict_insert(ctx, i ? "true" : "false", 0, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* validate and store the value */
    if ((value_len == ly_strlen_const("true")) && !strncmp(value, "true", ly_strlen_const("true"))) {
        i = 1;
    } else if ((value_len == ly_strlen_const("false")) && !strncmp(value, "false", ly_strlen_const("false"))) {
        i = 0;
    } else {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid boolean value \"%.*s\".", (int)value_len,
                (char *)value);
        goto cleanup;
    }
    storage->boolean = i;

    /* store canonical value, it always is */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, value, value_len, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

API LY_ERR
lyplg_type_compare_boolean(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->boolean != val2->boolean) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

API const void *
lyplg_type_print_boolean(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = sizeof value->boolean;
        }
        return &value->boolean;
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
 * @brief Plugin information for boolean type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_boolean[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BOOL_STR,

        .plugin.id = "libyang 2 - boolean, version 1",
        .plugin.store = lyplg_type_store_boolean,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_boolean,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_boolean,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
