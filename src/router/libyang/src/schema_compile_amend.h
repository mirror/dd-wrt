/**
 * @file schema_compile_amend.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Header for schema compilation of augments, deviations, and refines.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_COMPILE_AMEND_H_
#define LY_SCHEMA_COMPILE_AMEND_H_

#include "log.h"

struct ly_ctx;
struct lysp_qname;
struct lysp_node;
struct lysc_node;
struct lysc_ctx;
struct lysp_node_uses;
struct lys_module;

/**
 * @brief Compiled parsed augment structure. Just a temporary storage for applying the augment to data.
 */
struct lysc_augment {
    struct lyxp_expr *nodeid;                    /**< augment target */
    const struct lysp_module *aug_pmod;          /**< module where the augment is defined, for top-level augments
                                                      used to resolve prefixes, for uses augments used as the context pmod */
    const struct lysc_node *nodeid_ctx_node;     /**< nodeid context node for relative targets */

    struct lysp_node_augment *aug_p;             /**< pointer to the parsed augment to apply */
};

/**
 * @brief Compiled parsed deviation structure. Just a temporary storage for applying the deviation to data.
 */
struct lysc_deviation {
    struct lyxp_expr *nodeid;                    /**< deviation target, taken from the first deviation in
                                                      ::lysc_deviation.dev_pmods array, this module is used for resolving
                                                      prefixes used in the nodeid. */

    struct lysp_deviation **devs;                /**< sized array of all the parsed deviations for one target node */
    const struct lysp_module **dev_pmods;        /**< sized array of parsed modules of @p devs (where the specific deviations
                                                      are defined). */
    ly_bool not_supported;                       /**< whether this is a not-supported deviation */
};

/**
 * @brief Compiled parsed refine structure. Just a temporary storage for applying the refine to data.
 */
struct lysc_refine {
    struct lyxp_expr *nodeid;                    /**< refine target */
    const struct lysp_module *nodeid_pmod;       /**< module where the nodeid is defined, used to resolve prefixes */
    const struct lysc_node *nodeid_ctx_node;     /**< nodeid context node */
    struct lysp_node_uses *uses_p;               /**< parsed uses node of the refine, for tracking recursive refines */

    struct lysp_refine **rfns;                   /**< sized array of parsed refines to apply */
};

/**
 * @brief Prepare any uses augments and refines in the context to be applied during uses descendant node compilation.
 *
 * @param[in] ctx Compile context.
 * @param[in] uses_p Parsed uses structure with augments and refines.
 * @param[in] ctx_node Context node of @p uses_p meaning its first data definiition parent.
 * @return LY_ERR value.
 */
LY_ERR lys_precompile_uses_augments_refines(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p,
        const struct lysc_node *ctx_node);

/**
 * @brief Duplicate qname structure.
 *
 * @param[in] ctx libyang context.
 * @param[in,out] qname Structure to fill.
 * @param[in] orig_qname Structure to read from.
 * @return LY_ERR value.
 */
LY_ERR lysp_qname_dup(const struct ly_ctx *ctx, struct lysp_qname *qname, const struct lysp_qname *orig_qname);

/**
 * @brief Free a compiled augment temporary structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] aug Structure to free.
 */
void lysc_augment_free(const struct ly_ctx *ctx, struct lysc_augment *aug);

/**
 * @brief Free a compiled deviation temporary structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] dev Structure to free.
 */
void lysc_deviation_free(const struct ly_ctx *ctx, struct lysc_deviation *dev);

/**
 * @brief Free a compiled refine temporary structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] rfn Structure to free.
 */
void lysc_refine_free(const struct ly_ctx *ctx, struct lysc_refine *rfn);

/**
 * @brief Free a duplicate of parsed node. It is returned by ::lys_compile_node_deviations_refines().
 *
 * @param[in] ctx libyang context.
 * @param[in] dev_pnode Parsed node to free.
 */
void lysp_dev_node_free(const struct ly_ctx *ctx, struct lysp_node *dev_pnode);

/**
 * @brief Compile and apply any precompiled deviations and refines targetting a node.
 *
 * @param[in] ctx Compile context.
 * @param[in] pnode Parsed node to consider.
 * @param[in] parent First compiled parent of @p pnode.
 * @param[out] dev_pnode Copy of parsed node @p pnode with deviations and refines, if any. NULL if there are none.
 * @param[out] no_supported Whether a not-supported deviation is defined for the node.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_node_deviations_refines(struct lysc_ctx *ctx, const struct lysp_node *pnode,
        const struct lysc_node *parent, struct lysp_node **dev_pnode, ly_bool *not_supported);

/**
 * @brief Compile and apply any precompiled top-level or uses augments targetting a node.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Compiled node to consider.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_node_augments(struct lysc_ctx *ctx, struct lysc_node *node);

/**
 * @brief Prepare all top-level augments for the current module to be applied during data nodes compilation.
 *
 * @param[in] ctx Compile context.
 * @return LY_ERR value.
 */
LY_ERR lys_precompile_own_augments(struct lysc_ctx *ctx);

/**
 * @brief Prepare all deviations for the current module to be applied during data nodes compilation.
 *
 * @param[in] ctx Compile context.
 * @return LY_ERR value.
 */
LY_ERR lys_precompile_own_deviations(struct lysc_ctx *ctx);

/**
 * @brief Compile top-level augments and deviations defined in the current module.
 * Generally, just add the module refence to the target modules. But in case
 * of foreign augments, they are directly applied.
 *
 * @param[in] ctx Compile context.
 * @return LY_ERR value.
 */
LY_ERR lys_precompile_augments_deviations(struct lysc_ctx *ctx);

/**
 * @brief Revert precompilation of module augments and deviations. Meaning remove its reference from
 * all the target modules.
 *
 * @param[in] ctx libyang context.
 * @param[in] mod Mod whose precompilation to revert.
 */
void lys_precompile_augments_deviations_revert(struct ly_ctx *ctx, const struct lys_module *mod);

#endif /* LY_SCHEMA_COMPILE_AMEND_H_ */
