/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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
 ****************************************************************************/

/**
**  @file       hi_si.c
**
**  @author     Daniel J. Roelker <droelker@sourcefire.com>
**
**  @brief      This file contains functions to select server configurations
**              and begin the HttpInspect process.
**
**  The Session Inspection Module interfaces with the Stream Inspection
**  Module and the User Interface Module to select the appropriate
**  HttpInspect configuration and in the case of stateful inspection the
**  Session Inspection Module retrieves the user-data from the Stream
**  Module.  For stateless inspection, the Session Inspection Module uses
**  the same structure for use by each packet.
**
**  The main responsibility of this module is to supply the appropriate
**  data structures and configurations for the rest of the HttpInspect
**  process.  The module also determines what type of data is being
**  inspected, whether it is client, server, or neither.
**
**  NOTES:
**
**  - 2.25.03:  Initial Development.  DJR
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hi_return_codes.h"
#include "hi_ui_config.h"
#include "hi_ui_server_lookup.h"
#include "hi_si.h"
#include "hi_ad.h"
#include "stream_api.h"

#ifdef TARGET_BASED
extern int16_t hi_app_protocol_id;
#endif

/*
**  NAME
**    IsServer::
*/
/**
**  Given a server configuration and a port number, we decide if the port is
**  in the HTTP server port list.
**
**  @param ServerConf pointer to the server configuration
**  @param port       the port number to compare with the configuration
**  @param pdir       the packet direction (from client, server, etc.)
**
**  @return integer
**
**  @retval  0 means that the port is not a server port
**  @retval !0 means that the port is a server port
*/
static int IsServer(HTTPINSPECT_CONF *ServerConf, unsigned short port)
{
    if(ServerConf->ports[port/8] & (1 << (port%8) ))
    {
        return 1;
    }

    return 0;
}

