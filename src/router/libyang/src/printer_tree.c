/**
 * @file printer_tree.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief RFC tree printer for libyang data structure
 *
 * Copyright (c) 2015 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * @section TRP_DESIGN Design
 *
 * @code
 *          +---------+    +---------+    +---------+
 *   output |   trp   |    |   trb   |    |   tro   |
 *      <---+  Print  +<---+  Browse +<-->+  Obtain |
 *          |         |    |         |    |         |
 *          +---------+    +----+----+    +---------+
 *                              ^
 *                              |
 *                         +----+----+
 *                         |   trm   |
 *                         | Manager |
 *                         |         |
 *                         +----+----+
 *                              ^
 *                              | input
 *                              +
 * @endcode
 *
 * @subsection TRP_GLOSSARY Glossary
 *
 * @subsubsection TRP_trm trm
 * Manager functions are at the peak of abstraction. They are
 * able to print individual sections of the YANG tree diagram
 * (eg module, notifications, rpcs ...) and they call
 * Browse functions (@ref TRP_trb).
 *
 * @subsubsection TRP_trb trb
 * Browse functions contain a general algorithm (Preorder DFS)
 * for traversing the tree. It does not matter what data type
 * the tree contains (@ref lysc_node or @ref lysp_node), because it
 * requires a ready-made getter functions for traversing the tree
 * (@ref trt_fp_all) and transformation function to its own node
 * data type (@ref trt_node). These getter functions are generally
 * referred to as @ref TRP_tro. Browse functions can repeatedly
 * traverse nodes in the tree, for example, to calculate the alignment
 * gap before the nodes \<type\> in the YANG Tree Diagram.
 * The obtained @ref trt_node is passed to the @ref TRP_trp functions
 * to print the Tree diagram.
 *
 * @subsubsection TRP_tro tro
 * Functions that provide an extra wrapper for the libyang library.
 * The Obtain functions are further specialized according to whether
 * they operate on lysp_tree (@ref TRP_trop) or lysc_tree
 * (@ref TRP_troc). If they are general algorithms, then they have the
 * prefix \b tro_. The Obtain functions provide information to
 * @ref TRP_trb functions for printing the Tree diagram.
 *
 * @subsubsection TRP_trop trop
 * Functions for Obtaining information from Parsed schema tree.
 *
 * @subsubsection TRP_troc troc
 * Functions for Obtaining information from Compiled schema tree.
 *
 * @subsubsection TRP_trp trp
 * Print functions take care of the printing YANG diagram. They can
 * also split one node into multiple lines if the node does not fit
 * on one line.
 *
 * @subsubsection TRP_trt trt
 * Data type marking in the printer_tree module.
 *
 * @subsubsection TRP_trg trg
 * General functions.
 *
 * @subsection TRP_ADJUSTMENTS Adjustments
 * It is assumed that the changes are likely to take place mainly for
 * @ref TRP_tro, @ref TRP_trop or @ref TRP_troc functions because
 * they are the only ones dependent on libyang implementation.
 * In special cases, changes will also need to be made to the
 * @ref TRP_trp functions if a special algorithm is needed to print
 * (right now this is prepared for printing list's keys
 * and if-features).
 */

#include <assert.h>
#include <string.h>

#include "compat.h"
#include "ly_common.h"
#include "out_internal.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "printer_internal.h"
#include "printer_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief List of available actions.
 */
typedef enum {
    TRD_PRINT = 0,  /**< Normal behavior. It just prints. */
    TRD_CHAR_COUNT  /**< Characters will be counted instead of printing. */
} trt_ly_out_clb_arg_flag;

/**
 * @brief Structure is passed as 'writeclb' argument
 * to the ::ly_out_new_clb().
 */
struct ly_out_clb_arg {
    trt_ly_out_clb_arg_flag mode;   /**< flag specifying which action to take. */
    struct ly_out *out;             /**< The ly_out pointer delivered to the printer tree module via the main interface. */
    size_t counter;                 /**< Counter of printed characters. */
    LY_ERR last_error;              /**< The last error that occurred. If no error has occurred, it will be ::LY_SUCCESS. */
};

/**
 * @brief Initialize struct ly_out_clb_arg with default settings.
 */
#define TRP_INIT_LY_OUT_CLB_ARG(MODE, OUT, COUNTER, LAST_ERROR) \
    (struct ly_out_clb_arg) { \
        .mode = MODE, .out = OUT, \
        .counter = COUNTER, .last_error = LAST_ERROR \
    }

/**********************************************************************
 * Print getters
 *********************************************************************/

/**
 * @brief Callback functions that prints special cases.
 *
 * It just groups together tree context with trt_fp_print.
 */
struct trt_cf_print {
    const struct trt_tree_ctx *ctx;                             /**< Context of libyang tree. */

    void (*pf)(const struct trt_tree_ctx *, struct ly_out *);   /**< Pointing to function which printing list's keys or features. */
};

/**
 * @brief Callback functions for printing special cases.
 *
 * Functions with the suffix 'trp' can print most of the text on
 * output, just by setting the pointer to the string. But in some
 * cases, it's not that simple, because its entire string is fragmented
 * in memory. For example, for printing list's keys or if-features.
 * However, this depends on how the libyang library is implemented.
 * This implementation of the printer_tree module goes through
 * a lysp tree, but if it goes through a lysc tree, these special cases
 * would be different.
 * Functions must print including spaces or delimiters between names.
 */
struct trt_fp_print {
    void (*print_features_names)(const struct trt_tree_ctx *, struct ly_out *);   /**< Print list of features without {}? wrapper. */
    void (*print_keys)(const struct trt_tree_ctx *, struct ly_out *);             /**< Print list's keys without [] wrapper. */
};

/**
 * @brief Package which only groups getter function.
 */
struct trt_pck_print {
    const struct trt_tree_ctx *tree_ctx;    /**< Context of libyang tree. */
    struct trt_fp_print fps;                /**< Print function. */
};

/**
 * @brief Initialize struct trt_pck_print by parameters.
 */
#define TRP_INIT_PCK_PRINT(TREE_CTX, FP_PRINT) \
    (struct trt_pck_print) {.tree_ctx = TREE_CTX, .fps = FP_PRINT}

/**********************************************************************
 * Indent
 *********************************************************************/

/**
 * @brief Constants which are defined in the RFC or are observable
 * from the pyang tool.
 */
typedef enum {
    TRD_INDENT_EMPTY = 0,               /**< If the node is a case node, there is no space before the \<name\>. */
    TRD_INDENT_LONG_LINE_BREAK = 2,     /**< The new line should be indented so that it starts below \<name\> with
                                             a whitespace offset of at least two characters. */
    TRD_INDENT_LINE_BEGIN = 2,          /**< Indent below the keyword (module, augment ...).  */
    TRD_INDENT_BTW_SIBLINGS = 2,        /**< Indent between | and | characters. */
    TRD_INDENT_BEFORE_KEYS = 1,         /**< "..."___\<keys\>. */
    TRD_INDENT_BEFORE_TYPE = 4,         /**< "..."___\<type\>, but if mark is set then indent == 3. */
    TRD_INDENT_BEFORE_IFFEATURES = 1    /**< "..."___\<iffeatures\>. */
} trt_cnf_indent;

/**
 * @brief Type of indent in node.
 */
typedef enum {
    TRD_INDENT_IN_NODE_NORMAL = 0,  /**< Node fits on one line. */
    TRD_INDENT_IN_NODE_DIVIDED,     /**< The node must be split into multiple rows. */
    TRD_INDENT_IN_NODE_FAILED       /**< Cannot be crammed into one line. The condition for the maximum line length is violated. */
} trt_indent_in_node_type;

/** Constant to indicate the need to break a line. */
#define TRD_LINEBREAK -1

/**
 * @brief Records the alignment between the individual
 * elements of the node.
 *
 * @see trp_default_indent_in_node, trp_try_normal_indent_in_node
 */
struct trt_indent_in_node {
    trt_indent_in_node_type type;   /**< Type of indent in node. */
    int16_t btw_name_opts;          /**< Indent between node name and \<opts\>. */
    int16_t btw_opts_type;          /**< Indent between \<opts\> and \<type\>. */
    int16_t btw_type_iffeatures;    /**< Indent between type and features. Ignored if \<type\> missing. */
};

/**
 * @brief Type of wrappers to be printed.
 */
typedef enum {
    TRD_WRAPPER_TOP = 0,    /**< Related to the module. */
    TRD_WRAPPER_BODY        /**< Related to e.g. Augmentations or Groupings */
} trd_wrapper_type;

/**
 * @brief For resolving sibling symbol ('|') placement.
 *
 * Bit indicates where the sibling symbol must be printed.
 * This place is in multiples of ::TRD_INDENT_BTW_SIBLINGS.
 *
 * @see TRP_INIT_WRAPPER_TOP, TRP_INIT_WRAPPER_BODY,
 * trp_wrapper_set_mark, trp_wrapper_set_shift,
 * trp_wrapper_if_last_sibling, trp_wrapper_eq, trp_print_wrapper
 */
struct trt_wrapper {
    trd_wrapper_type type;  /**< Location of the wrapper. */
    uint64_t bit_marks1;    /**< The set bits indicate where the '|' character is to be printed.
                                 It follows that the maximum immersion of the printable node is 64. */
    uint32_t actual_pos;    /**< Actual position in bit_marks. */
};

/**
 * @brief Get wrapper related to the module section.
 *
 * @code
 * module: <module-name>
 *   +--<node>
 *   |
 * @endcode
 */
#define TRP_INIT_WRAPPER_TOP \
    (struct trt_wrapper) { \
        .type = TRD_WRAPPER_TOP, .actual_pos = 0, .bit_marks1 = 0 \
    }

/**
 * @brief Get wrapper related to subsection
 * e.g. Augmenations or Groupings.
 *
 * @code
 * module: <module-name>
 *   +--<node>
 *
 *   augment <target-node>:
 *     +--<node>
 * @endcode
 */
#define TRP_INIT_WRAPPER_BODY \
    (struct trt_wrapper) { \
        .type = TRD_WRAPPER_BODY, .actual_pos = 0, .bit_marks1 = 0 \
    }

/**
 * @brief Package which only groups wrapper and indent in node.
 */
struct trt_pck_indent {
    struct trt_wrapper wrapper;         /**< Coded "  |  |  " sequence. */
    struct trt_indent_in_node in_node;  /**< Indent in node. */
};

/**
 * @brief Initialize struct trt_pck_indent by parameters.
 */
#define TRP_INIT_PCK_INDENT(WRAPPER, INDENT_IN_NODE) \
    (struct trt_pck_indent){ \
        .wrapper = WRAPPER, .in_node = INDENT_IN_NODE \
    }

/**********************************************************************
 * flags
 *********************************************************************/

#define TRD_FLAGS_TYPE_EMPTY "--"
#define TRD_FLAGS_TYPE_RW "rw"
#define TRD_FLAGS_TYPE_RO "ro"
#define TRD_FLAGS_TYPE_RPC_INPUT_PARAMS "-w"
#define TRD_FLAGS_TYPE_USES_OF_GROUPING "-u"
#define TRD_FLAGS_TYPE_RPC "-x"
#define TRD_FLAGS_TYPE_NOTIF "-n"
#define TRD_FLAGS_TYPE_MOUNT_POINT "mp"

/**********************************************************************
 * node_name and opts
 *********************************************************************/

#define TRD_NODE_NAME_PREFIX_CHOICE "("
#define TRD_NODE_NAME_PREFIX_CASE ":("
#define TRD_NODE_NAME_TRIPLE_DOT "..."

/**
 * @brief Type of the node.
 *
 * Used mainly to complete the correct \<opts\> next to or
 * around the \<name\>.
 */
typedef enum {
    TRD_NODE_ELSE = 0,          /**< For some node which does not require special treatment. \<name\> */
    TRD_NODE_CASE,              /**< For case node. :(\<name\>) */
    TRD_NODE_CHOICE,            /**< For choice node. (\<name\>) */
    TRD_NODE_TRIPLE_DOT         /**< For collapsed sibling nodes and their children. Special case which doesn't belong here very well. */
} trt_node_type;

#define TRD_NODE_OPTIONAL "?"          /**< For an optional leaf, anydata, or anyxml. \<name\>? */
#define TRD_NODE_CONTAINER "!"         /**< For a presence container. \<name\>! */
#define TRD_NODE_LISTLEAFLIST "*"      /**< For a leaf-list or list. \<name\>* */

/**
 * @brief Type of node and his name.
 *
 * @see TRP_EMPTY_NODE_NAME, TRP_NODE_NAME_IS_EMPTY,
 * trp_print_node_name, trp_mark_is_used, trp_print_opts_keys
 */
struct trt_node_name {
    trt_node_type type;         /**< Type of the node relevant for printing. */
    ly_bool keys;               /**< Set to 1 if [\<keys\>] are to be printed. Valid for some types only. */
    const char *module_prefix;  /**< If the node is augmented into the tree from another module,
                                     so this is the prefix of that module. */
    const char *str;            /**< Name of the node. */
    const char *add_opts;       /**< Additional opts symbol from plugin. */
    const char *opts;           /**< The \<opts\> symbol. */
};

/**
 * @brief Create struct trt_node_name as empty.
 */
#define TRP_EMPTY_NODE_NAME \
    (struct trt_node_name) { \
        .type = TRD_NODE_ELSE, .keys = 0, .module_prefix = NULL, .str = NULL, .opts = NULL, .add_opts = NULL \
    }

/**
 * @brief Check if struct trt_node_name is empty.
 */
#define TRP_NODE_NAME_IS_EMPTY(NODE_NAME) \
    !NODE_NAME.str

/**********************************************************************
 * type
 *********************************************************************/

/**
 * @brief Type of the \<type\>
 */
typedef enum {
    TRD_TYPE_NAME = 0,  /**< Type is just a name that does not require special treatment. */
    TRD_TYPE_TARGET,    /**< Should have a form "-> TARGET", where TARGET is the leafref path. */
    TRD_TYPE_LEAFREF,   /**< This type is set automatically by the 'trp' algorithm.
                             So set type as ::TRD_TYPE_TARGET. */
    TRD_TYPE_EMPTY      /**< Type is not used at all. */
} trt_type_type;

/**
 * @brief \<type\> in the \<node\>.
 *
 * @see TRP_EMPTY_TRT_TYPE, TRP_TRT_TYPE_IS_EMPTY, trp_print_type
 */
struct trt_type {
    trt_type_type type; /**< Type of the \<type\>. */
    const char *str;    /**< Path or name of the type. */
};

/**
 * @brief Create empty struct trt_type.
 */
#define TRP_EMPTY_TRT_TYPE \
    (struct trt_type) {.type = TRD_TYPE_EMPTY, .str = NULL}

/**
 * @brief Check if struct trt_type is empty.
 */
#define TRP_TRT_TYPE_IS_EMPTY(TYPE_OF_TYPE) \
    TYPE_OF_TYPE.type == TRD_TYPE_EMPTY

/**
 * @brief Initialize struct trt_type by parameters.
 */
#define TRP_INIT_TRT_TYPE(TYPE_OF_TYPE, STRING) \
    (struct trt_type) {.type = TYPE_OF_TYPE, .str = STRING}

/**
 * @brief If-feature type.
 */
typedef enum {
    TRD_IFF_NON_PRESENT = 0,    /**< iffeatures are not present. */
    TRD_IFF_PRESENT,            /**< iffeatures are present and will be printed by
                                     trt_fp_print.print_features_names callback */
    TRD_IFF_OVERR               /**< iffeatures are override by plugin */
} trt_iffeatures_type;

/**
 * @brief \<if-features\>.
 */
struct trt_iffeatures {
    trt_iffeatures_type type;   /**< Type of iffeature. */
    char *str;                  /**< iffeatures string ready to print. Set if TRD_IFF_OVERR is set. */
};

/**
 * @brief Create empty iffeatures.
 */
#define TRP_EMPTY_TRT_IFFEATURES \
    (struct trt_iffeatures) {.type = TRD_IFF_NON_PRESENT}

/**
 * @brief Check if iffeatures is empty.
 *
 * @param[in] IFF_TYPE value from trt_iffeatures.type.
 * @return 1 if is empty.
 */
#define TRP_EMPTY_TRT_IFFEATURES_IS_EMPTY(IFF_TYPE) \
    (IFF_TYPE == TRD_IFF_NON_PRESENT)

/**********************************************************************
 * node
 *********************************************************************/

/**
 * @brief \<node\> data for printing.
 *
 * It contains RFC's:
 * \<status\>--\<flags\> \<name\>\<opts\> \<type\> \<if-features\>.
 * Item \<opts\> is moved to part struct trt_node_name.
 * For printing [\<keys\>] and if-features is required special
 * functions which prints them.
 *
 * @see TRP_EMPTY_NODE, trp_node_is_empty, trp_node_body_is_empty,
 * trp_print_node_up_to_name, trp_print_divided_node_up_to_name,
 * trp_print_node
 */
struct trt_node {
    const char *status;                 /**< \<status\>. */
    const char *flags;                  /**< \<flags\>. */
    struct trt_node_name name;          /**< \<node\> with \<opts\> mark or [\<keys\>]. */
    struct trt_type type;               /**< \<type\> contains the name of the type or type for leafref. */
    struct trt_iffeatures iffeatures;   /**< \<if-features\>. */
    ly_bool last_one;                   /**< Information about whether the node is the last. */
};

/**
 * @brief Create struct trt_node as empty.
 */
#define TRP_EMPTY_NODE \
    (struct trt_node) { \
        .status = NULL, \
        .flags = NULL, \
        .name = TRP_EMPTY_NODE_NAME, \
        .type = TRP_EMPTY_TRT_TYPE, \
        .iffeatures = TRP_EMPTY_TRT_IFFEATURES, \
        .last_one = 1 \
    }

/**
 * @brief Package which only groups indent and node.
 */
struct trt_pair_indent_node {
    struct trt_indent_in_node indent;
    struct trt_node node;
};

/**
 * @brief Initialize struct trt_pair_indent_node by parameters.
 */
#define TRP_INIT_PAIR_INDENT_NODE(INDENT_IN_NODE, NODE) \
    (struct trt_pair_indent_node) { \
        .indent = INDENT_IN_NODE, .node = NODE \
    }

/**********************************************************************
 * statement
 *********************************************************************/

#define TRD_KEYWORD_MODULE "module"
#define TRD_KEYWORD_SUBMODULE "submodule"
#define TRD_KEYWORD_AUGMENT "augment"
#define TRD_KEYWORD_RPC "rpcs"
#define TRD_KEYWORD_NOTIF "notifications"
#define TRD_KEYWORD_GROUPING "grouping"

/**
 * @brief Main sign of the tree nodes.
 *
 * @see TRP_EMPTY_KEYWORD_STMT, TRP_KEYWORD_STMT_IS_EMPTY
 * trt_print_keyword_stmt_begin, trt_print_keyword_stmt_str,
 * trt_print_keyword_stmt_end, trp_print_keyword_stmt
 */
struct trt_keyword_stmt {
    const char *section_name;   /**< String containing section name. */
    const char *argument;       /**< Name or path located begind section name. */
    ly_bool has_node;           /**< Flag if section has any nodes. */
};

