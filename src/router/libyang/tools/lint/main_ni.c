/**
 * @file main_ni.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool - noninteractive code
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include "libyang.h"

#include "common.h"
#include "out.h"
#include "tools/config.h"

/**
 * @brief Context structure to hold and pass variables in a structured form.
 */
struct context {
    /* libyang context for the run */
    struct ly_ctx *ctx;

    /* prepared output (--output option or stdout by default) */
    struct ly_out *out;

    struct ly_set searchpaths;

    /* options flags */
    uint8_t list;        /* -l option to print list of schemas */

    /* line length for 'tree' format */
    size_t line_length; /* --tree-line-length */

    /*
     * schema
     */
    /* set schema modules' features via --feature option (struct schema_features *) */
    struct ly_set schema_features;

    /* set of loaded schema modules (struct lys_module *) */
    struct ly_set schema_modules;

    /* options to parse and print schema modules */
    uint32_t schema_parse_options;
    uint32_t schema_print_options;

    /* specification of printing schema node subtree, option --schema-node */
    const char *schema_node_path;
    const struct lysc_node *schema_node;

    /* value of --format in case of schema format */
    LYS_OUTFORMAT schema_out_format;

    /*
     * data
     */
    /* various options based on --type option */
    enum lyd_type data_type;
    uint32_t data_parse_options;
    uint32_t data_validate_options;
    uint32_t data_print_options;

    /* flag for --merge option */
    uint8_t data_merge;

    /* value of --format in case of data format */
    LYD_FORMAT data_out_format;

    /* input data files (struct cmdline_file *) */
    struct ly_set data_inputs;

    /* storage for --operational */
    struct cmdline_file data_operational;
};

static void
erase_context(struct context *c)
{
    /* data */
    ly_set_erase(&c->data_inputs, free_cmdline_file);
    ly_in_free(c->data_operational.in, 1);

    /* schema */
    ly_set_erase(&c->schema_features, free_features);
    ly_set_erase(&c->schema_modules, NULL);

    /* context */
    ly_set_erase(&c->searchpaths, NULL);

    ly_out_free(c->out, NULL,  0);
    ly_ctx_destroy(c->ctx);
}

static void
version(void)
{
    printf("yanglint %s\n", PROJECT_VERSION);
}

