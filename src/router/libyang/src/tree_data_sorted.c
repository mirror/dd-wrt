/**
 * @file tree_data_sorted.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Red-black tree implementation from FRRouting project (https://github.com/FRRouting/frr).
 *
 * The effort of this implementation was to take the working Red-black tree implementation
 * and adapt its interface to libyang.
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

// SPDX-License-Identifier: ISC AND BSD-2-Clause
/*	$OpenBSD: subr_tree.c,v 1.9 2017/06/08 03:30:52 dlg Exp $ */
/*
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * Copyright (c) 2016 David Gwynne <dlg@openbsd.org>
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "log.h"
#include "ly_common.h"
#include "metadata.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_data_sorted.h"

/*
     metadata (root_meta)
      ^   |________
      |            |
      |            v                    --
      |      _____rbt__                   |
      |     |      |   |                  |
      |     v      |   v                  |
      |   _rbn_    | _rbn_____            | BST
      |     |      |   |      |           | (Red-black tree)
      |  ___|      |   |      v           |
      | |     _____|   |    _rbn_         |
      | |    |         |      |         --
      | v    v         v      v
 ... lyd1<-->lyd2<-->lyd3<-->lyd4 ...
   (leader)

   |                             |
   |_____________________________|
            (leaf-)list

 The (leaf-)list consists of data nodes (lyd). The first instance of the (leaf-)list is named leader,
 which contains metadata named 'lyds_tree'. This metadata has a reference to the root of the Red-black tree.
 This tree consists of nodes named 'rbn'. Each of these nodes contains a reference to a left or right child,
 as well as a reference to a data node.
*/

/*
 * A red-black tree is a binary search tree (BST) with the node color as an
 * extra attribute. It fulfills a set of conditions:
 *	- every search path from the root to a leaf consists of the same number of black nodes,
 *	- each red node (except for the root) has a black parent,
 *	- each leaf node is black.
 *
 * Every operation on a red-black tree is bounded as O(lg n).
 * The maximum height of a red-black tree is 2lg (n+1).
 */

#define RB_BLACK    0   /**< black node in a red-black tree */
#define RB_RED      1   /**< red node in a red-black tree */

/**
 * @brief Red-black node
 */
struct rb_node {
    struct rb_node *parent;     /**< parent node (NULL if this is a root node) */
    struct rb_node *left;       /**< left node with a lower value */
    struct rb_node *right;      /**< left node with a greater value */
    struct lyd_node *dnode;     /**< assigned libyang data node */
    uint8_t color;              /**< color for red-black node */
};

/**
 * @defgroup rbngetters Macros for accessing members.
 *
 * Useful if there is a need to reduce the memory space for red-black nodes in the future. The color bit can be hidden
 * in some (parent/left/right) pointer and their true values can be obtained by masking. This saves 8 bytes, but there
 * is no guarantee that the code will be cross-platform.
 *
 * @{
 */
#define RBN_LEFT(NODE) ((NODE)->left)
#define RBN_RIGHT(NODE) ((NODE)->right)
#define RBN_PARENT(NODE) ((NODE)->parent)
#define RBN_DNODE(NODE) ((NODE)->dnode)
#define RBN_COLOR(NODE) ((NODE)->color)
/** @} rbngetters */

/**
 * @brief Rewrite members from @p SRC to @p DST.
 *
 * @param[in] DST Destination node.
 * @param[in] SRC Source node.
 */
#define RBN_COPY(DST, SRC) \
    RBN_PARENT(DST) = RBN_PARENT(SRC); \
    RBN_LEFT(DST)   = RBN_LEFT(SRC); \
    RBN_RIGHT(DST)  = RBN_RIGHT(SRC); \
    RBN_COLOR(DST)  = RBN_COLOR(SRC);

/**
 * @brief Reset the red-black node and set new dnode.
 *
 * @param[in] RBN Node to reset.
 * @param[in] DNODE New dnode value for @p rbn.
 */
#define RBN_RESET(RBN, DNODE) \
    *(RBN) = (const struct rb_node){0}; \
    RBN_DNODE(RBN) = DNODE;

/**
 * @brief Metadata name of the Red-black tree.
 */
#define RB_NAME "lyds_tree"
#define RB_NAME_LEN strlen(RB_NAME)

/**
 * @brief Get red-black root from metadata.
 *
 * @param[in] META Pointer to the struct lyd_meta.
 * @param[out] RBT Root of the Red-black tree.
 */
#define RBT_GET(META, RBT) \
    { \
        struct lyd_value_lyds_tree *_lt; \
        LYD_VALUE_GET(&META->value, _lt); \
        RBT = _lt ? _lt->rbt : NULL; \
    }

/**
 * @brief Set a new red-black root to the metadata.
 *
 * @param[in] META Pointer to the struct lyd_meta.
 * @param[in] RBT Root of the Red-black tree.
 */
#define RBT_SET(META, RBT) \
    { \
        struct lyd_value_lyds_tree *_lt; \
        LYD_VALUE_GET(&META->value, _lt); \
        _lt->rbt = RBT; \
    }

/**
 * @brief Get Red-black tree from data node.
 *
 * @param[in] leader First instance of the (leaf-)list in sequence.
 * @param[out] meta Metadata from which the Red-black tree was obtained. The parameter is optional.
 * @return Root of the Red-black tree or NULL.
 */
static struct rb_node *
lyds_get_rb_tree(const struct lyd_node *leader, struct lyd_meta **meta)
{
    struct rb_node *rbt;
    struct lyd_meta *iter;

    if (meta) {
        *meta = NULL;
    }
    LY_LIST_FOR(leader->meta, iter) {
        if (!strcmp(iter->name, RB_NAME)) {
            if (meta) {
                *meta = iter;
            }
            RBT_GET(iter, rbt);
            return rbt;
        }
    }

    return NULL;
}

/**
 * @brief Call plugins_types sort callback or sort values by plugin order.
 *
 * @param[in] ctx libyang context.
 * @param[in] val1 First value to compare.
 * @param[in] val2 Second value to compare.
 * @return Negative number if val1 < val2,
 * @return Zero if val1 == val2,
 * @return Positive number if val1 > val2.
 */
static int
rb_sort_clb(const struct ly_ctx *ctx, const struct lyd_value *val1, const struct lyd_value *val2)
{
    assert(val1->realtype == val2->realtype);
    return val1->realtype->plugin->sort(ctx, val1, val2);
}

/**
 * @brief Compare red-black nodes by rb_node.dnode from the same Red-black tree.
 *
 * @param[in] n1 First leaf-list data node.
 * @param[in] n2 Second leaf-list data node.
 * @return Negative number if val1 < val2,
 * @return Zero if val1 == val2,
 * @return Positive number if val1 > val2.
 */
static int
rb_compare_leaflists(const struct lyd_node *n1, const struct lyd_node *n2)
{
    struct lyd_value *val1, *val2;

    /* compare leaf-list values */
    assert(n2->schema->nodetype == LYS_LEAFLIST);
    assert(n1->schema->nodetype == LYS_LEAFLIST);

    val1 = &((struct lyd_node_term *)n1)->value;
    val2 = &((struct lyd_node_term *)n2)->value;
    return rb_sort_clb(LYD_CTX(n1), val1, val2);
}

/**
 * @brief Compare red-black nodes by rb_node.dnode from the same Red-black tree.
 *
 * @param[in] n1 First list data node.
 * @param[in] n2 Second list data node.
 * @return Negative number if val1 < val2,
 * @return Zero if val1 == val2,
 * @return Positive number if val1 > val2.
 */
