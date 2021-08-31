/**
 * @file validation.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Validation
 *
 * Copyright (c) 2019 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

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
 * @param[out] disabled First when that evaluated false, if any.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE if a referenced node does not have its when evaluated.
 * @return LY_ERR value on error.
 */
static LY_ERR
lyd_validate_node_when(const struct lyd_node *tree, const struct lyd_node *node, const struct lysc_node *schema,
        const struct lysc_when **disabled)
{
    LY_ERR ret;
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
            ret = lyxp_eval(LYD_CTX(node), when->cond, schema->module, LY_VALUE_SCHEMA_RESOLVED, when->prefixes,
                    ctx_node, tree, &xp_set, LYXP_SCHEMA);
            lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN);

            /* return error or LY_EINCOMPLETE for dependant unresolved when */
            LY_CHECK_RET(ret);

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
 * @brief Evaluate when conditions of collected unres nodes.
 *
 * @param[in,out] tree Data tree, is updated if some nodes are autodeleted.
 * @param[in] mod Module of the @p tree to take into consideration when deleting @p tree and moving it.
 * If set, it is expected @p tree should point to the first node of @p mod. Otherwise it will simply be
 * the first top-level sibling.
 * @param[in] node_when Set with nodes with "when" conditions.
 * @param[in,out] diff Validation diff.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value on error.
 */
static LY_ERR
lyd_validate_unres_when(struct lyd_node **tree, const struct lys_module *mod, struct ly_set *node_when,
        struct lyd_node **diff)
{
    LY_ERR ret;
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
        ret = lyd_validate_node_when(*tree, node, node->schema, &disabled);
        if (!ret) {
            if (disabled) {
                /* when false */
                if (node->flags & LYD_WHEN_TRUE) {
                    /* autodelete */
                    lyd_del_move_root(tree, node, mod);
                    if (diff) {
                        /* add into diff */
                        ret = lyd_val_diff_add(node, LYD_DIFF_OP_DELETE, diff);
                        LY_CHECK_GOTO(ret, error);
                    }
                    lyd_free_tree(node);
                } else {
                    /* invalid data */
                    LOGVAL(LYD_CTX(node), LY_VCODE_NOWHEN, disabled->cond->expr);
                    ret = LY_EVALID;
                    goto error;
                }
            } else {
                /* when true */
                node->flags |= LYD_WHEN_TRUE;
            }

            /* remove this node from the set, its when was resolved */
            ly_set_rm_index(node_when, i, NULL);
        } else if (ret != LY_EINCOMPLETE) {
            /* error */
            goto error;
        }

        LOG_LOCBACK(1, 1, 0, 0);
    } while (i);

    return LY_SUCCESS;

error:
    LOG_LOCBACK(1, 1, 0, 0);
    return ret;
}

struct node_ext {
    struct lyd_node *node;
    struct lysc_ext_instance *ext;
};

static LY_ERR
node_ext_tovalidate_add(struct ly_set *node_exts, struct lysc_ext_instance *exts, struct lyd_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_ext_instance *ext;
    struct node_ext *rec;

    LY_ARRAY_FOR(exts, struct lysc_ext_instance, ext) {
        if (ext->def->plugin && ext->def->plugin->validate) {
            rec = malloc(sizeof *rec);
            LY_CHECK_ERR_RET(!rec, LOGMEM(LYD_CTX(node)), LY_EMEM);
            rec->ext = ext;
            rec->node = node;

            ret = ly_set_add(node_exts, rec, 1, NULL);
            if (ret) {
                free(rec);
                break;
            }
        }
    }

    return ret;
}

