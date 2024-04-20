/**
 * @file tree_schema_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_INTERNAL_H_
#define LY_TREE_SCHEMA_INTERNAL_H_

#include <stdint.h>

#include "ly_common.h"
#include "set.h"
#include "tree_schema.h"

struct lysc_ctx;
struct lys_glob_unres;

#define LY_YANG_SUFFIX ".yang"
#define LY_YANG_SUFFIX_LEN 5
#define LY_YIN_SUFFIX ".yin"
#define LY_YIN_SUFFIX_LEN 4

#define YIN_NS_URI "urn:ietf:params:xml:ns:yang:yin:1"

#define LY_PCRE2_MSG_LIMIT 256

/**
 * @brief The maximum depth at which the last nested block is located.
 * Designed to protect against corrupted input that causes a stack-overflow error.
 * For yang language and json format, the block is bounded by "{ }".
 * For the xml format, the opening and closing element tag is considered as the block.
 */
#define LY_MAX_BLOCK_DEPTH 500

/* list of the deviate modifications strings */
extern const char * const ly_devmod_list[];
#define ly_devmod2str(TYPE) ly_devmod_list[TYPE]

/**
 * @brief Check module version is at least 2 (YANG 1.1) because of the keyword presence.
 * Logs error message and returns LY_EVALID in case of module in YANG version 1.0.
 * @param[in] CTX yang parser context to get current module and for logging.
 * @param[in] KW keyword allowed only in YANG version 1.1 (or later) - for logging.
 * @param[in] PARENT parent statement where the KW is present - for logging.
 */
#define PARSER_CHECK_STMTVER2_RET(CTX, KW, PARENT) \
    if (PARSER_CUR_PMOD(CTX)->version < LYS_VERSION_1_1) {LOGVAL_PARSER((CTX), LY_VCODE_INCHILDSTMT2, KW, PARENT); return LY_EVALID;}

/* These 2 macros checks YANG's identifier grammar rule */
#define is_yangidentstartchar(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
#define is_yangidentchar(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || \
                              c == '_' || c == '-' || c == '.')

/* Macro to check YANG's yang-char grammar rule */
#define is_yangutf8char(c) ((c >= 0x20 && c <= 0xd7ff) || c == 0x09 || c == 0x0a || c == 0x0d || \
                            (c >= 0xe000 && c <= 0xfdcf)   || (c >= 0xfdf0 && c <= 0xfffd)   || \
                            (c >= 0x10000 && c <= 0x1fffd) || (c >= 0x20000 && c <= 0x2fffd) || \
                            (c >= 0x30000 && c <= 0x3fffd) || (c >= 0x40000 && c <= 0x2fffd) || \
                            (c >= 0x50000 && c <= 0x5fffd) || (c >= 0x60000 && c <= 0x6fffd) || \
                            (c >= 0x70000 && c <= 0x7fffd) || (c >= 0x80000 && c <= 0x8fffd) || \
                            (c >= 0x90000 && c <= 0x9fffd) || (c >= 0xa0000 && c <= 0xafffd) || \
                            (c >= 0xb0000 && c <= 0xbfffd) || (c >= 0xc0000 && c <= 0xcfffd) || \
                            (c >= 0xd0000 && c <= 0xdfffd) || (c >= 0xe0000 && c <= 0xefffd) || \
                            (c >= 0xf0000 && c <= 0xffffd) || (c >= 0x100000 && c <= 0x10fffd))

/**
 * @brief Try to find object with MEMBER string matching the IDENT in the given ARRAY.
 * Macro logs an error message and returns LY_EVALID in case of existence of a matching object.
 *
 * @param[in] CTX yang parser context for logging.
 * @param[in] ARRAY [sized array](@ref sizedarrays) of a generic objects with member named MEMBER to search.
 * @param[in] MEMBER Name of the member of the objects in the ARRAY to compare.
 * @param[in] STMT Name of the compared YANG statements for logging.
 * @param[in] IDENT String trying to find in the ARRAY's objects inside the MEMBER member.
 */
