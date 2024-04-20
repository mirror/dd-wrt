/**
 * @file cmd_data.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief 'data' command of the libyang's yanglint tool.
 *
 * Copyright (c) 2015-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "cmd.h"

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "libyang.h"

#include "common.h"
#include "yl_opt.h"

static void
cmd_data_help_header(void)
{
    printf("Usage: data [-emn] [-t TYPE]\n"
            "            [-F FORMAT] [-f FORMAT] [-d DEFAULTS] [-o OUTFILE] <data1> ...\n"
            "       data [-n] -t (rpc | notif | reply) [-O FILE]\n"
            "            [-F FORMAT] [-f FORMAT] [-d DEFAULTS] [-o OUTFILE] <data1> ...\n"
            "       data [-en] [-t TYPE] [-F FORMAT] -x XPATH [-o OUTFILE] <data1> ...\n"
            "                  Parse, validate and optionally print data instances\n");
}

static void
cmd_data_help_type(void)
{
    printf("  -t TYPE, --type=TYPE\n"
            "                Specify data tree type in the input data file(s):\n"
            "        data          - Complete datastore with status data (default type).\n"
            "        config        - Configuration datastore (without status data).\n"
            "        get           - Result of the NETCONF <get> operation.\n"
            "        getconfig     - Result of the NETCONF <get-config> operation.\n"
            "        edit          - Content of the NETCONF <edit-config> operation.\n"
            "        rpc           - Content of the NETCONF <rpc> message, defined as YANG's\n"
            "                        RPC/Action input statement.\n"
            "        nc-rpc        - Similar to 'rpc' but expect and check also the NETCONF\n"
            "                        envelopes <rpc> or <action>.\n"
            "        reply         - Reply to the RPC/Action. Note that the reply data are\n"
            "                        expected inside a container representing the original\n"
            "                        RPC/Action. This is necessary to identify appropriate\n"
            "                        data definitions in the schema module.\n"
            "        nc-reply      - Similar to 'reply' but expect and check also the NETCONF\n"
            "                        envelope <rpc-reply> with output data nodes as direct\n"
            "                        descendants. The original RPC/action invocation is expected\n"
            "                        in a separate parameter '-R' and is parsed as 'nc-rpc'.\n"
            "        notif         - Notification instance (content of the <notification>\n"
            "                        element without <eventTime>).\n"
            "        nc-notif      - Similar to 'notif' but expect and check also the NETCONF\n"
            "                        envelope <notification> with element <eventTime> and its\n"
            "                        sibling as the actual notification.\n");
}

static void
cmd_data_help_format(void)
{
    printf("  -f FORMAT, --format=FORMAT\n"
            "                Print the data in one of the following formats:\n"
            "                xml, json, lyb\n"
            "                Note that the LYB format requires the -o option specified.\n");
}

static void
cmd_data_help_in_format(void)
{
    printf("  -F FORMAT, --in-format=FORMAT\n"
            "                Load the data in one of the following formats:\n"
            "                xml, json, lyb\n"
            "                If input format not specified, it is detected from the file extension.\n");
}

static void
cmd_data_help_default(void)
{
    printf("  -d MODE, --default=MODE\n"
            "                Print data with default values, according to the MODE\n"
            "                (to print attributes, ietf-netconf-with-defaults model\n"
            "                must be loaded):\n"
            "      all             - Add missing default nodes.\n"
            "      all-tagged      - Add missing default nodes and mark all the default\n"
            "                        nodes with the attribute.\n"
            "      trim            - Remove all nodes with a default value.\n"
            "      implicit-tagged - Add missing nodes and mark them with the attribute.\n");
}

static void
cmd_data_help_xpath(void)
{
    printf("  -x XPATH, --xpath=XPATH\n"
            "                Evaluate XPATH expression and print the nodes satisfying the\n"
            "                expression. The output format is specific and the option cannot\n"
            "                be combined with the -f and -d options. Also all the data\n"
            "                inputs are merged into a single data tree where the expression\n"
            "                is evaluated, so the -m option is always set implicitly.\n");
}

void
cmd_data_help(void)
{
    cmd_data_help_header();
    printf("\n");
    cmd_data_help_type();
    printf("  -e, --present Validate only with the schema modules whose data actually\n"
            "                exist in the provided input data files. Takes effect only\n"
            "                with the 'data' or 'config' TYPEs. Used to avoid requiring\n"
            "                mandatory nodes from modules which data are not present in the\n"
            "                provided input data files.\n"
            "  -m, --merge   Merge input data files into a single tree and validate at\n"
            "                once.The option has effect only for 'data' and 'config' TYPEs.\n"
            "                In case of using -x option, the data are always merged.\n"
            "  -n, --not-strict\n"
            "                Do not require strict data parsing (silently skip unknown data),\n"
            "                has no effect for schemas.\n"
            "  -O FILE, --operational=FILE\n"
            "                Provide optional data to extend validation of the 'rpc',\n"
            "                'reply' or 'notif' TYPEs. The FILE is supposed to contain\n"
            "                the operational datastore referenced from the operation.\n"
            "                In case of a nested notification or action, its parent\n"
            "                existence is also checked in these operational data.\n"
            "  -R FILE, --reply-rpc=FILE\n"
            "                Provide source RPC for parsing of the 'nc-reply' TYPE. The FILE\n"
            "                is supposed to contain the source 'nc-rpc' operation of the reply.\n");
    cmd_data_help_format();
    cmd_data_help_in_format();
    printf("  -o OUTFILE, --output=OUTFILE\n"
            "                Write the output to OUTFILE instead of stdout.\n");
    cmd_data_help_xpath();
    printf("\n");
}

int
cmd_data_opt(struct yl_opt *yo, const char *cmdline, char ***posv, int *posc)
{
    int rc = 0, argc = 0;
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
        {"reply-rpc",   required_argument, NULL, 'R'},
        {"not-strict",  no_argument,       NULL, 'n'},
        {"type",        required_argument, NULL, 't'},
        {"xpath",       required_argument, NULL, 'x'},
        {NULL, 0, NULL, 0}
    };

    uint8_t data_type_set = 0;

    yo->data_parse_options = YL_DEFAULT_DATA_PARSE_OPTIONS;
    yo->data_validate_options = YL_DEFAULT_DATA_VALIDATE_OPTIONS;

    if ((rc = parse_cmdline(cmdline, &argc, &yo->argv))) {
        return rc;
    }

    while ((opt = getopt_long(argc, yo->argv, commands[CMD_DATA].optstring, options, &opt_index)) != -1) {
        switch (opt) {
        case 'd': /* --default */
            if (yo_opt_update_data_default(optarg, yo)) {
                YLMSG_E("Unknown default mode %s.", optarg);
                cmd_data_help_default();
                return 1;
            }
            break;
        case 'f': /* --format */
            if (yl_opt_update_data_out_format(optarg, yo)) {
                cmd_data_help_format();
                return 1;
            }
            break;
        case 'F': /* --in-format */
            if (yo_opt_update_data_in_format(optarg, yo)) {
                YLMSG_E("Unknown input format %s.", optarg);
                cmd_data_help_in_format();
                return 1;
            }
            break;
        case 'o': /* --output */
            if (yo->out) {
                YLMSG_E("Only a single output can be specified.");
                return 1;
            } else {
                if (ly_out_new_filepath(optarg, &yo->out)) {
                    YLMSG_E("Unable open output file %s (%s).", optarg, strerror(errno));
                    return 1;
                }
            }
            break;
        case 'O':   /* --operational */
            if (yo->data_operational.path) {
                YLMSG_E("The operational datastore (-O) cannot be set multiple times.");
                return 1;
            }
            yo->data_operational.path = optarg;
            break;
        case 'R':   /* --reply-rpc */
            if (yo->reply_rpc.path) {
                YLMSG_E("The PRC of the reply (-R) cannot be set multiple times.");
                return 1;
            }
            yo->reply_rpc.path = optarg;
            break;
        case 'e': /* --present */
            yo->data_validate_options |= LYD_VALIDATE_PRESENT;
            break;
        case 'm': /* --merge */
            yo->data_merge = 1;
            break;
        case 'n': /* --not-strict */
            yo->data_parse_options &= ~LYD_PARSE_STRICT;
            break;
        case 't': /* --type */
            if (data_type_set) {
                YLMSG_E("The data type (-t) cannot be set multiple times.");
                return 1;
            }

            if (yl_opt_update_data_type(optarg, yo)) {
                YLMSG_E("Unknown data tree type %s.", optarg);
                cmd_data_help_type();
                return 1;
            }

            data_type_set = 1;
            break;

        case 'x': /* --xpath */
            if (ly_set_add(&yo->data_xpath, optarg, 0, NULL)) {
                YLMSG_E("Storing XPath \"%s\" failed.", optarg);
                return 1;
            }
            break;

        case 'h': /* --help */
            cmd_data_help();
            return 1;
        default:
            YLMSG_E("Unknown option.");
            return 1;
        }
    }

    *posv = &yo->argv[optind];
    *posc = argc - optind;

    return rc;
}