static void
help(int shortout)
{

    printf("Usage:\n"
            "    yanglint [Options] [-f { yang | yin | info}] <schema>...\n"
            "        Validates the YANG module in <schema>, and all its dependencies.\n\n"
            "    yanglint [Options] [-f { xml | json }] <schema>... <file> ...\n"
            "        Validates the YANG modeled data in <file> according to the <schema>.\n\n"
            "    yanglint\n"
            "        Starts interactive mode with more features.\n\n");

    if (shortout) {
        return;
    }
    printf("Options:\n"
            "  -h, --help    Show this help message and exit.\n"
            "  -v, --version Show version number and exit.\n"
            "  -V, --verbose Show verbose messages, can be used multiple times to\n"
            "                increase verbosity.\n"
#ifndef NDEBUG
            "  -G GROUPS, --debug=GROUPS\n"
            "                Enable printing of specific debugging message group\n"
            "                (nothing will be printed unless verbosity is set to debug):\n"
            "                <group>[,<group>]* (dict, xpath)\n\n"
#endif

            "  -d MODE, --default=MODE\n"
            "                Print data with default values, according to the MODE\n"
            "                (to print attributes, ietf-netconf-with-defaults model\n"
            "                must be loaded):\n"
            "      all             - Add missing default nodes.\n"
            "      all-tagged      - Add missing default nodes and mark all the default\n"
            "                        nodes with the attribute.\n"
            "      trim            - Remove all nodes with a default value.\n"
            "      implicit-tagged - Add missing nodes and mark them with the attribute.\n\n"

            "  -D, --disable-searchdir\n"
            "                Do not implicitly search in current working directory for\n"
            "                schema modules. If specified a second time, do not even\n"
            "                search in the module directory (all modules must be \n"
            "                explicitly specified).\n\n"

            "  -p PATH, --path=PATH\n"
            "                Search path for schema (YANG/YIN) modules. The option can be\n"
            "                used multiple times. The current working directory and the\n"
            "                path of the module being added is used implicitly.\n\n"

            "  -F FEATURES, --features=FEATURES\n"
            "                Features to support, default all.\n"
            "                <modname>:[<feature>,]*\n\n"

            "  -i, --makeimplemented\n"
            "                Make the imported modules \"referenced\" from any loaded\n"
            "                module also implemented. If specified a second time, all the\n"
            "                modules are set implemented.\n\n"

            "  -l, --list    Print info about the loaded schemas.\n"
            "                (i - imported module, I - implemented module)\n"
            "                In case the -f option with data encoding is specified,\n"
            "                the list is printed as ietf-yang-library data.\n\n"

            "  -L LINE_LENGTH, --tree-line-length=LINE_LENGTH\n"
            "                The limit of the maximum line length on which the 'tree'\n"
            "                format will try to be printed.\n\n"

            "  -o OUTFILE, --output=OUTFILE\n"
            "                Write the output to OUTFILE instead of stdout.\n\n"

            "  -f FORMAT, --format=FORMAT\n"
            "                Convert input into FORMAT. Supported formats: \n"
            "                yang, yin, tree and info for schemas,\n"
            "                xml, json for data.\n\n"

            "  -P PATH, --schema-node=PATH\n"
            "                Print only the specified subtree of the schema.\n"
            "                The PATH is the XPath subset mentioned in documentation as\n"
            "                the Path format. The option can be combined with --single-node\n"
            "                option to print information only about the specified node.\n"
            "  -q, --single-node\n"
            "                Supplement to the --schema-node option to print information\n"
            "                only about a single node specified as PATH argument.\n\n"

            "  -n, --not-strict\n"
            "                Do not require strict data parsing (silently skip unknown data),\n"
            "                has no effect for schemas.\n\n"

            "  -e, --present Validate only with the schema modules whose data actually\n"
            "                exist in the provided input data files. Takes effect only\n"
            "                with the 'data' or 'config' TYPEs. Used to avoid requiring\n"
            "                mandatory nodes from modules which data are not present in the\n"
            "                provided input data files.\n\n"

            "  -t TYPE, --type=TYPE\n"
            "                Specify data tree type in the input data file(s):\n"
            "        data          - Complete datastore with status data (default type).\n"
            "        config        - Configuration datastore (without status data).\n"
            "        get           - Result of the NETCONF <get> operation.\n"
            "        getconfig     - Result of the NETCONF <get-config> operation.\n"
            "        edit          - Content of the NETCONF <edit-config> operation.\n"
            "        rpc           - Content of the NETCONF <rpc> message, defined as YANG's\n"
            "                        RPC/Action input statement.\n"
            "        reply         - Reply to the RPC/Action. Note that the reply data are\n"
            "                        expected inside a container representing the original\n"
            "                        RPC/Action. This is necessary to identify appropriate\n"
            "                        data definitions in the schema module.\n"
            "        notif         - Notification instance (content of the <notification>\n"
            "                        element without <eventTime>).\n"

            "  -O FILE, --operational=FILE\n"
            "                Provide optional data to extend validation of the 'rpc',\n"
            "                'reply' or 'notif' TYPEs. The FILE is supposed to contain\n"
            "                the :running configuration datastore and state data\n"
            "                (operational datastore) referenced from the RPC/Notification.\n\n"

            "  -m, --merge   Merge input data files into a single tree and validate at\n"
            "                once.The option has effect only for 'data' and 'config' TYPEs.\n\n"

            "  -y, --yang-library\n"
            "                Load and implement internal \"ietf-yang-library\" YANG module.\n"
            "                Note that this module includes definitions of mandatory state\n"
            "                data that can result in unexpected data validation errors.\n\n"
            "");
}

static void
libyang_verbclb(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    char *levstr;

    switch (level) {
    case LY_LLERR:
        levstr = "err :";
        break;
    case LY_LLWRN:
        levstr = "warn:";
        break;
    case LY_LLVRB:
        levstr = "verb:";
        break;
    default:
        levstr = "dbg :";
        break;
    }
    if (path) {
        fprintf(stderr, "libyang %s %s (%s)\n", levstr, msg, path);
    } else {
        fprintf(stderr, "libyang %s %s\n", levstr, msg);
    }
}