#define CHECK_UNIQUENESS(CTX, ARRAY, MEMBER, STMT, IDENT) \
    if (ARRAY) { \
        for (LY_ARRAY_COUNT_TYPE u_ = 0; u_ < LY_ARRAY_COUNT(ARRAY) - 1; ++u_) { \
            if (!strcmp((ARRAY)[u_].MEMBER, IDENT)) { \
                LOGVAL_PARSER(CTX, LY_VCODE_DUPIDENT, IDENT, STMT); \
                return LY_EVALID; \
            } \
        } \
    }

#define CHECK_NONEMPTY(CTX, VALUE_LEN, STMT) \
    if (!VALUE_LEN) { \
        LOGWRN(PARSER_CTX(CTX), "Empty argument of %s statement does not make sense.", STMT); \
    }

/*
 * Additional YANG constants
 */
#define Y_TAB_SPACES         8  /**< number of spaces instead of tab character */
#define LY_TYPE_DEC64_FD_MAX 18 /**< Maximal value of decimal64's fraction-digits */

/**
 * @brief List of YANG statement groups - the (sub)module's substatements
 */
enum yang_module_stmt {
    Y_MOD_MODULE_HEADER,
    Y_MOD_LINKAGE,
    Y_MOD_META,
    Y_MOD_REVISION,
    Y_MOD_BODY
};

/**
 * @brief Types of arguments of YANG statements
 */
enum yang_arg {
    Y_IDENTIF_ARG,        /**< YANG "identifier-arg-str" rule */
    Y_PREF_IDENTIF_ARG,   /**< YANG "identifier-ref-arg-str" or node-identifier rule */
    Y_STR_ARG,            /**< YANG "string" rule */
    Y_MAYBE_STR_ARG       /**< optional YANG "string" rule */
};

#define PARSER_CUR_PMOD(CTX) ((struct lysp_module *)(CTX)->parsed_mods->objs[(CTX)->parsed_mods->count - 1])
#define PARSER_CTX(CTX) ((CTX) ? PARSER_CUR_PMOD(CTX)->mod->ctx : NULL)
#define LOGVAL_PARSER(CTX, ...) LOGVAL(PARSER_CTX(CTX), __VA_ARGS__)

struct lysp_ctx {
    LYS_INFORMAT format;             /**< parser format */
    struct ly_set tpdfs_nodes;       /**< Set of nodes that contain typedef(s). Invalid in case of
                                          submodule, use ::lysp_ctx.main_ctx instead. */
    struct ly_set grps_nodes;        /**< Set of nodes that contain grouping(s). Invalid in case of
                                          submodule, use ::lysp_ctx.main_ctx instead. */
    struct ly_set ext_inst;          /**< parsed extension instances to finish parsing */

    struct ly_set *parsed_mods;      /**< (sub)modules being parsed, the last one is the current */
    struct lysp_ctx *main_ctx;       /**< This pointer must not be NULL. If this context deals with the submodule,
                                          then should be set to the context of the module to which it belongs,
                                          otherwise it points to the beginning of this structure. */
};

/**
 * @brief Internal context for yang schema parser.
 */
struct lysp_yang_ctx {
    LYS_INFORMAT format;             /**< parser format */
    struct ly_set tpdfs_nodes;       /**< Set of nodes that contain typedef(s). Invalid in case of
                                          submodule, use ::lysp_ctx.main_ctx instead. */
    struct ly_set grps_nodes;        /**< Set of nodes that contain grouping(s). Invalid in case of
                                          submodule, use ::lysp_ctx.main_ctx instead. */
    struct ly_set ext_inst;          /**< parsed extension instances to finish parsing */

    struct ly_set *parsed_mods;      /**< (sub)modules being parsed, the last one is the current */
    struct lysp_ctx *main_ctx;       /**< This pointer must not be NULL. If this context deals with the submodule,
                                          then should be set to the context of the module to which it belongs,
                                          otherwise it points to the beginning of this structure. */
    struct ly_in *in;                /**< input handler for the parser */
    uint64_t indent;                 /**< current position on the line for YANG indentation */
    uint32_t depth;                  /**< current number of nested blocks, see ::LY_MAX_BLOCK_DEPTH */
};

/**
 * @brief Internal context for yin schema parser.
 */
struct lysp_yin_ctx {
    LYS_INFORMAT format;             /**< parser format */
    struct ly_set tpdfs_nodes;       /**< Set of nodes that contain typedef(s). Invalid in case of
                                          submodule, use ::lysp_ctx.main_ctx instead. */
    struct ly_set grps_nodes;        /**< Set of nodes that contain grouping(s). Invalid in case of
                                          submodule, use ::lysp_ctx.main_ctx instead. */
    struct ly_set ext_inst;          /**< parsed extension instances to finish parsing */