int
cmd_data_dep(struct yl_opt *yo, int posc)
{
    if (yo->interactive && !posc) {
        YLMSG_E("Missing the data file to process.");
        return 1;
    }

    if (yo->data_merge) {
        if (yo->data_type || (yo->data_parse_options & LYD_PARSE_ONLY)) {
            /* switch off the option, incompatible input data type */
            YLMSG_W("The --merge option has effect only for 'data' and 'config' TYPEs.");
            yo->data_merge = 0;
        } else {
            /* postpone validation after the merge of all the input data */
            yo->data_parse_options |= LYD_PARSE_ONLY;
        }
    } else if (yo->data_xpath.count) {
        yo->data_merge = 1;
    }

    if (yo->data_xpath.count && (yo->schema_out_format || yo->data_out_format)) {
        YLMSG_E("The --format option cannot be combined with --xpath option.");
        if (yo->interactive) {
            cmd_data_help_xpath();
        }
        return 1;
    }
    if (yo->data_xpath.count && (yo->data_print_options & LYD_PRINT_WD_MASK)) {
        YLMSG_E("The --default option cannot be combined with --xpath option.");
        if (yo->interactive) {
            cmd_data_help_xpath();
        }
        return 1;
    }

    if (yo->data_operational.path && !yo->data_type) {
        YLMSG_W("Operational datastore takes effect only with RPCs/Actions/Replies/Notification input data types.");
        yo->data_operational.path = NULL;
    }

    if (yo->reply_rpc.path && (yo->data_type != LYD_TYPE_REPLY_NETCONF)) {
        YLMSG_W("Source RPC is needed only for NETCONF Reply input data type.");
        yo->data_operational.path = NULL;
    } else if (!yo->reply_rpc.path && (yo->data_type == LYD_TYPE_REPLY_NETCONF)) {
        YLMSG_E("Missing source RPC (-R) for NETCONF Reply input data type.");
        return 1;
    }

    if (!yo->out && (yo->data_out_format == LYD_LYB)) {
        YLMSG_E("The LYB format requires the -o option specified.");
        return 1;
    }

    /* default output stream */
    if (!yo->out) {
        if (ly_out_new_file(stdout, &yo->out)) {
            YLMSG_E("Unable to set stdout as output.");
            return 1;
        }
        yo->out_stdout = 1;
    }

    /* process the operational and/or reply RPC content if any */
    if (yo->data_operational.path) {
        if (get_input(yo->data_operational.path, NULL, &yo->data_operational.format, &yo->data_operational.in)) {
            return -1;
        }
    }
    if (yo->reply_rpc.path) {
        if (get_input(yo->reply_rpc.path, NULL, &yo->reply_rpc.format, &yo->reply_rpc.in)) {
            return -1;
        }
    }

    return 0;
}

