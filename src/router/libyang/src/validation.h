/**
 * @file validation.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Validation routines.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_VALIDATION_H_
#define LY_VALIDATION_H_

#include <stdint.h>

#include "log.h"

struct ly_ctx;
struct ly_set;
struct lyd_node;
struct lys_module;
struct lysc_node;

enum lyd_diff_op;

/**
 * @brief Add information about the node's extensions having their own validation callback into an unres set.
 *
 * @param[in,out] node_exts Unres set for holding information for validating extension instances.
 * @param[in] node Data node to be examined.
 * @return LY_ERR values.
 */
LY_ERR lysc_node_ext_tovalidate(struct ly_set *node_exts, struct lyd_node *node);

/**
 * @brief Add new changes into a diff. They are always merged.
 *
 * @param[in] node Node/subtree to add.
 * @param[in] op Operation of the change.
 * @param[in,out] diff Diff to update.
 * @return LY_ERR value.
 */
LY_ERR lyd_val_diff_add(const struct lyd_node *node, enum lyd_diff_op op, struct lyd_node **diff);

/**
 * @brief Finish validation of nodes and attributes. Specifically, when (is processed first) and type validation.
 *
 * !! It is assumed autodeleted nodes cannot be in the unresolved node type set !!
 *
 * @param[in,out] tree Data tree, is updated if some nodes are autodeleted.
 * @param[in] mod Module of the @p tree to take into consideration when deleting @p tree and moving it.
 * If set, it is expected @p tree should point to the first node of @p mod. Otherwise it will simply be
 * the first top-level sibling.
 * @param[in] node_when Set with nodes with "when" conditions, can be NULL.
 * @param[in] node_exts Set with nodes with extension instances with validation plugin callback, can be NULL.
 * @param[in] node_types Set with nodes with unresolved types, can be NULL
 * @param[in] meta_types Set with metdata with unresolved types, can be NULL.
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_unres(struct lyd_node **tree, const struct lys_module *mod, struct ly_set *node_when,
        struct ly_set *node_exts, struct ly_set *node_types, struct ly_set *meta_types, struct lyd_node **diff);

/**
 * @brief Validate new siblings. Specifically, check duplicated instances, autodelete default values and cases.
 *
 * !! It is assumed autodeleted nodes cannot yet be in the unresolved node type set !!
 *
 * @param[in,out] first First sibling.
 * @param[in] sparent Schema parent of the siblings, NULL for top-level siblings.
 * @param[in] mod Module of the siblings, NULL for nested siblings.
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate_new(struct lyd_node **first, const struct lysc_node *sparent, const struct lys_module *mod,
        struct lyd_node **diff);

/**
 * @brief Validate a data tree.
 *
 * @param[in,out] tree Data tree to validate, nodes may be autodeleted.
 * @param[in] module Module whose data (and schema restrictions) to validate, NULL for all modules.
 * @param[in] ctx libyang context.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[in] validate_subtree Whether subtree was already validated (as part of data parsing) or not (separate validation).
 * @param[in] node_when_p Set of nodes with when conditions, if NULL a local set is used.
 * @param[in] node_exts Set with nodes with extension instances with validation plugin callback, if NULL a local set is used.
 * @param[in] node_types_p Set of unres node types, if NULL a local set is used.
 * @param[in] meta_types_p Set of unres metadata types, if NULL a local set is used.
 * @param[out] diff Generated validation diff, not generated if NULL.
 * @return LY_ERR value.
 */
LY_ERR lyd_validate(struct lyd_node **tree, const struct lys_module *module, const struct ly_ctx *ctx, uint32_t val_opts,
        ly_bool validate_subtree, struct ly_set *node_when_p, struct ly_set *node_exts_p,
        struct ly_set *node_types_p, struct ly_set *meta_types_p, struct lyd_node **diff);

#endif /* LY_VALIDATION_H_ */
