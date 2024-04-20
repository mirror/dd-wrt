/**
 * @file cmd_searchpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'searchpath' command of the libyang's yanglint tool.
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

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

void
cmd_searchpath_help(void)
{
    printf("Usage: searchpath [--clear] [<modules-dir-path> ...]\n"
            "                  Set paths of directories where to search for imports and includes\n"
            "                  of the schema modules. Subdirectories are also searched. The current\n"
            "                  working directory and the path of the module being added is used implicitly.\n"
            "                  The 'load' command uses these paths to search even for the schema modules\n"
            "                  to be loaded.\n");
}

int
cmd_searchpath_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
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

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_SEARCHPATH].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'c':
            yo->searchdir_unset = 1;
            break;
        case 'h':
            cmd_searchpath_help();
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
cmd_searchpath_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    int rc = 0;

    if (yo->searchdir_unset) {
        ly_ctx_unset_searchdir(*ctx, NULL);
    } else if (!yo->searchdir_unset && !posv) {
        /* no argument - print the paths */
        const char * const *dirs = ly_ctx_get_searchdirs(*ctx);

        printf("List of the searchpaths:\n");
        for (uint32_t i = 0; dirs[i]; ++i) {
            printf("    %s\n", dirs[i]);
        }
    } else {
        rc = ly_ctx_set_searchdir(*ctx, posv);
    }

    return rc;
}
