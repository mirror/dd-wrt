/**
 * @file tree_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang representation of YANG schema trees.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_H_
#define LY_TREE_SCHEMA_H_

#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#include <stdint.h>
#include <stdio.h>

#include "log.h"
#include "ly_config.h"
#include "tree.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_ctx;
struct ly_path;
struct ly_set;
struct lys_module;
struct lysc_node;
struct lyxp_expr;

/**
 * @page howtoSchema YANG Modules
 *
 * To be able to work with YANG data instances, libyang has to represent YANG data models. All the processed modules are stored
 * in libyang [context](@ref howtoContext) and loaded using [parser functions](@ref howtoSchemaParsers). It means, that there is
 * no way to create/change YANG module programmatically. However, all the YANG model definitions are available and can be examined
 * through the C structures. All the context's modules together form YANG Schema for the data being instantiated.
 *
 * Any YANG module is represented as ::lys_module. In fact, the module is represented in two different formats. As ::lys_module.parsed,
 * there is a parsed schema reflecting the source YANG module. It is exactly what is read from the input. This format is good for
 * converting from one format to another (YANG to YIN and vice versa), but it is not very useful for validating/manipulating YANG
 * data. Therefore, there is ::lys_module.compiled storing the compiled YANG module. It is based on the parsed module, but all the
 * references are resolved. It means that, for example, there are no `grouping`s or `typedef`s since they are supposed to be placed instead of
 * `uses` or `type` references. This split also means, that the YANG module is fully validated after compilation of the parsed
 * representation of the module. YANG submodules are available only in the parsed representation. When a submodule is compiled, it
 * is fully integrated into its main module.
 *
 * The context can contain even modules without the compiled representation. Such modules are still useful as imports of other
 * modules. The grouping or typedef definition can be even compiled into the importing modules. This is actually the main
 * difference between the imported and implemented modules in the libyang context. The implemented modules are compiled while the
 * imported modules are only parsed.
 *
 * By default, the module is implemented (and compiled) in case it is explicitly loaded or referenced in another module as
 * target of leafref, augment or deviation. This behavior can be changed via context options ::LY_CTX_ALL_IMPLEMENTED, when
 * all the modules in the context are marked as implemented (note the problem with multiple revisions of a single module),
 * or by ::LY_CTX_REF_IMPLEMENTED option, extending the set of references making the module implemented by when, must and
 * default statements.
 *
 * All modules with deviation definition are always marked as implemented. The imported (not implemented) module can be set implemented by ::lys_set_implemented(). But
 * the implemented module cannot be changed back to just imported module. Note also that only one revision of a specific module
 * can be implemented in a single context. The imported modules are used only as a
 * source of definitions for types and groupings for uses statements. The data in such modules are ignored - caller
 * is not allowed to create the data (including instantiating identities) defined in the model via data parsers,
 * the default nodes are not added into any data tree and mandatory nodes are not checked in the data trees.
 *
 * The compiled schema tree nodes are able to hold private objects (::lysc_node.priv as a pointer to a structure,
 * function, variable, ...) used by a caller application unless ::LY_CTX_SET_PRIV_PARSED is set, in that case
 * the ::lysc_node.priv pointers are used by libyang.
 * Note that the object is not freed by libyang when the context is being destroyed. So the caller is responsible
 * for freeing the provided structure after the context is destroyed or the private pointer is set to NULL in
 * appropriate schema nodes where the object was previously set. Also ::lysc_tree_dfs_full() can be useful to manage
 * the private data.
 *
 * Despite all the schema structures and their members are available as part of the libyang API and callers can use
 * it to navigate through the schema tree structure or to obtain various information, we recommend to use the following
 * macros for the specific actions.
 * - ::LYSC_TREE_DFS_BEGIN and ::LYSC_TREE_DFS_END to traverse the schema tree (depth-first).
 * - ::LY_LIST_FOR and ::LY_ARRAY_FOR as described on @ref howtoStructures page.
 *
 * Further information about modules handling can be found on the following pages:
 * - @subpage howtoSchemaParsers
 * - @subpage howtoSchemaFeatures
 * - @subpage howtoPlugins
 * - @subpage howtoSchemaPrinters
 *
 * \note There are many functions to access information from the schema trees. Details are available in
 * the [Schema Tree module](@ref schematree).
 *
 * For information about difference between implemented and imported modules, see the
 * [context description](@ref howtoContext).
 *
 * Functions List (not assigned to above subsections)
 * --------------------------------------------------
 * - ::lys_getnext()
 * - ::lys_nodetype2str()
 * - ::lys_set_implemented()
 *
 * - ::lysc_has_when()
 * - ::lysc_owner_module()
 *
 * - ::lysc_node_child()
 * - ::lysc_node_actions()
 * - ::lysc_node_notifs()
 *
 * - ::lysp_node_child()
 * - ::lysp_node_actions()
 * - ::lysp_node_notifs()
 * - ::lysp_node_groupings()
 * - ::lysp_node_typedefs()
 */

/**
 * @page howtoSchemaFeatures YANG Features
 *
 * YANG feature statement is an important part of the language which can significantly affect the meaning of the schemas.
 * Modifying features may have similar effects as loading/removing schema from the context so it is limited to context
 * preparation period before working with data. YANG features, respectively their use in if-feature
 * statements, are evaluated as part of schema compilation so a feature-specific compiled schema tree is generated
 * as a result.
 *
 * To enable any features, they must currently be specified when implementing a new schema with ::lys_parse() or
 * ::ly_ctx_load_module(). To later examine what the status of a feature is, check its ::LYS_FENABLED flag or
 * search for it first with ::lys_feature_value(). Lastly, to evaluate compiled if-features, use ::lysc_iffeature_value().
 *
 * To iterate over all features of a particular YANG module, use ::lysp_feature_next().
 *
 * Note, that the feature's state can affect some of the output formats (e.g. Tree format).
 *
 * Functions List
 * --------------
 * - ::lys_feature_value()
 * - ::lysc_iffeature_value()
 * - ::lysp_feature_next()
 */

/**
 * @ingroup trees
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

/* *INDENT-OFF* */

/**
 * @brief Macro to iterate via all elements in a schema (sub)tree including input and output.
 * Note that __actions__ and __notifications__ of traversed nodes __are ignored__! To traverse
 * on all the nodes including those, use ::lysc_tree_dfs_full() instead.
 *
 * This is the opening part to the #LYSC_TREE_DFS_END - they always have to be used together.
 *
 * The function follows deep-first search algorithm:
 * <pre>
 *     1
 *    / \
 *   2   4
 *  /   / \
 * 3   5   6
 * </pre>
 *
 * Use the same parameters for #LYSC_TREE_DFS_BEGIN and #LYSC_TREE_DFS_END. While
 * START can be any of the lysc_node* types (including lysc_node_action and lysc_node_notif),
 * ELEM variable must be of the struct lysc_node* type.
 *
 * To skip a particular subtree, instead of the continue statement, set LYSC_TREE_DFS_continue
 * variable to non-zero value.
 *
 * Use with opening curly bracket '{' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param ELEM Iterator intended for use in the block.
 */
#define LYSC_TREE_DFS_BEGIN(START, ELEM) \
    { ly_bool LYSC_TREE_DFS_continue = 0; struct lysc_node *LYSC_TREE_DFS_next; \
    for ((ELEM) = (LYSC_TREE_DFS_next) = (struct lysc_node *)(START); \
         (ELEM); \
         (ELEM) = (LYSC_TREE_DFS_next), LYSC_TREE_DFS_continue = 0)

/**
 * @brief Macro to iterate via all elements in a (sub)tree. This is the closing part
 * to the #LYSC_TREE_DFS_BEGIN - they always have to be used together.
 *
 * Use the same parameters for #LYSC_TREE_DFS_BEGIN and #LYSC_TREE_DFS_END. While
 * START can be a pointer to any of the lysc_node* types (including lysc_node_action and lysc_node_notif),
 * ELEM variable must be pointer to the lysc_node type.
 *
 * Use with closing curly bracket '}' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param ELEM Iterator intended for use in the block.
 */
#define LYSC_TREE_DFS_END(START, ELEM) \
    /* select element for the next run - children first */ \
    if (LYSC_TREE_DFS_continue) { \
        (LYSC_TREE_DFS_next) = NULL; \
    } else { \
        (LYSC_TREE_DFS_next) = (struct lysc_node *)lysc_node_child(ELEM); \
    } \
    if (!(LYSC_TREE_DFS_next)) { \
        /* no children, try siblings */ \
        _LYSC_TREE_DFS_NEXT(START, ELEM, LYSC_TREE_DFS_next); \
    } \
    while (!(LYSC_TREE_DFS_next)) { \
        /* parent is already processed, go to its sibling */ \
        (ELEM) = (ELEM)->parent; \
        _LYSC_TREE_DFS_NEXT(START, ELEM, LYSC_TREE_DFS_next); \
    } }

/**
 * @brief Helper macro for #LYSC_TREE_DFS_END, should not be used directly!
 */
#define _LYSC_TREE_DFS_NEXT(START, ELEM, NEXT) \
    if ((ELEM) == (struct lysc_node *)(START)) { \
        /* we are done, no next element to process */ \
        break; \
    } \
    (NEXT) = (ELEM)->next;