    struct ly_set *parsed_mods;      /**< (sub)modules being parsed, the last one is the current */
    struct lysp_ctx *main_ctx;       /**< This pointer must not be NULL. If this context deals with the submodule,
                                          then should be set to the context of the module to which it belongs,
                                          otherwise it points to the beginning of this structure. */
    struct lyxml_ctx *xmlctx;        /**< context for xml parser */
};

/**
 * @brief Check that @p c is valid UTF8 code point for YANG string.
 *
 * @param[in] ctx parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_stringchar(struct lysp_ctx *ctx, uint32_t c);

/**
 * @brief Check that @p c is valid UTF8 code point for YANG identifier.
 *
 * @param[in] ctx parser context for logging. If NULL, does not log.
 * @param[in] c UTF8 code point of a character to check.
 * @param[in] first Flag to check the first character of an identifier, which is more restricted.
 * @param[in,out] prefix Storage for internally used flag in case of possible prefixed identifiers:
 * 0 - colon not yet found (no prefix)
 * 1 - @p c is the colon character
 * 2 - prefix already processed, now processing the identifier
 *
 * If the identifier cannot be prefixed, NULL is expected.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_identifierchar(struct lysp_ctx *ctx, uint32_t c, ly_bool first, uint8_t *prefix);

/**
 * @brief Check the currently present prefixes in the module for collision with the new one.
 *
 * @param[in] ctx Context for logging.
 * @param[in] imports List of current imports of the module to check prefix collision.
 * @param[in] module_prefix Prefix of the module to check collision.
 * @param[in] value Newly added prefix value (including its location to distinguish collision with itself).
 * @return LY_EEXIST when prefix is already used in the module, LY_SUCCESS otherwise
 */
LY_ERR lysp_check_prefix(struct lysp_ctx *ctx, struct lysp_import *imports, const char *module_prefix, const char **value);

/**
 * @brief Check date string (4DIGIT "-" 2DIGIT "-" 2DIGIT)
 *
 * @param[in] ctx Optional context for logging.
 * @param[in] date Date string to check (non-necessarily terminated by \0)
 * @param[in] date_len Length of the date string, 10 expected.
 * @param[in] stmt Statement name for error message.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_date(struct lysp_ctx *ctx, const char *date, size_t date_len, const char *stmt);

/**
 * @brief Find type specified type definition.
 *
 * @param[in] id Name of the type including possible prefix. Module where the prefix is being searched is start_module.
 * @param[in] start_node Context node where the type is being instantiated to be able to search typedefs in parents.
 * @param[in] start_module Module where the type is being instantiated for search for typedefs.
 * @param[in] ext Extension where the type is being instantiated, if any.
 * @param[out] type Built-in type identifier of the id. If #LY_TYPE_UNKNOWN, tpdf is expected to contain found YANG schema typedef statement.
 * @param[out] tpdf Found type definition.
 * @param[out] node Node where the found typedef is defined, NULL in case of a top-level typedef.
 * @return LY_ERR value.
 */
LY_ERR lysp_type_find(const char *id, struct lysp_node *start_node, const struct lysp_module *start_module,
        const struct lysc_ext_instance *ext, LY_DATA_TYPE *type, const struct lysp_tpdf **tpdf, struct lysp_node **node);

/**
 * @brief Check names of typedefs in the parsed module to detect collisions.
 *
 * @param[in] ctx Parser context for logging and to maintain tpdfs_nodes
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_typedefs(struct lysp_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of groupings in the parsed module to detect collisions.
 *
 * @param[in] ctx Parser context for logging and to maintain grps_nodes.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_groupings(struct lysp_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of features in the parsed module and submodules to detect collisions.
 *
 * @param[in] ctx Parser context.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_features(struct lysp_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of identities in the parsed module and submodules to detect collisions.
 *
 * @param[in] ctx Parser context.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_identities(struct lysp_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Just move the newest revision into the first position, does not sort the rest
 * @param[in] revs Sized-array of the revisions in a printable schema tree.
 */
void lysp_sort_revisions(struct lysp_revision *revs);