/**
 * @brief Create struct trt_keyword_stmt as empty.
 */
#define TRP_EMPTY_KEYWORD_STMT \
    (struct trt_keyword_stmt) {.section_name = NULL, .argument = NULL, .has_node = 0}

/**********************************************************************
 * Modify getters
 *********************************************************************/

struct trt_parent_cache;

/**
 * @brief Functions that change the state of the tree_ctx structure.
 *
 * The 'trop' or 'troc' functions are set here, which provide data
 * for the 'trp' printing functions and are also called from the
 * 'trb' browsing functions when walking through a tree. These callback
 * functions need to be checked or reformulated if changes to the
 * libyang library affect the printing tree. For all, if the value
 * cannot be returned, its empty version obtained by relevant TRP_EMPTY
 * macro is returned.
 */
struct trt_fp_modify_ctx {
    ly_bool (*parent)(struct trt_tree_ctx *);                                           /**< Jump to parent node. Return true if parent exists. */
    struct trt_node (*first_sibling)(struct trt_parent_cache, struct trt_tree_ctx *);   /**< Jump on the first of the siblings. */
    struct trt_node (*next_sibling)(struct trt_parent_cache, struct trt_tree_ctx *);    /**< Jump to next sibling of the current node. */
    struct trt_node (*next_child)(struct trt_parent_cache, struct trt_tree_ctx *);      /**< Jump to the child of the current node. */
};

/**
 * @brief Create modify functions for compiled tree.
 */
#define TRP_TRT_FP_MODIFY_COMPILED \
    (struct trt_fp_modify_ctx) { \
        .parent = troc_modi_parent, \
        .first_sibling = troc_modi_first_sibling, \
        .next_sibling = troc_modi_next_sibling, \
        .next_child = troc_modi_next_child, \
    }

/**
 * @brief Create modify functions for parsed tree.
 */
#define TRP_TRT_FP_MODIFY_PARSED \
    (struct trt_fp_modify_ctx) { \
        .parent = trop_modi_parent, \
        .first_sibling = trop_modi_first_sibling, \
        .next_sibling = trop_modi_next_sibling, \
        .next_child = trop_modi_next_child, \
    }

/**********************************************************************
 * Read getters
 *********************************************************************/

/**
 * @brief Functions that do not change the state of the tree_structure.
 *
 * For details see trt_fp_modify_ctx.
 */
struct trt_fp_read {
    struct trt_keyword_stmt (*module_name)(const struct trt_tree_ctx *);            /**< Get name of the module. */
    struct trt_node (*node)(struct trt_parent_cache, struct trt_tree_ctx *);        /**< Get current node. */
    ly_bool (*if_sibling_exists)(const struct trt_tree_ctx *);                      /**< Check if node's sibling exists. */
    ly_bool (*if_parent_exists)(const struct trt_tree_ctx *);                       /**< Check if node's parent exists. */
};

/**
 * @brief Create read functions for compiled tree.
 */
#define TRP_TRT_FP_READ_COMPILED \
    (struct trt_fp_read) { \
        .module_name = tro_read_module_name, \
        .node = troc_read_node, \
        .if_sibling_exists = troc_read_if_sibling_exists, \
        .if_parent_exists = tro_read_if_sibling_exists \
    }

/**
 * @brief Create read functions for parsed tree.
 */
#define TRP_TRT_FP_READ_PARSED \
    (struct trt_fp_read) { \
        .module_name = tro_read_module_name, \
        .node = trop_read_node, \
        .if_sibling_exists = trop_read_if_sibling_exists, \
        .if_parent_exists = tro_read_if_sibling_exists \
    }

/**********************************************************************
 * All getters
 *********************************************************************/

/**
 * @brief A set of all necessary functions that must be provided
 * for the printer.
 */
struct trt_fp_all {
    struct trt_fp_modify_ctx modify;    /**< Function pointers which modify state of trt_tree_ctx. */
    struct trt_fp_read read;            /**< Function pointers which only reads state of trt_tree_ctx. */
    struct trt_fp_print print;          /**< Functions pointers for printing special items in node. */
};

/**********************************************************************
 * Printer context
 *********************************************************************/

/**
 * @brief Main structure for @ref TRP_trp part.
 */
struct trt_printer_ctx {
    struct ly_out *out;     /**< Handler to printing. */
    struct trt_fp_all fp;   /**< @ref TRP_tro functions callbacks. */
    size_t max_line_length; /**< The maximum number of characters that can be
                               printed on one line, including the last. */
};

/**********************************************************************
 * Tro functions
 *********************************************************************/

/**
 * @brief The name of the section to which the node belongs.
 */
typedef enum {
    TRD_SECT_MODULE = 0,    /**< The node belongs to the "module: <module_name>:" label. */
    TRD_SECT_AUGMENT,       /**< The node belongs to some "augment <target-node>:" label. */
    TRD_SECT_RPCS,          /**< The node belongs to the "rpcs:" label. */
    TRD_SECT_NOTIF,         /**< The node belongs to the "notifications:" label. */
    TRD_SECT_GROUPING,      /**< The node belongs to some "grouping <grouping-name>:" label. */
    TRD_SECT_PLUG_DATA      /**< The node belongs to some plugin section. */
} trt_actual_section;

/**
 * @brief Types of nodes that have some effect on their children.
 */
typedef enum {
    TRD_ANCESTOR_ELSE = 0,      /**< Everything not listed. */
    TRD_ANCESTOR_RPC_INPUT,     /**< ::LYS_INPUT */
    TRD_ANCESTOR_RPC_OUTPUT,    /**< ::LYS_OUTPUT */
    TRD_ANCESTOR_NOTIF          /**< ::LYS_NOTIF */
} trt_ancestor_type;

/**
 * @brief Saved information when browsing the tree downwards.
 *
 * This structure helps prevent frequent retrieval of information
 * from the tree. Functions @ref TRP_trb are designed to preserve
 * this structures during their recursive calls. This functions do not
 * interfere in any way with this data. This structure
 * is used by @ref TRP_trop functions which, thanks to this
 * structure, can return a node with the correct data. The word
 * \b parent is in the structure name, because this data refers to
 * the last parent and at the same time the states of its
 * ancestors data. Only the function jumping on the child
 * (next_child(...)) creates this structure, because the pointer
 * to the current node moves down the tree. It's like passing
 * the genetic code to children. Some data must be inherited and
 * there are two approaches to this problem. Either it will always
 * be determined which inheritance states belong to the current node
 * (which can lead to regular travel to the root node) or
 * the inheritance states will be stored during the recursive calls.
 * So the problem was solved by the second option. Why does
 * the structure contain this data? Because it walks through
 * the lysp tree. For walks through the lysc tree is trt_parent_cache
 * useless.
 *
 * @see TRO_EMPTY_PARENT_CACHE, tro_parent_cache_for_child
 */
struct trt_parent_cache {
    trt_ancestor_type ancestor; /**< Some types of nodes have a special effect on their children. */
    uint16_t lys_status;        /**< Inherited status CURR, DEPRC, OBSLT. */
    uint16_t lys_config;        /**< Inherited config W or R. */
    const struct lysp_node_list *last_list; /**< The last ::LYS_LIST passed. */
};

/**
 * @brief Return trt_parent_cache filled with default values.
 */
#define TRP_EMPTY_PARENT_CACHE \
    (struct trt_parent_cache) { \
        .ancestor = TRD_ANCESTOR_ELSE, .lys_status = LYS_STATUS_CURR, \
        .lys_config = LYS_CONFIG_W, .last_list = NULL \
    }

/**
 * @brief Node override from plugin.
 */
struct lyplg_ext_sprinter_tree_node_override {
    const char *flags;        /**< Override for \<flags\>. */
    const char *add_opts;     /**< Additional symbols for \<opts\>. */
};

/**
 * @brief Context for plugin extension.
 */
struct trt_plugin_ctx {
    struct lyspr_tree_ctx *ctx;                                 /**< Pointer to main context. */
    struct lyspr_tree_schema *schema;                           /**< Current schema to print. */
    ly_bool filtered;                                           /**< Flag if current node is filtered. */
    struct lyplg_ext_sprinter_tree_node_override node_overr;    /**< Current node override. */
    ly_bool last_schema;                                        /**< Flag if schema is last. */
    ly_bool last_error;                                         /**< Last error from plugin. */
};

/**
 * @brief Main structure for browsing the libyang tree
 */
struct trt_tree_ctx {
    ly_bool lysc_tree;                              /**< The lysc nodes are used for browsing through the tree.
                                                         It is assumed that once set, it does not change.
                                                         If it is true then trt_tree_ctx.pn and
                                                         trt_tree_ctx.tpn are not used.
                                                         If it is false then trt_tree_ctx.cn is not used. */
    trt_actual_section section;                     /**< To which section pn points. */
    const struct lysp_module *pmod;                 /**< Parsed YANG schema tree. */
    const struct lysc_module *cmod;                 /**< Compiled YANG schema tree. */
    const struct lysp_node *pn;                     /**< Actual pointer to parsed node. */
    const struct lysp_node *tpn;                    /**< Pointer to actual top-node. */
    const struct lysc_node *cn;                     /**< Actual pointer to compiled node. */
    LY_ERR last_error;                              /**< Error value during printing. */

    struct trt_plugin_ctx plugin_ctx;               /**< Context for plugin. */
};

/**
 * @brief Create empty node override.
 */
#define TRP_TREE_CTX_EMPTY_NODE_OVERR \
    (struct lyplg_ext_sprinter_tree_node_override) { \
        .flags = NULL, \
        .add_opts = NULL, \
    }

/**
 * @brief Check if lysp node is available from
 * the current compiled node.
 *
 * Use only if trt_tree_ctx.lysc_tree is set to true.
 */
#define TRP_TREE_CTX_LYSP_NODE_PRESENT(CN) \
    (CN->priv)

/**
 * @brief Get lysp_node from trt_tree_ctx.cn.
 *
 * Use only if :TRP_TREE_CTX_LYSP_NODE_PRESENT returns true
 * for that node.
 */
#define TRP_TREE_CTX_GET_LYSP_NODE(CN) \
    ((const struct lysp_node *)CN->priv)

/** Getter function for ::trop_node_charptr(). */
typedef const char *(*trt_get_charptr_func)(const struct lysp_node *pn);

/**
 * @brief Simple getter functions for lysp and lysc nodes.
 *
 * This structure is useful if we have a general algorithm
 * (tro function) that can be used for both lysc and lysp nodes.
 * Thanks to this structure, we prevent code redundancy.
 * We don't have to write basically the same algorithm twice
 * for lysp and lysc trees.
 */
struct tro_getters {
    uint16_t (*nodetype)(const void *);         /**< Get nodetype. */
    const void *(*next)(const void *);          /**< Get sibling. */
    const void *(*parent)(const void *);        /**< Get parent. */
    const void *(*child)(const void *);         /**< Get child. */
    const void *(*actions)(const void *);       /**< Get actions. */
    const void *(*action_input)(const void *);  /**< Get input action from action node. */
    const void *(*action_output)(const void *); /**< Get output action from action node. */
    const void *(*notifs)(const void *);        /**< Get notifs. */
};

/**********************************************************************
 * Definition of the general Trg functions
 *********************************************************************/

/**
 * @brief Print a substring but limited to the maximum length.
 * @param[in] str is pointer to source.
 * @param[in] len is number of characters to be printed.
 * @param[in,out] out is output handler.
 * @return str parameter shifted by len.
 */
static const char *
trg_print_substr(const char *str, size_t len, struct ly_out *out)
{
    for (size_t i = 0; i < len; i++) {
        ly_print_(out, "%c", str[0]);
        str++;
    }
    return str;
}

/**
 * @brief Pointer is not NULL and does not point to an empty string.
 * @param[in] str is pointer to string to be checked.
 * @return 1 if str pointing to non empty string otherwise 0.
 */
static ly_bool
trg_charptr_has_data(const char *str)
{
    return (str) && (str[0] != '\0');
}

/**
 * @brief Check if @p word in @p src is present where words are
 * delimited by @p delim.
 * @param[in] src is source where words are separated by @p delim.
 * @param[in] word to be searched.
 * @param[in] delim is delimiter between @p words in @p src.
 * @return 1 if src contains @p word otherwise 0.
 */
static ly_bool
trg_word_is_present(const char *src, const char *word, char delim)
{
    const char *hit;

    if ((!src) || (src[0] == '\0') || (!word)) {
        return 0;
    }

    hit = strstr(src, word);

    if (hit) {
        /* word was founded at the begin of src
         * OR it match somewhere after delim
         */
        if ((hit == src) || (hit[-1] == delim)) {
            /* end of word was founded at the end of src
             * OR end of word was match somewhere before delim
             */
            char delim_or_end = (hit + strlen(word))[0];

            if ((delim_or_end == '\0') || (delim_or_end == delim)) {
                return 1;
            }
        }
        /* after -> hit is just substr and it's not the whole word */
        /* jump to the next word */
        for ( ; (src[0] != '\0') && (src[0] != delim); src++) {}
        /* skip delim */
        src = src[0] == '\0' ? src : src + 1;
        /* continue with searching */
        return trg_word_is_present(src, word, delim);
    } else {
        return 0;
    }
}

/**********************************************************************
 * Definition of printer functions
 *********************************************************************/

/**
 * @brief Write callback for ::ly_out_new_clb().
 *
 * @param[in] user_data is type of struct ly_out_clb_arg.
 * @param[in] buf contains input characters
 * @param[in] count is number of characters in buf.
 * @return Number of printed bytes.
 * @return Negative value in case of error.
 */
static ssize_t
trp_ly_out_clb_func(void *user_data, const void *buf, size_t count)
{
    LY_ERR erc = LY_SUCCESS;
    struct ly_out_clb_arg *data = (struct ly_out_clb_arg *)user_data;

    switch (data->mode) {
    case TRD_PRINT:
        erc = ly_write_(data->out, buf, count);
        break;
    case TRD_CHAR_COUNT:
        data->counter = data->counter + count;
        break;
    default:
        break;
    }

    if (erc != LY_SUCCESS) {
        data->last_error = erc;
        return -1;
    } else {
        return count;
    }
}

/**
 * @brief Check that indent in node can be considered as equivalent.
 * @param[in] first is the first indent in node.
 * @param[in] second is the second indent in node.
 * @return 1 if indents are equivalent otherwise 0.
 */
static ly_bool
trp_indent_in_node_are_eq(struct trt_indent_in_node first, struct trt_indent_in_node second)
{
    const ly_bool a = first.type == second.type;
    const ly_bool b = first.btw_name_opts == second.btw_name_opts;
    const ly_bool c = first.btw_opts_type == second.btw_opts_type;
    const ly_bool d = first.btw_type_iffeatures == second.btw_type_iffeatures;

    return a && b && c && d;
}

/**
 * @brief Setting space character because node is last sibling.
 * @param[in] wr is wrapper over which the shift operation
 * is to be performed.
 * @return New shifted wrapper.
 */
static struct trt_wrapper
trp_wrapper_set_shift(struct trt_wrapper wr)
{
    assert(wr.actual_pos < 64);
    /* +--<node>
     *    +--<node>
     */
    wr.actual_pos++;
    return wr;
}

/**
 * @brief Setting '|' symbol because node is divided or
 * it is not last sibling.
 * @param[in] wr is source of wrapper.
 * @return New wrapper which is marked at actual position and shifted.
 */
static struct trt_wrapper
trp_wrapper_set_mark(struct trt_wrapper wr)
{
    assert(wr.actual_pos < 64);
    wr.bit_marks1 |= 1U << wr.actual_pos;
    return trp_wrapper_set_shift(wr);
}

/**
 * @brief Setting ' ' symbol if node is last sibling otherwise set '|'.
 * @param[in] wr is actual wrapper.
 * @param[in] last_one is flag. Value 1 saying if the node is the last
 * and has no more siblings.
 * @return New wrapper for the actual node.
 */
static struct trt_wrapper
trp_wrapper_if_last_sibling(struct trt_wrapper wr, ly_bool last_one)
{
    return last_one ? trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);
}

/**
 * @brief Test if the wrappers are equivalent.
 * @param[in] first is the first wrapper.
 * @param[in] second is the second wrapper.
 * @return 1 if the wrappers are equivalent otherwise 0.
 */
static ly_bool
trp_wrapper_eq(struct trt_wrapper first, struct trt_wrapper second)
{
    const ly_bool a = first.type == second.type;
    const ly_bool b = first.bit_marks1 == second.bit_marks1;
    const ly_bool c = first.actual_pos == second.actual_pos;

    return a && b && c;
}

/**
 * @brief Print "  |  " sequence on line.
 * @param[in] wr is wrapper to be printed.
 * @param[in,out] out is output handler.
 */
static void
trp_print_wrapper(struct trt_wrapper wr, struct ly_out *out)
{
    uint32_t lb;

    if (wr.type == TRD_WRAPPER_TOP) {
        lb = TRD_INDENT_LINE_BEGIN;
    } else if (wr.type == TRD_WRAPPER_BODY) {
        lb = TRD_INDENT_LINE_BEGIN * 2;
    } else {
        lb = TRD_INDENT_LINE_BEGIN;
    }

    ly_print_(out, "%*c", lb, ' ');

    if (trp_wrapper_eq(wr, TRP_INIT_WRAPPER_TOP)) {
        return;
    }

    for (uint32_t i = 0; i < wr.actual_pos; i++) {
        /** Test if the bit on the index is set. */
        if ((wr.bit_marks1 >> i) & 1U) {
            ly_print_(out, "|");
        } else {
            ly_print_(out, " ");
        }

        if (i != wr.actual_pos) {
            ly_print_(out, "%*c", TRD_INDENT_BTW_SIBLINGS, ' ');
        }
    }
}

/**
 * @brief Check if struct trt_node is empty.
 * @param[in] node is item to test.
 * @return 1 if node is considered empty otherwise 0.
 */
static ly_bool
trp_node_is_empty(const struct trt_node *node)
{
    const ly_bool a = TRP_EMPTY_TRT_IFFEATURES_IS_EMPTY(node->iffeatures.type);
    const ly_bool b = TRP_TRT_TYPE_IS_EMPTY(node->type);
    const ly_bool c = TRP_NODE_NAME_IS_EMPTY(node->name);
    const ly_bool d = node->flags == NULL;
    const ly_bool e = node->status == NULL;

    return a && b && c && d && e;
}

/**
 * @brief Check if [\<keys\>], \<type\> and
 * \<iffeatures\> are empty/not_set.
 * @param[in] node is item to test.
 * @return 1 if node has no \<keys\> \<type\> or \<iffeatures\>
 * otherwise 0.
 */
static ly_bool
trp_node_body_is_empty(const struct trt_node *node)
{
    const ly_bool a = TRP_EMPTY_TRT_IFFEATURES_IS_EMPTY(node->iffeatures.type);
    const ly_bool b = TRP_TRT_TYPE_IS_EMPTY(node->type);
    const ly_bool c = !node->name.keys;

    return a && b && c;
}

/**
 * @brief Print entire struct trt_node_name structure.
 * @param[in] node_name is item to print.
 * @param[in,out] out is output handler.
 */
