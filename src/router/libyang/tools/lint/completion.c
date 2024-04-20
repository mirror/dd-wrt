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
#include "compat.h"
#include "linenoise/linenoise.h"

/* from the main.c */
extern struct ly_ctx *ctx;

/**
 * @brief Add a match to the completions array.
 *
 * @param[in] match Match to be added.
 * @param[in,out] matches Matches provided to the user as a completion hint.
 * @param[in,out] match_count Number of matches.
 */
static void
cmd_completion_add_match(const char *match, char ***matches, unsigned int *match_count)
{
    void *p;

    p = realloc(*matches, (*match_count + 1) * sizeof **matches);
    if (!p) {
        YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
        return;
    }
    *matches = p;
    (*matches)[*match_count] = strdup(match);
    if (!((*matches)[*match_count])) {
        YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
        return;
    }
    ++(*match_count);
}

/**
 * @brief Provides completion for command names.
 *
 * @param[in] hint User input.
 * @param[out] matches Matches provided to the user as a completion hint.
 * @param[out] match_count Number of matches.
 */
static void
get_cmd_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    int i;

    *match_count = 0;
    *matches = NULL;

    for (i = 0; commands[i].name; i++) {
        if (!strncmp(hint, commands[i].name, strlen(hint))) {
            cmd_completion_add_match(commands[i].name, matches, match_count);
        }
    }
}

/**
 * @brief Provides completion for arguments.
 *
 * @param[in] hint User input.
 * @param[in] args Array of all possible arguments. The last element must be NULL.
 * @param[out] matches Matches provided to the user as a completion hint.
 * @param[out] match_count Number of matches.
 */
static void
get_arg_completion(const char *hint, const char **args, char ***matches, unsigned int *match_count)
{
    int i;

    *match_count = 0;
    *matches = NULL;

    for (i = 0; args[i]; i++) {
        if (!strncmp(hint, args[i], strlen(hint))) {
            cmd_completion_add_match(args[i], matches, match_count);
        }
    }
    if (*match_count == 0) {
        for (i = 0; args[i]; i++) {
            cmd_completion_add_match(args[i], matches, match_count);
        }
    }
}

/**
 * @brief Provides completion for module names.
 *
 * @param[in] hint User input.
 * @param[out] matches Matches provided to the user as a completion hint.
 * @param[out] match_count Number of matches.
 */
static void
get_model_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    LY_ARRAY_COUNT_TYPE u;
    uint32_t idx = 0;
    const struct lys_module *module;

    *match_count = 0;
    *matches = NULL;

    while ((module = ly_ctx_get_module_iter(ctx, &idx))) {
        if (!strncmp(hint, module->name, strlen(hint))) {
            cmd_completion_add_match(module->name, matches, match_count);
        }

        LY_ARRAY_FOR(module->parsed->includes, u) {
            if (!strncmp(hint, module->parsed->includes[u].submodule->name, strlen(hint))) {
                cmd_completion_add_match(module->parsed->includes[u].submodule->name, matches, match_count);
            }
        }
    }
}

/**
 * @brief Add all child nodes of a single node to the completion hint.
 *
 * @param[in] parent Node of which children will be added to the hint.
 * @param[out] matches Matches provided to the user as a completion hint.
 * @param[out] match_count Number of matches.
 */
static void
single_hint_add_children(const struct lysc_node *parent, char ***matches, unsigned int *match_count)
{
    const struct lysc_node *node = NULL;
    char *match;

    if (!parent) {
        return;
    }

    while ((node = lys_getnext(node, parent, NULL, LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHCHOICE))) {
        match = lysc_path(node, LYSC_PATH_LOG, NULL, 0);
        cmd_completion_add_match(match, matches, match_count);
        free(match);
    }
}

/**
 * @brief Add module and/or node's children names to the hint.
 *
 * @param[in] module Compiled schema module.
 * @param[in] parent Parent node of which children are potential matches.
 * @param[in] hint_node_name Node name contained within the hint specified by user.
 * @param[in,out] matches Matches provided to the user as a completion hint.
 * @param[in,out] match_count Number of matches.
 * @param[out] last_node Last processed node.
 */