static int
rb_compare_lists(const struct lyd_node *n1, const struct lyd_node *n2)
{
    const struct lyd_node *k1, *k2;
    struct lyd_value *val1, *val2;
    int cmp;

    /* compare first list key */
    assert(n1->schema->nodetype & LYS_LIST);
    assert(n2->schema->nodetype & LYS_LIST);

    /* lyd_child() is not called due to optimization */
    k1 = ((const struct lyd_node_inner *)n1)->child;
    k2 = ((const struct lyd_node_inner *)n2)->child;
    val1 = &((struct lyd_node_term *)k1)->value;
    val2 = &((struct lyd_node_term *)k2)->value;
    cmp = rb_sort_clb(LYD_CTX(n1), val1, val2);
    if (cmp != 0) {
        return cmp;
    }

    /* continue with the next keys */
    k1 = k1->next;
    k2 = k2->next;
    while (k1 && k1->schema && (k1->schema->flags & LYS_KEY)) {
        assert(k1->schema == k2->schema);
        val1 = &((struct lyd_node_term *)k1)->value;
        val2 = &((struct lyd_node_term *)k2)->value;
        cmp = rb_sort_clb(LYD_CTX(n1), val1, val2);
        if (cmp != 0) {
            return cmp;
        }
        k1 = k1->next;
        k2 = k2->next;
    }
    return cmp;
}

/**
 * @brief Release unlinked red-black node.
 *
 * @param[in,out] rbn Node to free, is set to NULL.
 */
static void
rb_free_node(struct rb_node **rbn)
{
    free(*rbn);
    *rbn = NULL;
}

/**
 * @brief Traversing all red-black nodes.
 *
 * Traversal order is not the same as traversing data nodes.
 * The rb_next() is available for browsing in a sorted manner.
 *
 * @param[in] current_state Current state of iterator.
 * @param[out] next_state The updated state of the iterator.
 * @return Next node or the first node.
 */
static struct rb_node *
rb_iter_traversal(struct rb_node *current_state, struct rb_node **next_state)
{
    struct rb_node *iter, *parent, *next;

    for (iter = current_state; iter; iter = next) {
        if (RBN_LEFT(iter)) {
            next = RBN_LEFT(iter);
            continue;
        } else if (RBN_RIGHT(iter)) {
            next = RBN_RIGHT(iter);
            continue;
        }

        *next_state = parent = RBN_PARENT(iter);

        if (parent && (RBN_LEFT(parent) == iter)) {
            RBN_LEFT(parent) = NULL;
        } else if (parent && (RBN_RIGHT(parent) == iter)) {
            RBN_RIGHT(parent) = NULL;
        }

        return iter;
    }

    return NULL;
}

/**
 * @brief Iterator initialization for traversing red-black tree.
 *
 * @param[in] rbt Root of the Red-black tree.
 * @param[out] iter_state Iterator state which must be maintained during browsing.
 * @return First node.
 */
static struct rb_node *
rb_iter_begin(struct rb_node *rbt, struct rb_node **iter_state)
{
    return rb_iter_traversal(rbt, iter_state);
}

/**
 * @brief Get the following node when traversing red-black tree.
 *
 * @param[in,out] iter_state Iterator state which must be maintained during browsing.
 * @return Next node.
 */
static struct rb_node *
rb_iter_next(struct rb_node **iter_state)
{
    return rb_iter_traversal(*iter_state, iter_state);
}

void
lyds_free_tree(struct rb_node *rbt)
{
    struct rb_node *rbn, *iter_state;

    /* There is no rebalancing. */
    for (rbn = rb_iter_begin(rbt, &iter_state); rbn; rbn = rb_iter_next(&iter_state)) {
        rb_free_node(&rbn);
    }
}

static void
rb_set(struct rb_node *rbn, struct rb_node *parent)
{
    RBN_PARENT(rbn) = parent;
    RBN_LEFT(rbn) = RBN_RIGHT(rbn) = NULL;
    RBN_COLOR(rbn) = RB_RED;
}

static void
rb_set_blackred(struct rb_node *black, struct rb_node *red)
{
    RBN_COLOR(black) = RB_BLACK;
    RBN_COLOR(red) = RB_RED;
}

static void
rb_rotate_left(struct rb_node **rbt, struct rb_node *rbn)
{
    struct rb_node *parent;
    struct rb_node *tmp;

    tmp = RBN_RIGHT(rbn);
    RBN_RIGHT(rbn) = RBN_LEFT(tmp);
    if (RBN_RIGHT(rbn) != NULL) {
        RBN_PARENT(RBN_LEFT(tmp)) = rbn;
    }

    parent = RBN_PARENT(rbn);
    RBN_PARENT(tmp) = parent;
    if (parent != NULL) {
        if (rbn == RBN_LEFT(parent)) {
            RBN_LEFT(parent) = tmp;
        } else {
            RBN_RIGHT(parent) = tmp;
        }
    } else {
        *rbt = tmp;
    }

    RBN_LEFT(tmp) = rbn;
    RBN_PARENT(rbn) = tmp;
}

static void
rb_rotate_right(struct rb_node **rbt, struct rb_node *rbn)
{
    struct rb_node *parent;
    struct rb_node *tmp;

    tmp = RBN_LEFT(rbn);
    RBN_LEFT(rbn) = RBN_RIGHT(tmp);
    if (RBN_LEFT(rbn) != NULL) {
        RBN_PARENT(RBN_RIGHT(tmp)) = rbn;
    }

    parent = RBN_PARENT(rbn);
    RBN_PARENT(tmp) = parent;
    if (parent != NULL) {
        if (rbn == RBN_LEFT(parent)) {
            RBN_LEFT(parent) = tmp;
        } else {
            RBN_RIGHT(parent) = tmp;
        }
    } else {
        *rbt = tmp;
    }

    RBN_RIGHT(tmp) = rbn;
    RBN_PARENT(rbn) = tmp;
}

static void
rb_insert_color(struct rb_node **rbt, struct rb_node *rbn)
{
    struct rb_node *parent, *gparent, *tmp;

    while ((parent = RBN_PARENT(rbn)) != NULL &&
            RBN_COLOR(parent) == RB_RED) {
        gparent = RBN_PARENT(parent);

        if (parent == RBN_LEFT(gparent)) {
            tmp = RBN_RIGHT(gparent);
            if ((tmp != NULL) && (RBN_COLOR(tmp) == RB_RED)) {
                RBN_COLOR(tmp) = RB_BLACK;
                rb_set_blackred(parent, gparent);
                rbn = gparent;
                continue;
            }

            if (RBN_RIGHT(parent) == rbn) {
                rb_rotate_left(rbt, parent);
                tmp = parent;
                parent = rbn;
                rbn = tmp;
            }

            rb_set_blackred(parent, gparent);
            rb_rotate_right(rbt, gparent);
        } else {
            tmp = RBN_LEFT(gparent);
            if ((tmp != NULL) && (RBN_COLOR(tmp) == RB_RED)) {
                RBN_COLOR(tmp) = RB_BLACK;
                rb_set_blackred(parent, gparent);
                rbn = gparent;
                continue;
            }

            if (RBN_LEFT(parent) == rbn) {
                rb_rotate_right(rbt, parent);
                tmp = parent;
                parent = rbn;
                rbn = tmp;
            }

            rb_set_blackred(parent, gparent);
            rb_rotate_left(rbt, gparent);
        }
    }

    RBN_COLOR(*rbt) = RB_BLACK;
}

