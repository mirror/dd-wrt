/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "command.h"
#include "utilities.h"

static struct
{
    struct arg_rex *ps_mode;
} args;

int power_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Force chip into a specific power mode",
        args.ps_mode = arg_rex1(NULL, NULL, "(hibernate)", "hibernate", 0, "Power mode"),
        arg_rem(NULL, "Power mode 'hibernate' requires reset to recover the chip"));
    return 0;
}

int power(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    enum morse_cmd_power_mode mode;
    struct morse_cmd_req_force_power_mode *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    if (!strcmp(args.ps_mode->sval[0], "hibernate"))
    {
        mode = MORSE_CMD_POWER_MODE_HIBERNATE;
    }
    else
    {
        mctrl_err("Invalid power mode '%s'\n", args.ps_mode->sval[0]);
        return -1;
    }

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_force_power_mode);
    req->mode = htole32(mode);
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_FORCE_POWER_MODE,
                                 req_tbuff, rsp_tbuff);
exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(power, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
