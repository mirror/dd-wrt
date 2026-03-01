/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "portable_endian.h"
#include "command.h"
#include "utilities.h"

static struct {
    struct arg_rex *subcmd;
    struct arg_rex *decrease;
    struct arg_rex *increase;
    struct arg_lit *verbose;
} args;

int cac_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args,
        "Configure Centralized Authentication Control",
        args.subcmd = arg_rex0(NULL, NULL, "(get|set|enable|disable)", "{get|set|enable|disable}",
            1, "Subcommand"),
        arg_rem(NULL,
            "get - get configured rules"),
        arg_rem(NULL,
            "set - set rules (default)"),
        arg_rem(NULL,
            "enable|disable - for internal use by wpa_supplicant only"),
        args.decrease = arg_rexn("d", "decrease", "[0-9]{1,3},[0-9]{1,2}",
            "<ARFS>,<percent>", 0, MORSE_CMD_CAC_CFG_CHANGE_RULE_MAX, 0,
            "Auth Req Frames per Sec above which to decrease threshold by <percent>"),
        arg_rem(NULL,
            "Decrease rules must be specified in descending ARFS order (match highest first)"),
        args.increase = arg_rexn("i", "increase", "[0-9]{1,3},[0-9]{1,2}",
            "<ARFS>,<percent>", 0, MORSE_CMD_CAC_CFG_CHANGE_RULE_MAX, 0,
            "Auth Req Frames per Sec below which to increase threshold by <percent>"),
        arg_rem(NULL,
            "Increase rules must be specified in ascending ARFS order (match lowest first)"),
        arg_rem(NULL,
            "Up to 8 decrease or increase rules can be configured"),
        args.verbose = arg_lit0("v", "verbose", "Verbose output"));
    return 0;
}

static int cac_cmd_enable_or_disable(struct morse_cmd_req_cac *cac_req)
{
    if (args.decrease->count || args.increase->count)
    {
        /* Assume user error - request from wpa_supplicant would not do this */
        mctrl_err("enable and disable are for internal use only\n");
        return -EINVAL;
    }

    return 0;
}

static void cac_print_rule(struct morse_cmd_cac_change_rule *rule)
{
    int16_t threshold_change;
    threshold_change = le16toh(rule->threshold_change);
    bool is_decrease = threshold_change < 0;

    mctrl_print("When ARFS is %s %u, %s threshold by %d%%\n",
                is_decrease ? "greater than" : "less than",
                rule->arfs,
                is_decrease ? "decrease" : "increase",
                abs(threshold_change));
}

static void cac_print_rules(struct morse_cmd_resp_cac *cac_cfm)
{
    int i;
    struct morse_cmd_cac_change_rule *rule;

    for (i = 0; i < cac_cfm->rule_tot && i < MORSE_ARRAY_SIZE(cac_cfm->rule); i++)
    {
        rule = &cac_cfm->rule[i];
        cac_print_rule(rule);
    }
}

static int cac_add_rule_to_cmd(struct morse_cmd_cac_change_rule *rule,
                               const char *rule_str, bool is_decrease,
                               uint16_t *arfs_prev, uint16_t *threshold_change_prev)
{
    int ret;
    unsigned int arfs;
    unsigned int threshold_change;

    ret = sscanf(rule_str, "%u,%u\n", &arfs, &threshold_change);
    if (ret != 2)
    {
        mctrl_err("Unexpected rule parse error in %s\n", rule_str);
    }

    if (arfs < 1 || arfs > MORSE_CMD_CAC_CFG_ARFS_MAX)
    {
        mctrl_err("ARFS value (%u) is not between 1 and %d\n", arfs, MORSE_CMD_CAC_CFG_ARFS_MAX);
        return -EINVAL;
    }

    if (threshold_change < 1 || threshold_change > MORSE_CMD_CAC_CFG_CHANGE_MAX)
    {
        mctrl_err("Threshold change (%d) is not between 1%% and %d%%\n",
                  threshold_change, MORSE_CMD_CAC_CFG_CHANGE_MAX);
        return -EINVAL;
    }

    if (threshold_change >= *threshold_change_prev)
    {
        mctrl_err("Threshold value (%u) for %s rule is not in descending order\n",
                  threshold_change, is_decrease ? "decrease" : "increase");
        return -EINVAL;
    }

    rule->arfs = htole16(arfs);
    if (is_decrease)
    {
        if (arfs >= *arfs_prev)
        {
            mctrl_err("ARFS value (%u) for decrease rule is not in descending order\n", arfs);
            return -EINVAL;
        }
        rule->threshold_change = htole16((int16_t) -threshold_change);
    }
    else
    {
        if (arfs <= *arfs_prev)
        {
            mctrl_err("ARFS value (%u) for increase rule is not in ascending order\n", arfs);
            return -EINVAL;
        }
        rule->threshold_change = htole16(threshold_change);
    }
    *arfs_prev = arfs;
    *threshold_change_prev = threshold_change;

    if (args.verbose->count)
    {
        cac_print_rule(rule);
    }

    return 0;
}

