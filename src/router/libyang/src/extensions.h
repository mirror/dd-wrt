/**
 * @file extensions.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extension implementations.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_EXTENSIONS_H_
#define LY_EXTENSIONS_H_

#include "libyang.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup extensions
 * @{
 */

/**
 * @brief Extensions API version
 */
#define LYEXT_API_VERSION 1

/**
 * @brief Macro to store version of extension plugins API in the plugins.
 * It is matched when the plugin is being loaded by libyang.
 */
#ifdef STATIC
#define LYEXT_VERSION_CHECK
#else
#define LYEXT_VERSION_CHECK int lyext_api_version = LYEXT_API_VERSION;
#endif

/**
 * @brief Extension instance structure parent enumeration
 */
typedef enum {
    LYEXT_PAR_MODULE,              /**< ::lys_module or ::lys_submodule */
    LYEXT_PAR_NODE,                /**< ::lys_node (and the derived structures) */
    LYEXT_PAR_TPDF,                /**< ::lys_tpdf */
    LYEXT_PAR_TYPE,                /**< ::lys_type */
    LYEXT_PAR_TYPE_BIT,            /**< ::lys_type_bit */
    LYEXT_PAR_TYPE_ENUM,           /**< ::lys_type_enum */
    LYEXT_PAR_FEATURE,             /**< ::lys_feature */
    LYEXT_PAR_RESTR,               /**< ::lys_restr - YANG's must, range, length and pattern statements */
    LYEXT_PAR_WHEN,                /**< ::lys_when */
    LYEXT_PAR_IDENT,               /**< ::lys_ident */
    LYEXT_PAR_EXT,                 /**< ::lys_ext */
    LYEXT_PAR_EXTINST,             /**< ::lys_ext_instance */
    LYEXT_PAR_REFINE,              /**< ::lys_refine */
    LYEXT_PAR_DEVIATION,           /**< ::lys_deviation */
    LYEXT_PAR_DEVIATE,             /**< ::lys_deviate */
    LYEXT_PAR_IMPORT,              /**< ::lys_import */
    LYEXT_PAR_INCLUDE,             /**< ::lys_include */
    LYEXT_PAR_REVISION,            /**< ::lys_revision */
    LYEXT_PAR_IFFEATURE            /**< ::lys_iffeature */
} LYEXT_PAR;

/**
 * @brief List of substatement without extensions storage. If the module contains extension instances in these
 * substatements, they are stored with the extensions of the parent statement and flag to show to which substatement
 * they belongs to.
 *
 * For example, if the extension is supposed to be instantiated as a child to the description statement, libyang
 * stores the description just as its value. So, for example in case of the module's description, the description's
 * extension instance is actually stored in the lys_module's extensions list with the ::lys_ext_instance#insubstmt set to
 * #LYEXT_SUBSTMT_DESCRIPTION, ::lys_ext_instance#parent_type is LYEXT_PAR_MODULE and the ::lys_ext_instance#parent
 * points to the ::lys_module structure.
 *
 * The values are (convertible) subset of #LY_STMT
 */
