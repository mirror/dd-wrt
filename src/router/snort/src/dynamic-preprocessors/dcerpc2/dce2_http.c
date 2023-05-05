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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_http.h"
#include "snort_dce2.h"
#include "dce2_co.h"
#include "dce2_memory.h"
#include "dce2_stats.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"

#ifdef DUMP_BUFFER
#include "dcerpc2_buffer_dump.h"
#endif

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static DCE2_HttpSsnData * DCE2_HttpSsnInit(void);
static void DCE2_HttpProcess(DCE2_HttpSsnData *);

/********************************************************************
 * Function: DCE2_HttpSsnInit()
 *
 * Creates and initializes an rpc over http session data structure.
 *
 * Arguments: None
 *
 * Returns:
 *  DCE2_HttpSsnData *
 *      Valid pointer to an rpc over http session data structure.
 *      NULL if unable to allocate memory.
 *
 ********************************************************************/
static DCE2_HttpSsnData * DCE2_HttpSsnInit(void)
{
    DCE2_HttpSsnData *hsd = DCE2_Alloc(sizeof(DCE2_HttpSsnData), DCE2_MEM_TYPE__HTTP_SSN);

    if (hsd == NULL)
        return NULL;

    hsd->state = DCE2_HTTP_STATE__NONE;
    DCE2_CoInitTracker(&hsd->co_tracker);

    DCE2_ResetRopts(&hsd->sd.ropts);

    return hsd;
}

/********************************************************************
 * Function: DCE2_HttpProxySsnInit()
 *
 * Wrapper around main session data initialization.  Adds
 * statistical info for a proxy specific rpc over http session.
 *
 * Arguments: None
 *
 * Returns:
 *  DCE2_HttpSsnData *
 *      Valid pointer to an rpc over http session data structure.
 *      NULL if unable to allocate memory.
 *
 ********************************************************************/
DCE2_HttpSsnData * DCE2_HttpProxySsnInit(void)
{
    DCE2_HttpSsnData *hsd = DCE2_HttpSsnInit();

    if (hsd == NULL)
        return NULL;

    dce2_stats.http_proxy_sessions++;

    return hsd;
}

/********************************************************************
 * Function: DCE2_HttpServerSsnInit()
 *
 * Wrapper around main session data initialization.  Adds
 * statistical info for a server specific rpc over http session.
 *
 * Arguments: None
 *
 * Returns:
 *  DCE2_HttpSsnData *
 *      Valid pointer to an rpc over http session data structure.
 *      NULL if unable to allocate memory.
 *
 ********************************************************************/
DCE2_HttpSsnData * DCE2_HttpServerSsnInit(void)
{
    DCE2_HttpSsnData *hsd = DCE2_HttpSsnInit();

    if (hsd == NULL)
        return NULL;

    dce2_stats.http_server_sessions++;

    return hsd;
}

/********************************************************************
 * Function: DCE2_HttpProcessProxy()
 *
 * Wrapper arount main processing point for an RPC over HTTP
 * session.  Checks and sets session setup state for a proxy.
 *
 * Arguments:
 *  DCE2_HttpSsnData *
 *      Pointer to an RPC over HTTP session data structure.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_HttpProcessProxy(DCE2_HttpSsnData *hsd)
{
    const SFSnortPacket *p = hsd->sd.wire_pkt;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Processing RPC over HTTP proxy packet.\n"));
    dce2_stats.http_proxy_pkts++;

    if (hsd->state == DCE2_HTTP_STATE__NONE)
    {
        if (DCE2_SsnFromClient(p))
            hsd->state = DCE2_HTTP_STATE__INIT_CLIENT;
    }

    DCE2_HttpProcess(hsd);
}

/********************************************************************
 * Function: DCE2_HttpProcessServer()
 *
 * Wrapper arount main processing point for an RPC over HTTP
 * session.  Checks and sets session setup state for a server.
 *
 * Arguments:
 *  DCE2_HttpSsnData *
 *      Pointer to an RPC over HTTP session data structure.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_HttpProcessServer(DCE2_HttpSsnData *hsd)
{
    const SFSnortPacket *p = hsd->sd.wire_pkt;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MAIN, "Processing RPC over HTTP server packet.\n"));
    dce2_stats.http_server_pkts++;

    if (hsd->state == DCE2_HTTP_STATE__NONE)
    {
        if (DCE2_SsnFromServer(p))
            hsd->state = DCE2_HTTP_STATE__INIT_SERVER;
    }

    DCE2_HttpProcess(hsd);
}

/********************************************************************
 * Function: DCE2_HttpProcess()
 *
 * Main processing point for an RPC over HTTP session.
 *
 * Arguments:
 *  DCE2_HttpSsnData *
 *      Pointer to an RPC over HTTP session data structure.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_HttpProcess(DCE2_HttpSsnData *hsd)
{
    const SFSnortPacket *p = hsd->sd.wire_pkt;
    const uint8_t *data_ptr = p->payload;
    uint16_t data_len = p->payload_size;

    switch (hsd->state)
    {
        case DCE2_HTTP_STATE__INIT_CLIENT:
#ifdef DUMP_BUFFER
            dumpBuffer(DCERPC_HTTP_STATE_CLIENT_DUMP,data_ptr,data_len);
#endif
            hsd->state = DCE2_HTTP_STATE__INIT_SERVER;
            break;

        case DCE2_HTTP_STATE__INIT_SERVER:
#ifdef DUMP_BUFFER
            dumpBuffer(DCERPC_HTTP_STATE_SERVER_DUMP,data_ptr,data_len);
#endif
            /* Don't really need to look at server response, since if the client
             * RPC_CONNECT request was bad, the TCP session is terminated by
             * the server */
            hsd->state = DCE2_HTTP_STATE__RPC_DATA;
            break;

        case DCE2_HTTP_STATE__RPC_DATA:
#ifdef DUMP_BUFFER
            dumpBuffer(DCERPC_HTTP_STATE_RPC_DATA_DUMP,data_ptr,data_len);
#endif
            DCE2_CoProcess(&hsd->sd, &hsd->co_tracker, data_ptr, data_len);
            break;

        default:
            break;
    }
}

/********************************************************************
 * Function: DCE2_HttpDataFree()
 *
 * Frees dynamically allocated data within the RPC over HTTP
 * session data structure.
 *
 * Arguments:
 *  DCE2_HttpSsnData *
 *      Pointer to an RPC over HTTP session data structure.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_HttpDataFree(DCE2_HttpSsnData *hsd)
{
    if (hsd == NULL)
        return;

    DCE2_CoCleanTracker(&hsd->co_tracker);
}

/********************************************************************
 * Function: DCE2_HttpSsnFree()
 *
 * Frees the session data structure and any dynamically allocated
 * data within it.
 *
 * Arguments:
 *  void *
 *      Pointer to an RPC over HTTP session data structure.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_HttpSsnFree(void *ssn)
{
    DCE2_HttpSsnData *hsd = (DCE2_HttpSsnData *)ssn;

    if (hsd == NULL)
        return;

    DCE2_HttpDataFree(hsd);
    DCE2_Free((void *)hsd, sizeof(DCE2_HttpSsnData), DCE2_MEM_TYPE__HTTP_SSN);
}