int
cmd_data_store(struct ly_ctx **ctx, struct yl_opt *yo, const char *posv)
{
    (void) ctx;
    struct ly_in *in;
    LYD_FORMAT format_data;

    assert(posv);

    format_data = yo->data_in_format;

    /* process input data files provided as standalone command line arguments */
    if (get_input(posv, NULL, &format_data, &in)) {
        return 1;
    }

    if (!fill_cmdline_file(&yo->data_inputs, in, posv, format_data)) {
        ly_in_free(in, 1);
        return 1;
    }

    return 0;
}

/**
 * @brief Evaluate xpath adn print result.
 *
 * @param[in] tree Data tree.
 * @param[in] xpath Xpath to evaluate.
 * @return 0 on success.
 */
static int
evaluate_xpath(const struct lyd_node *tree, const char *xpath)
{
    struct ly_set *set = NULL;

    if (lyd_find_xpath(tree, xpath, &set)) {
        return -1;
    }

    /* print result */
    printf("XPath \"%s\" evaluation result:\n", xpath);
    if (!set->count) {
        printf("\tEmpty\n");
    } else {
        for (uint32_t u = 0; u < set->count; ++u) {
            struct lyd_node *node = (struct lyd_node *)set->objs[u];

            printf("  %s \"%s\"", lys_nodetype2str(node->schema->nodetype), node->schema->name);
            if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                printf(" (value: \"%s\")\n", lyd_get_value(node));
            } else if (node->schema->nodetype == LYS_LIST) {
                printf(" (");
                for (struct lyd_node *key = ((struct lyd_node_inner *)node)->child; key && lysc_is_key(key->schema); key = key->next) {
                    printf("%s\"%s\": \"%s\";", (key != ((struct lyd_node_inner *)node)->child) ? " " : "",
                            key->schema->name, lyd_get_value(key));
                }
                printf(")\n");
            } else {
                printf("\n");
            }
        }
    }

    ly_set_free(set, NULL);
    return 0;
}

