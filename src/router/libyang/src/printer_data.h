/**
 * @file printer_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data printers for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_DATA_H_
#define LY_PRINTER_DATA_H_

#include <stdint.h>
#include <stdio.h>

#include "log.h"
#include "out.h"
#include "tree_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_out;

/**
 * @page howtoDataPrinters Printing Data
 *
 * Data printers allows to serialize internal representation of a data tree in a specific format. libyang
 * supports the following data formats for printing:
 *
 * - XML
 *
 *   Basic format as specified in rules of mapping YANG modeled data to XML in
 *   [RFC 6020](http://tools.ietf.org/html/rfc6020).
 *
 * - JSON
 *
 *   The alternative data format available in RESTCONF protocol. Specification of JSON encoding of data modeled by YANG
 *   can be found in [RFC 7951](https://tools.ietf.org/html/rfc7951).
 *
 * By default, both formats are printed with indentation (formatting), which can be avoided by ::LYD_PRINT_SHRINK
 * [printer option](@ref dataprinterflags)). Other options adjust e.g. [with-defaults mode](@ref howtoDataWD).
 *
 * Besides the legacy functions from libyang 1.x (::lyd_print_clb(), ::lyd_print_fd(), ::lyd_print_file(), ::lyd_print_mem()
 * and ::lyd_print_path()) printing data into the specified output, there are also new functions using
 * [output handler](@ref howtoOutput) introduced in libyang 2.0. In contrast to
 * [schema printers](@ref howtoSchemaPrinters), there is no limit of the functionality and the functions can be used
 * interchangeable. The only think to note is that the new functions ::lyd_print_all() and ::lyd_print_tree() does not
 * accept ::LYD_PRINT_WITHSIBLINGS [printer option](@ref dataprinterflags)) since this flag differentiate the functions
 * themselves.
 *
 * Functions List
 * --------------
 * - ::lyd_print_all()
 * - ::lyd_print_tree()
 * - ::lyd_print_mem()
 * - ::lyd_print_fd()
 * - ::lyd_print_file()
 * - ::lyd_print_path()
 * - ::lyd_print_clb()
 */

/**
 * @ingroup datatree
 * @defgroup dataprinterflags Data printer flags
 *
 * Options to change default behavior of the data printers.
 *
 * @{
 */
#define LYD_PRINT_WITHSIBLINGS  0x01             /**< Flag for printing also the (following) sibling nodes of the data node.
                                                      The flag is not allowed for ::lyd_print_all() and ::lyd_print_tree(). */
#define LYD_PRINT_SHRINK        LY_PRINT_SHRINK  /**< Flag for output without indentation and formatting new lines. */
#define LYD_PRINT_KEEPEMPTYCONT 0x04             /**< Preserve empty non-presence containers */
#define LYD_PRINT_WD_MASK       0xF0             /**< Mask for with-defaults modes */
#define LYD_PRINT_WD_EXPLICIT   0x00             /**< Explicit with-defaults mode. Only the data explicitly being present in
                                                      the data tree are printed, so the implicitly added default nodes are
                                                      not printed. Note that this is the default value when no WD option is
                                                      specified. */
#define LYD_PRINT_WD_TRIM       0x10             /**< Trim mode avoids printing the nodes with the value equal to their
                                                      default value */
#define LYD_PRINT_WD_ALL        0x20             /**< With this option, all the nodes are printed as none of them are
                                                      considered default */
#define LYD_PRINT_WD_ALL_TAG    0x40             /**< Same as ::LYD_PRINT_WD_ALL but also adds attribute 'default' with value 'true' to
                                                      all nodes that has its default value. The 'default' attribute has namespace:
                                                      urn:ietf:params:xml:ns:netconf:default:1.0 and thus the attributes are
                                                      printed only when the ietf-netconf-with-defaults module is present in libyang
                                                      context (but in that case this namespace is always printed). */
#define LYD_PRINT_WD_IMPL_TAG   0x80             /**< Same as ::LYD_PRINT_WD_ALL_TAG but the attributes are added only to the nodes that
                                                      are not explicitly present in the original data tree despite their
                                                      value is equal to their default value.  There is the same limitation regarding
                                                      the presence of ietf-netconf-with-defaults module in libyang context. */
/**
 * @}
 */

/**
 * @brief Print the whole data tree of the root, including all the siblings.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] root The root element of the tree to print, can be any sibling.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags) except ::LYD_PRINT_WITHSIBLINGS.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_all(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, uint32_t options);

/**
 * @brief Print the selected data subtree.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] root The root element of the subtree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags) except ::LYD_PRINT_WITHSIBLINGS.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_tree(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, uint32_t options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, uint32_t options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, uint32_t options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] f File stream where to print the data.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, uint32_t options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] path File path where to print the data.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, uint32_t options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] user_data Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR lyd_print_clb(ly_write_clb writeclb, void *user_data, const struct lyd_node *root,
        LYD_FORMAT format, uint32_t options);

/**
 * @brief Check whether the node should be printed based on the printing options.
 *
 * @param[in] node Node to check.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return 0 if not,
 * @return non-0 if should be printed.
 */
LIBYANG_API_DECL ly_bool lyd_node_should_print(const struct lyd_node *node, uint32_t options);

#ifdef __cplusplus
}
#endif

#endif /* LY_PRINTER_DATA_H_ */
