/* $Id */

/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2011-2013 Sourcefire, Inc.
 **
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
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


/*
 * SIP preprocessor
 *
 * This is the main entry point for this preprocessor
 *
 * Author: Hui Cao
 * Date: 03-15-2011
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_plugin_api.h"
#include "snort_debug.h"

#include "preprocids.h"
#include "spp_sip.h"
#include "sip_config.h"
#include "sip_roptions.h"
#include "sip_parser.h"
#include "sip_dialog.h"
#include "sip_paf.h"

#include <assert.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#ifdef REG_TEST
#include "reg_test.h"
#endif

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats sipPerfStats;
#endif

#include "sf_types.h"

#ifdef DUMP_BUFFER
#include "sip_buffer_dump.h"
#endif

#ifdef SNORT_RELOAD
#include "appdata_adjuster.h"
#endif

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 1;

const char *PREPROC_NAME = "SF_SIP";

#define SetupSIP DYNAMIC_PREPROC_SETUP

#ifdef TARGET_BASED
int16_t sip_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

/*
 * Session state flags for SIPData::state_flags
 */

#define SIP_FLG_MISSED_PACKETS        (0x10000)

/*
 * Function prototype(s)
 */
SIPData * SIPGetNewSession(SFSnortPacket *, tSfPolicyId);
static void SIPInit( struct _SnortConfig *, char* );
static bool SIPGlobalIsEnabled(struct _SnortConfig *sc, tSfPolicyUserContextId sip_config);
static int SIPPolicyIsEnabled(struct _SnortConfig *sc, tSfPolicyUserContextId pContext, tSfPolicyId policyId, void* config);
static int SIPCheckConfig(struct _SnortConfig *);
static void FreeSIPData( void* );
static inline int SIP_Process(SFSnortPacket *, SIPData*);
static void SIPmain( void*, void* );
static inline int CheckSIPPort( uint16_t );
static void SIPFreeConfig(tSfPolicyUserContextId);
static void registerPortsForDispatch( struct _SnortConfig *sc, SIPConfig *policy );
static void registerPortsForReassembly( SIPConfig *policy, int direction );
static void _addPortsToStreamFilter(struct _SnortConfig *, SIPConfig *, tSfPolicyId);
static void SIP_PrintStats(int);
static void DisplaySIPStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif

static void SIPCleanExit(int, void *);

/********************************************************************
 * Global variables
 ********************************************************************/
uint32_t numSessions = 0;
SIP_Stats sip_stats;
SIPConfig *sip_eval_config;
tSfPolicyUserContextId sip_config;

#ifdef SNORT_RELOAD
static APPDATA_ADJUSTER *ada;
#endif

static size_t SIP_NumSessions()
{
    return (size_t) numSessions;
}

#ifdef SNORT_RELOAD
static void SIPReload(struct _SnortConfig *, char *, void **);
static int SIPReloadVerify(struct _SnortConfig *, void *);
static void * SIPReloadSwap(struct _SnortConfig *, void *);
static void SIPReloadSwapFree(void *);
#endif

static SIPMsg sipMsg;

int SIPPrintMemStats(FILE *fd, char *buffer, PreprocMemInfo *meminfo)
{
    int len = 0;
    time_t curr_time;

    if (fd)
    {
        len = fprintf(fd, ",%lu,%u"
                 ",%lu,%u,%u"
                 ",%lu,%u,%u,%lu"
                 , sip_stats.sessions
                 , numSessions
                 , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
                 , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
                 , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
                 , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
                 , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
                 , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
                 , meminfo[PP_MEM_CATEGORY_SESSION].used_memory +
                   meminfo[PP_MEM_CATEGORY_CONFIG].used_memory);

        return len;
    }

    curr_time = time(NULL); 

    if (buffer)
    {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "\n\nMemory Statistics of SIP on: %s\n"
            "    Total Sessions          : %lu\n"
            "    Current Active Sessions : %u\n\n"
            , ctime(&curr_time)
            , sip_stats.sessions
            , numSessions);
    } else {
        _dpd.logMsg("\n");
        _dpd.logMsg("Memory Statistics of SIP on: %s\n", ctime(&curr_time));
        _dpd.logMsg("    Total Sessions          : %lu\n", sip_stats.sessions); 
        _dpd.logMsg("    Current Active Sessions : %u\n\n", numSessions);
    }

    return len;
}

