/*!
 * @brief SSL Handling Routines
 *
 * @file sstp-ssl.c
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
 *
 * @TODO:
 *   - Implement functions to get 
 *     -> sstp_stream_recv_http(), this receives a http response
 *     -> sstp_stream_recv_sstp(), this receives a sstp packet
 *   
 *   - We need to make sure we can send *and* receive sstp packets at 
 *     the same time, e.g. while sending; we may need to receive.
 *
 *   - Handle certificate verification, need to get the 
 *     certificate digest for use in the communication
 *     -> sstp_stream_certhash();   // Get certificate hash
 *     -> sstp_stream_getsess();    // Get SSL session info
 *
 *   - Set the SSL_MODE_AUTO_RETRY
 */

#include <config.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "sstp-private.h"


/*!
 * @brief A asynchronous send or recv channel object
 */
typedef struct sstp_operation
{
    /*< The next operation */
    struct sstp_operation *next;

    /*< Timeout if any */
    timeval_st tout;

    /*< Associated buffer with this channel */
    sstp_buff_st *buf;

    /*< Complete callback function */
    sstp_complete_fn complete;

    /*< Argument to pass back the complete function */
    void *arg;
    
} sstp_operation_st;


/*! 
 * @brief The ssl client context
 */
struct sstp_stream
{
    /*< The send socket */
    int ssock;

    /*< The recv socket */
    int rsock;

    /*< Last activity seen on socket */
    time_t last;

    /*< The SSL connection context */
    SSL *ssl;

    /*< The SSL context structure */
    SSL_CTX *ssl_ctx;

    /*< The length check function */
    sstp_recv_fn recv_cb;

    /*< The send function */
    event_fn send_cb;

    /*< The event base */
    event_base_st *ev_base;

    /*< The receive event */
    event_st *ev_recv;

    /*< Channel for receive operation */
    sstp_operation_st recv;

    /*< The event structure */
    event_st *ev_send;

    /*< Channel for send operation */
    sstp_operation_st *send;

    /*< The list of free operations */
    sstp_operation_st *cache;
};


static int sstp_operation_add_read(sstp_stream_st *ctx, sstp_buff_st *buf,
    int event, int timeout, sstp_complete_fn complete, void *arg);

/*!
 * @brief Allocate a new operation or grab one from the cache
 */
static sstp_operation_st *sstp_operation_get(sstp_stream_st *ctx, 
    sstp_buff_st *buf, 
    int timeout, 
    sstp_complete_fn complete, 
    void *arg)
{
    sstp_operation_st *op = NULL;

    if (!ctx->cache) 
    {
        op = calloc(1, sizeof(sstp_operation_st));
        op->buf         = buf;
        op->complete    = complete;
        op->arg         = arg;
        op->tout.tv_sec = timeout;
    }
    else
    {
        op = ctx->cache;
        op->next        = NULL;
        op->buf         = buf;
        op->complete    = complete;
        op->arg         = arg;
        op->tout.tv_sec = timeout;
        ctx->cache      = op->next;
    }

    return op;
}

/*!
 * @brief Add another send operation to the list.
 */
static void sstp_operation_append(sstp_operation_st **head, 
        sstp_operation_st *item)
{   
    item->next = NULL;

    sstp_operation_st *ptr = *head;
    if (ptr == NULL)
    {
        *head = item;
        return;
    }

    while (ptr && ptr->next)
        ptr = ptr->next;

    ptr->next = item;
}

/*!
 * @brief Continue the send operation
 */
static void sstp_send_cont(int sock, short event, sstp_stream_st *ctx)
{
    sstp_operation_st *op;
    int ret = 0;

    while (ctx->send) 
    {
        op = ctx->send;
        ctx->send = op->next;

        /* Retry the send operation, better luck this time */
        ret = sstp_stream_send(ctx, op->buf, op->complete, op->arg, 
                op->tout.tv_sec);
        if (ret == SSTP_INPROG) 
            return;

        /* Notify the caller of the status */
        op->complete(ctx, op->buf, op->arg, ret);
        op->next   = ctx->cache;
        ctx->cache = op;
    }
}

/*! 
 * @brief Resume the send operation by retrying last operation
 */
