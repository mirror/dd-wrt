/**
 * @file cmd_extdata.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'extdata' command of the libyang's yanglint tool.
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
#define _POSIX_C_SOURCE 200809L /* strdup */

#include "cmd.h"

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

char *filename;

void
cmd_extdata_free(void)
{
    free(filename);
    filename = NULL;
}

void
cmd_extdata_help(void)
{
    printf("Usage: extdata [--clear] [<extdata-file-path>]\n"
            "                 File containing the specific data required by an extension. Required by\n"
            "                 the schema-mount extension, for example, when the operational data are\n"
            "                 expected in the file. File format is guessed.\n");
}

int
cmd_extdata_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"clear", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_EXTDATA].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'c':
            yo->extdata_unset = 1;
            free(filename);
            filename = NULL;
            break;
        case 'h':
            cmd_extdata_help();
            return 1;
        default:
            YLMSG_E("Unknown option.");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return 0;
}

int
cmd_extdata_dep(struct yl_opt *yo, int posc)
{
    if (!yo->extdata_unset && (posc > 1)) {
        YLMSG_E("Only one file must be entered.");
        return 1;
    }

    return 0;
}

int
cmd_extdata_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    if (yo->extdata_unset) {
        ly_ctx_set_ext_data_clb(*ctx, NULL, NULL);
    } else if (!yo->extdata_unset && !posv) {
        /* no argument - print the current file */
        printf("%s\n", filename ? filename : "No file set.");
    } else if (posv) {
        /* set callback providing run-time extension instance data */
        free(filename);
        filename = strdup(posv);
        if (!filename) {
            YLMSG_E("Memory allocation error.");
            return 1;
        }
        ly_ctx_set_ext_data_clb(*ctx, ext_data_clb, filename);
    }

    return 0;
}