static void
trp_print_node_name(struct trt_node_name node_name, struct ly_out *out)
{
    const char *mod_prefix;
    const char *colon;
    const char trd_node_name_suffix_choice[] = ")";
    const char trd_node_name_suffix_case[] = ")";

    if (TRP_NODE_NAME_IS_EMPTY(node_name)) {
        return;
    }

    if (node_name.module_prefix) {
        mod_prefix = node_name.module_prefix;
        colon = ":";
    } else {
        mod_prefix = "";
        colon = "";
    }

    switch (node_name.type) {
    case TRD_NODE_ELSE:
        ly_print_(out, "%s%s%s", mod_prefix, colon, node_name.str);
        break;
    case TRD_NODE_CASE:
        ly_print_(out, "%s%s%s%s%s", TRD_NODE_NAME_PREFIX_CASE, mod_prefix, colon, node_name.str, trd_node_name_suffix_case);
        break;
    case TRD_NODE_CHOICE:
        ly_print_(out, "%s%s%s%s%s", TRD_NODE_NAME_PREFIX_CHOICE,  mod_prefix, colon, node_name.str, trd_node_name_suffix_choice);
        break;
    case TRD_NODE_TRIPLE_DOT:
        ly_print_(out, "%s", TRD_NODE_NAME_TRIPLE_DOT);
        break;
    default:
        break;
    }

    if (node_name.add_opts) {
        ly_print_(out, "%s", node_name.add_opts);
    }
    if (node_name.opts) {
        ly_print_(out, "%s", node_name.opts);
    }
}

/**
 * @brief Check if mark (?, !, *, /, @) is implicitly contained in
 * struct trt_node_name.
 * @param[in] node_name is structure containing the 'mark'.
 * @return 1 if contain otherwise 0.
 */
static ly_bool
trp_mark_is_used(struct trt_node_name node_name)
{
    if (TRP_NODE_NAME_IS_EMPTY(node_name)) {
        return 0;
    } else if (node_name.keys) {
        return 0;
    }

    switch (node_name.type) {
    case TRD_NODE_ELSE:
    case TRD_NODE_CASE:
        return 0;
    default:
        if (node_name.add_opts || node_name.opts) {
            return 1;
        } else {
            return 0;
        }
    }
}

/**
 * @brief Print opts keys.
 * @param[in] node_name contains type of the node with his name.
 * @param[in] btw_name_opts is number of spaces between name and [keys].
 * @param[in] cf is basically a pointer to the function that prints
 * the keys.
 * @param[in,out] out is output handler.
 */
static void
trp_print_opts_keys(struct trt_node_name node_name, int16_t btw_name_opts, struct trt_cf_print cf, struct ly_out *out)
{
    if (!node_name.keys) {
        return;
    }

    /* <name><mark>___<keys>*/
    if (btw_name_opts > 0) {
        ly_print_(out, "%*c", btw_name_opts, ' ');
    }
    ly_print_(out, "[");
    cf.pf(cf.ctx, out);
    ly_print_(out, "]");
}

/**
 * @brief Print entire struct trt_type structure.
 * @param[in] type is item to print.
 * @param[in,out] out is output handler.
 */
static void
trp_print_type(struct trt_type type, struct ly_out *out)
{
    if (TRP_TRT_TYPE_IS_EMPTY(type)) {
        return;
    }

    switch (type.type) {
    case TRD_TYPE_NAME:
        ly_print_(out, "%s", type.str);
        break;
    case TRD_TYPE_TARGET:
        ly_print_(out, "-> %s", type.str);
        break;
    case TRD_TYPE_LEAFREF:
        ly_print_(out, "leafref");
    default:
        break;
    }
}

/**
 * @brief Print all iffeatures of node
 *
 * @param[in] iff is iffeatures to print.
 * @param[in] cf is basically a pointer to the function that prints the list of features.
 * @param[in,out] out is output handler.
 */
static void
trp_print_iffeatures(struct trt_iffeatures iff, struct trt_cf_print cf, struct ly_out *out)
{
    if (iff.type == TRD_IFF_PRESENT) {
        ly_print_(out, "{");
        cf.pf(cf.ctx, out);
        ly_print_(out, "}?");
    } else if (iff.type == TRD_IFF_OVERR) {
        ly_print_(out, "%s", iff.str);
    }
}

/**
 * @brief Print just \<status\>--\<flags\> \<name\> with opts mark.
 * @param[in] node contains items to print.
 * @param[in] out is output handler.
 */
static void
trp_print_node_up_to_name(const struct trt_node *node, struct ly_out *out)
{
    if (node->name.type == TRD_NODE_TRIPLE_DOT) {
        trp_print_node_name(node->name, out);
        return;
    }
    /* <status>--<flags> */
    ly_print_(out, "%s", node->status);
    ly_print_(out, "--");
    /* If the node is a case node, there is no space before the <name>
     * also case node has no flags.
     */
    if (node->flags && (node->name.type != TRD_NODE_CASE)) {
        ly_print_(out, "%s", node->flags);
        ly_print_(out, " ");
    }
    /* <name> */
    trp_print_node_name(node->name, out);
}

/**
 * @brief Print alignment (spaces) instead of
 * \<status\>--\<flags\> \<name\> for divided node.
 * @param[in] node contains items to print.
 * @param[in] out is output handler.
 */
static void
trp_print_divided_node_up_to_name(const struct trt_node *node, struct ly_out *out)
{
    uint32_t space = strlen(node->flags);

    if (node->name.type == TRD_NODE_CASE) {
        /* :(<name> */
        space += strlen(TRD_NODE_NAME_PREFIX_CASE);
    } else if (node->name.type == TRD_NODE_CHOICE) {
        /* (<name> */
        space += strlen(TRD_NODE_NAME_PREFIX_CHOICE);
    } else {
        /* _<name> */
        space += strlen(" ");
    }

    /* <name>
     * __
     */
    space += TRD_INDENT_LONG_LINE_BREAK;

    ly_print_(out, "%*c", space, ' ');
}

/**
 * @brief Print struct trt_node structure.
 * @param[in] node is item to print.
 * @param[in] pck package of functions for
 * printing [\<keys\>] and \<iffeatures\>.
 * @param[in] indent is the indent in node.
 * @param[in,out] out is output handler.
 */
static void
trp_print_node(const struct trt_node *node, struct trt_pck_print pck, struct trt_indent_in_node indent, struct ly_out *out)
{
    ly_bool triple_dot;
    ly_bool divided;
    struct trt_cf_print cf_print_keys;
    struct trt_cf_print cf_print_iffeatures;

    if (trp_node_is_empty(node)) {
        return;
    }

    /* <status>--<flags> <name><opts> <type> <if-features> */
    triple_dot = node->name.type == TRD_NODE_TRIPLE_DOT;
    divided = indent.type == TRD_INDENT_IN_NODE_DIVIDED;

    if (triple_dot) {
        trp_print_node_name(node->name, out);
        return;
    } else if (!divided) {
        trp_print_node_up_to_name(node, out);
    } else {
        trp_print_divided_node_up_to_name(node, out);
    }

    /* <opts> */
    /* <name>___<opts>*/
    cf_print_keys.ctx = pck.tree_ctx;
    cf_print_keys.pf = pck.fps.print_keys;

    trp_print_opts_keys(node->name, indent.btw_name_opts, cf_print_keys, out);

    /* <opts>__<type> */
    if (indent.btw_opts_type > 0) {
        ly_print_(out, "%*c", indent.btw_opts_type, ' ');
    }

    /* <type> */
    trp_print_type(node->type, out);

    /* <type>__<iffeatures> */
    if (indent.btw_type_iffeatures > 0) {
        ly_print_(out, "%*c", indent.btw_type_iffeatures, ' ');
    }

    /* <iffeatures> */
    cf_print_iffeatures.ctx = pck.tree_ctx;
    cf_print_iffeatures.pf = pck.fps.print_features_names;

    trp_print_iffeatures(node->iffeatures, cf_print_iffeatures, out);
}

/**
 * @brief Print keyword based on trt_keyword_stmt.type.
 * @param[in] ks is keyword statement to print.
 * @param[in,out] out is output handler
 */
static void
trt_print_keyword_stmt_begin(struct trt_keyword_stmt ks, struct ly_out *out)
{
    if (!strcmp(ks.section_name, TRD_KEYWORD_MODULE) ||
            !strcmp(ks.section_name, TRD_KEYWORD_SUBMODULE)) {
        ly_print_(out, "%s: ", ks.section_name);
        return;
    }

    ly_print_(out, "%*c", TRD_INDENT_LINE_BEGIN, ' ');
    if (ks.argument) {
        ly_print_(out, "%s ", ks.section_name);
    } else {
        ly_print_(out, "%s", ks.section_name);
    }
}

/**
 * @brief Print trt_keyword_stmt.str which is string of name or path.
 * @param[in] ks is keyword statement structure.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trt_print_keyword_stmt_str(struct trt_keyword_stmt ks, size_t mll, struct ly_out *out)
{
    uint32_t ind_initial;
    uint32_t ind_divided;
    /* flag if path must be splitted to more lines */
    ly_bool linebreak_was_set;
    /* flag if at least one subpath was printed */
    ly_bool subpath_printed;
    /* the sum of the sizes of the substrings on the current line */
    uint32_t how_far;
    /* pointer to start of the subpath */
    const char *sub_ptr;
    /* size of subpath from sub_ptr */
    size_t sub_len;

    if ((!ks.argument) || (ks.argument[0] == '\0')) {
        return;
    }

    /* module name cannot be splitted */
    if (!strcmp(ks.section_name, TRD_KEYWORD_MODULE) || !strcmp(ks.section_name, TRD_KEYWORD_SUBMODULE)) {
        ly_print_(out, "%s", ks.argument);
        return;
    }

    /* after -> for trd_keyword_stmt_body do */

    /* set begin indentation */
    ind_initial = TRD_INDENT_LINE_BEGIN + strlen(ks.section_name) + 1;
    ind_divided = ind_initial + TRD_INDENT_LONG_LINE_BREAK;
    linebreak_was_set = 0;
    subpath_printed = 0;
    how_far = 0;
    sub_ptr = ks.argument;
    sub_len = 0;

    while (sub_ptr[0] != '\0') {
        uint32_t ind;
        /* skip slash */
        const char *tmp = sub_ptr[0] == '/' ? sub_ptr + 1 : sub_ptr;

        /* get position of the end of substr */
        tmp = strchr(tmp, '/');
        /* set correct size if this is a last substring */
        sub_len = !tmp ? strlen(sub_ptr) : (size_t)(tmp - sub_ptr);
        /* actualize sum of the substring's sizes on the current line */
        how_far += sub_len;
        /* correction due to colon character if it this is last substring */
        how_far = *(sub_ptr + sub_len) == '\0' ? how_far + 1 : how_far;
        /* choose indentation which depends on
         * whether the string is printed on multiple lines or not
         */
        ind = linebreak_was_set ? ind_divided : ind_initial;
        if (ind + how_far <= mll) {
            /* printing before max line length */
            sub_ptr = trg_print_substr(sub_ptr, sub_len, out);
            subpath_printed = 1;
        } else {
            /* printing on new line */
            if (subpath_printed == 0) {
                /* first subpath is too long
                 * but print it at first line anyway
                 */
                sub_ptr = trg_print_substr(sub_ptr, sub_len, out);
                subpath_printed = 1;
                continue;
            }
            ly_print_(out, "\n");
            ly_print_(out, "%*c", ind_divided, ' ');
            linebreak_was_set = 1;
            sub_ptr = trg_print_substr(sub_ptr, sub_len, out);
            how_far = sub_len;
            subpath_printed = 1;
        }
    }
}

/**
 * @brief Print separator based on trt_keyword_stmt.type
 * @param[in] ks is keyword statement structure.
 * @param[in,out] out is output handler.
 */
static void
trt_print_keyword_stmt_end(struct trt_keyword_stmt ks, struct ly_out *out)
{
    if (!strcmp(ks.section_name, TRD_KEYWORD_MODULE) || !strcmp(ks.section_name, TRD_KEYWORD_SUBMODULE)) {
        return;
    } else if (ks.has_node) {
        ly_print_(out, ":");
    }
}

/**
 * @brief Print entire struct trt_keyword_stmt structure.
 * @param[in] ks is item to print.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trp_print_keyword_stmt(struct trt_keyword_stmt ks, size_t mll, struct ly_out *out)
{
    assert(ks.section_name);
    trt_print_keyword_stmt_begin(ks, out);
    trt_print_keyword_stmt_str(ks, mll, out);
    trt_print_keyword_stmt_end(ks, out);
}

/**********************************************************************
 * Main trp functions
 *********************************************************************/

/**
 * @brief Printing one line including wrapper and node
 * which can be incomplete (divided).
 * @param[in] node is \<node\> representation.
 * @param[in] pck contains special printing functions callback.
 * @param[in] indent contains wrapper and indent in node numbers.
 * @param[in,out] out is output handler.
 */
static void
trp_print_line(const struct trt_node *node, struct trt_pck_print pck, struct trt_pck_indent indent, struct ly_out *out)
{
    trp_print_wrapper(indent.wrapper, out);
    trp_print_node(node, pck, indent.in_node, out);
}

/**
 * @brief Printing one line including wrapper and
 * \<status\>--\<flags\> \<name\>\<option_mark\>.
 * @param[in] node is \<node\> representation.
 * @param[in] wr is wrapper for printing indentation before node.
 * @param[in] out is output handler.
 */
static void
trp_print_line_up_to_node_name(const struct trt_node *node, struct trt_wrapper wr, struct ly_out *out)
{
    trp_print_wrapper(wr, out);
    trp_print_node_up_to_name(node, out);
}

/**
 * @brief Check if leafref target must be change to string 'leafref'
 * because his target string is too long.
 * @param[in] node containing leafref target.
 * @param[in] wr is wrapper for printing indentation before node.
 * @param[in] mll is max line length.
 * @param[in] out is output handler.
 * @return true if leafref must be changed to string 'leafref'.
 */
static ly_bool
trp_leafref_target_is_too_long(const struct trt_node *node, struct trt_wrapper wr, size_t mll, struct ly_out *out)
{
    size_t type_len;
    struct ly_out_clb_arg *data;

    if (node->type.type != TRD_TYPE_TARGET) {
        return 0;
    }

    /* set ly_out to counting characters */
    data = out->method.clb.arg;

    data->counter = 0;
    data->mode = TRD_CHAR_COUNT;
    /* count number of printed bytes */
    trp_print_wrapper(wr, out);
    ly_print_(out, "%*c", TRD_INDENT_BTW_SIBLINGS, ' ');
    trp_print_divided_node_up_to_name(node, out);
    data->mode = TRD_PRINT;
    type_len = strlen(node->type.str);

    return data->counter + type_len > mll;
}

/**
 * @brief Get default indent in node based on node values.
 * @param[in] node is \<node\> representation.
 * @return Default indent in node assuming that the node
 * will not be divided.
 */
static struct trt_indent_in_node
trp_default_indent_in_node(const struct trt_node *node)
{
    struct trt_indent_in_node ret;
    uint32_t opts_len = 0;

    ret.type = TRD_INDENT_IN_NODE_NORMAL;

    /* btw_name_opts */
    ret.btw_name_opts = node->name.keys ? TRD_INDENT_BEFORE_KEYS : 0;

    /* btw_opts_type */
    if (!(TRP_TRT_TYPE_IS_EMPTY(node->type))) {
        if (trp_mark_is_used(node->name)) {
            opts_len += node->name.add_opts ? strlen(node->name.add_opts) : 0;
            opts_len += node->name.opts ? strlen(node->name.opts) : 0;
            ret.btw_opts_type = TRD_INDENT_BEFORE_TYPE > opts_len ? 1 : TRD_INDENT_BEFORE_TYPE - opts_len;
        } else {
            ret.btw_opts_type = TRD_INDENT_BEFORE_TYPE;
        }
    } else {
        ret.btw_opts_type = 0;
    }

    /* btw_type_iffeatures */
    ret.btw_type_iffeatures = node->iffeatures.type == TRD_IFF_PRESENT ? TRD_INDENT_BEFORE_IFFEATURES : 0;

    return ret;
}

/**
 * @brief Setting linebreaks in trt_indent_in_node.
 *
 * The order where the linebreak tag can be placed is from the end.
 *
 * @param[in] indent containing alignment lengths
 * or already linebreak marks.
 * @return indent with a newly placed linebreak tag.
 * @return .type set to TRD_INDENT_IN_NODE_FAILED if it is not possible
 * to place a more linebreaks.
 */
static struct trt_indent_in_node
trp_indent_in_node_place_break(struct trt_indent_in_node indent)
{
    /* somewhere must be set a line break in node */
    struct trt_indent_in_node ret = indent;

    /* gradually break the node from the end */
    if ((indent.btw_type_iffeatures != TRD_LINEBREAK) && (indent.btw_type_iffeatures != 0)) {
        ret.btw_type_iffeatures = TRD_LINEBREAK;
    } else if ((indent.btw_opts_type != TRD_LINEBREAK) && (indent.btw_opts_type != 0)) {
        ret.btw_opts_type = TRD_LINEBREAK;
    } else if ((indent.btw_name_opts != TRD_LINEBREAK) && (indent.btw_name_opts != 0)) {
        /* set line break between name and opts */
        ret.btw_name_opts = TRD_LINEBREAK;
    } else {
        /* it is not possible to place a more line breaks,
         * unfortunately the max_line_length constraint is violated
         */
        ret.type = TRD_INDENT_IN_NODE_FAILED;
    }
    return ret;
}

/**
 * @brief Set the first half of the node based on the linebreak mark.
 *
 * Items in the second half of the node will be empty.
 *
 * @param[in,out] innod contains information in which part of the \<node\>
 * the first half ends. Set first half of the node, indent is unchanged.
 */
static void
trp_first_half_node(struct trt_pair_indent_node *innod)
{
    if (innod->indent.btw_name_opts == TRD_LINEBREAK) {
        innod->node.type = TRP_EMPTY_TRT_TYPE;
        innod->node.iffeatures = TRP_EMPTY_TRT_IFFEATURES;
    } else if (innod->indent.btw_opts_type == TRD_LINEBREAK) {
        innod->node.type = TRP_EMPTY_TRT_TYPE;
        innod->node.iffeatures = TRP_EMPTY_TRT_IFFEATURES;
    } else if (innod->indent.btw_type_iffeatures == TRD_LINEBREAK) {
        innod->node.iffeatures = TRP_EMPTY_TRT_IFFEATURES;
    }
}

/**
 * @brief Set the second half of the node based on the linebreak mark.
 *
 * Items in the first half of the node will be empty.
 * Indentations belonging to the first node will be reset to zero.
 *
 * @param[in,out] innod contains information in which part of the \<node\>
 * the second half starts. Set second half of the node, indent is newly set.
 */
