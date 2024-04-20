/**
 * @file cmd_feature.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'feature' command of the libyang's yanglint tool.
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
#include "yl_schema_features.h"

void
cmd_feature_help(void)
{
    printf("Usage: feature [-f] <module> [<module>]*\n"
            "       feature -a [-f]\n"
            "                  Print features of all the modules with state of each one.\n\n"
            "  -f <module1, module2, ...>, --feature-param <module1, module2, ...>\n"
            "                  Generate features parameter for the command \"add\" \n"
            "                  in the form of -F <module-name>:<features>\n"
            "  -a, --all \n"
            "                  Print features of all implemented modules.\n");
}

int
cmd_feature_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"all", no_argument, NULL, 'a'},
        {"feature-param", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_FEATURE].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_feature_help();
            return 1;
        case 'a':
            yo->feature_print_all = 1;
            break;
        case 'f':
            yo->feature_param_format = 1;
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
cmd_feature_dep(struct yl_opt *yo, int posc)
{
    if (ly_out_new_file(stdout, &yo->out)) {
        YLMSG_E("Unable to print to the standard output.");
        return 1;
    }
    yo->out_stdout = 1;

    if (yo->interactive && !yo->feature_print_all && !posc) {
        YLMSG_E("Missing modules to print.");
        return 1;
    }

    return 0;
}

int
cmd_feature_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    const struct lys_module *mod;

    if (yo->feature_print_all) {
        print_all_features(yo->out, *ctx, yo->feature_param_format);
        return 0;
    }

    mod = ly_ctx_get_module_latest(*ctx, posv);
    if (!mod) {
        YLMSG_E("Module \"%s\" not found.", posv);
        return 1;
    }

    if (yo->feature_param_format) {
        print_feature_param(yo->out, mod);
    } else {
        print_features(yo->out, mod);
    }

    return 0;
}

int
cmd_feature_fin(struct ly_ctx *ctx, struct yl_opt *yo)
{
    (void) ctx;

    ly_print(yo->out, "\n");
    return 0;
}