/* *INDENT-ON* */

#define LY_REV_SIZE 11   /**< revision data string length (including terminating NULL byte) */

/**
 * @defgroup schemanodetypes Schema Node Types
 * Values of the ::lysp_node.nodetype and ::lysc_node.nodetype members.
 * @{
 */
#define LYS_UNKNOWN     0x0000    /**< uninitalized unknown statement node */
#define LYS_CONTAINER   0x0001    /**< container statement node */
#define LYS_CHOICE      0x0002    /**< choice statement node */
#define LYS_LEAF        0x0004    /**< leaf statement node */
#define LYS_LEAFLIST    0x0008    /**< leaf-list statement node */
#define LYS_LIST        0x0010    /**< list statement node */
#define LYS_ANYXML      0x0020    /**< anyxml statement node */
#define LYS_ANYDATA     0x0060    /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */
#define LYS_CASE        0x0080    /**< case statement node */

#define LYS_RPC         0x0100    /**< RPC statement node */
#define LYS_ACTION      0x0200    /**< action statement node */
#define LYS_NOTIF       0x0400    /**< notification statement node */

#define LYS_USES        0x0800    /**< uses statement node */
#define LYS_INPUT       0x1000    /**< RPC/action input node */
#define LYS_OUTPUT      0x2000    /**< RPC/action output node */
#define LYS_GROUPING    0x4000
#define LYS_AUGMENT     0x8000

#define LYS_NODETYPE_MASK 0xffff  /**< Mask for nodetypes, the value is limited for 16 bits */
/** @} schemanodetypes */

/**
 * @brief YANG import-stmt
 */
struct lysp_import {
    struct lys_module *module;       /**< pointer to the imported module
                                          (mandatory, but resolved when the referring module is completely parsed) */
    const char *name;                /**< name of the imported module (mandatory) */
    const char *prefix;              /**< prefix for the data from the imported schema (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< LYS_INTERNAL value (@ref snodeflags) */
    char rev[LY_REV_SIZE];           /**< revision-date of the imported module */
};

/**
 * @brief YANG include-stmt
 */
struct lysp_include {
    struct lysp_submodule *submodule;/**< pointer to the parsed submodule structure
                                         (mandatory, but resolved when the referring module is completely parsed) */
    const char *name;                /**< name of the included submodule (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    char rev[LY_REV_SIZE];           /**< revision-date of the included submodule */
    ly_bool injected;                /**< flag to mark includes copied into main module from submodules,
                                          only for backward compatibility with YANG 1.0, which does not require the
                                          main module to include all submodules. */
};

/**
 * @brief YANG extension-stmt
 */
struct lysp_ext {
    const char *name;                /**< extension name */
    const char *argname;             /**< argument name, NULL if not specified */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< LYS_STATUS_* and LYS_YINELEM_* values (@ref snodeflags) */

    struct lysc_ext *compiled;       /**< pointer to the compiled extension definition.
                                          The extension definition is compiled only if there is compiled extension instance,
                                          otherwise this pointer remains NULL. The compiled extension definition is shared
                                          among all extension instances. */
};

/**
 * @brief YANG feature-stmt
 */
struct lysp_feature {
    const char *name;                /**< feature name (mandatory) */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysc_iffeature *iffeatures_c;    /**< compiled if-features */
    struct lysp_feature **depfeatures;  /**< list of pointers to other features depending on this one
                                          ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement  */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values and
                                          LYS_FENABLED are allowed */
};

/**
 * @brief Compiled YANG if-feature-stmt
 */
struct lysc_iffeature {
    uint8_t *expr;                   /**< 2bits array describing the if-feature expression in prefix format, see @ref ifftokens */
    struct lysp_feature **features;  /**< array of pointers to the features used in expression ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Qualified name (optional prefix followed by an identifier).
 */
struct lysp_qname {
    const char *str;                 /**< qualified name string */
    const struct lysp_module *mod;   /**< module to resolve any prefixes found in the string, it must be
                                          stored explicitly because of deviations/refines */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_SINGLEQUOTED and
                                          LYS_DOUBLEQUOTED values allowed */
};

/**
 * @brief YANG identity-stmt
 */
struct lysp_ident {
    const char *name;                /**< identity name (mandatory), including possible prefix */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    const char **bases;              /**< list of base identifiers ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @brief Covers restrictions: range, length, pattern, must
 */
struct lysp_restr {
#define LYSP_RESTR_PATTERN_ACK   0x06
#define LYSP_RESTR_PATTERN_NACK  0x15
    struct lysp_qname arg;           /**< The restriction expression/value (mandatory);
                                          in case of pattern restriction, the first byte has a special meaning:
                                          0x06 (ACK) for regular match and 0x15 (NACK) for invert-match */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG revision-stmt
 */
struct lysp_revision {
    char date[LY_REV_SIZE];          /**< revision date (madatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Enumeration/Bit value definition
 */
struct lysp_type_enum {
    const char *name;                /**< name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    int64_t value;                   /**< enum's value or bit's position */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ and LYS_SET_VALUE
                                          values are allowed */
};

/**
 * @brief YANG type-stmt
 *
 * Some of the items in the structure may be mandatory, but it is necessary to resolve the type's base type first
 */
struct lysp_type {
    const char *name;                /**< name of the type (mandatory) */
    struct lysp_restr *range;        /**< allowed values range - numerical, decimal64 */
    struct lysp_restr *length;       /**< allowed length of the value - string, binary */
    struct lysp_restr *patterns;     /**< list of patterns ([sized array](@ref sizedarrays)) - string */
    struct lysp_type_enum *enums;    /**< list of enum-stmts ([sized array](@ref sizedarrays)) - enum */
    struct lysp_type_enum *bits;     /**< list of bit-stmts ([sized array](@ref sizedarrays)) - bits */
    struct lyxp_expr *path;          /**< parsed path - leafref */
    const char **bases;              /**< list of base identifiers ([sized array](@ref sizedarrays)) - identityref */
    struct lysp_type *types;         /**< list of sub-types ([sized array](@ref sizedarrays)) - union */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    const struct lysp_module *pmod;  /**< (sub)module where the type is defined (needed for deviations) */
    struct lysc_type *compiled;      /**< pointer to the compiled custom type, not used for built-in types */

