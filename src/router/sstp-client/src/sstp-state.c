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

#include <config.h>
#include "sstp-private.h"

/*!
 * @brief The context structure for the SSTP state machine
 */
struct sstp_state
{
    /*! The state of the machine */
    int state;

    /*! Specifies the server or client mode */
    int mode;

    /*! The current client connection */
    sstp_stream_st *stream;

    /*! The tx-buffer */
    sstp_buff_st *tx_buf;

    /*! The rx-buffer */
    sstp_buff_st *rx_buf;

    void *fwctx;

    sstp_state_forward_fn forward_cb;

    /*! The state transition function */
    sstp_state_change_fn state_cb;

    /*! The user context argument to state_cb */
    void *uarg;

    /*! The certificate hash protocol */
    int proto;

    /*! The disconnect status */
    int status;

    /*! The echo request counter */
    int echo;

    /*! The binding request value */
    uint8_t nounce[32];

    /*! The MPPE send key for HLAK */
    uint8_t mppe_send_key[16];

    /*! The MPEE receive key for HLAK */
    uint8_t mppe_recv_key[16];

};


void sstp_state_set_forward(sstp_state_st *state, sstp_state_forward_fn
        forward, void *arg)
{
    state->forward_cb = forward;
    state->fwctx = arg;
}

/*!
 * @par Make this generic
 */
static void sstp_state_send_complete(sstp_stream_st *stream,
    sstp_buff_st *buf, sstp_state_st *ctx, status_t status)
{
    if (SSTP_OKAY != status)
    {
        ctx->state_cb(ctx->uarg, SSTP_CALL_ABORT);
        return;
    }

    // Depends on the state .. but we might need to read here.

    return;
}


/*!
 * @brief Handle the SSTP control message: CALL_CONNECT_ACK
 */
static void sstp_state_connect_ack(sstp_state_st *ctx, sstp_msg_t type,
        sstp_buff_st *buf)
{
    sstp_attr_st *attrs[SSTP_ATTR_MAX + 1];
    sstp_attr_st *attr = NULL;
    status_t status    = SSTP_FAIL;
    int count = SSTP_ATTR_MAX + 1;
    int ret   = 0;
    int len   = 0;
    int index = 0;
    char *data= NULL;

    /* Obtain the attributes */
    ret = sstp_pkt_parse(buf, count, attrs);
    if (SSTP_OKAY != ret)
    {
        log_err("Could not parse attributes");
        goto done;
    }

    /* Get the crypto attribute */
    attr = attrs[SSTP_ATTR_CRYPTO_BIND_REQ];
    if (attr == NULL)
    {
        log_err("Could not get bind request attribute");
        goto done;
    }

    /* Get pointer and length */
    data = sstp_attr_data(attr);
    len  = sstp_attr_len(attr);

    /* Check the buffer */
    if (!data || len != 36)
    {
        log_err("Invalid Crypto Binding Request");
        goto done;
    }

    /* Get the cerficate protocol support */
    ctx->proto = data[index+3] & 0xFF;
    index += 4;

    /* Copy the binding request */
    memset(ctx->nounce, 0, sizeof(ctx->nounce));
    memcpy(ctx->nounce, &data[index], len - index);

    /* Lets handle the PPP negotiation */
    ctx->state_cb(ctx->uarg, SSTP_CALL_CONNECT);
    return;

done:
    
    if (SSTP_OKAY != status)
    {
        ctx->state_cb(ctx->uarg, SSTP_CALL_ABORT);
    }
}


/*!
 * @brief Send a Echo-Request to when timed out.
 */
static status_t sstp_state_echo_request(sstp_state_st *ctx)
{
    status_t status = SSTP_FAIL;

    log_info("Sending Echo-Request Message");

    /* Create the echo reply */
    status = sstp_pkt_init(ctx->tx_buf, SSTP_ECHO_REQUEST);
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Dump the packet */
    sstp_pkt_trace(ctx->tx_buf);

    /* Send the Echo Response back to server */
    status = sstp_stream_send(ctx->stream, ctx->tx_buf, (sstp_complete_fn)
            sstp_state_send_complete, ctx, 10);

    /* Increment the retry counter */
    ctx->echo++;
    
done:

    return status;
}


/*!
 * @brief Send a Echo Reply message in response to an Echo Request
 */
static status_t sstp_state_echo_reply(sstp_state_st *ctx)
{
    status_t status = SSTP_FAIL;

    log_info("Sending Echo-Reply Message");

    /* Create the echo reply */
    status = sstp_pkt_init(ctx->tx_buf, SSTP_ECHO_REPLY);
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Dump the packet */
    sstp_pkt_trace(ctx->tx_buf);

    /* Send the Echo Response back to server */
    status = sstp_stream_send(ctx->stream, ctx->tx_buf, (sstp_complete_fn)
            sstp_state_send_complete, ctx, 10);
    
done:

    return status;
}


