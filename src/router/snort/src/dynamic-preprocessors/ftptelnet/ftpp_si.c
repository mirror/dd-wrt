/*
 * ftpp_si.c
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
 * Kevin Liu <kliu@sourcefire.com>
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
 * Description:
 *
 * This file contains functions to select server configurations
 * and begin the FTPTelnet process.
 *
 * The Session Inspection Module interfaces with the Stream Inspection
 * Module and the User Interface Module to select the appropriate
 * FTPTelnet configuration and in the case of stateful inspection the
 * Session Inspection Module retrieves the user-data from the Stream
 * Module.  For stateless inspection, the Session Inspection Module uses
 * the same structure for use by each packet.
 *
 * The main responsibility of this module is to supply the appropriate
 * data structures and configurations for the rest of the FTPTelnet
 * process.  The module also determines what type of data is being
 * inspected, whether it is client, server, or neither.
 *
 * NOTES:
 * - 20.09.04:  Initial Development.  SAS
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ftpp_return_codes.h"
#include "ftpp_ui_config.h"
#include "ftpp_ui_client_lookup.h"
#include "ftpp_ui_server_lookup.h"
#include "ftpp_si.h"
#include "spp_ftptelnet.h"
#include "stream_api.h"
#include "snort_ftptelnet.h"
#include "sfPolicyUserData.h"
#include "ssl_include.h"
#include "memory_stats.h"

#ifndef WIN32
# include <ctype.h>
#endif

extern tSfPolicyUserContextId ftp_telnet_config;

static FTP_SESSION StaticSession;

/*
 * Function: PortMatch(PROTO_CONF *Conf, unsigned short port)
 *
 * Purpose: Given a configuration and a port number, we decide if
 *          the port is in the port list.
 *
 * Arguments: PROTO_CONF    => pointer to the client or server configuration
 *            port          => the port number to check for
 *
 * Returns: int => 0 indicates the port is not a client/server port.
 *                 1 indicates the port is one of the client/server ports.
 *
 */
static int PortMatch(PROTO_CONF *Conf, unsigned short port)
{
    if(Conf->ports[port])
    {
        return 1;
    }

    return 0;
}

/*
 * Function: TelnetFreeSession(void *preproc_session)
 *
 * Purpose: This function frees the data that is associated with a session.
 *
 * Arguments: preproc_session   => pointer to the session to free
 *
 * Returns: None
 */
static void TelnetFreeSession(void *preproc_session)
{
    TELNET_SESSION *ssn = (TELNET_SESSION *)preproc_session;
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = NULL;

    if (ssn == NULL)
        return;

    pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGet(ssn->global_conf, ssn->policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (ssn->global_conf != ftp_telnet_config))
        {
            sfPolicyUserDataClear (ssn->global_conf, ssn->policy_id);
            FTPTelnetFreeConfig(pPolicyConfig);

            if (sfPolicyUserPolicyGetActive(ssn->global_conf) == 0)
                FTPTelnetFreeConfigs(ssn->global_conf);
        }
    }

    ftp_telnet_stats.telnet_sessions--;
    ftp_telnet_stats.heap_memory -= sizeof(TELNET_SESSION);

    _dpd.snortFree(ssn, sizeof(TELNET_SESSION),
                   PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);
}

/*
 * Function: TelnetResetSession(TELNET_SESSION *Session)
 *
 * Purpose: This function resets all the variables that need to be
 *          initialized for a new Session.  I've tried to keep this to
 *          a minimum, so we don't have to worry about initializing big
 *          structures.
 *
 * Arguments: Session         => pointer to the session to reset
 *
 * Returns: int => return code indicating error or success
 *
 */
static inline int TelnetResetSession(TELNET_SESSION *Session)
{
    Session->ft_ssn.proto = FTPP_SI_PROTO_TELNET;
    Session->telnet_conf = NULL;
    Session->global_conf = NULL;

    Session->consec_ayt = 0;
    Session->encr_state = NO_STATE;

    Session->event_list.stack_count = 0;

    return FTPP_SUCCESS;
}

/*
 * Function: TelnetStatefulSessionInspection(Packet *p,
 *                              FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                              TELNET_SESSION **TelnetSession,
 *                              FTPP_SI_INPUT *SiInput)
 *
 * Purpose: Initialize the session and server configurations for
 *          this packet/stream.  In this function, we set the Session
 *          pointer (which includes the correct server configuration).
 *          The actual processing to find which IP is the server and
 *          which is the client, is done in the InitServerConf() function.
 *
 * Arguments: p             => pointer to the packet/stream
 *            GlobalConf    => pointer to the global configuration
 *            Session       => double pointer to the Session structure
 *            SiInput       => pointer to the session information
 *
 * Returns: int => return code indicating error or success
 *
 */