static void
rb_remove_color(struct rb_node **rbt, struct rb_node *parent, struct rb_node *rbn)
{
    struct rb_node *tmp;

    while ((rbn == NULL || RBN_COLOR(rbn) == RB_BLACK) &&
            rbn != *rbt && parent) {
        if (RBN_LEFT(parent) == rbn) {
            tmp = RBN_RIGHT(parent);
            if (RBN_COLOR(tmp) == RB_RED) {
                rb_set_blackred(tmp, parent);
                rb_rotate_left(rbt, parent);
                tmp = RBN_RIGHT(parent);
            }
            if (((RBN_LEFT(tmp) == NULL) ||
                    (RBN_COLOR(RBN_LEFT(tmp)) == RB_BLACK)) &&
                    ((RBN_RIGHT(tmp) == NULL) ||
                    (RBN_COLOR(RBN_RIGHT(tmp)) == RB_BLACK))) {
                RBN_COLOR(tmp) = RB_RED;
                rbn = parent;
                parent = RBN_PARENT(rbn);
            } else {
                if ((RBN_RIGHT(tmp) == NULL) ||
                        (RBN_COLOR(RBN_RIGHT(tmp)) == RB_BLACK)) {
                    struct rb_node *oleft;

                    oleft = RBN_LEFT(tmp);
                    if (oleft != NULL) {
                        RBN_COLOR(oleft) = RB_BLACK;
                    }

                    RBN_COLOR(tmp) = RB_RED;
                    rb_rotate_right(rbt, tmp);
                    tmp = RBN_RIGHT(parent);
                }

                RBN_COLOR(tmp) = RBN_COLOR(parent);
                RBN_COLOR(parent) = RB_BLACK;
                if (RBN_RIGHT(tmp)) {
                    RBN_COLOR(RBN_RIGHT(tmp)) = RB_BLACK;
                }

                rb_rotate_left(rbt, parent);
                rbn = *rbt;
                break;
            }
        } else {
            tmp = RBN_LEFT(parent);
            if (RBN_COLOR(tmp) == RB_RED) {
                rb_set_blackred(tmp, parent);
                rb_rotate_right(rbt, parent);
                tmp = RBN_LEFT(parent);
            }

            if (((RBN_LEFT(tmp) == NULL) ||
                    (RBN_COLOR(RBN_LEFT(tmp)) == RB_BLACK)) &&
                    ((RBN_RIGHT(tmp) == NULL) ||
                    (RBN_COLOR(RBN_RIGHT(tmp)) == RB_BLACK))) {
                RBN_COLOR(tmp) = RB_RED;
                rbn = parent;
                parent = RBN_PARENT(rbn);
            } else {
                if ((RBN_LEFT(tmp) == NULL) ||
                        (RBN_COLOR(RBN_LEFT(tmp)) == RB_BLACK)) {
                    struct rb_node *oright;

                    oright = RBN_RIGHT(tmp);
                    if (oright != NULL) {
                        RBN_COLOR(oright) = RB_BLACK;
                    }

                    RBN_COLOR(tmp) = RB_RED;
                    rb_rotate_left(rbt, tmp);
                    tmp = RBN_LEFT(parent);
                }

                RBN_COLOR(tmp) = RBN_COLOR(parent);
                RBN_COLOR(parent) = RB_BLACK;
                if (RBN_LEFT(tmp) != NULL) {
                    RBN_COLOR(RBN_LEFT(tmp)) = RB_BLACK;
                }

                rb_rotate_right(rbt, parent);
                rbn = *rbt;
                break;
            }
        }
    }

    if (rbn != NULL) {
        RBN_COLOR(rbn) = RB_BLACK;
    }
}

/**
 * @brief Remove node from the Red-black tree.
 *
 * @param[in,out] rbt Root of the Red-black tree. After the @p rbn is removed, the root may change.
 * @param[in] rbn Node to remove.
 * @return Removed node from the Red-black tree.
 */
static struct rb_node *
rb_remove(struct rb_node **rbt, struct rb_node *rbn)
{
    struct rb_node *child, *parent, *old = rbn;
    uint8_t color;

    if (RBN_LEFT(rbn) == NULL) {
        child = RBN_RIGHT(rbn);
    } else if (RBN_RIGHT(rbn) == NULL) {
        child = RBN_LEFT(rbn);
    } else {
        struct rb_node *tmp;

        rbn = RBN_RIGHT(rbn);
        while ((tmp = RBN_LEFT(rbn)) != NULL) {
            rbn = tmp;
        }

        child = RBN_RIGHT(rbn);
        parent = RBN_PARENT(rbn);
        color = RBN_COLOR(rbn);
        if (child != NULL) {
            RBN_PARENT(child) = parent;
        }
        if (parent != NULL) {
            if (RBN_LEFT(parent) == rbn) {
                RBN_LEFT(parent) = child;
            } else {
                RBN_RIGHT(parent) = child;
            }
        } else {
            *rbt = child;
        }
        if (RBN_PARENT(rbn) == old) {
            parent = rbn;
        }
        RBN_COPY(rbn, old);

        tmp = RBN_PARENT(old);
        if (tmp != NULL) {
            if (RBN_LEFT(tmp) == old) {
                RBN_LEFT(tmp) = rbn;
            } else {
                RBN_RIGHT(tmp) = rbn;
            }
        } else {
            *rbt = rbn;
        }

        RBN_PARENT(RBN_LEFT(old)) = rbn;
        if (RBN_RIGHT(old)) {
            RBN_PARENT(RBN_RIGHT(old)) = rbn;
        }

        goto color;
    }

    parent = RBN_PARENT(rbn);
    color = RBN_COLOR(rbn);

    if (child != NULL) {
        RBN_PARENT(child) = parent;
    }
    if (parent != NULL) {
        if (RBN_LEFT(parent) == rbn) {
            RBN_LEFT(parent) = child;
        } else {
            RBN_RIGHT(parent) = child;
        }
    } else {
        *rbt = child;
    }
color:
    if (color == RB_BLACK) {
        rb_remove_color(rbt, parent, child);
    }

    return old;
}

/**
 * @brief Insert new node to the Red-black tree.
 *
 * @param[in,out] rbt Root of the Red-black tree. After the @p rbn is inserted, the root may change.
 * @param[in] rbn Node to insert.
 * @param[out] max_p Set to 1 if the inserted node will also be maximum.
 */
static void
rb_insert_node(struct rb_node **rbt, struct rb_node *rbn, ly_bool *max_p)
{
    ly_bool max;
    struct rb_node *tmp;
    struct rb_node *parent = NULL;
    int comp = 0;

    int (*rb_compare)(const struct lyd_node *n1, const struct lyd_node *n2);

    if (RBN_DNODE(*rbt)->schema->nodetype == LYS_LEAFLIST) {
        rb_compare = rb_compare_leaflists;
    } else {
        rb_compare = rb_compare_lists;
    }

    max = 1;
    tmp = *rbt;
    while (tmp != NULL) {
        parent = tmp;

        comp = rb_compare(RBN_DNODE(tmp), RBN_DNODE(rbn));
        if (comp > 0) {
            tmp = RBN_LEFT(tmp);
            max = 0;
        } else {
            tmp = RBN_RIGHT(tmp);
        }
    }

    rb_set(rbn, parent);

    if (parent != NULL) {
        if (comp > 0) {
            RBN_LEFT(parent) = rbn;
        } else {
            RBN_RIGHT(parent) = rbn;
        }
    } else {
        *rbt = rbn;
    }

    rb_insert_color(rbt, rbn);

    if (max_p) {
        *max_p = max;
    }
}