/*!
 * @brief Send a disconnect message
 */
static status_t sstp_state_disconnect(sstp_state_st *ctx)
{
    status_t status = SSTP_FAIL;

    log_info("Sending Disconnect Message");

    /* Create the echo reply */
    status = sstp_pkt_init(ctx->tx_buf, SSTP_MSG_DISCONNECT);
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Dump the packet */
    sstp_pkt_trace(ctx->tx_buf);

    /* Send the Echo Response back to server */
    status = sstp_stream_send(ctx->stream, ctx->tx_buf, (sstp_complete_fn)
            sstp_state_send_complete, ctx, 10);
    
done:

    return status;
}


/*!
 * @brief Send a Disconnect ACK message to peer
 */
static status_t sstp_state_disconnect_ack(sstp_state_st *ctx)
{
    status_t status = SSTP_FAIL;

    log_info("Sending Disconnect Ack Message");

    /* Create the echo reply */
    status = sstp_pkt_init(ctx->tx_buf, SSTP_MSG_DISCONNECT_ACK);
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Dump the packet */
    sstp_pkt_trace(ctx->tx_buf);

    /* Send the Echo Response back to server */
    status = sstp_stream_send(ctx->stream, ctx->tx_buf, (sstp_complete_fn)
            sstp_state_send_complete, ctx, 10);
    
done:

    return status;
}


/*!
 * @brief Handle a Connect NAK message
 */
static status_t sstp_state_connect_nak(sstp_state_st *ctx, sstp_msg_t type,
        sstp_buff_st *buf)
{
    sstp_attr_st *attrs[SSTP_ATTR_MAX + 1];
    sstp_attr_st *attr = NULL;
    status_t retval = SSTP_FAIL;
    uint32_t status = 0;
    // uint8_t id = 0;
    int count  = SSTP_ATTR_MAX + 1;
    int ret    = 0;
    int len    = 0;
    int index  = 0;
    char *data = NULL;

    /* Obtain the attributes */
    ret = sstp_pkt_parse(buf, count, attrs);
    if (SSTP_OKAY != ret)
    {
        log_err("Could not parse attributes");
        goto done;
    }

    /* Get the status info attribute */
    attr = attrs[SSTP_ATTR_STATUS_INFO];
    if (!attr)
    {
        log_err("Could not get status info attribute");
        goto done;
    }

    /* Get the data pointers */
    data = sstp_attr_data(attr);
    len  = sstp_attr_len(attr);
    if (len < 4)
    {
        log_err("Invalid status attribute");
        goto done;
    }

    /* Get the faulty attribute */
    // id = data[index+3] & 0xFF;
    index += 4;

    /* Get the status */
    memcpy(&status, &data[index], sizeof(status));
    ctx->status = ntohl(status);
    index += sizeof(status);
    
    // TODO: DUMP ATTRIBUTE BUFFER HERE

    /* Success! */
    retval = SSTP_OKAY;

done:

    return retval;
}


/*!
 * @brief Handle control packets as they arrive
 */
static void sstp_state_handle_ctrl(sstp_state_st *state, sstp_buff_st *buf,
        sstp_msg_t type)
{
    status_t ret = SSTP_FAIL;

    // log_info("Handle Control Message: %d", type);
    switch (type)
    {
    case SSTP_MSG_CONNECT_ACK:
        sstp_state_connect_ack(state, type, buf);
        break;

    case SSTP_MSG_CONNECT_NAK:
        log_info("Connect NAK Message");
        ret = sstp_state_connect_nak(state, type, buf);
        if (SSTP_OKAY == ret)
        {
            sstp_state_disconnect(state);
        }
        break;

    case SSTP_MSG_ABORT:
        ret = sstp_state_connect_nak(state, type, buf);
        if (SSTP_OKAY == ret)
        {
            state->state_cb(state->uarg, SSTP_CALL_ABORT);
        }
        break;

    case SSTP_MSG_DISCONNECT:
        ret = sstp_state_connect_nak(state, type, buf);
        if (SSTP_OKAY == ret)
        {
            sstp_state_disconnect_ack(state);
        }
        state->state_cb(state->uarg, SSTP_CALL_DISCONNECT);
        break;

    case SSTP_MSG_DISCONNECT_ACK:
        state->state_cb(state->uarg, SSTP_CALL_DISCONNECT);
        break;

    case SSTP_ECHO_REQUEST:
        sstp_state_echo_reply(state);
        break;

    case SSTP_ECHO_REPLY:
        state->echo = 0;
        break;

    default:
        log_err("Unhandled Error message: %d", type);
        break;
    }
}


/*!
 * @brief Handle data packets
 */
