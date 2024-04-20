/**
 * @file yl_opt.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Settings options for the libyang context.
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>

#include "in.h" /* ly_in_free */

#include "common.h"
#include "yl_opt.h"
#include "yl_schema_features.h"

struct cmdline_file *
fill_cmdline_file(struct ly_set *set, struct ly_in *in, const char *path, LYD_FORMAT format)
{
    struct cmdline_file *rec;

    rec = malloc(sizeof *rec);
    if (!rec) {
        YLMSG_E("Allocating memory for data file information failed.");
        return NULL;
    }
    rec->in = in;
    rec->path = path;
    rec->format = format;

    if (set && ly_set_add(set, rec, 1, NULL)) {
        free(rec);
        YLMSG_E("Storing data file information failed.");
        return NULL;
    }

    return rec;
}

void
free_cmdline_file_items(struct cmdline_file *rec)
{
    if (rec && rec->in) {
        ly_in_free(rec->in, 1);
    }
}

void
free_cmdline_file(void *cmdline_file)
{
    struct cmdline_file *rec = (struct cmdline_file *)cmdline_file;

    if (rec) {
        free_cmdline_file_items(rec);
        free(rec);
    }
}

void
yl_opt_erase(struct yl_opt *yo)
{
    ly_bool interactive;

    interactive = yo->interactive;

    /* data */
    ly_set_erase(&yo->data_inputs, free_cmdline_file);
    ly_in_free(yo->data_operational.in, 1);
    ly_set_erase(&yo->data_xpath, NULL);

    /* schema */
    ly_set_erase(&yo->schema_features, yl_schema_features_free);
    ly_set_erase(&yo->schema_modules, NULL);

    /* context */
    free(yo->searchpaths);

    /* --reply-rpc */
    ly_in_free(yo->reply_rpc.in, 1);

    ly_out_free(yo->out, NULL, yo->out_stdout ? 0 : 1);

    free_cmdline(yo->argv);

    *yo = (const struct yl_opt) {
        0
    };
    yo->interactive = interactive;
}

int
yl_opt_update_schema_out_format(const char *arg, struct yl_opt *yo)
{
    if (!strcasecmp(arg, "yang")) {
        yo->schema_out_format = LYS_OUT_YANG;
        yo->data_out_format = 0;
    } else if (!strcasecmp(arg, "yin")) {
        yo->schema_out_format = LYS_OUT_YIN;
        yo->data_out_format = 0;
    } else if (!strcasecmp(arg, "info")) {
        yo->schema_out_format = LYS_OUT_YANG_COMPILED;
        yo->data_out_format = 0;
    } else if (!strcasecmp(arg, "tree")) {
        yo->schema_out_format = LYS_OUT_TREE;
        yo->data_out_format = 0;
    } else {
        return 1;
    }

    return 0;
}

int
yl_opt_update_data_out_format(const char *arg, struct yl_opt *yo)
{
    if (!strcasecmp(arg, "xml")) {
        yo->schema_out_format = 0;
        yo->data_out_format = LYD_XML;
    } else if (!strcasecmp(arg, "json")) {
        yo->schema_out_format = 0;
        yo->data_out_format = LYD_JSON;
    } else if (!strcasecmp(arg, "lyb")) {
        yo->schema_out_format = 0;
        yo->data_out_format = LYD_LYB;
    } else {
        return 1;
    }

    return 0;
}

static int
yl_opt_update_other_out_format(const char *arg, struct yl_opt *yo)
{
    if (!strcasecmp(arg, "feature-param")) {
        yo->feature_param_format = 1;
    } else {
        return 1;
    }

    return 0;
}

int
yl_opt_update_out_format(const char *arg, struct yl_opt *yo)
{
    if (!yl_opt_update_schema_out_format(arg, yo)) {
        return 0;
    }
    if (!yl_opt_update_data_out_format(arg, yo)) {
        return 0;
    }
    if (!yl_opt_update_other_out_format(arg, yo)) {
        return 0;
    }

    YLMSG_E("Unknown output format %s.", arg);
    return 1;
}

int
yl_opt_update_data_type(const char *arg, struct yl_opt *yo)
{
    if (!strcasecmp(arg, "config")) {
        yo->data_parse_options |= LYD_PARSE_NO_STATE;
        yo->data_validate_options |= LYD_VALIDATE_NO_STATE;
    } else if (!strcasecmp(arg, "get")) {
        yo->data_parse_options |= LYD_PARSE_ONLY;
    } else if (!strcasecmp(arg, "getconfig") || !strcasecmp(arg, "get-config") || !strcasecmp(arg, "edit")) {
        yo->data_parse_options |= LYD_PARSE_ONLY | LYD_PARSE_NO_STATE;
    } else if (!strcasecmp(arg, "rpc") || !strcasecmp(arg, "action")) {
        yo->data_type = LYD_TYPE_RPC_YANG;
    } else if (!strcasecmp(arg, "nc-rpc")) {
        yo->data_type = LYD_TYPE_RPC_NETCONF;
    } else if (!strcasecmp(arg, "reply") || !strcasecmp(arg, "rpcreply")) {
        yo->data_type = LYD_TYPE_REPLY_YANG;
    } else if (!strcasecmp(arg, "nc-reply")) {
        yo->data_type = LYD_TYPE_REPLY_NETCONF;
    } else if (!strcasecmp(arg, "notif") || !strcasecmp(arg, "notification")) {
        yo->data_type = LYD_TYPE_NOTIF_YANG;
    } else if (!strcasecmp(arg, "nc-notif")) {
        yo->data_type = LYD_TYPE_NOTIF_NETCONF;
    } else if (!strcasecmp(arg, "data")) {
        /* default option */
    } else {
        return 1;
    }

    return 0;
}

