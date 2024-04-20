/**
 * @file common.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's yanglint tool - common functions for both interactive and non-interactive mode.
 *
 * Copyright (c) 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L /* strdup, strndup */

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "compat.h"
#include "libyang.h"
#include "plugins_exts.h"
#include "yl_opt.h"

void
yl_log(ly_bool err, const char *format, ...)
{
    char msg[256];
    va_list ap;

    va_start(ap, format);
    vsnprintf(msg, 256, format, ap);
    va_end(ap);

    fprintf(stderr, "YANGLINT[%c]: %s\n", err ? 'E' : 'W', msg);
}

int
parse_schema_path(const char *path, char **dir, char **module)
{
    char *p;

    assert(dir);
    assert(module);

    /* split the path to dirname and basename for further work */
    *dir = strdup(path);
    /* FIXME: this is broken on Windows */
    *module = strrchr(*dir, '/');
    if (!(*module)) {
        *module = *dir;
        *dir = strdup("./");
    } else {
        *module[0] = '\0'; /* break the dir */
        *module = strdup((*module) + 1);
    }
    /* get the pure module name without suffix or revision part of the filename */
    if ((p = strchr(*module, '@'))) {
        /* revision */
        *p = '\0';
    } else if ((p = strrchr(*module, '.'))) {
        /* fileformat suffix */
        *p = '\0';
    }

    return 0;
}

int
get_input(const char *filepath, LYS_INFORMAT *format_schema, LYD_FORMAT *format_data, struct ly_in **in)
{
    struct stat st;

    /* check that the filepath exists and is a regular file */
    if (stat(filepath, &st) == -1) {
        YLMSG_E("Unable to use input filepath (%s) - %s.", filepath, strerror(errno));
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        YLMSG_E("Provided input file (%s) is not a regular file.", filepath);
        return -1;
    }

    if (get_format(filepath, format_schema, format_data)) {
        return -1;
    }

    if (in && ly_in_new_filepath(filepath, 0, in)) {
        YLMSG_E("Unable to process input file.");
        return -1;
    }

    return 0;
}

LYS_INFORMAT
get_schema_format(const char *filename)
{
    char *ptr;

    if ((ptr = strrchr(filename, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yang")) {
            return LYS_IN_YANG;
        } else if (!strcmp(ptr, "yin")) {
            return LYS_IN_YIN;
        } else {
            return LYS_IN_UNKNOWN;
        }
    } else {
        return LYS_IN_UNKNOWN;
    }
}

LYD_FORMAT
get_data_format(const char *filename)
{
    char *ptr;

    if ((ptr = strrchr(filename, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "xml")) {
            return LYD_XML;
        } else if (!strcmp(ptr, "json")) {
            return LYD_JSON;
        } else if (!strcmp(ptr, "lyb")) {
            return LYD_LYB;
        } else {
            return LYD_UNKNOWN;
        }
    } else {
        return LYD_UNKNOWN;
    }
}

int
get_format(const char *filepath, LYS_INFORMAT *schema_form, LYD_FORMAT *data_form)
{
    LYS_INFORMAT schema;
    LYD_FORMAT data;

    schema = !schema_form || !*schema_form ? LYS_IN_UNKNOWN : *schema_form;
    data = !data_form || !*data_form ? LYD_UNKNOWN : *data_form;

    if (!schema) {
        schema = get_schema_format(filepath);
    }
    if (!data) {
        data = get_data_format(filepath);
    }

    if (!schema && !data) {
        YLMSG_E("Input schema format for %s file not recognized.", filepath);
        return -1;
    } else if (!data && !schema) {
        YLMSG_E("Input data format for %s file not recognized.", filepath);
        return -1;
    }
    assert(schema || data);

    if (schema_form) {
        *schema_form = schema;
    }
    if (data_form) {
        *data_form = data;
    }

    return 0;
}

const struct lysc_node *
find_schema_path(const struct ly_ctx *ctx, const char *schema_path)
{
    const char *end, *module_name_end;
    char *module_name = NULL;
    const struct lysc_node *node = NULL, *parent_node = NULL, *parent_node_tmp = NULL;
    const struct lys_module *module;
    size_t node_name_len;
    ly_bool found_exact_match = 0;

    /* iterate over each '/' in the path */
    while (schema_path) {
        /* example: schema_path = /listen/endpoint
         * end == NULL for endpoint, end exists for listen */
        end = strchr(schema_path + 1, '/');
        if (end) {
            node_name_len = end - schema_path - 1;
        } else {
            node_name_len = strlen(schema_path + 1);
        }

        /* ex: schema_path = /ietf-interfaces:interfaces/interface/ietf-ip:ipv4 */
        module_name_end = strchr(schema_path, ':');
        if (module_name_end && (!end || (module_name_end < end))) {
            /* only get module's name, if it is in the current scope */
            free(module_name);
            /* - 1 because module_name_end points to ':' */
            module_name = strndup(schema_path + 1, module_name_end - schema_path - 1);
            if (!module_name) {
                YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
                parent_node = NULL;
                goto cleanup;
            }
            /* move the pointer to the beginning of the node's name - 1 */
            schema_path = module_name_end;

            /* recalculate the length of the node's name, because the module prefix mustn't be compared later */
            if (module_name_end < end) {
                node_name_len = end - module_name_end - 1;
            } else if (!end) {
                node_name_len = strlen(module_name_end + 1);
            }
        }

        module = ly_ctx_get_module_implemented(ctx, module_name);
        if (!module) {
            /* unknown module name */
            parent_node = NULL;
            goto cleanup;
        }

        /* iterate over the node's siblings / module's top level containers */
        while ((node = lys_getnext(node, parent_node, module->compiled, LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHCHOICE))) {
            if (end && !strncmp(node->name, schema_path + 1, node_name_len) && (node->name[node_name_len] == '\0')) {
                /* check if the whole node's name matches and it's not just a common prefix */
                parent_node = node;
                break;
            } else if (!strncmp(node->name, schema_path + 1, node_name_len)) {
                /* do the same here, however if there is no exact match, use the last node with the same prefix */
                if (strlen(node->name) == node_name_len) {
                    parent_node = node;
                    found_exact_match = 1;
                    break;
                } else {
                    parent_node_tmp = node;
                }
            }
        }

        if (!end && !found_exact_match) {
            /* no exact match */
            parent_node = parent_node_tmp;
        }
        found_exact_match = 0;

        /* next iter */
        schema_path = strchr(schema_path + 1, '/');
    }

cleanup:
    free(module_name);
    return parent_node;
}

LY_ERR
ext_data_clb(const struct lysc_ext_instance *ext, void *user_data, void **ext_data, ly_bool *ext_data_free)
{
    struct ly_ctx *ctx;
    struct lyd_node *data = NULL;

    ctx = ext->module->ctx;
    if (user_data) {
        lyd_parse_data_path(ctx, user_data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &data);
    }

    *ext_data = data;
    *ext_data_free = 1;
    return LY_SUCCESS;
}

LY_ERR
searchpath_strcat(char **searchpaths, const char *path)
{
    uint64_t len;
    char *new;

    if (!(*searchpaths)) {
        *searchpaths = strdup(path);
        return LY_SUCCESS;
    }

    len = strlen(*searchpaths) + strlen(path) + strlen(PATH_SEPARATOR);
    new = realloc(*searchpaths, sizeof(char) * len + 1);
    if (!new) {
        return LY_EMEM;
    }
    strcat(new, PATH_SEPARATOR);
    strcat(new, path);
    *searchpaths = new;

    return LY_SUCCESS;
}
