/**
 * @file plugins_exts_compile.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang support for YANG extensions implementation - schema compilation related items.
 *
 * Copyright (c) 2015 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_EXTS_COMPILE_H_
#define LY_PLUGINS_EXTS_COMPILE_H_

#include <stdint.h>

#include "log.h"
#include "tree_schema.h"

struct ly_ctx;
struct lyd_node;
struct lysc_ctx;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup pluginsExtensionsCompile Plugins: Extensions compilation support
 * @ingroup pluginsExtensions
 *
 * Helper functions to implement extension plugin's compile callback.
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
 * @brief YANG schema compilation context for use in ::lyplg_ext_compile_clb callback implementation.
 *
 * The structure stores complex information connected with the schema compilation process. In the most simple case,
 * the callback is just supposed to pass the provided callback to ::lys_compile_extension_instance() functions.
 *
 * To access various items from the context, use some of the following lysc_ctx_get_* getters.
 */
struct lysc_ctx;

/**
 * @brief YANG schema compilation context getter for libyang context.
 * @param[in] ctx YANG schema compilation context.
 * @return libyang context connected with the compilation context.
 */
struct ly_ctx *lysc_ctx_get_ctx(const struct lysc_ctx *ctx);

/**
 * @brief YANG schema compilation context getter for compilation options.
 * @param[in] ctx YANG schema compilation context.
 * @return pointer to the compilation options to allow modifying them with @ref scflags values.
 */
uint32_t *lysc_ctx_get_options(const struct lysc_ctx *ctx);

/**
 * @brief YANG schema compilation context getter for path being currently processed.
 * @param[in] ctx YANG schema compilation context.
 * @return path identifying the place in schema being currently processed by the schema compiler.
 */
const char *lysc_ctx_get_path(const struct lysc_ctx *ctx);

/**
 * @brief Compile substatements of an extension instance.
 *
 * Uses standard libyang schema compiler to transform YANG statements into the compiled schema structures. The plugins are
 * supposed to use this function when the extension instance's substatements are supposed to be compiled in a standard way
 * (or if just the @ref scflags are enough to modify the compilation process).
 *
 * @param[in] ctx YANG schema compile context to track the compilation state.
 * @param[in] ext_p Parsed representation of the extension instance being processed.
 * @param[in,out] ext Compiled extension instance with the prepared ::lysc_ext_instance.substmts array, which will be updated
 * by storing the compiled data.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID if compilation of the substatements fails.
 * @return LY_ENOT if the extension is disabled (by if-feature) and should be ignored.
 */
LY_ERR lys_compile_extension_instance(struct lysc_ctx *ctx, const struct lysp_ext_instance *ext_p, struct lysc_ext_instance *ext);

/**
 * @brief Update path in the compile context, which is used for logging where the compilation failed.
 *
 * @param[in] ctx Compile context with the path.
 * @param[in] parent_module Module of the current node's parent to check difference with the currently processed module (taken from @p ctx).
 * @param[in] name Name of the node to update path with. If NULL, the last segment is removed. If the format is `{keyword}`, the following
 * call updates the segment to the form `{keyword='name'}` (to remove this compound segment, 2 calls with NULL @p name must be used).
 */
void lysc_update_path(struct lysc_ctx *ctx, struct lys_module *parent_module, const char *name);

/**
 * @brief Duplicate the compiled extension (definition) structure.
 *
 * @param[in] orig The extension structure to duplicate.
 * @return The duplicated structure to use.
 */
struct lysc_ext *lysc_ext_dup(struct lysc_ext *orig);

/** @} extensions */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_EXTS_COMPILE_H_ */
