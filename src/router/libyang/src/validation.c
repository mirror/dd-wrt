/**
 * @file validation.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Validation
 *
 * Copyright (c) 2019 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf, strdup */

#include "validation.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "diff.h"
#include "hash_table.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "plugins_exts/metadata.h"
#include "plugins_types.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Check validation error taking into account multi-error validation.
 *
 * @param[in] r Local return value.
 * @param[in] err_cmd Command to perform on any error.
 * @param[in] val_opts Validation options.
 * @param[in] label Label to go to on fatal error.
 */
#define LY_VAL_ERR_GOTO(r, err_cmd, val_opts, label) \
        if (r) { \
            err_cmd; \
            if ((r != LY_EVALID) || !(val_opts & LYD_VALIDATE_MULTI_ERROR)) { \
                goto label; \
            } \
        }

LY_ERR
lyd_val_diff_add(const struct lyd_node *node, enum lyd_diff_op op, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *new_diff = NULL;
    const struct lyd_node *prev_inst;
    char *key = NULL, *value = NULL, *position = NULL;
    size_t buflen = 0, bufused = 0;
    uint32_t pos;

    assert((op == LYD_DIFF_OP_DELETE) || (op == LYD_DIFF_OP_CREATE));

    if ((op == LYD_DIFF_OP_CREATE) && lysc_is_userordered(node->schema)) {
        if (lysc_is_dup_inst_list(node->schema)) {
            pos = lyd_list_pos(node);

            /* generate position meta */
            if (pos > 1) {
                if (asprintf(&position, "%" PRIu32, pos - 1) == -1) {
                    LOGMEM(LYD_CTX(node));
                    ret = LY_EMEM;
                    goto cleanup;
                }
            } else {
                position = strdup("");
                LY_CHECK_ERR_GOTO(!position, LOGMEM(LYD_CTX(node)); ret = LY_EMEM, cleanup);
            }
        } else {
            if (node->prev->next && (node->prev->schema == node->schema)) {
                prev_inst = node->prev;
            } else {
                /* first instance */
                prev_inst = NULL;
            }

            if (node->schema->nodetype == LYS_LIST) {
                /* generate key meta */
                if (prev_inst) {
                    LY_CHECK_GOTO(ret = lyd_path_list_predicate(prev_inst, &key, &buflen, &bufused, 0), cleanup);
                } else {
                    key = strdup("");
                    LY_CHECK_ERR_GOTO(!key, LOGMEM(LYD_CTX(node)); ret = LY_EMEM, cleanup);
                }
            } else {
                /* generate value meta */
                if (prev_inst) {
                    value = strdup(lyd_get_value(prev_inst));
                    LY_CHECK_ERR_GOTO(!value, LOGMEM(LYD_CTX(node)); ret = LY_EMEM, cleanup);
                } else {
                    value = strdup("");
                    LY_CHECK_ERR_GOTO(!value, LOGMEM(LYD_CTX(node)); ret = LY_EMEM, cleanup);
                }
            }
        }
    }

    /* create new diff tree */
    LY_CHECK_GOTO(ret = lyd_diff_add(node, op, NULL, NULL, key, value, position, NULL, NULL, &new_diff), cleanup);

    /* merge into existing diff */
    ret = lyd_diff_merge_all(diff, new_diff, 0);

cleanup:
    lyd_free_tree(new_diff);
    free(key);
    free(value);
    free(position);
    return ret;
}

/**
 * @brief Evaluate all relevant "when" conditions of a node.
 *
 * @param[in] tree Data tree.
 * @param[in] node Node whose relevant when conditions will be evaluated.
 * @param[in] schema Schema node of @p node. It may not be possible to use directly if @p node is opaque.
 * @param[in] xpath_options Additional XPath options to use.
 * @param[out] disabled First when that evaluated false, if any.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE if a referenced node does not have its when evaluated.
 * @return LY_ERR value on error.
 */
static LY_ERR
lyd_validate_node_when(const struct lyd_node *tree, const struct lyd_node *node, const struct lysc_node *schema,
        uint32_t xpath_options, const struct lysc_when **disabled)
{
    LY_ERR r;
    const struct lyd_node *ctx_node;
    struct lyxp_set xp_set;
    LY_ARRAY_COUNT_TYPE u;

    assert(!node->schema || (node->schema == schema));

    *disabled = NULL;

    do {
        const struct lysc_when *when;
        struct lysc_when **when_list = lysc_node_when(schema);

        LY_ARRAY_FOR(when_list, u) {
            when = when_list[u];

            /* get context node */
            if (when->context == schema) {
                ctx_node = node;
            } else {
                assert((!when->context && !node->parent) || (when->context == node->parent->schema));
                ctx_node = lyd_parent(node);
            }

            /* evaluate when */
            memset(&xp_set, 0, sizeof xp_set);
            r = lyxp_eval(LYD_CTX(node), when->cond, schema->module, LY_VALUE_SCHEMA_RESOLVED, when->prefixes,
                    ctx_node, ctx_node, tree, NULL, &xp_set, LYXP_SCHEMA | xpath_options);
            lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN);

            /* return error or LY_EINCOMPLETE for dependant unresolved when */
            LY_CHECK_RET(r);

            if (!xp_set.val.bln) {
                /* false when */
                *disabled = when;
                return LY_SUCCESS;
            }
        }

        schema = schema->parent;
    } while (schema && (schema->nodetype & (LYS_CASE | LYS_CHOICE)));

    return LY_SUCCESS;
}

/**
 * @brief Properly delete a node as part of auto-delete validation tasks.
 *
 * @param[in,out] first First sibling, is updated if needed.
 * @param[in] del Node instance to delete.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in] np_cont_diff Whether to put NP container into diff or only its children.
 * @param[in,out] node Optional current iteration node, update it if it is deleted.
 * @param[in,out] node_when Optional set with nodes with "when" conditions, may be removed from.
 * @param[in,out] diff Validation diff.
 * @return 1 if @p node auto-deleted and updated to its next sibling.
 * @return 0 if @p node was not auto-deleted.
 */
static ly_bool
lyd_validate_autodel_node_del(struct lyd_node **first, struct lyd_node *del, const struct lys_module *mod,
        int np_cont_diff, struct lyd_node **node, struct ly_set *node_types, struct lyd_node **diff)
{
    struct lyd_node *iter;
    ly_bool node_autodel = 0;
    uint32_t idx;

    /* update pointers */
    lyd_del_move_root(first, del, mod);
    if (node && (del == *node)) {
        *node = (*node)->next;
        node_autodel = 1;
    }

    if (diff) {
        /* add into diff */
        if (!np_cont_diff && (del->schema->nodetype == LYS_CONTAINER) && !(del->schema->flags & LYS_PRESENCE)) {
            /* we do not want to track NP container changes, but remember any removed children */
            LY_LIST_FOR(lyd_child(del), iter) {
                lyd_val_diff_add(iter, LYD_DIFF_OP_DELETE, diff);
            }
        } else {
            lyd_val_diff_add(del, LYD_DIFF_OP_DELETE, diff);
        }
    }

    if (node_types && node_types->count) {
        /* remove from node_types set */
        LYD_TREE_DFS_BEGIN(del, iter) {
            if (ly_set_contains(node_types, iter, &idx)) {
                ly_set_rm_index(node_types, idx, NULL);
            }
            LYD_TREE_DFS_END(del, iter);
        }
    }

    /* free */
    lyd_free_tree(del);

    return node_autodel;
}

/**
 * @brief Evaluate when conditions of collected unres nodes.
 *
 * @param[in,out] tree Data tree, is updated if some nodes are autodeleted.
 * @param[in] mod Module of the @p tree to take into consideration when deleting @p tree and moving it.
 * If set, it is expected @p tree should point to the first node of @p mod. Otherwise it will simply be
 * the first top-level sibling.
 * @param[in] node_when Set with nodes with "when" conditions.
 * @param[in] val_opts Validation options.
 * @param[in] xpath_options Additional XPath options to use.
 * @param[in,out] node_types Set with nodes with unresolved types, remove any with false "when" parents.
 * @param[in,out] diff Validation diff.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value on error.
 */
static LY_ERR
lyd_validate_unres_when(struct lyd_node **tree, const struct lys_module *mod, struct ly_set *node_when, uint32_t val_opts,
        uint32_t xpath_options, struct ly_set *node_types, struct lyd_node **diff)
{
    LY_ERR rc = LY_SUCCESS, r;
    uint32_t i;
    const struct lysc_when *disabled;
    struct lyd_node *node = NULL;

    if (!node_when->count) {
        return LY_SUCCESS;
    }

    i = node_when->count;
    do {
        --i;
        node = node_when->dnodes[i];
        LOG_LOCSET(node->schema, node, NULL, NULL);

        /* evaluate all when expressions that affect this node's existence */
        r = lyd_validate_node_when(*tree, node, node->schema, xpath_options, &disabled);
        if (!r) {
            if (disabled) {
                /* when false */
                if (node->flags & LYD_WHEN_TRUE) {
                    /* autodelete */
                    lyd_validate_autodel_node_del(tree, node, mod, 1, NULL, node_types, diff);
                } else if (val_opts & LYD_VALIDATE_OPERATIONAL) {
                    /* only a warning */
                    LOGWRN(LYD_CTX(node), "When condition \"%s\" not satisfied.", disabled->cond->expr);
                } else {
                    /* invalid data */
                    LOGVAL(LYD_CTX(node), LY_VCODE_NOWHEN, disabled->cond->expr);
                    r = LY_EVALID;
                    LY_VAL_ERR_GOTO(r, rc = r, val_opts, error);
                }
            } else {
                /* when true */
                node->flags |= LYD_WHEN_TRUE;
            }

            /* remove this node from the set keeping the order, its when was resolved */
            ly_set_rm_index_ordered(node_when, i, NULL);
        } else if (r != LY_EINCOMPLETE) {
            /* error */
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, error);
        }

        LOG_LOCBACK(1, 1, 0, 0);
    } while (i);

    return rc;

