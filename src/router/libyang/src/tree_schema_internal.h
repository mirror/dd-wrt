/**
 * @file tree_schema_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
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

#include "common.h"
#include "set.h"
#include "tree_schema.h"
#include "xml.h"

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

/**
 * @brief Informational structure for YANG statements
 */
struct stmt_info_s {
    const char *name;      /**< name of the statement */
    const char *arg;       /**< name of YIN's attribute to present the statement */
    uint8_t flags;         /**< various flags to clarify printing of the statement */
#define STMT_FLAG_YIN 0x1 /**< has YIN element */
#define STMT_FLAG_ID 0x2  /**< the value is identifier -> no quotes */
};

/* statements informations filled in tree_schema.c */
extern struct stmt_info_s stmt_attr_info[];

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
    if ((CTX)->parsed_mod->version < LYS_VERSION_1_1) {LOGVAL_PARSER((CTX), LY_VCODE_INCHILDSTMT2, KW, PARENT); return LY_EVALID;}

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

#define PARSER_CTX(CTX) ((CTX)->parsed_mod->mod->ctx)
#define LOGVAL_PARSER(CTX, ...) LOGVAL((CTX) ? PARSER_CTX(CTX) : NULL, __VA_ARGS__)

struct lys_parser_ctx {
    LYS_INFORMAT format;            /**< parser format */
    struct ly_set tpdfs_nodes;      /**< set of typedef nodes */
    struct ly_set grps_nodes;       /**< set of grouping nodes */
    struct lysp_module *parsed_mod; /**< (sub)module being parsed */
    struct lys_glob_unres *unres;   /**< global unres structure */
};

/**
 * @brief Internal context for yang schema parser.
 */
struct lys_yang_parser_ctx {
    LYS_INFORMAT format;            /**< parser format */
    struct ly_set tpdfs_nodes;      /**< set of typedef nodes */
    struct ly_set grps_nodes;       /**< set of grouping nodes */
    struct lysp_module *parsed_mod; /**< (sub)module being parsed */
    struct lys_glob_unres *unres;   /**< global unres structure */
    struct ly_in *in;               /**< input handler for the parser */
    uint64_t indent;                /**< current position on the line for YANG indentation */
    uint32_t depth;                 /**< current number of nested blocks, see ::LY_MAX_BLOCK_DEPTH */
};

/**
 * @brief free lys parser context.
 */
void yang_parser_ctx_free(struct lys_yang_parser_ctx *ctx);

/**
 * @brief Internal context for yin schema parser.
 */
struct lys_yin_parser_ctx {
    LYS_INFORMAT format;           /**< parser format */
    struct ly_set tpdfs_nodes;     /**< set of typedef nodes */
    struct ly_set grps_nodes;      /**< set of grouping nodes */
    struct lysp_module *parsed_mod;/**< (sub)module being parsed */
    struct lys_glob_unres *unres;   /**< global unres structure */
    struct lyxml_ctx *xmlctx;      /**< context for xml parser */
};

/**
 * @brief free yin parser context
 *
 * @param[in] ctx Context to free.
 */
void yin_parser_ctx_free(struct lys_yin_parser_ctx *ctx);

/**
 * @brief Check that \p c is valid UTF8 code point for YANG string.
 *
 * @param[in] ctx parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_stringchar(struct lys_parser_ctx *ctx, uint32_t c);

/**
 * @brief Check that \p c is valid UTF8 code point for YANG identifier.
 *
 * @param[in] ctx parser context for logging.
 * @param[in] c UTF8 code point of a character to check.
 * @param[in] first Flag to check the first character of an identifier, which is more restricted.
 * @param[in,out] prefix Storage for internally used flag in case of possible prefixed identifiers:
 * 0 - colon not yet found (no prefix)
 * 1 - \p c is the colon character
 * 2 - prefix already processed, now processing the identifier
 *
 * If the identifier cannot be prefixed, NULL is expected.
 * @return LY_ERR values.
 */
LY_ERR lysp_check_identifierchar(struct lys_parser_ctx *ctx, uint32_t c, ly_bool first, uint8_t *prefix);

/**
 * @brief Check the currently present prefixes in the module for collision with the new one.
 *
 * @param[in] ctx Context for logging.
 * @param[in] imports List of current imports of the module to check prefix collision.
 * @param[in] module_prefix Prefix of the module to check collision.
 * @param[in] value Newly added prefix value (including its location to distinguish collision with itself).
 * @return LY_EEXIST when prefix is already used in the module, LY_SUCCESS otherwise
 */
