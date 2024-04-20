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
#include <sys/cdefs.h>

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
#include "schema_compile.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief information about YANG statements
 */
struct stmt_info_s stmt_attr_info[] = {
    [LY_STMT_NONE] = {NULL, NULL, 0},
    [LY_STMT_ACTION] = {"action", "name", STMT_FLAG_ID},
    [LY_STMT_ANYDATA] = {"anydata", "name", STMT_FLAG_ID},
    [LY_STMT_ANYXML] = {"anyxml", "name", STMT_FLAG_ID},
    [LY_STMT_ARGUMENT] = {"argument", "name", STMT_FLAG_ID},
    [LY_STMT_ARG_TEXT] = {"text", NULL, 0},
    [LY_STMT_ARG_VALUE] = {"value", NULL, 0},
    [LY_STMT_AUGMENT] = {"augment", "target-node", STMT_FLAG_ID},
    [LY_STMT_BASE] = {"base", "name", STMT_FLAG_ID},
    [LY_STMT_BELONGS_TO] = {"belongs-to", "module", STMT_FLAG_ID},
    [LY_STMT_BIT] = {"bit", "name", STMT_FLAG_ID},
    [LY_STMT_CASE] = {"case", "name", STMT_FLAG_ID},
    [LY_STMT_CHOICE] = {"choice", "name", STMT_FLAG_ID},
    [LY_STMT_CONFIG] = {"config", "value", STMT_FLAG_ID},
    [LY_STMT_CONTACT] = {"contact", "text", STMT_FLAG_YIN},
    [LY_STMT_CONTAINER] = {"container", "name", STMT_FLAG_ID},
    [LY_STMT_DEFAULT] = {"default", "value", 0},
    [LY_STMT_DESCRIPTION] = {"description", "text", STMT_FLAG_YIN},
    [LY_STMT_DEVIATE] = {"deviate", "value", STMT_FLAG_ID},
    [LY_STMT_DEVIATION] = {"deviation", "target-node", STMT_FLAG_ID},
    [LY_STMT_ENUM] = {"enum", "name", STMT_FLAG_ID},
    [LY_STMT_ERROR_APP_TAG] = {"error-app-tag", "value", 0},
    [LY_STMT_ERROR_MESSAGE] = {"error-message", "value", STMT_FLAG_YIN},
    [LY_STMT_EXTENSION] = {"extension", "name", STMT_FLAG_ID},
    [LY_STMT_EXTENSION_INSTANCE] = {NULL, NULL, 0},
    [LY_STMT_FEATURE] = {"feature", "name", STMT_FLAG_ID},
    [LY_STMT_FRACTION_DIGITS] = {"fraction-digits", "value", STMT_FLAG_ID},
    [LY_STMT_GROUPING] = {"grouping", "name", STMT_FLAG_ID},
    [LY_STMT_IDENTITY] = {"identity", "name", STMT_FLAG_ID},
    [LY_STMT_IF_FEATURE] = {"if-feature", "name", 0},
    [LY_STMT_IMPORT] = {"import", "module", STMT_FLAG_ID},
    [LY_STMT_INCLUDE] = {"include", "module", STMT_FLAG_ID},
    [LY_STMT_INPUT] = {"input", NULL, 0},
    [LY_STMT_KEY] = {"key", "value", 0},
    [LY_STMT_LEAF] = {"leaf", "name", STMT_FLAG_ID},
    [LY_STMT_LEAF_LIST] = {"leaf-list", "name", STMT_FLAG_ID},
    [LY_STMT_LENGTH] = {"length", "value", 0},
    [LY_STMT_LIST] = {"list", "name", STMT_FLAG_ID},
    [LY_STMT_MANDATORY] = {"mandatory", "value", STMT_FLAG_ID},
    [LY_STMT_MAX_ELEMENTS] = {"max-elements", "value", STMT_FLAG_ID},
    [LY_STMT_MIN_ELEMENTS] = {"min-elements", "value", STMT_FLAG_ID},
    [LY_STMT_MODIFIER] = {"modifier", "value", STMT_FLAG_ID},
    [LY_STMT_MODULE] = {"module", "name", STMT_FLAG_ID},
    [LY_STMT_MUST] = {"must", "condition", 0},
    [LY_STMT_NAMESPACE] = {"namespace", "uri", 0},
    [LY_STMT_NOTIFICATION] = {"notification", "name", STMT_FLAG_ID},
    [LY_STMT_ORDERED_BY] = {"ordered-by", "value", STMT_FLAG_ID},
    [LY_STMT_ORGANIZATION] = {"organization", "text", STMT_FLAG_YIN},
    [LY_STMT_OUTPUT] = {"output", NULL, 0},
    [LY_STMT_PATH] = {"path", "value", 0},
    [LY_STMT_PATTERN] = {"pattern", "value", 0},
    [LY_STMT_POSITION] = {"position", "value", STMT_FLAG_ID},
    [LY_STMT_PREFIX] = {"prefix", "value", STMT_FLAG_ID},
    [LY_STMT_PRESENCE] = {"presence", "value", 0},
    [LY_STMT_RANGE] = {"range", "value", 0},
    [LY_STMT_REFERENCE] = {"reference", "text", STMT_FLAG_YIN},
    [LY_STMT_REFINE] = {"refine", "target-node", STMT_FLAG_ID},
    [LY_STMT_REQUIRE_INSTANCE] = {"require-instance", "value", STMT_FLAG_ID},
    [LY_STMT_REVISION] = {"revision", "date", STMT_FLAG_ID},
    [LY_STMT_REVISION_DATE] = {"revision-date", "date", STMT_FLAG_ID},
    [LY_STMT_RPC] = {"rpc", "name", STMT_FLAG_ID},
    [LY_STMT_STATUS] = {"status", "value", STMT_FLAG_ID},
    [LY_STMT_SUBMODULE] = {"submodule", "name", STMT_FLAG_ID},
    [LY_STMT_SYNTAX_LEFT_BRACE] = {"{", NULL, 0},
    [LY_STMT_SYNTAX_RIGHT_BRACE] = {"}", NULL, 0},
    [LY_STMT_SYNTAX_SEMICOLON] = {";", NULL, 0},
    [LY_STMT_TYPE] = {"type", "name", STMT_FLAG_ID},
    [LY_STMT_TYPEDEF] = {"typedef", "name", STMT_FLAG_ID},
    [LY_STMT_UNIQUE] = {"unique", "tag", 0},
    [LY_STMT_UNITS] = {"units", "name", 0},
    [LY_STMT_USES] = {"uses", "name", STMT_FLAG_ID},
    [LY_STMT_VALUE] = {"value", "value", STMT_FLAG_ID},
    [LY_STMT_WHEN] = {"when", "condition", 0},
    [LY_STMT_YANG_VERSION] = {"yang-version", "value", STMT_FLAG_ID},
    [LY_STMT_YIN_ELEMENT] = {"yin-element", "value", STMT_FLAG_ID},
};

