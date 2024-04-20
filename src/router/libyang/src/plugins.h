/**
 * @file plugins.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Plugins manipulation.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_H_
#define LY_PLUGINS_H_

#include "log.h"

struct lyplg_ext_record;
struct lyplg_type_record;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howtoPlugins Plugins
 *
 * libyang supports two types of plugins to better support generic features of YANG that need some specific code for
 * their specific instances in YANG modules. This is the case of YANG types, which are derived from YANG built-in types.
 * The description of a derived type can specify some additional requirements or restriction that cannot be implemented
 * generically and some special code is needed. The second case for libyang plugins are YANG extensions. For YANG extensions,
 * most of the specification is hidden in their description (e.g. allowed substatements or place of the extension
 * instantiation) and libyang is not able to process such a text in a generic way.
 *
 * In both cases, libyang provides API to get functionality implementing the specifics of each type or extension.
 * Furthermore, there are several internal plugins, implementing built-in data types and selected derived types and YANG
 * extensions. These internal plugins uses the same API and can be taken as examples for implementing user plugins. Internal
 * plugins are always loaded with the first created [context](@ref howtoContext) and unloaded with destroying the last one.
 * The external plugins are in the same phase loaded from the default directories specified at compile time via cmake
 * variables `PLUGINS_DIR` (where the `extensions` and `types` subdirectories are added for each plugin type) or separately
 * via `PLUGINS_DIR_EXTENSIONS` and `PLUGINS_DIR_TYPES` for each plugin type. The default directories can be replaced runtime
 * using environment variables `LIBYANG_TYPES_PLUGINS_DIR` and `LIBYANG_EXTENSIONS_PLUGINS_DIR`.
 *
 * Order of the plugins determines their priority. libyang searches for the first match with the extension and type, so the
 * firstly loaded plugin for the specific item is used. Since the internal plugins are loaded always before the external
 * plugins, the internal plugins cannot be replaced.
 *
 * There is also a separate function ::lyplg_add() to add a plugin anytime later. Note, that such a plugin is being used
 * after it is added with the lowest priority among other already loaded plugins. Also note that since all the plugins are
 * unloaded with the destruction of the last context, creating a new context after that starts the standard plugins
 * initiation and the manually added plugins are not loaded automatically.
 *
 * The following pages contain description of the API for creating user plugins.
 *
 * - @subpage howtoPluginsTypes
 * - @subpage howtoPluginsExtensions
 */

/**
 * @defgroup plugins Plugins
 * @{
 *
 */

/**
 * @brief Identifiers of the plugin type.
 */
enum LYPLG {
    LYPLG_TYPE,      /**< Specific type (typedef) */
    LYPLG_EXTENSION  /**< YANG extension */
};

/**
 * @brief Manually load a plugin file.
 *
 * Note, that a plugin can be loaded only if there is at least one context. The loaded plugins are connected with the
 * existence of a context. When all the contexts are destroyed, all the plugins are unloaded.
 *
 * @param[in] pathname Path to the plugin file. It can contain types or extensions plugins, both are accepted and correctly
 * loaded.
 *
 * @return LY_SUCCESS if the file contains valid plugin compatible with the library version.
 * @return LY_EDENIED in case there is no context and the plugin cannot be loaded.
 * @return LY_EINVAL when pathname is NULL or the plugin contains invalid content for this libyang version.
 * @return LY_ESYS when the plugin file cannot be loaded.
 */
LIBYANG_API_DECL LY_ERR lyplg_add(const char *pathname);

/**
 * @brief Manually load extension plugins from memory
 *
 * Note, that a plugin can be loaded only if there is at least one context. The loaded plugins are connected with the
 * existence of a context. When all the contexts are destroyed, all the plugins are unloaded.
 *
 * @param[in] ctx The context to which the plugin should be associated with. If NULL, the plugin is considered to be shared
 * between all existing contexts.
 * @param[in] version The version of plugin records.
 * @param[in] recs An array of plugin records provided by the plugin implementation. The array must be terminated by a zeroed
 * record.
 *
 * @return LY_SUCCESS if the plugins with compatible version were successfully loaded.
 * @return LY_EDENIED in case there is no context and the plugin cannot be loaded.
 * @return LY_EINVAL when recs is NULL or the plugin contains invalid content for this libyang version.
 */
LIBYANG_API_DECL LY_ERR lyplg_add_extension_plugin(struct ly_ctx *ctx, uint32_t version, const struct lyplg_ext_record *recs);

/**
 * @brief Manually load type plugins from memory
 *
 * Note, that a plugin can be loaded only if there is at least one context. The loaded plugins are connected with the
 * existence of a context. When all the contexts are destroyed, all the plugins are unloaded.
 *
 * @param[in] ctx The context to which the plugin should be associated with. If NULL, the plugin is considered to be shared
 * between all existing contexts.
 * @param[in] version The version of plugin records.
 * @param[in] recs An array of plugin records provided by the plugin implementation. The array must be terminated by a zeroed
 * record.
 *
 * @return LY_SUCCESS if the plugins with compatible version were successfully loaded.
 * @return LY_EDENIED in case there is no context and the plugin cannot be loaded.
 * @return LY_EINVAL when recs is NULL or the plugin contains invalid content for this libyang version.
 */
LIBYANG_API_DECL LY_ERR lyplg_add_type_plugin(struct ly_ctx *ctx, uint32_t version, const struct lyplg_type_record *recs);
/** @} plugins */

#ifdef __cplusplus
}
#endif

#endif /* LY_PLUGINS_H_ */
