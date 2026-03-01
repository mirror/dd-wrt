/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "portable_endian.h"
#include "command.h"
#include "utilities.h"

#define TWT_WAKE_DURATION_UNIT                      (256)
#define TWT_WAKE_INTERVAL_EXPONENT_MAX_VAL          (31)
#define TWT_WAKE_DURATION_MAX_US                    (65280) /* UINT8_MAX * TWT_WAKE_DURATION_UNIT */
#define TWT_MAX_SETUP_COMMAND_VAL                   7
#define TWT_MAX_FLOW_ID_VAL                         7

#define TWT_WAKE_INTERVAL_DATATYPE "<wake interval>"
#define TWT_WAKE_INTERVAL_GLOSSARY "Wake interval (usecs)"

#define TWT_WAKE_DURATION_DATATYPE "<min wake duration>"
#define TWT_WAKE_DURATION_GLOSSARY "Minimum wake duration during TWT service period (usecs)"

#define TWT_SETUP_CMD_DATATYPE "<command>"
#define TWT_SETUP_CMD_GLOSSARY "TWT setup command to use:"

#define TWT_FLOW_ID_DATATYPE "<flow id>"
#define TWT_FLOW_ID_GLOSSARY "Flow id for TWT agreement (0-" STR(TWT_MAX_FLOW_ID_VAL) ")"

static struct {
    struct arg_rex *command;
} args;

static struct mm_argtable configure;
static struct mm_argtable remove_cmd;

static struct mm_argtable *subcmds[] =
{
    &configure, &remove_cmd,
};

static struct {
    struct arg_int *flow_id;
    struct arg_llong *wake_interval;
    struct arg_int *wake_duration;
    struct arg_int *setup_command;
} configure_args;

static struct {
    struct arg_int *flow_id;
} remove_args;


int twt_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
#define TWT_AVAILABLE_COMMANDS "conf|remove"

    MM_INIT_ARGTABLE(mm_args, "Install or remove a TWT agreement on a STA interface",
        args.command = arg_rex1(NULL, NULL, "(" TWT_AVAILABLE_COMMANDS ")",
            "{" TWT_AVAILABLE_COMMANDS "}", 0, "TWT subcommand"));
    args.command->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(&configure, "Configure TWT settings",
        configure_args.flow_id = arg_rint0("f", NULL, TWT_FLOW_ID_DATATYPE, 0, TWT_MAX_FLOW_ID_VAL,
            TWT_FLOW_ID_GLOSSARY),
        configure_args.wake_interval = arg_llong0("w", NULL, TWT_WAKE_INTERVAL_DATATYPE,
            TWT_WAKE_INTERVAL_GLOSSARY),
        configure_args.wake_duration = arg_rint0("d", NULL, TWT_WAKE_DURATION_DATATYPE, 0,
            TWT_WAKE_DURATION_MAX_US, TWT_WAKE_DURATION_GLOSSARY),
        configure_args.setup_command = arg_rint0("c", NULL, TWT_SETUP_CMD_DATATYPE, 0,
            TWT_MAX_SETUP_COMMAND_VAL, TWT_SETUP_CMD_GLOSSARY),
        arg_rem(NULL, "1: suggest"),
        arg_rem(NULL, "2: demand"),
        arg_rem(NULL, "3: grouping"),
        arg_rem(NULL, "4: accept"),
        arg_rem(NULL, "5: alternate"),
        arg_rem(NULL, "6: dictate"),
        arg_rem(NULL, "7: reject"));

    MM_INIT_ARGTABLE(&remove_cmd, "Remove TWT agreement",
        remove_args.flow_id = arg_rint0("f", NULL, TWT_FLOW_ID_DATATYPE, 0, TWT_MAX_FLOW_ID_VAL,
            TWT_FLOW_ID_GLOSSARY));

    return 0;
}

int twt_help(void)
{
    mm_help_argtable("twt conf", &configure);
    mm_help_argtable("twt remove", &remove_cmd);
    return 0;
}

static int twt_get_cmd(const char str[])
{
    if (strcmp("conf", str) == 0)
        return MORSE_CMD_TWT_CONF_OP_CONFIGURE;
    else if (strcmp("remove", str) == 0)
        return MORSE_CMD_TWT_CONF_OP_REMOVE_AGREEMENT;
    else
    {
        return -1;
    }
}

