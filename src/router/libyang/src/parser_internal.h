/**
 * @file parser_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Internal structures and functions for libyang parsers
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_INTERNAL_H_
#define LY_PARSER_INTERNAL_H_

#include "parser_data.h"
#include "set.h"

struct lyd_ctx;
struct ly_in;
struct lysp_ext_substmt;
struct lysp_stmt;
struct lysp_yang_ctx;
struct lysp_yin_ctx;
struct lysp_ctx;

/**
 * @brief Check data parser error taking into account multi-error validation.
 *
 * @param[in] r Local return value.
 * @param[in] err_cmd Command to perform on any error.
 * @param[in] lydctx Data parser context.
 * @param[in] label Label to go to on fatal error.
 */
#define LY_DPARSER_ERR_GOTO(r, err_cmd, lydctx, label) \
        if (r) { \
            err_cmd; \
            if ((r != LY_EVALID) || !lydctx || !(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || \
                    (ly_vecode(((struct lyd_ctx *)lydctx)->data_ctx->ctx) == LYVE_SYNTAX)) { \
                goto label; \
            } \
        }

/**
 * @brief Callback for ::lyd_ctx to free the structure
 *
 * @param[in] ctx Data parser context to free.
 */
typedef void (*lyd_ctx_free_clb)(struct lyd_ctx *ctx);

/**
 * @brief Internal data parser flags.
 */
#define LYD_INTOPT_RPC              0x01    /**< RPC request is being parsed. */
#define LYD_INTOPT_ACTION           0x02    /**< Action request is being parsed. */
#define LYD_INTOPT_REPLY            0x04    /**< RPC/action reply is being parsed. */
#define LYD_INTOPT_NOTIF            0x08    /**< Notification is being parsed. */
#define LYD_INTOPT_ANY              0x10    /**< Anydata/anyxml content is being parsed, there can be anything. */
#define LYD_INTOPT_WITH_SIBLINGS    0x20    /**< Parse the whole input with any siblings. */
#define LYD_INTOPT_NO_SIBLINGS      0x40    /**< If there are any siblings, return an error. */
#define LYD_INTOPT_EVENTTIME        0x80    /**< Parse notification eventTime node. */

/**
 * @brief Internal (common) context for YANG data parsers.
 *
 * Covers ::lyd_xml_ctx, ::lyd_json_ctx and ::lyd_lyb_ctx.
 */
struct lyd_ctx {
    const struct lysc_ext_instance *ext; /**< extension instance possibly changing document root context of the data being parsed */
    uint32_t parse_opts;           /**< various @ref dataparseroptions. */
    uint32_t val_opts;             /**< various @ref datavalidationoptions. */
    uint32_t int_opts;             /**< internal parser options */
    uint32_t path_len;             /**< used bytes in the path buffer */

#define LYD_PARSER_BUFSIZE 4078
    char path[LYD_PARSER_BUFSIZE]; /**< buffer for the generated path */
    struct ly_set node_when;       /**< set of nodes with "when" conditions */
    struct ly_set node_types;      /**< set of nodes validated with LY_EINCOMPLETE result */
    struct ly_set meta_types;      /**< set of metadata validated with LY_EINCOMPLETE result */
    struct ly_set ext_node;        /**< set of nodes with extension instances to validate */
    struct ly_set ext_val;         /**< set of nested subtrees parsed by extensions to validate */
    struct lyd_node *op_node;      /**< if an RPC/action/notification is being parsed, store the pointer to it */

    /* callbacks */
    lyd_ctx_free_clb free;         /**< destructor */

    struct {
        const struct ly_ctx *ctx;  /**< libyang context */
        uint64_t line;             /**< current line */
        struct ly_in *in;          /**< input structure */
    } *data_ctx;                   /**< generic pointer supposed to map to and access (common part of) XML/JSON/... parser contexts */
};

/**
 * @brief Internal context for XML data parser.
 */
struct lyd_xml_ctx {
    const struct lysc_ext_instance *ext;
    uint32_t parse_opts;
    uint32_t val_opts;
    uint32_t int_opts;
    uint32_t path_len;
    char path[LYD_PARSER_BUFSIZE];
    struct ly_set node_when;
    struct ly_set node_types;
    struct ly_set meta_types;
    struct ly_set ext_node;
    struct ly_set ext_val;
    struct lyd_node *op_node;

    /* callbacks */
    lyd_ctx_free_clb free;

    struct lyxml_ctx *xmlctx;      /**< XML context */
};

/**
 * @brief Internal context for JSON data parser.
 */
struct lyd_json_ctx {
    const struct lysc_ext_instance *ext;
    uint32_t parse_opts;
    uint32_t val_opts;
    uint32_t int_opts;
    uint32_t path_len;
    char path[LYD_PARSER_BUFSIZE];
    struct ly_set node_when;
    struct ly_set node_types;
    struct ly_set meta_types;
    struct ly_set ext_node;
    struct ly_set ext_val;
    struct lyd_node *op_node;

    /* callbacks */
    lyd_ctx_free_clb free;

    struct lyjson_ctx *jsonctx;         /**< JSON context */
    const struct lysc_node *any_schema; /**< parent anyxml/anydata schema node if parsing nested data tree */
};

/**
 * @brief Internal context for LYB data parser/printer.
 */
struct lyd_lyb_ctx {
    const struct lysc_ext_instance *ext;

    union {
        struct {
            uint32_t parse_opts;
            uint32_t val_opts;
        };
        uint32_t print_options;
    };
    uint32_t int_opts;
    uint32_t path_len;
    char path[LYD_PARSER_BUFSIZE];
    struct ly_set node_when;
    struct ly_set node_types;
    struct ly_set meta_types;
    struct ly_set ext_node;
    struct ly_set ext_val;
    struct lyd_node *op_node;

    /* callbacks */
    lyd_ctx_free_clb free;

    struct lylyb_ctx *lybctx;      /* LYB context */
};

/**
 * @brief Parsed extension instance data to validate.
 */
struct lyd_ctx_ext_val {
    struct lysc_ext_instance *ext;
    struct lyd_node *sibling;
};

/**
 * @brief Parsed data node with extension instance to validate.
 */
struct lyd_ctx_ext_node {
    struct lysc_ext_instance *ext;
    struct lyd_node *node;
};

/**
 * @brief Common part to supplement the specific ::lyd_ctx_free_clb callbacks.
 */
void lyd_ctx_free(struct lyd_ctx *ctx);

/**
 * @brief Parse submodule from YANG data.
 * @param[in,out] context Parser context.
 * @param[in] ly_ctx Context of YANG schemas.
 * @param[in] main_ctx Parser context of main module.
 * @param[in] in Input structure.
 * @param[out] submod Pointer to the parsed submodule structure.
 * @return LY_ERR value - LY_SUCCESS, LY_EINVAL or LY_EVALID.
 */
LY_ERR yang_parse_submodule(struct lysp_yang_ctx **context, struct ly_ctx *ly_ctx, struct lysp_ctx *main_ctx,
        struct ly_in *in, struct lysp_submodule **submod);

/**
 * @brief Parse module from YANG data.
 * @param[in] context Parser context.
 * @param[in] in Input structure.
 * @param[in,out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 * @return LY_ERR values.
 */
LY_ERR yang_parse_module(struct lysp_yang_ctx **context, struct ly_in *in, struct lys_module *mod);

/**
 * @brief Parse module from YIN data.
 *
 * @param[in,out] yin_ctx Context created during parsing, is used to finalize lysp_model after it's completly parsed.
 * @param[in] in Input structure.
 * @param[in,out] mod Prepared module structure where the parsed information, including the parsed
 * module structure, will be filled in.
 * @return LY_ERR values.
 */
LY_ERR yin_parse_module(struct lysp_yin_ctx **yin_ctx, struct ly_in *in, struct lys_module *mod);

/**
 * @brief Parse submodule from YIN data.
 *
 * @param[in,out] yin_ctx Context created during parsing, is used to finalize lysp_model after it's completly parsed.
 * @param[in] ctx Libyang context.
 * @param[in] main_ctx Parser context of main module.
 * @param[in] in Input structure.
 * @param[in,out] submod Submodule structure where the parsed information, will be filled in.
 * @return LY_ERR values.
 */
LY_ERR yin_parse_submodule(struct lysp_yin_ctx **yin_ctx, struct ly_ctx *ctx, struct lysp_ctx *main_ctx,
        struct ly_in *in, struct lysp_submodule **submod);

/**
 * @brief Parse XML string as a YANG data tree.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extension instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first top-level parsed node, used only if @p parent is NULL.
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] int_opts Internal data parser options.
 * @param[out] parsed Set to add all the parsed siblings into.
 * @param[out] subtree_sibling Set if ::LYD_PARSE_SUBTREE is used and another subtree is following in @p in.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse XML string as a NETCONF message.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extension instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first top-level parsed node, used only if @p parent is NULL.
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] data_type Expected NETCONF data type of the data.
 * @param[out] envp Individual parsed envelopes tree, may be returned possibly even on an error.
 * @param[out] parsed Set to add all the parsed siblings into.
 * @param[out] subtree_sibling Set if ::LYD_PARSE_SUBTREE is used and another subtree is following in @p in.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_xml_netconf(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, enum lyd_type data_type,
        struct lyd_node **envp, struct ly_set *parsed, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse JSON string as a YANG data tree.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extension instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first top-level parsed node, used only if @p parent is NULL.
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] int_opts Internal data parser options.
 * @param[out] parsed Set to add all the parsed siblings into.
 * @param[out] subtree_sibling Set if ::LYD_PARSE_SUBTREE is used and another subtree is following in @p in.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_json(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse JSON string as a RESTCONF message.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extension instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first top-level parsed node, used only if @p parent is NULL.
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] data_type Expected RESTCONF data type of the data.
 * @param[out] envp Individual parsed envelopes tree, may be returned possibly even on an error.
 * @param[out] parsed Set to add all the parsed siblings into.
 * @param[out] subtree_sibling Set if ::LYD_PARSE_SUBTREE is used and another subtree is following in @p in.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_json_restconf(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, enum lyd_type data_type,
        struct lyd_node **envp, struct ly_set *parsed, struct lyd_ctx **lydctx_p);

/**
 * @brief Parse binary LYB data as a YANG data tree.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Optional extension instance to parse data following the schema tree specified in the extension instance
 * @param[in] parent Parent to connect the parsed nodes to, if any.
 * @param[in,out] first_p Pointer to the first top-level parsed node, used only if @p parent is NULL.
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] int_opts Internal data parser options.
 * @param[out] parsed Set to add all the parsed siblings into.
 * @param[out] subtree_sibling Set if ::LYD_PARSE_SUBTREE is used and another subtree is following in @p in.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_lyb(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p);

/**
 * @brief Validate eventTime date-and-time value.
 *
 * @param[in] node Opaque eventTime node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value on error.
 */
LY_ERR lyd_parser_notif_eventtime_validate(const struct lyd_node *node);

/**
 * @brief Search all the parents for an operation node, check validity based on internal parser flags.
 *
 * @param[in] parent Parent to connect the parsed nodes to.
 * @param[in] int_opts Internal parser options.
 * @param[out] op Found operation, if any.
 * @return LY_ERR value.
 */
LY_ERR lyd_parser_find_operation(const struct lyd_node *parent, uint32_t int_opts, struct lyd_node **op);

/**
 * @brief Get schema node of a node being parsed, use nodes stored for logging.
 *
 * @param[in] node Node whose schema node to get.
 * @return Schema node even for an opaque node, NULL if none found.
 */
const struct lysc_node *lyd_parser_node_schema(const struct lyd_node *node);

/**
 * @brief Check that a data node representing the @p snode is suitable based on options.
 *
 * @param[in] lydctx Common data parsers context.
 * @param[in] snode Schema node to check.
 * @return LY_SUCCESS or LY_EVALID
 */
LY_ERR lyd_parser_check_schema(struct lyd_ctx *lydctx, const struct lysc_node *snode);

/**
 * @brief Wrapper around ::lyd_create_term() for data parsers.
 *
 * @param[in] lydctx Data parser context.
 * @param[in] schema Schema node of the new data node.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] hints [Data parser's hints](@ref lydvalhints) for the value's type.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_parser_create_term(struct lyd_ctx *lydctx, const struct lysc_node *schema, const void *value, size_t value_len,
        ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, struct lyd_node **node);

/**
 * @brief Wrapper around ::lyd_create_meta() for data parsers.
 *
 * @param[in] lydctx Data parser context.
 * @param[in] parent Parent of the created meta. Must be set if @p meta is NULL.
 * @param[in,out] meta First existing meta to connect to or empty pointer to set. Must be set if @p parent is NULL.
 * @param[in] mod Module of the created metadata.
 * @param[in] name Metadata name.
 * @param[in] name_len Length of @p name.
 * @param[in] value Metadata value.
 * @param[in] value_len Length of @p value.
 * @param[in,out] dynamic Whether the @p value is dynamically allocated, is adjusted once the value is assigned.
 * @param[in] format Prefix format.
 * @param[in] prefix_data Prefix format data (see ::ly_resolve_prefix()).
 * @param[in] hints [Value hint](@ref lydvalhints) from the parser regarding the value type.
 * @param[in] ctx_node Value context node.
 * @return LY_ERR value.
 */
LY_ERR lyd_parser_create_meta(struct lyd_ctx *lydctx, struct lyd_node *parent, struct lyd_meta **meta,
        const struct lys_module *mod, const char *name, size_t name_len, const void *value, size_t value_len,
        ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node);

/**
 * @brief Check that a list has all its keys.
 *
 * @param[in] node List to check.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT on a missing key.
 */
LY_ERR lyd_parse_check_keys(struct lyd_node *node);

/**
 * @brief Set data flags for a newly parsed node.
 *
 * @param[in] node Node to use.
 * @param[in,out] meta Node metadata, may be removed from.
 * @param[in] lydctx Data parsing context.
 * @param[in] ext Extension instance if @p node was parsed for one.
 * @return LY_ERR value.
 */
LY_ERR lyd_parse_set_data_flags(struct lyd_node *node, struct lyd_meta **meta, struct lyd_ctx *lydctx,
        struct lysc_ext_instance *ext);

/**
 * @brief Parse an instance extension statement.
 *
 * @param[in] pctx Parse context.
 * @param[in] substmt Parsed ext instance substatement info.
 * @param[in] stmt Parsed generic statement to process.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_ext_instance_stmt(struct lysp_ctx *pctx, struct lysp_ext_substmt *substmt, struct lysp_stmt *stmt);

#endif /* LY_PARSER_INTERNAL_H_ */
