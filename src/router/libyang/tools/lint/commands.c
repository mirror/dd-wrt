/**
 * @file commands.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool commands
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#include "compat.h"
#include "commands.h"
#include "libyang.h"
#include "../../src/tree_schema.h"
#include "../../src/tree_data.h"
#include "../../src/xpath.h"

COMMAND commands[];
extern int done;
extern struct ly_ctx *ctx;

void
cmd_add_help(void)
{
    printf("add [-i] <path-to-model> [<paths-to-other-models> ...]\n");
    printf("\t-i         - make all the imported modules implemented\n");
}

void
cmd_load_help(void)
{
    printf("load [-i] <model-name> [<other-model-names> ...]\n");
    printf("\t-i         - make all the imported modules implemented\n");
}

void
cmd_clear_help(void)
{
    printf("clear [<yang-library> | -e]\n");
    printf("\t Replace the current context with an empty one, searchpaths are not kept.\n");
    printf("\t If <yang-library> path specified, load the modules according to the yang library data.\n");
    printf("\t Option '-e' causes ietf-yang-library will not be loaded.\n");
}

void
cmd_print_help(void)
{
    printf("print [-f (yang | yin | tree[-rfc] [<tree-options>] | info [-P <info-path>] | jsons)] [-o <output-file>]"
           " <model-name>[@<revision>]\n");
    printf("\n");
    printf("\ttree-options:\t--tree-print-groupings\t(print top-level groupings in a separate section)\n");
    printf("\t             \t--tree-print-uses\t(print uses nodes instead the resolved grouping nodes)\n");
    printf("\t             \t--tree-no-leafref-target\t(do not print the target nodes of leafrefs)\n");
    printf("\t             \t--tree-path <schema-path>\t(print only the specified subtree)\n");
    printf("\t             \t--tree-line-length <line-length>\t(wrap lines if longer than line-length,\n");
    printf("\t             \t\tnot a strict limit, longer lines can often appear)\n");
    printf("\n");
    printf("\tinfo-path:\t<schema-path> | typedef[<schema-path>]/<typedef-name> |\n");
    printf("\t          \t| identity/<identity-name> | feature/<feature-name> |\n");
    printf("\t          \t| grouping[<schema-path>]/<grouping-name> |\n");
    printf("\t          \t| type/<schema-path-leaf-or-leaflist>\n");
    printf("\n");
    printf("\tschema-path:\t( /<module-name>:<node-identifier> )+\n");
}

void
cmd_data_help(void)
{
    printf("data [-(-s)trict] [-t TYPE] [-d DEFAULTS] [-o <output-file>] [-f (xml | json | lyb)] [-F (xml | json | lyb)]\n");
    printf("     [-r <running-file-name>] <data-file-name> [<RPC/action-data-file-name> | <yang-data name>]\n\n");
    printf("Accepted TYPEs:\n");
    printf("\tauto       - resolve data type (one of the following) automatically (as pyang does),\n");
    printf("\t             this option is applicable only in case of XML input data.\n");
    printf("\tdata       - LYD_OPT_DATA (default value) - complete datastore including status data.\n");
    printf("\tconfig     - LYD_OPT_CONFIG - complete configuration datastore.\n");
    printf("\tget        - LYD_OPT_GET - <get> operation result.\n");
    printf("\tgetconfig  - LYD_OPT_GETCONFIG - <get-config> operation result.\n");
    printf("\tedit       - LYD_OPT_EDIT - <edit-config>'s data (content of its <config> element).\n");
    printf("\trpc        - LYD_OPT_RPC - NETCONF RPC message.\n");
    printf("\trpcreply   - LYD_OPT_RPCREPLY (last parameter mandatory in this case)\n");
    printf("\tnotif      - LYD_OPT_NOTIF - NETCONF Notification message.\n");
    printf("\tyangdata   - LYD_OPT_DATA_TEMPLATE - yang-data extension (last parameter mandatory in this case)\n\n");
    printf("Accepted DEFAULTS:\n");
    printf("\tall        - add missing default nodes\n");
    printf("\tall-tagged - add missing default nodes and mark all the default nodes with the attribute.\n");
    printf("\ttrim       - remove all nodes with a default value\n");
    printf("\timplicit-tagged    - add missing nodes and mark them with the attribute\n\n");
    printf("Option -f determines output format, option -F the input format.\n\n");
    printf("Option -r:\n");
    printf("\tOptional parameter for 'rpc', 'rpcreply' and 'notif' TYPEs, the file contains running\n");
    printf("\tconfiguration datastore data referenced from the RPC/Notification. Note that the file is\n");
    printf("\tvalidated as 'data' TYPE. Special value '!' can be used as argument to ignore the\n");
    printf("\texternal references.\n\n");
    printf("\tIf an XPath expression (when/must) needs access to configuration data, you can provide\n");
    printf("\tthem in a file, which will be parsed as 'data' TYPE.\n\n");
}

void
cmd_xpath_help(void)
{
    printf("xpath [-t TYPE] [-x <additional-tree-file-name>] -e <XPath-expression>\n"
           "      <XML-data-file-name> [<JSON-rpc/action-schema-nodeid>]\n");
    printf("Accepted TYPEs:\n");
    printf("\tauto       - resolve data type (one of the following) automatically (as pyang does),\n");
    printf("\t             this option is applicable only in case of XML input data.\n");
    printf("\tconfig     - LYD_OPT_CONFIG\n");
    printf("\tget        - LYD_OPT_GET\n");
    printf("\tgetconfig  - LYD_OPT_GETCONFIG\n");
    printf("\tedit       - LYD_OPT_EDIT\n");
    printf("\trpc        - LYD_OPT_RPC\n");
    printf("\trpcreply   - LYD_OPT_RPCREPLY (last parameter mandatory in this case)\n");
    printf("\tnotif      - LYD_OPT_NOTIF\n\n");
    printf("Option -x:\n");
    printf("\tIf RPC/action/notification/RPC reply (for TYPEs 'rpc', 'rpcreply', and 'notif') includes\n");
    printf("\tan XPath expression (when/must) that needs access to the configuration data, you can provide\n");
    printf("\tthem in a file, which will be parsed as 'config'.\n");
}

void
cmd_list_help(void)
{
    printf("list [-f (xml | json)]\n\n");
    printf("\tBasic list output (no -f): i - imported module, I - implemented module\n");
}

void
cmd_feature_help(void)
{
    printf("feature [ -(-e)nable | -(-d)isable (* | <feature-name>[,<feature-name> ...]) ] <model-name>[@<revision>]\n");
}

void
cmd_searchpath_help(void)
{
    printf("searchpath [<model-dir-path> | --clear]\n\n");
    printf("\tThey are used to search for imports and includes of a model.\n");
    printf("\tThe \"load\" command uses these directories to find models directly.\n");
}

void
cmd_verb_help(void)
{
    printf("verb (error/0 | warning/1 | verbose/2 | debug/3)\n");
}

#ifndef NDEBUG

void
cmd_debug_help(void)
{
    printf("debug (dict | yang | yin | xpath | diff)+\n");
}

#endif

LYS_INFORMAT
get_schema_format(const char *path)
{
    char *ptr;

    if ((ptr = strrchr(path, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yin")) {
            return LYS_IN_YIN;
        } else if (!strcmp(ptr, "yang")) {
            return LYS_IN_YANG;
        } else {
            fprintf(stderr, "Input file in an unknown format \"%s\".\n", ptr);
            return LYS_IN_UNKNOWN;
        }
    } else {
        fprintf(stdout, "Input file \"%s\" without extension - unknown format.\n", path);
        return LYS_IN_UNKNOWN;
    }
}

int
cmd_add(const char *arg)
{
    int path_len, ret = 1, index = 0;
    char *path, *dir, *s, *arg_ptr;
    const char * const *searchpaths;
    const struct lys_module *model;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;

    if (strlen(arg) < 5) {
        cmd_add_help();
        return 1;
    }

    arg_ptr = strdup(arg + 3 /* ignore "add" */);

    for (s = strstr(arg_ptr, "-i"); s ; s = strstr(s + 2, "-i")) {
        if (s[2] == '\0' || s[2] == ' ') {
            ly_ctx_set_allimplemented(ctx);
            s[0] = s[1] = ' ';
        }
    }
    s = arg_ptr;

    while (arg_ptr[0] == ' ') {
        ++arg_ptr;
    }
    if (strchr(arg_ptr, ' ')) {
        path_len = strchr(arg_ptr, ' ') - arg_ptr;
    } else {
        path_len = strlen(arg_ptr);
    }
    path = strndup(arg_ptr, path_len);

    searchpaths = ly_ctx_get_searchdirs(ctx);
    if (searchpaths) {
        for (index = 0; searchpaths[index]; index++);
    }

    while (path) {
        format = get_schema_format(path);
        if (format == LYS_IN_UNKNOWN) {
            free(path);
            goto cleanup;
        }

        dir = strdup(path);
        ly_ctx_set_searchdir(ctx, dirname(dir));
        model = lys_parse_path(ctx, path, format);
        ly_ctx_unset_searchdirs(ctx, index);
        free(path);
        free(dir);

        if (!model) {
            /* libyang printed the error messages */
            goto cleanup;
        }

        /* next model */
        arg_ptr += path_len;
        while (arg_ptr[0] == ' ') {
            ++arg_ptr;
        }
        if (strchr(arg_ptr, ' ')) {
            path_len = strchr(arg_ptr, ' ') - arg_ptr;
        } else {
            path_len = strlen(arg_ptr);
        }

        if (path_len) {
            path = strndup(arg_ptr, path_len);
        } else {
            path = NULL;
        }
    }
    if (format == LYS_IN_UNKNOWN) {
        /* no schema on input */
        cmd_add_help();
        goto cleanup;
    }
    ret = 0;

