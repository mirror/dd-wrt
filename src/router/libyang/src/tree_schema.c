/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema tree implementation
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "tree_schema.h"

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_internal.h"
#include "parser_schema.h"
#include "path.h"
#include "plugins_exts.h"
#include "plugins_internal.h"
#include "schema_compile.h"
#include "schema_compile_amend.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema_free.h"
#include "tree_schema_internal.h"
#include "xpath.h"

const char * const ly_devmod_list[] = {
    [LYS_DEV_NOT_SUPPORTED] = "not-supported",
    [LYS_DEV_ADD] = "add",
    [LYS_DEV_DELETE] = "delete",
    [LYS_DEV_REPLACE] = "replace",
};

LIBYANG_API_DEF LY_ERR
lysc_tree_dfs_full(const struct lysc_node *root, lysc_dfs_clb dfs_clb, void *data)
{
    struct lysc_node *elem, *elem2;
    const struct lysc_node_action *action;
    const struct lysc_node_notif *notif;

    LY_CHECK_ARG_RET(NULL, root, dfs_clb, LY_EINVAL);

    LYSC_TREE_DFS_BEGIN(root, elem) {
        /* schema node */
        LY_CHECK_RET(dfs_clb(elem, data, &LYSC_TREE_DFS_continue));

        LY_LIST_FOR(lysc_node_actions(elem), action) {
            LYSC_TREE_DFS_BEGIN(action, elem2) {
                /* action subtree */
                LY_CHECK_RET(dfs_clb(elem2, data, &LYSC_TREE_DFS_continue));

                LYSC_TREE_DFS_END(action, elem2);
            }
        }

        LY_LIST_FOR(lysc_node_notifs(elem), notif) {
            LYSC_TREE_DFS_BEGIN(notif, elem2) {
                /* notification subtree */
                LY_CHECK_RET(dfs_clb(elem2, data, &LYSC_TREE_DFS_continue));

                LYSC_TREE_DFS_END(notif, elem2);
            }
        }

        LYSC_TREE_DFS_END(root, elem);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lysc_module_dfs_full(const struct lys_module *mod, lysc_dfs_clb dfs_clb, void *data)
{
    const struct lysc_node *root;

    LY_CHECK_ARG_RET(NULL, mod, mod->compiled, dfs_clb, LY_EINVAL);

    /* schema nodes */
    LY_LIST_FOR(mod->compiled->data, root) {
        LY_CHECK_RET(lysc_tree_dfs_full(root, dfs_clb, data));
    }

    /* RPCs */
    LY_LIST_FOR((const struct lysc_node *)mod->compiled->rpcs, root) {
        LY_CHECK_RET(lysc_tree_dfs_full(root, dfs_clb, data));
    }

    /* notifications */
    LY_LIST_FOR((const struct lysc_node *)mod->compiled->notifs, root) {
        LY_CHECK_RET(lysc_tree_dfs_full(root, dfs_clb, data));
    }

    return LY_SUCCESS;
}

static void
lys_getnext_into_case(const struct lysc_node_case *first_case, const struct lysc_node **last, const struct lysc_node **next)
{
    for ( ; first_case; first_case = (const struct lysc_node_case *)first_case->next) {
        if (first_case->child) {
            /* there is something to return */
            (*next) = first_case->child;
            return;
        }
    }

    /* no children in choice's cases, so go to the choice's sibling instead of into it */
    (*last) = (*next);
    (*next) = (*next)->next;
}

/**
 * @brief Generic getnext function for ::lys_getnext() and ::lys_getnext_ext().
 *
 * Gets next schema tree (sibling) node element that can be instantiated in a data tree. Returned node can
 * be from an augment. If the @p ext is provided, the function is locked inside the schema tree defined in the
 * extension instance.
 *
 * ::lys_getnext_() is supposed to be called sequentially. In the first call, the @p last parameter is usually NULL
 * and function starts returning i) the first @p parent's child or ii) the first top level element specified in the
 * given extension (if provided) or iii) the first top level element of the @p module.
 * Consequent calls suppose to provide the previously returned node as the @p last parameter and still the same
 * @p parent and @p module parameters.
 *
 * Without options, the function is used to traverse only the schema nodes that can be paired with corresponding
 * data nodes in a data tree. By setting some @p options the behavior can be modified to the extent that
 * all the schema nodes are iteratively returned.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree where the function starts processing.
 * @param[in] module In case of iterating on top level elements, the @p parent is NULL and
 * module must be specified.
 * @param[in] ext The extension instance to provide a separate schema tree. To consider the top level elements in the tree,
 * the @p parent must be NULL. Anyway, at least one of @p parent, @p module and @p ext parameters must be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Next schema tree node that can be instantiated in a data tree, NULL in case there is no such element.
 */
static const struct lysc_node *
lys_getnext_(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_module *module,
        const struct lysc_ext_instance *ext, uint32_t options)
{
    const struct lysc_node *next = NULL;
    ly_bool action_flag = 0, notif_flag = 0, sm_flag = options & LYS_GETNEXT_WITHSCHEMAMOUNT ? 0 : 1;
    LY_ARRAY_COUNT_TYPE u;
    struct ly_ctx *sm_ctx = NULL;
    const struct lys_module *mod;
    uint32_t idx;

    LY_CHECK_ARG_RET(NULL, parent || module || ext, NULL);

next:
    if (!last) {
        /* first call */

        /* learn where to start */
        if (parent) {
            /* schema subtree */
            next = last = lysc_node_child(parent);
        } else {
            /* top level data */
            if (ext) {
                lyplg_ext_get_storage(ext, LY_STMT_DATA_NODE_MASK, sizeof last, (const void **)&last);
                next = last;
            } else {
                next = last = module->data;
            }
        }
        if (!next) {
            /* try to get action or notification */
            goto repeat;
        }
        /* test if the next can be returned */
        goto check;

    } else if (last->nodetype & (LYS_RPC | LYS_ACTION)) {
        action_flag = 1;
        next = last->next;
    } else if (last->nodetype == LYS_NOTIF) {
        action_flag = notif_flag = 1;
        next = last->next;
    } else {
        next = last->next;
    }

repeat:
    if (!next) {
        if (last && !sm_flag && parent && (last->module->ctx != parent->module->ctx)) {
            sm_flag = 1;

            /* find the module of last */
            sm_ctx = last->module->ctx;
            idx = 0;
            while ((mod = ly_ctx_get_module_iter(sm_ctx, &idx))) {
                if (mod == last->module) {
                    break;
                }
            }
            assert(mod);

            /* get node from the next mounted module */
            while (!next && (mod = ly_ctx_get_module_iter(sm_ctx, &idx))) {
                if (!mod->implemented) {
                    continue;
                }

                next = lys_getnext(NULL, NULL, mod->compiled, options & ~LYS_GETNEXT_WITHSCHEMAMOUNT);
            }
        } else if (last && (last->parent != parent)) {
            /* go back to parent */
            last = last->parent;
            goto next;
        } else if (!action_flag) {
            action_flag = 1;
            if (ext) {
                lyplg_ext_get_storage(ext, LY_STMT_OP_MASK, sizeof next, (const void **)&next);
            } else if (parent) {
                next = (struct lysc_node *)lysc_node_actions(parent);
            } else {
                next = (struct lysc_node *)module->rpcs;
            }
        } else if (!notif_flag) {
            notif_flag = 1;
            if (ext) {
                lyplg_ext_get_storage(ext, LY_STMT_NOTIFICATION, sizeof next, (const void **)&next);
            } else if (parent) {
                next = (struct lysc_node *)lysc_node_notifs(parent);
            } else {
                next = (struct lysc_node *)module->notifs;
            }
        } else if (!sm_flag) {
            sm_flag = 1;
            if (parent) {
                LY_ARRAY_FOR(parent->exts, u) {
                    if (!strcmp(parent->exts[u].def->name, "mount-point") &&
                            !strcmp(parent->exts[u].def->module->name, "ietf-yang-schema-mount")) {
                        lyplg_ext_schema_mount_create_context(&parent->exts[u], &sm_ctx);
                        if (sm_ctx) {
                            /* some usable context created */
                            break;
                        }
                    }
                }
                if (sm_ctx) {
                    /* get the first node from the first usable module */
                    idx = 0;
                    while (!next && (mod = ly_ctx_get_module_iter(sm_ctx, &idx))) {
                        if (!mod->implemented) {
                            continue;
                        }

                        next = lys_getnext(NULL, NULL, mod->compiled, options & ~LYS_GETNEXT_WITHSCHEMAMOUNT);
                    }
                    if (!next) {
                        /* no nodes found */
                        ly_ctx_destroy(sm_ctx);
                    }
                }
            }
        } else {
            return NULL;
        }
        goto repeat;
    }
check:
    switch (next->nodetype) {
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_LIST:
    case LYS_LEAFLIST:
        break;
    case LYS_CASE:
        if (options & LYS_GETNEXT_WITHCASE) {
            break;
        } else {
            /* go into */
            lys_getnext_into_case((const struct lysc_node_case *)next, &last, &next);
        }
        goto repeat;
    case LYS_CONTAINER:
        if (!(next->flags & LYS_PRESENCE) && (options & LYS_GETNEXT_INTONPCONT)) {
            if (lysc_node_child(next)) {
                /* go into */
                next = lysc_node_child(next);
            } else {
                last = next;
                next = next->next;
            }
            goto repeat;
        }
        break;
    case LYS_CHOICE:
        if (options & LYS_GETNEXT_WITHCHOICE) {
            break;
        } else if ((options & LYS_GETNEXT_NOCHOICE) || !lysc_node_child(next)) {
            next = next->next;
        } else {
            if (options & LYS_GETNEXT_WITHCASE) {
                next = lysc_node_child(next);
            } else {
                /* go into */
                lys_getnext_into_case(((struct lysc_node_choice *)next)->cases, &last, &next);
            }
        }
        goto repeat;
    case LYS_INPUT:
        if (options & LYS_GETNEXT_OUTPUT) {
            /* skip */
            next = next->next;
        } else {
            /* go into */
            next = lysc_node_child(next);
        }
        goto repeat;
    case LYS_OUTPUT:
        if (!(options & LYS_GETNEXT_OUTPUT)) {
            /* skip */
            next = next->next;
        } else {
            /* go into */
            next = lysc_node_child(next);
        }
        goto repeat;
    default:
        /* we should not be here */
        LOGINT(module ? module->mod->ctx : parent ? parent->module->ctx : ext->module->ctx);
        return NULL;
    }

    return next;
}

LIBYANG_API_DEF const struct lysc_node *
lys_getnext(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_module *module, uint32_t options)
{
    return lys_getnext_(last, parent, module, NULL, options);
}

LIBYANG_API_DEF const struct lysc_node *
lys_getnext_ext(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_ext_instance *ext, uint32_t options)
{
    return lys_getnext_(last, parent, NULL, ext, options);
}

const struct lysc_node *
lysc_ext_find_node(const struct lysc_ext_instance *ext, const struct lys_module *module, const char *name, size_t name_len,
        uint16_t nodetype, uint32_t options)
{
    const struct lysc_node *node = NULL;

    LY_CHECK_ARG_RET(NULL, ext, name, NULL);
    if (!nodetype) {
        nodetype = LYS_NODETYPE_MASK;
    }

    if (module && (module != ext->module)) {
        return NULL;
    }

    while ((node = lys_getnext_ext(node, NULL, ext, options))) {
        if (!(node->nodetype & nodetype)) {
            continue;
        }

        if (name_len) {
            if (!ly_strncmp(node->name, name, name_len)) {
                return node;
            }
        } else {
            if (!strcmp(node->name, name)) {
                return node;
            }
        }
    }
    return NULL;
}

LIBYANG_API_DEF const struct lysc_node *
lys_find_child(const struct lysc_node *parent, const struct lys_module *module, const char *name, size_t name_len,
        uint16_t nodetype, uint32_t options)
{
    const struct lysc_node *node = NULL;

    LY_CHECK_ARG_RET(NULL, module, name, NULL);
    LY_CHECK_CTX_EQUAL_RET(parent ? parent->module->ctx : NULL, module->ctx, NULL);
    if (!nodetype) {
        nodetype = LYS_NODETYPE_MASK;
    }

    while ((node = lys_getnext(node, parent, module->compiled, options))) {
        if (!(node->nodetype & nodetype)) {
            continue;
        }
        if (node->module != module) {
            continue;
        }

        if (name_len) {
            if (!ly_strncmp(node->name, name, name_len)) {
                return node;
            }
        } else {
            if (!strcmp(node->name, name)) {
                return node;
            }
        }
    }

    return NULL;
}

LIBYANG_API_DEF LY_ERR
lys_find_xpath_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *xpath, uint32_t options,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx || ctx_node, xpath, set, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, ctx_node ? ctx_node->module->ctx : NULL, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options |= LYXP_SCNODE;
    }
    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile expression */
    ret = lyxp_expr_parse(ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(ctx, exp, NULL, LY_VALUE_JSON, NULL, ctx_node, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
    LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    (*set)->size = xp_set.used;

    for (i = 0; i < xp_set.used; ++i) {
        if (xp_set.val.scnodes[i].type == LYXP_NODE_ELEM) {
            ret = ly_set_add(*set, xp_set.val.scnodes[i].scnode, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free(ctx, exp);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_find_expr_atoms(const struct lysc_node *ctx_node, const struct lys_module *cur_mod, const struct lyxp_expr *expr,
        const struct lysc_prefix *prefixes, uint32_t options, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, cur_mod, expr, prefixes, set, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx_node ? ctx_node->module->ctx : NULL, cur_mod->ctx, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(cur_mod->ctx, expr, cur_mod, LY_VALUE_SCHEMA_RESOLVED, (void *)prefixes, ctx_node, ctx_node,
            &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
    LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(cur_mod->ctx); ret = LY_EMEM, cleanup);
    (*set)->size = xp_set.used;

    for (i = 0; i < xp_set.used; ++i) {
        if ((xp_set.val.scnodes[i].type == LYXP_NODE_ELEM) && (xp_set.val.scnodes[i].in_ctx >= LYXP_SET_SCNODE_ATOM_NODE)) {
            assert((xp_set.val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_NODE) ||
                    (xp_set.val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_VAL) ||
                    (xp_set.val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX));
            ret = ly_set_add(*set, xp_set.val.scnodes[i].scnode, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_find_xpath(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *xpath, uint32_t options,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx || ctx_node, xpath, set, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, ctx_node ? ctx_node->module->ctx : NULL, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }
    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* allocate return set */
    ret = ly_set_new(set);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile expression */
    ret = lyxp_expr_parse(ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(ctx, exp, NULL, LY_VALUE_JSON, NULL, ctx_node, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* transform into ly_set */
    (*set)->objs = malloc(xp_set.used * sizeof *(*set)->objs);
    LY_CHECK_ERR_GOTO(!(*set)->objs, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    (*set)->size = xp_set.used;

    for (i = 0; i < xp_set.used; ++i) {
        if ((xp_set.val.scnodes[i].type == LYXP_NODE_ELEM) && (xp_set.val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX)) {
            ret = ly_set_add(*set, xp_set.val.scnodes[i].scnode, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&xp_set);
    lyxp_expr_free(ctx, exp);
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_find_lypath_atoms(const struct ly_path *path, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;

    LY_CHECK_ARG_RET(NULL, path, set, LY_EINVAL);

    /* allocate return set */
    LY_CHECK_RET(ly_set_new(set));

    LY_ARRAY_FOR(path, u) {
        /* add nodes from the path */
        LY_CHECK_GOTO(ret = ly_set_add(*set, (void *)path[u].node, 0, NULL), cleanup);
        LY_ARRAY_FOR(path[u].predicates, v) {
            if ((path[u].predicates[v].type == LY_PATH_PREDTYPE_LIST) || (path[u].predicates[v].type == LY_PATH_PREDTYPE_LIST_VAR)) {
                /* add all the keys in a predicate */
                LY_CHECK_GOTO(ret = ly_set_add(*set, (void *)path[u].predicates[v].key, 0, NULL), cleanup);
            }
        }
    }

cleanup:
    if (ret) {
        ly_set_free(*set, NULL);
        *set = NULL;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_find_path_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t oper;
    struct lyxp_expr *expr = NULL;
    struct ly_path *p = NULL;

    LY_CHECK_ARG_RET(ctx, ctx || ctx_node, path, set, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(ctx, ctx_node ? ctx_node->module->ctx : NULL, LY_EINVAL);

    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* parse */
    ret = ly_path_parse(ctx, ctx_node, path, strlen(path), 0, LY_PATH_BEGIN_EITHER, LY_PATH_PREFIX_FIRST,
            LY_PATH_PRED_SIMPLE, &expr);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile */
    oper = output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, expr, oper, LY_PATH_TARGET_MANY, 0, LY_VALUE_JSON, NULL, &p);
    LY_CHECK_GOTO(ret, cleanup);

    /* resolve */
    ret = lys_find_lypath_atoms(p, set);

cleanup:
    ly_path_free(ctx, p);
    lyxp_expr_free(ctx, expr);
    return ret;
}

LIBYANG_API_DEF const struct lysc_node *
lys_find_path(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output)
{
    const struct lysc_node *snode = NULL;
    struct lyxp_expr *expr = NULL;
    struct ly_path *p = NULL;
    LY_ERR ret;
    uint8_t oper;

    LY_CHECK_ARG_RET(ctx, ctx || ctx_node, NULL);
    LY_CHECK_CTX_EQUAL_RET(ctx, ctx_node ? ctx_node->module->ctx : NULL, NULL);

    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* parse */
    ret = ly_path_parse(ctx, ctx_node, path, strlen(path), 0, LY_PATH_BEGIN_EITHER, LY_PATH_PREFIX_FIRST,
            LY_PATH_PRED_SIMPLE, &expr);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile */
    oper = output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, expr, oper, LY_PATH_TARGET_MANY, 0, LY_VALUE_JSON, NULL, &p);
    LY_CHECK_GOTO(ret, cleanup);

    /* get last node */
    snode = p[LY_ARRAY_COUNT(p) - 1].node;

cleanup:
    ly_path_free(ctx, p);
    lyxp_expr_free(ctx, expr);
    return snode;
}

char *
lysc_path_until(const struct lysc_node *node, const struct lysc_node *parent, LYSC_PATH_TYPE pathtype, char *buffer,
        size_t buflen)
{
    const struct lysc_node *iter, *par, *key;
    char *path = NULL;
    int len = 0;
    ly_bool skip_schema;

    if (buffer) {
        LY_CHECK_ARG_RET(node->module->ctx, buflen > 1, NULL);
        buffer[0] = '\0';
    }

    if ((pathtype == LYSC_PATH_DATA) || (pathtype == LYSC_PATH_DATA_PATTERN)) {
        /* skip schema-only nodes */
        skip_schema = 1;
    } else {
        skip_schema = 0;
    }

    for (iter = node; iter && (iter != parent) && (len >= 0); iter = iter->parent) {
        char *s;
        const char *slash;

        if (skip_schema && (iter->nodetype & (LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT))) {
            /* schema-only node */
            continue;
        }

        if ((pathtype == LYSC_PATH_DATA_PATTERN) && (iter->nodetype == LYS_LIST)) {
            key = NULL;
            while ((key = lys_getnext(key, iter, NULL, 0)) && lysc_is_key(key)) {
                s = buffer ? strdup(buffer) : path;

                /* print key predicate */
                if (buffer) {
                    len = snprintf(buffer, buflen, "[%s='%%s']%s", key->name, s ? s : "");
                } else {
                    len = asprintf(&path, "[%s='%%s']%s", key->name, s ? s : "");
                }
                free(s);

                if (buffer && (buflen <= (size_t)len)) {
                    /* not enough space in buffer */
                    break;
                }
            }
        }

        s = buffer ? strdup(buffer) : path;
        if (parent && (iter->parent == parent)) {
            slash = "";
        } else {
            slash = "/";
        }

        if (skip_schema) {
            par = lysc_data_parent(iter);
        } else {
            par = iter->parent;
        }

        if (!par || (par->module != iter->module)) {
            /* print prefix */
            if (buffer) {
                len = snprintf(buffer, buflen, "%s%s:%s%s", slash, iter->module->name, iter->name, s ? s : "");
            } else {
                len = asprintf(&path, "%s%s:%s%s", slash, iter->module->name, iter->name, s ? s : "");
            }
        } else {
            /* prefix is the same as in parent */
            if (buffer) {
                len = snprintf(buffer, buflen, "%s%s%s", slash, iter->name, s ? s : "");
            } else {
                len = asprintf(&path, "%s%s%s", slash, iter->name, s ? s : "");
            }
        }
        free(s);

        if (buffer && (buflen <= (size_t)len)) {
            /* not enough space in buffer */
            break;
        }
    }

    if (len < 0) {
        free(path);
        path = NULL;
    } else if (len == 0) {
        if (buffer) {
            strcpy(buffer, "/");
        } else {
            path = strdup("/");
        }
    }

    if (buffer) {
        return buffer;
    } else {
        return path;
    }
}

LIBYANG_API_DEF char *
lysc_path(const struct lysc_node *node, LYSC_PATH_TYPE pathtype, char *buffer, size_t buflen)
{
    return lysc_path_until(node, NULL, pathtype, buffer, buflen);
}

LY_ERR
_lys_set_implemented(struct lys_module *mod, const char **features, struct lys_glob_unres *unres)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lys_module *mod_iter;
    const char **imp_f, *all_f[] = {"*", NULL};
    uint32_t i;

    if (mod->implemented) {
        /* mod is already implemented, set the features */
        r = lys_set_features(mod->parsed, features);
        if (r == LY_EEXIST) {
            /* no changes */
            return LY_SUCCESS;
        } else if (!r) {
            /* mark the module as changed */
            mod->to_compile = 1;
        }

        return r;
    }

    /* implement, ignore recompilation because it must always take place later */
    r = lys_implement(mod, features, unres);
    LY_CHECK_ERR_GOTO(r && (r != LY_ERECOMPILE), ret = r, cleanup);

    if (mod->ctx->flags & LY_CTX_ALL_IMPLEMENTED) {
        /* implement all the imports as well */
        for (i = 0; i < unres->creating.count; ++i) {
            mod = unres->creating.objs[i];
            if (mod->implemented) {
                continue;
            }

            imp_f = (mod->ctx->flags & LY_CTX_ENABLE_IMP_FEATURES) ? all_f : NULL;
            r = lys_implement(mod, imp_f, unres);
            LY_CHECK_ERR_GOTO(r && (r != LY_ERECOMPILE), ret = r, cleanup);
        }
    }

    /* Try to find module with LYS_MOD_IMPORTED_REV flag. */
    i = 0;
    while ((mod_iter = ly_ctx_get_module_iter(mod->ctx, &i))) {
        if (!strcmp(mod_iter->name, mod->name) && (mod_iter != mod) && (mod_iter->latest_revision & LYS_MOD_IMPORTED_REV)) {
            LOGVRB("Implemented module \"%s@%s\" was not and will not be imported if the revision-date is missing"
                    " in the import statement. Instead, the revision \"%s\" is imported.", mod->name, mod->revision,
                    mod_iter->revision);
            break;
        }
    }

cleanup:
    return ret;
}

/**
 * @brief Check whether it may be needed to (re)compile a module from a particular dependency set
 * and if so, add it into its dep set.
 *
 * Dependency set includes all modules that need to be (re)compiled in case any of the module(s)
 * in the dep set are (re)compiled.
 *
 * The reason for recompilation is possible disabled nodes and updating
 * leafref targets to point to the newly compiled modules. Using the import relation, the
 * dependency is reflexive because of possible foreign augments and deviations, which are compiled
 * during the target module compilation.
 *
 * - every module must belong to exactly one dep set
 * - implement flag must be ignored because it can be changed during dep set compilation
 *
 * @param[in] mod Module to process.
 * @param[in,out] ctx_set Set with all not-yet-processed modules.
 * @param[in,out] dep_set Current dependency set to update.
 * @param[in] aux_set Set of traversed non-compiled modules, should be empty on first call.
 * @return LY_ERR value.
 */
static LY_ERR
lys_unres_dep_sets_create_mod_r(struct lys_module *mod, struct ly_set *ctx_set, struct ly_set *dep_set,
        struct ly_set *aux_set)
{
    struct lys_module *mod2;
    struct lysp_import *imports;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool found;

    if (LYS_IS_SINGLE_DEP_SET(mod)) {
        /* is already in a separate dep set */
        if (!lys_has_dep_mods(mod)) {
            /* break the dep set here, no modules depend on this one */
            return LY_SUCCESS;
        }

        if (ly_set_contains(aux_set, mod, NULL)) {
            /* it was traversed */
            return LY_SUCCESS;
        }

        /* add a new auxiliary module */
        LY_CHECK_RET(ly_set_add(aux_set, mod, 1, NULL));
    } else {
        if (!ly_set_contains(ctx_set, mod, &i)) {
            /* it was already processed */
            return LY_SUCCESS;
        }

        /* remove it from the set, we are processing it now */
        ly_set_rm_index(ctx_set, i, NULL);

        /* add a new dependent module into the dep set */
        LY_CHECK_RET(ly_set_add(dep_set, mod, 1, NULL));
    }

    /* process imports of the module and submodules */
    imports = mod->parsed->imports;
    LY_ARRAY_FOR(imports, u) {
        mod2 = imports[u].module;
        LY_CHECK_RET(lys_unres_dep_sets_create_mod_r(mod2, ctx_set, dep_set, aux_set));
    }
    LY_ARRAY_FOR(mod->parsed->includes, v) {
        imports = mod->parsed->includes[v].submodule->imports;
        LY_ARRAY_FOR(imports, u) {
            mod2 = imports[u].module;
            if (LYS_IS_SINGLE_DEP_SET(mod2) && !lys_has_dep_mods(mod2)) {
                /* break the dep set here, no modules depend on this one */
                continue;
            }

            LY_CHECK_RET(lys_unres_dep_sets_create_mod_r(imports[u].module, ctx_set, dep_set, aux_set));
        }
    }

    /* process modules and submodules importing this module */
    for (i = 0; i < mod->ctx->list.count; ++i) {
        mod2 = mod->ctx->list.objs[i];
        found = 0;

        imports = mod2->parsed->imports;
        LY_ARRAY_FOR(imports, u) {
            if (imports[u].module == mod) {
                found = 1;
                break;
            }
        }

        if (!found) {
            LY_ARRAY_FOR(mod2->parsed->includes, v) {
                imports = mod2->parsed->includes[v].submodule->imports;
                LY_ARRAY_FOR(imports, u) {
                    if (imports[u].module == mod) {
                        found = 1;
                        break;
                    }
                }

                if (found) {
                    break;
                }
            }
        }

        if (found) {
            LY_CHECK_RET(lys_unres_dep_sets_create_mod_r(mod2, ctx_set, dep_set, aux_set));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Add all simple modules (that have nothing to (re)compile) into separate dep sets.
 *
 * @param[in,out] ctx_set Set with all not-yet-processed modules.
 * @param[in,out] main_set Set of dependency module sets.
 * @return LY_ERR value.
 */
static LY_ERR
lys_unres_dep_sets_create_single(struct ly_set *ctx_set, struct ly_set *main_set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *m;
    uint32_t i = 0;
    struct ly_set *dep_set = NULL;

    while (i < ctx_set->count) {
        m = ctx_set->objs[i];
        if (LYS_IS_SINGLE_DEP_SET(m)) {
            /* remove it from the set, we are processing it now */
            ly_set_rm_index(ctx_set, i, NULL);

            /* this module can be in a separate dep set (but there still may be modules importing this one
             * that depend on imports of this one in case it defines groupings) */
            LY_CHECK_GOTO(ret = ly_set_new(&dep_set), cleanup);
            LY_CHECK_GOTO(ret = ly_set_add(dep_set, m, 1, NULL), cleanup);
            LY_CHECK_GOTO(ret = ly_set_add(main_set, dep_set, 1, NULL), cleanup);
            dep_set = NULL;
        } else {
            ++i;
        }
    }

cleanup:
    ly_set_free(dep_set, NULL);
    return ret;
}

LY_ERR
lys_unres_dep_sets_create(struct ly_ctx *ctx, struct ly_set *main_set, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *m;
    struct ly_set *dep_set = NULL, *ctx_set = NULL, aux_set = {0};
    uint32_t i;
    ly_bool found;

    assert(!main_set->count);

    /* start with a duplicate set of modules that we will remove from */
    LY_CHECK_GOTO(ret = ly_set_dup(&ctx->list, NULL, &ctx_set), cleanup);

    /* first create all dep sets with single modules */
    LY_CHECK_GOTO(ret = lys_unres_dep_sets_create_single(ctx_set, main_set), cleanup);

    if (mod && !ly_set_contains(ctx_set, mod, NULL)) {
        /* dep set for this module has already been created, nothing else to do */
        goto cleanup;
    }

    while (ctx_set->count) {
        /* create new dep set */
        LY_CHECK_GOTO(ret = ly_set_new(&dep_set), cleanup);

        if (mod) {
            /* use the module create a dep set with the rest of its dependent modules */
            LY_CHECK_GOTO(ret = lys_unres_dep_sets_create_mod_r(mod, ctx_set, dep_set, &aux_set), cleanup);
        } else {
            /* use first ctx mod to create a dep set with the rest of its dependent modules */
            LY_CHECK_GOTO(ret = lys_unres_dep_sets_create_mod_r(ctx_set->objs[0], ctx_set, dep_set, &aux_set), cleanup);
        }
        ly_set_erase(&aux_set, NULL);
        assert(dep_set->count);

        /* check whether there is any module that will be (re)compiled */
        found = 0;
        for (i = 0; i < dep_set->count; ++i) {
            m = dep_set->objs[i];
            if (m->to_compile) {
                found = 1;
                break;
            }
        }

        if (found) {
            /* if there is, all the implemented modules need to be recompiled */
            for (i = 0; i < dep_set->count; ++i) {
                m = dep_set->objs[i];
                if (m->implemented) {
                    m->to_compile = 1;
                }
            }
        }

        /* add the dep set into main set */
        LY_CHECK_GOTO(ret = ly_set_add(main_set, dep_set, 1, NULL), cleanup);
        dep_set = NULL;

        if (mod) {
            /* we need dep set only for this module */
            break;
        }
    }

#ifndef NDEBUG
    LOGDBG(LY_LDGDEPSETS, "dep sets created (%" PRIu32 "):", main_set->count);
    for (i = 0; i < main_set->count; ++i) {
        struct ly_set *iter_set = main_set->objs[i];

        LOGDBG(LY_LDGDEPSETS, "dep set #%" PRIu32 ":", i);
        for (uint32_t j = 0; j < iter_set->count; ++j) {
            m = iter_set->objs[j];
            LOGDBG(LY_LDGDEPSETS, "\t%s", m->name);
        }
    }
#endif

cleanup:
    assert(ret || main_set->objs);
    ly_set_erase(&aux_set, NULL);
    ly_set_free(dep_set, NULL);
    ly_set_free(ctx_set, NULL);
    return ret;
}

void
lys_unres_glob_revert(struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    uint32_t i, j, idx, temp_lo = 0;
    struct lysf_ctx fctx = {.ctx = ctx};
    struct ly_set *dep_set;
    LY_ERR ret;

    for (i = 0; i < unres->implementing.count; ++i) {
        fctx.mod = unres->implementing.objs[i];
        assert(fctx.mod->implemented);

        /* make the module correctly non-implemented again */
        fctx.mod->implemented = 0;
        lys_precompile_augments_deviations_revert(ctx, fctx.mod);
        lysc_module_free(&fctx, fctx.mod->compiled);
        fctx.mod->compiled = NULL;

        /* should not be made implemented */
        fctx.mod->to_compile = 0;
    }

    for (i = 0; i < unres->creating.count; ++i) {
        fctx.mod = unres->creating.objs[i];

        /* remove the module from the context */
        ly_set_rm(&ctx->list, fctx.mod, NULL);

        /* remove it also from dep sets */
        for (j = 0; j < unres->dep_sets.count; ++j) {
            dep_set = unres->dep_sets.objs[j];
            if (ly_set_contains(dep_set, fctx.mod, &idx)) {
                ly_set_rm_index(dep_set, idx, NULL);
                break;
            }
        }

        /* free the module */
        lys_module_free(&fctx, fctx.mod, 1);
    }

    /* remove the extensions as well */
    lysf_ctx_erase(&fctx);

    if (unres->implementing.count) {
        /* recompile previous context because some implemented modules are no longer implemented,
         * we can reuse the current to_compile flags */
        ly_temp_log_options(&temp_lo);
        ret = lys_compile_depset_all(ctx, &ctx->unres);
        ly_temp_log_options(NULL);
        if (ret) {
            LOGINT(ctx);
        }
    }
}

void
lys_unres_glob_erase(struct lys_glob_unres *unres)
{
    uint32_t i;

    for (i = 0; i < unres->dep_sets.count; ++i) {
        ly_set_free(unres->dep_sets.objs[i], NULL);
    }
    ly_set_erase(&unres->dep_sets, NULL);
    ly_set_erase(&unres->implementing, NULL);
    ly_set_erase(&unres->creating, NULL);

    assert(!unres->ds_unres.whens.count);
    assert(!unres->ds_unres.musts.count);
    assert(!unres->ds_unres.leafrefs.count);
    assert(!unres->ds_unres.disabled_leafrefs.count);
    assert(!unres->ds_unres.dflts.count);
    assert(!unres->ds_unres.disabled.count);
}

LIBYANG_API_DEF LY_ERR
lys_set_implemented(struct lys_module *mod, const char **features)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_glob_unres *unres = &mod->ctx->unres;

    LY_CHECK_ARG_RET(NULL, mod, LY_EINVAL);

    /* implement */
    ret = _lys_set_implemented(mod, features, unres);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(mod->ctx->flags & LY_CTX_EXPLICIT_COMPILE)) {
        /* create dep set for the module and mark all the modules that will be (re)compiled */
        LY_CHECK_GOTO(ret = lys_unres_dep_sets_create(mod->ctx, &unres->dep_sets, mod), cleanup);

        /* (re)compile the whole dep set (other dep sets will have no modules marked for compilation) */
        LY_CHECK_GOTO(ret = lys_compile_depset_all(mod->ctx, unres), cleanup);

        /* unres resolved */
        lys_unres_glob_erase(unres);
    }

cleanup:
    if (ret) {
        lys_unres_glob_revert(mod->ctx, unres);
        lys_unres_glob_erase(unres);
    }
    return ret;
}

/**
 * @brief Resolve (find) all imported and included modules.
 *
 * @param[in] pctx Parser context.
 * @param[in] pmod Parsed module to resolve.
 * @param[out] new_mods Set with all the newly loaded modules.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_resolve_import_include(struct lysp_ctx *pctx, struct lysp_module *pmod, struct ly_set *new_mods)
{
    struct lysp_import *imp;
    LY_ARRAY_COUNT_TYPE u, v;

    pmod->parsing = 1;
    LY_ARRAY_FOR(pmod->imports, u) {
        imp = &pmod->imports[u];
        if (!imp->module) {
            LY_CHECK_RET(lys_parse_load(PARSER_CTX(pctx), imp->name, imp->rev[0] ? imp->rev : NULL, new_mods, &imp->module));

            if (!imp->rev[0]) {
                /* This module must be selected for the next similar
                 * import without revision-date to avoid incorrect
                 * derived identities in the ::lys_module.identities.
                 */
                imp->module->latest_revision |= LYS_MOD_IMPORTED_REV;
            }
        }
        /* check for importing the same module twice */
        for (v = 0; v < u; ++v) {
            if (imp->module == pmod->imports[v].module) {
                LOGWRN(PARSER_CTX(pctx), "Single revision of the module \"%s\" imported twice.", imp->name);
            }
        }
    }
    LY_CHECK_RET(lysp_load_submodules(pctx, pmod, new_mods));

    pmod->parsing = 0;

    return LY_SUCCESS;
}

/**
 * @brief Generate path of the given paresed node.
 *
 * @param[in] node Schema path of this node will be generated.
 * @param[in] parent Build relative path only until this parent is found. If NULL, the full absolute path is printed.
 * @return NULL in case of memory allocation error, path of the node otherwise.
 * In case the @p buffer is NULL, the returned string is dynamically allocated and caller is responsible to free it.
 */
static char *
lysp_path_until(const struct lysp_node *node, const struct lysp_node *parent, const struct lysp_module *pmod)
{
    const struct lysp_node *iter, *par;
    char *path = NULL, *s;
    const char *slash;
    int len = 0;

    for (iter = node; iter && (iter != parent) && (len >= 0); iter = iter->parent) {
        if (parent && (iter->parent == parent)) {
            slash = "";
        } else {
            slash = "/";
        }

        s = path;
        par = iter->parent;
        if (!par) {
            /* print prefix */
            len = asprintf(&path, "%s%s:%s%s", slash, pmod->mod->name, iter->name, s ? s : "");
        } else {
            /* prefix is the same as in parent */
            len = asprintf(&path, "%s%s%s", slash, iter->name, s ? s : "");
        }
        free(s);
    }

    if (len < 0) {
        free(path);
        path = NULL;
    } else if (len == 0) {
        path = strdup("/");
    }

    return path;
}

/**
 * @brief Build log path for a parsed extension instance.
 *
 * @param[in] pcxt Parse context.
 * @param[in] ext Parsed extension instance.
 * @param[out] path Generated path.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_resolve_ext_instance_log_path(const struct lysp_ctx *pctx, const struct lysp_ext_instance *ext, char **path)
{
    char *buf = NULL;
    uint32_t used = 0, size = 0;

    if (ext->parent_stmt & LY_STMT_NODE_MASK) {
        /* parsed node path */
        buf = lysp_path_until(ext->parent, NULL, PARSER_CUR_PMOD(pctx));
        LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        size = used = strlen(buf);

        /* slash */
        size += 1;
        buf = realloc(buf, size + 1);
        LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        used += sprintf(buf + used, "/");
    } else {
        /* module */
        size += 1 + strlen(PARSER_CUR_PMOD(pctx)->mod->name) + 1;
        buf = realloc(buf, size + 1);
        LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        used += sprintf(buf + used, "/%s:", PARSER_CUR_PMOD(pctx)->mod->name);
    }

    /* extension name */
    size += 12 + strlen(ext->name) + 2;
    buf = realloc(buf, size + 1);
    LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
    used += sprintf(buf + used, "{extension='%s'}", ext->name);

    /* extension argument */
    if (ext->argument) {
        size += 1 + strlen(ext->argument);
        buf = realloc(buf, size + 1);
        LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(pctx)), LY_EMEM);
        used += sprintf(buf + used, "/%s", ext->argument);
    }

    *path = buf;
    return LY_SUCCESS;
}

/**
 * @brief Resolve (find) all extension instance records and finish their parsing.
 *
 * @param[in] pctx Parse context with all the parsed extension instances.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_resolve_ext_instance_records(struct lysp_ctx *pctx)
{
    LY_ERR r;
    struct lysf_ctx fctx = {.ctx = PARSER_CTX(pctx)};
    struct lysp_ext_instance *exts, *ext;
    const struct lys_module *mod;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;
    char *path = NULL;

    /* first finish parsing all extension instances ... */
    for (i = 0; i < pctx->ext_inst.count; ++i) {
        exts = pctx->ext_inst.objs[i];
        LY_ARRAY_FOR(exts, u) {
            ext = &exts[u];

            /* find the extension definition */
            LY_CHECK_RET(lysp_ext_find_definition(PARSER_CTX(pctx), ext, &mod, &ext->def));

            /* resolve the argument, if needed */
            LY_CHECK_RET(lysp_ext_instance_resolve_argument(PARSER_CTX(pctx), ext));

            /* find the extension record, if any */
            ext->record = lyplg_ext_record_find(mod->name, mod->revision, ext->def->name);
        }
    }

    /* ... then call the parse callback */
    for (i = 0; i < pctx->ext_inst.count; ++i) {
        exts = pctx->ext_inst.objs[i];
        u = 0;
        while (u < LY_ARRAY_COUNT(exts)) {
            ext = &exts[u];
            if (!ext->record || !ext->record->plugin.parse) {
                goto next_iter;
            }

            /* set up log path */
            if ((r = lysp_resolve_ext_instance_log_path(pctx, ext, &path))) {
                return r;
            }
            LOG_LOCSET(NULL, NULL, path, NULL);

            /* parse */
            r = ext->record->plugin.parse(pctx, ext);

            LOG_LOCBACK(0, 0, 1, 0);
            free(path);

            if (r == LY_ENOT) {
                /* instance should be ignored, remove it */
                lysp_ext_instance_free(&fctx, ext);
                LY_ARRAY_DECREMENT(exts);
                if (u < LY_ARRAY_COUNT(exts)) {
                    /* replace by the last item */
                    *ext = exts[LY_ARRAY_COUNT(exts)];
                } /* else if there are no more items, leave the empty array, we are not able to free it */
                continue;
            } else if (r) {
                /* error */
                return r;
            }

next_iter:
            ++u;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lys_parse_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, struct lysp_ctx *main_ctx,
        LY_ERR (*custom_check)(const struct ly_ctx *, struct lysp_module *, struct lysp_submodule *, void *),
        void *check_data, struct ly_set *new_mods, struct lysp_submodule **submodule)
{
    LY_ERR ret;
    struct lysp_submodule *submod = NULL, *latest_sp;
    struct lysp_yang_ctx *yangctx = NULL;
    struct lysp_yin_ctx *yinctx = NULL;
    struct lysp_ctx *pctx;
    struct lysf_ctx fctx = {.ctx = ctx};

    LY_CHECK_ARG_RET(ctx, ctx, in, LY_EINVAL);

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_submodule(&yinctx, ctx, main_ctx, in, &submod);
        pctx = (struct lysp_ctx *)yinctx;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_submodule(&yangctx, ctx, main_ctx, in, &submod);
        pctx = (struct lysp_ctx *)yangctx;
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        ret = LY_EINVAL;
        break;
    }
    LY_CHECK_GOTO(ret, error);
    assert(submod);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(submod->revs);

    /* decide the latest revision */
    latest_sp = (struct lysp_submodule *)ly_ctx_get_submodule2_latest(submod->mod, submod->name);
    if (latest_sp) {
        if (submod->revs) {
            if (!latest_sp->revs) {
                /* latest has no revision, so mod is anyway newer */
                submod->latest_revision = latest_sp->latest_revision;
                /* the latest_sp is zeroed later when the new module is being inserted into the context */
            } else if (strcmp(submod->revs[0].date, latest_sp->revs[0].date) > 0) {
                submod->latest_revision = latest_sp->latest_revision;
                /* the latest_sp is zeroed later when the new module is being inserted into the context */
            } else {
                latest_sp = NULL;
            }
        } else {
            latest_sp = NULL;
        }
    } else {
        submod->latest_revision = 1;
    }

    if (custom_check) {
        LY_CHECK_GOTO(ret = custom_check(ctx, NULL, submod, check_data), error);
    }

    if (latest_sp) {
        latest_sp->latest_revision = 0;
    }

    lys_parser_fill_filepath(ctx, in, &submod->filepath);

    /* resolve imports and includes */
    LY_CHECK_GOTO(ret = lysp_resolve_import_include(pctx, (struct lysp_module *)submod, new_mods), error);

    if (format == LYS_IN_YANG) {
        lysp_yang_ctx_free(yangctx);
    } else {
        lysp_yin_ctx_free(yinctx);
    }
    *submodule = submod;
    return LY_SUCCESS;

error:
    if (!submod || !submod->name) {
        LOGERR(ctx, ret, "Parsing submodule failed.");
    } else {
        LOGERR(ctx, ret, "Parsing submodule \"%s\" failed.", submod->name);
    }
    lysp_module_free(&fctx, (struct lysp_module *)submod);
    if (format == LYS_IN_YANG) {
        lysp_yang_ctx_free(yangctx);
    } else {
        lysp_yin_ctx_free(yinctx);
    }
    return ret;
}

/**
 * @brief Add ietf-netconf metadata to the parsed module. Operation, filter, and select are added.
 *
 * @param[in] pctx Parse context.
 * @param[in] mod Parsed module to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lysp_add_internal_ietf_netconf(struct lysp_ctx *pctx, struct lysp_module *mod)
{
    struct lysp_ext_instance *extp, *prev_exts = mod->exts;
    struct lysp_stmt *stmt;
    struct lysp_import *imp;
    uint32_t idx;

    /*
     * 1) edit-config's operation
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, extp, LY_EMEM);
    LY_CHECK_ERR_RET(!extp, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &extp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "operation", 0, &extp->argument));
    extp->format = LY_VALUE_SCHEMA;
    extp->prefix_data = mod;
    extp->parent = mod;
    extp->parent_stmt = LY_STMT_MODULE;
    extp->flags = LYS_INTERNAL;

    extp->child = stmt = calloc(1, sizeof *extp->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enumeration", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_TYPE;

    stmt->child = calloc(1, sizeof *stmt->child);
    stmt = stmt->child;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "merge", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "replace", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "create", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "delete", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "remove", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    /*
     * 2) filter's type
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, extp, LY_EMEM);
    LY_CHECK_ERR_RET(!extp, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &extp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &extp->argument));
    extp->format = LY_VALUE_SCHEMA;
    extp->prefix_data = mod;
    extp->parent = mod;
    extp->parent_stmt = LY_STMT_MODULE;
    extp->flags = LYS_INTERNAL;

    extp->child = stmt = calloc(1, sizeof *extp->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enumeration", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_TYPE;

    stmt->child = calloc(1, sizeof *stmt->child);
    stmt = stmt->child;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "subtree", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    stmt->next = calloc(1, sizeof *stmt->child);
    stmt = stmt->next;
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "enum", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "xpath", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_ENUM;

    /* if-feature for enum allowed only for YANG 1.1 modules */
    if (mod->version >= LYS_VERSION_1_1) {
        stmt->child = calloc(1, sizeof *stmt->child);
        stmt = stmt->child;
        LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
        LY_CHECK_RET(lydict_insert(mod->mod->ctx, "if-feature", 0, &stmt->stmt));
        LY_CHECK_RET(lydict_insert(mod->mod->ctx, "xpath", 0, &stmt->arg));
        stmt->format = LY_VALUE_SCHEMA;
        stmt->prefix_data = mod;
        stmt->kw = LY_STMT_IF_FEATURE;
    }

    /*
     * 3) filter's select
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, extp, LY_EMEM);
    LY_CHECK_ERR_RET(!extp, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &extp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "select", 0, &extp->argument));
    extp->format = LY_VALUE_SCHEMA;
    extp->prefix_data = mod;
    extp->parent = mod;
    extp->parent_stmt = LY_STMT_MODULE;
    extp->flags = LYS_INTERNAL;

    extp->child = stmt = calloc(1, sizeof *extp->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "yang_:xpath1.0", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_TYPE;

    if (!prev_exts) {
        /* first extension instances */
        assert(pctx->main_ctx == pctx);
        LY_CHECK_RET(ly_set_add(&pctx->ext_inst, mod->exts, 1, NULL));
    } else {
        /* replace previously stored extension instances */
        if (!ly_set_contains(&pctx->ext_inst, prev_exts, &idx)) {
            LOGINT_RET(mod->mod->ctx);
        }
        pctx->ext_inst.objs[idx] = mod->exts;
    }

    /* create new imports for the used prefixes */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-metadata", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-types", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "yang_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    return LY_SUCCESS;
}

/**
 * @brief Add ietf-netconf-with-defaults "default" metadata to the parsed module.
 *
 * @param[in] pctx Parse context.
 * @param[in] mod Parsed module to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lysp_add_internal_ietf_netconf_with_defaults(struct lysp_ctx *pctx, struct lysp_module *mod)
{
    struct lysp_ext_instance *extp, *prev_exts = mod->exts;
    struct lysp_stmt *stmt;
    struct lysp_import *imp;
    uint32_t idx;

    /* add new extension instance */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, extp, LY_EMEM);

    /* fill in the extension instance fields */
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &extp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "default", 0, &extp->argument));
    extp->format = LY_VALUE_SCHEMA;
    extp->prefix_data = mod;
    extp->parent = mod;
    extp->parent_stmt = LY_STMT_MODULE;
    extp->flags = LYS_INTERNAL;

    extp->child = stmt = calloc(1, sizeof *extp->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "boolean", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_TYPE;

    if (!prev_exts) {
        /* first extension instances */
        assert(pctx->main_ctx == pctx);
        LY_CHECK_RET(ly_set_add(&pctx->ext_inst, mod->exts, 1, NULL));
    } else {
        /* replace previously stored extension instances */
        if (!ly_set_contains(&pctx->ext_inst, prev_exts, &idx)) {
            LOGINT_RET(mod->mod->ctx);
        }
        pctx->ext_inst.objs[idx] = mod->exts;
    }

    /* create new import for the used prefix */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-metadata", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    return LY_SUCCESS;
}

LY_ERR
lys_parse_in(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format,
        LY_ERR (*custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
        void *check_data, struct ly_set *new_mods, struct lys_module **module)
{
    struct lys_module *mod = NULL, *latest, *mod_dup = NULL;
    LY_ERR ret;
    struct lysp_yang_ctx *yangctx = NULL;
    struct lysp_yin_ctx *yinctx = NULL;
    struct lysp_ctx *pctx = NULL;
    struct lysf_ctx fctx = {.ctx = ctx};
    ly_bool module_created = 0;

    assert(ctx && in && new_mods);

    if (module) {
        *module = NULL;
    }

    mod = calloc(1, sizeof *mod);
    LY_CHECK_ERR_RET(!mod, LOGMEM(ctx), LY_EMEM);
    mod->ctx = ctx;

    /* parse */
    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_module(&yinctx, in, mod);
        pctx = (struct lysp_ctx *)yinctx;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_module(&yangctx, in, mod);
        pctx = (struct lysp_ctx *)yangctx;
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        ret = LY_EINVAL;
        break;
    }
    LY_CHECK_GOTO(ret, cleanup);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(mod->parsed->revs);
    if (mod->parsed->revs) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, mod->parsed->revs[0].date, 0, &mod->revision), cleanup);
    }

    /* decide the latest revision */
    latest = ly_ctx_get_module_latest(ctx, mod->name);
    if (latest) {
        if (mod->revision) {
            if (!latest->revision) {
                /* latest has no revision, so mod is anyway newer */
                mod->latest_revision = latest->latest_revision & (LYS_MOD_LATEST_REV | LYS_MOD_LATEST_SEARCHDIRS);
                /* the latest is zeroed later when the new module is being inserted into the context */
            } else if (strcmp(mod->revision, latest->revision) > 0) {
                mod->latest_revision = latest->latest_revision & (LYS_MOD_LATEST_REV | LYS_MOD_LATEST_SEARCHDIRS);
                /* the latest is zeroed later when the new module is being inserted into the context */
            } else {
                latest = NULL;
            }
        } else {
            latest = NULL;
        }
    } else {
        mod->latest_revision = LYS_MOD_LATEST_REV;
    }

    if (custom_check) {
        LY_CHECK_GOTO(ret = custom_check(ctx, mod->parsed, NULL, check_data), cleanup);
    }

    /* check whether it is not already in the context in the same revision */
    mod_dup = ly_ctx_get_module(ctx, mod->name, mod->revision);
    if (mod_dup) {
        /* nothing to do */
        LOGVRB("Module \"%s@%s\" is already present in the context.", mod_dup->name,
                mod_dup->revision ? mod_dup->revision : "<none>");
        goto cleanup;
    }

    /* check whether there is not a namespace collision */
    mod_dup = ly_ctx_get_module_latest_ns(ctx, mod->ns);
    if (mod_dup && (mod_dup->revision == mod->revision)) {
        LOGERR(ctx, LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\".",
                mod_dup->name, mod->name, mod->ns);
        ret = LY_EINVAL;
        goto cleanup;
    }

    switch (in->type) {
    case LY_IN_FILEPATH:
        ly_check_module_filename(ctx, mod->name, mod->parsed->revs ? mod->parsed->revs[0].date : NULL, in->method.fpath.filepath);
        break;
    case LY_IN_FD:
    case LY_IN_FILE:
    case LY_IN_MEMORY:
        /* nothing special to do */
        break;
    case LY_IN_ERROR:
        LOGINT(ctx);
        ret = LY_EINT;
        goto cleanup;
    }
    lys_parser_fill_filepath(ctx, in, &mod->filepath);

    if (latest) {
        latest->latest_revision &= ~(LYS_MOD_LATEST_REV | LYS_MOD_LATEST_SEARCHDIRS);
    }

    /* add internal data in case specific modules were parsed */
    if (!strcmp(mod->name, "ietf-netconf")) {
        LY_CHECK_GOTO(ret = lysp_add_internal_ietf_netconf(pctx, mod->parsed), cleanup);
    } else if (!strcmp(mod->name, "ietf-netconf-with-defaults")) {
        LY_CHECK_GOTO(ret = lysp_add_internal_ietf_netconf_with_defaults(pctx, mod->parsed), cleanup);
    }

    /* add the module into newly created module set, will also be freed from there on any error */
    LY_CHECK_GOTO(ret = ly_set_add(new_mods, mod, 1, NULL), cleanup);
    module_created = 1;

    /* add into context */
    ret = ly_set_add(&ctx->list, mod, 1, NULL);
    LY_CHECK_GOTO(ret, cleanup);
    ctx->change_count++;

    /* resolve includes and all imports */
    LY_CHECK_GOTO(ret = lysp_resolve_import_include(pctx, mod->parsed, new_mods), cleanup);

    /* resolve extension instance plugin records */
    LY_CHECK_GOTO(ret = lysp_resolve_ext_instance_records(pctx), cleanup);

    /* check name collisions */
    LY_CHECK_GOTO(ret = lysp_check_dup_typedefs(pctx, mod->parsed), cleanup);
    LY_CHECK_GOTO(ret = lysp_check_dup_groupings(pctx, mod->parsed), cleanup);
    LY_CHECK_GOTO(ret = lysp_check_dup_features(pctx, mod->parsed), cleanup);
    LY_CHECK_GOTO(ret = lysp_check_dup_identities(pctx, mod->parsed), cleanup);

    /* compile features */
    LY_CHECK_GOTO(ret = lys_compile_feature_iffeatures(mod->parsed), cleanup);

    /* compile identities */
    LY_CHECK_GOTO(ret = lys_compile_identities(mod), cleanup);