static void sstp_recv_cont(int sock, short event, sstp_stream_st *ctx)
{
    sstp_operation_st *op = &ctx->recv;
    int ret = 0;

    /* Handle Timeout */
    if (EV_TIMEOUT & event)
    {
        op->complete(ctx, op->buf, op->arg, SSTP_TIMEOUT);
        return;
    }

    /* Try to receive data */
    ret = (ctx->recv_cb)(ctx, op->buf, op->complete, op->arg, 
            op->tout.tv_sec);
    if (ret == SSTP_INPROG)
        return;
        
    /* Notify the caller of the status */
    op->complete(ctx, op->buf, op->arg, ret);

    /* Re-add the event */
    sstp_operation_add_read(ctx, op->buf, EV_READ,  
            op->tout.tv_sec, op->complete, op->arg);
}


/*!
 * @brief Add a the read operation
 */
static int sstp_operation_add_read(sstp_stream_st *ctx, 
    sstp_buff_st *buf, 
    int event,
    int timeout, 
    sstp_complete_fn complete, 
    void *arg)
{
    int retval = SSTP_FAIL;
    int ret    = 0;

    sstp_operation_st *op = &ctx->recv;
    op->buf         = buf;
    op->complete    = complete;
    op->arg         = arg;
    op->tout.tv_sec = timeout;

    if (timeout > 0)
        event |= EV_TIMEOUT;

    if (event_pending(ctx->ev_recv, EV_READ | EV_WRITE | 
            EV_TIMEOUT, NULL)) 
    {
        event_del(ctx->ev_recv);
    }

    event_set(ctx->ev_recv, ctx->rsock, event,
        (event_fn) sstp_recv_cont, ctx);
    
    /* Set the event base */
    event_base_set(ctx->ev_base, ctx->ev_recv);

    /* Add the event */
    ret = event_add(ctx->ev_recv, (timeout > 0) ? 
            &op->tout : NULL);
    if (ret != 0) 
    {
        log_err("Could not add read event");
        goto done;
    }

    /* Success */
    retval = SSTP_OKAY;

done:

    return retval;
}

/*!
 * @brief Queue a write operation to the list of events
 */
static int sstp_operation_add_write(sstp_stream_st *ctx,
    sstp_buff_st *buf,
    int event,
    int timeout,
    sstp_complete_fn complete,
    void *arg)
{
    sstp_operation_st *op = NULL;
    int ret  = SSTP_FAIL;
    int pend = 0;

    op = sstp_operation_get(ctx, buf, timeout, complete, arg);
    if (!op) 
    {
        log_err("Could not allocate a free operation");
        goto done;
    }
    
    sstp_operation_append(&ctx->send, op);

    /* In case current operation is pending */
    pend = event_pending(ctx->ev_send, EV_READ | 
            EV_WRITE | EV_TIMEOUT, NULL);
    if (pend)
    {
        ret = SSTP_INPROG;
        goto done;
    }

    if (timeout > 0)
        event |= EV_TIMEOUT;

    /* Configure the event */
    event_set(ctx->ev_send, ctx->ssock, event, 
            (event_fn) ctx->send_cb, ctx);

    /* Set the event base */
    event_base_set(ctx->ev_base, ctx->ev_send);

    /* Add the event */
    ret = event_add(ctx->ev_send, (timeout > 0) ? 
            &op->tout : NULL);

    /* Success */
    ret = SSTP_OKAY;

done:

    return ret;
}

status_t sstp_get_cert_hash(sstp_stream_st *ctx, int proto, 
    unsigned char *hash, int hlen)
{
    status_t status    = SSTP_FAIL;
    const EVP_MD *type = (SSTP_PROTO_HASH_SHA256 & proto)
        ? EVP_sha256()
        : EVP_sha1() ;
    X509 *peer = NULL;
    int ret = 0;

    /* Reset the hash output */
    memset(hash, 0, hlen);

    /* Get the peer certificate */
    peer = SSL_get_peer_certificate(ctx->ssl);
    if (!peer)
    {
        log_err("Failed to get peer certificate");
        goto done;
    }

    /* Get the digest */
    ret = X509_digest(peer, type, hash, (unsigned int*) &hlen);
    if (ret != 1)
    {
        log_err("Failed to get certificate hash");
        goto done;
    }
    
    /* Success! */
    status = SSTP_OKAY;

done:

    return (status);
}

