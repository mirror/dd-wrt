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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "commands.h"
#include "../../linenoise/linenoise.h"
#include "libyang.h"

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
                fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                return;
            }
            *matches = p;
            (*matches)[*match_count-1] = strdup(commands[i].name);
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
    int i;
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
                fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                return;
            }
            *matches = p;
            (*matches)[*match_count-1] = strdup(module->name);
        }

        for (i = 0; i < module->inc_size; ++i) {
            if (!strncmp(hint, module->inc[i].submodule->name, strlen(hint))) {
                ++(*match_count);
                p = realloc(*matches, *match_count * sizeof **matches);
                if (!p) {
                    fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                    return;
                }
                *matches = p;
                (*matches)[*match_count-1] = strdup(module->inc[i].submodule->name);
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
    } else if ((!strncmp(buf, "searchpath ", 11) || !strncmp(buf, "data ", 5)
            || !strncmp(buf, "config ", 7) || !strncmp(buf, "filter ", 7)
            || !strncmp(buf, "xpath ", 6) || !strncmp(buf, "clear ", 6)) && !last_is_opt(hint)) {
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