LY_ERR lysp_check_prefix(struct lys_parser_ctx *ctx, struct lysp_import *imports, const char *module_prefix, const char **value);

/**
 * @brief Check date string (4DIGIT "-" 2DIGIT "-" 2DIGIT)
 *
 * @param[in] ctx Optional context for logging.
 * @param[in] date Date string to check (non-necessarily terminated by \0)
 * @param[in] date_len Length of the date string, 10 expected.
 * @param[in] stmt Statement name for error message.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_date(struct lys_parser_ctx *ctx, const char *date, size_t date_len, const char *stmt);

/**
 * @brief Check names of typedefs in the parsed module to detect collisions.
 *
 * @param[in] ctx Parser context for logging and to maintain tpdfs_nodes
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_typedefs(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of features in the parsed module and submodules to detect collisions.
 *
 * @param[in] ctx Parser context.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_features(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Check names of identities in the parsed module and submodules to detect collisions.
 *
 * @param[in] ctx Parser context.
 * @param[in] mod Module where the type is being defined.
 * @return LY_ERR value.
 */
LY_ERR lysp_check_dup_identities(struct lys_parser_ctx *ctx, struct lysp_module *mod);

/**
 * @brief Just move the newest revision into the first position, does not sort the rest
 * @param[in] revs Sized-array of the revisions in a printable schema tree.
 */
void lysp_sort_revisions(struct lysp_revision *revs);

/**
 * @brief Find type specified type definition.
 *
 * @param[in] id Name of the type including possible prefix. Module where the prefix is being searched is start_module.
 * @param[in] start_node Context node where the type is being instantiated to be able to search typedefs in parents.
 * @param[in] start_module Module where the type is being instantiated for search for typedefs.
 * @param[out] type Built-in type identifier of the id. If #LY_TYPE_UNKNOWN, tpdf is expected to contain found YANG schema typedef statement.
 * @param[out] tpdf Found type definition.
 * @param[out] node Node where the found typedef is defined, NULL in case of a top-level typedef.
 */
LY_ERR lysp_type_find(const char *id, struct lysp_node *start_node, const struct lysp_module *start_module,
        LY_DATA_TYPE *type, const struct lysp_tpdf **tpdf, struct lysp_node **node);

/**
 * @brief Validate enum name.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] name String to check.
 * @param[in] name_len Length of name.
 *
 * @return LY_ERR values
 */
LY_ERR lysp_check_enum_name(struct lys_parser_ctx *ctx, const char *name, size_t name_len);

/**
 * @brief Find and load a module of the given name.
 *
 * @param[in] ctx libyang context.
 * @param[in] name Name of the module to load.
 * @param[in] revison Optional revision of the module to load. If NULL, the newest revision is loaded.
 * @param[in] need_implemented Whether the module should be implemented. If revision is NULL and this flag is set,
 * the implemented module in the context is returned despite it might not be of the latest revision, because in this
 * case the module of the latest revision can not be made implemented.
 * @param[in] features All the features to enable if implementing the module.
 * @param[in] unres Global unres structure for all newly implemented modules.
 * @param[out] mod Created module structure.
 * @return LY_ERR value.
 */
LY_ERR lys_load_module(struct ly_ctx *ctx, const char *name, const char *revision, ly_bool need_implemented,
        const char **features, struct lys_glob_unres *unres, struct lys_module **mod);

/**
 * @brief Parse included submodules into the simply parsed YANG module.
 *
 * YANG 1.0 does not require the main module to include all the submodules. Therefore, parsing submodules can cause
 * reallocating and extending the includes array in the main module by the submodules included only in submodules.
 *
 * @param[in] pctx main parser context
 * @param[in] pmod Parsed module with the includes array to be processed.
 * @return LY_ERR value.
 */
LY_ERR lysp_load_submodules(struct lys_parser_ctx *pctx, struct lysp_module *pmod);

/**
 * @brief Free a parsed restriction.
 *
 * @param[in] ctx libyang context.
 * @param[in] restr Restriction to free.
 */
void lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr);

/**
 * @brief Free a parsed qualified name.
 *
 * @param[in] ctx libyang context.
 * @param[in] qname Qualified name to free.
 */
void lysp_qname_free(struct ly_ctx *ctx, struct lysp_qname *qname);

