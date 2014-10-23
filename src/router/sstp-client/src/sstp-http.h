/*!
 * @brief This process the HTTP handshake for SSTP
 *
 * @file sstp-http.h
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
#ifndef __SSTP_HTTP_H__
#define __SSTP_HTTP_H__

/*! The default path to invoke the SSTP API */
#define SSTP_HTTP_DFLT_PATH         \
    "/sra_{BA195980-CD49-458b-9E23-C84EE0ADCD75}/"

/*! The format of the HTTP PROXY connect */
#define SSTP_HTTP_PROXY_CONNECT_FMT \
    "CONNECT %s:443 HTTP/1.1\r\n"   \
    "SSTPVERSION: 1.0\r\n"          \
    "Connection: keep-alive\r\n"    \
    "User-Agent: %s\r\n"

/*! The appended property with user/pass */
#define SSTP_HTTP_PROXY_AUTH_FMT    \
    "Proxy-Authorization: %s"


/*< Forward declare the http context */
struct sstp_http;
typedef struct sstp_http sstp_http_st;


/*!
 * @brief HTTP hanshake complete callback
 */
typedef void (*sstp_http_done_fn)(void *ctx, int result);


/*!
 * @brief Create a HTTP context
 */
status_t sstp_http_create(sstp_http_st **http, const char *server,
        sstp_http_done_fn done_cb, void *uarg, int mode);


/*!
 * @brief Perform a SSTP handshake
 */
status_t sstp_http_handshake(sstp_http_st *http, sstp_stream_st *stream);


/*!
 * @brief Perform the Proxy connect
 */
status_t sstp_http_proxy(sstp_http_st *http, sstp_stream_st *stream);


/*!
 * @brief Set the credentials
 */
void sstp_http_setcreds(sstp_http_st *http, const char *user,
        const char *password);


/*!
 * @brief Set the UUID of the connection
 */
void sstp_http_setuuid(sstp_http_st *http, const char *uuid);


/*! 
 * @brief Free the HTTP context
 */
void sstp_http_free(sstp_http_st *http);


#endif /* #ifndef __SSTP_HTTP_H__ */
