/**
 * @file main_ni.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang's yanglint tool - noninteractive code
 *
 * Copyright (c) 2015-2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "libyang.h"

volatile uint8_t verbose = 0;

/* from commands.c */
int print_list(FILE *out, struct ly_ctx *ctx, LYD_FORMAT outformat);

void
help(int shortout)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "    yanglint [options] [-f { yang | yin | tree | tree-rfc | jsons}] <file>...\n");
    fprintf(stdout, "        Validates the YANG module in <file>, and all its dependencies.\n\n");
    fprintf(stdout, "    yanglint [options] [-f { xml | json }] <schema>... <file>...\n");
    fprintf(stdout, "        Validates the YANG modeled data in <file> according to the <schema>.\n\n");
    fprintf(stdout, "    yanglint\n");
    fprintf(stdout, "        Starts interactive mode with more features.\n\n");

    if (shortout) {
        return;
    }
    fprintf(stdout, "Options:\n"
        "  -h, --help            Show this help message and exit.\n"
        "  -v, --version         Show version number and exit.\n"
        "  -V, --verbose         Show verbose messages, can be used multiple times to\n"
        "                        increase verbosity.\n"
#ifndef NDEBUG
        "  -G GROUPS, --debug=GROUPS\n"
        "                        Enable printing of specific debugging message group\n"
        "                        (nothing will be printed unless verbosity is set to debug):\n"
        "                        <group>[,<group>]* (dict, yang, yin, xpath, diff)\n\n"
#endif
        "  -p PATH, --path=PATH  Search path for schema (YANG/YIN) modules. The option can be used multiple times.\n"
        "                        Current working directory and path of the module being added is used implicitly.\n\n"
        "  -D, --disable-searchdir\n"
        "                        Do not implicitly search in CWD for schema modules. If specified a second time,\n"
        "                        do not even search the module directory (all modules must be explicitly specified).\n\n"
        "  -s, --strict          Strict data parsing (do not skip unknown data),\n"
        "                        has no effect for schemas.\n\n"
        "  -m, --merge           Merge input data files into a single tree and validate at once,\n"
        "                        has no effect for the auto, rpc, rpcreply and notif TYPEs.\n\n"
        "  -f FORMAT, --format=FORMAT\n"
        "                        Convert to FORMAT. Supported formats: \n"
        "                        yang, yin, tree, tree-rfc and jsons (JSON) for schemas,\n"
        "                        xml, json for data.\n"
        "  -a, --auto            Modify the xml output by adding envelopes for autodetection.\n\n"
        "  -i, --allimplemented  Make all the imported modules implemented.\n\n"
        "  -l, --list            Print info about the loaded schemas in ietf-yang-library format,\n"
        "                        the -f option applies here to specify data encoding.\n"
        "                        (i - imported module, I - implemented module)\n\n"
        "  -o OUTFILE, --output=OUTFILE\n"
        "                        Write the output to OUTFILE instead of stdout.\n\n"
        "  -F FEATURES, --features=FEATURES\n"
        "                        Features to support, default all.\n"
        "                        <modname>:[<feature>,]*\n\n"
        "  -d MODE, --default=MODE\n"
        "                        Print data with default values, according to the MODE\n"
        "                        (to print attributes, ietf-netconf-with-defaults model\n"
        "                        must be loaded):\n"
        "        all             - Add missing default nodes.\n"
        "        all-tagged      - Add missing default nodes and mark all the default\n"
        "                          nodes with the attribute.\n"
        "        trim            - Remove all nodes with a default value.\n"
        "        implicit-tagged - Add missing nodes and mark them with the attribute.\n\n"
        "  -t TYPE, --type=TYPE\n"
        "                        Specify data tree type in the input data file:\n"
        "        auto            - Resolve data type (one of the following) automatically\n"
        "                          (as pyang does) - applicable only on XML input data.\n"
        "        data            - Complete datastore with status data (default type).\n"
        "        config          - Configuration datastore (without status data).\n"
        "        get             - Result of the NETCONF <get> operation.\n"
        "        getconfig       - Result of the NETCONF <get-config> operation.\n"
        "        edit            - Content of the NETCONF <edit-config> operation.\n"
        "        rpc             - Content of the NETCONF <rpc> message, defined as YANG's rpc input statement.\n"
        "        rpcreply        - Reply to the RPC. The input data <file>s are expected in pairs - each RPC reply\n"
        "                          input data <file> must be followed by the origin RPC input data <file> for the reply.\n"
        "                          The same rule of pairing applies also in case of 'auto' TYPE and input data file\n"
        "                          containing RPC reply.\n"
        "        notif           - Notification instance (content of the <notification> element without <eventTime>.\n\n"
        "  -O FILE, --operational=FILE\n"
        "                        - Optional parameter for 'rpc', 'rpcreply' and 'notif' TYPEs, the FILE contains running\n"
        "                          configuration datastore and state data (operational datastore) referenced from\n"
        "                          the RPC/Notification. The same data apply to all input data <file>s. Note that\n"
        "                          the file is validated as 'data' TYPE. Special value '!' can be used as FILE argument\n"
        "                          to ignore the external references.\n\n"
        "  -y YANGLIB_PATH       - Path to a yang-library data describing the initial context.\n\n"
        "Tree output specific options:\n"
        "  --tree-help           - Print help on tree symbols and exit.\n"
        "  --tree-print-groupings\n"
        "                        Print top-level groupings in a separate section.\n"
        "  --tree-print-uses     - Print uses nodes instead the resolved grouping nodes.\n"
        "  --tree-no-leafref-target\n"
        "                        Do not print target nodes of leafrefs.\n"
        "  --tree-path=SCHEMA_PATH\n"
        "                        Print only the specified subtree.\n"
        "  --tree-line-length=LINE_LENGTH\n"
        "                        Wrap lines if longer than the specified length (it is not a strict limit, longer lines\n"
        "                        can often appear).\n"
        "\n");
}

