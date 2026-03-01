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

static struct {
    struct arg_int *bytes;
    struct arg_lit *reset;
} args;

int maxampdulen_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set the max A-MPDU length",
        args.bytes = arg_int0(NULL, NULL, "<bytes>", "Maximum allowable A-MPDU length in bytes"),
        args.reset = arg_lit0("r", NULL, "Reset to chip default"));
    return 0;
}

int maxampdulen(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    int n_bytes = 0;
    struct morse_cmd_req_max_ampdu_length *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    if (args.bytes->count)
    {
        n_bytes = args.bytes->ival[0];
    }
    else if (args.reset->count)
    {
        n_bytes = -1;
    }
    else
    {
        mm_print_missing_argument(&args.bytes->hdr);
        mm_print_missing_argument(&args.reset->hdr);
        goto exit;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_max_ampdu_length);
    req->n_bytes = htole32(n_bytes);

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_MAX_AMPDU_LENGTH,
                                 req_tbuff, rsp_tbuff);
exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(maxampdulen, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
