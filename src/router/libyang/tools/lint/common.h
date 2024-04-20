/**
 * @file common.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool - common functions and definitions for both interactive and non-interactive mode.
 *
 * Copyright (c) 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <stdio.h>

#include "libyang.h"

#define PROMPT "> "

/**
 * @brief Default context creation options.
 */
#define YL_DEFAULT_CTX_OPTIONS LY_CTX_NO_YANGLIBRARY

/**
 * @brief Default data parsing flags.
 */
#define YL_DEFAULT_DATA_PARSE_OPTIONS LYD_PARSE_STRICT

/**
 * @brief Default data validation flags.
 */
#define YL_DEFAULT_DATA_VALIDATE_OPTIONS LYD_VALIDATE_MULTI_ERROR

/**
 * @brief log error message
 */
#define YLMSG_E(...) \
        yl_log(1, __VA_ARGS__);

/**
 * @brief log warning message
 */
#define YLMSG_W(...) \
        yl_log(0, __VA_ARGS__);

#ifndef _WIN32
# define PATH_SEPARATOR ":"
#else
# define PATH_SEPARATOR ";"
#endif

struct cmdline_file;

/**
 * @brief Log a yanglint message.
 *
 * @param[in] err Whether the message is an error or a warning.
 * @param[in] format Message format.
 * @param[in] ... Format arguments.
 */
void yl_log(ly_bool err, const char *format, ...);

/**
 * @brief Parse path of a schema module file into the directory and module name.
 *
 * @param[in] path Schema module file path to be parsed.
 * @param[out] dir Pointer to the directory path where the file resides. Caller is expected to free the returned string.
 * @param[out] module Pointer to the name of the module (without file suffixes or revision information) specified by the
 * @p path. Caller is expected to free the returned string.
 * @return 0 on success
 * @return -1 on error
 */
int parse_schema_path(const char *path, char **dir, char **module);

/**
 * @brief Get input handler for the specified path.
 *
 * Using the @p format_schema and @p format_data the type of the file can be limited (by providing NULL) or it can be
 * got known if both types are possible.
 *
 * @param[in] filepath Path of the file to open.
 * @param[out] format_schema Format of the schema detected from the file name. If NULL specified, the schema formats are
 * prohibited and such files are refused.
 * @param[out] format_data Format of the data detected from the file name. If NULL specified, the data formats are
 * prohibited and such files are refused.
 * @param[out] in Created input handler referring the file behind the @p filepath. Can be NULL.
 * @return 0 on success.
 * @return -1 on failure.
 */
int get_input(const char *filepath, LYS_INFORMAT *format_schema, LYD_FORMAT *format_data, struct ly_in **in);

/**
 * @brief Get schema format of the @p filename's content according to the @p filename's suffix.
 *
 * @param[in] filename Name of the file to examine.
 * @return Detected schema input format.
 */
LYS_INFORMAT get_schema_format(const char *filename);

/**
 * @brief Get data format of the @p filename's content according to the @p filename's suffix.
 *
 * @param[in] filename Name of the file to examine.
 * @return Detected data input format.
 */
LYD_FORMAT get_data_format(const char *filename);

/**
 * @brief Get format of the @p filename's content according to the @p filename's suffix.
 *
 * Either the @p schema or @p data parameter is set.
 *
 * @param[in] filepath Name of the file to examine.
 * @param[out] schema_form Pointer to a variable to store the input schema format.
 * @param[out] data_form Pointer to a variable to store the expected input data format.
 * @return zero in case a format was successfully detected.
 * @return nonzero in case it is not possible to get valid format from the @p filename.
 */
int get_format(const char *filepath, LYS_INFORMAT *schema_form, LYD_FORMAT *data_form);

/**
 * @brief Get the node specified by the path.
 *
 * @param[in] ctx libyang context with schema.
 * @param[in] schema_path Path to the wanted node.
 * @return Pointer to the schema node specified by the path on success, NULL otherwise.
 */
const struct lysc_node *find_schema_path(const struct ly_ctx *ctx, const char *schema_path);

/**
 * @brief General callback providing run-time extension instance data.
 *
 * @param[in] ext Compiled extension instance.
 * @param[in] user_data User-supplied callback data.
 * @param[out] ext_data Provided extension instance data.
 * @param[out] ext_data_free Whether the extension instance should free @p ext_data or not.
 * @return LY_ERR value.
 */
LY_ERR ext_data_clb(const struct lysc_ext_instance *ext, void *user_data, void **ext_data, ly_bool *ext_data_free);

/**
 * @brief Concatenation of paths into one string.
 *
 * @param[in,out] searchpaths Collection of paths in the single string. Paths are delimited by colon ":"
 * (on Windows, used semicolon ";" instead).
 * @param[in] path Path to add.
 * @return LY_ERR value.
 */
LY_ERR searchpath_strcat(char **searchpaths, const char *path);

#endif /* COMMON_H_ */
