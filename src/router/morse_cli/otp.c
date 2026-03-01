/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "command.h"
#include "morse_commands.h"
#include "utilities.h"

static struct
{
    struct arg_int *bank_num;
} args;

int otp_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
#define OTP_ARGTABLE_DESC "Read OTP bank"
#define OTP_BANK_NUM_DESC "Bank number to read from"

    MM_INIT_ARGTABLE(mm_args, OTP_ARGTABLE_DESC,
        args.bank_num = arg_rint1(NULL, NULL, "<bank num>", 0, UINT8_MAX, OTP_BANK_NUM_DESC)
    ); /* NOLINT (whitespace/parens) */
    return 0;
}

int otp(struct morsectrl *mors, int argc, char *argv[])
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
    req->bank_region = MORSE_CMD_OTP_REGION_ALL_BANK;


    req->bank_num = args.bank_num->ival[0];

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_OTP,
                                 req_tbuff, rsp_tbuff);

exit:
    if (ret == 0 && !req->write_otp)
        mctrl_print("OTP Bank %d: 0x%08x\n", args.bank_num->ival[0], resp->bank_val);

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(otp, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
