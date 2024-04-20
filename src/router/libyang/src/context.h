/**
 * @file context.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief internal context structures and functions
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_CONTEXT_H_
#define LY_CONTEXT_H_

#include <stdint.h>

#include "log.h"
#include "parser_schema.h"
#include "tree_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lys_module;

/**
 * @page howtoContext Context
 *
 * The context concept allows callers to work in environments with different sets of YANG modules.
 *
 * The first step with libyang is to create a new context using ::ly_ctx_new(). It returns a handler used in the following work.
 * Note that the context is supposed to provide a stable environment for work with the data. Therefore the caller should prepare
 * a complete context and after starting working with the data, the context and its content should not change. If it does,
 * in most cases it leads to the context being recompiled and any parsed data invalid. Despite the API not enforcing this
 * approach, it may change in future versions in the form of a locking mechanism which would allow further
 * optimization of data manipulation. Also note that modules cannot be removed from their context. If you need to change the set
 * of the schema modules in the context, the recommended way is to create a new context. To remove the context, there is ::ly_ctx_destroy() function.
 *
 * The context has [several options](@ref contextoptions) changing behavior when processing YANG modules being inserted. The
 * specific behavior is mentioned below. All the options can be set as a parameter when the context is being created or later
 * with ::ly_ctx_set_options().
 *
 * When creating a new context, another optional parameter is search_dir It provide directory where libyang
 * will automatically search for YANG modules being imported or included. There is actually a set of search paths which can be later
 * modified using ::ly_ctx_set_searchdir(), ::ly_ctx_unset_searchdir() and ::ly_ctx_unset_searchdir_last() functions. Before the values
 * in the set are used, also the current working directory is (non-recursively) searched. For the case of the explicitly set
 * search directories, they are searched recursively - all their subdirectories (and symlinks) are taken into account. Searching
 * in the current working directory can be avoided with the context's ::LY_CTX_DISABLE_SEARCHDIR_CWD option.
 * Searching in all the context's search dirs (without removing them) can be avoided with the context's
 * ::LY_CTX_DISABLE_SEARCHDIRS option (or via ::ly_ctx_set_options()). This automatic searching can be preceded
 * by a custom  module searching callback (::ly_module_imp_clb) set via ::ly_ctx_set_module_imp_clb(). The algorithm of
 * searching in search dirs is also available via API as ::lys_search_localfile() function.
 *
 * YANG modules are added into the context using [parser functions](@ref howtoSchemaParsers) - \b lys_parse*().
 * Alternatively, also ::ly_ctx_load_module() can be used - in that case the ::ly_module_imp_clb or automatic
 * search in search directories and in the current working directory is used, as described above. YANG submodules cannot be loaded
 * or even validated directly, they are loaded always only as includes of YANG modules. Explicitly parsed/loaded modules are
 * handled as implemented - libyang is able to instantiate data representing such a module. The modules loaded implicitly, are
 * not implemented and serve only as a source of grouping or typedef definitions. Context can hold multiple revisions of the same
 * YANG module, but only one of them can be implemented. Details about the difference between implemented and imported modules
 * can be found on @ref howtoSchema page. This behavior can be changed with the context's ::LY_CTX_ALL_IMPLEMENTED option, which
 * causes that all the parsed modules, whether loaded explicitly or implicitly, are set to be implemented. Note, that as
 * a consequence of this option, only a single revision of any module can be present in the context in this case. Also, a less
 * crude option ::LY_CTX_REF_IMPLEMENTED can be used to implement only referenced modules that should also be implemented.
 *
 * When loading/importing a module without revision, the latest revision of the required module is supposed to load.
 * For a context, the first time the latest revision of a module is requested, it is properly searched for and loaded.
 * However, when this module is requested (without revision) the second time, the one found previously is returned.
 * This has the advantage of not searching for the module repeatedly but there is a drawback in case the content of search
 * directories is updated and a later revision become available.
 *
 * Context holds all the schema modules internally. To get a specific module, use ::ly_ctx_get_module() (or some of its
 * variants). If you need to do something with all the modules in the context, it is advised to iterate over them using
 * ::ly_ctx_get_module_iter(). Alternatively, the ::ly_ctx_get_yanglib_data() function can be used to get complex information about the schemas in the context
 * in the form of data tree defined by <a href="https://tools.ietf.org/html/rfc7895">ietf-yang-library</a> module.
 *
 * YANG data can be parsed by \b lyd_parse_*() functions. Note, that functions for schema have \b lys_
 * prefix (or \b lysp_ for the parsed and \b lysc_ for the compiled schema - for details see @ref howtoSchema page) while
 * functions for instance data have \b lyd_ prefix. Details about data formats or handling data without the appropriate
 * YANG module in context can be found on @ref howtoData page.
 *
 * Besides the YANG modules, context holds also [error information](@ref howtoErrors) and
 * [database of strings](@ref howtoContextDict), both connected with the processed YANG modules and data.
 *
 * - @subpage howtoErrors
 * - @subpage howtoContextDict
 *
 * \note API for this group of functions is available in the [context module](@ref context).
 *
 * Functions List
 * --------------
 *
 * - ::ly_ctx_new()
 * - ::ly_ctx_destroy()
 *
 * - ::ly_ctx_set_searchdir()
 * - ::ly_ctx_get_searchdirs()
 * - ::ly_ctx_unset_searchdir()
 * - ::ly_ctx_unset_searchdir_last()
 *
 * - ::ly_ctx_set_options()
 * - ::ly_ctx_get_options()
 * - ::ly_ctx_unset_options()
 *
 * - ::ly_ctx_set_module_imp_clb()
 * - ::ly_ctx_get_module_imp_clb()
 *
 * - ::ly_ctx_load_module()
 * - ::ly_ctx_get_module_iter()
 * - ::ly_ctx_get_module()
 * - ::ly_ctx_get_module_ns()
 * - ::ly_ctx_get_module_implemented()
 * - ::ly_ctx_get_module_implemented_ns()
 * - ::ly_ctx_get_module_latest()
 * - ::ly_ctx_get_module_latest_ns()
 * - ::ly_ctx_get_submodule()
 * - ::ly_ctx_get_submodule_latest()
 * - ::ly_ctx_get_submodule2()
 * - ::ly_ctx_get_submodule2_latest()
 * - ::ly_ctx_reset_latests()
 *
 * - ::ly_ctx_get_yanglib_data()
 *
 * - ::ly_ctx_get_change_count()
 * - ::ly_ctx_internal_modules_count()
 *
 * - ::lys_search_localfile()
 * - ::lys_set_implemented()
 *
 */