typedef enum {
    LYEXT_SUBSTMT_ALL = -1,      /**< special value for the lys_ext_iter() */
    LYEXT_SUBSTMT_SELF = 0,      /**< extension of the structure itself, not substatement's */
    LYEXT_SUBSTMT_ARGUMENT,      /**< extension of the argument statement, can appear in lys_ext */
    LYEXT_SUBSTMT_BASE,          /**< extension of the base statement, can appear (repeatedly) in lys_type and lys_ident */
    LYEXT_SUBSTMT_BELONGSTO,     /**< extension of the belongs-to statement, can appear in lys_submodule */
    LYEXT_SUBSTMT_CONTACT,       /**< extension of the contact statement, can appear in lys_module */
    LYEXT_SUBSTMT_DEFAULT,       /**< extension of the default statement, can appear in lys_node_leaf, lys_node_leaflist,
                                      lys_node_choice and lys_deviate */
    LYEXT_SUBSTMT_DESCRIPTION,   /**< extension of the description statement, can appear in lys_module, lys_submodule,
                                      lys_node, lys_import, lys_include, lys_ext, lys_feature, lys_tpdf, lys_restr,
                                      lys_ident, lys_deviation, lys_type_enum, lys_type_bit, lys_when and lys_revision */
    LYEXT_SUBSTMT_ERRTAG,        /**< extension of the error-app-tag statement, can appear in lys_restr */
    LYEXT_SUBSTMT_ERRMSG,        /**< extension of the error-message statement, can appear in lys_restr */
    LYEXT_SUBSTMT_KEY,           /**< extension of the key statement, can appear in lys_node_list */
    LYEXT_SUBSTMT_NAMESPACE,     /**< extension of the namespace statement, can appear in lys_module */
    LYEXT_SUBSTMT_ORGANIZATION,  /**< extension of the organization statement, can appear in lys_module and lys_submodule */
    LYEXT_SUBSTMT_PATH,          /**< extension of the path statement, can appear in lys_type */
    LYEXT_SUBSTMT_PREFIX,        /**< extension of the prefix statement, can appear in lys_module, lys_submodule (for
                                      belongs-to's prefix) and lys_import */
    LYEXT_SUBSTMT_PRESENCE,      /**< extension of the presence statement, can appear in lys_node_container */
    LYEXT_SUBSTMT_REFERENCE,     /**< extension of the reference statement, can appear in lys_module, lys_submodule,
                                      lys_node, lys_import, lys_include, lys_revision, lys_tpdf, lys_restr, lys_ident,
                                      lys_ext, lys_feature, lys_deviation, lys_type_enum, lys_type_bit and lys_when */
    LYEXT_SUBSTMT_REVISIONDATE,  /**< extension of the revision-date statement, can appear in lys_import and lys_include */
    LYEXT_SUBSTMT_UNITS,         /**< extension of the units statement, can appear in lys_tpdf, lys_node_leaf,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_VALUE,         /**< extension of the value statement, can appear in lys_type_enum */
    LYEXT_SUBSTMT_VERSION,       /**< extension of the yang-version statement, can appear in lys_module and lys_submodule */
    LYEXT_SUBSTMT_MODIFIER,      /**< extension of the modifier statement, can appear in lys_restr */
    LYEXT_SUBSTMT_REQINSTANCE,   /**< extension of the require-instance statement, can appear in lys_type */
    LYEXT_SUBSTMT_YINELEM,       /**< extension of the yin-element statement, can appear in lys_ext */
    LYEXT_SUBSTMT_CONFIG,        /**< extension of the config statement, can appear in lys_node and lys_deviate */
    LYEXT_SUBSTMT_MANDATORY,     /**< extension of the mandatory statement, can appear in lys_node_leaf, lys_node_choice,
                                      lys_node_anydata and lys_deviate */
    LYEXT_SUBSTMT_ORDEREDBY,     /**< extension of the ordered-by statement, can appear in lys_node_list and lys_node_leaflist */
    LYEXT_SUBSTMT_STATUS,        /**< extension of the status statement, can appear in lys_tpdf, lys_node, lys_ident,
                                      lys_ext, lys_feature, lys_type_enum and lys_type_bit */
    LYEXT_SUBSTMT_DIGITS,        /**< extension of the fraction-digits statement, can appear in lys_type */
    LYEXT_SUBSTMT_MAX,           /**< extension of the max-elements statement, can appear in lys_node_list,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_MIN,           /**< extension of the min-elements statement, can appear in lys_node_list,
                                      lys_node_leaflist and lys_deviate */
    LYEXT_SUBSTMT_POSITION,      /**< extension of the position statement, can appear in lys_type_bit */
    LYEXT_SUBSTMT_UNIQUE,        /**< extension of the unique statement, can appear in lys_node_list and lys_deviate */
} LYEXT_SUBSTMT;

/**
 * @brief Callback to check that the extension can be instantiated inside the provided node
 *
 * @param[in] parent The parent of the instantiated extension.
 * @param[in] parent_type The type of the structure provided as \p parent.
 * @param[in] substmt_type libyang does not store all the extension instances in the structures where they are
 *                         instantiated in the module. In some cases (see #LYEXT_SUBSTMT) they are stored in parent
 *                         structure and marked with flag to know in which substatement of the parent the extension
 *                         was originally instantiated.
 * @return 0 - yes
 *         1 - no
 *         2 - ignore / skip without an error
 */
typedef int (*lyext_check_position_clb)(const void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT substmt_type);

/**
 * @brief Callback to check that the extension instance is correct - have
 * the valid argument, cardinality, etc.
 *
 * @param[in] ext Extension instance to be checked.
 * @return 0 - ok
 *         1 - error
 */
typedef int (*lyext_check_result_clb)(struct lys_ext_instance *ext);

/**
 * @brief Callback to decide whether the extension will be inherited into the provided schema node. The extension
 * instance is always from some of the node's parents. The inherited extension instances are marked with the
 * #LYEXT_OPT_INHERIT flag.
 *
 * @param[in] ext Extension instance to be inherited.
 * @param[in] node Schema node where the node is supposed to be inherited.
 * @return 0 - yes
 *         1 - no (do not process the node's children)
 *         2 - no, but continue with children
 */
typedef int (*lyext_check_inherit_clb)(struct lys_ext_instance *ext, struct lys_node *node);

/**
 * @brief Callback to decide if data is valid towards to schema.
 *
 * @param[in] ext Extension instance to be checked.
 * @param[in] node Data node, which try to valid.
 *
 * @return 0 - valid
 *         1 - invalid
 */
typedef int (*lyext_valid_data_clb)(struct lys_ext_instance *ext, struct lyd_node *node);

struct lyext_plugin {
    LYEXT_TYPE type;                          /**< type of the extension, according to it the structure will be casted */
    uint16_t flags;                           /**< [extension flags](@ref extflags) */