API const char *
ly_stmt2str(enum ly_stmt stmt)
{
    if (stmt == LY_STMT_EXTENSION_INSTANCE) {
        return "extension instance";
    } else {
        return stmt_attr_info[stmt].name;
    }
}

const char * const ly_devmod_list[] = {
    [LYS_DEV_NOT_SUPPORTED] = "not-supported",
    [LYS_DEV_ADD] = "add",
    [LYS_DEV_DELETE] = "delete",
    [LYS_DEV_REPLACE] = "replace",
};

API LY_ERR
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

API LY_ERR
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
 * ::lys_getnext_() is supposed to be called sequentially. In the first call, the \p last parameter is usually NULL
 * and function starts returning i) the first \p parent's child or ii) the first top level element specified in the
 * given extension (if provided) or iii) the first top level element of the \p module.
 * Consequent calls suppose to provide the previously returned node as the \p last parameter and still the same
 * \p parent and \p module parameters.
 *
 * Without options, the function is used to traverse only the schema nodes that can be paired with corresponding
 * data nodes in a data tree. By setting some \p options the behavior can be modified to the extent that
 * all the schema nodes are iteratively returned.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree where the function starts processing.
 * @param[in] module In case of iterating on top level elements, the \p parent is NULL and
 * module must be specified.
 * @param[in] ext The extension instance to provide a separate schema tree. To consider the top level elements in the tree,
 * the \p parent must be NULL. Anyway, at least one of @p parent, @p module and @p ext parameters must be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Next schema tree node that can be instantiated in a data tree, NULL in case there is no such element.
 */
