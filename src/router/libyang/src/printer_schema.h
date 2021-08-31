/**
 * @file printer_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema printers for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_SCHEMA_H_
#define LY_PRINTER_SCHEMA_H_

#include <stdint.h>
#include <stdio.h>

#include "log.h"
#include "out.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_out;
struct lys_module;
struct lysc_node;
struct lysp_submodule;

/**
 * @page howtoSchemaPrinters Module Printers
 *
 * Schema printers allows to serialize internal representations of a schema module in a specific format. libyang
 * supports the following schema formats for printing:
 *
 * - YANG
 *
 *   Basic YANG schemas format described in [RFC 6020](http://tools.ietf.org/html/rfc6020) and
 *   [RFC 7951](http://tools.ietf.org/html/rfc7951) (so both YANG 1.0 and YANG 1.1 versions are supported).
 *
 * - YANG compiled
 *
 *   Syntactically, this format is based on standard YANG format. In contrast to standard YANG format, YANG compiled format
 *   represents the module how it is used by libyang - with all uses expanded, augments and deviations applied, etc.
 *   (more details about the compiled modules can be found on @ref howtoContext page).
 *
 * - YIN
 *
 *   Alternative XML-based format to YANG - YANG Independent Notation. The details can be found in
 *   [RFC 6020](http://tools.ietf.org/html/rfc6020#section-11) and
 *   [RFC 7951](http://tools.ietf.org/html/rfc7951#section-13).
 *
 * - Tree Diagram
 *
 *   Simple tree diagram providing overview of the module. The details can be found in
 *   [RFC 8340](https://tools.ietf.org/html/rfc8340).
 *
 * For simpler transition from libyang 1.x (and for some simple use cases), there are functions (::lys_print_clb(),
 * ::lys_print_fd(), ::lys_print_file() and ::lys_print_mem()) to print the complete module into the specified output. But note,
 * that these functions are limited to print only the complete module.
 *
 * The full functionality of the schema printers is available via functions using [output handler](@ref howtoOutput). Besides
 * the ::lys_print_module() function to print the complete module, there are functions to print a submodule
 * (::lys_print_submodule()) or a subtree (::lys_print_node()). Note that these functions might not support all the output
 * formats mentioned above.
 *
 * Functions List
 * --------------
 * - ::lys_print_module()
 * - ::lys_print_submodule()
 * - ::lys_print_node()
 *
 * - ::lys_print_clb()
 * - ::lys_print_fd()
 * - ::lys_print_file()
 * - ::lys_print_mem()
 * - ::lys_print_path()
 */

/**
 * @addtogroup schematree
 * @{
 */

/**
 * @defgroup schemaprinterflags Schema output options
 *
 * Options to change default behavior of the schema printers.
 *
 * @{
 */
#define LYS_PRINT_SHRINK             LY_PRINT_SHRINK /**< Flag for output without indentation and formatting new lines. */
#define LYS_PRINT_NO_SUBSTMT         0x10            /**< Print only top-level/referede node information,
                                                          do not print information from the substatements */
// #define LYS_PRINT_TREE_RFC        0x01 /**< Conform to the RFC8340 tree output (only for tree format) */
// #define LYS_PRINT_TREE_GROUPING   0x02 /**< Print groupings separately (only for tree format) */
// #define LYS_PRINT_TREE_USES       0x04 /**< Print only uses instead the resolved grouping nodes (only for tree format) */
// #define LYS_PRINT_TREE_NO_LEAFREF 0x08 /**< Do not print the target of leafrefs (only for tree format) */

/** @} schemaprinterflags */

/**
 * @brief Schema output formats accepted by libyang [printer functions](@ref howtoSchemaPrinters).
 */
typedef enum {
    LYS_OUT_UNKNOWN = 0, /**< unknown format, used as return value in case of error */
    LYS_OUT_YANG = 1,    /**< YANG schema output format */
    LYS_OUT_YANG_COMPILED = 2, /**< YANG schema output format of the compiled schema tree */
    LYS_OUT_YIN = 3,     /**< YIN schema output format */
    LYS_OUT_TREE         /**< Tree schema output format */
} LYS_OUTFORMAT;

/**
 * @brief Schema module printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] module Main module with the parsed schema to print.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_module(struct ly_out *out, const struct lys_module *module, LYS_OUTFORMAT format, size_t line_length,
        uint32_t options);

/**
 * @brief Schema submodule printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] submodule Parsed submodule to print.
 * @param[in] format Output format (LYS_OUT_YANG_COMPILED is not supported).
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_submodule(struct ly_out *out, const struct lysp_submodule *submodule, LYS_OUTFORMAT format,
        size_t line_length, uint32_t options);

/**
 * @brief Print schema tree in the specified format into a memory block.
 * It is up to caller to free the returned string by free().
 *
 * This is just a wrapper around ::lys_print_module() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options);

/**
 * @brief Print schema tree in the specified format into a file descriptor.
 *
 * This is just a wrapper around ::lys_print_module() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options);

/**
 * @brief Print schema tree in the specified format into a file stream.
 *
 * This is just a wrapper around ::lys_print_module() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] module Schema tree to print.
 * @param[in] f File stream where to print the schema.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options);

/**
 * @brief Print schema tree in the specified format into a file.
 *
 * This is just a wrapper around ::lys_print_module() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] path File where to print the schema.
 * @param[in] module Schema tree to print.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options);

/**
 * @brief Print schema tree in the specified format using a provided callback.
 *
 * This is just a wrapper around ::lys_print_module() for simple use cases.
 * In case of a complex use cases, use lys_print with ly_out output handler.
 *
 * @param[in] module Schema tree to print.
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] user_data Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] format Schema output format.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_clb(ly_write_clb writeclb, void *user_data,
        const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options);

/**
 * @brief Schema node printer.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] node Schema node to print.
 * @param[in] format Output format.
 * @param[in] line_length Maximum characters to be printed on a line, 0 for unlimited. Only for #LYS_OUT_TREE printer.
 * @param[in] options Schema output options (see @ref schemaprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lys_print_node(struct ly_out *out, const struct lysc_node *node, LYS_OUTFORMAT format, size_t line_length, uint32_t options);

/** @} schematree */

#ifdef __cplusplus
}
#endif

#endif /* LY_PRINTER_SCHEMA_H_ */