static int
fill_context_inputs(int argc, char *argv[], struct context *c)
{
    struct ly_in *in;

    /* process the operational content if any */
    if (c->data_operational.path) {
        if (get_input(c->data_operational.path, NULL, &c->data_operational.format, &c->data_operational.in)) {
            return -1;
        }
    }

    for (int i = 0; i < argc - optind; i++) {
        LYS_INFORMAT format_schema = LYS_IN_UNKNOWN;
        LYD_FORMAT format_data = LYD_UNKNOWN;
        in = NULL;

        if (get_input(argv[optind + i], &format_schema, &format_data, &in)) {
            goto error;
        }

        if (format_schema) {
            LY_ERR ret;
            uint8_t path_unset = 1; /* flag to unset the path from the searchpaths list (if not already present) */
            char *dir, *module;
            const char *fall = "*";
            const char **features = &fall;
            const struct lys_module *mod;

            if (parse_schema_path(argv[optind + i], &dir, &module)) {
                goto error;
            }

            /* add temporarily also the path of the module itself */
            if (ly_ctx_set_searchdir(c->ctx, dir) == LY_EEXIST) {
                path_unset = 0;
            }

            /* get features list for this module */
            get_features(&c->schema_features, module, &features);

            /* temporary cleanup */
            free(dir);
            free(module);

            ret = lys_parse(c->ctx, in, format_schema, features, &mod);
            ly_ctx_unset_searchdir_last(c->ctx, path_unset);
            ly_in_free(in, 1);
            in = NULL;
            if (ret) {
                YLMSG_E("Processing schema module from %s failed.\n", argv[optind + i]);
                goto error;
            }

            if (c->schema_out_format) {
                /* modules will be printed */
                if (ly_set_add(&c->schema_modules, (void *)mod, 1, NULL)) {
                    YLMSG_E("Storing parsed schema module (%s) for print failed.\n", argv[optind + i]);
                    goto error;
                }
            }
        } else if (format_data) {
            if (!fill_cmdline_file(&c->data_inputs, in, argv[optind + i], format_data)) {
                goto error;
            }
            in = NULL;
        } else {
            ly_in_free(in, 1);
            in = NULL;
        }
    }

    return 0;

error:
    ly_in_free(in, 1);
    return -1;
}

/**
 * @brief Process command line options and store the settings into the context.
 *
 * return -1 in case of error;
 * return 0 in case of success and ready to process
 * return 1 in case of success, but expect to exit.
 */