/**
 * @brief Validate enum name.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] name String to check.
 * @param[in] name_len Length of name.
 *
 * @return LY_ERR values
 */
LY_ERR lysp_check_enum_name(struct lysp_ctx *ctx, const char *name, size_t name_len);

/**
 * @brief Find source data for a specific module, parse it, and add into the context.
 *
 * @param[in] ctx libyang context.
 * @param[in] name Name of the module to load.
 * @param[in] revision Optional revision of the module to load. If NULL, the newest revision is loaded.
 * @param[in,out] new_mods Set of all the new mods added to the context. Includes this module and all of its imports.
 * @param[out] mod Created module structure.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lys_parse_load(struct ly_ctx *ctx, const char *name, const char *revision, struct ly_set *new_mods,
        struct lys_module **mod);

/**
 * @brief Parse included submodules into the simply parsed YANG module.
 *
 * YANG 1.0 does not require the main module to include all the submodules. Therefore, parsing submodules can cause
 * reallocating and extending the includes array in the main module by the submodules included only in submodules.
 *
 * @param[in] pctx main parser context
 * @param[in] pmod Parsed module with the includes array to be processed.
 * @param[in,out] new_mods Set of all the new mods added to the context. Includes this module and all of its imports.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_submodules(struct lysp_ctx *pctx, struct lysp_module *pmod, struct ly_set *new_mods);

/**
 * @brief Get address of a node's actions list if any.
 * Decides the node's type and in case it has an actions list, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's actions member if any, NULL otherwise.
 */
struct lysp_node_action **lysp_node_actions_p(struct lysp_node *node);

/**
 * @brief Get address of a node's notifications list if any.
 * Decides the node's type and in case it has a notifications list, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's notifs member if any, NULL otherwise.
 */
struct lysp_node_notif **lysp_node_notifs_p(struct lysp_node *node);

/**
 * @brief Get address of a node's child pointer if any.
 * Decides the node's type and in case it has a children list, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's child member if any, NULL otherwise.
 */
struct lysp_node **lysp_node_child_p(struct lysp_node *node);

/**
 * @brief Get the address of the node's musts member, if any.
 * Decides the node's type and in case it has a musts member, returns its address.
 *
 * @param[in] node Node to examine.
 * @return The address of the node's musts member if any, NULL otherwise.
 */
struct lysp_restr **lysp_node_musts_p(const struct lysp_node *node);

/**
 * @brief Get the node's musts member, if any.
 * Decides the node's type and in case it has a musts member, returns its address.
 *
 * @param[in] node Node to examine.
 * @return The node's musts member if any, NULL otherwise.
 */
struct lysp_restr *lysp_node_musts(const struct lysp_node *node);

/**
 * @brief Get the address of the node's when member, if any.
 * Decides the node's type and in case it has a when, returns it.
 *
 * @param[in] node Node to examine.
 * @return The address of the node's when member if any, NULL otherwise.
 */
struct lysp_when **lysp_node_when_p(const struct lysp_node *node);

/**
 * @brief Get the node's when member, if any.
 * Decides the node's type and in case it has a when, returns it.
 *
 * @param[in] node Node to examine.
 * @return The node's when member if any, NULL otherwise.
 */
struct lysp_when *lysp_node_when(const struct lysp_node *node);

/**
 * @brief Get address of a node's child pointer if any.
 * Decides the node's type and in case it has a children list, returns its address.
 *
 * Do not use for RPC and action nodes.
 *
 * @param[in] node Node to check.
 * @return Address of the node's child member if any, NULL otherwise.
 */
struct lysc_node **lysc_node_child_p(const struct lysc_node *node);

/**
 * @brief Get address of a node's notifs pointer if any.
 * Decides the node's type and in case it has a notifs array, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's notifs member if any, NULL otherwise.
 */
struct lysc_node_notif **lysc_node_notifs_p(struct lysc_node *node);

/**
 * @brief Get address of a node's actions pointer if any.
 * Decides the node's type and in case it has a actions array, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's actions member if any, NULL otherwise.
 */
struct lysc_node_action **lysc_node_actions_p(struct lysc_node *node);

/**
 * @brief Get address of a node's when member if any.
 * Decides the node's type and in case it has a when member, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's when member if any, NULL otherwise.
 */
