/**
 * @file configuration.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yanglint configuration
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "configuration.h"

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "linenoise/linenoise.h"

#include "common.h"

/* Yanglint home (appended to ~/) */
#define YL_DIR ".yanglint"

char *
get_yanglint_dir(void)
{
    int ret;
    struct passwd *pw;
    char *user_home, *yl_dir;

    if (!(pw = getpwuid(getuid()))) {
        YLMSG_E("Determining home directory failed (%s).\n", strerror(errno));
        return NULL;
    }
    user_home = pw->pw_dir;

    yl_dir = malloc(strlen(user_home) + 1 + strlen(YL_DIR) + 1);
    if (!yl_dir) {
        YLMSG_E("Memory allocation failed (%s).\n", strerror(errno));
        return NULL;
    }
    sprintf(yl_dir, "%s/%s", user_home, YL_DIR);

    ret = access(yl_dir, R_OK | X_OK);
    if (ret == -1) {
        if (errno == ENOENT) {
            /* directory does not exist */
            YLMSG_W("Configuration directory \"%s\" does not exist, creating it.\n", yl_dir);
            if (mkdir(yl_dir, 00700)) {
                YLMSG_E("Configuration directory \"%s\" cannot be created (%s).\n", yl_dir, strerror(errno));
                free(yl_dir);
                return NULL;
            }
        } else {
            YLMSG_E("Configuration directory \"%s\" exists but cannot be accessed (%s).\n", yl_dir, strerror(errno));
            free(yl_dir);
            return NULL;
        }
    }

    return yl_dir;
}

void
load_config(void)
{
    char *yl_dir, *history_file;

    if ((yl_dir = get_yanglint_dir()) == NULL) {
        return;
    }

    history_file = malloc(strlen(yl_dir) + 9);
    if (!history_file) {
        YLMSG_E("Memory allocation failed (%s).\n", strerror(errno));
        free(yl_dir);
        return;
    }

    sprintf(history_file, "%s/history", yl_dir);
    if (access(history_file, F_OK) && (errno == ENOENT)) {
        YLMSG_W("No saved history.\n");
    } else if (linenoiseHistoryLoad(history_file)) {
        YLMSG_E("Failed to load history.\n");
    }

    free(history_file);
    free(yl_dir);
}

void
store_config(void)
{
    char *yl_dir, *history_file;

    if ((yl_dir = get_yanglint_dir()) == NULL) {
        return;
    }

    history_file = malloc(strlen(yl_dir) + 9);
    if (!history_file) {
        YLMSG_E("Memory allocation failed (%s).\n", strerror(errno));
        free(yl_dir);
        return;
    }

    sprintf(history_file, "%s/history", yl_dir);
    if (linenoiseHistorySave(history_file)) {
        YLMSG_E("Failed to save history.\n");
    }

    free(history_file);
    free(yl_dir);
}
