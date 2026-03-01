/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "command.h"
#include "utilities.h"

struct
{
    struct arg_rex *enable;
} args;

int long_sleep_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Configure long sleep mode (allow sleeping through DTIM)",
        args.enable = arg_rex1(NULL, NULL, MM_ARGTABLE_ENABLE_REGEX, MM_ARGTABLE_ENABLE_DATATYPE, 0,
            "Enable/disable long sleep mode"));
    return 0;
}


int long_sleep(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    int enabled;
    struct morse_cmd_req_set_long_sleep_config *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    enabled = expression_to_int(args.enable->sval[0]);

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_long_sleep_config);
    req->enabled = enabled;

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_LONG_SLEEP_CONFIG,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(long_sleep, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