static void
add_all_children_nodes(const struct lysc_module *module, const struct lysc_node *parent,
        const char *hint_node_name, char ***matches, unsigned int *match_count, const struct lysc_node **last_node)
{
    const struct lysc_node *node;
    char *match, *node_name = NULL;

    *last_node = NULL;

    if (!parent && !module) {
        return;
    }

    node = NULL;
    while ((node = lys_getnext(node, parent, module, LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHCHOICE))) {
        if (parent && (node->module != parent->module)) {
            /* augmented node */
            if (asprintf(&node_name, "%s:%s", node->module->name, node->name) == -1) {
                YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
                break;
            }
        } else {
            node_name = strdup(node->name);
            if (!node_name) {
                YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
                break;
            }
        }
        if (!hint_node_name || !strncmp(hint_node_name, node_name, strlen(hint_node_name))) {
            /* adding just module names + their top level node(s) to the hint */
            *last_node = node;
            match = lysc_path(node, LYSC_PATH_LOG, NULL, 0);
            cmd_completion_add_match(match, matches, match_count);
            free(match);
        }
        free(node_name);
    }
}

/**
 * @brief Provides completion for schemas.
 *
 * @param[in] hint User input.
 * @param[out] matches Matches provided to the user as a completion hint.
 * @param[out] match_count Number of matches.
 */
static void
get_schema_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    const struct lys_module *module;
    uint32_t idx;
    const char *start;
    char *end, *module_name = NULL, *path = NULL;
    const struct lysc_node *parent, *last_node;
    int rc = 0;
    size_t len;

    *match_count = 0;
    *matches = NULL;

    if (strlen(hint)) {
        if (hint[0] != '/') {
            return;
        }
        start = hint + 1;
    } else {
        start = hint;
    }

    end = strchr(start, ':');
    if (!end) {
        /* no module name */
        len = strlen(start);

        /* go through all the modules */
        idx = 0;
        while ((module = ly_ctx_get_module_iter(ctx, &idx))) {
            if (!module->implemented) {
                continue;
            }

            if (!len || !strncmp(start, module->name, len)) {
                /* add all their (matching) top level nodes */
                add_all_children_nodes(module->compiled, NULL, NULL, matches, match_count, &last_node);
            }
        }
    } else {
        /* module name known */
        module_name = strndup(start, end - start);
        if (!module_name) {
            YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
            rc = 1;
            goto cleanup;
        }

        module = ly_ctx_get_module_implemented(ctx, module_name);
        if (!module) {
            goto cleanup;
        }

        /* find the last '/', if it is at the beginning of the hint, only path up to the top level node is known,
         * else the name of the last node starts after the found '/' */
        start = strrchr(hint, '/');
        if (!start) {
            goto cleanup;
        }

        if (start == hint) {
            /* only the (incomplete) top level node path, add all (matching) top level nodes */
            add_all_children_nodes(module->compiled, NULL, end + 1, matches, match_count, &last_node);
            goto cleanup;
        }

        /* get rid of stuff after the last '/' to obtain the parent node */
        path = strndup(hint, start - hint);
        if (!path) {
            YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
            rc = 1;
            goto cleanup;
        }

        /* get the last parent in the hint (it may not exist) */
        parent = find_schema_path(ctx, path);

        /* add all (matching) child nodes of the parent */
        add_all_children_nodes(NULL, parent, start + 1, matches, match_count, &last_node);
    }

cleanup:
    if (!rc && (*match_count == 1)) {
        /* to avoid a single hint (space at the end), add all children as hints */
        single_hint_add_children(last_node, matches, match_count);
    }
    free(path);
    free(module_name);
}

/**
 * @brief Get all possible argument hints for option.
 *
 * @param[in] hint User input.
 * @param[out] matches Matches provided to the user as a completion hint.
 * @param[out] match_count Number of matches.
 */
static void
get_print_format_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"yang", "yin", "tree", "info", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @copydoc get_print_format_arg
 */
static void
get_data_type_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"data", "config", "get", "getconfig", "edit", "rpc", "reply", "notif", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @copydoc get_print_format_arg
 */
static void
get_data_in_format_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"xml", "json", "lyb", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @copydoc get_print_format_arg
 */
static void
get_data_default_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"all", "all-tagged", "trim", "implicit-tagged", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @copydoc get_print_format_arg
 */
static void
get_list_format_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"xml", "json", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @copydoc get_print_format_arg
 */
static void
get_verb_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"error", "warning", "verbose", "debug", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @copydoc get_print_format_arg
 */
static void
get_debug_arg(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *args[] = {"dict", "xpath", "dep-sets", NULL};

    get_arg_completion(hint, args, matches, match_count);
}

