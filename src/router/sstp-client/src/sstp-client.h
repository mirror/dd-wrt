/*!
 * @brief This is the sstp-client code
 *
 * @file sstp-client.c
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

#ifndef __SSTP_CLIENT_H__
#define __SSTP_CLIENT_H__

/*!
 * @brief Simple peer structure for our oposite end]
 *
 * @par TODO: Add multiple servers + ipv6 support, async lookup w/libevent
 */
typedef struct sstp_peer
{
    /*! The peer name */
    char name[128];

    /*! The address information of our peer */
    struct sockaddr addr;

    /*! The address length */
    int alen;
    
    /*! The peer's ssl session (for re-connect) */
    void *ssl_session;

} sstp_peer_st;


/*!
 * @brief Client context structure
 */
typedef struct
{
    /*! The active server url */
    sstp_url_st *url;

    /*! The server peer */
    sstp_peer_st host;

    /*! The extended options */
    sstp_option_st option;

    /*! The SSL I/O streams */
    sstp_stream_st *stream;

    /*! The pppd context */
    sstp_pppd_st *pppd;

    /*! The HTTP handshake context */
    sstp_http_st *http;

    /*! The SSTP layer state machine */
    sstp_state_st *state;

    /*! The ip-up notification helper */
    sstp_event_st *event;

    /*! The particular server route */
    sstp_route_st route;

    /*! The route context */
    sstp_route_ctx_st *route_ctx;

    /*! The SSL context */
    SSL_CTX *ssl_ctx;

    /*! The event base */
    event_base_st *ev_base;

} sstp_client_st;


#endif  /* #ifndef __SSTP_CLIENT_H__ */