int
yo_opt_update_data_default(const char *arg, struct yl_opt *yo)
{
    if (!strcasecmp(arg, "all")) {
        yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL;
    } else if (!strcasecmp(arg, "all-tagged")) {
        yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL_TAG;
    } else if (!strcasecmp(arg, "trim")) {
        yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_TRIM;
    } else if (!strcasecmp(arg, "implicit-tagged")) {
        yo->data_print_options = (yo->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_IMPL_TAG;
    } else {
        return 1;
    }

    return 0;
}

int
yo_opt_update_data_in_format(const char *arg, struct yl_opt *yo)
{
    if (!strcasecmp(arg, "xml")) {
        yo->data_in_format = LYD_XML;
    } else if (!strcasecmp(arg, "json")) {
        yo->data_in_format = LYD_JSON;
    } else if (!strcasecmp(arg, "lyb")) {
        yo->data_in_format = LYD_LYB;
    } else {
        return 1;
    }

    return 0;
}

void
yo_opt_update_make_implemented(struct yl_opt *yo)
{
    if (yo->ctx_options & LY_CTX_REF_IMPLEMENTED) {
        yo->ctx_options &= ~LY_CTX_REF_IMPLEMENTED;
        yo->ctx_options |= LY_CTX_ALL_IMPLEMENTED;
    } else {
        yo->ctx_options |= LY_CTX_REF_IMPLEMENTED;
    }
}

void
yo_opt_update_disable_searchdir(struct yl_opt *yo)
{
    if (yo->ctx_options & LY_CTX_DISABLE_SEARCHDIR_CWD) {
        yo->ctx_options &= ~LY_CTX_DISABLE_SEARCHDIR_CWD;
        yo->ctx_options |= LY_CTX_DISABLE_SEARCHDIRS;
    } else {
        yo->ctx_options |= LY_CTX_DISABLE_SEARCHDIR_CWD;
    }
}

void
free_cmdline(char *argv[])
{
    if (argv) {
        free(argv[0]);
        free(argv);
    }
}

int
parse_cmdline(const char *cmdline, int *argc_p, char **argv_p[])
{
    int count;
    char **vector;
    char *ptr;
    char qmark = 0;

    assert(cmdline);
    assert(argc_p);
    assert(argv_p);

    /* init */
    optind = 0; /* reinitialize getopt() */
    count = 1;
    vector = malloc((count + 1) * sizeof *vector);
    vector[0] = strdup(cmdline);

    /* command name */
    strtok(vector[0], " ");

    /* arguments */
    while ((ptr = strtok(NULL, " "))) {
        size_t len;
        void *r;

        len = strlen(ptr);

        if (qmark) {
            /* still in quotated text */
            /* remove NULL termination of the previous token since it is not a token,
             * but a part of the quotation string */
            ptr[-1] = ' ';

            if ((ptr[len - 1] == qmark) && (ptr[len - 2] != '\\')) {
                /* end of quotation */
                qmark = 0;
                /* shorten the argument by the terminating quotation mark */
                ptr[len - 1] = '\0';
            }
            continue;
        }

        /* another token in cmdline */
        ++count;
        r = realloc(vector, (count + 1) * sizeof *vector);
        if (!r) {
            YLMSG_E("Memory allocation failed (%s:%d, %s).", __FILE__, __LINE__, strerror(errno));
            free(vector);
            return -1;
        }
        vector = r;
        vector[count - 1] = ptr;

        if ((ptr[0] == '"') || (ptr[0] == '\'')) {
            /* remember the quotation mark to identify end of quotation */
            qmark = ptr[0];

            /* move the remembered argument after the quotation mark */
            ++vector[count - 1];

            /* check if the quotation is terminated within this token */
            if ((ptr[len - 1] == qmark) && (ptr[len - 2] != '\\')) {
                /* end of quotation */
                qmark = 0;
                /* shorten the argument by the terminating quotation mark */
                ptr[len - 1] = '\0';
            }
        }
    }
    vector[count] = NULL;

    *argc_p = count;
    *argv_p = vector;

    return 0;
}
