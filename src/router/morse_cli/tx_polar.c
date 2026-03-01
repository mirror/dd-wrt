/*
 * Copyright 2021 Morse Micro
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
#include "morse_commands.h"
#include "utilities.h"

static struct
{
    struct arg_rex *enable;
} args;

int tx_polar_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Enable Tx Polar Mode",
        args.enable = arg_rex1(NULL, NULL, MM_ARGTABLE_ENABLE_REGEX, MM_ARGTABLE_ENABLE_DATATYPE, 0,
            "Enable/disable polar mode (default enabled)"));
    return 0;
}

int tx_polar(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    int enable;
    struct morse_cmd_req_tx_polar *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    enable = expression_to_int(args.enable->sval[0]);

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_tx_polar);
    req->enable = enable;
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_TX_POLAR,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(tx_polar, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
