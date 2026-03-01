/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

/*
 * Transport layer for communication over TCP socket to an embedded device with SLIP framing.
 *
 * Transport frame format:
 *
 *          +-----------------------------------------+-----------+-----------+
 *          |       Command/Response Payload          |   Seq #   |   CRC16   |
 *          +-----------------------------------------+-----------+-----------+
 *
 * * Seq # is used to match command to response. The content is arbitrary and the response will
 *   echoed the value provided in the command.
 * * The command and response payload are opaque to this layer.
 * * CRC16 is a CRC16 calculated over the sequence # and payload. See below for implementation
 *   details.
 *
 * The above frame is then slip encoded before transmission over the TCP socket. On the receive
 * side it is SLIP decoded before the CRC16 is validated and sequence # checked.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "transport.h"
#include "transport_private.h"
#include "../utilities.h"

#include "slip.h"

#define SEQNUM_LEN                  (4)
#define CRC_LEN                     (2)

static const struct morsectrl_transport_ops tcp_slip_ops;

struct tcp_slip_data
{
    char hostname[256];
    int port;
    int socketfd;
};

/** @brief Data structure used to represent an instance of this trasport. */
struct morsectrl_tcp_slip_transport
{
    struct morsectrl_transport common;
    struct tcp_slip_data tcp_slip;
};

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *        tcp_slip field.
 */
static struct tcp_slip_data *tcp_slip_get_data(struct morsectrl_transport *transport)
{
    struct morsectrl_tcp_slip_transport *tcp_slip_transport =
        (struct morsectrl_tcp_slip_transport *)transport;
    return &tcp_slip_transport->tcp_slip;
}


/**
 * @brief Prints an error message if possible.
 *
 * @param transport     Transport to print the error message from.
 * @param error_code    Error code.
 * @param error_msg     Error message.
 */
static void tcp_slip_error(int error_code, char *error_msg)
{
    morsectrl_transport_err("TCP_SLIP", error_code, error_msg);
}

/**
 * @brief Parse the configuration for the SLIP over TCP socket.
 *
 * @param transport     The transport structure.
 * @param debug         Indicates whether debug print statements are enabled.
 * @param iface_opts    String containing the interface to use. May be NULL.
 * @param cfg_opts      Comma separated string with SLIP over TCP configuration options.
 * @return              0 on success otherwise relevant error.
 */
static int tcp_slip_parse(struct morsectrl_transport **transport,
                           bool debug,
                           const char *iface_opts,
                           const char *cfg_opts)
{
    struct tcp_slip_data *tcp_slip_data;
    const char *hostname_end;
    size_t hostname_len;

    struct morsectrl_tcp_slip_transport *tcp_slip_transport =
        calloc(1, sizeof(*tcp_slip_transport));
    if (!tcp_slip_transport)
    {
        mctrl_err("Transport memory allocation failure\n");
        return -ETRANSNOMEM;
    }

    tcp_slip_transport->common.debug = debug;
    tcp_slip_transport->common.tops = &tcp_slip_ops;
    *transport = &tcp_slip_transport->common;

    if (cfg_opts == NULL || strlen(cfg_opts) == 0)
    {
        goto config_syntax_error;
    }

    tcp_slip_data = tcp_slip_get_data(*transport);
    hostname_end = strchr(cfg_opts, ':');
    if (hostname_end == NULL)
    {
        goto config_syntax_error;
    }

    hostname_len = hostname_end - cfg_opts;
    if (hostname_len > sizeof(tcp_slip_data->hostname) - 1)
    {
        mctrl_err("Hostname too long (max supported %u chars)\n",
                  sizeof(tcp_slip_data->hostname) - 1);
        return -ETRANSNOMEM;
    }

    memcpy(tcp_slip_data->hostname, cfg_opts, hostname_len);
    tcp_slip_data->hostname[hostname_len] = '\0';
    tcp_slip_data->port = atoi(hostname_end + 1);

    if (tcp_slip_data->port == 0)
    {
        goto config_syntax_error;
    }

    return 0;

config_syntax_error:
    mctrl_err("Must specify the TCP socket to connect to: -c <hostname>:<port>\n");
    return -ETRANSNOMEM;
}


