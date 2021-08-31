/**
 * @file printer_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internal structures and functions for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_INTERNAL_H_
#define LY_PRINTER_INTERNAL_H_

#include "out.h"
#include "printer_data.h"
#include "printer_schema.h"

struct lysp_module;
struct lysp_submodule;

/**
 * @brief Generic YANG schema printer context
 *
 * Note that the YANG extensions API provides getter to the members for the extension plugins.
 */
struct lyspr_ctx {
    struct ly_out *out;              /**< output specification */
    uint16_t level;                  /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    uint32_t options;                /**< Schema output options (see @ref schemaprinterflags). */
    const struct lys_module *module; /**< schema to print */
};

/**
 * @brief YANG printer of the parsed module. Full YANG printer.
 *
 * @param[in] out Output specification.
 * @param[in] modp Parsed module to print.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR yang_print_parsed_module(struct ly_out *out, const struct lysp_module *modp, uint32_t options);

/**
 * @brief Helper macros for data printers
 */
#define DO_FORMAT (!(ctx->options & LY_PRINT_SHRINK))
#define LEVEL ctx->level                      /**< current level */
#define INDENT (DO_FORMAT ? (LEVEL)*2 : 0),"" /**< indentation parameters for printer functions */
#define LEVEL_INC LEVEL++                     /**< increase indentation level */
#define LEVEL_DEC LEVEL--                     /**< decrease indentation level */

#define XML_NS_INDENT 8

/**
 * @brief YANG printer of the parsed submodule. Full YANG printer.
 *
 * @param[in] out Output specification.
 * @param[in] submodp Parsed submodule to print.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR yang_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t options);

/**
 * @brief YANG printer of the compiled schemas.
 *
 * This printer provides information about modules how they are understood by libyang.
 * Despite the format is inspired by YANG, it is not fully compatible and should not be
 * used as a standard YANG format.
 *
 * @param[in] out Output specification.
 * @param[in] module Schema to be printed (the compiled member is used).
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR yang_print_compiled(struct ly_out *out, const struct lys_module *module, uint32_t options);

/**
 * @brief YANG printer of the compiled schema node
 *
 * This printer provides information about modules how they are understood by libyang.
 * Despite the format is inspired by YANG, it is not fully compatible and should not be
 * used as a standard YANG format.
 *
 * @param[in] out Output specification.
 * @param[in] node Schema node to be printed including all its substatements.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR yang_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options);

/**
 * @brief YIN printer of the parsed module. Full YIN printer.
 *
 * @param[in] out Output specification.
 * @param[in] modp Parsed module to print.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR yin_print_parsed_module(struct ly_out *out, const struct lysp_module *modp, uint32_t options);

/**
 * @brief YIN printer of the parsed submodule. Full YIN printer.
 *
 * @param[in] out Output specification.
 * @param[in] submodp Parsed submodule to print.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR yin_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t options);

/**
 * @brief Full YANG Tree Diagram printer.
 *
 * The module should be compiled and the @ref contextoptions must be set to LY_CTX_SET_PRIV_PARSED.
 * If not, the printer will use parsed (unresolved) YANG schema tree, which means,
 * for example, that `grouping` sections will be on the output.
 *
 * @param[in] out Output specification.
 * @param[in] module Main module.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for ::LYS_OUT_TREE printer.
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR tree_print_module(struct ly_out *out, const struct lys_module *module, uint32_t options,
        size_t line_length);

/**
 * @brief YANG Tree Diagram printer of the parsed submodule. Full Tree printer.
 *
 * @param[in] out Output specification.
 * @param[in] submodp Parsed submodule to print.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for ::LYS_OUT_TREE printer.
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR tree_print_parsed_submodule(struct ly_out *out, const struct lysp_submodule *submodp, uint32_t options,
        size_t line_length);

/**
 * @brief YANG Tree Diagram printer of the compiled schema node.
 *
 * @param[in] out Output specification.
 * @param[in] node Schema node to be printed including all its substatements.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for ::LYS_OUT_TREE printer.
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR tree_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options,
        size_t line_length);

/**
 * @brief XML printer of YANG data.
 *
 * @param[in] out Output specification.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR xml_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options);

/**
 * @brief JSON printer of YANG data.
 *
 * @param[in] out Output specification.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR json_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options);

/**
 * @brief LYB printer of YANG data.
 *
 * @param[in] out Output structure.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value, number of the printed bytes is updated in ::ly_out.printed.
 */
LY_ERR lyb_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options);

#endif /* LY_PRINTER_INTERNAL_H_ */
