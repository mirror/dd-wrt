/**
 * @file cmd_list.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'list' command of the libyang's yanglint tool.
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
#include <stdio.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"

void
cmd_list_help(void)
{
    printf("Usage: list [-f (xml | json)]\n"
            "                  Print the list of modules in the current context\n\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Print the list as ietf-yang-library data in the specified\n"
            "                  data FORMAT. If format not specified, a simple list is\n"
            "                  printed with an indication of imported (i) / implemented (I)\n"
            "                  modules.\n");
}

void
cmd_list(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    LYD_FORMAT format = LYD_UNKNOWN;
    struct ly_out *out = NULL;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "f:h", options, &opt_index)) != -1) {
        switch (opt) {
        case 'f': /* --format */
            if (!strcasecmp(optarg, "xml")) {
                format = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                format = LYD_JSON;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                cmd_list_help();
                goto cleanup;
            }
            break;
        case 'h':
            cmd_list_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (!ly_out_new_file(stdout, &out)) {
        print_list(out, *ctx, format);
        ly_out_free(out, NULL,  0);
    } else {
        YLMSG_E("Unable to print to the standard output.\n");
    }

cleanup:
    free_cmdline(argv);
}
