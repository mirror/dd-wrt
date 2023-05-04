/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * File: ssl_inspect.h
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 * Brief: Header file for SSL inspection
*/

#ifndef SSL_INSPECT_H
#define SSL_INSPECT_H

/*********************INCLUDES***************/
#include "ssl_config.h"

/*********************DEFINES***************/

#define SSLPP_ENCRYPTED_FLAGS (SSL_HS_SDONE_FLAG | SSL_CLIENT_KEYX_FLAG | \
                               SSL_CAPP_FLAG | SSL_SAPP_FLAG)
#define SSLPP_ENCRYPTED_FLAGS2 (SSL_HS_SDONE_FLAG | SSL_CHANGE_CIPHER_FLAG | \
                                SSL_CAPP_FLAG | SSL_SAPP_FLAG)

#define GENERATOR_SPP_SSLPP 137
#define     SSL_INVALID_CLIENT_HELLO               1
#define     SSL_INVALID_SERVER_HELLO               2
#define     SSL_ALERT_HB_REQUEST                   3
#define     SSL_ALERT_HB_RESPONSE                  4

#define SSL_INVALID_CLIENT_HELLO_STR "(spp_ssl) Invalid Client HELLO after Server HELLO Detected"
#define SSL_INVALID_SERVER_HELLO_STR "(spp_ssl) Invalid Server HELLO without Client HELLO Detected"
#define SSL_HEARTBLEED_REQUEST_STR "(spp_ssl) Heartbeat Read Overrun Attempt Detected"
#define SSL_HEARTBLEED_RESPONSE_STR "(spp_ssl) Large Heartbeat Response Detected"
    
/*********************DATASTRUCTURES***************/
typedef struct _SSLPP_counters
{
    uint64_t stopped;
    uint64_t disabled;
    uint64_t decoded;
    uint64_t alerts;
    uint64_t cipher_change;
    uint64_t unrecognized;
    uint64_t completed_hs;
    uint64_t bad_handshakes;
    uint64_t hs_chello;
    uint64_t hs_shello;
    uint64_t hs_cert;
    uint64_t hs_skey;
    uint64_t hs_ckey;
    uint64_t hs_finished;
    uint64_t hs_sdone;
    uint64_t capp;
    uint64_t sapp;

} SSLPP_counters_t;

#endif /* SSL_INSPECT_H */
