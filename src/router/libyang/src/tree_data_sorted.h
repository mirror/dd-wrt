/**
 * @file tree_data_sorted.h
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Binary search tree (BST) for sorting data nodes.
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef _LYDS_TREE_H_
#define _LYDS_TREE_H_

#include "log.h"

struct lyd_node;
struct rb_node;
struct lyd_meta;

/* This functionality applies to list and leaf-list with the "ordered-by system" statement,
 * which is implicit. The BST is implemented using a Red-black tree and is used for sorting nodes.
 * For example, a list of valid users would typically be sorted alphabetically. This tree is saved
 * in the first instance of the leaf-list/list in the metadata named lyds_tree. Thanks to the tree,
 * it is possible to insert a sibling data node in such a way that the order of the nodes is preserved.
 * The decision whether the value is greater or less takes place in the callback function ::lyplg_type_sort_clb
 * in the corresponding type.
 *
 * Parameters always must be from the same context.
 */

/**
 * @brief BST and 'lyds_tree' metadata pool.
 *
 * The structure stores a free Red-black tree and metadata, which can be reused. Thanks to this,
 * it is possible to work with data more efficiently and thus prevent repeated allocation and freeing of memory.
 */
struct lyds_pool {
    struct rb_node *rbn;            /**< The current free node to use. If set, the pool is not empty. */
    struct lyd_meta *meta;          /**< Pointer to the list of free 'lyds_tree' metadata to reuse. */
    /* Private items */
    struct rb_node *iter_state;     /**< Internal iterator over a Red-black tree. Pointer to a successor. */
};

/**
 * @brief Check that ordering is supported for the @p node.
 *
 * If the function returns 0 for a given node, other lyds_* or rb_* functions must not be called for this node.
 *
 * @param[in] node Node to check. Expected (leaf-)list or list with key(s).
 * @return 1 if @p node can be sorted.
 */
ly_bool lyds_is_supported(const struct lyd_node *node);

/**
 * @brief Determine if the data node supports lyds and if the node is a 'leader'.
 *
 * @param[in] NODE Data node (struct lyd_node) to check.
 * @return 1 if @p NODE is the first instance of the (leaf-)list with lyds support.
 */
#define LYDS_NODE_IS_LEADER(NODE) \
    (lyds_is_supported(NODE) && (!NODE->prev->next || (NODE->prev->schema != NODE->schema)))

/**
 * @brief Create the 'lyds_tree' metadata.
 *
 * @param[in] leader First instance of the (leaf-)list. If the node already contains the metadata,
 * then nothing happens. The BST is unchanged or empty.
 * @param[out] meta Newly created 'lyds_tree' metadata.
 * @return LY_ERR value.
 */
LY_ERR lyds_create_metadata(struct lyd_node *leader, struct lyd_meta **meta);

/**
 * @brief Create new BST node.
 *
 * @param[in] node Data node to link with new red-black node.
 * @param[out] rbn Created red-black node.
 * @return LY_SUCCESS on success.
 */
LY_ERR lyds_create_node(struct lyd_node *node, struct rb_node **rbn);

/**
 * @brief Insert the @p node into BST and into @p leader's siblings.
 *
 * Sibling data nodes of the @p leader are also modified for sorting to take place.
 * The function automatically take care of lyds_create_metadata() and lyds_create_tree() calls.
 * Hash for data nodes is not added.
 *
 * @param[in,out] first_sibling First sibling node, used for optimization.
 * @param[in,out] leader First instance of the (leaf-)list. After executing the function,
 * @p leader does not have to be be first if @p node was inserted before @p leader.
 * @param[in] node A single (without siblings) node or tree to be inserted. It must be unlinked.
 * @return LY_ERR value.
 */
LY_ERR lyds_insert(struct lyd_node **first_sibling, struct lyd_node **leader, struct lyd_node *node);

