/**
 * @file main.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
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

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "libyang.h"

void
help(void)
{
    fprintf(stdout, "YANG Regular Expressions processor.\n");
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "    yangre [-hv]\n");
    fprintf(stdout, "    yangre [-V] -p <regexp1> [-i] [-p <regexp2> [-i] ...] <string>\n");
    fprintf(stdout, "    yangre [-V] -f <file>\n");
    fprintf(stdout, "Returns 0 if string matches the pattern(s), 1 if not and -1 on error.\n\n");
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
    fprintf(stdout, "yangre %d.%d.%d\n", LY_VERSION_MAJOR, LY_VERSION_MINOR, LY_VERSION_MICRO);
}

void
pattern_error(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) path; /* unused */

    if (level == LY_LLERR && strcmp(msg, "Module \"yangre\" parsing failed.")) {
        fprintf(stderr, "yangre error: %s\n", msg);
    }
}

static const char *module_start = "module yangre {"
    "yang-version 1.1;"
    "namespace urn:cesnet:libyang:yangre;"
    "prefix re;"
    "leaf pattern {"
    "  type string {";
static const char *module_invertmatch = " { modifier invert-match; }";
static const char *module_match = ";";
static const char *module_end = "}}}";

static int
add_pattern(char ***patterns, int **inverts, int *counter, char *pattern)
{
    void *reallocated1, *reallocated2;

    (*counter)++;
    reallocated1 = realloc(*patterns, *counter * sizeof **patterns);
    reallocated2 = realloc(*inverts, *counter * sizeof **inverts);
    if (!reallocated1 || !reallocated2) {
        fprintf(stderr, "yangre error: memory allocation error.\n");
        free(reallocated1);
        free(reallocated2);
        return EXIT_FAILURE;
    }
    (*patterns) = reallocated1;
    (*patterns)[*counter - 1] = strdup(pattern);
    (*inverts) = reallocated2;
    (*inverts)[*counter - 1] = 0;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int i, opt_index = 0, ret = -1, verbose = 0, blankline = 0;
    struct option options[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"file",             required_argument, NULL, 'f'},
        {"invert-match",     no_argument,       NULL, 'i'},
        {"pattern",          required_argument, NULL, 'p'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
        {NULL,               0,                 NULL, 0}
    };
    char **patterns = NULL, *str = NULL, *modstr = NULL, *s;
    int *invert_match = NULL;
    int patterns_count = 0;
    struct ly_ctx *ctx = NULL;
    const struct lys_module *mod;
    FILE *infile = NULL;
    size_t len = 0;
    ssize_t l;

    opterr = 0;
    while ((i = getopt_long(argc, argv, "hf:ivVp:", options, &opt_index)) != -1) {
        switch (i) {
        case 'h':
            help();
            ret = -2; /* continue to allow printing version and help at once */
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
            infile = fopen(optarg, "r");
            if (!infile) {
                fprintf(stderr, "yangre error: unable to open input file %s (%s).\n", optarg, strerror(errno));
                goto cleanup;
            }

            while((l = getline(&str, &len, infile)) != -1) {
                if (!blankline && str[0] == '\n') {
                    /* blank line */
                    blankline = 1;
                    continue;
                }
                if (str[0] != '\n' && str[l - 1] == '\n') {
                    /* remove ending newline */
                    str[l - 1] = '\0';
                }
                if (blankline) {
                    /* done - str is now the string to check */
                    blankline = 0;
                    break;
                    /* else read the patterns */
                } else if (add_pattern(&patterns, &invert_match, &patterns_count,
                                       str[0] == ' ' ? &str[1] : str)) {
                    goto cleanup;
                }
                if (str[0] == ' ') {
                    /* set invert-match */
                    invert_match[patterns_count - 1] = 1;
                }
            }
            if (blankline) {
                /* corner case, no input after blankline meaning the pattern to check is empty */
                if (str != NULL) {
                    free(str);
                }
                str = malloc(sizeof(char));
                str[0] = '\0';
            }
            break;
        case 'i':
            if (!patterns_count || invert_match[patterns_count - 1]) {
                help();
                fprintf(stderr, "yangre error: invert-match option must follow some pattern.\n");
                goto cleanup;
            }
            invert_match[patterns_count - 1] = 1;
            break;
        case 'p':
            if (infile) {
                help();
                fprintf(stderr, "yangre error: command line patterns cannot be mixed with file input.\n");
                goto cleanup;
            }
            if (add_pattern(&patterns, &invert_match, &patterns_count, optarg)) {
                goto cleanup;
            }
            break;
        case 'v':
            version();
            ret = -2; /* continue to allow printing version and help at once */
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

    if (ret == -2) {
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

    for (modstr = (char*)module_start, i = 0; i < patterns_count; i++) {
        if (asprintf(&s, "%s pattern %s%s", modstr, patterns[i], invert_match[i] ? module_invertmatch : module_match) == -1) {
            fprintf(stderr, "yangre error: memory allocation failed.\n");
            goto cleanup;
        }
        if (modstr != module_start) {
            free(modstr);
        }
        modstr = s;
    }
    if (asprintf(&s, "%s%s", modstr, module_end) == -1) {
        fprintf(stderr, "yangre error: memory allocation failed.\n");
        goto cleanup;
    }
    if (modstr != module_start) {
        free(modstr);
    }
    modstr = s;

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        goto cleanup;
    }

    ly_set_log_clb(pattern_error, 0);
    mod = lys_parse_mem(ctx, modstr, LYS_IN_YANG);
    if (!mod || !mod->data) {
        goto cleanup;
    }

    ret = lyd_validate_value(mod->data, str);
    if (verbose) {
        for (i = 0; i < patterns_count; i++) {
            fprintf(stdout, "pattern  %d: %s\n", i + 1, patterns[i]);
            fprintf(stdout, "matching %d: %s\n", i + 1, invert_match[i] ? "inverted" : "regular");
        }
        fprintf(stdout, "string    : %s\n", str);
        fprintf(stdout, "result    : %s\n", ret ? "not matching" : "matching");
    }

cleanup:
    ly_ctx_destroy(ctx, NULL);
    for (i = 0; i < patterns_count; i++) {
        free(patterns[i]);
    }
    free(patterns);
    free(invert_match);
    free(modstr);
    if (infile) {
        fclose(infile);
        free(str);
    }

    return ret;
}
