/**
 * @file leafref.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in leafref type plugin.
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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */
#include "tree_data_internal.h" /* lyd_link_leafref_node */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesLeafref leafref (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------: | :-------: | :--: | :-----: |
 * | exact same format as the leafref target ||||
 */

LIBYANG_API_DEF LY_ERR
lyplg_type_store_leafref(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref *)type;

    assert(type_lr->realtype);

    /* store the value as the real type of the leafref target */
    ret = type_lr->realtype->plugin->store(ctx, type_lr->realtype, value, value_len, options, format, prefix_data,
            hints, ctx_node, storage, unres, err);
    if (ret == LY_EINCOMPLETE) {
        /* it is irrelevant whether the target type needs some resolving */
        ret = LY_SUCCESS;
    }
    LY_CHECK_RET(ret);

    if (type_lr->require_instance) {
        /* needs to be resolved */
        return LY_EINCOMPLETE;
    } else {
        return LY_SUCCESS;
    }
}

LIBYANG_API_DEF LY_ERR
lyplg_type_validate_leafref(const struct ly_ctx *ctx, const struct lysc_type *type, const struct lyd_node *ctx_node,
        const struct lyd_node *tree, struct lyd_value *storage, struct ly_err_item **err)
{
    LY_ERR ret;
    struct lysc_type_leafref *type_lr = (struct lysc_type_leafref *)type;
    char *errmsg = NULL, *path;
    struct ly_set *targets = NULL;
    uint32_t i;

    *err = NULL;

    if (!type_lr->require_instance) {
        /* redundant to resolve */
        return LY_SUCCESS;
    }

    ret = lyplg_type_resolve_leafref(type_lr, ctx_node, storage, tree, (ly_ctx_get_options(ctx) & LY_CTX_LEAFREF_LINKING) ? &targets : NULL, &errmsg);
    if (ret != LY_SUCCESS) {
        path = lyd_path(ctx_node, LYD_PATH_STD, NULL, 0);
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, path, strdup("instance-required"), "%s", errmsg);
        free(errmsg);
        goto cleanup;
    }

    if (ly_ctx_get_options(ctx) & LY_CTX_LEAFREF_LINKING) {
        for (i = 0; i < targets->count; ++i) {
            ret = lyd_link_leafref_node((struct lyd_node_term *)targets->dnodes[i], (struct lyd_node_term *)ctx_node);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    ly_set_free(targets, NULL);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_compare_leafref(const struct ly_ctx *ctx, const struct lyd_value *val1, const struct lyd_value *val2)
{
    return val1->realtype->plugin->compare(ctx, val1, val2);
}

LIBYANG_API_DEF int
lyplg_type_sort_leafref(const struct ly_ctx *ctx, const struct lyd_value *val1, const struct lyd_value *val2)
{
    return val1->realtype->plugin->sort(ctx, val1, val2);
}

LIBYANG_API_DEF const void *
lyplg_type_print_leafref(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    return value->realtype->plugin->print(ctx, value, format, prefix_data, dynamic, value_len);
}

LIBYANG_API_DEF LY_ERR
lyplg_type_dup_leafref(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    return original->realtype->plugin->duplicate(ctx, original, dup);
}

LIBYANG_API_DEF void
lyplg_type_free_leafref(const struct ly_ctx *ctx, struct lyd_value *value)
{
    value->realtype->plugin->free(ctx, value);
}

/**
 * @brief Plugin information for leafref type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_leafref[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_LEAFREF_STR,

        .plugin.id = "libyang 2 - leafref, version 1",
        .plugin.store = lyplg_type_store_leafref,
        .plugin.validate = lyplg_type_validate_leafref,
        .plugin.compare = lyplg_type_compare_leafref,
        .plugin.sort = lyplg_type_sort_leafref,
        .plugin.print = lyplg_type_print_leafref,
        .plugin.duplicate = lyplg_type_dup_leafref,
        .plugin.free = lyplg_type_free_leafref,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