/**
 * @defgroup context Context
 * @{
 *
 * Structures and functions to manipulate with the libyang context containers.
 *
 * The \em context concept allows callers to work in environments with different sets of YANG schemas.
 * More detailed information can be found at @ref howtoContext page.
 */

/**
 * @struct ly_ctx
 * @brief libyang context handler.
 */
struct ly_ctx;

/**
 * @ingroup context
 * @defgroup contextoptions Context options
 *
 * Options to change context behavior.
 *
 * @{
 */

#define LY_CTX_ALL_IMPLEMENTED 0x01 /**< All the imported modules of the schema being parsed are implemented. */
#define LY_CTX_REF_IMPLEMENTED 0x02 /**< Implement all imported modules "referenced" from an implemented module.
                                        Normally, leafrefs, augment and deviation targets are implemented as
                                        specified by YANG 1.1. In addition to this, implement any modules of
                                        nodes referenced by when and must conditions and by any default values.
                                        Generally, only if all these modules are implemented, the explicitly
                                        implemented modules can be properly used and instantiated in data. */
#define LY_CTX_NO_YANGLIBRARY  0x04 /**< Do not internally implement ietf-yang-library module. The option
                                        causes that function ::ly_ctx_get_yanglib_data() does not work (returns ::LY_EINVAL) until
                                        the ietf-yang-library module is loaded manually. While any revision
                                        of this schema can be loaded with this option, note that the only
                                        revisions implemented by ::ly_ctx_get_yanglib_data() are 2016-06-21 and 2019-01-04.
                                        This option cannot be changed on existing context. */