static const struct lysc_node *
lys_getnext_(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_module *module,
        const struct lysc_ext_instance *ext, uint32_t options)
{
    const struct lysc_node *next = NULL;
    ly_bool action_flag = 0, notif_flag = 0;
    struct lysc_node **data_p = NULL;

    LY_CHECK_ARG_RET(NULL, parent || module || ext, NULL);

next:
    if (!last) {
        /* first call */

        /* get know where to start */
        if (parent) {
            /* schema subtree */
            next = last = lysc_node_child(parent);
        } else {
            /* top level data */
            if (ext) {
                lysc_ext_substmt(ext, LY_STMT_CONTAINER /* matches all nodes */, (void **)&data_p, NULL);
                next = last = data_p ? *data_p : NULL;
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
        /* possibly go back to parent */
        data_p = NULL;
        if (last && (last->parent != parent)) {
            last = last->parent;
            goto next;
        } else if (!action_flag) {
            action_flag = 1;
            if (ext) {
                lysc_ext_substmt(ext, LY_STMT_RPC /* matches also actions */, (void **)&data_p, NULL);
                next = data_p ? *data_p : NULL;
            } else if (parent) {
                next = (struct lysc_node *)lysc_node_actions(parent);
            } else {
                next = (struct lysc_node *)module->rpcs;
            }
        } else if (!notif_flag) {
            notif_flag = 1;
            if (ext) {
                lysc_ext_substmt(ext, LY_STMT_NOTIFICATION, (void **)&data_p, NULL);
                next = data_p ? *data_p : NULL;
            } else if (parent) {
                next = (struct lysc_node *)lysc_node_notifs(parent);
            } else {
                next = (struct lysc_node *)module->notifs;
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

API const struct lysc_node *
lys_getnext(const struct lysc_node *last, const struct lysc_node *parent, const struct lysc_module *module, uint32_t options)
{
    return lys_getnext_(last, parent, module, NULL, options);
}

API const struct lysc_node *
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

API const struct lysc_node *
lys_find_child(const struct lysc_node *parent, const struct lys_module *module, const char *name, size_t name_len,
        uint16_t nodetype, uint32_t options)
{
    const struct lysc_node *node = NULL;

    LY_CHECK_ARG_RET(NULL, module, name, NULL);
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

API LY_ERR
lys_find_xpath_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *xpath, uint32_t options,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set;
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx || ctx_node, xpath, set, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }
    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    memset(&xp_set, 0, sizeof xp_set);

    /* compile expression */
    ret = lyxp_expr_parse(ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(ctx, exp, NULL, LY_VALUE_JSON, NULL, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
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

API LY_ERR
lys_find_expr_atoms(const struct lysc_node *ctx_node, const struct lys_module *cur_mod, const struct lyxp_expr *expr,
        const struct lysc_prefix *prefixes, uint32_t options, struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, cur_mod, expr, prefixes, set, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }

    /* atomize expression */
    ret = lyxp_atomize(cur_mod->ctx, expr, cur_mod, LY_VALUE_SCHEMA_RESOLVED, (void *)prefixes, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
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

API LY_ERR
lys_find_xpath(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *xpath, uint32_t options,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_set xp_set = {0};
    struct lyxp_expr *exp = NULL;
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, ctx || ctx_node, xpath, set, LY_EINVAL);
    if (!(options & LYXP_SCNODE_ALL)) {
        options = LYXP_SCNODE;
    }
    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* compile expression */
    ret = lyxp_expr_parse(ctx, xpath, 0, 1, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* atomize expression */
    ret = lyxp_atomize(ctx, exp, NULL, LY_VALUE_JSON, NULL, ctx_node, &xp_set, options);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate return set */
    ret = ly_set_new(set);
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

API LY_ERR
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
        if (path[u].pred_type == LY_PATH_PREDTYPE_LIST) {
            LY_ARRAY_FOR(path[u].predicates, v) {
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

API LY_ERR
lys_find_path_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output,
        struct ly_set **set)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t oper;
    struct lyxp_expr *expr = NULL;
    struct ly_path *p = NULL;

    LY_CHECK_ARG_RET(ctx, ctx || ctx_node, path, set, LY_EINVAL);

    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* parse */
    ret = lyxp_expr_parse(ctx, path, strlen(path), 0, &expr);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile */
    oper = output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, expr, LY_PATH_LREF_FALSE, oper, LY_PATH_TARGET_MANY,
            LY_VALUE_JSON, NULL, NULL, &p);
    LY_CHECK_GOTO(ret, cleanup);

    /* resolve */
    ret = lys_find_lypath_atoms(p, set);

cleanup:
    ly_path_free(ctx, p);
    lyxp_expr_free(ctx, expr);
    return ret;
}

API const struct lysc_node *
lys_find_path(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path, ly_bool output)
{
    const struct lysc_node *snode = NULL;
    struct lyxp_expr *exp = NULL;
    struct ly_path *p = NULL;
    LY_ERR ret;
    uint8_t oper;

    LY_CHECK_ARG_RET(ctx, ctx || ctx_node, NULL);

    if (!ctx) {
        ctx = ctx_node->module->ctx;
    }

    /* parse */
    ret = lyxp_expr_parse(ctx, path, strlen(path), 0, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile */
    oper = output ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, exp, LY_PATH_LREF_FALSE, oper, LY_PATH_TARGET_MANY,
            LY_VALUE_JSON, NULL, NULL, &p);
    LY_CHECK_GOTO(ret, cleanup);

    /* get last node */
    snode = p[LY_ARRAY_COUNT(p) - 1].node;

cleanup:
    ly_path_free(ctx, p);
    lyxp_expr_free(ctx, exp);
    return snode;
}

char *
lysc_path_until(const struct lysc_node *node, const struct lysc_node *parent, LYSC_PATH_TYPE pathtype, char *buffer,
        size_t buflen)
{
    const struct lysc_node *iter;
    char *path = NULL;
    int len = 0;

    if (buffer) {
        LY_CHECK_ARG_RET(node->module->ctx, buflen > 1, NULL);
        buffer[0] = '\0';
    }

    switch (pathtype) {
    case LYSC_PATH_LOG:
    case LYSC_PATH_DATA:
        for (iter = node; iter && (iter != parent) && (len >= 0); iter = iter->parent) {
            char *s, *id;
            const char *slash;

            if ((pathtype == LYSC_PATH_DATA) && (iter->nodetype & (LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT))) {
                /* schema-only node */
                continue;
            }

            s = buffer ? strdup(buffer) : path;
            id = strdup(iter->name);
            if (parent && (iter->parent == parent)) {
                slash = "";
            } else {
                slash = "/";
            }
            if (!iter->parent || (iter->parent->module != iter->module)) {
                /* print prefix */
                if (buffer) {
                    len = snprintf(buffer, buflen, "%s%s:%s%s", slash, iter->module->name, id, s ? s : "");
                } else {
                    len = asprintf(&path, "%s%s:%s%s", slash, iter->module->name, id, s ? s : "");
                }
            } else {
                /* prefix is the same as in parent */
                if (buffer) {
                    len = snprintf(buffer, buflen, "%s%s%s", slash, id, s ? s : "");
                } else {
                    len = asprintf(&path, "%s%s%s", slash, id, s ? s : "");
                }
            }
            free(s);
            free(id);

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
        break;
    }

    if (buffer) {
        return buffer;
    } else {
        return path;
    }
}

API char *
lysc_path(const struct lysc_node *node, LYSC_PATH_TYPE pathtype, char *buffer, size_t buflen)
{
    return lysc_path_until(node, NULL, pathtype, buffer, buflen);
}

LY_ERR
lys_set_implemented_r(struct lys_module *mod, const char **features, struct lys_glob_unres *unres)
{
    struct lys_module *m;

    assert(!mod->implemented);

    /* we have module from the current context */
    m = ly_ctx_get_module_implemented(mod->ctx, mod->name);
    if (m) {
        assert(m != mod);

        /* check collision with other implemented revision */
        LOGERR(mod->ctx, LY_EDENIED, "Module \"%s%s%s\" is present in the context in other implemented revision (%s).",
                mod->name, mod->revision ? "@" : "", mod->revision ? mod->revision : "", m->revision ? m->revision : "none");
        return LY_EDENIED;
    }

    /* enable features */
    LY_CHECK_RET(lys_enable_features(mod->parsed, features));

    if (mod->ctx->flags & LY_CTX_EXPLICIT_COMPILE) {
        /* do not compile the module yet */
        mod->to_compile = 1;
        return LY_SUCCESS;
    }

    /* add the module into newly implemented module set */
    LY_CHECK_RET(ly_set_add(&unres->implementing, mod, 1, NULL));

    /* mark the module implemented, check for collision was already done */
    mod->implemented = 1;

    /* compile the schema */
    LY_CHECK_RET(lys_compile(mod, 0, unres));

    /* new module is implemented and compiled */
    unres->full_compilation = 0;

    return LY_SUCCESS;
}

API LY_ERR
lys_set_implemented(struct lys_module *mod, const char **features)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lys_glob_unres unres = {0};

    LY_CHECK_ARG_RET(NULL, mod, LY_EINVAL);

    if (mod->implemented) {
        /* mod is already implemented, set the features */
        r = lys_set_features(mod->parsed, features);
        if (r == LY_EEXIST) {
            /* no changes */
            return LY_SUCCESS;
        } else if (r) {
            /* error */
            return r;
        }

        if (mod->ctx->flags & LY_CTX_EXPLICIT_COMPILE) {
            /* just mark the module as changed */
            mod->to_compile = 1;
            return LY_SUCCESS;
        } else {
            /* full recompilation */
            return lys_recompile(mod->ctx, 1);
        }
    }

    /* implement this module and any other required modules, recursively */
    ret = lys_set_implemented_r(mod, features, &unres);

    /* the first module being implemented is finished, resolve global unres, consolidate the set */
    if (!ret) {
        ret = lys_compile_unres_glob(mod->ctx, &unres);
    }
    if (ret) {
        /* failure, full compile revert */
        lys_compile_unres_glob_revert(mod->ctx, &unres);
    }

    lys_compile_unres_glob_erase(mod->ctx, &unres);
    return ret;
}

static LY_ERR
lys_resolve_import_include(struct lys_parser_ctx *pctx, struct lysp_module *pmod)
{
    struct lysp_import *imp;
    LY_ARRAY_COUNT_TYPE u, v;

    pmod->parsing = 1;
    LY_ARRAY_FOR(pmod->imports, u) {
        imp = &pmod->imports[u];
        if (!imp->module) {
            LY_CHECK_RET(lys_load_module(PARSER_CTX(pctx), imp->name, imp->rev[0] ? imp->rev : NULL, 0, NULL,
                    pctx->unres, &imp->module));
        }
        /* check for importing the same module twice */
        for (v = 0; v < u; ++v) {
            if (imp->module == pmod->imports[v].module) {
                LOGWRN(PARSER_CTX(pctx), "Single revision of the module \"%s\" imported twice.", imp->name);
            }
        }
    }
    LY_CHECK_RET(lysp_load_submodules(pctx, pmod));

    pmod->parsing = 0;

    return LY_SUCCESS;
}

LY_ERR
lys_parse_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, struct lys_parser_ctx *main_ctx,
        LY_ERR (*custom_check)(const struct ly_ctx *, struct lysp_module *, struct lysp_submodule *, void *),
        void *check_data, struct lysp_submodule **submodule)
{
    LY_ERR ret;
    struct lysp_submodule *submod = NULL, *latest_sp;
    struct lys_yang_parser_ctx *yangctx = NULL;
    struct lys_yin_parser_ctx *yinctx = NULL;
    struct lys_parser_ctx *pctx;

    LY_CHECK_ARG_RET(ctx, ctx, in, LY_EINVAL);

    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_submodule(&yinctx, ctx, main_ctx, in, &submod);
        pctx = (struct lys_parser_ctx *)yinctx;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_submodule(&yangctx, ctx, main_ctx, in, &submod);
        pctx = (struct lys_parser_ctx *)yangctx;
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
    LY_CHECK_GOTO(ret = lys_resolve_import_include(pctx, (struct lysp_module *)submod), error);

    /* remap possibly changed and reallocated typedefs and groupings list back to the main context */
    memcpy(&main_ctx->tpdfs_nodes, &pctx->tpdfs_nodes, sizeof main_ctx->tpdfs_nodes);
    memcpy(&main_ctx->grps_nodes, &pctx->grps_nodes, sizeof main_ctx->grps_nodes);

    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
    }
    *submodule = submod;
    return LY_SUCCESS;

error:
    lysp_module_free((struct lysp_module *)submod);
    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
    }
    return ret;
}

/**
 * @brief Add ietf-netconf metadata to the parsed module. Operation, filter, and select are added.
 *
 * @param[in] mod Parsed module to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lys_parsed_add_internal_ietf_netconf(struct lysp_module *mod)
{
    struct lysp_ext_instance *ext_p;
    struct lysp_stmt *stmt;
    struct lysp_import *imp;

    /*
     * 1) edit-config's operation
     */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "operation", 0, &ext_p->argument));
    ext_p->format = LY_VALUE_SCHEMA;
    ext_p->prefix_data = mod;
    ext_p->flags = LYS_INTERNAL;
    ext_p->parent_stmt = LY_STMT_MODULE;
    ext_p->parent_stmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
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
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &ext_p->argument));
    ext_p->format = LY_VALUE_SCHEMA;
    ext_p->prefix_data = mod;
    ext_p->flags = LYS_INTERNAL;
    ext_p->parent_stmt = LY_STMT_MODULE;
    ext_p->parent_stmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
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
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "select", 0, &ext_p->argument));
    ext_p->format = LY_VALUE_SCHEMA;
    ext_p->prefix_data = mod;
    ext_p->flags = LYS_INTERNAL;
    ext_p->parent_stmt = LY_STMT_MODULE;
    ext_p->parent_stmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "yang_:xpath1.0", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_TYPE;

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
 * @param[in] mod Parsed module to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lys_parsed_add_internal_ietf_netconf_with_defaults(struct lysp_module *mod)
{
    struct lysp_ext_instance *ext_p;
    struct lysp_stmt *stmt;
    struct lysp_import *imp;

    /* add new extension instance */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->exts, ext_p, LY_EMEM);

    /* fill in the extension instance fields */
    LY_CHECK_ERR_RET(!ext_p, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_:annotation", 0, &ext_p->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "default", 0, &ext_p->argument));
    ext_p->format = LY_VALUE_SCHEMA;
    ext_p->prefix_data = mod;
    ext_p->flags = LYS_INTERNAL;
    ext_p->parent_stmt = LY_STMT_MODULE;
    ext_p->parent_stmt_index = 0;

    ext_p->child = stmt = calloc(1, sizeof *ext_p->child);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(mod->mod->ctx), LY_EMEM);
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "type", 0, &stmt->stmt));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "boolean", 0, &stmt->arg));
    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = mod;
    stmt->kw = LY_STMT_TYPE;

    /* create new import for the used prefix */
    LY_ARRAY_NEW_RET(mod->mod->ctx, mod->imports, imp, LY_EMEM);

    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "ietf-yang-metadata", 0, &imp->name));
    LY_CHECK_RET(lydict_insert(mod->mod->ctx, "md_", 0, &imp->prefix));
    imp->flags = LYS_INTERNAL;

    return LY_SUCCESS;
}

