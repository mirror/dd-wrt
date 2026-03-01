/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

/*
 * Transport layer for communication over UART interface to an embedded device with SLIP framing.
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
 * The above frame is then slip encoded before transmission over the UART. On the receive
 * side it is SLIP decoded before the CRC16 is validated and sequence # checked.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "transport.h"
#include "transport_private.h"
#include "../utilities.h"

#include "slip.h"
#include "uart.h"

#define DEFAULT_BAUDRATE            (115200)

#define SEQNUM_LEN                  (4)
#define CRC_LEN                     (2)

static const struct morsectrl_transport_ops uart_slip_ops;

/** @brief Data structure used to represent an instance of this transport. */
struct morsectrl_uart_slip_transport
{
    struct morsectrl_transport common;
    struct uart_config uart_config;
    struct uart_ctx *uart_ctx;
};

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *        uart_config field.
 */
static struct uart_config *uart_slip_cfg(struct morsectrl_transport *transport)
{
    struct morsectrl_uart_slip_transport *uart_slip_transport =
        (struct morsectrl_uart_slip_transport *)transport;
    return &uart_slip_transport->uart_config;
}

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, set the uart_ctx field.
 */
static void uart_slip_ctx_set(struct morsectrl_transport *transport, struct uart_ctx *ctx)
{
    struct morsectrl_uart_slip_transport *uart_slip_transport =
        (struct morsectrl_uart_slip_transport *)transport;
    uart_slip_transport->uart_ctx = ctx;
}

/**
 * @brief Given a pointer to a @ref morsectrl_transport instance, return a reference to the
 *        uart_ctx field.
 */
static struct uart_ctx *uart_slip_ctx(struct morsectrl_transport *transport)
{
    struct morsectrl_uart_slip_transport *uart_slip_transport =
        (struct morsectrl_uart_slip_transport *)transport;
    return uart_slip_transport->uart_ctx;
}

/**
 * @brief Prints an error message if possible.
 *
 * @param transport     Transport to print the error message from.
 * @param error_code    Error code.
 * @param error_msg     Error message.
 */
static void uart_slip_error(int error_code, char *error_msg)
{
    morsectrl_transport_err("UART_SLIP", error_code, error_msg);
}

/**
 * @brief Parse the configuration for the SLIP over UART interface.
 *
 * @param transport     The transport structure.
 * @param debug         Indicates whether debug print statements are enabled.
 * @param iface_opts    String containing the interface to use. May be NULL.
 * @param cfg_opts      Comma separated string with SLIP over UART configuration options.
 * @return              0 on success otherwise relevant error.
 */
static int uart_slip_parse(struct morsectrl_transport **transport,
                           bool debug,
                           const char *iface_opts,
                           const char *cfg_opts)
{
    struct uart_config *config;

    struct morsectrl_uart_slip_transport *uart_slip_transport =
        calloc(1, sizeof(*uart_slip_transport));
    if (!uart_slip_transport)
    {
        mctrl_err("Transport memory allocation failure\n");
        return -ETRANSNOMEM;
    }

    uart_slip_transport->common.debug = debug;
    uart_slip_transport->common.tops = &uart_slip_ops;
    *transport = &uart_slip_transport->common;
    config = uart_slip_cfg(*transport);

    if (cfg_opts == NULL || strlen(cfg_opts) == 0)
    {
        mctrl_err("Must specify the path to the UART file. For example: -c /dev/ttyACM0\n");
        return -ETRANSNOMEM;
    }

    strncpy(config->dev_name, cfg_opts, sizeof(config->dev_name) - 1);
    config->baudrate = DEFAULT_BAUDRATE;

    return 0;
}


/**
 * @brief Initialise an SLIP over UART interface.
 *
 * @note This should be done after parsing the configuration.
 *
 * @param transport Transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int uart_slip_init(struct morsectrl_transport *transport)
{
    struct uart_ctx *ctx;
    struct uart_config *config = uart_slip_cfg(transport);


    srand(time(NULL));

    ctx = uart_init(config);
    if (ctx == NULL)
        return ETRANSERR;
    uart_slip_ctx_set(transport, ctx);

    return ETRANSSUCC;
}

/**
 * @brief De-initialise an FTDI Transport.
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
static int uart_slip_deinit(struct morsectrl_transport *transport)
{
    struct uart_ctx *ctx = uart_slip_ctx(transport);

    uart_slip_ctx_set(transport, NULL);

    return uart_deinit(ctx);
}


/**
 * @brief Allocate @ref morsectrl_transport_buff.
 *
 * @param transport Transport structure.
 * @param size      Size of command and morse headers or raw data.
 * @return          Allocated @ref morsectrl_transport_buff or NULL on failure.
 */
