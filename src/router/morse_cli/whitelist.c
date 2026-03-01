/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "portable_endian.h"
#if defined(__WINDOWS__)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include "command.h"
#include "transport/transport.h"

#define IPADDR_STR_LEN (16)

#define WHITELIST_PARAM_PORT_MAX (65535)

static struct
{
    struct arg_lit *clear;
    struct arg_int *llc_protocol;
    struct arg_int *ip_protocol;
    struct arg_str *src_ip;
    struct arg_str *dest_ip;
    struct arg_str *netmask;
    struct arg_int *src_port;
    struct arg_int *dest_port;
} args;

int whitelist_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Configure the packet whitelist filter",
        args.llc_protocol = arg_int0("l", NULL, "<LLC proto>",
            "Link layer protocol - e.g. 0x0800 for IPv4"),
        args.ip_protocol = arg_int0("i", NULL, "<IPv4 proto>",
            "IPv4 protocol - e.g. 6 for TCP or 17 for UDP"),
        args.src_ip = arg_str0("s", NULL, "<src IP>",
            "Source IP address in dotted decimal notation"),
        args.dest_ip = arg_str0("d", NULL, "<dest IP>",
            "Destination IP address in dotted decimal notation"),
        args.netmask = arg_str0("n", NULL, "<netmask>",
            "Netmask in dotted decimal notation"),
        args.src_port = arg_int0("S", NULL, "<src port>",
            "UDP or TCP source port - range 1-65535"),
        args.dest_port = arg_int0("D", NULL, "<dest port>",
            "UDP or TCP destination port - range 1-65535"),
        args.clear = arg_lit0("c", NULL,
            "Clear all whitelist entries"));
    return 0;
}

int whitelist(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_req_set_whitelist *req;
    int add_arg_count = 0;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
    {
        goto exit;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_whitelist);
    memset(req, 0, sizeof(*req));

    add_arg_count = args.llc_protocol->count +
            args.ip_protocol->count +
            args.src_ip->count +
            args.dest_ip->count +
            args.netmask->count +
            args.src_port->count +
            args.dest_port->count;

    if (args.clear->count == 1)
    {
        if (add_arg_count != 0)
        {
            mctrl_err("Invalid parameters specified for Clear operation\n");
            goto exit;
        }
        req->flags |= MORSE_CMD_WHITELIST_FLAGS_CLEAR;
    }
    else
    {
        if (add_arg_count == 0)
        {
            mctrl_err("No filter parameters specified\n");
            goto exit;
        }
    }

    if (args.llc_protocol->count)
    {
        req->llc_protocol = htobe16(args.llc_protocol->ival[0]);
    }

    if (args.ip_protocol->count)
    {
        req->ip_protocol = args.ip_protocol->ival[0];
    }

    if (args.src_ip->count)
    {
        if (inet_pton(AF_INET, args.src_ip->sval[0], &req->src_ip) != 1)
        {
            mctrl_err("Invalid source IP address %s\n", args.src_ip->sval[0]);
            goto exit;
        }
    }

    if (args.dest_ip->count)
    {
        if (inet_pton(AF_INET, args.dest_ip->sval[0], &req->dest_ip) != 1)
        {
            mctrl_err("Invalid destination IP address %s\n", args.dest_ip->sval[0]);
            goto exit;
        }
    }

    if (args.netmask->count)
    {
        if (inet_pton(AF_INET, args.netmask->sval[0], &req->netmask) != 1)
        {
            mctrl_err("Invalid netmask %s\n", args.netmask->sval[0]);
            goto exit;
        }
        if (args.src_ip->count == 0 && args.dest_ip->count == 0)
        {
                mctrl_err("Netmask provided without source or destination IP address\n");
                goto exit;
        }
        if (req->src_ip && ((req->src_ip & req->netmask) != req->src_ip))
        {
                mctrl_err("Netmask is invalid for source IP address\n");
                goto exit;
        }
        if (req->dest_ip && ((req->dest_ip & req->netmask) != req->dest_ip))
        {
                mctrl_err("Netmask is invalid for destination IP address\n");
                goto exit;
        }
    }

    if (args.src_port->count)
    {
        if (args.src_port->ival[0] > WHITELIST_PARAM_PORT_MAX)
        {
            mctrl_err("Invalid source port %d\n", args.src_port->ival[0]);
            goto exit;
        }
        req->src_port = htobe16(args.src_port->ival[0]);
    }

    if (args.dest_port->count)
    {
        if (args.dest_port->ival[0] > WHITELIST_PARAM_PORT_MAX)
        {
            mctrl_err("Invalid destination port %d\n", args.dest_port->ival[0]);
            goto exit;
        }
        req->dest_port = htobe16(args.dest_port->ival[0]);
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_WHITELIST,
                                 req_tbuff, rsp_tbuff);
exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER(whitelist, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
