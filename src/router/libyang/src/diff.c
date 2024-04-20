/**
 * @file diff.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief diff functions
 *
 * Copyright (c) 2020 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf, strdup */

#include "diff.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "log.h"
#include "plugins_exts.h"
#include "plugins_exts/metadata.h"
#include "plugins_types.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

#define LOGERR_META(ctx, meta_name, node) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Failed to find metadata \"%s\" for node \"%s\".", meta_name, __path); \
            free(__path); \
        }

#define LOGERR_NOINST(ctx, node) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Failed to find node \"%s\" instance in data.", __path); \
            free(__path); \
        }

#define LOGERR_UNEXPVAL(ctx, node, data_source) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Unexpected value of node \"%s\" in %s.", __path, data_source); \
            free(__path); \
        }

#define LOGERR_MERGEOP(ctx, node, src_op, trg_op) \
        { \
            char *__path = lyd_path(node, LYD_PATH_STD, NULL, 0); \
            LOGERR(ctx, LY_EINVAL, "Unable to merge operation \"%s\" with \"%s\" for node \"%s\".", \
                    lyd_diff_op2str(trg_op), lyd_diff_op2str(src_op), __path); \
            free(__path); \
        }

static const char *
lyd_diff_op2str(enum lyd_diff_op op)
{
    switch (op) {
    case LYD_DIFF_OP_CREATE:
        return "create";
    case LYD_DIFF_OP_DELETE:
        return "delete";
    case LYD_DIFF_OP_REPLACE:
        return "replace";
    case LYD_DIFF_OP_NONE:
        return "none";
    }

    LOGINT(NULL);
    return NULL;
}

static enum lyd_diff_op
lyd_diff_str2op(const char *str)
{
    switch (str[0]) {
    case 'c':
        assert(!strcmp(str, "create"));
        return LYD_DIFF_OP_CREATE;
    case 'd':
        assert(!strcmp(str, "delete"));
        return LYD_DIFF_OP_DELETE;
    case 'r':
        assert(!strcmp(str, "replace"));
        return LYD_DIFF_OP_REPLACE;
    case 'n':
        assert(!strcmp(str, "none"));
        return LYD_DIFF_OP_NONE;
    }

    LOGINT(NULL);
    return 0;
}

/**
 * @brief Create diff metadata for a nested user-ordered node with the effective operation "create".
 *
 * @param[in] node User-rodered node to update.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_add_create_nested_userord(struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    const char *meta_name, *meta_val;
    size_t buflen = 0, bufused = 0;
    uint32_t pos;
    char *dyn = NULL;

    assert(lysc_is_userordered(node->schema));

    /* get correct metadata name and value */
    if (lysc_is_dup_inst_list(node->schema)) {
        meta_name = "yang:position";

        pos = lyd_list_pos(node);
        if (asprintf(&dyn, "%" PRIu32, pos) == -1) {
            LOGMEM(LYD_CTX(node));
            rc = LY_EMEM;
            goto cleanup;
        }
        meta_val = dyn;
    } else if (node->schema->nodetype == LYS_LIST) {
        meta_name = "yang:key";

        if (node->prev->next && (node->prev->schema == node->schema)) {
            LY_CHECK_GOTO(rc = lyd_path_list_predicate(node->prev, &dyn, &buflen, &bufused, 0), cleanup);
            meta_val = dyn;
        } else {
            meta_val = "";
        }
    } else {
        meta_name = "yang:value";

        if (node->prev->next && (node->prev->schema == node->schema)) {
            meta_val = lyd_get_value(node->prev);
        } else {
            meta_val = "";
        }
    }

    /* create the metadata */
    LY_CHECK_GOTO(rc = lyd_new_meta(NULL, node, NULL, meta_name, meta_val, 0, NULL), cleanup);

cleanup:
    free(dyn);
    return rc;
}

/**
 * @brief Find metadata/an attribute of a node.
 *
 * @param[in] node Node to search.
 * @param[in] name Metadata/attribute name.
 * @param[out] meta Metadata found, NULL if not found.
 * @param[out] attr Attribute found, NULL if not found.
 */
static void
lyd_diff_find_meta(const struct lyd_node *node, const char *name, struct lyd_meta **meta, struct lyd_attr **attr)
{
    struct lyd_meta *m;
    struct lyd_attr *a;

    if (meta) {
        *meta = NULL;
    }
    if (attr) {
        *attr = NULL;
    }

    if (node->schema) {
        assert(meta);

        LY_LIST_FOR(node->meta, m) {
            if (!strcmp(m->name, name) && !strcmp(m->annotation->module->name, "yang")) {
                *meta = m;
                break;
            }
        }
    } else {
        assert(attr);

        LY_LIST_FOR(((struct lyd_node_opaq *)node)->attr, a) {
            /* name */
            if (strcmp(a->name.name, name)) {
                continue;
            }

            /* module */
            switch (a->format) {
            case LY_VALUE_JSON:
                if (strcmp(a->name.module_name, "yang")) {
                    continue;
                }
                break;
            case LY_VALUE_XML:
                if (strcmp(a->name.module_ns, "urn:ietf:params:xml:ns:yang:1")) {
                    continue;
                }
                break;
            default:
                LOGINT(LYD_CTX(node));
                return;
            }

            *attr = a;
            break;
        }
    }
}

/**
 * @brief Learn operation of a diff node.
 *
 * @param[in] diff_node Diff node.
 * @param[out] op Operation.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_get_op(const struct lyd_node *diff_node, enum lyd_diff_op *op)
{
    struct lyd_meta *meta = NULL;
    struct lyd_attr *attr = NULL;
    const struct lyd_node *diff_parent;
    const char *str;
    char *path;

    for (diff_parent = diff_node; diff_parent; diff_parent = lyd_parent(diff_parent)) {
        lyd_diff_find_meta(diff_parent, "operation", &meta, &attr);
        if (!meta && !attr) {
            continue;
        }

        str = meta ? lyd_get_meta_value(meta) : attr->value;
        if ((str[0] == 'r') && (diff_parent != diff_node)) {
            /* we do not care about this operation if it's in our parent */
            continue;
        }
        *op = lyd_diff_str2op(str);
        return LY_SUCCESS;
    }

    /* operation not found */
    path = lyd_path(diff_node, LYD_PATH_STD, NULL, 0);
    LOGERR(LYD_CTX(diff_node), LY_EINVAL, "Node \"%s\" without an operation.", path);
    free(path);
    return LY_EINT;
}

/**
 * @brief Remove metadata/an attribute from a node.
 *
 * @param[in] node Node to update.
 * @param[in] name Metadata/attribute name.
 */
static void
lyd_diff_del_meta(struct lyd_node *node, const char *name)
{
    struct lyd_meta *meta;
    struct lyd_attr *attr;

    lyd_diff_find_meta(node, name, &meta, &attr);

    if (meta) {
        lyd_free_meta_single(meta);
    } else if (attr) {
        lyd_free_attr_single(LYD_CTX(node), attr);
    }
}

LY_ERR
lyd_diff_add(const struct lyd_node *node, enum lyd_diff_op op, const char *orig_default, const char *orig_value,
        const char *key, const char *value, const char *position, const char *orig_key, const char *orig_position,
        struct lyd_node **diff)
{
    struct lyd_node *dup, *siblings, *match = NULL, *diff_parent = NULL, *elem;
    const struct lyd_node *parent = NULL;
    enum lyd_diff_op cur_op;
    struct lyd_meta *meta;
    uint32_t diff_opts;

    assert(diff);

    /* replace leaf always needs orig-default and orig-value */
    assert((node->schema->nodetype != LYS_LEAF) || (op != LYD_DIFF_OP_REPLACE) || (orig_default && orig_value));

    /* create on userord needs key/value */
    assert((node->schema->nodetype != LYS_LIST) || !(node->schema->flags & LYS_ORDBY_USER) || (op != LYD_DIFF_OP_CREATE) ||
            (lysc_is_dup_inst_list(node->schema) && position) || key);
    assert((node->schema->nodetype != LYS_LEAFLIST) || !(node->schema->flags & LYS_ORDBY_USER) ||
            (op != LYD_DIFF_OP_CREATE) || (lysc_is_dup_inst_list(node->schema) && position) || value);

    /* move on userord needs both key and orig-key/value and orig-value */
    assert((node->schema->nodetype != LYS_LIST) || !(node->schema->flags & LYS_ORDBY_USER) || (op != LYD_DIFF_OP_REPLACE) ||
            (lysc_is_dup_inst_list(node->schema) && position && orig_position) || (key && orig_key));
    assert((node->schema->nodetype != LYS_LEAFLIST) || !(node->schema->flags & LYS_ORDBY_USER) ||
            (op != LYD_DIFF_OP_REPLACE) || (lysc_is_dup_inst_list(node->schema) && position && orig_position) ||
            (value && orig_value));

    /* find the first existing parent */
    siblings = *diff;
    do {
        /* find next node parent */
        parent = node;
        while (parent->parent && (!diff_parent || (parent->parent->schema != diff_parent->schema))) {
            parent = lyd_parent(parent);
        }

        if (lysc_is_dup_inst_list(parent->schema)) {
            /* assume it never exists, we are not able to distinguish whether it does or not */
            match = NULL;
            break;
        }

        /* check whether it exists in the diff */
        if (lyd_find_sibling_first(siblings, parent, &match)) {
            break;
        }

        /* another parent found */
        diff_parent = match;

        /* move down in the diff */
        siblings = lyd_child_no_keys(match);
    } while (parent != node);