int twt(struct morsectrl *mors, int argc, char *argv[])
{
    int cmd_id;
    int ret = -1;
    int i;
    struct morsectrl_transport_buff *req_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;
    struct morse_cmd_req_set_twt_conf *twt_req = NULL;
    uint8_t flow_id = 0; /* flow id always set to 0 for now */
    uint32_t wake_duration_us = 0;
    uint64_t wake_interval_us = 0;
    uint64_t target_wake_time = 0;
    uint8_t setup_cmd = 0;

    cmd_id = twt_get_cmd(args.command->sval[0]);

    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);
    if (!rsp_tbuff)
        goto exit;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*twt_req));
    if (!req_tbuff)
        goto exit;

    twt_req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_twt_conf);
    switch (cmd_id)
    {
        case MORSE_CMD_TWT_CONF_OP_CONFIGURE:
        {
            ret = mm_parse_argtable("twt conf", &configure, argc, argv);
            if (ret != 0)
            {
                goto exit;
            }
            if (configure_args.flow_id->count +
                configure_args.setup_command->count +
                configure_args.wake_duration->count +
                configure_args.wake_interval->count == 0)
            {
                mctrl_print("At least one of -w, -d or -c is required\n");
                ret = -1;
                goto exit;
            }

            if (configure_args.flow_id->count)
            {
                flow_id = configure_args.flow_id->ival[0];
            }
            if (configure_args.wake_interval->count)
            {
                wake_interval_us = configure_args.wake_interval->ival[0];
            }
            if (configure_args.wake_duration->count)
            {
                wake_duration_us = configure_args.wake_duration->ival[0];
            }
            if (configure_args.setup_command->count)
            {
                setup_cmd = configure_args.setup_command->ival[0];
            }

            twt_req->flow_id = flow_id;
            twt_req->opcode = cmd_id;
            twt_req->wake_interval.wake_interval_us = htole64(wake_interval_us);
            twt_req->wake_duration_us = htole32(wake_duration_us);
            twt_req->twt_setup_command = setup_cmd;
            break;
        }
        case MORSE_CMD_TWT_CONF_OP_REMOVE_AGREEMENT:
        {
            ret = mm_parse_argtable("twt remove", &remove_cmd, argc, argv);
            if (ret != 0)
            {
                goto exit;
            }
            if (remove_args.flow_id->count)
            {
                flow_id = remove_args.flow_id->ival[0];
            }
            twt_req->flow_id = flow_id;
            twt_req->opcode = cmd_id;
        }
        break;
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_TWT_CONF, req_tbuff,
            rsp_tbuff);

exit:
    /* Check if the reason we got here is because --help was given */
    if (mm_check_help_argtable(subcmds, MORSE_ARRAY_SIZE(subcmds)))
    {
        ret = 0;
    }
    else if (ret == 0 &&
             (cmd_id == MORSE_CMD_TWT_CONF_OP_CONFIGURE ||
              cmd_id == MORSE_CMD_TWT_CONF_OP_CONFIGURE_EXPLICIT ||
              cmd_id == MORSE_CMD_TWT_CONF_OP_FORCE_INSTALL_AGREEMENT))
    {
        if (twt_req != NULL)
        {
            mctrl_print("Installed TWT Agreement[flowid:%d]\n", flow_id);
            mctrl_print("    Wake interval: %" PRId64 " us\n",
                wake_interval_us);
            mctrl_print("    Wake duration: %d us\n", wake_duration_us);
            mctrl_print("    Target Wake Time: %" PRId64 "\n",
                target_wake_time);
            mctrl_print("    Implict: true\n");
        }
    }
    else if (cmd_id == MORSE_CMD_TWT_CONF_OP_REMOVE_AGREEMENT)
    {
        mctrl_print("Removed TWT Agreement[flowid:%d]\n", twt_req->flow_id);
    }

    if (req_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
    }
    if (rsp_tbuff)
    {
        morsectrl_transport_buff_free(rsp_tbuff);
    }

    for (i = 0; i < MORSE_ARRAY_SIZE(subcmds); i++)
    {
        mm_free_argtable(subcmds[i]);
    }
    return ret;
}

MM_CLI_HANDLER_CUSTOM_HELP(twt, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