/* Called at preprocessor setup time. Links preprocessor keyword
 * to corresponding preprocessor initialization function.
 *
 * PARAMETERS:	None.
 *
 * RETURNS:	Nothing.
 *
 */
void SetupSIP(void)
{
    _dpd.registerMemoryStatsFunc(PP_SIP, SIPPrintMemStats);
    /* Link preprocessor keyword to initialization function
     * in the preprocessor list. */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc( "sip", SIPInit );
#else
    _dpd.registerPreproc("sip", SIPInit, SIPReload,
            SIPReloadVerify, SIPReloadSwap, SIPReloadSwapFree);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getSIPBuffers, SIP_BUFFER_DUMP_FUNC);
#endif
}

SIPConfig *getParsingSIPConfig(struct _SnortConfig *sc)
{
    SIPConfig * sip_parsing_config;
#ifdef SNORT_RELOAD
    tSfPolicyUserContextId sip_swap_config = (tSfPolicyUserContextId)_dpd.getRelatedReloadData(sc, "sip");
    if (sip_swap_config)
        sip_parsing_config = sfPolicyUserDataGetCurrent(sip_swap_config);
    else
#endif
        sip_parsing_config = sfPolicyUserDataGetCurrent(sip_config);
    return sip_parsing_config;
}

#ifdef REG_TEST
static inline void PrintSIPSize(void)
{
    _dpd.logMsg("\nSIP Session Size: %lu\n", (long unsigned int)sizeof(SIPData));
}
#endif

/* Initializes the SIP preprocessor module and registers
 * it in the preprocessor list.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to argument string to process for config
 *                      data.
 *
 * RETURNS:     Nothing.
 */
static void SIPInit(struct _SnortConfig *sc, char *argp)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SIPConfig *pDefaultPolicyConfig = NULL;
    SIPConfig *pPolicyConfig = NULL;

#ifdef REG_TEST
    PrintSIPSize();
#endif
	/* For SFR CLI */
	_dpd.controlSocketRegisterHandler(CS_TYPE_SIP_STATS, NULL, NULL, &DisplaySIPStats);

    if (sip_config == NULL)
    {
        //create a context
        sip_config = sfPolicyConfigCreate();
        if (sip_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                    "for SIP config.\n");
        }

        _dpd.addPreprocConfCheck(sc, SIPCheckConfig);
        _dpd.registerPreprocStats(SIP_NAME, SIP_PrintStats);
        _dpd.addPreprocExit(SIPCleanExit, NULL, PRIORITY_LAST, PP_SIP);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("sip", (void *)&sipPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

#ifdef TARGET_BASED
        sip_app_id = _dpd.findProtocolReference("sip");
        if (sip_app_id == SFTARGET_UNKNOWN_PROTOCOL)
            sip_app_id = _dpd.addProtocolReference("sip");

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_SIP, sip_app_id );

#endif
    }

    sfPolicyUserPolicySet (sip_config, policy_id);
    pDefaultPolicyConfig = (SIPConfig *)sfPolicyUserDataGetDefault(sip_config);
    pPolicyConfig = (SIPConfig *)sfPolicyUserDataGetCurrent(sip_config);
    if ((pPolicyConfig != NULL) && (pDefaultPolicyConfig == NULL))
    {
        DynamicPreprocessorFatalMessage("SIP preprocessor can only be "
                "configured once.\n");
    }

    pPolicyConfig = (SIPConfig *)_dpd.snortAlloc(1, sizeof(SIPConfig), PP_SIP, 
                                                 PP_MEM_CATEGORY_CONFIG);
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                "SIP preprocessor configuration.\n");
    }

    sfPolicyUserDataSetCurrent(sip_config, pPolicyConfig);
    SIP_RegRuleOptions(sc);
    ParseSIPArgs(pPolicyConfig, (u_char *)argp);