cleanup:
    if (ret && (ret != LY_EEXIST)) {
        if (mod && mod->name) {
            /* there are cases when path is not available for parsing error, so this additional
             * message tries to add information about the module where the error occurred */
            struct ly_err_item *e = ly_err_last(ctx);

            if (e && (!e->path || !strncmp(e->path, "Line ", ly_strlen_const("Line ")))) {
                LOGERR(ctx, ret, "Parsing module \"%s\" failed.", mod->name);
            }
        }
    }
    if (!module_created) {
        fctx.mod = mod;
        lys_module_free(&fctx, mod, 0);
        lysf_ctx_erase(&fctx);

        mod = mod_dup;
    }

    if (format == LYS_IN_YANG) {
        lysp_yang_ctx_free(yangctx);
    } else {
        lysp_yin_ctx_free(yinctx);
    }

    if (!ret && module) {
        *module = mod;
    }
    return ret;
}

static LYS_INFORMAT
lys_parse_get_format(const struct ly_in *in, LYS_INFORMAT format)
{
    if (!format && (in->type == LY_IN_FILEPATH)) {
        /* unknown format - try to detect it from filename's suffix */
        const char *path = in->method.fpath.filepath;
        size_t len = strlen(path);

        /* ignore trailing whitespaces */
        for ( ; len > 0 && isspace(path[len - 1]); len--) {}

        if ((len >= LY_YANG_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_YANG_SUFFIX_LEN], LY_YANG_SUFFIX, LY_YANG_SUFFIX_LEN)) {
            format = LYS_IN_YANG;
        } else if ((len >= LY_YIN_SUFFIX_LEN + 1) &&
                !strncmp(&path[len - LY_YIN_SUFFIX_LEN], LY_YIN_SUFFIX, LY_YIN_SUFFIX_LEN)) {
            format = LYS_IN_YIN;
        } /* else still unknown */
    }

    return format;
}