static status_t sstp_state_handle_data(sstp_state_st *state, 
        sstp_buff_st *buf)
{
    status_t ret = SSTP_FAIL;

    /* Forward the data back to the pppd layer */
    ret = state->forward_cb(state->fwctx, sstp_pkt_data(buf), 
            sstp_pkt_data_len(buf));
    if (SSTP_OKAY != ret)
    {
        log_err("Could not forward packet to pppd");
    }

    return ret;
}

/*!
 * @brief Handle the sstp packet received
 */
static void sstp_state_handle_packet(sstp_state_st *ctx, sstp_buff_st *buf)
{
    sstp_msg_t type;

    /* Dump Packet */
    sstp_pkt_trace(buf);

    /* Handle the packet type */
    switch (sstp_pkt_type(buf, &type))
    {
    case SSTP_PKT_DATA:
        sstp_state_handle_data(ctx, buf);
        break;

    case SSTP_PKT_CTRL:
        sstp_state_handle_ctrl(ctx, buf, type);
        break;

    case SSTP_PKT_UNKNOWN:
        log_err("Unrecognized SSTP message");
        break;
    }
}


/*!
 * @brief Called from sstp_client_recv_sstp() when a complete sstp packet
 *  has been received.
 */
static void sstp_state_recv(sstp_stream_st *stream, sstp_buff_st *buf,
        sstp_state_st *ctx, status_t status)
{
    switch (status)
    {
    case SSTP_TIMEOUT:
        
        /* If we have seen no traffic, then disconnect */
        if (ctx->echo > 4)
        {
            ctx->state_cb(ctx->uarg, SSTP_CALL_ABORT);
            return;
        }

        /* Send a echo request */
        sstp_state_echo_request(ctx);
        break;

    case SSTP_OKAY:
        sstp_state_handle_packet(ctx, buf);
        break;

    case SSTP_FAIL:
    default:
        ctx->state_cb(ctx->uarg, status);
        return;
    }

    /* Setup a receiver for SSTP messages */
    sstp_stream_setrecv(ctx->stream, sstp_stream_recv_sstp, ctx->rx_buf,
            (sstp_complete_fn) sstp_state_recv, ctx, 60);
}


/*! 
 * @brief Send the connect request to the server
 */
static status_t sstp_state_send_request(sstp_state_st *ctx)
{
    status_t status = SSTP_FAIL;
    uint16_t proto  = htons(SSTP_ENCAP_PROTO_PPP);
    int ret;

    log_info("Sending Connect-Request Message");

    /* Initialize the pointer using */
    ret = sstp_pkt_init(ctx->tx_buf, SSTP_MSG_CONNECT_REQ);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Append an attribute */
    ret = sstp_pkt_attr(ctx->tx_buf, SSTP_ATTR_ENCAP_PROTO, 
            sizeof(proto), &proto);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Dump the packet */
    sstp_pkt_trace(ctx->tx_buf);

    /* Send the Call Connect request to the server */
    status = sstp_stream_send(ctx->stream, ctx->tx_buf, (sstp_complete_fn)
            sstp_state_send_complete, ctx, 10);
    if (SSTP_OKAY == status)
    {
        /* Setup a receiver for SSTP messages */
        sstp_stream_setrecv(ctx->stream, sstp_stream_recv_sstp, ctx->rx_buf,
                (sstp_complete_fn) sstp_state_recv, ctx, 60);
    }
 
done:

    if (SSTP_OKAY != status)
    {
        ctx->state_cb(ctx->uarg, SSTP_CALL_ABORT);
    }

    return status;
}


/*!
 * @brief Send the Connected message to the server
 */