#ifdef SNORT_RELOAD
    pDefaultPolicyConfig = (SIPConfig *)sfPolicyUserDataGetDefault(sip_config);
    //we don't know the order in which policies are init
    //maybe default (policy 0) isn't init until after another policy is init
    //however a default policy is guranteed
    //avoid core
    //Also, if SIP isn't enabled, then why waste memory?
#ifdef REG_TEST
    if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
    {
        printf("SIP-reload SIPInit-before : %p\n", ada);
    }
#endif
    if (pDefaultPolicyConfig != NULL && ada == NULL && SIPGlobalIsEnabled(sc, sip_config))
    {
        ada = ada_init(SIP_NumSessions, PP_SIP, (size_t)pDefaultPolicyConfig->maxNumSessions);
        if (ada == NULL)
            DynamicPreprocessorFatalMessage("Could not allocate memory for SIP ada\n");
    }
#ifdef REG_TEST
    if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
    {
        printf("SIP-reload SIPInit-after : %p\n", ada);
    }
#endif
#endif

}

/*********************************************************************
 * Overload PCRE options: this is to support the "H"
 *
 * For SIP messages, uri Buffers will point to SIP instead of HTTP
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet structure
 *
 * Returns:
 *  None
 *
 *********************************************************************/
static inline void SIP_overloadURI(SFSnortPacket *p, SIPMsg *sipMsg)
{
    if ( sipMsg->header )
        _dpd.setHttpBuffer(HTTP_BUFFER_HEADER, sipMsg->header, sipMsg->headerLen);

    if ( sipMsg->body_data )
        _dpd.setHttpBuffer(HTTP_BUFFER_CLIENT_BODY, sipMsg->body_data, sipMsg->bodyLen);
}


/*********************************************************************
 * Main entry point for SIP processing.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet structure
 *
 * Returns:
 *  int - 	SIP_SUCCESS
 *		    SIP_FAILURE
 *
 *********************************************************************/
static inline int SIP_Process(SFSnortPacket *p, SIPData* sessp)
{
    int status;
    char* sip_buff = (char*) p->payload;
    char* end;
    SIP_Roptions *pRopts;

    memset(&sipMsg, 0, SIPMSG_ZERO_LEN);


    /*Input parameters*/
    sipMsg.isTcp = IsTCP(p);

    end =  sip_buff + p->payload_size;


    status = sip_parse(&sipMsg, sip_buff, end);

    if (SIP_SUCCESS == status)
    {
        SIP_overloadURI(p, &sipMsg);
        /*Update the dialog state*/
        SIP_updateDialog(&sipMsg, &(sessp->dialogs), p);
    }
    /*Update the session data*/
    pRopts = &(sessp->ropts);
    pRopts->methodFlag = sipMsg.methodFlag;
    pRopts->header_data = sipMsg.header;
    pRopts->header_len = sipMsg.headerLen;
    pRopts->body_len = sipMsg.bodyLen;
    pRopts->body_data = sipMsg.body_data;
    pRopts->status_code = sipMsg.status_code;


    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "SIP message header length: %d\n",
            sipMsg.headerLen));
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Parsed method: %.*s, Flag: 0x%x\n",
            sipMsg.methodLen, sipMsg.method, sipMsg.methodFlag));
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Parsed status code:  %d\n",
            sipMsg.status_code));
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Parsed header address: %p.\n",
            sipMsg.header));
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Parsed body address: %p.\n",
            sipMsg.body_data));

    sip_freeMsg(&sipMsg);

    return status;
}
/* Main runtime entry point for SIP preprocessor.
 * Analyzes SIP packets for anomalies/exploits.
 *
 * PARAMETERS:
 *
 * packetp:    Pointer to current packet to process.
 * contextp:    Pointer to context block, not used.
 *
 * RETURNS:     Nothing.
 */