void
tree_help(void)
{
    fprintf(stdout, "Each node is printed as:\n\n");
    fprintf(stdout, "<status> <flags> <name> <opts> <type> <if-features>\n\n"
                    "  <status> is one of:\n"
                    "    + for current\n"
                    "    x for deprecated\n"
                    "    o for obsolete\n\n"
                    "  <flags> is one of:\n"
                    "    rw for configuration data\n"
                    "    ro for status data\n"
                    "    -x for RPCs\n"
                    "    -n for Notification\n\n"
                    "  <name> is the name of the node\n"
                    "    (<name>) means that the node is a choice node\n"
                    "    :(<name>) means that the node is a case node\n\n"
                    "    if the node is augmented into the tree from another module,\n"
                    "    it is printed with the module name as <module-name>:<name>.\n\n"
                    "  <opts> is one of:\n"
                    "    ? for an optional leaf or choice\n"
                    "    ! for a presence container\n"
                    "    * for a leaf-list or list\n"
                    "    [<keys>] for a list's keys\n\n"
                    "  <type> is the name of the type for leafs and leaf-lists\n"
                    "    If there is a default value defined, it is printed within\n"
                    "    angle brackets <default-value>.\n"
                    "    If the type is a leafref, the type is printed as -> TARGET`\n\n"
                    "  <if-features> is the list of features this node depends on,\n"
                    "    printed within curly brackets and a question mark {...}?\n\n");
}

void
version(void)
{
    fprintf(stdout, "yanglint SO %d.%d.%d\n", LY_VERSION_MAJOR, LY_VERSION_MINOR, LY_VERSION_MICRO);
}
void
libyang_verbclb(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    char *levstr;

    if (level <= verbose) {
        switch(level) {
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
            fprintf(stderr, "%s %s (%s)\n", levstr, msg, path);
        } else {
            fprintf(stderr, "%s %s\n", levstr, msg);
        }
    }
}

/*
 * return:
 * 0 - error
 * 1 - schema format
 * 2 - data format
 */
static int
get_fileformat(const char *filename, LYS_INFORMAT *schema, LYD_FORMAT *data)
{
    char *ptr;
    LYS_INFORMAT informat_s;
    LYD_FORMAT informat_d;

    /* get the file format */
    if ((ptr = strrchr(filename, '.')) != NULL) {
        ++ptr;
        if (!strcmp(ptr, "yin")) {
            informat_s = LYS_IN_YIN;
            informat_d = 0;
        } else if (!strcmp(ptr, "yang")) {
            informat_s = LYS_IN_YANG;
            informat_d = 0;
        } else if (!strcmp(ptr, "xml")) {
            informat_s = 0;
            informat_d = LYD_XML;
        } else if (!strcmp(ptr, "json")) {
            informat_s = 0;
            informat_d = LYD_JSON;
        } else {
            fprintf(stderr, "yanglint error: input file in an unknown format \"%s\".\n", ptr);
            return 0;
        }
    } else {
        fprintf(stderr, "yanglint error: input file \"%s\" without file extension - unknown format.\n", filename);
        return 0;
    }

    if (data) {
        (*data) = informat_d;
    }
    if (schema) {
        (*schema) = informat_s;
    }

    if (informat_s) {
        return 1;
    } else {
        return 2;
    }
}