/*
**  NAME
**    InitServerConf::
*/
/**
**  When a session is initialized, we must select the appropriate server
**  configuration and select the type of inspection based on the source and
**  destination ports.
**
**  IMPORTANT NOTE:
**    We should check to make sure that there are some unique configurations,
**    otherwise we can just default to the global default and work some magic
**    that way.
**
**  @param GlobalConf     pointer to the global configuration
**  @param ServerConf     pointer to the address of the server config so we can
**                        set it.
**  @param SiInput        pointer to the packet info (sip,dip,sport,dport)
**  @param piInspectMode  pointer so we can set the inspection mode
**
**  @return integer
**
**  @retval HI_SUCCESS  function successful
*/
static int InitServerConf(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
                          HTTPINSPECT_CONF **ServerConf,
                          HTTPINSPECT_CONF **ClientConf,
                          HI_SI_INPUT *SiInput, int *piInspectMode, void *ssnptr)
{
    HTTPINSPECT_CONF *ServerConfSip;
    HTTPINSPECT_CONF *ServerConfDip;
    int iServerSip;
    int iServerDip;
    int iErr = 0;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
    int http_id_found = 0;
#endif
    sfaddr_t sip;
    sfaddr_t dip;

    //structure copy
    sip = SiInput->sip;
    dip = SiInput->dip;

    /*
    **  We find the server configurations for both the source and dest. IPs.
    **  There should be a check on the global configuration to see if there
    **  is at least one unique server configuration.  If there isn't then we
    **  assume the global server configuration.
    */
    ServerConfDip = hi_ui_server_lookup_find(GlobalConf->server_lookup,
            &dip,
            &iErr);

    if(!ServerConfDip)
    {
        ServerConfDip = GlobalConf->global_server;
    }

    ServerConfSip = hi_ui_server_lookup_find(GlobalConf->server_lookup,
            &sip,
           &iErr);

    if(!ServerConfSip)
    {
        ServerConfSip = GlobalConf->global_server;
    }

    /*
    **  We check the IP and the port to see if the HTTP server is talking in
    **  the session.  This should tell us whether it is client communication
    **  or server configuration.  If both IPs and ports are servers, then there
    **  is a sort of problem.  We don't know which side is the client and which
    **  side is the server so we have to assume one.
    **
    **  In stateful processing, we only do this stage on the startup of a
    **  session, so we can still assume that the initial packet is the client
    **  talking.
    */
    iServerSip = IsServer(ServerConfSip, SiInput->sport);
    iServerDip = IsServer(ServerConfDip, SiInput->dport);
#ifdef TARGET_BASED
    if (session_api)
    {
        app_id = session_api->get_application_protocol_id(ssnptr);
        if (app_id == hi_app_protocol_id)
        {
            http_id_found = 1;
        }
        if (app_id > 0 && !http_id_found)
        {
            /* This packet was identified as something else. Forget it. */
            iServerSip = 0;
            iServerDip = 0;
        }
    }
#endif

    /*
    **  We default to the no HTTP traffic case
    */
    *piInspectMode = HI_SI_NO_MODE;
    *ServerConf = NULL;

    /*
    **  Depending on the type of packet direction we get from the
    **  state machine, we evaluate client/server differently.
    */
    switch(SiInput->pdir)
    {
        case HI_SI_NO_MODE:
            /*
            **  We check for the case where both SIP and DIP
            **  appear to be servers.  In this case, we assume client
            **  and process that way.
            */
            if(iServerSip && iServerDip)
            {
                *piInspectMode = HI_SI_CLIENT_MODE;
                *ServerConf = ServerConfDip;
                *ClientConf = ServerConfSip;
            }
            else if(iServerSip)
            {
                *piInspectMode = HI_SI_SERVER_MODE;
                *ServerConf = ServerConfSip;
                *ClientConf = ServerConfDip;
            }
            else if(iServerDip)
            {
                *piInspectMode = HI_SI_CLIENT_MODE;
                *ServerConf = ServerConfDip;
                *ClientConf = ServerConfSip;
            }
            break;

        case HI_SI_CLIENT_MODE:
#ifdef TARGET_BASED
            if(iServerDip || http_id_found)
#else
            if(iServerDip)
#endif
            {
                *piInspectMode = HI_SI_CLIENT_MODE;
                *ServerConf = ServerConfDip;
                *ClientConf = ServerConfSip;
            }
            break;

        case HI_SI_SERVER_MODE:
#ifdef TARGET_BASED
            if(iServerSip || http_id_found)
#else
            if(iServerSip)
#endif
            {
                *piInspectMode = HI_SI_SERVER_MODE;
                *ServerConf = ServerConfSip;
                *ClientConf = ServerConfDip;
            }
            break;

        default:
            *piInspectMode = HI_SI_NO_MODE;
            *ServerConf = NULL;
            *ClientConf = NULL;
            break;
    }

    return HI_SUCCESS;
}

/*
**  NAME
**    ResetSession::
*/
/**
**  This function resets all the variables that need to be initialized for
**  a new Session.  I've tried to keep this to a minimum, so we don't have
**  to worry about initializing big structures.
**
**  @param Session  pointer to the session to reset
**
**  @return integer
**
**  @retval HI_SUCCESS
*/
static inline int ResetSession(HI_SESSION *Session)
{
    Session->client.event_list.stack_count      = 0;
    Session->server.event_list.stack_count      = 0;
    Session->anom_server.event_list.stack_count = 0;

    memset(&Session->client.request, 0, sizeof(Session->client.request));
    memset(&Session->server.response, 0, sizeof(Session->server.response));

    return HI_SUCCESS;
}

