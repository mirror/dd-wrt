/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 *
 ****************************************************************************
 * Provides session handling of an RPC over HTTP transport.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifndef _DCE2_HTTP_H_
#define _DCE2_HTTP_H_

#include "dce2_session.h"
#include "dce2_utils.h"
#include "dce2_co.h"
#include "dce2_tcp.h"
#include "sf_types.h"
#include "sf_snort_packet.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_HTTP_PROXY__RPC_CONNECT_STR "RPC_CONNECT"
#define DCE2_HTTP_SERVER__RPC_VERS_STR   "ncacn_http/1.0"

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_HttpState
{
    DCE2_HTTP_STATE__NONE,
    DCE2_HTTP_STATE__INIT_CLIENT,
    DCE2_HTTP_STATE__INIT_SERVER,
    DCE2_HTTP_STATE__RPC_DATA

} DCE2_HttpState;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_HttpSsnData
{
    DCE2_SsnData sd;
    DCE2_HttpState state;
    DCE2_CoTracker co_tracker;

} DCE2_HttpSsnData;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static inline DCE2_TransType DCE2_HttpAutodetectProxy(const SFSnortPacket *);
static inline DCE2_TransType DCE2_HttpAutodetectServer(const SFSnortPacket *);
static inline int DCE2_HttpDecode(const SFSnortPacket *);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
DCE2_HttpSsnData * DCE2_HttpProxySsnInit(void);
DCE2_HttpSsnData * DCE2_HttpServerSsnInit(void);
void DCE2_HttpProcessProxy(DCE2_HttpSsnData *);
void DCE2_HttpProcessServer(DCE2_HttpSsnData *);
void DCE2_HttpDataFree(DCE2_HttpSsnData *);
void DCE2_HttpSsnFree(void *);

/********************************************************************
 * Function: DCE2_HttpAutodetectProxy()
 *
 * Tries to autodetect an RPC over HTTP proxy.  Looks for session
 * setup strings.
 *
 * Arguments:
 *  const SFSnortPacket *
 *      Pointer to the packet going through the system.
 *
 * Returns:
 *  DCE2_TransType
 *      DCE2_TRANS_TYPE__HTTP_PROXY if a proxy is autodetected.
 *      DCE2_TRANS_TYPE__NONE if a proxy is not autodetected.
 *
 ********************************************************************/
static inline DCE2_TransType DCE2_HttpAutodetectProxy(const SFSnortPacket *p)
{
    const char *buf = NULL;
    unsigned buf_len = 0;

    if (DCE2_SsnFromServer(p))
        return DCE2_TRANS_TYPE__NONE;

    /* Use the http decode buffer if possible */
    if (DCE2_HttpDecode(p))
    {
        buf = (char*)_dpd.getHttpBuffer(HTTP_BUFFER_METHOD, &buf_len);
    }

    if (buf == NULL)
    {
        buf = (char *)p->payload;
        buf_len = p->payload_size;
    }

    if (buf_len >= strlen(DCE2_HTTP_PROXY__RPC_CONNECT_STR))
    {
        if (strncmp(buf, DCE2_HTTP_PROXY__RPC_CONNECT_STR, strlen(DCE2_HTTP_PROXY__RPC_CONNECT_STR)) == 0)
            return DCE2_TRANS_TYPE__HTTP_PROXY;
    }

    return DCE2_TRANS_TYPE__NONE;
}

/********************************************************************
 * Function: DCE2_HttpAutodetectServer()
 *
 * Tries to autodetect an RPC over HTTP server.  Looks for session
 * setup strings.
 *
 * Arguments:
 *  const SFSnortPacket *
 *      Pointer to the packet going through the system.
 *
 * Returns:
 *  DCE2_TransType
 *      DCE2_TRANS_TYPE__HTTP_SERVER if a server is autodetected.
 *      DCE2_TRANS_TYPE__NONE if a server is not autodetected.
 *
 ********************************************************************/
static inline DCE2_TransType DCE2_HttpAutodetectServer(const SFSnortPacket *p)
{
    if (DCE2_SsnFromClient(p))
        return DCE2_TRANS_TYPE__NONE;

    if (p->payload_size >= strlen(DCE2_HTTP_SERVER__RPC_VERS_STR))
    {
        if (strncmp((char *)p->payload, DCE2_HTTP_SERVER__RPC_VERS_STR,
                    strlen(DCE2_HTTP_SERVER__RPC_VERS_STR)) == 0)
        {
            return DCE2_TRANS_TYPE__HTTP_SERVER;
        }
    }

    return DCE2_TRANS_TYPE__NONE;
}

/********************************************************************
 * Function: DCE2_HttpDecode()
 *
 * Returns whether or not this packet was decoded by http_inspect.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet
 *
 * Returns:
 *  int
 *      Non-zero if the packet was http_inspect decoded
 *      Zero if the packet was not http_inspect decoded
 *
 ********************************************************************/
static inline int DCE2_HttpDecode(const SFSnortPacket *p)
{
    return p->flags & FLAG_HTTP_DECODE;
}

#endif  /* _DCE2_HTTP_H_ */