static void
trp_second_half_node(struct trt_pair_indent_node *innod)
{
    if (innod->indent.btw_name_opts < 0) {
        /* Logically, the information up to token <opts> should
         * be deleted, but the the trp_print_node function needs it to
         * create the correct indent.
         */
        innod->indent.btw_name_opts = 0;
        innod->indent.btw_opts_type = TRP_TRT_TYPE_IS_EMPTY(innod->node.type) ? 0 : TRD_INDENT_BEFORE_TYPE;
        innod->indent.btw_type_iffeatures = innod->node.iffeatures.type == TRD_IFF_NON_PRESENT ? 0 : TRD_INDENT_BEFORE_IFFEATURES;
    } else if (innod->indent.btw_opts_type == TRD_LINEBREAK) {
        innod->indent.btw_name_opts = 0;
        innod->indent.btw_opts_type = 0;
        innod->indent.btw_type_iffeatures = innod->node.iffeatures.type == TRD_IFF_NON_PRESENT ? 0 : TRD_INDENT_BEFORE_IFFEATURES;
    } else if (innod->indent.btw_type_iffeatures == TRD_LINEBREAK) {
        innod->node.type = TRP_EMPTY_TRT_TYPE;
        innod->indent.btw_name_opts = 0;
        innod->indent.btw_opts_type = 0;
        innod->indent.btw_type_iffeatures = 0;
    }
}

/**
 * @brief Get the correct alignment for the node.
 *
 * This function is recursively called itself. It's like a backend
 * function for a function ::trp_try_normal_indent_in_node().
 *
 * @param[in] pck contains speciall callback functions for printing.
 * @param[in] wrapper contains information about '|' context.
 * @param[in] mll is max line length.
 * @param[in,out] cnt counting number of characters to print.
 * @param[in,out] out is output handler.
 * @param[in,out] innod pair of node and indentation numbers of that node.
 */
static void
trp_try_normal_indent_in_node_(struct trt_pck_print pck, struct trt_wrapper wrapper, size_t mll, size_t *cnt,
        struct ly_out *out, struct trt_pair_indent_node *innod)
{
    trp_print_line(&innod->node, pck, TRP_INIT_PCK_INDENT(wrapper, innod->indent), out);

    if (*cnt <= mll) {
        /* success */
        return;
    } else {
        innod->indent = trp_indent_in_node_place_break(innod->indent);
        if (innod->indent.type != TRD_INDENT_IN_NODE_FAILED) {
            /* erase information in node due to line break */
            trp_first_half_node(innod);
            /* check if line fits, recursive call */
            *cnt = 0;
            trp_try_normal_indent_in_node_(pck, wrapper, mll, cnt, out, innod);
            /* make sure that the result will be with the status divided
             * or eventually with status failed */
            innod->indent.type = innod->indent.type == TRD_INDENT_IN_NODE_FAILED ? TRD_INDENT_IN_NODE_FAILED : TRD_INDENT_IN_NODE_DIVIDED;
        }
        return;
    }
}

/**
 * @brief Get the correct alignment for the node.
 *
 * @param[in] node is \<node\> representation.
 * @param[in] pck contains speciall callback functions for printing.
 * @param[in] indent contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 * @param[out] innod If the node does not fit in the line, some indent variable has negative value as a line break sign
 * and therefore ::TRD_INDENT_IN_NODE_DIVIDED is set.
 * If the node fits into the line, all indent variables values has non-negative number and therefore
 * ::TRD_INDENT_IN_NODE_NORMAL is set.
 * If the node does not fit into the line, all indent variables has negative or zero values, function failed
 * and therefore ::TRD_INDENT_IN_NODE_FAILED is set.
 */
static void
trp_try_normal_indent_in_node(const struct trt_node *node, struct trt_pck_print pck, struct trt_pck_indent indent,
        size_t mll, struct ly_out *out, struct trt_pair_indent_node *innod)
{
    struct ly_out_clb_arg *data;

    *innod = TRP_INIT_PAIR_INDENT_NODE(indent.in_node, *node);

    /* set ly_out to counting characters */
    data = out->method.clb.arg;

    data->counter = 0;
    data->mode = TRD_CHAR_COUNT;
    trp_try_normal_indent_in_node_(pck, indent.wrapper, mll, &data->counter, out, innod);
    data->mode = TRD_PRINT;
}

/**
 * @brief Auxiliary function for ::trp_print_entire_node()
 * that prints split nodes.
 * @param[in] node is node representation.
 * @param[in] ppck contains speciall callback functions for printing.
 * @param[in] ipck contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trp_print_divided_node(const struct trt_node *node, struct trt_pck_print ppck, struct trt_pck_indent ipck, size_t mll, struct ly_out *out)
{
    ly_bool entire_node_was_printed;
    struct trt_pair_indent_node innod;

    trp_try_normal_indent_in_node(node, ppck, ipck, mll, out, &innod);

    if (innod.indent.type == TRD_INDENT_IN_NODE_FAILED) {
        /* nothing can be done, continue as usual */
        innod.indent.type = TRD_INDENT_IN_NODE_DIVIDED;
    }

    trp_print_line(&innod.node, ppck, TRP_INIT_PCK_INDENT(ipck.wrapper, innod.indent), out);
    entire_node_was_printed = trp_indent_in_node_are_eq(ipck.in_node, innod.indent);

    if (!entire_node_was_printed) {
        ly_print_(out, "\n");
        /* continue with second half node */
        innod.node = *node;
        trp_second_half_node(&innod);
        /* continue with printing node */
        trp_print_divided_node(&innod.node, ppck, TRP_INIT_PCK_INDENT(ipck.wrapper, innod.indent), mll, out);
    } else {
        return;
    }
}

/**
 * @brief Printing of the wrapper and the whole node,
 * which can be divided into several lines.
 * @param[in] node_p is node representation.
 * @param[in] ppck contains speciall callback functions for printing.
 * @param[in] ipck contains wrapper and indent in node numbers.
 * @param[in] mll is max line length.
 * @param[in,out] out is output handler.
 */
static void
trp_print_entire_node(const struct trt_node *node_p, struct trt_pck_print ppck, struct trt_pck_indent ipck, size_t mll,
        struct ly_out *out)
{
    struct trt_pair_indent_node innod;
    struct trt_pck_indent tmp;
    struct trt_node node;

    node = *node_p;
    if (trp_leafref_target_is_too_long(&node, ipck.wrapper, mll, out)) {
        node.type.type = TRD_TYPE_LEAFREF;
    }

    /* check if normal indent is possible */
    trp_try_normal_indent_in_node(&node, ppck, ipck, mll, out, &innod);

    if (innod.indent.type == TRD_INDENT_IN_NODE_NORMAL) {
        /* node fits to one line */
        trp_print_line(&node, ppck, ipck, out);
    } else if (innod.indent.type == TRD_INDENT_IN_NODE_DIVIDED) {
        /* node will be divided */
        /* print first half */
        tmp = TRP_INIT_PCK_INDENT(ipck.wrapper, innod.indent);
        /* pretend that this is normal node */
        tmp.in_node.type = TRD_INDENT_IN_NODE_NORMAL;

        trp_print_line(&innod.node, ppck, tmp, out);
        ly_print_(out, "\n");

        /* continue with second half on new line */
        innod.node = node;
        trp_second_half_node(&innod);
        tmp = TRP_INIT_PCK_INDENT(trp_wrapper_if_last_sibling(ipck.wrapper, node.last_one), innod.indent);

        trp_print_divided_node(&innod.node, ppck, tmp, mll, out);
    } else if (innod.indent.type == TRD_INDENT_IN_NODE_FAILED) {
        /* node name is too long */
        trp_print_line_up_to_node_name(&node, ipck.wrapper, out);

        if (trp_node_body_is_empty(&node)) {
            return;
        } else {
            ly_print_(out, "\n");

            innod.node = node;
            trp_second_half_node(&innod);
            innod.indent.type = TRD_INDENT_IN_NODE_DIVIDED;
            tmp = TRP_INIT_PCK_INDENT(trp_wrapper_if_last_sibling(ipck.wrapper, node.last_one), innod.indent);

            trp_print_divided_node(&innod.node, ppck, tmp, mll, out);
        }
    }
}

/**
 * @brief Check if parent-stmt is valid for printing extensinon.
 *
 * @param[in] lysc_tree flag if ext is from compiled tree.
 * @param[in] ext Extension to check.
 * @return 1 if extension is valid.
 */
static ly_bool
trp_ext_parent_is_valid(ly_bool lysc_tree, void *ext)
{
    enum ly_stmt parent_stmt;

    if (lysc_tree) {
        parent_stmt = ((struct lysc_ext_instance *)ext)->parent_stmt;
    } else {
        parent_stmt = ((struct lysp_ext_instance *)ext)->parent_stmt;
    }
    if ((parent_stmt & LY_STMT_OP_MASK) || (parent_stmt & LY_STMT_DATA_NODE_MASK) ||
            (parent_stmt & LY_STMT_SUBMODULE) || parent_stmt & LY_STMT_MODULE) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Check if printer_tree can use node extension.
 *
 * @param[in] lysc_tree Flag if @p node is compiled.
 * @param[in] node to check. Its type is lysc_node or lysp_node.
 * @return Pointer to extension instance which printer_tree can used.
 */
static void *
trp_ext_is_present(ly_bool lysc_tree, const void *node)
{
    const struct lysp_node *pn;
    const struct lysc_node *cn;
    LY_ARRAY_COUNT_TYPE i;
    void *ret = NULL;

    if (!node) {
        return NULL;
    }

    if (lysc_tree) {
        cn = (const struct lysc_node *)node;
        LY_ARRAY_FOR(cn->exts, i) {
            if (!(cn->exts && cn->exts->def->plugin && cn->exts->def->plugin->printer_ctree)) {
                continue;
            }
            if (!trp_ext_parent_is_valid(1, &cn->exts[i])) {
                continue;
            }
            ret = &cn->exts[i];
            break;
        }
    } else {
        pn = (const struct lysp_node *)node;
        LY_ARRAY_FOR(pn->exts, i) {
            if (!(pn->exts && pn->exts->record && pn->exts->record->plugin.printer_ptree)) {
                continue;
            }
            if (!trp_ext_parent_is_valid(0, &pn->exts[i])) {
                continue;
            }
            ret = &pn->exts[i];
            break;
        }
    }

    return ret;
}

/**
 * @brief Check if printer_tree can use node extension.
 *
 * @param[in] tc Context with current node.
 * @return 1 if some extension for printer_tree is valid.
 */
static ly_bool
trp_ext_is_present_in_node(struct trt_tree_ctx *tc)
{
    if (tc->lysc_tree && trp_ext_is_present(tc->lysc_tree, tc->cn)) {
        return 1;
    } else if (trp_ext_is_present(tc->lysc_tree, tc->pn)) {
        return 1;
    }

    return 0;
}

/**
 * @brief Release allocated memory and set pointers to NULL.
 *
 * @param[in,out] overr is override structure to release.
 * @param[out] filtered is flag to reset.
 */
static void
trp_ext_free_node_override(struct lyplg_ext_sprinter_tree_node_override *overr, ly_bool *filtered)
{
    *filtered = 0;
    overr->flags = NULL;
    overr->add_opts = NULL;
}

/**
 * @brief Release private plugin data.
 *
 * @param[in,out] plug_ctx is plugin context.
 */
static void
trp_ext_free_plugin_ctx(struct lyspr_tree_ctx *plug_ctx)
{
    LY_ARRAY_FREE(plug_ctx->schemas);
    if (plug_ctx->free_plugin_priv) {
        plug_ctx->free_plugin_priv(plug_ctx->plugin_priv);
    }
}

/**********************************************************************
 * trop and troc getters
 *********************************************************************/

/**
 * @brief Get nodetype.
 * @param[in] node is any lysp_node.
 */
static uint16_t
trop_nodetype(const void *node)
{
    return ((const struct lysp_node *)node)->nodetype;
}

/**
 * @brief Get sibling.
 * @param[in] node is any lysp_node.
 */
static const void *
trop_next(const void *node)
{
    return ((const struct lysp_node *)node)->next;
}

/**
 * @brief Get parent.
 * @param[in] node is any lysp_node.
 */
static const void *
trop_parent(const void *node)
{
    return ((const struct lysp_node *)node)->parent;
}

/**
 * @brief Try to get child.
 * @param[in] node is any lysp_node.
 */
static const void *
trop_child(const void *node)
{
    return lysp_node_child(node);
}

/**
 * @brief Try to get action.
 * @param[in] node is any lysp_node.
 */
static const void *
trop_actions(const void *node)
{
    return lysp_node_actions(node);
}

/**
 * @brief Try to get action.
 * @param[in] node must be of type lysp_node_action.
 */
static const void *
trop_action_input(const void *node)
{
    return &((const struct lysp_node_action *)node)->input;
}

/**
 * @brief Try to get action.
 * @param[in] node must be of type lysp_node_action.
 */
static const void *
trop_action_output(const void *node)
{
    return &((const struct lysp_node_action *)node)->output;
}

/**
 * @brief Try to get action.
 * @param[in] node is any lysp_node.
 */
static const void *
trop_notifs(const void *node)
{
    return lysp_node_notifs(node);
}

/**
 * @brief Fill struct tro_getters with @ref TRP_trop getters
 * which are adapted to lysp nodes.
 */
static struct tro_getters
trop_init_getters(void)
{
    return (struct tro_getters) {
               .nodetype = trop_nodetype,
               .next = trop_next,
               .parent = trop_parent,
               .child = trop_child,
               .actions = trop_actions,
               .action_input = trop_action_input,
               .action_output = trop_action_output,
               .notifs = trop_notifs
    };
}

/**
 * @brief Get nodetype.
 * @param[in] node is any lysc_node.
 */
static uint16_t
troc_nodetype(const void *node)
{
    return ((const struct lysc_node *)node)->nodetype;
}

/**
 * @brief Get sibling.
 * @param[in] node is any lysc_node.
 */
static const void *
troc_next(const void *node)
{
    return ((const struct lysc_node *)node)->next;
}

/**
 * @brief Get parent.
 * @param[in] node is any lysc_node.
 */
static const void *
troc_parent(const void *node)
{
    return ((const struct lysc_node *)node)->parent;
}

/**
 * @brief Try to get child.
 * @param[in] node is any lysc_node.
 */
static const void *
troc_child(const void *node)
{
    return lysc_node_child(node);
}

/**
 * @brief Try to get action.
 * @param[in] node is any lysc_node.
 */
static const void *
troc_actions(const void *node)
{
    return lysc_node_actions(node);
}

/**
 * @brief Try to get action.
 * @param[in] node must be of type lysc_node_action.
 */
static const void *
troc_action_input(const void *node)
{
    return &((const struct lysc_node_action *)node)->input;
}

/**
 * @brief Try to get action.
 * @param[in] node must be of type lysc_node_action.
 */
static const void *
troc_action_output(const void *node)
{
    return &((const struct lysc_node_action *)node)->output;
}

/**
 * @brief Try to get action.
 * @param[in] node is any lysc_node.
 */
static const void *
troc_notifs(const void *node)
{
    return lysc_node_notifs(node);
}

/**
 * @brief Fill struct tro_getters with @ref TRP_troc getters
 * which are adapted to lysc nodes.
 */
static struct tro_getters
troc_init_getters(void)
{
    return (struct tro_getters) {
               .nodetype = troc_nodetype,
               .next = troc_next,
               .parent = troc_parent,
               .child = troc_child,
               .actions = troc_actions,
               .action_input = troc_action_input,
               .action_output = troc_action_output,
               .notifs = troc_notifs
    };
}

/**********************************************************************
 * tro functions
 *********************************************************************/

/**
 * @brief Call override function for @p node.
 *
 * @param[in] lysc_tree if @p node is compiled.
 * @param[in] node to create override.
 * @param[in] erase_node_overr if override structure must be reseted.
 * @param[in,out] plc current plugin context.
 * @return pointer to override structure or NULL. Override structure in @p plc is updated too.
 */
static struct lyplg_ext_sprinter_tree_node_override *
tro_set_node_overr(ly_bool lysc_tree, const void *node, ly_bool erase_node_overr, struct trt_plugin_ctx *plc)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyplg_ext_sprinter_tree_node_override *no;
    struct lyspr_tree_ctx *plug_ctx;
    struct lysc_ext_instance *ce;
    struct lysp_ext_instance *pe;

    if (erase_node_overr) {
        trp_ext_free_node_override(&plc->node_overr, &plc->filtered);
    }
    no = &plc->node_overr;
    if (!plc->ctx && lysc_tree && (ce = trp_ext_is_present(lysc_tree, node))) {
        rc = ce->def->plugin->printer_ctree(ce, NULL, &no->flags, &no->add_opts);
    } else if (!plc->ctx && (pe = trp_ext_is_present(lysc_tree, node))) {
        rc = pe->record->plugin.printer_ptree(pe, NULL, &no->flags, &no->add_opts);
    } else if (plc->ctx) {
        if (plc->schema && plc->schema->compiled && plc->schema->cn_overr) {
            rc = plc->schema->cn_overr(node, plc->ctx->plugin_priv, &plc->filtered, &no->flags, &no->add_opts);
        } else if (plc->schema && plc->schema->pn_overr) {
            rc = plc->schema->pn_overr(node, plc->ctx->plugin_priv, &plc->filtered, &no->flags, &no->add_opts);
        } else {
            no = NULL;
        }
        if (trp_ext_is_present(lysc_tree, node)) {
            plug_ctx = plc->ctx;
            plc->ctx = NULL;
            tro_set_node_overr(lysc_tree, node, 0, plc);
            plc->ctx = plug_ctx;
        }
    } else {
        no = NULL;
    }

    if (rc) {
        plc->last_error = rc;
        no = NULL;
    }

    return no;
}

/**
 * @brief Get next sibling of the current node.
 *
 * This is a general algorithm that is able to
 * work with lysp_node or lysc_node.
 *
 * @param[in] node points to lysp_node or lysc_node.
 * @param[in] tc current tree context.
 * @return next sibling node.
 */
static const void *
tro_next_sibling(const void *node, const struct trt_tree_ctx *tc)
{
    struct tro_getters get;
    struct trt_plugin_ctx plugin_ctx;
    const void *tmp, *parent, *sibl;

    assert(node);

    get = tc->lysc_tree ? troc_init_getters() : trop_init_getters();

    if (get.nodetype(node) & (LYS_RPC | LYS_ACTION)) {
        if ((tmp = get.next(node))) {
            /* next action exists */
            sibl = tmp;
        } else if ((parent = get.parent(node))) {
            /* maybe if notif exists as sibling */
            sibl = get.notifs(parent);
        } else {
            sibl = NULL;
        }
    } else if (get.nodetype(node) & LYS_INPUT) {
        if ((parent = get.parent(node))) {
            /* if output action has data */
            if (get.child(get.action_output(parent))) {
                /* then next sibling is output action */
                sibl = get.action_output(parent);
            } else {
                /* input action cannot have siblings other
                 * than output action.
                 */
                sibl = NULL;
            }
        } else {
            /* there is no way how to get output action */
            sibl = NULL;
        }
    } else if (get.nodetype(node) & LYS_OUTPUT) {
        /* output action cannot have siblings */
        sibl = NULL;
    } else if (get.nodetype(node) & LYS_NOTIF) {
        /* must have as a sibling only notif */
        sibl = get.next(node);
    } else {
        /* for rest of nodes */
        if ((tmp = get.next(node))) {
            /* some sibling exists */
            sibl = tmp;
        } else if ((parent = get.parent(node))) {
            /* Action and notif are siblings too.
             * They can be reached through parent.
             */
            if ((tmp = get.actions(parent))) {
                /* next sibling is action */
                sibl = tmp;
            } else if ((tmp = get.notifs(parent))) {
                /* next sibling is notif */
                sibl = tmp;
            } else {
                /* sibling not exists */
                sibl = NULL;
            }
        } else {
            /* sibling not exists */
            sibl = NULL;
        }
    }

    plugin_ctx = tc->plugin_ctx;
    if (sibl && tro_set_node_overr(tc->lysc_tree, sibl, 1, &plugin_ctx) && plugin_ctx.filtered) {
        return tro_next_sibling(sibl, tc);
    }

    return sibl;
}