/**
 * @brief Return the first lesser node (previous).
 *
 * @param[in] rbn Node from which the previous node is wanted.
 * @return Return the first lesser node.
 * @return NULL if @p rbn has the least value.
 */
static struct rb_node *
rb_prev(struct rb_node *rbn)
{
    if (RBN_LEFT(rbn)) {
        rbn = RBN_LEFT(rbn);
        while (RBN_RIGHT(rbn)) {
            rbn = RBN_RIGHT(rbn);
        }
    } else {
        if (RBN_PARENT(rbn) && (rbn == RBN_RIGHT(RBN_PARENT(rbn)))) {
            rbn = RBN_PARENT(rbn);
        } else {
            while (RBN_PARENT(rbn) &&
                    (rbn == RBN_LEFT(RBN_PARENT(rbn)))) {
                rbn = RBN_PARENT(rbn);
            }
            rbn = RBN_PARENT(rbn);
        }
    }

    return rbn;
}

/**
 * @brief Return the first greater node (next).
 *
 * @param[in] rbn Node from which the next node is wanted.
 * @return Return the first greater node.
 * @return NULL if @p rbn has the greatest value.
 */
static struct rb_node *
rb_next(struct rb_node *rbn)
{
    if (RBN_RIGHT(rbn) != NULL) {
        rbn = RBN_RIGHT(rbn);
        while (RBN_LEFT(rbn) != NULL) {
            rbn = RBN_LEFT(rbn);
        }
    } else {
        if (RBN_PARENT(rbn) && (rbn == RBN_LEFT(RBN_PARENT(rbn)))) {
            rbn = RBN_PARENT(rbn);
        } else {
            while (RBN_PARENT(rbn) &&
                    (rbn == RBN_RIGHT(RBN_PARENT(rbn)))) {
                rbn = RBN_PARENT(rbn);
            }
            rbn = RBN_PARENT(rbn);
        }
    }

    return rbn;
}

/**
 * @brief Find @p target value in the Red-black tree.
 *
 * @param[in] rbt Root of the Red-black tree.
 * @param[in] target Node containing the value to find.
 * @return red-black node which contains the same value as @p target or NULL.
 */
static struct rb_node *
rb_find(struct rb_node *rbt, struct lyd_node *target)
{
    struct rb_node *iter, *pivot;
    int comp;

    int (*rb_compare)(const struct lyd_node *n1, const struct lyd_node *n2);

    if (RBN_DNODE(rbt) == target) {
        return rbt;
    }

    if (RBN_DNODE(rbt)->schema->nodetype == LYS_LEAFLIST) {
        rb_compare = rb_compare_leaflists;
    } else {
        rb_compare = rb_compare_lists;
    }

    iter = rbt;
    do {
        comp = rb_compare(RBN_DNODE(iter), target);
        if (comp > 0) {
            iter = RBN_LEFT(iter);
        } else if (comp < 0) {
            iter = RBN_RIGHT(iter);
        } else if (RBN_DNODE(iter) == target) {
            return iter;
        } else {
            /* sequential search in nodes having the same value */
            pivot = iter;

            /* search in predecessors */
            for (iter = rb_prev(pivot); iter; iter = rb_prev(iter)) {
                if (rb_compare(RBN_DNODE(iter), target) != 0) {
                    break;
                } else if (RBN_DNODE(iter) == target) {
                    return iter;
                }
            }

            /* search in successors */
            for (iter = rb_next(pivot); iter; iter = rb_next(iter)) {
                if (rb_compare(RBN_DNODE(iter), target) != 0) {
                    break;
                } else if (RBN_DNODE(iter) == target) {
                    return iter;
                }
            }

            /* node not found */
            break;
        }
    } while (iter != NULL);

    return NULL;
}

LY_ERR
lyds_create_node(struct lyd_node *node, struct rb_node **rbn)
{
    *rbn = calloc(1, sizeof **rbn);
    LY_CHECK_ERR_RET(!(*rbn), LOGERR(LYD_CTX(node), LY_EMEM, "Allocation of red-black node failed."), LY_EMEM);
    RBN_DNODE(*rbn) = node;

    return LY_SUCCESS;
}

void
lyds_pool_add(struct lyd_node *leader, struct lyds_pool *pool)
{
    struct rb_node *rbt;
    struct lyd_meta *root_meta, *tmp;

    assert(pool && leader);

    rbt = lyds_get_rb_tree(leader, &root_meta);
    if (root_meta) {
        lyd_unlink_meta_single(root_meta);
        if (pool->meta) {
            tmp = pool->meta;
            pool->meta = root_meta;
            root_meta->next = tmp;
        } else {
            pool->meta = root_meta;
        }
    }

    if (!rbt) {
        return;
    }

    if (pool->rbn) {
        RBN_RESET(pool->rbn, NULL);
    }

    if (pool->iter_state) {
        /* insert rbn back */
        assert(pool->rbn);
        if (RBN_LEFT(pool->iter_state)) {
            assert(!RBN_RIGHT(pool->iter_state));
            RBN_RIGHT(pool->iter_state) = pool->rbn;
        } else {
            assert(!RBN_LEFT(pool->iter_state));
            RBN_LEFT(pool->iter_state) = pool->rbn;
        }
        RBN_PARENT(pool->rbn) = pool->iter_state;
    }

    if (pool->rbn) {
        /* link rbt with rbn */
        RBN_LEFT(pool->rbn) = rbt;
        RBN_PARENT(rbt) = pool->rbn;
        pool->rbn = rb_iter_begin(rbt, &pool->iter_state);
    } else {
        /* set new red black tree */
        pool->rbn = rb_iter_begin(rbt, &pool->iter_state);
    }
}

/**
 * @brief Get metadata from the pool.
 *
 * @param[in,out] pool Pool containing metadata.
 * @return Free metadata to reuse or NULL.
 */
static struct lyd_meta *
lyds_pool_get_meta(struct lyds_pool *pool)
{
    struct lyd_meta *meta, *next;

    if (!pool->meta) {
        return NULL;
    }

    next = pool->meta->next;
    meta = pool->meta;
    meta->next = NULL;
    pool->meta = next;

    return meta;
}

void
lyds_pool_clean(struct lyds_pool *pool)
{
    struct lyd_meta *meta, *next;
    struct rb_node *iter;

    for (iter = pool->rbn; iter; iter = rb_iter_next(&pool->iter_state)) {
        rb_free_node(&iter);
    }
    pool->rbn = NULL;

    for (meta = pool->meta; meta; meta = next) {
        next = meta->next ? meta->next : NULL;
        RBT_SET(meta, NULL);
        lyd_free_meta_single(meta);
    }
    pool->meta = NULL;
}

/**
 * @brief Remove red-black node from the Red-black tree using the data node.
 *
 * @param[in] root_meta Metadata from leader containing a reference to the Red-black tree.
 * @param[in,out] rbt Root of the Red-black tree.
 * @param[in] node Data node used to find the corresponding red-black node.
 * @param[out] removed Removed node from Red-black tree. It can be deallocated or reset for further use.
 */
