/**
 * @file tree_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang representation of data model trees.
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_SCHEMA_H_
#define LY_TREE_SCHEMA_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup datatree
 * @brief Macro to iterate via all sibling elements without affecting the list itself
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket '{'. All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param ELEM Iterator.
 */
#define LY_TREE_FOR(START, ELEM) \
    for ((ELEM) = (START); \
         (ELEM); \
         (ELEM) = (ELEM)->next)

/**
 * @ingroup datatree
 * @brief Macro to iterate via all sibling elements allowing to modify the list itself (e.g. removing elements)
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type.
 *
 * Use with opening curly bracket '{'. All parameters must be of the same type.
 *
 * @param START Pointer to the starting element.
 * @param NEXT Temporary storage to allow removing of the current iterator content.
 * @param ELEM Iterator.
 */
#define LY_TREE_FOR_SAFE(START, NEXT, ELEM) \
    for ((ELEM) = (START); \
         (ELEM) ? (NEXT = (ELEM)->next, 1) : 0; \
         (ELEM) = (NEXT))

/**
 * @ingroup datatree
 * @brief Macro to iterate via all elements in a tree. This is the opening part
 * to the #LY_TREE_DFS_END - they always have to be used together.
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
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type. Use the same parameters for
 * #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_END.
 *
 * Since the next node is selected as part of #LY_TREE_DFS_END, do not use
 * continue statement between the #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_END.
 *
 * Use with opening curly bracket '{' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param NEXT Temporary storage, do not use.
 * @param ELEM Iterator intended for use in the block.
 */
#define LY_TREE_DFS_BEGIN(START, NEXT, ELEM)                                  \
    for ((ELEM) = (NEXT) = (START);                                           \
         (ELEM);                                                              \
         (ELEM) = (NEXT))

/**
 * @ingroup datatree
 * @brief Macro to iterate via all elements in a tree. This is the closing part
 * to the #LY_TREE_DFS_BEGIN - they always have to be used together.
 *
 * Works for all types of nodes despite it is data or schema tree, but all the
 * parameters must be pointers to the same type - basic type of the tree (struct
 * lys_node*, struct lyd_node* or struct lyxml_elem*). Use the same parameters for
 * #LY_TREE_DFS_BEGIN and #LY_TREE_DFS_END. If the START parameter is a derived
 * type (e.g. lys_node_leaf), caller is supposed to cast it to the base type
 * identical to the other parameters.
 *
 * Use with closing curly bracket '}' after the macro.
 *
 * @param START Pointer to the starting element processed first.
 * @param NEXT Temporary storage, do not use.
 * @param ELEM Iterator intended for use in the block.
 */

#ifdef __cplusplus
#define TYPES_COMPATIBLE(type1, type2) typeid(*(type1)) == typeid(type2)
#elif defined(__GNUC__) || defined(__clang__)
#define TYPES_COMPATIBLE(type1, type2) __builtin_types_compatible_p(__typeof__(*(type1)), type2)
#else
#define TYPES_COMPATIBLE(type1, type2) _Generic(*(type1), type2: 1, default: 0)
#endif

#define LY_TREE_DFS_END(START, NEXT, ELEM)                                    \
    /* select element for the next run - children first */                    \
    if (TYPES_COMPATIBLE(ELEM, struct lyd_node)) {                            \
        /* child exception for leafs, leaflists and anyxml without children */\
        if (((struct lyd_node *)(ELEM))->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) { \
            (NEXT) = NULL;                                                    \
        } else {                                                              \
            (NEXT) = (ELEM)->child;                                           \
        }                                                                     \
    } else if (TYPES_COMPATIBLE(ELEM, struct lys_node)) {                     \
        /* child exception for leafs, leaflists and anyxml without children */\
        if (((struct lys_node *)(ELEM))->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) { \
            (NEXT) = NULL;                                                    \
        } else {                                                              \
            (NEXT) = (ELEM)->child;                                           \
        }                                                                     \
    } else {                                                                  \
        (NEXT) = (ELEM)->child;                                               \
    }                                                                         \
                                                                              \
    if (!(NEXT)) {                                                            \
        /* no children */                                                     \
        if ((ELEM) == (START)) {                                              \
            /* we are done, (START) has no children */                        \
            break;                                                            \
        }                                                                     \
        /* try siblings */                                                    \
        (NEXT) = (ELEM)->next;                                                \
    }                                                                         \
    while (!(NEXT)) {                                                         \
        /* parent is already processed, go to its sibling */                  \
        if (TYPES_COMPATIBLE(ELEM, struct lys_node)                           \
                && (((struct lys_node *)(ELEM)->parent)->nodetype == LYS_AUGMENT)) {  \
            (ELEM) = (ELEM)->parent->prev;                                    \
        } else {                                                              \
            (ELEM) = (ELEM)->parent;                                          \
        }                                                                     \
        /* no siblings, go back through parents */                            \
        if (TYPES_COMPATIBLE(ELEM, struct lys_node)) {                        \
            /* due to possible augments */                                    \
            if (lys_parent((struct lys_node *)(ELEM)) == lys_parent((struct lys_node *)(START))) { \
                /* we are done, no next element to process */                 \
                break;                                                        \
            }                                                                 \
        } else if ((ELEM)->parent == (START)->parent) {                       \
            /* we are done, no next element to process */                     \
            break;                                                            \
        }                                                                     \
        (NEXT) = (ELEM)->next;                                                \
    }

/**
 * @defgroup schematree Schema Tree
 * @{
 *
 * Data structures and functions to manipulate and access schema tree.
 */

#define LY_ARRAY_MAX(var) (sizeof(var) == 8 ? ULLONG_MAX : ((1ULL << (sizeof(var) * 8)) - 1)) /**< maximal index the size_var is able to hold */

#define LY_REV_SIZE 11   /**< revision data string length (including terminating NULL byte) */

/**
 * @brief Schema input formats accepted by libyang [parser functions](@ref howtoschemasparsers).
 */
typedef enum {
    LYS_IN_UNKNOWN = 0,  /**< unknown format, used as return value in case of error */
    LYS_IN_YANG = 1,     /**< YANG schema input format */
    LYS_IN_YIN = 2       /**< YIN schema input format */
} LYS_INFORMAT;

/**
 * @brief Schema output formats accepted by libyang [printer functions](@ref howtoschemasprinters).
 */
typedef enum {
    LYS_OUT_UNKNOWN = 0, /**< unknown format, used as return value in case of error */
    LYS_OUT_YANG = 1,    /**< YANG schema output format */
    LYS_OUT_YIN = 2,     /**< YIN schema output format */
    LYS_OUT_TREE,        /**< Tree schema output format, for more information see the [printers](@ref howtoschemasprinters) page */
    LYS_OUT_INFO,        /**< Info schema output format, for more information see the [printers](@ref howtoschemasprinters) page */
    LYS_OUT_JSON,        /**< JSON schema output format, reflecting YIN format with conversion of attributes to object's members */
} LYS_OUTFORMAT;

/**
 * @defgroup schemaprinterflags Schema printer flags
 * @brief Schema output flags accepted by libyang [printer functions](@ref howtoschemasprinters).
 *
 * @{
 */
#define LYS_OUTOPT_TREE_RFC        0x01 /**< Conform to the RFC TODO tree output (only for tree format) */
#define LYS_OUTOPT_TREE_GROUPING   0x02 /**< Print groupings separately (only for tree format) */
#define LYS_OUTOPT_TREE_USES       0x04 /**< Print only uses instead the resolved grouping nodes (only for tree format) */
#define LYS_OUTOPT_TREE_NO_LEAFREF 0x08 /**< Do not print the target of leafrefs (only for tree format) */

/**
 * @}
 */

/* shortcuts for common in and out formats */
#define LYS_YANG 1       /**< YANG schema format, used for #LYS_INFORMAT and #LYS_OUTFORMAT */
#define LYS_YIN 2        /**< YIN schema format, used for #LYS_INFORMAT and #LYS_OUTFORMAT */

/**
 * @brief YANG schema node types
 *
 * Values are defined as separated bit values to allow checking using bitwise operations for multiple nodes.
 */
typedef enum lys_nodetype {
    LYS_UNKNOWN = 0x0000,        /**< uninitialized unknown statement node */
    LYS_CONTAINER = 0x0001,      /**< container statement node */
    LYS_CHOICE = 0x0002,         /**< choice statement node */
    LYS_LEAF = 0x0004,           /**< leaf statement node */
    LYS_LEAFLIST = 0x0008,       /**< leaf-list statement node */
    LYS_LIST = 0x0010,           /**< list statement node */
    LYS_ANYXML = 0x0020,         /**< anyxml statement node */
    LYS_CASE = 0x0040,           /**< case statement node */
    LYS_NOTIF = 0x0080,          /**< notification statement node */
    LYS_RPC = 0x0100,            /**< rpc statement node */
    LYS_INPUT = 0x0200,          /**< input statement node */
    LYS_OUTPUT = 0x0400,         /**< output statement node */
    LYS_GROUPING = 0x0800,       /**< grouping statement node */
    LYS_USES = 0x1000,           /**< uses statement node */
    LYS_AUGMENT = 0x2000,        /**< augment statement node */
    LYS_ACTION = 0x4000,         /**< action statement node */
    LYS_ANYDATA = 0x8020,        /**< anydata statement node, in tests it can be used for both #LYS_ANYXML and #LYS_ANYDATA */
    LYS_EXT = 0x10000            /**< complex extension instance, ::lys_ext_instance_complex */
} LYS_NODE;

/* all nodes sharing the node namespace except RPCs and notifications */
#define LYS_NO_RPC_NOTIF_NODE 0x807F

#define LYS_ANY 0xFFFF

/**
 * @defgroup extensions YANG Extensions
 *
 * @{
 */

/**
 * @brief List of YANG statements
 *
 * The description of each statement contains the storage type for the case the statement is specified by extension
 * plugin to appear as a substatement to the extension instance. Note that the storage type/structure are used in
 * case of #LY_STMT_CARD_OPT or #LY_STMT_CARD_MAND. In other cases, the data are stored as a pointer to the
 * NULL-terminated array of base types:
 *
 *     char*     -> char**
 *     lys_type* -> lys_type**
 *     uint8_t   -> uint8_t*
 *
 * There are some items, that are not, in case of multiple instances, stored as an array of pointers.
 * 1. The value is ORed with the previous value in the storage. Initial value is 0. This is the case of
 *    e.g. #LY_STMT_STATUS. These items actually does not allow to have multiple instances (it does not make sense).
 * 2. The lys_node_* data types are stored as a data tree, so in case of multiple instances, they are stored
 *    as siblings to the first node.
 *
 * The values <= #LY_STMT_UNIQUE are compatible with #LYEXT_SUBSTMT values which defines the subset of YANG statements
 * that does not store extension instances directly.
 */
