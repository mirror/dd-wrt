/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "portable_endian.h"
#include "command.h"
#include "utilities.h"


static void print_error(enum morse_cmd_dhcp_retcode code)
{
    switch (code)
    {
        case MORSE_CMD_DHCP_RETCODE_NOT_ENABLED:
        {
            mctrl_err("DHCP client is not enabled\n");
            break;
        }
        case MORSE_CMD_DHCP_RETCODE_ALREADY_ENABLED:
        {
            mctrl_err("DHCP client is already enabled\n");
            break;
        }
        case MORSE_CMD_DHCP_RETCODE_NO_LEASE:
        {
            mctrl_err("DHCP client does not have a lease\n");
            break;
        }
        case MORSE_CMD_DHCP_RETCODE_HAVE_LEASE:
        {
            mctrl_err("DHCP client already has a lease\n");
            break;
        }
        case MORSE_CMD_DHCP_RETCODE_BUSY:
        {
            mctrl_err("DHCP client is currently performing a discovery or renewal\n");
            break;
        }

        default:
            mctrl_err("DHCP client threw an error: %d\n", code);
            break;
    }
}

static struct
{
    struct arg_rex *option;
} args;

int dhcpc_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Configure DHCP client offload",
        args.option = arg_rex1(NULL, NULL, "(enable|discover|get|clear|renew|rebind|update)",
             "{enable|discover|get|clear|renew|rebind|update}", 0, NULL),
        arg_rem("enable", "Enable DHCP client"),
        arg_rem("discover", "Do a discovery and obtain a lease"),
        arg_rem("get", "Get the current lease"),
        arg_rem("clear", "Clear the current lease"),
        arg_rem("renew", "Renew the current lease"),
        arg_rem("rebind", "Rebind the current lease"),
        arg_rem("update", "Send a lease update to the driver"));
    return 0;
}

int dhcpc(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = 0;
    ipv4_addr_t my_ip;
    ipv4_addr_t netmask;
    ipv4_addr_t router;
    ipv4_addr_t dns;

    struct morse_cmd_req_dhcp_offload *cmd_dhcp;
    struct morse_cmd_resp_dhcp_offload *rsp_dhcp;
    struct morsectrl_transport_buff *cmd_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*cmd_dhcp));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp_dhcp));
    if (!cmd_tbuff || !rsp_tbuff)
    {
        ret = -1;
        goto exit;
    }

    cmd_dhcp = TBUFF_TO_REQ(cmd_tbuff, struct morse_cmd_req_dhcp_offload);
    rsp_dhcp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_dhcp_offload);

    if (cmd_dhcp == NULL ||
        rsp_dhcp == NULL)
    {
        ret = -1;
        goto exit;
    }

    /* assume vif_id 0 */
    memset(cmd_dhcp, 0, sizeof(*cmd_dhcp));

    if (strcmp(args.option->sval[0], "enable") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_ENABLE);
    }
    else if (strcmp(args.option->sval[0], "discover") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_DO_DISCOVERY);
    }
    else if (strcmp(args.option->sval[0], "get") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_GET_LEASE);
    }
    else if (strcmp(args.option->sval[0], "clear") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_CLEAR_LEASE);
    }
    else if (strcmp(args.option->sval[0], "renew") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_RENEW_LEASE);
    }
    else if (strcmp(args.option->sval[0], "rebind") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_REBIND_LEASE);
    }
    else if (strcmp(args.option->sval[0], "update") == 0)
    {
        cmd_dhcp->opcode = htole32(MORSE_CMD_DHCP_OPCODE_SEND_LEASE_UPDATE);
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_DHCP_OFFLOAD,
                                 cmd_tbuff,
                                 rsp_tbuff);

    if (ret < 0)
    {
        goto exit;
    }
    else if (rsp_dhcp->retcode != MORSE_CMD_DHCP_RETCODE_SUCCESS)
    {
        print_error(le32toh(rsp_dhcp->retcode));
        goto exit;
    }
    else
    {
        if (le32toh(cmd_dhcp->opcode) == MORSE_CMD_DHCP_OPCODE_GET_LEASE)
        {
            my_ip.as_u32 = le32toh(rsp_dhcp->my_ip);
            netmask.as_u32 = le32toh(rsp_dhcp->netmask);
            router.as_u32 = le32toh(rsp_dhcp->router);
            dns.as_u32 = le32toh(rsp_dhcp->dns);
            mctrl_print("Current DHCP Lease\n");
            mctrl_print("IP Address: %d.%d.%d.%d\n", my_ip.octet[0],
                    my_ip.octet[1], my_ip.octet[2], my_ip.octet[3]);
            mctrl_print("Netmask: %d.%d.%d.%d\n", netmask.octet[0],
                    netmask.octet[1], netmask.octet[2],
                    netmask.octet[3]);
            mctrl_print("Router Address: %d.%d.%d.%d\n", router.octet[0],
                    router.octet[1], router.octet[2],
                    router.octet[3]);
            mctrl_print("DNS Address: %d.%d.%d.%d\n", dns.octet[0],
                    dns.octet[1], dns.octet[2], dns.octet[3]);
        }
    }

exit:
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER(dhcpc, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
