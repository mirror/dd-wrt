/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "portable_endian.h"

#include "command.h"
#include "utilities.h"

/* Limits on duty cycle, expressed in  percent */
#define DUTY_CYCLE_MIN      0.01
#define DUTY_CYCLE_MAX      100.0

enum duty_cycle_cmd
{
    DUTY_CYCLE_CMD_DISABLE,
    DUTY_CYCLE_CMD_ENABLE,
    DUTY_CYCLE_CMD_AIRTIME
};

static struct {
    struct arg_rex *enable;
    struct arg_dbl *value;
    struct arg_int *mode;
    struct arg_lit *omit_cr;
} args;

int duty_cycle_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Query (default) or configure duty cycle mode",
        args.enable = arg_rex0(NULL, NULL, "(enable|disable|airtime)", "{enable|disable|airtime}",
        0, "Set duty cycle mode"),
        arg_rem(NULL, "enable: Enable duty cycle mode"),
        arg_rem(NULL, "disable: Disable duty cycle mode"),
        arg_rem(NULL, "airtime: Return remaining airtime (usecs), in burst mode only"),
        args.value = arg_dbl0(NULL, NULL, "<value>",
            "Set duty cycle in % (" STR(DUTY_CYCLE_MIN) "-" STR(DUTY_CYCLE_MAX) ")"),
        args.mode = arg_rint0("m", NULL, "<mode>", 0, 1, "Mode of operation. 0: spread, 1: burst"),
        arg_rem(NULL, "Default mode: spread"),
        args.omit_cr = arg_lit0("o", NULL, "Omit control responses from the duty cycle budget"));
    return 0;
}

static int duty_cycle_parse_cmd(const char *str)
{
    if (strcmp("enable", str) == 0)
        return DUTY_CYCLE_CMD_ENABLE;

    if (strcmp("disable", str) == 0)
        return DUTY_CYCLE_CMD_DISABLE;

    if (strcmp("airtime", str) == 0)
        return DUTY_CYCLE_CMD_AIRTIME;

    return -1;
}

static int get_duty_cycle(struct morsectrl *mors, bool burst_airtime_only)
{
    int ret = -1;
    struct morse_cmd_req_set_duty_cycle *req;
    struct morse_cmd_resp_get_duty_cycle *resp;
    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp));

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_duty_cycle);
    resp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_get_duty_cycle);

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_DUTY_CYCLE,
                                     req_tbuff, rsp_tbuff);

    if (ret < 0)
    {
        mctrl_err("Failed to read duty cycle\n");
        goto exit;
    }

    if (burst_airtime_only)
    {
        if (resp->config_ext.set.mode == MORSE_CMD_DUTY_CYCLE_MODE_BURST)
        {
            mctrl_print("%u\n", resp->config_ext.airtime_remaining_us);
        }
        else
        {
            mctrl_err("Command not supported when in spread mode\n");
            ret = -1;
        }

        goto exit;
    }

    mctrl_print("Mode: %s\n",
            (resp->config_ext.set.mode == MORSE_CMD_DUTY_CYCLE_MODE_BURST) ? "burst" : "spread");
    mctrl_print("Configured duty cycle: %.2f%%\n", (float)(le32toh(resp->config.duty_cycle)) / 100);
    mctrl_print("Control responses omitted from duty cycle calculation: %d\n",
            resp->config.omit_control_responses);

    if (resp->config_ext.set.mode == MORSE_CMD_DUTY_CYCLE_MODE_BURST)
    {
        mctrl_print("Airtime remaining (us): %u\n", resp->config_ext.airtime_remaining_us);
        mctrl_print("Burst window duration (us): %u\n",
                resp->config_ext.burst_window_duration_us);
    }

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

static int set_duty_cycle(struct morsectrl *mors, struct morse_cmd_duty_cycle_configuration *cfg,
                          struct morse_cmd_duty_cycle_set_configuration_ext *cfg_ext,
                          uint8_t set_cfgs)
{
    int ret = -1;
    struct morse_cmd_req_set_duty_cycle *req;
    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit_set;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_duty_cycle);

    memset(req, 0, sizeof(*req));

    /* Range checked by caller */
    req->set_cfgs = set_cfgs;
    req->config.duty_cycle = cfg->duty_cycle;
    req->config.omit_control_responses = cfg->omit_control_responses;

    if (req->set_cfgs & MORSE_CMD_DUTY_CYCLE_SET_CFG_EXT)
    {
        req->config_ext.mode = cfg_ext->mode;
        if (req->set_cfgs & MORSE_CMD_DUTY_CYCLE_SET_CFG_BURST_RECORD_UNIT)
        {
            req->config_ext.burst_record_unit_us = cfg_ext->burst_record_unit_us;
        }
    }

    /* Send duty cycle command directly to the firmware if driver commands are not supported. */
    if (morsectrl_transport_has_driver(mors->transport))
    {
        ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_DRIVER_SET_DUTY_CYCLE,
                                     req_tbuff, rsp_tbuff);
    }
    else
    {
        ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_DUTY_CYCLE,
                                     req_tbuff, rsp_tbuff);
    }

exit_set:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

int duty_cycle(struct morsectrl *mors, int argc, char *argv[])
{
    struct morse_cmd_duty_cycle_configuration cfg = { 0 };
    struct morse_cmd_duty_cycle_set_configuration_ext cfg_ext = { 0 };
    uint8_t set_cfgs = 0;

    if (args.enable->count == 0)
    {
        /* No command supplied, user wants to get duty cycle settings */
        return get_duty_cycle(mors, false);
    }

    int req = duty_cycle_parse_cmd(args.enable->sval[0]);

    if (req == DUTY_CYCLE_CMD_AIRTIME)
    {
        /* User want's to get airtime information */
        return get_duty_cycle(mors, true);
    }
    if (req == DUTY_CYCLE_CMD_ENABLE)
    {
        float duty_cycle;

        if (args.value->count == 0)
        {
            mm_print_missing_argument(&args.value->hdr);
            return -1;
        }

        /* Specify what is being set in this command */
        set_cfgs |= MORSE_CMD_DUTY_CYCLE_SET_CFG_DUTY_CYCLE;
        set_cfgs |= MORSE_CMD_DUTY_CYCLE_SET_CFG_EXT;

        /* Parse duty cycle settings */
        duty_cycle = args.value->dval[0];
        if ((duty_cycle < (float)DUTY_CYCLE_MIN) || (duty_cycle > (float)DUTY_CYCLE_MAX))
        {
            mctrl_err("Invalid duty cycle %f (%.2f-%.2f).\n",
                    duty_cycle, DUTY_CYCLE_MIN, DUTY_CYCLE_MAX);
            return -1;
        }

        cfg.duty_cycle = htole32(duty_cycle * 100);
        cfg_ext.mode = MORSE_CMD_DUTY_CYCLE_MODE_SPREAD; /* default mode */

        if (args.omit_cr->count)
        {
            cfg.omit_control_responses = 1;
            set_cfgs |= MORSE_CMD_DUTY_CYCLE_SET_CFG_OMIT_CONTROL_RESP;
        }

        if (args.mode->count)
        {
            cfg_ext.mode = args.mode->ival[0];
        }

    }
    else if (req == DUTY_CYCLE_CMD_DISABLE)
    {
        set_cfgs |= MORSE_CMD_DUTY_CYCLE_SET_CFG_DUTY_CYCLE;
        cfg.duty_cycle = htole32(100 * 100); /* 100% dc indicates disabled */
    }

    return set_duty_cycle(mors, &cfg, &cfg_ext, set_cfgs);
}

MM_CLI_HANDLER(duty_cycle, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