LY_ERR
lysc_node_ext_tovalidate(struct ly_set *node_exts, struct lyd_node *node)
{
    const struct lysc_node *schema = node->schema;

    if (!schema) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    /* node's extensions */
    LY_CHECK_RET(node_ext_tovalidate_add(node_exts, schema->exts, node));

    if (schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        /* type's extensions */
        LY_CHECK_RET(node_ext_tovalidate_add(node_exts, ((struct lysc_node_leaf *)schema)->type->exts, node));
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_validate_unres(struct lyd_node **tree, const struct lys_module *mod, struct ly_set *node_when, struct ly_set *node_exts,
        struct ly_set *node_types, struct ly_set *meta_types, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t i;

    if (node_when) {
        /* evaluate all when conditions */
        uint32_t prev_count;
        do {
            prev_count = node_when->count;
            LY_CHECK_RET(lyd_validate_unres_when(tree, mod, node_when, diff));
            /* there must have been some when conditions resolved */
        } while (prev_count > node_when->count);

        /* there could have been no cyclic when dependencies, checked during compilation */
        assert(!node_when->count);
    }

    if (node_exts && node_exts->count) {
        i = node_exts->count;
        do {
            --i;

            struct node_ext *item = node_exts->objs[i];

            LOG_LOCSET(item->node->schema, item->node, NULL, NULL);
            ret = item->ext->def->plugin->validate(item->ext, item->node);
            LOG_LOCBACK(item->node->schema ? 1 : 0, 1, 0, 0);
            LY_CHECK_RET(ret);

            /* remove this node from the set */
            ly_set_rm_index(node_exts, i, free);
        } while (i);
    }

    if (node_types && node_types->count) {
        /* finish incompletely validated terminal values (traverse from the end for efficient set removal) */
        i = node_types->count;
        do {
            --i;

            struct lyd_node_term *node = node_types->objs[i];
            struct lysc_type *type = ((struct lysc_node_leaf *)node->schema)->type;

            /* resolve the value of the node */
            LOG_LOCSET(node->schema, &node->node, NULL, NULL);
            ret = lyd_value_validate_incomplete(LYD_CTX(node), type, &node->value, &node->node, *tree);
            LOG_LOCBACK(node->schema ? 1 : 0, 1, 0, 0);
            LY_CHECK_RET(ret);

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
            struct lysc_type *type = *(struct lysc_type **)meta->annotation->substmts[ANNOTATION_SUBSTMT_TYPE].storage;

            /* validate and store the value of the metadata */
            ret = lyd_value_validate_incomplete(LYD_CTX(meta->parent), type, &meta->value, meta->parent, *tree);
            LY_CHECK_RET(ret);

            /* remove this attr from the set */
            ly_set_rm_index(meta_types, i, NULL);
        } while (i);
    }

    return ret;
}

/**
 * @brief Validate instance duplication.
 *
 * @param[in] first First sibling to search in.
 * @param[in] node Data node instance to check.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_duplicates(const struct lyd_node *first, const struct lyd_node *node)
{
    struct lyd_node **match_p;
    ly_bool fail = 0;

    assert(node->flags & LYD_NEW);

    /* key-less list or non-configuration leaf-list */
    if (lysc_is_dup_inst_list(node->schema)) {
        /* duplicate instances allowed */
        return LY_SUCCESS;
    }

    /* find exactly the same next instance using hashes if possible */
    if (node->parent && node->parent->children_ht) {
        if (!lyht_find_next(node->parent->children_ht, &node, node->hash, (void **)&match_p)) {
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
        LOGVAL(node->schema->module->ctx, LY_VCODE_DUP, node->schema->name);
        return LY_EVALID;
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
 * @brief Properly delete a node as part of autodelete validation tasks.
 *
 * @param[in,out] first First sibling, is updated if needed.
 * @param[in] node Node instance to delete.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] next_p Temporary LY_LIST_FOR_SAFE next pointer, is updated if needed.
 * @param[in,out] diff Validation diff.
 */
static void
lyd_validate_autodel_node_del(struct lyd_node **first, struct lyd_node *node, const struct lys_module *mod,
        struct lyd_node **next_p, struct lyd_node **diff)
{
    struct lyd_node *iter;

    lyd_del_move_root(first, node, mod);
    if (node == *next_p) {
        *next_p = (*next_p)->next;
    }
    if (diff) {
        /* add into diff */
        if ((node->schema->nodetype == LYS_CONTAINER) && !(node->schema->flags & LYS_PRESENCE)) {
            /* we do not want to track NP container changes, but remember any removed children */
            LY_LIST_FOR(lyd_child(node), iter) {
                lyd_val_diff_add(iter, LYD_DIFF_OP_DELETE, diff);
            }
        } else {
            lyd_val_diff_add(node, LYD_DIFF_OP_DELETE, diff);
        }
    }
    lyd_free_tree(node);
}

/**
 * @brief Autodelete old instances to prevent validation errors.
 *
 * @param[in,out] first First sibling to search in, is updated if needed.
 * @param[in] node New data node instance to check.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] next_p Temporary LY_LIST_FOR_SAFE next pointer, is updated if needed.
 * @param[in,out] diff Validation diff.
 */
static void
lyd_validate_autodel_dup(struct lyd_node **first, struct lyd_node *node, const struct lys_module *mod,
        struct lyd_node **next_p, struct lyd_node **diff)
{
    struct lyd_node *match, *next;

    assert(node->flags & LYD_NEW);

    if (lyd_val_has_default(node->schema)) {
        assert(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_CONTAINER));
        LYD_LIST_FOR_INST_SAFE(*first, node->schema, next, match) {
            if ((match->flags & LYD_DEFAULT) && !(match->flags & LYD_NEW)) {
                /* default instance found, remove it */
                lyd_validate_autodel_node_del(first, match, mod, next_p, diff);

                /* remove only a single container/leaf default instance, if there are more, it is an error */
                if (node->schema->nodetype & (LYS_LEAF | LYS_CONTAINER)) {
                    break;
                }
            }
        }
    }
}

/**
 * @brief Autodelete leftover default nodes of deleted cases (that have no existing explicit data).
 *
 * @param[in,out] first First sibling to search in, is updated if needed.
 * @param[in] node Default data node instance to check.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] next_p Temporary LY_LIST_FOR_SAFE next pointer, is updated if needed.
 * @param[in,out] diff Validation diff.
 */
static void
lyd_validate_autodel_case_dflt(struct lyd_node **first, struct lyd_node *node, const struct lys_module *mod,
        struct lyd_node **next_p, struct lyd_node **diff)
{
    struct lysc_node_choice *choic;
    struct lyd_node *iter = NULL;
    const struct lysc_node *slast = NULL;

    assert(node->flags & LYD_DEFAULT);

    if (!node->schema->parent || (node->schema->parent->nodetype != LYS_CASE)) {
        /* the default node is not a descendant of a case */
        return;
    }

    choic = (struct lysc_node_choice *)node->schema->parent->parent;
    assert(choic->nodetype == LYS_CHOICE);

    if (choic->dflt && (choic->dflt == (struct lysc_node_case *)node->schema->parent)) {
        /* data of a default case, keep them */
        return;
    }

    /* try to find an explicit node of the case */
    while ((iter = lys_getnext_data(iter, *first, &slast, node->schema->parent, NULL))) {
        if (!(iter->flags & LYD_DEFAULT)) {
            break;
        }
    }

    if (!iter) {
        /* there are only default nodes of the case meaning it does not exist and neither should any default nodes
         * of the case, remove this one default node */
        lyd_validate_autodel_node_del(first, node, mod, next_p, diff);
    }
}

LY_ERR
lyd_validate_new(struct lyd_node **first, const struct lysc_node *sparent, const struct lys_module *mod,
        struct lyd_node **diff)
{
    struct lyd_node *next, *node;
    const struct lysc_node *snode = NULL;

    assert(first && (sparent || mod));

    while (*first && (snode = lys_getnext(snode, sparent, mod ? mod->compiled : NULL, LYS_GETNEXT_WITHCHOICE))) {
        /* check case duplicites */
        if (snode->nodetype == LYS_CHOICE) {
            LY_CHECK_RET(lyd_validate_cases(first, mod, (struct lysc_node_choice *)snode, diff));
        }
    }

    LY_LIST_FOR_SAFE(*first, next, node) {
        if (mod && (lyd_owner_module(node) != mod)) {
            /* all top-level data from this module checked */
            break;
        }

        if (!(node->flags & (LYD_NEW | LYD_DEFAULT))) {
            /* check only new and default nodes */
            continue;
        }

        LOG_LOCSET(node->schema, node, NULL, NULL);

        if (node->flags & LYD_NEW) {
            LY_ERR ret;

            /* remove old default(s) of the new node if it exists */
            lyd_validate_autodel_dup(first, node, mod, &next, diff);

            /* then check new node instance duplicities */
            ret = lyd_validate_duplicates(*first, node);
            LY_CHECK_ERR_RET(ret, LOG_LOCBACK(node->schema ? 1 : 0, 1, 0, 0), ret);

            /* this node is valid */
            node->flags &= ~LYD_NEW;
        }

        LOG_LOCBACK(node->schema ? 1 : 0, 1, 0, 0);

        if (node->flags & LYD_DEFAULT) {
            /* remove leftover default nodes from a no-longer existing case */
            lyd_validate_autodel_case_dflt(first, node, mod, &next, diff);
        }
    }

    return LY_SUCCESS;
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
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *tree, *dummy = NULL;

    /* find root */
    if (parent) {
        tree = (struct lyd_node *)parent;
        while (tree->parent) {
            tree = lyd_parent(tree);
        }
        tree = lyd_first_sibling(tree);
    } else {
        assert(!first || !first->prev->next);
        tree = (struct lyd_node *)first;
    }

    /* create dummy opaque node */
    ret = lyd_new_opaq((struct lyd_node *)parent, snode->module->ctx, snode->name, NULL, NULL, snode->module->name, &dummy);
    LY_CHECK_GOTO(ret, cleanup);

    /* connect it if needed */
    if (!parent) {
        if (first) {
            lyd_insert_sibling((struct lyd_node *)first, dummy, &tree);
        } else {
            assert(!tree);
            tree = dummy;
        }
    }

    /* evaluate all when */
    ret = lyd_validate_node_when(tree, dummy, snode, disabled);
    if (ret == LY_EINCOMPLETE) {
        /* all other when must be resolved by now */
        LOGINT(snode->module->ctx);
        ret = LY_EINT;
        goto cleanup;
    } else if (ret) {
        /* error */
        goto cleanup;
    }

cleanup:
    lyd_free_tree(dummy);
    return ret;
}

/**
 * @brief Validate mandatory node existence.
 *
 * @param[in] first First sibling to search in.
 * @param[in] parent Data parent.
 * @param[in] snode Schema node to validate.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_mandatory(const struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *snode)
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
        /* node instance not found */
        if (snode->nodetype == LYS_CHOICE) {
            LOGVAL(snode->module->ctx, LY_VCODE_NOMAND_CHOIC, snode->name);
        } else {
            LOGVAL(snode->module->ctx, LY_VCODE_NOMAND, snode->name);
        }
        return LY_EVALID;
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
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_minmax(const struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *snode,
        uint32_t min, uint32_t max)
{
    uint32_t count = 0;
    struct lyd_node *iter;
    const struct lysc_when *disabled;
    ly_bool invalid_instance = 0;

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
            LOG_LOCSET(NULL, iter, NULL, NULL);
            invalid_instance = 1;
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

        if (!disabled) {
            LOGVAL(snode->module->ctx, LY_VCODE_NOMIN, snode->name);
            goto failure;
        }
    } else if (max && (count > max)) {
        LOGVAL(snode->module->ctx, LY_VCODE_NOMAX, snode->name);
        goto failure;
    }

    return LY_SUCCESS;

failure:
    LOG_LOCBACK(0, invalid_instance, 0, 0);
    return LY_EVALID;
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
 * @brief Callback for comparing 2 list unique leaf values.
 *
 * Implementation of ::lyht_value_equal_cb.
 *
 * @param[in] cb_data 0 to compare all uniques, n to compare only n-th unique.
 */
static ly_bool
lyd_val_uniq_list_equal(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *cb_data)
{
    struct ly_ctx *ctx;
    struct lysc_node_list *slist;
    struct lyd_node *diter, *first, *second;
    struct lyd_value *val1, *val2;
    char *path1, *path2, *uniq_str, *ptr;
    LY_ARRAY_COUNT_TYPE u, v, action;

    assert(val1_p && val2_p);

    first = *((struct lyd_node **)val1_p);
    second = *((struct lyd_node **)val2_p);
    action = (LY_ARRAY_COUNT_TYPE)cb_data;

    assert(first && (first->schema->nodetype == LYS_LIST));
    assert(second && (second->schema == first->schema));

    ctx = first->schema->module->ctx;

    slist = (struct lysc_node_list *)first->schema;

    /* compare unique leaves */
    if (action > 0) {
        u = action - 1;
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
            /* all unique leafs are the same in this set, create this nice error */
            path1 = lyd_path(first, LYD_PATH_STD, NULL, 0);
            path2 = lyd_path(second, LYD_PATH_STD, NULL, 0);

            /* use buffer to rebuild the unique string */
#define UNIQ_BUF_SIZE 1024
            uniq_str = malloc(UNIQ_BUF_SIZE);
            uniq_str[0] = '\0';
            ptr = uniq_str;
            LY_ARRAY_FOR(slist->uniques[u], v) {
                if (v) {
                    strcpy(ptr, " ");
                    ++ptr;
                }
                ptr = lysc_path_until((struct lysc_node *)slist->uniques[u][v], &slist->node, LYSC_PATH_LOG,
                        ptr, UNIQ_BUF_SIZE - (ptr - uniq_str));
                if (!ptr) {
                    /* path will be incomplete, whatever */
                    break;
                }

                ptr += strlen(ptr);
            }
            LOG_LOCSET(NULL, second, NULL, NULL);
            LOGVAL(ctx, LY_VCODE_NOUNIQ, uniq_str, path1, path2);
            LOG_LOCBACK(0, 1, 0, 0);

            free(path1);
            free(path2);
            free(uniq_str);
#undef UNIQ_BUF_SIZE

            return 1;
        }

        if (action > 0) {
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
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_unique(const struct lyd_node *first, const struct lysc_node *snode, const struct lysc_node_leaf ***uniques)
{
    const struct lyd_node *diter;
    struct ly_set *set;
    LY_ARRAY_COUNT_TYPE u, v, x = 0;
    LY_ERR ret = LY_SUCCESS;
    uint32_t hash, i, size = 0;
    size_t key_len;
    ly_bool dyn;
    const void *hash_key;
    struct hash_table **uniqtables = NULL;
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
        if (lyd_val_uniq_list_equal(&set->objs[0], &set->objs[1], 0, (void *)0)) {
            /* instance duplication */
            ret = LY_EVALID;
            goto cleanup;
        }
    } else if (set->count > 2) {
        /* use hashes for comparison */
        /* first, allocate the table, the size depends on number of items in the set,
         * the following code detects number of upper zero bits in the items' counter value ... */
        for (i = (sizeof set->count * CHAR_BIT) - 1; i > 0; i--) {
            size = set->count << i;
            size = size >> i;
            if (size == set->count) {
                break;
            }
        }
        LY_CHECK_ERR_GOTO(!i, LOGINT(ctx); ret = LY_EINT, cleanup);
        /* ... and then we convert it to the position of the highest non-zero bit ... */
        i = (sizeof set->count * CHAR_BIT) - i;
        /* ... and by using it to shift 1 to the left we get the closest sufficient hash table size */
        size = 1 << i;

        uniqtables = malloc(LY_ARRAY_COUNT(uniques) * sizeof *uniqtables);
        LY_CHECK_ERR_GOTO(!uniqtables, LOGMEM(ctx); ret = LY_EMEM, cleanup);
        x = LY_ARRAY_COUNT(uniques);
        for (v = 0; v < x; v++) {
            uniqtables[v] = lyht_new(size, sizeof(struct lyd_node *), lyd_val_uniq_list_equal, (void *)(v + 1L), 0);
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
                    hash = dict_hash_multi(hash, hash_key, key_len);
                    if (dyn) {
                        free((void *)hash_key);
                    }
                }
                if (!val) {
                    /* skip this list instance since its unique set is incomplete */
                    continue;
                }

                /* finish the hash value */
                hash = dict_hash_multi(hash, NULL, 0);

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
        lyht_free(uniqtables[v]);
    }
    free(uniqtables);

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
    LY_ERR ret = LY_SUCCESS;
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

        LOG_LOCSET(snode, NULL, NULL, NULL);

        /* check min-elements and max-elements */
        if (snode->nodetype == LYS_LIST) {
            slist = (struct lysc_node_list *)snode;
            if (slist->min || slist->max) {
                ret = lyd_validate_minmax(first, parent, snode, slist->min, slist->max);
                LY_CHECK_GOTO(ret, error);
            }
        } else if (snode->nodetype == LYS_LEAFLIST) {
            sllist = (struct lysc_node_leaflist *)snode;
            if (sllist->min || sllist->max) {
                ret = lyd_validate_minmax(first, parent, snode, sllist->min, sllist->max);
                LY_CHECK_GOTO(ret, error);
            }

        } else if (snode->flags & LYS_MAND_TRUE) {
            /* check generic mandatory existence */
            ret = lyd_validate_mandatory(first, parent, snode);
            LY_CHECK_GOTO(ret, error);
        }

        /* check unique */
        if (snode->nodetype == LYS_LIST) {
            slist = (struct lysc_node_list *)snode;
            if (slist->uniques) {
                ret = lyd_validate_unique(first, snode, (const struct lysc_node_leaf ***)slist->uniques);
                LY_CHECK_GOTO(ret, error);
            }
        }

        if (snode->nodetype == LYS_CHOICE) {
            /* find the existing case, if any */
            LY_LIST_FOR(lysc_node_child(snode), scase) {
                if (lys_getnext_data(NULL, first, NULL, scase, NULL)) {
                    /* validate only this case */
                    ret = lyd_validate_siblings_schema_r(first, parent, scase, mod, val_opts, int_opts);
                    LY_CHECK_GOTO(ret, error);
                    break;
                }
            }
        }

        LOG_LOCBACK(1, 0, 0, 0);
    }

    return LY_SUCCESS;

error:
    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
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
            LOGWRN(snode->module->ctx, "Obsolete schema node \"%s\" instantiated in data.", snode->name);
            break;
        }

        snode = snode->parent;
    } while (snode && (snode->nodetype & (LYS_CHOICE | LYS_CASE)));
}