typedef enum {
    LY_STMT_NODE = -1,    /**< mask for values #LY_STMT_ACTION - #LY_STMT_USES */
    LY_STMT_UNKNOWN = 0,  /**< error return value */
    LY_STMT_ARGUMENT = 1, /**< stored as __const char*__ + __uint8_t__, the second item contains yin element
                               interpreted as follows: 1 - true; 2 - false/default, in case of multiple instances,
                               the argument's values and yin elements are stored in two arrays
                               (__const char **__ + __uint8_t *__) */
    LY_STMT_BASE,         /**< stored as __const char*__ */
    LY_STMT_BELONGSTO,    /**< belongs-to, stored as __const char*[2]__, the second item contains belongs-to's prefix,
                               in case of multiple instances, the belongs-to's module values and prefixes are stored in
                               two arrays (__const char **[2]__) */
    LY_STMT_CONTACT,      /**< stored as __const char*__ */
    LY_STMT_DEFAULT,      /**< stored as __const char*__ */
    LY_STMT_DESCRIPTION,  /**< stored as __const char*__ */
    LY_STMT_ERRTAG,       /**< error-app-tag, stored as __const char*__ */
    LY_STMT_ERRMSG,       /**< error-message, stored as __const char*__ */
    LY_STMT_KEY,          /**< stored as __const char*__ */
    LY_STMT_NAMESPACE,    /**< stored as __const char*__ */
    LY_STMT_ORGANIZATION, /**< organization, stored as __const char*__ */
    LY_STMT_PATH,         /**< stored as __const char*__ */
    LY_STMT_PREFIX,       /**< stored as __const char*__ */
    LY_STMT_PRESENCE,     /**< stored as __const char*__ */
    LY_STMT_REFERENCE,    /**< stored as __const char*__ */
    LY_STMT_REVISIONDATE, /**< revision-date, stored as __const char*__ */
    LY_STMT_UNITS,        /**< stored as __const char*__ */
    LY_STMT_VALUE,        /**< stored as __int32_t*__ */
    LY_STMT_VERSION,      /**< not supported in extension instances */
    LY_STMT_MODIFIER,     /**< stored as __uint8_t__ interpreted as follows: 0 - not set/default; 1 - invert-match
                               does not allow multiple instances */
    LY_STMT_REQINSTANCE,  /**< require-instance, stored as __uint8_t__ interpreted as follows: 0 - not set/default;
                               1 - true; 2 - false, does not allow multiple instances */
    LY_STMT_YINELEM,      /**< not supported in extension instances  */
    LY_STMT_CONFIG,       /**< stored as __uint16_t__ value (ORed with the previous value(s)), possible values are
                               #LYS_CONFIG_R and #LYS_CONFIG_W (both ORed with #LYS_CONFIG_SET), does not allow multiple
                               instances */
    LY_STMT_MANDATORY,    /**< stored as __uint16_t__ value (ORed with the previous value(s)), possible values are
                               #LYS_MAND_TRUE and #LYS_MAND_FALSE, does not allow multiple instances */
    LY_STMT_ORDEREDBY,    /**< ordered-by, stored as __uint16_t__ value (ORed with the previous value(s)), possible
                               value is #LYS_USERORDERED, does not allow multiple instances */
    LY_STMT_STATUS,       /**< stored as __uint16_t__ value (ORed with the previous value(s)), possible values are
                               #LYS_STATUS_CURR, #LYS_STATUS_DEPRC and #LYS_STATUS_OBSLT, does not allow multiple
                               instances */
    LY_STMT_DIGITS,       /**< fraction-digits, stored as __uint8_t__ */
    LY_STMT_MAX,          /**< max-elements, stored as __uint32_t*__ */
    LY_STMT_MIN,          /**< min-elements, stored as __uint32_t*__ */
    LY_STMT_POSITION,     /**< stored as __uint32_t*__ */
    LY_STMT_UNIQUE,       /**< stored as ::lys_unique* */
    LY_STMT_MODULE,       /**< stored as ::lys_module* */
    LY_STMT_ACTION,       /**< stored as ::lys_node_rpc_action*, part of the data tree */
    LY_STMT_ANYDATA,      /**< stored as ::lys_node_anydata*, part of the data tree  */
    LY_STMT_ANYXML,       /**< stored as ::lys_node_anydata*, part of the data tree  */
    LY_STMT_CASE,         /**< stored as ::lys_node_case*, part of the data tree  */
    LY_STMT_CHOICE,       /**< stored as ::lys_node_choice*, part of the data tree  */
    LY_STMT_CONTAINER,    /**< stored as ::lys_node_container*, part of the data tree  */
    LY_STMT_GROUPING,     /**< stored as ::lys_node_grp*, part of the data tree  */
    LY_STMT_INPUT,        /**< stored as ::lys_node_inout*, part of the data tree, but it cannot appear multiple times */
    LY_STMT_LEAF,         /**< stored as ::lys_node_leaf*, part of the data tree  */
    LY_STMT_LEAFLIST,     /**< leaf-list, stored as ::lys_node_leaflist*, part of the data tree  */
    LY_STMT_LIST,         /**< stored as ::lys_node_list*, part of the data tree  */
    LY_STMT_NOTIFICATION, /**< stored as ::lys_node_notif*, part of the data tree  */
    LY_STMT_OUTPUT,       /**< stored as ::lys_node_anydata*, part of the data tree, but it cannot apper multiple times */
    LY_STMT_USES,         /**< stored as ::lys_node_uses*, part of the data tree  */
    LY_STMT_TYPEDEF,      /**< stored as ::lys_tpdf* */
    LY_STMT_TYPE,         /**< stored as ::lys_type* */
    LY_STMT_IFFEATURE,    /**< if-feature, stored as ::lys_iffeature* */
    LY_STMT_LENGTH,       /**< stored as ::lys_restr* */
    LY_STMT_MUST,         /**< stored as ::lys_restr* */
    LY_STMT_PATTERN,      /**< stored as ::lys_restr* */
    LY_STMT_RANGE,        /**< stored as ::lys_restr* */
    LY_STMT_WHEN,         /**< stored as ::lys_when* */
    LY_STMT_REVISION,     /**< stored as ::lys_revision */
    LY_STMT_SUBMODULE,    /**< not supported - submodules are tightly connected with their modules so it does not make
                               any sense to have them instantiated under an extension instance */
    LY_STMT_RPC,          /**< not supported, use actions instead */
    LY_STMT_BIT,          /**< not supported in extension instances */
    LY_STMT_ENUM,         /**< not supported in extension instances */
    LY_STMT_REFINE,       /**< not supported in extension instances */
    LY_STMT_AUGMENT,      /**< not supported in extension instances */
    LY_STMT_DEVIATE,      /**< not supported in extension instances */
    LY_STMT_DEVIATION,    /**< not supported in extension instances */
    LY_STMT_EXTENSION,    /**< not supported in extension instances */
    LY_STMT_FEATURE,      /**< not supported in extension instances */
    LY_STMT_IDENTITY,     /**< not supported in extension instances */
    LY_STMT_IMPORT,       /**< not supported in extension instances */
    LY_STMT_INCLUDE,      /**< not supported in extension instances */
} LY_STMT;

/**
 * @brief Possible cardinalities of the YANG statements.
 *
 * Used in extensions plugins to define cardinalities of the extension instance substatements.
 */
typedef enum {
    LY_STMT_CARD_OPT,    /* 0..1 */
    LY_STMT_CARD_MAND,   /* 1 */
    LY_STMT_CARD_SOME,   /* 1..n */
    LY_STMT_CARD_ANY     /* 0..n */
} LY_STMT_CARD;

/**
 * @brief Extension types
 */
typedef enum {
    LYEXT_ERR = -1,                /**< error value when #LYEXT_TYPE is expected as return value of a function */
    LYEXT_FLAG = 0,                /**< simple extension with no substatements;
                                        instance is stored directly as ::lys_ext_instance and no cast is needed;
                                        plugin is expected directly as ::lyext_plugin and no cast is done */
    LYEXT_COMPLEX                  /**< complex extension with YANG substatement(s);
                                        instance is stored as ::lys_ext_instance_complex to which it can be cast from
                                        ::lys_ext_instance;
                                        plugin is expected as ::lyext_plugin_complex to which it can be cast from
                                        ::lyext_plugin */
} LYEXT_TYPE;

/**
 * @defgroup extflags Extension flags
 * @ingroup extensions
 *
 * Various flags for extensions.

 * @{
 */
#define LYEXT_OPT_INHERIT    0x01    /**< When instantiated in lys_node, the extension is supposed to be inherited
                                          into the children lys_node elements. The plugin can affect inheriting by a
                                          callback to decide if the extension instance is supposed to be inherited.
                                          The extension instance with this flag is not printed and it is just a shadow
                                          copy of the original extension instance in some of the parents. */
/** @cond INTERNAL */
#define LYEXT_OPT_YANG       0x02    /**< temporarily stored pointer to string, which contain prefix and name of extension */
#define LYEXT_OPT_CONTENT    0x04    /**< content of lys_ext_instance_complex is copied from source (not dup, just memcpy). */
/** @endcond */
#define LYEXT_OPT_VALID      0x08    /**< needed to call calback for validation */
#define LYEXT_OPT_VALID_SUBTREE 0x10 /**< The plugin needs to do validation on nodes in the subtree of the extended node
                                          (i.e. not only the extended node nor its direct children). valid_data callback
                                          will be called when any descendant node in the subtree of the extended node is
                                          modified. */
#define LYEXT_OPT_PLUGIN1    0x0100  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN2    0x0200  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN3    0x0400  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN4    0x0800  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN5    0x1000  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN6    0x2000  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN7    0x4000  /**< reserved flag for plugin-specific use */
#define LYEXT_OPT_PLUGIN8    0x8000  /**< reserved flag for plugin-specific use */
/**
 * @}
 */

/**
 * @brief Description of the extension instance substatement.
 *
 * Provided by extensions plugins to libyang to be able to parse the content of extension instances.
 */
struct lyext_substmt {
    LY_STMT stmt;              /**< allowed substatement */
    size_t offset;             /**< offset in the ::lys_ext_instance_complex#content */
    LY_STMT_CARD cardinality;  /**< cardinality of the substatement */
};

/**
 * @brief YANG extension definition
 */
struct lys_ext {
    const char *name;                /**< extension name */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< LYS_STATUS_* and LYS_YINELEM values (@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t padding[5];              /**< padding for compatibility with ::lys_node */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    const char *argument;            /**< argument name, NULL if not specified, replacement for ::lys_node's iffeature */
    struct lys_module *module;       /**< link to the extension's data model */
    struct lyext_plugin *plugin;     /**< pointer to the plugin's data if any */
};

/**
 * @brief Generic extension instance structure
 *
 * The structure can be cast to another lys_ext_instance_* structure according to the extension type
 * that can be get via lys_ext_type() function. Check the #LYEXT_TYPE values to get know the specific mapping
 * between the extension type and lys_ext_instance_* structures.
 */
struct lys_ext_instance {
    struct lys_ext *def;             /**< definition of the instantiated extension,
                                          according to the type in the extension's plugin structure, the
                                          structure can be cast to the more specific structure */
    void *parent;                    /**< pointer to the parent element holding the extension instance(s), use
                                          ::lys_ext_instance#parent_type to access the schema element */
    const char *arg_value;           /**< value of the instance's argument, if defined */
    uint16_t flags;                  /**< [extension flags](@ref extflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t insubstmt_index;         /**< since some of the statements can appear multiple times, it is needed to
                                          keep the position of the specific statement instance which contains
                                          this extension instance. Order of both, the extension and the statement,
                                          instances is the same. The index is filled only for LYEXT_SUBSTMT_BASE,
                                          LYEXT_SUBSTMT_DEFAULT and LYEXT_SUBSTMT_UNIQUE values of the
                                          ::lys_ext_instance#insubstmt member. To get the correct pointer to the
                                          data connected with the index, use lys_ext_instance_substmt() */
    uint8_t insubstmt;               /**< #LYEXT_SUBSTMT - id for the case the extension instance is actually inside
                                          some of the node's members (substatements). libyang does not store extension
                                          instances for all possible statements to save some, commonly unused, space. */
    uint8_t parent_type;             /**< #LYEXT_PAR - type of the parent structure */
    uint8_t ext_type;                /**< extension type (#LYEXT_TYPE) */
    uint8_t padding;                 /**< 32b padding */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    void *priv;                      /**< private caller's data, not used by libyang */
    struct lys_module *module;       /**< pointer to the extension instance's module (mandatory) */
    LYS_NODE nodetype;               /**< LYS_EXT */
};

/**
 * @brief Complex extension instance structure
 *
 * The structure extends the generic ::lys_ext_instance structure to be able to hold substatements as defined by the
 * plugin.
 */
struct lys_ext_instance_complex {
    struct lys_ext *def;             /**< definition of the instantiated extension, the plugin's type is #LYEXT_COMPLEX */
    void *parent;                    /**< pointer to the parent element holding the extension instance(s), use
                                          ::lys_ext_instance#parent_type to access the schema element */
    const char *arg_value;           /**< value of the instance's argument, if defined */
    uint16_t flags;                  /**< [extension flags](@ref extflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t insubstmt_index;         /**< since some of the statements can appear multiple times, it is needed to
                                          keep the position of the specific statement instance which contains
                                          this extension instance. Order of both, the extension and the statement,
                                          instances is the same. The index is filled only for LYEXT_SUBSTMT_BASE,
                                          LYEXT_SUBSTMT_DEFAULT and LYEXT_SUBSTMT_UNIQUE values of the
                                          ::lys_ext_instance#insubstmt member. To get the correct pointer to the
                                          data connected with the index, use lys_ext_instance_substmt() */
    uint8_t insubstmt;               /**< #LYEXT_SUBSTMT - id for the case the extension instance is actually inside
                                          some of the node's members (substatements). libyang does not store extension
                                          instances for all possible statements to save some, commonly unused, space. */
    uint8_t parent_type;             /**< #LYEXT_PAR - type of the parent structure */
    uint8_t ext_type;                /**< extension type (#LYEXT_TYPE) */
    uint8_t padding;                 /**< 32b padding */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    void *priv;                      /**< private caller's data, not used by libyang */
    struct lys_module *module;       /**< pointer to the extension instance's module (mandatory) */
    LYS_NODE nodetype;               /**< LYS_EXT */