struct lysc_when ***lysc_node_when_p(const struct lysc_node *node);

/**
 * @brief Get address of a node's musts member if any.
 * Decides the node's type and in case it has a musts member, returns its address.
 *
 * @param[in] node Node to check.
 * @return Address of the node's musts member if any, NULL otherwise.
 */
struct lysc_must **lysc_node_musts_p(const struct lysc_node *node);

/**
 * @brief Find parsed extension definition for the given extension instance.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Extension instance for which the definition will be searched.
 * @param[out] ext_mod Module of the extension definition of @p ext.
 * @param[out] ext_def Optional found extension definition.
 * @return LY_SUCCESS when the definition was found.
 * @return LY_EVALID when the extension instance is invalid and/or the definition not found.
 */
LY_ERR lysp_ext_find_definition(const struct ly_ctx *ctx, const struct lysp_ext_instance *ext, const struct lys_module **ext_mod,
        struct lysp_ext **ext_def);

/**
 * @brief Get schema node in extension instance according to the given parameters.
 *
 * Wraps ::lys_getnext_ext() and match according to the given arguments.
 *
 * @param[in] ext Extension instance which top-level schema node is being searched.
 * @param[in] module Optional parameter to match the extension instance's (and its data) module.
 * @param[in] name Name of the schema node to find, if the string is not NULL-terminated, the @p name_len must be set.
 * @param[in] name_len Length of the @p name string, use in case the @p name is not NULL-terminated string.
 * @param[in] nodetype Allowed [type of the node](@ref schemanodetypes).
 * @param[in] options ORed [lys_getnext options](@ref sgetnextflags).
 * @return Found schema node if there is some satisfy the provided requirements.
 */
const struct lysc_node *lysc_ext_find_node(const struct lysc_ext_instance *ext, const struct lys_module *module,
        const char *name, size_t name_len, uint16_t nodetype, uint32_t options);

/**
 * @brief When the module comes from YIN format, the argument name is unknown because of missing extension definition
 * (it might come from import modules which is not yet parsed at that time). Therefore, all the attributes are stored
 * as substatements and resolving argument is postponed.
 *
 * @param[in] ctx libyang context
 * @param[in] ext_p Parsed extension to be updated.
 * @return LY_ERR value.
 */
LY_ERR lysp_ext_instance_resolve_argument(struct ly_ctx *ctx, struct lysp_ext_instance *ext_p);

/**
 * @brief Iterate over the specified type of the extension instances
 *
 * @param[in] ext ([Sized array](@ref sizedarrays)) of extensions to explore
 * @param[in] index Index in the @p ext array where to start searching (first call with 0, the consequent calls with
 *            the returned index increased by 1 (until the iteration is not terminated by returning LY_ARRAY_COUNT(ext).
 * @param[in] substmt The statement the extension is supposed to belong to.
 * @result index in the ext array, LY_ARRAY_COUNT(ext) value if not present.
 */
LY_ARRAY_COUNT_TYPE lysp_ext_instance_iter(struct lysp_ext_instance *ext, LY_ARRAY_COUNT_TYPE index, enum ly_stmt substmt);

/**
 * @brief Stringify YANG built-in type.
 *
 * @param[in] basetype Built-in type ID to stringify.
 * @return Constant string with the name of the built-in type.
 */
const char *lys_datatype2str(LY_DATA_TYPE basetype);

/**
 * @brief Implement a module and resolve all global unres.
 *
 * @param[in] mod Module to implement.
 * @param[in] features Features to set, see ::lys_set_features().
 * @param[in] unres Global unres with all the created modules.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR _lys_set_implemented(struct lys_module *mod, const char **features, struct lys_glob_unres *unres);

/**
 * @brief Create dependency sets for all modules in a context.
 * Also sets to_compile flags for all the modules that should be (re)compiled.
 *
 * @param[in] ctx Context to use.
 * @param[in,out] main_set Set of dependency module sets.
 * @param[in] mod Optional only module whose dependency set is needed, otherwise all sets are created.
 * @return LY_ERR value.
 */
LY_ERR lys_unres_dep_sets_create(struct ly_ctx *ctx, struct ly_set *main_set, struct lys_module *mod);

/**
 * @brief Revert changes stored in global compile context after a failed compilation.
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres to use.
 */