/**
 * @brief Get child of the current node.
 *
 * This is a general algorithm that is able to
 * work with lysp_node or lysc_node.
 *
 * @param[in] node points to lysp_node or lysc_node.
 * @param[in] tc current tree context.
 * @return child node.
 */
static const void *
tro_next_child(const void *node, const struct trt_tree_ctx *tc)
{
    struct tro_getters get;
    struct trt_plugin_ctx plugin_ctx;
    const void *tmp, *child;

    assert(node);

    get = tc->lysc_tree ? troc_init_getters() : trop_init_getters();

    if (get.nodetype(node) & (LYS_ACTION | LYS_RPC)) {
        if (get.child(get.action_input(node))) {
            /* go to LYS_INPUT */
            child = get.action_input(node);
        } else if (get.child(get.action_output(node))) {
            /* go to LYS_OUTPUT */
            child = get.action_output(node);
        } else {
            /* input action and output action have no data */
            child = NULL;
        }
    } else {
        if ((tmp = get.child(node))) {
            child = tmp;
        } else {
            /* current node can't have children or has no children */
            /* but maybe has some actions or notifs */
            if ((tmp = get.actions(node))) {
                child = tmp;
            } else if ((tmp = get.notifs(node))) {
                child = tmp;
            } else {
                child = NULL;
            }
        }
    }

    plugin_ctx = tc->plugin_ctx;
    if (child && tro_set_node_overr(tc->lysc_tree, child, 1, &plugin_ctx) && plugin_ctx.filtered) {
        return tro_next_sibling(child, tc);
    }

    return child;
}

/**
 * @brief Get new trt_parent_cache if we apply the transfer
 * to the child node in the tree.
 * @param[in] ca is parent cache for current node.
 * @param[in] tc contains current tree node.
 * @return Cache for the current node.
 */
static struct trt_parent_cache
tro_parent_cache_for_child(struct trt_parent_cache ca, const struct trt_tree_ctx *tc)
{
    struct trt_parent_cache ret = TRP_EMPTY_PARENT_CACHE;

    if (!tc->lysc_tree) {
        const struct lysp_node *pn = tc->pn;

        ret.ancestor =
                pn->nodetype & (LYS_INPUT) ? TRD_ANCESTOR_RPC_INPUT :
                pn->nodetype & (LYS_OUTPUT) ? TRD_ANCESTOR_RPC_OUTPUT :
                pn->nodetype & (LYS_NOTIF) ? TRD_ANCESTOR_NOTIF :
                ca.ancestor;

        ret.lys_status =
                pn->flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT) ? pn->flags :
                ca.lys_status;

        ret.lys_config =
                ca.ancestor == TRD_ANCESTOR_RPC_INPUT ? 0 : /* because <flags> will be -w */
                ca.ancestor == TRD_ANCESTOR_RPC_OUTPUT ? LYS_CONFIG_R :
                pn->flags & (LYS_CONFIG_R | LYS_CONFIG_W) ? pn->flags :
                ca.lys_config;

        ret.last_list =
                pn->nodetype & (LYS_LIST) ? (struct lysp_node_list *)pn :
                ca.last_list;
    }

    return ret;
}

/**
 * @brief Transformation of the Schema nodes flags to
 * Tree diagram \<status\>.
 * @param[in] flags is node's flags obtained from the tree.
 */
static char *
tro_flags2status(uint16_t flags)
{
    return flags & LYS_STATUS_OBSLT ? "o" :
           flags & LYS_STATUS_DEPRC ? "x" :
           "+";
}

/**
 * @brief Transformation of the Schema nodes flags to Tree diagram
 * \<flags\> but more specifically 'ro' or 'rw'.
 * @param[in] flags is node's flags obtained from the tree.
 */
static char *
tro_flags2config(uint16_t flags)
{
    return flags & LYS_CONFIG_R ? TRD_FLAGS_TYPE_RO :
           flags & LYS_CONFIG_W ? TRD_FLAGS_TYPE_RW :
           TRD_FLAGS_TYPE_EMPTY;
}

/**
 * @brief Print current node's iffeatures.
 * @param[in] tc is tree context.
 * @param[in,out] out is output handler.
 */
static void
tro_print_features_names(const struct trt_tree_ctx *tc, struct ly_out *out)
{
    const struct lysp_qname *iffs;

    if (tc->lysc_tree) {
        assert(TRP_TREE_CTX_LYSP_NODE_PRESENT(tc->cn));
        iffs = TRP_TREE_CTX_GET_LYSP_NODE(tc->cn)->iffeatures;
    } else {
        iffs = tc->pn->iffeatures;
    }
    LY_ARRAY_COUNT_TYPE i;

    LY_ARRAY_FOR(iffs, i) {
        if (i == 0) {
            ly_print_(out, "%s", iffs[i].str);
        } else {
            ly_print_(out, ",%s", iffs[i].str);
        }
    }

}

/**
 * @brief Print current list's keys.
 *
 * Well, actually printing keys in the lysp_tree is trivial,
 * because char* points to all keys. However, special functions have
 * been reserved for this, because in principle the list of elements
 * can have more implementations.
 *
 * @param[in] tc is tree context.
 * @param[in,out] out is output handler.
 */
static void
tro_print_keys(const struct trt_tree_ctx *tc, struct ly_out *out)
{
    const struct lysp_node_list *list;

    if (tc->lysc_tree) {
        assert(TRP_TREE_CTX_LYSP_NODE_PRESENT(tc->cn));
        list = (const struct lysp_node_list *)TRP_TREE_CTX_GET_LYSP_NODE(tc->cn);
    } else {
        list = (const struct lysp_node_list *)tc->pn;
    }
    assert(list->nodetype & LYS_LIST);

    if (trg_charptr_has_data(list->key)) {
        ly_print_(out, "%s", list->key);
    }
}

/**
 * @brief Get address of the current node.
 * @param[in] tc contains current node.
 * @return Address of lysc_node or lysp_node, or NULL.
 */
static const void *
tro_tree_ctx_get_node(const struct trt_tree_ctx *tc)
{
    return tc->lysc_tree ?
           (const void *)tc->cn :
           (const void *)tc->pn;
}

/**
 * @brief Get address of current node's child.
 * @param[in,out] tc contains current node.
 */
static const void *
tro_tree_ctx_get_child(const struct trt_tree_ctx *tc)
{
    if (!tro_tree_ctx_get_node(tc)) {
        return NULL;
    }

    if (tc->lysc_tree) {
        return lysc_node_child(tc->cn);
    } else {
        return lysp_node_child(tc->pn);
    }
}

/**
 * @brief Get rpcs section if exists.
 * @param[in,out] tc is tree context.
 * @return Section representation if it exists. The @p tc is modified
 * and his pointer points to the first node in rpcs section.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
tro_modi_get_rpcs(struct trt_tree_ctx *tc)
{
    assert(tc);
    const void *actions;
    struct trt_keyword_stmt ret = {0};

    if (tc->lysc_tree) {
        actions = tc->cmod->rpcs;
        if (actions) {
            tc->cn = actions;
        }
    } else {
        actions = tc->pmod->rpcs;
        if (actions) {
            tc->pn = actions;
            tc->tpn = tc->pn;
        }
    }

    if (actions) {
        tc->section = TRD_SECT_RPCS;
        ret.section_name = TRD_KEYWORD_RPC;
        ret.has_node = tro_tree_ctx_get_node(tc) ? 1 : 0;
    }

    return ret;
}

/**
 * @brief Get notification section if exists
 * @param[in,out] tc is tree context.
 * @return Section representation if it exists.
 * The @p tc is modified and his pointer points to the
 * first node in notification section.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
tro_modi_get_notifications(struct trt_tree_ctx *tc)
{
    assert(tc);
    const void *notifs;
    struct trt_keyword_stmt ret = {0};

    if (tc->lysc_tree) {
        notifs = tc->cmod->notifs;
        if (notifs) {
            tc->cn = notifs;
        }
    } else {
        notifs = tc->pmod->notifs;
        if (notifs) {
            tc->pn = notifs;
            tc->tpn = tc->pn;
        }
    }

    if (notifs) {
        tc->section = TRD_SECT_NOTIF;
        ret.section_name = TRD_KEYWORD_NOTIF;
        ret.has_node = tro_tree_ctx_get_node(tc) ? 1 : 0;
    }

    return ret;
}

static struct trt_keyword_stmt
tro_get_ext_section(struct trt_tree_ctx *tc, void *ext, struct lyspr_tree_ctx *plug_ctx)
{
    struct trt_keyword_stmt ret = {0};
    struct lysc_ext_instance *ce = NULL;
    struct lysp_ext_instance *pe = NULL;

    if (tc->lysc_tree) {
        ce = ext;
        ret.section_name = ce->def->name;
        ret.argument = ce->argument;
        ret.has_node = plug_ctx->schemas->ctree ? 1 : 0;
    } else {
        pe = ext;
        ret.section_name = pe->def->name;
        ret.argument = pe->argument;
        ret.has_node = plug_ctx->schemas->ptree ? 1 : 0;
    }

    return ret;
}

/**
 * @brief Get name of the module.
 * @param[in] tc is context of the tree.
 */
static struct trt_keyword_stmt
tro_read_module_name(const struct trt_tree_ctx *tc)
{
    assert(tc);
    struct trt_keyword_stmt ret;

    ret.section_name = !tc->lysc_tree && tc->pmod->is_submod ?
            TRD_KEYWORD_SUBMODULE :
            TRD_KEYWORD_MODULE;

    ret.argument = !tc->lysc_tree ?
            LYSP_MODULE_NAME(tc->pmod) :
            tc->cmod->mod->name;

    ret.has_node = tro_tree_ctx_get_node(tc) ? 1 : 0;

    return ret;
}

static ly_bool
tro_read_if_sibling_exists(const struct trt_tree_ctx *tc)
{
    const void *parent;

    if (tc->lysc_tree) {
        parent = troc_parent(tc->cn);
    } else {
        parent = trop_parent(tc->pn);
    }

    return parent ? 1 : 0;
}

/**
 * @brief Create implicit "case" node as parent of @p node.
 * @param[in] node child of implicit case node.
 * @param[out] case_node created case node.
 */
static void
tro_create_implicit_case_node(const struct trt_node *node, struct trt_node *case_node)
{
    case_node->status = node->status;
    case_node->flags = TRD_FLAGS_TYPE_EMPTY;
    case_node->name.type = TRD_NODE_CASE;
    case_node->name.keys = node->name.keys;
    case_node->name.module_prefix = node->name.module_prefix;
    case_node->name.str = node->name.str;
    case_node->name.opts = node->name.opts;
    case_node->name.add_opts = node->name.add_opts;
    case_node->type = TRP_EMPTY_TRT_TYPE;
    case_node->iffeatures = TRP_EMPTY_TRT_IFFEATURES;
    case_node->last_one = node->last_one;
}

/**********************************************************************
 * Definition of trop reading functions
 *********************************************************************/

/**
 * @brief Check if list statement has keys.
 * @param[in] pn is pointer to the list.
 * @return 1 if has keys, otherwise 0.
 */
static ly_bool
trop_list_has_keys(const struct lysp_node *pn)
{
    return trg_charptr_has_data(((const struct lysp_node_list *)pn)->key);
}

/**
 * @brief Check if it contains at least one feature.
 * @param[in] pn is current node.
 * @return 1 if has if-features, otherwise 0.
 */
static ly_bool
trop_node_has_iffeature(const struct lysp_node *pn)
{
    LY_ARRAY_COUNT_TYPE u;
    const struct lysp_qname *iffs;

    ly_bool ret = 0;

    iffs = pn->iffeatures;
    LY_ARRAY_FOR(iffs, u) {
        ret = 1;
        break;
    }
    return ret;
}

/**
 * @brief Find out if leaf is also the key in last list.
 * @param[in] pn is pointer to leaf.
 * @param[in] ca_last_list is pointer to last visited list.
 * Obtained from trt_parent_cache.
 * @return 1 if leaf is also the key, otherwise 0.
 */
static ly_bool
trop_leaf_is_key(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;
    const struct lysp_node_list *list = ca_last_list;

    if (!list) {
        return 0;
    }
    return trg_charptr_has_data(list->key) ?
           trg_word_is_present(list->key, leaf->name, ' ') : 0;
}

/**
 * @brief Check if container's type is presence.
 * @param[in] pn is pointer to container.
 * @return 1 if container has presence statement, otherwise 0.
 */
static ly_bool
trop_container_has_presence(const struct lysp_node *pn)
{
    return trg_charptr_has_data(((struct lysp_node_container *)pn)->presence);
}

/**
 * @brief Get leaflist's path without lysp_node type control.
 * @param[in] pn is pointer to the leaflist.
 */
static const char *
trop_leaflist_refpath(const struct lysp_node *pn)
{
    const struct lysp_node_leaflist *list = (const struct lysp_node_leaflist *)pn;

    return list->type.path ? list->type.path->expr : NULL;
}

/**
 * @brief Get leaflist's type name without lysp_node type control.
 * @param[in] pn is pointer to the leaflist.
 */
static const char *
trop_leaflist_type_name(const struct lysp_node *pn)
{
    const struct lysp_node_leaflist *list = (const struct lysp_node_leaflist *)pn;

    return list->type.name;
}

/**
 * @brief Get leaf's path without lysp_node type control.
 * @param[in] pn is pointer to the leaf node.
 */
static const char *
trop_leaf_refpath(const struct lysp_node *pn)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;

    return leaf->type.path ? leaf->type.path->expr : NULL;
}

/**
 * @brief Get leaf's type name without lysp_node type control.
 * @param[in] pn is pointer to the leaf's type name.
 */
static const char *
trop_leaf_type_name(const struct lysp_node *pn)
{
    const struct lysp_node_leaf *leaf = (const struct lysp_node_leaf *)pn;

    return leaf->type.name;
}

/**
 * @brief Get pointer to data using node type specification
 * and getter function.
 *
 * @param[in] flags is node type specification.
 * If it is the correct node, the getter function is called.
 * @param[in] f is getter function which provides the desired
 * char pointer from the structure.
 * @param[in] pn pointer to node.
 * @return NULL if node has wrong type or getter function return
 * pointer to NULL.
 * @return Pointer to desired char pointer obtained from the node.
 */
static const char *
trop_node_charptr(uint16_t flags, trt_get_charptr_func f, const struct lysp_node *pn)
{
    if (pn->nodetype & flags) {
        const char *ret = f(pn);

        return trg_charptr_has_data(ret) ? ret : NULL;
    } else {
        return NULL;
    }
}

/**
 * @brief Resolve \<status\> of the current node.
 * @param[in] nodetype is node's type obtained from the tree.
 * @param[in] flags is node's flags obtained from the tree.
 * @param[in] ca_lys_status is inherited status obtained from trt_parent_cache.
 * @return The status type.
 */
static char *
trop_resolve_status(uint16_t nodetype, uint16_t flags, uint16_t ca_lys_status)
{
    if (nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        /* LYS_INPUT and LYS_OUTPUT is special case */
        return tro_flags2status(ca_lys_status);
        /* if ancestor's status is deprc or obslt
         * and also node's status is not set
         */
    } else if ((ca_lys_status & (LYS_STATUS_DEPRC | LYS_STATUS_OBSLT)) && !(flags & (LYS_STATUS_CURR | LYS_STATUS_DEPRC | LYS_STATUS_OBSLT))) {
        /* get ancestor's status */
        return tro_flags2status(ca_lys_status);
    } else {
        /* else get node's status */
        return tro_flags2status(flags);
    }
}

/**
 * @brief Resolve \<flags\> of the current node.
 * @param[in] nodetype is node's type obtained from the tree.
 * @param[in] flags is node's flags obtained from the tree.
 * @param[in] ca_ancestor is ancestor type obtained from trt_parent_cache.
 * @param[in] ca_lys_config is inherited config item obtained from trt_parent_cache.
 * @param[in] no Override structure for flags.
 * @return The flags type.
 */
static const char *
trop_resolve_flags(uint16_t nodetype, uint16_t flags, trt_ancestor_type ca_ancestor, uint16_t ca_lys_config,
        struct lyplg_ext_sprinter_tree_node_override *no)
{
    if (no && no->flags) {
        return no->flags;
    } else if ((nodetype & LYS_INPUT) || (ca_ancestor == TRD_ANCESTOR_RPC_INPUT)) {
        return TRD_FLAGS_TYPE_RPC_INPUT_PARAMS;
    } else if ((nodetype & LYS_OUTPUT) || (ca_ancestor == TRD_ANCESTOR_RPC_OUTPUT)) {
        return TRD_FLAGS_TYPE_RO;
    } else if (ca_ancestor == TRD_ANCESTOR_NOTIF) {
        return TRD_FLAGS_TYPE_RO;
    } else if (nodetype & LYS_NOTIF) {
        return TRD_FLAGS_TYPE_NOTIF;
    } else if (nodetype & LYS_USES) {
        return TRD_FLAGS_TYPE_USES_OF_GROUPING;
    } else if (nodetype & (LYS_RPC | LYS_ACTION)) {
        return TRD_FLAGS_TYPE_RPC;
    } else if (!(flags & (LYS_CONFIG_R | LYS_CONFIG_W))) {
        /* config is not set. Look at ancestor's config */
        return tro_flags2config(ca_lys_config);
    } else {
        return tro_flags2config(flags);
    }
}

/**
 * @brief Resolve node type of the current node.
 * @param[in] pn is pointer to the current node in the tree.
 * @param[in] ca_last_list is pointer to the last visited list. Obtained from the trt_parent_cache.
 * @param[out] type Resolved type of node.
 * @param[out] opts Resolved opts of node.
 */
static void
trop_resolve_node_opts(const struct lysp_node *pn, const struct lysp_node_list *ca_last_list, trt_node_type *type,
        const char **opts)
{
    if (pn->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        *type = TRD_NODE_ELSE;
    } else if (pn->nodetype & LYS_CASE) {
        *type = TRD_NODE_CASE;
    } else if ((pn->nodetype & LYS_CHOICE) && !(pn->flags & LYS_MAND_TRUE)) {
        *type = TRD_NODE_CHOICE;
        *opts = TRD_NODE_OPTIONAL;
    } else if (pn->nodetype & LYS_CHOICE) {
        *type = TRD_NODE_CHOICE;
    } else if ((pn->nodetype & LYS_CONTAINER) && (trop_container_has_presence(pn))) {
        *opts = TRD_NODE_CONTAINER;
    } else if (pn->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        *opts = TRD_NODE_LISTLEAFLIST;
    } else if ((pn->nodetype & (LYS_ANYDATA | LYS_ANYXML)) && !(pn->flags & LYS_MAND_TRUE)) {
        *opts = TRD_NODE_OPTIONAL;
    } else if ((pn->nodetype & LYS_LEAF) && !(pn->flags & LYS_MAND_TRUE) && (!trop_leaf_is_key(pn, ca_last_list))) {
        *opts = TRD_NODE_OPTIONAL;
    } else {
        *type = TRD_NODE_ELSE;
    }
}

