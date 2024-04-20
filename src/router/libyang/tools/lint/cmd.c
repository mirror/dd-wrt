/**
 * @file cmd.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool general commands
 *
 * Copyright (c) 2015-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "cmd.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "common.h"
#include "compat.h"
#include "libyang.h"

COMMAND commands[];
extern int done;

void
cmd_free(void)
{
    uint16_t i;

    for (i = 0; commands[i].name; i++) {
        if (commands[i].free_func) {
            commands[i].free_func();
        }
    }
}

int
cmd_quit_exec(struct ly_ctx **UNUSED(ctx), struct yl_opt *UNUSED(yo), const char *UNUSED(posv))
{
    done = 1;
    return 0;
}

/* Also keep enum COMMAND_INDEX updated. */
COMMAND commands[] = {
    {
        "help", cmd_help_opt, NULL, cmd_help_exec, NULL, cmd_help_help, NULL,
        "Display commands description", "h"
    },
    {
        "add", cmd_add_opt, cmd_add_dep, cmd_add_exec, NULL, cmd_add_help, NULL,
        "Add a new module from a specific file", "DF:hiX"
    },
    {
        "load", cmd_load_opt, cmd_load_dep, cmd_load_exec, NULL, cmd_load_help, NULL,
        "Load a new schema from the searchdirs", "F:hiX"
    },
    {
        "print", cmd_print_opt, cmd_print_dep, cmd_print_exec, NULL, cmd_print_help, NULL,
        "Print a module", "f:hL:o:P:q"
    },
    {
        "data", cmd_data_opt, cmd_data_dep, cmd_data_store, cmd_data_process, cmd_data_help, NULL,
        "Load, validate and optionally print instance data", "d:ef:F:hmo:O:R:r:nt:x:"
    },
    {
        "list", cmd_list_opt, cmd_list_dep, cmd_list_exec, NULL, cmd_list_help, NULL,
        "List all the loaded modules", "f:h"
    },
    {
        "feature", cmd_feature_opt, cmd_feature_dep, cmd_feature_exec, cmd_feature_fin, cmd_feature_help, NULL,
        "Print all features of module(s) with their state", "haf"
    },
    {
        "searchpath", cmd_searchpath_opt, NULL, cmd_searchpath_exec, NULL, cmd_searchpath_help, NULL,
        "Print/set the search path(s) for schemas", "ch"
    },
    {
        "extdata", cmd_extdata_opt, cmd_extdata_dep, cmd_extdata_exec, NULL, cmd_extdata_help, cmd_extdata_free,
        "Set the specific data required by an extension", "ch"
    },
    {
        "clear", cmd_clear_opt, cmd_clear_dep, cmd_clear_exec, NULL, cmd_clear_help, NULL,
        "Clear the context - remove all the loaded modules", "iyhY:"
    },
    {
        "verb", cmd_verb_opt, cmd_verb_dep, cmd_verb_exec, NULL, cmd_verb_help, NULL,
        "Change verbosity", "h"
    },
#ifndef NDEBUG
    {
        "debug", cmd_debug_opt, cmd_debug_dep, cmd_debug_store, cmd_debug_setlog, cmd_debug_help, NULL,
        "Display specific debug message groups", "h"
    },
#endif
    {"quit", NULL, NULL, cmd_quit_exec, NULL, NULL, NULL, "Quit the program", "h"},
    /* synonyms for previous commands */
    {"?", NULL, NULL, cmd_help_exec, NULL, NULL, NULL, "Display commands description", "h"},
    {"exit", NULL, NULL, cmd_quit_exec, NULL, NULL, NULL, "Quit the program", "h"},
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};