    uint8_t fraction_digits;         /**< number of fraction digits - decimal64 */
    uint8_t require_instance;        /**< require-instance flag - leafref, instance */
    uint16_t flags;                  /**< [schema node flags](@ref spnodeflags) */
};

/**
 * @brief YANG typedef-stmt
 */
struct lysp_tpdf {
    const char *name;                /**< name of the newly defined type (mandatory) */
    const char *units;               /**< units of the newly defined type */
    struct lysp_qname dflt;          /**< default value of the newly defined type, it may or may not be a qualified name */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_type type;           /**< base type from which the typedef is derived (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref spnodeflags) */
};

/**
 * @brief YANG when-stmt
 */
struct lysp_when {
    const char *cond;                /**< specified condition (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief YANG refine-stmt
 */
struct lysp_refine {
    const char *nodeid;              /**< target descendant schema nodeid (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    const char *presence;            /**< presence description */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
};

/**
 * @ingroup schematree
 * @defgroup deviatetypes Deviate types
 *
 * Type of the deviate operation (used as ::lysp_deviate.mod)
 *
 * @{
 */
#define LYS_DEV_NOT_SUPPORTED 1      /**< deviate type not-supported */
#define LYS_DEV_ADD 2                /**< deviate type add */
#define LYS_DEV_DELETE 3             /**< deviate type delete */
#define LYS_DEV_REPLACE 4            /**< deviate type replace */
/** @} deviatetypes */

/**
 * @brief Generic deviate structure to get type and cast to lysp_deviate_* structure
 */
struct lysp_deviate {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysp_deviate_add {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *units;               /**< units of the values */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_qname *uniques;      /**< list of uniques specifications ([sized array](@ref sizedarrays)) */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviate_del {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    const char *units;               /**< units of the values */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_qname *uniques;      /**< list of uniques specifications ([sized array](@ref sizedarrays)) */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)) */
};

struct lysp_deviate_rpl {
    uint8_t mod;                     /**< [type](@ref deviatetypes) of the deviate modification */
    struct lysp_deviate *next;       /**< next deviate structure in the list */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lysp_type *type;          /**< type of the node */
    const char *units;               /**< units of the values */
    struct lysp_qname dflt;          /**< default value */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_deviation {
    const char *nodeid;              /**< target absolute schema nodeid (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_deviate *deviates;   /**< list of deviate specifications (linked list) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @ingroup snodeflags
 * @defgroup spnodeflags Parsed schema nodes flags
 *
 * Various flags for parsed schema nodes (used as ::lysp_node.flags).
 *
 *     1 - container    6 - anydata/anyxml    11 - output       16 - grouping   21 - enum
 *     2 - choice       7 - case              12 - feature      17 - uses       22 - type
 *     3 - leaf         8 - notification      13 - identity     18 - refine     23 - stmt
 *     4 - leaflist     9 - rpc               14 - extension    19 - augment
 *     5 - list        10 - input             15 - typedef      20 - deviate
 *
 *                                             1 1 1 1 1 1 1 1 1 1 2 2 2 2
 *     bit name              1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x| | | | | | | | | | | |x| |x| | | |
 *         LYS_SET_BASE     | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x| | | | | | | | | | | |x| |x| | | |
 *         LYS_SET_BIT      | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       3 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| | |
 *         LYS_SET_ENUM     | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       4 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| | |
 *         LYS_SET_FRDIGITS | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       5 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|x|x| | |
 *         LYS_SET_LENGTH   | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       6 LYS_MAND_TRUE    | |x|x| | |x| | | | | | | | | | | |x| |x| | | |
 *         LYS_SET_PATH     | | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_FENABLED     | | | | | | | | | | | |x| | | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       7 LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | | | | |x| |x| | | |
 *         LYS_ORDBY_USER   | | | |x|x| | | | | | | | | | | | | | | | | | |
 *         LYS_SET_PATTERN  | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       8 LYS_ORDBY_SYSTEM | | | |x|x| | | | | | | | | | | | | | | | | | |
 *         LYS_YINELEM_TRUE | | | | | | | | | | | | | |x| | | | | | | | | |
 *         LYS_SET_RANGE    | | | | | | | | | | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       9 LYS_YINELEM_FALSE| | | | | | | | | | | | | |x| | | | | | | | | |
 *         LYS_SET_TYPE     | | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_SINGLEQUOTED | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      10 LYS_SET_VALUE    | | | | | | | | | | | | | | | | | | | | |x| | |
 *         LYS_SET_REQINST  | | | | | | | | | | | | | | | | | | | | | |x| |
 *         LYS_SET_MIN      | | | |x|x| | | | | | | | | | | | |x| |x| | | |
 *         LYS_DOUBLEQUOTED | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      11 LYS_SET_MAX      | | | |x|x| | | | | | | | | | | | |x| |x| | | |
 *         LYS_USED_GRP     | | | | | | | | | | | | | | | |x| | | | | | | |
 *         LYS_YIN_ATTR     | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      12 LYS_YIN_ARGUMENT | | | | | | | | | | | | | | | | | | | | | | |x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      13 LYS_INTERNAL     |x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * @ingroup snodeflags
 * @defgroup scnodeflags Compiled schema nodes flags
 *
 * Various flags for compiled schema nodes (used as ::lysc_node.flags).
 *
 *     1 - container    6 - anydata/anyxml    11 - identity
 *     2 - choice       7 - case              12 - extension
 *     3 - leaf         8 - notification      13 - bitenum
 *     4 - leaflist     9 - rpc/action        14 - when
 *     5 - list        10 - feature
 *
 *                                             1 1 1 1 1
 *     bit name              1 2 3 4 5 6 7 8 9 0 1 2 3 4
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       1 LYS_CONFIG_W     |x|x|x|x|x|x|x| | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       2 LYS_CONFIG_R     |x|x|x|x|x|x|x| | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       3 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x|x|x|x|x|x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       4 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x|x|x|x|x|x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       5 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x|x|x|x|x|x|
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       6 LYS_MAND_TRUE    |x|x|x|x|x|x| | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       7 LYS_ORDBY_USER   | | | |x|x| | | | | | | | | |
 *         LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       8 LYS_ORDBY_SYSTEM | | | |x|x| | | | | | | | | |
 *         LYS_PRESENCE     |x| | | | | | | | | | | | | |
 *         LYS_UNIQUE       | | |x| | | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       9 LYS_KEY          | | |x| | | | | | | | | | | |
 *         LYS_DISABLED     | | | | | | | | | | | | |x| |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      10 LYS_SET_DFLT     | | |x|x| | |x| | | | | | | |
 *         LYS_IS_ENUM      | | | | | | | | | | | | |x| |
 *         LYS_KEYLESS      | | | | |x| | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      11 LYS_SET_UNITS    | | |x|x| | | | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      12 LYS_SET_CONFIG   |x|x|x|x|x|x| | | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      13 LYS_IS_INPUT     |x|x|x|x|x|x|x| | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      14 LYS_IS_OUTPUT    |x|x|x|x|x|x|x| | | | | | | |
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      15 LYS_IS_NOTIF     |x|x|x|x|x|x|x| | | | | | | |
 *     ---------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

/**
 * @defgroup snodeflags Schema nodes flags
 *
 * Various flags for schema nodes ([parsed](@ref spnodeflags) as well as [compiled](@ref scnodeflags)).
 *
 * @{
 */
#define LYS_CONFIG_W     0x01        /**< config true; */
#define LYS_CONFIG_R     0x02        /**< config false; */
#define LYS_CONFIG_MASK  0x03        /**< mask for config value */
#define LYS_STATUS_CURR  0x04        /**< status current; */
#define LYS_STATUS_DEPRC 0x08        /**< status deprecated; */
#define LYS_STATUS_OBSLT 0x10        /**< status obsolete; */
#define LYS_STATUS_MASK  0x1C        /**< mask for status value */
#define LYS_MAND_TRUE    0x20        /**< mandatory true; applicable only to ::lysp_node_choice/::lysc_node_choice,
                                          ::lysp_node_leaf/::lysc_node_leaf and ::lysp_node_anydata/::lysc_node_anydata.
                                          The ::lysc_node_leaflist and ::lysc_node_leaflist have this flag in case that min-elements > 0.
                                          The ::lysc_node_container has this flag if it is not a presence container and it has at least one
                                          child with LYS_MAND_TRUE. */
#define LYS_MAND_FALSE   0x40        /**< mandatory false; applicable only to ::lysp_node_choice/::lysc_node_choice,
                                          ::lysp_node_leaf/::lysc_node_leaf and ::lysp_node_anydata/::lysc_node_anydata.
                                          This flag is present only in case the mandatory false statement was explicitly specified. */
#define LYS_MAND_MASK    0x60        /**< mask for mandatory values */
#define LYS_PRESENCE     0x80        /**< flag for presence property of a container, but it is not only for explicit presence
                                          containers, but also for NP containers with some meaning, applicable only to
                                          ::lysc_node_container */
#define LYS_UNIQUE       0x80        /**< flag for leafs being part of a unique set, applicable only to ::lysc_node_leaf */
#define LYS_KEY          0x0100      /**< flag for leafs being a key of a list, applicable only to ::lysc_node_leaf */
#define LYS_KEYLESS      0x0200      /**< flag for list without any key, applicable only to ::lysc_node_list */
#define LYS_DISABLED     0x0100      /**< internal flag for a disabled statement, used only for bits/enums */
#define LYS_FENABLED     0x20        /**< feature enabled flag, applicable only to ::lysp_feature. */
#define LYS_ORDBY_SYSTEM 0x80        /**< ordered-by system configuration lists, applicable only to
                                          ::lysc_node_leaflist/::lysp_node_leaflist and ::lysc_node_list/::lysp_node_list */
#define LYS_ORDBY_USER   0x40        /**< ordered-by user configuration lists, applicable only to
                                          ::lysc_node_leaflist/::lysp_node_leaflist and ::lysc_node_list/::lysp_node_list;
                                          is always set for state leaf-lists, and key-less lists */
#define LYS_ORDBY_MASK   0xC0        /**< mask for ordered-by values */
#define LYS_YINELEM_TRUE 0x80        /**< yin-element true for extension's argument */
#define LYS_YINELEM_FALSE 0x0100     /**< yin-element false for extension's argument */
#define LYS_YINELEM_MASK 0x0180      /**< mask for yin-element value */
#define LYS_USED_GRP     0x0400      /**< internal flag for validating not-instantiated groupings
                                          (resp. do not validate again the instantiated groupings). */
#define LYS_SET_VALUE    0x0200      /**< value attribute is set */
#define LYS_SET_MIN      0x0200      /**< min attribute is set */
#define LYS_SET_MAX      0x0400      /**< max attribute is set */

#define LYS_SET_BASE     0x0001      /**< type's flag for present base substatement */
#define LYS_SET_BIT      0x0002      /**< type's flag for present bit substatement */
#define LYS_SET_ENUM     0x0004      /**< type's flag for present enum substatement */
#define LYS_SET_FRDIGITS 0x0008      /**< type's flag for present fraction-digits substatement */
#define LYS_SET_LENGTH   0x0010      /**< type's flag for present length substatement */
#define LYS_SET_PATH     0x0020      /**< type's flag for present path substatement */
#define LYS_SET_PATTERN  0x0040      /**< type's flag for present pattern substatement */
#define LYS_SET_RANGE    0x0080      /**< type's flag for present range substatement */
#define LYS_SET_TYPE     0x0100      /**< type's flag for present type substatement */
#define LYS_SET_REQINST  0x0200      /**< type's flag for present require-instance substatement */
#define LYS_SET_DFLT     0x0200      /**< flag to mark leaf/leaflist with own (or refined) default value, not a default value taken from its type, and default
                                          cases of choice. This information is important for refines, since it is prohibited to make leafs
                                          with default statement mandatory. In case the default leaf value is taken from type, it is thrown
                                          away when it is refined to be mandatory node. Similarly it is used for deviations to distinguish
                                          between own default or the default values taken from the type. */
#define LYS_SET_UNITS    0x0400      /**< flag to know if the leaf's/leaflist's units are their own (flag set) or it is taken from the type. */
#define LYS_SET_CONFIG   0x0800      /**< flag to know if the config property was set explicitly (flag set) or it is inherited. */

#define LYS_SINGLEQUOTED 0x0100      /**< flag for single-quoted string (argument of an extension instance's substatement),
                                          only when the source is YANG */
#define LYS_DOUBLEQUOTED 0x0200      /**< flag for double-quoted string (argument of an extension instance's substatement),
                                          only when the source is YANG */

#define LYS_YIN_ATTR     0x0400      /**< flag to identify YIN attribute parsed as extension's substatement, only when the source is YIN */
#define LYS_YIN_ARGUMENT 0x0800      /**< flag to identify statement representing extension's argument, only when the source is YIN */

#define LYS_INTERNAL     0x1000      /**< flag to identify internal parsed statements that should not be printed */

#define LYS_IS_ENUM      0x0200      /**< flag to simply distinguish type in struct lysc_type_bitenum_item */

#define LYS_IS_INPUT     0x1000      /**< flag for nodes that are in the subtree of an input statement */

#define LYS_IS_OUTPUT    0x2000      /**< flag for nodes that are in the subtree of an output statement */

#define LYS_IS_NOTIF     0x4000      /**< flag for nodes that are in the subtree of a notification statement */

#define LYS_FLAGS_COMPILED_MASK 0xff /**< mask for flags that maps to the compiled structures */
/** @} snodeflags */

/**
 * @brief Generic YANG data node
 */
struct lysp_node {
    struct lysp_node *parent;        /**< parent node (NULL if this is a top-level node) */
    uint16_t nodetype;               /**< [type of the node](@ref schemanodetypes) (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    struct lysp_node *next;          /**< next sibling node (NULL if there is no one) */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement */
    const char *ref;                 /**< reference statement */
    struct lysp_qname *iffeatures;   /**< list of if-feature expressions ([sized array](@ref sizedarrays)),
                                          must be qname because of refines */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Extension structure of the lysp_node for YANG container
 */
struct lysp_node_container {
    union {
        struct lysp_node node;        /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_CONTAINER */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* container */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_when *when;          /**< when statement */
    const char *presence;            /**< presence description */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_node_action *actions;/**< list of actions (linked list) */
    struct lysp_node_notif *notifs;  /**< list of notifications (linked list) */
};

struct lysp_node_leaf {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_LEAF */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* leaf */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_when *when;          /**< when statement */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    struct lysp_qname dflt;          /**< default value, it may or may not be a qualified name */
};

struct lysp_node_leaflist {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_LEAFLIST */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* leaf-list */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_when *when;          /**< when statement */
    struct lysp_type type;           /**< type of the leaf node (mandatory) */
    const char *units;               /**< units of the leaf's type */
    struct lysp_qname *dflts;        /**< list of default values ([sized array](@ref sizedarrays)), they may or
                                          may not be qualified names */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_node_list {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_LIST */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* list */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_when *when;          /**< when statement */
    const char *key;                 /**< keys specification */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_node_action *actions;/**< list of actions (linked list) */
    struct lysp_node_notif *notifs;  /**< list of notifications (linked list) */
    struct lysp_qname *uniques;      /**< list of unique specifications ([sized array](@ref sizedarrays)) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */
};

struct lysp_node_choice {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_CHOICE */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* choice */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_when *when;          /**< when statement */
    struct lysp_qname dflt;          /**< default case */
};

struct lysp_node_case {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_CASE */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* case */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_when *when;          /**< when statement */
};

struct lysp_node_anydata {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_ANYXML or LYS_ANYDATA */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* anyxml/anydata */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_when *when;          /**< when statement */
};

struct lysp_node_uses {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_USES */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< pointer to the next sibling node (NULL if there is no one) */
            const char *name;        /**< grouping name reference (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* uses */
    struct lysp_refine *refines;     /**< list of uses's refines ([sized array](@ref sizedarrays)) */
    struct lysp_node_augment *augments; /**< list of augments (linked list) */
    struct lysp_when *when;          /**< when statement */
};

/**
 * @brief YANG input-stmt and output-stmt
 */
struct lysp_node_action_inout {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_INPUT or LYS_OUTPUT */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node *next;  /**< NULL */
            const char *name;        /**< empty string */
            const char *dsc;         /**< ALWAYS NULL, compatibility member with ::lysp_node */
            const char *ref;         /**< ALWAYS NULL, compatibility member with ::lysp_node */
            struct lysp_qname *iffeatures; /**< ALWAYS NULL, compatibility member with ::lysp_node */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* inout */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
};

/**
 * @brief YANG rpc-stmt and action-stmt
 */
struct lysp_node_action {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_RPC or LYS_ACTION */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            struct lysp_node_action *next; /**< pointer to the next action (NULL if there is no one) */
            const char *name;        /**< grouping name reference (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* action */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */

    struct lysp_node_action_inout input;  /**< RPC's/Action's input */
    struct lysp_node_action_inout output; /**< RPC's/Action's output */
};

/**
 * @brief YANG notification-stmt
 */
struct lysp_node_notif {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent; /**< parent node (NULL if this is a top-level node) */
            uint16_t nodetype;       /**< LYS_NOTIF */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
            struct lysp_node_notif *next; /**< pointer to the next notification (NULL if there is no one) */
            const char *name;        /**< grouping name reference (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                            /**< common part corresponding to ::lysp_node */

    /* notif */
    struct lysp_restr *musts;        /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */
    struct lysp_node *child;         /**< list of data nodes (linked list) */
};

/**
 * @brief YANG grouping-stmt
 */
struct lysp_node_grp {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent;/**< parent node (NULL if this is a top-level grouping) */
            uint16_t nodetype;       /**< LYS_GROUPING */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
            struct lysp_node_grp *next; /**< pointer to the next grouping (NULL if there is no one) */
            const char *name;        /**< grouping name (mandatory) */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< ALWAYS NULL, compatibility member with ::lysp_node */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                                /**< common part corresponding to ::lysp_node */

    /* grp */
    struct lysp_tpdf *typedefs;       /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings;  /**< list of groupings (linked list) */
    struct lysp_node *child;          /**< list of data nodes (linked list) */
    struct lysp_node_action *actions; /**< list of actions (linked list) */
    struct lysp_node_notif *notifs;   /**< list of notifications (linked list) */
};

/**
 * @brief YANG uses-augment-stmt and augment-stmt (compatible with struct lysp_node )
 */
struct lysp_node_augment {
    union {
        struct lysp_node node;       /**< implicit cast for the members compatible with ::lysp_node */

        struct {
            struct lysp_node *parent;/**< parent node (NULL if this is a top-level augment) */
            uint16_t nodetype;       /**< LYS_AUGMENT */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
            struct lysp_node_augment *next; /**< pointer to the next augment (NULL if there is no one) */
            const char *nodeid;      /**< target schema nodeid (mandatory) - absolute for global augments, descendant for uses's augments */
            const char *dsc;         /**< description statement */
            const char *ref;         /**< reference statement */
            struct lysp_qname *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
            struct lysp_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
        };
    };                                /**< common part corresponding to ::lysp_node */

    struct lysp_node *child;         /**< list of data nodes (linked list) */
    struct lysp_when *when;          /**< when statement */
    struct lysp_node_action *actions;/**< list of actions (linked list) */
    struct lysp_node_notif *notifs;  /**< list of notifications (linked list) */
};

/**
 * @brief supported YANG schema version values
 */
typedef enum LYS_VERSION {
    LYS_VERSION_UNDEF = 0,  /**< no specific version, YANG 1.0 as default */
    LYS_VERSION_1_0 = 1,    /**< YANG 1 (1.0) */
    LYS_VERSION_1_1 = 2     /**< YANG 1.1 */
} LYS_VERSION;

/**
 * @brief Printable YANG schema tree structure representing YANG module.
 *
 * Simple structure corresponding to the YANG format. The schema is only syntactically validated.
 */
struct lysp_module {
    struct lys_module *mod;          /**< covering module structure */

    struct lysp_revision *revs;      /**< list of the module revisions ([sized array](@ref sizedarrays)), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysp_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */
    struct lysp_include *includes;   /**< list of included submodules ([sized array](@ref sizedarrays)) */
    struct lysp_ext *extensions;     /**< list of extension statements ([sized array](@ref sizedarrays)) */
    struct lysp_feature *features;   /**< list of feature definitions ([sized array](@ref sizedarrays)) */
    struct lysp_ident *identities;   /**< list of identities ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */
    struct lysp_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysp_node_augment *augments; /**< list of augments (linked list) */
    struct lysp_node_action *rpcs;   /**< list of RPCs (linked list) */
    struct lysp_node_notif *notifs;  /**< list of notifications (linked list) */
    struct lysp_deviation *deviations; /**< list of deviations ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
    uint8_t parsing : 1;             /**< flag for circular check */
    uint8_t is_submod : 1;           /**< always 0 */
};

struct lysp_submodule {
    struct lys_module *mod;          /**< belongs to parent module (submodule - mandatory) */

