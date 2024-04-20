/**
 * @file cmd.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool commands header
 *
 * Copyright (c) 2015-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "libyang.h"

struct yl_opt;

/**
 * @brief command information
 *
 * Callback functions are in the order they should be called.
 * First, the 'opt_func' should be called, which parses arguments from the command line and sets flags or pointers in
 * the struct yl_opt. This type of function is for interactive mode and is optional.
 * Then the 'dep_func' callback can check the struct yl_opt settings. Other items that depend on them can also be
 * set. There is also an possibility for controlling the number of positional arguments and its implications.
 * The most important callback is 'exec_func' where the command itself is executed. This function can even replace the
 * entire libyang context. The function parameters are mainly found in the yl_opt structure. Optionally, the function
 * can be called with a positional argument obtained from the command line. Some 'exec_func' are adapted to be called
 * from non-interactive yanglint mode.
 * The 'fun_func' complements the 'exec_func'. In some cases, the command execution must be divided into two stages.
 * For example, the 'exec_func' is used to fill some items in the yl_opt structure from the positional
 * arguments and then the 'fin_func' is used to perform the final action.
 */
typedef struct {
    /* User printable name of the function. */
    char *name;

    /* Convert command line options to the data struct yl_opt. */
    int (*opt_func)(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);
    /* Additionally set dependent items and perform error checking. */
    int (*dep_func)(struct yl_opt *yo, int posc);
    /* Execute command. */
    int (*exec_func)(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
    /* Finish execution of command. */
    int (*fin_func)(struct ly_ctx *ctx, struct yl_opt *yo);
    /* Display command help. */
    void (*help_func)(void);
    /* Freeing global variables allocated by the command. */
    void (*free_func)(void);
    /* Documentation for this function. */
    char *helpstring;
    /* Option characters used in function getopt_long. */
    char *optstring;
} COMMAND;

/**
 * @brief The list of available commands.
 */
extern COMMAND commands[];

/**
 * @brief Index for global variable ::commands.
 */
enum COMMAND_INDEX {
    CMD_HELP = 0,
    CMD_ADD,
    CMD_LOAD,
    CMD_PRINT,
    CMD_DATA,
    CMD_LIST,
    CMD_FEATURE,
    CMD_SEARCHPATH,
    CMD_EXTDATA,
    CMD_CLEAR,
    CMD_VERB,
#ifndef NDEBUG
    CMD_DEBUG,
#endif
};

/**
 * @brief For each cmd, call the COMMAND.free_func in the variable 'commands'.
 */
void cmd_free(void);

/* cmd_add.c */

/**
 * @brief Parse the arguments of an interactive command.
 *
 * @param[out] yo Context for yanglint.
 * @param[in] cmdline String containing command line arguments.
 * @param[out] posv Pointer to argv to a section of positional arguments.
 * @param[out] posc Number of positional arguments.
 * @return 0 on success.
 */
int cmd_add_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @brief Check the options and set dependent items in @p yo.
 *
 * @param[in,out] yo context for yanglint.
 * @param[in] posc number of positional arguments.
 * @return 0 on success.
 */
int cmd_add_dep(struct yl_opt *yo, int posc);

/**
 * @brief Parse and compile a new module using filepath.
 *
 * @param[in,out] ctx Context for libyang.
 * @param[in,out] yo Context for yanglint.
 * @param[in] posv Path to the file where the new module is located.
 * @return 0 on success.
 */
int cmd_add_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_add_help(void);

/* cmd_clear.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_clear_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_clear_dep(struct yl_opt *yo, int posc);

/**
 * @brief Clear libyang context.
 *
 * @param[in,out] ctx context for libyang that will be replaced with an empty one.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv not used.
 * @return 0 on success.
 */
int cmd_clear_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_clear_help(void);

/* cmd_data.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_data_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_data_dep(struct yl_opt *yo, int posc);

/**
 * @brief Store data file for later processing.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Path to the file where the data is located.
 * @return 0 on success.
 */
int cmd_data_store(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);

/**
 * @brief Parse, validate and optionally print data instances.
 *
 * @param[in] ctx Context for libyang.
 * @param[in] yo Context of yanglint. All necessary parameters should already be set.
 * @return 0 on success.
 */
int cmd_data_process(struct ly_ctx *ctx, struct yl_opt *yo);
void cmd_data_help(void);

/* cmd_list.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_list_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_list_dep(struct yl_opt *yo, int posc);

/**
 * @brief Print the list of modules in the current context.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Not used.
 * @return 0 on success.
 */
int cmd_list_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_list_help(void);

/* cmd_feature.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_feature_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_feature_dep(struct yl_opt *yo, int posc);

/**
 * @brief Print the features the modules.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Name of the module which features are to be printed.
 * @return 0 on success.
 */
int cmd_feature_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);

/**
 * @brief Printing of features ends.
 *
 * @param[in] ctx context for libyang. Not used.
 * @param[in] yo context for yanglint.
 * @return 0 on success.
 */
int cmd_feature_fin(struct ly_ctx *ctx, struct yl_opt *yo);
void cmd_feature_help(void);

/* cmd_load.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_load_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_load_dep(struct yl_opt *yo, int posc);

/**
 * @brief Parse and compile a new module using module name.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Name of the module to be loaded into the context.
 * @return 0 on success.
 */
int cmd_load_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_load_help(void);

/* cmd_print.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_print_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_print_dep(struct yl_opt *yo, int posc);

/**
 * @brief Print a schema module.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Name of the module to be printed. Can be NULL in the case of printing a node.
 * @return 0 on success.
 */
int cmd_print_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_print_help(void);

/* cmd_searchpath.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_searchpath_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @brief Set the paths of directories where to search schema modules.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Path to the directory. Can be NULL in the case of printing a current searchdirs.
 * @return 0 on success.
 */
int cmd_searchpath_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_searchpath_help(void);

/* cmd_extdata.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_extdata_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_extdata_dep(struct yl_opt *yo, int posc);

/**
 * @brief Set path to the file required by the extension.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Path to the directory. Can be NULL in the case of printing a current path.
 * @return 0 on success.
 */
int cmd_extdata_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_extdata_help(void);
void cmd_extdata_free(void);

/* cmd_help.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_help_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @brief Print help.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Name of the command which help message is to be printed. Can be NULL.
 * @return 0 on success.
 */
int cmd_help_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_help_help(void);

/* cmd_verb.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_verb_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_verb_dep(struct yl_opt *yo, int posc);

/**
 * @brief Set the verbose level.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Name of the verbose level to be set.
 * @return 0 on success.
 */
int cmd_verb_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);
void cmd_verb_help(void);

/* cmd_debug.c */

/**
 * @copydoc cmd_add_opt
 */
int cmd_debug_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc);

/**
 * @copydoc cmd_add_dep
 */
int cmd_debug_dep(struct yl_opt *yo, int posc);

/**
 * @brief Store the type of debug messages for later processing.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint.
 * @param[in] posv Name of the debug type to be set.
 * @return 0 on success.
 */
int cmd_debug_store(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv);

/**
 * @brief Set debug logging.
 *
 * @param[in,out] ctx context for libyang.
 * @param[in,out] yo context for yanglint. All necessary parameters should already be set.
 * @return 0 on success.
 */
int cmd_debug_setlog(struct ly_ctx *ctx, struct yl_opt *yo);
void cmd_debug_help(void);

#endif /* COMMANDS_H_ */