    if (match && (parent == node)) {
        /* special case when there is already an operation on our descendant */
        assert(!lyd_diff_get_op(diff_parent, &cur_op) && (cur_op == LYD_DIFF_OP_NONE));
        (void)cur_op;

        /* move it to the end where it is expected (matters for user-ordered lists) */
        if (lysc_is_userordered(diff_parent->schema)) {
            for (elem = diff_parent; elem->next && (elem->next->schema == elem->schema); elem = elem->next) {}
            if (elem != diff_parent) {
                LY_CHECK_RET(lyd_insert_after(elem, diff_parent));
            }
        }

        /* will be replaced by the new operation but keep the current op for descendants */
        lyd_diff_del_meta(diff_parent, "operation");
        LY_LIST_FOR(lyd_child_no_keys(diff_parent), elem) {
            lyd_diff_find_meta(elem, "operation", &meta, NULL);
            if (meta) {
                /* explicit operation, fine */
                continue;
            }

            /* set the none operation */
            LY_CHECK_RET(lyd_new_meta(NULL, elem, NULL, "yang:operation", "none", 0, NULL));
        }

        dup = diff_parent;
    } else {
        diff_opts = LYD_DUP_NO_META | LYD_DUP_WITH_PARENTS | LYD_DUP_WITH_FLAGS;
        if ((op != LYD_DIFF_OP_REPLACE) || !lysc_is_userordered(node->schema) || (node->schema->flags & LYS_CONFIG_R)) {
            /* move applies only to the user-ordered list, no descendants */
            diff_opts |= LYD_DUP_RECURSIVE;
        }

        /* duplicate the subtree (and connect to the diff if possible) */
        if (diff_parent) {
            LY_CHECK_RET(lyd_dup_single_to_ctx(node, LYD_CTX(diff_parent), (struct lyd_node_inner *)diff_parent,
                    diff_opts, &dup));
        } else {
            LY_CHECK_RET(lyd_dup_single(node, NULL, diff_opts, &dup));
        }

        /* find the first duplicated parent */
        if (!diff_parent) {
            diff_parent = lyd_parent(dup);
            while (diff_parent && diff_parent->parent) {
                diff_parent = lyd_parent(diff_parent);
            }
        } else {
            diff_parent = dup;
            while (diff_parent->parent && (diff_parent->parent->schema == parent->schema)) {
                diff_parent = lyd_parent(diff_parent);
            }
        }

        /* no parent existed, must be manually connected */
        if (!diff_parent) {
            /* there actually was no parent to duplicate */
            lyd_insert_sibling(*diff, dup, diff);
        } else if (!diff_parent->parent) {
            lyd_insert_sibling(*diff, diff_parent, diff);
        }

        /* add parent operation, if any */
        if (diff_parent && (diff_parent != dup)) {
            LY_CHECK_RET(lyd_new_meta(NULL, diff_parent, NULL, "yang:operation", "none", 0, NULL));
        }
    }

    /* add subtree operation */
    LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:operation", lyd_diff_op2str(op), 0, NULL));

    if (op == LYD_DIFF_OP_CREATE) {
        /* all nested user-ordered (leaf-)lists need special metadata for create op */
        LYD_TREE_DFS_BEGIN(dup, elem) {
            if ((elem != dup) && lysc_is_userordered(elem->schema)) {
                LY_CHECK_RET(lyd_diff_add_create_nested_userord(elem));
            }
            LYD_TREE_DFS_END(dup, elem);
        }
    }

    /* orig-default */
    if (orig_default) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-default", orig_default, 0, NULL));
    }

    /* orig-value */
    if (orig_value) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-value", orig_value, 0, NULL));
    }

    /* key */
    if (key) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:key", key, 0, NULL));
    }

    /* value */
    if (value) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:value", value, 0, NULL));
    }

    /* position */
    if (position) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:position", position, 0, NULL));
    }

    /* orig-key */
    if (orig_key) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-key", orig_key, 0, NULL));
    }

    /* orig-position */
    if (orig_position) {
        LY_CHECK_RET(lyd_new_meta(NULL, dup, NULL, "yang:orig-position", orig_position, 0, NULL));
    }

    return LY_SUCCESS;
}

/**
 * @brief Get a userord entry for a specific user-ordered list/leaf-list. Create if does not exist yet.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] schema Schema node of the list/leaf-list.
 * @param[in,out] userord Sized array of userord items.
 * @return Userord item for all the user-ordered list/leaf-list instances.
 */
static struct lyd_diff_userord *
lyd_diff_userord_get(const struct lyd_node *first, const struct lysc_node *schema, struct lyd_diff_userord **userord)
{
    struct lyd_diff_userord *item;
    struct lyd_node *iter;
    const struct lyd_node **node;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(*userord, u) {
        if ((*userord)[u].schema == schema) {
            return &(*userord)[u];
        }
    }

    /* it was not added yet, add it now */
    LY_ARRAY_NEW_RET(schema->module->ctx, *userord, item, NULL);

    item->schema = schema;
    item->pos = 0;
    item->inst = NULL;

    /* store all the instance pointers in the current order */
    if (first) {
        LYD_LIST_FOR_INST(lyd_first_sibling(first), first->schema, iter) {
            LY_ARRAY_NEW_RET(schema->module->ctx, item->inst, node, NULL);
            *node = iter;
        }
    }

    return item;
}

/**
 * @brief Get all the metadata to be stored in a diff for the 2 nodes. Can be used only for user-ordered
 * lists/leaf-lists.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] second Node from the second tree, can be NULL (on delete).
 * @param[in] options Diff options.
 * @param[in] userord_item Userord item of @p first and/or @p second node.
 * @param[out] op Operation.
 * @param[out] orig_default Original default metadata.
 * @param[out] value Value metadata.
 * @param[out] orig_value Original value metadata
 * @param[out] key Key metadata.
 * @param[out] orig_key Original key metadata.
 * @param[out] position Position metadata.
 * @param[out] orig_position Original position metadata.
 * @return LY_SUCCESS on success,
 * @return LY_ENOT if there is no change to be added into diff,
 * @return LY_ERR value on other errors.
 */