static int TelnetStatefulSessionInspection(SFSnortPacket *p,
        FTPTELNET_GLOBAL_CONF *GlobalConf,
        TELNET_SESSION **TelnetSession,
        FTPP_SI_INPUT *SiInput)
{
    if (p->stream_session)
    {
        TELNET_SESSION *NewSession = (TELNET_SESSION *)_dpd.snortAlloc(1,
                                                       sizeof(TELNET_SESSION),
                                                       PP_FTPTELNET,
                                                       PP_MEM_CATEGORY_SESSION);

        ftp_telnet_stats.telnet_sessions++;
        if (ftp_telnet_stats.telnet_sessions > ftp_telnet_stats.max_telnet_sessions)
            ftp_telnet_stats.max_telnet_sessions = ftp_telnet_stats.telnet_sessions;

        ftp_telnet_stats.heap_memory += sizeof(TELNET_SESSION); 

        tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();

        if (NewSession == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory for "
                                            "new Telnet session.\n");
        }

        TelnetResetSession(NewSession);

        NewSession->ft_ssn.proto = FTPP_SI_PROTO_TELNET;
        NewSession->telnet_conf = GlobalConf->telnet_config;

        NewSession->global_conf = ftp_telnet_config;
        NewSession->policy_id = policy_id;
        GlobalConf->ref_count++;

        SiInput->pproto = FTPP_SI_PROTO_TELNET;

        _dpd.sessionAPI->set_application_data(p->stream_session,
                PP_FTPTELNET, NewSession, &TelnetFreeSession);

        *TelnetSession = NewSession;
        return FTPP_SUCCESS;
    }

    return FTPP_NONFATAL_ERR;
}

/*
 * Function: TelnetStatelessSessionInspection(Packet *p,
 *                              FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                              TELNET_SESSION **TelnetSession,
 *                              FTPP_SI_INPUT *SiInput)
 *
 * Purpose: Initialize the session and server configurations for this
 *          packet/stream.  It is important to note in stateless mode that
 *          we assume no knowledge of the state of a connection, other
 *          than the knowledge that we can glean from an individual packet.
 *          So in essence, each packet is it's own session and there
 *          is no knowledge retained from one packet to another.  If you
 *          want to track a telnet session for real, use stateful mode.
 *
 *          In this function, we set the Session pointer (which includes
 *          the correct server configuration).  The actual processing to
 *          find which IP is the server and which is the client, is done in
 *          the InitServerConf() function.
 *
 * Arguments: p             => pointer to the packet/stream
 *            GlobalConf    => pointer to the global configuration
 *            Session       => double pointer to the Session structure
 *            SiInput       => pointer to the session information
 *
 * Returns: int => return code indicating error or success
 *
 */
static int TelnetStatelessSessionInspection(SFSnortPacket *p,
        FTPTELNET_GLOBAL_CONF *GlobalConf,
        TELNET_SESSION **Session,
        FTPP_SI_INPUT *SiInput)
{
    static TELNET_SESSION TelnetStaticSession;

    TelnetResetSession(&TelnetStaticSession);

    SiInput->pproto = FTPP_SI_PROTO_TELNET;
    TelnetStaticSession.telnet_conf = GlobalConf->telnet_config;
    TelnetStaticSession.global_conf = ftp_telnet_config;

    *Session = &TelnetStaticSession;

    return FTPP_SUCCESS;
}


/*
 * Function: TelnetSessionInspection(Packet *p,
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          FTPP_SI_INPUT *SiInput,
 *                          int *piInspectMode)
 *
 * Purpose: The Session Inspection module selects the appropriate
 *          configuration for the session, and the type of inspection
 *          to be performed (client or server.)
 *
 *          When the Session Inspection module is in stateful mode, it
 *          checks to see if there is a TELNET_SESSION pointer already
 *          associated with the stream.  If there is, then it uses that
 *          session pointer, otherwise it calculates the server configuration
 *          using the FTP_SI_INPUT and returns a TELNET_SESSION pointer.  In
 *          stateful mode, this means that memory is allocated, but in
 *          stateless mode, the same session pointer is used for all packets
 *          to reduce the allocation overhead.
 *
 *          The inspection mode can be either client or server.
 *
 * Arguments: p             => pointer to the packet/stream
 *            GlobalConf    => pointer to the global configuration
 *            Session       => double pointer to the Session structure
 *            SiInput       => pointer to the session information
 *            piInspectMode => pointer for setting inspection mode
 *
 * Returns: int => return code indicating error or success
 *
 */
