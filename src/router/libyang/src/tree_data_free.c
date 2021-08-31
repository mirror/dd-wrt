/**
 * @file tree_data_free.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Freeing functions for data tree structures
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "dict.h"
#include "hash_table.h"
#include "log.h"
#include "plugins_types.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

static void
lyd_free_meta(struct lyd_meta *meta, ly_bool siblings)
{
    struct lyd_meta *iter;

    if (!meta) {
        return;
    }

    if (meta->parent) {
        if (meta->parent->meta == meta) {
            if (siblings) {
                meta->parent->meta = NULL;
            } else {
                meta->parent->meta = meta->next;
            }
        } else {
            for (iter = meta->parent->meta; iter->next != meta; iter = iter->next) {}
            if (iter->next) {
                if (siblings) {
                    iter->next = NULL;
                } else {
                    iter->next = meta->next;
                }
            }
        }
    }

    if (!siblings) {
        meta->next = NULL;
    }

    for (iter = meta; iter; ) {
        meta = iter;
        iter = iter->next;

        lydict_remove(meta->annotation->module->ctx, meta->name);
        meta->value.realtype->plugin->free(meta->annotation->module->ctx, &meta->value);
        free(meta);
    }
}

API void
lyd_free_meta_single(struct lyd_meta *meta)
{
    lyd_free_meta(meta, 0);
}

API void
lyd_free_meta_siblings(struct lyd_meta *meta)
{
    lyd_free_meta(meta, 1);
}

static void
lyd_free_attr(const struct ly_ctx *ctx, struct lyd_attr *attr, ly_bool siblings)
{
    struct lyd_attr *iter;

    LY_CHECK_ARG_RET(NULL, ctx, );
    if (!attr) {
        return;
    }

    if (attr->parent) {
        if (attr->parent->attr == attr) {
            if (siblings) {
                attr->parent->attr = NULL;
            } else {
                attr->parent->attr = attr->next;
            }
        } else {
            for (iter = attr->parent->attr; iter->next != attr; iter = iter->next) {}
            if (iter->next) {
                if (siblings) {
                    iter->next = NULL;
                } else {
                    iter->next = attr->next;
                }
            }
        }
    }

    if (!siblings) {
        attr->next = NULL;
    }

    for (iter = attr; iter; ) {
        attr = iter;
        iter = iter->next;

        ly_free_prefix_data(attr->format, attr->val_prefix_data);
        lydict_remove(ctx, attr->name.name);
        lydict_remove(ctx, attr->name.prefix);
        lydict_remove(ctx, attr->name.module_ns);
        lydict_remove(ctx, attr->value);
        free(attr);
    }
}

API void
lyd_free_attr_single(const struct ly_ctx *ctx, struct lyd_attr *attr)
{
    lyd_free_attr(ctx, attr, 0);
}

API void
lyd_free_attr_siblings(const struct ly_ctx *ctx, struct lyd_attr *attr)
{
    lyd_free_attr(ctx, attr, 1);
}

/**
 * @brief Free Data (sub)tree.
 * @param[in] node Data node to be freed.
 * @param[in] top Recursion flag to unlink the root of the subtree being freed.
 */
static void
lyd_free_subtree(struct lyd_node *node, ly_bool top)
{
    struct lyd_node *iter, *next;
    struct lyd_node_opaq *opaq = NULL;

    assert(node);

    if (!node->schema) {
        opaq = (struct lyd_node_opaq *)node;

        /* free the children */
        LY_LIST_FOR_SAFE(lyd_child(node), next, iter) {
            lyd_free_subtree(iter, 0);
        }

        lydict_remove(LYD_CTX(opaq), opaq->name.name);
        lydict_remove(LYD_CTX(opaq), opaq->name.prefix);
        lydict_remove(LYD_CTX(opaq), opaq->name.module_ns);
        lydict_remove(LYD_CTX(opaq), opaq->value);
        ly_free_prefix_data(opaq->format, opaq->val_prefix_data);
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        /* remove children hash table in case of inner data node */
        lyht_free(((struct lyd_node_inner *)node)->children_ht);
        ((struct lyd_node_inner *)node)->children_ht = NULL;

        /* free the children */
        LY_LIST_FOR_SAFE(lyd_child(node), next, iter) {
            lyd_free_subtree(iter, 0);
        }
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        /* only frees the value this way */
        lyd_any_copy_value(node, NULL, 0);
    } else if (node->schema->nodetype & LYD_NODE_TERM) {
        ((struct lysc_node_leaf *)node->schema)->type->plugin->free(LYD_CTX(node), &((struct lyd_node_term *)node)->value);
    }

    if (!node->schema) {
        lyd_free_attr_siblings(LYD_CTX(node), opaq->attr);
    } else {
        /* free the node's metadata */
        lyd_free_meta_siblings(node->meta);
    }

    /* unlink only the nodes from the first level, nodes in subtree are freed all, so no unlink is needed */
    if (top) {
        lyd_unlink_tree(node);
    }

    free(node);
}

API void
lyd_free_tree(struct lyd_node *node)
{
    if (!node) {
        return;
    }

    lyd_free_subtree(node, 1);
}

static void
lyd_free_(struct lyd_node *node, ly_bool top)
{
    struct lyd_node *iter, *next;

    if (!node) {
        return;
    }

    /* get the first (top-level) sibling */
    if (top) {
        for ( ; node->parent; node = lyd_parent(node)) {}
    }
    while (node->prev->next) {
        node = node->prev;
    }

    LY_LIST_FOR_SAFE(node, next, iter) {
        /* in case of the top-level nodes (node->parent is NULL), no unlinking needed */
        lyd_free_subtree(iter, iter->parent ? 1 : 0);
    }
}

API void
lyd_free_siblings(struct lyd_node *node)
{
    lyd_free_(node, 0);
}

API void
lyd_free_all(struct lyd_node *node)
{
    lyd_free_(node, 1);
}
