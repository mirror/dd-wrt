/**
 * @file cmd_print.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'print' command of the libyang's yanglint tool.
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

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

void
cmd_print_help(void)
{
    printf("Usage: print [-f (yang | yin | tree [-q -P PATH -L LINE_LENGTH ] | info [-q -P PATH])]\n"
            "            [-o OUTFILE] [<module-name1>[@revision]] ...\n"
            "                  Print a schema module. The <module-name> is not required\n"
            "                  only in case the -P option is specified. For yang, yin and tree\n"
            "                  formats, a submodule can also be printed.\n\n"
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

int
cmd_print_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
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

    yo->schema_out_format = LYS_OUT_TREE;

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_PRINT].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'o': /* --output */
            if (yo->out) {
                YLMSG_E("Only a single output can be specified.");
                return 1;
            } else {
                if (ly_out_new_filepath(optarg, &yo->out)) {
                    YLMSG_E("Unable open output file %s (%s).", optarg, strerror(errno));
                    return 1;
                }
            }
            break;

        case 'f': /* --format */
            if (yl_opt_update_schema_out_format(optarg, yo)) {
                cmd_print_help();
                return 1;
            }
            break;

        case 'L': /* --tree-line-length */
            yo->line_length = atoi(optarg);
            break;

        case 'P': /* --schema-node */
            yo->schema_node_path = optarg;
            break;

        case 'q': /* --single-node */
            yo->schema_print_options |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 'h':
            cmd_print_help();
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
cmd_print_dep(struct yl_opt *yo, int posc)
{
    /* file name */
    if (yo->interactive && !posc && !yo->schema_node_path) {
        YLMSG_E("Missing the name of the module to print.");
        return 1;
    }

    if ((yo->schema_out_format != LYS_OUT_TREE) && yo->line_length) {
        YLMSG_W("--tree-line-length take effect only in case of the tree output format.");
    }

    if (!yo->out) {
        if (ly_out_new_file(stdout, &yo->out)) {
            YLMSG_E("Could not use stdout to print output.");
        }
        yo->out_stdout = 1;
    }

    if (yo->schema_out_format == LYS_OUT_TREE) {
        /* print tree from lysc_nodes */
        yo->ctx_options |= LY_CTX_SET_PRIV_PARSED;
    }

    return 0;
}

static LY_ERR
print_submodule(struct ly_out *out, struct ly_ctx **ctx, char *name, char *revision, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    const struct lysp_submodule *submodule;

    submodule = revision ?
            ly_ctx_get_submodule(*ctx, name, revision) :
            ly_ctx_get_submodule_latest(*ctx, name);

    erc = submodule ?
            lys_print_submodule(out, submodule, format, line_length, options) :
            LY_ENOTFOUND;

    if (!erc) {
        return 0;
    } else if ((erc == LY_ENOTFOUND) && revision) {
        YLMSG_E("No submodule \"%s\" found.", name);
    } else {
        YLMSG_E("Unable to print submodule %s.", name);
    }

    return erc;
}

static LY_ERR
print_module(struct ly_out *out, struct ly_ctx **ctx, char *name, char *revision, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR erc;
    struct lys_module *module;

    module = revision ?
            ly_ctx_get_module(*ctx, name, revision) :
            ly_ctx_get_module_latest(*ctx, name);

    erc = module ?
            lys_print_module(out, module, format, line_length, options) :
            LY_ENOTFOUND;

    if (!erc) {
        return 0;
    } else if ((erc == LY_ENOTFOUND) && revision) {
        YLMSG_E("No module \"%s\" found.", name);
    } else {
        YLMSG_E("Unable to print module %s.", name);
    }

    return erc;
}

static int
cmd_print_module(const char *posv, struct ly_out *out, struct ly_ctx **ctx, LYS_OUTFORMAT format,
        size_t line_length, uint32_t options)
{
    LY_ERR erc;
    char *name = NULL, *revision;

    name = strdup(posv);
    /* get revision */
    revision = strchr(name, '@');
    if (revision) {
        revision[0] = '\0';
        ++revision;
    }

    erc = print_module(out, ctx, name, revision, format, line_length, options);

    if (erc == LY_ENOTFOUND) {
        erc = print_submodule(out, ctx, name, revision, format, line_length, options);
    }

    free(name);
    return erc;
}

/**
 * @brief Print schema node path.
 *
 * @param[in] ctx Context for libyang.
 * @param[in] yo Context for yanglint.
 * @return 0 on success.
 */
static int
print_node(struct ly_ctx *ctx, struct yl_opt *yo)
{
    const struct lysc_node *node;
    uint32_t temp_lo = 0;

    if (yo->interactive) {
        /* Use the same approach as for completion. */
        node = find_schema_path(ctx, yo->schema_node_path);
        if (!node) {
            YLMSG_E("The requested schema node \"%s\" does not exists.", yo->schema_node_path);
            return 1;
        }
    } else {
        /* turn off logging so that the message is not repeated */
        ly_temp_log_options(&temp_lo);
        /* search operation input */
        node = lys_find_path(ctx, NULL, yo->schema_node_path, 0);
        if (!node) {
            /* restore logging so an error may be displayed */
            ly_temp_log_options(NULL);
            /* search operation output */
            node = lys_find_path(ctx, NULL, yo->schema_node_path, 1);
            if (!node) {
                YLMSG_E("Invalid schema path.");
                return 1;
            }
        }
    }

    if (lys_print_node(yo->out, node, yo->schema_out_format, yo->line_length, yo->schema_print_options)) {
        YLMSG_E("Unable to print schema node %s.", yo->schema_node_path);
        return 1;
    }

    return 0;
}

int
cmd_print_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    int rc = 0;

    if (yo->ctx_options & LY_CTX_SET_PRIV_PARSED) {
        /* print tree from lysc_nodes */
        ly_ctx_set_options(*ctx, LY_CTX_SET_PRIV_PARSED);
    }

    if (yo->schema_node_path) {
        rc = print_node(*ctx, yo);
    } else if (!yo->interactive && yo->submodule) {
        rc = print_submodule(yo->out, ctx, yo->submodule, NULL, yo->schema_out_format, yo->line_length,
                yo->schema_print_options);
    } else {
        rc = cmd_print_module(posv, yo->out, ctx, yo->schema_out_format, yo->line_length, yo->schema_print_options);
        if (!yo->last_one && (yo->schema_out_format == LYS_OUT_TREE)) {
            ly_print(yo->out, "\n");
        }
    }

    return rc;
}