/**
 * @brief Get the string before the hint, which autocompletion is for.
 *
 * @param[in] buf Complete user input.
 * @param[in] hint Hint part of the user input.
 * @return Pointer to the last string.
 */
static const char *
get_last_str(const char *buf, const char *hint)
{
    const char *ptr;

    if (buf == hint) {
        return buf;
    }

    ptr = hint - 1;
    while (ptr[0] == ' ') {
        --ptr;
        if (buf == ptr) {
            return buf;
        }
    }

    while (ptr[-1] != ' ') {
        --ptr;
        if (buf == ptr) {
            return buf;
        }
    }

    return ptr;
}

/* callback */
void
complete_cmd(const char *buf, const char *hint, linenoiseCompletions *lc)
{
    struct autocomplete {
        enum COMMAND_INDEX ci;      /**< command index to global variable 'commands' */
        const char *opt;            /**< optional option */
        void (*ln_cb)(const char *, const char *, linenoiseCompletions *);  /**< linenoise callback to call */
        void (*yl_cb)(const char *, char ***, unsigned int *);              /**< yanglint callback to call */
    } ac[] = {
        {CMD_ADD,         NULL,    linenoisePathCompletion, NULL},
        {CMD_PRINT,       "-f",    NULL, get_print_format_arg},
        {CMD_PRINT,       "-P",    NULL, get_schema_completion},
        {CMD_PRINT,       "-o",    linenoisePathCompletion, NULL},
        {CMD_PRINT,       NULL,    NULL, get_model_completion},
        {CMD_SEARCHPATH,  NULL,    linenoisePathCompletion, NULL},
        {CMD_EXTDATA,     NULL,    linenoisePathCompletion, NULL},
        {CMD_CLEAR,       "-Y",    linenoisePathCompletion, NULL},
        {CMD_DATA,        "-t",    NULL, get_data_type_arg},
        {CMD_DATA,        "-O",    linenoisePathCompletion, NULL},
        {CMD_DATA,        "-R",    linenoisePathCompletion, NULL},
        {CMD_DATA,        "-f",    NULL, get_data_in_format_arg},
        {CMD_DATA,        "-F",    NULL, get_data_in_format_arg},
        {CMD_DATA,        "-d",    NULL, get_data_default_arg},
        {CMD_DATA,        "-o",    linenoisePathCompletion, NULL},
        {CMD_DATA,        NULL,    linenoisePathCompletion, NULL},
        {CMD_LIST,        NULL,    NULL, get_list_format_arg},
        {CMD_FEATURE,     NULL,    NULL, get_model_completion},
        {CMD_VERB,        NULL,    NULL, get_verb_arg},
        {CMD_DEBUG,       NULL,    NULL, get_debug_arg},
    };
    size_t name_len;
    const char *last, *name, *getoptstr;
    char opt[3] = {'\0', ':', '\0'};
    char **matches = NULL;
    unsigned int match_count = 0, i;

    if (buf == hint) {
        /* command autocomplete */
        get_cmd_completion(hint, &matches, &match_count);

    } else {
        for (i = 0; i < (sizeof ac / sizeof *ac); ++i) {
            /* Find the right command. */
            name = commands[ac[i].ci].name;
            name_len = strlen(name);
            if (strncmp(buf, name, name_len) || (buf[name_len] != ' ')) {
                /* not this command */
                continue;
            }

            /* Select based on the right option. */
            last = get_last_str(buf, hint);
            opt[0] = (last[0] == '-') && last[1] ? last[1] : '\0';
            getoptstr = commands[ac[i].ci].optstring;
            if (!ac[i].opt && opt[0] && strstr(getoptstr, opt)) {
                /* completion for the argument must be defined */
                continue;
            } else if (ac[i].opt && opt[0] && strncmp(ac[i].opt, last, strlen(ac[i].opt))) {
                /* completion for (another) option */
                continue;
            } else if (ac[i].opt && !opt[0]) {
                /* completion is defined for option */
                continue;
            }

            /* callback */
            if (ac[i].ln_cb) {
                ac[i].ln_cb(buf, hint, lc);
            } else {
                ac[i].yl_cb(hint, &matches, &match_count);
            }
            break;
        }
    }

    /* transform matches into autocompletion, if needed */
    for (i = 0; i < match_count; ++i) {
        linenoiseAddCompletion(lc, matches[i]);
        free(matches[i]);
    }
    free(matches);
}