/**
 * @brief Resolve \<type\> of the current node.
 * @param[in] pn is current node.
 * @return Resolved type.
 */
static struct trt_type
trop_resolve_type(const struct lysp_node *pn)
{
    const char *tmp = NULL;

    if (!pn) {
        return TRP_EMPTY_TRT_TYPE;
    } else if ((tmp = trop_node_charptr(LYS_LEAFLIST, trop_leaflist_refpath, pn))) {
        return TRP_INIT_TRT_TYPE(TRD_TYPE_TARGET, tmp);
    } else if ((tmp = trop_node_charptr(LYS_LEAFLIST, trop_leaflist_type_name, pn))) {
        return TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, tmp);
    } else if ((tmp = trop_node_charptr(LYS_LEAF, trop_leaf_refpath, pn))) {
        return TRP_INIT_TRT_TYPE(TRD_TYPE_TARGET, tmp);
    } else if ((tmp = trop_node_charptr(LYS_LEAF, trop_leaf_type_name, pn))) {
        return TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, tmp);
    } else if (pn->nodetype == LYS_ANYDATA) {
        return TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, "anydata");
    } else if (pn->nodetype & LYS_ANYXML) {
        return TRP_INIT_TRT_TYPE(TRD_TYPE_NAME, "anyxml");
    } else {
        return TRP_EMPTY_TRT_TYPE;
    }
}

/**
 * @brief Resolve iffeatures.
 *
 * @param[in] pn is current parsed node.
 * @return Resolved iffeatures.
 */
static struct trt_iffeatures
trop_resolve_iffeatures(const struct lysp_node *pn)
{
    struct trt_iffeatures iff;

    if (pn && trop_node_has_iffeature(pn)) {
        iff.type = TRD_IFF_PRESENT;
        iff.str = NULL;
    } else {
        iff.type = TRD_IFF_NON_PRESENT;
        iff.str = NULL;
    }

    return iff;
}

/**
 * @brief Transformation of current lysp_node to struct trt_node.
 * @param[in] ca contains stored important data
 * when browsing the tree downwards.
 * @param[in] tc is context of the tree.
 */
static struct trt_node
trop_read_node(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    const struct lysp_node *pn;
    struct trt_node ret;
    struct lyplg_ext_sprinter_tree_node_override *no;

    assert(tc && tc->pn && tc->pn->nodetype != LYS_UNKNOWN);

    no = tro_set_node_overr(tc->lysc_tree, tc->pn, 1, &tc->plugin_ctx);

    pn = tc->pn;
    ret = TRP_EMPTY_NODE;

    /* <status> */
    ret.status = trop_resolve_status(pn->nodetype, pn->flags, ca.lys_status);

    /* <flags> */
    ret.flags = trop_resolve_flags(pn->nodetype, pn->flags, ca.ancestor, ca.lys_config, no);

    /* set type of the node */
    trop_resolve_node_opts(pn, ca.last_list, &ret.name.type, &ret.name.opts);
    ret.name.add_opts = no && no->add_opts ? no->add_opts : NULL;
    ret.name.keys = (tc->pn->nodetype & LYS_LIST) && trop_list_has_keys(tc->pn);

    /* The parsed tree is not compiled, so no node can be augmented
     * from another module. This means that nodes from the parsed tree
     * will never have the prefix.
     */
    ret.name.module_prefix = NULL;

    /* set node's name */
    ret.name.str = pn->name;

    /* <type> */
    ret.type = trop_resolve_type(pn);

    /* <iffeature> */
    ret.iffeatures = trop_resolve_iffeatures(pn);

    ret.last_one = !tro_next_sibling(pn, tc);

    return ret;
}

/**
 * @brief Find out if the current node has siblings.
 * @param[in] tc is context of the tree.
 * @return 1 if sibling exists otherwise 0.
 */
static ly_bool
trop_read_if_sibling_exists(const struct trt_tree_ctx *tc)
{
    return tro_next_sibling(tc->pn, tc) != NULL;
}

/**********************************************************************
 * Modify trop getters
 *********************************************************************/

/**
 * @brief Change current node pointer to its parent
 * but only if parent exists.
 * @param[in,out] tc is tree context.
 * Contains pointer to the current node.
 * @return 1 if the node had parents and the change was successful.
 * @return 0 if the node did not have parents.
 * The pointer to the current node did not change.
 */
