/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool
 *
 * Copyright (c) 2015-2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "compat.h"
#include "commands.h"
#include "configuration.h"
#include "completion.h"
#include "../../linenoise/linenoise.h"
#include "libyang.h"

int done;
struct ly_ctx *ctx = NULL;

/* main_ni.c */
int main_ni(int argc, char *argv[]);

int
main(int argc, char* argv[])
{
    char *cmd, *cmdline, *cmdstart;
    int i, j;

    if (argc > 1) {
        /* run in non-interactive mode */
        return main_ni(argc, argv);
    }

    /* continue in interactive mode */
    linenoiseSetCompletionCallback(complete_cmd);
    load_config();

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return 1;
    }

    while (!done) {
        /* get the command from user */
        cmdline = linenoise(PROMPT);

        /* EOF -> exit */
        if (cmdline == NULL) {
            done = 1;
            cmdline = strdup("quit");
        }

        /* empty line -> wait for another command */
        if (*cmdline == '\0') {
            free(cmdline);
            continue;
        }

        /* isolate the command word. */
        for (i = 0; cmdline[i] && (cmdline[i] == ' '); i++);
        cmdstart = cmdline + i;
        for (j = 0; cmdline[i] && (cmdline[i] != ' '); i++, j++);
        cmd = strndup(cmdstart, j);

        /* parse the command line */
        for (i = 0; commands[i].name; i++) {
            if (strcmp(cmd, commands[i].name) == 0) {
                break;
            }
        }

        /* execute the command if any valid specified */
        if (commands[i].name) {
            /* display help */
            if ((strchr(cmdstart, ' ') != NULL) && ((strncmp(strchr(cmdstart, ' ')+1, "-h", 2) == 0)
                    || (strncmp(strchr(cmdstart, ' ')+1, "--help", 6) == 0))) {
                if (commands[i].help_func != NULL) {
                    commands[i].help_func();
                } else {
                    printf("%s\n", commands[i].helpstring);
                }
            } else {
                commands[i].func((const char *)cmdstart);
            }
        } else {
            /* if unknown command specified, tell it to user */
            fprintf(stderr, "%s: no such command, type 'help' for more information.\n", cmd);
        }
        linenoiseHistoryAdd(cmdline);

        free(cmd);
        free(cmdline);
    }

    store_config();
    ly_ctx_destroy(ctx, NULL);

    return 0;
}