static void
rb_remove_node(struct lyd_meta *root_meta, struct rb_node **rbt, struct lyd_node *node, struct rb_node **removed)
{
    struct rb_node *rbn;

    assert(root_meta && rbt && node);

    if (!*rbt) {
        return;
    }

    /* find @p node in the Red-black tree. */
    rbn = rb_find(*rbt, node);
    if (!rbn) {
        /* node was not inserted to the lyds tree due to optimization */
        return;
    }
    assert(RBN_DNODE(rbn) == node);

    /* remove node */
    rbn = rb_remove(rbt, rbn);
    *removed = rbn;
    if (rbn == *rbt) {
        /* rbn was the last node, assurance that root will be set to NULL */
        *rbt = NULL;
    }

    /* the root of the Red-black tree may changed due to removal, so update the pointer to the root */
    RBT_SET(root_meta, *rbt);
}

ly_bool
lyds_is_supported(const struct lyd_node *node)
{
    if (!node->schema || !(node->schema->flags & LYS_ORDBY_SYSTEM)) {
        return 0;
    } else if (node->schema->nodetype == LYS_LEAFLIST) {
        return 1;
    } else if ((node->schema->nodetype == LYS_LIST) && !(node->schema->flags & LYS_KEYLESS)) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Unlink @p meta and insert it into @p dst data node.
 *
 * @param[in] dst Data node that will contain @p meta.
 * @param[in] meta Metadata that will be moved.
 */
static void
lyds_move_meta(struct lyd_node *dst, struct lyd_meta *meta)
{
    lyd_unlink_meta_single(meta);
    lyd_insert_meta(dst, meta, 0);
}

/**
 * @brief Connect data node with siblings so that the nodes are sorted.
 *
 * @param[in,out] first_sibling First sibling node.
 * @param[in,out] leader First instance of the (leaf-)list.
 * @param[in] node Data node to link.
 * @param[in] root_meta Metadata containing Red-black tree. Can be moved to a new leader.
 * @param[in] rbn red-black node of @p node.
 */
static void
lyds_link_data_node(struct lyd_node **first_sibling, struct lyd_node **leader, struct lyd_node *node,
        struct lyd_meta *root_meta, struct rb_node *rbn)
{
    struct rb_node *prev;

    /* insert @p node also into the data node (struct lyd_node) siblings */
    prev = rb_prev(rbn);
    if (prev) {
        lyd_insert_after_node(first_sibling, RBN_DNODE(prev), RBN_DNODE(rbn));
    } else {
        /* leader is no longer the first, the first is @p node */
        lyd_insert_before_node(*leader, RBN_DNODE(rbn));
        *leader = node;
        /* move metadata from the old leader to the new one */
        lyds_move_meta(node, root_meta);
        /* update first_sibling */
        *first_sibling = node->prev->next ? *first_sibling : node;
    }
}

/**
 * @brief Additionally create the Red-black nodes.
 *
 * @param[in,out] first_sibling First sibling node.
 * @param[in,out] leader First instance of the (leaf-)list.
 * @param[in] root_meta From the @p leader, metadata in which is the root of the Red-black tree.
 * @param[in] rbt From the @p root_meta, root of the Red-black tree.
 * @param[in] node Start node from which rb nodes will be created.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_additionally_create_rb_nodes(struct lyd_node **first_sibling, struct lyd_node **leader,
        struct lyd_meta *root_meta, struct rb_node **rbt, struct lyd_node *node)
{
    LY_ERR ret;
    ly_bool max;
    struct rb_node *rbn;
    struct lyd_node *iter;

    for (iter = node; iter && (iter->schema == (*leader)->schema); iter = iter->next) {
        ret = lyds_create_node(iter, &rbn);
        LY_CHECK_RET(ret);
        rb_insert_node(rbt, rbn, &max);
        if (!max) {
            /* nodes were not sorted, they will be sorted now */
            lyd_unlink_ignore_lyds(first_sibling, iter);
            lyds_link_data_node(first_sibling, leader, iter, root_meta, rbn);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Additionally create the Red-black tree for the sorted nodes.
 *
 * @param[in,out] first_sibling First sibling node.
 * @param[in,out] leader First instance of the (leaf-)list.
 * @param[in] root_meta From the @p leader, metadata in which is the root of the Red-black tree.
 * @param[in] rbt From the @p root_meta, root of the Red-black tree.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_additionally_create_rb_tree(struct lyd_node **first_sibling, struct lyd_node **leader,
        struct lyd_meta *root_meta, struct rb_node **rbt)
{
    LY_ERR ret;
    struct rb_node *rbn;

    /* let's begin with the leader */
    ret = lyds_create_node(*leader, &rbn);
    LY_CHECK_RET(ret);
    *rbt = rbn;

    /* continue with the rest of the nodes */
    ret = lyds_additionally_create_rb_nodes(first_sibling, leader, root_meta, rbt, (*leader)->next);

    /* store pointer to the root */
    RBT_SET(root_meta, *rbt);

    return ret;
}

/**
 * @brief Additionally reuse the Red-black tree for the sorted nodes.
 *
 * @param[in,out] first_sibling First sibling node.
 * @param[in,out] leader First instance of the (leaf-)list.
 * @param[in] root_meta From the @p leader, metadata in which is the root of the Red-black tree.
 * @param[in] rbt From the @p root_meta, root of the Red-black tree.
 * @param[in,out] pool Pool from which the lyds data will be reused
 * @param[out] next Data node for which no free red-black node was available,
 * so not all nodes have been processed. Or is set to NULL, so all were sorted successfully.
 */
static void
lyds_additionally_reuse_rb_tree(struct lyd_node **first_sibling, struct lyd_node **leader, struct lyd_meta *root_meta,
        struct rb_node **rbt, struct lyds_pool *pool, struct lyd_node **next)
{
    ly_bool max;
    struct lyd_node *iter;

    /* let's begin with the leader */
    RBN_RESET(pool->rbn, *leader);
    *rbt = pool->rbn;
    pool->rbn = rb_iter_next(&pool->iter_state);

    /* continue with the rest of the nodes */
    for (iter = (*leader)->next; iter && (iter->schema == (*leader)->schema); iter = iter->next) {
        if (!pool->rbn) {
            *next = iter;
            return;
        }
        RBN_RESET(pool->rbn, iter);
        rb_insert_node(rbt, pool->rbn, &max);
        if (!max) {
            /* nodes were not sorted, they will be sorted now */
            lyd_unlink_ignore_lyds(first_sibling, iter);
            lyds_link_data_node(first_sibling, leader, iter, root_meta, pool->rbn);
        }
        pool->rbn = rb_iter_next(&pool->iter_state);
    }

    *next = NULL;
}

LY_ERR
lyds_create_metadata(struct lyd_node *leader, struct lyd_meta **meta_p)
{
    LY_ERR ret;
    uint32_t i;
    struct lyd_meta *meta;
    struct lys_module *modyang;

    assert(leader && (!leader->prev->next || (leader->schema != leader->prev->schema)));

    lyds_get_rb_tree(leader, &meta);
    if (meta) {
        /* nothing to do, the metadata is already set */
        return LY_SUCCESS;
    }

    /* Check that the 'yang' module is defined. */
    i = 0;
    while ((modyang = ly_ctx_get_module_iter(LYD_CTX(leader), &i))) {
        if (!strcmp(modyang->name, "yang")) {
            break;
        }
    }
    LY_CHECK_ERR_RET(!modyang, LOGERR(LYD_CTX(leader), LY_EINT, "The yang module is not installed."), LY_EINT);

    /* create new metadata, its rbt is NULL */
    ret = lyd_create_meta(leader, &meta, modyang, RB_NAME, RB_NAME_LEN, NULL, 0, 0, 1, NULL,
            LY_VALUE_CANON, NULL, LYD_HINT_DATA, NULL, 0, NULL);
    LY_CHECK_RET(ret);

    if (meta_p) {
        *meta_p = meta;
    }

    return LY_SUCCESS;
}

/**
 * @brief Create and insert a new red-black node.
 *
 * The data node itself is not sorted. To to this, call the lyds_link_data_node().
 *
 * @param[in] node Data node to be accessed from the Red-black tree.
 * @param[in,out] rbt Root of the Red-black tree.
 * @param[out] rbn Created and inserted red-black node containing @p node.
 * @return LY_ERR value.
 */
static LY_ERR
rb_insert(struct lyd_node *node, struct rb_node **rbt, struct rb_node **rbn)
{
    LY_ERR ret;

    /* create a new red-black node to which the @p node will be assigned */
    ret = lyds_create_node(node, rbn);
    LY_CHECK_RET(ret);

    /* insert red-black node with @p node to the Red-black tree */
    rb_insert_node(rbt, *rbn, NULL);

    return LY_SUCCESS;
}

LY_ERR
lyds_insert(struct lyd_node **first_sibling, struct lyd_node **leader, struct lyd_node *node)
{
    LY_ERR ret;
    struct rb_node *rbt, *rbn;
    struct lyd_meta *root_meta;

    /* @p node must not be part of another Red-black tree, only single node can satisfy this condition */
    assert(LYD_NODE_IS_ALONE(node) && leader && node);

    /* Clear the @p node. It may have unnecessary data due to duplication or due to lyds_unlink() calls. */
    rbt = lyds_get_rb_tree(node, &root_meta);
    if (root_meta) {
        assert(!rbt || (!RBN_LEFT(rbt) && !RBN_RIGHT(rbt)));
        /* metadata in @p node will certainly no longer be needed */
        lyd_free_meta_single(root_meta);
    }

    /* get the Red-black tree from the @p leader */
    rbt = lyds_get_rb_tree(*leader, &root_meta);
    if (!root_meta) {
        LY_CHECK_RET(lyds_create_metadata(*leader, &root_meta));
    }
    if (!rbt) {
        /* Due to optimization, the Red-black tree has not been created so far, so it will be
         * created additionally now. It may still not be worth creating a tree and it may be better
         * to insert the node by linear search instead, but that is a case for further optimization.
         */
        ret = lyds_additionally_create_rb_tree(first_sibling, leader, root_meta, &rbt);
        LY_CHECK_RET(ret);
    }

    /* Insert the node to the correct order. */
    ret = rb_insert(node, &rbt, &rbn);
    LY_CHECK_RET(ret);
    lyds_link_data_node(first_sibling, leader, node, root_meta, rbn);

    /* the root of the Red-black tree may changed due to insertion, so update the pointer to the root */
    RBT_SET(root_meta, rbt);

    return LY_SUCCESS;
}

LY_ERR
lyds_insert2(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node **leader,
        struct lyd_node *node, struct lyds_pool *pool)
{
    LY_ERR ret;
    struct rb_node *rbt = NULL, *rbn;
    struct lyd_node *ld, *next;
    struct lyd_meta *root_meta;

    assert(pool && pool->rbn && first_sibling && node);

    if (!*leader || ((*leader)->schema != node->schema)) {
        /* leader has not been visited yet */
        ld = NULL;
        lyd_find_sibling_schema(*first_sibling, node->schema, &ld);
        if (!ld) {
            /* the destination has no (leaf-)list instance yet. */
            lyd_insert_node_ordby_schema(parent, first_sibling, node);
            *leader = node;
            goto cleanup;
        } else {
            /* leader has been found */
            *leader = ld;
        }
    }

    /* get Red-black tree from leader */
    rbt = lyds_get_rb_tree(*leader, &root_meta);
    if (!root_meta) {
        /* leader needs metadata */
        root_meta = lyds_pool_get_meta(pool);
        if (!root_meta) {
            /* there is no free structure for metadata, a new one must be allocated */
            ret = lyds_create_metadata(*leader, &root_meta);
            LY_CHECK_RET(ret);
        } else {
            /* reuse metadata */
            lyd_insert_meta(*leader, root_meta, 0);
        }
    }
    if (!rbt) {
        /* Red-black tree needed */
        lyds_additionally_reuse_rb_tree(first_sibling, leader, root_meta, &rbt, pool, &next);
        if (next) {
            /* there is no free structure for red-black node, a new ones must be allocated */
            ret = lyds_additionally_create_rb_nodes(first_sibling, leader, root_meta, &rbt, next);
            LY_CHECK_RET(ret);
        }
    }

    /* insert @p node */
    if (pool->rbn) {
        /* reuse free red-black node and insert */
        rbn = pool->rbn;
        RBN_RESET(rbn, node);
        rb_insert_node(&rbt, rbn, NULL);
        /* prepare for the next node */
        pool->rbn = rb_iter_next(&pool->iter_state);
    } else {
        /* allocate new node and insert */
        ret = rb_insert(node, &rbt, &rbn);
        LY_CHECK_RET(ret);
    }
    /* connect @p node with siblings so that the order is maintained */
    lyds_link_data_node(first_sibling, leader, node, root_meta, rbn);

cleanup:
    if (rbt) {
        RBT_SET(root_meta, rbt);
    }
    lyd_insert_hash(node);
    *first_sibling = node->prev->next ? *first_sibling : node;

    return LY_SUCCESS;
}

void
lyds_unlink(struct lyd_node **leader, struct lyd_node *node)
{
    struct rb_node *rbt, *removed = NULL;
    struct lyd_meta *root_meta;

    if (!node || !leader || !*leader) {
        return;
    }

    /* get the Red-black tree from the leader */
    rbt = lyds_get_rb_tree(*leader, &root_meta);

    /* find out if leader_p is alone */
    if (!root_meta || LYD_NODE_IS_ALONE(*leader)) {
        return;
    }

    if (*leader == node) {
        /* move the metadata to the next instance. */
        lyds_move_meta((*leader)->next, root_meta);
    }

    rb_remove_node(root_meta, &rbt, node, &removed);
    rb_free_node(&removed);
}

void
lyds_split(struct lyd_node **first_sibling, struct lyd_node *leader, struct lyd_node *node, struct lyd_node **next_p)
{
    struct rb_node *rbt, *rbn;
    struct lyd_node *iter, *next, *start, *dst;
    struct lyd_meta *root_meta;

    assert(leader && node);

    rbt = lyds_get_rb_tree(leader, &root_meta);
    if (!rbt || (leader == node)) {
        /* Second list is just unlinked */
        start = node->next;
        lyd_unlink_ignore_lyds(first_sibling, node);
        dst = node;
        LY_LIST_FOR_SAFE(start, next, iter) {
            if (iter->schema != node->schema) {
                break;
            }
            lyd_unlink_ignore_lyds(first_sibling, iter);
            lyd_insert_after_node(&node, dst, iter);
            dst = iter;
        }
        *next_p = iter;
        return;
    }

    start = node->next;
    if (!start || (start->schema != node->schema)) {
        /* @p node is the last node, remove from Red-black tree and unlink */
        rb_remove_node(root_meta, &rbt, node, &rbn);
        rb_free_node(&rbn);
        lyd_unlink_ignore_lyds(first_sibling, node);
        *next_p = start;
        goto cleanup;
    }

    /* remove @p node from Red-black tree and unlink */
    rb_remove_node(root_meta, &rbt, node, &rbn);
    rb_free_node(&rbn);
    lyd_unlink_ignore_lyds(first_sibling, node);

    /* remove the rest of nodes from Red-black tree and unlink */
    dst = node;
    LY_LIST_FOR_SAFE(start, next, iter) {
        if (iter->schema != node->schema) {
            break;
        }
        rb_remove_node(root_meta, &rbt, iter, &rbn);
        rb_free_node(&rbn);
        lyd_unlink_ignore_lyds(first_sibling, iter);
        /* insert them to the second (leaf-)list */
        lyd_insert_after_node(&node, dst, iter);
        dst = iter;
    }
    *next_p = iter;

cleanup:
    RBT_SET(root_meta, rbt);
}

void
lyds_free_metadata(struct lyd_node *node)
{
    struct lyd_meta *root_meta;

    if (node) {
        lyds_get_rb_tree(node, &root_meta);
        lyd_free_meta_single(root_meta);
    }
}

/**
 * @brief Merge nodes (src) without Red-black tree into (dst) nodes with Red-black tree.
 *
 * @param[in,out] first_dst First sibling node, destination.
 * @param[in,out] leader_dst First instance of the destination (leaf-)list.
 * @param[in] root_meta_dst Metadata 'lyds_tree' from @p leader_dst.
 * @param[in] rbt_dst Root of the destination Red-black tree.
 * @param[in,out] first_src First sibling node, source.
 * @param[in] leader_src First instance of the source (leaf-)list.
 * @param[out] next_p Data node located after source (leaf-)list.
 * On error, it points to the some node in source (leaf-)list that failed to merge.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_merge_nodes1(struct lyd_node **first_dst, struct lyd_node **leader_dst, struct lyd_meta *root_meta_dst,
        struct rb_node *rbt_dst, struct lyd_node **first_src, struct lyd_node *leader_src, struct lyd_node **next_p)
{
    LY_ERR ret;
    struct rb_node *rbn;
    struct lyd_node *iter, *next;
    const struct lysc_node *schema;

    schema = leader_src->schema;
    for (iter = leader_src; iter && (iter->schema == schema); iter = next) {
        ret = rb_insert(iter, &rbt_dst, &rbn);
        if (ret) {
            /* allocation failed, @p next_p must refer to failed node */
            break;
        }
        next = iter->next;
        lyd_unlink_ignore_lyds(first_src, iter);
        lyds_link_data_node(first_dst, leader_dst, iter, root_meta_dst, rbn);
        lyd_insert_hash(iter);
    }
    *next_p = iter;

    RBT_SET(root_meta_dst, rbt_dst);

    return LY_SUCCESS;
}

