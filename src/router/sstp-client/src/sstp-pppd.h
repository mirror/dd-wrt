/*!
 * @brief Managing the interface with pppd
 *
 * @file sstp-pppd.h
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
#ifndef __SSTP_PPPD_H__
#define __SSTP_PPPD_H__

/*! Auth using MS-CHAP-V2 */
#define SSTP_PPP_AUTH_CHAP  0xc223

/*! Auth using PAP */
#define SSTP_PPP_AUTH_PAP   0xc023

/*! Check when IPCP layer is up */
#define SSTP_PPP_IPCP       0x8021

struct sstp_pppd;
typedef struct sstp_pppd sstp_pppd_st;


/*! 
 * @brief PPP state events
 */
typedef enum
{
    SSTP_PPP_DOWN = 1,
    SSTP_PPP_UP   = 2,
    SSTP_PPP_AUTH = 3,

} sstp_pppd_event_t;


/*!
 * @brief PPP Session details
 */ 
typedef struct 
{
    /*< The established session length */
    unsigned long established;

    /*< The number of bytes received from server */
    unsigned long long rx_bytes;

    /*< The number of bytes sent to server */
    unsigned long long tx_bytes;

} sstp_session_st;


/*!
 * @brief Client callback
 */
typedef void (*sstp_pppd_fn)(void *ctx, sstp_pppd_event_t ev);


/*!
 * @brief Log the pppd session
 */
void sstp_pppd_session_details(sstp_pppd_st *ctx, sstp_session_st *sess);


/*!
 * @brief Return the chap context
 */
sstp_chap_st *sstp_pppd_getchap(sstp_pppd_st *ctx);


/*!
 * @brief Start the PPP negotiations
 */
status_t sstp_pppd_start(sstp_pppd_st *ctx, sstp_option_st *opts, 
    const char *sockname);

/*!
 * @brief Try to terminate the PPP process
 */
status_t sstp_pppd_stop(sstp_pppd_st *ctx);


/*!
 * @brief Forward data back to the pppd daemon from server
 */
status_t sstp_pppd_send(sstp_pppd_st *ctx, const char *buf, int len);


/*!
 * @brief Create the pppd context
 */
status_t sstp_pppd_create(sstp_pppd_st **ctx, event_base_st *base, 
    sstp_stream_st *stream, sstp_pppd_fn notify, void *arg);


/*!
 * @brief Free the pppd context
 */
void sstp_pppd_free(sstp_pppd_st *ctx);


#endif /* #ifndef __SSTP_SSL_H__ */