/**
 * @brief Free a parsed node.
 *
 * @param[in] ctx libyang context.
 * @param[in] node Node to free.
 */
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);

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
 * @param[in, out] ext_mod Pointer to the module where the extension definition of the @p ext to correctly resolve prefixes.
 * @param[out] ext_def Pointer to return found extension definition.
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
 * There are 3 places which need the argument, so they resolve it when missing - YIN and YANG printers and extension instance
 * compiler.
 *
 * @param[in] ctx libyang context
 * @param[in] ext_p Parsed extension to be updated.
 * @param[in] ext_def Extension definition, found with ::lysp_ext_find_definition().
 * @return LY_ERR value.
 */
LY_ERR lysp_ext_instance_resolve_argument(struct ly_ctx *ctx, struct lysp_ext_instance *ext_p, struct lysp_ext *ext_def);

/**
 * @brief Iterate over the specified type of the extension instances
 *
 * @param[in] ext ([Sized array](@ref sizedarrays)) of extensions to explore
 * @param[in] index Index in the \p ext array where to start searching (first call with 0, the consequent calls with
 *            the returned index increased by 1 (until the iteration is not terminated by returning LY_ARRAY_COUNT(ext).
 * @param[in] substmt The statement the extension is supposed to belong to.
 * @result index in the ext array, LY_ARRAY_COUNT(ext) value if not present.
 */
LY_ARRAY_COUNT_TYPE lysp_ext_instance_iter(struct lysp_ext_instance *ext, LY_ARRAY_COUNT_TYPE index, enum ly_stmt substmt);

/**
 * @brief Get the covering schema module structure for the given parsed module structure.
 *
 * @param[in] ctx libyang context to search.
 * @param[in] mod Parsed schema structure.
 * @return Corresponding lys_module structure for the given parsed schema structure.
 */
struct lys_module *lysp_find_module(struct ly_ctx *ctx, const struct lysp_module *mod);

/**
 * @brief Stringify YANG built-in type.
 * @param[in] basetype Built-in type ID to stringify.
 * @return Constant string with the name of the built-in type.
 */
const char *lys_datatype2str(LY_DATA_TYPE basetype);

/**
 * @brief Implement a module (just like ::lys_set_implemented()), can be called recursively.
 *
 * @param[in] mod Module to implement.
 * @param[in] features Array of features to enable.
 * @param[in,out] unres Global unres to add to.
 * @return LY_ERR value.
 */
LY_ERR lys_set_implemented_r(struct lys_module *mod, const char **features, struct lys_glob_unres *unres);

typedef LY_ERR (*lys_custom_check)(const struct ly_ctx *ctx, struct lysp_module *mod, struct lysp_submodule *submod,
        void *check_data);

/**
 * @brief Create a new module.
 *
 * It is parsed, opionally compiled, added into the context, and the latest_revision flag is updated.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in Input structure.
 * @param[in] format Format of the input data (YANG or YIN).
 * @param[in] need_implemented Whether module needs to be implemented and compiled.
 * @param[in] custom_check Callback to check the parsed schema before it is accepted.
 * @param[in] check_data Caller's data to pass to the custom_check callback.
 * @param[in] features Array of features to enable ended with NULL. NULL for all features disabled and '*' for all enabled.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @param[out] module Created module.
 * @return LY_ERR value.
 */
LY_ERR lys_create_module(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, ly_bool need_implemented,
        lys_custom_check custom_check, void *check_data, const char **features, struct lys_glob_unres *unres,
        struct lys_module **module);

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
 * @param[out] submodule Parsed submodule.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_submodule(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format,
        struct lys_parser_ctx *main_ctx, lys_custom_check custom_check,
        void *check_data, struct lysp_submodule **submodule);

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
 * @brief Free the printable YANG schema tree structure. Works for both modules and submodules.
 *
 * @param[in] module Printable YANG schema tree structure to free.
 */
void lysp_module_free(struct lysp_module *module);

/**
 * @brief Free the parsed type structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in] type Parsed schema type structure to free. Note that the type itself is not freed.
 */
void lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type);

/**
 * @brief Free the parsed extension instance structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in] type Parsed extension instance structure to free. Note that the instance itself is not freed.
 */
void lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext);

/**
 * @param[in,out] exts [sized array](@ref sizedarrays) For extension instances in case of statements that do not store extension instances in their own list.
 */
LY_ERR lysp_stmt_parse(struct lysc_ctx *ctx, const struct lysp_stmt *stmt, void **result, struct lysp_ext_instance **exts);

