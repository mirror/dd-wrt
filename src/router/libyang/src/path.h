/**
 * @file path.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Path structure and manipulation routines.
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PATH_H_
#define LY_PATH_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "tree.h"
#include "tree_data.h"

struct ly_ctx;
struct lys_module;
struct lysc_ext_instance;
struct lysc_node;
struct lyxp_expr;

enum ly_path_pred_type {
    LY_PATH_PREDTYPE_POSITION,  /**< position predicate - [2] */
    LY_PATH_PREDTYPE_LIST,      /**< keys predicate - [key1='val1'][key2='val2']... */
    LY_PATH_PREDTYPE_LEAFLIST,  /**< leaflist value predicate - [.='value'] */
    LY_PATH_PREDTYPE_LIST_VAR   /**< keys predicate with variable instead of value - [key1=$USER]... */
};

/**
 * @brief Structure for simple path predicate.
 */
struct ly_path_predicate {
    enum ly_path_pred_type type;            /**< Predicate type (see YANG ABNF) */
    union {
        uint64_t position;                  /**< position value for the position-predicate */
        struct {
            const struct lysc_node *key;    /**< key node of the predicate, NULL in case of a leaf-list predicate */
            union {
                struct lyd_value value;     /**< stored value representation according to the key's type (realtype ref) */
                char *variable;             /**< XPath variable used instead of the value */
            };
        };
    };
};

/**
 * @brief Structure for holding one segment of resolved path on schema including
 * simple predicates. Is used as a [sized array](@ref sizedarrays).
 */
struct ly_path {
    const struct lysc_node *node; /**< Schema node representing the path segment, first node has special meaning:
                                       - is a top-level node - path is absolute,
                                       - is inner node - path is relative */
    const struct lysc_ext_instance *ext;    /**< Extension instance of @p node, if any */
    struct ly_path_predicate *predicates;   /**< [Sized array](@ref sizedarrays) of the path segment's predicates */
};

/**
 * @defgroup path_begin_options Path begin options.
 * @{
 */
#define LY_PATH_BEGIN_ABSOLUTE  0x01    /**< path must be absolute */
#define LY_PATH_BEGIN_EITHER    0x02    /**< path be be either absolute or relative */
/** @} */

/**
 * @defgroup path_prefix_options Path prefix options.
 * @{
 */
#define LY_PATH_PREFIX_OPTIONAL     0x10    /**< prefixes in the path are optional (XML path) */
#define LY_PATH_PREFIX_MANDATORY    0x20    /**< prefixes in the path are mandatory (XML instance-identifier) */
#define LY_PATH_PREFIX_FIRST        0x40    /**< prefixes in the path are mandatory only in the first node of absolute path (JSON path) */
#define LY_PATH_PREFIX_STRICT_INHERIT 0x80  /**< prefixes in the path are mandatory in case they differ from the
                                                 previous prefixes, otherwise they are prohibited (JSON instance-identifier) */
/** @} */

/**
 * @defgroup path_pred_options Path predicate options.
 * @{
 */
#define LY_PATH_PRED_KEYS       0x0100  /** expected predicate only - [node='value']* */
#define LY_PATH_PRED_SIMPLE     0x0200  /** expected predicates - ( [node='value'] | [node=$VAR] )*; [.='value']; [1] */
#define LY_PATH_PRED_LEAFREF    0x0400  /** expected predicates only leafref - [node=current()/../../../node/node];
                                            at least 1 ".." and 1 "node" after */
/** @} */

/**
 * @brief Parse path into XPath token structure and perform all additional checks.
 *
 * @param[in] ctx libyang context.
 * @param[in] ctx_node Optional context node, used for logging.
 * @param[in] str_path Path to parse.
 * @param[in] path_len Length of @p str_path.
 * @param[in] lref Whether leafref is being parsed or not.
 * @param[in] begin Begin option (@ref path_begin_options).
 * @param[in] prefix Prefix option (@ref path_prefix_options).
 * @param[in] pred Predicate option (@ref path_pred_options).
 * @param[out] expr Parsed path.
 * @return LY_ERR value.
 */
LY_ERR ly_path_parse(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *str_path, size_t path_len,
        ly_bool lref, uint16_t begin, uint16_t prefix, uint16_t pred, struct lyxp_expr **expr);

/**
 * @brief Parse predicate into XPath token structure and perform all additional checks.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_node Optional current (original context) node, used for logging.
 * @param[in] str_path Path to parse.
 * @param[in] path_len Length of @p str_path.
 * @param[in] prefix Prefix option (@ref path_prefix_options).
 * @param[in] pred Predicate option (@ref path_pred_options).
 * @param[out] expr Parsed path.
 * @return LY_ERR value.
 */
LY_ERR ly_path_parse_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const char *str_path,
        size_t path_len, uint16_t prefix, uint16_t pred, struct lyxp_expr **expr);

/**
 * @defgroup path_oper_options Path operation options.
 * @{
 */
#define LY_PATH_OPER_INPUT  0x01    /**< if any RPC/action is traversed, its input nodes are used */
#define LY_PATH_OPER_OUTPUT 0x02    /**< if any RPC/action is traversed, its output nodes are used */
/** @} */

/* lref */

/**
 * @defgroup path_target_options Path target options.
 * @{
 */
#define LY_PATH_TARGET_SINGLE  0x10    /**< last (target) node must identify an exact instance */
#define LY_PATH_TARGET_MANY    0x20    /**< last (target) node may identify all instances (of leaf-list/list) */
/** @} */

