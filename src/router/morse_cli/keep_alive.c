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

static struct
{
    struct arg_int *idle_period;
    struct arg_lit *dot11_spec;
} args;

int keepalive_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set the BSS max idle period",
        args.idle_period = arg_int1(NULL, NULL, "<period>",
            "BSS idle period (1000 TUs) after which a keepalive will be sent"),
        args.dot11_spec = arg_lit0("a", NULL, "Interpret idle period as per IEEE802.11ah spec"));
    return 0;
}

int keepalive(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_set_keep_alive_offload *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    uint16_t bss_max_idle_period = args.idle_period->ival[0];

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_keep_alive_offload);

    req->interpret_as_11ah = (args.dot11_spec->count > 0);

    req->bss_max_idle_period = htole16(bss_max_idle_period);

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_KEEP_ALIVE_OFFLOAD,
        req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(keepalive, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
