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

#include "command.h"
#include "utilities.h"

static struct
{
    struct arg_rex *macaddr;
    struct arg_str *payload;
} args;

int wakeaction_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Send a wake action frame to a destination",
        args.macaddr = arg_rex1(NULL, NULL, MAC_CMD_REGEX,
            "<MAC Address>", ARG_REX_ICASE, "Destination MAC address"),
        args.payload = arg_str1(NULL, NULL, "<payload>", "Hex string of payload to send"));
    return 0;
}

int wakeaction(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_send_wake_action_frame* req = NULL;
    struct morsectrl_transport_buff *req_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;
    int payload_size = 0;

    payload_size = strlen(args.payload->sval[0]);
    if ((payload_size % 2) != 0)
    {
        mctrl_err("Invalid hex string, length must be a multiple of 2\n");
        return -1;
    }

    payload_size = (payload_size / 2);
    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req) + payload_size);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_send_wake_action_frame);
    if (!req)
        goto exit;

    req->payload_size = htole32(payload_size);

    if (hexstr2bin(args.payload->sval[0], req->payload, payload_size) < 0)
    {
        mctrl_err("Invalid hex string\n");
        goto exit;
    }

    if (str_to_mac_addr(req->dest_addr, args.macaddr->sval[0]) < 0)
    {
        mctrl_err("Invalid MAC address - must be in the format aa:bb:cc:dd:ee:ff\n");
        goto exit;
    }

    ret = morsectrl_send_command(mors->transport,
        MORSE_CMD_ID_SEND_WAKE_ACTION_FRAME, req_tbuff, rsp_tbuff);

exit:
    if (req_tbuff)
        morsectrl_transport_buff_free(req_tbuff);

    if (rsp_tbuff)
        morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER(wakeaction, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