cleanup:
    free(s);
    ly_ctx_unset_allimplemented(ctx);

    return ret;
}

int
cmd_load(const char *arg)
{
    int name_len, ret = 1;
    char *name, *s, *arg_ptr;
    const struct lys_module *model;

    if (strlen(arg) < 6) {
        cmd_load_help();
        return 1;
    }

    arg_ptr = strdup(arg + 4 /* ignore "load" */);

    for (s = strstr(arg_ptr, "-i"); s ; s = strstr(s + 2, "-i")) {
        if (s[2] == '\0' || s[2] == ' ') {
            ly_ctx_set_allimplemented(ctx);
            s[0] = s[1] = ' ';
        }
    }
    s = arg_ptr;

    while (arg_ptr[0] == ' ') {
        ++arg_ptr;
    }
    if (strchr(arg_ptr, ' ')) {
        name_len = strchr(arg_ptr, ' ') - arg_ptr;
    } else {
        name_len = strlen(arg_ptr);
    }
    name = strndup(arg_ptr, name_len);

    while (name) {
        model = ly_ctx_load_module(ctx, name, NULL);
        free(name);
        if (!model) {
            /* libyang printed the error messages */
            goto cleanup;
        }

        /* next model */
        arg_ptr += name_len;
        while (arg_ptr[0] == ' ') {
            ++arg_ptr;
        }
        if (strchr(arg_ptr, ' ')) {
            name_len = strchr(arg_ptr, ' ') - arg_ptr;
        } else {
            name_len = strlen(arg_ptr);
        }

        if (name_len) {
            name = strndup(arg_ptr, name_len);
        } else {
            name = NULL;
        }
    }
    ret = 0;

cleanup:
    free(s);
    ly_ctx_unset_allimplemented(ctx);

    return ret;
}