/**
 * @brief Merge nodes which belongs before @p leader_dst.
 *
 * Reminder:
 * Merge nodes (src) with Red-black tree into (dst) nodes without Red-black tree.
 * Create red-black nodes from destination (leaf-)list and insert them into @p rbt_src.
 * At the end of the lyds_merge_nodes2(), rbt_src will move and become rbt_dst.
 *
 * @param[in,out] leader_dst First instance of the destination (leaf-)list.
 * @param[in,out] first_src First sibling node, source.
 * @param[in] leader_src First instance of the source (leaf-)list.
 * @param[in,out] rbt_src Root of the source Red-black tree.
 * @param[out] dst_iter Last merged node into the @p rbt_src.
 * @param[out] next_p On error, points to data node which failed to merge.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_merge_nodes2_front(struct lyd_node **leader_dst, struct lyd_node **first_src, struct lyd_node *leader_src,
        struct rb_node **rbt_src, struct rb_node **dst_iter, struct lyd_node **next_p)
{
    LY_ERR ret;
    struct rb_node *prev, *iter;
    struct lyd_node *dst;

    /* insert destination leader */
    ret = rb_insert(*leader_dst, rbt_src, dst_iter);
    LY_CHECK_ERR_RET(ret, *next_p = leader_src, ret);

    /* iterate over source RB tree and move the nodes belonging before destination leader */
    prev = rb_prev(*dst_iter);
    dst = *leader_dst;
    for (iter = prev; iter; iter = rb_prev(iter)) {
        lyd_unlink_ignore_lyds(first_src, RBN_DNODE(iter));
        lyd_insert_before_node(dst, RBN_DNODE(iter));
        lyd_insert_hash(RBN_DNODE(iter));
        dst = RBN_DNODE(iter);
    }
    if (prev) {
        *leader_dst = dst;
    }

    return LY_SUCCESS;
}

