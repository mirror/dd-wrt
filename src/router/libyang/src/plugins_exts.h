/**
 * @file plugins_exts.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang support for YANG extensions implementation.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_H_
#define LY_PLUGINS_EXTS_H_

#include "log.h"
#include "parser_data.h"
#include "plugins.h"
#include "tree_data.h"
#include "tree_edit.h"
#include "tree_schema.h"

struct ly_ctx;
struct ly_in;
struct lyd_node;
struct lysc_ctx;
struct lysc_ext_substmt;
struct lysp_ctx;
struct lyspr_ctx;
struct lyspr_tree_ctx;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howtoPluginsExtensions Extension Plugins
 *
 * Note that the part of the libyang API here is available only by including a separated `<libyang/plugins_exts.h>` header
 * file. Also note that the extension plugins API is versioned separately from libyang itself, so backward incompatible
 * changes can come even without changing libyang major version.
 *
 * YANG extensions are very complex. Usually only its description specifies how it is supposed to behave, what are the
 * allowed substatements, their cardinality or if the standard YANG statements placed inside the extension differs somehow
 * in their meaning or behavior. libyang provides the Extension plugins API to implement such extensions and add its support
 * into libyang itself. However we tried our best, the API is not (and it cannot be) so universal and complete to cover all
 * possibilities. There are definitely use cases which cannot be simply implemented only with this API.
 *
 * libyang implements 3 important extensions: [NACM](https://tools.ietf.org/html/rfc8341), [Metadata](@ref howtoDataMetadata)
 * and [yang-data](@ref howtoDataYangdata). Despite the core implementation in all three cases is done via extension plugin
 * API, also other parts of the libyang code had to be extended to cover complete scope of the extensions.
 *
 * We believe, that the API is capable to allow implementation of very wide range of YANG extensions. However, if you see
 * limitations for the particular YANG extension, don't hesitate to contact the project developers to discuss all the
 * options, including updating the API.
 *
 * The plugin's functionality is provided to libyang via a set of callbacks specified as an array of ::lyplg_ext_record
 * structures using the ::LYPLG_EXTENSIONS macro.
 *
 * The most important callbacks are ::lyplg_ext.parse and ::lyplg_ext.compile. They are responsible for parsing and
 * compiling extension instance. This should include validating all the substatements, their values, or placement of
 * the extension instance itself. If needed, the processed data can be stored in some form into the compiled schema
 * representation of the extension instance. To make this task as easy as possible, libyang provides several
 * [parsing](@ref pluginsExtensionsParse) and [compilation](@ref pluginsExtensionsCompile) helper functions to process
 * known YANG statements exactly as if they were standard YANG statements.
 *
 * The data validation callback ::lyplg_ext.validate is used for additional validation of a data nodes that contains the
 * connected extension instance directly (as a substatement) or indirectly in case of terminal nodes via their type (no
 * matter if the extension instance is placed directly in the leaf's/leaf-list's type or in the type of the referenced
 * typedef).
 *
 * The ::lyplg_ext.printer_info callback implement printing the compiled extension instance data when the schema (module) is
 * being printed in the ::LYS_OUT_YANG_COMPILED (info) format. As for compile callback, there are also
 * [helper functions](@ref pluginsExtensionsSprinterInfo) to access printer's context and to print standard YANG statements
 * placed in the extension instance by libyang itself.
 *
 * The ::lyplg_ext.printer_ctree and ::lyplg_ext.printer_ptree callbacks implement printing of YANG tree diagrams
 * (RFC 8340) for extension instance data. These callbacks are called for extension instances that have
 * parents of type ::LY_STMT_MODULE, ::LY_STMT_SUBMODULE. Or these callbacks are called if the printer_tree finds
 * a compiled/parsed data-node containing an extension instance. The callbacks should then decide which nodes
 * should be printed within the extension instance. In addition, it is possible to register additional callbacks
 * to the printer_tree context to override the form of the each node in the extension instance.
 *
 * The last callback, ::lyplg_ext.cfree, is supposed to free all the data allocated by the ::lyplg_ext.compile callback.
 * To free the data created by helper function ::lyplg_ext_compile_extension_instance(), the plugin can used
 * ::lyplg_ext_cfree_instance_substatements().
 *
 * The plugin information contains also the plugin identifier (::lyplg_type.id). This string can serve to identify the
 * specific plugin responsible to storing data value. In case the user can recognize the id string, it can access the
 * plugin specific data with the appropriate knowledge of its structure.
 *
 * Logging information from an extension plugin is possible via ::lyplg_ext_parse_log() and ::lyplg_ext_compile_log() functions.
 */

/**
 * @defgroup pluginsExtensions Plugins: Extensions
 *
 * Structures and functions to for libyang plugins implementing specific YANG extensions defined in YANG modules. For more
 * information, see @ref howtoPluginsTypes.
 *
 * This part of libyang API is available by including `<libyang/plugins_ext.h>` header file.
 *
 * @{
 */

/**
 * @brief Extensions API version
 */
#define LYPLG_EXT_API_VERSION 6

/**
 * @brief Mask for an operation statement.
 *
 * This mask matches action and RPC.
 */
#define LY_STMT_OP_MASK (LY_STMT_ACTION | LY_STMT_RPC)

/**
 * @brief Mask for a data node statement.
 *
 * This mask matches anydata, anyxml, case, choice, container, leaf, leaf-list, and list.
 */
#define LY_STMT_DATA_NODE_MASK (LY_STMT_ANYDATA | LY_STMT_ANYXML | LY_STMT_CASE | LY_STMT_CHOICE | LY_STMT_CONTAINER |\
        LY_STMT_LEAF | LY_STMT_LEAF_LIST | LY_STMT_LIST)

