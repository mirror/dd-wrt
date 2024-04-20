/**
 * @file cmd_clear.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'clear' command of the libyang's yanglint tool.
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
cmd_clear_help(void)
{
    printf("Usage: clear [--yang-library]\n"
            "                  Replace the current context with an empty one, searchpaths\n"
            "                  are not kept.\n"
            "  -y, --yang-library\n"
            "                  Load and implement internal \"ietf-yang-library\" YANG module.\n"
            "                  Note that this module includes definitions of mandatory state\n"
            "                  data that can result in unexpected data validation errors.\n");
#if 0
    "                  If <yang-library-data> path specified, load the modules\n"
    "                  according to the provided yang library data.\n"
#endif
}

void
cmd_clear(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"yang-library", no_argument, NULL, 'y'},
        {"help",         no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    uint16_t options_ctx = LY_CTX_NO_YANGLIBRARY;
    struct ly_ctx *ctx_new;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "yh", options, &opt_index)) != -1) {
        switch (opt) {
        case 'y':
            options_ctx &= ~LY_CTX_NO_YANGLIBRARY;
            break;
        case 'h':
            cmd_clear_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (ly_ctx_new(NULL, options_ctx, &ctx_new)) {
        YLMSG_W("Failed to create context.\n");
        goto cleanup;
    }

    ly_ctx_destroy(*ctx);
    *ctx = ctx_new;

cleanup:
    free_cmdline(argv);
}
