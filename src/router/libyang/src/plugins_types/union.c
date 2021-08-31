/**
 * @file union.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in union type plugin.
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
#include <sys/cdefs.h>

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
 * @subsection howtoDataLYBTypesUnion union (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 4 | yes | `uint32_t *` | index of the resolved type in ::lysc_type_union.types |
 * | exact same format as the resolved type ||||
 *
 * Note that loading union value in this format prevents it from changing its real (resolved) type.
 */

/**
 * @brief Store subvalue as a specific type.
 *
 * @param[in] ctx libyang context.
 * @param[in] type Specific union type to use for storing.
 * @param[in] subvalue Union subvalue structure.
 * @param[in] resolve Whether the value needs to be resolved (validated by a callback).
 * @param[in] ctx_node Context node for prefix resolution.
 * @param[in] tree Data tree for resolving (validation).
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
union_store_type(const struct ly_ctx *ctx, struct lysc_type *type, struct lyd_value_union *subvalue,
        ly_bool resolve, const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lys_glob_unres *unres,
        struct ly_err_item **err)
{
    LY_ERR ret;
    void *value;
    size_t value_len;

    if (subvalue->format == LY_VALUE_LYB) {
        /* skip the type index */
        value = ((char *)subvalue->original) + 4;
        value_len = subvalue->orig_len - 4;
    } else {
        value = subvalue->original;
        value_len = subvalue->orig_len;
    }

    ret = type->plugin->store(ctx, type, value, value_len, 0, subvalue->format, subvalue->prefix_data, subvalue->hints,
            subvalue->ctx_node, &subvalue->value, unres, err);
    LY_CHECK_RET((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), ret);

    if (resolve && (ret == LY_EINCOMPLETE)) {
        /* we need the value resolved */
        ret = subvalue->value.realtype->plugin->validate(ctx, type, ctx_node, tree, &subvalue->value, err);
        if (ret) {
            /* resolve failed, we need to free the stored value */
            type->plugin->free(ctx, &subvalue->value);
        }
    }

    return ret;
}

/**
 * @brief Find the first valid type for a union value.
 *
 * @param[in] ctx libyang context.
 * @param[in] types Sized array of union types.
 * @param[in] subvalue Union subvalue structure.
 * @param[in] resolve Whether the value needs to be resolved (validated by a callback).
 * @param[in] ctx_node Context node for prefix resolution.
 * @param[in] tree Data tree for resolving (validation).
 * @param[in,out] unres Global unres structure.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
union_find_type(const struct ly_ctx *ctx, struct lysc_type **types, struct lyd_value_union *subvalue,
        ly_bool resolve, const struct lyd_node *ctx_node, const struct lyd_node *tree, struct lys_glob_unres *unres,
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    uint32_t prev_lo;

    if (!types || !LY_ARRAY_COUNT(types)) {
        return LY_EINVAL;
    }

    *err = NULL;

    /* turn logging off */
    prev_lo = ly_log_options(0);

    /* use the first usable subtype to store the value */
    for (u = 0; u < LY_ARRAY_COUNT(types); ++u) {
        ret = union_store_type(ctx, types[u], subvalue, resolve, ctx_node, tree, unres, err);
        if ((ret == LY_SUCCESS) || (ret == LY_EINCOMPLETE)) {
            break;
        }

        ly_err_free(*err);
        *err = NULL;
    }

    if (u == LY_ARRAY_COUNT(types)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid union value \"%.*s\" - no matching subtype found.",
                (int)subvalue->orig_len, (char *)subvalue->original);
    }

    /* restore logging */
    ly_log_options(prev_lo);
    return ret;
}

API LY_ERR
lyplg_type_store_union(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)type;
    struct lyd_value_union *subvalue;
    uint32_t type_idx;

    *err = NULL;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, subvalue);
    LY_CHECK_ERR_GOTO(!subvalue, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* basic validation */
        if (value_len < 4) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB union value size %zu (expected at least 4).",
                    value_len);
            goto cleanup;
        }
        type_idx = *(uint32_t *)value;
        if (type_idx >= LY_ARRAY_COUNT(type_u->types)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB union type index %" PRIu32
                    " (type count " LY_PRI_ARRAY_COUNT_TYPE ").", type_idx, LY_ARRAY_COUNT(type_u->types));
            goto cleanup;
        }
    }

    /* remember the original value */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        subvalue->original = (void *)value;
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
    } else if (value_len) {
        subvalue->original = calloc(1, value_len);
        LY_CHECK_ERR_GOTO(!subvalue->original, ret = LY_EMEM, cleanup);
        memcpy(subvalue->original, value, value_len);
    } else {
        subvalue->original = strdup("");
        LY_CHECK_ERR_GOTO(!subvalue->original, ret = LY_EMEM, cleanup);
    }
    subvalue->orig_len = value_len;

    /* store format-specific data for later prefix resolution */
    ret = lyplg_type_prefix_data_new(ctx, value, value_len, format, prefix_data, &subvalue->format,
            &subvalue->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);
    subvalue->hints = hints;
    subvalue->ctx_node = ctx_node;

    if (format == LY_VALUE_LYB) {
        /* use the specific type to store the value */
        ret = union_store_type(ctx, type_u->types[type_idx], subvalue, 0, NULL, NULL, unres, err);
        LY_CHECK_GOTO((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), cleanup);
    } else {
        /* use the first usable subtype to store the value */
        ret = union_find_type(ctx, type_u->types, subvalue, 0, NULL, NULL, unres, err);
        LY_CHECK_GOTO((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE), cleanup);
    }

    /* store canonical value, if any (use the specific type value) */
    ret = lydict_insert(ctx, subvalue->value._canonical, 0, &storage->_canonical);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if ((ret != LY_SUCCESS) && (ret != LY_EINCOMPLETE)) {
        lyplg_type_free_union(ctx, storage);
    }
    return ret;
}

