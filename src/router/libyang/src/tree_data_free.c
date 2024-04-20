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

#include "dict.h"
#include "hash_table.h"
#include "log.h"
#include "ly_common.h"
#include "plugins_exts/metadata.h"
#include "plugins_types.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_data_sorted.h"
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

LIBYANG_API_DEF void
lyd_free_meta_single(struct lyd_meta *meta)
{
    lyd_free_meta(meta, 0);
}

LIBYANG_API_DEF void
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

LIBYANG_API_DEF void
lyd_free_attr_single(const struct ly_ctx *ctx, struct lyd_attr *attr)
{
    lyd_free_attr(ctx, attr, 0);
}

LIBYANG_API_DEF void
lyd_free_attr_siblings(const struct ly_ctx *ctx, struct lyd_attr *attr)
{
    lyd_free_attr(ctx, attr, 1);
}

void
lyd_free_leafref_links_rec(struct lyd_leafref_links_rec *rec)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_leafref_links_rec *rec2;

    assert(rec);

    /* remove links of leafref nodes */
    LY_ARRAY_FOR(rec->leafref_nodes, u) {
        if (lyd_get_or_create_leafref_links_record(rec->leafref_nodes[u], &rec2, 0) == LY_SUCCESS) {
            LY_ARRAY_REMOVE_VALUE(rec2->target_nodes, rec->node);
            if ((LY_ARRAY_COUNT(rec2->leafref_nodes) == 0) && (LY_ARRAY_COUNT(rec2->target_nodes) == 0)) {
                lyd_free_leafref_nodes(rec->leafref_nodes[u]);
            }
        }
    }
    LY_ARRAY_FREE(rec->leafref_nodes);
    rec->leafref_nodes = NULL;

    /* remove links of target nodes */
    LY_ARRAY_FOR(rec->target_nodes, u) {
        if (lyd_get_or_create_leafref_links_record(rec->target_nodes[u], &rec2, 0) == LY_SUCCESS) {
            LY_ARRAY_REMOVE_VALUE(rec2->leafref_nodes, rec->node);
            if ((LY_ARRAY_COUNT(rec2->leafref_nodes) == 0) && (LY_ARRAY_COUNT(rec2->target_nodes) == 0)) {
                lyd_free_leafref_nodes(rec->target_nodes[u]);
            }
        }
    }
    LY_ARRAY_FREE(rec->target_nodes);
    rec->target_nodes = NULL;
}

void
lyd_free_leafref_nodes(const struct lyd_node_term *node)
{
    struct ly_ht *ht;
    uint32_t hash;
    struct lyd_leafref_links_rec *rec;

    assert(node);

    if (lyd_get_or_create_leafref_links_record(node, &rec, 0)) {
        return;
    }

    /* free entry content */
    lyd_free_leafref_links_rec(rec);

    /* free entry itself from hash table */
    ht = LYD_CTX(node)->leafref_links_ht;
    hash = lyht_hash((const char *)&node, sizeof node);
    lyht_remove(ht, rec, hash);
}

/**
 * @brief Free Data (sub)tree.
 *
 * @param[in] node Data node to be freed.
 */
static void
lyd_free_subtree(struct lyd_node *node)
{
    struct lyd_node *iter, *next;
    struct lyd_node_opaq *opaq = NULL;

    assert(node);

    if (!node->schema) {
        opaq = (struct lyd_node_opaq *)node;

        /* free the children */
        LY_LIST_FOR_SAFE(lyd_child(node), next, iter) {
            lyd_free_subtree(iter);
        }

        lydict_remove(LYD_CTX(opaq), opaq->name.name);
        lydict_remove(LYD_CTX(opaq), opaq->name.prefix);
        lydict_remove(LYD_CTX(opaq), opaq->name.module_ns);
        lydict_remove(LYD_CTX(opaq), opaq->value);
        ly_free_prefix_data(opaq->format, opaq->val_prefix_data);
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        /* remove children hash table in case of inner data node */
        lyht_free(((struct lyd_node_inner *)node)->children_ht, NULL);

        /* free the children */
        LY_LIST_FOR_SAFE(lyd_child(node), next, iter) {
            lyd_free_subtree(iter);
        }
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        /* only frees the value this way */
        lyd_any_copy_value(node, NULL, 0);
    } else if (node->schema->nodetype & LYD_NODE_TERM) {
        struct lyd_node_term *node_term = (struct lyd_node_term *)node;

        ((struct lysc_node_leaf *)node->schema)->type->plugin->free(LYD_CTX(node), &node_term->value);
        lyd_free_leafref_nodes(node_term);
    }

    if (!node->schema) {
        lyd_free_attr_siblings(LYD_CTX(node), opaq->attr);
    } else {
        /* free the node's metadata */
        lyd_free_meta_siblings(node->meta);
    }

    free(node);
}

LIBYANG_API_DEF void
lyd_free_tree(struct lyd_node *node)
{
    if (!node) {
        return;
    }

    if (lysc_is_key(node->schema) && node->parent) {
        LOGERR(LYD_CTX(node), LY_EINVAL, "Cannot free a list key \"%s\", free the list instance instead.", LYD_NAME(node));
        return;
    }

    lyd_unlink(node);
    lyd_free_subtree(node);
}

static void
lyd_free_(struct lyd_node *node)
{
    struct lyd_node *iter, *next, *first_sibling = NULL;

    if (!node) {
        return;
    }

    LY_LIST_FOR_SAFE(lyd_first_sibling(node), next, iter) {
        if (lysc_is_key(iter->schema) && iter->parent) {
            LOGERR(LYD_CTX(iter), LY_EINVAL, "Cannot free a list key \"%s\", free the list instance instead.", LYD_NAME(iter));
            return;
        }

        /* in case of the top-level nodes (node->parent is NULL), no unlinking needed */
        if (iter->parent) {
            lyds_free_metadata(iter);
            lyd_unlink_ignore_lyds(&first_sibling, iter);
        }
        lyd_free_subtree(iter);
    }
}

LIBYANG_API_DEF void
lyd_free_siblings(struct lyd_node *node)
{
    lyd_free_(node);
}

LIBYANG_API_DEF void
lyd_free_all(struct lyd_node *node)
{
    if (!node) {
        return;
    }

    /* get top-level node */
    for ( ; node->parent; node = lyd_parent(node)) {}

    lyd_free_(node);
}
