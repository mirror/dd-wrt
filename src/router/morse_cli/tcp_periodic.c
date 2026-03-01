/*
 * Copyright 2025 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "portable_endian.h"
#if defined(__WINDOWS__)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include "command.h"
#include "utilities.h"
#include "transport/transport.h"

#define TCP_PERIODIC_PORT_MAX (65535)

static struct mm_argtable tcpp_config;
static struct mm_argtable tcpp_connect;
static struct mm_argtable tcpp_transmit;
static struct mm_argtable tcpp_disconnect;
static struct mm_argtable *subcmds[] =
{
    &tcpp_config, &tcpp_connect, &tcpp_transmit, &tcpp_disconnect
};

static struct {
    struct arg_str *ip;
    struct arg_str *netmask;
    struct arg_str *gateway;
    struct arg_rex *gateway_mac;
    struct arg_int *periodicity;
    struct arg_int *evt_on_disconnect;
    struct arg_str *payload;
} config_args;

static struct {
    struct arg_str *remote_ip;
    struct arg_int *remote_port;
    struct arg_rex *remote_mac;
} connect_args;

static struct {
    struct arg_rex *command;
} args;

static enum morse_cmd_tcp_periodic_subcmd tcp_periodic_get_cmd(const char str[])
{
    if (strcmp("config", str) == 0) return MORSE_CMD_TCP_PERIODIC_SUBCMD_CONFIG;
    else if (strcmp("connect", str) == 0) return MORSE_CMD_TCP_PERIODIC_SUBCMD_CONNECT;
    else if (strcmp("transmit", str) == 0) return MORSE_CMD_TCP_PERIODIC_SUBCMD_TX;
    else if (strcmp("disconnect", str) == 0) return MORSE_CMD_TCP_PERIODIC_SUBCMD_DISCONNECT;
    else
    {
        return -1;
    }
}

int tcp_periodic_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Configure and control periodic TCP transmit",
        args.command = arg_rex1(NULL, NULL, "(config|connect|transmit|disconnect)",
            "{config|connect|transmit|disconnect}", 0, "Periodic TCP subcommand"));
    args.command->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(&tcpp_config, "Configure behaviour of periodic TCP transmit",
        config_args.ip = arg_str0("s", NULL, "<IP of local interface>",
            "IP address of local interface in dotted decimal notation"),
        config_args.netmask = arg_str0("n", NULL, "<netmask>",
            "Netmask in dotted decimal notation"),
        config_args.gateway = arg_str0("g", NULL, "<gateway>",
            "Gateway of local interface in dotted decimal notation"),
        config_args.gateway_mac = arg_rex0("m", "gateway-mac", MAC_CMD_REGEX,
            "<gateway mac address>", ARG_REX_ICASE, "MAC address of gateway"),
        config_args.periodicity = arg_rint0("t", NULL, "<transmit periodicity>",
            0, INT_MAX, "Transmit periodicity of TCP data (secs)"),
        config_args.payload = arg_str0("p", NULL, "<payload>", "Hex string of payload to transmit"),
        config_args.evt_on_disconnect = arg_rint0("d", "event-on-disconnect", NULL,
            0, 1, "Chip will generate an event on socket closure/disconnect"));

    MM_INIT_ARGTABLE(&tcpp_connect, "Connect to a remote tcp socket",
        connect_args.remote_ip = arg_str1(NULL, NULL, "<remote IP address>",
            "Remote IP address in dotted decimal notation"),
        connect_args.remote_port = arg_rint1(NULL, NULL, "<remote port>",
            0, TCP_PERIODIC_PORT_MAX, "Remote port of TCP socket"),
        connect_args.remote_mac = arg_rex0("r", "remote-mac", MAC_CMD_REGEX,
            "<remote mac address>", ARG_REX_ICASE, "MAC address of remote IP address"));

    MM_INIT_ARGTABLE(&tcpp_transmit, "Transmit payload now (resets periodicity timer)");

    MM_INIT_ARGTABLE(&tcpp_disconnect, "Disconnect and close TCP socket");

    return 0;
}

int tcp_periodic_help(void)
{
    mm_help_argtable("tcp_periodic config", &tcpp_config);
    mm_help_argtable("tcp_periodic connect", &tcpp_connect);
    mm_help_argtable("tcp_periodic transmit", &tcpp_transmit);
    mm_help_argtable("tcp_periodic disconnect", &tcpp_disconnect);
    return 0;
}

int net_ip_configure(struct morsectrl* mors)
{
    int ret = 0;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_req_net_ip *req;
    uint32_t conf_flags = 0;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
    {
        ret = -ENOMEM;
        goto exit;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_net_ip);
    memset(req, 0, sizeof(*req));

    if (config_args.ip->count)
    {
        if (inet_pton(AF_INET, config_args.ip->sval[0], &req->configure.ip) != 1)
        {
            mctrl_err("Invalid local IP address: %s\n", config_args.ip->sval[0]);
            ret = -EINVAL;
            goto exit;
        }

        conf_flags |= MORSE_CMD_NET_IP_CONFIG_VAL_IP;
    }

    if (config_args.netmask->count)
    {
        if (inet_pton(AF_INET, config_args.netmask->sval[0], &req->configure.netmask) != 1)
        {
            mctrl_err("Invalid netmask: %s\n", config_args.netmask->sval[0]);
            ret = -EINVAL;
            goto exit;
        }

        conf_flags |= MORSE_CMD_NET_IP_CONFIG_VAL_NETMASK;
    }

    if (config_args.gateway->count)
    {
        if (inet_pton(AF_INET, config_args.gateway->sval[0], &req->configure.gateway) != 1)
        {
            mctrl_err("Invalid gateway: %s\n", config_args.gateway->sval[0]);
            ret = -EINVAL;
            goto exit;
        }

        conf_flags |= MORSE_CMD_NET_IP_CONFIG_VAL_GATEWAY;
    }

    if (config_args.gateway_mac->count)
    {
        if (str_to_mac_addr(req->configure.gateway_mac, config_args.gateway_mac->sval[0]) < 0)
        {
            mctrl_err("Invalid MAC address: %s\n", config_args.gateway_mac->sval[0]);
            ret = -EINVAL;
            goto exit;
        }

        conf_flags |= MORSE_CMD_NET_IP_CONFIG_VAL_GATEWAY_MAC;
    }

    req->configure.flags |= htole32(conf_flags);
    req->sub_cmd = htole32(MORSE_CMD_NET_IP_SUBCMD_CONFIG);
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_NET_IP, req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

int tcp_periodic_configure(struct morsectrl* mors, int argc, char *argv[],
                           struct morsectrl_transport_buff **req_tbuff)
{
    int ret;
    int payload_len = 0;
    struct morse_cmd_req_tcp_periodic *req;
    uint32_t conf_flags = 0;

    ret = mm_parse_argtable("tcp_periodic config", &tcpp_config, argc, argv);
    if (ret)
        return ret;

    /* The payload in the config struct is variable in length - it is required for cmd request
     * allocation.
     */
    if (config_args.payload->count)
    {
        payload_len = strlen(config_args.payload->sval[0]);
        if ((payload_len % 2) != 0)
        {
            mctrl_err("Invalid payload hex string, length must be a multiple of 2\n");
            return ret;
        }
        payload_len = (payload_len / 2);
    }

    *req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req) + payload_len);
    if (!(*req_tbuff))
        return -ENOMEM;

    req = TBUFF_TO_REQ((*req_tbuff), struct morse_cmd_req_tcp_periodic);
    memset(req, 0, sizeof(*req));

    if (config_args.periodicity->count)
    {
        req->configure.periodicity_s = htole32(config_args.periodicity->ival[0]);
        conf_flags |= MORSE_CMD_TCP_PERIODIC_CONFIG_VAL_PERIODICITY;
    }

    if (config_args.evt_on_disconnect->count)
    {
        req->configure.evt_on_disconnect = htole32(config_args.evt_on_disconnect->ival[0]);
        conf_flags |= MORSE_CMD_TCP_PERIODIC_CONFIG_VAL_EVT_ON_DISCONNECT;
    }

    if (payload_len)
    {
        if (hexstr2bin(config_args.payload->sval[0], req->configure.payload, payload_len) < 0)
        {
            mctrl_err("Invalid hex string: %s\n", config_args.payload->sval[0]);
            return -EINVAL;
        }
        req->configure.payload_len = htole32(payload_len);
        conf_flags |= MORSE_CMD_TCP_PERIODIC_CONFIG_VAL_PAYLOAD;
    }

    if (config_args.ip->count ||
        config_args.netmask->count ||
        config_args.gateway->count ||
        config_args.gateway_mac->count)
    {
        /* Local endpoint configuration is handled by an orthogonal, separate command request
         * (NET_IP).
         */
        ret = net_ip_configure(mors);
        if (ret)
            return ret;
    }
    req->configure.flags |= htole32(conf_flags);
    req->sub_cmd = htole32(MORSE_CMD_TCP_PERIODIC_SUBCMD_CONFIG);
    return 0;
}