static int
fill_context(int argc, char *argv[], struct context *c)
{
    int ret;

    int opt, opt_index;
    struct option options[] = {
        {"default",          required_argument, NULL, 'd'},
        {"disable-searchdir", no_argument,      NULL, 'D'},
        {"present",          no_argument,       NULL, 'e'},
        {"format",           required_argument, NULL, 'f'},
        {"features",         required_argument, NULL, 'F'},
#ifndef NDEBUG
        {"debug",            required_argument, NULL, 'G'},
#endif
        {"help",             no_argument,       NULL, 'h'},
        {"makeimplemented",  no_argument,       NULL, 'i'},
        {"list",             no_argument,       NULL, 'l'},
        {"tree-line-length", required_argument, NULL, 'L'},
        {"merge",            no_argument,       NULL, 'm'},
        {"yang-library",     no_argument,       NULL, 'y'},
        {"output",           required_argument, NULL, 'o'},
        {"operational",      required_argument, NULL, 'O'},
        {"path",             required_argument, NULL, 'p'},
        {"schema-node",      required_argument, NULL, 'P'},
        {"single-node",      no_argument,       NULL, 'q'},
        {"not-strict",       no_argument,       NULL, 'n'},
        {"type",             required_argument, NULL, 't'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
        {NULL,               0,                 NULL, 0}
    };

    uint16_t options_ctx = YL_DEFAULT_CTX_OPTIONS;
    uint8_t data_type_set = 0;

    c->data_parse_options = YL_DEFAULT_DATA_PARSE_OPTIONS;
    c->line_length = 0;

#ifndef NDEBUG
    while ((opt = getopt_long(argc, argv, "d:Def:F:hilL:myo:p:P:qnt:vV", options, &opt_index)) != -1) {
#else
    while ((opt = getopt_long(argc, argv, "d:Def:F:G:hilL:myo:p:P:qnt:vV", options, &opt_index)) != -1) {
#endif
        switch (opt) {
        case 'd': /* --default */
            if (!strcasecmp(optarg, "all")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL;
            } else if (!strcasecmp(optarg, "all-tagged")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL_TAG;
            } else if (!strcasecmp(optarg, "trim")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_TRIM;
            } else if (!strcasecmp(optarg, "implicit-tagged")) {
                c->data_print_options = (c->data_print_options & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_IMPL_TAG;
            } else {
                YLMSG_E("Unknown default mode %s\n", optarg);
                help(1);
                return -1;
            }
            break;

        case 'D': /* --disable-search */
            if (options_ctx & LY_CTX_DISABLE_SEARCHDIRS) {
                YLMSG_W("The -D option specified too many times.\n");
            }
            if (options_ctx & LY_CTX_DISABLE_SEARCHDIR_CWD) {
                options_ctx &= ~LY_CTX_DISABLE_SEARCHDIR_CWD;
                options_ctx |= LY_CTX_DISABLE_SEARCHDIRS;
            } else {
                options_ctx |= LY_CTX_DISABLE_SEARCHDIR_CWD;
            }
            break;

        case 'p': { /* --path */
            struct stat st;

            if (stat(optarg, &st) == -1) {
                YLMSG_E("Unable to use search path (%s) - %s.\n", optarg, strerror(errno));
                return -1;
            }
            if (!S_ISDIR(st.st_mode)) {
                YLMSG_E("Provided search path is not a directory.\n");
                return -1;
            }

            if (ly_set_add(&c->searchpaths, optarg, 0, NULL)) {
                YLMSG_E("Storing searchpath failed.\n");
                return -1;
            }

            break;
        } /* case 'p' */

        case 'i': /* --makeimplemented */
            if (options_ctx & LY_CTX_REF_IMPLEMENTED) {
                options_ctx &= ~LY_CTX_REF_IMPLEMENTED;
                options_ctx |= LY_CTX_ALL_IMPLEMENTED;
            } else {
                options_ctx |= LY_CTX_REF_IMPLEMENTED;
            }
            break;

        case 'F': /* --features */
            if (parse_features(optarg, &c->schema_features)) {
                return -1;
            }
            break;

        case 'l': /* --list */
            c->list = 1;
            break;

        case 'L': /* --tree-line-length */
            c->line_length = atoi(optarg);
            break;

        case 'o': /* --output */
            if (c->out) {
                YLMSG_E("Only a single output can be specified.\n");
                return -1;
            } else {
                if (ly_out_new_filepath(optarg, &c->out)) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    return -1;
                }
            }
            break;

        case 'f': /* --format */
            if (!strcasecmp(optarg, "yang")) {
                c->schema_out_format = LYS_OUT_YANG;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "yin")) {
                c->schema_out_format = LYS_OUT_YIN;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "info")) {
                c->schema_out_format = LYS_OUT_YANG_COMPILED;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "tree")) {
                c->schema_out_format = LYS_OUT_TREE;
                c->data_out_format = 0;
            } else if (!strcasecmp(optarg, "xml")) {
                c->schema_out_format = 0;
                c->data_out_format = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                c->schema_out_format = 0;
                c->data_out_format = LYD_JSON;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                help(1);
                return -1;
            }
            break;

        case 'P': /* --schema-node */
            c->schema_node_path = optarg;
            break;

        case 'q': /* --single-node */
            c->schema_print_options |= LYS_PRINT_NO_SUBSTMT;
            break;

        case 'n': /* --not-strict */
            c->data_parse_options &= ~LYD_PARSE_STRICT;
            break;

        case 'e': /* --present */
            c->data_validate_options |= LYD_VALIDATE_PRESENT;
            break;

        case 't': /* --type */
            if (data_type_set) {
                YLMSG_E("The data type (-t) cannot be set multiple times.\n");
                return -1;
            }

            if (!strcasecmp(optarg, "config")) {
                c->data_parse_options |= LYD_PARSE_NO_STATE;
                c->data_validate_options |= LYD_VALIDATE_NO_STATE;
            } else if (!strcasecmp(optarg, "get")) {
                c->data_parse_options |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "getconfig") || !strcasecmp(optarg, "get-config")) {
                c->data_parse_options |= LYD_PARSE_ONLY | LYD_PARSE_NO_STATE;
            } else if (!strcasecmp(optarg, "edit")) {
                c->data_parse_options |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "rpc") || !strcasecmp(optarg, "action")) {
                c->data_type = LYD_TYPE_RPC_YANG;
            } else if (!strcasecmp(optarg, "reply") || !strcasecmp(optarg, "rpcreply")) {
                c->data_type = LYD_TYPE_REPLY_YANG;
            } else if (!strcasecmp(optarg, "notif") || !strcasecmp(optarg, "notification")) {
                c->data_type = LYD_TYPE_NOTIF_YANG;
            } else if (!strcasecmp(optarg, "data")) {
                /* default option */
            } else {
                YLMSG_E("Unknown data tree type %s\n", optarg);
                help(1);
                return -1;
            }

            data_type_set = 1;
            break;

        case 'O': /* --operational */
            if (c->data_operational.path) {
                YLMSG_E("The operational datastore (-O) cannot be set multiple times.\n");
                return -1;
            }
            c->data_operational.path = optarg;
            break;

        case 'm': /* --merge */
            c->data_merge = 1;
            break;

        case 'y': /* --yang-library */
            options_ctx &= ~LY_CTX_NO_YANGLIBRARY;
            break;

        case 'h': /* --help */
            help(0);
            return 1;

        case 'v': /* --version */
            version();
            return 1;

        case 'V': { /* --verbose */
            LY_LOG_LEVEL verbosity = ly_log_level(LY_LLERR);
            ly_log_level(verbosity);

            if (verbosity < LY_LLDBG) {
                ly_log_level(verbosity + 1);
            }
            break;
        } /* case 'V' */