status_t sstp_verify_cert(sstp_stream_st *ctx, const char *host, int opts)
{
    status_t status = SSTP_FAIL;
    X509_NAME *name = NULL;
    X509 *peer = NULL;
    char result[256];
    
    /* Get the peer certificate */
    peer = SSL_get_peer_certificate(ctx->ssl);
    if (!peer)
    {
        log_err("Could not get peer certificate");
        goto done;
    }

    /* Verify the certificate chain */
    if (SSTP_VERIFY_CERT & opts)
    {
        int ret = SSL_get_verify_result(ctx->ssl);
        if (X509_V_OK != ret)
        { 
            log_info("SSL certificate verification failed: %s (%d)", 
                    X509_verify_cert_error_string(ret), ret);
            goto done;
        }
    }

    /* Verify the name of the server */
    if (SSTP_VERIFY_NAME & opts)
    {
        /* Extract the subject name field */
        name = X509_get_subject_name(peer);
        if (!name)
        {
            log_err("Could not get subject name");
            goto done;
        }

        /* Get the common name of the certificate */
        X509_NAME_get_text_by_NID(name, NID_commonName, 
                result, sizeof(result));
        if (strcasecmp(host, result))
        {
            log_info("The certificate did not match the host: %s", host);
            goto done;
        }
    }

    /* Success */
    status = SSTP_OKAY;

done:

    return status;
}

status_t sstp_last_activity(sstp_stream_st *stream, int seconds)
{
    if (difftime(time(NULL), stream->last) > seconds)
    {
        return SSTP_FAIL;
    }

    return SSTP_OKAY;
}


/* 
 * Stubbed function for now...
 */
status_t sstp_stream_recv_http(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout)
{
    return SSTP_NOTIMPL;
}


status_t sstp_stream_recv_plain(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout)
{
    status_t status = SSTP_FAIL;
    int ret = 0;

    /* Save the arguments in case of callback */
    ctx->recv_cb = sstp_stream_recv_plain;
    
    /* Receive data */
    ret = recv(ctx->rsock, buf->data + buf->off, 
            buf->max - buf->off, 0);
    if (ret <= 0)
    {
        log_err("Unrecoverable socket error, %d", errno);
        goto done;
    }

    buf->off += ret;

    /* Success */
    status = SSTP_OKAY;

done:
    
    return status;
}

status_t sstp_stream_recv(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout)
{
    status_t status = SSTP_FAIL;
    short event = 0;
    int ret = 0;

    /* Setup the timeout */
    if (timeout > 0)
    {
        event |= EV_TIMEOUT;
    }
    ctx->recv_cb = sstp_stream_recv;

    /* Activity Timer */
    ctx->last = time(NULL);

    /* Try to read from the SSL socket until it blocks */
    ret = SSL_read(ctx->ssl, buf->data + buf->off, buf->max - buf->off);
    switch (SSL_get_error(ctx->ssl, ret))
    {
    case SSL_ERROR_NONE:
        buf->off += ret;
        status = SSTP_OKAY;
        break;

    case SSL_ERROR_WANT_READ:
        sstp_operation_add_read(ctx, buf, EV_READ, timeout,
            complete, arg);
        status = SSTP_INPROG;
        goto done;

    case SSL_ERROR_WANT_WRITE:
        sstp_operation_add_read(ctx, buf, EV_WRITE, timeout,
            complete, arg);
        status = SSTP_INPROG;
        goto done;
    
    default:
        log_err("Unrecoverable SSL error %d", ret);
        goto done;
    }

    status = SSTP_OKAY;

done:

    return status;
}

status_t sstp_stream_recv_sstp(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout)
{
    status_t status = SSTP_FAIL;
    int ret = 0;

    /* Activity Timer */
    ctx->last = time(NULL);

    do
    {
        /* Try to the header first, then the entire packet */
        buf->len = (buf->off >= 4)
            ? sstp_pkt_len(buf)
            : 4 ;

        /* Try to read from the SSL socket */
        ret = SSL_read(ctx->ssl, buf->data + buf->off, 
                buf->len - buf->off);
        switch (SSL_get_error(ctx->ssl, ret))
        {
        case SSL_ERROR_NONE:
            buf->off += ret;
            break;

        case SSL_ERROR_WANT_READ:
            sstp_operation_add_read(ctx, buf, EV_READ, timeout,
                complete, arg);
            status = SSTP_INPROG;
            goto done;

        case SSL_ERROR_WANT_WRITE:
            sstp_operation_add_read(ctx, buf, EV_WRITE, timeout,
                complete, arg);
            status = SSTP_INPROG;
            goto done;
        
        default:
            log_err("Unrecoverable SSL error");
            goto done;
        }

    } while (buf->off < sstp_pkt_len(buf));

    /* Success */
    status = SSTP_OKAY;

done:

    return status;
}

void sstp_stream_setrecv(struct sstp_stream *ctx, sstp_recv_fn recv_cb,
    sstp_buff_st *buf, sstp_complete_fn complete, void *arg, int timeout)
{
    /* Setup the channel */
    ctx->recv_cb = recv_cb;
    sstp_operation_add_read(ctx, buf, EV_READ, timeout, 
            complete, arg);
    sstp_buff_reset(buf);
}


