/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdlib.h>
#include <stdio.h>

#include "command.h"
#include "utilities.h"

/* Lifetime packet expiry in us */
#define TX_PACKET_EXPIRY_MIN_US 50000
#define TX_PACKET_EXPIRY_MAX_US 500000

static struct
{
    struct arg_int *lifetime;
} args;

int tx_pkt_lifetime_us_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set the TX packet lifetime expiry",
        args.lifetime = arg_rint1(NULL, NULL, NULL, TX_PACKET_EXPIRY_MIN_US,
        TX_PACKET_EXPIRY_MAX_US, "TX packet expiry (usecs): "
        STR(TX_PACKET_EXPIRY_MIN_US) "-" STR(TX_PACKET_EXPIRY_MAX_US)));
    return 0;
}

int tx_pkt_lifetime_us(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_set_tx_pkt_lifetime_usecs *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    uint32_t lifetime_us;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_tx_pkt_lifetime_usecs);

    lifetime_us = args.lifetime->ival[0];
    req->lifetime_usecs = htole32(lifetime_us);

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_TX_PKT_LIFETIME_USECS,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(tx_pkt_lifetime_us, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