#define LY_CTX_DISABLE_SEARCHDIRS 0x08  /**< Do not search for schemas in context's searchdirs neither in current
                                        working directory. It is entirely skipped and the only way to get
                                        schema data for imports or for ::ly_ctx_load_module() is to use the
                                        callbacks provided by caller via ::ly_ctx_set_module_imp_clb() */
#define LY_CTX_DISABLE_SEARCHDIR_CWD 0x10 /**< Do not automatically search for schemas in current working
                                        directory, which is by default searched automatically (despite not
                                        recursively). */
#define LY_CTX_PREFER_SEARCHDIRS 0x20 /**< When searching for schema, prefer searchdirs instead of user callback. */
#define LY_CTX_SET_PRIV_PARSED 0x40 /**< For all compiled nodes, their private objects (::lysc_node.priv) are used
                                        by libyang as a reference to the corresponding parsed node (::lysp_node).
                                        The exception are \"case\" statements, which are omitted (shorthand),
                                        in that case the private objects are set to NULL.
                                        So if this option is set, the user must not change private objects.
                                        Setting this option by ::ly_ctx_set_options() may result in context recompilation.
                                        Resetting this option by ::ly_ctx_unset_options() cause that private
                                        objects will be set to NULL. */
#define LY_CTX_EXPLICIT_COMPILE 0x80 /**< If this flag is set, the compiled modules and their schema nodes are
                                        not automatically updated (compiled) on any context changes. In other words, they do
                                        not immediately take effect. To do that, call ::ly_ctx_compile(). Changes
                                        requiring compilation include adding new modules, changing their features,
                                        and implementing parsed-only modules. This option allows efficient compiled
                                        context creation without redundant recompilations. */
#define LY_CTX_ENABLE_IMP_FEATURES 0x0100 /**< By default, all features of newly implemented imported modules of
                                        a module that is being loaded are disabled. With this flag they all become
                                        enabled. */
#define LY_CTX_LEAFREF_EXTENDED 0x0200 /**< By default, path attribute of leafref accepts only path as defined in RFC 7950.
                                        By using this option, the path attribute will also allow using XPath functions as deref() */

/** @} contextoptions */

/**
 * @brief Create libyang context.
 *
 * Context is used to hold all information about schemas. Usually, the application is supposed
 * to work with a single context in which libyang is holding all schemas (and other internal
 * information) according to which the data trees will be processed and validated. So, the schema
 * trees are tightly connected with the specific context and they are held by the context internally
 * - caller does not need to keep pointers to the schemas returned by ::lys_parse(), context knows
 * about them. The data trees created with \b lyd_parse_*() are still connected with the specific context,
 * but they are not internally held by the context. The data tree just points and lean on some data
 * held by the context (schema tree, string dictionary, etc.). Therefore, in case of data trees, caller
 * is supposed to keep pointers returned by the \b lyd_parse_*() functions and manage the data tree on its own. This
 * also affects the number of instances of both tree types. While you can have only one instance of
 * specific schema connected with a single context, number of data tree instances is not connected.
 *
 * @param[in] search_dir Directory (or directories) where libyang will search for the imported or included modules
 * and submodules. If no such directory is available, NULL is accepted. Several directories can be specified,
 * delimited by colon ":" (on Windows, use semicolon ";" instead).
 * @param[in] options Context options, see @ref contextoptions.
 * @param[out] new_ctx Pointer to the created libyang context if LY_SUCCESS returned.
 * @return LY_ERR return value.
 */
LIBYANG_API_DECL LY_ERR ly_ctx_new(const char *search_dir, uint16_t options, struct ly_ctx **new_ctx);

