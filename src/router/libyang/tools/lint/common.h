/**
 * @file common.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool - common functions and definitions for both interactive and non-interactive mode.
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
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
 * @brief log error message
 */
#define YLMSG_E(MSG, ...) \
        fprintf(stderr, "YANGLINT[E]: " MSG, ##__VA_ARGS__)

/**
 * @brief log warning message
 */
#define YLMSG_W(MSG, ...) \
        fprintf(stderr, "YANGLINT[W]: " MSG, ##__VA_ARGS__)

/**
 * @brief Storage for the list of the features (their names) in a specific YANG module.
 */
struct schema_features {
    char *module;
    char **features;
};

/**
 * @brief Data connected with a file provided on a command line as a file path.
 */
struct cmdline_file {
    struct ly_in *in;
    const char *path;
    LYD_FORMAT format;
};

/**
 * @brief Free the schema features list (struct schema_features *)
 * @param[in,out] flist The (struct schema_features *) to free.
 */
void free_features(void *flist);

/**
 * @brief Get the list of features connected with the specific YANG module.
 *
 * @param[in] fset The set of features information (struct schema_features *).
 * @param[in] module Name of the YANG module which features should be found.
 * @param[out] features Pointer to the list of features being returned.
 */
void get_features(struct ly_set *fset, const char *module, const char ***features);

/**
 * @brief Parse features being specified for the specific YANG module.
 *
 * Format of the input @p fstring is as follows: <module_name>:[<feature>,]*
 *
 * @param[in] fstring Input string to be parsed.
 * @param[in, out] fset Features information set (of struct schema_features *). The set is being filled.
 */
int parse_features(const char *fstring, struct ly_set *fset);

/**
 * @brief Parse path of a schema module file into the directory and module name.
 *
 * @param[in] path Schema module file path to be parsed.
 * @param[out] dir Pointer to the directory path where the file resides. Caller is expected to free the returned string.
 * @param[out] module Pointer to the name of the module (without file suffixes or revision information) specified by the
 * @path. Caller is expected to free the returned string.
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
 * @param[out] in Created input handler referring the file behind the @p filepath.
 * @return 0 on success.
 * @return -1 on failure.
 */
int get_input(const char *filepath, LYS_INFORMAT *format_schema, LYD_FORMAT *format_data, struct ly_in **in);

/**
 * @brief Free the command line file data (struct cmdline_file *)
 * @param[in,out] cmdline_file The (struct cmdline_file *) to free.
 */
void free_cmdline_file(void *cmdline_file);

/**
 * @brief Create and fill the command line file data (struct cmdline_file *).
 * @param[in] set Optional parameter in case the record is supposed to be added into a set.
 * @param[in] in Input file handler.
 * @param[in] path Filepath of the file.
 * @param[in] format Format of the data file.
 * @return The created command line file structure.
 * @return NULL on failure
 */
struct cmdline_file *fill_cmdline_file(struct ly_set *set, struct ly_in *in, const char *path, LYD_FORMAT format);

/**
 * @brief Helper function to prepare argc, argv pair from a command line string.
 *
 * @param[in] cmdline Complete command line string.
 * @param[out] argc_p Pointer to store argc value.
 * @param[out] argv_p Pointer to store argv vector.
 * @return 0 on success, non-zero on failure.
 */
int parse_cmdline(const char *cmdline, int *argc_p, char **argv_p[]);

/**
 * @brief Destructor for the argument vector prepared by ::parse_cmdline().
 *
 * @param[in,out] argv Argument vector to destroy.
 */
void free_cmdline(char *argv[]);

/**
 * @brief Get expected format of the @p filename's content according to the @p filename's suffix.
 * @param[in] filename Name of the file to examine.
 * @param[out] schema Pointer to a variable to store the expected input schema format. Do not provide the pointer in case a
 * schema format is not expected.
 * @param[out] data Pointer to a variable to store the expected input data format. Do not provide the pointer in case a data
 * format is not expected.
 * @return zero in case a format was successfully detected.
 * @return nonzero in case it is not possible to get valid format from the @p filename.
 */
int get_format(const char *filename, LYS_INFORMAT *schema, LYD_FORMAT *data);

/**
 * @brief Print list of schemas in the context.
 *
 * @param[in] out Output handler where to print.
 * @param[in] ctx Context to print.
 * @param[in] outformat Optional output format. If not specified (:LYD_UNKNOWN), a simple list with single module per line
 * is printed. Otherwise, the ietf-yang-library data are printed in the specified format.
 * @return zero in case the data successfully printed.
 * @return nonzero in case of error.
 */
int print_list(struct ly_out *out, struct ly_ctx *ctx, LYD_FORMAT outformat);

/**
 * @brief Process the input data files - parse, validate and print according to provided options.
 *
 * @param[in] ctx libyang context with schema.
 * @param[in] data_type The type of data in the input files.
 * @param[in] merge Flag if the data should be merged before validation.
 * @param[in] format Data format for printing.
 * @param[in] out The output handler for printing.
 * @param[in] options_parse Parser options.
 * @param[in] options_validate Validation options.
 * @param[in] options_print Printer options.
 * @param[in] operational_f Optional operational datastore file information for the case of an extended validation of
 * operation(s).
 * @param[in] inputs Set of file informations of input data files.
 * @param[in] xpath The set of XPaths to be evaluated on the processed data tree, basic information about the resulting set
 * is printed. Alternative to data printing.
 * return LY_ERR value.
 */
LY_ERR process_data(struct ly_ctx *ctx, enum lyd_type data_type, uint8_t merge, LYD_FORMAT format, struct ly_out *out,
        uint32_t options_parse, uint32_t options_validate, uint32_t options_print,
        struct cmdline_file *operational_f, struct ly_set *inputs, struct ly_set *xpaths);

#endif /* COMMON_H_ */
