/**
 * @file tree_schema_free.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief internal freeing functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_FREE_H_
#define LY_TREE_SCHEMA_FREE_H_

#include "set.h"
#include "tree_schema.h"

struct lysp_yang_ctx;
struct lysp_yin_ctx;

struct lysf_ctx {
    struct ly_ctx *ctx;
    struct lys_module *mod;
    struct ly_set ext_set;
};

/**
 * @brief Macro to free [sized array](@ref sizedarrays) of items using the provided free function. The ARRAY itself is also freed,
 * but the memory is not sanitized.
 */
#define FREE_ARRAY(CTX, ARRAY, FUNC) {LY_ARRAY_COUNT_TYPE c__; LY_ARRAY_FOR(ARRAY, c__){(FUNC)(CTX, &(ARRAY)[c__]);}LY_ARRAY_FREE(ARRAY);}

/**
 * @brief Macro to free the specified MEMBER of a structure using the provided free function. The memory is not sanitized.
 */
#define FREE_MEMBER(CTX, MEMBER, FUNC) if (MEMBER) {(FUNC)(CTX, MEMBER);free(MEMBER);}

/**
 * @brief Macro to free [sized array](@ref sizedarrays) of strings stored in the context's dictionary. The ARRAY itself is also freed,
 * but the memory is not sanitized.
 */
#define FREE_STRINGS(CTX, ARRAY) {LY_ARRAY_COUNT_TYPE c__; LY_ARRAY_FOR(ARRAY, c__){lydict_remove(CTX, ARRAY[c__]);}LY_ARRAY_FREE(ARRAY);}

/**
 * @brief Free a parsed qualified name.
 *
 * @param[in] ctx libyang context.
 * @param[in] qname Qualified name to free.
 */
void lysp_qname_free(const struct ly_ctx *ctx, struct lysp_qname *qname);

/**
 * @brief Free the parsed extension instance structure.
 *
 * @param[in] ctx Free context.
 * @param[in] ext Parsed extension instance structure to free. Note that the instance itself is not freed.
 */
void lysp_ext_instance_free(struct lysf_ctx *ctx, struct lysp_ext_instance *ext);

/**
 * @brief Free a parsed restriction.
 *
 * @param[in] ctx Free context.
 * @param[in] restr Restriction to free.
 */
void lysp_restr_free(struct lysf_ctx *ctx, struct lysp_restr *restr);

/**
 * @brief Free the parsed type structure.
 *
 * @param[in] ctx Free context.
 * @param[in] type Parsed schema type structure to free. Note that the type itself is not freed.
 */
void lysp_type_free(struct lysf_ctx *ctx, struct lysp_type *type);

/**
 * @brief Free the parsed when structure.
 *
 * @param[in] ctx Free context.
 * @param[in] when Parsed schema when structure to free. Note that the structure itself is not freed.
 */
void lysp_when_free(struct lysf_ctx *ctx, struct lysp_when *when);

/**
 * @brief Free the parsed deviate structure.
 *
 * @param[in] ctx Free context.
 * @param[in] d Parsed schema deviate structure to free. Note that the structure itself is not freed.
 */
void lysp_deviate_free(struct lysf_ctx *ctx, struct lysp_deviate *d);

/**
 * @brief Free the parsed deviation structure.
 *
 * @param[in] ctx Free context.
 * @param[in] dev Parsed schema deviation structure to free. Note that the structure itself is not freed.
 */
void lysp_deviation_free(struct lysf_ctx *ctx, struct lysp_deviation *dev);

/**
 * @brief Free a parsed node.
 *
 * @param[in] ctx Free context.
 * @param[in] node Node to free.
 */
void lysp_node_free(struct lysf_ctx *ctx, struct lysp_node *node);

/**
 * @brief Free the parsed YANG schema tree structure. Works for both modules and submodules.
 *
 * @param[in] ctx Free context.
 * @param[in] module Parsed YANG schema tree structure to free.
 */
void lysp_module_free(struct lysf_ctx *ctx, struct lysp_module *module);

/**
 * @brief Free the compiled extension instance structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] ext Compiled extension instance structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_ext_instance_free(struct lysf_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Free the compiled if-feature structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] iff Compiled if-feature structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_iffeature_free(struct lysf_ctx *ctx, struct lysc_iffeature *iff);

/**
 * @brief Free a compiled pattern.
 *
 * @param[in] ctx Free context.
 * @param[in] pattern Pointer to the pattern to free.
 */
void lysc_pattern_free(struct lysf_ctx *ctx, struct lysc_pattern **pattern);

/**
 * @brief Free a bit/enum item.
 *
 * @param[in] ctx Free context.
 * @param[in] item Bit/enum item to free.
 */
void lysc_enum_item_free(struct lysf_ctx *ctx, struct lysc_type_bitenum_item *item);

/**
 * @brief Free the compiled type structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] type Compiled type structure to be freed. The structure has refcount, so it is freed only in case
 * the value is decreased to 0.
 */
void lysc_type_free(struct lysf_ctx *ctx, struct lysc_type *type);

/**
 * @brief Free the compiled container node structure.
 *
 * Only the container-specific members are freed, for generic node free function,
 * use ::lysc_node_free().
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled container node structure to be freed.
 */
void lysc_node_container_free(struct lysf_ctx *ctx, struct lysc_node_container *node);

/**
 * @brief Free the compiled node structure.
 *
 * @param[in] ctx Free context.
 * @param[in] node Compiled node structure to be freed.
 * @param[in] unlink Whether to first unlink the node before freeing.
 */
void lysc_node_free(struct lysf_ctx *ctx, struct lysc_node *node, ly_bool unlink);

/**
 * @brief Free the compiled schema structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] module Compiled schema module structure to free.
 */
void lysc_module_free(struct lysf_ctx *ctx, struct lysc_module *module);

/**
 * @brief Free the schema structure. It just frees, it does not remove the schema from its context.
 *
 * @param[in] ctx Free context.
 * @param[in,out] module Schema module structure to free.
 * @param[in] remove_links Whether to remove links in other modules to structures in this module. Not needed if
 * the whole context is being freed.
 */
void lys_module_free(struct lysf_ctx *ctx, struct lys_module *module, ly_bool remove_links);

/**
 * @brief Erase free context.
 *
 * @param[in] ctx Free context to erase.
 */
void lysf_ctx_erase(struct lysf_ctx *ctx);

/**
 * @brief Free lys parser context.
 *
 * @param[in] ctx Context to free.
 */
void lysp_yang_ctx_free(struct lysp_yang_ctx *ctx);

/**
 * @brief Free yin parser context
 *
 * @param[in] ctx Context to free.
 */
void lysp_yin_ctx_free(struct lysp_yin_ctx *ctx);

#endif /* LY_TREE_SCHEMA_FREE_H_ */
