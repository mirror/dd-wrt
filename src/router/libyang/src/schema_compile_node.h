/**
 * @file schema_compile_node.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for schema compilation of common nodes.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_COMPILE_NODE_H_
#define LY_SCHEMA_COMPILE_NODE_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "tree.h"
#include "tree_schema.h"

struct ly_ctx;
struct ly_set;
struct lysc_ctx;

/**
 * @brief Compile information from the when statement by either standard compilation or by reusing
 * another compiled when structure.
 *
 * @param[in] ctx Compile context.
 * @param[in] when_p Parsed when structure.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in] parent Parent node, if any.
 * @param[in] ctx_node Context node for the when statement.
 * @param[in] node Compiled node to which to add the compiled when.
 * @param[in,out] when_c Optional, pointer to the previously compiled @p when_p to be reused. Set to NULL
 * for the first call.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_when(struct lysc_ctx *ctx, const struct lysp_when *when_p, uint16_t inherited_flags,
        const struct lysc_node *parent, const struct lysc_node *ctx_node, struct lysc_node *node, struct lysc_when **when_c);

/**
 * @brief Compile information from the must statement
 *
 * @param[in] ctx Compile context.
 * @param[in] must_p The parsed must statement structure.
 * @param[in,out] must Prepared (empty) compiled must structure to fill.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_must(struct lysc_ctx *ctx, const struct lysp_restr *must_p, struct lysc_must *must);

/**
 * @brief Compile the parsed range restriction.
 *
 * @param[in] ctx Compile context.
 * @param[in] range_p Parsed range structure to compile.
 * @param[in] basetype Base YANG built-in type of the node with the range restriction.
 * @param[in] length_restr Flag to distinguish between range and length restrictions. Only for logging.
 * @param[in] frdigits The fraction-digits value in case of LY_TYPE_DEC64 basetype.
 * @param[in] base_range Range restriction of the type from which the current type is derived. The current
 * range restriction must be more restrictive than the base_range.
 * @param[in,out] range Pointer to the created current range structure.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_type_range(struct lysc_ctx *ctx, const struct lysp_restr *range_p, LY_DATA_TYPE basetype,
        ly_bool length_restr, uint8_t frdigits, struct lysc_range *base_range, struct lysc_range **range);

/**
 * @brief Checks pattern syntax.
 *
 * @param[in] ctx Context.
 * @param[in] pattern Pattern to check.
 * @param[in,out] code Compiled PCRE2 pattern. If NULL, the compiled information used to validate pattern are freed.
 * @return LY_ERR value - LY_SUCCESS, LY_EMEM, LY_EVALID.
 */
LY_ERR lys_compile_type_pattern_check(struct ly_ctx *ctx, const char *pattern, pcre2_code **code);

/**
 * @brief Compile parsed pattern restriction in conjunction with the patterns from base type.
 *
 * @param[in] ctx Compile context.
 * @param[in] patterns_p Array of parsed patterns from the current type to compile.
 * @param[in] base_patterns Compiled patterns from the type from which the current type is derived.
 * Patterns from the base type are inherited to have all the patterns that have to match at one place.
 * @param[out] patterns Pointer to the storage for the patterns of the current type.
 * @return LY_ERR LY_SUCCESS, LY_EMEM, LY_EVALID.
 */
LY_ERR lys_compile_type_patterns(struct lysc_ctx *ctx, const struct lysp_restr *patterns_p,
        struct lysc_pattern **base_patterns, struct lysc_pattern ***patterns);

/**
 * @brief Compile parsed type's enum structures (for enumeration and bits types).
 *
 * @param[in] ctx Compile context.
 * @param[in] enums_p Array of the parsed enum structures to compile.
 * @param[in] basetype Base YANG built-in type from which the current type is derived. Only LY_TYPE_ENUM and LY_TYPE_BITS are expected.
 * @param[in] base_enums Array of the compiled enums information from the (latest) base type to check if the current enums are compatible.
 * @param[out] bitenums Newly created array of the compiled bitenums information for the current type.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_type_enums(struct lysc_ctx *ctx, const struct lysp_type_enum *enums_p, LY_DATA_TYPE basetype,
        struct lysc_type_bitenum_item *base_enums, struct lysc_type_bitenum_item **bitenums);

/**
 * @brief Compile information about the leaf/leaf-list's type.
 *
 * @param[in] ctx Compile context.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status of referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[out] type Newly created (or reused with increased refcount) type structure with the filled information about the type.
 * @param[out] units Storage for inheriting units value from the typedefs the current type derives from.
 * @param[out] dflt Default value for the type.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_type(struct lysc_ctx *ctx, struct lysp_node *context_pnode, uint16_t context_flags,
        const char *context_name, const struct lysp_type *type_p, struct lysc_type **type, const char **units,
        struct lysp_qname **dflt);

/**
 * @brief Connect the node into the siblings list and check its name uniqueness. Also,
 * keep specific order of augments targetting the same node.
 *
 * @param[in] ctx Compile context
 * @param[in] parent Parent node holding the children list, in case of node from a choice's case,
 * the choice itself is expected instead of a specific case node.
 * @param[in] node Schema node to connect into the list.
 * @return LY_ERR value - LY_SUCCESS or LY_EEXIST.
 * In case of LY_EEXIST, the node is actually kept in the tree, so do not free it directly.
 */
LY_ERR lys_compile_node_connect(struct lysc_ctx *ctx, struct lysc_node *parent, struct lysc_node *node);

/**
 * @brief Compile parsed action's input/output node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed inout node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node_() with filled generic node information
 * is enriched with the inout-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_node_action_inout(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node);

/**
 * @brief Compile choice children.
 *
 * @param[in] ctx Compile context
 * @param[in] child_p Parsed choice children nodes.
 * @param[in] node Compiled choice node to compile and add children to.
 * @param[in,out] child_set Optional set to add all the compiled nodes into (can be more in case of uses).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_node_choice_child(struct lysc_ctx *ctx, struct lysp_node *child_p, struct lysc_node *node,
        struct ly_set *child_set);

/**
 * @brief Set LYS_MAND_TRUE flag for the non-presence container parents.
 *
 * A non-presence container is mandatory in case it has at least one mandatory children. This function propagate
 * the flag to such parents from a mandatory children.
 *
 * @param[in] parent A schema node to be examined if the mandatory child make it also mandatory.
 * @param[in] add Flag to distinguish adding the mandatory flag (new mandatory children appeared) or removing the flag
 * (mandatory children was removed).
 */
void lys_compile_mandatory_parents(struct lysc_node *parent, ly_bool add);

/**
 * @brief Validate grouping that was defined but not used in the schema itself.
 *
 * The grouping does not need to be compiled (and it is compiled here, but the result is forgotten immediately),
 * but to have the complete result of the schema validity, even such groupings are supposed to be checked.
 *
 * @param[in] ctx Compile context.
 * @param[in] pnode Parsed parent node of the grouping, NULL for top-level.
 * @param[in] grp Parsed grouping node to check.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_grouping(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysp_node_grp *grp);

/**
 * @brief Compile parsed schema node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed schema node.
 * @param[in] parent Compiled parent node where the current node is supposed to be connected. It is
 * NULL for top-level nodes, in such a case the module where the node will be connected is taken from
 * the compile context.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in,out] child_set Optional set to add all the compiled nodes into (can be more in case of uses).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile_node(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *parent, uint16_t inherited_flags,
        struct ly_set *child_set);

#endif /* LY_SCHEMA_COMPILE_NODE_H_ */