LIBYANG_API_DEF LY_ERR
lys_parse(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, const char **features, struct lys_module **module)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *mod;

    if (module) {
        *module = NULL;
    }
    LY_CHECK_ARG_RET(NULL, ctx, in, LY_EINVAL);

    format = lys_parse_get_format(in, format);
    LY_CHECK_ARG_RET(ctx, format, LY_EINVAL);

    /* remember input position */
    in->func_start = in->current;

    /* parse */
    ret = lys_parse_in(ctx, in, format, NULL, NULL, &ctx->unres.creating, &mod);
    LY_CHECK_GOTO(ret, cleanup);

    /* implement */
    ret = _lys_set_implemented(mod, features, &ctx->unres);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(ctx->flags & LY_CTX_EXPLICIT_COMPILE)) {
        /* create dep set for the module and mark all the modules that will be (re)compiled */
        LY_CHECK_GOTO(ret = lys_unres_dep_sets_create(ctx, &ctx->unres.dep_sets, mod), cleanup);

        /* (re)compile the whole dep set (other dep sets will have no modules marked for compilation) */
        LY_CHECK_GOTO(ret = lys_compile_depset_all(ctx, &ctx->unres), cleanup);

        /* unres resolved */
        lys_unres_glob_erase(&ctx->unres);
    }