int
main_ni(int argc, char* argv[])
{
    int ret = EXIT_FAILURE;
    int opt, opt_index = 0, i, featsize = 0;
    struct option options[] = {
        {"auto",             no_argument,       NULL, 'a'},
        {"default",          required_argument, NULL, 'd'},
        {"format",           required_argument, NULL, 'f'},
        {"features",         required_argument, NULL, 'F'},
        {"tree-print-groupings", no_argument,   NULL, 'g'},
        {"tree-print-uses",  no_argument,       NULL, 'u'},
        {"tree-no-leafref-target", no_argument, NULL, 'n'},
        {"tree-path",        required_argument, NULL, 'P'},
        {"tree-line-length", required_argument, NULL, 'L'},
        {"help",             no_argument,       NULL, 'h'},
        {"tree-help",        no_argument,       NULL, 'H'},
        {"allimplemented",   no_argument,       NULL, 'i'},
        {"disable-cwd-search", no_argument,     NULL, 'D'},
        {"list",             no_argument,       NULL, 'l'},
        {"merge",            no_argument,       NULL, 'm'},
        {"output",           required_argument, NULL, 'o'},
        {"path",             required_argument, NULL, 'p'},
        {"running",          required_argument, NULL, 'r'},
        {"operational",      required_argument, NULL, 'O'},
        {"strict",           no_argument,       NULL, 's'},
        {"type",             required_argument, NULL, 't'},
        {"version",          no_argument,       NULL, 'v'},
        {"verbose",          no_argument,       NULL, 'V'},
#ifndef NDEBUG
        {"debug",            required_argument, NULL, 'G'},
#endif
        {NULL,               required_argument, NULL, 'y'},
        {NULL,               0,                 NULL, 0}
    };
    FILE *out = stdout;
    struct ly_ctx *ctx = NULL;
    const struct lys_module *mod;
    LYS_OUTFORMAT outformat_s = 0;
    LYS_INFORMAT informat_s;
    LYD_FORMAT informat_d, outformat_d = 0, ylformat = 0;
    struct ly_set *searchpaths = NULL;
    const char *oper_file = NULL, *envelope_s = NULL, *outtarget_s = NULL;
    char **feat = NULL, *ptr, *featlist, *ylpath = NULL, *dir;
    struct stat st;
    uint32_t u;
    int options_dflt = 0, options_parser = 0, options_ctx = LY_CTX_NOYANGLIBRARY, envelope = 0, autodetection = 0;
    int merge = 0, list = 0, outoptions_s = 0, outline_length_s = 0;
    struct dataitem {
        const char *filename;
        struct lyxml_elem *xml;
        struct lyd_node *tree;
        struct dataitem *next;
        LYD_FORMAT format;
        int type;
    } *data = NULL, *data_item, *data_prev = NULL;
    struct ly_set *mods = NULL;
    struct lyd_node *oper = NULL, *subroot, *next, *node;
    void *p;
    int index = 0;
    struct lyxml_elem *iter, *elem;

    opterr = 0;
#ifndef NDEBUG
    while ((opt = getopt_long(argc, argv, "ad:f:F:gunP:L:hHiDlmo:p:r:O:st:vVG:y:", options, &opt_index)) != -1)
#else
    while ((opt = getopt_long(argc, argv, "ad:f:F:gunP:L:hHiDlmo:p:r:O:st:vVy:", options, &opt_index)) != -1)
#endif
    {
        switch (opt) {
        case 'a':
            envelope = 1;
            break;
        case 'd':
            if (!strcmp(optarg, "all")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_ALL;
            } else if (!strcmp(optarg, "all-tagged")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_ALL_TAG;
            } else if (!strcmp(optarg, "trim")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_TRIM;
            } else if (!strcmp(optarg, "implicit-tagged")) {
                options_dflt = (options_dflt & ~LYP_WD_MASK) | LYP_WD_IMPL_TAG;
            } else {
                fprintf(stderr, "yanglint error: unknown default mode %s\n", optarg);
                help(1);
                goto cleanup;
            }
            break;
        case 'f':
            if (!strcasecmp(optarg, "tree")) {
                outformat_s = LYS_OUT_TREE;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "tree-rfc")) {
                outformat_s = LYS_OUT_TREE;
                outoptions_s |= LYS_OUTOPT_TREE_RFC;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "yin")) {
                outformat_s = LYS_OUT_YIN;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "yang")) {
                outformat_s = LYS_OUT_YANG;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "jsons")) {
                outformat_s = LYS_OUT_JSON;
                outformat_d = 0;
            } else if (!strcasecmp(optarg, "xml")) {
                outformat_s = 0;
                outformat_d = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                outformat_s = 0;
                outformat_d = LYD_JSON;
            } else {
                fprintf(stderr, "yanglint error: unknown output format %s\n", optarg);
                help(1);
                goto cleanup;
            }
            break;
        case 'F':
            featsize++;
            if (!feat) {
                p = malloc(sizeof *feat);
            } else {
                p = realloc(feat, featsize * sizeof *feat);
            }
            if (!p) {
                fprintf(stderr, "yanglint error: Memory allocation failed (%s:%d, %s)", __FILE__, __LINE__, strerror(errno));
                goto cleanup;
            }
            feat = p;
            feat[featsize - 1] = strdup(optarg);
            ptr = strchr(feat[featsize - 1], ':');
            if (!ptr) {
                fprintf(stderr, "yanglint error: Invalid format of the features specification (%s)", optarg);
                goto cleanup;
            }
            *ptr = '\0';

            break;
        case 'g':
            outoptions_s |= LYS_OUTOPT_TREE_GROUPING;
            break;
        case 'u':
            outoptions_s |= LYS_OUTOPT_TREE_USES;
            break;
        case 'n':
            outoptions_s |= LYS_OUTOPT_TREE_NO_LEAFREF;
            break;
        case 'P':
            outtarget_s = optarg;
            break;
        case 'L':
            outline_length_s = atoi(optarg);
            break;
        case 'h':
            help(0);
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'H':
            tree_help();
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'i':
            options_ctx |= LY_CTX_ALLIMPLEMENTED;
            break;
        case 'D':
            if (options_ctx & LY_CTX_DISABLE_SEARCHDIRS) {
                fprintf(stderr, "yanglint error: -D specified too many times.\n");
                goto cleanup;
            } else if (options_ctx & LY_CTX_DISABLE_SEARCHDIR_CWD) {
                options_ctx &= ~LY_CTX_DISABLE_SEARCHDIR_CWD;
                options_ctx |= LY_CTX_DISABLE_SEARCHDIRS;
            } else {
                options_ctx |= LY_CTX_DISABLE_SEARCHDIR_CWD;
            }
            break;
        case 'l':
            list = 1;
            break;
        case 'm':
            merge = 1;
            break;
        case 'o':
            if (out != stdout) {
                fclose(out);
            }
            out = fopen(optarg, "w");
            if (!out) {
                fprintf(stderr, "yanglint error: unable open output file %s (%s)\n", optarg, strerror(errno));
                goto cleanup;
            }
            break;
        case 'p':
            if (stat(optarg, &st) == -1) {
                fprintf(stderr, "yanglint error: Unable to use search path (%s) - %s.\n", optarg, strerror(errno));
                goto cleanup;
            }
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "yanglint error: Provided search path is not a directory.\n");
                goto cleanup;
            }
            if (!searchpaths) {
                searchpaths = ly_set_new();
            }
            ly_set_add(searchpaths, optarg, 0);
            break;
        case 'r':
        case 'O':
            if (oper_file || (options_parser & LYD_OPT_NOEXTDEPS)) {
                fprintf(stderr, "yanglint error: The operational datastore (-O) cannot be set multiple times.\n");
                goto cleanup;
            }
            if (optarg[0] == '!') {
                /* ignore extenral dependencies to the operational datastore */
                options_parser |= LYD_OPT_NOEXTDEPS;
            } else {
                /* external file with the operational datastore */
                oper_file = optarg;
            }
            break;
        case 's':
            options_parser |= LYD_OPT_STRICT;
            break;
        case 't':
            if (!strcmp(optarg, "auto")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK);
                autodetection = 1;
            } else if (!strcmp(optarg, "config")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
            } else if (!strcmp(optarg, "get")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
            } else if (!strcmp(optarg, "getconfig")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
            } else if (!strcmp(optarg, "edit")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
            } else if (!strcmp(optarg, "data")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_NO_YANGLIB;
            } else if (!strcmp(optarg, "rpc")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
            } else if (!strcmp(optarg, "rpcreply")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY;
            } else if (!strcmp(optarg, "notif")) {
                options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
            } else {
                fprintf(stderr, "yanglint error: unknown data tree type %s\n", optarg);
                help(1);
                goto cleanup;
            }
            break;
        case 'v':
            version();
            ret = EXIT_SUCCESS;
            goto cleanup;
        case 'V':
            verbose++;
            break;
