/*
 * Copyright 2024 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

/*
 * Serial Line Internet Protocol (SLIP) implementation.
 *
 * SLIP was originally designed as an encapsulation for IP over serial ports, but can be used
 * for framing of any packet-based data for transmission over a serial port.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define SLIP_RX_BUFFER_SIZE     (2000)

struct slip_rx_state
{
    uint8_t *buffer;
    size_t buffer_length;
    size_t length;
    bool escape;
    bool frame_started;
};

#define SLIP_RX_STATE_INIT(_buffer, _buffer_length) { _buffer, _buffer_length, 0, false, false}

static inline void slip_rx_state_reset(struct slip_rx_state *state)
{
    state->escape = false;
    state->length = 0;
    state->frame_started = true;
}


enum slip_rx_status
{
    SLIP_RX_COMPLETE,       /**< @brief A complete packet with length > 0 has been received. */
    SLIP_RX_IN_PROGRESS,    /**< @brief Receive is still in progress. */
    SLIP_RX_BUFFER_LIMIT,   /**< @brief Receive buffer limit has been reached. */
    SLIP_RX_ERROR,          /**< @brief An erroneous packet has been received. */
};

/**
 * @brief Handle reception of a character in a SLIP stream.
 *
 * When reception of a packet is successful, this will return @c SLIP_RX_COMPLETE and the
 * packet can be found in `state->buffer` with length `state->length`.
 *
 * @param state Current slip state. Will be updated by this function.
 * @param c     The received character.
 *
 * @return an appropriate value of @ref slip_rx_status.
 */
enum slip_rx_status slip_rx(struct slip_rx_state *state, uint8_t c);

/**
 * @brief Function to send a character on the slip transport.
 *
 * @param c     The character to transmit
 * @param arg   Opaque argument, as passed to @c slip_tx().
 *
 * @return 0 on success, otherwise a negative error code.
 */
typedef int (*slip_transport_tx_fn)(uint8_t c, void *arg);

/**
 * @brief Transmit a packet with with SLIP framing.
 *
 * @param transport_tx_fn   Function to invoke to send characters on the transport.
 * @param transport_tx_arg  Argument to pass to @p transport_tx_fn.
 * @param packet            The packet to transmit.
 * @param packet_len        The length of the packet.
 *
 * @return 0 on success, otherwise an error code as returned by @p transport_tx_fn.
 */
int slip_tx(slip_transport_tx_fn transport_tx_fn, void *transport_tx_arg,
            const uint8_t *packet, size_t packet_len);
