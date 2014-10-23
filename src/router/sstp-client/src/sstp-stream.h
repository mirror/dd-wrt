/*!
 * @brief Declarations for SSL Handling Routines
 *
 * @file sstp-ssl.h
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
#ifndef __SSTP_SSL_H__
#define __SSTP_SSL_H__

#include <sys/types.h>
#include <sys/socket.h>


#define SSTP_VERIFY_NONE        0x00    // Don't verify certificate
#define SSTP_VERIFY_NAME        0x01    // Verify the Certificate name
#define SSTP_VERIFY_CERT        0x02    // Verify the Certificate with CA
#define SSTP_VERIFY_CRL         0x04    // Verify against CRL service


/*
 * NOTE:
 *  The naming convention of sstp_stream is probably not correct. These should
 *  probably be ssl_sock_st, and we should add functions to accept() in case
 *  of a server. 
 */

struct sstp_stream;
typedef struct sstp_stream sstp_stream_st;


/*!
 * @brief Get the certificate hash from the peer certificate
 */
status_t sstp_get_cert_hash(sstp_stream_st *ctx, int proto, 
    unsigned char *hash, int hlen);

/*!
 * @brief Verify the certificate
 */
status_t sstp_verify_cert(sstp_stream_st *ctx, const char *host, int opts);


/*!
 * @brief Check if the activity on the socket is longer than @a seconds
 */
status_t sstp_last_activity(sstp_stream_st *client, int seconds);


/*!
 * @brief A function to notify of complete send or receive event
 */
typedef void (*sstp_complete_fn)(sstp_stream_st *stream, 
        sstp_buff_st *buf, void *ctx, status_t status);

/*!
 * @brief The handler function that will handle the receive of the data
 */
typedef status_t (*sstp_recv_fn)(sstp_stream_st *ctx, sstp_buff_st *buf,
        sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief A Handler for receiving SSTP packets
 */
status_t sstp_stream_recv_sstp(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief A generic handler for receiving anything else (blob)
 */
status_t sstp_stream_recv(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief A handler for reciving a HTTP request
 */
status_t sstp_stream_recv_http(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief Setup a socket handler for the SSL connection
 * 
 * @par Note:
 *  The @a handler can be either a custom function or it can be any
 *  of the sstp_stream_recv, sstp_client_recv_sstp, or 
 *  sstp_stream_recv_http functions.
 */
void sstp_stream_setrecv(sstp_stream_st *ctx, sstp_recv_fn recv_cb,
        sstp_buff_st *buf, sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief Send a buffer using non-blocking I/O on the SSL socket.
 * @param client    [IN] The client context to communicate on
 * @param buf       [IN] The buffer structure
 * @param complete  [IN] The callback to call when SSTP_INPROG is returned
 *
 * @return SSTP_OKAY when buffer is written successfully to the socket, 
 *  SSTP_FAIL if an error occured during the write, and
 *  SSTP_INPROG if the operation would block
 */
status_t sstp_stream_send(sstp_stream_st *client, sstp_buff_st *buf,
        sstp_complete_fn complete, void *ctx, int timeout);


/*!
 * @brief Send data on a plain text socket
 */
status_t sstp_stream_send_plain(sstp_stream_st *stream, sstp_buff_st *buf,
    sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief Perform a plain text receive
 */
status_t sstp_stream_recv_plain(sstp_stream_st *ctx, sstp_buff_st *buf, 
        sstp_complete_fn complete, void *arg, int timeout);


/*!
 * @brief Connect a SSL socket using non-blocking I/O
 */
status_t sstp_stream_connect(sstp_stream_st *client, struct sockaddr *addr,
        int addrlen, sstp_complete_fn complete, void *ctx, int timout);


/*!
 * @brief Create the client
 */
status_t sstp_stream_create(sstp_stream_st **client, event_base_st *base, 
        SSL_CTX *ssl);


/*!
 * @brief Destroy a SSL Client
 */
status_t sstp_stream_destroy(sstp_stream_st *client);


#endif /* #ifndef __SSTP_SSL_H__ */
