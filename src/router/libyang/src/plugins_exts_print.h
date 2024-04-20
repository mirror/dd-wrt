/**
 * @file plugins_exts_print.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extensions implementation - schema print related items.
 *
 * Copyright (c) 2015 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_PRINT_H_
#define LY_PLUGINS_EXTS_PRINT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup pluginsExtensionsPrint Plugins: Extensions printer support
 * @ingroup pluginsExtensions
 *
 * Helper functions to implement extension plugin's sprinter callback.
 *
 * @{
 */

/**
 * @brief YANG printer context for use in ::lyplg_ext_schema_printer_clb callback implementation.
 *
 * The structure provides basic information how the compiled schema is supposed to be printed and where. In the most simple
 * case, the provided context is just passed into ::lysc_print_extension_instance() function which handles printing the
 * extension's substatements in the standard way.
 *
 * To access various items from the context, use some of the following lys_ypr_ctx_get_* getters.
 */
struct lyspr_ctx;

/**
 * @brief YANG printer context getter for output handler.
 * @param[in] ctx YANG printer context.
 * @return Output handler where the data are being printed. Note that the address of the handler pointer in the context is
 * returned to allow to modify the handler.
 */
struct ly_out **lys_ypr_ctx_get_out(const struct lyspr_ctx *ctx);

/**
 * @brief YANG printer context getter for printer options.
 * @param[in] ctx YANG printer context.
 * @return pointer to the printer options to allow modifying them with @ref schemaprinterflags values.
 */
uint32_t *lys_ypr_ctx_get_options(const struct lyspr_ctx *ctx);

/**
 * @brief YANG printer context getter for printer indentation level.
 * @param[in] ctx YANG printer context.
 * @return pointer to the printer's indentation level to allow modifying its value.
 */
uint16_t *lys_ypr_ctx_get_level(const struct lyspr_ctx *ctx);

/**
 * @brief Print substatements of an extension instance
 *
 * Generic function to access YANG printer functions from the extension plugins (::lyplg_ext_schema_printer_clb).
 *
 * @param[in] ctx YANG printer context to provide output handler and other information for printing.
 * @param[in] ext The compiled extension instance to access the extensions and substatements data.
 * @param[in, out] flag Flag to be shared with the caller regarding the opening brackets - 0 if the '{' not yet printed,
 * 1 otherwise.
 */
void lysc_print_extension_instance(struct lyspr_ctx *ctx, const struct lysc_ext_instance *ext, ly_bool *flag);

/** @} pluginsExtensionsPrint */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_PRINT_H_ */