int TelnetSessionInspection(SFSnortPacket *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
        TELNET_SESSION **TelnetSession, FTPP_SI_INPUT *SiInput, int *piInspectMode)
{
    int iRet;
    int iTelnetSip;
    int iTelnetDip;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;

    /* If possible, use Stream API to determine protocol. */
    if (_dpd.streamAPI)
    {
        app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);
    }
    if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
    {
        return FTPP_INVALID_PROTO;
    }
    if (app_id == telnet_app_id)
    {
        if (SiInput->pdir == FTPP_SI_CLIENT_MODE ||
            SiInput->pdir == FTPP_SI_SERVER_MODE)
        {
            *piInspectMode = (int)SiInput->pdir;
        }
    }
    else if (app_id && app_id != telnet_app_id)
    {
        return FTPP_INVALID_PROTO;
    }
    else
    {
#endif
        iTelnetSip = PortMatch((PROTO_CONF*)GlobalConf->telnet_config,
                               SiInput->sport);
        iTelnetDip = PortMatch((PROTO_CONF*)GlobalConf->telnet_config,
                               SiInput->dport);

        if (iTelnetSip)
        {
            *piInspectMode = FTPP_SI_SERVER_MODE;
        }
        else if (iTelnetDip)
        {
            *piInspectMode = FTPP_SI_CLIENT_MODE;
        }
        else
        {
            return FTPP_INVALID_PROTO;
        }
#ifdef TARGET_BASED
    }