/**
 * @brief Checking that a parent data node exists in the datastore for the nested-notification and action.
 *
 * @param[in] op Operation to check.
 * @param[in] oper_tree Data from datastore.
 * @param[in] operational_f Operational datastore file information.
 * @return LY_ERR value.
 */
static LY_ERR
check_operation_parent(struct lyd_node *op, struct lyd_node *oper_tree, struct cmdline_file *operational_f)
{
    LY_ERR ret;
    struct ly_set *set = NULL;
    char *path = NULL;

    if (!op || !lyd_parent(op)) {
        /* The function is defined only for nested-notification and action. */
        return LY_SUCCESS;
    }

    if (!operational_f || (operational_f && !operational_f->in)) {
        YLMSG_E("The --operational parameter needed to validate operation \"%s\" is missing.", LYD_NAME(op));
        ret = LY_EVALID;
        goto cleanup;
    }

    path = lyd_path(lyd_parent(op), LYD_PATH_STD, NULL, 0);
    if (!path) {
        ret = LY_EMEM;
        goto cleanup;
    }

    if (!oper_tree) {
        YLMSG_W("Operational datastore is empty or contains unknown data.");
        YLMSG_E("Operation \"%s\" parent \"%s\" not found in the operational data.", LYD_NAME(op), path);
        ret = LY_EVALID;
        goto cleanup;
    }
    if ((ret = lyd_find_xpath(oper_tree, path, &set))) {
        goto cleanup;
    }
    if (!set->count) {
        YLMSG_E("Operation \"%s\" parent \"%s\" not found in the operational data.", LYD_NAME(op), path);
        ret = LY_EVALID;
        goto cleanup;
    }

cleanup:
    ly_set_free(set, NULL);
    free(path);

    return ret;
}

