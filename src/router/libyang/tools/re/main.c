/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief libyang's YANG Regular Expression tool
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "libyang.h"

#include "compat.h"
#include "tools/config.h"

struct yr_pattern {
    char *expr;
    ly_bool invert;
};

void
help(void)
{
    fprintf(stdout, "YANG Regular Expressions processor.\n");
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "    yangre [-hv]\n");
    fprintf(stdout, "    yangre [-V] -p <regexp1> [-i] [-p <regexp2> [-i] ...] <string>\n");
    fprintf(stdout, "    yangre [-V] -f <file>\n");
    fprintf(stdout, "Returns 0 if string matches the pattern(s) or if otherwise successful.\n");
    fprintf(stdout, "Returns 1 on error.\n");
    fprintf(stdout, "Returns 2 if string does not match the pattern(s).\n\n");
    fprintf(stdout, "Options:\n"
            "  -h, --help              Show this help message and exit.\n"
            "  -v, --version           Show version number and exit.\n"
            "  -V, --verbose           Print the processing information.\n"
            "  -i, --invert-match      Invert-match modifier for the closest preceding\n"
            "                          pattern.\n"
            "  -p, --pattern=\"REGEXP\"  Regular expression including the quoting,\n"
            "                          which is applied the same way as in a YANG module.\n"
            "  -f, --file=\"FILE\"     List of patterns and the <string> (separated by an\n"
            "                          empty line) are taken from <file>. Invert-match is\n"
            "                          indicated by the single space character at the \n"
            "                          beginning of the pattern line. YANG quotation around\n"
            "                          patterns is still expected, but that avoids issues with\n"
            "                          reading quotation by shell. Avoid newline at the end\n"
            "                          of the string line to represent empty <string>.");
    fprintf(stdout, "Examples:\n"
            "  pattern \"[0-9a-fA-F]*\";      -> yangre -p '\"[0-9a-fA-F]*\"' '1F'\n"
            "  pattern '[a-zA-Z0-9\\-_.]*';  -> yangre -p \"'[a-zA-Z0-9\\-_.]*'\" 'a-b'\n"
            "  pattern [xX][mM][lL].*;      -> yangre -p '[xX][mM][lL].*' 'xml-encoding'\n\n");
    fprintf(stdout, "Note that to pass YANG quoting through your shell, you are supposed to use\n"
            "the other quotation around. For not-quoted patterns, use single quotes.\n\n");
}

void
version(void)
{
    fprintf(stdout, "yangre %s\n", PROJECT_VERSION);
}

void
pattern_error(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) path; /* unused */

    if (level == LY_LLERR) {
        fprintf(stderr, "yangre error: %s\n", msg);
    }
}

static int
add_pattern(struct yr_pattern **patterns, int *counter, char *pattern)
{
    void *reallocated;
    int orig_counter;

    /* Store the original number of items. */
    orig_counter = *counter;

    /* Reallocate 'patterns' memory with additional space. */
    reallocated = realloc(*patterns, (orig_counter + 1) * sizeof **patterns);
    if (!reallocated) {
        goto error;
    }
    (*patterns) = reallocated;
    /* Allocated memory is now larger. */
    (*counter)++;
    /* Copy the pattern and store it to the additonal space. */
    (*patterns)[orig_counter].expr = strdup(pattern);
    if (!(*patterns)[orig_counter].expr) {
        goto error;
    }
    (*patterns)[orig_counter].invert = 0;

    return 0;

error:
    fprintf(stderr, "yangre error: memory allocation error.\n");
    return 1;
}

static int
create_empty_string(char **str)
{
    free(*str);
    *str = malloc(sizeof(char));
    if (!(*str)) {
        fprintf(stderr, "yangre error: memory allocation failed.\n");
        return 1;
    }
    (*str)[0] = '\0';

    return 0;
}

static ly_bool
file_is_empty(FILE *fp)
{
    int c;

    c = fgetc(fp);
    if (c == EOF) {
        return 1;
    } else {
        ungetc(c, fp);
        return 0;
    }
}

/**
 * @brief Open the @p filepath, parse patterns and given string-argument.
 *
 * @param[in] filepath File to parse. Contains patterns and string.
 * @param[out] infile The file descriptor of @p filepath.
 * @param[out] patterns Storage of patterns.
 * @param[out] patterns_count Number of items in @p patterns.
 * @param[out] strarg The string-argument to check.
 * @return 0 on success.
 */