/*!
 * @brief Continue the send operation
 */
static void sstp_send_cont_plain(int sock, short event, 
        sstp_stream_st *ctx)
{
    sstp_operation_st *op = NULL;
    int ret = 0;
    
    op = ctx->send;
    ctx->send = op->next;

    /* Retry the send operation, better luck this time */
    ret = sstp_stream_send_plain(ctx, op->buf, op->complete, 
            op->arg, op->tout.tv_sec);
    switch (ret)
    {
    case SSTP_FAIL:
    case SSTP_OKAY:

        /* Notify the caller of the status */
        op->complete(ctx, op->buf, op->arg, ret);
        ctx->send = op->next;
        op->next  = ctx->cache;
        ctx->cache= op;
        break;

    case SSTP_INPROG:

        /* This state is already handled */
        break;
    }
}


status_t sstp_stream_send_plain(sstp_stream_st *stream, sstp_buff_st *buf,
    sstp_complete_fn complete, void *arg, int timeout)
{
    int ret = 0;

    /* Non-blocking send */
    ret = send(stream->ssock, buf->data + buf->off,
            buf->len - buf->off, 0);
    if (ret <= 0)
    {
        log_err("Unrecoverable socket error, %d", errno);
        return SSTP_FAIL;
    }

    /* Did we complete the write */
    buf->off += ret;
    if (buf->off < buf->len)
    {
        stream->send_cb = (event_fn) sstp_send_cont_plain;
        sstp_operation_add_write(stream, buf, EV_WRITE, timeout,
                complete, stream);

        /* Send in progress */
        return SSTP_INPROG;
    }

    return SSTP_OKAY;
}


status_t sstp_stream_send(sstp_stream_st *stream, sstp_buff_st *buf,
    sstp_complete_fn complete, void *arg, int timeout)
{
    int ret = 0;

    stream->last = time(NULL);
    stream->send_cb = (event_fn) sstp_send_cont;

    /* 
     * If we try SSL_write before previous operation is complete, we
     * will end up with a SSL error and disconnect. There's two ways
     * this can happen: 
     *  1. Sending a response to SSTP protocol related packet
     *  2. PPP data to be forwarded
     */

    if (event_pending(stream->ev_send, EV_READ | EV_WRITE, NULL))
    {
        sstp_operation_add_write(stream, buf, EV_WRITE, timeout,
                complete, arg);
        return SSTP_INPROG;
    }

    do
    {
        /* Try SSL write to the socket */
        int err = 0;
        ret = SSL_write(stream->ssl, buf->data + buf->off, 
                buf->len - buf->off);
        switch ((err = SSL_get_error(stream->ssl, ret)))
        {
        case SSL_ERROR_NONE:
            buf->off += ret;
            break;

        case SSL_ERROR_WANT_READ:
            sstp_operation_add_write(stream, buf, EV_READ, 
                    timeout, complete, arg);
            return SSTP_INPROG;
        
        case SSL_ERROR_WANT_WRITE:
            sstp_operation_add_write(stream, buf, EV_WRITE, 
                    timeout, complete, arg);
            return SSTP_INPROG;

        default:
            log_err("Unrecoverable socket error, %d", err);
            return SSTP_FAIL;
        }

    } while (buf->off < buf->len);

    return SSTP_OKAY;
}


static status_t sstp_stream_setup(sstp_stream_st *stream)
{
    /* Associate the streams */
    stream->ssl = SSL_new(stream->ssl_ctx);
    if (stream->ssl == NULL)
    {
        log_err("Could not create SSL session", -1);
        goto done;
    }

    /* Associate a socket with the connection */
    if (SSL_set_fd(stream->ssl, stream->ssock) < 0)
    {   
        log_err("Could not set SSL socket");
        goto done;
    }   

    /* Set Client Mode (connect) */
    SSL_set_connect_state(stream->ssl);

    /* Success */
    return SSTP_OKAY;

done:

    if (stream->ssl != NULL)
    {   
        SSL_free(stream->ssl);
        stream->ssl = NULL;
    }   

    return SSTP_FAIL;
}