#endif

    /*
     * We get the server configuration and the session structure differently
     * depending on what type of inspection we are doing.  In the case of
     * stateful processing, we may get the session structure from the Stream
     * Reassembly module (which includes the server configuration) or the
     * structure will be allocated and added to the stream pointer for the
     * rest of the session.
     *
     * In stateless mode, we just use a static variable that is contained in
     * the function here.
     */
    if(GlobalConf->inspection_type == FTPP_UI_CONFIG_STATEFUL)
    {
        iRet = TelnetStatefulSessionInspection(p, GlobalConf, TelnetSession, SiInput);
        if (iRet)
            return iRet;
    }
    else
    {
        /* Assume stateless processing otherwise */
        iRet = TelnetStatelessSessionInspection(p, GlobalConf, TelnetSession, SiInput);
        if (iRet)
            return iRet;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: FTPGetPacketDir(Packet *p)
 *
 * Purpose: Attempts to determine the direction of an FTP packet by
 *          examining the first 3 bytes.  If all three are numeric,
 *          the packet is a server response packet.
 *
 * Arguments: p             => pointer to the Packet
 *
 * Returns: int => return code indicating the mode
 *
 */
int FTPGetPacketDir(SFSnortPacket *p)
{
    if (p->payload_size >= 3)
    {
        if (isdigit(p->payload[0]) &&
            isdigit(p->payload[1]) &&
            isdigit(p->payload[2]) )
        {
            return FTPP_SI_SERVER_MODE;
        }
        else
        {
            return FTPP_SI_CLIENT_MODE;
        }
    }
    return FTPP_SI_NO_MODE;
}

/*
 * Function: FTPInitConf(Packet *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                       FTP_CLIENT_PROTO_CONF **ClientConf,
 *                       FTP_SERVER_PROTO_CONF **ServerConf,
 *                       FTPP_SI_INPUT *SiInput, int *piInspectMode)
 *
 * Purpose: When a session is initialized, we must select the appropriate
 *          server configuration and select the type of inspection based
 *          on the source and destination ports.
 *
 * IMPORTANT NOTE:
 *   We should check to make sure that there are some unique configurations,
 *   otherwise we can just default to the global default and work some magic
 *   that way.
 *
 * Arguments: p                 => pointer to the Packet/Session
 *            GlobalConf        => pointer to the global configuration
 *            ClientConf        => pointer to the address of the client
 *                                 config so we can set it.
 *            ServerConf        => pointer to the address of the server
 *                                 config so we can set it.
 *            SiInput           => pointer to the packet info
 *            piInspectMode     => pointer so we can set the inspection mode
 *
 * Returns: int => return code indicating error or success
 *
 */
static int FTPInitConf(SFSnortPacket *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
                          FTP_CLIENT_PROTO_CONF **ClientConf,
                          FTP_SERVER_PROTO_CONF **ServerConf,
                          FTPP_SI_INPUT *SiInput, int *piInspectMode)
{
    FTP_CLIENT_PROTO_CONF *ClientConfSip;
    FTP_CLIENT_PROTO_CONF *ClientConfDip;
    FTP_SERVER_PROTO_CONF *ServerConfSip;
    FTP_SERVER_PROTO_CONF *ServerConfDip;
    int iServerSip;
    int iServerDip;
    int iErr = 0;
    int iRet = FTPP_SUCCESS;
#ifdef TARGET_BASED
    int16_t app_id = 0;
#endif
    sfaddr_t sip;
    sfaddr_t dip;

    //structure copy
    sip = SiInput->sip;
    dip = SiInput->dip;

    /*
     * We find the client configurations for both the source and dest IPs.
     * There should be a check on the global configuration to see if there
     * is at least one unique client configuration.  If there isn't then we
     * assume the global client configuration.
     */
    ClientConfDip = ftpp_ui_client_lookup_find(GlobalConf->client_lookup,
            &dip,
            &iErr);

    if(!ClientConfDip)
    {
        ClientConfDip = GlobalConf->default_ftp_client;
    }

    ClientConfSip = ftpp_ui_client_lookup_find(GlobalConf->client_lookup,
            &sip,
            &iErr);

    if(!ClientConfSip)
    {
        ClientConfSip = GlobalConf->default_ftp_client;
    }

    /*
     * Now, we find the server configurations for both the source and dest IPs.
     * There should be a check on the global configuration to see if there
     * is at least one unique client configuration.  If there isn't then we
     * assume the global client configuration.
     */
    ServerConfDip = ftpp_ui_server_lookup_find(GlobalConf->server_lookup,
            &dip,
            &iErr);

    if(!ServerConfDip)
    {
        ServerConfDip = GlobalConf->default_ftp_server;
    }

    ServerConfSip = ftpp_ui_server_lookup_find(GlobalConf->server_lookup,
            &sip,
            &iErr);

    if(!ServerConfSip)
    {
        ServerConfSip = GlobalConf->default_ftp_server;
    }

    /*
     * We check the IP and the port to see if the FTP client is talking in
     * the session.  This should tell us whether it is client communication
     * or server configuration.  If both IPs and ports are servers, then there
     * is a sort of problem.  We don't know which side is the client and which
     * side is the server so we have to assume one.
     *
     * In stateful processing, we only do this stage on the startup of a
     * session, so we can still assume that the initial packet is the client
     * talking.
     */
    iServerDip = PortMatch((PROTO_CONF*)ServerConfDip, SiInput->dport);
    iServerSip = PortMatch((PROTO_CONF*)ServerConfSip, SiInput->sport);

    /*
     * We default to the no FTP traffic case
     */
    *piInspectMode = FTPP_SI_NO_MODE;
    *ClientConf = NULL;
    *ServerConf = NULL;

    /*
     * Depending on the type of packet direction we get from the
     * state machine, we evaluate client/server differently.
     */
    switch(SiInput->pdir)
    {
        case FTPP_SI_NO_MODE:

#ifdef TARGET_BASED
            app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

            if (app_id == ftp_app_id || app_id == 0)
            {
#endif

            /*
             * We check for the case where both SIP and DIP
             * appear to be servers.  In this case, we assume server
             * and process that way.
             */
            if(iServerSip && iServerDip)
            {
                /*
                 * We check for the case where both SIP and DIP
                 * appear to be servers.  In this case, we look at
                 * the first few bytes of the packet to try to
                 * determine direction -- 3 digits indicate server
                 * response.
                 */

                /* look at the first few bytes of the packet.  We might
                 * be wrong if this is a reassembled packet and we catch
                 * a server response mid-stream.
                 */
                *piInspectMode = FTPGetPacketDir(p);
                if (*piInspectMode == FTPP_SI_SERVER_MODE)
                {
                    /* Packet is from server --> src is Server */
                    *ClientConf = ClientConfDip;
                    *ServerConf = ServerConfSip;
                }
                else /* Assume client */
                {
                    /* Packet is from client --> dest is Server */
                    *piInspectMode = FTPP_SI_CLIENT_MODE;
                    *ClientConf = ClientConfSip;
                    *ServerConf = ServerConfDip;
                }
                SiInput->pproto = FTPP_SI_PROTO_FTP;
            }
            else if(iServerDip)
            {
                /* Packet is from client --> dest is Server */
                *piInspectMode = FTPP_SI_CLIENT_MODE;
                *ClientConf = ClientConfSip;
                *ServerConf = ServerConfDip;
                SiInput->pproto = FTPP_SI_PROTO_FTP;
            }
            else if(iServerSip)
            {
                /* Packet is from server --> src is Server */
                *piInspectMode = FTPP_SI_SERVER_MODE;
                *ClientConf = ClientConfDip;
                *ServerConf = ServerConfSip;
                SiInput->pproto = FTPP_SI_PROTO_FTP;
            }
            break;

#ifdef TARGET_BASED
            }
#endif

        case FTPP_SI_CLIENT_MODE:
            /* Packet is from client --> dest is Server */
#ifdef TARGET_BASED
            app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

            if ((app_id == ftp_app_id) || (!app_id && iServerDip))
#else
            if(iServerDip)
#endif
            {
                *piInspectMode = FTPP_SI_CLIENT_MODE;
                *ClientConf = ClientConfSip;
                *ServerConf = ServerConfDip;
                SiInput->pproto = FTPP_SI_PROTO_FTP;
            }
            else
            {
                *piInspectMode = FTPP_SI_NO_MODE;
                iRet = FTPP_NONFATAL_ERR;
            }
            break;

        case FTPP_SI_SERVER_MODE:
            /* Packet is from server --> src is Server */
#ifdef TARGET_BASED
            app_id = _dpd.sessionAPI->get_application_protocol_id(p->stream_session);

            if ((app_id == ftp_app_id) || (!app_id && iServerSip))
#else
            if(iServerSip)
#endif
            {
                *piInspectMode = FTPP_SI_SERVER_MODE;
                *ClientConf = ClientConfDip;
                *ServerConf = ServerConfSip;
                SiInput->pproto = FTPP_SI_PROTO_FTP;
            }
            else
            {
                *piInspectMode = FTPP_SI_NO_MODE;
                iRet = FTPP_NONFATAL_ERR;
            }
            break;

        default:
            *piInspectMode = FTPP_SI_NO_MODE;
            *ClientConf = NULL;
            *ServerConf = NULL;
            break;
    }

    return iRet;
}

/*
 * Function: FTPFreeSession(void *preproc_session)
 *
 * Purpose: This function frees the data that is associated with a session.
 *
 * Arguments: preproc_session   => pointer to the session to free
 *
 * Returns: None
 */
static void FTPFreeSession(void *preproc_session)
{
    FTP_SESSION *ssn = (FTP_SESSION *)preproc_session;
    FTPTELNET_GLOBAL_CONF *pPolicyConfig = NULL;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    if (ssn == NULL)
        return;

    pPolicyConfig = (FTPTELNET_GLOBAL_CONF *)sfPolicyUserDataGet(ssn->global_conf, ssn->policy_id);

    if (pPolicyConfig != NULL)
    {
        pPolicyConfig->ref_count--;
        if ((pPolicyConfig->ref_count == 0) &&
            (ssn->global_conf != ftp_telnet_config))
        {

            sfPolicyUserDataClear (ssn->global_conf, ssn->policy_id);
            FTPTelnetFreeConfig(pPolicyConfig);

            if (sfPolicyUserPolicyGetActive(ssn->global_conf) == 0)
                FTPTelnetFreeConfigs(ssn->global_conf);
        }
    }

    if (ssn->filename)
    {
        ftp_telnet_stats.heap_memory -= (strlen(ssn->filename) + 1);
        _dpd.snortFree(ssn->filename, (strlen(ssn->filename) + 1),
                       PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);
    }

    if ( ssl_cb )
        ssl_cb->session_free(ssn->flow_id);

    ftp_telnet_stats.ftp_sessions--;
    ftp_telnet_stats.heap_memory -= sizeof(FTP_SESSION);
#ifdef TARGET_BASED
    FTP_DATA_SESSION *datassn = ssn->datassn;
    if(datassn && (ssn == datassn->ftpssn))
        datassn->ftpssn = NULL;
    _dpd.snortFree(ssn, sizeof(FTP_SESSION),
                   PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);
#endif
}

#ifdef TARGET_BASED
/* Function: FTPDataSessionNew
 *
 * Create an ftp-data session from a packet
 */
FTP_DATA_SESSION * FTPDataSessionNew(SFSnortPacket *p)
{
    FTP_DATA_SESSION *ftpdata = _dpd.snortAlloc(1, sizeof *ftpdata,
                                       PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);

    if (!ftpdata)
        return NULL;

    ftpdata->ft_ssn.proto = FTPP_SI_PROTO_FTP_DATA;
    ftpdata->flow_id = 0;

    /* Get the ftp-ctrl session key */
    ftpdata->ftp_key = _dpd.sessionAPI->get_session_key(p);

    if (!ftpdata->ftp_key)
    {
        _dpd.snortFree(ftpdata, sizeof *ftpdata,
                       PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);
        ftpdata = NULL;
        return ftpdata;
    }

    ftp_telnet_stats.ftp_data_sessions++;
    if (ftp_telnet_stats.ftp_data_sessions > ftp_telnet_stats.max_ftp_data_sessions)
        ftp_telnet_stats.max_ftp_data_sessions = ftp_telnet_stats.ftp_data_sessions;
    
    ftp_telnet_stats.heap_memory += (sizeof (*ftpdata) + sizeof(StreamSessionKey));

    return ftpdata;
}

/*
 * Function: FTPDataSessionFree
 *
 * Free an ftp-data session
 */
void FTPDataSessionFree(void *p_ssn)
{
    FTP_DATA_SESSION *ssn = (FTP_DATA_SESSION *)p_ssn;
    ssl_callback_interface_t *ssl_cb = (ssl_callback_interface_t *)_dpd.getSSLCallback();

    if (!ssn)
        return;

    FTP_SESSION * ftpssn = ssn->ftpssn;
    if(ftpssn && (ssn == ftpssn->datassn))
        ftpssn->datassn = NULL;

    /* ftp-data key shouldn't exist without this but */
    if (ssn->ftp_key)
    {
      // This Key is accounted in Stream during alloc
        _dpd.snortFree(ssn->ftp_key, sizeof(StreamSessionKey),
                       PP_STREAM, PP_MEM_CATEGORY_SESSION);
    }

    if (ssn->filename)
    {
        ftp_telnet_stats.heap_memory -= (strlen(ssn->filename) + 1);
        _dpd.snortFree(ssn->filename, (strlen(ssn->filename) + 1),
                       PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);
    }

    if ( ssl_cb )
        ssl_cb->session_free(ssn->flow_id);

    ftp_telnet_stats.ftp_data_sessions--;
    ftp_telnet_stats.heap_memory -= sizeof(FTP_DATA_SESSION);

    _dpd.snortFree(ssn, sizeof(FTP_DATA_SESSION),
                   PP_FTPTELNET, PP_MEM_CATEGORY_SESSION);
}

/* Function: FTPDataDirection
 *
 * Return true if packet is from the "sending" host
 * Return false if packet is from the "receiving" host
 */
bool FTPDataDirection(SFSnortPacket *p, FTP_DATA_SESSION *ftpdata)
{
    uint32_t direction;
    uint32_t pktdir = _dpd.sessionAPI->get_packet_direction(p);

    if (ftpdata->mode == FTPP_XFER_ACTIVE)
        direction = ftpdata->direction ?  FLAG_FROM_SERVER : FLAG_FROM_CLIENT;
    else
        direction = ftpdata->direction ?  FLAG_FROM_CLIENT : FLAG_FROM_SERVER;

    return (pktdir == direction);
}

#endif /* TARGET_BASED */

/*
 * Function: FTPResetSession(FTP_SESSION *FtpSession, int first)
 *
 * Purpose: This function resets all the variables that need to be
 *          initialized for a new Session.  I've tried to keep this to
 *          a minimum, so we don't have to worry about initializing big
 *          structures.
 *
 * Arguments: FtpSession    => pointer to the session to reset
 *            first         => indicator whether this is a new conf
 *
 * Returns: int => return code indicating error or success
 *
 */
static inline int FTPResetSession(FTP_SESSION *FtpSession)
{
    FtpSession->ft_ssn.proto = FTPP_SI_PROTO_FTP;

    FtpSession->server.response.pipeline_req = 0;
    FtpSession->server.response.state = 0;
    FtpSession->client.request.pipeline_req = 0;
    FtpSession->client.state = 0;

    FtpSession->client_conf = NULL;
    FtpSession->server_conf = NULL;
    FtpSession->global_conf = NULL;

    FtpSession->encr_state = NO_STATE;
    FtpSession->encr_state_chello = false;
    FtpSession->flow_id = 0;
    IP_CLEAR(FtpSession->clientIP);
    FtpSession->clientPort = 0;
    IP_CLEAR(FtpSession->serverIP);
    FtpSession->serverPort = 0;
    FtpSession->data_chan_state = NO_STATE;
    FtpSession->data_chan_index = 0;
    FtpSession->data_xfer_index = 0;
    FtpSession->ftp_cmd_pipe_index = 1;
    FtpSession->rest_cmd_offset = 0;

    FtpSession->event_list.stack_count = 0;

    return FTPP_SUCCESS;
}

/*
 * Function: FTPStatefulSessionInspection(Packet *p,
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          FTP_SESSION **FtpSession,
 *                          FTPP_SI_INPUT *SiInput, int *piInspectMode)
 *
 * Purpose: Initialize the session and server configurations for this
 *          packet/stream.  In this function, we set the Session pointer
 *          (which includes the correct server configuration).  The actual
 *          processing to find which IP is the server and which is the
 *          client, is done in the InitServerConf() function.
 *
 * Arguments: p                 => pointer to the Packet/Session
 *            GlobalConf        => pointer to the global configuration
 *            Session           => double pointer to the Session structure
 *            SiInput           => pointer to the session information
 *            piInspectMode     => pointer so the inspection mode can be set
 *
 * Returns: int => return code indicating error or success
 *
 */
static int FTPStatefulSessionInspection(SFSnortPacket *p,
        FTPTELNET_GLOBAL_CONF *GlobalConf,
        FTP_SESSION **FtpSession,
        FTPP_SI_INPUT *SiInput, int *piInspectMode)
{
    if (p->stream_session)
    {
        FTP_CLIENT_PROTO_CONF *ClientConf;
        FTP_SERVER_PROTO_CONF *ServerConf;
        int iRet;

        iRet = FTPInitConf(p, GlobalConf, &ClientConf, &ServerConf, SiInput, piInspectMode);
        if (iRet)
            return iRet;

        if (*piInspectMode)
        {
            FTP_SESSION *NewSession = (FTP_SESSION *)_dpd.snortAlloc(1,
                                                      sizeof(FTP_SESSION),
                                                      PP_FTPTELNET, 
                                                      PP_MEM_CATEGORY_SESSION);

            ftp_telnet_stats.ftp_sessions++;
            if (ftp_telnet_stats.ftp_sessions > ftp_telnet_stats.max_ftp_sessions)
                ftp_telnet_stats.max_ftp_sessions = ftp_telnet_stats.ftp_sessions;
   
            ftp_telnet_stats.heap_memory += sizeof(FTP_SESSION);

            tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();

            if (NewSession == NULL)
            {
                DynamicPreprocessorFatalMessage("Failed to allocate memory for "
                                                "new FTP session.\n");
            }

            FTPResetSession(NewSession);

            NewSession->ft_ssn.proto = FTPP_SI_PROTO_FTP;
            NewSession->client_conf = ClientConf;
            NewSession->server_conf = ServerConf;

            NewSession->global_conf = ftp_telnet_config;
            NewSession->policy_id = policy_id;
            GlobalConf->ref_count++;

            _dpd.sessionAPI->set_application_data
                (p->stream_session, PP_FTPTELNET, NewSession, &FTPFreeSession);

            *FtpSession = NewSession;
            SiInput->pproto = FTPP_SI_PROTO_FTP;
            return FTPP_SUCCESS;
        }
    }

    return FTPP_INVALID_PROTO;
}

/*
 * Function: FTPStatelessSessionInspection(Packet *p,
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          FTP_SESSION **FtpSession,
 *                          FTPP_SI_INPUT *SiInput, int *piInspectMode)
 *
 * Purpose: Initialize the session and server configurations for this
 *          packet/stream.  It is important to note in stateless mode that
 *          we assume no knowledge of the state of a connection, other than
 *          the knowledge that we can glean from an individual packet.  So
 *          in essence, each packet is it's own session and there is no
 *          knowledge retained from one packet to another.  If you want to
 *          track an FTP session for real, use stateful mode.
 *
 *          In this function, we set the Session pointer (which includes
 *          the correct server configuration).  The actual processing to find
 *          which IP is the server and which is the client, is done in the
 *          InitServerConf() function.
 *
 * Arguments: p                 => pointer to the Packet/Session
 *            GlobalConf        => pointer to the global configuration
 *            Session           => double pointer to the Session structure
 *            SiInput           => pointer to the session information
 *            piInspectMode     => pointer so the inspection mode can be set
 *
 * Returns: int => return code indicating error or success
 *
 */
static int FTPStatelessSessionInspection(SFSnortPacket *p,
        FTPTELNET_GLOBAL_CONF *GlobalConf,
        FTP_SESSION **FtpSession,
        FTPP_SI_INPUT *SiInput, int *piInspectMode)
{
    FTP_CLIENT_PROTO_CONF *ClientConf;
    FTP_SERVER_PROTO_CONF *ServerConf;
    int iRet;

    FTPResetSession(&StaticSession);

    iRet = FTPInitConf(p, GlobalConf, &ClientConf, &ServerConf, SiInput, piInspectMode);
    if (iRet)
        return iRet;

    StaticSession.ft_ssn.proto = FTPP_SI_PROTO_FTP;
    StaticSession.global_conf = ftp_telnet_config;
    StaticSession.client_conf = ClientConf;
    StaticSession.server_conf = ServerConf;

    SiInput->pproto = FTPP_SI_PROTO_FTP;
    *FtpSession = &StaticSession;

    return FTPP_SUCCESS;
}


/*
 * Function: FTPSessionInspection(Packet *p,
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          FTPP_SI_INPUT *SiInput, int *piInspectMode)
 *
 * Purpose: The Session Inspection module selects the appropriate client
 *          configuration for the session, and the type of inspection to
 *          be performed (client or server.)
 *
 *          When the Session Inspection module is in stateful mode, it
 *          checks to see if there is a FTP_SESSION pointer already
 *          associated with the stream.  If there is, then it uses that
 *          session pointer, otherwise it calculates the server
 *          configuration using the FTP_SI_INPUT and returns a FTP_SESSION
 *          pointer.  In stateful mode, this means that memory is allocated,
 *          but in stateless mode, the same session pointer is used for all
 *          packets to reduce the allocation overhead.
 *
 *          The inspection mode can be either client or server.
 *
 * Arguments: p                 => pointer to the Packet/Session
 *            GlobalConf        => pointer to the global configuration
 *            SiInput           => pointer to the session information
 *            piInspectMode     => pointer so the inspection mode can be set
 *
 * Returns: int => return code indicating error or success
 *
 */
int FTPSessionInspection(SFSnortPacket *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
        FTP_SESSION **FtpSession, FTPP_SI_INPUT *SiInput, int *piInspectMode)
{
    int iRet;

    /*
     * We get the server configuration and the session structure differently
     * depending on what type of inspection we are doing.  In the case of
     * stateful processing, we may get the session structure from the Stream
     * Reassembly module (which includes the server configuration) or the
     * structure will be allocated and added to the stream pointer for the
     * rest of the session.
     *
     * In stateless mode, we just use a static variable that is contained in
     * the function here.
     */
    if(GlobalConf->inspection_type == FTPP_UI_CONFIG_STATEFUL)
    {
        iRet = FTPStatefulSessionInspection(p, GlobalConf, FtpSession, SiInput, piInspectMode);
        if (iRet)
            return iRet;
    }
    else
    {
        /* Assume stateless processing otherwise */
        iRet = FTPStatelessSessionInspection(p, GlobalConf, FtpSession, SiInput, piInspectMode);
        if (iRet)
            return iRet;
    }

    return FTPP_SUCCESS;
}

/*
 * Function: ftpp_si_determine_proto(Packet *p,
 *                          FTPTELNET_GLOBAL_CONF *GlobalConf,
 *                          FTPP_SI_INPUT *SiInput, int *piInspectMode)
 *
 * Purpose: The Protocol Determination module determines whether this is
 *          an FTP or telnet request.  If this is an FTP request, it sets
 *          the FTP Session data and inspection mode.
 *
 *          The inspection mode can be either client or server.
 *
 * Arguments: p                 => pointer to the Packet/Session
 *            GlobalConf        => pointer to the global configuration
 *            SiInput           => pointer to the session information
 *            piInspectMode     => pointer so the inspection mode can be set
 *
 * Returns: int => return code indicating error or success
 *
 */
int ftpp_si_determine_proto(SFSnortPacket *p, FTPTELNET_GLOBAL_CONF *GlobalConf,
        FTP_TELNET_SESSION **ft_ssn, FTPP_SI_INPUT *SiInput, int *piInspectMode)
{
    /* Default to no FTP or Telnet case */
    SiInput->pproto = FTPP_SI_PROTO_UNKNOWN;
    *piInspectMode = FTPP_SI_NO_MODE;

    TelnetSessionInspection(p, GlobalConf, (TELNET_SESSION **)ft_ssn, SiInput, piInspectMode);
    if (SiInput->pproto == FTPP_SI_PROTO_TELNET)
        return FTPP_SUCCESS;

    FTPSessionInspection(p, GlobalConf, (FTP_SESSION **)ft_ssn, SiInput, piInspectMode);
    if (SiInput->pproto == FTPP_SI_PROTO_FTP)
        return FTPP_SUCCESS;

    return FTPP_INVALID_PROTO;
}
