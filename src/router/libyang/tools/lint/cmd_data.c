/**
 * @file cmd_data.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief 'data' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2015-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "cmd.h"

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"

void
cmd_data_help(void)
{
    printf("Usage: data [-emn] [-t TYPE]\n"
            "            [-F FORMAT] [-f FORMAT] [-d DEFAULTS] [-o OUTFILE] <data1> ...\n"
            "       data [-n] -t (rpc | notif | reply) [-O FILE]\n"
            "            [-F FORMAT] [-f FORMAT] [-d DEFAULTS] [-o OUTFILE] <data1> ...\n"
            "       data [-en] [-t TYPE] [-F FORMAT] -x XPATH [-o OUTFILE] <data1> ...\n"
            "                  Parse, validate and optionally print data instances\n\n"

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
            "                        element without <eventTime>).\n\n"

            "  -e, --present Validate only with the schema modules whose data actually\n"
            "                exist in the provided input data files. Takes effect only\n"
            "                with the 'data' or 'config' TYPEs. Used to avoid requiring\n"
            "                mandatory nodes from modules which data are not present in the\n"
            "                provided input data files.\n"
            "  -m, --merge   Merge input data files into a single tree and validate at\n"
            "                once.The option has effect only for 'data' and 'config' TYPEs.\n"
            "                In case of using -x option, the data are always merged.\n"
            "  -n, --not-strict\n"
            "                Do not require strict data parsing (silently skip unknown data),\n"
            "                has no effect for schemas.\n\n"
            "  -O FILE, --operational=FILE\n"
            "                Provide optional data to extend validation of the 'rpc',\n"
            "                'reply' or 'notif' TYPEs. The FILE is supposed to contain\n"
            "                the :running configuration datastore and state data\n"
            "                (operational datastore) referenced from the RPC/Notification.\n\n"

            "  -f FORMAT, --format=FORMAT\n"
            "                Print the data in one of the following formats:\n"
            "                xml, json, lyb\n"
            "                Note that the LYB format requires the -o option specified.\n"
            "  -F FORMAT, --in-format=FORMAT\n"
            "                Load the data in one of the following formats:\n"
            "                xml, json, lyb\n"
            "                If input format not specified, it is detected from the file extension.\n"
            "  -d MODE, --default=MODE\n"
            "                Print data with default values, according to the MODE\n"
            "                (to print attributes, ietf-netconf-with-defaults model\n"
            "                must be loaded):\n"
            "      all             - Add missing default nodes.\n"
            "      all-tagged      - Add missing default nodes and mark all the default\n"
            "                        nodes with the attribute.\n"
            "      trim            - Remove all nodes with a default value.\n"
            "      implicit-tagged - Add missing nodes and mark them with the attribute.\n"
            "  -o OUTFILE, --output=OUTFILE\n"
            "                Write the output to OUTFILE instead of stdout.\n\n"

            "  -x XPATH, --xpath=XPATH\n"
            "                Evaluate XPATH expression and print the nodes satysfying the.\n"
            "                expression. The output format is specific and the option cannot\n"
            "                be combined with the -f and -d options. Also all the data\n"
            "                inputs are merged into a single data tree where the expression\n"
            "                is evaluated, so the -m option is always set implicitly.\n\n");

}

void
cmd_data(struct ly_ctx **ctx, const char *cmdline)
{
    int argc = 0;
    char **argv = NULL;
    int opt, opt_index;
    struct option options[] = {
        {"defaults",    required_argument, NULL, 'd'},
        {"present",     no_argument,       NULL, 'e'},
        {"format",      required_argument, NULL, 'f'},
        {"in-format",   required_argument, NULL, 'F'},
        {"help",        no_argument,       NULL, 'h'},
        {"merge",       no_argument,       NULL, 'm'},
        {"output",      required_argument, NULL, 'o'},
        {"operational", required_argument, NULL, 'O'},
        {"not-strict",  no_argument,       NULL, 'n'},
        {"type",        required_argument, NULL, 't'},
        {"xpath",       required_argument, NULL, 'x'},
        {NULL, 0, NULL, 0}
    };

    uint8_t data_merge = 0;
    uint32_t options_print = 0;
    uint32_t options_parse = YL_DEFAULT_DATA_PARSE_OPTIONS;
    uint32_t options_validate = 0;
    enum lyd_type data_type = 0;
    uint8_t data_type_set = 0;
    LYD_FORMAT outformat = LYD_UNKNOWN;
    LYD_FORMAT informat = LYD_UNKNOWN;
    struct ly_out *out = NULL;
    struct cmdline_file *operational = NULL;
    struct ly_set inputs = {0};
    struct ly_set xpaths = {0};

    if (parse_cmdline(cmdline, &argc, &argv)) {
        goto cleanup;
    }

    while ((opt = getopt_long(argc, argv, "d:ef:F:hmo:O:r:nt:x:", options, &opt_index)) != -1) {
        switch (opt) {
        case 'd': /* --default */
            if (!strcasecmp(optarg, "all")) {
                options_print = (options_print & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL;
            } else if (!strcasecmp(optarg, "all-tagged")) {
                options_print = (options_print & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_ALL_TAG;
            } else if (!strcasecmp(optarg, "trim")) {
                options_print = (options_print & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_TRIM;
            } else if (!strcasecmp(optarg, "implicit-tagged")) {
                options_print = (options_print & ~LYD_PRINT_WD_MASK) | LYD_PRINT_WD_IMPL_TAG;
            } else {
                YLMSG_E("Unknown default mode %s\n", optarg);
                cmd_data_help();
                goto cleanup;
            }
            break;
        case 'f': /* --format */
            if (!strcasecmp(optarg, "xml")) {
                outformat = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                outformat = LYD_JSON;
            } else if (!strcasecmp(optarg, "lyb")) {
                outformat = LYD_LYB;
            } else {
                YLMSG_E("Unknown output format %s\n", optarg);
                cmd_data_help();
                goto cleanup;
            }
            break;
        case 'F': /* --in-format */
            if (!strcasecmp(optarg, "xml")) {
                informat = LYD_XML;
            } else if (!strcasecmp(optarg, "json")) {
                informat = LYD_JSON;
            } else if (!strcasecmp(optarg, "lyb")) {
                informat = LYD_LYB;
            } else {
                YLMSG_E("Unknown input format %s\n", optarg);
                cmd_data_help();
                goto cleanup;
            }
            break;
        case 'o': /* --output */
            if (out) {
                YLMSG_E("Only a single output can be specified.\n");
                goto cleanup;
            } else {
                if (ly_out_new_filepath(optarg, &out)) {
                    YLMSG_E("Unable open output file %s (%s)\n", optarg, strerror(errno));
                    goto cleanup;
                }
            }
            break;
        case 'O': { /* --operational */
            struct ly_in *in;
            LYD_FORMAT f;

            if (operational) {
                YLMSG_E("The operational datastore (-O) cannot be set multiple times.\n");
                goto cleanup;
            }
            if (get_input(optarg, NULL, &f, &in)) {
                goto cleanup;
            }
            operational = fill_cmdline_file(NULL, in, optarg, f);
            break;
        } /* case 'O' */

        case 'e': /* --present */
            options_validate |= LYD_VALIDATE_PRESENT;
            break;
        case 'm': /* --merge */
            data_merge = 1;
            break;
        case 'n': /* --not-strict */
            options_parse &= ~LYD_PARSE_STRICT;
            break;
        case 't': /* --type */
            if (data_type_set) {
                YLMSG_E("The data type (-t) cannot be set multiple times.\n");
                goto cleanup;
            }

            if (!strcasecmp(optarg, "config")) {
                options_parse |= LYD_PARSE_NO_STATE;
                options_validate |= LYD_VALIDATE_NO_STATE;
            } else if (!strcasecmp(optarg, "get")) {
                options_parse |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "getconfig") || !strcasecmp(optarg, "get-config")) {
                options_parse |= LYD_PARSE_ONLY | LYD_PARSE_NO_STATE;
            } else if (!strcasecmp(optarg, "edit")) {
                options_parse |= LYD_PARSE_ONLY;
            } else if (!strcasecmp(optarg, "rpc") || !strcasecmp(optarg, "action")) {
                data_type = LYD_TYPE_RPC_YANG;
            } else if (!strcasecmp(optarg, "reply") || !strcasecmp(optarg, "rpcreply")) {
                data_type = LYD_TYPE_REPLY_YANG;
            } else if (!strcasecmp(optarg, "notif") || !strcasecmp(optarg, "notification")) {
                data_type = LYD_TYPE_NOTIF_YANG;
            } else if (!strcasecmp(optarg, "data")) {
                /* default option */
            } else {
                YLMSG_E("Unknown data tree type %s.\n", optarg);
                cmd_data_help();
                goto cleanup;
            }

            data_type_set = 1;
            break;

        case 'x': /* --xpath */
            if (ly_set_add(&xpaths, optarg, 0, NULL)) {
                YLMSG_E("Storing XPath \"%s\" failed.\n", optarg);
                goto cleanup;
            }
            break;

        case 'h': /* --help */
            cmd_data_help();
            goto cleanup;
        default:
            YLMSG_E("Unknown option.\n");
            goto cleanup;
        }
    }

    if (optind == argc) {
        YLMSG_E("Missing the data file to process.\n");
        goto cleanup;
    }

    if (data_merge) {
        if (data_type || (options_parse & LYD_PARSE_ONLY)) {
            /* switch off the option, incompatible input data type */
            data_merge = 0;
        } else {
            /* postpone validation after the merge of all the input data */
            options_parse |= LYD_PARSE_ONLY;
        }
    } else if (xpaths.count) {
        data_merge = 1;
    }

    if (xpaths.count && outformat) {
        YLMSG_E("The --format option cannot be combined with --xpath option.\n");
        cmd_data_help();
        goto cleanup;
    }

    if (operational && !data_type) {
        YLMSG_W("Operational datastore takes effect only with RPCs/Actions/Replies/Notifications input data types.\n");
        free_cmdline_file(operational);
        operational = NULL;
    }

    /* process input data files provided as standalone command line arguments */
    for (int i = 0; i < argc - optind; i++) {
        struct ly_in *in;

        if (get_input(argv[optind + i], NULL, &informat, &in)) {
            goto cleanup;
        }

        if (!fill_cmdline_file(&inputs, in, argv[optind + i], informat)) {
            ly_in_free(in, 1);
            goto cleanup;
        }
    }

    /* default output stream */
    if (!out) {
        if (ly_out_new_file(stdout, &out)) {
            YLMSG_E("Unable to set stdout as output.\n");
            goto cleanup;
        }
    }

    /* parse, validate and print data */
    if (process_data(*ctx, data_type, data_merge, outformat, out,
            options_parse, options_validate, options_print,
            operational, &inputs, &xpaths)) {
        goto cleanup;
    }

cleanup:
    ly_out_free(out, NULL, 0);
    ly_set_erase(&inputs, free_cmdline_file);
    ly_set_erase(&xpaths, NULL);
    free_cmdline_file(operational);
    free_cmdline(argv);
}
