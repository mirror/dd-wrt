/**
 * @file yl_opt.h
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Settings options for the libyang context.
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef YL_OPT_H_
#define YL_OPT_H_

#include "parser_data.h" /* enum lyd_type */
#include "printer_schema.h" /* LYS_OUTFORMAT */
#include "set.h" /* ly_set */

/**
 * @brief Data connected with a file provided on a command line as a file path.
 */
struct cmdline_file {
    struct ly_in *in;
    const char *path;
    LYD_FORMAT format;
};

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
 * @brief Free the command line file data items.
 * @param[in,out] rec record to free.
 */
void free_cmdline_file_items(struct cmdline_file *rec);

/**
 * @brief Free the command line file data (struct cmdline_file *).
 * @param[in,out] cmdline_file The (struct cmdline_file *) to free.
 */
void free_cmdline_file(void *cmdline_file);

/**
 * @brief Context structure to hold and pass variables in a structured form.
 */
struct yl_opt {
    /* Set to 1 if yanglint running in the interactive mode */
    ly_bool interactive;
    ly_bool last_one;

    /* libyang context for the run */
    char *yang_lib_file;
    uint16_t ctx_options;

    /* prepared output (--output option or stdout by default) */
    ly_bool out_stdout;
    struct ly_out *out;

    char *searchpaths;
    ly_bool searchdir_unset;

    /* options flags */
    uint8_t list;        /* -l option to print list of schemas */

    /* line length for 'tree' format */
    size_t line_length; /* --tree-line-length */

    uint32_t dbg_groups;

    /*
     * schema
     */
    /* set schema modules' features via --features option (struct schema_features *) */
    struct ly_set schema_features;

    /* set of loaded schema modules (struct lys_module *) */
    struct ly_set schema_modules;

    /* options to parse and print schema modules */
    uint32_t schema_parse_options;
    uint32_t schema_print_options;

    /* specification of printing schema node subtree, option --schema-node */
    char *schema_node_path;
    char *submodule;

    /* name of file containing explicit context passed to callback
     * for schema-mount extension.  This also causes a callback to
     * be registered.
     */
    char *schema_context_filename;
    ly_bool extdata_unset;

    /* value of --format in case of schema format */
    LYS_OUTFORMAT schema_out_format;
    ly_bool feature_param_format;
    ly_bool feature_print_all;

    /*
     * data
     */
    /* various options based on --type option */
    enum lyd_type data_type;
    uint32_t data_parse_options;
    uint32_t data_validate_options;
    uint32_t data_print_options;

    /* flag for --merge option */
    uint8_t data_merge;

    /* value of --format in case of data format */
    LYD_FORMAT data_out_format;

    /* value of --in-format in case of data format */
    LYD_FORMAT data_in_format;

    /* input data files (struct cmdline_file *) */
    struct ly_set data_inputs;

    /* storage for --operational */
    struct cmdline_file data_operational;

    /* storage for --reply-rpc */
    struct cmdline_file reply_rpc;

    /* storage for --data-xpath */
    struct ly_set data_xpath;

    char **argv;
};

/**
 * @brief Erase all values in @p opt.
 *
 * The yl_opt.interactive item is not deleted.
 *
 * @param[in,out] yo Option context to erase.
 */
void yl_opt_erase(struct yl_opt *yo);

/**
 * @brief Update @p yo according to the @p arg of the schema --format parameter.
 *
 * @param[in] arg Format parameter argument (for example yang, yin, ...).
 * @param[out] yo yanglint options used to update.
 * @return 0 on success.
 */
int yl_opt_update_schema_out_format(const char *arg, struct yl_opt *yo);

/**
 * @brief Update @p yo according to the @p arg of the data --format parameter.
 *
 * @param[in] arg Format parameter argument (for example xml, json, ...).
 * @param[out] yo yanglint options used to update.
 * @return 0 on success.
 */
int yl_opt_update_data_out_format(const char *arg, struct yl_opt *yo);

/**
 * @brief Update @p yo according to the @p arg of the general --format parameter.
 *
 * @param[in] arg Format parameter argument (for example yang, xml, ...).
 * @param[out] yo yanglint options used to update.
 * @return 0 on success.
 */
int yl_opt_update_out_format(const char *arg, struct yl_opt *yo);

/**
 * @brief Update @p yo according to the @p arg of the data --type parameter.
 *
 * @param[in] arg Format parameter argument (for example config, rpc, ...).
 * @param[out] yo yanglint options used to update.
 * @return 0 on success.
 */
int yl_opt_update_data_type(const char *arg, struct yl_opt *yo);

/**
 * @brief Update @p yo according to the @p arg of the data --default parameter.
 *
 * @param[in] arg Format parameter argument (for example all, trim, ...).
 * @param[out] yo yanglint options used to update.
 * @return 0 on success.
 */
int yo_opt_update_data_default(const char *arg, struct yl_opt *yo);

/**
 * @brief Update @p yo according to the @p arg of the data --in-format parameter.
 *
 * @param[in] arg Format parameter argument (for example xml, json, ...).
 * @param[out] yo yanglint options used to update.
 * @return 0 on success.
 */
int yo_opt_update_data_in_format(const char *arg, struct yl_opt *yo);

/**
 * @brief Update @p yo according to the --make-implemented parameter.
 *
 * @param[in,out] yo yanglint options used to update.
 */
void yo_opt_update_make_implemented(struct yl_opt *yo);

/**
 * @brief Update @p yo according to the --disable-searchdir parameter.
 *
 * @param[in,out] yo yanglint options used to update.
 */
void yo_opt_update_disable_searchdir(struct yl_opt *yo);

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

#endif /* YL_OPT_H_ */