error:
    LOG_LOCBACK(1, 1, 0, 0);
    return rc;
}

LY_ERR
lyd_validate_unres(struct lyd_node **tree, const struct lys_module *mod, enum lyd_type data_type, struct ly_set *node_when,
        uint32_t when_xp_opts, struct ly_set *node_types, struct ly_set *meta_types, struct ly_set *ext_node,
        struct ly_set *ext_val, uint32_t val_opts, struct lyd_node **diff)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t i;

    if (ext_val && ext_val->count) {
        /* first validate parsed extension data */
        i = ext_val->count;
        do {
            --i;

            struct lyd_ctx_ext_val *ext_v = ext_val->objs[i];

            /* validate extension data */
            r = ext_v->ext->def->plugin->validate(ext_v->ext, ext_v->sibling, *tree, data_type, val_opts, diff);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* remove this item from the set */
            ly_set_rm_index(ext_val, i, free);
        } while (i);
    }

    if (ext_node && ext_node->count) {
        /* validate data nodes with extension instances */
        i = ext_node->count;
        do {
            --i;

            struct lyd_ctx_ext_node *ext_n = ext_node->objs[i];

            /* validate the node */
            r = ext_n->ext->def->plugin->node(ext_n->ext, ext_n->node, val_opts);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* remove this item from the set */
            ly_set_rm_index(ext_node, i, free);
        } while (i);
    }

    if (node_when) {
        /* evaluate all when conditions */
        uint32_t prev_count;

        do {
            prev_count = node_when->count;
            r = lyd_validate_unres_when(tree, mod, node_when, val_opts, when_xp_opts, node_types, diff);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* there must have been some when conditions resolved */
        } while (prev_count > node_when->count);

        /* there could have been no cyclic when dependencies, checked during compilation */
        assert(!node_when->count || ((rc == LY_EVALID) && (val_opts & LYD_VALIDATE_MULTI_ERROR)));
    }

    if (node_types && node_types->count) {
        /* finish incompletely validated terminal values (traverse from the end for efficient set removal) */
        i = node_types->count;
        do {
            --i;

            struct lyd_node_term *node = node_types->objs[i];
            struct lysc_type *type = ((struct lysc_node_leaf *)node->schema)->type;

            /* resolve the value of the node */
            LOG_LOCSET(NULL, &node->node, NULL, NULL);
            r = lyd_value_validate_incomplete(LYD_CTX(node), type, &node->value, &node->node, *tree);
            LOG_LOCBACK(0, 1, 0, 0);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* remove this node from the set */
            ly_set_rm_index(node_types, i, NULL);
        } while (i);
    }

    if (meta_types && meta_types->count) {
        /* ... and metadata values */
        i = meta_types->count;
        do {
            --i;

            struct lyd_meta *meta = meta_types->objs[i];
            struct lysc_type *type;

            /* validate and store the value of the metadata */
            lyplg_ext_get_storage(meta->annotation, LY_STMT_TYPE, sizeof type, (const void **)&type);
            r = lyd_value_validate_incomplete(LYD_CTX(meta->parent), type, &meta->value, meta->parent, *tree);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* remove this attr from the set */
            ly_set_rm_index(meta_types, i, NULL);
        } while (i);
    }

cleanup:
    return rc;
}