static int cac_cmd_get(struct morse_cmd_req_cac *cac_req)
{
    if (args.decrease->count || args.increase->count)
    {
        /* Assume user error - request from wpa_supplicant would not do this */
        mctrl_err("Decrease and increase parameters are invalid for the get subcommand\n");
        return -EINVAL;
    }

    return 0;
}

static int cac_cmd_set(struct morse_cmd_req_cac *cac_req)
{
    int ret = 0;
    uint16_t arfs_prev;
    uint16_t threshold_change_prev;
    const char *rule_str;
    int rule_idx = 0;
    struct morse_cmd_cac_change_rule *rule;
    int i;

    cac_req->opcode = MORSE_CMD_CAC_OP_CFG_SET;
    cac_req->rule_tot = args.decrease->count + args.increase->count;

    if (cac_req->rule_tot == 0)
    {
        mctrl_err("No rules specified\n");
        return -EINVAL;
    }

    if (cac_req->rule_tot > MORSE_ARRAY_SIZE(cac_req->rule))
    {
        mctrl_err("Max number of decrease and increase rules is %d\n",
                  (int)MORSE_ARRAY_SIZE(cac_req->rule));
        return -EINVAL;
    }

    /* Add decrease rules to the command
     * - ARFS must be in descending order
     * - Threshold change must be in descending order
     */
    arfs_prev = MORSE_CMD_CAC_CFG_ARFS_MAX + 1;
    threshold_change_prev = MORSE_CMD_CAC_CFG_CHANGE_MAX + 1;
    for (i = 0; i < args.decrease->count && ret == 0; i++)
    {
        rule_str = args.decrease->sval[i];
        rule = &cac_req->rule[rule_idx];
        ret = cac_add_rule_to_cmd(rule, rule_str, true, &arfs_prev, &threshold_change_prev);
        rule_idx++;
    }

    /* Add increase rules to the command
     * - ARFS must be in ascending order
     * - Threshold change must be in descending order
     */
    arfs_prev = 0;
    threshold_change_prev = MORSE_CMD_CAC_CFG_CHANGE_MAX + 1;
    for (i = 0; i < args.increase->count && ret == 0; i++)
    {
        rule_str = args.increase->sval[i];
        rule = &cac_req->rule[rule_idx];
        ret = cac_add_rule_to_cmd(rule, rule_str, false, &arfs_prev, &threshold_change_prev);
        rule_idx++;
    }

    return ret;
}

static int cac_handle_command(struct morse_cmd_req_cac *cac_req)
{
    if (strcmp(args.subcmd->sval[0], "enable") == 0)
    {
        cac_req->opcode = MORSE_CMD_CAC_OP_ENABLE;
        return cac_cmd_enable_or_disable(cac_req);
    }

    if (strcmp(args.subcmd->sval[0], "disable") == 0)
    {
        cac_req->opcode = MORSE_CMD_CAC_OP_DISABLE;
        return cac_cmd_enable_or_disable(cac_req);
    }

    if (strcmp(args.subcmd->sval[0], "get") == 0)
    {
        cac_req->opcode = MORSE_CMD_CAC_OP_CFG_GET;
        return cac_cmd_get(cac_req);
    }

    if (args.subcmd->count == 0 ||
        strcmp(args.subcmd->sval[0], "set") == 0)
    {
        cac_req->opcode = MORSE_CMD_CAC_OP_CFG_SET;
        return cac_cmd_set(cac_req);
    }

    return -EINVAL;
}

int cac(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -EINVAL;
    struct morse_cmd_req_cac *cac_req = NULL;
    struct morse_cmd_resp_cac *cac_cfm = NULL;
    struct morsectrl_transport_buff *cmd_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*cac_req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*cac_cfm));
    if (!cmd_tbuff || !rsp_tbuff)
    {
        goto exit;
    }

    cac_req = TBUFF_TO_REQ(cmd_tbuff, struct morse_cmd_req_cac);
    cac_cfm = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_cac);

    ret = cac_handle_command(cac_req);
    if (ret < 0)
    {
        goto exit;
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_CAC, cmd_tbuff,
            rsp_tbuff);

    if (ret == 0 && cac_req->opcode == MORSE_CMD_CAC_OP_CFG_GET)
    {
        cac_print_rules(cac_cfm);
    }

exit:
    if (cmd_tbuff)
    {
        morsectrl_transport_buff_free(cmd_tbuff);
    }

    if (rsp_tbuff)
    {
        morsectrl_transport_buff_free(rsp_tbuff);
    }

    return ret;
}

MM_CLI_HANDLER(cac, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
