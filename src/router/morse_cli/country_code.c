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

#define COUNTRY_CODE_BANK_VAL_NOT_SET   (0x04040)


int country_code_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
#define COUNTRY_CODE_ARGTABLE_DESC "Read country code OTP bank"

    MM_INIT_ARGTABLE(mm_args, COUNTRY_CODE_ARGTABLE_DESC
    ); /* NOLINT (whitespace/parens) */
    return 0;
}

int country_code(struct morsectrl *mors, int argc, char *argv[])
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


    req->bank_region = MORSE_CMD_OTP_REGION_COUNTRY;
    req->bank_num = UINT8_MAX;

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_OTP,
                                 req_tbuff, rsp_tbuff);

exit:
    if (ret == 0 && !req->write_otp)
    {
        uint32_t bank_val = le32toh(resp->bank_val);
        if (!bank_val || bank_val == COUNTRY_CODE_BANK_VAL_NOT_SET)
            mctrl_print("Country code is not set\n");
        else
            mctrl_print("%c%c\n", (uint8_t) bank_val, (uint8_t)(bank_val >> 8));
    }

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(country_code, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