#ifndef NDEBUG
        case 'G':
            u = 0;
            ptr = optarg;
            while (ptr[0]) {
                if (!strncmp(ptr, "dict", 4)) {
                    u |= LY_LDGDICT;
                    ptr += 4;
                } else if (!strncmp(ptr, "yang", 4)) {
                    u |= LY_LDGYANG;
                    ptr += 4;
                } else if (!strncmp(ptr, "yin", 3)) {
                    u |= LY_LDGYIN;
                    ptr += 3;
                } else if (!strncmp(ptr, "xpath", 5)) {
                    u |= LY_LDGXPATH;
                    ptr += 5;
                } else if (!strncmp(ptr, "diff", 4)) {
                    u |= LY_LDGDIFF;
                    ptr += 4;
                }

                if (ptr[0]) {
                    if (ptr[0] != ',') {
                        fprintf(stderr, "yanglint error: unknown debug group string \"%s\"\n", optarg);
                        goto cleanup;
                    }
                    ++ptr;
                }
            }
            ly_verb_dbg(u);
            break;
#endif
        case 'y':
            ptr = strrchr(optarg, '.');
            if (ptr) {
                ptr++;
                if (!strcmp(ptr, "xml")) {
                    ylformat = LYD_XML;
                } else if (!strcmp(ptr, "json")) {
                    ylformat = LYD_JSON;
                } else {
                    fprintf(stderr, "yanglint error: yang-library file in an unknown format \"%s\".\n", ptr);
                    goto cleanup;
                }
            } else {
                fprintf(stderr, "yanglint error: yang-library file in an unknown format.\n");
                goto cleanup;
            }
            ylpath = optarg;

            /* use internal ietf-yang-library schema */
            options_ctx &= ~LY_CTX_NOYANGLIBRARY;
            break;
        default:
            help(1);
            if (optopt) {
                fprintf(stderr, "yanglint error: invalid option: -%c\n", optopt);
            } else {
                fprintf(stderr, "yanglint error: invalid option: %s\n", argv[optind - 1]);
            }
            goto cleanup;
        }
    }

    /* check options compatibility */
    if (!list && optind >= argc) {
        help(1);
        fprintf(stderr, "yanglint error: missing <file> to process\n");
        goto cleanup;
    }
    if (outformat_s && outformat_s != LYS_OUT_TREE && (optind + 1) < argc) {
        /* we have multiple schemas to be printed as YIN or YANG */
        fprintf(stderr, "yanglint error: too many schemas to convert and store.\n");
        goto cleanup;
    }
    if (outoptions_s || outtarget_s || outline_length_s) {
        if (outformat_d || (outformat_s && outformat_s != LYS_OUT_TREE)) {
            /* we have --tree-print-grouping with other output format than tree */
            fprintf(stderr,
                    "yanglint warning: --tree options take effect only in case of the tree output format.\n");
        }
    }
    if (merge) {
        if (autodetection || (options_parser & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF))) {
            fprintf(stderr, "yanglint warning: merging not allowed, ignoring option -m.\n");
            merge = 0;
        } else {
            /* first, files will be parsed as trusted to allow missing data, then the data trees will be merged
             * and the result will be validated */
            options_parser |= LYD_OPT_TRUSTED;
        }
    }
    if (!outformat_d && options_dflt) {
        /* we have options for printing default nodes, but data output not specified */
        fprintf(stderr, "yanglint warning: default mode is ignored when not printing data.\n");
    }
    if (outformat_s && (options_parser || autodetection)) {
        /* we have options for printing data tree, but output is schema */
        fprintf(stderr, "yanglint warning: data parser options are ignored when printing schema.\n");
    }
    if (oper_file && (!autodetection && !(options_parser & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)))) {
        fprintf(stderr, "yanglint warning: operational datastore applies only to RPCs or Notifications.\n");
        /* ignore operational datastore file */
        oper_file = NULL;
    }
    if ((options_parser & LYD_OPT_TYPEMASK) == LYD_OPT_DATA) {
        /* add option to ignore ietf-yang-library data for implicit data type */
        options_parser |= LYD_OPT_DATA_NO_YANGLIB;
    }

    /* set callback for printing libyang messages */
    ly_set_log_clb(libyang_verbclb, 1);

    /* create libyang context */
    if (ylpath) {
        ctx = ly_ctx_new_ylpath(searchpaths ? (const char*)searchpaths->set.g[0] : NULL, ylpath, ylformat, options_ctx);
    } else {
        ctx = ly_ctx_new(NULL, options_ctx);
    }
    if (!ctx) {
        goto cleanup;
    }

    /* set searchpaths */
    if (searchpaths) {
        for (u = 0; u < searchpaths->number; u++) {
            ly_ctx_set_searchdir(ctx, (const char*)searchpaths->set.g[u]);
        }
        index = u + 1;
    }

    /* derefered setting of verbosity in libyang after context initiation */
    ly_verb(verbose);

    mods = ly_set_new();


    /* divide input files */
    for (i = 0; i < argc - optind; i++) {
        /* get the file format */
        if (!get_fileformat(argv[optind + i], &informat_s, &informat_d)) {
            goto cleanup;
        }

        if (informat_s) {
            /* load/validate schema */
            if (verbose >= 2) {
                fprintf(stdout, "Validating %s schema file.\n", argv[optind + i]);
            }
            dir = strdup(argv[optind + i]);
            ly_ctx_set_searchdir(ctx, dirname(dir));
            mod = lys_parse_path(ctx, argv[optind + i], informat_s);
            ly_ctx_unset_searchdirs(ctx, index);
            free(dir);
            if (!mod) {
                goto cleanup;
            }
            ly_set_add(mods, (void *)mod, 0);
        } else {
            if (autodetection && informat_d != LYD_XML) {
                /* data file content autodetection is possible only for XML input */
                fprintf(stderr, "yanglint error: data type autodetection is applicable only to XML files.\n");
                goto cleanup;
            }

            /* remember data filename and its format */
            if (!data) {
                data = data_item = malloc(sizeof *data);
            } else {
                for (data_item = data; data_item->next; data_item = data_item->next);
                data_item->next = malloc(sizeof *data_item);
                data_item = data_item->next;
            }
            data_item->filename = argv[optind + i];
            data_item->format = informat_d;
            data_item->type = options_parser & LYD_OPT_TYPEMASK;
            data_item->tree = NULL;
            data_item->xml = NULL;
            data_item->next = NULL;
        }
    }

    if (outformat_d && !data && !list) {
        fprintf(stderr, "yanglint error: no input data file for the specified data output format.\n");
        goto cleanup;
    }

    /* enable specified features, if not specified, all the module's features are enabled */
    u = 4; /* skip internal libyang modules */
    while ((mod = ly_ctx_get_module_iter(ctx, &u))) {
        for (i = 0; i < featsize; i++) {
            if (!strcmp(feat[i], mod->name)) {
                /* parse features spec */
                featlist = strdup(feat[i] + strlen(feat[i]) + 1);
                ptr = NULL;
                while((ptr = strtok(ptr ? NULL : featlist, ","))) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Enabling feature %s in module %s.\n", ptr, mod->name);
                    }
                    if (lys_features_enable(mod, ptr)) {
                        fprintf(stderr, "Feature %s not defined in module %s.\n", ptr, mod->name);
                    }
                }
                free(featlist);
                break;
            }
        }
        if (i == featsize) {
            if (verbose >= 2) {
                fprintf(stdout, "Enabling all features in module %s.\n", mod->name);
            }
            lys_features_enable(mod, "*");
        }
    }

    /* convert (print) to FORMAT */
    if (outformat_s) {
        if (outformat_s == LYS_OUT_JSON && mods->number > 1) {
            fputs("[", out);
        }
        for (u = 0; u < mods->number; u++) {
            if (u) {
                if (outformat_s == LYS_OUT_JSON) {
                    fputs(",\n", out);
                } else {
                    fputs("\n", out);
                }
            }
            lys_print_file(out, (struct lys_module *)mods->set.g[u], outformat_s, outtarget_s, outline_length_s, outoptions_s);
        }
        if (outformat_s == LYS_OUT_JSON) {
            if (mods->number > 1) {
                fputs("]\n", out);
            } else if (mods->number == 1) {
                fputs("\n", out);
            }
        }
    } else if (data) {
        ly_errno = 0;

        /* prepare operational datastore when specified for RPC/Notification */
        if (oper_file) {
            /* get the file format */
            if (!get_fileformat(oper_file, NULL, &informat_d)) {
                goto cleanup;
            } else if (!informat_d) {
                fprintf(stderr, "yanglint error: The operational data are expected in XML or JSON format.\n");
                goto cleanup;
            }
            oper = lyd_parse_path(ctx, oper_file, informat_d, LYD_OPT_DATA_NO_YANGLIB | LYD_OPT_TRUSTED);
            if (!oper) {
                fprintf(stderr, "yanglint error: Failed to parse the operational datastore file for RPC/Notification validation.\n");
                goto cleanup;
            }
        }

        for (data_item = data, data_prev = NULL; data_item; data_prev = data_item, data_item = data_item->next) {
            /* parse data file - via LYD_OPT_TRUSTED postpone validation when all data are loaded and merged */
            if (autodetection) {
                /* erase option not covered by LYD_OPT_TYPEMASK, but used according to the type */
                options_parser &= ~LYD_OPT_DATA_NO_YANGLIB;
                /* automatically detect data type from the data top level */
                data_item->xml = lyxml_parse_path(ctx, data_item->filename, 0);
                if (!data_item->xml) {
                    fprintf(stderr, "yanglint error: parsing XML data for data type autodetection failed.\n");
                    goto cleanup;
                }

                /* NOTE: namespace is ignored to simplify usage of this feature */
                if (!strcmp(data_item->xml->name, "data")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as complete datastore.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_DATA_NO_YANGLIB;
                    data_item->type = LYD_OPT_DATA;
                } else if (!strcmp(data_item->xml->name, "config")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as config data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_CONFIG;
                    data_item->type = LYD_OPT_CONFIG;
                } else if (!strcmp(data_item->xml->name, "get-reply")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <get> reply data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GET;
                    data_item->type = LYD_OPT_GET;
                } else if (!strcmp(data_item->xml->name, "get-config-reply")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <get-config> reply data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_GETCONFIG;
                    data_item->type = LYD_OPT_GETCONFIG;
                } else if (!strcmp(data_item->xml->name, "edit-config")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <edit-config> data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_EDIT;
                    data_item->type = LYD_OPT_EDIT;
                } else if (!strcmp(data_item->xml->name, "rpc")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <rpc> data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC;
                    data_item->type = LYD_OPT_RPC;
                } else if (!strcmp(data_item->xml->name, "rpc-reply")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <rpc-reply> data.\n", data_item->filename);
                    }

                    data_item->type = LYD_OPT_RPCREPLY;
                    if (!data_item->next || (data_prev && !data_prev->tree)) {
                        fprintf(stderr, "RPC reply (%s) must be paired with the original RPC, see help.\n", data_item->filename);
                        goto cleanup;
                    }

                    continue;
                } else if (!strcmp(data_item->xml->name, "notification")) {
                    if (verbose >= 2) {
                        fprintf(stdout, "Parsing %s as <notification> data.\n", data_item->filename);
                    }
                    options_parser = (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_NOTIF;
                    data_item->type = LYD_OPT_NOTIF;

                    /* ignore eventTime element if present */
                    while (data_item->xml->child && !strcmp(data_item->xml->child->name, "eventTime")) {
                        lyxml_free(ctx, data_item->xml->child);
                    }
                } else {
                    fprintf(stderr, "yanglint error: invalid top-level element \"%s\" for data type autodetection.\n",
                            data_item->xml->name);
                    goto cleanup;
                }

                data_item->tree = lyd_parse_xml(ctx, &data_item->xml->child, options_parser, oper);
                if (data_prev && data_prev->type == LYD_OPT_RPCREPLY) {
parse_reply:
                    /* check result of the RPC parsing, we are going to do another parsing in this step */
                    if (ly_errno) {
                        goto cleanup;
                    }

                    /* check that we really have RPC for the reply */
                    if (data_item->type != LYD_OPT_RPC) {
                        fprintf(stderr, "yanglint error: RPC reply (%s) must be paired with the original RPC, see help.\n", data_prev->filename);
                        goto cleanup;
                    }

                    if (data_prev->format == LYD_XML) {
                        /* ignore <ok> and <rpc-error> elements if present */
                        u = 0;
                        LY_TREE_FOR_SAFE(data_prev->xml->child, iter, elem) {
                            if (!strcmp(data_prev->xml->child->name, "ok")) {
                                if (u) {
                                    /* rpc-error or ok already present */
                                    u = 0x8; /* error flag */
                                } else {
                                    u = 0x1 | 0x4; /* <ok> flag with lyxml_free() flag */
                                }
                            } else if (!strcmp(data_prev->xml->child->name, "rpc-error")) {
                                if (u && (u & 0x1)) {
                                    /* ok already present, rpc-error can be present multiple times */
                                    u = 0x8; /* error flag */
                                } else {
                                    u = 0x2 | 0x4; /* <rpc-error> flag with lyxml_free() flag */
                                }
                            }

                            if (u == 0x8) {
                                fprintf(stderr, "yanglint error: Invalid RPC reply (%s) content.\n", data_prev->filename);
                                goto cleanup;
                            } else if (u & 0x4) {
                                lyxml_free(ctx, data_prev->xml->child);
                                u &= ~0x4; /* unset lyxml_free() flag */
                            }
                        }

                        /* finally, parse RPC reply from the previous step */
                        data_prev->tree = lyd_parse_xml(ctx, &data_prev->xml->child,
                                                        (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY, data_item->tree, oper);
                    } else { /* LYD_JSON */
                        data_prev->tree = lyd_parse_path(ctx, data_prev->filename, data_item->format,
                                                         (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPCREPLY, data_item->tree, oper);
                    }
                }
            } else if ((options_parser & LYD_OPT_TYPEMASK) == LYD_OPT_RPCREPLY) {
                if (data_prev && !data_prev->tree) {
                    /* now we should have RPC for the preceding RPC reply */
                    data_item->tree = lyd_parse_path(ctx, data_item->filename, data_item->format,
                                                     (options_parser & ~LYD_OPT_TYPEMASK) | LYD_OPT_RPC, oper);
                    data_item->type = LYD_OPT_RPC;
                    goto parse_reply;
                } else {
                    /* now we have RPC reply which will be parsed in next step together with its RPC */
                    if (!data_item->next) {
                        fprintf(stderr, "yanglint error: RPC reply (%s) must be paired with the original RPC, see help.\n", data_item->filename);
                        goto cleanup;
                    }
                    if (data_item->format == LYD_XML) {
                        /* create rpc-reply container to unify handling with autodetection */
                        data_item->xml = calloc(1, sizeof *data_item->xml);
                        if (!data_item->xml) {
                            fprintf(stderr, "yanglint error: Memory allocation failed failed.\n");
                            goto cleanup;
                        }
                        data_item->xml->name = lydict_insert(ctx, "rpc-reply", 9);
                        data_item->xml->prev = data_item->xml;
                        data_item->xml->child = lyxml_parse_path(ctx, data_item->filename, LYXML_PARSE_MULTIROOT | LYXML_PARSE_NOMIXEDCONTENT);
                        if (data_item->xml->child) {
                            data_item->xml->child->parent = data_item->xml;
                        }
                    }
                    continue;
                }
            } else {
                data_item->tree = lyd_parse_path(ctx, data_item->filename, data_item->format, options_parser, oper);
            }
            if (ly_errno) {
                goto cleanup;
            }

            if (merge && data != data_item) {
                if (!data->tree) {
                    data->tree = data_item->tree;
                } else if (data_item->tree) {
                    /* merge results */
                    if (lyd_merge(data->tree, data_item->tree, LYD_OPT_DESTRUCT | LYD_OPT_EXPLICIT)) {
                        fprintf(stderr, "yanglint error: merging multiple data trees failed.\n");
                        goto cleanup;
                    }
                }
                data_item->tree = NULL;
            }
        }

        if (merge) {
            /* validate the merged data tree, do not trust the input, invalidate all the data first */
            LY_TREE_FOR(data->tree, subroot) {
                LY_TREE_DFS_BEGIN(subroot, next, node) {
                    node->validity = LYD_VAL_OK;
                    switch (node->schema->nodetype) {
                    case LYS_LIST:
                        node->validity |= LYD_VAL_UNIQUE;
                        /* falls through */
                    case LYS_CONTAINER:
                    case LYS_NOTIF:
                    case LYS_RPC:
                    case LYS_ACTION:
                        node->validity |= LYD_VAL_MAND;
                        break;
                    default:
                        break;
                    }
                    LY_TREE_DFS_END(subroot, next, node)
                }
            }
            if (lyd_validate(&data->tree, options_parser & ~LYD_OPT_TRUSTED, ctx)) {
                goto cleanup;
            }
        }

        /* print only if data output format specified */
        if (outformat_d) {
            for (data_item = data; data_item; data_item = data_item->next) {
                if (!merge && verbose >= 2) {
                    fprintf(stdout, "File %s:\n", data_item->filename);
                }
                if (outformat_d == LYD_XML && envelope) {
                    switch (data_item->type) {
                    case LYD_OPT_DATA:
                        envelope_s = "data";
                        break;
                    case LYD_OPT_CONFIG:
                        envelope_s = "config";
                        break;
                    case LYD_OPT_GET:
                        envelope_s = "get-reply";
                        break;
                    case LYD_OPT_GETCONFIG:
                        envelope_s = "get-config-reply";
                        break;
                    case LYD_OPT_EDIT:
                        envelope_s = "edit-config";
                        break;
                    case LYD_OPT_RPC:
                        envelope_s = "rpc";
                        break;
                    case LYD_OPT_RPCREPLY:
                        envelope_s = "rpc-reply";
                        break;
                    case LYD_OPT_NOTIF:
                        envelope_s = "notification";
                        break;
                    }
                    fprintf(out, "<%s>\n", envelope_s);
                    if (data_item->type == LYD_OPT_RPC && data_item->tree->schema->nodetype != LYS_RPC) {
                        /* action */
                        fprintf(out, "<action xmlns=\"urn:ietf:params:xml:ns:yang:1\">\n");
                    }
                }
                lyd_print_file(out, (data_item->type == LYD_OPT_RPCREPLY) ? data_item->tree->child : data_item->tree,
                               outformat_d, LYP_WITHSIBLINGS | LYP_FORMAT | options_dflt);
                if (envelope_s) {
                    if (data_item->type == LYD_OPT_RPC && data_item->tree->schema->nodetype != LYS_RPC) {
                        fprintf(out, "</action>\n");
                    }
                    fprintf(out, "</%s>\n", envelope_s);
                }
                if (merge) {
                    /* stop after first item */
                    break;
                }
            }
        }
    }

    if (list) {
        print_list(out, ctx, outformat_d);
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (out && out != stdout) {
        fclose(out);
    }
    ly_set_free(mods);
    ly_set_free(searchpaths);
    for (i = 0; i < featsize; i++) {
        free(feat[i]);
    }
    free(feat);
    for (; data; data = data_item) {
        data_item = data->next;
        lyxml_free(ctx, data->xml);
        lyd_free_withsiblings(data->tree);
        free(data);
    }
    lyd_free_withsiblings(oper);
    ly_ctx_destroy(ctx, NULL);

    return ret;
}