static void SIPmain( void* ipacketp, void* contextp )
{
    SIPData* sessp = NULL;
    uint8_t source = 0;
    uint8_t dest = 0;

    SFSnortPacket* packetp;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    PROFILE_VARS;

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__START_MSG));

    packetp = (SFSnortPacket*) ipacketp;
    sfPolicyUserPolicySet (sip_config, policy_id);

    // preconditions - what we registered for
    assert((IsUDP(packetp) || IsTCP(packetp)) &&
        packetp->payload && packetp->payload_size);

    if (IsTCP(packetp))
    {
        if (!_dpd.readyForProcess(packetp))
        {
            /* Packet will be rebuilt, so wait for it */
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Packet will be reassembled\n"));
            return;
        }
        if (_dpd.sessionAPI->get_application_data(packetp->stream_session, PP_SSL) &&
            !_dpd.streamAPI->is_session_decrypted(packetp->stream_session))
        {
            /* Packet is a non-SIP/encrypted SIP one, skip those */
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Packet is encrypted or not a SIP packet\n"));
            return;
        }
    }

    PREPROC_PROFILE_START(sipPerfStats);

    sip_eval_config = sfPolicyUserDataGetCurrent(sip_config);

    /* Attempt to get a previously allocated SIP block. */
    sessp = _dpd.sessionAPI->get_application_data(packetp->stream_session, PP_SIP);
    if (sessp != NULL)
    {
        sip_eval_config = sfPolicyUserDataGet(sessp->config, sessp->policy_id);

    }

    if (sessp == NULL)
    {
        /* If not doing autodetection, check the ports to make sure this is
         * running on an SIP port, otherwise no need to examine the traffic.
         */
#ifdef TARGET_BASED
        app_id = _dpd.sessionAPI->get_application_protocol_id(packetp->stream_session);
        if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Unknown protocol - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(sipPerfStats);
            return;
        }

        else if (app_id && (app_id != sip_app_id))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Not SIP - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(sipPerfStats);
            return;
        }

        else if (!app_id)
        {
#endif
            source = (uint8_t)CheckSIPPort( packetp->src_port );
            dest = (uint8_t)CheckSIPPort( packetp->dst_port );

            if ( !source && !dest )
            {
                /* Not one of the ports we care about. */
                DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Not SIP ports - not inspecting.\n"));
                DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
                PREPROC_PROFILE_END(sipPerfStats);
                return;
            }
#ifdef TARGET_BASED
        }
#endif
        /* Check the stream session. If it does not currently
         * have our SIP data-block attached, create one.
         */
        sessp = SIPGetNewSession(packetp, policy_id);

        if ( !sessp )
        {
            /* Could not get/create the session data for this packet. */
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Create session error - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(sipPerfStats);
            return;
        }

    }

    /* Don't process if we've missed packets */
    if (sessp->state_flags & SIP_FLG_MISSED_PACKETS)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Missed packets - not inspecting.\n"));
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
        PREPROC_PROFILE_END(sipPerfStats);
        return;
    }

    /* If we picked up mid-stream or missed any packets (midstream pick up
     * means we've already missed packets) set missed packets flag and make
     * sure we don't do any more reassembly on this session */
    if (IsTCP(packetp))
    {
        if ((_dpd.sessionAPI->get_session_flags(packetp->stream_session) & SSNFLAG_MIDSTREAM)
                || _dpd.streamAPI->missed_packets(packetp->stream_session, SSN_DIR_BOTH))
        {
            _dpd.streamAPI->set_reassembly(packetp->stream_session,
                    STREAM_FLPOLICY_IGNORE, SSN_DIR_BOTH,
                    STREAM_FLPOLICY_SET_ABSOLUTE);

            sessp->state_flags |= SIP_FLG_MISSED_PACKETS;
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Missed packets - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(sipPerfStats);
            return;
        }
    }

    /*
     * Start process PAYLOAD
     */
    SIP_Process(packetp,sessp);

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "%s\n", SIP_DEBUG__END_MSG));
    PREPROC_PROFILE_END(sipPerfStats);

}

