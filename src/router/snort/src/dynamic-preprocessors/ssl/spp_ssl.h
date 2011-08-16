/*
** Copyright (C) 2007-2011 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
 * Adam Keeton
 * spp_ssl.h
 * 10/10/07
 */

#ifndef SPP_SSLPP_H
#define SPP_SSLPP_H

#include "sf_types.h"
#include "sfcommon.h"
#include "ssl.h"

/* Prototypes for public interface */
extern void SetupSSLPP(void);

/* Configuration flags */
#define SSLPP_DISABLE_FLAG    0x0001 
#define SSLPP_TRUSTSERVER_FLAG  0x0002

typedef struct _SSLPP_config 
{
    ports_tbl_t ports;
    uint16_t flags;

} SSLPP_config_t;

typedef struct 
{
    /*
     * SSL preprocessor global configuration structure.
     */
    SSLPP_config_t config;

} tSslPolicyConfig;

extern tSslPolicyConfig sslPolicyConfig[];

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

#define SSLPP_TRUE 1
#define SSLPP_FALSE 0

#define SSLPP_ENCRYPTED_FLAGS (SSL_HS_SDONE_FLAG | SSL_CLIENT_KEYX_FLAG | \
                               SSL_CAPP_FLAG | SSL_SAPP_FLAG)
#define SSLPP_ENCRYPTED_FLAGS2 (SSL_HS_SDONE_FLAG | SSL_CHANGE_CIPHER_FLAG | \
                                SSL_CAPP_FLAG | SSL_SAPP_FLAG)

#define GENERATOR_SPP_SSLPP 137
#define     SSL_INVALID_CLIENT_HELLO               1
#define     SSL_INVALID_SERVER_HELLO               2

#define SSL_INVALID_CLIENT_HELLO_STR "(ssp_ssl) Invalid Client HELLO after Server HELLO Detected"
#define SSL_INVALID_SERVER_HELLO_STR "(ssp_ssl) Invalid Server HELLO without Client HELLO Detected"
    
#endif /* SPP_SSLPP_H */
