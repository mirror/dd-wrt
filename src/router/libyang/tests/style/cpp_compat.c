/**
 * @file cpp_compat.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief test for C++ compatibility of public headers (macros)
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"
#include "plugins_exts.h"
#include "plugins_types.h"

int
main(void)
{
    struct ly_ctx *ctx = NULL;
    const struct lys_module *mod;
    int *sa = NULL, *item;
    struct test_list {
        int val;
        struct test_list *next;
    } *tl = NULL, *tl_item = NULL, *tl_next;
    LY_ARRAY_COUNT_TYPE u;
    const struct lysc_node *scnode;
    struct lyd_node *data = NULL, *next, *elem, *opaq = NULL;
    LY_ERR ret = LY_SUCCESS;

    if ((ret = ly_ctx_new(NULL, 0, &ctx))) {
        goto cleanup;
    }
    if (!(mod = ly_ctx_get_module_latest(ctx, "ietf-yang-library"))) {
        ret = LY_EINT;
        goto cleanup;
    }
    if ((ret = ly_ctx_get_yanglib_data(ctx, &data, "%u", ly_ctx_get_change_count(ctx)))) {
        goto cleanup;
    }
    if ((ret = lyd_new_opaq(NULL, ctx, "name", "val", NULL, "module", &opaq))) {
        goto cleanup;
    }

    /* tree_edit.h / tree.h */
    LY_ARRAY_NEW_GOTO(ctx, sa, item, ret, cleanup);
    LY_ARRAY_NEW_GOTO(ctx, sa, item, ret, cleanup);
    LY_ARRAY_FOR(sa, int, item) {}
    LY_ARRAY_FREE(sa);
    sa = NULL;

    LY_ARRAY_CREATE_GOTO(ctx, sa, 2, ret, cleanup);
    LY_ARRAY_INCREMENT(sa);
    LY_ARRAY_INCREMENT(sa);
    LY_ARRAY_FOR(sa, u) {}
    LY_ARRAY_DECREMENT_FREE(sa);
    LY_ARRAY_DECREMENT_FREE(sa);

    LY_LIST_NEW_GOTO(ctx, &tl, tl_item, next, ret, cleanup);
    LY_LIST_NEW_GOTO(ctx, &tl, tl_item, next, ret, cleanup);
    LY_LIST_FOR(tl, tl_item) {}
    LY_LIST_FOR_SAFE(tl, tl_next, tl_item) {}
    tl_item = tl->next;

    /* tree_data.h */
    LYD_TREE_DFS_BEGIN(data, elem) {
        LYD_TREE_DFS_END(data, elem);
    }
    LYD_LIST_FOR_INST(data, data->schema, elem) {}
    LYD_LIST_FOR_INST_SAFE(data, data->schema, next, elem) {}
    (void)LYD_CTX(data);
    (void)LYD_NAME(data);

    /* tree_schema.h */
    LYSC_TREE_DFS_BEGIN(mod->compiled->data, scnode) {
        LYSC_TREE_DFS_END(mod->compiled->data, scnode);
    }
    (void)LYSP_MODULE_NAME(mod->parsed);
    (void)lysc_is_userordered(data->schema);
    (void)lysc_is_key(data->schema);
    (void)lysc_is_np_cont(data->schema);
    (void)lysc_is_dup_inst_list(data->schema);

cleanup:
    free(tl_item);
    free(tl);
    lyd_free_tree(opaq);
    lyd_free_all(data);
    ly_ctx_destroy(ctx);
    return ret;
}