void lys_unres_glob_revert(struct ly_ctx *ctx, struct lys_glob_unres *unres);

/**
 * @brief Erase the global compile context.
 *
 * @param[in] unres Global unres to erase.
 */
void lys_unres_glob_erase(struct lys_glob_unres *unres);

typedef LY_ERR (*lys_custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod,
        void *check_data);

/**
 * @brief Parse a module and add it into the context.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in Input structure.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @param[in,out] new_mods Set of all the new mods added to the context. Includes this module and all of its imports.
 * @param[out] module Created module.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error, @p new_mods may be modified.
 */
LY_ERR lys_parse_in(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, lys_custom_check custom_check,
        void *check_data, struct ly_set *new_mods, struct lys_module **module);

/**
 * @brief Parse submodule.
 *
 * The latest_revision flag of submodule is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in Input structure.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] main_ctx Parser context of the main module.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @param[in] new_mods Set of all the new mods added to the context. Includes this module and all of its imports.
 * @param[out] submodule Parsed submodule.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, struct lysp_ctx *main_ctx,
        lys_custom_check custom_check, void *check_data, struct ly_set *new_mods, struct lysp_submodule **submodule);

/**
 * @brief Fill filepath value if available in input handler @p in
 *
 * @param[in] ctx Context with dictionary where the filepath value will be stored.
 * @param[in] in Input handler to examine (filepath is not available for all the input types).
 * @param[out] filepath Address of the variable where the filepath is stored.
 */
void lys_parser_fill_filepath(struct ly_ctx *ctx, struct ly_in *in, const char **filepath);

/**
 * @brief Get the @ref ifftokens from the given position in the 2bits array
 * (libyang format of the if-feature expression).
 * @param[in] list The 2bits array with the compiled if-feature expression.
 * @param[in] pos Position (0-based) to specify from which position get the operator.
 */
uint8_t lysc_iff_getop(uint8_t *list, size_t pos);

/**
 * @brief match yang keyword
 *
 * @param[in,out] in Input structure, is updated.
 * @param[in,out] indent Pointer to the counter of current position on the line for YANG indentation (optional).
 * @return yang_keyword values.
 */
enum ly_stmt lysp_match_kw(struct ly_in *in, uint64_t *indent);

/**
 * @brief Generate path of the given node in the requested format.
 *
 * @param[in] node Schema path of this node will be generated.
 * @param[in] parent Build relative path only until this parent is found. If NULL, the full absolute path is printed.
 * @param[in] pathtype Format of the path to generate.
 * @param[in,out] buffer Prepared buffer of the @p buflen length to store the generated path.
 *                If NULL, memory for the complete path is allocated.
 * @param[in] buflen Size of the provided @p buffer.
 * @return NULL in case of memory allocation error, path of the node otherwise.
 * In case the @p buffer is NULL, the returned string is dynamically allocated and caller is responsible to free it.
 */
char *lysc_path_until(const struct lysc_node *node, const struct lysc_node *parent, LYSC_PATH_TYPE pathtype, char *buffer,
        size_t buflen);

/**
 * @brief Get format-specific prefix for a module.
 *
 * This function is available for type plugins via ::lyplg_type_get_prefix() API function.
 *
 * @param[in] mod Module whose prefix to get.
 * @param[in] format Format of the prefix.
 * @param[in] prefix_data Format-specific data based on @p format:
 *      LY_VALUE_CANON           - NULL
 *      LY_VALUE_SCHEMA          - const struct ::lysp_module* (module used for resolving imports to prefixes)
 *      LY_VALUE_SCHEMA_RESOLVED - struct ::lysc_prefix* (sized array of pairs: prefix - module)
 *      LY_VALUE_XML             - struct ::ly_set* (set of all returned modules as struct ::lys_module)
 *      LY_VALUE_JSON            - NULL
 *      LY_VALUE_LYB             - NULL
 * @return Module prefix to print.
 * @return NULL on error.
 */
const char *ly_get_prefix(const struct lys_module *mod, LY_VALUE_FORMAT format, void *prefix_data);

