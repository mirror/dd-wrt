/**
 * @file completion.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool auto completion
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L /* strdup */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "cmd.h"
#include "common.h"
#include "linenoise/linenoise.h"

/* from the main.c */
extern struct ly_ctx *ctx;

static void
get_cmd_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    int i;
    void *p;

    *match_count = 0;
    *matches = NULL;

    for (i = 0; commands[i].name; i++) {
        if (!strncmp(hint, commands[i].name, strlen(hint))) {
            ++(*match_count);
            p = realloc(*matches, *match_count * sizeof **matches);
            if (!p) {
                YLMSG_E("Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                return;
            }
            *matches = p;
            (*matches)[*match_count - 1] = strdup(commands[i].name);
        }
    }
}

static int
last_is_opt(const char *hint)
{
    const char *ptr;

    /* last is option */
    if (hint[0] == '-') {
        return 1;
    }

    do {
        --hint;
    } while (hint[0] == ' ');

    /* last is option argument */
    ptr = strrchr(hint, ' ');
    if (ptr) {
        ++ptr;
        if (ptr[0] == '-') {
            return 1;
        }
    }

    return 0;
}

static void
get_model_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    LY_ARRAY_COUNT_TYPE u;
    uint32_t idx = 0;
    const struct lys_module *module;
    void *p;

    *match_count = 0;
    *matches = NULL;

    while ((module = ly_ctx_get_module_iter(ctx, &idx))) {
        if (!strncmp(hint, module->name, strlen(hint))) {
            ++(*match_count);
            p = realloc(*matches, *match_count * sizeof **matches);
            if (!p) {
                YLMSG_E("Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                return;
            }
            *matches = p;
            (*matches)[*match_count - 1] = strdup(module->name);
        }

        LY_ARRAY_FOR(module->parsed->includes, u) {
            if (!strncmp(hint, module->parsed->includes[u].submodule->name, strlen(hint))) {
                ++(*match_count);
                p = realloc(*matches, *match_count * sizeof **matches);
                if (!p) {
                    YLMSG_E("Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                    return;
                }
                *matches = p;
                (*matches)[*match_count - 1] = strdup(module->parsed->includes[u].submodule->name);
            }
        }
    }
}

void
complete_cmd(const char *buf, const char *hint, linenoiseCompletions *lc)
{
    char **matches = NULL;
    unsigned int match_count = 0, i;

    if (!strncmp(buf, "add ", 4)) {
        linenoisePathCompletion(buf, hint, lc);
    } else if ((!strncmp(buf, "searchpath ", 11) || !strncmp(buf, "data ", 5) ||
            !strncmp(buf, "config ", 7) || !strncmp(buf, "filter ", 7) ||
            !strncmp(buf, "xpath ", 6) || !strncmp(buf, "clear ", 6)) && !last_is_opt(hint)) {
        linenoisePathCompletion(buf, hint, lc);
    } else if ((!strncmp(buf, "print ", 6) || !strncmp(buf, "feature ", 8)) && !last_is_opt(hint)) {
        get_model_completion(hint, &matches, &match_count);
    } else if (!strchr(buf, ' ') && hint[0]) {
        get_cmd_completion(hint, &matches, &match_count);
    }

    for (i = 0; i < match_count; ++i) {
        linenoiseAddCompletion(lc, matches[i]);
        free(matches[i]);
    }
    free(matches);
}