LY_ERR
lys_create_module(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, ly_bool need_implemented,
        LY_ERR (*custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod, void *data),
        void *check_data, const char **features, struct lys_glob_unres *unres, struct lys_module **module)
{
    struct lys_module *mod = NULL, *latest, *mod_dup, *mod_impl;
    struct lysp_submodule *submod;
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lys_yang_parser_ctx *yangctx = NULL;
    struct lys_yin_parser_ctx *yinctx = NULL;
    struct lys_parser_ctx *pctx = NULL;
    char *filename, *rev, *dot;
    size_t len;
    ly_bool implement;

    assert(ctx && in && (!features || need_implemented) && unres);

    if (module) {
        *module = NULL;
    }

    if (ctx->flags & LY_CTX_ALL_IMPLEMENTED) {
        implement = 1;
    } else {
        implement = need_implemented;
    }

    mod = calloc(1, sizeof *mod);
    LY_CHECK_ERR_RET(!mod, LOGMEM(ctx), LY_EMEM);
    mod->ctx = ctx;

    /* parse */
    switch (format) {
    case LYS_IN_YIN:
        ret = yin_parse_module(&yinctx, in, mod, unres);
        pctx = (struct lys_parser_ctx *)yinctx;
        break;
    case LYS_IN_YANG:
        ret = yang_parse_module(&yangctx, in, mod, unres);
        pctx = (struct lys_parser_ctx *)yangctx;
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        ret = LY_EINVAL;
        break;
    }
    LY_CHECK_GOTO(ret, free_mod_cleanup);

    /* make sure that the newest revision is at position 0 */
    lysp_sort_revisions(mod->parsed->revs);
    if (mod->parsed->revs) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx, mod->parsed->revs[0].date, 0, &mod->revision), free_mod_cleanup);
    }

    /* decide the latest revision */
    latest = (struct lys_module *)ly_ctx_get_module_latest(ctx, mod->name);
    if (latest) {
        if (mod->revision) {
            if (!latest->revision) {
                /* latest has no revision, so mod is anyway newer */
                mod->latest_revision = latest->latest_revision;
                /* the latest is zeroed later when the new module is being inserted into the context */
            } else if (strcmp(mod->revision, latest->revision) > 0) {
                mod->latest_revision = latest->latest_revision;
                /* the latest is zeroed later when the new module is being inserted into the context */
            } else {
                latest = NULL;
            }
        } else {
            latest = NULL;
        }
    } else {
        mod->latest_revision = 1;
    }

    if (custom_check) {
        LY_CHECK_GOTO(ret = custom_check(ctx, mod->parsed, NULL, check_data), free_mod_cleanup);
    }

    /* check whether it is not already in the context in the same revision */
    mod_dup = (struct lys_module *)ly_ctx_get_module(ctx, mod->name, mod->revision);
    if (implement) {
        mod_impl = ly_ctx_get_module_implemented(ctx, mod->name);
        if (mod_impl && (mod_impl != mod_dup)) {
            LOGERR(ctx, LY_EDENIED, "Module \"%s@%s\" is already implemented in the context.", mod_impl->name,
                    mod_impl->revision ? mod_impl->revision : "<none>");
            ret = LY_EDENIED;
            goto free_mod_cleanup;
        }
    }
    if (mod_dup) {
        if (implement) {
            if (!mod_dup->implemented) {
                /* just implement it */
                LY_CHECK_GOTO(ret = lys_set_implemented_r(mod_dup, features, unres), free_mod_cleanup);
                goto free_mod_cleanup;
            }

            /* nothing to do */
            LOGVRB("Module \"%s@%s\" is already implemented in the context.", mod_dup->name,
                    mod_dup->revision ? mod_dup->revision : "<none>");
            goto free_mod_cleanup;
        }

        /* nothing to do */
        LOGVRB("Module \"%s@%s\" is already present in the context.", mod_dup->name,
                mod_dup->revision ? mod_dup->revision : "<none>");
        goto free_mod_cleanup;
    }

    switch (in->type) {
    case LY_IN_FILEPATH:
        /* check that name and revision match filename */
        filename = strrchr(in->method.fpath.filepath, '/');
        if (!filename) {
            filename = in->method.fpath.filepath;
        } else {
            filename++;
        }
        rev = strchr(filename, '@');
        dot = strrchr(filename, '.');

        /* name */
        len = strlen(mod->name);
        if (strncmp(filename, mod->name, len) ||
                ((rev && (rev != &filename[len])) || (!rev && (dot != &filename[len])))) {
            LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, mod->name);
        }
        if (rev) {
            len = dot - ++rev;
            if (!mod->parsed->revs || (len != LY_REV_SIZE - 1) || strncmp(mod->parsed->revs[0].date, rev, len)) {
                LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                        mod->parsed->revs ? mod->parsed->revs[0].date : "none");
            }
        }

        break;
    case LY_IN_FD:
    case LY_IN_FILE:
    case LY_IN_MEMORY:
        /* nothing special to do */
        break;
    case LY_IN_ERROR:
        LOGINT(ctx);
        ret = LY_EINT;
        goto free_mod_cleanup;
    }
    lys_parser_fill_filepath(ctx, in, &mod->filepath);

    if (latest) {
        latest->latest_revision = 0;
    }

    /* add internal data in case specific modules were parsed */
    if (!strcmp(mod->name, "ietf-netconf")) {
        LY_CHECK_GOTO(ret = lys_parsed_add_internal_ietf_netconf(mod->parsed), free_mod_cleanup);
    } else if (!strcmp(mod->name, "ietf-netconf-with-defaults")) {
        LY_CHECK_GOTO(ret = lys_parsed_add_internal_ietf_netconf_with_defaults(mod->parsed), free_mod_cleanup);
    }

    /* add the module into newly created module set, will also be freed from there on any error */
    LY_CHECK_GOTO(ret = ly_set_add(&unres->creating, mod, 1, NULL), free_mod_cleanup);

    /* add into context */
    ret = ly_set_add(&ctx->list, mod, 1, NULL);
    LY_CHECK_GOTO(ret, cleanup);
    ctx->change_count++;

    /* resolve includes and all imports */
    LY_CHECK_GOTO(ret = lys_resolve_import_include(pctx, mod->parsed), cleanup);

    /* check name collisions */
    LY_CHECK_GOTO(ret = lysp_check_dup_typedefs(pctx, mod->parsed), cleanup);
    /* TODO groupings */
    LY_CHECK_GOTO(ret = lysp_check_dup_features(pctx, mod->parsed), cleanup);
    LY_CHECK_GOTO(ret = lysp_check_dup_identities(pctx, mod->parsed), cleanup);

    /* compile features */
    LY_CHECK_GOTO(ret = lys_compile_feature_iffeatures(mod->parsed), cleanup);

    /* pre-compile identities of the module and any submodules */
    LY_CHECK_GOTO(ret = lys_identity_precompile(NULL, ctx, mod->parsed, mod->parsed->identities, &mod->identities), cleanup);
    LY_ARRAY_FOR(mod->parsed->includes, u) {
        submod = mod->parsed->includes[u].submodule;
        ret = lys_identity_precompile(NULL, ctx, (struct lysp_module *)submod, submod->identities, &mod->identities);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (implement) {
        /* implement (compile) */
        LY_CHECK_GOTO(ret = lys_set_implemented_r(mod, features, unres), cleanup);
    }

    /* success */
    goto cleanup;

free_mod_cleanup:
    lys_module_free(mod);
    if (ret) {
        mod = NULL;
    } else {
        /* return the existing module */
        assert(mod_dup);
        mod = mod_dup;
    }

cleanup:
    if (pctx) {
        ly_set_erase(&pctx->tpdfs_nodes, NULL);
    }
    if (format == LYS_IN_YANG) {
        yang_parser_ctx_free(yangctx);
    } else {
        yin_parser_ctx_free(yinctx);
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

API LY_ERR
lys_parse(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, const char **features, const struct lys_module **module)
{
    LY_ERR ret;
    struct lys_glob_unres unres = {0};

    if (module) {
        *module = NULL;
    }
    LY_CHECK_ARG_RET(NULL, ctx, in, LY_EINVAL);

    format = lys_parse_get_format(in, format);
    LY_CHECK_ARG_RET(ctx, format, LY_EINVAL);

    /* remember input position */
    in->func_start = in->current;

    ret = lys_create_module(ctx, in, format, 1, NULL, NULL, features, &unres, (struct lys_module **)module);
    LY_CHECK_GOTO(ret, cleanup);

    /* resolve global unres */
    ret = lys_compile_unres_glob(ctx, &unres);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (ret) {
        lys_compile_unres_glob_revert(ctx, &unres);
    }
    lys_compile_unres_glob_erase(ctx, &unres);
    if (ret && module) {
        *module = NULL;
    }
    return ret;
}

API LY_ERR
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, data, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_memory(data, &in), LOGERR(ctx, ret, "Unable to create input handler."), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

API LY_ERR
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const struct lys_module **module)
{
    LY_ERR ret;
    struct ly_in *in = NULL;

    LY_CHECK_ARG_RET(ctx, fd > -1, format != LYS_IN_UNKNOWN, LY_EINVAL);

    LY_CHECK_ERR_RET(ret = ly_in_new_fd(fd, &in), LOGERR(ctx, ret, "Unable to create input handler."), ret);

    ret = lys_parse(ctx, in, format, NULL, module);
    ly_in_free(in, 0);

    return ret;
}

API LY_ERR
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, const struct lys_module **module)
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

API LY_ERR
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
