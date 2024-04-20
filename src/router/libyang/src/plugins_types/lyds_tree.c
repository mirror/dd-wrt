/**
 * @file lyds_tree.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Internal type plugin for sorting data nodes.
 *
 * Copyright (c) 2019-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <assert.h> /* assert */
#include <stddef.h> /* NULL */
#include <string.h> /* memset */

#include "compat.h"
#include "libyang.h"
#include "ly_common.h"
#include "tree_data_sorted.h"

static void lyplg_type_free_lyds(const struct ly_ctx *ctx, struct lyd_value *value);

static LY_ERR
lyplg_type_store_lyds(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t UNUSED(value_len), uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data),
        uint32_t UNUSED(hints), const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage,
        struct lys_glob_unres *UNUSED(unres), struct ly_err_item **UNUSED(err))
{
    int ret;
    struct rb_node *rbt = NULL;
    struct lyd_value_lyds_tree *val = NULL;

    /* Prepare value memory. */
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);

    if (format == LY_VALUE_CANON) {
        /* The canonical value for lyds_tree type is the empty string, so @p value is like NULL. */
        memset(storage->fixed_mem, 0, LYD_VALUE_FIXED_MEM_SIZE);
        storage->realtype = type;
        return LY_SUCCESS;
    } else if ((format != LY_VALUE_LYB) || (options & LYPLG_TYPE_STORE_DYNAMIC)) {
        return LY_EVALID;
    }

    /* Create a new Red-black tree. The insertion of additional data nodes should be done via lyds_insert(). */
    ret = lyds_create_node((struct lyd_node *)value, &rbt);
    LY_CHECK_GOTO(ret, cleanup);

    /* Set the root of the Red-black tree. */
    storage->realtype = type;
    val->rbt = rbt;

cleanup:
    if (ret) {
        lyplg_type_free_lyds(ctx, storage);
    }

    return ret;
}

static void
lyplg_type_free_lyds(const struct ly_ctx *UNUSED(ctx), struct lyd_value *value)
{
    struct lyd_value_lyds_tree *val = NULL;

    /* The canonical value is not used at all. */
    assert(!value->_canonical);
    LYD_VALUE_GET(value, val);

    /* Release Red-black tree. */
    lyds_free_tree(val->rbt);
    LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    memset(value->fixed_mem, 0, LYD_VALUE_FIXED_MEM_SIZE);
}

static LY_ERR
lyplg_type_dupl_lyds(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *original, struct lyd_value *dup)
{
    /* The duplicate is not created here, but at the caller, which creates a duplicate lyds tree
     * implicitly by inserting duplicate nodes into the data tree.
     */
    memset(dup, 0, sizeof *dup);
    dup->realtype = original->realtype;

    return LY_SUCCESS;
}

static LY_ERR
lyplg_type_compare_lyds(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *UNUSED(val1),
        const struct lyd_value *UNUSED(val2))
{
    return LY_ENOT;
}

static int
lyplg_type_sort_lyds(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *UNUSED(val1),
        const struct lyd_value *UNUSED(val2))
{
    return 0;
}

static const void *
lyplg_type_print_lyds(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *UNUSED(value),
        LY_VALUE_FORMAT UNUSED(format), void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = 0;
    }

    return "";
}

/**
 * @brief Plugin information for lyds_tree type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_lyds_tree[] = {
    {
        .module = "yang",
        .revision = NULL,
        .name = "lyds_tree",

        .plugin.id = "libyang 2 - lyds_tree, version 1",
        .plugin.store = lyplg_type_store_lyds,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_lyds,
        .plugin.sort = lyplg_type_sort_lyds,
        .plugin.print = lyplg_type_print_lyds,
        .plugin.duplicate = lyplg_type_dupl_lyds,
        .plugin.free = lyplg_type_free_lyds,
        .plugin.lyb_data_len = 0
    },
    {0}
};