    struct lysp_revision *revs;      /**< list of the module revisions ([sized array](@ref sizedarrays)), the first revision
                                          in the list is always the last (newest) revision of the module */
    struct lysp_import *imports;     /**< list of imported modules ([sized array](@ref sizedarrays)) */
    struct lysp_include *includes;   /**< list of included submodules ([sized array](@ref sizedarrays)) */
    struct lysp_ext *extensions;     /**< list of extension statements ([sized array](@ref sizedarrays)) */
    struct lysp_feature *features;   /**< list of feature definitions ([sized array](@ref sizedarrays)) */
    struct lysp_ident *identities;   /**< list of identities ([sized array](@ref sizedarrays)) */
    struct lysp_tpdf *typedefs;      /**< list of typedefs ([sized array](@ref sizedarrays)) */
    struct lysp_node_grp *groupings; /**< list of groupings (linked list) */
    struct lysp_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysp_node_augment *augments; /**< list of augments (linked list) */
    struct lysp_node_action *rpcs;   /**< list of RPCs (linked list) */
    struct lysp_node_notif *notifs;  /**< list of notifications (linked list) */
    struct lysp_deviation *deviations; /**< list of deviations ([sized array](@ref sizedarrays)) */
    struct lysp_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    uint8_t version;                 /**< yang-version (LYS_VERSION values) */
    uint8_t parsing : 1;             /**< flag for circular check */
    uint8_t is_submod : 1;           /**< always 1 */

    uint8_t latest_revision : 2;     /**< flag to mark the latest available revision:
                                          1 - the latest revision in searchdirs was not searched yet and this is the
                                          latest revision in the current context
                                          2 - searchdirs were searched and this is the latest available revision */
    const char *name;                /**< name of the module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    const char *prefix;              /**< submodule belongsto prefix of main module (mandatory) */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
};

/**
 * @brief Get the parsed module or submodule name.
 *
 * @param[in] PMOD Parsed module or submodule.
 * @return Module or submodule name.
 */
#define LYSP_MODULE_NAME(PMOD) (PMOD->is_submod ? ((struct lysp_submodule *)PMOD)->name : ((struct lysp_module *)PMOD)->mod->name)

/**
 * @brief Compiled prefix data pair mapping of prefixes to modules. In case the format is ::LY_VALUE_SCHEMA_RESOLVED,
 * the expected prefix data is a sized array of these structures.
 */
struct lysc_prefix {
    char *prefix;                   /**< used prefix */
    const struct lys_module *mod;   /**< mapping to a module */
};

/**
 * @brief Compiled YANG extension-stmt
 *
 * Note that the compiled extension definition is created only in case the extension is instantiated. It is not available
 * from the compiled schema, but from the parsed extension definition which is being searched when an extension instance
 * is being compiled.
 */
struct lysc_ext {
    const char *name;                /**< extension name */
    const char *argname;             /**< argument name, NULL if not specified */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_ext *plugin;        /**< Plugin implementing the specific extension */
    struct lys_module *module;       /**< module structure */
    uint16_t flags;                  /**< LYS_STATUS_* value (@ref snodeflags) */
};

/**
 * @brief YANG when-stmt
 */
struct lysc_when {
    struct lyxp_expr *cond;          /**< XPath when condition */
    struct lysc_node *context;       /**< context node for evaluating the expression, NULL if the context is root node */
    struct lysc_prefix *prefixes;    /**< compiled used prefixes in the condition */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint32_t refcount;               /**< reference counter since some of the when statements are shared among several nodes */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS is allowed */
};

/**
 * @brief YANG identity-stmt
 */
struct lysc_ident {
    const char *name;                /**< identity name (mandatory, no prefix) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lys_module *module;       /**< module structure */
    struct lysc_ident **derived;     /**< list of (pointers to the) derived identities ([sized array](@ref sizedarrays))
                                          It also contains references to identities located in unimplemented modules. */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
};

/**
 * @defgroup ifftokens if-feature expression tokens
 * Tokens of if-feature expression used in ::lysc_iffeature.expr.
 *
 * @{
 */
#define LYS_IFF_NOT  0x00 /**< operand "not" */
#define LYS_IFF_AND  0x01 /**< operand "and" */
#define LYS_IFF_OR   0x02 /**< operand "or" */
#define LYS_IFF_F    0x03 /**< feature */
/** @} ifftokens */

/**
 * @brief Compiled YANG revision statement
 */
struct lysc_revision {
    char date[LY_REV_SIZE];          /**< revision-date (mandatory) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_range {
    struct lysc_range_part {
        union {                      /**< min boundary */
            int64_t min_64;          /**< for int8, int16, int32, int64 and decimal64 ( >= LY_TYPE_DEC64) */
            uint64_t min_u64;        /**< for uint8, uint16, uint32, uint64, string and binary ( < LY_TYPE_DEC64) */
        };
        union {                      /**< max boundary */
            int64_t max_64;          /**< for int8, int16, int32, int64 and decimal64 ( >= LY_TYPE_DEC64) */
            uint64_t max_u64;        /**< for uint8, uint16, uint32, uint64, string and binary ( < LY_TYPE_DEC64) */
        };
    } *parts;                        /**< compiled range expression ([sized array](@ref sizedarrays)) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_pattern {
    const char *expr;                /**< original, not compiled, regular expression */
    pcre2_code *code;                /**< compiled regular expression */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    uint32_t inverted : 1;             /**< invert-match flag */
    uint32_t refcount : 31;            /**< reference counter */
};

struct lysc_must {
    struct lyxp_expr *cond;          /**< XPath when condition */
    struct lysc_prefix *prefixes;    /**< compiled used prefixes in the condition */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    const char *emsg;                /**< error-message */
    const char *eapptag;             /**< error-app-tag value */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

struct lysc_type {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing, it may be accessed concurrently when
                                          creating/freeing data node values that reference it (instance-identifier) */
};

struct lysc_type_num {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_dec {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    uint8_t fraction_digits;         /**< fraction digits specification */
    struct lysc_range *range;        /**< Optional range limitation */
};

struct lysc_type_str {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_range *length;       /**< optional length limitation */
    struct lysc_pattern **patterns;  /**< optional list of pointers to pattern limitations ([sized array](@ref sizedarrays)) */
};

struct lysc_type_bitenum_item {
    const char *name;            /**< enumeration identifier */
    const char *dsc;             /**< description */
    const char *ref;             /**< reference */
    struct lysc_ext_instance *exts;    /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    union {
        int32_t value;           /**< integer value associated with the enumeration */
        uint32_t position;       /**< non-negative integer value associated with the bit */
    };
    uint16_t flags;              /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ and LYS_IS_ENUM values
                                      are allowed */
};

struct lysc_type_enum {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_type_bitenum_item *enums; /**< enumerations list ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bits {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_type_bitenum_item *bits; /**< bits list ([sized array](@ref sizedarrays)), mandatory (at least 1 item),
                                              the items are ordered by their position value. */
};

struct lysc_type_leafref {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lyxp_expr *path;          /**< parsed target path, compiled path cannot be stored because of type sharing */
    struct lysc_prefix *prefixes;    /**< resolved prefixes used in the path */
    struct lysc_type *realtype;      /**< pointer to the real (first non-leafref in possible leafrefs chain) type. */
    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_identityref {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_ident **bases;       /**< list of pointers to the base identities ([sized array](@ref sizedarrays)),
                                          mandatory (at least 1 item) */
};

struct lysc_type_instanceid {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    uint8_t require_instance;        /**< require-instance flag */
};

struct lysc_type_union {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_type **types;        /**< list of types in the union ([sized array](@ref sizedarrays)), mandatory (at least 1 item) */
};

struct lysc_type_bin {
    const char *name;                /**< referenced typedef name (without prefix, if any), NULL for built-in types */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    struct lyplg_type *plugin;       /**< type's manipulation callbacks plugin */
    LY_DATA_TYPE basetype;           /**< base type of the type */
    uint32_t refcount;               /**< reference counter for type sharing */