cleanup:
    if (ret) {
        lys_unres_glob_revert(ctx, &ctx->unres);
        lys_unres_glob_erase(&ctx->unres);
    } else if (module) {
        *module = mod;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, data, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_memory(data, &in), LOGERR(ctx, ret, "Unable to create input handler."), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, fd > -1, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_fd(fd, &in), LOGERR(ctx, ret, "Unable to create input handler."), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, path, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_filepath(path, 0, &in),
            LOGERR(ctx, ret, "Unable to create input handler for filepath %s.", path), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_search_localfile(const char * const *searchpaths, ly_bool cwd, const char *name, const char *revision,
        char **localfile, LYS_INFORMAT *format)
{
    LY_ERR ret = LY_EMEM;
    size_t len, flen, match_len = 0, dir_len;
    ly_bool implicit_cwd = 0;
    char *wd, *wn = NULL;
    DIR *dir = NULL;
    struct dirent *file;
    char *match_name = NULL;
    LYS_INFORMAT format_aux, match_format = 0;
    struct ly_set *dirs;
    struct stat st;

    LY_CHECK_ARG_RET(NULL, localfile, LY_EINVAL);

    /* start to fill the dir fifo with the context's search path (if set)
     * and the current working directory */
    LY_CHECK_RET(ly_set_new(&dirs));

    len = strlen(name);
    if (cwd) {
        wd = get_current_dir_name();
        if (!wd) {
            LOGMEM(NULL);
            goto cleanup;
        } else {
            /* add implicit current working directory (./) to be searched,
             * this directory is not searched recursively */
            ret = ly_set_add(dirs, wd, 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
            implicit_cwd = 1;
        }
    }
    if (searchpaths) {
        for (uint64_t i = 0; searchpaths[i]; i++) {
            /* check for duplicities with the implicit current working directory */
            if (implicit_cwd && !strcmp(dirs->objs[0], searchpaths[i])) {
                implicit_cwd = 0;
                continue;
            }
            wd = strdup(searchpaths[i]);
            if (!wd) {
                LOGMEM(NULL);
                goto cleanup;
            } else {
                ret = ly_set_add(dirs, wd, 0, NULL);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
    }
    wd = NULL;

    /* start searching */
    while (dirs->count) {
        free(wd);
        free(wn); wn = NULL;

        dirs->count--;
        wd = (char *)dirs->objs[dirs->count];
        dirs->objs[dirs->count] = NULL;
        LOGVRB("Searching for \"%s\" in \"%s\".", name, wd);

        if (dir) {
            closedir(dir);
        }
        dir = opendir(wd);
        dir_len = strlen(wd);
        if (!dir) {
            LOGWRN(NULL, "Unable to open directory \"%s\" for searching (sub)modules (%s).", wd, strerror(errno));
        } else {
            while ((file = readdir(dir))) {
                if (!strcmp(".", file->d_name) || !strcmp("..", file->d_name)) {
                    /* skip . and .. */
                    continue;
                }
                free(wn);
                if (asprintf(&wn, "%s/%s", wd, file->d_name) == -1) {
                    LOGMEM(NULL);
                    goto cleanup;
                }
                if (stat(wn, &st) == -1) {
                    LOGWRN(NULL, "Unable to get information about \"%s\" file in \"%s\" when searching for (sub)modules (%s)",
                            file->d_name, wd, strerror(errno));
                    continue;
                }
                if (S_ISDIR(st.st_mode) && (dirs->count || !implicit_cwd)) {
                    /* we have another subdirectory in searchpath to explore,
                     * subdirectories are not taken into account in current working dir (dirs->set.g[0]) */
                    ret = ly_set_add(dirs, wn, 0, NULL);
                    LY_CHECK_GOTO(ret, cleanup);

                    /* continue with the next item in current directory */
                    wn = NULL;
                    continue;
                } else if (!S_ISREG(st.st_mode)) {
                    /* not a regular file (note that we see the target of symlinks instead of symlinks */
                    continue;
                }

                /* here we know that the item is a file which can contain a module */
                if (strncmp(name, file->d_name, len) ||
                        ((file->d_name[len] != '.') && (file->d_name[len] != '@'))) {
                    /* different filename than the module we search for */
                    continue;
                }

                /* get type according to filename suffix */
                flen = strlen(file->d_name);
                if ((flen >= LY_YANG_SUFFIX_LEN + 1) &&
                        !strcmp(&file->d_name[flen - LY_YANG_SUFFIX_LEN], LY_YANG_SUFFIX)) {
                    format_aux = LYS_IN_YANG;
                } else if ((flen >= LY_YIN_SUFFIX_LEN + 1) &&
                        !strcmp(&file->d_name[flen - LY_YIN_SUFFIX_LEN], LY_YIN_SUFFIX)) {
                    format_aux = LYS_IN_YIN;
                } else {
                    /* not supportde suffix/file format */
                    continue;
                }

                if (revision) {
                    /* we look for the specific revision, try to get it from the filename */
                    if (file->d_name[len] == '@') {
                        /* check revision from the filename */
                        if (strncmp(revision, &file->d_name[len + 1], strlen(revision))) {
                            /* another revision */
                            continue;
                        } else {
                            /* exact revision */
                            free(match_name);
                            match_name = wn;
                            wn = NULL;
                            match_len = dir_len + 1 + len;
                            match_format = format_aux;
                            goto success;
                        }
                    } else {
                        /* continue trying to find exact revision match, use this only if not found */
                        free(match_name);
                        match_name = wn;
                        wn = NULL;
                        match_len = dir_len + 1 + len;
                        match_format = format_aux;
                        continue;
                    }
                } else {
                    /* remember the revision and try to find the newest one */
                    if (match_name) {
                        if ((file->d_name[len] != '@') ||
                                lysp_check_date(NULL, &file->d_name[len + 1],
                                flen - ((format_aux == LYS_IN_YANG) ? LY_YANG_SUFFIX_LEN : LY_YIN_SUFFIX_LEN) - len - 1, NULL)) {
                            continue;
                        } else if ((match_name[match_len] == '@') &&
                                (strncmp(&match_name[match_len + 1], &file->d_name[len + 1], LY_REV_SIZE - 1) >= 0)) {
                            continue;
                        }
                        free(match_name);
                    }

                    match_name = wn;
                    wn = NULL;
                    match_len = dir_len + 1 + len;
                    match_format = format_aux;
                    continue;
                }
            }
        }
    }

success:
    (*localfile) = match_name;
    match_name = NULL;
    if (format) {
        (*format) = match_format;
    }
    ret = LY_SUCCESS;

cleanup:
    free(wn);
    free(wd);
    if (dir) {
        closedir(dir);
    }
    free(match_name);
    ly_set_free(dirs, free);

    return ret;
}
