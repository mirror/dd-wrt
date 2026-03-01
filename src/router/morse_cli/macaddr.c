/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <unistd.h>

#include "command.h"
#include "utilities.h"


int macaddr_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Read or write the chip MAC address"
            ); /* NOLINT (whitespace/parens) */
    return 0;
}


int macaddr(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_mac_addr *req;
    struct morse_cmd_resp_mac_addr *resp;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp));
    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_mac_addr);
    resp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_mac_addr);
    req->write = false;


    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_MAC_ADDR,
                                 req_tbuff, rsp_tbuff);
exit:
    if (!ret)
    {
        uint8_t *mac_octet = resp->octet;
        mctrl_print("Chip MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac_octet[0], mac_octet[1], mac_octet[2], mac_octet[3],
               mac_octet[4], mac_octet[5]);
    }
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(macaddr, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
