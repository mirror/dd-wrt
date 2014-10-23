/*!
 * @brief State machine for SSTP layer
 *
 * @file sstp-state.h
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef __SSTP_STATE_H__
#define __SSTP_STATE_H__


#define SSTP_ST_CALL_CONNECT_REQ       0x0004
#define SSTP_ST_CALL_CONNECT_ACK       0x0008
#define SSTP_ST_CALL_CONNECTED         0x0010
#define SSTP_ST_CALL_ABORT             0x0020
#define SSTP_ST_DISCONNECT             0x0040
#define SSTP_ST_DISCONNECT_ACK         0x0080
#define SSTP_ST_ESTABLISHED            0x1000

typedef enum
{
    SSTP_CALL_ABORT         = 1,
    SSTP_CALL_CONNECT       = 2,
    SSTP_CALL_ESTABLISHED   = 3,
    SSTP_CALL_DISCONNECT    = 4,

} sstp_state_t;


struct sstp_state;
typedef struct sstp_state sstp_state_st;


/*!
 * @brief Signal to the upper layer any state transitions
 * @param state Can be any of the following states:
 *    - SSTP_ST_ABORT       Connection Failed
 *    - SSTP_ST_CONNECT     Start Higher Layer (PPP)
 *    - SSTP_ST_ESTABLISH   Tunnel Established
 *    - SSTP_ST_DISCONNECT  Connection disconnected
 */
typedef void (*sstp_state_change_fn)(void *arg, sstp_state_t state);


/*!
 * @brief Set the data forwarder function
 */
typedef status_t (*sstp_state_forward_fn)(void *arg, uint8_t *data, 
        int size);

/*!
 * @brief Create the SSTP state machine
 */
status_t sstp_state_create(sstp_state_st **state, sstp_stream_st *stream,
        sstp_state_change_fn state_cb, void *ctx, int mode);


/*! 
 * @brief Set the MPPE keys after PPP negotiation have finished
 * @param state     The state context
 * @param skey      The MPPE send key
 * @param slen      The length of the MPPE send key
 * @param rkey      The MPPE recv key
 * @param rlen      The length of the MPPE recv key
 */
status_t sstp_state_mppe_keys(sstp_state_st *state, unsigned char *skey,
        size_t slen, unsigned char* rkey, size_t rlen);


/*!
 * @brief Set the forward function to the local peer
 */
void sstp_state_set_forward(sstp_state_st *state, sstp_state_forward_fn 
        forward, void *arg);

/*!
 * @brief Will start the SSTP handshake
 */
status_t sstp_state_start(sstp_state_st *state);


/*!
 * @brief Continue the call connect
 */
status_t sstp_state_accept(sstp_state_st *ctx);


/*!
 * @brief Sets the CHAP context
 */
void sstp_state_chap_challenge(sstp_state_st *ctx, sstp_chap_st *chap);


/*!
 * @brief Return reason for why call was aborted
 */
const char *sstp_state_reason(sstp_state_st *ctx);


/*!
 * @brief Free the SSTP state machine
 */
void sstp_state_free(sstp_state_st *state);


#endif /* #ifdef __SSTP_STATE_H__ */