/**
 * @brief Merge nodes which belongs between destination (leaf-)list nodes.
 *
 * Reminder:
 * Merge nodes (src) with Red-black tree into (dst) nodes without Red-black tree.
 * Create red-black nodes from destination (leaf-)list and insert them into @p rbt_src.
 * At the end of the lyds_merge_nodes2(), rbt_src will move and become rbt_dst.
 *
 * @param[in,out] first_dst First sibling node, destination.
 * @param[in,out] leader_dst First instance of the destination (leaf-)list.
 * @param[in,out] first_src First sibling node, source.
 * @param[in,out] rbt_src Root of the source Red-black tree.
 * @param[out] dst_iter Last merged node into the @p rbt_src.
 * @param[out] next_p On error, points to data node which failed to merge.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_merge_nodes2_among(struct lyd_node **first_dst, struct lyd_node **leader_dst, struct lyd_node **first_src,
        struct rb_node **rbt_src, struct rb_node **dst_iter, struct lyd_node **next_p)
{
    LY_ERR ret;
    struct rb_node *rbn, *iter, *rbn_prev;
    struct lyd_node *dst, *node;
    const struct lysc_node *schema;

    schema = (*leader_dst)->schema;
    rbn = *dst_iter;
    for (node = RBN_DNODE(*dst_iter); node->next && (node->schema == schema); node = dst->next) {
        /* insert node from destination (leaf-)list into @p rbt_src */
        rbn_prev = rbn;
        ret = rb_insert(node->next, rbt_src, &rbn);
        LY_CHECK_ERR_RET(ret, *next_p = RBN_DNODE(rb_next(rbn_prev)), ret);

        dst = node;
        /* move source data nodes between next-to-last inserted node and last inserted node */
        for (iter = rb_next(rbn_prev); iter != rbn; iter = rb_next(iter)) {
            lyd_unlink_ignore_lyds(first_src, RBN_DNODE(iter));
            /* insert source data nodes into destination (leaf-)list */
            lyd_insert_after_node(first_dst, dst, RBN_DNODE(iter));
            lyd_insert_hash(RBN_DNODE(iter));
            dst = RBN_DNODE(iter);
        }
    }
    *dst_iter = rbn;

    return LY_SUCCESS;
}

/**
 * @brief Merge nodes which belongs after destination (leaf-)list nodes.
 *
 * Reminder:
 * Merge nodes (src) with Red-black tree into (dst) nodes without Red-black tree.
 * Create red-black nodes from destination (leaf-)list and insert them into @p rbt_src.
 * At the end of the lyds_merge_nodes2(), rbt_src will move and become rbt_dst.
 *
 * @param[in,out] first_dst First sibling node, destination.
 * @param[in,out] first_src First sibling node, source.
 * @param[in] dst_iter Last merged node into the @p rbt_src.
 * @param[out] next_p Data node located after source (leaf-)list.
 */
static void
lyds_merge_nodes2_back(struct lyd_node **first_dst, struct lyd_node **first_src, struct rb_node *dst_iter,
        struct lyd_node **next_p)
{
    struct rb_node *iter, *begin;
    struct lyd_node *dst;