/**
 * @brief Free a parsed node.
 *
 * @param[in] ctx libyang context.
 * @param[in] node Node to free.
 */
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);

/**
 * @brief Free the compiled type structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] type Compiled type structure to be freed. The structure has refcount, so it is freed only in case the value is decreased to 0.
 */
void lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type);

/**
 * @brief Free the compiled if-feature structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] iff Compiled if-feature structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_iffeature_free(struct ly_ctx *ctx, struct lysc_iffeature *iff);

/**
 * @brief Free the compiled identity structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] ident Compiled identity structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_ident_free(struct ly_ctx *ctx, struct lysc_ident *ident);

/**
 * @brief Free the compiled must structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] must Compiled must structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_must_free(struct ly_ctx *ctx, struct lysc_must *must);

/**
 * @brief Free the data inside compiled input/output structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] inout Compiled inout structure to be cleaned.
 * Since the structure is part of the RPC/action structure, it is not freed itself.
 */
void lysc_node_action_inout_free(struct ly_ctx *ctx, struct lysc_node_action_inout *inout);

/**
 * @brief Free the data inside compiled RPC/action structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] action Compiled action structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_node_action_free(struct ly_ctx *ctx, struct lysc_node_action *action);

/**
 * @brief Free the items inside the compiled Notification structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] notif Compiled Notification structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_node_notif_free(struct ly_ctx *ctx, struct lysc_node_notif *notif);

/**
 * @brief Free the compiled extension definition and NULL the provided pointer.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] ext Compiled extendion definition to be freed.
 */
void lysc_extension_free(struct ly_ctx *ctx, struct lysc_ext **ext);

/**
 * @brief Free the compiled extension instance structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] ext Compiled extension instance structure to be cleaned.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
void lysc_ext_instance_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Free the compiled node structure.
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in] node Compiled node structure to be freed.
 * @param[in] unlink Whether to first unlink the node before freeing.
 */
void lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node, ly_bool unlink);

/**
 * @brief Free the compiled container node structure.
 *
 * Only the container-specific members are freed, for generic node free function,
 * use ::lysc_node_free().
 *
 * @param[in] ctx libyang context where the string data resides in a dictionary.
 * @param[in,out] node Compiled container node structure to be freed.
 */
void lysc_node_container_free(struct ly_ctx *ctx, struct lysc_node_container *node);

/**
 * @brief Free the compiled schema structure.
 * @param[in,out] module Compiled schema module structure to free.
 */
void lysc_module_free(struct lysc_module *module);

/**
 * @brief Free the schema structure. It just frees, it does not remove the schema from its context.
 * @param[in,out] module Schema module structure to free.
 */
void lys_module_free(struct lys_module *module);

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
 * @brief Get nearest @p schema parent (including the node itself) that can be instantiated in data.
 *
 * @param[in] schema Schema node to get the nearest data node for.
 * @return Schema data node, NULL if top-level (in data).
 */
const struct lysc_node *lysc_data_node(const struct lysc_node *schema);

/**
 * @brief Same as ::lysc_data_node() but never returns the node itself.
 */
#define lysc_data_parent(SCHEMA) lysc_data_node((SCHEMA)->parent)

/**
 * @brief Get format-specific prefix for a module.
 *
 * For type plugins available as ::ly_type_print_get_prefix().
 *
 * @param[in] mod Module whose prefix to get.
 * @param[in] format Format of the prefix.
 * @param[in] prefix_data Format-specific data based on @p format:
 *      LY_VALUE_CANON           - NULL
 *      LY_VALUE_SCHEMA          - const struct lysp_module * (module used for resolving imports to prefixes)
 *      LY_VALUE_SCHEMA_RESOLVED - struct lyd_value_prefix * (sized array of pairs: prefix - module)
 *      LY_VALUE_XML             - struct ly_set * (set of all returned modules as ::struct lys_module)
 *      LY_VALUE_JSON            - NULL
 *      LY_VALUE_LYB             - NULL
 * @return Module prefix to print.
 * @return NULL on error.
 */
const char *ly_get_prefix(const struct lys_module *mod, LY_VALUE_FORMAT format, void *prefix_data);

/**
 * @brief Resolve format-specific prefixes to modules.
 *
 * For type plugins available as ::ly_type_store_resolve_prefix().
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

#endif /* LY_TREE_SCHEMA_INTERNAL_H_ */
