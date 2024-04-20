/**
 * @file cmd_searchpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'searchpath' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
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

void
cmd_searchpath_help(void)
{
    printf("Usage: searchpath [--clear] [<modules-dir-path> ...]\n"
            "                  Set paths of directories where to search for imports and\n"
            "                  includes of the schema modules. The current working directory\n"
            "                  and the path of the module being added is used implicitly.\n"
            "                  The 'load' command uses these paths to search even for the\n"
            "                  schema modules to be loaded.\n");
}

void
cmd_searchpath(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"clear", no_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    int8_t cleared = 0;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "ch", options, &opt_index)) != -1) {
        switch (opt) {
        case 'c':
            ly_ctx_unset_searchdir(*ctx, NULL);
            cleared = 1;
            break;
        case 'h':
            cmd_searchpath_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (!cleared && (argc == optind)) {
        /* no argument - print the paths */
        const char * const *dirs = ly_ctx_get_searchdirs(*ctx);

        printf("List of the searchpaths:\n");
        for (uint32_t i = 0; dirs[i]; ++i) {
            printf("    %s\n", dirs[i]);
        }
        goto cleanup;
    }

    for (int i = 0; i < argc - optind; i++) {
        if (ly_ctx_set_searchdir(*ctx, argv[optind + i])) {
            goto cleanup;
        }
    }

cleanup:
    free_cmdline(argv);
}