/**
 * @brief Create libyang context according to the provided yang-library data in a file.
 *
 * This function loads the yang-library data from the given path. If you need to pass the data as
 * string, use ::::ly_ctx_new_ylmem(). Both functions extend functionality of ::ly_ctx_new() by loading
 * modules specified in the ietf-yang-library form into the context being created.
 * The preferred tree model revision is 2019-01-04. However, only the first module-set is processed and loaded
 * into the context. If there are no matching nodes from this tree, the legacy tree (originally from model revision 2016-04-09)
 * is processed. Note, that the modules are loaded the same way as in case of ::ly_ctx_load_module(), so the schema paths in the
 * yang-library data are ignored and the modules are loaded from the context's search locations. On the other hand, YANG features
 * of the modules are set as specified in the yang-library data.
 * To get yang library data from a libyang context, use ::ly_ctx_get_yanglib_data().
 *
 * @param[in] search_dir Directory where libyang will search for the imported or included modules and submodules.
 * If no such directory is available, NULL is accepted.
 * @param[in] path Path to the file containing yang-library-data in the specified format
 * @param[in] format Format of the data in the provided file.
 * @param[in] options Context options, see @ref contextoptions.
 * @param[in,out] ctx If *ctx is not NULL, the existing libyang context is modified.  Otherwise, a pointer to a
 * newly created context is returned here if LY_SUCCESS.
 * @return LY_ERR return value
 */
LIBYANG_API_DECL LY_ERR ly_ctx_new_ylpath(const char *search_dir, const char *path, LYD_FORMAT format, int options,
        struct ly_ctx **ctx);

/**
 * @brief Create libyang context according to the provided yang-library data in a string.
 *
 * Details in ::ly_ctx_new_ylpath().
 *
 * @param[in] search_dir Directory where libyang will search for the imported or included modules and submodules.
 * If no such directory is available, NULL is accepted.
 * @param[in] data String containing yang-library data in the specified format.
 * @param[in] format Format of the data in the provided file.
 * @param[in] options Context options, see @ref contextoptions.
 * @param[in,out] ctx If *ctx is not NULL, the existing libyang context is modified.  Otherwise, a pointer to a
 * newly created context is returned here if LY_SUCCESS.
 * @return LY_ERR return value
 */
LIBYANG_API_DECL LY_ERR ly_ctx_new_ylmem(const char *search_dir, const char *data, LYD_FORMAT format, int options,
        struct ly_ctx **ctx);

/**
 * @brief Create libyang context according to the provided yang-library data in a data tree.
 *
 * Details in ::ly_ctx_new_ylpath().
 *
 * @param[in] search_dir Directory where libyang will search for the imported or included modules and submodules.
 * If no such directory is available, NULL is accepted.
 * @param[in] tree Data tree containing yang-library data.
 * @param[in] options Context options, see @ref contextoptions.
 * @param[in,out] ctx If *ctx is not NULL, the existing libyang context is modified.  Otherwise, a pointer to a
 * newly created context is returned here if LY_SUCCESS.
 * @return LY_ERR return value
 */
LIBYANG_API_DECL LY_ERR ly_ctx_new_yldata(const char *search_dir, const struct lyd_node *tree, int options,
        struct ly_ctx **ctx);

/**
 * @brief Compile (recompile) the context applying all the performed changes after the last context compilation.
 * Should be used only if ::LY_CTX_EXPLICIT_COMPILE option is set, has no effect otherwise.
 *
 * @param[in] ctx Context to compile.
 * @return LY_ERR return value.
 */
LIBYANG_API_DECL LY_ERR ly_ctx_compile(struct ly_ctx *ctx);

/**
 * @brief Add the search path into libyang context
 *
 * To reset search paths set in the context, use ::ly_ctx_unset_searchdir() and then
 * set search paths again.
 *
 * @param[in] ctx Context to be modified.
 * @param[in] search_dir New search path to add to the current paths previously set in ctx.
 * @return LY_ERR return value.
 */
LIBYANG_API_DECL LY_ERR ly_ctx_set_searchdir(struct ly_ctx *ctx, const char *search_dir);

/**
 * @brief Clean the search path(s) from the libyang context
 *
 * To remove the recently added search path(s), use ::ly_ctx_unset_searchdir_last().
 *
 * @param[in] ctx Context to be modified.
 * @param[in] value Searchdir to be removed, use NULL to remove them all.
 * @return LY_ERR return value
 */
