/**
 * @file cmd_list.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'list' command of the libyang's yanglint tool.
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
#include <stdio.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

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

int
cmd_list_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    yo->data_out_format = LYD_UNKNOWN;

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_LIST].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'f': /* --format */
            if (!strcasecmp(optarg, "xml")) {
                yo->data_out_format = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                yo->data_out_format = LYD_JSON;
            } else {
                YLMSG_E("Unknown output format %s.", optarg);
                cmd_list_help();
                return 1;
            }
            break;
        case 'h':
            cmd_list_help();
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
cmd_list_dep(struct yl_opt *yo, int posc)
{
    if (posc) {
        YLMSG_E("No positional arguments are allowed.");
        return 1;
    }
    if (ly_out_new_file(stdout, &yo->out)) {
        YLMSG_E("Unable to print to the standard output.");
        return 1;
    }
    yo->out_stdout = 1;

    return 0;
}

/**
 * @brief Print yang library data.
 *
 * @param[in] ctx Context for libyang.
 * @param[in] data_out_format Output format of printed data.
 * @param[in] out Output handler.
 * @return 0 on success.
 */
static int
print_yang_lib_data(struct ly_ctx *ctx, LYD_FORMAT data_out_format, struct ly_out *out)
{
    struct lyd_node *ylib;

    if (ly_ctx_get_yanglib_data(ctx, &ylib, "%u", ly_ctx_get_change_count(ctx))) {
        YLMSG_E("Getting context info (ietf-yang-library data) failed. If the YANG module is missing or not implemented, "
                "use an option to add it internally.");
        return 1;
    }

    lyd_print_all(out, ylib, data_out_format, 0);
    lyd_free_all(ylib);

    return 0;
}

int
cmd_list_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) posv;
    uint32_t idx = 0, has_modules = 0;
    const struct lys_module *mod;

    if (yo->data_out_format != LYD_UNKNOWN) {
        /* ietf-yang-library data are printed in the specified format */
        if (print_yang_lib_data(*ctx, yo->data_out_format, yo->out)) {
            return 1;
        }
        return 0;
    }

    /* iterate schemas in context and provide just the basic info */
    ly_print(yo->out, "List of the loaded models:\n");
    while ((mod = ly_ctx_get_module_iter(*ctx, &idx))) {
        has_modules++;

        /* conformance print */
        if (mod->implemented) {
            ly_print(yo->out, "    I");
        } else {
            ly_print(yo->out, "    i");
        }

        /* module print */
        ly_print(yo->out, " %s", mod->name);
        if (mod->revision) {
            ly_print(yo->out, "@%s", mod->revision);
        }

        /* submodules print */
        if (mod->parsed && mod->parsed->includes) {
            uint64_t u = 0;

            ly_print(yo->out, " (");
            LY_ARRAY_FOR(mod->parsed->includes, u) {
                ly_print(yo->out, "%s%s", !u ? "" : ",", mod->parsed->includes[u].name);
                if (mod->parsed->includes[u].rev[0]) {
                    ly_print(yo->out, "@%s", mod->parsed->includes[u].rev);
                }
            }
            ly_print(yo->out, ")");
        }

        /* finish the line */
        ly_print(yo->out, "\n");
    }

    if (!has_modules) {
        ly_print(yo->out, "\t(none)\n");
    }

    ly_print_flush(yo->out);

    return 0;
}
