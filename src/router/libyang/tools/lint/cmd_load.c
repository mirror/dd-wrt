/**
 * @file cmd_load.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'load' command of the libyang's yanglint tool.
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
#include <string.h>

#include "libyang.h"

#include "common.h"

void
cmd_load_help(void)
{
    printf("Usage: load [-i] <module-name1>[@<revision>] [<module-name2>[@revision] ...]\n"
            "                  Add a new module of the specified name, yanglint will find\n"
            "                  them in searchpaths. if the <revision> of the module not\n"
            "                  specified, the latest revision available is loaded.\n\n"
            "  -F FEATURES, --features=FEATURES\n"
            "                  Features to support, default all.\n"
            "                  <modname>:[<feature>,]*\n"
            "  -i, --makeimplemented\n"
            "                  Make the imported modules \"referenced\" from any loaded\n"
            "                  <schema> module also implemented. If specified a second time,\n"
            "                  all the modules are set implemented.\n");
}

void
cmd_load(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"features", required_argument, NULL, 'F'},
        {"help", no_argument, NULL, 'h'},
        {"makeimplemented", no_argument, NULL, 'i'},
        {NULL, 0, NULL, 0}
    };
    uint16_t options_ctx = 0;
    struct ly_set fset = {0};

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "F:hi", options, &opt_index)) != -1) {
        switch (opt) {
        case 'F': /* --features */
            if (parse_features(optarg, &fset)) {
                goto cleanup;
            }
            break;

        case 'h':
            cmd_load_help();
            goto cleanup;

        case 'i': /* --makeimplemented */
            if (options_ctx & LY_CTX_REF_IMPLEMENTED) {
                options_ctx &= ~LY_CTX_REF_IMPLEMENTED;
                options_ctx |= LY_CTX_ALL_IMPLEMENTED;
            } else {
                options_ctx |= LY_CTX_REF_IMPLEMENTED;
            }
            break;

        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (argc == optind) {
        /* no argument */
        cmd_add_help();
        goto cleanup;
    }

    if (options_ctx) {
        ly_ctx_set_options(*ctx, options_ctx);
    }

    for (int i = 0; i < argc - optind; i++) {
        /* process the schema module files */
        char *revision;
        const char **features = NULL;

        /* get revision */
        revision = strchr(argv[optind + i], '@');
        if (revision) {
            revision[0] = '\0';
            ++revision;
        }

        /* get features list for this module */
        get_features(&fset, argv[optind + i], &features);

        /* load the module */
        if (!ly_ctx_load_module(*ctx, argv[optind + i], revision, features)) {
            /* libyang printed the error messages */
            goto cleanup;
        }
    }

cleanup:
    if (options_ctx) {
        ly_ctx_unset_options(*ctx, options_ctx);
    }
    ly_set_erase(&fset, free_features);
    free_cmdline(argv);
}
