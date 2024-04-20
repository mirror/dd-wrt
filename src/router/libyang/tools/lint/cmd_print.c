/**
 * @file cmd_print.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'print' command of the libyang's yanglint tool.
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

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"

void
cmd_print_help(void)
{
    printf("Usage: print [-f (yang | yin | tree [-q -P PATH -L LINE_LENGTH ] | info [-q -P PATH])]\n"
            "            [-o OUTFILE] [<module-name1>[@revision]] ...\n"
            "                  Print a schema module. The <module-name> is not required\n"
            "                  only in case the -P option is specified.\n\n"
            "  -f FORMAT, --format=FORMAT\n"
            "                  Print the module in the specified FORMAT. If format not\n"
            "                  specified, the 'tree' format is used.\n"
            "  -L LINE_LENGTH, --tree-line-length=LINE_LENGTH\n"
            "                 The limit of the maximum line length on which the 'tree'\n"
            "                 format will try to be printed.\n"
            "  -P PATH, --schema-node=PATH\n"
            "                 Print only the specified subtree of the schema.\n"
            "                 The PATH is the XPath subset mentioned in documentation as\n"
            "                 the Path format. The option can be combined with --single-node\n"
            "                 option to print information only about the specified node.\n"
            "  -q, --single-node\n"
            "                 Supplement to the --schema-node option to print information\n"
            "                 only about a single node specified as PATH argument.\n"
            "  -o OUTFILE, --output=OUTFILE\n"
            "                  Write the output to OUTFILE instead of stdout.\n");
}

static LY_ERR
cmd_print_submodule(struct ly_out *out, struct ly_ctx **ctx, char *name, char *revision, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    const struct lysp_submodule *submodule;

    submodule = revision ?
            ly_ctx_get_submodule(*ctx, name, revision) :
            ly_ctx_get_submodule_latest(*ctx, name);

    erc = submodule ?
            lys_print_submodule(out, submodule, format, line_length, options) :
            LY_ENOTFOUND;

    return erc;
}

static LY_ERR
cmd_print_module(struct ly_out *out, struct ly_ctx **ctx, char *name, char *revision, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    struct lys_module *module;

    module = revision ?
            ly_ctx_get_module(*ctx, name, revision) :
            ly_ctx_get_module_latest(*ctx, name);

    erc = module ?
            lys_print_module(out, module, format, line_length, options) :
            LY_ENOTFOUND;

    return erc;
}

static void
cmd_print_modules(int argc, char **argv, struct ly_out *out, struct ly_ctx **ctx, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    char *name, *revision;
    ly_bool search_submodul;
    const int stop = argc - optind;

    for (int i = 0; i < stop; i++) {
        name = argv[optind + i];
        /* get revision */
        revision = strchr(name, '@');
        if (revision) {
            revision[0] = '\0';
            ++revision;
        }

        erc = cmd_print_module(out, ctx, name, revision, format, line_length, options);

        if (erc == LY_ENOTFOUND) {
            search_submodul = 1;
            erc = cmd_print_submodule(out, ctx, name, revision, format, line_length, options);
        } else {
            search_submodul = 0;
        }

        if (erc == LY_SUCCESS) {
            /* for YANG Tree Diagrams printing it's more readable to print a blank line between modules. */
            if ((format == LYS_OUT_TREE) && (i + 1 < stop)) {
                ly_print(out, "\n");
            }
            continue;
        } else if (erc == LY_ENOTFOUND) {
            if (revision) {
                YLMSG_E("No (sub)module \"%s\" in revision %s found.\n", name, revision);
            } else {
                YLMSG_E("No (sub)module \"%s\" found.\n", name);
            }
            break;
        } else {
            if (search_submodul) {
                YLMSG_E("Unable to print submodule %s.\n", name);
            } else {
                YLMSG_E("Unable to print module %s.\n", name);
            }
            break;
        }
    }
}

void
cmd_print(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"format", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 'h'},
        {"tree-line-length", required_argument, NULL, 'L'},
        {"output", required_argument, NULL, 'o'},
        {"schema-node", required_argument, NULL, 'P'},
        {"single-node", no_argument, NULL, 'q'},
        {NULL, 0, NULL, 0}
    };
    uint16_t options_print = 0;
    const char *node_path = NULL;
    LYS_OUTFORMAT format = LYS_OUT_TREE;
    struct ly_out *out = NULL;
    ly_bool out_stdout = 0;
    size_t line_length = 0;

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "f:hL:o:P:q", options, &opt_index)) != -1) {
        switch (opt) {
        case 'o': /* --output */
            if (out) {
                if (ly_out_filepath(out, optarg) != NULL) {
                    YLMSG_E("Unable to use output file %s for printing output.\n", optarg);
                    goto cleanup;
                }
            } else {
                if (ly_out_new_filepath(optarg, &out)) {
                    YLMSG_E("Unable to use output file %s for printing output.\n", optarg);
                    goto cleanup;
                }
            }
            break;

        case 'f': /* --format */
            if (!strcasecmp(optarg, "yang")) {
                format = LYS_OUT_YANG;
            } else if (!strcasecmp(optarg, "yin")) {
                format = LYS_OUT_YIN;
            } else if (!strcasecmp(optarg, "info")) {
                format = LYS_OUT_YANG_COMPILED;
            } else if (!strcasecmp(optarg, "tree")) {
                format = LYS_OUT_TREE;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                cmd_print_help();
                goto cleanup;
            }
            break;

        case 'L': /* --tree-line-length */
            line_length = atoi(optarg);
            break;

        case 'P': /* --schema-node */
            node_path = optarg;
            break;

        case 'q': /* --single-node */
            options_print |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 'h':
            cmd_print_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    /* file name */
    if ((argc == optind) && !node_path) {
        YLMSG_E("Missing the name of the module to print.\n");
        goto cleanup;
    }

    if ((format != LYS_OUT_TREE) && line_length) {
        YLMSG_E("--tree-line-length take effect only in case of the tree output format.\n");
        goto cleanup;
    }

    if (!out) {
        if (ly_out_new_file(stdout, &out)) {
            YLMSG_E("Could not use stdout to print output.\n");
            goto cleanup;
        }
        out_stdout = 1;
    }

    if (format == LYS_OUT_TREE) {
        /* print tree from lysc_nodes */
        ly_ctx_set_options(*ctx, LY_CTX_SET_PRIV_PARSED);
    }

    if (node_path) {
        const struct lysc_node *node;
        node = lys_find_path(*ctx, NULL, node_path, 0);
        if (!node) {
            node = lys_find_path(*ctx, NULL, node_path, 1);

            if (!node) {
                YLMSG_E("The requested schema node \"%s\" does not exists.\n", node_path);
                goto cleanup;
            }
        }
        if (lys_print_node(out, node, format, 0, options_print)) {
            YLMSG_E("Unable to print schema node %s.\n", node_path);
            goto cleanup;
        }
    } else {
        cmd_print_modules(argc, argv, out, ctx, format, line_length, options_print);
        goto cleanup;
    }

cleanup:
    free_cmdline(argv);
    ly_out_free(out, NULL, out_stdout ? 0 : 1);
}