/**
 * @brief Validate instance duplication.
 *
 * @param[in] first First sibling to search in.
 * @param[in] node Data node instance to check.
 * @param[in] val_opts Validation options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_duplicates(const struct lyd_node *first, const struct lyd_node *node, uint32_t val_opts)
{
    struct lyd_node **match_p, *match;
    ly_bool fail = 0;

    assert(node->flags & LYD_NEW);

    /* key-less list or non-configuration leaf-list */
    if (lysc_is_dup_inst_list(node->schema)) {
        /* duplicate instances allowed */
        return LY_SUCCESS;
    }

    /* find exactly the same next instance using hashes if possible */
    if (node->parent && node->parent->children_ht) {
        lyd_find_sibling_first(first, node, &match);
        assert(match);

        if (match != node) {
            fail = 1;
        } else if (!lyht_find_next(node->parent->children_ht, &node, node->hash, (void **)&match_p)) {
            fail = 1;
        }
    } else {
        for ( ; first; first = first->next) {
            if (first == node) {
                continue;
            }

            if (node->schema->nodetype & (LYD_NODE_ANY | LYS_LEAF)) {
                if (first->schema == node->schema) {
                    fail = 1;
                    break;
                }
            } else if (!lyd_compare_single(first, node, 0)) {
                fail = 1;
                break;
            }
        }
    }

    if (fail) {
        if ((node->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) && (val_opts & LYD_VALIDATE_OPERATIONAL)) {
            /* only a warning */
            LOG_LOCSET(NULL, node, NULL, NULL);
            LOGWRN(node->schema->module->ctx, "Duplicate instance of \"%s\".", node->schema->name);
            LOG_LOCBACK(0, 1, 0, 0);
        } else {
            LOG_LOCSET(NULL, node, NULL, NULL);
            LOGVAL(node->schema->module->ctx, LY_VCODE_DUP, node->schema->name);
            LOG_LOCBACK(0, 1, 0, 0);
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Validate multiple case data existence with possible autodelete.
 *
 * @param[in,out] first First sibling to search in, is updated if needed.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in] choic Choice node whose cases to check.
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_cases(struct lyd_node **first, const struct lys_module *mod, const struct lysc_node_choice *choic,
        struct lyd_node **diff)
{
    const struct lysc_node *scase, *iter, *old_case = NULL, *new_case = NULL;
    struct lyd_node *match, *to_del;
    ly_bool found;

    LOG_LOCSET(&choic->node, NULL, NULL, NULL);

    LY_LIST_FOR((struct lysc_node *)choic->cases, scase) {
        found = 0;
        iter = NULL;
        match = NULL;
        while ((match = lys_getnext_data(match, *first, &iter, scase, NULL))) {
            if (match->flags & LYD_NEW) {
                /* a new case data found, nothing more to look for */
                found = 2;
                break;
            } else {
                /* and old case data found */
                if (found == 0) {
                    found = 1;
                }
            }
        }

        if (found == 1) {
            /* there should not be 2 old cases */
            if (old_case) {
                /* old data from 2 cases */
                LOGVAL(choic->module->ctx, LY_VCODE_DUPCASE, old_case->name, scase->name);
                LOG_LOCBACK(1, 0, 0, 0);
                return LY_EVALID;
            }

            /* remember an old existing case */
            old_case = scase;
        } else if (found == 2) {
            if (new_case) {
                /* new data from 2 cases */
                LOGVAL(choic->module->ctx, LY_VCODE_DUPCASE, new_case->name, scase->name);
                LOG_LOCBACK(1, 0, 0, 0);
                return LY_EVALID;
            }

            /* remember a new existing case */
            new_case = scase;
        }
    }

    LOG_LOCBACK(1, 0, 0, 0);

    if (old_case && new_case) {
        /* auto-delete old case */
        iter = NULL;
        match = NULL;
        to_del = NULL;
        while ((match = lys_getnext_data(match, *first, &iter, old_case, NULL))) {
            lyd_del_move_root(first, to_del, mod);

            /* free previous node */
            lyd_free_tree(to_del);
            if (diff) {
                /* add into diff */
                LY_CHECK_RET(lyd_val_diff_add(match, LYD_DIFF_OP_DELETE, diff));
            }
            to_del = match;
        }
        lyd_del_move_root(first, to_del, mod);
        lyd_free_tree(to_del);
    }

    return LY_SUCCESS;
}

/**
 * @brief Check whether a schema node can have some default values (true for NP containers as well).
 *
 * @param[in] schema Schema node to check.
 * @return non-zero if yes,
 * @return 0 otherwise.
 */
static int
lyd_val_has_default(const struct lysc_node *schema)
{
    switch (schema->nodetype) {
    case LYS_LEAF:
        if (((struct lysc_node_leaf *)schema)->dflt) {
            return 1;
        }
        break;
    case LYS_LEAFLIST:
        if (((struct lysc_node_leaflist *)schema)->dflts) {
            return 1;
        }
        break;
    case LYS_CONTAINER:
        if (!(schema->flags & LYS_PRESENCE)) {
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

/**
 * @brief Auto-delete leaf-list default instances to prevent validation errors.
 *
 * @param[in,out] first First sibling to search in, is updated if needed.
 * @param[in,out] node New data node instance to check, is updated if auto-deleted.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] diff Validation diff.
 * @return 1 if @p node auto-deleted and updated to its next sibling.
 * @return 0 if @p node was not auto-deleted.
 */
static ly_bool
lyd_validate_autodel_leaflist_dflt(struct lyd_node **first, struct lyd_node **node, const struct lys_module *mod,
        struct lyd_node **diff)
{
    const struct lysc_node *schema;
    struct lyd_node *iter, *next;
    ly_bool found = 0, node_autodel = 0;

    assert((*node)->flags & LYD_NEW);

    schema = (*node)->schema;
    assert(schema->nodetype == LYS_LEAFLIST);

    /* check whether there is any explicit instance */
    LYD_LIST_FOR_INST(*first, schema, iter) {
        if (!(iter->flags & LYD_DEFAULT)) {
            found = 1;
            break;
        }
    }
    if (!found) {
        /* no explicit instance, keep defaults as they are */
        return 0;
    }

    LYD_LIST_FOR_INST_SAFE(*first, schema, next, iter) {
        if (iter->flags & LYD_DEFAULT) {
            /* default instance found, remove it */
            if (lyd_validate_autodel_node_del(first, iter, mod, 0, node, NULL, diff)) {
                node_autodel = 1;
            }
        }
    }

    return node_autodel;
}

/**
 * @brief Auto-delete container or leaf default instances to prevent validation errors.
 *
 * @param[in,out] first First sibling to search in, is updated if needed.
 * @param[in,out] node New data node instance to check, is updated if auto-deleted.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] diff Validation diff.
 * @return 1 if @p node auto-deleted and updated to its next sibling.
 * @return 0 if @p node was not auto-deleted.
 */
static ly_bool
lyd_validate_autodel_cont_leaf_dflt(struct lyd_node **first, struct lyd_node **node, const struct lys_module *mod,
        struct lyd_node **diff)
{
    const struct lysc_node *schema;
    struct lyd_node *iter, *next;
    ly_bool found = 0, node_autodel = 0;

    assert((*node)->flags & LYD_NEW);

    schema = (*node)->schema;
    assert(schema->nodetype & (LYS_LEAF | LYS_CONTAINER));

    /* check whether there is any explicit instance */
    LYD_LIST_FOR_INST(*first, schema, iter) {
        if (!(iter->flags & LYD_DEFAULT)) {
            found = 1;
            break;
        }
    }

    if (found) {
        /* remove all default instances */
        LYD_LIST_FOR_INST_SAFE(*first, schema, next, iter) {
            if (iter->flags & LYD_DEFAULT) {
                /* default instance, remove it */
                if (lyd_validate_autodel_node_del(first, iter, mod, 0, node, NULL, diff)) {
                    node_autodel = 1;
                }
            }
        }
    } else {
        /* remove a single old default instance, if any */
        LYD_LIST_FOR_INST(*first, schema, iter) {
            if ((iter->flags & LYD_DEFAULT) && !(iter->flags & LYD_NEW)) {
                /* old default instance, remove it */
                if (lyd_validate_autodel_node_del(first, iter, mod, 0, node, NULL, diff)) {
                    node_autodel = 1;
                }
                break;
            }
        }
    }

    return node_autodel;
}

/**
 * @brief Auto-delete leftover default nodes of deleted cases (that have no existing explicit data).
 *
 * @param[in,out] first First sibling to search in, is updated if needed.
 * @param[in,out] node Default data node instance to check.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] diff Validation diff.
 * @return 1 if @p node auto-deleted and updated to its next sibling.
 * @return 0 if @p node was not auto-deleted.
 */
static ly_bool
lyd_validate_autodel_case_dflt(struct lyd_node **first, struct lyd_node **node, const struct lys_module *mod,
        struct lyd_node **diff)
{
    const struct lysc_node *schema;
    struct lysc_node_choice *choic;
    struct lyd_node *iter = NULL;
    const struct lysc_node *slast = NULL;
    ly_bool node_autodel = 0;

    assert((*node)->flags & LYD_DEFAULT);

    schema = (*node)->schema;

    if (!schema->parent || (schema->parent->nodetype != LYS_CASE)) {
        /* the default node is not a descendant of a case */
        return 0;
    }

    choic = (struct lysc_node_choice *)schema->parent->parent;
    assert(choic->nodetype == LYS_CHOICE);

    if (choic->dflt && (choic->dflt == (struct lysc_node_case *)schema->parent)) {
        /* data of a default case, keep them */
        return 0;
    }

    /* try to find an explicit node of the case */
    while ((iter = lys_getnext_data(iter, *first, &slast, schema->parent, NULL))) {
        if (!(iter->flags & LYD_DEFAULT)) {
            break;
        }
    }

    if (!iter) {
        /* there are only default nodes of the case meaning it does not exist and neither should any default nodes
         * of the case, remove this one default node */
        if (lyd_validate_autodel_node_del(first, *node, mod, 0, node, NULL, diff)) {
            node_autodel = 1;
        }
    }

    return node_autodel;
}

/**
 * @brief Validate new siblings in choices, recursively for nested choices.
 *
 * @param[in,out] first First sibling.
 * @param[in] sparent Schema parent of the siblings, NULL for top-level siblings.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in] val_opts Validation options.
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_choice_r(struct lyd_node **first, const struct lysc_node *sparent, const struct lys_module *mod,
        uint32_t val_opts, struct lyd_node **diff)
{
    LY_ERR r, rc = LY_SUCCESS;
    const struct lysc_node *snode = NULL;

    while (*first && (snode = lys_getnext(snode, sparent, mod ? mod->compiled : NULL, LYS_GETNEXT_WITHCHOICE))) {
        /* check case duplicites */
        if (snode->nodetype == LYS_CHOICE) {
            r = lyd_validate_cases(first, mod, (struct lysc_node_choice *)snode, diff);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* check for nested choice */
            r = lyd_validate_choice_r(first, snode, mod, val_opts, diff);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
        }
    }

cleanup:
    return rc;
}

LY_ERR
lyd_validate_new(struct lyd_node **first, const struct lysc_node *sparent, const struct lys_module *mod,
        uint32_t val_opts, struct lyd_node **diff)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_node *node;
    const struct lysc_node *last_dflt_schema = NULL;

    assert(first && (sparent || mod));

    /* validate choices */
    r = lyd_validate_choice_r(first, sparent, mod, val_opts, diff);
    LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

    node = *first;
    while (node) {
        if (!node->schema || (mod && (lyd_owner_module(node) != mod))) {
            /* opaque node or all top-level data from this module checked */
            break;
        }

        if (!(node->flags & (LYD_NEW | LYD_DEFAULT))) {
            /* check only new and default nodes */
            node = node->next;
            continue;
        }

        if (lyd_val_has_default(node->schema) && (node->schema != last_dflt_schema) && (node->flags & LYD_NEW)) {
            /* remove old default(s) of the new node if an explicit instance exists */
            last_dflt_schema = node->schema;
            if (node->schema->nodetype == LYS_LEAFLIST) {
                if (lyd_validate_autodel_leaflist_dflt(first, &node, mod, diff)) {
                    continue;
                }
            } else {
                if (lyd_validate_autodel_cont_leaf_dflt(first, &node, mod, diff)) {
                    continue;
                }
            }
        }

        if (node->flags & LYD_NEW) {
            /* then check new node instance duplicities */
            r = lyd_validate_duplicates(*first, node, val_opts);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* this node is valid */
            node->flags &= ~LYD_NEW;
        }

        if (node->flags & LYD_DEFAULT) {
            /* remove leftover default nodes from a no-longer existing case */
            if (lyd_validate_autodel_case_dflt(first, &node, mod, diff)) {
                continue;
            }
        }

        /* next iter */
        node = node->next;
    }

cleanup:
    return rc;
}

/**
 * @brief Evaluate any "when" conditions of a non-existent data node with existing parent.
 *
 * @param[in] first First data sibling of the non-existing node.
 * @param[in] parent Data parent of the non-existing node.
 * @param[in] snode Schema node of the non-existing node.
 * @param[out] disabled First when that evaluated false, if any.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_dummy_when(const struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *snode,
        const struct lysc_when **disabled)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *tree, *dummy = NULL;
    uint32_t xp_opts;

    /* find root */
    if (parent) {
        tree = (struct lyd_node *)parent;
        while (tree->parent) {
            tree = lyd_parent(tree);
        }
        tree = lyd_first_sibling(tree);
    } else {
        /* is the first sibling from the same module, but may not be the actual first */
        tree = lyd_first_sibling(first);
    }

    /* create dummy opaque node */
    rc = lyd_new_opaq((struct lyd_node *)parent, snode->module->ctx, snode->name, NULL, NULL, snode->module->name, &dummy);
    LY_CHECK_GOTO(rc, cleanup);

    /* connect it if needed */
    if (!parent) {
        if (first) {
            lyd_insert_sibling((struct lyd_node *)first, dummy, &tree);
        } else {
            assert(!tree);
            tree = dummy;
        }
    }

    /* explicitly specified accesible tree */
    if (snode->flags & LYS_CONFIG_W) {
        xp_opts = LYXP_ACCESS_TREE_CONFIG;
    } else {
        xp_opts = LYXP_ACCESS_TREE_ALL;
    }

    /* evaluate all when */
    rc = lyd_validate_node_when(tree, dummy, snode, xp_opts, disabled);
    if (rc == LY_EINCOMPLETE) {
        /* all other when must be resolved by now */
        LOGINT(snode->module->ctx);
        rc = LY_EINT;
        goto cleanup;
    } else if (rc) {
        /* error */
        goto cleanup;
    }

cleanup:
    lyd_free_tree(dummy);
    return rc;
}