int tcp_periodic_connect(struct morsectrl* mors, int argc, char *argv[],
                         struct morsectrl_transport_buff **req_tbuff)
{
    int ret;
    struct morse_cmd_req_tcp_periodic *req;

    ret = mm_parse_argtable("tcp_periodic connect", &tcpp_connect, argc, argv);
    if (ret)
        return ret;

    *req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    if (!(*req_tbuff))
        return -ENOMEM;

    req = TBUFF_TO_REQ((*req_tbuff), struct morse_cmd_req_tcp_periodic);
    memset(req, 0, sizeof(*req));

    if (inet_pton(AF_INET, connect_args.remote_ip->sval[0], &req->connect.remote_ip) != 1)
    {
        mctrl_err("Invalid remote IP address: %s\n", connect_args.remote_ip->sval[0]);
        return -EINVAL;
    }
    req->connect.remote_port = htobe16(connect_args.remote_port->ival[0]);

    if (connect_args.remote_mac->count)
    {
        if (str_to_mac_addr(req->connect.remote_mac, connect_args.remote_mac->sval[0]) < 0)
        {
            mctrl_err("Invalid MAC address: %s\n", connect_args.remote_mac->sval[0]);
            return -EINVAL;
        }
    }

    req->sub_cmd = htole32(MORSE_CMD_TCP_PERIODIC_SUBCMD_CONNECT);
    return 0;
}