static int
parse_patterns_file(const char *filepath, FILE **infile, struct yr_pattern **patterns, int *patterns_count, char **strarg)
{
    int blankline = 0;
    char *str = NULL;
    size_t len = 0;
    ssize_t l;

    *infile = fopen(filepath, "rb");
    if (!(*infile)) {
        fprintf(stderr, "yangre error: unable to open input file %s (%s).\n", optarg, strerror(errno));
        goto error;
    }
    if (file_is_empty(*infile)) {
        if (create_empty_string(strarg)) {
            goto error;
        }
        return 0;
    }

    while ((l = getline(&str, &len, *infile)) != -1) {
        if (!blankline && ((str[0] == '\n') || ((str[0] == '\r') && (str[1] == '\n')))) {
            /* blank line */
            blankline = 1;
            continue;
        }
        if ((str[0] != '\n') && (str[0] != '\r') && (str[l - 1] == '\n')) {
            /* remove ending newline */
            if ((l > 1) && (str[l - 2] == '\r') && (str[l - 1] == '\n')) {
                str[l - 2] = '\0';
            } else {
                str[l - 1] = '\0';
            }
        }
        if (blankline) {
            /* done - str is now the string to check */
            blankline = 0;
            *strarg = str;
            break;
            /* else read the patterns */
        } else if (add_pattern(patterns, patterns_count, (str[0] == ' ') ? &str[1] : str)) {
            goto error;
        }
        if (str[0] == ' ') {
            /* set invert-match */
            (*patterns)[*patterns_count - 1].invert = 1;
        }
    }
    if (!str || (blankline && (str[0] != '\0'))) {
        /* corner case, no input after blankline meaning the pattern to check is empty */
        if (create_empty_string(&str)) {
            goto error;
        }
    }
    *strarg = str;

    return 0;

error:
    free(str);
    if (*infile) {
        fclose(*infile);
        *infile = NULL;
    }
    *strarg = NULL;

    return 1;
}

static char *
modstr_init(void)
{
    const char *module_start = "module yangre {"
            "yang-version 1.1;"
            "namespace urn:cesnet:libyang:yangre;"
            "prefix re;"
            "leaf pattern {"
            "  type string {";

    return strdup(module_start);
}

static char *
modstr_add_pattern(char **modstr, const struct yr_pattern *pattern)
{
    char *new;
    const char *module_invertmatch = " { modifier invert-match; }";
    const char *module_match = ";";

    if (asprintf(&new, "%s pattern %s%s", *modstr, pattern->expr,
            pattern->invert ? module_invertmatch : module_match) == -1) {
        fprintf(stderr, "yangre error: memory allocation failed.\n");
        return NULL;
    }
    free(*modstr);
    *modstr = NULL;

    return new;
}

static char *
modstr_add_ending(char **modstr)
{
    char *new;
    static const char *module_end = "}}}";

    if (asprintf(&new, "%s%s", *modstr, module_end) == -1) {
        fprintf(stderr, "yangre error: memory allocation failed.\n");
        return NULL;
    }
    free(*modstr);
    *modstr = NULL;

    return new;
}

static int
create_module(struct yr_pattern *patterns, int patterns_count, char **mod)
{
    int i;
    char *new = NULL, *modstr;

    if (!(modstr = modstr_init())) {
        goto error;
    }

    for (i = 0; i < patterns_count; i++) {
        if (!(new = modstr_add_pattern(&modstr, &patterns[i]))) {
            goto error;
        }
        modstr = new;
    }

    if (!(new = modstr_add_ending(&modstr))) {
        goto error;
    }

    *mod = new;

    return 0;

error:
    *mod = NULL;
    free(new);
    free(modstr);

    return 1;
}

