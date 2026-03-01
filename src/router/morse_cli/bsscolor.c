/*
 * Copyright 2020 Morse Micro
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

static struct
{
    struct arg_int *bsscolor;
} args;

int bsscolor_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set BSS color",
        args.bsscolor = arg_rint1(NULL, NULL, "<color>", 0, 7, "BSS color (0-7)"));
    return 0;
}

int bsscolor(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint32_t color;
    struct morse_cmd_req_set_bss_color *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(0));

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_bss_color);

    color = args.bsscolor->ival[0];

    req->bss_color = color;
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_BSS_COLOR,
                                 req_tbuff, rsp_tbuff);
exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(bsscolor, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