#ifndef NDEBUG
        case 'G': { /* --debug */
            uint32_t dbg_groups = 0;
            const char *ptr = optarg;

            while (ptr[0]) {
                if (!strncasecmp(ptr, "dict", sizeof "dict" - 1)) {
                    dbg_groups |= LY_LDGDICT;
                    ptr += sizeof "dict" - 1;
                } else if (!strncasecmp(ptr, "xpath", sizeof "xpath" - 1)) {
                    dbg_groups |= LY_LDGXPATH;
                    ptr += sizeof "xpath" - 1;
                }

                if (ptr[0]) {
                    if (ptr[0] != ',') {
                        YLMSG_E("Unknown debug group string \"%s\"\n", optarg);
                        return -1;
                    }
                    ++ptr;
                }
            }
            ly_log_dbg_groups(dbg_groups);
            break;
        } /* case 'G' */
#endif
        } /* switch */
    }

    /* libyang context */
    if (ly_ctx_new(NULL, options_ctx, &c->ctx)) {
        YLMSG_E("Unable to create libyang context.\n");
        return -1;
    }
    for (uint32_t u = 0; u < c->searchpaths.count; ++u) {
        ly_ctx_set_searchdir(c->ctx, c->searchpaths.objs[u]);
    }

    /* additional checks for the options combinations */
    if (!c->list && (optind >= argc)) {
        help(1);
        YLMSG_E("Missing <schema> to process.\n");
        return 1;
    }

    if (c->data_merge) {
        if (c->data_type || (c->data_parse_options & LYD_PARSE_ONLY)) {
            /* switch off the option, incompatible input data type */
            c->data_merge = 0;
        } else {
            /* postpone validation after the merge of all the input data */
            c->data_parse_options |= LYD_PARSE_ONLY;
        }
    }

    if (c->data_operational.path && !c->data_type) {
        YLMSG_E("Operational datastore takes effect only with RPCs/Actions/Replies/Notifications input data types.\n");
        c->data_operational.path = NULL;
    }

    if ((c->schema_out_format != LYS_OUT_TREE) && c->line_length) {
        YLMSG_E("--tree-line-length take effect only in case of the tree output format.\n");
    }

    /* default output stream */
    if (!c->out) {
        if (ly_out_new_file(stdout, &c->out)) {
            YLMSG_E("Unable to set stdout as output.\n");
            return -1;
        }
    }

    if (c->schema_out_format == LYS_OUT_TREE) {
        /* print tree from lysc_nodes */
        ly_ctx_set_options(c->ctx, LY_CTX_SET_PRIV_PARSED);
    }

    /* process input files provided as standalone command line arguments,
     * schema modules are parsed and inserted into the context,
     * data files are just checked and prepared into internal structures for further processing */
    ret = fill_context_inputs(argc, argv, c);
    if (ret) {
        return ret;
    }

    /* the second batch of checks */
    if (c->schema_print_options && !c->schema_out_format) {
        YLMSG_W("Schema printer options specified, but the schema output format is missing.\n");
    }
    if (c->schema_parse_options && !c->schema_modules.count) {
        YLMSG_W("Schema parser options specified, but no schema input file provided.\n");
    }
    if (c->data_print_options && !c->data_out_format) {
        YLMSG_W("data printer options specified, but the data output format is missing.\n");
    }
    if (((c->data_parse_options != YL_DEFAULT_DATA_PARSE_OPTIONS) || c->data_type) && !c->data_inputs.count) {
        YLMSG_W("Data parser options specified, but no data input file provided.\n");
    }

    if (c->schema_node_path) {
        c->schema_node = lys_find_path(c->ctx, NULL, c->schema_node_path, 0);
        if (!c->schema_node) {
            c->schema_node = lys_find_path(c->ctx, NULL, c->schema_node_path, 1);

            if (!c->schema_node) {
                YLMSG_E("Invalid schema path.\n");
                return -1;
            }
        }
    }

    return 0;
}