int
cmd_print(const char *arg)
{
    int c, argc, option_index, ret = 1, tree_ll = 0, tree_opts = 0;
    char **argv = NULL, *ptr, *target_path = NULL, *model_name, *revision;
    const char *out_path = NULL;
    const struct lys_module *module;
    LYS_OUTFORMAT format = LYS_OUT_TREE;
    FILE *output = stdout;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"tree-print-groupings", no_argument, 0, 'g'},
        {"tree-print-uses", no_argument, 0, 'u'},
        {"tree-no-leafref-target", no_argument, 0, 'n'},
        {"tree-path", required_argument, 0, 'P'},
        {"info-path", required_argument, 0, 'P'},
        {"tree-line-length", required_argument, 0, 'L'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc+2)*sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto cleanup;
        }
        argv = rlcd;
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:go:guP:L:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_print_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "yang")) {
                format = LYS_OUT_YANG;
            } else if (!strcmp(optarg, "yin")) {
                format = LYS_OUT_YIN;
            } else if (!strcmp(optarg, "tree")) {
                format = LYS_OUT_TREE;
            } else if (!strcmp(optarg, "tree-rfc")) {
                format = LYS_OUT_TREE;
                tree_opts |= LYS_OUTOPT_TREE_RFC;
            } else if (!strcmp(optarg, "info")) {
                format = LYS_OUT_INFO;
            } else if (!strcmp(optarg, "jsons")) {
                format = LYS_OUT_JSON;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto cleanup;
            }
            break;
        case 'o':
            if (out_path) {
                fprintf(stderr, "Output specified twice.\n");
                goto cleanup;
            }
            out_path = optarg;
            break;
        case 'g':
            tree_opts |= LYS_OUTOPT_TREE_GROUPING;
            break;
        case 'u':
            tree_opts |= LYS_OUTOPT_TREE_USES;
            break;
        case 'n':
            tree_opts |= LYS_OUTOPT_TREE_NO_LEAFREF;
            break;
        case 'P':
            target_path = optarg;
            break;
        case 'L':
            tree_ll = atoi(optarg);
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* file name */
    if (optind == argc) {
        fprintf(stderr, "Missing the module name.\n");
        goto cleanup;
    }

    /* tree fromat with or without gropings */
    if ((tree_opts || tree_ll) && format != LYS_OUT_TREE) {
        fprintf(stderr, "--tree options take effect only in case of the tree output format.\n");
    }

    /* module, revision */
    model_name = argv[optind];
    revision = NULL;
    if (strchr(model_name, '@')) {
        revision = strchr(model_name, '@');
        revision[0] = '\0';
        ++revision;
    }

    module = ly_ctx_get_module(ctx, model_name, revision, 0);
    if (!module) {
        /* not a module, try to find it as a submodule */
        module = (const struct lys_module *)ly_ctx_get_submodule(ctx, NULL, NULL, model_name, revision);
    }

    if (!module) {
        if (revision) {
            fprintf(stderr, "No (sub)module \"%s\" in revision %s found.\n", model_name, revision);
        } else {
            fprintf(stderr, "No (sub)module \"%s\" found.\n", model_name);
        }
        goto cleanup;
    }

    if (out_path) {
        output = fopen(out_path, "w");
        if (!output) {
            fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
            goto cleanup;
        }
    }

    ret = lys_print_file(output, module, format, target_path, tree_ll, tree_opts);
    if (format == LYS_OUT_JSON) {
        fputs("\n", output);
    }

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    return ret;
}

static LYD_FORMAT
detect_data_format(char *filepath)
{
    size_t len;

    /* detect input format according to file suffix */
    len = strlen(filepath);
    for (; isspace(filepath[len - 1]); len--, filepath[len] = '\0'); /* remove trailing whitespaces */
    if (len >= 5 && !strcmp(&filepath[len - 4], ".xml")) {
        return LYD_XML;
    } else if (len >= 6 && !strcmp(&filepath[len - 5], ".json")) {
        return LYD_JSON;
    } else if (len >= 5 && !strcmp(&filepath[len - 4], ".lyb")) {
        return LYD_LYB;
    } else {
        return LYD_UNKNOWN;
    }
}

static int
parse_data(char *filepath, LYD_FORMAT informat, int *options, struct lyd_node *val_tree, const char *rpc_act_file,
           struct lyd_node **result)
{
    struct lyxml_elem *xml = NULL;
    struct lyd_node *data = NULL, *rpc_act = NULL;
    int opts = *options;

    if (informat == LYD_UNKNOWN) {
        /* detect input format according to file suffix */
        informat = detect_data_format(filepath);
    }
    if (informat == LYD_UNKNOWN) {
        fprintf(stderr, "Unable to resolve format of the input file, please add \".xml\", \".json\", or \".lyb\" suffix.\n");
        return EXIT_FAILURE;
    }

    ly_errno = LY_SUCCESS;