/**
 * @brief Validate mandatory node existence.
 *
 * @param[in] first First sibling to search in.
 * @param[in] parent Data parent.
 * @param[in] snode Schema node to validate.
 * @param[in] val_opts Validation options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_mandatory(const struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *snode,
        uint32_t val_opts)
{
    const struct lysc_when *disabled;

    if (snode->nodetype == LYS_CHOICE) {
        /* some data of a choice case exist */
        if (lys_getnext_data(NULL, first, NULL, snode, NULL)) {
            return LY_SUCCESS;
        }
    } else {
        assert(snode->nodetype & (LYS_LEAF | LYS_CONTAINER | LYD_NODE_ANY));

        if (!lyd_find_sibling_val(first, snode, NULL, 0, NULL)) {
            /* data instance found */
            return LY_SUCCESS;
        }
    }

    disabled = NULL;
    if (lysc_has_when(snode)) {
        /* if there are any when conditions, they must be true for a validation error */
        LY_CHECK_RET(lyd_validate_dummy_when(first, parent, snode, &disabled));
    }

    if (!disabled) {
        if (val_opts & LYD_VALIDATE_OPERATIONAL) {
            /* only a warning */
            LOG_LOCSET(parent ? NULL : snode, parent, NULL, NULL);
            if (snode->nodetype == LYS_CHOICE) {
                LOGWRN(snode->module->ctx, "Mandatory choice \"%s\" data do not exist.", snode->name);
            } else {
                LOGWRN(snode->module->ctx, "Mandatory node \"%s\" instance does not exist.", snode->name);
            }
            LOG_LOCBACK(parent ? 0 : 1, parent ? 1 : 0, 0, 0);
        } else {
            /* node instance not found */
            LOG_LOCSET(parent ? NULL : snode, parent, NULL, NULL);
            if (snode->nodetype == LYS_CHOICE) {
                LOGVAL_APPTAG(snode->module->ctx, "missing-choice", LY_VCODE_NOMAND_CHOIC, snode->name);
            } else {
                LOGVAL(snode->module->ctx, LY_VCODE_NOMAND, snode->name);
            }
            LOG_LOCBACK(parent ? 0 : 1, parent ? 1 : 0, 0, 0);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Validate min/max-elements constraints, if any.
 *
 * @param[in] first First sibling to search in.
 * @param[in] parent Data parent.
 * @param[in] snode Schema node to validate.
 * @param[in] min Minimum number of elements, 0 for no restriction.
 * @param[in] max Max number of elements, 0 for no restriction.
 * @param[in] val_opts Validation options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_minmax(const struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *snode,
        uint32_t min, uint32_t max, uint32_t val_opts)
{
    uint32_t count = 0;
    struct lyd_node *iter;
    const struct lysc_when *disabled;

    assert(min || max);

    LYD_LIST_FOR_INST(first, snode, iter) {
        ++count;

        if (min && (count == min)) {
            /* satisfied */
            min = 0;
            if (!max) {
                /* nothing more to check */
                break;
            }
        }
        if (max && (count > max)) {
            /* not satisifed */
            break;
        }
    }

    if (min) {
        assert(count < min);

        disabled = NULL;
        if (lysc_has_when(snode)) {
            /* if there are any when conditions, they must be true for a validation error */
            LY_CHECK_RET(lyd_validate_dummy_when(first, parent, snode, &disabled));
        }

        if (disabled) {
            /* satisfied */
            min = 0;
        }
    }
    if (max && (count <= max)) {
        /* satisfied */
        max = 0;
    }

    if (min) {
        if (val_opts & LYD_VALIDATE_OPERATIONAL) {
            /* only a warning */
            LOG_LOCSET(snode, NULL, NULL, NULL);
            LOGWRN(snode->module->ctx, "Too few \"%s\" instances.", snode->name);
            LOG_LOCBACK(1, 0, 0, 0);
        } else {
            LOG_LOCSET(snode, NULL, NULL, NULL);
            LOGVAL_APPTAG(snode->module->ctx, "too-few-elements", LY_VCODE_NOMIN, snode->name);
            LOG_LOCBACK(1, 0, 0, 0);
            return LY_EVALID;
        }
    } else if (max) {
        if (val_opts & LYD_VALIDATE_OPERATIONAL) {
            /* only a warning */
            LOG_LOCSET(NULL, iter, NULL, NULL);
            LOGWRN(snode->module->ctx, "Too many \"%s\" instances.", snode->name);
            LOG_LOCBACK(0, 1, 0, 0);
        } else {
            LOG_LOCSET(NULL, iter, NULL, NULL);
            LOGVAL_APPTAG(snode->module->ctx, "too-many-elements", LY_VCODE_NOMAX, snode->name);
            LOG_LOCBACK(0, 1, 0, 0);
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Find node referenced by a list unique statement.
 *
 * @param[in] uniq_leaf Unique leaf to find.
 * @param[in] list List instance to use for the search.
 * @return Found leaf,
 * @return NULL if no leaf found.
 */
static struct lyd_node *
lyd_val_uniq_find_leaf(const struct lysc_node_leaf *uniq_leaf, const struct lyd_node *list)
{
    struct lyd_node *node;
    const struct lysc_node *iter;
    size_t depth = 0, i;

    /* get leaf depth */
    for (iter = &uniq_leaf->node; iter && (iter != list->schema); iter = lysc_data_parent(iter)) {
        ++depth;
    }

    node = (struct lyd_node *)list;
    while (node && depth) {
        /* find schema node with this depth */
        for (i = depth - 1, iter = &uniq_leaf->node; i; iter = lysc_data_parent(iter)) {
            --i;
        }

        /* find iter instance in children */
        assert(iter->nodetype & (LYS_CONTAINER | LYS_LEAF));
        lyd_find_sibling_val(lyd_child(node), iter, NULL, 0, &node);
        --depth;
    }

    return node;
}

/**
 * @brief Unique list validation callback argument.
 */
struct lyd_val_uniq_arg {
    LY_ARRAY_COUNT_TYPE action; /**< Action to perform - 0 to compare all uniques, n to compare only n-th unique. */
    uint32_t val_opts;          /**< Validation options. */
};

/**
 * @brief Callback for comparing 2 list unique leaf values.
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyd_val_uniq_list_equal(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *cb_data)
{
    struct ly_ctx *ctx;
    struct lysc_node_list *slist;
    struct lyd_node *diter, *first, *second;
    struct lyd_value *val1, *val2;
    char *path1, *path2, *uniq_str, *ptr;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lyd_val_uniq_arg *arg = cb_data;
    const uint32_t uniq_err_msg_size = 1024;

    assert(val1_p && val2_p);

    first = *((struct lyd_node **)val1_p);
    second = *((struct lyd_node **)val2_p);

    assert(first && (first->schema->nodetype == LYS_LIST));
    assert(second && (second->schema == first->schema));

    ctx = first->schema->module->ctx;

    slist = (struct lysc_node_list *)first->schema;

    /* compare unique leaves */
    if (arg->action > 0) {
        u = arg->action - 1;
        if (u < LY_ARRAY_COUNT(slist->uniques)) {
            goto uniquecheck;
        }
    }
    LY_ARRAY_FOR(slist->uniques, u) {
uniquecheck:
        LY_ARRAY_FOR(slist->uniques[u], v) {
            /* first */
            diter = lyd_val_uniq_find_leaf(slist->uniques[u][v], first);
            if (diter) {
                val1 = &((struct lyd_node_term *)diter)->value;
            } else {
                /* use default value */
                val1 = slist->uniques[u][v]->dflt;
            }

            /* second */
            diter = lyd_val_uniq_find_leaf(slist->uniques[u][v], second);
            if (diter) {
                val2 = &((struct lyd_node_term *)diter)->value;
            } else {
                /* use default value */
                val2 = slist->uniques[u][v]->dflt;
            }

            if (!val1 || !val2 || val1->realtype->plugin->compare(val1, val2)) {
                /* values differ or either one is not set */
                break;
            }
        }
        if (v && (v == LY_ARRAY_COUNT(slist->uniques[u]))) {
            /* all unique leaves are the same in this set, create this nice error */
            path1 = lyd_path(first, LYD_PATH_STD, NULL, 0);
            path2 = lyd_path(second, LYD_PATH_STD, NULL, 0);

            /* use buffer to rebuild the unique string */
            uniq_str = malloc(uniq_err_msg_size);
            uniq_str[0] = '\0';
            ptr = uniq_str;
            LY_ARRAY_FOR(slist->uniques[u], v) {
                if (v) {
                    strcpy(ptr, " ");
                    ++ptr;
                }
                ptr = lysc_path_until((struct lysc_node *)slist->uniques[u][v], &slist->node, LYSC_PATH_LOG,
                        ptr, uniq_err_msg_size - (ptr - uniq_str));
                if (!ptr) {
                    /* path will be incomplete, whatever */
                    break;
                }

                ptr += strlen(ptr);
            }
            LOG_LOCSET(NULL, second, NULL, NULL);
            if (arg->val_opts & LYD_VALIDATE_OPERATIONAL) {
                /* only a warning */
                LOGWRN(ctx, "Unique data leaf(s) \"%s\" not satisfied in \"%s\" and \"%s\".", uniq_str, path1, path2);
            } else {
                LOGVAL_APPTAG(ctx, "data-not-unique", LY_VCODE_NOUNIQ, uniq_str, path1, path2);
            }
            LOG_LOCBACK(0, 1, 0, 0);

            free(path1);
            free(path2);
            free(uniq_str);

            if (!(arg->val_opts & LYD_VALIDATE_OPERATIONAL)) {
                return 1;
            }
        }

        if (arg->action > 0) {
            /* done */
            return 0;
        }
    }

    return 0;
}

/**
 * @brief Validate list unique leaves.
 *
 * @param[in] first First sibling to search in.
 * @param[in] snode Schema node to validate.
 * @param[in] uniques List unique arrays to validate.
 * @param[in] val_opts Validation options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_unique(const struct lyd_node *first, const struct lysc_node *snode, const struct lysc_node_leaf ***uniques,
        uint32_t val_opts)
{
    const struct lyd_node *diter;
    struct ly_set *set;
    LY_ARRAY_COUNT_TYPE u, v, x = 0;
    LY_ERR ret = LY_SUCCESS;
    uint32_t hash, i;
    size_t key_len;
    ly_bool dyn;
    const void *hash_key;
    struct lyd_val_uniq_arg arg, *args = NULL;
    struct ly_ht **uniqtables = NULL;
    struct lyd_value *val;
    struct ly_ctx *ctx = snode->module->ctx;

    assert(uniques);

    /* get all list instances */
    LY_CHECK_RET(ly_set_new(&set));
    LY_LIST_FOR(first, diter) {
        if (diter->schema == snode) {
            ret = ly_set_add(set, (void *)diter, 1, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

    if (set->count == 2) {
        /* simple comparison */
        arg.action = 0;
        arg.val_opts = val_opts;
        if (lyd_val_uniq_list_equal(&set->objs[0], &set->objs[1], 0, &arg)) {
            /* instance duplication */
            ret = LY_EVALID;
            goto cleanup;
        }
    } else if (set->count > 2) {
        /* use hashes for comparison */
        uniqtables = malloc(LY_ARRAY_COUNT(uniques) * sizeof *uniqtables);
        args = malloc(LY_ARRAY_COUNT(uniques) * sizeof *args);
        LY_CHECK_ERR_GOTO(!uniqtables || !args, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        x = LY_ARRAY_COUNT(uniques);
        for (v = 0; v < x; v++) {
            args[v].action = v + 1;
            args[v].val_opts = val_opts;
            uniqtables[v] = lyht_new(lyht_get_fixed_size(set->count), sizeof(struct lyd_node *),
                    lyd_val_uniq_list_equal, &args[v], 0);
            LY_CHECK_ERR_GOTO(!uniqtables[v], LOGMEM(ctx); ret = LY_EMEM, cleanup);
        }

        for (i = 0; i < set->count; i++) {
            /* loop for unique - get the hash for the instances */
            for (u = 0; u < x; u++) {
                val = NULL;
                for (v = hash = 0; v < LY_ARRAY_COUNT(uniques[u]); v++) {
                    diter = lyd_val_uniq_find_leaf(uniques[u][v], set->objs[i]);
                    if (diter) {
                        val = &((struct lyd_node_term *)diter)->value;
                    } else {
                        /* use default value */
                        val = uniques[u][v]->dflt;
                    }
                    if (!val) {
                        /* unique item not present nor has default value */
                        break;
                    }

                    /* get hash key */
                    hash_key = val->realtype->plugin->print(NULL, val, LY_VALUE_LYB, NULL, &dyn, &key_len);
                    hash = lyht_hash_multi(hash, hash_key, key_len);
                    if (dyn) {
                        free((void *)hash_key);
                    }
                }
                if (!val) {
                    /* skip this list instance since its unique set is incomplete */
                    continue;
                }

                /* finish the hash value */
                hash = lyht_hash_multi(hash, NULL, 0);

                /* insert into the hashtable */
                ret = lyht_insert(uniqtables[u], &set->objs[i], hash, NULL);
                if (ret == LY_EEXIST) {
                    /* instance duplication */
                    ret = LY_EVALID;
                }
                LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
            }
        }
    }

cleanup:
    ly_set_free(set, NULL);
    for (v = 0; v < x; v++) {
        if (!uniqtables[v]) {
            /* failed when allocating uniquetables[j], following j are not allocated */
            break;
        }
        lyht_free(uniqtables[v], NULL);
    }
    free(uniqtables);
    free(args);

    return ret;
}

/**
 * @brief Validate data siblings based on generic schema node restrictions, recursively for schema-only nodes.
 *
 * @param[in] first First sibling to search in.
 * @param[in] parent Data parent.
 * @param[in] sparent Schema parent of the nodes to check.
 * @param[in] mod Module of the nodes to check.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[in] int_opts Internal parser options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_siblings_schema_r(const struct lyd_node *first, const struct lyd_node *parent,
        const struct lysc_node *sparent, const struct lysc_module *mod, uint32_t val_opts, uint32_t int_opts)
{
    LY_ERR r, rc = LY_SUCCESS;
    const struct lysc_node *snode = NULL, *scase;
    struct lysc_node_list *slist;
    struct lysc_node_leaflist *sllist;
    uint32_t getnext_opts;

    getnext_opts = LYS_GETNEXT_WITHCHOICE | (int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0);

    /* disabled nodes are skipped by lys_getnext */
    while ((snode = lys_getnext(snode, sparent, mod, getnext_opts))) {
        if ((val_opts & LYD_VALIDATE_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
            continue;
        }

        /* check min-elements and max-elements */
        if (snode->nodetype == LYS_LIST) {
            slist = (struct lysc_node_list *)snode;
            if (slist->min || slist->max) {
                r = lyd_validate_minmax(first, parent, snode, slist->min, slist->max, val_opts);
                LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
            }
        } else if (snode->nodetype == LYS_LEAFLIST) {
            sllist = (struct lysc_node_leaflist *)snode;
            if (sllist->min || sllist->max) {
                r = lyd_validate_minmax(first, parent, snode, sllist->min, sllist->max, val_opts);
                LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
            }

        } else if (snode->flags & LYS_MAND_TRUE) {
            /* check generic mandatory existence */
            r = lyd_validate_mandatory(first, parent, snode, val_opts);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
        }

        /* check unique */
        if (snode->nodetype == LYS_LIST) {
            slist = (struct lysc_node_list *)snode;
            if (slist->uniques) {
                r = lyd_validate_unique(first, snode, (const struct lysc_node_leaf ***)slist->uniques, val_opts);
                LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
            }
        }

        if (snode->nodetype == LYS_CHOICE) {
            /* find the existing case, if any */
            LY_LIST_FOR(lysc_node_child(snode), scase) {
                if (lys_getnext_data(NULL, first, NULL, scase, NULL)) {
                    /* validate only this case */
                    r = lyd_validate_siblings_schema_r(first, parent, scase, mod, val_opts, int_opts);
                    LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
                    break;
                }
            }
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Validate obsolete nodes, only warnings are printed.
 *
 * @param[in] node Node to check.
 */
static void
lyd_validate_obsolete(const struct lyd_node *node)
{
    const struct lysc_node *snode;

    snode = node->schema;
    do {
        if (snode->flags & LYS_STATUS_OBSLT) {
            LOG_LOCSET(NULL, node, NULL, NULL);
            LOGWRN(snode->module->ctx, "Obsolete schema node \"%s\" instantiated in data.", snode->name);
            LOG_LOCBACK(0, 1, 0, 0);
            break;
        }

        snode = snode->parent;
    } while (snode && (snode->nodetype & (LYS_CHOICE | LYS_CASE)));
}

/**
 * @brief Validate must conditions of a data node.
 *
 * @param[in] node Node to validate.
 * @param[in] val_opts Validation options.
 * @param[in] int_opts Internal parser options.
 * @param[in] xpath_options Additional XPath options to use.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_must(const struct lyd_node *node, uint32_t val_opts, uint32_t int_opts, uint32_t xpath_options)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyxp_set xp_set;
    struct lysc_must *musts;
    const struct lyd_node *tree;
    const struct lysc_node *schema;
    const char *emsg, *eapptag;
    LY_ARRAY_COUNT_TYPE u;

    assert((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_REPLY)) != (LYD_INTOPT_RPC | LYD_INTOPT_REPLY));
    assert((int_opts & (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) != (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY));

    if (node->schema->nodetype & (LYS_ACTION | LYS_RPC)) {
        if (int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION)) {
            schema = &((struct lysc_node_action *)node->schema)->input.node;
        } else if (int_opts & LYD_INTOPT_REPLY) {
            schema = &((struct lysc_node_action *)node->schema)->output.node;
        } else {
            LOGINT_RET(LYD_CTX(node));
        }
    } else {
        schema = node->schema;
    }
    musts = lysc_node_musts(schema);
    if (!musts) {
        /* no must to evaluate */
        return LY_SUCCESS;
    }

    /* find first top-level node */
    for (tree = node; tree->parent; tree = lyd_parent(tree)) {}
    tree = lyd_first_sibling(tree);

    LY_ARRAY_FOR(musts, u) {
        memset(&xp_set, 0, sizeof xp_set);

        /* evaluate must */
        r = lyxp_eval(LYD_CTX(node), musts[u].cond, node->schema->module, LY_VALUE_SCHEMA_RESOLVED,
                musts[u].prefixes, node, node, tree, NULL, &xp_set, LYXP_SCHEMA | xpath_options);
        if (r == LY_EINCOMPLETE) {
            LOGERR(LYD_CTX(node), LY_EINCOMPLETE,
                    "Must \"%s\" depends on a node with a when condition, which has not been evaluated.", musts[u].cond->expr);
        }
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* check the result */
        lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN);
        if (!xp_set.val.bln) {
            if (val_opts & LYD_VALIDATE_OPERATIONAL) {
                /* only a warning */
                emsg = musts[u].emsg;
                LOG_LOCSET(NULL, node, NULL, NULL);
                if (emsg) {
                    LOGWRN(LYD_CTX(node), "%s", emsg);
                } else {
                    LOGWRN(LYD_CTX(node), "Must condition \"%s\" not satisfied.", musts[u].cond->expr);
                }
                LOG_LOCBACK(0, 1, 0, 0);
            } else {
                /* use specific error information */
                emsg = musts[u].emsg;
                eapptag = musts[u].eapptag ? musts[u].eapptag : "must-violation";
                LOG_LOCSET(NULL, node, NULL, NULL);
                if (emsg) {
                    LOGVAL_APPTAG(LYD_CTX(node), eapptag, LYVE_DATA, "%s", emsg);
                } else {
                    LOGVAL_APPTAG(LYD_CTX(node), eapptag, LY_VCODE_NOMUST, musts[u].cond->expr);
                }
                LOG_LOCBACK(0, 1, 0, 0);
                r = LY_EVALID;
                LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
            }
        }
    }

cleanup:
    return rc;
}

/**
 * @brief Perform all remaining validation tasks, the data tree must be final when calling this function.
 *
 * @param[in] first First sibling.
 * @param[in] parent Data parent.
 * @param[in] sparent Schema parent of the siblings, NULL for top-level siblings.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in] val_opts Validation options (@ref datavalidationoptions).
 * @param[in] int_opts Internal parser options.
 * @param[in] must_xp_opts Additional XPath options to use for evaluating "must".
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_final_r(struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *sparent,
        const struct lys_module *mod, uint32_t val_opts, uint32_t int_opts, uint32_t must_xp_opts)
{
    LY_ERR r, rc = LY_SUCCESS;
    const char *innode;
    struct lyd_node *node;

    /* validate all restrictions of nodes themselves */
    LY_LIST_FOR(first, node) {
        if (node->flags & LYD_EXT) {
            /* ext instance data should have already been validated */
            continue;
        }

        /* opaque data */
        if (!node->schema) {
            r = lyd_parse_opaq_error(node);
            goto next_iter;
        }

        if (!node->parent && mod && (lyd_owner_module(node) != mod)) {
            /* all top-level data from this module checked */
            break;
        }

        /* no state/input/output/op data */
        innode = NULL;
        if ((val_opts & LYD_VALIDATE_NO_STATE) && (node->schema->flags & LYS_CONFIG_R)) {
            innode = "state";
        } else if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION)) && (node->schema->flags & LYS_IS_OUTPUT)) {
            innode = "output";
        } else if ((int_opts & LYD_INTOPT_REPLY) && (node->schema->flags & LYS_IS_INPUT)) {
            innode = "input";
        } else if (!(int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_REPLY)) && (node->schema->nodetype == LYS_RPC)) {
            innode = "rpc";
        } else if (!(int_opts & (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) && (node->schema->nodetype == LYS_ACTION)) {
            innode = "action";
        } else if (!(int_opts & LYD_INTOPT_NOTIF) && (node->schema->nodetype == LYS_NOTIF)) {
            innode = "notification";
        }
        if (innode) {
            LOG_LOCSET(NULL, node, NULL, NULL);
            LOGVAL(LYD_CTX(node), LY_VCODE_UNEXPNODE, innode, node->schema->name);
            LOG_LOCBACK(0, 1, 0, 0);
            r = LY_EVALID;
            goto next_iter;
        }

        /* obsolete data */
        lyd_validate_obsolete(node);

        /* node's musts */
        if ((r = lyd_validate_must(node, val_opts, int_opts, must_xp_opts))) {
            goto next_iter;
        }

        /* node value was checked by plugins */

next_iter:
        LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
    }

    /* validate schema-based restrictions */
    r = lyd_validate_siblings_schema_r(first, parent, sparent, mod ? mod->compiled : NULL, val_opts, int_opts);
    LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

    LY_LIST_FOR(first, node) {
        if (!node->schema || (!node->parent && mod && (lyd_owner_module(node) != mod))) {
            /* only opaque data following or all top-level data from this module checked */
            break;
        }

        /* validate all children recursively */
        r = lyd_validate_final_r(lyd_child(node), node, node->schema, NULL, val_opts, int_opts, must_xp_opts);
        LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

        /* set default for containers */
        lyd_cont_set_dflt(node);
    }

cleanup:
    return rc;
}

/**
 * @brief Validate extension instance data by storing it in its unres set.
 *
 * @param[in] sibling First sibling with ::LYD_EXT flag, all the following ones are expected to have it, too.
 * @param[in,out] ext_val Set with parsed extension instance data to validate.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_nested_ext(struct lyd_node *sibling, struct ly_set *ext_val)
{
    struct lyd_node *node;
    struct lyd_ctx_ext_val *ext_v;
    struct lysc_ext_instance *nested_exts, *ext = NULL;
    LY_ARRAY_COUNT_TYPE u;

    /* check of basic assumptions */
    if (!sibling->parent || !sibling->parent->schema) {
        LOGINT_RET(LYD_CTX(sibling));
    }
    LY_LIST_FOR(sibling, node) {
        if (!(node->flags & LYD_EXT)) {
            LOGINT_RET(LYD_CTX(sibling));
        }
    }

    /* try to find the extension instance */
    nested_exts = sibling->parent->schema->exts;
    LY_ARRAY_FOR(nested_exts, u) {
        if (nested_exts[u].def->plugin->validate) {
            if (ext) {
                /* more extension instances with validate callback */
                LOGINT_RET(LYD_CTX(sibling));
            }
            ext = &nested_exts[u];
        }
    }
    if (!ext) {
        /* no extension instance with validate callback */
        LOGINT_RET(LYD_CTX(sibling));
    }

    /* store for validation */
    ext_v = malloc(sizeof *ext_v);
    LY_CHECK_ERR_RET(!ext_v, LOGMEM(LYD_CTX(sibling)), LY_EMEM);
    ext_v->ext = ext;
    ext_v->sibling = sibling;
    LY_CHECK_RET(ly_set_add(ext_val, ext_v, 1, NULL));

    return LY_SUCCESS;
}

LY_ERR
lyd_validate_node_ext(struct lyd_node *node, struct ly_set *ext_node)
{
    struct lyd_ctx_ext_node *ext_n;
    struct lysc_ext_instance *exts;
    LY_ARRAY_COUNT_TYPE u;

    /* try to find a relevant extension instance with node callback */
    exts = node->schema->exts;
    LY_ARRAY_FOR(exts, u) {
        if (exts[u].def->plugin && exts[u].def->plugin->node) {
            /* store for validation */
            ext_n = malloc(sizeof *ext_n);
            LY_CHECK_ERR_RET(!ext_n, LOGMEM(LYD_CTX(node)), LY_EMEM);
            ext_n->ext = &exts[u];
            ext_n->node = node;
            LY_CHECK_RET(ly_set_add(ext_node, ext_n, 1, NULL));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Validate the whole data subtree.
 *
 * @param[in] root Subtree root.
 * @param[in,out] node_when Set for nodes with when conditions.
 * @param[in,out] node_types Set for unres node types.
 * @param[in,out] meta_types Set for unres metadata types.
 * @param[in,out] ext_node Set with nodes with extensions to validate.
 * @param[in,out] ext_val Set for parsed extension data to validate.
 * @param[in] val_opts Validation options.
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_subtree(struct lyd_node *root, struct ly_set *node_when, struct ly_set *node_types,
        struct ly_set *meta_types, struct ly_set *ext_node, struct ly_set *ext_val, uint32_t val_opts,
        struct lyd_node **diff)
{
    LY_ERR r, rc = LY_SUCCESS;
    const struct lyd_meta *meta;
    const struct lysc_type *type;
    struct lyd_node *node;
    uint32_t impl_opts;

    LYD_TREE_DFS_BEGIN(root, node) {
        if (node->flags & LYD_EXT) {
            /* validate using the extension instance callback */
            return lyd_validate_nested_ext(node, ext_val);
        }

        if (!node->schema) {
            /* do not validate opaque nodes */
            goto next_node;
        }

        LY_LIST_FOR(node->meta, meta) {
            lyplg_ext_get_storage(meta->annotation, LY_STMT_TYPE, sizeof type, (const void **)&type);
            if (type->plugin->validate) {
                /* metadata type resolution */
                r = ly_set_add(meta_types, (void *)meta, 1, NULL);
                LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
            }
        }

        if ((node->schema->nodetype & LYD_NODE_TERM) && ((struct lysc_node_leaf *)node->schema)->type->plugin->validate) {
            /* node type resolution */
            r = ly_set_add(node_types, (void *)node, 1, NULL);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        } else if (node->schema->nodetype & LYD_NODE_INNER) {
            /* new node validation, autodelete */
            r = lyd_validate_new(lyd_node_child_p(node), node->schema, NULL, val_opts, diff);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

            /* add nested defaults */
            impl_opts = 0;
            if (val_opts & LYD_VALIDATE_NO_STATE) {
                impl_opts |= LYD_IMPLICIT_NO_STATE;
            }
            if (val_opts & LYD_VALIDATE_NO_DEFAULTS) {
                impl_opts |= LYD_IMPLICIT_NO_DEFAULTS;
            }
            r = lyd_new_implicit_r(node, lyd_node_child_p(node), NULL, NULL, NULL, NULL, NULL, impl_opts, diff);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        }

        if (lysc_has_when(node->schema)) {
            /* when evaluation */
            r = ly_set_add(node_when, (void *)node, 1, NULL);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        }

        /* store for ext instance node validation, if needed */
        r = lyd_validate_node_ext(node, ext_node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

next_node:
        LYD_TREE_DFS_END(root, node);
    }

cleanup:
    return rc;
}

LY_ERR
lyd_validate(struct lyd_node **tree, const struct lys_module *module, const struct ly_ctx *ctx, uint32_t val_opts,
        ly_bool validate_subtree, struct ly_set *node_when_p, struct ly_set *node_types_p, struct ly_set *meta_types_p,
        struct ly_set *ext_node_p, struct ly_set *ext_val_p, struct lyd_node **diff)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_node *first, *next, **first2, *iter;
    const struct lys_module *mod;
    struct ly_set node_types = {0}, meta_types = {0}, node_when = {0}, ext_node = {0}, ext_val = {0};
    uint32_t i = 0, impl_opts;

    assert(tree && ctx);
    assert((node_when_p && node_types_p && meta_types_p && ext_node_p && ext_val_p) ||
            (!node_when_p && !node_types_p && !meta_types_p && !ext_node_p && !ext_val_p));

    if (!node_when_p) {
        node_when_p = &node_when;
        node_types_p = &node_types;
        meta_types_p = &meta_types;
        ext_node_p = &ext_node;
        ext_val_p = &ext_val;
    }

    next = *tree;
    while (1) {
        if (val_opts & LYD_VALIDATE_PRESENT) {
            mod = lyd_data_next_module(&next, &first);
        } else {
            mod = lyd_mod_next_module(next, module, ctx, &i, &first);
        }
        if (!mod) {
            break;
        }
        if (!first || (first == *tree)) {
            /* make sure first2 changes are carried to tree */
            first2 = tree;
        } else {
            first2 = &first;
        }

        /* validate new top-level nodes of this module, autodelete */
        r = lyd_validate_new(first2, *first2 ? lysc_data_parent((*first2)->schema) : NULL, mod, val_opts, diff);
        LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

        /* add all top-level defaults for this module, if going to validate subtree, do not add into unres sets
         * (lyd_validate_subtree() adds all the nodes in that case) */
        impl_opts = 0;
        if (val_opts & LYD_VALIDATE_NO_STATE) {
            impl_opts |= LYD_IMPLICIT_NO_STATE;
        }
        if (val_opts & LYD_VALIDATE_NO_DEFAULTS) {
            impl_opts |= LYD_IMPLICIT_NO_DEFAULTS;
        }
        r = lyd_new_implicit_r(lyd_parent(*first2), first2, NULL, mod, validate_subtree ? NULL : node_when_p,
                validate_subtree ? NULL : node_types_p, validate_subtree ? NULL : ext_node_p, impl_opts, diff);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* our first module node pointer may no longer be the first */
        first = *first2;
        lyd_first_module_sibling(&first, mod);
        if (!first || (first == *tree)) {
            first2 = tree;
        } else {
            first2 = &first;
        }

        if (validate_subtree) {
            /* process nested nodes */
            LY_LIST_FOR(*first2, iter) {
                if (lyd_owner_module(iter) != mod) {
                    break;
                }

                r = lyd_validate_subtree(iter, node_when_p, node_types_p, meta_types_p, ext_node_p, ext_val_p,
                        val_opts, diff);
                LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
            }
        }

        /* finish incompletely validated terminal values/attributes and when conditions */
        r = lyd_validate_unres(first2, mod, LYD_TYPE_DATA_YANG, node_when_p, 0, node_types_p, meta_types_p,
                ext_node_p, ext_val_p, val_opts, diff);
        LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

        if (!(val_opts & LYD_VALIDATE_NOT_FINAL)) {
            /* perform final validation that assumes the data tree is final */
            r = lyd_validate_final_r(*first2, NULL, NULL, mod, val_opts, 0, 0);
            LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);
        }
    }

cleanup:
    ly_set_erase(&node_when, NULL);
    ly_set_erase(&node_types, NULL);
    ly_set_erase(&meta_types, NULL);
    ly_set_erase(&ext_node, free);
    ly_set_erase(&ext_val, free);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_validate_all(struct lyd_node **tree, const struct ly_ctx *ctx, uint32_t val_opts, struct lyd_node **diff)
{
    LY_CHECK_ARG_RET(NULL, tree, *tree || ctx, LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(*tree ? LYD_CTX(*tree) : NULL, ctx, LY_EINVAL);
    if (!ctx) {
        ctx = LYD_CTX(*tree);
    }
    if (diff) {
        *diff = NULL;
    }

    return lyd_validate(tree, NULL, ctx, val_opts, 1, NULL, NULL, NULL, NULL, NULL, diff);
}

LIBYANG_API_DEF LY_ERR
lyd_validate_module(struct lyd_node **tree, const struct lys_module *module, uint32_t val_opts, struct lyd_node **diff)
{
    LY_CHECK_ARG_RET(NULL, tree, module, !(val_opts & LYD_VALIDATE_PRESENT), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(*tree ? LYD_CTX(*tree) : NULL, module->ctx, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }

    return lyd_validate(tree, module, module->ctx, val_opts, 1, NULL, NULL, NULL, NULL, NULL, diff);
}

LIBYANG_API_DEF LY_ERR
lyd_validate_module_final(struct lyd_node *tree, const struct lys_module *module, uint32_t val_opts)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_node *first;
    const struct lys_module *mod;
    uint32_t i = 0;

    LY_CHECK_ARG_RET(NULL, module, !(val_opts & (LYD_VALIDATE_PRESENT | LYD_VALIDATE_NOT_FINAL)), LY_EINVAL);
    LY_CHECK_CTX_EQUAL_RET(LYD_CTX(tree), module->ctx, LY_EINVAL);

    /* module is unchanged but we need to get the first module data node */
    mod = lyd_mod_next_module(tree, module, module->ctx, &i, &first);
    assert(mod);

    /* perform final validation that assumes the data tree is final */
    r = lyd_validate_final_r(first, NULL, NULL, mod, val_opts, 0, 0);
    LY_VAL_ERR_GOTO(r, rc = r, val_opts, cleanup);

cleanup:
    return rc;
}

/**
 * @brief Find nodes for merging an operation into data tree for validation.
 *
 * @param[in] op_tree Full operation data tree.
 * @param[in] op_node Operation node itself.
 * @param[in] tree Data tree to be merged into.
 * @param[out] op_subtree Operation subtree to merge.
 * @param[out] tree_sibling Data tree sibling to merge next to, is set if @p tree_parent is NULL.
 * @param[out] tree_parent Data tree parent to merge into, is set if @p tree_sibling is NULL.
 */
static void
lyd_val_op_merge_find(const struct lyd_node *op_tree, const struct lyd_node *op_node, const struct lyd_node *tree,
        struct lyd_node **op_subtree, struct lyd_node **tree_sibling, struct lyd_node **tree_parent)
{
    const struct lyd_node *tree_iter, *op_iter;
    struct lyd_node *match = NULL;
    uint32_t i, cur_depth, op_depth;

    *op_subtree = NULL;
    *tree_sibling = NULL;
    *tree_parent = NULL;

    /* learn op depth (top-level being depth 0) */
    op_depth = 0;
    for (op_iter = op_node; op_iter != op_tree; op_iter = lyd_parent(op_iter)) {
        ++op_depth;
    }

    /* find where to merge op */
    tree_iter = tree;
    cur_depth = op_depth;
    while (cur_depth && tree_iter) {
        /* find op iter in tree */
        lyd_find_sibling_first(tree_iter, op_iter, &match);
        if (!match) {
            break;
        }

        /* move tree_iter */
        tree_iter = lyd_child(match);

        /* move depth */
        --cur_depth;

        /* find next op parent */
        op_iter = op_node;
        for (i = 0; i < cur_depth; ++i) {
            op_iter = lyd_parent(op_iter);
        }
    }

    assert(op_iter);
    *op_subtree = (struct lyd_node *)op_iter;
    if (!tree || tree_iter) {
        /* there is no tree whatsoever or this is the last found sibling */
        *tree_sibling = (struct lyd_node *)tree_iter;
    } else {
        /* matching parent was found but it has no children to insert next to */
        assert(match);
        *tree_parent = match;
    }
}

/**
 * @brief Validate an RPC/action request, reply, or notification.
 *
 * @param[in] op_tree Full operation data tree.
 * @param[in] op_node Operation node itself.
 * @param[in] dep_tree Tree to be used for validating references from the operation subtree.
 * @param[in] int_opts Internal parser options.
 * @param[in] data_type Type of validated data.
 * @param[in] validate_subtree Whether subtree was already validated (as part of data parsing) or not (separate validation).
 * @param[in] node_when_p Set of nodes with when conditions, if NULL a local set is used.
 * @param[in] node_types_p Set of unres node types, if NULL a local set is used.
 * @param[in] meta_types_p Set of unres metadata types, if NULL a local set is used.
 * @param[in] ext_node_p Set of unres nodes with extensions to validate, if NULL a local set is used.
 * @param[in] ext_val_p Set of parsed extension data to validate, if NULL a local set is used.
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
static LY_ERR
_lyd_validate_op(struct lyd_node *op_tree, struct lyd_node *op_node, const struct lyd_node *dep_tree, enum lyd_type data_type,
        uint32_t int_opts, ly_bool validate_subtree, struct ly_set *node_when_p, struct ly_set *node_types_p,
        struct ly_set *meta_types_p,  struct ly_set *ext_node_p, struct ly_set *ext_val_p, struct lyd_node **diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *tree_sibling, *tree_parent, *op_subtree, *op_parent, *op_sibling_before, *op_sibling_after, *child;
    struct ly_set node_types = {0}, meta_types = {0}, node_when = {0}, ext_node = {0}, ext_val = {0};

    assert(op_tree && op_node);
    assert((node_when_p && node_types_p && meta_types_p && ext_node_p && ext_val_p) ||
            (!node_when_p && !node_types_p && !meta_types_p && !ext_node_p && !ext_val_p));

    if (!node_when_p) {
        node_when_p = &node_when;
        node_types_p = &node_types;
        meta_types_p = &meta_types;
        ext_node_p = &ext_node;
        ext_val_p = &ext_val;
    }

    /* merge op_tree into dep_tree */
    lyd_val_op_merge_find(op_tree, op_node, dep_tree, &op_subtree, &tree_sibling, &tree_parent);
    op_sibling_before = op_subtree->prev->next ? op_subtree->prev : NULL;
    op_sibling_after = op_subtree->next;
    op_parent = lyd_parent(op_subtree);

    lyd_unlink(op_subtree);
    lyd_insert_node(tree_parent, &tree_sibling, op_subtree, 0);
    if (!dep_tree) {
        dep_tree = tree_sibling;
    }

    if (int_opts & LYD_INTOPT_REPLY) {
        /* add output children defaults */
        rc = lyd_new_implicit_r(op_node, lyd_node_child_p(op_node), NULL, NULL, node_when_p, node_types_p,
                ext_node_p, LYD_IMPLICIT_OUTPUT, diff);
        LY_CHECK_GOTO(rc, cleanup);

        if (validate_subtree) {
            /* skip validating the operation itself, go to children directly */
            LY_LIST_FOR(lyd_child(op_node), child) {
                rc = lyd_validate_subtree(child, node_when_p, node_types_p, meta_types_p, ext_node_p, ext_val_p, 0, diff);
                LY_CHECK_GOTO(rc, cleanup);
            }
        }
    } else {
        if (validate_subtree) {
            /* prevalidate whole operation subtree */
            rc = lyd_validate_subtree(op_node, node_when_p, node_types_p, meta_types_p, ext_node_p, ext_val_p, 0, diff);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* finish incompletely validated terminal values/attributes and when conditions on the full tree,
     * account for unresolved 'when' that may appear in the non-validated dependency data tree */
    LY_CHECK_GOTO(rc = lyd_validate_unres((struct lyd_node **)&dep_tree, NULL, data_type, node_when_p, LYXP_IGNORE_WHEN,
            node_types_p, meta_types_p, ext_node_p, ext_val_p, 0, diff), cleanup);

    /* perform final validation of the operation/notification */
    lyd_validate_obsolete(op_node);
    LY_CHECK_GOTO(rc = lyd_validate_must(op_node, 0, int_opts, LYXP_IGNORE_WHEN), cleanup);

    /* final validation of all the descendants */
    rc = lyd_validate_final_r(lyd_child(op_node), op_node, op_node->schema, NULL, 0, int_opts, LYXP_IGNORE_WHEN);
    LY_CHECK_GOTO(rc, cleanup);

cleanup:
    /* restore operation tree */
    lyd_unlink(op_subtree);
    if (op_sibling_before) {
        lyd_insert_after_node(op_sibling_before, op_subtree);
        lyd_insert_hash(op_subtree);
    } else if (op_sibling_after) {
        lyd_insert_before_node(op_sibling_after, op_subtree);
        lyd_insert_hash(op_subtree);
    } else if (op_parent) {
        lyd_insert_node(op_parent, NULL, op_subtree, 0);
    }

    ly_set_erase(&node_when, NULL);
    ly_set_erase(&node_types, NULL);
    ly_set_erase(&meta_types, NULL);
    ly_set_erase(&ext_node, free);
    ly_set_erase(&ext_val, free);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyd_validate_op(struct lyd_node *op_tree, const struct lyd_node *dep_tree, enum lyd_type data_type, struct lyd_node **diff)
{
    struct lyd_node *op_node;
    uint32_t int_opts;
    struct ly_set ext_val = {0};
    LY_ERR rc;

    LY_CHECK_ARG_RET(NULL, op_tree, !dep_tree || !dep_tree->parent, (data_type == LYD_TYPE_RPC_YANG) ||
            (data_type == LYD_TYPE_NOTIF_YANG) || (data_type == LYD_TYPE_REPLY_YANG), LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }
    if (data_type == LYD_TYPE_RPC_YANG) {
        int_opts = LYD_INTOPT_RPC | LYD_INTOPT_ACTION;
    } else if (data_type == LYD_TYPE_NOTIF_YANG) {
        int_opts = LYD_INTOPT_NOTIF;
    } else {
        int_opts = LYD_INTOPT_REPLY;
    }

    if (op_tree->schema && (op_tree->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))) {
        /* we have the operation/notification, adjust the pointers */
        op_node = op_tree;
        while (op_tree->parent) {
            op_tree = lyd_parent(op_tree);
        }
    } else {
        /* find the operation/notification */
        while (op_tree->parent) {
            op_tree = lyd_parent(op_tree);
        }
        LYD_TREE_DFS_BEGIN(op_tree, op_node) {
            if (!op_node->schema) {
                return lyd_parse_opaq_error(op_node);
            } else if (op_node->flags & LYD_EXT) {
                /* fully validate the rest using the extension instance callback */
                LY_CHECK_RET(lyd_validate_nested_ext(op_node, &ext_val));
                rc = lyd_validate_unres((struct lyd_node **)&dep_tree, NULL, data_type, NULL, 0, NULL, NULL, NULL,
                        &ext_val, 0, diff);
                ly_set_erase(&ext_val, free);
                return rc;
            }

            if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) &&
                    (op_node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
                break;
            } else if ((int_opts & LYD_INTOPT_NOTIF) && (op_node->schema->nodetype == LYS_NOTIF)) {
                break;
            }
            LYD_TREE_DFS_END(op_tree, op_node);
        }
    }

    if (int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) {
        if (!op_node || !(op_node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(LYD_CTX(op_tree), LY_EINVAL, "No RPC/action to validate found.");
            return LY_EINVAL;
        }
    } else {
        if (!op_node || (op_node->schema->nodetype != LYS_NOTIF)) {
            LOGERR(LYD_CTX(op_tree), LY_EINVAL, "No notification to validate found.");
            return LY_EINVAL;
        }
    }

    /* validate */
    return _lyd_validate_op(op_tree, op_node, dep_tree, data_type, int_opts, 1, NULL, NULL, NULL, NULL, NULL, diff);
}