    /* to this point the structure is compatible with the generic ::lys_ext_instance structure */
    struct lyext_substmt *substmt;   /**< pointer to the plugin's list of substatements' information */
    char content[1];                 /**< content of the extension instance */
};

/**
 * @brief Get address of the substatement structure to which the extension instance refers.
 *
 * If the extension instance's substmt value is 0, NULL is returned (extension instance refers to the parent, not to
 * any of the parent's substatements).
 *
 * Returned pointer is supposed to be cast according to the extension instance's substmt value:
 * - #LYEXT_SUBSTMT_ARGUMENT     -> const char* (lys_ext_instance.arg_value)
 * - #LYEXT_SUBSTMT_BASE         -> struct lys_ident* (lys_type.info.ident.ref[index] or lys_ident.base[index])
 * - #LYEXT_SUBSTMT_BELONGSTO    -> struct lys_module* (lys_submodule.belongsto)
 * - #LYEXT_SUBSTMT_CONFIG       -> uint16_t* (e.g. lys_node.flags, use #LYS_CONFIG_MASK to get only the config flag)
 * - #LYEXT_SUBSTMT_CONTACT      -> const char* (e.g. lys_module.contact)
 * - #LYEXT_SUBSTMT_DEFAULT      -> const char* (e.g. lys_node_leaflist.dflt[index]) or struct lys_node*
 *                                  (lys_node_choice.dflt) depending on the parent
 * - #LYEXT_SUBSTMT_DESCRIPTION  -> const char* (e.g. lys_module.dsc)
 * - #LYEXT_SUBSTMT_ERRTAG       -> const char* (lys_restr.eapptag)
 * - #LYEXT_SUBSTMT_ERRMSG       -> const char* (lys_restr.emsg)
 * - #LYEXT_SUBSTMT_DIGITS       -> uint8_t* (lys_type.info.dec64.dig)
 * - #LYEXT_SUBSTMT_KEY          -> struct lys_node_leaf** (lys_node_list.keys)
 * - #LYEXT_SUBSTMT_MANDATORY    -> uint16_t* (e.g. lys_node.flags, use #LYS_MAND_MASK to get only the mandatory flag)
 * - #LYEXT_SUBSTMT_MAX          -> uint32_t* (e.g. lys_node_list.max)
 * - #LYEXT_SUBSTMT_MIN          -> uint32_t* (e.g. lys_node_list.min)
 * - #LYEXT_SUBSTMT_MODIFIER     -> NULL, the substatement is stored as a special value of the first byte of the
 *                                  restriction's expression (ly_restr.expr[0])
 * - #LYEXT_SUBSTMT_NAMESPACE    -> cont char* (lys_module.ns)
 * - #LYEXT_SUBSTMT_ORDEREDBY    -> uint16_t* (e.g. lys_node.flags, use #LYS_USERORDERED as mask to get the flag)
 * - #LYEXT_SUBSTMT_ORGANIZATION -> const char* (e.g. lys_module.org)
 * - #LYEXT_SUBSTMT_PATH         -> const char* (lys_type.info.lref.path)
 * - #LYEXT_SUBSTMT_POSITION     -> uint32_t* (bit.pos)
 * - #LYEXT_SUBSTMT_PREFIX       -> const char* (e.g. lys_module.prefix)
 * - #LYEXT_SUBSTMT_PRESENCE     -> const char* (lys_node_container.presence)
 * - #LYEXT_SUBSTMT_REFERENCE    -> const char* (e.g. lys_node.ref)
 * - #LYEXT_SUBSTMT_REQINSTANCE  -> int8_t* (lys_type.info.lref.req or lys_type.info.inst.req)
 * - #LYEXT_SUBSTMT_REVISIONDATE -> const char* (e.g. lys_import.rev)
 * - #LYEXT_SUBSTMT_STATUS       -> uint16_t* (e.g. lys_node.flags, use #LYS_STATUS_MASK to get only the status flag)
 * - #LYEXT_SUBSTMT_UNIQUE       -> struct lys_unique* (lys_node_list.unique[index])
 * - #LYEXT_SUBSTMT_UNITS        -> const char* (e.g. lys_node_leaf.units)
 * - #LYEXT_SUBSTMT_VALUE        -> int32_t* (enm.value
 * - #LYEXT_SUBSTMT_VERSION      -> NULL, the version is stored as a bit-field value so the address cannot be returned,
 *                                  however the value can be simply accessed as ((struct lys_module*)ext->parent)->version
 * - #LYEXT_SUBSTMT_YINELEM      -> NULL, the substatement is stored as a flag
 *
 * @param[in] ext The extension instance to explore
 * @return Pointer to the data connected with the statement where the extension was instantiated. Details about
 * casting the returned pointer are described above.
 */
const void *lys_ext_instance_substmt(const struct lys_ext_instance *ext);

/**
 * @brief Get the position of the extension instance in the extensions list.
 *
 * @param[in] def Definition of the extension to search
 * @param[in] ext Extensions list as they are stored in the schema tree nodes
 * @param[in] ext_size Number of items in the extensions list
 * @return -1 in case the extension is not present in the list, index of the extension in the provided list otherwise
 */
int lys_ext_instance_presence(struct lys_ext *def, struct lys_ext_instance **ext, uint8_t ext_size);

/**
 * @brief get pointer to the place where the specified extension's substatement is supposed to be stored in the complex
 * extension instance.
 *
 * @param[in] stmt Substatement to get
 * @param[in] ext Complex extension instance to be explored.
 * @param[out] info Optional output parameter providing information about the \p stmt from the plugin.
 * @return Address of the storage in the \p ext, NULL if the substatement is not allowed in this extension or any other
 * error (e.g. invalid input data).
 */
void *lys_ext_complex_get_substmt(LY_STMT stmt, struct lys_ext_instance_complex *ext, struct lyext_substmt **info);

/**
 * @brief Get list of all the loaded plugins, both extension and user type ones.
 *
 * @return Const list of all the plugin names finished with NULL.
 */
const char * const *ly_get_loaded_plugins(void);

/**
 * @brief Load the available YANG extension and type plugins from the plugin directory (LIBDIR/libyang/).
 *
 * This function is automatically called whenever a new context is created. Note that the removed plugins are kept
 * in use until all the created contexts are destroyed via ly_ctx_destroy(), so only the newly added plugins are
 * usually loaded by this function.
 */
void ly_load_plugins(void);

/* don't need the contents of these types, just forward-declare them for the next 2 functions. */
struct lyext_plugin_list;
struct lytype_plugin_list;

/**
 * @brief Directly register a YANG extension by pointer.
 *
 * This is intended to be called by executables or libraries using libyang, while bringing along their own
 * application-specific extensions.  Instead of loading them from separate module files through dlopen (which can
 * introduce additional problems like mismatching or incorrectly installed modules), they can be directly added
 * by reference.
 */
int ly_register_exts(struct lyext_plugin_list *plugin, const char *log_name);

/**
 * @brief Directly register a YANG type by pointer.
 *
 * This is the analog of ly_register_exts(), for types instead of extensions.
 */
int ly_register_types(struct lytype_plugin_list *plugin, const char *log_name);

/**
 * @brief Unload all the YANG extension and type plugins.
 *
 * This function is automatically called whenever the context is destroyed. Note, that in case there is still a
 * libyang context in use, the function does nothing since unloading the plugins would break the context's modules
 * which may refer/use the plugins.
 *
 * Since the function is called with ly_ctx_destroy(), there is usually no need to call this function manually.
 */
int ly_clean_plugins(void);

/**
 * @}
 */

/**
 * @brief supported YANG schema version values
 */
typedef enum LYS_VERSION {
    LYS_VERSION_UNDEF = 0,  /**< no specific version, YANG 1.0 as default */
    LYS_VERSION_1 = 1,      /**< YANG 1.0 */
    LYS_VERSION_1_1 = 2     /**< YANG 1.1 */
} LYS_VERSION;

/**
 * @brief Main schema node structure representing YANG module.
 *
 * Compatible with ::lys_submodule structure with exception of the last, #ns member, which is replaced by
 * ::lys_submodule#belongsto member. Sometimes, ::lys_submodule can be provided casted to ::lys_module. Such a thing
 * can be determined via the #type member value.
 */
struct lys_module {
    struct ly_ctx *ctx;              /**< libyang context of the module (mandatory) */
    const char *name;                /**< name of the module (mandatory) */
    const char *prefix;              /**< prefix of the module (mandatory) */
    const char *dsc;                 /**< description of the module */
    const char *ref;                 /**< cross-reference for the module */
    const char *org;                 /**< party/company responsible for the module */
    const char *contact;             /**< contact information for the module */
    const char *filepath;            /**< path, if the schema was read from a file, NULL in case of reading from memory */
    uint8_t type:1;                  /**< 0 - structure type used to distinguish structure from ::lys_submodule */
    uint8_t version:3;               /**< yang-version (LYS_VERSION):
                                          - 0 = not specified, YANG 1.0 as default,
                                          - 1 = YANG 1.0,
                                          - 2 = YANG 1.1 */
    uint8_t deviated:2;              /**< deviated flag:
                                          - 0 = not deviated,
                                          - 1 = the module is deviated by another module,
                                          - 2 = deviation applied to this module are temporarily off */
    uint8_t disabled:1;              /**< flag if the module is disabled in the context */
    uint8_t implemented:1;           /**< flag if the module is implemented, not just imported */
    uint8_t latest_revision:1;       /**< flag if the module was loaded without specific revision and is
                                          the latest revision found */
    uint8_t padding1:7;              /**< padding for 32b alignment */
    uint8_t padding2[2];

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in #rev array */
    uint8_t imp_size;                /**< number of elements in #imp array */
    uint8_t inc_size;                /**< number of elements in #inc array */

    uint16_t ident_size;             /**< number of elements in #ident array */
    uint16_t tpdf_size;              /**< number of elements in #tpdf array */

    uint8_t features_size;           /**< number of elements in #features array */
    uint8_t augment_size;            /**< number of elements in #augment array */
    uint8_t deviation_size;          /**< number of elements in #deviation array */
    uint8_t extensions_size;         /**< number of elements in #extensions array */
    uint8_t ext_size;                /**< number of elements in #ext array */

    struct lys_revision *rev;        /**< array of the module revisions, revisions[0] is always the last (newest)
                                          revision of the module */
    struct lys_import *imp;          /**< array of imported modules */
    struct lys_include *inc;         /**< array of included submodules */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_ident *ident;         /**< array of identities */
    struct lys_feature *features;    /**< array of feature definitions */
    struct lys_node_augment *augment;/**< array of augments */
    struct lys_deviation *deviation; /**< array of specified deviations */
    struct lys_ext *extensions;      /**< array of specified extensions */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */

    /* specific module's items in comparison to submodules */
    struct lys_node *data;           /**< first data statement, includes also RPCs and Notifications */
    const char *ns;                  /**< namespace of the module (mandatory) */
};

/**
 * @brief Submodule schema node structure that can be included into a YANG module.
 *
 * Compatible with ::lys_module structure with exception of the last, #belongsto member, which is replaced by
 * ::lys_module#data and ::lys_module#ns members. Sometimes, ::lys_submodule can be provided casted to ::lys_module.
 * Such a thing can be determined via the #type member value.
 */
struct lys_submodule {
    struct ly_ctx *ctx;              /**< libyang context of the submodule (mandatory) */
    const char *name;                /**< name of the submodule (mandatory) */
    const char *prefix;              /**< prefix of the belongs-to module */
    const char *dsc;                 /**< description of the submodule */
    const char *ref;                 /**< cross-reference for the submodule */
    const char *org;                 /**< party responsible for the submodule */
    const char *contact;             /**< contact information for the submodule */
    const char *filepath;            /**< path to the file from which the submodule was read */
    uint8_t type:1;                  /**< 1 - structure type used to distinguish structure from ::lys_module */
    uint8_t version:3;               /**< yang-version (LYS_VERSION):
                                          - 0 = not specified, YANG 1.0 as default,
                                          - 1 = YANG 1.0,
                                          - 2 = YANG 1.1 */
    uint8_t deviated:2;              /**< deviated flag (same as in main module):
                                          - 0 = not deviated,
                                          - 1 = the module is deviated by another module,
                                          - 2 = deviation applied to this module are temporarily off */
    uint8_t disabled:1;              /**< flag if the module is disabled in the context (same as in main module) */
    uint8_t implemented:1;           /**< flag if the module is implemented, not just imported (same as in main module) */
    uint8_t padding[3];              /**< padding for 32b alignment */

    /* array sizes */
    uint8_t rev_size;                /**< number of elements in #rev array */
    uint8_t imp_size;                /**< number of elements in #imp array */
    uint8_t inc_size;                /**< number of elements in #inc array */

    uint16_t ident_size;             /**< number of elements in #ident array */
    uint16_t tpdf_size;              /**< number of elements in #tpdf array */