static status_t sstp_state_send_connect(sstp_state_st *ctx)
{
    status_t status = SSTP_FAIL;
    status_t ret    = SSTP_FAIL;
    int pos         = 0;
    int len         = 32;
    uint8_t type    = 0;
    uint8_t data[100];
    cmac_ctx_st cmac;

    log_info("Sending Connected Message");

    /* Reset the memory */
    memset(data, 0, sizeof(data));

    /* Get the protocol type supported by this message */
    type = (ctx->proto & SSTP_PROTO_HASH_SHA256)
           ? SSTP_PROTO_HASH_SHA256
           : SSTP_PROTO_HASH_SHA1;
    
    /* Certificate Hash Protocol */
    data[3] = type;
    pos += 4;

    /* The server generated random (nounce) */
    memcpy(&data[pos], ctx->nounce, sizeof(ctx->nounce));
    pos += sizeof(ctx->nounce);

    /* The server certificate hash */
    ret = sstp_get_cert_hash(ctx->stream, ctx->proto,
            &data[pos], len);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }
    
    /* Create the message */
    ret = sstp_pkt_init(ctx->tx_buf, SSTP_MSG_CONNECTED);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Add the attribute */
    ret = sstp_pkt_attr(ctx->tx_buf, SSTP_ATTR_CRYPTO_BIND, 
            sizeof(data), data);
    if (SSTP_OKAY != ret)
    {
        goto done;
    }

    /* Get the CMAC Field */
    sstp_cmac_init(&cmac, (int) type);
    sstp_cmac_send_key(&cmac, ctx->mppe_send_key, 
            sizeof(ctx->mppe_send_key));
    sstp_cmac_recv_key(&cmac, ctx->mppe_recv_key,
            sizeof(ctx->mppe_recv_key));
    sstp_cmac_result(&cmac, (uint8_t*) &ctx->tx_buf->data[0], 
            ctx->tx_buf->len, (uint8_t*) &ctx->tx_buf->data[80], 32);

    /* Dump the packet */
    sstp_pkt_trace(ctx->tx_buf);

    /* Success */
    status = sstp_stream_send(ctx->stream, ctx->tx_buf, (sstp_complete_fn)
            sstp_state_send_complete, ctx, 10);
    if (SSTP_OKAY == status)
    {
        ctx->state_cb(ctx->uarg, SSTP_CALL_ESTABLISHED);
    }

    /* Set the established flag */
    ctx->state |= SSTP_ST_ESTABLISHED;

done:

    /* In case of failure */
    if (SSTP_OKAY != status)
    {
        ctx->state_cb(ctx->uarg, SSTP_CALL_ABORT);
    }

    return status;
}



status_t sstp_state_start(sstp_state_st *state)
{
    int retval = SSTP_FAIL;

    switch (state->mode)
    {
    case SSTP_MODE_CLIENT:
        
        /* Send the connect request to the server */
        retval = sstp_state_send_request(state);
        break;

    case SSTP_MODE_SERVER:
    default:
        retval = SSTP_NOTIMPL;
        break;
    }
    
    return (retval);
}


status_t sstp_state_accept(sstp_state_st *ctx)
{
    status_t ret = SSTP_FAIL;

    switch (ctx->mode)
    {
    case SSTP_MODE_CLIENT:
        /* Send the connect ACK to server */
        ret = sstp_state_send_connect(ctx);
        break;

    case SSTP_MODE_SERVER:
    default:
        ret = SSTP_NOTIMPL;
        break;
    }

    return ret;
}


status_t sstp_state_mppe_keys(sstp_state_st *ctx, unsigned char *skey,
        size_t slen, unsigned char* rkey, size_t rlen)
{
    status_t status = SSTP_FAIL;

    /* Check the length */
    if ((slen != sizeof(ctx->mppe_send_key)) ||
        (rlen != sizeof(ctx->mppe_recv_key)))
    {
        goto done;
    }

    /* Copy the MPPE keys */
    memcpy(ctx->mppe_send_key, skey, sizeof(ctx->mppe_send_key));
    memcpy(ctx->mppe_recv_key, rkey, sizeof(ctx->mppe_recv_key));

    /* Success */
    status = SSTP_OKAY;

done:

    return status;
}


const char *sstp_state_reason(sstp_state_st *ctx)
{
    return (ctx->state != 0)
        ? sstp_attr_status_str(ctx->status)
        : "Reason was not known";
}


void sstp_state_free(sstp_state_st *state)
{
    if (!state)
    {
        return;
    }

    /* Free the receive buffer */
    if (state->rx_buf)
    {
        sstp_buff_destroy(state->rx_buf);
        state->rx_buf = NULL;
    }

    /* Free the transmit buffer */
    if (state->tx_buf)
    {
        sstp_buff_destroy(state->tx_buf);
        state->tx_buf = NULL;
    }

    /* Free the state object */
    free(state);
}


status_t sstp_state_create(sstp_state_st **state, sstp_stream_st *stream,
        sstp_state_change_fn state_cb, void *ctx, int mode)
{
    int status = 0;
    int ret    = 0;

    /* Allocate memory for the state object */
    *state = calloc(1, sizeof(sstp_state_st));
    if (!*state)
    {
        goto done;
    }

    /* Initialize the State context */
    (*state)->uarg     = ctx;
    (*state)->state_cb = state_cb;
    (*state)->mode     = mode;
    (*state)->stream   = stream;

    /* Allocate send buffer */
    ret = sstp_buff_create(&(*state)->tx_buf, 16384);
    if (SSTP_OKAY != ret)
    {   
        goto done;
    }

    /* Allocate receive buffer */
    ret = sstp_buff_create(&(*state)->rx_buf, 16384);
    if (SSTP_OKAY != ret)
    {   
        goto done;
    }

    /* Success */
    status = SSTP_OKAY;

done:
    
    if (SSTP_OKAY != status)
    {
        sstp_state_free(*state);
        *state = NULL;
    }

    return status;
}


