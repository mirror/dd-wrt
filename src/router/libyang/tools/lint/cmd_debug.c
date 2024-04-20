/**
 * @file cmd_debug.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'verb' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2023-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "cmd.h"

#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

struct debug_groups {
    char *name;
    uint32_t flag;
} const dg [] = {
    {"dict", LY_LDGDICT},
    {"xpath", LY_LDGXPATH},
    {"dep-sets", LY_LDGDEPSETS},
};
#define DG_LENGTH (sizeof dg / sizeof *dg)

void
cmd_debug_help(void)
{
    uint32_t i;

    printf("Usage: debug (");
    for (i = 0; i < DG_LENGTH; i++) {
        if ((i + 1) == DG_LENGTH) {
            printf("%s", dg[i].name);
        } else {
            printf("%s | ", dg[i].name);
        }
    }
    printf(")+\n");
}

int
cmd_debug_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
    int opt, opt_index;
    struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_DEBUG].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'h':
            cmd_debug_help();
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
cmd_debug_dep(struct yl_opt *yo, int posc)
{
    (void) yo;

    if (yo->interactive && !posc) {
        /* no argument */
        cmd_debug_help();
        return 1;
    }

    return 0;
}

int
cmd_debug_store(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) ctx;
    uint32_t i;
    ly_bool set;

    assert(posv);

    set = 0;
    for (i = 0; i < DG_LENGTH; i++) {
        if (!strcasecmp(posv, dg[i].name)) {
            yo->dbg_groups |= dg[i].flag;
            set = 1;
            break;
        }
    }

    if (!set) {
        YLMSG_E("Unknown debug group \"%s\".", posv);
        return 1;
    }

    return 0;
}

int
cmd_debug_setlog(struct ly_ctx *ctx, struct yl_opt *yo)
{
    (void) ctx;
    return ly_log_dbg_groups(yo->dbg_groups);
}
