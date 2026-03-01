/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include "command.h"
#include "utilities.h"

static struct arg_rex *medium_eva_arg;

int medium_eval_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Enable/disable medium evaluation mode",
                     medium_eva_arg = arg_rex1(NULL, NULL, MM_ARGTABLE_ENABLE_REGEX,
                         MM_ARGTABLE_ENABLE_DATATYPE, 0, "Enable/disable medium evaluation mode"));
    return 0;
}

int medium_eval(struct morsectrl *mors, int argc, char *argv[])
{
    int ret  = -1;
    int enabled;
    struct morse_cmd_req_medium_eval *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    enabled = expression_to_int(medium_eva_arg->sval[0]);

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_medium_eval);
    req->enable = enabled;
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_MEDIUM_EVAL,
                                 req_tbuff, rsp_tbuff);
exit:
    if (req_tbuff)
        morsectrl_transport_buff_free(req_tbuff);
    if (rsp_tbuff)
        morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(medium_eval, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