    uint8_t features_size;           /**< number of elements in #features array */
    uint8_t augment_size;            /**< number of elements in #augment array */
    uint8_t deviation_size;          /**< number of elements in #deviation array */
    uint8_t extensions_size;         /**< number of elements in #extensions array */
    uint8_t ext_size;                /**< number of elements in #ext array */

    struct lys_revision *rev;        /**< array of the module revisions, revisions[0] is always the last (newest)
                                          revision of the submodule */
    struct lys_import *imp;          /**< array of imported modules */
    struct lys_include *inc;         /**< array of included submodules */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_ident *ident;         /**< array if identities */
    struct lys_feature *features;    /**< array of feature definitions */
    struct lys_node_augment *augment;/**< array of augments */
    struct lys_deviation *deviation; /**< array of specified deviations */
    struct lys_ext *extensions;      /**< array of specified extensions */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */

    /* specific submodule's items in comparison to modules */
    struct lys_module *belongsto;    /**< belongs-to (parent module) */
};

/**
 * @brief YANG built-in types
 */
typedef enum {
    LY_TYPE_DER = 0,      /**< Derived type */
    LY_TYPE_BINARY,       /**< Any binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
    LY_TYPE_BITS,         /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
    LY_TYPE_BOOL,         /**< "true" or "false" ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
    LY_TYPE_DEC64,        /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3))*/
    LY_TYPE_EMPTY,        /**< A leaf that does not have any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
    LY_TYPE_ENUM,         /**< Enumerated strings ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
    LY_TYPE_IDENT,        /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
    LY_TYPE_INST,         /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
    LY_TYPE_LEAFREF,      /**< A reference to a leaf instance ([RFC 6020 sec 9.9](http://tools.ietf.org/html/rfc6020#section-9.9))*/
    LY_TYPE_STRING,       /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
    LY_TYPE_UNION,        /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */
    LY_TYPE_INT8,         /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT8,        /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT16,        /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT16,       /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT32,        /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT32,       /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_INT64,        /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UINT64,       /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    LY_TYPE_UNKNOWN,      /**< Unknown type (used in edit-config leaves) */
} LY_DATA_TYPE;
#define LY_DATA_TYPE_COUNT 20 /**< Number of different types */

/**
 *
 */
struct lys_type_info_binary {
    struct lys_restr *length;    /**< length restriction (optional), see
                                      [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4) */
};

/**
 * @brief Single bit value specification for ::lys_type_info_bits.
 */
struct lys_type_bit {
    const char *name;                /**< bit's name (mandatory) */
    const char *dsc;                 /**< bit's description (optional) */
    const char *ref;                 /**< bit's reference (optional) */
    uint16_t flags;                  /**< bit's flags, whether the position was auto-assigned
                                          and the status(one of LYS_NODE_STATUS_* values or 0 for default) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* 32b padding for compatibility with ::lys_node */
    uint32_t pos;                    /**< bit's position (mandatory) */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
};

/**
 * @brief Container for information about bits types (#LY_TYPE_BINARY), used in ::lys_type_info.
 */
struct lys_type_info_bits {
    struct lys_type_bit *bit;/**< array of bit definitions */
    unsigned int count;      /**< number of bit definitions in the bit array */
};

/**
 * @brief Container for information about decimal64 types (#LY_TYPE_DEC64), used in ::lys_type_info.
 */
struct lys_type_info_dec64 {
    struct lys_restr *range; /**< range restriction (optional), see
                                  [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4) */
    uint8_t dig;             /**< fraction-digits restriction (mandatory). Note that in case of types not directly
                                  derived from built-in decimal64, dig is present even it cannot be specified in schema.
                                  That's because the value is inherited for simpler access to the value and easier
                                  manipulation with the decimal64 data */
    uint64_t div;            /**< auxiliary value for moving decimal point (dividing the stored value to get the real value) */
};

/**
 * @brief Single enumeration value specification for ::lys_type_info_enums.
 */
struct lys_type_enum {
    const char *name;                /**< enum's name (mandatory) */
    const char *dsc;                 /**< enum's description (optional) */
    const char *ref;                 /**< enum's reference (optional) */
    uint16_t flags;                  /**< enum's flags, whether the value was auto-assigned
                                          and the status(one of LYS_NODE_STATUS_* values or 0 for default) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* 32b padding for compatibility with ::lys_node */
    int32_t value;                   /**< enum's value (mandatory) */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
};

/**
 * @brief Container for information about enumeration types (#LY_TYPE_ENUM), used in ::lys_type_info.
 */
struct lys_type_info_enums {
    struct lys_type_enum *enm;/**< array of enum definitions */
    unsigned int count;       /**< number of enum definitions in the enm array */
};

/**
 * @brief Container for information about identity types (#LY_TYPE_IDENT), used in ::lys_type_info.
 */
struct lys_type_info_ident {
    struct lys_ident **ref;   /**< array of pointers (reference) to the identity definition (mandatory) */
    unsigned int count;       /**< number of base identity references */
};

/**
 * @brief Container for information about instance-identifier types (#LY_TYPE_INST), used in ::lys_type_info.
 */
struct lys_type_info_inst {
    int8_t req;              /**< require-instance restriction, see
                                  [RFC 6020 sec. 9.13.2](http://tools.ietf.org/html/rfc6020#section-9.13.2):
                                  - -1 = false,
                                  - 0 not defined (true),
                                  - 1 = true */
};

/**
 * @brief Container for information about integer types, used in ::lys_type_info.
 */
struct lys_type_info_num {
    struct lys_restr *range; /**< range restriction (optional), see
                                  [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4) */
};

/**
 * @brief Container for information about leafref types (#LY_TYPE_LEAFREF), used in ::lys_type_info.
 */
struct lys_type_info_lref {
    const char *path;        /**< path to the referred leaf or leaf-list node (mandatory), see
                                  [RFC 6020 sec. 9.9.2](http://tools.ietf.org/html/rfc6020#section-9.9.2) */
    struct lys_node_leaf* target; /**< target schema node according to path */
    int8_t req;              /**< require-instance restriction:
                                  - -1 = false,
                                  - 0 not defined (true),
                                  - 1 = true */
};

/**
 * @brief Container for information about string types (#LY_TYPE_STRING), used in ::lys_type_info.
 */
struct lys_type_info_str {
    struct lys_restr *length;/**< length restriction (optional), see
                                  [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4) */
    struct lys_restr *patterns; /**< array of pattern restrictions (optional), see
                                  [RFC 6020 sec. 9.4.6](http://tools.ietf.org/html/rfc6020#section-9.4.6)
                                  In each pattern, the first byte of expr is modifier:
                                  - 0x06 (ACK) for match
                                  - 0x15 (NACK) for invert-match
                                  So the expression itself always starts at expr[1] */
    unsigned int pat_count;  /**< number of pattern definitions in the patterns array */
#ifdef LY_ENABLED_CACHE
    void **patterns_pcre;    /**< array of compiled patterns to optimize its evaluation, represented as
                                  array of pointers to results of pcre_compile() and pcre_study().
                                  For internal use only. */
#endif
};

/**
 * @brief Container for information about union types (#LY_TYPE_UNION), used in ::lys_type_info.
 */
struct lys_type_info_union {
    struct lys_type *types;  /**< array of union's subtypes */
    unsigned int count;      /**< number of subtype definitions in types array */
    int has_ptr_type;        /**< types include an instance-identifier or leafref meaning the union must always be resolved
                                  after parsing */
};

/**
 * @brief Union for holding type-specific information in ::lys_type.
 */
union lys_type_info {
    struct lys_type_info_binary binary; /**< part for #LY_TYPE_BINARY */
    struct lys_type_info_bits bits;     /**< part for #LY_TYPE_BITS */
    struct lys_type_info_dec64 dec64;   /**< part for #LY_TYPE_DEC64 */
    struct lys_type_info_enums enums;   /**< part for #LY_TYPE_ENUM */
    struct lys_type_info_ident ident;   /**< part for #LY_TYPE_IDENT */
    struct lys_type_info_inst inst;     /**< part for #LY_TYPE_INST */
    struct lys_type_info_num num;       /**< part for integer types */
    struct lys_type_info_lref lref;     /**< part for #LY_TYPE_LEAFREF */
    struct lys_type_info_str str;       /**< part for #LY_TYPE_STRING */
    struct lys_type_info_union uni;     /**< part for #LY_TYPE_UNION */
};

/**
 * @brief YANG type structure providing information from the schema
 */
struct lys_type {
    LY_DATA_TYPE _PACKED base;       /**< base type */
    uint8_t value_flags;             /**< value type flags */
    uint8_t ext_size;                /**< number of elements in #ext array */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_tpdf *der;            /**< pointer to the superior typedef. If NULL,
                                          structure provides information about one of the built-in types */
    struct lys_tpdf *parent;         /**< except ::lys_tpdf, it can points also to ::lys_node_leaf or ::lys_node_leaflist
                                          so access only the compatible members! */
    union lys_type_info info;        /**< detailed type-specific information */
    /*
     * here is an overview of the info union:
     * LY_TYPE_BINARY (binary)
     * struct lys_restr *binary.length;   length restriction (optional), see
     *                                    [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_BITS (bits)
     * struct lys_type_bit *bits.bit;     array of bit definitions
     *   const char *bits.bit[i].name;    bit's name (mandatory)
     *   const char *bits.bit[i].dsc;     bit's description (optional)
     *   const char *bits.bit[i].ref;     bit's reference (optional)
     *   uint8_t bits.bit[i].flags;       bit's flags, whether the position was auto-assigned
     *                                    and the status(one of LYS_NODE_STATUS_* values or 0 for default)
     *   uint8_t bits.bit[i].iffeature_size;            number of elements in the bit's #iffeature array
     *   uint8_t bits.bit[i].ext_size;                  number of elements in the bit's #ext array
     *   uint32_t bits.bit[i].pos;        bit's position (mandatory)
     *   struct lys_iffeature *bits.bit[i].iffeature;   array of bit's if-feature expressions
     *   struct lys_ext_instance **bits.bit[i].ext;     array of pointers to the bit's extension instances (optional)
     * unsigned int bits.count;           number of bit definitions in the bit array
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_DEC64 (dec64)
     * struct lys_restr *dec64.range;     range restriction (optional), see
     *                                    [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4)
     * struct lys_ext_instance **dec64.ext;             array of pointers to the bit's extension instances (optional)
     * uint8_t dec64.ext_size;                          number of elements in the bit's #ext array
     * uint8_t dec64.dig;                 fraction-digits restriction (mandatory)
     * uint64_t dec64.div;                auxiliary value for moving decimal point (dividing the stored value to get
     *                                    the real value) (mandatory, corresponds to the fraction-digits)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_ENUM (enums)
     * struct lys_type_enum *enums.enm;   array of enum definitions
     *   const char *enums.enm[i].name;   enum's name (mandatory)
     *   const char *enums.enm[i].dsc;    enum's description (optional)
     *   const char *enums.enm[i].ref;    enum's reference (optional)
     *   uint8_t enums.enm[i].flags;      enum's flags, whether the value was auto-assigned
     *                                    and the status(one of LYS_NODE_STATUS_* values or 0 for default)
     *   uint8_t enums.enum[i].iffeature_size;          number of elements in the bit's #iffeature array
     *   uint8_t enums.enum[i].ext_size;                number of elements in the bit's #ext array
     *   int32_t enums.enm[i].value;      enum's value (mandatory)
     *   struct lys_iffeature *enums.enum[i].iffeature; array of bit's if-feature expressions
     *   struct lys_ext_instance **enums.enum[i].ext;   array of pointers to the bit's extension instances (optional)
     * unsigned int enums.count;          number of enum definitions in the enm array
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_IDENT (ident)
     * struct lys_ident **ident.ref;      array of pointers (reference) to the identity definition (mandatory)
     * unsigned int ident.count;          number of base identity references
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_INST (inst)
     * int8_t inst.req;                   require-identifier restriction, see
     *                                    [RFC 6020 sec. 9.13.2](http://tools.ietf.org/html/rfc6020#section-9.13.2):
     *                                    - -1 = false,
     *                                    - 0 not defined,
     *                                    - 1 = true
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_*INT* (num)
     * struct lys_restr *num.range;       range restriction (optional), see
     *                                    [RFC 6020 sec. 9.2.4](http://tools.ietf.org/html/rfc6020#section-9.2.4)
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_LEAFREF (lref)
     * const char *lref.path;             path to the referred leaf or leaf-list node (mandatory), see
     *                                    [RFC 6020 sec. 9.9.2](http://tools.ietf.org/html/rfc6020#section-9.9.2)
     * struct lys_node_leaf *lref.target; target schema node according to path
     * int8_t lref.req;                   require-instance restriction: -1 = false; 0 not defined (true); 1 = true
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_STRING (str)
     * struct lys_restr *str.length;      length restriction (optional), see
     *                                    [RFC 6020 sec. 9.4.4](http://tools.ietf.org/html/rfc6020#section-9.4.4)
     * struct lys_restr *str.patterns;    array of pattern restrictions (optional), see
     *                                    [RFC 6020 sec. 9.4.6](http://tools.ietf.org/html/rfc6020#section-9.4.6)
     * unsigned int str.pat_count;        number of pattern definitions in the patterns array
     * -----------------------------------------------------------------------------------------------------------------
     * LY_TYPE_UNION (uni)
     * struct lys_type *uni.types;        array of union's subtypes
     * unsigned int uni.count;            number of subtype definitions in types array
     * int uni.has_ptr_type;              types recursively include an instance-identifier or leafref (union must always
     *                                    be resolved after it is parsed)
     */
};

#define LYS_IFF_NOT  0x00
#define LYS_IFF_AND  0x01
#define LYS_IFF_OR   0x02
#define LYS_IFF_F    0x03

/**
 * @brief Compiled if-feature expression structure
 */
struct lys_iffeature {
    uint8_t *expr;                   /**< 2bits array describing the if-feature expression in prefix format */
    uint8_t ext_size;                /**< number of elements in #ext array */
    struct lys_feature **features;   /**< array of pointers to the features used in expression */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
};

/**
 * @defgroup snodeflags Schema nodes flags
 * @ingroup schematree
 *
 * Various flags for schema nodes.
 *
 *     1 - container    6 - anydata/anyxml    11 - output       16 - type(def)
 *     2 - choice       7 - case              12 - grouping     17 - identity
 *     3 - leaf         8 - notification      13 - uses         18 - refine
 *     4 - leaflist     9 - rpc               14 - augment      19 - extension
 *     5 - list        10 - input             15 - feature
 *
 *                                            1 1 1 1 1 1 1 1 1 1
 *                          1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
 *     --------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      1 LYS_USESGRP      | | | | | | | | | | | | |x| | | | | | |
 *        LYS_AUTOASSIGNED | | | | | | | | | | | | | | | |x| | | |
 *        LYS_CONFIG_W     |x|x|x|x|x|x|x| | | | | | | | | | |x| |
 *        LYS_NOTAPPLIED   | | | | | | | | | | | | | |x| | | | | |
 *        LYS_YINELEM      | | | | | | | | | | | | | | | | | | |x|
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      2 LYS_CONFIG_R     |x|x|x|x|x|x|x| | | | | | | | | | |x| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      3 LYS_CONFIG_SET   |x|x|x|x|x|x| | | | | | | | | | | | | |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      4 LYS_STATUS_CURR  |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|
 *        LYS_RFN_MAXSET   | | | | | | | | | | | | | | | | | |x| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      5 LYS_STATUS_DEPRC |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|
 *        LYS_RFN_MINSET   | | | | | | | | | | | | | | | | | |x| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      6 LYS_STATUS_OBSLT |x|x|x|x|x|x|x|x|x| | |x|x|x|x|x|x| |x|
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      7 LYS_MAND_TRUE    | |x|x| | |x| | | | | | | | | | | |x| |
 *        LYS_IMPLICIT     | | | | | | |x| | |x|x| | | | | | | | |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      8 LYS_MAND_FALSE   | |x|x| | |x| | | | | | | | | | | |x| |
 *        LYS_INCL_STATUS  |x| | | |x| | | | | | | | | | | | | | |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      9 LYS_USERORDERED  | | | |x|x| | | | | | | | | | | | |r| |
 *        LYS_UNIQUE       | | |x| | | | | | | | | | | | | | |r| |
 *        LYS_FENABLED     | | | | | | | | | | | | | | |x| | |r| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     10 LYS_XPCONF_DEP   |x|x|x|x|x|x|x|x|x|x|x| |x|x| | | |r| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     11 LYS_XPSTATE_DEP  |x|x|x|x|x|x|x|x|x|x|x| |x|x| | | |r| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     12 LYS_LEAFREF_DEP  |x|x|x|x|x|x|x|x|x|x|x| |x|x| | | |r| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     13 LYS_DFLTJSON     | | |x|x| | | | | | | | | | | |x| |r| |
 *                         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     14 LYS_VALID_EXT    |x| |x|x|x|x| | | | | | | | | |x| | | |
 *     --------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *     x - used
 *     r - reserved for internal use
 * @{
 */
#define LYS_CONFIG_W     0x01        /**< config true; */
#define LYS_CONFIG_R     0x02        /**< config false; */
#define LYS_CONFIG_SET   0x04        /**< config explicitely set in the node */
#define LYS_CONFIG_MASK  0x03        /**< mask for config value */
#define LYS_STATUS_CURR  0x08        /**< status current; */
#define LYS_STATUS_DEPRC 0x10        /**< status deprecated; */
#define LYS_STATUS_OBSLT 0x20        /**< status obsolete; */
#define LYS_STATUS_MASK  0x38        /**< mask for status value */
#define LYS_RFN_MAXSET   0x08        /**< refine has max-elements set */
#define LYS_RFN_MINSET   0x10        /**< refine has min-elements set */
#define LYS_MAND_TRUE    0x40        /**< mandatory true; applicable only to
                                          ::lys_node_choice, ::lys_node_leaf and ::lys_node_anydata */
#define LYS_MAND_FALSE   0x80        /**< mandatory false; applicable only to
                                          ::lys_node_choice, ::lys_node_leaf and ::lys_node_anydata */
#define LYS_INCL_STATUS  0x80        /**< flag that the subtree includes status node(s), applicable only to
                                          ::lys_node_container and lys_node_list */
#define LYS_MAND_MASK    0xc0        /**< mask for mandatory values */
#define LYS_USERORDERED  0x100       /**< ordered-by user lists, applicable only to
                                          ::lys_node_list and ::lys_node_leaflist */
#define LYS_FENABLED     0x100       /**< feature enabled flag, applicable only to ::lys_feature */
#define LYS_UNIQUE       0x100       /**< part of the list's unique, applicable only to ::lys_node_leaf */
#define LYS_AUTOASSIGNED 0x01        /**< value was auto-assigned, applicable only to
                                          ::lys_type enum and bits flags */
#define LYS_USESGRP      0x01        /**< flag for resolving uses in groupings, applicable only to ::lys_node_uses */
#define LYS_IMPLICIT     0x40        /**< flag for implicitely created LYS_INPUT, LYS_OUTPUT and LYS_CASE nodes */
#define LYS_XPCONF_DEP   0x200       /**< flag marking nodes, whose validation (when, must expressions)
                                          depends on configuration data nodes outside their subtree (applicable only
                                          to RPCs, notifications, and actions) */
#define LYS_XPSTATE_DEP  0x400       /**< flag marking nodes, whose validation (when, must expressions)
                                          depends on state data nodes outside their subtree (applicable only to RPCs,
                                          notifications, and actions) */
#define LYS_LEAFREF_DEP  0x800       /**< flag marking nodes, whose validation (leafrefs)
                                          depends on nodes outside their subtree (applicable to RPCs,
                                          notifications, actions) or outside their module (applicate to data) */
#define LYS_DFLTJSON     0x1000       /**< default value (in ::lys_node_leaf, ::lys_node_leaflist, :lys_tpdf) was
                                          converted into JSON format, since it contains identityref value which is
                                          being used in JSON format (instead of module prefixes, we use the module
                                          names) */
#define LYS_NOTAPPLIED   0x01        /**< flag for the not applied augments to allow keeping the resolved target */
#define LYS_YINELEM      0x01        /**< yin-element true for extension's argument */
#define LYS_VALID_EXT    0x2000      /**< flag marking nodes that need to be validated using an extension validation function */
#define LYS_VALID_EXT_SUBTREE 0x4000 /**< flag marking nodes that need to be validated using an extension
                                          validation function when one of their children nodes is modified */

/**
 * @}
 */

#ifdef LY_ENABLED_CACHE

/**
 * @brief Maximum number of hashes stored in a schema node if cache is enabled.
 */
#define LYS_NODE_HASH_COUNT 4

#endif

/**
 * @brief Common structure representing single YANG data statement describing.
 *
 * This is a common structure to allow having a homogeneous tree of nodes despite the nodes are actually
 * heterogeneous. It allow one to go through the tree in a simple way. However, if you want to work with
 * the node in some way or get more appropriate information, you are supposed to cast it to the appropriate
 * lys_node_* structure according to the #nodetype value.
 *
 * To traverse through all the child elements, use #LY_TREE_FOR or #LY_TREE_FOR_SAFE macro. To traverse
 * the whole subtree, use #LY_TREE_DFS_BEGIN macro.
 *
 * To cover all possible schema nodes, the ::lys_node type is used in ::lyd_node#schema for referencing schema
 * definition for a specific data node instance.
 *
 * The #priv member is completely out of libyang control. It is just a pointer to allow libyang
 * caller to store some proprietary data (e.g. callbacks) connected with the specific schema tree node.
 */
struct lys_node {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    uint8_t padding[4];              /**< 32b padding - on 64b it just fills the already required data making the
                                          space for type-specific values used by the structures derived from lys_node,
                                          on 32b it adds one word to lys_node, but this space is anyway required by
                                          the (really used) derived structures, so there is no wasting (except
                                          ::lys_node_choice, ::lys_node_case and ::lys_node_augment) */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node \note Since other lys_node_*
                                          structures represent end nodes, this member
                                          is replaced in those structures. Therefore, be careful with accessing
                                          this member without having information about the ::lys_node#nodetype. */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif
};

/**
 * @brief Schema container node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when,
 * #must, #tpdf, and #presence members.
 *
 * The container schema node can be instantiated in the data tree, so the ::lys_node_container can be directly
 * referenced from ::lyd_node#schema.
 */
struct lys_node_container {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[1];              /**< padding for 32b alignment */
    uint8_t must_size;               /**< number of elements in the #must array */
    uint16_t tpdf_size;              /**< number of elements in the #tpdf array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_CONTAINER */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific container's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_restr *must;          /**< array of must constraints */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    const char *presence;            /**< presence description, used also as a presence flag (optional) */
};

/**
 * @brief Schema choice node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when and
 * #dflt members.
 *
 * The choice schema node has no instance in the data tree, so the ::lys_node_choice cannot be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_choice {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[4];              /**< padding for 32b alignment */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_CHOICE */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific choice's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_node *dflt;           /**< default case of the choice (optional) */
};