    struct lysc_range *length;       /**< optional length limitation */
};

/**
 * @brief Maximum number of hashes stored in a schema node.
 */
#define LYS_NODE_HASH_COUNT 4

/**
 * @brief Compiled YANG data node
 */
struct lysc_node {
    uint16_t nodetype;               /**< [type of the node](@ref schemanodetypes) (mandatory) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
    struct lys_module *module;       /**< module structure */
    struct lysc_node *parent;        /**< parent node (NULL in case of top level node) */
    struct lysc_node *next;          /**< next sibling node (NULL if there is no one) */
    struct lysc_node *prev;          /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description */
    const char *ref;                 /**< reference */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
    void *priv;                      /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
};

struct lysc_node_action_inout {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_INPUT or LYS_OUTPUT */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent;/**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (output node for input, NULL for output) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node (input and output node pointing to each other) */
            const char *name;        /**< "input" or "output" */
            const char *dsc;         /**< ALWAYS NULL, compatibility member with ::lysc_node */
            const char *ref;         /**< ALWAYS NULL, compatibility member with ::lysc_node */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /** private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
};

struct lysc_node_action {
    union {
        struct lysc_node node;               /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_RPC or LYS_ACTION */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node - RPC) */
            struct lysc_node_action *next; /**< next sibling node (NULL if there is no one) */
            struct lysc_node_action *prev; /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< action/RPC name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /** private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)),
                                          the action/RPC nodes do not contain the when statement on their own, but they can
                                          inherit it from the parent's uses. */
    struct lysc_node_action_inout input;  /**< RPC's/action's input */
    struct lysc_node_action_inout output; /**< RPC's/action's output */

};

struct lysc_node_notif {
    union {
        struct lysc_node node;                       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_NOTIF */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node_notif *next; /**< next sibling node (NULL if there is no one) */
            struct lysc_node_notif *prev; /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< Notification name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /** private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)),
                                          the notification nodes do not contain the when statement on their own, but they can
                                          inherit it from the parent's uses. */
};

struct lysc_node_container {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_CONTAINER */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_node_action *actions;/**< first of actions nodes (linked list) */
    struct lysc_node_notif *notifs;  /**< first of notifications nodes (linked list) */
};

struct lysc_node_case {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_CASE */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser, unused */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< name of the case, including the implicit case */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_node *child;         /**< first child node of the case (linked list). Note that all the children of all the sibling cases are linked
                                          each other as siblings with the parent pointer pointing to appropriate case node. */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
};

struct lysc_node_choice {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_CHOICE */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser, unused */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_node_case *cases;    /**< list of all the cases (linked list) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_node_case *dflt;     /**< default case of the choice, only a pointer into the cases array. */
};

struct lysc_node_leaf {
    union {
        struct lysc_node node;               /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_LEAF */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    struct lyd_value *dflt;          /**< default value, use ::lyd_value_get_canonical() to get the canonical string */
};

struct lysc_node_leaflist {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_LEAFLIST */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when;         /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_type *type;          /**< type of the leaf node (mandatory) */

    const char *units;               /**< units of the leaf's type */
    struct lyd_value **dflts;        /**< list ([sized array](@ref sizedarrays)) of default values, use
                                        ::lyd_value_get_canonical() to get the canonical strings */

    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint */

};

struct lysc_node_list {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_LIST */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_node *child;         /**< first child node (linked list) */
    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when; /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
    struct lysc_node_action *actions;/**< first of actions nodes (linked list) */
    struct lysc_node_notif *notifs;  /**< first of notifications nodes (linked list) */

    struct lysc_node_leaf ***uniques;/**< list of sized arrays of pointers to the unique nodes ([sized array](@ref sizedarrays)) */
    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint */
};

struct lysc_node_anydata {
    union {
        struct lysc_node node;       /**< implicit cast for the members compatible with ::lysc_node */

        struct {
            uint16_t nodetype;       /**< LYS_ANYXML or LYS_ANYDATA */
            uint16_t flags;          /**< [schema node flags](@ref snodeflags) */
            uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
            struct lys_module *module; /**< module structure */
            struct lysc_node *parent; /**< parent node (NULL in case of top level node) */
            struct lysc_node *next;  /**< next sibling node (NULL if there is no one) */
            struct lysc_node *prev;  /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */
            const char *name;        /**< node name (mandatory) */
            const char *dsc;         /**< description */
            const char *ref;         /**< reference */
            struct lysc_ext_instance *exts; /**< list of the extension instances ([sized array](@ref sizedarrays)) */
            void *priv;              /**< private arbitrary user data, not used by libyang unless ::LY_CTX_SET_PRIV_PARSED is set */
        };
    };

    struct lysc_must *musts;         /**< list of must restrictions ([sized array](@ref sizedarrays)) */
    struct lysc_when **when; /**< list of pointers to when statements ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Compiled YANG schema tree structure representing YANG module.
 *
 * Semantically validated YANG schema tree for data tree parsing.
 * Contains only the necessary information for the data validation.
 */
struct lysc_module {
    struct lys_module *mod;          /**< covering module structure */