API LY_ERR
lyplg_type_validate_union(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_union *type_u = (struct lysc_type_union *)storage->realtype;
    struct lyd_value_union *val = storage->subvalue;

    *err = NULL;

    if (!val->value.realtype->plugin->validate) {
        /* nothing to resolve */
        return LY_SUCCESS;
    }

    /* resolve the stored value */
    if (!val->value.realtype->plugin->validate(ctx, type, ctx_node, tree, &val->value, err)) {
        /* resolve successful */
        return LY_SUCCESS;
    }

    /* Resolve failed, we have to try another subtype of the union.
     * Unfortunately, since the realtype can change (e.g. in leafref), we are not able to detect
     * which of the subtype's were tried the last time, so we have to try all of them again.
     */
    ly_err_free(*err);
    *err = NULL;

    if (val->format == LY_VALUE_LYB) {
        /* use the specific type to store the value */
        uint32_t type_idx = *(uint32_t *)val->original;
        ret = union_store_type(ctx, type_u->types[type_idx], val, 1, ctx_node, tree, NULL, err);
        LY_CHECK_RET(ret);
    } else {
        /* use the first usable subtype to store the value */
        ret = union_find_type(ctx, type_u->types, val, 1, ctx_node, tree, NULL, err);
        LY_CHECK_RET(ret);
    }

    /* store and resolve the value */
    ret = union_find_type(ctx, type_u->types, val, 1, ctx_node, tree, NULL, err);
    LY_CHECK_RET(ret);

    /* success, update the canonical value, if any generated */
    lydict_remove(ctx, storage->_canonical);
    LY_CHECK_RET(lydict_insert(ctx, val->value._canonical, 0, &storage->_canonical));
    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_compare_union(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->subvalue->value.realtype != val2->subvalue->value.realtype) {
        return LY_ENOT;
    }
    return val1->subvalue->value.realtype->plugin->compare(&val1->subvalue->value, &val2->subvalue->value);
}

API const void *
lyplg_type_print_union(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    const void *ret;

    ret = value->subvalue->value.realtype->plugin->print(ctx, &value->subvalue->value, format, prefix_data, dynamic, value_len);
    if (!value->_canonical && (format == LY_VALUE_CANON)) {
        /* the canonical value is supposed to be stored now */
        lydict_insert(ctx, value->subvalue->value._canonical, 0, (const char **)&value->_canonical);
    }

    return ret;
}

API LY_ERR
lyplg_type_dup_union(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_union *orig_val = original->subvalue, *dup_val;

    /* init dup value */
    memset(dup, 0, sizeof *dup);
    dup->realtype = original->realtype;

    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_GOTO(ret, cleanup);

    dup_val = calloc(1, sizeof *dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    dup->subvalue = dup_val;

    ret = orig_val->value.realtype->plugin->duplicate(ctx, &orig_val->value, &dup_val->value);
    LY_CHECK_GOTO(ret, cleanup);

    if (orig_val->orig_len) {
        dup_val->original = calloc(1, orig_val->orig_len);
        LY_CHECK_ERR_GOTO(!dup_val->original, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        memcpy(dup_val->original, orig_val->original, orig_val->orig_len);
    } else {
        dup_val->original = strdup("");
        LY_CHECK_ERR_GOTO(!dup_val->original, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    }
    dup_val->orig_len = orig_val->orig_len;

    dup_val->format = orig_val->format;
    ret = lyplg_type_prefix_data_dup(ctx, orig_val->format, orig_val->prefix_data, &dup_val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (ret) {
        lyplg_type_free_union(ctx, dup);
    }
    return ret;
}

API void
lyplg_type_free_union(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_union *val;

    lydict_remove(ctx, value->_canonical);
    LYD_VALUE_GET(value, val);
    if (val) {
        if (val->value.realtype) {
            val->value.realtype->plugin->free(ctx, &val->value);
        }
        lyplg_type_prefix_data_free(val->format, val->prefix_data);
        free(val->original);

        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for union type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_union[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_UNION_STR,

        .plugin.id = "libyang 2 - union,version 1",
        .plugin.store = lyplg_type_store_union,
        .plugin.validate = lyplg_type_validate_union,
        .plugin.compare = lyplg_type_compare_union,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_union,
        .plugin.duplicate = lyplg_type_dup_union,
        .plugin.free = lyplg_type_free_union
    },
    {0}
};