    lyext_check_position_clb check_position;  /**< callbcak for testing that the extension can be instantiated
                                                   under the provided parent. Mandatory callback. */
    lyext_check_result_clb check_result;      /**< callback for testing if the argument value of the extension instance
                                                   is valid. Mandatory if the extension has the argument. */
    lyext_check_inherit_clb check_inherit;    /**< callback to decide if the extension is supposed to be inherited into
                                                   the provided node, the callback is used only if the flags contains
                                                   #LYEXT_OPT_INHERIT flag */
    lyext_valid_data_clb valid_data;          /**< callback to valid if data is valid toward to schema */
};

struct lyext_plugin_complex {
    LYEXT_TYPE type;                          /**< type of the extension, according to it the structure will be casted */
    uint16_t flags;                           /**< [extension flags](@ref extflags) */

    lyext_check_position_clb check_position;  /**< callbcak for testing that the extension can be instantiated
                                                   under the provided parent. Mandatory callback. */
    lyext_check_result_clb check_result;      /**< callback for testing if the argument value of the extension instance
                                                   is valid. Mandatory if the extension has the argument. */
    lyext_check_inherit_clb check_inherit;    /**< callback to decide if the extension is supposed to be inherited into
                                                   the provided node, the callback is used only if the flags contains
                                                   #LYEXT_OPT_INHERIT flag */
    lyext_valid_data_clb valid_data;          /**< callback to valid if data is valid toward to schema */
    struct lyext_substmt *substmt;            /**< NULL-terminated array of allowed substatements and restrictions
                                                   to their instantiation inside the extension instance */
    size_t instance_size;                     /**< size of the instance structure to allocate, the structure is
                                                   is provided as ::lys_ext_instance_complex, but the content array
                                                   is accessed according to the substmt specification provided by
                                                   plugin */
};

struct lyext_plugin_list {
    const char *module;          /**< name of the module where the extension is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optional approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin extension. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the extension */
    struct lyext_plugin *plugin; /**< plugin for the extension */
};

/**
 * @brief Logging function for extension plugins, use #LYEXT_LOG macro instead!
 */
void lyext_log(const struct ly_ctx *ctx, LY_LOG_LEVEL level, const char *plugin, const char *function, const char *format, ...);

/**
 * @brief Logging macro for extension plugins
 *
 * @param[in] ctx Context to store the error in.
 * @param[in] level #LY_LOG_LEVEL value with the message importance.
 * @param[in] plugin Plugin name.
 * @param[in] str Format string as in case of printf function.
 * @param[in] args Parameters to expand in format string.
 */
#define LYEXT_LOG(ctx, level, plugin, str, args...)       \
    lyext_log(ctx, level, plugin, __func__, str, ##args); \

/**
 * @brief Type of object concerned by a validation error.
 * This is used to determine how to compute the path of the element at issue.
 */
typedef enum {
    LYEXT_VLOG_NONE = 0,
    LYEXT_VLOG_XML, /**< const struct ::lyxml_elem* */
    LYEXT_VLOG_LYS, /**< const struct ::lys_node* */
    LYEXT_VLOG_LYD, /**< const struct ::lyd_node* */
    LYEXT_VLOG_STR, /**< const char* */
    LYEXT_VLOG_PREV, /**< Use the same path as the previous validation error */
} LYEXT_VLOG_ELEM;

/**
 * @brief Validation logging function for extension plugins, use #LYEXT_VLOG macro instead!
 */
void lyext_vlog(const struct ly_ctx *ctx, LY_VECODE vecode, const char *plugin, const char *function,
                LYEXT_VLOG_ELEM elem_type, const void *elem, const char *format, ...);

/**
 * @brief Validation logging macro for extension plugins
 *
 * @param[in] ctx Context to store the error in.
 * @param[in] vecode #LY_VECODE validation error code.
 * @param[in] plugin Plugin name.
 * @param[in] elem_type #LYEXT_VLOG_ELEM what to expect in \p elem.
 * @param[in] elem The element at issue.
 * @param[in] str Format string as in case of printf function.
 * @param[in] args Parameters to expand in format string.
 */
#define LYEXT_VLOG(ctx, vecode, plugin, elem_type, elem, str, args...)    \
    lyext_vlog(ctx, vecode, plugin, __func__, elem_type, elem, str, ##args)

/**
 * @brief Free iffeature structure. In API only for plugins that want to handle if-feature statements similarly
 * to libyang.
 *
 * @param[in] ctx libyang context.
 * @param[in] iffeature iffeature array to free.
 * @param[in] iffeature_size size of array \p iffeature.
 * @param[in] shallow Whether to make only shallow free.
 * @param[in] private_destructor Custom destructor for freeing any extension instances.
 */
void lys_iffeature_free(struct ly_ctx *ctx, struct lys_iffeature *iffeature, uint8_t iffeature_size, int shallow,
                        void (*private_destructor)(const struct lys_node *node, void *priv));

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LY_EXTENSIONS_H_ */
