/**
 * @file enumeration.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in enumeration type plugin.
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

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesEnumeration enumeration (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 4        | yes | `int32 *` | assigned value of the enum |
 */

API LY_ERR
lyplg_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    struct lysc_type_enum *type_enum = (struct lysc_type_enum *)type;
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool found = 0;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != 4) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB enumeration value size %zu (expected 4).",
                    value_len);
            goto cleanup;
        }

        /* find the matching enumeration value item */
        LY_ARRAY_FOR(type_enum->enums, u) {
            if (type_enum->enums[u].value == *(int32_t *)value) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* value not found */
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid enumeration value % " PRIi32 ".",
                    *(int32_t *)value);
            goto cleanup;
        }
    } else {
        /* check hints */
        ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
        LY_CHECK_GOTO(ret, cleanup);

        /* find the matching enumeration value item */
        LY_ARRAY_FOR(type_enum->enums, u) {
            if (!ly_strncmp(type_enum->enums[u].name, value, value_len)) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* enum not found */
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid enumeration value \"%.*s\".", (int)value_len,
                    (char *)value);
            goto cleanup;
        }
    }

    /* store value */
    storage->enum_item = &type_enum->enums[u];

    /* store canonical value */
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
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

API const void *
lyplg_type_print_enum(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = 4;
        }
        return &value->enum_item->value;
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
 * @brief Plugin information for enumeration type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_enumeration[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_ENUM_STR,

        .plugin.id = "libyang 2 - enumeration, version 1",
        .plugin.store = lyplg_type_store_enum,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_enum,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