static ly_bool
trop_modi_parent(struct trt_tree_ctx *tc)
{
    assert(tc && tc->pn);
    /* If no parent exists, stay in actual node. */
    if ((tc->pn != tc->tpn) && (tc->pn->parent)) {
        tc->pn = tc->pn->parent;
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Change the current node pointer to its child
 * but only if exists.
 * @param[in] ca contains inherited data from ancestors.
 * @param[in,out] tc is context of the tree.
 * Contains pointer to the current node.
 * @return Non-empty \<node\> representation of the current
 * node's child. The @p tc is modified.
 * @return Empty \<node\> representation if child don't exists.
 * The @p tc is not modified.
 */
static struct trt_node
trop_modi_next_child(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    const struct lysp_node *tmp;

    assert(tc && tc->pn);

    if ((tmp = tro_next_child(tc->pn, tc))) {
        tc->pn = tmp;
        return trop_read_node(ca, tc);
    } else {
        return TRP_EMPTY_NODE;
    }
}

/**
 * @brief Change the pointer to the current node to its next sibling
 * only if exists.
 * @param[in] ca contains inherited data from ancestors.
 * @param[in,out] tc is tree context.
 * Contains pointer to the current node.
 * @return Non-empty \<node\> representation if sibling exists.
 * The @p tc is modified.
 * @return Empty \<node\> representation otherwise.
 * The @p tc is not modified.
 */
static struct trt_node
trop_modi_next_sibling(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    const struct lysp_node *pn;

    assert(tc && tc->pn);

    pn = tro_next_sibling(tc->pn, tc);

    if (pn) {
        if ((tc->tpn == tc->pn) && (tc->section != TRD_SECT_PLUG_DATA)) {
            tc->tpn = pn;
        }
        tc->pn = pn;
        return trop_read_node(ca, tc);
    } else {
        return TRP_EMPTY_NODE;
    }
}

/**
 * @brief Change the current node pointer to the first child of node's
 * parent. If current node is already first sibling/child then nothing
 * will change.
 * @param[in] ca Settings of parent.
 * @param[in,out] tc is tree context.
 * @return node for printing.
 */
static struct trt_node
trop_modi_first_sibling(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    struct trt_node node;

    assert(tc && tc->pn);

    if (trop_modi_parent(tc)) {
        node = trop_modi_next_child(ca, tc);
    } else if (tc->plugin_ctx.schema) {
        tc->pn = tc->plugin_ctx.schema->ptree;
        tc->tpn = tc->pn;
        node = trop_read_node(ca, tc);
    } else {
        /* current node is top-node */
        switch (tc->section) {
        case TRD_SECT_MODULE:
            tc->pn = tc->pmod->data;
            tc->tpn = tc->pn;
            break;
        case TRD_SECT_AUGMENT:
            tc->pn = (const struct lysp_node *)tc->pmod->augments;
            tc->tpn = tc->pn;
            break;
        case TRD_SECT_RPCS:
            tc->pn = (const struct lysp_node *)tc->pmod->rpcs;
            tc->tpn = tc->pn;
            break;
        case TRD_SECT_NOTIF:
            tc->pn = (const struct lysp_node *)tc->pmod->notifs;
            tc->tpn = tc->pn;
            break;
        case TRD_SECT_GROUPING:
            tc->pn = (const struct lysp_node *)tc->pmod->groupings;
            tc->tpn = tc->pn;
            break;
        case TRD_SECT_PLUG_DATA:
            /* Nothing to do. */
            break;
        default:
            assert(0);
        }
        node = trop_read_node(ca, tc);
    }

    if (tc->plugin_ctx.filtered) {
        node = trop_modi_next_sibling(ca, tc);
    }

    return node;
}

/**
 * @brief Get next (or first) augment section if exists.
 * @param[in,out] tc is tree context. It is modified and his current
 * node is set to the lysp_node_augment.
 * @return Section's representation if (next augment) section exists.
 * @return Empty section structure otherwise.
 */
static struct trt_keyword_stmt
trop_modi_next_augment(struct trt_tree_ctx *tc)
{
    assert(tc);
    const struct lysp_node_augment *augs;
    struct trt_keyword_stmt ret = {0};

    /* if next_augment func was called for the first time */
    if (tc->section != TRD_SECT_AUGMENT) {
        tc->section = TRD_SECT_AUGMENT;
        augs = tc->pmod->augments;
    } else {
        /* get augment sibling from top-node pointer */
        augs = (const struct lysp_node_augment *)tc->tpn->next;
    }

    if (augs) {
        tc->pn = &augs->node;
        tc->tpn = tc->pn;
        ret.section_name = TRD_KEYWORD_AUGMENT;
        ret.argument = augs->nodeid;
        ret.has_node = tro_tree_ctx_get_node(tc) ? 1 : 0;
    }

    return ret;
}

/**
 * @brief Get next (or first) grouping section if exists
 * @param[in,out] tc is tree context. It is modified and his current
 * node is set to the lysp_node_grp.
 * @return The next (or first) section representation if it exists.
 * @return Empty section representation otherwise.
 */
static struct trt_keyword_stmt
trop_modi_next_grouping(struct trt_tree_ctx *tc)
{
    assert(tc);
    const struct lysp_node_grp *grps;
    struct trt_keyword_stmt ret = {0};

    if (tc->section != TRD_SECT_GROUPING) {
        tc->section = TRD_SECT_GROUPING;
        grps = tc->pmod->groupings;
    } else {
        grps = (const struct lysp_node_grp *)tc->tpn->next;
    }

    if (grps) {
        tc->pn = &grps->node;
        tc->tpn = tc->pn;
        ret.section_name = TRD_KEYWORD_GROUPING;
        ret.argument = grps->name;
        ret.has_node = tro_tree_ctx_get_child(tc) ? 1 : 0;
    }

    return ret;
}

/**********************************************************************
 * Definition of troc reading functions
 *********************************************************************/

/**
 * @copydoc trop_read_if_sibling_exists
 */
static ly_bool
troc_read_if_sibling_exists(const struct trt_tree_ctx *tc)
{
    return tro_next_sibling(tc->cn, tc) != NULL;
}

/**
 * @brief Resolve \<flags\> of the current node.
 *
 * Use this function only if trt_tree_ctx.lysc_tree is true.
 *
 * @param[in] nodetype is current lysc_node.nodetype.
 * @param[in] flags is current lysc_node.flags.
 * @param[in] no Override structure for flags.
 * @return The flags type.
 */
static const char *
troc_resolve_flags(uint16_t nodetype, uint16_t flags, struct lyplg_ext_sprinter_tree_node_override *no)
{
    if (no && no->flags) {
        return no->flags;
    } else if ((nodetype & LYS_INPUT) || (flags & LYS_IS_INPUT)) {
        return TRD_FLAGS_TYPE_RPC_INPUT_PARAMS;
    } else if ((nodetype & LYS_OUTPUT) || (flags & LYS_IS_OUTPUT)) {
        return TRD_FLAGS_TYPE_RO;
    } else if (nodetype & LYS_IS_NOTIF) {
        return TRD_FLAGS_TYPE_RO;
    } else if (nodetype & LYS_NOTIF) {
        return TRD_FLAGS_TYPE_NOTIF;
    } else if (nodetype & LYS_USES) {
        return TRD_FLAGS_TYPE_USES_OF_GROUPING;
    } else if (nodetype & (LYS_RPC | LYS_ACTION)) {
        return TRD_FLAGS_TYPE_RPC;
    } else {
        return tro_flags2config(flags);
    }
}

/**
 * @brief Resolve node type of the current node.
 *
 * Use this function only if trt_tree_ctx.lysc_tree is true.
 *
 * @param[in] nodetype is current lysc_node.nodetype.
 * @param[in] flags is current lysc_node.flags.
 * @param[out] type Resolved type of node.
 * @param[out] opts Resolved opts.
 */
static void
troc_resolve_node_opts(uint16_t nodetype, uint16_t flags, trt_node_type *type, const char **opts)
{
    if (nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        *type = TRD_NODE_ELSE;
    } else if (nodetype & LYS_CASE) {
        *type = TRD_NODE_CASE;
    } else if ((nodetype & LYS_CHOICE) && !(flags & LYS_MAND_TRUE)) {
        *type = TRD_NODE_CHOICE;
        *opts = TRD_NODE_OPTIONAL;
    } else if (nodetype & LYS_CHOICE) {
        *type = TRD_NODE_CHOICE;
    } else if ((nodetype & LYS_CONTAINER) && (flags & LYS_PRESENCE)) {
        *opts = TRD_NODE_CONTAINER;
    } else if (nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        *opts = TRD_NODE_LISTLEAFLIST;
    } else if ((nodetype & (LYS_ANYDATA | LYS_ANYXML)) && !(flags & LYS_MAND_TRUE)) {
        *opts = TRD_NODE_OPTIONAL;
    } else if ((nodetype & LYS_LEAF) && !(flags & (LYS_MAND_TRUE | LYS_KEY))) {
        *opts = TRD_NODE_OPTIONAL;
    } else {
        *type = TRD_NODE_ELSE;
    }
}

/**
 * @brief Resolve prefix (\<prefix\>:\<name\>) of node that has been
 * placed from another module via an augment statement.
 *
 * @param[in] cn is current compiled node.
 * @param[in] current_compiled_module is module whose nodes are
 * currently being printed.
 * @return Prefix of foreign module or NULL.
 */
static const char *
troc_resolve_node_prefix(const struct lysc_node *cn, const struct lysc_module *current_compiled_module)
{
    const struct lys_module *node_module;
    const char *ret = NULL;

    node_module = cn->module;
    if (!node_module || !current_compiled_module) {
        return NULL;
    } else if (node_module->compiled != current_compiled_module) {
        ret = node_module->prefix;
    }

    return ret;
}

/**
 * @brief Transformation of current lysc_node to struct trt_node.
 * @param[in] ca is not used.
 * @param[in] tc is context of the tree.
 */
static struct trt_node
troc_read_node(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    (void) ca;
    const struct lysc_node *cn;
    struct trt_node ret;
    struct lyplg_ext_sprinter_tree_node_override *no;

    assert(tc && tc->cn);

    no = tro_set_node_overr(tc->lysc_tree, tc->cn, 1, &tc->plugin_ctx);

    cn = tc->cn;
    ret = TRP_EMPTY_NODE;

    /* <status> */
    ret.status = tro_flags2status(cn->flags);

    /* <flags> */
    ret.flags = troc_resolve_flags(cn->nodetype, cn->flags, no);

    /* set type of the node */
    troc_resolve_node_opts(cn->nodetype, cn->flags, &ret.name.type, &ret.name.opts);
    ret.name.add_opts = no && no->add_opts ? no->add_opts : NULL;
    ret.name.keys = (cn->nodetype & LYS_LIST) && !(cn->flags & LYS_KEYLESS);

    /* <prefix> */
    ret.name.module_prefix = troc_resolve_node_prefix(cn, tc->cmod);

    /* set node's name */
    ret.name.str = cn->name;

    /* <type> */
    ret.type = trop_resolve_type(TRP_TREE_CTX_GET_LYSP_NODE(cn));

    /* <iffeature> */
    ret.iffeatures = trop_resolve_iffeatures(TRP_TREE_CTX_GET_LYSP_NODE(cn));

    ret.last_one = !tro_next_sibling(cn, tc);

    return ret;
}

/**********************************************************************
 * Modify troc getters
 *********************************************************************/

/**
 * @copydoc ::trop_modi_parent()
 */
static ly_bool
troc_modi_parent(struct trt_tree_ctx *tc)
{
    assert(tc && tc->cn);
    /* If no parent exists, stay in actual node. */
    if (tc->cn->parent) {
        tc->cn = tc->cn->parent;
        return 1;
    } else {
        return 0;
    }
}

/**
 * @copydoc ::trop_modi_next_sibling()
 */
static struct trt_node
troc_modi_next_sibling(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    const struct lysc_node *cn;

    assert(tc && tc->cn);

    cn = tro_next_sibling(tc->cn, tc);

    /* if next sibling exists */
    if (cn) {
        /* update trt_tree_ctx */
        tc->cn = cn;
        return troc_read_node(ca, tc);
    } else {
        return TRP_EMPTY_NODE;
    }
}

/**
 * @copydoc trop_modi_next_child()
 */
static struct trt_node
troc_modi_next_child(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    const struct lysc_node *tmp;

    assert(tc && tc->cn);

    if ((tmp = tro_next_child(tc->cn, tc))) {
        tc->cn = tmp;
        return troc_read_node(ca, tc);
    } else {
        return TRP_EMPTY_NODE;
    }
}

/**
 * @copydoc ::trop_modi_first_sibling()
 */
static struct trt_node
troc_modi_first_sibling(struct trt_parent_cache ca, struct trt_tree_ctx *tc)
{
    struct trt_node node;

    assert(tc && tc->cn);

    if (troc_modi_parent(tc)) {
        node = troc_modi_next_child(ca, tc);
    } else if (tc->plugin_ctx.schema) {
        tc->cn = tc->plugin_ctx.schema->ctree;
        node = troc_read_node(ca, tc);
    } else {
        /* current node is top-node */
        switch (tc->section) {
        case TRD_SECT_MODULE:
            tc->cn = tc->cn->module->compiled->data;
            break;
        case TRD_SECT_RPCS:
            tc->cn = (const struct lysc_node *)tc->cmod->rpcs;
            break;
        case TRD_SECT_NOTIF:
            tc->cn = (const struct lysc_node *)tc->cmod->notifs;
            break;
        case TRD_SECT_PLUG_DATA:
            /* nothing to do */
            break;
        default:
            assert(0);
        }
        node = troc_read_node(ca, tc);
    }

    if (tc->plugin_ctx.filtered) {
        node = troc_modi_next_sibling(ca, tc);
    }

    return node;
}

/**********************************************************************
 * Definition of tree browsing functions
 *********************************************************************/

static uint32_t
trb_gap_to_opts(const struct trt_node *node)
{
    uint32_t len = 0;

    if (node->name.keys) {
        return 0;
    }

    if (node->flags) {
        len += strlen(node->flags);
        /* space between flags and name */
        len += 1;
    } else {
        /* space between -- and name */
        len += 1;
    }

    switch (node->name.type) {
    case TRD_NODE_CASE:
        /* ':' is already counted. Plus parentheses. */
        len += 2;
        break;
    case TRD_NODE_CHOICE:
        /* Plus parentheses. */
        len += 2;
        break;
    default:
        break;
    }

    if (node->name.module_prefix) {
        /* prefix_name and ':' */
        len += strlen(node->name.module_prefix) + 1;
    }
    if (node->name.str) {
        len += strlen(node->name.str);
    }
    if (node->name.add_opts) {
        len += strlen(node->name.add_opts);
    }
    if (node->name.opts) {
        len += strlen(node->name.opts);
    }

    return len;
}

static uint32_t
trb_gap_to_type(const struct trt_node *node)
{
    uint32_t len, opts_len;

    if (node->name.keys) {
        return 0;
    }

    len = trb_gap_to_opts(node);
    /* Gap between opts and type. */
    opts_len = 0;
    opts_len += node->name.add_opts ? strlen(node->name.add_opts) : 0;
    opts_len += node->name.opts ? strlen(node->name.opts) : 0;
    if (opts_len >= TRD_INDENT_BEFORE_TYPE) {
        /* At least one space should be there. */
        len += 1;
    } else if (node->name.add_opts || node->name.opts) {
        len += TRD_INDENT_BEFORE_TYPE - opts_len;
    } else {
        len += TRD_INDENT_BEFORE_TYPE;
    }

    return len;
}

/**
 * @brief Calculate the trt_indent_in_node.btw_opts_type indent size
 * for a particular node.
 * @param[in] node for which we get btw_opts_type.
 * @param[in] max_gap_before_type is the maximum value of btw_opts_type
 * that it can have.
 * @return Indent between \<opts\> and \<type\> for node.
 */
static int16_t
trb_calc_btw_opts_type(const struct trt_node *node, int16_t max_gap_before_type)
{
    uint32_t to_opts_len;

    to_opts_len = trb_gap_to_opts(node);
    if (to_opts_len == 0) {
        return 1;
    } else {
        return max_gap_before_type - to_opts_len;
    }
}

/**
 * @brief Print node.
 *
 * This function is wrapper for ::trp_print_entire_node().
 * But difference is that take @p max_gap_before_type which will be
 * used to set the unified alignment.
 *
 * @param[in] node to print.
 * @param[in] max_gap_before_type is number of indent before \<type\>.
 * @param[in] wr is wrapper for printing indentation before node.
 * @param[in] pc contains mainly functions for printing.
 * @param[in] tc is tree context.
 */
static void
trb_print_entire_node(const struct trt_node *node, uint32_t max_gap_before_type, struct trt_wrapper wr,
        struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_indent_in_node ind = trp_default_indent_in_node(node);

    if ((max_gap_before_type > 0) && (node->type.type != TRD_TYPE_EMPTY)) {
        /* print actual node with unified indent */
        ind.btw_opts_type = trb_calc_btw_opts_type(node, max_gap_before_type);
    }
    /* after -> print actual node with default indent */
    trp_print_entire_node(node, TRP_INIT_PCK_PRINT(tc, pc->fp.print),
            TRP_INIT_PCK_INDENT(wr, ind), pc->max_line_length, pc->out);
}

/**
 * @brief Check if parent of the current node is the last
 * of his siblings.
 *
 * To mantain stability use this function only if the current node is
 * the first of the siblings.
 * Side-effect -> current node is set to the first sibling
 * if node has a parent otherwise no side-effect.
 *
 * @param[in] fp contains all @ref TRP_tro callback functions.
 * @param[in,out] tc is tree context.
 * @return 1 if parent is last sibling otherwise 0.
 */
static ly_bool
trb_node_is_last_sibling(const struct trt_fp_all *fp, struct trt_tree_ctx *tc)
{
    if (fp->read.if_parent_exists(tc)) {
        return !fp->read.if_sibling_exists(tc);
    } else {
        return !fp->read.if_sibling_exists(tc) && tc->plugin_ctx.last_schema;
    }
}

/**
 * @brief For all siblings find maximal space from '--' to \<type\>.
 *
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 * @return max space.
 */
static uint32_t
trb_max_gap_to_type(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_node node;
    int32_t maxlen, len;

    maxlen = 0;
    for (node = pc->fp.modify.first_sibling(ca, tc);
            !trp_node_is_empty(&node);
            node = pc->fp.modify.next_sibling(ca, tc)) {
        len = trb_gap_to_type(&node);
        maxlen = maxlen < len ? len : maxlen;
    }
    pc->fp.modify.first_sibling(ca, tc);

    return maxlen;
}

/**
 * @brief Find out if it is possible to unify
 * the alignment before \<type\>.
 *
 * The goal is for all node siblings to have the same alignment
 * for \<type\> as if they were in a column. All siblings who cannot
 * adapt because they do not fit on the line at all are ignored.
 * Side-effect -> Current node is set to the first sibling.
 *
 * @param[in] ca contains inherited data from ancestors.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 * @return positive number indicating the maximum number of spaces
 * before \<type\> if the length of the flags, node name and opts is 0. To calculate
 * the trt_indent_in_node.btw_opts_type indent size for a particular
 * node, use the ::trb_calc_btw_opts_type().
*/
static uint32_t
trb_try_unified_indent(struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    return trb_max_gap_to_type(ca, pc, tc);
}

/**
 * @brief Check if there is no case statement
 * under the choice statement.
 *
 * It can return true only if the Parsed schema tree
 * is used for browsing.
 *
 * @param[in] tc is tree context.
 * @return 1 if implicit case statement is present otherwise 0.
 */
static ly_bool
trb_need_implicit_node_case(struct trt_tree_ctx *tc)
{
    return !tc->lysc_tree && tc->pn->parent &&
           (tc->pn->parent->nodetype & LYS_CHOICE) &&
           (tc->pn->nodetype & (LYS_ANYDATA | LYS_CHOICE | LYS_CONTAINER |
           LYS_LEAF | LYS_LEAFLIST));
}

static void trb_print_subtree_nodes(struct trt_node *node, uint32_t max_gap_before_type,
        struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc);

/**
 * @brief Print implicit case node and his subtree.
 *
 * @param[in] node is child of implicit case.
 * @param[in] wr is wrapper for printing identation before node.
 * @param[in] pc contains mainly functions for printing.
 * @param[in] tc is tree context. Its settings should be the same as
 * before the function call.
 * @return new indentation wrapper for @p node.
 */
static struct trt_wrapper
trb_print_implicit_node(const struct trt_node *node, struct trt_wrapper wr, struct trt_printer_ctx *pc,
        struct trt_tree_ctx *tc)
{
    struct trt_node case_node;
    struct trt_wrapper wr_case_child;

    tro_create_implicit_case_node(node, &case_node);
    ly_print_(pc->out, "\n");
    trb_print_entire_node(&case_node, 0, wr, pc, tc);
    ly_print_(pc->out, "\n");
    wr_case_child = pc->fp.read.if_sibling_exists(tc) ?
            trp_wrapper_set_mark(wr) : trp_wrapper_set_shift(wr);
    return wr_case_child;
}

/**
 * @brief Calculate the wrapper about how deep in the tree the node is.
 * @param[in] wr_in A wrapper to use as a starting point
 * @param[in] node from which to count.
 * @return wrapper for @p node.
 */
static struct trt_wrapper
trb_count_depth(const struct trt_wrapper *wr_in, const struct lysc_node *node)
{
    struct trt_wrapper wr = wr_in ? *wr_in : TRP_INIT_WRAPPER_TOP;
    const struct lysc_node *parent;

    if (!node) {
        return wr;
    }

    for (parent = node->parent; parent; parent = parent->parent) {
        wr = trp_wrapper_set_shift(wr);
    }

    return wr;
}

/**
 * @brief Print all parent nodes of @p node and the @p node itself.
 *
 * Side-effect -> trt_tree_ctx.cn will be set to @p node.
 *
 * @param[in] node on which the function is focused.
 * @param[in] wr_in for printing identation before node.
 * @param[in] pc is @ref TRP_trp settings.
 * @param[in,out] tc is context of tree printer.
 */
static void
trb_print_parents(const struct lysc_node *node, struct trt_wrapper *wr_in, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    uint32_t max_gap_before_type;
    struct trt_wrapper wr;
    struct trt_node print_node;

    assert(pc && tc && tc->section == TRD_SECT_MODULE);

    /* stop recursion */
    if (!node) {
        return;
    }
    trb_print_parents(node->parent, wr_in, pc, tc);

    /* setup for printing */
    tc->cn = node;
    wr = trb_count_depth(wr_in, node);

    /* print node */
    ly_print_(pc->out, "\n");
    print_node = pc->fp.read.node(TRP_EMPTY_PARENT_CACHE, tc);
    /* siblings do not print, so the node is always considered the last */
    print_node.last_one = 1;
    max_gap_before_type = trb_max_gap_to_type(TRP_EMPTY_PARENT_CACHE, pc, tc);
    tc->cn = node;
    trb_print_entire_node(&print_node, max_gap_before_type, wr, pc, tc);
}

/**
 * @brief Set current node on its child.
 * @param[in,out] tc contains current node.
 */
static void
trb_tree_ctx_set_child(struct trt_tree_ctx *tc)
{
    const void *node = tro_tree_ctx_get_child(tc);

    if (tc->lysc_tree) {
        tc->cn = node;
    } else {
        tc->pn = node;
    }
}

/**
 * @brief Move extension iterator to the next position.
 *
 * @param[in] lysc_tree flag if exts is from compiled tree.
 * @param[in] exts is current array of extensions.
 * @param[in,out] i is state of iterator.
 * @return Pointer to the first/next extension.
 */
static void *
trb_ext_iter_next(ly_bool lysc_tree, void *exts, uint64_t *i)
{
    void *ext = NULL;
    struct lysc_ext_instance *ce;
    struct lysp_ext_instance *pe;

    if (!exts) {
        return NULL;
    }

    if (lysc_tree) {
        ce = exts;
        while (*i < LY_ARRAY_COUNT(ce)) {
            if (ce->def->plugin && trp_ext_parent_is_valid(1, &ce[*i])) {
                ext = &ce[*i];
                break;
            }
            ++(*i);
        }
    } else {
        pe = exts;
        while (*i < LY_ARRAY_COUNT(pe)) {
            if (trp_ext_parent_is_valid(0, &pe[*i])) {
                ext = &pe[*i];
                break;
            }
            ++(*i);
        }
    }
    ++(*i);

    return ext;
}

/**
 * @brief Iterate over extensions in module.
 *
 * @param[in] tc contains current node.
 * @param[in,out] i is state of iterator.
 * @return First/next extension or NULL.
 */
static void *
trb_mod_ext_iter(const struct trt_tree_ctx *tc, uint64_t *i)
{
    if (tc->lysc_tree) {
        return trb_ext_iter_next(1, tc->cmod->exts, i);
    } else {
        return trb_ext_iter_next(0, tc->pmod->exts, i);
    }
}

/**
 * @brief Iterate over extensions in node.
 *
 * @param[in] tc contains current node.
 * @param[in,out] i is state of iterator.
 * @return First/next extension or NULL.
 */
static void *
trb_ext_iter(const struct trt_tree_ctx *tc, uint64_t *i)
{
    if (tc->lysc_tree) {
        return trb_ext_iter_next(1, tc->cn->exts, i);
    } else {
        return trb_ext_iter_next(0, tc->pn->exts, i);
    }
}

/**
 * @brief Initialize plugin context.
 *
 * @param[in] compiled if @p ext is lysc structure.
 * @param[in] ext current processed extension.
 * @param[out] plug_ctx is plugin context which will be initialized.
 * @param[out] ignore plugin callback is NULL.
 * @return LY_ERR value.
 */
static LY_ERR
tro_ext_printer_tree(ly_bool compiled, void *ext, const struct lyspr_tree_ctx *plug_ctx, ly_bool *ignore)
{
    struct lysc_ext_instance *ext_comp;
    struct lysp_ext_instance *ext_pars;
    const struct lyplg_ext *plugin;
    const char *flags = NULL, *add_opts = NULL;

    if (compiled) {
        ext_comp = ext;
        plugin = ext_comp->def->plugin;
        if (!plugin->printer_ctree) {
            *ignore = 1;
            return LY_SUCCESS;
        }
        return plugin->printer_ctree(ext, plug_ctx, &flags, &add_opts);
    } else {
        ext_pars = ext;
        plugin = &ext_pars->record->plugin;
        if (!plugin->printer_ptree) {
            *ignore = 1;
            return LY_SUCCESS;
        }
        return plugin->printer_ptree(ext, plug_ctx, &flags, &add_opts);
    }

    return LY_SUCCESS;
}

/**
 * @brief Reset tree context by plugin context.
 *
 * @param[in] plug_ctx is plugin context.
 * @param[in] i which index in schemas should be used.
 * @param[in] pc are printing functions.
 * @param[out] tc tree context which will be updated.
 */
static void
trm_reset_tree_ctx_by_plugin(struct lyspr_tree_ctx *plug_ctx, LY_ARRAY_COUNT_TYPE i, struct trt_printer_ctx *pc,
        struct trt_tree_ctx *tc)
{
    tc->plugin_ctx.ctx = plug_ctx;
    tc->pmod = NULL;
    tc->cmod = NULL;
    if (plug_ctx->schemas[i].compiled) {
        tc->lysc_tree = 1;
        tc->cn = plug_ctx->schemas[i].ctree;
        tc->plugin_ctx.schema = &plug_ctx->schemas[i];
        pc->fp.modify = TRP_TRT_FP_MODIFY_COMPILED;
        pc->fp.read = TRP_TRT_FP_READ_COMPILED;
    } else {
        tc->lysc_tree = 0;
        tc->pn = plug_ctx->schemas[i].ptree;
        tc->tpn = tc->pn;
        tc->plugin_ctx.schema = &plug_ctx->schemas[i];
        pc->fp.modify = TRP_TRT_FP_MODIFY_PARSED;
        pc->fp.read = TRP_TRT_FP_READ_PARSED;
    }
}

/**
 * @brief Print schemas from plugin context.
 *
 * @param[in] plug_ctx is plugin context.
 * @param[in] last_nodes if this schemas will be the last.
 * @param[in] max_gap_before_type is gap before type.
 * @param[in] wr is indentation wrapper.
 * @param[in] ca containing information from parent.
 * @param[in] pc functions for tree traversing.
 * @param[in] tc current tree context.
 */
static void
trb_ext_print_schemas(struct lyspr_tree_ctx *plug_ctx, ly_bool last_nodes, uint32_t max_gap_before_type,
        struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    LY_ARRAY_COUNT_TYPE i;
    struct trt_printer_ctx pc_dupl;
    struct trt_tree_ctx tc_dupl;
    struct trt_node node;

    tc_dupl = *tc;
    pc_dupl = *pc;

    LY_ARRAY_FOR(plug_ctx->schemas, i) {
        trm_reset_tree_ctx_by_plugin(plug_ctx, i, pc, tc);
        tc->plugin_ctx.last_schema = last_nodes && ((i + 1) == LY_ARRAY_COUNT(plug_ctx->schemas));
        node = TRP_EMPTY_NODE;
        trb_print_subtree_nodes(&node, max_gap_before_type, wr, ca, pc, tc);
        *tc = tc_dupl;
    }

    *pc = pc_dupl;
}

/**
 * @brief Count unified indentation across schemas from extension instance.
 *
 * @param[in] plug_ctx is plugin context.
 * @param[in] ca containing parent settings.
 * @param[out] max_gap_before_type is result of unified indent.
 * @param[in] pc functions for tree traversing.
 * @param[in] tc is tree context.
 */
static void
trb_ext_try_unified_indent(struct lyspr_tree_ctx *plug_ctx, struct trt_parent_cache ca, uint32_t *max_gap_before_type,
        struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    LY_ARRAY_COUNT_TYPE i;
    struct trt_printer_ctx pc_dupl;
    struct trt_tree_ctx tc_dupl;
    uint32_t max;

    tc_dupl = *tc;
    pc_dupl = *pc;

    LY_ARRAY_FOR(plug_ctx->schemas, i) {
        trm_reset_tree_ctx_by_plugin(plug_ctx, i, pc, tc);
        max = trb_try_unified_indent(ca, pc, tc);
        *max_gap_before_type = max > *max_gap_before_type ? max : *max_gap_before_type;
        *tc = tc_dupl;
    }

    *pc = pc_dupl;
}

/**
 * @brief For every extension instance print all schemas.
 *
 * @param[in] wr indentation wrapper for node.
 * @param[in] ca parent settings.
 * @param[in] pc function used for tree traversing.
 * @param[in] tc tree context.
 */
static void
trb_ext_print_instances(struct trt_wrapper wr, struct trt_parent_cache ca, struct trt_printer_ctx *pc,
        struct trt_tree_ctx *tc)
{
    LY_ERR rc;
    LY_ARRAY_COUNT_TYPE i;
    uint64_t last_instance = UINT64_MAX;
    void *ext;
    ly_bool child_exists, ignore = 0;
    uint32_t max, max_gap_before_type = 0;

    ca = tro_parent_cache_for_child(ca, tc);
    /* if node is last sibling, then do not add '|' to wrapper */
    wr = trb_node_is_last_sibling(&pc->fp, tc) ?
            trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);

    if (tc->lysc_tree) {
        child_exists = tro_next_child(tc->cn, tc) ? 1 : 0;
    } else {
        child_exists = tro_next_child(tc->pn, tc) ? 1 : 0;
    }

    i = 0;
    while ((ext = trb_ext_iter(tc, &i))) {
        struct lyspr_tree_ctx plug_ctx = {0};

        rc = tro_ext_printer_tree(tc->lysc_tree, ext, &plug_ctx, &ignore);
        LY_CHECK_ERR_GOTO(rc, tc->last_error = rc, end);
        if (ignore) {
            ignore = 0;
            continue;
        }
        trb_ext_try_unified_indent(&plug_ctx, ca, &max_gap_before_type, pc, tc);
        if (plug_ctx.schemas) {
            last_instance = i;
        }
        trp_ext_free_plugin_ctx(&plug_ctx);
    }

    if (child_exists) {
        pc->fp.modify.next_child(ca, tc);
        max = trb_try_unified_indent(ca, pc, tc);
        max_gap_before_type = max > max_gap_before_type ? max : max_gap_before_type;
        pc->fp.modify.parent(tc);
    }

    i = 0;
    while ((ext = trb_ext_iter(tc, &i))) {
        struct lyspr_tree_ctx plug_ctx = {0};

        rc = tro_ext_printer_tree(tc->lysc_tree, ext, &plug_ctx, &ignore);
        LY_CHECK_ERR_GOTO(rc, tc->last_error = rc, end);
        if (ignore) {
            ignore = 0;
            continue;
        }
        if (!child_exists && (last_instance == i)) {
            trb_ext_print_schemas(&plug_ctx, 1, max_gap_before_type, wr, ca, pc, tc);
        } else {
            trb_ext_print_schemas(&plug_ctx, 0, max_gap_before_type, wr, ca, pc, tc);
        }
        trp_ext_free_plugin_ctx(&plug_ctx);
    }

end:
    return;
}

/**
 * @brief Print subtree of nodes.
 *
 * The current node is expected to be the root of the subtree.
 * Before root node is no linebreak printing. This must be addressed by
 * the caller. Root node will also be printed. Behind last printed node
 * is no linebreak.
 *
 * @param[in,out] node current processed node used as iterator.
 * @param[in] max_gap_before_type is result from
 * ::trb_try_unified_indent() function for root node.
 * Set parameter to 0 if distance does not matter.
 * @param[in] wr is wrapper saying how deep in the whole tree
 * is the root of the subtree.
 * @param[in] ca is parent_cache from root's parent.
 * If root is top-level node, insert ::TRP_EMPTY_PARENT_CACHE.
 * @param[in] pc is @ref TRP_trp settings.
 * @param[in,out] tc is context of tree printer.
 */
static void
trb_print_subtree_nodes(struct trt_node *node, uint32_t max_gap_before_type, struct trt_wrapper wr,
        struct trt_parent_cache ca, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    if (!trp_node_is_empty(node)) {
        /* Print root node. */
        trb_print_entire_node(node, max_gap_before_type, wr, pc, tc);
        if (trp_ext_is_present_in_node(tc)) {
            trb_ext_print_instances(wr, ca, pc, tc);
        }
        /* if node is last sibling, then do not add '|' to wrapper */
        wr = trb_node_is_last_sibling(&pc->fp, tc) ?
                trp_wrapper_set_shift(wr) : trp_wrapper_set_mark(wr);
        /* go to the child */
        ca = tro_parent_cache_for_child(ca, tc);
        *node = pc->fp.modify.next_child(ca, tc);
        if (trp_node_is_empty(node)) {
            return;
        }
        /* TODO comment browse through instances + filtered. try unified indentation for children */
        max_gap_before_type = trb_try_unified_indent(ca, pc, tc);
    } else {
        /* Root node is ignored, continue with child. */
        *node = pc->fp.modify.first_sibling(ca, tc);
    }

    do {
        if (!tc->plugin_ctx.filtered && !trb_need_implicit_node_case(tc)) {
            /* normal behavior */
            ly_print_(pc->out, "\n");
            trb_print_subtree_nodes(node, max_gap_before_type, wr, ca, pc, tc);
        } else if (!tc->plugin_ctx.filtered) {
            struct trt_wrapper wr_case_child;

            wr_case_child = trb_print_implicit_node(node, wr, pc, tc);
            trb_print_subtree_nodes(node, max_gap_before_type, wr_case_child, ca, pc, tc);
        }
        /* go to the actual node's sibling */
        *node = pc->fp.modify.next_sibling(ca, tc);
    } while (!trp_node_is_empty(node));

    /* get back from child node to root node */
    pc->fp.modify.parent(tc);
}

/**
 * @brief Print all parents and their children.
 *
 * This function is suitable for printing top-level nodes that
 * do not have ancestors. Function call ::trb_print_subtree_nodes()
 * for all top-level siblings. Use this function after 'module' keyword
 * or 'augment' and so. The nodes may not be exactly top-level in the
 * tree, but the function considers them that way.
 *
 * @param[in] wr is wrapper saying how deeply the top-level nodes are
 * immersed in the tree.
 * @param[pc] pc contains mainly functions for printing.
 * @param[in,out] tc is tree context.
 */
static void
trb_print_family_tree(struct trt_wrapper wr, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_parent_cache ca;
    struct trt_node node;
    uint32_t max_gap_before_type;

    if (!tro_tree_ctx_get_node(tc)) {
        return;
    }

    ca = TRP_EMPTY_PARENT_CACHE;
    max_gap_before_type = trb_try_unified_indent(ca, pc, tc);

    if (!tc->lysc_tree) {
        if ((tc->section == TRD_SECT_GROUPING) && (tc->tpn == tc->pn->parent)) {
            ca.lys_config = 0x0;
        }
    }

    for (node = pc->fp.modify.first_sibling(ca, tc);
            !trp_node_is_empty(&node);
            node = pc->fp.modify.next_sibling(ca, tc)) {
        ly_print_(pc->out, "\n");
        trb_print_subtree_nodes(&node, max_gap_before_type, wr, ca, pc, tc);
    }
}

/**********************************************************************
 * Definition of trm main functions
 *********************************************************************/

/**
 * @brief Settings if lysp_node are used for browsing through the tree.
 *
 * @param[in] module YANG schema tree structure representing
 * YANG module.
 * @param[in] out is output handler.
 * @param[in] max_line_length is the maximum line length limit
 * that should not be exceeded.
 * @param[in,out] pc will be adapted to lysp_tree.
 * @param[in,out] tc will be adapted to lysp_tree.
 */
static void
trm_lysp_tree_ctx(const struct lys_module *module, struct ly_out *out, size_t max_line_length,
        struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    *tc = (struct trt_tree_ctx) {
        .lysc_tree = 0,
        .section = TRD_SECT_MODULE,
        .pmod = module->parsed,
        .cmod = NULL,
        .pn = module->parsed ? module->parsed->data : NULL,
        .tpn = module->parsed ? module->parsed->data : NULL,
        .cn = NULL,
        .last_error = 0,
        .plugin_ctx = {
            .ctx = NULL,
            .schema = NULL,
            .filtered = 0,
            .node_overr = TRP_TREE_CTX_EMPTY_NODE_OVERR,
            .last_schema = 1,
            .last_error = 0
        }

    };

    pc->out = out;

    pc->fp.modify = TRP_TRT_FP_MODIFY_PARSED;
    pc->fp.read = TRP_TRT_FP_READ_PARSED;

    pc->fp.print = (struct trt_fp_print) {
        .print_features_names = tro_print_features_names,
        .print_keys = tro_print_keys
    };

    pc->max_line_length = max_line_length;
}

/**
 * @brief Settings if lysc_node are used for browsing through the tree.
 *
 * Pointers to current nodes will be set to module data.
 *
 * @param[in] module YANG schema tree structure representing
 * YANG module.
 * @param[in] out is output handler.
 * @param[in] max_line_length is the maximum line length limit
 * that should not be exceeded.
 * @param[in,out] pc will be adapted to lysc_tree.
 * @param[in,out] tc will be adapted to lysc_tree.
 */
static void
trm_lysc_tree_ctx(const struct lys_module *module, struct ly_out *out, size_t max_line_length,
        struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    *tc = (struct trt_tree_ctx) {
        .lysc_tree = 1,
        .section = TRD_SECT_MODULE,
        .pmod = module->parsed,
        .cmod = module->compiled,
        .tpn = NULL,
        .pn = NULL,
        .cn = module->compiled->data,
        .last_error = 0,
        .plugin_ctx = {
            .ctx = NULL,
            .schema = NULL,
            .filtered = 0,
            .node_overr = TRP_TREE_CTX_EMPTY_NODE_OVERR,
            .last_schema = 1,
            .last_error = 0
        }

    };

    pc->out = out;

    pc->fp.modify = TRP_TRT_FP_MODIFY_COMPILED;
    pc->fp.read = TRP_TRT_FP_READ_COMPILED;

    pc->fp.print = (struct trt_fp_print) {
        .print_features_names = tro_print_features_names,
        .print_keys = tro_print_keys
    };

    pc->max_line_length = max_line_length;
}

/**
 * @brief Reset settings to browsing through the lysc tree.
 * @param[in,out] pc resets to @ref TRP_troc functions.
 * @param[in,out] tc resets to lysc browsing.
 */
static void
trm_reset_to_lysc_tree_ctx(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    LY_ERR erc;

    erc = tc->last_error;
    trp_ext_free_node_override(&tc->plugin_ctx.node_overr, &tc->plugin_ctx.filtered);
    trm_lysc_tree_ctx(tc->pmod->mod, pc->out, pc->max_line_length, pc, tc);
    tc->last_error = erc;
}

/**
 * @brief Reset settings to browsing through the lysp tree.
 * @param[in,out] pc resets to @ref TRP_trop functions.
 * @param[in,out] tc resets to lysp browsing.
 */
static void
trm_reset_to_lysp_tree_ctx(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    LY_ERR erc;

    erc = tc->last_error;
    trp_ext_free_node_override(&tc->plugin_ctx.node_overr, &tc->plugin_ctx.filtered);
    trm_lysp_tree_ctx(tc->pmod->mod, pc->out, pc->max_line_length, pc, tc);
    tc->last_error = erc;
}

/**
 * @brief If augment's target node is located on the current module.
 * @param[in] pn is examined augment.
 * @param[in] pmod is current module.
 * @return 1 if nodeid refers to the local node, otherwise 0.
 */
static ly_bool
trm_nodeid_target_is_local(const struct lysp_node_augment *pn, const struct lysp_module *pmod)
{
    const char *id, *prefix, *name;
    size_t prefix_len, name_len;
    const struct lys_module *mod;
    ly_bool ret = 0;

    if (pn == NULL) {
        return ret;
    }

    id = pn->nodeid;
    if (!id) {
        return ret;
    }
    /* only absolute-schema-nodeid is taken into account */
    assert(id[0] == '/');
    ++id;

    ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len);
    if (prefix) {
        mod = ly_resolve_prefix(pmod->mod->ctx, prefix, prefix_len, LY_VALUE_SCHEMA, pmod);
        ret = mod ? (mod->parsed == pmod) : 0;
    } else {
        ret = 1;
    }

    return ret;
}