/**
 * @brief Process the input data files - parse, validate and print according to provided options.
 *
 * @param[in] ctx libyang context with schema.
 * @param[in] type The type of data in the input files.
 * @param[in] merge Flag if the data should be merged before validation.
 * @param[in] out_format Data format for printing.
 * @param[in] out The output handler for printing.
 * @param[in] parse_options Parser options.
 * @param[in] validate_options Validation options.
 * @param[in] print_options Printer options.
 * @param[in] operational Optional operational datastore file information for the case of an extended validation of
 * operation(s).
 * @param[in] reply_rpc Source RPC operation file information for parsing NETCONF rpc-reply.
 * @param[in] inputs Set of file informations of input data files.
 * @param[in] xpaths The set of XPaths to be evaluated on the processed data tree, basic information about the resulting set
 * is printed. Alternative to data printing.
 * @return LY_ERR value.
 */
static LY_ERR
process_data(struct ly_ctx *ctx, enum lyd_type type, uint8_t merge, LYD_FORMAT out_format,
        struct ly_out *out, uint32_t parse_options, uint32_t validate_options, uint32_t print_options,
        struct cmdline_file *operational, struct cmdline_file *reply_rpc, struct ly_set *inputs,
        struct ly_set *xpaths)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *tree = NULL, *op = NULL, *envp = NULL, *merged_tree = NULL, *oper_tree = NULL;
    const char *xpath;
    struct ly_set *set = NULL;

    /* additional operational datastore */
    if (operational && operational->in) {
        ret = lyd_parse_data(ctx, NULL, operational->in, operational->format, LYD_PARSE_ONLY, 0, &oper_tree);
        if (ret) {
            YLMSG_E("Failed to parse operational datastore file \"%s\".", operational->path);
            goto cleanup;
        }
    }

    for (uint32_t u = 0; u < inputs->count; ++u) {
        struct cmdline_file *input_f = (struct cmdline_file *)inputs->objs[u];

        switch (type) {
        case LYD_TYPE_DATA_YANG:
            ret = lyd_parse_data(ctx, NULL, input_f->in, input_f->format, parse_options, validate_options, &tree);
            break;
        case LYD_TYPE_RPC_YANG:
        case LYD_TYPE_REPLY_YANG:
        case LYD_TYPE_NOTIF_YANG:
            ret = lyd_parse_op(ctx, NULL, input_f->in, input_f->format, type, &tree, &op);
            break;
        case LYD_TYPE_RPC_NETCONF:
        case LYD_TYPE_NOTIF_NETCONF:
            ret = lyd_parse_op(ctx, NULL, input_f->in, input_f->format, type, &envp, &op);

            /* adjust pointers */
            for (tree = op; lyd_parent(tree); tree = lyd_parent(tree)) {}
            break;
        case LYD_TYPE_REPLY_NETCONF:
            /* parse source RPC operation */
            assert(reply_rpc && reply_rpc->in);
            ret = lyd_parse_op(ctx, NULL, reply_rpc->in, reply_rpc->format, LYD_TYPE_RPC_NETCONF, &envp, &op);
            if (ret) {
                YLMSG_E("Failed to parse source NETCONF RPC operation file \"%s\".", reply_rpc->path);
                goto cleanup;
            }

            /* adjust pointers */
            for (tree = op; lyd_parent(tree); tree = lyd_parent(tree)) {}

            /* free input */
            lyd_free_siblings(lyd_child(op));

            /* we do not care */
            lyd_free_all(envp);
            envp = NULL;

            ret = lyd_parse_op(ctx, op, input_f->in, input_f->format, type, &envp, NULL);
            break;
        default:
            YLMSG_E("Internal error (%s:%d).", __FILE__, __LINE__);
            goto cleanup;
        }

        if (ret) {
            YLMSG_E("Failed to parse input data file \"%s\".", input_f->path);
            goto cleanup;
        }

        if (merge) {
            /* merge the data so far parsed for later validation and print */
            if (!merged_tree) {
                merged_tree = tree;
            } else {
                ret = lyd_merge_siblings(&merged_tree, tree, LYD_MERGE_DESTRUCT);
                if (ret) {
                    YLMSG_E("Merging %s with previous data failed.", input_f->path);
                    goto cleanup;
                }
            }
            tree = NULL;
        } else if (out_format) {
            /* print */
            switch (type) {
            case LYD_TYPE_DATA_YANG:
                lyd_print_all(out, tree, out_format, print_options);
                break;
            case LYD_TYPE_RPC_YANG:
            case LYD_TYPE_REPLY_YANG:
            case LYD_TYPE_NOTIF_YANG:
            case LYD_TYPE_RPC_NETCONF:
            case LYD_TYPE_NOTIF_NETCONF:
                lyd_print_tree(out, tree, out_format, print_options);
                break;
            case LYD_TYPE_REPLY_NETCONF:
                /* just the output */
                lyd_print_tree(out, lyd_child(tree), out_format, print_options);
                break;
            default:
                assert(0);
            }
        } else {
            /* validation of the RPC/Action/reply/Notification with the operational datastore, if any */
            switch (type) {
            case LYD_TYPE_DATA_YANG:
                /* already validated */
                break;
            case LYD_TYPE_RPC_YANG:
            case LYD_TYPE_RPC_NETCONF:
                ret = lyd_validate_op(tree, oper_tree, LYD_TYPE_RPC_YANG, NULL);
                break;
            case LYD_TYPE_REPLY_YANG:
            case LYD_TYPE_REPLY_NETCONF:
                ret = lyd_validate_op(tree, oper_tree, LYD_TYPE_REPLY_YANG, NULL);
                break;
            case LYD_TYPE_NOTIF_YANG:
            case LYD_TYPE_NOTIF_NETCONF:
                ret = lyd_validate_op(tree, oper_tree, LYD_TYPE_NOTIF_YANG, NULL);
                break;
            default:
                assert(0);
            }
            if (ret) {
                if (operational->path) {
                    YLMSG_E("Failed to validate input data file \"%s\" with operational datastore \"%s\".",
                            input_f->path, operational->path);
                } else {
                    YLMSG_E("Failed to validate input data file \"%s\".", input_f->path);
                }
                goto cleanup;
            }

            if ((ret = check_operation_parent(op, oper_tree, operational))) {
                goto cleanup;
            }
        }

        /* next iter */
        lyd_free_all(tree);
        tree = NULL;
        lyd_free_all(envp);
        envp = NULL;
    }

    if (merge) {
        /* validate the merged result */
        ret = lyd_validate_all(&merged_tree, ctx, validate_options, NULL);
        if (ret) {
            YLMSG_E("Merged data are not valid.");
            goto cleanup;
        }

        if (out_format) {
            /* and print it */
            lyd_print_all(out, merged_tree, out_format, print_options);
        }

        for (uint32_t u = 0; xpaths && (u < xpaths->count); ++u) {
            xpath = (const char *)xpaths->objs[u];
            ly_set_free(set, NULL);
            ret = lys_find_xpath(ctx, NULL, xpath, LYS_FIND_NO_MATCH_ERROR, &set);
            if (ret || !set->count) {
                ret = (ret == LY_SUCCESS) ? LY_EINVAL : ret;
                YLMSG_E("The requested xpath failed.");
                goto cleanup;
            }
            if (evaluate_xpath(merged_tree, xpath)) {
                goto cleanup;
            }
        }
    }

cleanup:
    lyd_free_all(tree);
    lyd_free_all(envp);
    lyd_free_all(merged_tree);
    lyd_free_all(oper_tree);
    ly_set_free(set, NULL);
    return ret;
}

int
cmd_data_process(struct ly_ctx *ctx, struct yl_opt *yo)
{
    /* parse, validate and print data */
    if (process_data(ctx, yo->data_type, yo->data_merge, yo->data_out_format, yo->out, yo->data_parse_options,
            yo->data_validate_options, yo->data_print_options, &yo->data_operational, &yo->reply_rpc,
            &yo->data_inputs, &yo->data_xpath)) {
        return 1;
    }

    return 0;
}