LIBYANG_API_DECL LY_ERR ly_ctx_unset_searchdir(struct ly_ctx *ctx, const char *value);

/**
 * @brief Remove the least recently added search path(s) from the libyang context.
 *
 * To remove a specific search path by its value, use ::ly_ctx_unset_searchdir().
 *
 * @param[in] ctx Context to be modified.
 * @param[in] count Number of the searchdirs to be removed (starting by the least recently added).
 * If the value is higher then the actual number of search paths, all paths are removed and no error is returned.
 * Value 0 does not change the search path set.
 * @return LY_ERR return value
 */
LIBYANG_API_DECL LY_ERR ly_ctx_unset_searchdir_last(struct ly_ctx *ctx, uint32_t count);

/**
 * @brief Get the NULL-terminated list of the search paths in libyang context. Do not modify the result!
 *
 * @param[in] ctx Context to query.
 * @return NULL-terminated list (array) of the search paths, NULL if no searchpath was set.
 * Do not modify the provided data in any way!
 */
LIBYANG_API_DECL const char * const *ly_ctx_get_searchdirs(const struct ly_ctx *ctx);

/**
 * @brief Get the currently set context's options.
 *
 * @param[in] ctx Context to query.
 * @return Combination of all the currently set context's options, see @ref contextoptions.
 */
LIBYANG_API_DECL uint16_t ly_ctx_get_options(const struct ly_ctx *ctx);

/**
 * @brief Set some of the context's options, see @ref contextoptions.
 * @param[in] ctx Context to be modified.
 * @param[in] option Combination of the context's options to be set, see @ref contextoptions.
 * If there is to be a change to ::LY_CTX_SET_PRIV_PARSED, the context will be recompiled
 * and all ::lysc_node.priv in the modules will be overwritten, see ::LY_CTX_SET_PRIV_PARSED.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR ly_ctx_set_options(struct ly_ctx *ctx, uint16_t option);

/**
 * @brief Unset some of the context's options, see @ref contextoptions.
 * @param[in] ctx Context to be modified.
 * @param[in] option Combination of the context's options to be unset, see @ref contextoptions.
 * @return LY_ERR value.
 */
LIBYANG_API_DECL LY_ERR ly_ctx_unset_options(struct ly_ctx *ctx, uint16_t option);

/**
 * @brief Get the change count of the context (module set) during its life-time.
 *
 * @param[in] ctx Context to be examined.
 * @return Context change count.
 */
LIBYANG_API_DECL uint16_t ly_ctx_get_change_count(const struct ly_ctx *ctx);

/**
 * @brief Get the hash of all the modules in the context. Since order of the modules is significant,
 * even when 2 contexts have the same modules but loaded in a different order, the hash will differ.
 *
 * Hash consists of all module names (1), their revisions (2), all enabled features (3), and their
 * imported/implemented state (4).
 *
 * @param[in] ctx Context to be examined.
 * @return Context modules hash.
 */
LIBYANG_API_DECL uint32_t ly_ctx_get_modules_hash(const struct ly_ctx *ctx);

/**
 * @brief Callback for freeing returned module data in #ly_module_imp_clb.
 *
 * @param[in] module_data Data to free.
 * @param[in] user_data User-supplied callback data, same as for #ly_module_imp_clb.
 */
typedef void (*ly_module_imp_data_free_clb)(void *module_data, void *user_data);

/**
 * @brief Callback for retrieving missing included or imported models in a custom way.
 *
 * When @p submod_name is provided, the submodule is requested instead of the module (in this case only
 * the module name without its revision is provided).
 *
 * If an @arg free_module_data callback is provided, it will be used later to free the allegedly const data
 * which were returned by this callback.
 *
 * @param[in] mod_name Missing module name.
 * @param[in] mod_rev Optional missing module revision. If NULL and submod_name is not provided, the latest revision is
 * requested, the parsed module is then marked by the latest_revision flag.
 * @param[in] submod_name Optional missing submodule name.
 * @param[in] submod_rev Optional missing submodule revision. If NULL and submod_name is provided, the latest revision is
 * requested, the parsed submodule is then marked by the latest_revision flag.
 * @param[in] user_data User-supplied callback data.
 * @param[out] format Format of the returned module data.
 * @param[out] module_data Requested module data.
 * @param[out] free_module_data Callback for freeing the returned module data. If not set, the data will be left untouched.
 * @return LY_ERR value. If the returned value differs from LY_SUCCESS, libyang continue in trying to get the module data
 * according to the settings of its mechanism to search for the imported/included schemas.
 */
