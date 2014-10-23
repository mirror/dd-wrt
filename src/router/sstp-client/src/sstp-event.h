/*!
 * @brief Event API for sstp-client
 *
 * @file sstp-event.c
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
#ifndef __SSTP_EVENT_H__
#define __SSTP_EVENT_H__

/*< Forward declare event context */
struct sstp_event;
typedef struct sstp_event sstp_event_st;


/*!
 * @brief A callback function for when waiting for ip-up from pppd
 */
typedef void (*sstp_event_fn)(void *ctx, int status);


/*!
 * @brief Create an event to listen for callback
 */
status_t sstp_event_create(sstp_event_st **ctx, sstp_option_st *opts,
        event_base_st *base, sstp_event_fn event_cb, void *arg);


/*! 
 * @brief Get the socket name for the callback
 */
const char *sstp_event_sockname(sstp_event_st *ctx);


/*!
 * @brief Get the results from the event
 */
status_t sstp_event_mppe_result(sstp_event_st *ctx, uint8_t **skey, 
        size_t *slen, uint8_t **rkey, size_t *rlen);

/*!
 * @brief Shutdown and remove the socket
 */
void sstp_event_free(sstp_event_st *ctx);


#endif
