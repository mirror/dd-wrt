/*
  This file is part of libmicrohttpd
  Copyright (C) 2017, 2020 Karlson2k (Evgeny Grin)
  Copyright (C) 2019 ng0

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

/**
 * @file mhd_send.h
 * @brief Declarations of send() wrappers.
 * @author ng0
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_SEND_H
#define MHD_SEND_H

#include "platform.h"
#include "internal.h"
#if defined(HAVE_STDBOOL_H)
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <errno.h>
#include "mhd_sockets.h"
#include "connection.h"
#ifdef HTTPS_SUPPORT
#include "connection_https.h"
#endif

#if defined(HAVE_SENDMSG) || defined(HAVE_WRITEV) || \
  defined(MHD_WINSOCK_SOCKETS)
#define MHD_VECT_SEND 1
#endif /* HAVE_SENDMSG || HAVE_WRITEV || MHD_WINSOCK_SOCKETS */

/**
 * Initialises static variables
 */
void
MHD_send_init_static_vars_ (void);


/**
 * Send buffer to the client, push data from network buffer if requested
 * and full buffer is sent.
 *
 * @param connection the MHD_Connection structure
 * @param buffer content of the buffer to send
 * @param buffer_size the size of the @a buffer (in bytes)
 * @param push_data set to true to force push the data to the network from
 *                  system buffers (usually set for the last piece of data),
 *                  set to false to prefer holding incomplete network packets
 *                  (more data will be send for the same reply).
 * @return sum of the number of bytes sent from both buffers or
 *         error code (negative)
 */
ssize_t
MHD_send_data_ (struct MHD_Connection *connection,
                const char *buffer,
                size_t buffer_size,
                bool push_data);


/**
 * Send reply header with optional reply body.
 *
 * @param connection the MHD_Connection structure
 * @param header content of header to send
 * @param header_size the size of the @a header (in bytes)
 * @param never_push_hdr set to true to disable internal algorithm
 *                       that can push automatically header data
 *                       alone to the network
 * @param body content of the body to send (optional, may be NULL)
 * @param body_size the size of the @a body (in bytes)
 * @param complete_response set to true if complete response
 *                          is provided by @a header and @a body,
 *                          set to false if additional body data
 *                          will be sent later
 * @return sum of the number of bytes sent from both buffers or
 *         error code (negative)
 */
ssize_t
MHD_send_hdr_and_body_ (struct MHD_Connection *connection,
                        const char *header,
                        size_t header_size,
                        bool never_push_hdr,
                        const char *body,
                        size_t body_size,
                        bool complete_response);

#if defined(_MHD_HAVE_SENDFILE)
/**
 * Function for sending responses backed by file FD.
 *
 * @param connection the MHD connection structure
 * @return actual number of bytes sent
 */
ssize_t
MHD_send_sendfile_ (struct MHD_Connection *connection);

#endif


/**
 * Set required TCP_NODELAY state for connection socket
 *
 * The function automatically updates sk_nodelay state.
 * @param connection the connection to manipulate
 * @param nodelay_state the requested new state of socket
 * @return true if succeed, false if failed or not supported
 *         by the current platform / kernel.
 */
bool
MHD_connection_set_nodelay_state_ (struct MHD_Connection *connection,
                                   bool nodelay_state);


/**
 * Set required cork state for connection socket
 *
 * The function automatically updates sk_corked state.
 *
 * @param connection the connection to manipulate
 * @param cork_state the requested new state of socket
 * @return true if succeed, false if failed or not supported
 *         by the current platform / kernel.
 */
bool
MHD_connection_set_cork_state_ (struct MHD_Connection *connection,
                                bool cork_state);


/**
 * Function for sending responses backed by a an array of memory buffers.
 *
 * @param connection the MHD connection structure
 * @param r_iov the pointer to iov response structure with tracking
 * @param push_data set to true to force push the data to the network from
 *                  system buffers (usually set for the last piece of data),
 *                  set to false to prefer holding incomplete network packets
 *                  (more data will be send for the same reply).
 * @return actual number of bytes sent
 */
ssize_t
MHD_send_iovec_ (struct MHD_Connection *connection,
                 struct MHD_iovec_track_ *const r_iov,
                 bool push_data);


#endif /* MHD_SEND_H */