/**********************************************************************
 *  Retrieves the SIP data block registered with the stream
 * session associated w/ the current packet. If none exists,
 * allocates it and registers it with the stream API.
 *
 * Arguments:
 *
 * packetp:	Pointer to the packet from which/in which to
 * 		retrieve/store the SIP data block.
 *
 * RETURNS:	Pointer to an SIP data block, upon success.
 *		NULL, upon failure.
 **********************************************************************/
SIPData * SIPGetNewSession(SFSnortPacket *packetp, tSfPolicyId policy_id)
{
    SIPData* datap = NULL;
    static int MaxSessionsAlerted = 0;
    /* Sanity check(s) */
    assert( packetp );
    if ( !packetp->stream_session )
    {
        return NULL;
    }
    if(numSessions > ((SIPConfig *)sfPolicyUserDataGetCurrent(sip_config))->maxNumSessions)
    {
        if (!MaxSessionsAlerted)
            ALERT(SIP_EVENT_MAX_SESSIONS,SIP_EVENT_MAX_SESSIONS_STR);
        MaxSessionsAlerted = 1;
        return NULL;
    }
    else
    {
        MaxSessionsAlerted = 0;
    }
    datap = (SIPData *)_dpd.snortAlloc(1, sizeof(SIPData), PP_SIP,
                                       PP_MEM_CATEGORY_SESSION);

    if ( !datap )
        return NULL;

    /*Register the new SIP data block in the stream session. */
    _dpd.sessionAPI->set_application_data(
            packetp->stream_session,
            PP_SIP, datap, FreeSIPData );
            
    /* We're interested in this session. Turn on stream reassembly. */
    if ( !(_dpd.streamAPI->get_reassembly_direction(packetp->stream_session) & SSN_DIR_BOTH ))
    {
        _dpd.streamAPI->set_reassembly(packetp->stream_session,
                STREAM_FLPOLICY_FOOTPRINT, SSN_DIR_BOTH, STREAM_FLPOLICY_SET_ABSOLUTE);
    }

#ifdef SNORT_RELOAD
    ada_add(ada, (void *)datap, packetp->stream_session);
#endif

    datap->policy_id = policy_id;
    datap->config = sip_config;
    ((SIPConfig *)sfPolicyUserDataGetCurrent(sip_config))->ref_count++;
    numSessions++;
    sip_stats.sessions++;
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Number of sessions created: %u\n", numSessions));

    return datap;
}


/***********************************************************************
 * Registered as a callback with our SIP data blocks when
 * they are added to the underlying stream session. Called
 * by the stream preprocessor when a session is about to be
 * destroyed.
 *
 * PARAMETERS:
 *
 * idatap:	Pointer to the moribund data.
 *
 * RETURNS:	Nothing.
 ***********************************************************************/
static void FreeSIPData( void* idatap )
{
    SIPData *ssn = (SIPData *)idatap;
    SIPConfig *config = NULL;

    if (ssn == NULL)
        return;
    if (numSessions > 0)
        numSessions--;

#ifdef SNORT_RELOAD
    ada_appdata_freed( ada, idatap );
#endif

    /*Free all the dialog data*/
    sip_freeDialogs(&ssn->dialogs);

    /*Clean the configuration data*/
    if (ssn->config != NULL)
    {
        config = (SIPConfig *)sfPolicyUserDataGet(ssn->config, ssn->policy_id);
    }

    if (config == NULL)
    {
        _dpd.snortFree(ssn, sizeof(SIPData), PP_SIP, PP_MEM_CATEGORY_SESSION);
        return;
    }

    config->ref_count--;
    if ((config->ref_count == 0) &&	(ssn->config != sip_config))
    {
        sfPolicyUserDataClear (ssn->config, ssn->policy_id);
        _dpd.snortFree(config, sizeof(SIPConfig), PP_SIP, PP_MEM_CATEGORY_CONFIG);

        if (sfPolicyUserPolicyGetActive(ssn->config) == 0)
        {
            /* No more outstanding configs - free the config array */
            SIPFreeConfig(ssn->config);
        }

    }

    _dpd.snortFree(ssn, sizeof(SIPData), PP_SIP, PP_MEM_CATEGORY_SESSION);
}
/* **********************************************************************
 * Validates given port as an SIP server port.
 *
 * PARAMETERS:
 *
 * port:	Port to validate.
 *
 * RETURNS:	SIP_TRUE, if the port is indeed an SIP server port.
 *		    SIP_FALSE, otherwise.
 ***********************************************************************/
