/**
 * @file cmd_add.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'add' command of the libyang's yanglint tool.
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

#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"
#include "yl_schema_features.h"

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
            "                  Features to support, default all in all implemented modules.\n"
            "                  Specify separately for each module.\n"
            "                  <modname>:[<feature>,]*\n"
            "  -i, --make-implemented\n"
            "                  Make the imported modules \"referenced\" from any loaded\n"
            "                  <schema> module also implemented. If specified a second time,\n"
            "                  all the modules are set implemented.\n"
            "  -X, --extended-leafref\n"
            "                  Allow usage of deref() XPath function within leafref.\n");
}

int
cmd_add_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"disable-searchdir", no_argument, NULL, 'D'},
        {"features", required_argument, NULL, 'F'},
        {"help", no_argument, NULL, 'h'},
        {"make-implemented", no_argument, NULL, 'i'},
        {"extended-leafref", no_argument, NULL, 'X'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_ADD].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'D': /* --disable--search */
            if (yo->ctx_options & LY_CTX_DISABLE_SEARCHDIRS) {
                YLMSG_W("The -D option specified too many times.");
            }
            yo_opt_update_disable_searchdir(yo);
            break;

        case 'F': /* --features */
            if (parse_features(optarg, &yo->schema_features)) {
                return 1;
            }
            break;

        case 'h':
            cmd_add_help();
            return 1;

        case 'i': /* --make-implemented */
            yo_opt_update_make_implemented(yo);
            break;

        case 'X': /* --extended-leafref */
            yo->ctx_options |= LY_CTX_LEAFREF_EXTENDED;
            break;

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
cmd_add_dep(struct yl_opt *yo, int posc)
{
    if (yo->interactive && !posc) {
        /* no argument */
        cmd_add_help();
        return 1;
    }
    if (!yo->schema_features.count) {
        /* no features, enable all of them */
        yo->ctx_options |= LY_CTX_ENABLE_IMP_FEATURES;
    }

    return 0;
}

static int
store_parsed_module(const char *filepath, struct lys_module *mod, struct yl_opt *yo)
{
    assert(!yo->interactive);

    if (yo->schema_out_format || yo->feature_param_format) {
        if (ly_set_add(&yo->schema_modules, (void *)mod, 1, NULL)) {
            YLMSG_E("Storing parsed schema module (%s) for print failed.", filepath);
            return 1;
        }
    }

    return 0;
}

int
cmd_add_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    const char *all_features[] = {"*", NULL};
    LY_ERR ret;
    uint8_t path_unset = 1; /* flag to unset the path from the searchpaths list (if not already present) */
    char *dir, *modname = NULL;
    const char **features = NULL;
    struct ly_in *in = NULL;
    struct lys_module *mod;

    assert(posv);

    if (yo->ctx_options) {
        ly_ctx_set_options(*ctx, yo->ctx_options);
    }

    if (parse_schema_path(posv, &dir, &modname)) {
        return 1;
    }

    if (!yo->interactive) {
        /* The module should already be parsed if yang_lib_file was set. */
        if (yo->yang_lib_file && (mod = ly_ctx_get_module_implemented(*ctx, modname))) {
            ret = store_parsed_module(posv, mod, yo);
            goto cleanup;
        }
        /* parse module */
    }

    /* add temporarily also the path of the module itself */
    if (ly_ctx_set_searchdir(*ctx, dir) == LY_EEXIST) {
        path_unset = 0;
    }

    /* get features list for this module */
    if (!yo->schema_features.count) {
        features = all_features;
    } else {
        get_features(&yo->schema_features, modname, &features);
    }

    /* prepare input handler */
    ret = ly_in_new_filepath(posv, 0, &in);
    if (ret) {
        goto cleanup;
    }

    /* parse the file */
    ret = lys_parse(*ctx, in, LYS_IN_UNKNOWN, features, &mod);
    ly_in_free(in, 1);
    ly_ctx_unset_searchdir_last(*ctx, path_unset);
    if (ret) {
        goto cleanup;
    }

    if (!yo->interactive) {
        if ((ret = store_parsed_module(posv, mod, yo))) {
            goto cleanup;
        }
    }

cleanup:
    free(dir);
    free(modname);

    return ret;
}
