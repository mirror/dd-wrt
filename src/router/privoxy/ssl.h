#ifndef SSL_H_INCLUDED
#define SSL_H_INCLUDED
/*********************************************************************
*
* File        :  $Source: $
*
* Purpose     :  File with TLS/SSL extension. Contains methods for
*                creating, using and closing TLS/SSL connections.
*
* Copyright   :  Written by and Copyright (c) 2017 Vaclav Svec. FIT CVUT.
*
*                This program is free software; you can redistribute it
*                and/or modify it under the terms of the GNU General
*                Public License as published by the Free Software
*                Foundation; either version 2 of the License, or (at
*                your option) any later version.
*
*                This program is distributed in the hope that it will
*                be useful, but WITHOUT ANY WARRANTY; without even the
*                implied warranty of MERCHANTABILITY or FITNESS FOR A
*                PARTICULAR PURPOSE.  See the GNU General Public
*                License for more details.
*
*                The GNU General Public License should be included with
*                this file.  If not, you can view it at
*                http://www.gnu.org/copyleft/gpl.html
*                or write to the Free Software Foundation, Inc., 59
*                Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*********************************************************************/

#include "project.h"

/* Boolean functions to get information about TLS/SSL connections */
extern int    client_use_ssl(const struct client_state *csp);
extern int    server_use_ssl(const struct client_state *csp);
extern size_t is_ssl_pending(struct ssl_attr *ssl_attr);
extern int tunnel_established_successfully(const char *response, unsigned int response_len);

/* Functions for sending and receiving data over TLS/SSL connections */
extern int  ssl_send_data(struct ssl_attr *ssl_attr, const unsigned char *buf, size_t len);
extern int ssl_send_data_delayed(struct ssl_attr *ssl_attr, const unsigned char *buf,
                                 size_t len, unsigned int delay);
extern int  ssl_recv_data(struct ssl_attr *ssl_attr, unsigned char *buf, size_t maxLen);
extern long ssl_flush_socket(struct ssl_attr *ssl_attr, struct iob *iob);
extern void ssl_send_certificate_error(struct client_state *csp);

/* Functions for opening and closing TLS/SSL connections */
extern int  create_client_ssl_connection(struct client_state *csp);
extern int  create_server_ssl_connection(struct client_state *csp);
extern void close_client_and_server_ssl_connections(struct client_state *csp);
extern void close_server_ssl_connection(struct client_state *csp);
extern void close_client_ssl_connection(struct client_state *csp);

/* misc helper functions */
extern int ssl_base64_encode(unsigned char *dst, size_t dlen, size_t *olen,
                             const unsigned char *src, size_t slen );
extern void ssl_crt_verify_info(char *buf, size_t size, struct client_state *csp);

#ifdef FEATURE_GRACEFUL_TERMINATION
extern void ssl_release(void);
#endif

#endif /* ndef SSL_H_INCLUDED */