/**
 * @brief Resolve format-specific prefixes to modules.
 *
 * @param[in] ctx libyang context.
 * @param[in] prefix Prefix to resolve.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of the prefix.
 * @param[in] prefix_data Format-specific data based on @p format:
 *      LY_VALUE_CANON           - NULL
 *      LY_VALUE_SCHEMA          - const struct lysp_module * (module used for resolving prefixes from imports)
 *      LY_VALUE_SCHEMA_RESOLVED - struct lyd_value_prefix * (sized array of pairs: prefix - module)
 *      LY_VALUE_XML             - const struct ly_set * (set with defined namespaces stored as ::lyxml_ns)
 *      LY_VALUE_JSON            - NULL
 *      LY_VALUE_LYB             - NULL
 * @return Resolved prefix module,
 * @return NULL otherwise.
 */
const struct lys_module *ly_resolve_prefix(const struct ly_ctx *ctx, const void *prefix, size_t prefix_len,
        LY_VALUE_FORMAT format, const void *prefix_data);

/**
 * @brief Learn whether @p PMOD needs to be recompiled if it is implemented.
 *
 * @param[in] PMOD Parsed module or submodule.
 * @return Whether it has statements that are recompiled or not.
 */
#define LYSP_HAS_RECOMPILED(PMOD) \
        (PMOD->data || PMOD->rpcs || PMOD->notifs || PMOD->exts)

/**
 * @brief Learn whether the module has statements that need to be recompiled or not.
 *
 * @param[in] mod Module to examine.
 * @return Whether it has statements that are recompiled or not.
 */
ly_bool lys_has_recompiled(const struct lys_module *mod);

/**
 * @brief Learn whether @p PMOD needs to be compiled if it is implemented.
 *
 * @param[in] PMOD Parsed module or submodule.
 * @return Whether it needs (has) a compiled module or not.
 */
#define LYSP_HAS_COMPILED(PMOD) \
        (LYSP_HAS_RECOMPILED(PMOD) || PMOD->augments || PMOD->deviations)

/**
 * @brief Learn whether the module has statements that need to be compiled or not.
 *
 * @param[in] mod Module to examine.
 * @return Whether it needs compiled module or not.
 */
ly_bool lys_has_compiled(const struct lys_module *mod);

/**
 * @brief Learn whether the module has any grouping statements or not.
 *
 * @param[in] mod Module to examine.
 * @return Whether it has groupings or not.
 */
ly_bool lys_has_dep_mods(const struct lys_module *mod);

/**
 * @brief Get YANG string keyword of a statement.
 *
 * @return YANG string keyword of the statement.
 */
const char *lys_stmt_str(enum ly_stmt stmt);

/**
 * @brief Get YIN argument (attribute) name of a statement.
 *
 * @return YIN argument name, if any.
 */
const char *lys_stmt_arg(enum ly_stmt stmt);

#define LY_STMT_FLAG_YIN 0x1 /**< has YIN element */
#define LY_STMT_FLAG_ID 0x2  /**< the value is identifier -> no quotes */

/**
 * @brief Get statement printer flags.
 *
 * @return Additional statement information as LY_STMT_FLAG_* flags.
 */
uint8_t lys_stmt_flags(enum ly_stmt stmt);

/**
 * @brief Learn whether the module qualifies for a single dep set with only this module or not.
 *
 * @param[in] mod Module to examine.
 * @return Whether it qualifies as a single dep set or not.
 */
#define LYS_IS_SINGLE_DEP_SET(mod) \
        (!(mod)->parsed->features && (!lys_has_compiled(mod) || ((mod)->compiled && !lys_has_recompiled(mod))))

/**
 * @brief Get pointer to a compiled ext instance storage for a specific statement.
 *
 * @param[in] ext Compiled ext instance.
 * @param[in] stmt Compiled statement. Can be a mask when the first match is returned, it is expected the storage is
 * the same for all the masked statements.
 * @param[out] storage_p Pointer to a compiled ext instance substatement storage, NULL if was not compiled.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the substatement is not supported.
 */
LY_ERR lyplg_ext_get_storage_p(const struct lysc_ext_instance *ext, int stmt, const void ***storage_p);

/**
 * @brief Warning if the filename does not match the expected module name and version
 *
 * @param[in] ctx Context for logging
 * @param[in] name Expected module name
 * @param[in] revision Expected module revision, or NULL if not to be checked
 * @param[in] filename File path to be checked
 */
void ly_check_module_filename(const struct ly_ctx *ctx, const char *name, const char *revision, const char *filename);

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
