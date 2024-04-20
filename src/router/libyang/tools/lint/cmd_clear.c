/**
 * @file cmd_clear.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'clear' command of the libyang's yanglint tool.
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

void
cmd_clear_help(void)
{
    printf("Usage: clear [-i] [-y]\n"
            "                  Replace the current context with an empty one, searchpaths\n"
            "                  are not kept.\n\n"
            "  -i, --make-implemented\n"
            "                When loading a module into the context, the imported 'referenced'\n"
            "                modules will also be implemented. If specified a second time,\n"
            "                all the modules will be implemented.\n"
            "  -y, --yang-library\n"
            "                  Load and implement internal \"ietf-yang-library\" YANG module.\n"
            "                  Note that this module includes definitions of mandatory state\n"
            "                  data that can result in unexpected data validation errors.\n"
            "  -Y FILE, --yang-library-file=FILE\n"
            "                Parse FILE with \"ietf-yang-library\" data and use them to\n"
            "                create an exact YANG schema context. Searchpaths defined so far\n"
            "                are used, but then deleted.\n");
}

int
cmd_clear_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"make-implemented",    no_argument, NULL, 'i'},
        {"yang-library",        no_argument, NULL, 'y'},
        {"yang-library-file",   no_argument, NULL, 'Y'},
        {"help",             no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    yo->ctx_options = LY_CTX_NO_YANGLIBRARY;

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_CLEAR].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'i':
            if (yo->ctx_options & LY_CTX_REF_IMPLEMENTED) {
                yo->ctx_options &= ~LY_CTX_REF_IMPLEMENTED;
                yo->ctx_options |= LY_CTX_ALL_IMPLEMENTED;
            } else {
                yo->ctx_options |= LY_CTX_REF_IMPLEMENTED;
            }
            break;
        case 'y':
            yo->ctx_options &= ~LY_CTX_NO_YANGLIBRARY;
            break;
        case 'Y':
            yo->ctx_options &= ~LY_CTX_NO_YANGLIBRARY;
            yo->yang_lib_file = optarg;
            if (!yo->yang_lib_file) {
                YLMSG_E("Memory allocation error.");
                return 1;
            }
            break;
        case 'h':
            cmd_clear_help();
            return 1;
        default:
            YLMSG_E("Unknown option.");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return rc;
}

int
cmd_clear_dep(struct yl_opt *yo, int posc)
{
    (void) yo;

    if (posc) {
        YLMSG_E("No positional arguments are allowed.");
        return 1;
    }

    return 0;
}

/**
 * @brief Convert searchpaths into single string.
 *
 * @param[in] ctx Context with searchpaths.
 * @param[out] searchpaths Collection of paths in the single string. Paths are delimited by colon ":"
 * (on Windows, used semicolon ";" instead).
 * @return LY_ERR value.
 */
static LY_ERR
searchpaths_to_str(const struct ly_ctx *ctx, char **searchpaths)
{
    uint32_t i;
    int rc = 0;
    const char * const *dirs = ly_ctx_get_searchdirs(ctx);

    for (i = 0; dirs[i]; ++i) {
        rc = searchpath_strcat(searchpaths, dirs[i]);
        if (!rc) {
            break;
        }
    }

    return rc;
}

int
cmd_clear_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) posv;
    struct ly_ctx *ctx_new = NULL;

    if (yo->yang_lib_file) {
        if (searchpaths_to_str(*ctx, &yo->searchpaths)) {
            YLMSG_E("Storing searchpaths failed.");
            return 1;
        }
        if (ly_ctx_new_ylpath(yo->searchpaths, yo->yang_lib_file, LYD_UNKNOWN, yo->ctx_options, &ctx_new)) {
            YLMSG_E("Unable to create libyang context with yang-library data.");
            return 1;
        }
    } else {
        if (ly_ctx_new(NULL, yo->ctx_options, &ctx_new)) {
            YLMSG_W("Failed to create context.");
            return 1;
        }
    }

    /* Global variables in commands are also deleted. */
    cmd_free();

    ly_ctx_destroy(*ctx);
    *ctx = ctx_new;

    return 0;
}