/**
 * @brief Mask for a node statement.
 *
 * This mask matches notification, input, output, action, RPC, anydata, anyxml, augment, case, choice, container,
 * grouping, leaf, leaf-list, list, and uses.
 */
#define LY_STMT_NODE_MASK 0xFFFF

/**
 * @brief List of YANG statements
 *
 * Their description mentions what types are stored for each statement. Note that extension instance storage
 * always stores a pointer to the type, not the type itself.
 */
enum ly_stmt {
    LY_STMT_NONE = 0,

    LY_STMT_NOTIFICATION = 0x0001,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_notif *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_notif *` */
    LY_STMT_INPUT = 0x0002,     /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_action_inout *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_action_inout *` */
    LY_STMT_OUTPUT = 0x0004,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_action_inout *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_action_inout *` */
    LY_STMT_ACTION = 0x0008,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_action *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_action *` */
    LY_STMT_RPC = 0x0010,       /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_action *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_action *` */
    LY_STMT_ANYDATA = 0x0020,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_anydata *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_anydata *` */
    LY_STMT_ANYXML = 0x0040,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_anydata *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_anydata *` */
    LY_STMT_AUGMENT = 0x0080,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_augment *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_CASE = 0x0100,      /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_case *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_case *` */
    LY_STMT_CHOICE = 0x0200,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_choice *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_choice *` */
    LY_STMT_CONTAINER = 0x0400, /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_container *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_container *` */
    LY_STMT_GROUPING = 0x0800,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_grp *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_LEAF = 0x1000,      /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_leaf *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_leaf *` */
    LY_STMT_LEAF_LIST = 0x2000, /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_leaflist *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_leaflist *` */
    LY_STMT_LIST = 0x4000,      /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_list *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node_list *` */
    LY_STMT_USES = 0x8000,      /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_node_uses *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_node *` */