int tcp_periodic_transmit(struct morsectrl* mors, int argc, char *argv[],
                          struct morsectrl_transport_buff **req_tbuff)
{
    int ret;
    struct morse_cmd_req_tcp_periodic *req;

    ret = mm_parse_argtable("tcp_periodic transmit", &tcpp_transmit, argc, argv);
    if (ret)
        return ret;

    *req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    if (!(*req_tbuff))
        return -ENOMEM;

    req = TBUFF_TO_REQ((*req_tbuff), struct morse_cmd_req_tcp_periodic);
    memset(req, 0, sizeof(*req));
    req->sub_cmd = htole32(MORSE_CMD_TCP_PERIODIC_SUBCMD_TX);
    return 0;
}

int tcp_periodic_disconnect(struct morsectrl* mors, int argc, char *argv[],
                            struct morsectrl_transport_buff **req_tbuff)
{
    int ret;
    struct morse_cmd_req_tcp_periodic *req;

    ret = mm_parse_argtable("tcp_periodic disconnect", &tcpp_disconnect, argc, argv);
    if (ret)
        return ret;

    *req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    if (!(*req_tbuff))
        return -ENOMEM;

    req = TBUFF_TO_REQ((*req_tbuff), struct morse_cmd_req_tcp_periodic);
    memset(req, 0, sizeof(*req));
    req->sub_cmd = htole32(MORSE_CMD_TCP_PERIODIC_SUBCMD_DISCONNECT);
    return 0;
}

int tcp_periodic(struct morsectrl *mors, int argc, char *argv[])
{
    int i;
    int ret = -1;
    struct morsectrl_transport_buff *req_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;
    enum morse_cmd_tcp_periodic_subcmd subcmd = tcp_periodic_get_cmd(args.command->sval[0]);

    /* Nothing carried in response */
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);
    if (!rsp_tbuff)
    {
        ret = -ENOMEM;
        goto exit;
    }

    switch (subcmd)
    {
        case MORSE_CMD_TCP_PERIODIC_SUBCMD_CONFIG:
        {
            ret = tcp_periodic_configure(mors, argc, argv, &req_tbuff);
            break;
        }
        case MORSE_CMD_TCP_PERIODIC_SUBCMD_CONNECT:
        {
            ret = tcp_periodic_connect(mors, argc, argv, &req_tbuff);
            break;
        }
        case MORSE_CMD_TCP_PERIODIC_SUBCMD_TX:
        {
            ret = tcp_periodic_transmit(mors, argc, argv, &req_tbuff);
            break;
        }
        case MORSE_CMD_TCP_PERIODIC_SUBCMD_DISCONNECT:
        {
            ret = tcp_periodic_disconnect(mors, argc, argv, &req_tbuff);
            break;
        }
        default:
        {
            mctrl_err("Unknown sub-command: %s\n", args.command->sval[0]);
            ret = -1;
            break;
        }
    }

    if (ret)
    {
        /* Failed to parse sub-command */
        goto exit;
    }

    /* Send command */
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_TCP_PERIODIC, req_tbuff, rsp_tbuff);

exit:
    /* Check if the reason we got here is because --help was given */
    if (mm_check_help_argtable(subcmds, MORSE_ARRAY_SIZE(subcmds)))
    {
        ret = 0;
    }

    for (i = 0; i < MORSE_ARRAY_SIZE(subcmds); i++)
    {
        mm_free_argtable(subcmds[i]);
    }

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER_CUSTOM_HELP(tcp_periodic, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
