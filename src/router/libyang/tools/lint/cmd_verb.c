/**
 * @file cmd_verb.c
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

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

void
cmd_verb_help(void)
{
    printf("Usage: verb (error | warning | verbose | debug)\n");
}

int
cmd_verb_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
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

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_VERB].optstring, options, &opt_index)) != -1) {
        if (opt == 'h') {
            cmd_verb_help();
            return 1;
        } else {
            YLMSG_E("Unknown option.");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return 0;
}

int
cmd_verb_dep(struct yl_opt *yo, int posc)
{
    (void) yo;

    if (posc > 1) {
        YLMSG_E("Only a single verbosity level can be set.");
        cmd_verb_help();
        return 1;
    }

    return 0;
}

int
cmd_verb_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) ctx, (void) yo;

    if (!posv) {
        /* no argument - print current value */
        LY_LOG_LEVEL level = ly_log_level(LY_LLERR);

        ly_log_level(level);
        printf("Current verbosity level: ");
        if (level == LY_LLERR) {
            printf("error\n");
        } else if (level == LY_LLWRN) {
            printf("warning\n");
        } else if (level == LY_LLVRB) {
            printf("verbose\n");
        } else if (level == LY_LLDBG) {
            printf("debug\n");
        }
        return 0;
    } else {
        if (!strcasecmp("error", posv) || !strcmp("0", posv)) {
            ly_log_level(LY_LLERR);
        } else if (!strcasecmp("warning", posv) || !strcmp("1", posv)) {
            ly_log_level(LY_LLWRN);
        } else if (!strcasecmp("verbose", posv) || !strcmp("2", posv)) {
            ly_log_level(LY_LLVRB);
        } else if (!strcasecmp("debug", posv) || !strcmp("3", posv)) {
            ly_log_level(LY_LLDBG);
        } else {
            YLMSG_E("Unknown verbosity \"%s\".", posv);
            return 1;
        }
    }

    return 0;
}