typedef LY_ERR (*ly_module_imp_clb)(const char *mod_name, const char *mod_rev, const char *submod_name, const char *submod_rev,
        void *user_data, LYS_INFORMAT *format, const char **module_data, ly_module_imp_data_free_clb *free_module_data);

/**
 * @brief Get the custom callback for missing import/include module retrieval.
 *
 * @param[in] ctx Context to read from.
 * @param[in] user_data Optional pointer for getting the user-supplied callback data.
 * @return Callback or NULL if not set.
 */
LIBYANG_API_DECL ly_module_imp_clb ly_ctx_get_module_imp_clb(const struct ly_ctx *ctx, void **user_data);

/**
 * @brief Set missing include or import module callback. It is meant to be used when the models
 * are not locally available (such as when downloading modules from a NETCONF server), it should
 * not be required in other cases.
 *
 * @param[in] ctx Context that will use this callback.
 * @param[in] clb Callback responsible for returning the missing model.
 * @param[in] user_data Arbitrary data that will always be passed to the callback @p clb.
 */
LIBYANG_API_DECL void ly_ctx_set_module_imp_clb(struct ly_ctx *ctx, ly_module_imp_clb clb, void *user_data);

/**
 * @brief Callback for getting arbitrary run-time data required by an extension instance.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] user_data User-supplied callback data.
 * @param[out] ext_data Provided extension instance data.
 * @param[out] ext_data_free Whether the extension instance should free @p ext_data or not.
 * @return LY_ERR value.
 */
typedef LY_ERR (*ly_ext_data_clb)(const struct lysc_ext_instance *ext, void *user_data, void **ext_data,
        ly_bool *ext_data_free);

/**
 * @brief Set callback providing run-time extension instance data. The expected data depend on the extension.
 * Data expected by internal extensions:
 *
 * - *ietf-yang-schema-mount:mount-point* (struct lyd_node \*\*ext_data)\n
 * Operational data tree with at least `ietf-yang-library` data describing the mounted schema and
 * `ietf-yang-schema-mount` **validated** data describing the specific mount point
 * ([ref](https://datatracker.ietf.org/doc/html/rfc8528#section-3.3)).
 *
 * @param[in] ctx Context that will use this callback.
 * @param[in] clb Callback responsible for returning the extension instance data.
 * @param[in] user_data Arbitrary data that will always be passed to the callback @p clb.
 */
LIBYANG_API_DECL ly_ext_data_clb ly_ctx_set_ext_data_clb(struct ly_ctx *ctx, ly_ext_data_clb clb, void *user_data);