static inline int CheckSIPPort( uint16_t port )
{
    if ( sip_eval_config->ports[ PORT_INDEX(port) ] & CONV_PORT( port ) )
    {
        return SIP_TRUE;
    }

    return SIP_FALSE;
}

static void registerPortsForDispatch( struct _SnortConfig *sc, SIPConfig *policy )
{
    if ( _dpd.isPreprocEnabled( sc, PP_APP_ID ) )
    {
        _dpd.sessionAPI->enable_preproc_all_ports( sc,
                                                   PP_SIP,
                                                   PROTO_BIT__UDP | PROTO_BIT__TCP ); 
    }
    else
    {
        int port;
        for ( port = 0; port < MAXPORTS; port++ )
        {
            if( isPortEnabled( policy->ports, port ) )
                _dpd.sessionAPI->enable_preproc_for_port( sc,
                                                          PP_SIP,
                                                          PROTO_BIT__UDP | PROTO_BIT__TCP,
                                                          port ); 
        }
    }
}

static void registerPortsForReassembly( SIPConfig *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
    }
}

static void _addPortsToStreamFilter(struct _SnortConfig *sc, SIPConfig *config, tSfPolicyId policy_id)
{
    uint32_t portNum;

    assert(config);
    assert(_dpd.streamAPI);

    for (portNum = 0; portNum < MAXPORTS; portNum++)
    {
        if(config->ports[(portNum/8)] & (1<<(portNum%8)))
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_UDP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
            _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_TCP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
            register_sip_paf_port(sc, portNum, policy_id);
        }
    }
}

#ifdef TARGET_BASED

static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, sip_app_id, PORT_MONITOR_SESSION, policy_id, 1);
    register_sip_paf_service(sc, sip_app_id, policy_id);
}
#endif

static int SIPCheckPolicyConfig(struct _SnortConfig *sc, tSfPolicyUserContextId config, tSfPolicyId policy_id, void* pData)
{
    SIPConfig *sip_policy = ( SIPConfig * ) pData;

    if ( sip_policy->disabled )
        return 0;

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("SIPCheckPolicyConfig(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    if (policy_id != 0)
    {
        SIPConfig *default_sip_policy = ( SIPConfig * ) sfPolicyUserDataGetDefault( config );
        if(default_sip_policy == NULL)
        {
            _dpd.errMsg("SIPCheckPolicyConfig(): SIP default policy must be configured\n");
            return -1;
        }

        sip_policy->maxNumSessions = default_sip_policy->maxNumSessions;
     }
    
    _dpd.setParserPolicy( sc, policy_id );
    _dpd.addPreproc( sc, SIPmain, PRIORITY_APPLICATION, PP_SIP, PROTO_BIT__UDP|PROTO_BIT__TCP );

    // register ports with session and stream
    registerPortsForDispatch( sc, sip_policy );
    registerPortsForReassembly( sip_policy, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStreamFilter(sc, sip_policy, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif

    return 0;
}

static bool SIPGlobalIsEnabled(struct _SnortConfig *sc, tSfPolicyUserContextId config)
{
   return sfPolicyUserDataIterate(sc, config, SIPPolicyIsEnabled) != 0;
}

static int SIPPolicyIsEnabled(struct _SnortConfig *sc, tSfPolicyUserContextId pContext, tSfPolicyId policyId, void* config)
{
    SIPConfig *sip_policy_config = (SIPConfig *) config;
    if (sip_policy_config == NULL || sip_policy_config->disabled)
        return 0;

    return 1;
}

int SIPCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate (sc, sip_config, SIPCheckPolicyConfig)))
        return rval;

    return 0;
}