static LY_ERR
lyd_diff_userord_attrs(const struct lyd_node *first, const struct lyd_node *second, uint16_t options,
        struct lyd_diff_userord *userord_item, enum lyd_diff_op *op, const char **orig_default, char **value,
        char **orig_value, char **key, char **orig_key, char **position, char **orig_position)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysc_node *schema;
    size_t buflen, bufused;
    uint32_t first_pos, second_pos, comp_opts;

    assert(first || second);

    *orig_default = NULL;
    *value = NULL;
    *orig_value = NULL;
    *key = NULL;
    *orig_key = NULL;
    *position = NULL;
    *orig_position = NULL;

    schema = first ? first->schema : second->schema;
    assert(lysc_is_userordered(schema));

    /* find user-ordered first position */
    if (first) {
        for (first_pos = 0; first_pos < LY_ARRAY_COUNT(userord_item->inst); ++first_pos) {
            if (userord_item->inst[first_pos] == first) {
                break;
            }
        }
        assert(first_pos < LY_ARRAY_COUNT(userord_item->inst));
    } else {
        first_pos = 0;
    }

    /* prepare position of the next instance */
    second_pos = userord_item->pos++;

    /* learn operation first */
    if (!second) {
        *op = LYD_DIFF_OP_DELETE;
    } else if (!first) {
        *op = LYD_DIFF_OP_CREATE;
    } else {
        comp_opts = lysc_is_dup_inst_list(second->schema) ? LYD_COMPARE_FULL_RECURSION : 0;
        if (lyd_compare_single(second, userord_item->inst[second_pos], comp_opts)) {
            /* in first, there is a different instance on the second position, we are going to move 'first' node */
            *op = LYD_DIFF_OP_REPLACE;
        } else if ((options & LYD_DIFF_DEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
            /* default flag change */
            *op = LYD_DIFF_OP_NONE;
        } else {
            /* no changes */
            return LY_ENOT;
        }
    }

    /*
     * set each attribute correctly based on the operation and node type
     */

    /* orig-default */
    if ((schema->nodetype == LYS_LEAFLIST) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_NONE))) {
        if (first->flags & LYD_DEFAULT) {
            *orig_default = "true";
        } else {
            *orig_default = "false";
        }
    }

    /* value */
    if ((schema->nodetype == LYS_LEAFLIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            *value = strdup(lyd_get_value(userord_item->inst[second_pos - 1]));
            LY_CHECK_ERR_GOTO(!*value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        } else {
            *value = strdup("");
            LY_CHECK_ERR_GOTO(!*value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* orig-value */
    if ((schema->nodetype == LYS_LEAFLIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            *orig_value = strdup(lyd_get_value(userord_item->inst[first_pos - 1]));
            LY_CHECK_ERR_GOTO(!*orig_value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        } else {
            *orig_value = strdup("");
            LY_CHECK_ERR_GOTO(!*orig_value, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* key */
    if ((schema->nodetype == LYS_LIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            buflen = bufused = 0;
            LY_CHECK_GOTO(rc = lyd_path_list_predicate(userord_item->inst[second_pos - 1], key, &buflen, &bufused, 0), cleanup);
        } else {
            *key = strdup("");
            LY_CHECK_ERR_GOTO(!*key, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* orig-key */
    if ((schema->nodetype == LYS_LIST) && !lysc_is_dup_inst_list(schema) &&
            ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            buflen = bufused = 0;
            LY_CHECK_GOTO(rc = lyd_path_list_predicate(userord_item->inst[first_pos - 1], orig_key, &buflen, &bufused, 0), cleanup);
        } else {
            *orig_key = strdup("");
            LY_CHECK_ERR_GOTO(!*orig_key, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* position */
    if (lysc_is_dup_inst_list(schema) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_CREATE))) {
        if (second_pos) {
            if (asprintf(position, "%" PRIu32, second_pos) == -1) {
                LOGMEM(schema->module->ctx);
                rc = LY_EMEM;
                goto cleanup;
            }
        } else {
            *position = strdup("");
            LY_CHECK_ERR_GOTO(!*position, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /* orig-position */
    if (lysc_is_dup_inst_list(schema) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_DELETE))) {
        if (first_pos) {
            if (asprintf(orig_position, "%" PRIu32, first_pos) == -1) {
                LOGMEM(schema->module->ctx);
                rc = LY_EMEM;
                goto cleanup;
            }
        } else {
            *orig_position = strdup("");
            LY_CHECK_ERR_GOTO(!*orig_position, LOGMEM(schema->module->ctx); rc = LY_EMEM, cleanup);
        }
    }

    /*
     * update our instances - apply the change
     */
    if (*op == LYD_DIFF_OP_CREATE) {
        /* insert the instance */
        LY_ARRAY_CREATE_GOTO(schema->module->ctx, userord_item->inst, 1, rc, cleanup);
        if (second_pos < LY_ARRAY_COUNT(userord_item->inst)) {
            memmove(userord_item->inst + second_pos + 1, userord_item->inst + second_pos,
                    (LY_ARRAY_COUNT(userord_item->inst) - second_pos) * sizeof *userord_item->inst);
        }
        LY_ARRAY_INCREMENT(userord_item->inst);
        userord_item->inst[second_pos] = second;

    } else if (*op == LYD_DIFF_OP_DELETE) {
        /* remove the instance */
        if (first_pos + 1 < LY_ARRAY_COUNT(userord_item->inst)) {
            memmove(userord_item->inst + first_pos, userord_item->inst + first_pos + 1,
                    (LY_ARRAY_COUNT(userord_item->inst) - first_pos - 1) * sizeof *userord_item->inst);
        }
        LY_ARRAY_DECREMENT(userord_item->inst);

    } else if (*op == LYD_DIFF_OP_REPLACE) {
        /* move the instances */
        memmove(userord_item->inst + second_pos + 1, userord_item->inst + second_pos,
                (first_pos - second_pos) * sizeof *userord_item->inst);
        userord_item->inst[second_pos] = first;
    }

cleanup:
    if (rc) {
        free(*value);
        *value = NULL;
        free(*orig_value);
        *orig_value = NULL;
        free(*key);
        *key = NULL;
        free(*orig_key);
        *orig_key = NULL;
        free(*position);
        *position = NULL;
        free(*orig_position);
        *orig_position = NULL;
    }
    return rc;
}

/**
 * @brief Get all the metadata to be stored in a diff for the 2 nodes. Cannot be used for user-ordered
 * lists/leaf-lists.
 *
 * @param[in] first Node from the first tree, can be NULL (on create).
 * @param[in] second Node from the second tree, can be NULL (on delete).
 * @param[in] options Diff options.
 * @param[out] op Operation.
 * @param[out] orig_default Original default metadata.
 * @param[out] orig_value Original value metadata.
 * @return LY_SUCCESS on success,
 * @return LY_ENOT if there is no change to be added into diff,
 * @return LY_ERR value on other errors.
 */
static LY_ERR
lyd_diff_attrs(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, enum lyd_diff_op *op,
        const char **orig_default, char **orig_value)
{
    const struct lysc_node *schema;
    const char *str_val;

    assert(first || second);

    *orig_default = NULL;
    *orig_value = NULL;

    schema = first ? first->schema : second->schema;
    assert(!lysc_is_userordered(schema));

    /* learn operation first */
    if (!second) {
        *op = LYD_DIFF_OP_DELETE;
    } else if (!first) {
        *op = LYD_DIFF_OP_CREATE;
    } else {
        switch (schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            /* no changes */
            return LY_ENOT;
        case LYS_LIST:
        case LYS_LEAFLIST:
            if ((options & LYD_DIFF_DEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
                /* default flag change */
                *op = LYD_DIFF_OP_NONE;
            } else {
                /* no changes */
                return LY_ENOT;
            }
            break;
        case LYS_LEAF:
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (lyd_compare_single(first, second, 0)) {
                /* different values */
                *op = LYD_DIFF_OP_REPLACE;
            } else if ((options & LYD_DIFF_DEFAULTS) && ((first->flags & LYD_DEFAULT) != (second->flags & LYD_DEFAULT))) {
                /* default flag change */
                *op = LYD_DIFF_OP_NONE;
            } else {
                /* no changes */
                return LY_ENOT;
            }
            break;
        default:
            LOGINT_RET(schema->module->ctx);
        }
    }

    /*
     * set each attribute correctly based on the operation and node type
     */

    /* orig-default */
    if ((schema->nodetype & LYD_NODE_TERM) && ((*op == LYD_DIFF_OP_REPLACE) || (*op == LYD_DIFF_OP_NONE))) {
        if (first->flags & LYD_DEFAULT) {
            *orig_default = "true";
        } else {
            *orig_default = "false";
        }
    }

    /* orig-value */
    if ((schema->nodetype & (LYS_LEAF | LYS_ANYDATA)) && (*op == LYD_DIFF_OP_REPLACE)) {
        if (schema->nodetype == LYS_LEAF) {
            str_val = lyd_get_value(first);
            *orig_value = strdup(str_val ? str_val : "");
            LY_CHECK_ERR_RET(!*orig_value, LOGMEM(schema->module->ctx), LY_EMEM);
        } else {
            LY_CHECK_RET(lyd_any_value_str(first, orig_value));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Find a matching instance of a node in a data tree.
 *
 * @param[in] siblings Siblings to search in.
 * @param[in] target Target node to search for.
 * @param[in] defaults Whether to consider (or ignore) default values.
 * @param[in,out] dup_inst_ht Duplicate instance cache.
 * @param[out] match Found match, NULL if no matching node found.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_find_match(const struct lyd_node *siblings, const struct lyd_node *target, ly_bool defaults,
        struct ly_ht **dup_inst_ht, struct lyd_node **match)
{
    LY_ERR r;

    if (!target->schema) {
        /* try to find the same opaque node */
        r = lyd_find_sibling_opaq_next(siblings, LYD_NAME(target), match);
    } else if (target->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        /* try to find the exact instance */
        r = lyd_find_sibling_first(siblings, target, match);
    } else {
        /* try to simply find the node, there cannot be more instances */
        r = lyd_find_sibling_val(siblings, target->schema, NULL, 0, match);
    }
    if (r && (r != LY_ENOTFOUND)) {
        return r;
    }

    /* update match as needed */
    LY_CHECK_RET(lyd_dup_inst_next(match, siblings, dup_inst_ht));

    if (*match && ((*match)->flags & LYD_DEFAULT) && !defaults) {
        /* ignore default nodes */
        *match = NULL;
    }
    return LY_SUCCESS;
}

/**
 * @brief Perform diff for all siblings at certain depth, recursively.
 *
 * For user-ordered lists/leaf-lists a specific structure is used for storing
 * the current order. The idea is to apply all the generated diff changes
 * virtually on the first tree so that we can continue to generate correct
 * changes after some were already generated.
 *
 * The algorithm then uses second tree position-based changes with a before
 * (preceding) item anchor.
 *
 * Example:
 *
 * Virtual first tree leaf-list order:
 * 1 2 [3] 4 5
 *
 * Second tree leaf-list order:
 * 1 2 [5] 3 4
 *
 * We are at the 3rd node now. We look at whether the nodes on the 3rd position
 * match - they do not - move nodes so that the 3rd position node is final ->
 * -> move node 5 to the 3rd position -> move node 5 after node 2.
 *
 * Required properties:
 * Stored operations (move) should not be affected by later operations -
 * - would cause a redundantly long list of operations, possibly inifinite.
 *
 * Implemenation justification:
 * First, all delete operations and only then move/create operations are stored.
 * Also, preceding anchor is used and after each iteration another node is
 * at its final position. That results in the invariant that all preceding
 * nodes are final and will not be changed by the later operations, meaning
 * they can safely be used as anchors for the later operations.
 *
 * @param[in] first First tree first sibling.
 * @param[in] second Second tree first sibling.
 * @param[in] options Diff options.
 * @param[in] nosiblings Whether to skip following siblings.
 * @param[in,out] diff Diff to append to.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_siblings_r(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, ly_bool nosiblings,
        struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyd_node *iter_first, *iter_second;
    struct lyd_node *match_second, *match_first;
    struct lyd_diff_userord *userord = NULL, *userord_item;
    struct ly_ht *dup_inst_first = NULL, *dup_inst_second = NULL;
    LY_ARRAY_COUNT_TYPE u;
    enum lyd_diff_op op;
    const char *orig_default;
    char *orig_value, *key, *value, *position, *orig_key, *orig_position;

    /* compare first tree to the second tree - delete, replace, none */
    LY_LIST_FOR(first, iter_first) {
        if (!iter_first->schema) {
            continue;
        }

        assert(!(iter_first->schema->flags & LYS_KEY));
        if ((iter_first->flags & LYD_DEFAULT) && !(options & LYD_DIFF_DEFAULTS)) {
            /* skip default nodes */
            continue;
        }

        /* find a match in the second tree */
        LY_CHECK_GOTO(ret = lyd_diff_find_match(second, iter_first, options & LYD_DIFF_DEFAULTS, &dup_inst_second,
                &match_second), cleanup);

        if (lysc_is_userordered(iter_first->schema)) {
            /* get (create) userord entry */
            userord_item = lyd_diff_userord_get(iter_first, iter_first->schema, &userord);
            LY_CHECK_ERR_GOTO(!userord_item, LOGMEM(LYD_CTX(iter_first)); ret = LY_EMEM, cleanup);

            /* we are handling only user-ordered node delete now */
            if (!match_second) {
                /* get all the attributes */
                LY_CHECK_GOTO(ret = lyd_diff_userord_attrs(iter_first, match_second, options, userord_item, &op,
                        &orig_default, &value, &orig_value, &key, &orig_key, &position, &orig_position), cleanup);

                /* there must be changes, it is deleted */
                assert(op == LYD_DIFF_OP_DELETE);
                ret = lyd_diff_add(iter_first, op, orig_default, orig_value, key, value, position, orig_key,
                        orig_position, diff);

                free(orig_value);
                free(key);
                free(value);
                free(position);
                free(orig_key);
                free(orig_position);
                LY_CHECK_GOTO(ret, cleanup);
            }
        } else {
            /* get all the attributes */
            ret = lyd_diff_attrs(iter_first, match_second, options, &op, &orig_default, &orig_value);

            /* add into diff if there are any changes */
            if (!ret) {
                if (op == LYD_DIFF_OP_DELETE) {
                    ret = lyd_diff_add(iter_first, op, orig_default, orig_value, NULL, NULL, NULL, NULL, NULL, diff);
                } else {
                    assert(match_second);
                    ret = lyd_diff_add(match_second, op, orig_default, orig_value, NULL, NULL, NULL, NULL, NULL, diff);
                }

                free(orig_value);
                LY_CHECK_GOTO(ret, cleanup);
            } else if (ret == LY_ENOT) {
                ret = LY_SUCCESS;
            } else {
                goto cleanup;
            }
        }

        /* check descendants, if any, recursively */
        if (match_second) {
            LY_CHECK_GOTO(ret = lyd_diff_siblings_r(lyd_child_no_keys(iter_first), lyd_child_no_keys(match_second),
                    options, 0, diff), cleanup);
        }

        if (nosiblings) {
            break;
        }
    }

    /* reset all cached positions */
    LY_ARRAY_FOR(userord, u) {
        userord[u].pos = 0;
    }

    /* compare second tree to the first tree - create, user-ordered move */
    LY_LIST_FOR(second, iter_second) {
        if (!iter_second->schema) {
            continue;
        }

        assert(!(iter_second->schema->flags & LYS_KEY));
        if ((iter_second->flags & LYD_DEFAULT) && !(options & LYD_DIFF_DEFAULTS)) {
            /* skip default nodes */
            continue;
        }

        /* find a match in the first tree */
        LY_CHECK_GOTO(ret = lyd_diff_find_match(first, iter_second, options & LYD_DIFF_DEFAULTS, &dup_inst_first,
                &match_first), cleanup);

        if (lysc_is_userordered(iter_second->schema)) {
            /* get userord entry */
            userord_item = lyd_diff_userord_get(match_first, iter_second->schema, &userord);
            LY_CHECK_ERR_GOTO(!userord_item, LOGMEM(LYD_CTX(iter_second)); ret = LY_EMEM, cleanup);

            /* get all the attributes */
            ret = lyd_diff_userord_attrs(match_first, iter_second, options, userord_item, &op, &orig_default,
                    &value, &orig_value, &key, &orig_key, &position, &orig_position);

            /* add into diff if there are any changes */
            if (!ret) {
                ret = lyd_diff_add(iter_second, op, orig_default, orig_value, key, value, position, orig_key,
                        orig_position, diff);

                free(orig_value);
                free(key);
                free(value);
                free(position);
                free(orig_key);
                free(orig_position);
                LY_CHECK_GOTO(ret, cleanup);
            } else if (ret == LY_ENOT) {
                ret = LY_SUCCESS;
            } else {
                goto cleanup;
            }
        } else if (!match_first) {
            /* get all the attributes */
            LY_CHECK_GOTO(ret = lyd_diff_attrs(match_first, iter_second, options, &op, &orig_default, &orig_value), cleanup);

            /* there must be changes, it is created */
            assert(op == LYD_DIFF_OP_CREATE);
            ret = lyd_diff_add(iter_second, op, orig_default, orig_value, NULL, NULL, NULL, NULL, NULL, diff);

            free(orig_value);
            LY_CHECK_GOTO(ret, cleanup);
        } /* else was handled */

        if (nosiblings) {
            break;
        }
    }

cleanup:
    lyd_dup_inst_free(dup_inst_first);
    lyd_dup_inst_free(dup_inst_second);
    LY_ARRAY_FOR(userord, u) {
        LY_ARRAY_FREE(userord[u].inst);
    }
    LY_ARRAY_FREE(userord);
    return ret;
}

static LY_ERR
lyd_diff(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, ly_bool nosiblings,
        struct lyd_node **diff)
{
    const struct ly_ctx *ctx;

    LY_CHECK_ARG_RET(NULL, diff, LY_EINVAL);

    if (first) {
        ctx = LYD_CTX(first);
    } else if (second) {
        ctx = LYD_CTX(second);
    } else {
        ctx = NULL;
    }

    if (first && second && (lysc_data_parent(first->schema) != lysc_data_parent(second->schema))) {
        LOGERR(ctx, LY_EINVAL, "Invalid arguments - cannot create diff for unrelated data (%s()).", __func__);
        return LY_EINVAL;
    }

    *diff = NULL;

    return lyd_diff_siblings_r(first, second, options, nosiblings, diff);
}

LIBYANG_API_DEF LY_ERR
lyd_diff_tree(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, struct lyd_node **diff)
{
    return lyd_diff(first, second, options, 1, diff);
}

LIBYANG_API_DEF LY_ERR
lyd_diff_siblings(const struct lyd_node *first, const struct lyd_node *second, uint16_t options, struct lyd_node **diff)
{
    return lyd_diff(first, second, options, 0, diff);
}

/**
 * @brief Insert a diff node into a data tree.
 *
 * @param[in,out] first_node First sibling of the data tree.
 * @param[in] parent_node Data tree sibling parent node.
 * @param[in] new_node Node to insert.
 * @param[in] userord_anchor Optional anchor (key, value, or position) of relative (leaf-)list instance. If not set,
 * the user-ordered instance will be inserted at the first position.
 * @return err_info, NULL on success.
 */
static LY_ERR
lyd_diff_insert(struct lyd_node **first_node, struct lyd_node *parent_node, struct lyd_node *new_node,
        const char *userord_anchor)
{
    LY_ERR ret;
    struct lyd_node *anchor;
    uint32_t pos, anchor_pos;
    int found;

    assert(new_node);

    if (!*first_node) {
        if (!parent_node) {
            /* no parent or siblings */
            *first_node = new_node;
            return LY_SUCCESS;
        }

        /* simply insert into parent, no other children */
        if (userord_anchor) {
            LOGERR(LYD_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                    new_node->schema->name);
            return LY_EINVAL;
        }
        return lyd_insert_child(parent_node, new_node);
    }

    assert(!(*first_node)->parent || (lyd_parent(*first_node) == parent_node));

    if (!lysc_is_userordered(new_node->schema)) {
        /* simple insert */
        return lyd_insert_sibling(*first_node, new_node, first_node);
    }

    if (userord_anchor) {
        /* find the anchor sibling */
        if (lysc_is_dup_inst_list(new_node->schema)) {
            anchor_pos = atoi(userord_anchor);
            if (!anchor_pos) {
                LOGERR(LYD_CTX(new_node), LY_EINVAL, "Invalid user-ordered anchor value \"%s\".", userord_anchor);
                return LY_EINVAL;
            }

            found = 0;
            pos = 1;
            LYD_LIST_FOR_INST(*first_node, new_node->schema, anchor) {
                if (pos == anchor_pos) {
                    found = 1;
                    break;
                }
                ++pos;
            }
            if (!found) {
                LOGERR(LYD_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                        new_node->schema->name);
                return LY_EINVAL;
            }
        } else {
            ret = lyd_find_sibling_val(*first_node, new_node->schema, userord_anchor, 0, &anchor);
            if (ret == LY_ENOTFOUND) {
                LOGERR(LYD_CTX(new_node), LY_EINVAL, "Node \"%s\" instance to insert next to not found.",
                        new_node->schema->name);
                return LY_EINVAL;
            } else if (ret) {
                return ret;
            }
        }

        /* insert after */
        LY_CHECK_RET(lyd_insert_after(anchor, new_node));
        assert(new_node->prev == anchor);
        if (*first_node == new_node) {
            *first_node = anchor;
        }
    } else {
        /* find the first instance */
        ret = lyd_find_sibling_val(*first_node, new_node->schema, NULL, 0, &anchor);
        LY_CHECK_RET(ret && (ret != LY_ENOTFOUND), ret);

        if (anchor) {
            /* insert before the first instance */
            LY_CHECK_RET(lyd_insert_before(anchor, new_node));
            if ((*first_node)->prev->next) {
                assert(!new_node->prev->next);
                *first_node = new_node;
            }
        } else {
            /* insert anywhere */
            LY_CHECK_RET(lyd_insert_sibling(*first_node, new_node, first_node));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Apply diff subtree on data tree nodes, recursively.
 *
 * @param[in,out] first_node First sibling of the data tree.
 * @param[in] parent_node Parent of the first sibling.
 * @param[in] diff_node Current diff node.
 * @param[in] diff_cb Optional diff callback.
 * @param[in] cb_data User data for @p diff_cb.
 * @param[in,out] dup_inst Duplicate instance cache for all @p diff_node siblings.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_apply_r(struct lyd_node **first_node, struct lyd_node *parent_node, const struct lyd_node *diff_node,
        lyd_diff_cb diff_cb, void *cb_data, struct ly_ht **dup_inst)
{
    LY_ERR ret;
    struct lyd_node *match, *diff_child;
    const char *str_val, *meta_str;
    enum lyd_diff_op op;
    struct lyd_meta *meta;
    struct ly_ht *child_dup_inst = NULL;
    const struct ly_ctx *ctx = LYD_CTX(diff_node);

    /* read all the valid attributes */
    LY_CHECK_RET(lyd_diff_get_op(diff_node, &op));

    /* handle specific user-ordered (leaf-)lists operations separately */
    if (lysc_is_userordered(diff_node->schema) && ((op == LYD_DIFF_OP_CREATE) || (op == LYD_DIFF_OP_REPLACE))) {
        if (op == LYD_DIFF_OP_REPLACE) {
            /* find the node (we must have some siblings because the node was only moved) */
            LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
            LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);
        } else {
            /* duplicate the node */
            LY_CHECK_RET(lyd_dup_single(diff_node, NULL, LYD_DUP_NO_META, &match));
        }

        /* get "key", "value", or "position" metadata string value */
        if (lysc_is_dup_inst_list(diff_node->schema)) {
            meta_str = "yang:position";
        } else if (diff_node->schema->nodetype == LYS_LIST) {
            meta_str = "yang:key";
        } else {
            meta_str = "yang:value";
        }
        meta = lyd_find_meta(diff_node->meta, NULL, meta_str);
        LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_str, diff_node), LY_EINVAL);
        str_val = lyd_get_meta_value(meta);

        /* insert/move the node */
        if (str_val[0]) {
            ret = lyd_diff_insert(first_node, parent_node, match, str_val);
        } else {
            ret = lyd_diff_insert(first_node, parent_node, match, NULL);
        }
        if (ret) {
            if (op == LYD_DIFF_OP_CREATE) {
                lyd_free_tree(match);
            }
            return ret;
        }

        goto next_iter_r;
    }

    /* apply operation */
    switch (op) {
    case LYD_DIFF_OP_NONE:
        /* find the node */
        LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
        LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);

        if (match->schema->nodetype & LYD_NODE_TERM) {
            /* special case of only dflt flag change */
            if (diff_node->flags & LYD_DEFAULT) {
                match->flags |= LYD_DEFAULT;
            } else {
                match->flags &= ~LYD_DEFAULT;
            }
        } else {
            /* none operation on nodes without children is redundant and hence forbidden */
            if (!lyd_child_no_keys(diff_node)) {
                LOGERR(ctx, LY_EINVAL, "Operation \"none\" is invalid for node \"%s\" without children.",
                        LYD_NAME(diff_node));
                return LY_EINVAL;
            }
        }
        break;
    case LYD_DIFF_OP_CREATE:
        /* duplicate the node */
        LY_CHECK_RET(lyd_dup_single(diff_node, NULL, LYD_DUP_NO_META, &match));

        /* insert it at the end */
        ret = 0;
        if (parent_node) {
            if (match->flags & LYD_EXT) {
                ret = lyplg_ext_insert(parent_node, match);
            } else {
                ret = lyd_insert_child(parent_node, match);
            }
        } else {
            ret = lyd_insert_sibling(*first_node, match, first_node);
        }
        if (ret) {
            lyd_free_tree(match);
            return ret;
        }

        break;
    case LYD_DIFF_OP_DELETE:
        /* find the node */
        LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
        LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);

        /* remove it */
        if ((match == *first_node) && !match->parent) {
            assert(!parent_node);
            /* we have removed the top-level node */
            *first_node = (*first_node)->next;
        }
        lyd_free_tree(match);

        /* we are not going recursively in this case, the whole subtree was already deleted */
        return LY_SUCCESS;
    case LYD_DIFF_OP_REPLACE:
        if (!(diff_node->schema->nodetype & (LYS_LEAF | LYS_ANYDATA))) {
            LOGERR(ctx, LY_EINVAL, "Operation \"replace\" is invalid for %s node \"%s\".",
                    lys_nodetype2str(diff_node->schema->nodetype), LYD_NAME(diff_node));
            return LY_EINVAL;
        }

        /* find the node */
        LY_CHECK_RET(lyd_diff_find_match(*first_node, diff_node, 1, dup_inst, &match));
        LY_CHECK_ERR_RET(!match, LOGERR_NOINST(ctx, diff_node), LY_EINVAL);

        /* update the value */
        if (diff_node->schema->nodetype == LYS_LEAF) {
            ret = lyd_change_term(match, lyd_get_value(diff_node));
            LY_CHECK_ERR_RET(ret && (ret != LY_EEXIST), LOGERR_UNEXPVAL(ctx, match, "data"), LY_EINVAL);
        } else {
            struct lyd_node_any *any = (struct lyd_node_any *)diff_node;

            LY_CHECK_RET(lyd_any_copy_value(match, &any->value, any->value_type));
        }

        /* with flags */
        match->flags = diff_node->flags;
        break;
    default:
        LOGINT_RET(ctx);
    }

next_iter_r:
    if (diff_cb) {
        /* call callback */
        LY_CHECK_RET(diff_cb(diff_node, match, cb_data));
    }

    /* apply diff recursively */
    ret = LY_SUCCESS;
    LY_LIST_FOR(lyd_child_no_keys(diff_node), diff_child) {
        ret = lyd_diff_apply_r(lyd_node_child_p(match), match, diff_child, diff_cb, cb_data, &child_dup_inst);
        if (ret) {
            break;
        }
    }

    lyd_dup_inst_free(child_dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_apply_module(struct lyd_node **data, const struct lyd_node *diff, const struct lys_module *mod,
        lyd_diff_cb diff_cb, void *cb_data)
{
    const struct lyd_node *root;
    struct ly_ht *dup_inst = NULL;
    LY_ERR ret = LY_SUCCESS;

    LY_LIST_FOR(diff, root) {
        if (mod && (lyd_owner_module(root) != mod)) {
            /* skip data nodes from different modules */
            continue;
        }

        /* apply relevant nodes from the diff datatree */
        ret = lyd_diff_apply_r(data, NULL, root, diff_cb, cb_data, &dup_inst);
        if (ret) {
            break;
        }
    }

    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_apply_all(struct lyd_node **data, const struct lyd_node *diff)
{
    return lyd_diff_apply_module(data, diff, NULL, NULL, NULL);
}

/**
 * @brief Update operations on a diff node when the new operation is NONE.
 *
 * @param[in] diff_match Node from the diff.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_none(struct lyd_node *diff_match, enum lyd_diff_op cur_op, const struct lyd_node *src_diff)
{
    switch (cur_op) {
    case LYD_DIFF_OP_NONE:
    case LYD_DIFF_OP_CREATE:
    case LYD_DIFF_OP_REPLACE:
        if (src_diff->schema->nodetype & LYD_NODE_TERM) {
            /* NONE on a term means only its dflt flag was changed */
            diff_match->flags &= ~LYD_DEFAULT;
            diff_match->flags |= src_diff->flags & LYD_DEFAULT;
        }
        break;
    default:
        /* delete operation is not valid */
        LOGERR_MERGEOP(LYD_CTX(diff_match), diff_match, cur_op, LYD_DIFF_OP_NONE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Set a specific operation of a node. Delete the previous operation, if any.
 * Does not change the default flag.
 *
 * @param[in] node Node to change.
 * @param[in] op Operation to set.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_change_op(struct lyd_node *node, enum lyd_diff_op op)
{
    lyd_diff_del_meta(node, "operation");

    if (node->schema) {
        return lyd_new_meta(LYD_CTX(node), node, NULL, "yang:operation", lyd_diff_op2str(op), 0, NULL);
    } else {
        return lyd_new_attr(node, "yang", "operation", lyd_diff_op2str(op), NULL);
    }
}

/**
 * @brief Update operations on a diff node when the new operation is REPLACE.
 *
 * @param[in] diff_match Node from the diff.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_replace(struct lyd_node *diff_match, enum lyd_diff_op cur_op, const struct lyd_node *src_diff)
{
    LY_ERR ret;
    const char *str_val, *meta_name, *orig_meta_name;
    struct lyd_meta *meta;
    const struct lys_module *mod;
    const struct lyd_node_any *any;
    const struct ly_ctx *ctx = LYD_CTX(diff_match);

    /* get "yang" module for the metadata */
    mod = ly_ctx_get_module_latest(LYD_CTX(diff_match), "yang");
    assert(mod);

    switch (cur_op) {
    case LYD_DIFF_OP_REPLACE:
    case LYD_DIFF_OP_CREATE:
        switch (diff_match->schema->nodetype) {
        case LYS_LIST:
        case LYS_LEAFLIST:
            /* it was created/moved somewhere, but now it will be created/moved somewhere else,
             * keep orig_key/orig_value (only replace oper) and replace key/value */
            assert(lysc_is_userordered(diff_match->schema));
            if (lysc_is_dup_inst_list(diff_match->schema)) {
                meta_name = "position";
            } else if (diff_match->schema->nodetype == LYS_LIST) {
                meta_name = "key";
            } else {
                meta_name = "value";
            }

            lyd_diff_del_meta(diff_match, meta_name);
            meta = lyd_find_meta(src_diff->meta, mod, meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_name, src_diff), LY_EINVAL);
            LY_CHECK_RET(lyd_dup_meta_single(meta, diff_match, NULL));
            break;
        case LYS_LEAF:
            /* replaced with the exact same value, impossible */
            if (!lyd_compare_single(diff_match, src_diff, 0)) {
                LOGERR_UNEXPVAL(ctx, diff_match, "target diff");
                return LY_EINVAL;
            }

            /* modify the node value */
            if (lyd_change_term(diff_match, lyd_get_value(src_diff))) {
                LOGINT_RET(LYD_CTX(src_diff));
            }

            if (cur_op == LYD_DIFF_OP_REPLACE) {
                /* compare values whether there is any change at all */
                meta = lyd_find_meta(diff_match->meta, mod, "orig-value");
                LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, "orig-value", diff_match), LY_EINVAL);
                str_val = lyd_get_meta_value(meta);
                ret = lyd_value_compare((struct lyd_node_term *)diff_match, str_val, strlen(str_val));
                if (!ret) {
                    /* values are the same, remove orig-value meta and set oper to NONE */
                    lyd_free_meta_single(meta);
                    LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_NONE));
                }
            }

            /* modify the default flag */
            diff_match->flags &= ~LYD_DEFAULT;
            diff_match->flags |= src_diff->flags & LYD_DEFAULT;
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (!lyd_compare_single(diff_match, src_diff, 0)) {
                LOGERR_UNEXPVAL(ctx, diff_match, "target diff");
                return LY_EINVAL;
            }

            /* modify the node value */
            any = (struct lyd_node_any *)src_diff;
            LY_CHECK_RET(lyd_any_copy_value(diff_match, &any->value, any->value_type));
            break;
        default:
            LOGINT_RET(LYD_CTX(src_diff));
        }
        break;
    case LYD_DIFF_OP_NONE:
        switch (diff_match->schema->nodetype) {
        case LYS_LIST:
            /* it is moved now */
            assert(lysc_is_userordered(diff_match->schema));

            /* change the operation */
            LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_REPLACE));

            /* set orig-meta and meta */
            if (lysc_is_dup_inst_list(diff_match->schema)) {
                meta_name = "position";
                orig_meta_name = "orig-position";
            } else {
                meta_name = "key";
                orig_meta_name = "orig-key";
            }

            meta = lyd_find_meta(src_diff->meta, mod, orig_meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, orig_meta_name, src_diff), LY_EINVAL);
            LY_CHECK_RET(lyd_dup_meta_single(meta, diff_match, NULL));

            meta = lyd_find_meta(src_diff->meta, mod, meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_name, src_diff), LY_EINVAL);
            LY_CHECK_RET(lyd_dup_meta_single(meta, diff_match, NULL));
            break;
        case LYS_LEAF:
            /* only dflt flag changed, now value changed as well, update the operation */
            LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_REPLACE));

            /* modify the node value */
            if (lyd_change_term(diff_match, lyd_get_value(src_diff))) {
                LOGINT_RET(LYD_CTX(src_diff));
            }
            break;
        default:
            LOGINT_RET(LYD_CTX(src_diff));
        }
        break;
    default:
        /* delete operation is not valid */
        LOGERR_MERGEOP(ctx, diff_match, cur_op, LYD_DIFF_OP_REPLACE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Update operations in a diff node when the new operation is CREATE.
 *
 * @param[in,out] diff_match Node from the diff, may be replaced.
 * @param[in,out] diff Diff root node, may be updated.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @param[in] options Diff merge options.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_create(struct lyd_node **diff_match, struct lyd_node **diff, enum lyd_diff_op cur_op,
        const struct lyd_node *src_diff, uint16_t options)
{
    struct lyd_node *child, *src_dup, *to_free = NULL;
    const struct lysc_node_leaf *sleaf = NULL;
    uint32_t trg_flags;
    const char *meta_name, *orig_meta_name;
    struct lyd_meta *meta, *orig_meta;
    const struct ly_ctx *ctx = LYD_CTX(*diff_match);
    LY_ERR r;

    /* create operation is valid only for data nodes */
    LY_CHECK_ERR_RET(!src_diff->schema, LOGINT(ctx), LY_EINT);

    switch (cur_op) {
    case LYD_DIFF_OP_DELETE:
        /* remember current flags */
        trg_flags = (*diff_match)->flags;

        if (lysc_is_userordered(src_diff->schema)) {
            assert((*diff_match)->schema);

            /* get anchor metadata */
            if (lysc_is_dup_inst_list((*diff_match)->schema)) {
                meta_name = "yang:position";
                orig_meta_name = "yang:orig-position";
            } else if ((*diff_match)->schema->nodetype == LYS_LIST) {
                meta_name = "yang:key";
                orig_meta_name = "yang:orig-key";
            } else {
                meta_name = "yang:value";
                orig_meta_name = "yang:orig-value";
            }
            meta = lyd_find_meta(src_diff->meta, NULL, meta_name);
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, meta_name, src_diff), LY_EINVAL);
            orig_meta = lyd_find_meta((*diff_match)->meta, NULL, orig_meta_name);
            LY_CHECK_ERR_RET(!orig_meta, LOGERR_META(ctx, orig_meta_name, *diff_match), LY_EINVAL);

            /* the (incorrect) assumption made here is that there are no previous diff nodes that would affect
             * the anchors stored in the metadata */
            if (strcmp(lyd_get_meta_value(meta), lyd_get_meta_value(orig_meta))) {
                /* deleted + created at another position -> operation REPLACE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_REPLACE));

                /* add anchor metadata */
                LY_CHECK_RET(lyd_dup_meta_single(meta, *diff_match, NULL));
            } else {
                /* deleted + created at the same position -> operation NONE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));

                /* delete anchor metadata */
                lyd_free_meta_single(orig_meta);
            }
        } else if (src_diff->schema->nodetype == LYS_LEAF) {
            if (options & LYD_DIFF_MERGE_DEFAULTS) {
                /* we are dealing with a leaf and are handling default values specially (as explicit nodes) */
                sleaf = (struct lysc_node_leaf *)src_diff->schema;
            }

            if (sleaf && sleaf->dflt && !sleaf->dflt->realtype->plugin->compare(sleaf->dflt,
                    &((struct lyd_node_term *)src_diff)->value)) {
                /* we deleted it, so a default value was in-use, and it matches the created value -> operation NONE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));
            } else if (!lyd_compare_single(*diff_match, src_diff, 0)) {
                /* deleted + created -> operation NONE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));
            } else if ((*diff_match)->schema) {
                /* we deleted it, but it was created with a different value -> operation REPLACE */
                LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_REPLACE));

                /* current value is the previous one (meta) */
                LY_CHECK_RET(lyd_new_meta(LYD_CTX(src_diff), *diff_match, NULL, "yang:orig-value",
                        lyd_get_value(*diff_match), 0, NULL));

                /* update the value itself */
                LY_CHECK_RET(lyd_change_term(*diff_match, lyd_get_value(src_diff)));
            } else {
                /* also operation REPLACE but we need to change an opaque node into a data node */
                LY_CHECK_RET(lyd_dup_single(src_diff, (*diff_match)->parent, LYD_DUP_NO_META | LYD_DUP_WITH_FLAGS, &src_dup));
                if (!(*diff_match)->parent) {
                    /* will always be inserted before diff_match, which is opaque */
                    LY_CHECK_RET(lyd_insert_sibling(*diff_match, src_dup, diff));
                }
                to_free = *diff_match;
                *diff_match = src_dup;

                r = lyd_new_meta(ctx, src_dup, NULL, "yang:orig-value", lyd_get_value(to_free), 0, NULL);
                lyd_free_tree(to_free);
                LY_CHECK_RET(r);
                LY_CHECK_RET(lyd_new_meta(ctx, src_dup, NULL, "yang:operation", lyd_diff_op2str(LYD_DIFF_OP_REPLACE), 0, NULL));
            }
        } else {
            /* deleted + created -> operation NONE */
            LY_CHECK_RET(lyd_diff_change_op(*diff_match, LYD_DIFF_OP_NONE));
        }

        assert((*diff_match)->schema);
        if ((*diff_match)->schema->nodetype & LYD_NODE_TERM) {
            /* add orig-dflt metadata */
            LY_CHECK_RET(lyd_new_meta(LYD_CTX(src_diff), *diff_match, NULL, "yang:orig-default",
                    trg_flags & LYD_DEFAULT ? "true" : "false", 0, NULL));

            /* update dflt flag itself */
            (*diff_match)->flags &= ~LYD_DEFAULT;
            (*diff_match)->flags |= src_diff->flags & LYD_DEFAULT;
        }

        /* but the operation of its children should remain DELETE */
        LY_LIST_FOR(lyd_child_no_keys(*diff_match), child) {
            LY_CHECK_RET(lyd_diff_change_op(child, LYD_DIFF_OP_DELETE));
        }
        break;
    default:
        /* create and replace operations are not valid */
        LOGERR_MERGEOP(LYD_CTX(src_diff), *diff_match, cur_op, LYD_DIFF_OP_CREATE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Update operations on a diff node when the new operation is DELETE.
 *
 * @param[in] diff_match Node from the diff.
 * @param[in] cur_op Current operation of @p diff_match.
 * @param[in] src_diff Current source diff node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_delete(struct lyd_node *diff_match, enum lyd_diff_op cur_op, const struct lyd_node *src_diff)
{
    struct lyd_node *child;
    struct lyd_meta *meta;
    const char *meta_name;
    const struct ly_ctx *ctx = LYD_CTX(diff_match);

    /* we can delete only exact existing nodes */
    LY_CHECK_ERR_RET(lyd_compare_single(diff_match, src_diff, 0), LOGINT(LYD_CTX(src_diff)), LY_EINT);

    switch (cur_op) {
    case LYD_DIFF_OP_CREATE:
        /* it was created, but then deleted -> set NONE operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_NONE));

        if (diff_match->schema->nodetype & LYD_NODE_TERM) {
            /* add orig-default meta because it is expected */
            LY_CHECK_RET(lyd_new_meta(LYD_CTX(src_diff), diff_match, NULL, "yang:orig-default",
                    src_diff->flags & LYD_DEFAULT ? "true" : "false", 0, NULL));
        } else if (!lysc_is_dup_inst_list(diff_match->schema)) {
            /* keep operation for all descendants (for now) */
            LY_LIST_FOR(lyd_child_no_keys(diff_match), child) {
                LY_CHECK_RET(lyd_diff_change_op(child, cur_op));
            }
        } /* else key-less list, for which all the descendants act as keys */
        break;
    case LYD_DIFF_OP_REPLACE:
        /* remove the redundant metadata */
        if (lysc_is_userordered(diff_match->schema)) {
            if (lysc_is_dup_inst_list(diff_match->schema)) {
                meta_name = "position";
            } else if (diff_match->schema->nodetype == LYS_LIST) {
                meta_name = "key";
            } else {
                meta_name = "value";
            }
        } else {
            assert(diff_match->schema->nodetype == LYS_LEAF);

            /* switch value for the original one */
            meta = lyd_find_meta(diff_match->meta, NULL, "yang:orig-value");
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, "yang:orig-value", diff_match), LY_EINVAL);
            if (lyd_change_term(diff_match, lyd_get_meta_value(meta))) {
                LOGERR_UNEXPVAL(ctx, diff_match, "target diff");
                return LY_EINVAL;
            }

            /* switch default for the original one, then remove the meta */
            meta = lyd_find_meta(diff_match->meta, NULL, "yang:orig-default");
            LY_CHECK_ERR_RET(!meta, LOGERR_META(ctx, "yang:orig-default", diff_match), LY_EINVAL);
            diff_match->flags &= ~LYD_DEFAULT;
            if (meta->value.boolean) {
                diff_match->flags |= LYD_DEFAULT;
            }
            lyd_free_meta_single(meta);

            meta_name = "orig-value";
        }
        lyd_diff_del_meta(diff_match, meta_name);

        /* it was being changed, but should be deleted instead -> set DELETE operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_DELETE));
        break;
    case LYD_DIFF_OP_NONE:
        /* it was not modified, but should be deleted -> set DELETE operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_match, LYD_DIFF_OP_DELETE));
        break;
    default:
        /* delete operation is not valid */
        LOGERR_MERGEOP(LYD_CTX(diff_match), diff_match, cur_op, LYD_DIFF_OP_DELETE);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check whether this diff node is redundant (does not change data).
 *
 * @param[in] diff Diff node.
 * @return 0 if not, non-zero if it is.
 */
static int
lyd_diff_is_redundant(struct lyd_node *diff)
{
    enum lyd_diff_op op;
    struct lyd_meta *meta, *orig_val_meta = NULL, *val_meta = NULL;
    struct lyd_node *child;
    const struct lys_module *mod;
    const char *str, *orig_meta_name, *meta_name;

    assert(diff);

    if (lysc_is_dup_inst_list(diff->schema)) {
        /* all descendants are keys */
        child = NULL;
    } else {
        child = lyd_child_no_keys(diff);
    }
    mod = ly_ctx_get_module_latest(LYD_CTX(diff), "yang");
    assert(mod);

    /* get node operation */
    LY_CHECK_RET(lyd_diff_get_op(diff, &op), 0);

    if ((op == LYD_DIFF_OP_REPLACE) && lysc_is_userordered(diff->schema)) {
        /* get metadata names */
        if (lysc_is_dup_inst_list(diff->schema)) {
            meta_name = "position";
            orig_meta_name = "orig-position";
        } else if (diff->schema->nodetype == LYS_LIST) {
            meta_name = "key";
            orig_meta_name = "orig-key";
        } else {
            meta_name = "value";
            orig_meta_name = "orig-value";
        }

        /* check for redundant move */
        orig_val_meta = lyd_find_meta(diff->meta, mod, orig_meta_name);
        val_meta = lyd_find_meta(diff->meta, mod, meta_name);
        assert(orig_val_meta && val_meta);

        if (!lyd_compare_meta(orig_val_meta, val_meta)) {
            /* there is actually no move */
            lyd_free_meta_single(orig_val_meta);
            lyd_free_meta_single(val_meta);
            if (child) {
                /* change operation to NONE, we have siblings */
                lyd_diff_change_op(diff, LYD_DIFF_OP_NONE);
                return 0;
            }

            /* redundant node, BUT !!
             * In diff the move operation is always converted to be INSERT_AFTER, which is fine
             * because the data that this is applied on should not change for the diff lifetime.
             * However, when we are merging 2 diffs, this conversion is actually lossy because
             * if the data change, the move operation can also change its meaning. In this specific
             * case the move operation will be lost. But it can be considered a feature, it is not supported.
             */
            return 1;
        }
    } else if (op == LYD_DIFF_OP_NONE) {
        if (!diff->schema) {
            /* opaque node with none must be redundant */
            return 1;
        }

        if (diff->schema->nodetype & LYD_NODE_TERM) {
            /* check whether at least the default flags are different */
            meta = lyd_find_meta(diff->meta, mod, "orig-default");
            assert(meta);
            str = lyd_get_meta_value(meta);

            /* if previous and current dflt flags are the same, this node is redundant */
            if ((!strcmp(str, "true") && (diff->flags & LYD_DEFAULT)) || (!strcmp(str, "false") && !(diff->flags & LYD_DEFAULT))) {
                return 1;
            }
            return 0;
        }
    }

    if (!child && (op == LYD_DIFF_OP_NONE)) {
        return 1;
    }

    return 0;
}

/**
 * @brief Merge sysrepo diff subtree with another diff, recursively.
 *
 * @param[in] src_diff Source diff node.
 * @param[in] diff_parent Current sysrepo diff parent.
 * @param[in] diff_cb Optional diff callback.
 * @param[in] cb_data User data for @p diff_cb.
 * @param[in,out] dup_inst Duplicate instance cache for all @p src_diff siblings.
 * @param[in] options Diff merge options.
 * @param[in,out] diff Diff root node.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_merge_r(const struct lyd_node *src_diff, struct lyd_node *diff_parent, lyd_diff_cb diff_cb, void *cb_data,
        struct ly_ht **dup_inst, uint16_t options, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *child, *diff_node = NULL;
    enum lyd_diff_op src_op, cur_op;
    struct ly_ht *child_dup_inst = NULL;

    /* get source node operation */
    LY_CHECK_RET(lyd_diff_get_op(src_diff, &src_op));

    /* find an equal node in the current diff */
    LY_CHECK_RET(lyd_diff_find_match(diff_parent ? lyd_child_no_keys(diff_parent) : *diff, src_diff, 1, dup_inst, &diff_node));

    if (diff_node) {
        /* get target (current) operation */
        LY_CHECK_RET(lyd_diff_get_op(diff_node, &cur_op));

        /* merge operations */
        switch (src_op) {
        case LYD_DIFF_OP_REPLACE:
            ret = lyd_diff_merge_replace(diff_node, cur_op, src_diff);
            break;
        case LYD_DIFF_OP_CREATE:
            if ((cur_op == LYD_DIFF_OP_CREATE) && lysc_is_dup_inst_list(diff_node->schema)) {
                /* special case of creating duplicate (leaf-)list instances */
                goto add_diff;
            }

            ret = lyd_diff_merge_create(&diff_node, diff, cur_op, src_diff, options);
            break;
        case LYD_DIFF_OP_DELETE:
            ret = lyd_diff_merge_delete(diff_node, cur_op, src_diff);
            break;
        case LYD_DIFF_OP_NONE:
            /* key-less list can never have "none" operation since all its descendants are acting as "keys" */
            assert((src_diff->schema->nodetype != LYS_LIST) || !lysc_is_dup_inst_list(src_diff->schema));
            ret = lyd_diff_merge_none(diff_node, cur_op, src_diff);
            break;
        default:
            LOGINT_RET(LYD_CTX(src_diff));
        }
        if (ret) {
            LOGERR(LYD_CTX(src_diff), LY_EOTHER, "Merging operation \"%s\" failed.", lyd_diff_op2str(src_op));
            return ret;
        }

        if (diff_cb) {
            /* call callback */
            LY_CHECK_RET(diff_cb(src_diff, diff_node, cb_data));
        }

        /* update diff parent */
        diff_parent = diff_node;

        /* for diff purposes, all key-less list descendants actually act as keys (identifying the same instances),
         * so there is nothing to merge for these "keys" */
        if (!lysc_is_dup_inst_list(src_diff->schema)) {
            /* merge src_diff recursively */
            LY_LIST_FOR(lyd_child_no_keys(src_diff), child) {
                ret = lyd_diff_merge_r(child, diff_parent, diff_cb, cb_data, &child_dup_inst, options, diff);
                if (ret) {
                    break;
                }
            }
            lyd_dup_inst_free(child_dup_inst);
            LY_CHECK_RET(ret);
        }
    } else {
add_diff:
        /* add new diff node with all descendants */
        if ((src_diff->flags & LYD_EXT) && diff_parent) {
            LY_CHECK_RET(lyd_dup_single_to_ctx(src_diff, LYD_CTX(diff_parent), (struct lyd_node_inner *)diff_parent,
                    LYD_DUP_RECURSIVE | LYD_DUP_WITH_FLAGS, &diff_node));
        } else {
            LY_CHECK_RET(lyd_dup_single(src_diff, (struct lyd_node_inner *)diff_parent,
                    LYD_DUP_RECURSIVE | LYD_DUP_WITH_FLAGS, &diff_node));
        }

        /* insert node into diff if not already */
        if (!diff_parent) {
            lyd_insert_sibling(*diff, diff_node, diff);
        }

        /* update operation */
        LY_CHECK_RET(lyd_diff_change_op(diff_node, src_op));

        if (diff_cb) {
            /* call callback with no source diff node since it was duplicated and just added */
            LY_CHECK_RET(diff_cb(NULL, diff_node, cb_data));
        }

        /* update diff parent */
        diff_parent = diff_node;
    }

    /* remove any redundant nodes */
    if (lyd_diff_is_redundant(diff_parent)) {
        if (diff_parent == *diff) {
            *diff = (*diff)->next;
        }
        lyd_free_tree(diff_parent);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_merge_module(struct lyd_node **diff, const struct lyd_node *src_diff, const struct lys_module *mod,
        lyd_diff_cb diff_cb, void *cb_data, uint16_t options)
{
    const struct lyd_node *src_root;
    struct ly_ht *dup_inst = NULL;
    LY_ERR ret = LY_SUCCESS;

    LY_LIST_FOR(src_diff, src_root) {
        if (mod && (lyd_owner_module(src_root) != mod)) {
            /* skip data nodes from different modules */
            continue;
        }

        /* apply relevant nodes from the diff datatree */
        LY_CHECK_GOTO(ret = lyd_diff_merge_r(src_root, NULL, diff_cb, cb_data, &dup_inst, options, diff), cleanup);
    }

cleanup:
    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_merge_tree(struct lyd_node **diff_first, struct lyd_node *diff_parent, const struct lyd_node *src_sibling,
        lyd_diff_cb diff_cb, void *cb_data, uint16_t options)
{
    LY_ERR ret;
    struct ly_ht *dup_inst = NULL;

    if (!src_sibling) {
        return LY_SUCCESS;
    }

    ret = lyd_diff_merge_r(src_sibling, diff_parent, diff_cb, cb_data, &dup_inst, options, diff_first);
    lyd_dup_inst_free(dup_inst);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_merge_all(struct lyd_node **diff, const struct lyd_node *src_diff, uint16_t options)
{
    return lyd_diff_merge_module(diff, src_diff, NULL, NULL, NULL, options);
}

static LY_ERR
lyd_diff_reverse_value(struct lyd_node *node, const struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *meta;
    const char *val1 = NULL;
    char *val2;
    uint32_t flags;

    assert(node->schema->nodetype & (LYS_LEAF | LYS_ANYDATA));

    meta = lyd_find_meta(node->meta, mod, "orig-value");
    LY_CHECK_ERR_RET(!meta, LOGERR_META(LYD_CTX(node), "orig-value", node), LY_EINVAL);

    /* orig-value */
    val1 = lyd_get_meta_value(meta);

    /* current value */
    if (node->schema->nodetype == LYS_LEAF) {
        val2 = strdup(lyd_get_value(node));
    } else {
        LY_CHECK_RET(lyd_any_value_str(node, &val2));
    }

    /* switch values, keep default flag */
    flags = node->flags;
    if (node->schema->nodetype == LYS_LEAF) {
        LY_CHECK_GOTO(ret = lyd_change_term(node, val1), cleanup);
    } else {
        union lyd_any_value anyval = {.str = val1};

        LY_CHECK_GOTO(ret = lyd_any_copy_value(node, &anyval, LYD_ANYDATA_STRING), cleanup);
    }
    node->flags = flags;
    LY_CHECK_GOTO(ret = lyd_change_meta(meta, val2), cleanup);

cleanup:
    free(val2);
    return ret;
}

static LY_ERR
lyd_diff_reverse_default(struct lyd_node *node, const struct lys_module *mod)
{
    struct lyd_meta *meta;
    uint32_t flag1, flag2;

    meta = lyd_find_meta(node->meta, mod, "orig-default");
    LY_CHECK_ERR_RET(!meta, LOGINT(mod->ctx), LY_EINT);

    /* orig-default */
    if (meta->value.boolean) {
        flag1 = LYD_DEFAULT;
    } else {
        flag1 = 0;
    }

    /* current default */
    flag2 = node->flags & LYD_DEFAULT;

    if (flag1 == flag2) {
        /* no default state change so nothing to reverse */
        return LY_SUCCESS;
    }

    /* switch defaults */
    node->flags &= ~LYD_DEFAULT;
    node->flags |= flag1;
    LY_CHECK_RET(lyd_change_meta(meta, flag2 ? "true" : "false"));

    return LY_SUCCESS;
}

static LY_ERR
lyd_diff_reverse_meta(struct lyd_node *node, const struct lys_module *mod, const char *name1, const char *name2)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_meta *meta1, *meta2;
    const char *val1 = NULL;
    char *val2 = NULL;

    meta1 = lyd_find_meta(node->meta, mod, name1);
    LY_CHECK_ERR_RET(!meta1, LOGERR_META(LYD_CTX(node), name1, node), LY_EINVAL);

    meta2 = lyd_find_meta(node->meta, mod, name2);
    LY_CHECK_ERR_RET(!meta2, LOGERR_META(LYD_CTX(node), name2, node), LY_EINVAL);

    /* value1 */
    val1 = lyd_get_meta_value(meta1);

    /* value2 */
    val2 = strdup(lyd_get_meta_value(meta2));

    /* switch values */
    LY_CHECK_GOTO(ret = lyd_change_meta(meta1, val2), cleanup);
    LY_CHECK_GOTO(ret = lyd_change_meta(meta2, val1), cleanup);

cleanup:
    free(val2);
    return ret;
}

/**
 * @brief Remove specific operation from all the nodes in a subtree.
 *
 * @param[in] diff Diff subtree to process.
 * @param[in] op Only expected operation.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_diff_reverse_remove_op_r(struct lyd_node *diff, enum lyd_diff_op op)
{
    struct lyd_node *elem;
    struct lyd_meta *meta;

    LYD_TREE_DFS_BEGIN(diff, elem) {
        meta = lyd_find_meta(elem->meta, NULL, "yang:operation");
        if (meta) {
            LY_CHECK_ERR_RET(lyd_diff_str2op(lyd_get_meta_value(meta)) != op, LOGINT(LYD_CTX(diff)), LY_EINT);
            lyd_free_meta_single(meta);
        }

        LYD_TREE_DFS_END(diff, elem);
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyd_diff_reverse_all(const struct lyd_node *src_diff, struct lyd_node **diff)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lys_module *mod;
    struct lyd_node *root, *elem, *iter;
    enum lyd_diff_op op;

    LY_CHECK_ARG_RET(NULL, diff, LY_EINVAL);

    if (!src_diff) {
        *diff = NULL;
        return LY_SUCCESS;
    }

    /* duplicate diff */
    LY_CHECK_RET(lyd_dup_siblings(src_diff, NULL, LYD_DUP_RECURSIVE, diff));

    /* find module with metadata needed for later */
    mod = ly_ctx_get_module_latest(LYD_CTX(src_diff), "yang");
    LY_CHECK_ERR_GOTO(!mod, LOGINT(LYD_CTX(src_diff)); ret = LY_EINT, cleanup);

    LY_LIST_FOR(*diff, root) {
        LYD_TREE_DFS_BEGIN(root, elem) {
            /* skip all keys */
            if (!lysc_is_key(elem->schema)) {
                /* find operation attribute, if any */
                LY_CHECK_GOTO(ret = lyd_diff_get_op(elem, &op), cleanup);

                switch (op) {
                case LYD_DIFF_OP_CREATE:
                    /* reverse create to delete */
                    LY_CHECK_GOTO(ret = lyd_diff_change_op(elem, LYD_DIFF_OP_DELETE), cleanup);

                    /* check all the children for the same operation, nothing else is expected */
                    LY_LIST_FOR(lyd_child(elem), iter) {
                        lyd_diff_reverse_remove_op_r(iter, LYD_DIFF_OP_CREATE);
                    }

                    LYD_TREE_DFS_continue = 1;
                    break;
                case LYD_DIFF_OP_DELETE:
                    /* reverse delete to create */
                    LY_CHECK_GOTO(ret = lyd_diff_change_op(elem, LYD_DIFF_OP_CREATE), cleanup);

                    /* check all the children for the same operation, nothing else is expected */
                    LY_LIST_FOR(lyd_child(elem), iter) {
                        lyd_diff_reverse_remove_op_r(iter, LYD_DIFF_OP_DELETE);
                    }

                    LYD_TREE_DFS_continue = 1;
                    break;
                case LYD_DIFF_OP_REPLACE:
                    switch (elem->schema->nodetype) {
                    case LYS_LEAF:
                        /* leaf value change */
                        LY_CHECK_GOTO(ret = lyd_diff_reverse_value(elem, mod), cleanup);
                        LY_CHECK_GOTO(ret = lyd_diff_reverse_default(elem, mod), cleanup);
                        break;
                    case LYS_ANYXML:
                    case LYS_ANYDATA:
                        /* any value change */
                        LY_CHECK_GOTO(ret = lyd_diff_reverse_value(elem, mod), cleanup);
                        break;
                    case LYS_LEAFLIST:
                        /* leaf-list move */
                        LY_CHECK_GOTO(ret = lyd_diff_reverse_default(elem, mod), cleanup);
                        if (lysc_is_dup_inst_list(elem->schema)) {
                            LY_CHECK_GOTO(ret = lyd_diff_reverse_meta(elem, mod, "orig-position", "position"), cleanup);
                        } else {
                            LY_CHECK_GOTO(ret = lyd_diff_reverse_meta(elem, mod, "orig-value", "value"), cleanup);
                        }
                        break;
                    case LYS_LIST:
                        /* list move */
                        if (lysc_is_dup_inst_list(elem->schema)) {
                            LY_CHECK_GOTO(ret = lyd_diff_reverse_meta(elem, mod, "orig-position", "position"), cleanup);
                        } else {
                            LY_CHECK_GOTO(ret = lyd_diff_reverse_meta(elem, mod, "orig-key", "key"), cleanup);
                        }
                        break;
                    default:
                        LOGINT(LYD_CTX(src_diff));
                        ret = LY_EINT;
                        goto cleanup;
                    }
                    break;
                case LYD_DIFF_OP_NONE:
                    switch (elem->schema->nodetype) {
                    case LYS_LEAF:
                    case LYS_LEAFLIST:
                        /* default flag change */
                        LY_CHECK_GOTO(ret = lyd_diff_reverse_default(elem, mod), cleanup);
                        break;
                    default:
                        /* nothing to do */
                        break;
                    }
                    break;
                }
            }

            LYD_TREE_DFS_END(root, elem);
        }
    }

cleanup:
    if (ret) {
        lyd_free_siblings(*diff);
        *diff = NULL;
    }
    return ret;
}
