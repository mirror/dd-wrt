/**
 * @file diff.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief internal diff header
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_DIFF_H_
#define LY_DIFF_H_

#include <stdint.h>

#include "log.h"

struct lyd_node;

/**
 * @brief Internal structure for storing current (virtual) user-ordered instances order.
 */
struct lyd_diff_userord {
    const struct lysc_node *schema; /**< User-ordered list/leaf-list schema node. */
    uint64_t pos;                   /**< Current position in the second tree. */
    const struct lyd_node **inst;   /**< Sized array of current instance order. */
};

/**
 * @brief Diff operations.
 */
enum lyd_diff_op {
    LYD_DIFF_OP_CREATE,    /**< Subtree created. */
    LYD_DIFF_OP_DELETE,    /**< Subtree deleted. */
    LYD_DIFF_OP_REPLACE,   /**< Node value changed or (leaf-)list instance moved. */
    LYD_DIFF_OP_NONE       /**< No change of an existing inner node or default flag change of a term node. */
};

/**
 * @brief Add a new change into diff.
 *
 * @param[in] node Node (subtree) to add into diff.
 * @param[in] op Operation to set.
 * @param[in] orig_default Original default metadata to set.
 * @param[in] orig_value Original value metadata to set.
 * @param[in] key Key metadata to set.
 * @param[in] value Value metadata to set.
 * @param[in] position Position metadata to set.
 * @param[in] orig_key Original key metadata to set.
 * @param[in] orig_position Original position metadata to set.
 * @param[in,out] diff Diff to append to.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_diff_add(const struct lyd_node *node, enum lyd_diff_op op, const char *orig_default, const char *orig_value,
        const char *key, const char *value, const char *position, const char *orig_key, const char *orig_position,
        struct lyd_node **diff);

#endif /* LY_DIFF_H_ */
