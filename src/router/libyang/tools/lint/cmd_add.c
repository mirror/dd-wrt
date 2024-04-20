/**
 * @file cmd_add.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'add' command of the libyang's yanglint tool.
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
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

#include "common.h"

void
cmd_add_help(void)
{
    printf("Usage: add [-iD] <schema1> [<schema2> ...]\n"
            "                  Add a new module from a specific file.\n\n"
            "  -D, --disable-searchdir\n"
            "                  Do not implicitly search in current working directory for\n"
            "                  the import schema modules. If specified a second time, do not\n"
            "                  even search in the module directory (all modules must be \n"
            "                  explicitly specified).\n"
            "  -F FEATURES, --features=FEATURES\n"
            "                  Features to support, default all.\n"
            "                  <modname>:[<feature>,]*\n"
            "  -i, --makeimplemented\n"
            "                  Make the imported modules \"referenced\" from any loaded\n"
            "                  <schema> module also implemented. If specified a second time,\n"
            "                  all the modules are set implemented.\n");
}

void
cmd_add(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"disable-searchdir", no_argument, NULL, 'D'},
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

    while ((opt = getopt_long(argc, argv, "D:F:hi", options, &opt_index)) != -1) {
        switch (opt) {
        case 'D': /* --disable--search */
            if (options_ctx & LY_CTX_DISABLE_SEARCHDIRS) {
                YLMSG_W("The -D option specified too many times.\n");
            }
            if (options_ctx & LY_CTX_DISABLE_SEARCHDIR_CWD) {
                options_ctx &= ~LY_CTX_DISABLE_SEARCHDIR_CWD;
                options_ctx |= LY_CTX_DISABLE_SEARCHDIRS;
            } else {
                options_ctx |= LY_CTX_DISABLE_SEARCHDIR_CWD;
            }
            break;

        case 'F': /* --features */
            if (parse_features(optarg, &fset)) {
                goto cleanup;
            }
            break;

        case 'h':
            cmd_add_help();
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
        LY_ERR ret;
        uint8_t path_unset = 1; /* flag to unset the path from the searchpaths list (if not already present) */
        char *dir, *module;
        const char **features = NULL;
        struct ly_in *in = NULL;

        if (parse_schema_path(argv[optind + i], &dir, &module)) {
            goto cleanup;
        }

        /* add temporarily also the path of the module itself */
        if (ly_ctx_set_searchdir(*ctx, dirname(dir)) == LY_EEXIST) {
            path_unset = 0;
        }

        /* get features list for this module */
        get_features(&fset, module, &features);

        /* temporary cleanup */
        free(dir);
        free(module);

        /* prepare input handler */
        ret = ly_in_new_filepath(argv[optind + i], 0, &in);
        if (ret) {
            goto cleanup;
        }

        /* parse the file */
        ret = lys_parse(*ctx, in, LYS_IN_UNKNOWN, features, NULL);
        ly_in_free(in, 1);
        ly_ctx_unset_searchdir_last(*ctx, path_unset);

        if (ret) {
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