    LY_STMT_ARGUMENT = 0x10000, /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_ext *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_ext *` */
    LY_STMT_BASE = 0x20000,     /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char **`[]
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_ident *` */
    LY_STMT_BELONGS_TO = 0x30000,   /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_submodule *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_BIT = 0x40000,      /**< ::lysp_ext_substmt.storage - `struct lysp_type_enum *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_type_enum *`
                                     ::lysc_ext_substmt.storage - `struct lysc_type_bitenum_item *`[]
                                     ::lysc_ext_instance.parent - `struct lysc_type_bitenum_item *` */
    LY_STMT_CONFIG = 0x50000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `uint16_t *`
                                     ::lysc_ext_substmt.storage - `uint16_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_CONTACT = 0x60000,  /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_(sub)module *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_DEFAULT = 0x70000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_qname *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_node *`, `struct lysc_type *` (typedef) */
    LY_STMT_DESCRIPTION = 0x80000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - compiled parent statement */
    LY_STMT_DEVIATE = 0x90000,  /**< ::lysp_ext_substmt.storage - `struct lysp_deviate *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_deviate *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - not compiled */
    LY_STMT_DEVIATION = 0xA0000,    /**< ::lysp_ext_substmt.storage - `struct lysp_deviation *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_deviation *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_ENUM = 0xB0000,     /**< ::lysp_ext_substmt.storage - `struct lysp_type_enum *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_type_enum *`
                                     ::lysc_ext_substmt.storage - `struct lysc_type_bitenum_item *`[]
                                     ::lysc_ext_instance.parent - `struct lysc_type_bitenum_item *` */
    LY_STMT_ERROR_APP_TAG = 0xC0000,    /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - compiled restriction structure */
    LY_STMT_ERROR_MESSAGE = 0xD0000,    /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - compiled restriction structure */
    LY_STMT_EXTENSION = 0xE0000,    /**< ::lysp_ext_substmt.storage - `struct lysp_ext *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_ext *`
                                     ::lysc_ext_substmt.storage - not compiled explicitly
                                     ::lysc_ext_instance.parent - `struct lysc_ext *` */
    LY_STMT_EXTENSION_INSTANCE = 0xF0000,   /**< ::lysp_ext_substmt.storage - `struct lysp_ext_instance *`[]
                                     ::lysc_ext_substmt.storage - `struct lysc_ext_instance *`[] */
    LY_STMT_FEATURE = 0x100000, /**< ::lysp_ext_substmt.storage - `struct lysp_feature *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_feature *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - not compiled */
    LY_STMT_FRACTION_DIGITS = 0x110000, /**< ::lysp_ext_substmt.storage - `uint8_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_type *`
                                     ::lysc_ext_substmt.storage - `uint8_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_type *` */
    LY_STMT_IDENTITY = 0x120000,    /**< ::lysp_ext_substmt.storage - `struct lysp_ident *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_ident *`
                                     ::lysc_ext_substmt.storage - `struct lysc_ident *`[]
                                     ::lysc_ext_instance.parent - `struct lysc_ident *` */
    LY_STMT_IF_FEATURE = 0x130000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_qname *`[]
                                     ::lysc_ext_substmt.storage - no storage, evaluated when compiled
                                     ::lysc_ext_instance.parent - compiled parent statement */
    LY_STMT_IMPORT = 0x140000,  /**< ::lysp_ext_substmt.storage - `struct lysp_import *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_import *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - not compiled */
    LY_STMT_INCLUDE = 0x150000, /**< ::lysp_ext_substmt.storage - `struct lysp_include *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_include *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - not compiled */
    LY_STMT_KEY = 0x160000,     /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_node_list *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_node_list *` */
    LY_STMT_LENGTH = 0x170000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_range *` */
    LY_STMT_MANDATORY = 0x180000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `uint16_t *`
                                     ::lysc_ext_substmt.storage - `uint16_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_MAX_ELEMENTS = 0x190000,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `uint32_t *`
                                     ::lysc_ext_substmt.storage - `uint32_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_node_list *` */
    LY_STMT_MIN_ELEMENTS = 0x1A0000,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `uint32_t *`
                                     ::lysc_ext_substmt.storage - `uint32_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_node_list *` */
    LY_STMT_MODIFIER = 0x1B0000,    /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_pattern *` */
    LY_STMT_MODULE = 0x1C0000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_module *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_MUST = 0x1D0000,    /**< ::lysp_ext_substmt.storage - `struct lysp_restr *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage - `struct lysc_must *`[]
                                     ::lysc_ext_instance.parent - `struct lysc_must *` */
    LY_STMT_NAMESPACE = 0x1E0000,   /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_module *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_ORDERED_BY = 0x1F0000,  /**< ::lysp_ext_substmt.storage - `uint16_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_node *`
                                     ::lysc_ext_substmt.storage - `uint16_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_ORGANIZATION = 0x200000,    /**< ::lysp_ext_substmt.storage - `const char *`
                                     ::lysp_ext_instance.parent - `struct lysp_(sub)module *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_PATH = 0x210000,    /**< ::lysp_ext_substmt.storage - `struct lyxp_expr *`
                                     ::lysp_ext_instance.parent - `struct lysp_type *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_type *` */
    LY_STMT_PATTERN = 0x220000, /**< ::lysp_ext_substmt.storage - `struct lysp_restr *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage - `struct lysc_pattern **`[]
                                     ::lysc_ext_instance.parent - `struct lysc_pattern *` */
    LY_STMT_POSITION = 0x230000,    /**< ::lysp_ext_substmt.storage - `int64_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_type_enum *`
                                     ::lysc_ext_substmt.storage - `int64_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_type_bitenum_item *` */
    LY_STMT_PREFIX = 0x240000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_PRESENCE = 0x250000,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_node_container *` */
    LY_STMT_RANGE = 0x260000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_restr *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_range *` */
    LY_STMT_REFERENCE = 0x270000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - compiled parent statement */
    LY_STMT_REFINE = 0x280000,  /**< ::lysp_ext_substmt.storage - `struct lysp_refine *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_refine *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_node *` */
    LY_STMT_REQUIRE_INSTANCE = 0x290000,    /**< ::lysp_ext_substmt.storage - `uint8_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_type *`
                                     ::lysc_ext_substmt.storage - `uint8_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_type *` */
    LY_STMT_REVISION = 0x2A0000,    /**< ::lysp_ext_substmt.storage - `struct lysp_revision *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_revision *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - not compiled */
    LY_STMT_REVISION_DATE = 0x2B0000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - not compiled */
    LY_STMT_STATUS = 0x2C0000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `uint16_t *`
                                     ::lysc_ext_substmt.storage - `uint16_t *`
                                     ::lysc_ext_instance.parent - compiled parent statement */
    LY_STMT_SUBMODULE = 0x2D0000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_submodule *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_TYPE = 0x2E0000,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_type *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_type *` */
    LY_STMT_TYPEDEF = 0x2F0000, /**< ::lysp_ext_substmt.storage - `struct lysp_tpdf *`[]
                                     ::lysp_ext_instance.parent - `struct lysp_tpdf *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_type *` */
    LY_STMT_UNIQUE = 0x300000,  /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_qname *`[]
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_node_list *` */
    LY_STMT_UNITS = 0x310000,   /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `const char *`
                                     ::lysc_ext_substmt.storage - `const char *`
                                     ::lysc_ext_instance.parent - `struct lysc_node *`, `struct lysc_type *` (typedef) */
    LY_STMT_VALUE = 0x320000,   /**< ::lysp_ext_substmt.storage - `int64_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_type_enum *`
                                     ::lysc_ext_substmt.storage - `int64_t *`
                                     ::lysc_ext_instance.parent - `struct lysc_type_bitenum_item *` */
    LY_STMT_WHEN = 0x330000,    /**< ::lysp_ext_substmt.storage and ::lysp_ext_instance.parent - `struct lysp_when *`
                                     ::lysc_ext_substmt.storage and ::lysc_ext_instance.parent - `struct lysc_when *` */
    LY_STMT_YANG_VERSION = 0x340000,    /**< ::lysp_ext_substmt.storage - `uint8_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_(sub)module *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_module *` */
    LY_STMT_YIN_ELEMENT = 0x350000, /**< ::lysp_ext_substmt.storage - `uint16_t *`
                                     ::lysp_ext_instance.parent - `struct lysp_ext *`
                                     ::lysc_ext_substmt.storage - not compiled
                                     ::lysc_ext_instance.parent - `struct lysc_ext *` */

    /* separated from the list of statements
     * the following tokens are part of the syntax and parsers have to work
     * with them, but they are not a standard YANG statements
     */
    LY_STMT_SYNTAX_SEMICOLON,
    LY_STMT_SYNTAX_LEFT_BRACE,
    LY_STMT_SYNTAX_RIGHT_BRACE,

    /*
     * YIN-specific tokens, still they are part of the syntax, but not the standard statements
     */
    LY_STMT_ARG_TEXT,
    LY_STMT_ARG_VALUE
};

/**
 * @brief Structure representing a generic parsed YANG substatement in an extension instance.
 */
struct lysp_stmt {
    const char *stmt;                /**< identifier of the statement */
    const char *arg;                 /**< statement's argument */
    LY_VALUE_FORMAT format;          /**< prefix format of the identifier/argument (::LY_VALUE_XML is YIN format) */
    void *prefix_data;               /**< Format-specific data for prefix resolution (see ly_resolve_prefix()) */

    struct lysp_stmt *next;          /**< link to the next statement */
    struct lysp_stmt *child;         /**< list of the statement's substatements (linked list) */
    uint16_t flags;                  /**< statement flags, can be set to LYS_YIN_ATTR */
    enum ly_stmt kw;                 /**< numeric respresentation of the stmt value */
};

/**
 * @brief Structure representing a parsed known YANG substatement in an extension instance.
 */
struct lysp_ext_substmt {
    enum ly_stmt stmt;  /**< parsed substatement */
    void *storage;      /**< pointer to the parsed storage of the statement according to the specific
                             lys_ext_substmt::stmt */
};

/**
 * @brief YANG extension parsed instance.
 */
struct lysp_ext_instance {
    const char *name;                       /**< extension identifier, including possible prefix */
    const char *argument;                   /**< optional value of the extension's argument */
    LY_VALUE_FORMAT format;                 /**< prefix format of the extension name/argument (::LY_VALUE_XML is YIN format) */
    void *prefix_data;                      /**< format-specific data for prefix resolution (see ly_resolve_prefix()) */
    struct lysp_ext *def;                   /**< pointer to the extension definition */

    void *parent;                           /**< pointer to the parent statement holding the extension instance(s), use
                                                 ::lysp_ext_instance#parent_stmt to access the value/structure */
    enum ly_stmt parent_stmt;               /**< type of the parent statement */
    LY_ARRAY_COUNT_TYPE parent_stmt_index;  /**< index of the stamenet in case the parent does not point to the parent
                                                 statement directly and it is an array */
    uint16_t flags;                         /**< ::LYS_INTERNAL value (@ref snodeflags) */

    const struct lyplg_ext_record *record;  /**< extension definition plugin record, if any */
    struct lysp_ext_substmt *substmts;      /**< list of supported known YANG statements with the pointer to their
                                                 parsed data ([sized array](@ref sizedarrays)) */
    void *parsed;                           /**< private plugin parsed data */
    struct lysp_stmt *child;                /**< list of generic (unknown) YANG statements */
};

/**
 * @brief Structure representing a compiled known YANG substatement in an extension instance.
 */
struct lysc_ext_substmt {
    enum ly_stmt stmt;  /**< compiled substatement */
    void *storage;      /**< pointer to the compiled storage of the statement according to the specific
                             lys_ext_substmt::stmt */
};

/**
 * @brief YANG extension compiled instance.
 */
struct lysc_ext_instance {
    struct lysc_ext *def;               /**< pointer to the extension definition */
    const char *argument;               /**< optional value of the extension's argument */
    struct lys_module *module;          /**< module where the extension instantiated is defined */
    struct lysc_ext_instance *exts;     /**< list of the extension instances ([sized array](@ref sizedarrays)) */

    void *parent;                       /**< pointer to the parent element holding the extension instance(s), use
                                             ::lysc_ext_instance#parent_stmt to access the value/structure */
    enum ly_stmt parent_stmt;           /**< type of the parent statement */
    LY_ARRAY_COUNT_TYPE parent_stmt_index;  /**< index of the stamenet in case the parent does not point to the parent
                                                 statement directly and it is an array */

    struct lysc_ext_substmt *substmts;  /**< list of supported known YANG statements with the pointer to their
                                             compiled data ([sized array](@ref sizedarrays)) */
    void *compiled;                     /**< private plugin compiled data */
};

/**
 * @brief Macro to define plugin information in external plugins
 *
 * Use as follows:
 * LYPLG_EXTENSIONS = {{<filled information of ::lyplg_ext_record>}, ..., {0}};
 */
#define LYPLG_EXTENSIONS \
    uint32_t plugins_extensions_apiver__ = LYPLG_EXT_API_VERSION; \
    const struct lyplg_ext_record plugins_extensions__[]

/**
 * @defgroup pluginsExtensionsParse Plugins: Extensions parsing support
 * @ingroup pluginsExtensions
 *
 * Implementing extension plugin parse callback.
 *
 * @{
 */

/**
 * @brief Callback for parsing extension instance substatements.
 *
 * All known YANG substatements can easily be parsed using ::lyplg_ext_parse_extension_instance.
 *
 * @param[in] pctx Parse context.
 * @param[in,out] ext Parsed extension instance data.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the extension instance is not supported and should be removed.
 * @return LY_ERR error on error.
 */
typedef LY_ERR (*lyplg_ext_parse_clb)(struct lysp_ctx *pctx, struct lysp_ext_instance *ext);

/**
 * @brief Log a message from an extension plugin using the parsed extension instance.
 *
 * @param[in] pctx Parse context to use.
 * @param[in] ext Parsed extensiopn instance.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err_no Error type code.
 * @param[in] format Format string to print.
 * @param[in] ... Format variable parameters.
 */
LIBYANG_API_DECL void lyplg_ext_parse_log(const struct lysp_ctx *pctx, const struct lysp_ext_instance *ext,
        LY_LOG_LEVEL level, LY_ERR err_no, const char *format, ...);

/**
 * @brief Get current parsed module from a parse context.
 *
 * @param[in] pctx Parse context.
 * @return Current (local) parse mod.
 */
LIBYANG_API_DECL const struct lysp_module *lyplg_ext_parse_get_cur_pmod(const struct lysp_ctx *pctx);

/**
 * @brief Parse substatements of an extension instance.
 *
 * Uses standard libyang schema compiler to transform YANG statements into the parsed schema structures. The plugins are
 * supposed to use this function when the extension instance's substatements can be parsed in a standard way.
 *
 * @param[in] pctx Parse context.
 * @param[in,out] ext Parsed extension instance with the prepared ::lysp_ext_instance.substmts array, which will be
 * updated by storing the parsed data.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_parse_extension_instance(struct lysp_ctx *pctx, struct lysp_ext_instance *ext);

/** @} pluginsExtensionsParse */

/**
 * @defgroup pluginsExtensionsCompile Plugins: Extensions compilation support
 * @ingroup pluginsExtensions
 *
 * Implementing extension plugin compile callback.
 *
 * @{
 */

/**
 * @defgroup scflags Schema compile flags
 *
 * Flags to modify schema compilation process and change the way how the particular statements are being compiled. *
 * @{
 */
#define LYS_COMPILE_GROUPING        0x01            /**< Compiling (validation) of a non-instantiated grouping.
                                                      In this case not all the restrictions are checked since they can
                                                      be valid only in the real placement of the grouping. This is
                                                      the case of any restriction that needs to look out of the statements
                                                      themselves, since the context is not known. */
#define LYS_COMPILE_DISABLED        0x02            /**< Compiling a disabled subtree (by its if-features). Meaning
                                                      it will be removed at the end of compilation and should not be
                                                      added to any unres sets. */
#define LYS_COMPILE_NO_CONFIG       0x04            /**< ignore config statements, neither inherit config value */
#define LYS_COMPILE_NO_DISABLED     0x08            /**< ignore if-feature statements */

#define LYS_COMPILE_RPC_INPUT       (LYS_IS_INPUT | LYS_COMPILE_NO_CONFIG)  /**< Internal option when compiling schema tree of RPC/action input */
#define LYS_COMPILE_RPC_OUTPUT      (LYS_IS_OUTPUT | LYS_COMPILE_NO_CONFIG) /**< Internal option when compiling schema tree of RPC/action output */
#define LYS_COMPILE_NOTIFICATION    (LYS_IS_NOTIF | LYS_COMPILE_NO_CONFIG)  /**< Internal option when compiling schema tree of Notification */

/** @} scflags */

/**
 * @brief Callback to compile extension from the lysp_ext_instance to the lysc_ext_instance. The later structure is generally prepared
 * and only the extension specific data are supposed to be added (if any).
 *
 * The parsed generic statements can be processed by the callback on its own or the ::lyplg_ext_compile_extension_instance()
 * function can be used to let the compilation to libyang following the standard rules for processing the YANG statements.
 *
 * @param[in] cctx Current compile context.
 * @param[in] extp Parsed extension instance data.
 * @param[in,out] ext Prepared compiled extension instance structure where an addition, extension-specific, data are
 * supposed to be placed for later use (data validation or use of external tool).
 * @return LY_SUCCESS in case of success.
 * @return LY_ENOT in case the extension instance is not supported and should be removed.
 * @return LY_ERR error on error.
 */
typedef LY_ERR (*lyplg_ext_compile_clb)(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp,
        struct lysc_ext_instance *ext);

/**
 * @brief Log a message from an extension plugin using the compiled extension instance.
 *
 * @param[in] cctx Optional compile context to generate the path from.
 * @param[in] ext Compiled extension instance.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err_no Error type code.
 * @param[in] format Format string to print.
 */
LIBYANG_API_DECL void lyplg_ext_compile_log(const struct lysc_ctx *cctx, const struct lysc_ext_instance *ext,
        LY_LOG_LEVEL level, LY_ERR err_no, const char *format, ...);

/**
 * @brief Log a message from an extension plugin using the compiled extension instance with an explicit error path.
 *
 * @param[in] path Log error path to use.
 * @param[in] ext Compiled extension instance.
 * @param[in] level Log message level (error, warning, etc.)
 * @param[in] err_no Error type code.
 * @param[in] format Format string to print.
 */
LIBYANG_API_DECL void lyplg_ext_compile_log_path(const char *path, const struct lysc_ext_instance *ext,
        LY_LOG_LEVEL level, LY_ERR err_no, const char *format, ...);

/**
 * @brief Log a message from an extension plugin using the compiled extension instance and a generated error item.
 *
 * @param[in] err Error item to log.
 * @param[in] ext Compiled extension instance.
 */
LIBYANG_API_DEF void lyplg_ext_compile_log_err(const struct ly_err_item *err, const struct lysc_ext_instance *ext);

/**
 * @brief YANG schema compilation context getter for libyang context.
 *
 * @param[in] ctx YANG schema compilation context.
 * @return libyang context connected with the compilation context.
 */
LIBYANG_API_DECL struct ly_ctx *lyplg_ext_compile_get_ctx(const struct lysc_ctx *ctx);

/**
 * @brief YANG schema compilation context getter for compilation options.
 *
 * @param[in] ctx YANG schema compilation context.
 * @return pointer to the compilation options to allow modifying them with @ref scflags values.
 */
LIBYANG_API_DECL uint32_t *lyplg_ext_compile_get_options(const struct lysc_ctx *ctx);

/**
 * @brief YANG schema compilation context getter for current module.
 *
 * @param[in] ctx YANG schema compilation context.
 * @return current module.
 */
LIBYANG_API_DECL const struct lys_module *lyplg_ext_compile_get_cur_mod(const struct lysc_ctx *ctx);

/**
 * @brief YANG schema compilation context getter for currently processed module.
 *
 * @param[in] ctx YANG schema compilation context.
 * @return Currently processed module.
 */
LIBYANG_API_DECL struct lysp_module *lyplg_ext_compile_get_pmod(const struct lysc_ctx *ctx);

/**
 * @brief Compile substatements of an extension instance.
 *
 * Uses standard libyang schema compiler to transform YANG statements into the compiled schema structures. The plugins are
 * supposed to use this function when the extension instance's substatements are supposed to be compiled in a standard way
 * (or if just the @ref scflags are enough to modify the compilation process).
 *
 * @param[in] ctx Compile context.
 * @param[in] extp Parsed representation of the extension instance being processed.
 * @param[in,out] ext Compiled extension instance with the prepared ::lysc_ext_instance.substmts array, which will be updated
 * by storing the compiled data.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID if compilation of the substatements fails.
 * @return LY_ENOT if the extension is disabled (by if-feature) and should be ignored.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_compile_extension_instance(struct lysc_ctx *ctx, const struct lysp_ext_instance *extp,
        struct lysc_ext_instance *ext);

/** @} pluginsExtensionsCompile */

/**
 * @defgroup pluginsExtensionsSprinterInfo Plugins: Extensions schema info printer support
 * @ingroup pluginsExtensions
 *
 * Implementing extension plugin schema info printer callback.
 *
 * @{
 */

/**
 * @brief Callback to print the compiled extension instance's private data in the INFO format.
 *
 * @param[in] ctx YANG printer context to provide output handler and other information for printing.
 * @param[in] ext The compiled extension instance, mainly to access the extensions.
 * @param[in,out] flag Flag to be shared with the caller regarding the opening brackets - 0 if the '{' not yet printed,
 * 1 otherwise.
 * @return LY_SUCCESS when everything was fine, other LY_ERR values in case of failure
 */
typedef LY_ERR (*lyplg_ext_sprinter_info_clb)(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag);

/**
 * @brief YANG printer context getter for output handler.
 *
 * @param[in] ctx YANG printer context.
 * @return Output handler where the data are being printed. Note that the address of the handler pointer in the context is
 * returned to allow to modify the handler.
 */
LIBYANG_API_DECL struct ly_out **lyplg_ext_print_get_out(const struct lyspr_ctx *ctx);

/**
 * @brief YANG printer context getter for printer options.
 *
 * @param[in] ctx YANG printer context.
 * @return pointer to the printer options to allow modifying them with @ref schemaprinterflags values.
 */
LIBYANG_API_DECL uint32_t *lyplg_ext_print_get_options(const struct lyspr_ctx *ctx);

/**
 * @brief YANG printer context getter for printer indentation level.
 *
 * @param[in] ctx YANG printer context.
 * @return pointer to the printer's indentation level to allow modifying its value.
 */
LIBYANG_API_DECL uint16_t *lyplg_ext_print_get_level(const struct lyspr_ctx *ctx);

/**
 * @brief Print substatements of an extension instance in info format (compiled YANG).
 *
 * Generic function to access YANG printer functions from the extension plugins (::lyplg_ext_sprinter_info_clb).
 *
 * @param[in] ctx YANG printer context to provide output handler and other information for printing.
 * @param[in] ext The compiled extension instance to access the extensions and substatements data.
 * @param[in,out] flag Flag to be shared with the caller regarding the opening brackets - 0 if the '{' not yet printed,
 * 1 otherwise.
 */
LIBYANG_API_DECL void lyplg_ext_print_info_extension_instance(struct lyspr_ctx *ctx, const struct lysc_ext_instance *ext,
        ly_bool *flag);

/** @} pluginsExtensionsSprinterInfo */

/**
 * @defgroup pluginsExtensionsSprinterTree Plugins: Extensions schema parsed and compiled tree printer support
 * @ingroup pluginsExtensions
 *
 * Implementing extension plugin schema parsed and compiled tree printer callback.
 *
 * @{
 */

/**
 * @brief Callback to print parent node of @p ext or to print the contents of the extension.
 *
 * Function is called in two different cases. If the printer_tree needs the tree-diagram form of a parent node,
 * then @p ctx is set to NULL. In the second case, if printer_tree needs to print the contents of the extension,
 * then @p ctx is set and function must prepare the nodes that should be printed using the
 * lyplg_ext_sprinter_tree* functions.
 *
 * @param[in] ext Extension instance.
 * @param[in,out] ctx Context for the tree printer. Extension contents can be inserted into it by functions
 * lyplg_ext_sprinter_ctree_add_ext_nodes(), lyplg_ext_sprinter_ctree_add_nodes() or by their ptree alternatives.
 * It parameter is set to NULL, then @p flags and @p add_opts are used by printer_tree.
 * @param[out] flags Optional override tree-diagram \<flags\> in a parent node. If @p ctx is set, ignore this parameter.
 * @param[out] add_opts Additional tree-diagram \<opts\> string in a parent node which is printed before \<opts\>. If @p ctx
 * is set, ignore this parameter.
 * @return LY_ERR value.
 */
typedef LY_ERR (*lyplg_ext_sprinter_ctree_clb)(struct lysc_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **flags, const char **add_opts);

/**
 * @brief Callback for rewriting the tree-diagram form of a specific node.
 *
 * If this callback is set, then it is called for each node that belongs to the extension instance.
 *
 * @param[in] node Node whose tree-diagram form can be modified by the function.
 * @param[in,out] plugin_priv Private context set by plugin.
 * @param[out] skip Flag set to 1 removes the node from printed diagram.
 * @param[out] flags Override tree-diagram \<flags\> string in the @p node.
 * @param[out] add_opts Additional tree-diagram \<opts\> string in the @p node which is printed before \<opts\>.
 * @return LY_ERR value.
 */
typedef LY_ERR (*lyplg_ext_sprinter_ctree_override_clb)(const struct lysc_node *node, const void *plugin_priv,
        ly_bool *skip, const char **flags, const char **add_opts);

/**
 * @brief Registration of printing a group of nodes, which is already in the extension.
 *
 * @param[in] ctx Context of printer_tree in which the group of nodes is saved and later printed.
 * @param[in] ext Extension in which the group of nodes will be searched.
 * @param[in] clb Override function that will be applied to each delivered node.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_sprinter_ctree_add_ext_nodes(const struct lyspr_tree_ctx *ctx,
        struct lysc_ext_instance *ext, lyplg_ext_sprinter_ctree_override_clb clb);

/**
 * @brief Registration of printing the group of nodes which were defined in the plugin.
 *
 * @param[in] ctx Context of printer_tree in which the group of nodes is saved and later printed.
 * @param[in] nodes Points to the first node in group.
 * @param[in] clb Override function that will be applied to each delivered node.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_sprinter_ctree_add_nodes(const struct lyspr_tree_ctx *ctx, struct lysc_node *nodes,
        lyplg_ext_sprinter_ctree_override_clb clb);

/**
 * @brief Registration of plugin-private data defined by the plugin that is shared between override_clb calls.
 *
 * @param[in] ctx Context of printer_tree in which plugin-private data will be saved.
 * @param[in] plugin_priv Plugin-private data shared between oberride_clb calls.
 * @param[in] free_clb Release function for @p plugin_priv.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_sprinter_tree_set_priv(const struct lyspr_tree_ctx *ctx, void *plugin_priv,
        void (*free_clb)(void *plugin_priv));

/**
 * @copydoc lyplg_ext_sprinter_ctree_clb
 */
typedef LY_ERR (*lyplg_ext_sprinter_ptree_clb)(struct lysp_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **flags, const char **add_opts);

/**
 * @copydoc lyplg_ext_sprinter_ctree_override_clb
 */
typedef LY_ERR (*lyplg_ext_sprinter_ptree_override_clb)(const struct lysp_node *node, const void *plugin_priv,
        ly_bool *skip, const char **flags, const char **add_opts);

/**
 * @copydoc lyplg_ext_sprinter_ctree_add_ext_nodes
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_sprinter_ptree_add_ext_nodes(const struct lyspr_tree_ctx *ctx,
        struct lysp_ext_instance *ext, lyplg_ext_sprinter_ptree_override_clb clb);

/**
 * @copydoc lyplg_ext_sprinter_ctree_add_nodes
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_sprinter_ptree_add_nodes(const struct lyspr_tree_ctx *ctx, struct lysp_node *nodes,
        lyplg_ext_sprinter_ptree_override_clb clb);

/** @} pluginsExtensionsSprinterTree */

/*
 * data node
 */

/**
 * @brief Callback called for all data nodes connected to the extension instance.
 *
 * Can be used for additional data node validation. Is called only after the whole data tree is created and standard
 * validation succeeds. Not called when parsing data and ::LYD_PARSE_ONLY is used.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] node Data node to process.
 * @param[in] validate_options Options used for the validation phase, see @ref datavalidationoptions.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
typedef LY_ERR (*lyplg_ext_data_node_clb)(struct lysc_ext_instance *ext, struct lyd_node *node, uint32_t validate_options);

/*
 * snode
 */

/**
 * @brief Callback for getting a schema node for a new YANG instance data described by an extension instance.
 * Needed only if the extension instance supports some nested standard YANG data.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] parent Parsed parent data node. Set if @p sparent is NULL.
 * @param[in] sparent Schema parent node. Set if @p parent is NULL.
 * @param[in] prefix Element prefix, if any.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of @p prefix.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[in] name Element name.
 * @param[in] name_len Length of @p name.
 * @param[out] snode Schema node to use for parsing the node.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the data are not described by @p ext.
 * @return LY_ERR on error.
 */
typedef LY_ERR (*lyplg_ext_data_snode_clb)(struct lysc_ext_instance *ext, const struct lyd_node *parent,
        const struct lysc_node *sparent, const char *prefix, size_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data,
        const char *name, size_t name_len, const struct lysc_node **snode);

/*
 * validate
 */

/**
 * @brief Callback for validating parsed YANG instance data described by an extension instance.
 *
 * This callback is used only for nested data definition (with a standard YANG schema parent).
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] sibling First sibling with schema node returned by ::lyplg_ext_data_snode_clb.
 * @param[in] dep_tree Tree to be used for validating references from the operation subtree, if operation.
 * @param[in] data_type Validated data type, can be ::LYD_TYPE_DATA_YANG, ::LYD_TYPE_RPC_YANG, ::LYD_TYPE_NOTIF_YANG,
 * or ::LYD_TYPE_REPLY_YANG.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[out] diff Optional diff with any changes made by the validation.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
typedef LY_ERR (*lyplg_ext_data_validate_clb)(struct lysc_ext_instance *ext, struct lyd_node *sibling,
        const struct lyd_node *dep_tree, enum lyd_type data_type, uint32_t val_opts, struct lyd_node **diff);

/*
 * parse free
 */

/**
 * @brief Callback to free the extension-specific data created by its parsing.
 *
 * @param[in] ctx libyang context.
 * @param[in,out] ext Parsed extension structure to free.
 */
typedef void (*lyplg_ext_parse_free_clb)(const struct ly_ctx *ctx, struct lysp_ext_instance *ext);

/**
 * @brief Free the extension instance's data parsed with ::lyplg_ext_parse_extension_instance().
 *
 * @param[in] ctx libyang context
 * @param[in] substmts Extension instance substatements to free.
 */
LIBYANG_API_DECL void lyplg_ext_pfree_instance_substatements(const struct ly_ctx *ctx, struct lysp_ext_substmt *substmts);

/*
 * compile free
 */

/**
 * @brief Callback to free the extension-specific data created by its compilation.
 *
 * @param[in] ctx libyang context.
 * @param[in,out] ext Compiled extension structure to free.
 */
typedef void (*lyplg_ext_compile_free_clb)(const struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Free the extension instance's data compiled with ::lyplg_ext_compile_extension_instance().
 *
 * @param[in] ctx libyang context
 * @param[in] substmts Extension instance substatements to free.
 */
LIBYANG_API_DECL void lyplg_ext_cfree_instance_substatements(const struct ly_ctx *ctx, struct lysc_ext_substmt *substmts);

/**
 * @brief Extension plugin implementing various aspects of a YANG extension.
 *
 * Every plugin should have at least either ::parse() or ::compile() callback defined but other than that **all**
 * the callbacks are **optional**.
 */
struct lyplg_ext {
    const char *id;                         /**< plugin identification (mainly for distinguish incompatible versions
                                                 of the plugins for external tools) */
    lyplg_ext_parse_clb parse;              /**< callback to parse the extension instance substatements */
    lyplg_ext_compile_clb compile;          /**< callback to compile extension instance from the parsed data */
    lyplg_ext_sprinter_info_clb printer_info;   /**< callback to print the compiled content (info format) of the extension
                                                     instance */
    lyplg_ext_sprinter_ctree_clb printer_ctree; /**< callback to print tree format of compiled node containing the
                                                     compiled content of the extension instance */
    lyplg_ext_sprinter_ptree_clb printer_ptree; /**< callback to print tree format of parsed node containing the
                                                     parsed content of the extension instance */
    lyplg_ext_data_node_clb node;           /**< callback to validate most relevant data instance for the extension
                                                 instance */
    lyplg_ext_data_snode_clb snode;         /**< callback to get schema node for nested YANG data */
    lyplg_ext_data_validate_clb validate;   /**< callback to validate parsed data instances according to the extension
                                                 definition */

    lyplg_ext_parse_free_clb pfree;         /**< free the extension-specific data created by its parsing */
    lyplg_ext_compile_free_clb cfree;       /**< free the extension-specific data created by its compilation */
};

struct lyplg_ext_record {
    /* plugin identification */
    const char *module;          /**< name of the module where the extension is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin functions. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< YANG name of the extension */

    /* runtime data */
    struct lyplg_ext plugin;     /**< data to utilize plugin implementation */
};

/**
 * @brief Stringify statement identifier.
 *
 * @param[in] stmt The statement identifier to stringify.
 * @return Constant string representation of the given @p stmt.
 */
LIBYANG_API_DECL const char *lyplg_ext_stmt2str(enum ly_stmt stmt);

/**
 * @brief Convert nodetype to statement identifier
 *
 * @param[in] nodetype Nodetype to convert.
 * @return Statement identifier representing the given @p nodetype.
 */
LIBYANG_API_DECL enum ly_stmt lyplg_ext_nodetype2stmt(uint16_t nodetype);

/**
 * @brief Get compiled ext instance storage for a specific statement.
 *
 * @param[in] ext Compiled ext instance.
 * @param[in] stmt Compiled statement. Can be a mask when the first match is returned, it is expected the storage is
 * the same for all the masked statements.
 * @param[in] storage_size Size of the value at @p storage address (dereferenced).
 * @param[out] storage Compiled ext instance substatement storage, NULL if was not compiled.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the substatement is not supported.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_get_storage(const struct lysc_ext_instance *ext, int stmt, uint32_t storage_size,
        const void **storage);

/**
 * @brief Get parsed ext instance storage for a specific statement.
 *
 * @param[in] ext Compiled ext instance.
 * @param[in] stmt Parsed statement. Can be a mask when the first match is returned, it is expected the storage is
 * the same for all the masked statements.
 * @param[in] storage_size Size of the value at @p storage address (dereferenced).
 * @param[out] storage Parsed ext instance substatement storage, NULL if was not parsed.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the substatement is not supported.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_parsed_get_storage(const struct lysc_ext_instance *ext, int stmt,
        uint32_t storage_size, const void **storage);

/**
 * @brief Get specific run-time extension instance data from a callback set by ::ly_ctx_set_ext_data_clb().
 *
 * @param[in] ctx Context with the callback.
 * @param[in] ext Compiled extension instance.
 * @param[out] ext_data Provided extension instance data.
 * @param[out] ext_data_free Whether the extension instance should free @p ext_data or not.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_get_data(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, void **ext_data,
        ly_bool *ext_data_free);

/**
 * @brief Insert extension instance data into a parent.
 *
 * @param[in] parent Parent node to insert into.
 * @param[in] first First top-level sibling node to insert.
 * @return LY_SUCCESS on success.
 * @return LY_ERR error on error.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_insert(struct lyd_node *parent, struct lyd_node *first);

/**
 * @brief Expand parent-reference xpath expressions
 *
 * @param[in] ext Context allocated for extension.
 * @param[out] refs Set of schema node matching parent-reference XPaths.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_schema_mount_get_parent_ref(const struct lysc_ext_instance *ext, struct ly_set **refs);

/**
 * @brief Allocate a new context for a particular instance of the yangmnt:mount-point extension.
 * Caller is responsible for **freeing** the created context.
 *
 * @param[in] ext Compiled extension instance.
 * @param[out] ctx Context with modules loaded from the list found in the extension data.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyplg_ext_schema_mount_create_context(const struct lysc_ext_instance *ext, struct ly_ctx **ctx);

/** @} pluginsExtensions */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_H_ */