static void sstp_connect_complete(int sock, short event, 
        sstp_stream_st *stream)
{
    sstp_operation_st *op = NULL;
    status_t status = SSTP_FAIL;
    int ret = -1;

    op = stream->send;
    stream->send = op->next;

    /* In case connect timed out */
    if (EV_TIMEOUT & event)
    {
        log_err("Connect timed out");
        goto done;
    }

    /* Configure the SSL context */
    ret = sstp_stream_setup(stream);
    if (SSTP_OKAY != ret)
    {
        log_err("Could not configure SSL socket");
        goto done;
    }

    /* Success! */
    status = SSTP_CONNECTED;
    op->next = stream->cache;
    stream->cache = op;

done:

    /* Propagate the information */
    op->complete(stream, NULL, op->arg, status);
}

status_t sstp_stream_connect(sstp_stream_st *stream, struct sockaddr *addr,
        int alen, sstp_complete_fn complete, void *arg, int timeout)
{
    int ret = (-1);

    /* Create the socket */
    stream->ssock = socket(PF_INET, SOCK_STREAM, 0);
    if (0 > stream->ssock)
    {          
        log_err("Could not create socket");
        goto done;
    }

    stream->rsock = dup(stream->ssock);

    /* Set socket non-blocking mode */
    ret = sstp_set_nonbl(stream->ssock, 1);
    if (SSTP_OKAY != ret)
    {
        log_err("Unable to set non-blocking operation");
        goto done;
    }   
    
    /* Set socket non-blocking mode */
    ret = sstp_set_nonbl(stream->rsock, 1);
    if (SSTP_OKAY != ret)
    {
        log_err("Unable to set non-blocking operation");
        goto done;
    }   
    
    /* Set send buffer size */
    ret = sstp_set_sndbuf(stream->ssock, 32768);
    if (SSTP_OKAY != ret)      
    {                                              
        log_warn("Unable to set send buffer size", errno);
    }

    /* Connect to the server (non-blocking) */
    ret = connect(stream->ssock, addr, alen);
    if (ret == -1)
    {
        /* If we are not blocking b/c of connection in progress */
        if (errno != EINPROGRESS)
        {
            log_err("Connection failed (%d)", errno);
            goto done;
        }

        /* Add a send operation */
        stream->send_cb = (event_fn) sstp_connect_complete;
        ret = sstp_operation_add_write(stream, NULL, EV_WRITE, 
                timeout, complete, arg);
        if (ret != SSTP_OKAY) {
            log_err("Could not add send event");
            goto done;
        }

        return SSTP_INPROG;
    }

    /* Success */
    return SSTP_OKAY;

done:

    /* Cleanup */
    if (stream->ssock >= 0)
    {
        close(stream->ssock);
    }
    
    return SSTP_FAIL;
}

status_t sstp_stream_destroy(sstp_stream_st *stream)
{
    sstp_operation_st *ptr = NULL;
    status_t retval = SSTP_FAIL;
    int ret = -1;
    
    /* Get the current socket */
    if (stream->ssock <= 0)
    {
        log_debug("No socket associated");
        goto done;
    }

    /* Set blocking mode */
    ret = sstp_set_nonbl(stream->ssock, 0);
    if (SSTP_OKAY != ret)
    {
        log_warn("Unable to set blocking mode socket");
        goto done;
    }

    /* Shutdown the server */
    SSL_shutdown(stream->ssl);

    /* Free resources */
    SSL_free(stream->ssl);
    stream->ssl = NULL;

    if (stream->ssock)
        close(stream->ssock);

    if (stream->rsock)
        close(stream->rsock);

    /* Remove the send event */
    if (stream->ev_send) 
    {
        event_del(stream->ev_send);
        event_free(stream->ev_send);
        stream->ev_send = NULL;
    }

    /* Remove the recv event */
    if (stream->ev_recv)
    {
        event_del(stream->ev_recv);
        event_free(stream->ev_recv);
        stream->ev_recv = NULL;
    }

    /* Free the list of send events */
    ptr = stream->send;
    while (ptr) {
        sstp_operation_st *next = ptr->next;
        free(ptr);
        ptr = next;
    }

    /* Free the stream */
    free(stream);

    /* Success */
    retval = SSTP_OKAY;

done:

    return (retval);
}


status_t sstp_stream_create(sstp_stream_st **stream, event_base_st *base, 
        SSL_CTX *ssl)
{
    /* Create a new stream */
    sstp_stream_st *stream_= calloc(1, sizeof(sstp_stream_st));
    if (!stream_)
    {
        return SSTP_FAIL;
    }

    /* Associate stream with ssl context */
    stream_->ev_base = base;
    stream_->ev_recv = event_new(base, -1, 0, NULL, NULL);
    stream_->ev_send = event_new(base, -1, 0, NULL, NULL);
    stream_->ssl_ctx = ssl;
    *stream = stream_;

    /* Success */
    return SSTP_OKAY;
}