/**
 * @brief Schema leaf node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #type,
 * #units, #must_size, #must and #dflt members. In addition, the structure is compatible with the ::lys_node_leaflist
 * structure except the last #dflt member, which is replaced by ::lys_node_leaflist#min and ::lys_node_leaflist#max
 * members.
 *
 * The leaf schema node can be instantiated in the data tree, so the ::lys_node_leaf can be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_leaf {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[3];              /**< padding for 32b alignment */
    uint8_t must_size;               /**< number of elements in the #must array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_LEAF */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    void *child;                     /**< dummy attribute as a replacement for ::lys_node's child member */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific leaf's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_restr *must;          /**< array of must constraints */
    struct lys_type type;            /**< YANG data type definition of the leaf (mandatory) */
    const char *units;               /**< units of the data type (optional) */

    /* to this point, struct lys_node_leaf is compatible with struct lys_node_leaflist */
    const char *dflt;                /**< default value of the leaf */
};

/**
 * @brief Schema leaf-list node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #type,
 * #units, #must_size, #must, #min and #max members. In addition, the structure is compatible with the ::lys_node_leaf
 * structure except the last #min and #max members, which are replaced by ::lys_node_leaf#dflt member.
 *
 * The leaf-list schema node can be instantiated in the data tree, so the ::lys_node_leaflist can be directly
 * referenced from ::lyd_node#schema.
 */
struct lys_node_leaflist {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[2];              /**< padding for 32b alignment */
    uint8_t dflt_size;               /**< number of elements in the #dflt array */
    uint8_t must_size;               /**< number of elements in the #must array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_LEAFLIST */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct ly_set *backlinks;        /**< replacement for ::lys_node's child member, it is NULL except the leaf/leaflist
                                          is target of a leafref. In that case the set stores ::lys_node leafref objects
                                          with path referencing the current ::lys_node_leaf */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific leaf-list's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_restr *must;          /**< array of must constraints */
    struct lys_type type;            /**< YANG data type definition of the leaf (mandatory) */
    const char *units;               /**< units of the data type (optional) */

    /* to this point, struct lys_node_leaflist is compatible with struct lys_node_leaf
     * on the other hand, the min and max are compatible with struct lys_node_list */
    const char **dflt;               /**< array of default value(s) of the leaflist */
    uint32_t min;                    /**< min-elements constraint (optional) */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded (optional) */
};