int
main_ni(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS, r;
    struct context c = {0};

    /* set callback for printing libyang messages */
    ly_set_log_clb(libyang_verbclb, 1);

    r = fill_context(argc, argv, &c);
    if (r < 0) {
        ret = EXIT_FAILURE;
    }
    if (r) {
        goto cleanup;
    }

    /* do the required job - parse, validate, print */

    if (c.list) {
        /* print the list of schemas */
        print_list(c.out, c.ctx, c.data_out_format);
    } else {
        if (c.schema_out_format) {
            if (c.schema_node) {
                ret = lys_print_node(c.out, c.schema_node, c.schema_out_format, 0, c.schema_print_options);
                if (ret) {
                    YLMSG_E("Unable to print schema node %s.\n", c.schema_node_path);
                    goto cleanup;
                }
            } else {
                for (uint32_t u = 0; u < c.schema_modules.count; ++u) {
                    ret = lys_print_module(c.out, (struct lys_module *)c.schema_modules.objs[u], c.schema_out_format,
                            c.line_length, c.schema_print_options);
                    /* for YANG Tree Diagrams printing it's more readable to print a blank line between modules. */
                    if ((c.schema_out_format == LYS_OUT_TREE) && (u + 1 < c.schema_modules.count)) {
                        ly_print(c.out, "\n");
                    }
                    if (ret) {
                        YLMSG_E("Unable to print module %s.\n", ((struct lys_module *)c.schema_modules.objs[u])->name);
                        goto cleanup;
                    }
                }
            }
        }

        /* do the data validation despite the schema was printed */
        if (c.data_inputs.size) {
            if (process_data(c.ctx, c.data_type, c.data_merge, c.data_out_format, c.out,
                    c.data_parse_options, c.data_validate_options, c.data_print_options,
                    &c.data_operational, &c.data_inputs, NULL)) {
                goto cleanup;
            }
        }
    }

cleanup:
    /* cleanup */
    erase_context(&c);

    return ret;
}