    begin = rb_next(dst_iter);
    LY_CHECK_RET(!begin,; );
    dst = RBN_DNODE(dst_iter);
    for (iter = begin; iter; iter = rb_next(iter)) {
        *next_p = RBN_DNODE(iter)->next;
        lyd_unlink_ignore_lyds(first_src, RBN_DNODE(iter));
        lyd_insert_after_node(first_dst, dst, RBN_DNODE(iter));
        lyd_insert_hash(RBN_DNODE(iter));
        dst = RBN_DNODE(iter);
    }
}

/**
 * @brief Merge nodes (src) with Red-black tree into (dst) nodes without Red-black tree.
 *
 * Create red-black nodes from destination (leaf-)list and insert them into @p rbt_src.
 * At the end of the lyds_merge_nodes2(), rbt_src will move and become rbt_dst.
 *
 * @param[in,out] first_dst First sibling node, destination.
 * @param[in,out] leader_dst First instance of the destination (leaf-)list.
 * @param[in,out] first_src First sibling node, source.
 * @param[in] leader_src First instance of the source (leaf-)list.
 * @param[in] root_meta_src Metadata 'lyds_tree' from @p leader_src.
 * @param[in] rbt_src Root of the source Red-black tree.
 * @param[out] next_p Data node located after source (leaf-)list.
 * On error, points to data node which failed to merge.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_merge_nodes2(struct lyd_node **first_dst, struct lyd_node **leader_dst,
        struct lyd_node **first_src, struct lyd_node *leader_src, struct lyd_meta *root_meta_src,
        struct rb_node *rbt_src, struct lyd_node **next_p)
{
    LY_ERR ret;
    struct rb_node *dst_iter;

    /* merge first destination node, move source nodes which belongs before this node */
    ret = lyds_merge_nodes2_front(leader_dst, first_src, leader_src, &rbt_src, &dst_iter, next_p);
    LY_CHECK_GOTO(ret, cleanup);
    *first_dst = !(*first_dst)->prev->next ? *first_dst : *leader_dst;

    /* RB tree si moved from source to destination (leaf-)list */
    lyds_move_meta(*leader_dst, root_meta_src);

    /* merge the rest of the destination nodes, move corresponding source nodes */
    ret = lyds_merge_nodes2_among(first_dst, leader_dst, first_src, &rbt_src, &dst_iter, next_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* move the rest of the source nodes */
    lyds_merge_nodes2_back(first_dst, first_src, dst_iter, next_p);

    if (*next_p && ((*next_p)->schema == (*leader_dst)->schema)) {
        ret = lyds_merge_nodes1(first_dst, leader_dst, root_meta_src, rbt_src, first_src, *next_p, next_p);
    }

cleanup:
    RBT_SET(root_meta_src, rbt_src);

    return ret;
}

/**
 * @brief Merge nodes (src) with Red-black tree into (dst) nodes with Red-black tree.
 *
 * Possible improvement: find out which rb_tree is bigger and then move nodes into it.
 * Current implementation blindly guesses that the source Red-black tree is smaller.
 *
 * @param[in,out] first_dst First sibling node, destination.
 * @param[in,out] leader_dst First instance of the destination (leaf-)list.
 * @param[in] root_meta_dst Metadata 'lyds_tree' from @p leader_dst.
 * @param[in] rbt_dst Root of the destination Red-black tree.
 * @param[in,out] first_src First sibling node, source.
 * @param[in] root_meta_src Metadata 'lyds_tree' from @p leader_src.
 * @param[in] rbt_src Root of the source Red-black tree.
 * @param[out] next_p Data node located after source (leaf-)list.
 * @return LY_ERR value.
 */
static LY_ERR
lyds_merge_nodes3(struct lyd_node **first_dst, struct lyd_node **leader_dst, struct lyd_meta *root_meta_dst,
        struct rb_node *rbt_dst, struct lyd_node **first_src, struct lyd_meta *root_meta_src,
        struct rb_node *rbt_src, struct lyd_node **next_p)
{
    struct rb_node *iter, *iter_state;
    struct lyd_node *node;

    /* release source RB tree */
    RBT_SET(root_meta_src, NULL);
    lyd_free_meta_single(root_meta_src);

    /* 'randomly' iterate over all source nodes, merge and move to the destination */
    for (iter = rb_iter_begin(rbt_src, &iter_state); iter; iter = rb_iter_next(&iter_state)) {
        node = RBN_DNODE(iter);
        *next_p = node->next;
        RBN_RESET(iter, node);
        rb_insert_node(&rbt_dst, iter, NULL);
        lyd_unlink_ignore_lyds(first_src, node);
        lyds_link_data_node(first_dst, leader_dst, node, root_meta_dst, iter);
        lyd_insert_hash(node);
    }

    if (!*next_p || ((*next_p)->schema != (*leader_dst)->schema)) {
        RBT_SET(root_meta_dst, rbt_dst);
    } else {
        LY_CHECK_RET(lyds_merge_nodes1(first_dst, leader_dst, root_meta_dst, rbt_dst, first_src, *next_p, next_p));
    }

    return LY_SUCCESS;
}

LY_ERR
lyds_merge(struct lyd_node **first_dst, struct lyd_node **leader_dst, struct lyd_node **first_src,
        struct lyd_node *leader_src, struct lyd_node **next_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct rb_node *rbt_dst, *rbt_src;
    struct lyd_meta *root_meta_dst, *root_meta_src = NULL;

    assert(leader_dst && leader_src && next_p);

    rbt_dst = lyds_get_rb_tree(*leader_dst, &root_meta_dst);
    rbt_src = lyds_get_rb_tree(leader_src, &root_meta_src);

    if (root_meta_src && !rbt_src) {
        /* Release unnecessary metadata which can be empty, e.g. when duplicating a node */
        lyd_free_meta_single(root_meta_src);
    }

    if (!rbt_dst && !rbt_src) {
        /* create RB tree from destination nodes, merge and move source nodes */
        LY_CHECK_RET(lyds_create_metadata(*leader_dst, &root_meta_dst));
        LY_CHECK_RET(lyds_additionally_create_rb_tree(first_dst, leader_dst, root_meta_dst, &rbt_dst));
        ret = lyds_merge_nodes1(first_dst, leader_dst, root_meta_dst, rbt_dst, first_src, leader_src, next_p);
    } else if (rbt_dst && !rbt_src) {
        /* just merge and move source nodes */
        ret = lyds_merge_nodes1(first_dst, leader_dst, root_meta_dst, rbt_dst, first_src, leader_src, next_p);
    } else if (!rbt_dst && rbt_src) {
        /* merge destination nodes with RB tree, move RB tree and source nodes */
        ret = lyds_merge_nodes2(first_dst, leader_dst, first_src, leader_src, root_meta_src, rbt_src, next_p);
    } else {
        /* merge and move source nodes, release source RB tree */
        assert(rbt_dst && rbt_src);
        lyds_merge_nodes3(first_dst, leader_dst, root_meta_dst, rbt_dst, first_src, root_meta_src, rbt_src, next_p);
    }

    return ret;
}

int
lyds_compare_single(struct lyd_node *node1, struct lyd_node *node2)
{
    assert(node1 && node2 && (node1->schema == node2->schema) && lyds_is_supported(node1));

    if (node1 == node2) {
        return 0;
    } else if (node1->schema->nodetype == LYS_LEAFLIST) {
        return rb_compare_leaflists(node1, node2);
    } else {
        return rb_compare_lists(node1, node2);
    }
}