/**
 * @brief Schema list node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #min,
 * #max, #must_size, #tpdf_size, #keys_size, #unique_size, #must, #tpdf, #keys and #unique members.
 *
 * The list schema node can be instantiated in the data tree, so the ::lys_node_list can be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_list {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t must_size;               /**< number of elements in the #must array */
    uint8_t tpdf_size;               /**< number of elements in the #tpdf array */
    uint8_t keys_size;               /**< number of elements in the #keys array */
    uint8_t unique_size;             /**< number of elements in the #unique array (number of unique statements) */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_LIST */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific list's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_restr *must;          /**< array of must constraints */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_node_leaf **keys;     /**< array of pointers to the key nodes */
    struct lys_unique *unique;       /**< array of unique statement structures */

    uint32_t min;                    /**< min-elements constraint */
    uint32_t max;                    /**< max-elements constraint, 0 means unbounded */

    const char *keys_str;            /**< string defining the keys, must be stored besides the keys array since the
                                          keys may not be present in case the list is inside grouping */

};

/**
 * @brief Schema anydata (and anyxml) node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #must_size
 * and #must members.
 *
 * ::lys_node_anydata is terminating node in the schema tree, so the #child member value is always NULL.
 *
 * The anydata and anyxml schema nodes can be instantiated in the data tree, so the ::lys_node_anydata can be directly
 * referenced from ::lyd_node#schema.
 */
struct lys_node_anydata {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[3];              /**< padding for 32b alignment */
    uint8_t must_size;               /**< number of elements in the #must array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_ANYDATA or #LYS_ANYXML */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< always NULL */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific anyxml's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_restr *must;          /**< array of must constraints */
};

/**
 * @brief Schema uses node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when, #grp,
 * #refine_size, #augment_size, #refine and #augment members.
 *
 * ::lys_node_uses is terminating node in the schema tree. However, it references data from a specific grouping so the
 * #child pointer points to the copy of grouping data applying specified refine and augment statements.
 *
 * The uses schema node has no instance in the data tree, so the ::lys_node_uses cannot be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_uses {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* and LYS_USESGRP
                                          values are allowed */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[2];              /**< padding for 32b alignment */
    uint8_t refine_size;             /**< number of elements in the #refine array */
    uint8_t augment_size;            /**< number of elements in the #augment array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_USES */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node imported from the referenced grouping */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific uses's data */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_refine *refine;       /**< array of refine changes to the referred grouping */
    struct lys_node_augment *augment;/**< array of local augments to the referred grouping */
    struct lys_node_grp *grp;        /**< referred grouping definition (mandatory) */
};

/**
 * @brief Schema grouping node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #tpdf_size and
 * #tpdf members.
 *
 * ::lys_node_grp contains data specifications in the schema tree. However, the data does not directly form the schema
 * data tree. Instead, they are referenced via uses (::lys_node_uses) statement and copies of the grouping data are
 * actually placed into the uses nodes. Therefore, the nodes you can find under the ::lys_node_grp are not referenced
 * from ::lyd_node#schema.
 */
struct lys_node_grp {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t padding_iffsize;         /**< padding byte for the ::lys_node's iffeature_size */

    /* non compatible 32b with ::lys_node */
    uint16_t unres_count;            /**< internal counter for unresolved uses, should be always 0 when the module is parsed */
    uint16_t tpdf_size;              /**< number of elements in #tpdf array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    void *padding_iff;               /**< padding pointer for the ::lys_node's iffeature pointer */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_GROUPING */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific grouping's data */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
};

/**
 * @brief Schema case node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #when member.
 *
 * The case schema node has no instance in the data tree, so the ::lys_node_case cannot be directly referenced from
 * ::lyd_node#schema.
 */
struct lys_node_case {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[4];              /**< padding for 32b alignment */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_CASE */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific case's data */
    struct lys_when *when;           /**< when statement (optional) */
};

/**
 * @brief RPC input and output node structure.
 *
 * The structure is compatible with ::lys_node, but the most parts are not usable. Therefore the ::lys_node#name,
 * ::lys_node#dsc, ::lys_node#ref and ::lys_node#flags were replaced by empty bytes in fill arrays.
 * The reason to keep these useless bytes in the structure is to keep the #nodetype, #parent, #child, #next and #prev
 * members accessible when functions are using the object via a generic ::lyd_node structure. But note that the
 * ::lys_node#iffeature_size is replaced by the #tpdf_size member and ::lys_node#iffeature is replaced by the #tpdf
 * member.
 *
 * Note, that the inout nodes are always present in ::lys_node_rpc_action node as its input and output children
 * nodes. If they are not specified explicitely in the schema, they are implicitly added to serve as possible target
 * of augments. These implicit elements can be recognised via #LYS_IMPLICIT bit in flags member of the input/output
 * node.
 */
struct lys_node_inout {
    const char *name;
    void *fill1[2];                  /**< padding for compatibility with ::lys_node - dsc and ref */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_IMPLICIT is applicable */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t padding_iffsize;         /**< padding byte for the ::lys_node's iffeature_size */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[1];              /**< padding for 32b alignment */
    uint8_t must_size;               /**< number of elements in the #must array */
    uint16_t tpdf_size;              /**< number of elements in the #tpdf array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    void *padding_iff;               /**< padding pointer for the ::lys_node's iffeature pointer */
    struct lys_module *module;       /**< link to the node's data model */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_INPUT or #LYS_OUTPUT */
    struct lys_node *parent;         /**< pointer to the parent rpc node  */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

    /* specific inout's data */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_restr *must;          /**< array of must constraints */
};

/**
 * @brief Schema notification node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #tpdf_size and
 * #tpdf members.
 */
struct lys_node_notif {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[1];              /**< padding for 32b alignment */
    uint8_t must_size;               /**< number of elements in the #must array */
    uint16_t tpdf_size;              /**< number of elements in the #tpdf array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_NOTIF */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific rpc's data */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
    struct lys_restr *must;          /**< array of must constraints */
};

/**
 * @brief Schema rpc/action node structure.
 *
 * Beginning of the structure is completely compatible with ::lys_node structure extending it by the #tpdf_size and
 * #tpdf members.
 *
 * Note, that the rpc/action node has always input and output children nodes. If they are not specified explicitly in
 * the schema, they are implicitly added to server as possible target of augments. These implicit elements can be
 * recognized via #LYS_IMPLICIT bit in flags member of the input/output node.
 */
struct lys_node_rpc_action {
    const char *name;                /**< node name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[2];              /**< padding for 32b alignment */
    uint16_t tpdf_size;              /**< number of elements in the #tpdf array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< type of the node (mandatory) - #LYS_RPC or #LYS_ACTION */
    struct lys_node *parent;         /**< pointer to the parent node, NULL in case of a top level node */
    struct lys_node *child;          /**< pointer to the first child node */
    struct lys_node *next;           /**< pointer to the next sibling node (NULL if there is no one) */
    struct lys_node *prev;           /**< pointer to the previous sibling node \note Note that this pointer is
                                          never NULL. If there is no sibling node, pointer points to the node
                                          itself. In case of the first node, this pointer points to the last
                                          node in the list. */

    void *priv;                      /**< private caller's data, not used by libyang */

#ifdef LY_ENABLED_CACHE
    uint8_t hash[LYS_NODE_HASH_COUNT]; /**< schema hash required for LYB printer/parser */
#endif

    /* specific rpc's data */
    struct lys_tpdf *tpdf;           /**< array of typedefs */
};

/**
 * @brief YANG augment structure (covering both possibilities - uses's substatement as well as (sub)module's substatement).
 *
 * This structure is partially interchangeable with ::lys_node structure with the following exceptions:
 * - ::lys_node#name member is replaced by ::lys_node_augment#target_name member
 * - ::lys_node_augment structure is extended by the #when and #target member
 *
 * ::lys_node_augment is not placed between all other nodes defining data node. However, it must be compatible with
 * ::lys_node structure since its children actually keeps the parent pointer to point to the original augment node
 * instead of the target node they augments (the target node is accessible via the ::lys_node_augment#target pointer).
 * The fact that a schema node comes from augment can be get via testing the #nodetype of its parent - the value in
 * ::lys_node_augment is #LYS_AUGMENT.
 */
struct lys_node_augment {
    const char *target_name;         /**< schema node identifier of the node where the augment content is supposed to be
                                          placed (mandatory). */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* non compatible 32b with ::lys_node */
    uint8_t padding[4];              /**< padding for 32b alignment */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    LYS_NODE nodetype;               /**< #LYS_AUGMENT */
    struct lys_node *parent;         /**< uses node or NULL in case of module's top level augment */
    struct lys_node *child;          /**< augmenting data \note The child here points to the data which are also
                                          placed as children in the target node. Children are connected within the
                                          child list of the target, but their parent member still points to the augment
                                          node (this way they can be distinguished from the original target's children).
                                          It is necessary to check this carefully. */

    /* replaces #next and #prev members of ::lys_node */
    struct lys_when *when;           /**< when statement (optional) */
    struct lys_node *target;         /**< pointer to the target node */

    /* again compatible members with ::lys_node */
    void *priv;                      /**< private caller's data, not used by libyang */
};

/**
 * @brief Container for list modifications in ::lys_refine_mod.
 */
struct lys_refine_mod_list {
    uint32_t min;            /**< new min-elements value. Applicable to #LYS_LIST and #LYS_LEAFLIST target nodes */
    uint32_t max;            /**< new max-elements value. Applicable to #LYS_LIST and #LYS_LEAFLIST target nodes */
};

/**
 * @brief Union to hold target modification in ::lys_refine.
 */
union lys_refine_mod {
    const char *presence;        /**< presence description. Applicable to #LYS_CONTAINER target node */
    struct lys_refine_mod_list list;  /**< container for list's attributes,
                                      applicable to #LYS_LIST and #LYS_LEAFLIST target nodes */
};

/**
 * @brief YANG uses's refine substatement structure, see [RFC 6020 sec. 7.12.2](http://tools.ietf.org/html/rfc6020#section-7.12.2)
 */
struct lys_refine {
    const char *target_name;         /**< descendant schema node identifier of the target node to be refined (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only config and mandatory flags apply */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* 32b padding for compatibility with ::lys_node */
    uint16_t target_type;            /**< limitations (get from specified refinements) for target node type:
                                          - 0 = no limitations,
                                          - ORed #LYS_NODE values if there are some limitations */
    uint8_t must_size;               /**< number of elements in the #must array */
    uint8_t dflt_size;               /**< number of elements in the #dflt array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the node's module (mandatory) */

    struct lys_restr *must;          /**< array of additional must restrictions to be added to the target */
    const char **dflt;               /**< array of new default values. Applicable to #LYS_LEAF, #LYS_LEAFLIST and
                                          #LYS_CHOICE target nodes, but multiple defaults are valid only in case of
                                          #LYS_LEAFLIST.*/

    union lys_refine_mod mod;        /**< mutually exclusive target modifications according to the possible target_type */
};


/**
 * @brief Possible deviation modifications, see [RFC 6020 sec. 7.18.3.2](http://tools.ietf.org/html/rfc6020#section-7.18.3.2)
 */
typedef enum lys_deviate_type {
    LY_DEVIATE_NO,                   /**< not-supported */
    LY_DEVIATE_ADD,                  /**< add */
    LY_DEVIATE_RPL,                  /**< replace */
    LY_DEVIATE_DEL                   /**< delete */
} LYS_DEVIATE_TYPE;

/**
 * @brief YANG deviate statement structure, see [RFC 6020 sec. 7.18.3.2](http://tools.ietf.org/html/rfc6020#section-7.18.3.2)
 */
struct lys_deviate {
    LYS_DEVIATE_TYPE mod;            /**< type of deviation modification */

    uint8_t flags;                   /**< Properties: config, mandatory */
    uint8_t dflt_size;               /**< Properties: default - number of elements in the #dflt array */
    uint8_t ext_size;                 /**< number of elements in #ext array */

    uint8_t min_set;                 /**< Since min can be 0, this flag says if it is default value or 0 was set */
    uint8_t max_set;                 /**< Since max can be 0, this flag says if it is default value or 0 (unbounded) was set */
    uint8_t must_size;               /**< Properties: must - number of elements in the #must array */
    uint8_t unique_size;             /**< Properties: unique - number of elements in the #unique array */

    uint32_t min;                    /**< Properties: min-elements */
    uint32_t max;                    /**< Properties: max-elements */