/**
 * @brief Get YANG module of the given name and revision.
 *
 * @param[in] ctx Context to work in.
 * @param[in] name Name of the YANG module to get.
 * @param[in] revision Requested revision date of the YANG module to get. If not specified,
 * the schema with no revision is returned, if it is present in the context.
 * @return Pointer to the YANG module, NULL if no schema in the context follows the name and revision requirements.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module(const struct ly_ctx *ctx, const char *name, const char *revision);

/**
 * @brief Get the latest revision of the YANG module specified by its name.
 *
 * YANG modules with no revision are supposed to be the oldest one.
 *
 * @param[in] ctx Context where to search.
 * @param[in] name Name of the YANG module to get.
 * @return The latest revision of the specified YANG module in the given context, NULL if no YANG module of the
 * given name is present in the context.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module_latest(const struct ly_ctx *ctx, const char *name);

/**
 * @brief Get the (only) implemented YANG module specified by its name.
 *
 * @param[in] ctx Context where to search.
 * @param[in] name Name of the YANG module to get.
 * @return The only implemented YANG module revision of the given name in the given context. NULL if there is no
 * implemented module of the given name.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module_implemented(const struct ly_ctx *ctx, const char *name);

/**
 * @brief Iterate over all modules in the given context.
 *
 * @param[in] ctx Context with the modules.
 * @param[in,out] index Index of the next module to get. Value of 0 starts from the beginning.
 * The value is updated with each call, so to iterate over all modules the same variable is supposed
 * to be used in all calls starting with value 0.
 * @return Next context module, NULL if the last was already returned.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module_iter(const struct ly_ctx *ctx, uint32_t *index);

/**
 * @brief Get YANG module of the given namespace and revision.
 *
 * @param[in] ctx Context to work in.
 * @param[in] ns Namespace of the YANG module to get.
 * @param[in] revision Requested revision date of the YANG module to get. If not specified,
 * the schema with no revision is returned, if it is present in the context.
 * @return Pointer to the YANG module, NULL if no schema in the context follows the namespace and revision requirements.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module_ns(const struct ly_ctx *ctx, const char *ns, const char *revision);

/**
 * @brief Get the latest revision of the YANG module specified by its namespace.
 *
 * YANG modules with no revision are supposed to be the oldest one.
 *
 * @param[in] ctx Context where to search.
 * @param[in] ns Namespace of the YANG module to get.
 * @return The latest revision of the specified YANG module in the given context, NULL if no YANG module of the
 * given namespace is present in the context.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module_latest_ns(const struct ly_ctx *ctx, const char *ns);

/**
 * @brief Get the (only) implemented YANG module specified by its namespace.
 *
 * @param[in] ctx Context where to search.
 * @param[in] ns Namespace of the YANG module to get.
 * @return The only implemented YANG module revision of the given namespace in the given context. NULL if there is no
 * implemented module of the given namespace.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_get_module_implemented_ns(const struct ly_ctx *ctx, const char *ns);

/**
 * @brief Get a specific submodule from context. If its belongs-to module is known, use ::ly_ctx_get_submodule2().
 *
 * @param[in] ctx libyang context to search in.
 * @param[in] submodule Submodule name to find.
 * @param[in] revision Revision of the submodule to find, NULL for a submodule without a revision.
 * @return Found submodule, NULL if there is none.
 */
LIBYANG_API_DECL const struct lysp_submodule *ly_ctx_get_submodule(const struct ly_ctx *ctx, const char *submodule,
        const char *revision);

/**
 * @brief Get the latests revision of a submodule from context. If its belongs-to module is known,
 * use ::ly_ctx_get_submodule2_latest().
 *
 * @param[in] ctx libyang context to search in.
 * @param[in] submodule Submodule name to find.
 * @return Found submodule, NULL if there is none.
 */
LIBYANG_API_DECL const struct lysp_submodule *ly_ctx_get_submodule_latest(const struct ly_ctx *ctx, const char *submodule);

/**
 * @brief Get a specific submodule from a module. If the belongs-to module is not known, use ::ly_ctx_get_submodule().
 *
 * @param[in] module Belongs-to module to search in.
 * @param[in] submodule Submodule name to find.
 * @param[in] revision Revision of the submodule to find, NULL for a submodule without a revision.
 * @return Found submodule, NULL if there is none.
 */
LIBYANG_API_DECL const struct lysp_submodule *ly_ctx_get_submodule2(const struct lys_module *module, const char *submodule,
        const char *revision);

/**
 * @brief Get the latest revision of a submodule from a module. If the belongs-to module is not known,
 * use ::ly_ctx_get_submodule_latest().
 *
 * @param[in] module Belongs-to module to search in.
 * @param[in] submodule Submodule name to find.
 * @return Found submodule, NULL if there is none.
 */
LIBYANG_API_DECL const struct lysp_submodule *ly_ctx_get_submodule2_latest(const struct lys_module *module,
        const char *submodule);