    struct lysc_node *data;          /**< list of module's top-level data nodes (linked list) */
    struct lysc_node_action *rpcs;   /**< first of actions nodes (linked list) */
    struct lysc_node_notif *notifs;  /**< first of notifications nodes (linked list) */
    struct lysc_ext_instance *exts;  /**< list of the extension instances ([sized array](@ref sizedarrays)) */
};

/**
 * @brief Examine whether a node is user-ordered list or leaf-list.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is user-ordered or not.
 */
#define lysc_is_userordered(lysc_node) \
    ((!lysc_node || !(lysc_node->nodetype & (LYS_LEAFLIST | LYS_LIST)) || !(lysc_node->flags & LYS_ORDBY_USER)) ? 0 : 1)

/**
 * @brief Examine whether a node is a list's key.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is a key or not.
 */
#define lysc_is_key(lysc_node) \
    ((!lysc_node || (lysc_node->nodetype != LYS_LEAF) || !(lysc_node->flags & LYS_KEY)) ? 0 : 1)

/**
 * @brief Examine whether a node is a non-presence container.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is a NP container or not.
 */
#define lysc_is_np_cont(lysc_node) \
    ((!lysc_node || (lysc_node->nodetype != LYS_CONTAINER) || (lysc_node->flags & LYS_PRESENCE)) ? 0 : 1)

/**
 * @brief Examine whether a node is a key-less list or a non-configuration leaf-list.
 *
 * @param[in] lysc_node Schema node to examine.
 * @return Boolean value whether the @p node is a list with duplicate instances allowed.
 */
#define lysc_is_dup_inst_list(lysc_node) \
    ((lysc_node && (((lysc_node->nodetype == LYS_LIST) && (lysc_node->flags & LYS_KEYLESS)) || \
            ((lysc_node->nodetype == LYS_LEAFLIST) && !(lysc_node->flags & LYS_CONFIG_W)))) ? 1 : 0)

/**
 * @brief Get nearest @p schema parent (including the node itself) that can be instantiated in data.
 *
 * @param[in] schema Schema node to get the nearest data node for.
 * @return Schema data node, NULL if top-level (in data).
 */
LIBYANG_API_DECL const struct lysc_node *lysc_data_node(const struct lysc_node *schema);

/**
 * @brief Same as ::lysc_data_node() but never returns the node itself.
 */
#define lysc_data_parent(SCHEMA) lysc_data_node((SCHEMA) ? (SCHEMA)->parent : NULL)

/**
 * @brief Check whether the schema node data instance existence depends on any when conditions.
 * This node and any direct parent choice and case schema nodes are also examined for when conditions.
 *
 * Be careful, this function is not recursive and checks only conditions that apply to this node directly.
 * Meaning if there are any conditions associated with any data parent instance of @p node, they are not returned.
 *
 * @param[in] node Schema node to examine.
 * @return When condition associated with the node data instance, NULL if there is none.
 */
LIBYANG_API_DECL const struct lysc_when *lysc_has_when(const struct lysc_node *node);

/**
 * @brief Get the owner module of the schema node. It is the module of the top-level node. Generally,
 * in case of augments it is the target module, recursively, otherwise it is the module where the node is defined.
 *
 * @param[in] node Schema node to examine.
 * @return Module owner of the node.
 */
LIBYANG_API_DECL const struct lys_module *lysc_owner_module(const struct lysc_node *node);

/**
 * @brief Get the groupings linked list of the given (parsed) schema node.
 * Decides the node's type and in case it has a groupings array, returns it.
 * @param[in] node Node to examine.
 * @return The node's groupings linked list if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysp_node_grp *lysp_node_groupings(const struct lysp_node *node);

/**
 * @brief Get the typedefs sized array of the given (parsed) schema node.
 * Decides the node's type and in case it has a typedefs array, returns it.
 * @param[in] node Node to examine.
 * @return The node's typedefs sized array if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysp_tpdf *lysp_node_typedefs(const struct lysp_node *node);

/**
 * @brief Get the actions/RPCs linked list of the given (parsed) schema node.
 * Decides the node's type and in case it has a actions/RPCs array, returns it.
 * @param[in] node Node to examine.
 * @return The node's actions/RPCs linked list if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysp_node_action *lysp_node_actions(const struct lysp_node *node);

/**
 * @brief Get the Notifications linked list of the given (parsed) schema node.
 * Decides the node's type and in case it has a Notifications array, returns it.
 * @param[in] node Node to examine.
 * @return The node's Notifications linked list if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysp_node_notif *lysp_node_notifs(const struct lysp_node *node);

/**
 * @brief Get the children linked list of the given (parsed) schema node.
 * Decides the node's type and in case it has a children list, returns it.
 * @param[in] node Node to examine.
 * @return The node's children linked list if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysp_node *lysp_node_child(const struct lysp_node *node);

/**
 * @brief Get the actions/RPCs linked list of the given (compiled) schema node.
 * Decides the node's type and in case it has a actions/RPCs array, returns it.
 * @param[in] node Node to examine.
 * @return The node's actions/RPCs linked list if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysc_node_action *lysc_node_actions(const struct lysc_node *node);

/**
 * @brief Get the Notifications linked list of the given (compiled) schema node.
 * Decides the node's type and in case it has a Notifications array, returns it.
 * @param[in] node Node to examine.
 * @return The node's Notifications linked list if any, NULL otherwise.
 */
LIBYANG_API_DECL const struct lysc_node_notif *lysc_node_notifs(const struct lysc_node *node);

/**
 * @brief Get the children linked list of the given (compiled) schema node.
 *
 * Note that ::LYS_CHOICE has only ::LYS_CASE children.
 * Also, ::LYS_RPC and ::LYS_ACTION have the first child ::LYS_INPUT, its sibling is ::LYS_OUTPUT.
 *
 * @param[in] node Node to examine.
 * @return Children linked list if any,
 * @return NULL otherwise.
 */
LIBYANG_API_DECL const struct lysc_node *lysc_node_child(const struct lysc_node *node);

/**
 * @brief Get the must statements list if present in the @p node
 *
 * @param[in] node Node to examine.
 * @return Pointer to the list of must restrictions ([sized array](@ref sizedarrays))
 * @return NULL if there is no must statement in the node, no matter if it is not even allowed or just present
 */
LIBYANG_API_DECL struct lysc_must *lysc_node_musts(const struct lysc_node *node);

/**
 * @brief Get the when statements list if present in the @p node
 *
 * @param[in] node Node to examine.
 * @return Pointer to the list of pointers to when statements ([sized array](@ref sizedarrays))
 * @return NULL if there is no when statement in the node, no matter if it is not even allowed or just present
 */
LIBYANG_API_DECL struct lysc_when **lysc_node_when(const struct lysc_node *node);

/**
 * @brief Get the target node of a leafref node.
 *
 * @param[in] node Leafref node.
 * @return Leafref target, NULL on any error.
 */
LIBYANG_API_DECL const struct lysc_node *lysc_node_lref_target(const struct lysc_node *node);

/**
 * @brief Callback to be called for every schema node in a DFS traversal.
 *
 * @param[in] node Current node.
 * @param[in] data Arbitrary user data.
 * @param[out] dfs_continue Set to true if the current subtree should be skipped and continue with siblings instead.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value to terminate DFS and return this value.
 */
typedef LY_ERR (*lysc_dfs_clb)(struct lysc_node *node, void *data, ly_bool *dfs_continue);

/**
 * @brief DFS traversal of all the schema nodes in a (sub)tree including any actions and nested notifications.
 *
 * Node with children, actions, and notifications is traversed in this order:
 * 1) each child subtree;
 * 2) each action subtree;
 * 3) each notification subtree.
 *
 * For algorithm illustration or traversal with actions and notifications skipped, see ::LYSC_TREE_DFS_BEGIN.
 *
 * @param[in] root Schema root to fully traverse.
 * @param[in] dfs_clb Callback to call for each node.
 * @param[in] data Arbitrary user data passed to @p dfs_clb.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value returned by @p dfs_clb.
 */
LIBYANG_API_DECL LY_ERR lysc_tree_dfs_full(const struct lysc_node *root, lysc_dfs_clb dfs_clb, void *data);

/**
 * @brief DFS traversal of all the schema nodes in a module including RPCs and notifications.
 *
 * For more details, see ::lysc_tree_dfs_full().
 *
 * @param[in] mod Module to fully traverse.
 * @param[in] dfs_clb Callback to call for each node.
 * @param[in] data Arbitrary user data passed to @p dfs_clb.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value returned by @p dfs_clb.
 */
LIBYANG_API_DECL LY_ERR lysc_module_dfs_full(const struct lys_module *mod, lysc_dfs_clb dfs_clb, void *data);

/**
 * @brief Get how the if-feature statement currently evaluates.
 *
 * @param[in] iff Compiled if-feature statement to evaluate.
 * @return LY_SUCCESS if the statement evaluates to true,
 * @return LY_ENOT if it evaluates to false,
 * @return LY_ERR on error.
 */
LIBYANG_API_DECL LY_ERR lysc_iffeature_value(const struct lysc_iffeature *iff);

/**
 * @brief Get how the if-feature statement is evaluated for certain identity.
 *
 * The function can be called even if the identity does not contain
 * if-features, in which case ::LY_SUCCESS is returned.
 *
 * @param[in] ident Compiled identity statement to evaluate.
 * @return LY_SUCCESS if the statement evaluates to true,
 * @return LY_ENOT if it evaluates to false,
 * @return LY_ERR on error.
 */
LIBYANG_API_DECL LY_ERR lys_identity_iffeature_value(const struct lysc_ident *ident);

/**
 * @brief Get the next feature in the module or submodules.
 *
 * @param[in] last Last returned feature.
 * @param[in] pmod Parsed module and submodules whose features to iterate over.
 * @param[in,out] idx Submodule index, set to 0 on first call.
 * @return Next found feature, NULL if the last has already been returned.
 */
LIBYANG_API_DECL struct lysp_feature *lysp_feature_next(const struct lysp_feature *last, const struct lysp_module *pmod,
        uint32_t *idx);

/**
 * @defgroup findxpathoptions Atomize XPath options
 * Options to modify behavior of ::lys_find_xpath() and ::lys_find_xpath_atoms() searching for schema nodes in schema tree.
 * @{
 */
#define LYS_FIND_XP_SCHEMA  0x08    /**< Apply node access restrictions defined for 'when' and 'must' evaluation. */
#define LYS_FIND_XP_OUTPUT  0x10    /**< Search RPC/action output nodes instead of input ones. */
#define LYS_FIND_NO_MATCH_ERROR 0x40    /**< Return error if a path segment matches no nodes, otherwise only warning
                                             is printed. */
#define LYS_FIND_SCHEMAMOUNT    0x0200  /**< Traverse also nodes from mounted modules. If any such nodes are returned,
                                             the caller **must free** their context! */
/** @} findxpathoptions */

/**
 * @brief Get all the schema nodes that are required for @p xpath to be evaluated (atoms).
 *
 * @param[in] ctx libyang context to use. May be NULL if @p ctx_node is set.
 * @param[in] ctx_node XPath schema context node. Use NULL for the root node.
 * @param[in] xpath Data XPath expression filtering the matching nodes. ::LY_VALUE_JSON prefix format is expected.
 * @param[in] options Whether to apply some node access restrictions, see @ref findxpathoptions.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lys_find_xpath_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *xpath,
        uint32_t options, struct ly_set **set);

/**
 * @brief Get all the schema nodes that are required for @p expr to be evaluated (atoms).
 *
 * @param[in] ctx_node XPath schema context node. Use NULL for the root node.
 * @param[in] cur_mod Current module for the expression (where it was "instantiated").
 * @param[in] expr Parsed expression to use.
 * @param[in] prefixes Sized array of compiled prefixes.
 * @param[in] options Whether to apply some node access restrictions, see @ref findxpathoptions.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lys_find_expr_atoms(const struct lysc_node *ctx_node, const struct lys_module *cur_mod,
        const struct lyxp_expr *expr, const struct lysc_prefix *prefixes, uint32_t options, struct ly_set **set);

/**
 * @brief Evaluate an @p xpath expression on schema nodes.
 *
 * @param[in] ctx libyang context to use for absolute @p xpath. May be NULL if @p ctx_node is set.
 * @param[in] ctx_node XPath schema context node for relative @p xpath. Use NULL for the root node.
 * @param[in] xpath Data XPath expression filtering the matching nodes. ::LY_VALUE_JSON prefix format is expected.
 * @param[in] options Whether to apply some node access restrictions, see @ref findxpathoptions.
 * @param[out] set Set of found schema nodes.
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value if an error occurred.
 */
LIBYANG_API_DECL LY_ERR lys_find_xpath(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *xpath,
        uint32_t options, struct ly_set **set);

/**
 * @brief Get all the schema nodes that are required for @p path to be evaluated (atoms).
 *
 * @param[in] path Compiled path to use.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_SUCCESS on success, @p set is returned.
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lys_find_lypath_atoms(const struct ly_path *path, struct ly_set **set);

/**
 * @brief Get all the schema nodes that are required for @p path to be evaluated (atoms).
 *
 * @param[in] ctx libyang context to use for absolute @p path. May be NULL if @p ctx_node is set.
 * @param[in] ctx_node XPath schema context node for relative @p path. Use NULL for the root node.
 * @param[in] path JSON path to examine.
 * @param[in] output Search operation output instead of input.
 * @param[out] set Set of found atoms (schema nodes).
 * @return LY_ERR value on error.
 */
LIBYANG_API_DECL LY_ERR lys_find_path_atoms(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *path,
        ly_bool output, struct ly_set **set);

/**
 * @brief Get a schema node based on the given data path (JSON format, see @ref howtoXPath).
 *
 * @param[in] ctx libyang context to use for absolute @p path. May be NULL if @p ctx_node is set.
 * @param[in] ctx_node XPath schema context node for relative @p path. Use NULL for the root node.
 * @param[in] path JSON path of the node to get.
 * @param[in] output Search operation output instead of input.
 * @return Found schema node or NULL.
 */
LIBYANG_API_DECL const struct lysc_node *lys_find_path(const struct ly_ctx *ctx, const struct lysc_node *ctx_node,
        const char *path, ly_bool output);

/**
 * @brief Types of the different schema paths.
 */
typedef enum {
    LYSC_PATH_LOG,  /**< Descriptive path format used in log messages */
    LYSC_PATH_DATA, /**< Similar to ::LYSC_PATH_LOG except that schema-only nodes (choice, case) are skipped */
    LYSC_PATH_DATA_PATTERN  /**< Similar to ::LYSC_PATH_DATA but there are predicates for all list keys added with
                                 "%s" where their values should be so that they can be printed there */
} LYSC_PATH_TYPE;

/**
 * @brief Generate path of the given node in the requested format.
 *
 * @param[in] node Schema path of this node will be generated.
 * @param[in] pathtype Format of the path to generate.
 * @param[in,out] buffer Prepared buffer of the @p buflen length to store the generated path.
 *                If NULL, memory for the complete path is allocated.
 * @param[in] buflen Size of the provided @p buffer.
 * @return NULL in case of memory allocation error, path of the node otherwise.
 * In case the @p buffer is NULL, the returned string is dynamically allocated and caller is responsible to free it.
 */
LIBYANG_API_DECL char *lysc_path(const struct lysc_node *node, LYSC_PATH_TYPE pathtype, char *buffer, size_t buflen);

/**
 * @brief Available YANG schema tree structures representing YANG module.
 */
struct lys_module {
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *revision;            /**< revision of the module (if present) */
    const char *ns;                  /**< namespace of the module (module - mandatory) */
    const char *prefix;              /**< module prefix or submodule belongsto prefix of main module (mandatory) */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */

    struct lysp_module *parsed;      /**< Simply parsed (unresolved) YANG schema tree */
    struct lysc_module *compiled;    /**< Compiled and fully validated YANG schema tree for data parsing.
                                          Available only for implemented modules. */

    struct lysc_ident *identities;   /**< List of compiled identities of the module ([sized array](@ref sizedarrays))
                                          also contains the disabled identities when their if-feature(s) are evaluated to \"false\",
                                          and also the list is filled even if the module is not implemented.
                                          The list is located here because it avoids problems when the module became implemented in
                                          future (no matter if implicitly via augment/deviate or explicitly via
                                          ::lys_set_implemented()). Note that if the module is not implemented (compiled), the
                                          identities cannot be instantiated in data (in identityrefs). */

    struct lys_module **augmented_by;/**< List of modules that augment this module ([sized array](@ref sizedarrays)) */
    struct lys_module **deviated_by; /**< List of modules that deviate this module ([sized array](@ref sizedarrays)) */

    ly_bool implemented;             /**< flag if the module is implemented, not just imported */
    ly_bool to_compile;              /**< flag marking a module that was changed but not (re)compiled, see
                                          ::LY_CTX_EXPLICIT_COMPILE. */
    uint8_t latest_revision;         /**< Flag to mark the latest available revision, see [latest_revision options](@ref latestrevflags). */
};

/**
 * @defgroup latestrevflags Options for ::lys_module.latest_revision.
 *
 * Various information bits of ::lys_module.latest_revision.
 *
 * @{
 */
#define LYS_MOD_LATEST_REV          0x01 /**< This is the latest revision of the module in the current context. */
#define LYS_MOD_LATEST_SEARCHDIRS   0x02 /**< This is the latest revision of the module found in searchdirs. */
#define LYS_MOD_IMPORTED_REV        0x04 /**< This is the module revision used when importing the module without
                                              an explicit revision-date. It is used for all such imports regardless of
                                              any changes made in the context. */
#define LYS_MOD_LATEST_IMPCLB       0x08 /**< This is the latest revision of the module obtained from import callback. */
/** @} latestrevflags */

/**
 * @brief Get the current real status of the specified feature in the module.
 *
 * If the feature is enabled, but some of its if-features are false, the feature is considered
 * disabled.
 *
 * @param[in] module Module where the feature is defined.
 * @param[in] feature Name of the feature to inspect.
 * @return LY_SUCCESS if the feature is enabled,
 * @return LY_ENOT if the feature is disabled,
 * @return LY_ENOTFOUND if the feature was not found.
 */
LIBYANG_API_DECL LY_ERR lys_feature_value(const struct lys_module *module, const char *feature);

/**
 * @brief Get next schema (sibling) node element in the schema order that can be instantiated in a data tree.
 * Returned node may be from an augment.
 *
 * ::lys_getnext() is supposed to be called sequentially. In the first call, the @p last parameter is usually NULL
 * and function starts returning 1) the first @p parent child (if it is set) or 2) the first top level element of
 * @p module. Consequent calls should provide the previously returned node as @p last and the same @p parent and
 * @p module parameters.
 *
 * Without options, the function is used to traverse only the schema nodes that can be paired with corresponding
 * data nodes in a data tree. By setting some @p options the behavior can be modified to the extent that
 * all the schema nodes are iteratively returned.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree to iterate over. If set, @p module is ignored.
 * @param[in] module Module of the top level elements to iterate over. If @p parent is NULL, it must be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Next schema tree node, NULL in case there are no more.
 */
LIBYANG_API_DECL const struct lysc_node *lys_getnext(const struct lysc_node *last, const struct lysc_node *parent,
        const struct lysc_module *module, uint32_t options);

/**
 * @brief Get next schema (sibling) node element in the schema order of an extension that can be instantiated in
 * a data tree.
 *
 * It is just ::lys_getnext() for extensions.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree to iterate over. If set, @p ext is ignored.
 * @param[in] ext Extension instance with schema nodes to iterate over. If @p parent is NULL, it must be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Next schema tree node, NULL in case there are no more.
 */
LIBYANG_API_DECL const struct lysc_node *lys_getnext_ext(const struct lysc_node *last, const struct lysc_node *parent,
        const struct lysc_ext_instance *ext, uint32_t options);

/**
 * @defgroup sgetnextflags Options for ::lys_getnext() and ::lys_getnext_ext().
 *
 * Various options setting behavior of ::lys_getnext() and ::lys_getnext_ext().
 *
 * @{
 */
#define LYS_GETNEXT_WITHCHOICE   0x01 /**< ::lys_getnext() option to allow returning #LYS_CHOICE nodes instead of looking into them */
#define LYS_GETNEXT_NOCHOICE     0x02 /**< ::lys_getnext() option to ignore (kind of conditional) nodes within choice node */
#define LYS_GETNEXT_WITHCASE     0x04 /**< ::lys_getnext() option to allow returning #LYS_CASE nodes instead of looking into them */
#define LYS_GETNEXT_INTONPCONT   0x08 /**< ::lys_getnext() option to look into non-presence container, instead of returning container itself */
#define LYS_GETNEXT_OUTPUT       0x10 /**< ::lys_getnext() option to provide RPC's/action's output schema nodes instead of input schema nodes
                                            provided by default */
#define LYS_GETNEXT_WITHSCHEMAMOUNT 0x20    /**< ::lys_getnext() option to also traverse top-level nodes of all the mounted modules
                                                 on the parent mount point but note that if any such nodes are returned,
                                                 the caller **must free** their context */
/** @} sgetnextflags */

/**
 * @brief Get child node according to the specified criteria.
 *
 * @param[in] parent Optional parent of the node to find. If not specified, the module's top-level nodes are searched.
 * @param[in] module module of the node to find. It is also limitation for the children node of the given parent.
 * @param[in] name Name of the node to find.
 * @param[in] name_len Optional length of the name in case it is not NULL-terminated string.
 * @param[in] nodetype Optional criteria (to speedup) specifying nodetype(s) of the node to find.
 * Used as a bitmask, so multiple nodetypes can be specified.
 * @param[in] options [ORed options](@ref sgetnextflags).
 * @return Found node if any.
 */
LIBYANG_API_DECL const struct lysc_node *lys_find_child(const struct lysc_node *parent, const struct lys_module *module,
        const char *name, size_t name_len, uint16_t nodetype, uint32_t options);

/**
 * @brief Make the specific module implemented.
 *
 * If the module is already implemented but with a different set of features, the whole context is recompiled.
 *
 * @param[in] mod Module to make implemented. It is not an error
 * to provide already implemented module, it just does nothing.
 * @param[in] features Optional array specifying the enabled features terminated with NULL overriding any previous
 * feature setting. The feature string '*' enables all the features and array of length 1 with only the terminating
 * NULL explicitly disables all the features. In case the parameter is NULL, the features are untouched - left disabled
 * in a newly implemented module or with the current features settings in case the module is already implemented.
 * @return LY_SUCCESS on success.
 * @return LY_EDENIED in case the context contains some other revision of the same module which is already implemented.
 * @return LY_ERR on other errors during module compilation.
 */
LIBYANG_API_DECL LY_ERR lys_set_implemented(struct lys_module *mod, const char **features);

/**
 * @brief Stringify schema nodetype.
 *
 * @param[in] nodetype Nodetype to stringify.
 * @return Constant string with the name of the node's type.
 */
LIBYANG_API_DECL const char *lys_nodetype2str(uint16_t nodetype);

/**
 * @brief Getter for original XPath expression from a parsed expression.
 *
 * @param[in] path Parsed expression.
 * @return Original string expression.
 */
LIBYANG_API_DECL const char *lyxp_get_expr(const struct lyxp_expr *path);

/** @} schematree */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
