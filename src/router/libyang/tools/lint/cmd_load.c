/**
 * @file cmd_load.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'load' command of the libyang's yanglint tool.
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
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"
#include "yl_schema_features.h"

void
cmd_load_help(void)
{
    printf("Usage: load [-i] <module-name1>[@<revision>] [<module-name2>[@revision] ...]\n"
            "                  Add a new module of the specified name, yanglint will find\n"
            "                  them in searchpaths. If the <revision> of the module not\n"
            "                  specified, the latest revision available is loaded.\n\n"
            "  -F FEATURES, --features=FEATURES\n"
            "                  Features to support, default all in all implemented modules.\n"
            "                  <modname>:[<feature>,]*\n"
            "  -i, --make-implemented\n"
            "                  Make the imported modules \"referenced\" from any loaded\n"
            "                  <schema> module also implemented. If specified a second time,\n"
            "                  all the modules are set implemented.\n"
            "  -X, --extended-leafref\n"
            "                  Allow usage of deref() XPath function within leafref.\n");
}

int
cmd_load_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"features", required_argument, NULL, 'F'},
        {"help", no_argument, NULL, 'h'},
        {"make-implemented", no_argument, NULL, 'i'},
        {"extended-leafref", no_argument, NULL, 'X'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_LOAD].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'F': /* --features */
            if (parse_features(optarg, &yo->schema_features)) {
                return 1;
            }
            break;

        case 'h':
            cmd_load_help();
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
cmd_load_dep(struct yl_opt *yo, int posc)
{
    if (yo->interactive && !posc) {
        /* no argument */
        cmd_load_help();
        return 1;
    }

    if (!yo->schema_features.count) {
        /* no features, enable all of them */
        yo->ctx_options |= LY_CTX_ENABLE_IMP_FEATURES;
    }

    return 0;
}

int
cmd_load_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    const char *all_features[] = {"*", NULL};
    char *revision;
    const char **features = NULL;

    assert(posv);

    if (yo->ctx_options) {
        ly_ctx_set_options(*ctx, yo->ctx_options);
        yo->ctx_options = 0;
    }

    /* get revision */
    revision = strchr(posv, '@');
    if (revision) {
        revision[0] = '\0';
        ++revision;
    }

    /* get features list for this module */
    if (!yo->schema_features.count) {
        features = all_features;
    } else {
        get_features(&yo->schema_features, posv, &features);
    }

    /* load the module */
    if (!ly_ctx_load_module(*ctx, posv, revision, features)) {
        /* libyang printed the error messages */
        return 1;
    }

    return 0;
}