    struct lys_restr *must;          /**< Properties: must - array of must constraints */
    struct lys_unique *unique;       /**< Properties: unique - array of unique statement structures */
    struct lys_type *type;           /**< Properties: type - pointer to type in target, type cannot be deleted or added */
    const char *units;               /**< Properties: units */
    const char **dflt;               /**< Properties: default (both type and choice represented as string value;
                                                      for deviating leaf-list we need it as an array */
    struct lys_ext_instance **ext;    /**< array of pointers to the extension instances */
};

/**
 * @brief YANG deviation statement structure, see [RFC 6020 sec. 7.18.3](http://tools.ietf.org/html/rfc6020#section-7.18.3)
 */
struct lys_deviation {
    const char *target_name;          /**< schema node identifier of the node where the deviation is supposed to be
                                           applied (mandatory). */
    const char *dsc;                  /**< description (optional) */
    const char *ref;                  /**< reference (optional) */
    struct lys_node *orig_node;       /**< original (non-deviated) node (mandatory) */

    uint8_t deviate_size;             /**< number of elements in the #deviate array */
    uint8_t ext_size;                 /**< number of elements in #ext array */
    struct lys_deviate *deviate;      /**< deviate information */
    struct lys_ext_instance **ext;    /**< array of pointers to the extension instances */
};

/**
 * @brief YANG import structure used to reference other schemas (modules).
 */
struct lys_import {
    struct lys_module *module;       /**< link to the imported module (mandatory) */
    const char *prefix;              /**< prefix for the data from the imported schema (mandatory) */
    char rev[LY_REV_SIZE];           /**< revision-date of the imported module (optional) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    const char *dsc;                 /**< description (optional) */
    const char *ref;                 /**< reference (optional) */
};

/**
 * @brief YANG include structure used to reference submodules.
 */
struct lys_include {
    struct lys_submodule *submodule; /**< link to the included submodule (mandatory) */
    char rev[LY_REV_SIZE];           /**< revision-date of the included submodule (optional) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    const char *dsc;                 /**< description (optional) */
    const char *ref;                 /**< reference (optional) */
};

/**
 * @brief YANG revision statement for (sub)modules
 */
struct lys_revision {
    char date[LY_REV_SIZE];          /**< revision-date (mandatory) */
    uint8_t ext_size;                /**< number of elements in #ext array */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    const char *dsc;                 /**< revision's dsc (optional) */
    const char *ref;                 /**< revision's reference (optional) */
};

/**
 * @brief YANG typedef structure providing information from the schema
 */
struct lys_tpdf {
    const char *name;                /**< name of the newly defined type (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ and LYS_DFLTJSON values (or 0) are allowed */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t padding_iffsize;         /**< padding byte for the ::lys_node's iffeature_size */
    uint8_t has_union_leafref;       /**< flag to mark typedefs with a leafref inside a union */

    /* 24b padding for compatibility with ::lys_node */
    uint8_t padding[3];              /**< padding for 32b alignment */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    const char *units;               /**< units of the newly defined type (optional) */
    struct lys_module *module;       /**< pointer to the module where the data type is defined (mandatory),
                                          NULL in case of built-in typedefs */

    struct lys_type type;            /**< base type from which the typedef is derived (mandatory). In case of a special
                                          built-in typedef (from yang_types.c), only the base member is filled */
    const char *dflt;                /**< default value of the newly defined type (optional) */
};

/**
 * @brief YANG list's unique statement structure, see [RFC 6020 sec. 7.8.3](http://tools.ietf.org/html/rfc6020#section-7.8.3)
 */
struct lys_unique {
    const char **expr;               /**< array of unique expressions specifying target leafs to be unique */
    uint8_t expr_size;               /**< size of the #expr array */
    uint8_t trg_type;                /**< config of the targets: 0 - not specified; 1 - config true; 2 - config false */
};

/**
 * @brief YANG feature definition structure
 */
struct lys_feature {
    const char *name;                /**< feature name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values and
                                          #LYS_FENABLED value are allowed */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* 32b padding for compatibility with ::lys_node */
    uint8_t padding[4];              /**< padding for 32b alignment */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< link to the features's data model (mandatory) */
    struct ly_set *depfeatures;      /**< set of other features depending on this one */
};

/**
 * @brief YANG validity restriction (must, length, etc.) structure providing information from the schema
 */
struct lys_restr {
    const char *expr;                /**< The restriction expression/value (mandatory);
                                          in case of pattern restriction, the first byte has a special meaning:
                                          0x06 (ACK) for regular match and 0x15 (NACK) for invert-match */
    const char *dsc;                 /**< description (optional) */
    const char *ref;                 /**< reference (optional) */
    const char *eapptag;             /**< error-app-tag value (optional) */
    const char *emsg;                /**< error-message (optional) */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint16_t flags;                  /**< only flags #LYS_XPCONF_DEP and #LYS_XPSTATE_DEP can be specified */
};

/**
 * @brief YANG when restriction, see [RFC 6020 sec. 7.19.5](http://tools.ietf.org/html/rfc6020#section-7.19.5)
 */
struct lys_when {
    const char *cond;                /**< specified condition (mandatory) */
    const char *dsc;                 /**< description (optional) */
    const char *ref;                 /**< reference (optional) */
    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint16_t flags;                  /**< only flags #LYS_XPCONF_DEP and #LYS_XPSTATE_DEP can be specified */
};

/**
 * @brief Structure to hold information about identity, see  [RFC 6020 sec. 7.16](http://tools.ietf.org/html/rfc6020#section-7.16)
 *
 * First 5 members maps to ::lys_node.
 */
struct lys_ident {
    const char *name;                /**< identity name (mandatory) */
    const char *dsc;                 /**< description statement (optional) */
    const char *ref;                 /**< reference statement (optional) */
    uint16_t flags;                  /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_ values are allowed */
    uint8_t ext_size;                /**< number of elements in #ext array */
    uint8_t iffeature_size;          /**< number of elements in the #iffeature array */

    /* 32b padding for compatibility with ::lys_node */
    uint8_t padding[3];              /**< padding for 32b alignment */
    uint8_t base_size;               /**< number of elements in the #base array */

    struct lys_ext_instance **ext;   /**< array of pointers to the extension instances */
    struct lys_iffeature *iffeature; /**< array of if-feature expressions */
    struct lys_module *module;       /**< pointer to the module where the identity is defined */

    struct lys_ident **base;         /**< array of pointers to the base identities */
    struct ly_set *der;              /**< set of backlinks to the derived identities */
};

/**
 * @brief Load a schema into the specified context.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified
 * format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format);

/**
 * @brief Read a schema from file descriptor into the specified context.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format);

/**
 * @brief Load a schema into the specified context from a file.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the input data (YANG or YIN).
 * @return Pointer to the data model structure or NULL on error.
 */
const struct lys_module *lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format);

/**
 * @brief Search for the schema file in the specified searchpaths.
 *
 * @param[in] searchpaths NULL-terminated array of paths to be searched (recursively). Current working
 * directory is searched automatically (but non-recursively if not in the provided list). Caller can use
 * result of the ly_ctx_get_searchdirs().
 * @param[in] cwd Flag to implicitly search also in the current working directory (non-recursively).
 * @param[in] name Name of the schema to find.
 * @param[in] revision Revision of the schema to find. If NULL, the newest found schema filepath is returned.
 * @param[out] localfile Mandatory output variable containing absolute path of the found schema. If no schema
 * complying the provided restriction is found, NULL is set.
 * @param[out] format Optional output variable containing expected format of the schema document according to the
 * file suffix.
 * @return EXIT_FAILURE on error, EXIT_SUCCESS otherwise (even if the file is not found, then the *localfile is NULL).
 */
int lys_search_localfile(const char * const *searchpaths, int cwd, const char *name, const char *revision, char **localfile, LYS_INFORMAT *format);

/**
 * @brief Get list of all the defined features in the module and its submodules.
 *
 * @param[in] module Module to explore.
 * @param[out] states Optional output parameter providing states of all features
 * returned by function in the resulting array. Indexes in both arrays corresponds
 * each other. Similarly to lys_feature_state(), possible values in the state array
 * are 1 (enabled) and 0 (disabled). Caller is supposed to free the array when it
 * is no more needed.
 * @return NULL-terminated array of all the defined features. The returned array
 * must be freed by the caller, do not free names in the array. Also remember
 * that the names will be freed with freeing the context of the module.
 */
const char **lys_features_list(const struct lys_module *module, uint8_t **states);

/**
 * @brief Enable specified feature in the module. In case its if-feature evaluates
 * to false, return an error.
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be enabled.
 * @param[in] feature Name of the feature to enable. To enable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int lys_features_enable(const struct lys_module *module, const char *feature);

/**
 * @brief Disable specified feature in the module. If it causes some dependant features
 * to be disabled, they are also set to disabled.
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be disabled.
 * @param[in] feature Name of the feature to disable. To disable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int lys_features_disable(const struct lys_module *module, const char *feature);

/**
 * @brief Enable specified feature in the module disregarding its if-features.
 *
 * @param[in] module Module where the feature will be enabled.
 * @param[in] feature Name of the feature to enable. To enable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int lys_features_enable_force(const struct lys_module *module, const char *feature);

/**
 * @brief Disable specified feature in the module disregarding dependant features.
 *
 * By default, when the module is loaded by libyang parser, all features are disabled.
 *
 * @param[in] module Module where the feature will be disabled.
 * @param[in] feature Name of the feature to disable. To disable all features at once, use asterisk character.
 * @return 0 on success, 1 when the feature is not defined in the specified module
 */
int lys_features_disable_force(const struct lys_module *module, const char *feature);

/**
 * @brief Get the current status of the specified feature in the module. Even if the
 * feature is enabled but some of its if-features evaluate to false, it is reported
 * as disabled.
 *
 * @param[in] module Module where the feature is defined.
 * @param[in] feature Name of the feature to inspect.
 * @return
 * - 1 if feature is enabled,
 * - 0 if feature is disabled,
 * - -1 in case of error (e.g. feature is not defined)
 */
int lys_features_state(const struct lys_module *module, const char *feature);

/**
 * @brief Check if the schema node is disabled in the schema tree, i.e. there is any disabled if-feature statement
 * affecting the node.
 *
 * @param[in] node Schema node to check.
 * @param[in] recursive - 0 to check if-feature only in the \p node schema node,
 * - 1 to check if-feature in all ascendant schema nodes
 * - 2 to check if-feature in all ascendant schema nodes until there is a node possibly having an instance in a data tree
 * @return - NULL if enabled,
 * - pointer to the node with the unsatisfied (disabling) if-feature expression.
 */
const struct lys_node *lys_is_disabled(const struct lys_node *node, int recursive);

/**
 * @brief Learn how the if-feature statement currently evaluates.
 *
 * @param[in] iff if-feature statement to evaluate.
 * @return If the statement evaluates to true, 1 is returned. 0 is returned when the statement evaluates to false.
 */
int lys_iffeature_value(const struct lys_iffeature *iff);

/**
 * @brief Check if the schema leaf node is used as a key for a list.
 *
 * @param[in] node Schema leaf node to check
 * @param[out] index Optional parameter to return position in the list's keys array.
 * @return NULL if the \p node is not a key, pointer to the list if the \p node is the key of this list
 */
const struct lys_node_list *lys_is_key(const struct lys_node_leaf *node, uint8_t *index);

/**
 * @brief Get next schema tree (sibling) node element that can be instantiated in a data tree. Returned node can
 * be from an augment.
 *
 * lys_getnext() is supposed to be called sequentially. In the first call, the \p last parameter is usually NULL
 * and function starts returning i) the first \p parent child or ii) the first top level element of the \p module.
 * Consequent calls suppose to provide the previously returned node as the \p last parameter and still the same
 * \p parent and \p module parameters.
 *
 * Without options, the function is used to traverse only the schema nodes that can be paired with corresponding
 * data nodes in a data tree. By setting some \p options the behaviour can be modified to the extent that
 * all the schema nodes are iteratively returned.
 *
 * @param[in] last Previously returned schema tree node, or NULL in case of the first call.
 * @param[in] parent Parent of the subtree where the function starts processing (__cannot be__ #LYS_USES, use its parent).
 * If it is #LYS_AUGMENT, only the children of that augment are returned.
 * @param[in] module In case of iterating on top level elements, the \p parent is NULL and
 * module must be specified (cannot be submodule).
 * @param[in] options ORed options LYS_GETNEXT_*.
 * @return Next schema tree node that can be instanciated in a data tree, NULL in case there is no such element.
 */
const struct lys_node *lys_getnext(const struct lys_node *last, const struct lys_node *parent,
                                   const struct lys_module *module, int options);