/**
 * @brief Initalise an SLIP over TCP socket.
 *
 * @note This should be done after parsing the configuration.
 *
 * @param transport Transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int tcp_slip_init(struct morsectrl_transport *transport)
{
    struct tcp_slip_data *tcp_slip_data = tcp_slip_get_data(transport);
    int ret;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    char port_buf[8];
    struct addrinfo *addrinfo;
    struct addrinfo *addrinfo_iter;

    srand(time(NULL));

    snprintf(port_buf, sizeof(port_buf), "%d", tcp_slip_data->port);
    ret = getaddrinfo(tcp_slip_data->hostname, port_buf, &hints, &addrinfo);
    if (ret != 0)
    {
        mctrl_err("Failed to resolve (%s:%s)\n", tcp_slip_data->hostname, port_buf);
        return -ETRANSERR;
    }

    for (addrinfo_iter = addrinfo; addrinfo_iter != NULL; addrinfo_iter = addrinfo_iter->ai_next)
    {
        int sockfd;

        ret = socket(addrinfo_iter->ai_family, addrinfo_iter->ai_socktype,
                     addrinfo_iter->ai_protocol);
        if (ret < 0)
        {
            continue;
        }

        sockfd = ret;

        ret = connect(sockfd, addrinfo_iter->ai_addr, addrinfo_iter->ai_addrlen);
        if (ret == 0)
        {
            /* Success */
            tcp_slip_data->socketfd = sockfd;
            break;
        }

        close(sockfd);
    }

    freeaddrinfo(addrinfo);

    if (ret != 0)
    {
        mctrl_err("Failed to connect to %s:%s\n", tcp_slip_data->hostname, port_buf);
        return -ETRANSERR;
    }

    return ETRANSSUCC;
}

/**
 * @brief De-initalise an FTDI Transport.
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int tcp_slip_deinit(struct morsectrl_transport *transport)
{
    struct tcp_slip_data *tcp_slip_data = tcp_slip_get_data(transport);

    if (tcp_slip_data->socketfd > 0)
    {
        close(tcp_slip_data->socketfd);
    }

    return 0;
}


/**
 * @brief Allocate @ref morsectrl_transport_buff.
 *
 * @param transport Transport structure.
 * @param size      Size of command and morse headers or raw data.
 * @return          Allocated @ref morsectrl_transport_buff or NULL on failure.
 */
static struct morsectrl_transport_buff *tcp_slip_alloc(struct morsectrl_transport *transport,
                                                        size_t size)
{
    struct morsectrl_transport_buff *buff;

    if (!transport)
        return NULL;

    if (size <= 0)
        return NULL;

    buff = calloc(1, sizeof(*buff));
    if (!buff)
        return NULL;

    buff->capacity = size + SEQNUM_LEN + CRC_LEN;
    buff->memblock = calloc(1, buff->capacity);
    if (!buff->memblock)
    {
        free(buff);
        return NULL;
    }
    buff->data = buff->memblock;
    buff->data_len = size;

    return buff;
}