static struct morsectrl_transport_buff *uart_slip_alloc(struct morsectrl_transport *transport,
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

static int uart_slip_tx_char(uint8_t c, void *arg)
{
    int ret;
    struct morsectrl_transport *transport = arg;
    struct uart_ctx *ctx = uart_slip_ctx(transport);


    ret = uart_write(ctx, &c, 1);
    if (ret == 1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static int uart_slip_send(struct morsectrl_transport *transport,
                         struct morsectrl_transport_buff *req,
                         struct morsectrl_transport_buff *resp)
{
    struct uart_ctx *ctx;
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
    ret = slip_tx(uart_slip_tx_char, transport, req->data, req->data_len);
    req->data_len = original_cmd_data_len;

    if (ret != 0)
    {
        uart_slip_error(ret, "Failed to send command");
        goto fail;
    }

    resp->data_len = 0;

    ctx = uart_slip_ctx(transport);

    clock_t timeout;
    clock_t last_time;
    enum
    {
        SLEEP_DURATION_MS = 1,
        START_OF_TRANSFER_TIMEOUT_CLOCKS = 60 * CLOCKS_PER_SEC, /* 60 sec to allow for long ops */
        TRANSFER_IN_PROGRESS_TIMEOUT_CLOCKS = 10 * CLOCKS_PER_SEC / 1000,
    };

    while (true)
    {
        timeout = START_OF_TRANSFER_TIMEOUT_CLOCKS;
        last_time = clock();
        slip_rx_state_reset(&slip_rx_state);
        do
        {
            uint8_t rx_char;
            ret = uart_read(ctx, &rx_char, 1);
            if (ret < 0)
            {
                uart_slip_error(ret, "Failed to rx command");
                goto fail;
            }
            else if (ret == 0)
            {
                if (clock() - last_time > timeout)
                {
                    /* Timeout occurred, exit failure */
                    ret = -ETRANSERR;
                    uart_slip_error(ret, "RX Timeout");
                    goto fail;
                }
                sleep_ms(SLEEP_DURATION_MS);
                continue;
            }
            slip_rx_status = slip_rx(&slip_rx_state, rx_char);

            if (slip_rx_state.frame_started)
            {
                /* We have received at least one byte, set shorter timeout. */
                last_time = clock();
                timeout = TRANSFER_IN_PROGRESS_TIMEOUT_CLOCKS;
            }
        } while (slip_rx_status == SLIP_RX_IN_PROGRESS);

        if (slip_rx_status != SLIP_RX_COMPLETE)
        {
            if (slip_rx_status == SLIP_RX_BUFFER_LIMIT)
            {
                uart_slip_error(-ETRANSERR, "Response exceeded allocated buffer");
            }
            uart_slip_error(-ETRANSERR, "Slip RX transfer incomplete");
            ret = -ETRANSERR;
            /* Drop buffer and try again. Probably had some junk data in the UART stream.
             * It is almost certain the next "complete" buffer will be junk too, but we'll handle
             * that with CRC + SEQ check.
             */
            continue;
        }

        resp->data_len = slip_rx_state.length;
        if (resp->data_len < SEQNUM_LEN + CRC_LEN)
        {
            if (resp->data_len > 0)
            {
                uart_slip_error(-ETRANSERR, "Received frame too short. Ignoring it...");
            }
            continue;
        }

        /* Remove and validate CRC */
        resp->data_len -= CRC_LEN;
        crc = crc16_gen(resp->data, resp->data_len);
        crc_field = resp->data + resp->data_len;
        if ((crc_field[0] != (crc & 0xff)) || crc_field[1] != ((crc >> 8) & 0xff))
        {
            uart_slip_error(-ETRANSERR, "CRC error for received frame. Ignoring it...");
            continue;
        }

        /* Remove and validate sequence number */
        resp->data_len -= SEQNUM_LEN;
        rsp_seq_num_field = resp->data + resp->data_len;

        if (memcmp(cmd_seq_num_field, rsp_seq_num_field, SEQNUM_LEN) != 0)
        {
            uart_slip_error(-ETRANSERR, "Seq # incorrect for received frame. Ignoring it...");
            continue;
        }

        return ETRANSSUCC;
    }
fail:
    return ret;
}

static const struct morsectrl_transport_ops uart_slip_ops = {
    .name = "uart_slip",
    .description = "Tunnel commands over a UART interface using SLIP framing",
    .has_reset = false,
    .has_driver = false,
    .parse = uart_slip_parse,
    .init = uart_slip_init,
    .deinit = uart_slip_deinit,
    .write_alloc = uart_slip_alloc,
    .read_alloc = uart_slip_alloc,
    .send = uart_slip_send,
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

REGISTER_TRANSPORT(uart_slip_ops);
