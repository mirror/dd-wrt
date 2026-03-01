/*
 * Copyright 2025 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "command.h"
#include "utilities.h"


int boardtype_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
#define BOARDTYPE_ARGTABLE_DESC "Read board type OTP bank"

    MM_INIT_ARGTABLE(mm_args, BOARDTYPE_ARGTABLE_DESC
    ); /* NOLINT (whitespace/parens) */
    return 0;
}

int boardtype(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;

    struct morse_cmd_req_otp *req;
    struct morse_cmd_resp_otp *resp;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp));

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_otp);
    resp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_otp);
    req->write_otp = 0;


    req->bank_region = MORSE_CMD_OTP_REGION_BOARDTYPE;
    req->bank_num = UINT8_MAX;

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_OTP,
                                 req_tbuff, rsp_tbuff);

exit:
    if (ret == 0 && !req->write_otp)
    {
        if (!resp->bank_val)
            mctrl_print("Board type is not set\n");
        else
            mctrl_print("0x%x\n", resp->bank_val);
    }

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(boardtype, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