/**
 * @brief Insert the @p node into BST and into @p leader's siblings and use @p pool.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first_sibling First sibling, NULL if no top-level sibling exist yet.
 * Can be also NULL if @p parent is set.
 * @param[in,out] leader First instance of the (leaf-)list. It can be set to NULL or even incorrectly set,
 * in which case the leader is found inside the function. It is convenient to keep the found pointer
 * at the caller and be used during the next call.
 * @param[in] node Individual node (without siblings) to insert.
 * @param[in,out] pool Pool from which the lyds data will be reused. If empty, data is allocated as needed.
 * @return LY_ERR value.
 */
LY_ERR lyds_insert2(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node **leader,
        struct lyd_node *node, struct lyds_pool *pool);

/**
 * @brief Unlink (remove) the specified data node from BST.
 *
 * Pointers in sibling data nodes (lyd_node) are NOT modified. This means that the data node is NOT unlinked.
 * Even if the BST will remain empty, lyds_tree metadata will remain.
 * Hash for data nodes is not removed.
 *
 * @param[in,out] leader First instance of (leaf-)list. If it is NULL, nothing happens.
 * @param[in] node Some instance of (leaf-)list to be unlinked.
 */
void lyds_unlink(struct lyd_node **leader, struct lyd_node *node);

/**
 * @brief Unlink lyds data from @p leader and add them to the pool.
 *
 * @param[in] leader First instance of the (leaf-)list.
 * @param[in,out] pool Pool to add lyds data to.
 */
void lyds_pool_add(struct lyd_node *leader, struct lyds_pool *pool);

/**
 * @brief Clear lyds data in pool, memory deallocation.
 *
 * @param[in,out] pool Pool from which the lyds data will be released. Pointers will be set to NULL.
 */
void lyds_pool_clean(struct lyds_pool *pool);

/**
 * @brief Split the (leaf-)list in two.
 *
 * The second (leaf-)list is unlinked from the rest of the data nodes.
 * The hash for the data nodes is removed.
 *
 * @param[in,out] first_sibling First sibling node, used for optimization.
 * @param[in] leader First instance of (leaf-)list.
 * @param[in] node Node in the (leaf-)list from which the second (leaf-)list will start.
 * @param[out] next Data node located after the second (leaf-)list.
 * The rest of the (leaf-)list nodes will belong under the second (leaf-)list.
 */
void lyds_split(struct lyd_node **first_sibling, struct lyd_node *leader, struct lyd_node *node,
        struct lyd_node **next);

/**
 * @brief Merge source (leaf-)list nodes into destination (leaf-)list nodes.
 *
 * Pointers in sibling data nodes (lyd_node) are modified.
 * The hash for the data nodes will be adjusted.
 *
 * @param[in,out] first_dst First sibling node, destination.
 * @param[in,out] leader_dst Destination (leaf-)list, first instance. It may not contain
 * the lyds_tree metadata or BST. After merge @p leader_dst can be reset to new leader.
 * @param[in,out] first_src First sibling node, source.
 * @param[in] leader_src Source (leaf-)list, first instance. It may not contain the lyds_tree metadata or BST.
 * @param[out] next Data node located after source (leaf-)list.
 * On error, points to data node which failed to merge.
 * @return LY_ERR value.
 */
LY_ERR lyds_merge(struct lyd_node **first_dst, struct lyd_node **leader_dst, struct lyd_node **first_src,
        struct lyd_node *leader_src, struct lyd_node **next);

/**
 * @brief Compare (sort) 2 data nodes.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @return Negative number if val1 < val2,
 * @return Zero if val1 == val2,
 * @return Positive number if val1 > val2.
 */
int lyds_compare_single(struct lyd_node *node1, struct lyd_node *node2);

/**
 * @brief Release the metadata including BST.
 *
 * No more nodes can be inserted after the function is executed.
 *
 * @param[in] node Data node of the type (leaf-)list that may contain metadata and BST.
 */
void lyds_free_metadata(struct lyd_node *node);

/**
 * @brief Release all BST nodes including the root.
 *
 * @param[in] rbt Root of the Red-black tree.
 */
void lyds_free_tree(struct rb_node *rbt);

#endif /* _LYDS_TREE_H_ */
