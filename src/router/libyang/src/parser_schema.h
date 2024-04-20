/**
 * @file parser_schema.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema parsers for libyang
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_SCHEMA_H_
#define LY_PARSER_SCHEMA_H_

#ifdef __cplusplus
extern "C" {
#endif

struct ly_in;
struct lys_module;

/**
 * @page howtoSchemaParsers Parsing YANG Modules
 *
 * YANG module parsers allow to read YANG module from a specific format. libyang supports the following module formats:
 *
 * - YANG
 *
 *   Basic YANG schemas format described in [RFC 6020](http://tools.ietf.org/html/rfc6020) and
 *   [RFC 7951](http://tools.ietf.org/html/rfc7951) (so both YANG 1.0 and YANG 1.1 versions are supported).
 *
 * - YIN
 *
 *   Alternative XML-based format to YANG - YANG Independent Notation. The details can be found in
 *   [RFC 6020](http://tools.ietf.org/html/rfc6020#section-11) and
 *   [RFC 7951](http://tools.ietf.org/html/rfc7951#section-13).
 *
 * When the [context](@ref howtoContext) is created, it already contains the following YANG modules, which
 * are implemented internally by libyang:
 * - ietf-yang-metadata@2016-08-05
 * - yang@2020-06-17
 * - ietf-inet-types@2013-07-15
 * - ietf-yang-types@2013-07-15
 * - ietf-datastores@2018-02-14
 * - ietf-yang-library@2019-01-04
 *
 * The `yang` module is the libyang's internal module to provide namespace and definitions of for various YANG
 * attributes described in [RFC 7951](https://tools.ietf.org/html/rfc6243) (such as `insert` attribute for
 * edit-config's data).
 *
 * Other modules can be added to the context manually with the functions listed below. Besides them,
 * it is also possible to use ::ly_ctx_load_module() which tries to find the required module automatically - using
 * ::ly_module_imp_clb or automatic search in working directory and in the context's search directories. For details, see
 * [how the context works](@ref howtoContext).
 *
 * YANG modules are loaded in two steps. First, the input YANG/YIN data are parsed into \b lysp_* structures that reflect
 * the structure of the input module and submodule(s). Mostly just syntax checks are done, no reference or type checking is
 * performed in this step. If the module is supposed to be implemented, not just imported by another module, the second step
 * is to compile it. The compiled tree may significantly differ from the source (parsed) tree structure. All the references
 * are resolved, groupings are instantiated, types are resolved (and compiled by joining all the relevant restrictions
 * when derived from another types) and many other syntactical checks are done.
 *
 * There is the main parsing function ::lys_parse() wirking with the libyang [input handler](@ref howtoInput). However,
 * to simplify some of the usecases, it is also possible to use other functions accepting input data from various sources.
 *
 * Functions List
 * --------------
 * - ::lys_parse()
 * - ::lys_parse_mem()
 * - ::lys_parse_fd()
 * - ::lys_parse_path()
 *
 * - ::lys_search_localfile()
 * - ::ly_ctx_set_module_imp_clb()
 * - ::ly_ctx_load_module()
 */

/**
 * @addtogroup schematree
 * @{
 */

/**
 * @brief Schema input formats accepted by libyang [parser functions](@ref howtoSchemaParsers).
 */
typedef enum {
    LYS_IN_UNKNOWN = 0,  /**< unknown format, used as return value in case of error */
    LYS_IN_YANG = 1,     /**< YANG schema input format */
    LYS_IN_YIN = 3       /**< YIN schema input format */
} LYS_INFORMAT;

/**
 * @brief Load a schema into the specified context.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] in The input handle to provide the dumped data model in the specified format.
 * @param[in] format Format of the schema to parse. Can be 0 to try to detect format from the input handler.
 * @param[in] features Array of features to enable ended with NULL. If NULL, no features are enabled.
 * @param[out] module Optional parsed module.
 * @return LY_ERR value.
 */
LY_ERR lys_parse(struct ly_ctx *ctx, struct ly_in *in, LYS_INFORMAT format, const char **features,
        const struct lys_module **module);

/**
 * @brief Load a schema into the specified context.
 *
 * This function is comsidered for a simple use, if you have a complex usecase,
 * consider use of ::lys_parse() with a standalone input handler.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] data The string containing the dumped data model in the specified format.
 * @param[in] format Format of the schema to parse. Can be 0 to try to detect format from the input handler.
 * @param[out] module Optional parsed module.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const struct lys_module **module);

/**
 * @brief Read a schema from file descriptor into the specified context.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * This function is comsidered for a simple use, if you have a complex usecase,
 * consider use of ::lys_parse() with a standalone input handler.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the schema
 *            in the specified format.
 * @param[in] format Format of the schema to parse. Can be 0 to try to detect format from the input handler.
 * @param[out] module Optional parsed module.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const struct lys_module **module);

/**
 * @brief Load a schema into the specified context from a file.
 *
 * This function is comsidered for a simple use, if you have a complex usecase,
 * consider use of ::lys_parse() with a standalone input handler.
 *
 * @param[in] ctx libyang context where to process the data model.
 * @param[in] path Path to the file with the model in the specified format.
 * @param[in] format Format of the schema to parse. Can be 0 to try to detect format from the input handler.
 * @param[out] module Optional parsed module.
 * @return LY_ERR value.
 */
LY_ERR lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format, const struct lys_module **module);

/**
 * @brief Search for the schema file in the specified searchpaths.
 *
 * @param[in] searchpaths NULL-terminated array of paths to be searched (recursively). Current working
 * directory is searched automatically (but non-recursively if not in the provided list). Caller can use
 * result of the ::ly_ctx_get_searchdirs().
 * @param[in] cwd Flag to implicitly search also in the current working directory (non-recursively).
 * @param[in] name Name of the schema to find.
 * @param[in] revision Revision of the schema to find. If NULL, the newest found schema filepath is returned.
 * @param[out] localfile Mandatory output variable containing absolute path of the found schema. If no schema
 * complying the provided restriction is found, NULL is set.
 * @param[out] format Optional output variable containing expected format of the schema document according to the
 * file suffix.
 * @return LY_ERR value (LY_SUCCESS is returned even if the file is not found, then the *localfile is NULL).
 */
LY_ERR lys_search_localfile(const char * const *searchpaths, ly_bool cwd, const char *name, const char *revision,
        char **localfile, LYS_INFORMAT *format);

/** @} schematree */

#ifdef __cplusplus
}
#endif

#endif /* LY_PARSER_SCHEMA_H_ */