/**
 * @brief Reset cached latest revision information of the schemas in the context.
 *
 * This function is deprecated and should not be used.
 *
 * When a (sub)module is imported/included without revision, the latest revision is
 * searched. libyang searches for the latest revision in searchdirs and/or via provided
 * import callback ::ly_module_imp_clb() just once. Then it is expected that the content
 * of searchdirs or data returned by the callback does not change. So when it changes,
 * it is necessary to force searching for the latest revision in case of loading another
 * module, which what this function does.
 *
 * The latest revision information is also reset when the searchdirs set changes via
 * ::ly_ctx_set_searchdir().
 *
 * @param[in] ctx libyang context where the latest revision information is going to be reset.
 */
LIBYANG_API_DECL void ly_ctx_reset_latests(struct ly_ctx *ctx);

/**
 * @brief Learn the number of internal modules of a context. Internal modules
 * is considered one that was loaded during the context creation.
 *
 * @param[in] ctx libyang context to examine.
 * @return Number of internal modules.
 */
LIBYANG_API_DECL uint32_t ly_ctx_internal_modules_count(const struct ly_ctx *ctx);

/**
 * @brief Try to find the model in the searchpaths of \p ctx and load it into it. If custom missing
 * module callback is set, it is used instead.
 *
 * The context itself is searched for the requested module first. If \p revision is not specified
 * (the module of the latest revision is requested) and there is implemented revision of the requested
 * module in the context, this implemented revision is returned despite there might be a newer revision.
 * This behavior is cause by the fact that it is not possible to have multiple implemented revisions of
 * the same module in the context.
 *
 * @param[in] ctx Context to add to.
 * @param[in] name Name of the module to load.
 * @param[in] revision Optional revision date of the module. If not specified, the latest revision is loaded.
 * @param[in] features Optional array of features ended with NULL to be enabled if the module is being implemented.
 * The feature string '*' enables all and array of length 1 with only the terminating NULL explicitly disables all
 * the features. In case the parameter is NULL, the features are untouched - left disabled in newly loaded module or
 * with the current features settings in case the module is already present in the context.
 * @return Pointer to the data model structure, NULL if not found or some error occurred.
 */
LIBYANG_API_DECL struct lys_module *ly_ctx_load_module(struct ly_ctx *ctx, const char *name, const char *revision,
        const char **features);

/**
 * @brief Get data of the internal ietf-yang-library module with information about all the loaded modules.
 * ietf-yang-library module must be loaded.
 *
 * Note that "/ietf-yang-library:yang-library/datastore" list instances are not created and should be
 * appended by the caller. There is a single "/ietf-yang-library:yang-library/schema" instance created
 * with the key value "complete".
 *
 * If the data identifier can be limited to the existence and changes of this context, the following
 * last 2 parameters can be used:
 *
 * "%u" as @p content_id_format and ::ly_ctx_get_change_count() as its parameter;
 * "%u" as @p content_id_format and ::ly_ctx_get_modules_hash() as its parameter.
 *
 * @param[in] ctx Context with the modules.
 * @param[out] root Generated yang-library data.
 * @param[in] content_id_format Format string (printf-like) for the yang-library data identifier, which is
 * the "content_id" node in the 2019-01-04 revision of ietf-yang-library.
 * @param[in] ... Parameters for @p content_id_format.
 * @return LY_ERR value
 */
LIBYANG_API_DECL LY_ERR ly_ctx_get_yanglib_data(const struct ly_ctx *ctx, struct lyd_node **root,
        const char *content_id_format, ...);

/**
 * @brief Free all internal structures of the specified context.
 *
 * The function should be used before terminating the application to destroy
 * and free all structures internally used by libyang. If the caller uses
 * multiple contexts, the function should be called for each used context.
 *
 * All instance data are supposed to be freed before destroying the context using ::lyd_free_all(), for example.
 * Data models (schemas) are destroyed automatically as part of ::ly_ctx_destroy() call.
 *
 * Note that the data stored by user into the ::lysc_node.priv pointer are kept
 * untouched and the caller is responsible for freeing this private data.
 *
 * @param[in] ctx libyang context to destroy
 */
LIBYANG_API_DECL void ly_ctx_destroy(struct ly_ctx *ctx);

/** @} context */

#ifdef __cplusplus
}
#endif

#endif /* LY_CONTEXT_H_ */