/*
**  NAME
**    StatelessSessionInspection::
*/
/**
**  Initialize the session and server configurations for this packet/stream.
**
**  It is important to note in stateless mode that we assume no knowledge of the
**  state of a connection, other than the knowledge that we can glean from an
**  individual packet.  So in essence, each packet is it's own session and there
**  is no knowledge retained from one packet to another.  If you want to track
**  an HTTP session for real, use stateful mode.
**
**  In this function, we set the Session pointer (which includes the correct
**  server configuration).  The actual processing to find which IP is the
**  server and which is the client, is done in the InitServerConf() function.
**
**  @param GlobalConf    pointer to the global configuration
**  @param Session       double pointer to the Session structure
**  @param SiInput       pointer to the session information
**  @param piInspectMode pointer so the inspection mode can be set
**
**  @return integer
**
**  @retval HI_SUCCESS function successful
*/
static int StatelessSessionInspection(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
        HI_SESSION **Session, HI_SI_INPUT *SiInput, int *piInspectMode,
        Packet *p)
{
    static HI_SESSION StaticSession;
    HTTPINSPECT_CONF *ServerConf = NULL;
    HTTPINSPECT_CONF *ClientConf = NULL;
    int iRet;

    ResetSession(&StaticSession);

    iRet = InitServerConf(GlobalConf, &ServerConf, &ClientConf, SiInput, piInspectMode, p->ssnptr);
    if (iRet)
    {
        return iRet;
    }

    StaticSession.server_conf = ServerConf;
    StaticSession.client_conf = ClientConf;
    StaticSession.global_conf = GlobalConf;

    *Session = &StaticSession;

    return HI_SUCCESS;
}


/*
**  NAME
**    hi_si_session_inspection::
*/
/**
**  The Session Inspection module selects the appropriate server configuration
**  for the session, and the type of inspection to be performed (client or
**  server.)
**
**  When the Session Inspection module is in stateful mode, it checks to see if
**  there is a HI_SESSION pointer already associated with the stream.  If there
**  is, then it uses that session pointer, otherwise it calculates the server
**  configuration using the HI_SI_INPUT and returns a HI_SESSION pointer.  In
**  stateful mode, this means that memory is allocated, but in stateless mode,
**  the same session pointer is used for all packets to reduce the allocation
**  overhead.
**
**  The inspection mode can be either client, server, or neither.  In the case
**  of neither, the packet is inspected for rogue HTTP servers and HTTP
**  tunneling.
**
**  @param GlobalConf    pointer to the global configuration
**  @param Session       double pointer so the session can be set
**  @param SiInput       session input pointer for data
**  @param piInspectMode pointer for setting inspection mode
**
**  @return integer
**
**  @retval HI_SUCCESS        function successful
**  @retval HI_MEM_ALLOC_FAIL failure to allocate memory
**  @retval HI_INVALID_ARG    argument was invalid (NULL pointers, etc)
*/
int hi_si_session_inspection(HTTPINSPECT_GLOBAL_CONF *GlobalConf,
        HI_SESSION **Session, HI_SI_INPUT *SiInput, int *piInspectMode,
        Packet *p)
{
    int iRet;

    /*
    **  We get the server configuration and the session structure differently
    **  depending on what type of inspection we are doing.  In the case of
    **  stateful processing, we may get the session structure from the Stream
    **  Reassembly module (which includes the server configuration) or the
    **  structure will be allocated and added to the stream pointer for the
    **  rest of the session.
    **
    **  In stateless mode, we just use a static variable that is contained in
    **  the function here.
    */
    iRet = StatelessSessionInspection(GlobalConf, Session, SiInput, piInspectMode, p);
    if (iRet)
        return iRet;

    return HI_SUCCESS;
}

/*
**  NAME
**    GetHttpConf::
*/
/**
**  A wrapper over InitServerConf for getting server/client conf
**
**
**  @param GlobalConf     pointer to the global configuration
**  @param ServerConf     pointer to the address of the server config so we can
**                        set it.
**  @param SiInput        pointer to the packet info (sip,dip,sport,dport)
**  @param piInspectMode  pointer so we can set the inspection mode
**
**  @return integer
**
**  @retval HI_SUCCESS  function successful
*/

int GetHttpConf(
        HTTPINSPECT_GLOBAL_CONF *GlobalConf,
        HTTPINSPECT_CONF **ServerConf,
        HTTPINSPECT_CONF **ClientConf,
        HI_SI_INPUT *SiInput, int *piInspectMode, void *ssnptr)
{
    return  InitServerConf(GlobalConf, ServerConf, ClientConf, SiInput, piInspectMode, ssnptr);
}

