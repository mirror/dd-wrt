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

#define _GNU_SOURCE /* strdup */

#include "plugins_types.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesEnumeration enumeration (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 4        | yes | `int32 *` | assigned little-endian value of the enum |
 */

LIBYANG_API_DEF LY_ERR
lyplg_type_store_enum(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    struct lysc_type_enum *type_enum = (struct lysc_type_enum *)type;
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool found = 0;
    int64_t num = 0;
    int32_t num_val;

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

        /* convert the value to host byte order */
        memcpy(&num, value, value_len);
        num = le64toh(num);
        num_val = num;

        /* find the matching enumeration value item */
        LY_ARRAY_FOR(type_enum->enums, u) {
            if (type_enum->enums[u].value == num_val) {
                found = 1;
                break;
            }
        }

        if (!found) {
            /* value not found */
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid enumeration value % " PRIi32 ".", num_val);
            goto cleanup;
        }

        /* store value */
        storage->enum_item = &type_enum->enums[u];

        /* canonical settings via dictionary due to free callback */
        ret = lydict_insert(ctx, type_enum->enums[u].name, 0, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);

        /* success */
        goto cleanup;
    }

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

    /* store value */
    storage->enum_item = &type_enum->enums[u];

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
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

LIBYANG_API_DEF int
lyplg_type_sort_enum(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    if (val1->enum_item->value > val2->enum_item->value) {
        return -1;
    } else if (val1->enum_item->value < val2->enum_item->value) {
        return 1;
    } else {
        return 0;
    }
}

LIBYANG_API_DEF const void *
lyplg_type_print_enum(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    int64_t prev_num = 0, num = 0;
    void *buf;

    if (format == LY_VALUE_LYB) {
        prev_num = num = value->enum_item->value;
        num = htole64(num);
        if (num == prev_num) {
            /* values are equal, little-endian */
            *dynamic = 0;
            if (value_len) {
                *value_len = 4;
            }
            return &value->enum_item->value;
        } else {
            /* values differ, big-endian */
            buf = calloc(1, 4);
            LY_CHECK_RET(!buf, NULL);

            *dynamic = 1;
            if (value_len) {
                *value_len = 4;
            }
            memcpy(buf, &num, 4);
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
        .plugin.sort = lyplg_type_sort_enum,
        .plugin.print = lyplg_type_print_enum,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
        .plugin.lyb_data_len = 4,
    },
    {0}
};