    if ((opts & LYD_OPT_TYPEMASK) == LYD_OPT_TYPEMASK) {
        /* automatically detect data type from the data top level */
        if (informat != LYD_XML) {
            fprintf(stderr, "Only XML data can be automatically explored.\n");
            return EXIT_FAILURE;
        }

        xml = lyxml_parse_path(ctx, filepath, 0);
        if (!xml) {
            fprintf(stderr, "Failed to parse XML data for automatic type detection.\n");
            return EXIT_FAILURE;
        }

        /* NOTE: namespace is ignored to simplify usage of this feature */

        if (!strcmp(xml->name, "data")) {
            fprintf(stdout, "Parsing %s as complete datastore.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_ADD_YANGLIB;
        } else if (!strcmp(xml->name, "config")) {
            fprintf(stdout, "Parsing %s as config data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
        } else if (!strcmp(xml->name, "get-reply")) {
            fprintf(stdout, "Parsing %s as <get> reply data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
        } else if (!strcmp(xml->name, "get-config-reply")) {
            fprintf(stdout, "Parsing %s as <get-config> reply data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
        } else if (!strcmp(xml->name, "edit-config")) {
            fprintf(stdout, "Parsing %s as <edit-config> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
        } else if (!strcmp(xml->name, "rpc")) {
            fprintf(stdout, "Parsing %s as <rpc> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
        } else if (!strcmp(xml->name, "rpc-reply")) {
            if (!rpc_act_file) {
                fprintf(stderr, "RPC/action reply data require additional argument (file with the RPC/action).\n");
                lyxml_free(ctx, xml);
                return EXIT_FAILURE;
            }
            fprintf(stdout, "Parsing %s as <rpc-reply> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
            rpc_act = lyd_parse_path(ctx, rpc_act_file, informat, LYD_OPT_RPC, val_tree);
            if (!rpc_act) {
                fprintf(stderr, "Failed to parse RPC/action.\n");
                lyxml_free(ctx, xml);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(xml->name, "notification")) {
            fprintf(stdout, "Parsing %s as <notification> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
        } else if (!strcmp(xml->name, "yang-data")) {
            fprintf(stdout, "Parsing %s as <yang-data> data.\n", filepath);
            opts = (opts & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_TEMPLATE;
            if (!rpc_act_file) {
                fprintf(stderr, "YANG-DATA require additional argument (name instance of yang-data extension).\n");
                lyxml_free(ctx, xml);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Invalid top-level element for automatic data type recognition.\n");
            lyxml_free(ctx, xml);
            return EXIT_FAILURE;
        }

        if (opts & LYD_OPT_RPCREPLY) {
            data = lyd_parse_xml(ctx, &xml->child, opts, rpc_act, val_tree);
        } else if (opts & (LYD_OPT_RPC | LYD_OPT_NOTIF)) {
            data = lyd_parse_xml(ctx, &xml->child, opts, val_tree);
        } else if (opts & LYD_OPT_DATA_TEMPLATE) {
            data = lyd_parse_xml(ctx, &xml->child, opts, rpc_act_file);
        } else {
            data = lyd_parse_xml(ctx, &xml->child, opts);
        }
        lyxml_free(ctx, xml);
    } else {
        if (opts & LYD_OPT_RPCREPLY) {
            if (!rpc_act_file) {
                fprintf(stderr, "RPC/action reply data require additional argument (file with the RPC/action).\n");
                return EXIT_FAILURE;
            }
            rpc_act = lyd_parse_path(ctx, rpc_act_file, informat, LYD_OPT_RPC, val_tree);
            if (!rpc_act) {
                fprintf(stderr, "Failed to parse RPC/action.\n");
                return EXIT_FAILURE;
            }
            data = lyd_parse_path(ctx, filepath, informat, opts, rpc_act, val_tree);
        } else if (opts & (LYD_OPT_RPC | LYD_OPT_NOTIF)) {
            data = lyd_parse_path(ctx, filepath, informat, opts, val_tree);
        } else if (opts & LYD_OPT_DATA_TEMPLATE) {
            if (!rpc_act_file) {
                fprintf(stderr, "YANG-DATA require additional argument (name instance of yang-data extension).\n");
                return EXIT_FAILURE;
            }
            data = lyd_parse_path(ctx, filepath, informat, opts, rpc_act_file);
        } else {
            if (!(opts & LYD_OPT_TYPEMASK)) {
                /* automatically add yang-library data */
                opts |= LYD_OPT_DATA_ADD_YANGLIB;
            }
            data = lyd_parse_path(ctx, filepath, informat, opts);
        }
    }
    lyd_free_withsiblings(rpc_act);

    if (ly_errno) {
        fprintf(stderr, "Failed to parse data.\n");
        lyd_free_withsiblings(data);
        return EXIT_FAILURE;
    }

    *result = data;
    *options = opts;
    return EXIT_SUCCESS;
}

int
cmd_data(const char *arg)
{
    int c, argc, option_index, ret = 1;
    int options = 0, printopt = 0;
    char **argv = NULL, *ptr;
    const char *out_path = NULL;
    struct lyd_node *data = NULL, *val_tree = NULL;
    LYD_FORMAT outformat = LYD_UNKNOWN, informat = LYD_UNKNOWN;
    FILE *output = stdout;
    static struct option long_options[] = {
        {"defaults", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {"in-format", required_argument, 0, 'F'},
        {"option", required_argument, 0, 't'},
        {"output", required_argument, 0, 'o'},
        {"running", required_argument, 0, 'r'},
        {"strict", no_argument, 0, 's'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc + 2) * sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto cleanup;
        }
        argv = rlcd;
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "d:hf:F:o:st:r:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'd':
            if (!strcmp(optarg, "all")) {
                printopt = (printopt & ~LYP_WD_MASK) | LYP_WD_ALL;
            } else if (!strcmp(optarg, "all-tagged")) {
                printopt = (printopt & ~LYP_WD_MASK) | LYP_WD_ALL_TAG;
            } else if (!strcmp(optarg, "trim")) {
                printopt = (printopt & ~LYP_WD_MASK) | LYP_WD_TRIM;
            } else if (!strcmp(optarg, "implicit-tagged")) {
                printopt = (printopt & ~LYP_WD_MASK) | LYP_WD_IMPL_TAG;
            }
            break;
        case 'h':
            cmd_data_help();
            ret = 0;
            goto cleanup;
        case 'f':
            if (!strcmp(optarg, "xml")) {
                outformat = LYD_XML;
            } else if (!strcmp(optarg, "json")) {
                outformat = LYD_JSON;
            } else if (!strcmp(optarg, "lyb")) {
                outformat = LYD_LYB;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto cleanup;
            }
            break;
        case 'F':
            if (!strcmp(optarg, "xml")) {
                informat = LYD_XML;
            } else if (!strcmp(optarg, "json")) {
                informat = LYD_JSON;
            } else if (!strcmp(optarg, "lyb")) {
                informat = LYD_LYB;
            } else {
                fprintf(stderr, "Unknown input format \"%s\".\n", optarg);
                goto cleanup;
            }
            break;
        case 'o':
            if (out_path) {
                fprintf(stderr, "Output specified twice.\n");
                goto cleanup;
            }
            out_path = optarg;
            break;
        case 'r':
            if (val_tree || (options & LYD_OPT_NOEXTDEPS)) {
                fprintf(stderr, "The running datastore (-r) cannot be set multiple times.\n");
                goto cleanup;
            }
            if (optarg[0] == '!') {
                /* ignore extenral dependencies to the running datastore */
                options |= LYD_OPT_NOEXTDEPS;
            } else {
                /* external file with the running datastore */
                val_tree = lyd_parse_path(ctx, optarg, LYD_XML, LYD_OPT_DATA_NO_YANGLIB);
                if (!val_tree) {
                    fprintf(stderr, "Failed to parse the additional data tree for validation.\n");
                    goto cleanup;
                }
            }
            break;
        case 's':
            options |= LYD_OPT_STRICT;
            options |= LYD_OPT_OBSOLETE;
            break;
        case 't':
            if (!strcmp(optarg, "auto")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_TYPEMASK;
            } else if (!strcmp(optarg, "data")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA;
            } else if (!strcmp(optarg, "config")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
            } else if (!strcmp(optarg, "get")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
            } else if (!strcmp(optarg, "getconfig")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
            } else if (!strcmp(optarg, "edit")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
            } else if (!strcmp(optarg, "rpc")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
            } else if (!strcmp(optarg, "rpcreply")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
            } else if (!strcmp(optarg, "notif")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
            } else if (!strcmp(optarg, "yangdata")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_TEMPLATE;
            } else {
                fprintf(stderr, "Invalid parser option \"%s\".\n", optarg);
                cmd_data_help();
                goto cleanup;
            }
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* file name */
    if (optind == argc) {
        fprintf(stderr, "Missing the data file name.\n");
        goto cleanup;
    }

    if (parse_data(argv[optind], informat, &options, val_tree, argv[optind + 1], &data)) {
        goto cleanup;
    }

    if (out_path) {
        output = fopen(out_path, "w");
        if (!output) {
            fprintf(stderr, "Could not open the output file (%s).\n", strerror(errno));
            goto cleanup;
        }
    }

    if (outformat != LYD_UNKNOWN) {
        if (options & LYD_OPT_RPCREPLY) {
            lyd_print_file(output, data->child, outformat, LYP_WITHSIBLINGS | LYP_FORMAT | printopt);
        } else {
            lyd_print_file(output, data, outformat, LYP_WITHSIBLINGS | LYP_FORMAT | printopt);
        }
    }

    ret = 0;

cleanup:
    free(*argv);
    free(argv);

    if (output && (output != stdout)) {
        fclose(output);
    }

    lyd_free_withsiblings(val_tree);
    lyd_free_withsiblings(data);

    return ret;
}

int
cmd_xpath(const char *arg)
{
    int c, argc, option_index, ret = 1, long_str;
    char **argv = NULL, *ptr, *expr = NULL;
    unsigned int i, j;
    int options = 0;
    LYD_FORMAT informat = LYD_UNKNOWN;
    struct lyd_node *data = NULL, *node, *val_tree = NULL;
    struct lyd_node_leaf_list *key;
    struct ly_set *set;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"expr", required_argument, 0, 'e'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    long_str = 0;
    argc = 1;
    argv = malloc(2 * sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        if (long_str) {
            ptr[-1] = ' ';
            if (ptr[strlen(ptr) - 1] == long_str) {
                long_str = 0;
                ptr[strlen(ptr) - 1] = '\0';
            }
        } else {
            rlcd = realloc(argv, (argc + 2) * sizeof *argv);
            if (!rlcd) {
                fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                goto cleanup;
            }
            argv = rlcd;
            argv[argc] = ptr;
            if (ptr[0] == '"') {
                long_str = '"';
                ++argv[argc];
            }
            if (ptr[0] == '\'') {
                long_str = '\'';
                ++argv[argc];
            }
            if (ptr[strlen(ptr) - 1] == long_str) {
                long_str = 0;
                ptr[strlen(ptr) - 1] = '\0';
            }
            ++argc;
        }
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "he:t:x:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_xpath_help();
            ret = 0;
            goto cleanup;
        case 'e':
            expr = optarg;
            break;
        case 't':
            if (!strcmp(optarg, "auto")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_TYPEMASK;
            } else if (!strcmp(optarg, "config")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
            } else if (!strcmp(optarg, "get")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
            } else if (!strcmp(optarg, "getconfig")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
            } else if (!strcmp(optarg, "edit")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
            } else if (!strcmp(optarg, "rpc")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
            } else if (!strcmp(optarg, "rpcreply")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
            } else if (!strcmp(optarg, "notif")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
            } else if (!strcmp(optarg, "yangdata")) {
                options = (options & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_TEMPLATE;
            } else {
                fprintf(stderr, "Invalid parser option \"%s\".\n", optarg);
                cmd_data_help();
                goto cleanup;
            }
            break;
        case 'x':
            val_tree = lyd_parse_path(ctx, optarg, LYD_XML, LYD_OPT_CONFIG);
            if (!val_tree) {
                fprintf(stderr, "Failed to parse the additional data tree for validation.\n");
                goto cleanup;
            }
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "Missing the file with data.\n");
        goto cleanup;
    }

    if (!expr) {
        fprintf(stderr, "Missing the XPath expression.\n");
        goto cleanup;
    }

    if (parse_data(argv[optind], informat, &options, val_tree, argv[optind + 1], &data)) {
        goto cleanup;
    }

    if (!(set = lyd_find_path(data, expr))) {
        goto cleanup;
    }

    /* print result */
    printf("Result:\n");
    if (!set->number) {
        printf("\tEmpty\n");
    } else {
        for (i = 0; i < set->number; ++i) {
            node = set->set.d[i];
            switch (node->schema->nodetype) {
            case LYS_CONTAINER:
                printf("\tContainer ");
                break;
            case LYS_LEAF:
                printf("\tLeaf ");
                break;
            case LYS_LEAFLIST:
                printf("\tLeaflist ");
                break;
            case LYS_LIST:
                printf("\tList ");
                break;
            case LYS_ANYXML:
                printf("\tAnyxml ");
                break;
            case LYS_ANYDATA:
                printf("\tAnydata ");
                break;
            default:
                printf("\tUnknown ");
                break;
            }
            printf("\"%s\"", node->schema->name);
            if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                printf(" (val: %s)", ((struct lyd_node_leaf_list *)node)->value_str);
            } else if (node->schema->nodetype == LYS_LIST) {
                key = (struct lyd_node_leaf_list *)node->child;
                printf(" (");
                for (j = 0; j < ((struct lys_node_list *)node->schema)->keys_size; ++j) {
                    if (j) {
                        printf(" ");
                    }
                    printf("\"%s\": %s", key->schema->name, key->value_str);
                    key = (struct lyd_node_leaf_list *)key->next;
                }
                printf(")");
            }
            printf("\n");
        }
    }
    printf("\n");

    ly_set_free(set);
    ret = 0;

cleanup:
    free(*argv);
    free(argv);

    lyd_free_withsiblings(data);

    return ret;
}

int
print_list(FILE *out, struct ly_ctx *ctx, LYD_FORMAT outformat)
{
    struct lyd_node *ylib;
    uint32_t idx = 0, has_modules = 0;
    uint8_t u;
    const struct lys_module *mod;

    if (outformat != LYD_UNKNOWN) {
        ylib = ly_ctx_info(ctx);
        if (!ylib) {
            fprintf(stderr, "Getting context info (ietf-yang-library data) failed.\n");
            return 1;
        }

        lyd_print_file(out, ylib, outformat, LYP_WITHSIBLINGS | LYP_FORMAT);
        lyd_free_withsiblings(ylib);
        return 0;
    }

    /* iterate schemas in context and provide just the basic info */
    fprintf(out, "List of the loaded models:\n");
    while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
        has_modules++;

        /* conformance print */
        if (mod->implemented) {
            fprintf(out, "\tI");
        } else {
            fprintf(out, "\ti");
        }

        /* module print */
        fprintf(out, " %s", mod->name);
        if (mod->rev_size) {
            fprintf(out, "@%s", mod->rev[0].date);
        }

        /* submodules print */
        if (mod->inc_size) {
            fprintf(out, " (");
            for (u = 0; u < mod->inc_size; u++) {
                fprintf(out, "%s%s", !u ? "" : ",", mod->inc[u].submodule->name);
                if (mod->inc[u].submodule->rev_size) {
                    fprintf(out, "@%s", mod->inc[u].submodule->rev[0].date);
                }
            }
            fprintf(out, ")");
        }

        /* finish the line */
        fprintf(out, "\n");
    }

    if (!has_modules) {
        fprintf(out, "\t(none)\n");
    }

    return 0;
}

int
cmd_list(const char *arg)
{
    char **argv = NULL, *ptr;
    int c, argc, option_index;
    LYD_FORMAT outformat = LYD_UNKNOWN;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc+2)*sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto error;
        }
        argv = rlcd;
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "hf:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_data_help();
            free(*argv);
            free(argv);
            return 0;
        case 'f':
            if (!strcmp(optarg, "xml")) {
                outformat = LYD_XML;
            } else if (!strcmp(optarg, "json")) {
                outformat = LYD_JSON;
            } else {
                fprintf(stderr, "Unknown output format \"%s\".\n", optarg);
                goto error;
            }
            break;
        case '?':
            /* getopt_long() prints message */
            goto error;
        }
    }
    if (optind != argc) {
        fprintf(stderr, "Unknown parameter \"%s\"\n", argv[optind]);
error:
        free(*argv);
        free(argv);
        return 1;
    }
    free(*argv);
    free(argv);

    return print_list(stdout, ctx, outformat);
}

int
cmd_feature(const char *arg)
{
    int c, i, argc, option_index, ret = 1, task = 0;
    unsigned int max_len;
    char **argv = NULL, *ptr, *model_name, *revision, *feat_names = NULL;
    const char **names;
    uint8_t *states;
    const struct lys_module *module;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"enable", required_argument, 0, 'e'},
        {"disable", required_argument, 0, 'd'},
        {NULL, 0, 0, 0}
    };
    void *rlcd;

    argc = 1;
    argv = malloc(2*sizeof *argv);
    *argv = strdup(arg);
    ptr = strtok(*argv, " ");
    while ((ptr = strtok(NULL, " "))) {
        rlcd = realloc(argv, (argc + 2) * sizeof *argv);
        if (!rlcd) {
            fprintf(stderr, "Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
            goto cleanup;
        }
        argv = rlcd;
        argv[argc++] = ptr;
    }
    argv[argc] = NULL;

    optind = 0;
    while (1) {
        option_index = 0;
        c = getopt_long(argc, argv, "he:d:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            cmd_feature_help();
            ret = 0;
            goto cleanup;
        case 'e':
            if (task) {
                fprintf(stderr, "Only one of enable or disable can be specified.\n");
                goto cleanup;
            }
            task = 1;
            feat_names = optarg;
            break;
        case 'd':
            if (task) {
                fprintf(stderr, "Only one of enable, or disable can be specified.\n");
                goto cleanup;
            }
            task = 2;
            feat_names = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option \"%d\".\n", (char)c);
            goto cleanup;
        }
    }

    /* module name */
    if (optind == argc) {
        fprintf(stderr, "Missing the module name.\n");
        goto cleanup;
    }

    revision = NULL;
    model_name = argv[optind];
    if (strchr(model_name, '@')) {
        revision = strchr(model_name, '@');
        revision[0] = '\0';
        ++revision;
    }

    module = ly_ctx_get_module(ctx, model_name, revision, 0);
    if (!module) {
        /* not a module, try to find it as a submodule */
        module = (const struct lys_module *)ly_ctx_get_submodule(ctx, NULL, NULL, model_name, revision);
    }

    if (module == NULL) {
        if (revision) {
            fprintf(stderr, "No (sub)module \"%s\" in revision %s found.\n", model_name, revision);
        } else {
            fprintf(stderr, "No (sub)module \"%s\" found.\n", model_name);
        }
        goto cleanup;
    }

    if (!task) {
        printf("%s features:\n", module->name);

        names = lys_features_list(module, &states);

        /* get the max len */
        max_len = 0;
        for (i = 0; names[i]; ++i) {
            if (strlen(names[i]) > max_len) {
                max_len = strlen(names[i]);
            }
        }
        for (i = 0; names[i]; ++i) {
            printf("\t%-*s (%s)\n", max_len, names[i], states[i] ? "on" : "off");
        }
        free(names);
        free(states);
        if (!i) {
            printf("\t(none)\n");
        }
    } else {
        feat_names = strtok(feat_names, ",");
        while (feat_names) {
            if (((task == 1) && lys_features_enable(module, feat_names))
                    || ((task == 2) && lys_features_disable(module, feat_names))) {
                fprintf(stderr, "Feature \"%s\" not found.\n", feat_names);
                ret = 1;
            }
            feat_names = strtok(NULL, ",");
        }
    }

cleanup:
    free(*argv);
    free(argv);

    return ret;
}

int
cmd_searchpath(const char *arg)
{
    const char *path;
    const char * const *searchpaths;
    int index;
    struct stat st;

    for (path = strchr(arg, ' '); path && (path[0] == ' '); ++path);
    if (!path || (path[0] == '\0')) {
        searchpaths = ly_ctx_get_searchdirs(ctx);
        if (searchpaths) {
            for (index = 0; searchpaths[index]; index++) {
                fprintf(stdout, "%s\n", searchpaths[index]);
            }
        }
        return 0;
    }

    if ((!strncmp(path, "-h", 2) && (path[2] == '\0' || path[2] == ' ')) ||
        (!strncmp(path, "--help", 6) && (path[6] == '\0' || path[6] == ' '))) {
        cmd_searchpath_help();
        return 0;
    } else if (!strncmp(path, "--clear", 7) && (path[7] == '\0' || path[7] == ' ')) {
        ly_ctx_unset_searchdirs(ctx, -1);
        return 0;
    }

    if (stat(path, &st) == -1) {
        fprintf(stderr, "Failed to stat the search path (%s).\n", strerror(errno));
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "\"%s\" is not a directory.\n", path);
        return 1;
    }

    ly_ctx_set_searchdir(ctx, path);

    return 0;
}

int
cmd_clear(const char *arg)
{
    int i, options = 0;
    char *ylpath;
    const char * const *searchpaths;
    struct ly_ctx *ctx_new;
    LYD_FORMAT format;

    /* get optional yang library file name */
    for (i = 5; arg[i] && isspace(arg[i]); i++);
    if (arg[i]) {
        if (arg[i] == '-' && arg[i + 1] == 'e') {
            options = LY_CTX_NOYANGLIBRARY;
            goto create_empty;
        } else {
            ylpath = strdup(&arg[i]);
            format = detect_data_format(ylpath);
            if (format == LYD_UNKNOWN) {
                free(ylpath);
                fprintf(stderr, "Unable to resolve format of the yang library file, please add \".xml\" or \".json\" suffix.\n");
                goto create_empty;
            }
            searchpaths = ly_ctx_get_searchdirs(ctx);
            ctx_new = ly_ctx_new_ylpath(searchpaths ? searchpaths[0] : NULL, ylpath, format, 0);
            free(ylpath);
        }
    } else {
create_empty:
        ctx_new = ly_ctx_new(NULL, options);
    }

    if (!ctx_new) {
        fprintf(stderr, "Failed to create context.\n");
        return 1;
    }

    /* final switch */
    ly_ctx_destroy(ctx, NULL);
    ctx = ctx_new;

    return 0;
}

int
cmd_verb(const char *arg)
{
    const char *verb;
    if (strlen(arg) < 5) {
        cmd_verb_help();
        return 1;
    }

    verb = arg + 5;
    if (!strcmp(verb, "error") || !strcmp(verb, "0")) {
        ly_verb(LY_LLERR);
#ifndef NDEBUG
        ly_verb_dbg(0);
#endif
    } else if (!strcmp(verb, "warning") || !strcmp(verb, "1")) {
        ly_verb(LY_LLWRN);
#ifndef NDEBUG
        ly_verb_dbg(0);
#endif
    } else if (!strcmp(verb, "verbose")  || !strcmp(verb, "2")) {
        ly_verb(LY_LLVRB);
#ifndef NDEBUG
        ly_verb_dbg(0);
#endif
    } else if (!strcmp(verb, "debug")  || !strcmp(verb, "3")) {
        ly_verb(LY_LLDBG);
#ifndef NDEBUG
        ly_verb_dbg(LY_LDGDICT | LY_LDGYANG | LY_LDGYIN | LY_LDGXPATH | LY_LDGDIFF);
#endif
    } else {
        fprintf(stderr, "Unknown verbosity \"%s\"\n", verb);
        return 1;
    }

    return 0;
}

#ifndef NDEBUG

int
cmd_debug(const char *arg)
{
    const char *beg, *end;
    int grps = 0;
    if (strlen(arg) < 6) {
        cmd_debug_help();
        return 1;
    }

    end = arg + 6;
    while (end[0]) {
        for (beg = end; isspace(beg[0]); ++beg);
        if (!beg[0]) {
            break;
        }

        for (end = beg; (end[0] && !isspace(end[0])); ++end);

        if (!strncmp(beg, "dict", end - beg)) {
            grps |= LY_LDGDICT;
        } else if (!strncmp(beg, "yang", end - beg)) {
            grps |= LY_LDGYANG;
        } else if (!strncmp(beg, "yin", end - beg)) {
            grps |= LY_LDGYIN;
        } else if (!strncmp(beg, "xpath", end - beg)) {
            grps |= LY_LDGXPATH;
        } else if (!strncmp(beg, "diff", end - beg)) {
            grps |= LY_LDGDIFF;
        } else {
            fprintf(stderr, "Unknown debug group \"%.*s\"\n", (int)(end - beg), beg);
            return 1;
        }
    }
    ly_verb_dbg(grps);

    return 0;
}

#endif

int
cmd_quit(const char *UNUSED(arg))
{
    done = 1;
    return 0;
}

int
cmd_help(const char *arg)
{
    int i;
    char *args = strdup(arg);
    char *cmd = NULL;

    strtok(args, " ");
    if ((cmd = strtok(NULL, " ")) == NULL) {

generic_help:
        fprintf(stdout, "Available commands:\n");

        for (i = 0; commands[i].name; i++) {
            if (commands[i].helpstring != NULL) {
                fprintf(stdout, "  %-15s %s\n", commands[i].name, commands[i].helpstring);
            }
        }
    } else {
        /* print specific help for the selected command */

        /* get the command of the specified name */
        for (i = 0; commands[i].name; i++) {
            if (strcmp(cmd, commands[i].name) == 0) {
                break;
            }
        }

        /* execute the command's help if any valid command specified */
        if (commands[i].name) {
            if (commands[i].help_func != NULL) {
                commands[i].help_func();
            } else {
                printf("%s\n", commands[i].helpstring);
            }
        } else {
            /* if unknown command specified, print the list of commands */
            printf("Unknown command \'%s\'\n", cmd);
            goto generic_help;
        }
    }

    free(args);
    return 0;
}

COMMAND commands[] = {
        {"help", cmd_help, NULL, "Display commands description"},
        {"add", cmd_add, cmd_add_help, "Add a new model from a specific file"},
        {"load", cmd_load, cmd_load_help, "Load a new model from the searchdirs"},
        {"print", cmd_print, cmd_print_help, "Print a model"},
        {"data", cmd_data, cmd_data_help, "Load, validate and optionally print instance data"},
        {"xpath", cmd_xpath, cmd_xpath_help, "Get data nodes satisfying an XPath expression"},
        {"list", cmd_list, cmd_list_help, "List all the loaded models"},
        {"feature", cmd_feature, cmd_feature_help, "Print/enable/disable all/specific features of models"},
        {"searchpath", cmd_searchpath, cmd_searchpath_help, "Print/set the search path(s) for models"},
        {"clear", cmd_clear, cmd_clear_help, "Clear the context - remove all the loaded models"},
        {"verb", cmd_verb, cmd_verb_help, "Change verbosity"},
#ifndef NDEBUG
        {"debug", cmd_debug, cmd_debug_help, "Display specific debug message groups"},
#endif
        {"quit", cmd_quit, NULL, "Quit the program"},
        /* synonyms for previous commands */
        {"?", cmd_help, NULL, "Display commands description"},
        {"exit", cmd_quit, NULL, "Quit the program"},
        {NULL, NULL, NULL, NULL}
};