static int tcp_slip_tx_char(uint8_t c, void *arg)
{
    int ret;
    struct morsectrl_transport *transport = arg;
    struct tcp_slip_data *tcp_slip_data = tcp_slip_get_data(transport);

    ret = write(tcp_slip_data->socketfd, &c, 1);
    if (ret == 1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static int tcp_slip_send(struct morsectrl_transport *transport,
                         struct morsectrl_transport_buff *req,
                         struct morsectrl_transport_buff *resp)
{
    struct tcp_slip_data *tcp_slip_data;
    int ret = -ETRANSERR;
    int i;
    uint8_t *cmd_seq_num_field;
    uint8_t *rsp_seq_num_field;
    uint8_t *crc_field;
    uint16_t crc;
    size_t original_cmd_data_len;
    struct slip_rx_state slip_rx_state = SLIP_RX_STATE_INIT(resp->data, resp->capacity);
    enum slip_rx_status slip_rx_status = SLIP_RX_IN_PROGRESS;

    if (!transport || !transport->tops || !req || !resp)
    {
        return -ETRANSERR;
    }

    tcp_slip_data = tcp_slip_get_data(transport);

    /* We need to restore the data_len field before the function returns, so we stash the
     * value here. */
    original_cmd_data_len = req->data_len;

    /* Append random sequence number */
    cmd_seq_num_field = req->data + req->data_len;
    req->data_len += SEQNUM_LEN;
    MCTRL_ASSERT(req->data_len <= req->capacity, "Tx buffer insufficient (%u < %u)",
                 req->capacity, req->data_len);
    for (i = 0; i < SEQNUM_LEN; i++)
    {
        /* NOLINTNEXTLINE(runtime/threadsafe_fn)*/
        cmd_seq_num_field[i] = rand();
    }

    /* Append CRC16 */
    crc = crc16_gen(req->data, req->data_len);
    crc_field = req->data + req->data_len;
    req->data_len += CRC_LEN;
    MCTRL_ASSERT(req->data_len <= req->capacity, "Tx buffer insufficient (%u < %u)",
                 req->capacity, req->data_len);
    crc_field[0] = crc & 0x0ff;
    crc_field[1] = (crc >> 8) & 0x0ff;

    /* Slip encode and transmit the packet */
    ret = slip_tx(tcp_slip_tx_char, transport, req->data, req->data_len);
    req->data_len = original_cmd_data_len;

    if (ret != 0)
    {
        tcp_slip_error(ret, "Failed to send command");
        goto fail;
    }

    resp->data_len = 0;

    while (true)
    {
        slip_rx_state_reset(&slip_rx_state);
        do
        {
            uint8_t rx_char;
            ret = read(tcp_slip_data->socketfd, &rx_char, 1);
            if (ret < 0)
            {
                tcp_slip_error(ret, "Failed to rx command");
                goto fail;
            }
            else if (ret == 0)
            {
                continue;
            }

            slip_rx_status = slip_rx(&slip_rx_state, rx_char);
        } while (slip_rx_status == SLIP_RX_IN_PROGRESS);

        if (slip_rx_status != SLIP_RX_COMPLETE)
        {
            if (slip_rx_status == SLIP_RX_BUFFER_LIMIT)
            {
                tcp_slip_error(-ETRANSERR, "Response exceeded allocated buffer");
            }
            tcp_slip_error(-ETRANSERR, "Slip RX transfer incomplete");
            ret = -ETRANSERR;
            goto fail;
        }

        resp->data_len = slip_rx_state.length;
        if (resp->data_len < SEQNUM_LEN + CRC_LEN)
        {
            if (resp->data_len > 0)
            {
                tcp_slip_error(-ETRANSERR, "Received frame too short. Ignoring it...");
            }
            continue;
        }

        /* Remove and validate CRC */
        resp->data_len -= CRC_LEN;
        crc = crc16_gen(resp->data, resp->data_len);
        crc_field = resp->data + resp->data_len;
        if ((crc_field[0] != (crc & 0xff)) || crc_field[1] != ((crc >> 8) & 0xff))
        {
            tcp_slip_error(-ETRANSERR, "CRC error for received frame. Ignoring it...");
            continue;
        }

        /* Remove and validate sequence number */
        resp->data_len -= SEQNUM_LEN;
        rsp_seq_num_field = resp->data + resp->data_len;

        if (memcmp(cmd_seq_num_field, rsp_seq_num_field, SEQNUM_LEN) != 0)
        {
            tcp_slip_error(-ETRANSERR, "Seq # incorrect for received frame. Ignoring it...");
            continue;
        }

        return ETRANSSUCC;
    }
fail:
    return ret;
}

static const struct morsectrl_transport_ops tcp_slip_ops = {
    .name = "tcp_slip",
    .description = "Tunnel commands over a TCP stream using SLIP framing",
    .has_reset = false,
    .has_driver = false,
    .parse = tcp_slip_parse,
    .init = tcp_slip_init,
    .deinit = tcp_slip_deinit,
    .write_alloc = tcp_slip_alloc,
    .read_alloc = tcp_slip_alloc,
    .send = tcp_slip_send,
    .reg_read = NULL,
    .reg_write = NULL,
    .mem_read = NULL,
    .mem_write = NULL,
    .raw_read = NULL,
    .raw_write = NULL,
    .raw_read_write = NULL,
    .reset_device = NULL,
    .get_ifname = NULL,
};

REGISTER_TRANSPORT(tcp_slip_ops);