/**
 * @brief Printing section module, rpcs, notifications or yang-data.
 *
 * First node must be the first child of 'module',
 * 'rpcs', 'notifications' or 'yang-data'.
 *
 * @param[in] ks is section representation.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_section_as_family_tree(struct trt_keyword_stmt ks, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    assert(ks.section_name);

    trp_print_keyword_stmt(ks, pc->max_line_length, pc->out);
    if (!strcmp(ks.section_name, TRD_KEYWORD_MODULE) || !strcmp(ks.section_name, TRD_KEYWORD_SUBMODULE)) {
        trb_print_family_tree(TRP_INIT_WRAPPER_TOP, pc, tc);
    } else {
        trb_print_family_tree(TRP_INIT_WRAPPER_BODY, pc, tc);
    }
}

/**
 * @brief Printing section augment or grouping.
 *
 * First node is 'augment' or 'grouping' itself.
 *
 * @param[in] ks is section representation.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_section_as_subtree(struct trt_keyword_stmt ks, struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    assert(ks.section_name);
    trp_print_keyword_stmt(ks, pc->max_line_length, pc->out);
    trb_tree_ctx_set_child(tc);
    trb_print_family_tree(TRP_INIT_WRAPPER_BODY, pc, tc);
}

/**
 * @brief Print 'module' keyword, its name and all nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_module_section(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trm_print_section_as_family_tree(pc->fp.read.module_name(tc), pc, tc);
}

/**
 * @brief For all augment sections: print 'augment' keyword,
 * its target node and all nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_augmentations(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    ly_bool once;
    ly_bool origin_was_lysc_tree = 0;
    struct trt_keyword_stmt ks;

    if (tc->lysc_tree) {
        origin_was_lysc_tree = 1;
        trm_reset_to_lysp_tree_ctx(pc, tc);
    }

    once = 1;
    for (ks = trop_modi_next_augment(tc); ks.section_name; ks = trop_modi_next_augment(tc)) {

        if (origin_was_lysc_tree) {
            /* if lysc tree is used, then only augments targeting
             * another module are printed
             */
            if (trm_nodeid_target_is_local((const struct lysp_node_augment *)tc->tpn, tc->pmod)) {
                continue;
            }
        }

        if (once) {
            ly_print_(pc->out, "\n");
            ly_print_(pc->out, "\n");
            once = 0;
        } else {
            ly_print_(pc->out, "\n");
        }

        trm_print_section_as_subtree(ks, pc, tc);
    }

    if (origin_was_lysc_tree) {
        trm_reset_to_lysc_tree_ctx(pc, tc);
    }
}

/**
 * @brief For rpcs section: print 'rpcs' keyword and all its nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_rpcs(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_keyword_stmt rpc;

    rpc = tro_modi_get_rpcs(tc);

    if (rpc.section_name) {
        ly_print_(pc->out, "\n");
        ly_print_(pc->out, "\n");
        trm_print_section_as_family_tree(rpc, pc, tc);
    }
}

/**
 * @brief For notifications section: print 'notifications' keyword
 * and all its nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_notifications(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    struct trt_keyword_stmt notifs;

    notifs = tro_modi_get_notifications(tc);

    if (notifs.section_name) {
        ly_print_(pc->out, "\n");
        ly_print_(pc->out, "\n");
        trm_print_section_as_family_tree(notifs, pc, tc);
    }
}

/**
 * @brief For all grouping sections: print 'grouping' keyword, its name
 * and all nodes.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_groupings(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    ly_bool once;
    struct trt_keyword_stmt ks;

    if (tc->lysc_tree) {
        return;
    }

    once = 1;
    for (ks = trop_modi_next_grouping(tc); ks.section_name; ks = trop_modi_next_grouping(tc)) {
        if (once) {
            ly_print_(pc->out, "\n");
            ly_print_(pc->out, "\n");
            once = 0;
        } else {
            ly_print_(pc->out, "\n");
        }
        trm_print_section_as_subtree(ks, pc, tc);
    }
}

/**
 * @brief Print all sections defined in plugins.
 *
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_plugin_ext(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    LY_ERR rc;
    ly_bool once;
    LY_ARRAY_COUNT_TYPE i = 0, j;
    struct trt_keyword_stmt ks, prev_ks = {0};
    struct trt_printer_ctx pc_dupl;
    struct trt_tree_ctx tc_dupl;
    struct trt_node node;
    ly_bool ignore = 0;
    uint32_t max_gap_before_type;
    void *ext;

    tc->section = TRD_SECT_PLUG_DATA;

    tc_dupl = *tc;
    pc_dupl = *pc;

    once = 1;

    while ((ext = trb_mod_ext_iter(tc, &i))) {
        struct lyspr_tree_ctx plug_ctx = {0};

        rc = tro_ext_printer_tree(tc->lysc_tree, ext, &plug_ctx, &ignore);
        LY_CHECK_ERR_GOTO(rc, tc->last_error = rc, end);
        if (!plug_ctx.schemas || ignore) {
            ignore = 0;
            continue;
        }

        ks = tro_get_ext_section(tc, ext, &plug_ctx);
        if (once || (prev_ks.section_name && strcmp(prev_ks.section_name, ks.section_name))) {
            ly_print_(pc->out, "\n");
            ly_print_(pc->out, "\n");
            once = 0;
        } else {
            ly_print_(pc->out, "\n");
        }
        trp_print_keyword_stmt(ks, pc->max_line_length, pc->out);

        max_gap_before_type = 0;
        trb_ext_try_unified_indent(&plug_ctx, TRP_EMPTY_PARENT_CACHE, &max_gap_before_type, pc, tc);
        LY_ARRAY_FOR(plug_ctx.schemas, j) {
            trm_reset_tree_ctx_by_plugin(&plug_ctx, j, pc, tc);
            node = TRP_EMPTY_NODE;
            trb_print_subtree_nodes(&node, max_gap_before_type, TRP_INIT_WRAPPER_BODY, TRP_EMPTY_PARENT_CACHE, pc, tc);
        }

        *tc = tc_dupl;
        trp_ext_free_plugin_ctx(&plug_ctx);
        prev_ks = ks;
    }

end:
    *pc = pc_dupl;
    return;
}

/**
 * @brief Print sections module, augment, rpcs, notifications,
 * grouping, yang-data.
 * @param[in] pc contains mainly functions for printing.
 * @param[in,out] tc is the tree context.
 */
static void
trm_print_sections(struct trt_printer_ctx *pc, struct trt_tree_ctx *tc)
{
    trm_print_module_section(pc, tc);
    trm_print_augmentations(pc, tc);
    trm_print_rpcs(pc, tc);
    trm_print_notifications(pc, tc);
    trm_print_groupings(pc, tc);
    trm_print_plugin_ext(pc, tc);
    ly_print_(pc->out, "\n");
}

static LY_ERR
tree_print_check_error(struct ly_out_clb_arg *out, struct trt_tree_ctx *tc)
{
    if (out->last_error) {
        return out->last_error;
    } else if (tc->last_error) {
        return tc->last_error;
    } else {
        return LY_SUCCESS;
    }
}

/**********************************************************************
 * Definition of module interface
 *********************************************************************/

LY_ERR
tree_print_module(struct ly_out *out, const struct lys_module *module, uint32_t UNUSED(options), size_t line_length)
{
    struct trt_printer_ctx pc;
    struct trt_tree_ctx tc;
    struct ly_out *new_out;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = TRP_INIT_LY_OUT_CLB_ARG(TRD_PRINT, out, 0, LY_SUCCESS);

    LY_CHECK_ARG_RET3(module->ctx, out, module, module->parsed, LY_EINVAL);

    if ((erc = ly_out_new_clb(&trp_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    if ((module->ctx->flags & LY_CTX_SET_PRIV_PARSED) && module->compiled) {
        trm_lysc_tree_ctx(module, new_out, line_length, &pc, &tc);
    } else {
        trm_lysp_tree_ctx(module, new_out, line_length, &pc, &tc);
    }

    trm_print_sections(&pc, &tc);
    erc = tree_print_check_error(&clb_arg, &tc);

    ly_out_free(new_out, NULL, 1);

    return erc;
}

LY_ERR
tree_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options, size_t line_length)
{
    struct trt_printer_ctx pc;
    struct trt_tree_ctx tc;
    struct ly_out *new_out;
    struct trt_wrapper wr;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = TRP_INIT_LY_OUT_CLB_ARG(TRD_PRINT, out, 0, LY_SUCCESS);

    assert(out && node);

    if (!(node->module->ctx->flags & LY_CTX_SET_PRIV_PARSED)) {
        return LY_EINVAL;
    }

    if ((erc = ly_out_new_clb(&trp_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    trm_lysc_tree_ctx(node->module, new_out, line_length, &pc, &tc);

    trp_print_keyword_stmt(pc.fp.read.module_name(&tc), pc.max_line_length, pc.out);
    trb_print_parents(node, NULL, &pc, &tc);

    if (!(options & LYS_PRINT_NO_SUBSTMT)) {
        tc.cn = lysc_node_child(node);
        wr = trb_count_depth(NULL, tc.cn);
        trb_print_family_tree(wr, &pc, &tc);
    }
    ly_print_(out, "\n");

    erc = tree_print_check_error(&clb_arg, &tc);
    ly_out_free(new_out, NULL, 1);

    return erc;
}

LY_ERR
tree_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t UNUSED(options),
        size_t line_length)
{
    struct trt_printer_ctx pc;
    struct trt_tree_ctx tc;
    struct ly_out *new_out;
    LY_ERR erc;
    struct ly_out_clb_arg clb_arg = TRP_INIT_LY_OUT_CLB_ARG(TRD_PRINT, out, 0, LY_SUCCESS);

    assert(submodp);
    LY_CHECK_ARG_RET(submodp->mod->ctx, out, LY_EINVAL);

    if ((erc = ly_out_new_clb(&trp_ly_out_clb_func, &clb_arg, &new_out))) {
        return erc;
    }

    line_length = line_length == 0 ? SIZE_MAX : line_length;
    trm_lysp_tree_ctx(submodp->mod, new_out, line_length, &pc, &tc);
    tc.pmod = (struct lysp_module *)submodp;
    tc.tpn = submodp->data;
    tc.pn = tc.tpn;

    trm_print_sections(&pc, &tc);
    erc = tree_print_check_error(&clb_arg, &tc);

    ly_out_free(new_out, NULL, 1);

    return erc;
}