/**
 * @brief Validate must conditions of a data node.
 *
 * @param[in] node Node to validate.
 * @param[in] int_opts Internal parser options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_must(const struct lyd_node *node, uint32_t int_opts)
{
    struct lyxp_set xp_set;
    struct lysc_must *musts;
    const struct lyd_node *tree;
    const struct lysc_node *schema;
    LY_ARRAY_COUNT_TYPE u;

    assert((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_REPLY)) != (LYD_INTOPT_RPC | LYD_INTOPT_REPLY));
    assert((int_opts & (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) != (LYD_INTOPT_ACTION | LYD_INTOPT_REPLY));

    if (node->schema->nodetype & (LYS_ACTION | LYS_RPC)) {
        if (int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION)) {
            schema = &((struct lysc_node_action *)node->schema)->input.node;
        } else if (int_opts & LYD_INTOPT_REPLY) {
            schema = &((struct lysc_node_action *)node->schema)->output.node;
        } else {
            LOGINT(LYD_CTX(node));
            return LY_EINT;
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
        LY_CHECK_RET(lyxp_eval(LYD_CTX(node), musts[u].cond, node->schema->module, LY_VALUE_SCHEMA_RESOLVED,
                musts[u].prefixes, node, tree, &xp_set, LYXP_SCHEMA));

        /* check the result */
        lyxp_set_cast(&xp_set, LYXP_SET_BOOLEAN);
        if (!xp_set.val.bln) {
            LOGVAL(LYD_CTX(node), LY_VCODE_NOMUST, musts[u].cond->expr);
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
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
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_final_r(struct lyd_node *first, const struct lyd_node *parent, const struct lysc_node *sparent,
        const struct lys_module *mod, uint32_t val_opts, uint32_t int_opts)
{
    const char *innode = NULL;
    struct lyd_node *next = NULL, *node;

    /* validate all restrictions of nodes themselves */
    LY_LIST_FOR_SAFE(first, next, node) {
        if (mod && (lyd_owner_module(node) != mod)) {
            /* all top-level data from this module checked */
            break;
        }

        LOG_LOCSET(node->schema, node, NULL, NULL);

        /* opaque data */
        if (!node->schema) {
            LOGVAL(LYD_CTX(node), LYVE_DATA, "Opaque node \"%s\" found.", ((struct lyd_node_opaq *)node)->name.name);
            LOG_LOCBACK(0, 1, 0, 0);
            return LY_EVALID;
        }

        /* no state/input/output data */
        if ((val_opts & LYD_VALIDATE_NO_STATE) && (node->schema->flags & LYS_CONFIG_R)) {
            innode = "state";
            goto unexpected_node;
        } else if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION)) && (node->schema->flags & LYS_IS_OUTPUT)) {
            innode = "output";
            goto unexpected_node;
        } else if ((int_opts & LYD_INTOPT_REPLY) && (node->schema->flags & LYS_IS_INPUT)) {
            innode = "input";
            goto unexpected_node;
        }

        /* obsolete data */
        lyd_validate_obsolete(node);

        /* node's musts */
        LY_CHECK_RET(lyd_validate_must(node, int_opts));

        /* node value was checked by plugins */

        LOG_LOCBACK(1, 1, 0, 0);
    }

    /* validate schema-based restrictions */
    LY_CHECK_RET(lyd_validate_siblings_schema_r(first, parent, sparent, mod ? mod->compiled : NULL, val_opts, int_opts));

    LY_LIST_FOR(first, node) {
        /* validate all children recursively */
        LY_CHECK_RET(lyd_validate_final_r(lyd_child(node), node, node->schema, NULL, val_opts, int_opts));

        /* set default for containers */
        if (node->schema && (node->schema->nodetype == LYS_CONTAINER) && !(node->schema->flags & LYS_PRESENCE)) {
            LY_LIST_FOR(lyd_child(node), next) {
                if (!(next->flags & LYD_DEFAULT)) {
                    break;
                }
            }
            if (!next) {
                node->flags |= LYD_DEFAULT;
            }
        }
    }

    return LY_SUCCESS;