/**
 * @brief Compile path into ly_path structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_mod Current module of the path (where it was "instantiated"). Used for nodes in schema-nodeid
 * without a prefix for ::LY_VALUE_SCHEMA and ::LY_VALUE_SCHEMA_RESOLVED format.
 * @param[in] ctx_node Optional context node.
 * @param[in] top_ext Extension instance containing the definition of the data being created. It is used to find the top-level
 * node inside the extension instance instead of a module. Note that this is the case not only if the @p ctx_node is NULL,
 * but also if the relative path starting in @p ctx_node reaches the document root via double dots.
 * @param[in] expr Parsed path.
 * @param[in] oper Oper option (@ref path_oper_options).
 * @param[in] target Target option (@ref path_target_options).
 * @param[in] limit_access_tree Whether to limit accessible tree.
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[out] path Compiled path.
 * @return LY_ERR value.
 */
LY_ERR ly_path_compile(const struct ly_ctx *ctx, const struct lys_module *cur_mod, const struct lysc_node *ctx_node,
        const struct lysc_ext_instance *top_ext, const struct lyxp_expr *expr, uint16_t oper, uint16_t target,
        ly_bool limit_access_tree, LY_VALUE_FORMAT format, void *prefix_data, struct ly_path **path);

/**
 * @brief Compile path into ly_path structure. Any predicates of a leafref are only checked, not compiled.
 *
 * @param[in] ctx libyang context.
 * @param[in] ctx_node Context node.
 * @param[in] top_ext Extension instance containing the definition of the data being created. It is used to find the top-level
 * node inside the extension instance instead of a module. Note that this is the case not only if the @p ctx_node is NULL,
 * but also if the relative path starting in @p ctx_node reaches the document root via double dots.
 * @param[in] expr Parsed path.
 * @param[in] oper Oper option (@ref path_oper_options).
 * @param[in] target Target option (@ref path_target_options).
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[out] path Compiled path.
 * @return LY_ERR value.
 */
LY_ERR ly_path_compile_leafref(const struct ly_ctx *ctx, const struct lysc_node *ctx_node,
        const struct lysc_ext_instance *top_ext, const struct lyxp_expr *expr, uint16_t oper, uint16_t target,
        LY_VALUE_FORMAT format, void *prefix_data, struct ly_path **path);

/**
 * @brief Compile predicate into ly_path_predicate structure. Only simple predicates (not leafref) are supported.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_node Optional current (original context) node.
 * @param[in] cur_mod Current module of the path (where it was "instantiated"). Used for nodes without a prefix
 * for ::LY_VALUE_SCHEMA and ::LY_VALUE_SCHEMA_RESOLVED format.
 * @param[in] ctx_node Context node, node for which the predicate is defined.
 * @param[in] expr Parsed path.
 * @param[in,out] tok_idx Index in @p expr, is adjusted for parsed tokens.
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[out] predicates Compiled predicates.
 * @return LY_ERR value.
 */
LY_ERR ly_path_compile_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const struct lys_module *cur_mod,
        const struct lysc_node *ctx_node, const struct lyxp_expr *expr, uint32_t *tok_idx, LY_VALUE_FORMAT format,
        void *prefix_data, struct ly_path_predicate **predicates);

/**
 * @brief Resolve at least partially the target defined by ly_path structure. Not supported for leafref!
 *
 * @param[in] path Path structure specifying the target.
 * @param[in] start Starting node for relative paths, can be any for absolute paths.
 * @param[in] vars Array of defined variables to use in predicates, may be NULL.
 * @param[in] with_opaq Whether to consider opaque nodes or not.
 * @param[out] path_idx Last found path segment index, can be NULL, set to 0 if not found.
 * @param[out] match Last found matching node, can be NULL, set to NULL if not found.
 * @return LY_ENOTFOUND if no nodes were found,
 * @return LY_EINCOMPLETE if some node was found but not the last one,
 * @return LY_SUCCESS when the last node in the path was found,
 * @return LY_ERR on another error.
 */
LY_ERR ly_path_eval_partial(const struct ly_path *path, const struct lyd_node *start, const struct lyxp_var *vars,
        ly_bool with_opaq, LY_ARRAY_COUNT_TYPE *path_idx, struct lyd_node **match);

/**
 * @brief Resolve the target defined by ly_path structure. Not supported for leafref!
 *
 * @param[in] path Path structure specifying the target.
 * @param[in] start Starting node for relative paths, can be any for absolute paths.
 * @param[in] vars Array of defined variables to use in predicates, may be NULL.
 * @param[out] match Found matching node, can be NULL, set to NULL if not found.
 * @return LY_ENOTFOUND if no nodes were found,
 * @return LY_SUCCESS when the last node in the path was found,
 * @return LY_ERR on another error.
 */
LY_ERR ly_path_eval(const struct ly_path *path, const struct lyd_node *start, const struct lyxp_var *vars,
        struct lyd_node **match);

/**
 * @brief Duplicate ly_path structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] path Path to duplicate.
 * @param[out] dup Duplicated path.
 * @return LY_ERR value.
 */
LY_ERR ly_path_dup(const struct ly_ctx *ctx, const struct ly_path *path, struct ly_path **dup);

/**
 * @brief Free ly_path_predicate structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] predicates Predicates ([sized array](@ref sizedarrays)) to free.
 */
void ly_path_predicates_free(const struct ly_ctx *ctx, struct ly_path_predicate *predicates);

/**
 * @brief Free ly_path structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] path The structure ([sized array](@ref sizedarrays)) to free.
 */
void ly_path_free(const struct ly_ctx *ctx, struct ly_path *path);

#endif /* LY_PATH_H_ */