static void SIPCleanExit(int signal, void *data)
{
    if (sip_config != NULL)
    {
        SIPFreeConfig(sip_config);
        sip_config = NULL;
#ifdef SNORT_RELOAD
        ada_delete(ada);
        ada = NULL;
#endif
    }
}
static int SIPFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
)
{
    SIPConfig *pPolicyConfig = (SIPConfig *)pData;

    //do any housekeeping before freeing SIPConfig

    sfPolicyUserDataClear (config, policyId);

    SIP_FreeConfig(pPolicyConfig);
    return 0;
}

void SIPFreeConfig(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, SIPFreeConfigPolicy);
    sfPolicyConfigDelete(config);
}

static void DisplaySIPStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int i = 0;
    int len = 0;

    if (sip_stats.sessions) {
        len += snprintf(buffer, CS_STATS_BUF_SIZE,  "SIP Preprocessor Statistics\n"
            "  Total sessions: "STDu64"\n", sip_stats.sessions);
        if (sip_stats.events)
            len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "  SIP anomalies : "STDu64"\n", sip_stats.events);

        if (sip_stats.dialogs)
            len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "  Total  dialogs: "STDu64"\n", sip_stats.dialogs);

        len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "  Requests: "STDu64"\n", sip_stats.requests[0]);
        while (NULL != StandardMethods[i].name && len < CS_STATS_BUF_SIZE) {
            len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "%16s:   "STDu64"\n",
                    StandardMethods[i].name, sip_stats.requests[StandardMethods[i].methodFlag]);
            i++;
        }
        len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "  Responses: "STDu64"\n", sip_stats.responses[TOTAL_RESPONSES]);
        for (i = 1; i <NUM_OF_RESPONSE_TYPES && len < CS_STATS_BUF_SIZE; i++) {
            len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  "             %dxx:   "STDu64"\n", i, sip_stats.responses[i]);
        }
        len += snprintf(buffer+len, CS_STATS_BUF_SIZE-len,  " Ignore sessions:   "STDu64"\n"
            " Ignore channels:   "STDu64"\n", sip_stats.ignoreSessions, sip_stats.ignoreChannels);
    } else {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "SIP Stats not available\n Total Sessions:"STDu64"\n", sip_stats.sessions );
    }

    if (-1 == f(te, (const uint8_t *)buffer, len)) {
        _dpd.logMsg("Unable to send data to the frontend\n");
    }
}

/******************************************************************
 * Print statistics being kept by the preprocessor.
 *
 * Arguments:
 *  int - whether Snort is exiting or not
 *
 * Returns: None
 *
 ******************************************************************/
static void SIP_PrintStats(int exiting)
{
    int i;
    _dpd.logMsg("SIP Preprocessor Statistics\n");
    _dpd.logMsg("  Total sessions: "STDu64"\n", sip_stats.sessions);
    if (sip_stats.sessions > 0)
    {
        if (sip_stats.events > 0)
            _dpd.logMsg("  SIP anomalies : "STDu64"\n", sip_stats.events);
        if (sip_stats.dialogs > 0)
            _dpd.logMsg("  Total  dialogs: "STDu64"\n", sip_stats.dialogs);

        _dpd.logMsg("  Requests: "STDu64"\n", sip_stats.requests[0]);
        i = 0;
        while (NULL != StandardMethods[i].name)
        {
            _dpd.logMsg("%16s:   "STDu64"\n",
                    StandardMethods[i].name, sip_stats.requests[StandardMethods[i].methodFlag]);
            i++;
        }

        _dpd.logMsg("  Responses: "STDu64"\n", sip_stats.responses[TOTAL_RESPONSES]);
        for (i = 1; i <NUM_OF_RESPONSE_TYPES; i++ )
        {
            _dpd.logMsg("             %dxx:   "STDu64"\n", i, sip_stats.responses[i]);
        }

        _dpd.logMsg(" Ignore sessions:   "STDu64"\n", sip_stats.ignoreSessions);
        _dpd.logMsg(" Ignore channels:   "STDu64"\n", sip_stats.ignoreChannels);
    }
}
#ifdef SNORT_RELOAD
static void SIPReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId sip_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    SIPConfig *pDefaultPolicyConfig = NULL;
    SIPConfig * pPolicyConfig = NULL;

    if (sip_swap_config == NULL)
    {
        //create a context
        sip_swap_config = sfPolicyConfigCreate();
        if (sip_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory for SIP config.\n");
        }
        *new_config = (void *)sip_swap_config;
    }

    sfPolicyUserPolicySet (sip_swap_config, policy_id);
    pPolicyConfig = (SIPConfig *)sfPolicyUserDataGetCurrent(sip_swap_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("SIP preprocessor can only be configured once.\n");
    }

    pPolicyConfig = (SIPConfig *)_dpd.snortAlloc(1, sizeof(SIPConfig), PP_SIP,
                                                 PP_MEM_CATEGORY_CONFIG);
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                "SIP preprocessor configuration.\n");
    }
    sfPolicyUserDataSetCurrent(sip_swap_config, pPolicyConfig);
    SIP_RegRuleOptions(sc);
    ParseSIPArgs(pPolicyConfig, (u_char *)args);

    pDefaultPolicyConfig = (SIPConfig *)sfPolicyUserDataGetDefault(sip_config);
    //we don't know the order in which policies are init
    //maybe default (policy 0) isn't init until after another policy is init
    //however a default policy is guranteed
    //avoid core
    //Also, if SIP isn't enabled, then why waste memory?
