/**
 * @file cmd_help.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'help' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2023-2023 CESNET, z.s.p.o.
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
cmd_help_help(void)
{
    printf("Usage: help [cmd ...]\n");
}

int
cmd_help_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
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

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_HELP].optstring, options, &opt_index)) != -1) {
        if (opt == 'h') {
            cmd_help_help();
            return 1;
        } else {
            YLMSG_E("Unknown option.");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return rc;
}

static void
print_generic_help(void)
{
    printf("Available commands:\n");
    for (uint16_t i = 0; commands[i].name; i++) {
        if (commands[i].helpstring != NULL) {
            printf("  %-15s %s\n", commands[i].name, commands[i].helpstring);
        }
    }
}

int
cmd_help_exec(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void)ctx, (void)yo;

    if (!posv) {
        print_generic_help();
    } else {
        /* print specific help for the selected command(s) */

        int8_t match = 0;

        /* get the command of the specified name */
        for (uint16_t i = 0; commands[i].name; i++) {
            if (strcmp(posv, commands[i].name) == 0) {
                match = 1;
                if (commands[i].help_func != NULL) {
                    commands[i].help_func();
                } else {
                    printf("%s: %s\n", posv, commands[i].helpstring);
                }
                break;
            }
        }
        if (!match) {
            /* if unknown command specified, print the list of commands */
            printf("Unknown command \'%s\'\n", posv);
            print_generic_help();
        }
    }

    return 0;
}