#define LYS_GETNEXT_WITHCHOICE   0x01 /**< lys_getnext() option to allow returning #LYS_CHOICE nodes instead of looking into them */
#define LYS_GETNEXT_WITHCASE     0x02 /**< lys_getnext() option to allow returning #LYS_CASE nodes instead of looking into them */
#define LYS_GETNEXT_WITHGROUPING 0x04 /**< lys_getnext() option to allow returning #LYS_GROUPING nodes instead of skipping them */
#define LYS_GETNEXT_WITHINOUT    0x08 /**< lys_getnext() option to allow returning #LYS_INPUT and #LYS_OUTPUT nodes
                                           instead of looking into them */
#define LYS_GETNEXT_WITHUSES     0x10 /**< lys_getnext() option to allow returning #LYS_USES nodes instead of looking into them */
#define LYS_GETNEXT_INTOUSES     0x20 /**< lys_getnext() option to allow to go into uses, takes effect only
                                           with #LYS_GETNEXT_WITHUSES, otherwise it goes into uses automatically */
#define LYS_GETNEXT_INTONPCONT   0x40 /**< lys_getnext() option to look into non-presence container, instead of returning container itself */
#define LYS_GETNEXT_PARENTUSES   0x80 /**< lys_getnext() option to allow parent to be #LYS_USES, in which case only
                                           the direct children are traversed */
#define LYS_GETNEXT_NOSTATECHECK 0x100 /**< lys_getnext() option to skip checking module validity (import-only, disabled) and
                                            relevant if-feature conditions state */

/**
 * @brief Get next type of a union.
 *
 * @param[in] last Last returned type. NULL on first call.
 * @param[in] type Union type structure.
 * @return Next union type in order, NULL if all were returned or on error.
 */
const struct lys_type *lys_getnext_union_type(const struct lys_type *last, const struct lys_type *type);

/**
 * @brief Search for schema nodes matching the provided path.
 *
 * Learn more about the path format at page @ref howtoxpath.
 * Either \p cur_module or \p cur_node must be set.
 *
 * @param[in] cur_module Current module name.
 * @param[in] cur_node Current (context) schema node.
 * @param[in] path Schema path expression filtering the matching nodes.
 * @return Set of found schema nodes. In case of an error, NULL is returned.
 */
struct ly_set *lys_find_path(const struct lys_module *cur_module, const struct lys_node *cur_node, const char *path);

/**
 * @brief Types of context nodes, #LYXP_NODE_ROOT_CONFIG used only in when or must conditions.
 */
enum lyxp_node_type {
    /* XML document roots */
    LYXP_NODE_ROOT,             /* access to all the data (node value first top-level node) */
    LYXP_NODE_ROOT_CONFIG,      /* <running> data context, no state data (node value first top-level node) */

    /* XML elements */
    LYXP_NODE_ELEM,             /* XML element (most common) */
    LYXP_NODE_TEXT,             /* XML text element (extremely specific use, unlikely to be ever needed) */
    LYXP_NODE_ATTR,             /* XML attribute (in YANG cannot happen, do not use for the context node) */

    LYXP_NODE_NONE              /* invalid node type, do not use */
};

/**
 * @brief Get all the partial XPath nodes (atoms) that are required for \p expr to be evaluated.
 *
 * @param[in] ctx_node Context (current) schema node. Fake roots are distinguished using \p ctx_node_type
 * and then this node can be any node from the module (so, for example, do not put node added by an augment from another module).
 * @param[in] ctx_node_type Context (current) schema node type. Most commonly is #LYXP_NODE_ELEM, but if
 * your context node is supposed to be the root, you can specify what kind of root it is.
 * @param[in] expr XPath expression to be evaluated. Must be in JSON data format (prefixes are model names). Otherwise
 * follows full __must__ or __when__ YANG expression syntax (see schema path @ref howtoxpath, but is not limited to that).
 * @param[in] options Whether to apply some evaluation restrictions #LYXP_MUST or #LYXP_WHEN.
 *
 * @return Set of atoms (schema nodes), NULL on error.
 */
struct ly_set *lys_xpath_atomize(const struct lys_node *ctx_node, enum lyxp_node_type ctx_node_type,
                                 const char *expr, int options);

#define LYXP_MUST 0x01 /**< lys_xpath_atomize() option to apply must statement data tree access restrictions */
#define LYXP_WHEN 0x02 /**< lys_xpath_atomize() option to apply when statement data tree access restrictions */

/**
 * @brief Call lys_xpath_atomize() on all the when and must expressions of the node. This node must be
 * a descendant of an input, output, or notification node. This subtree then forms the local subtree.
 *
 * @param[in] node Node to examine.
 * @param[in] options Bitmask of #LYXP_RECURSIVE and #LYXP_NO_LOCAL.
 */
struct ly_set *lys_node_xpath_atomize(const struct lys_node *node, int options);

#define LYXP_RECURSIVE 0x01 /**< lys_node_xpath_atomize() option to return schema node dependencies of all the expressions in the subtree */
#define LYXP_NO_LOCAL 0x02  /**< lys_node_xpath_atomize() option to discard schema node dependencies from the local subtree */

/**
 * @brief Build schema path (usable as path, see @ref howtoxpath) of the schema node.
 *
 * The path includes prefixes of all the nodes and is hence unequivocal in any context.
 * Options can be specified to use a different format of the path.
 *
 * @param[in] node Schema node to be processed.
 * @param[in] options Additional path modification options (#LYS_PATH_FIRST_PREFIX).
 * @return NULL on error, on success the buffer for the resulting path is allocated and caller is supposed to free it
 * with free().
 */
char *lys_path(const struct lys_node *node, int options);

#define LYS_PATH_FIRST_PREFIX 0x01 /**< lys_path() option for the path not to include prefixes of all the nodes,
 * but only for the first one that will be interpreted as the current module (more at @ref howtoxpath). This path is
 * less suitable for further processing but better for displaying as it is shorter. */

/**
 * @brief Build data path (usable as path, see @ref howtoxpath) of the schema node.
 * @param[in] node Schema node to be processed.
 * @return NULL on error, on success the buffer for the resulting path is allocated and caller is supposed to free it
 * with free().
 */
char *lys_data_path(const struct lys_node *node);

/**
 * @brief Build the data path pattern of a schema node.
 *
 * For example, when @p node is a leaf in a list with key1 and key2, in a container and `'%s'` is set as
 * @p placeholder, the result would be `/model:container/list[key1='%s'][key2='%s']/leaf`. That can
 * be used as `printf(3)` format string, for example.
 *
 * @param[in] node Schema node to be processed.
 * @param[in] placeholder Placeholder to insert in the returned path when
 *                        list key values are encountered.
 * @return NULL on error, on success the buffer for the resulting path is
 *         allocated and caller is supposed to free it with free().
 */
char *lys_data_path_pattern(const struct lys_node *node, const char *placeholder);

/**
 * @brief Return parent node in the schema tree.
 *
 * In case of augmenting node, it returns the target tree node where the augmenting
 * node was placed, not the augment definition node. Function just wraps usage of the
 * ::lys_node#parent pointer in this special case.
 *
 * @param[in] node Child node to the returned parent node.
 * @return The parent node from the schema tree, NULL in case of top level nodes.
 */
struct lys_node *lys_parent(const struct lys_node *node);

/**
 * @brief Return main module of the schema tree node.
 *
 * In case of regular YANG module, it returns ::lys_node#module pointer,
 * but in case of submodule, it returns pointer to the main module.
 *
 * @param[in] node Schema tree node to be examined
 * @return pointer to the main module (schema structure), NULL in case of error.
 */
struct lys_module *lys_node_module(const struct lys_node *node);

/**
 * @brief Return main module of the module.
 *
 * In case of regular YANG module, it returns itself,
 * but in case of submodule, it returns pointer to the main module.
 *
 * @param[in] module Module to be examined
 * @return pointer to the main module (schema structure).
 */
struct lys_module *lys_main_module(const struct lys_module *module);

/**
 * @brief Find the implemented revision of the given module in the context.
 *
 * If there is no revision of the module implemented, the given module is returned
 * without any change. It is up to the caller to set the module implemented via
 * lys_set_implemented() when needed.
 *
 * Also note that the result can be a disabled module and the caller is supposed to decide
 * if it should by enabled via lys_set_enabled(). This is to avoid to try to set another
 * revision of the module implemented that would fail due to the disabled, but the implemented
 * module.
 *
 * @param[in] mod Module to be searched.
 * @return The implemented revision of the module if any, the given module otherwise.
 */
struct lys_module *lys_implemented_module(const struct lys_module *mod);

/**
 * @brief Mark imported module as "implemented".
 *
 * All the modules explicitly loaded are marked as "implemented", but in case of loading module
 * automatically as an import of another module, it is marked as imported and in that case it
 * is not allowed to load data of this module. On the other hand, the mandatory data nodes of
 * such a module are not required nor the (top-level) default nodes defined in this module are
 * created in the data trees.
 *
 * When a module is marked as "implemented" it is not allowed to set it back to "imported".
 *
 * Note that it is not possible to mark "implemented" multiple revisions of a same module within
 * a single context. In such a case the function fails.
 *
 * If the module is currently disabled, this function enables the module implicitly.
 *
 * @param[in] module The module to be set implemented.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int lys_set_implemented(const struct lys_module *module);

/**
 * @brief Disable module in its context to avoid its further usage (it will be hidden for module getters).
 *
 * The function also disables all the modules in the context that depends on the provided module to disable.
 * If the imported modules are not used by any other module in the context, they are also disabled. The result
 * of this function can be reverted by lys_set_enabled() function.
 *
 * Since the disabled modules are hidden from the common module getters, there is a special
 * ly_ctx_get_disabled_module_iter() to go through the disabled modules in the context.
 *
 * libyang internal modules (those present when the context is created) cannot be disabled. Any module
 * loaded into the context is, by default, enabled.
 *
 * @param[in] module Module to be enabled.
 * @return EXIT_SUCCESS or EXIT_FAILURE (in case of invalid parameter).
 */
int lys_set_disabled(const struct lys_module *module);

/**
 * @brief Enable previously disabled module.
 *
 * The function tries to revert previous call of the lys_set_disabled() so it checks other disabled
 * modules in the context depending on the specified module and if it is possible, also the other modules
 * are going to be enabled. Similarly, all the imported modules that were previously supposed as useless
 * are enabled.
 *
 * libyang internal modules (those present when the context is created) are always enabled. Any other module
 * loaded into the context is, by default, enabled.
 *
 * @param[in] module Module to be enabled.
 * @return EXIT_SUCCESS or EXIT_FAILURE (in case of invalid parameter).
 */
int lys_set_enabled(const struct lys_module *module);

/**
 * @brief Set a schema private pointer to a user pointer.
 *
 * @param[in] node Node, whose private field will be assigned.
 * @param[in] priv Arbitrary user-specified pointer.
 * @return previous private object of the \p node (NULL if this is the first call on the \p node). Note, that
 * the caller is in this case responsible (if it is necessary) for freeing the replaced private object. In case
 * of invalid (NULL) \p node, NULL is returned and #ly_errno is set to #LY_EINVAL.
 */
void *lys_set_private(const struct lys_node *node, void *priv);

/**
 * @brief Print schema tree in the specified format into a memory block.
 * It is up to caller to free the returned string by free().
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter. It specifies which particular node/subtree in the module will be printed.
 * Only for #LYS_OUT_INFO and #LYS_OUT_TREE formats. Use fully qualified schema path (@ref howtoxpath).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
                  int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file descriptor.
 *
 * @param[in] module Schema tree to print.
 * @param[in] fd File descriptor where to print the data.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter. It specifies which particular node/subtree in the module will be printed.
 * Only for #LYS_OUT_INFO and #LYS_OUT_TREE formats. Use fully qualified schema path (@ref howtoxpath).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
                 int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file stream.
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter. It specifies which particular node/subtree in the module will be printed.
 * Only for #LYS_OUT_INFO and #LYS_OUT_TREE formats. Use fully qualified schema path (@ref howtoxpath).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
                   int line_length, int options);

/**
 * @brief Print schema tree in the specified format into a file.
 *
 * @param[in] path File where to print the schema.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter. It specifies which particular node/subtree in the module will be printed.
 * Only for #LYS_OUT_INFO and #LYS_OUT_TREE formats. Use fully qualified schema path (@ref howtoxpath).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node,
                   int line_length, int options);

/**
 * @brief Print schema tree in the specified format using a provided callback.
 *
 * @param[in] module Schema tree to print.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Schema output format.
 * @param[in] target_node Optional parameter. It specifies which particular node/subtree in the module will be printed.
 * Only for #LYS_OUT_INFO and #LYS_OUT_TREE formats. Use fully qualified schema path (@ref howtoxpath).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return 0 on success, 1 on failure (#ly_errno is set).
 */
int lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                  const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node, int line_length, int options);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_SCHEMA_H_ */
