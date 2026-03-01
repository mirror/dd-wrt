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

#define GLOBAL_OP_CLASS_MIN 64
#define GLOBAL_OP_CLASS_MAX 77

static struct
{
    struct arg_int *s1g_op_class;
    struct arg_int *prim_chan_global;
} args;

int opclass_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set S1G operating class for S1G operation element",
    args.s1g_op_class = arg_int1(NULL, NULL, "<S1G opclass>", "S1G operating class"),
    args.prim_chan_global = arg_rint0("l", NULL, NULL, GLOBAL_OP_CLASS_MIN, GLOBAL_OP_CLASS_MAX,
        "Global operating class for primary channel"));
    return 0;
}

int opclass(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_set_s1g_op_class *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_s1g_op_class);

    req->opclass = args.s1g_op_class->ival[0];

    if (args.prim_chan_global->count)
    {
        req->prim_opclass = args.prim_chan_global->ival[0];
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_S1G_OP_CLASS,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(opclass, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