unexpected_node:
    LOGVAL(LYD_CTX(node), LY_VCODE_UNEXPNODE, innode, node->schema->name);
    LOG_LOCBACK(1, 1, 0, 0);
    return LY_EVALID;
}

/**
 * @brief Validate the whole data subtree.
 *
 * @param[in] root Subtree root.
 * @param[in,out] node_when Set for nodes with when conditions.
 * @param[in,out] node_exts Set for nodes and extension instances with validation plugin callback.
 * @param[in,out] node_types Set for unres node types.
 * @param[in,out] meta_types Set for unres metadata types.
 * @param[in] impl_opts Implicit options, see @ref implicitoptions.
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_validate_subtree(struct lyd_node *root, struct ly_set *node_when, struct ly_set *node_exts, struct ly_set *node_types,
        struct ly_set *meta_types, uint32_t impl_opts, struct lyd_node **diff)
{
    const struct lyd_meta *meta;
    struct lyd_node *node;

    LYD_TREE_DFS_BEGIN(root, node) {
        LY_LIST_FOR(node->meta, meta) {
            if ((*(const struct lysc_type **)meta->annotation->substmts[ANNOTATION_SUBSTMT_TYPE].storage)->plugin->validate) {
                /* metadata type resolution */
                LY_CHECK_RET(ly_set_add(meta_types, (void *)meta, 1, NULL));
            }
        }

        if ((node->schema->nodetype & LYD_NODE_TERM) && ((struct lysc_node_leaf *)node->schema)->type->plugin->validate) {
            /* node type resolution */
            LY_CHECK_RET(ly_set_add(node_types, (void *)node, 1, NULL));
        } else if (node->schema->nodetype & LYD_NODE_INNER) {
            /* new node validation, autodelete */
            LY_CHECK_RET(lyd_validate_new(lyd_node_child_p(node), node->schema, NULL, diff));

            /* add nested defaults */
            LY_CHECK_RET(lyd_new_implicit_r(node, lyd_node_child_p(node), NULL, NULL, NULL, NULL, NULL, impl_opts, diff));
        }

        if (lysc_has_when(node->schema)) {
            /* when evaluation */
            LY_CHECK_RET(ly_set_add(node_when, (void *)node, 1, NULL));
        }
        LY_CHECK_RET(lysc_node_ext_tovalidate(node_exts, node));

        LYD_TREE_DFS_END(root, node);
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_validate(struct lyd_node **tree, const struct lys_module *module, const struct ly_ctx *ctx, uint32_t val_opts,
        ly_bool validate_subtree, struct ly_set *node_when_p, struct ly_set *node_exts_p,
        struct ly_set *node_types_p, struct ly_set *meta_types_p, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *first, *next, **first2, *iter;
    const struct lys_module *mod;
    struct ly_set node_types = {0}, meta_types = {0}, node_when = {0}, node_exts = {0};
    uint32_t i = 0;

    assert(tree && ctx);
    assert((node_when_p && node_exts_p && node_types_p && meta_types_p) ||
            (!node_when_p && !node_exts_p && !node_types_p && !meta_types_p));

    if (!node_when_p) {
        node_when_p = &node_when;
        node_exts_p = &node_exts;
        node_types_p = &node_types;
        meta_types_p = &meta_types;
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
        ret = lyd_validate_new(first2, NULL, mod, diff);
        LY_CHECK_GOTO(ret, cleanup);

        /* add all top-level defaults for this module, if going to validate subtree, do not add into unres sets
         * (lyd_validate_subtree() adds all the nodes in that case) */
        ret = lyd_new_implicit_r(NULL, first2, NULL, mod, validate_subtree ? NULL : node_when_p, validate_subtree ? NULL : node_exts_p,
                validate_subtree ? NULL : node_types_p, (val_opts & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, diff);
        LY_CHECK_GOTO(ret, cleanup);

        /* our first module node pointer may no longer be the first */
        while (*first2 && (*first2)->prev->next && (lyd_owner_module(*first2) == lyd_owner_module((*first2)->prev))) {
            *first2 = (*first2)->prev;
        }

        if (validate_subtree) {
            /* process nested nodes */
            LY_LIST_FOR(*first2, iter) {
                ret = lyd_validate_subtree(iter, node_when_p, node_exts_p, node_types_p, meta_types_p,
                        (val_opts & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, diff);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }

        /* finish incompletely validated terminal values/attributes and when conditions */
        ret = lyd_validate_unres(first2, mod, node_when_p, node_exts_p, node_types_p, meta_types_p, diff);
        LY_CHECK_GOTO(ret, cleanup);

        /* perform final validation that assumes the data tree is final */
        ret = lyd_validate_final_r(*first2, NULL, NULL, mod, val_opts, 0);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    ly_set_erase(&node_when, NULL);
    ly_set_erase(&node_exts, NULL);
    ly_set_erase(&node_types, NULL);
    ly_set_erase(&meta_types, NULL);
    return ret;
}

API LY_ERR
lyd_validate_all(struct lyd_node **tree, const struct ly_ctx *ctx, uint32_t val_opts, struct lyd_node **diff)
{
    LY_CHECK_ARG_RET(NULL, tree, *tree || ctx, LY_EINVAL);
    if (!ctx) {
        ctx = LYD_CTX(*tree);
    }
    if (diff) {
        *diff = NULL;
    }

    return lyd_validate(tree, NULL, ctx, val_opts, 1, NULL, NULL, NULL, NULL, diff);
}

API LY_ERR
lyd_validate_module(struct lyd_node **tree, const struct lys_module *module, uint32_t val_opts, struct lyd_node **diff)
{
    LY_CHECK_ARG_RET(NULL, tree, *tree || module, LY_EINVAL);
    if (diff) {
        *diff = NULL;
    }

    return lyd_validate(tree, module, (*tree) ? LYD_CTX(*tree) : module->ctx, val_opts, 1, NULL, NULL, NULL, NULL, diff);
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
    struct lyd_node *match;
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
 * @param[in] validate_subtree Whether subtree was already validated (as part of data parsing) or not (separate validation).
 * @param[in] node_when_p Set of nodes with when conditions, if NULL a local set is used.
 * @param[in] node_exts Set of nodes with extension instances with validation plugin callback, if NULL a local set is used.
 * @param[in] node_types_p Set of unres node types, if NULL a local set is used.
 * @param[in] meta_types_p Set of unres metadata types, if NULL a local set is used.
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
static LY_ERR
_lyd_validate_op(struct lyd_node *op_tree, struct lyd_node *op_node, const struct lyd_node *dep_tree,
        uint32_t int_opts, ly_bool validate_subtree, struct ly_set *node_when_p, struct ly_set *node_exts_p,
        struct ly_set *node_types_p, struct ly_set *meta_types_p, struct lyd_node **diff)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_node *tree_sibling, *tree_parent, *op_subtree, *op_parent, *child;
    struct ly_set node_types = {0}, meta_types = {0}, node_when = {0}, node_exts = {0};

    assert(op_tree && op_node);
    assert((node_when_p && node_exts_p && node_types_p && meta_types_p) ||
            (!node_when_p && !node_exts_p && !node_types_p && !meta_types_p));

    if (!node_when_p) {
        node_when_p = &node_when;
        node_exts_p = &node_exts;
        node_types_p = &node_types;
        meta_types_p = &meta_types;
    }

    /* merge op_tree into dep_tree */
    lyd_val_op_merge_find(op_tree, op_node, dep_tree, &op_subtree, &tree_sibling, &tree_parent);
    op_parent = lyd_parent(op_subtree);
    lyd_unlink_tree(op_subtree);
    lyd_insert_node(tree_parent, &tree_sibling, op_subtree);
    if (!dep_tree) {
        dep_tree = tree_sibling;
    }

    LOG_LOCSET(NULL, op_node, NULL, NULL);

    if (int_opts & LYD_INTOPT_REPLY) {
        /* add output children defaults */
        rc = lyd_new_implicit_r(op_node, lyd_node_child_p(op_node), NULL, NULL, node_when_p, node_exts_p, node_types_p,
                LYD_IMPLICIT_OUTPUT, diff);
        LY_CHECK_GOTO(rc, cleanup);

        if (validate_subtree) {
            /* skip validating the operation itself, go to children directly */
            LY_LIST_FOR(lyd_child(op_node), child) {
                rc = lyd_validate_subtree(child, node_when_p, node_exts_p, node_types_p, meta_types_p, 0, diff);
                LY_CHECK_GOTO(rc, cleanup);
            }
        }
    } else {
        if (validate_subtree) {
            /* prevalidate whole operation subtree */
            rc = lyd_validate_subtree(op_node, node_when_p, node_exts_p, node_types_p, meta_types_p, 0, diff);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* finish incompletely validated terminal values/attributes and when conditions on the full tree */
    LY_CHECK_GOTO(rc = lyd_validate_unres((struct lyd_node **)&dep_tree, NULL,
            node_when_p, node_exts_p, node_types_p, meta_types_p, diff), cleanup);

    /* perform final validation of the operation/notification */
    lyd_validate_obsolete(op_node);
    LY_CHECK_GOTO(rc = lyd_validate_must(op_node, int_opts), cleanup);

    /* final validation of all the descendants */
    LY_CHECK_GOTO(rc = lyd_validate_final_r(lyd_child(op_node), op_node, op_node->schema, NULL, 0, int_opts), cleanup);

cleanup:
    LOG_LOCBACK(0, 1, 0, 0);
    /* restore operation tree */
    lyd_unlink_tree(op_subtree);
    if (op_parent) {
        lyd_insert_node(op_parent, NULL, op_subtree);
    }

    ly_set_erase(&node_when, NULL);
    ly_set_erase(&node_exts, NULL);
    ly_set_erase(&node_types, NULL);
    ly_set_erase(&meta_types, NULL);
    return rc;
}

API LY_ERR
lyd_validate_op(struct lyd_node *op_tree, const struct lyd_node *dep_tree, enum lyd_type data_type, struct lyd_node **diff)
{
    struct lyd_node *op_node;
    uint32_t int_opts;

    LY_CHECK_ARG_RET(NULL, op_tree, !op_tree->parent, !dep_tree || !dep_tree->parent, (data_type == LYD_TYPE_RPC_YANG) ||
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

    /* find the operation/notification */
    LYD_TREE_DFS_BEGIN(op_tree, op_node) {
        if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) &&
                (op_node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            break;
        } else if ((int_opts & LYD_INTOPT_NOTIF) && (op_node->schema->nodetype == LYS_NOTIF)) {
            break;
        }
        LYD_TREE_DFS_END(op_tree, op_node);
    }
    if (int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_REPLY)) {
        if (!(op_node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(LYD_CTX(op_tree), LY_EINVAL, "No RPC/action to validate found.");
            return LY_EINVAL;
        }
    } else {
        if (op_node->schema->nodetype != LYS_NOTIF) {
            LOGERR(LYD_CTX(op_tree), LY_EINVAL, "No notification to validate found.");
            return LY_EINVAL;
        }
    }

    /* validate */
    return _lyd_validate_op(op_tree, op_node, dep_tree, int_opts, 1, NULL, NULL, NULL, NULL, diff);
}
