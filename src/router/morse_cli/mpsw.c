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

/** No upper bound value for airtime duration */
#define AIRTIME_UNLIMITED 0

#define NUM_BOUNDS_VALUES 2

static struct {
    struct arg_csi *bounds;
    struct arg_int *len;
    struct arg_rex *enable;
} args;

int mpsw_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(
        mm_args, "Get (default) or set Minimum Packet Spacing Window parameters",
        args.bounds = arg_csi0("b", NULL, "<low usecs>,<high usecs>", NUM_BOUNDS_VALUES,
            "Min required/max allowable packet airtime duration to trigger spacing"),
        args.len = arg_int0("w", NULL, "<length>",
            "Length of time to close the TX window between packets"),
        args.enable = arg_rex0("e", NULL, "(0|1)", "{0|1}", 0,
            "Enable airtime bounds checking and packet spacing enforcement"));
    return 0;
}

static void print_mpsw_cfg(struct morse_cmd_mpsw_configuration *cfg)
{
    mctrl_print("                 MPSW Active: %d\n", cfg->enable);
    mctrl_print("       Airtime Minimum Bound: %u\n", cfg->airtime_min_us);
    mctrl_print("       Airtime Maximum Bound: %u\n", cfg->airtime_max_us);
    mctrl_print("Packet Spacing Window Length: %u\n", cfg->packet_space_window_length_us);
}

int mpsw(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;

    struct morse_cmd_req_mpsw_config *req_mpsw;
    struct morse_cmd_resp_mpsw_config *rsp_mpsw;
    struct morsectrl_transport_buff *req_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    uint32_t airtime_min_us;
    uint32_t airtime_max_us;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req_mpsw));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp_mpsw));
    if (!req_tbuff || !rsp_tbuff)
    {
        ret = -1;
        goto exit;
    }

    req_mpsw = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_mpsw_config);
    rsp_mpsw = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_mpsw_config);

    if (req_mpsw == NULL ||
        rsp_mpsw == NULL)
    {
        goto exit;
    }

    memset(req_mpsw, 0, sizeof(*req_mpsw));

    if (args.bounds->count)
    {
        airtime_min_us = args.bounds->ival[0][0];
        airtime_max_us = args.bounds->ival[0][1];

        if (((airtime_min_us > airtime_max_us) &&
             (airtime_max_us != AIRTIME_UNLIMITED)) ||
             (airtime_min_us == airtime_max_us))
        {
            mctrl_err(
                "airtime min (%d) must be less than airtime max (%d), or airtime max must be %d\n",
                airtime_min_us, airtime_max_us,
                AIRTIME_UNLIMITED);
            goto exit;
        }

        req_mpsw->set_cfgs |= MORSE_CMD_SET_MPSW_CFG_AIRTIME_BOUNDS;
        req_mpsw->config.airtime_min_us = htole32(airtime_min_us);
        req_mpsw->config.airtime_max_us = htole32(airtime_max_us);
    }

    if (args.len->count)
    {
        req_mpsw->set_cfgs |= MORSE_CMD_SET_MPSW_CFG_PKT_SPC_WIN_LEN;
        req_mpsw->config.packet_space_window_length_us = htole32(args.len->ival[0]);
    }

    if (args.enable->count)
    {
        req_mpsw->set_cfgs |= MORSE_CMD_SET_MPSW_CFG_ENABLED;
        req_mpsw->config.enable = expression_to_int(args.enable->sval[0]);
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_MPSW_CONFIG,
                                 req_tbuff,
                                 rsp_tbuff);

exit:
    if (!ret)
    {
        print_mpsw_cfg(&rsp_mpsw->config);
    }

    morsectrl_transport_buff_free(req_tbuff);

    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER(mpsw, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