#ifdef REG_TEST
    if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
    {
        printf("SIP-reload SIPReload-before : %p\n", ada);
    }
#endif
    if (pDefaultPolicyConfig != NULL && SIPGlobalIsEnabled(sc, sip_swap_config) && ada == NULL)
    {
        ada = ada_init(SIP_NumSessions, PP_SIP, (size_t)pDefaultPolicyConfig->maxNumSessions);
        if (ada == NULL)
            DynamicPreprocessorFatalMessage("Could not allocate memory for SIP ada\n");
    }
#ifdef REG_TEST
    if (REG_TEST_FLAG_RELOAD & getRegTestFlags())
    {
        printf("SIP-reload SIPReload-after : %p\n", ada);
    }
#endif
}

static int SIPReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId sip_swap_config = (tSfPolicyUserContextId)swap_config;
    SIPConfig * default_swap_config = NULL;
    SIPConfig * current_default_config = NULL;
    int rval;

    if (sip_swap_config == NULL)
        return 0;

    // validate each policy and do per policy initialization processing
    if ((rval = sfPolicyUserDataIterate (sc, sip_swap_config, SIPCheckPolicyConfig)))
        return rval;

    default_swap_config = (SIPConfig *)sfPolicyUserDataGet(sip_swap_config, _dpd.getDefaultPolicy());

    if (sip_config != NULL)
    {
        current_default_config = (SIPConfig *)sfPolicyUserDataGet(sip_config, _dpd.getDefaultPolicy());
        //not possible
        if (!current_default_config)
            return 0;

        tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
        if (!SIPGlobalIsEnabled(sc, sip_swap_config))
        {
            ada_reload_disable(&ada, sc, "sip-disable-mem-dump", policy_id);
        }
        else if (SIPGlobalIsEnabled(sc, sip_config) && default_swap_config->maxNumSessions < current_default_config->maxNumSessions)
        {
            ada_reload_adjust_register(ada, policy_id, sc, "sip-mem-reloader",
                    (size_t) default_swap_config->maxNumSessions);
        }
    }

   return 0;
}

static int SIPFreeUnusedConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
)
{
    SIPConfig *pPolicyConfig = (SIPConfig *)pData;

    //do any housekeeping before freeing SIPConfig
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        SIP_FreeConfig(pPolicyConfig);
    }
    return 0;
}

static void * SIPReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId sip_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = sip_config;

    if (sip_swap_config == NULL)
        return NULL;
        
    sip_config = sip_swap_config;

    sfPolicyUserDataFreeIterate (old_config, SIPFreeUnusedConfigPolicy);
    if (sfPolicyUserPolicyGetActive(old_config) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_config;
    }

    return NULL;
}

static void SIPReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    SIPFreeConfig((tSfPolicyUserContextId)data);
}
#endif