static void
print_verbose(struct ly_ctx *ctx, struct yr_pattern *patterns, int patterns_count, char *str, LY_ERR match)
{
    int i;

    for (i = 0; i < patterns_count; i++) {
        fprintf(stdout, "pattern  %d: %s\n", i + 1, patterns[i].expr);
        fprintf(stdout, "matching %d: %s\n", i + 1, patterns[i].invert ? "inverted" : "regular");
    }
    fprintf(stdout, "string    : %s\n", str);
    if (match == LY_SUCCESS) {
        fprintf(stdout, "result    : matching\n");
    } else if (match == LY_EVALID) {
        fprintf(stdout, "result    : not matching\n");
    } else {
        fprintf(stdout, "result    : error (%s)\n", ly_errmsg(ctx));
    }
}

int
main(int argc, char *argv[])
{
    LY_ERR match;
    int i, opt_index = 0, ret = 1, verbose = 0;
    struct option options[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"file",             required_argument, NULL, 'f'},
        {"invert-match",     no_argument,       NULL, 'i'},
        {"pattern",          required_argument, NULL, 'p'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
        {NULL,               0,                 NULL, 0}
    };
    struct yr_pattern *patterns = NULL;
    char *str = NULL, *modstr = NULL;
    int patterns_count = 0;
    struct ly_ctx *ctx = NULL;
    struct lys_module *mod;
    FILE *infile = NULL;
    ly_bool info_printed = 0;

    opterr = 0;
    while ((i = getopt_long(argc, argv, "hf:ivVp:", options, &opt_index)) != -1) {
        switch (i) {
        case 'h':
            help();
            info_printed = 1;
            break;
        case 'f':
            if (infile) {
                help();
                fprintf(stderr, "yangre error: multiple input files are not supported.\n");
                goto cleanup;
            } else if (patterns_count) {
                help();
                fprintf(stderr, "yangre error: command line patterns cannot be mixed with file input.\n");
                goto cleanup;
            }
            if (parse_patterns_file(optarg, &infile, &patterns, &patterns_count, &str)) {
                goto cleanup;
            }
            break;
        case 'i':
            if (!patterns_count || patterns[patterns_count - 1].invert) {
                help();
                fprintf(stderr, "yangre error: invert-match option must follow some pattern.\n");
                goto cleanup;
            }
            patterns[patterns_count - 1].invert = 1;
            break;
        case 'p':
            if (infile) {
                help();
                fprintf(stderr, "yangre error: command line patterns cannot be mixed with file input.\n");
                goto cleanup;
            }
            if (add_pattern(&patterns, &patterns_count, optarg)) {
                goto cleanup;
            }
            break;
        case 'v':
            version();
            info_printed = 1;
            break;
        case 'V':
            verbose = 1;
            break;
        default:
            help();
            if (optopt) {
                fprintf(stderr, "yangre error: invalid option: -%c\n", optopt);
            } else {
                fprintf(stderr, "yangre error: invalid option: %s\n", argv[optind - 1]);
            }
            goto cleanup;
        }
    }

    if (info_printed) {
        ret = 0;
        goto cleanup;
    }

    if (!str) {
        /* check options compatibility */
        if (optind >= argc) {
            help();
            fprintf(stderr, "yangre error: missing <string> parameter to process.\n");
            goto cleanup;
        } else if (!patterns_count) {
            help();
            fprintf(stderr, "yangre error: missing pattern parameter to use.\n");
            goto cleanup;
        }
        str = argv[optind];
    }

    if (create_module(patterns, patterns_count, &modstr)) {
        goto cleanup;
    }

    if (ly_ctx_new(NULL, 0, &ctx)) {
        goto cleanup;
    }

    ly_set_log_clb(pattern_error, 0);
    if (lys_parse_mem(ctx, modstr, LYS_IN_YANG, &mod) || !mod->compiled || !mod->compiled->data) {
        goto cleanup;
    }

    /* check the value */
    match = lyd_value_validate(ctx, mod->compiled->data, str, strlen(str), NULL, NULL, NULL);

    if (verbose) {
        print_verbose(ctx, patterns, patterns_count, str, match);
    }
    if (match == LY_SUCCESS) {
        ret = 0;
    } else if (match == LY_EVALID) {
        ret = 2;
    } else {
        ret = 1;
    }

cleanup:
    ly_ctx_destroy(ctx);
    for (i = 0; i < patterns_count; i++) {
        free(patterns[i].expr);
    }
    if (patterns_count) {
        free(patterns);
    }
    free(modstr);
    if (infile) {
        fclose(infile);
        free(str);
    }

    return ret;
}
