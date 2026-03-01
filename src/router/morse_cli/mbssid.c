/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#ifndef MORSE_WIN_BUILD
#include <net/if.h>
#endif
#include "portable_endian.h"

#include "command.h"
#include "utilities.h"

#define BSS_MIN 0
#define BSS_MAX 2
#define BSS_ID_DEFAULT 0

static struct {
    struct arg_str *iface;
    struct arg_int *max_supported_bss;
} args;

int mbssid_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Advertise BSS in the beacons of another BSS",
        args.iface = arg_str1("t", NULL, "<transmitting BSS>",
            "Transmitting interface name, e.g. wlan0"),
        args.max_supported_bss = arg_rint1("m", NULL, "<max BSS ID>", BSS_MIN, BSS_MAX,
            "Maximum number of BSSs supported (" STR(BSS_MIN) "-" STR(BSS_MAX) ")"));
    return 0;
}

int mbssid(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint8_t max_bssid_indicator = BSS_ID_DEFAULT;
    struct morse_cmd_req_mbssid *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_mbssid);

    strncpy((char *)req->transmitter_iface, args.iface->sval[0],
            sizeof(req->transmitter_iface) - 1);

    max_bssid_indicator = args.max_supported_bss->ival[0];
    req->max_bssid_indicator = max_bssid_indicator;

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_MBSSID,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(mbssid, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
