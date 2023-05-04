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
 * GTP preprocessor
 *
 * This is the main entry point for this preprocessor
 *
 * Author: Hui Cao
 * Date: 07-15-2011
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
#include "spp_gtp.h"
#include "gtp_config.h"
#include "gtp_roptions.h"
#include "gtp_parser.h"

#include <assert.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#ifdef DUMP_BUFFER
#include "gtp_buffer_dump.h"
#endif

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats gtpPerfStats;
#endif

#include "sf_types.h"

const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int BUILD_VERSION = 1;

const char *PREPROC_NAME = "SF_GTP";

#define SetupGTP DYNAMIC_PREPROC_SETUP

#ifdef TARGET_BASED
int16_t gtp_app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif

/*
 * Session state flags for GTPData::state_flags
 */

#define GTP_FLG_REASSEMBLY_SET        (0x20000)
/*
 * Function prototype(s)
 */
GTPData * GTPGetNewSession(SFSnortPacket *, tSfPolicyId);
static void GTPInit( struct _SnortConfig *, char* );
static int GTPCheckConfig(struct _SnortConfig *);
static void FreeGTPData( void* );
static inline int GTP_Process(SFSnortPacket *, GTPData*);
static void GTPmain( void*, void* );
static inline int CheckGTPPort( uint16_t );
static void GTPFreeConfig(tSfPolicyUserContextId);
static void registerPortsForDispatch( struct _SnortConfig *sc, GTPConfig *policy );
static void registerPortsForReassembly( GTPConfig *policy, int direction );
static void _addPortsToStreamFilter(struct _SnortConfig *, GTPConfig *, tSfPolicyId);
static void GTP_PrintStats(int);
#ifdef TARGET_BASED
static void _addServicesToStreamFilter(struct _SnortConfig *, tSfPolicyId);
#endif

static void GTPCleanExit(int, void *);

/********************************************************************
 * Global variables
 ********************************************************************/
uint32_t numSessions = 0;
GTP_Stats gtp_stats;
GTPConfig *gtp_eval_config;
tSfPolicyUserContextId gtp_config;

#ifdef SNORT_RELOAD
static void GTPReload(struct _SnortConfig *, char *, void **);
static int GTPReloadVerify(struct _SnortConfig *, void *);
static void * GTPReloadSwap(struct _SnortConfig *, void *);
static void GTPReloadSwapFree(void *);
#endif


/* Called at preprocessor setup time. Links preprocessor keyword
 * to corresponding preprocessor initialization function.
 *
 * PARAMETERS:	None.
 *
 * RETURNS:	Nothing.
 *
 */
void SetupGTP(void)
{
    /* Link preprocessor keyword to initialization function
     * in the preprocessor list. */
#ifndef SNORT_RELOAD
    _dpd.registerPreproc( "gtp", GTPInit );
#else
    _dpd.registerPreproc("gtp", GTPInit, GTPReload,
            GTPReloadVerify, GTPReloadSwap, GTPReloadSwapFree);
#endif
#ifdef DUMP_BUFFER
    _dpd.registerBufferTracer(getGTPBuffers, GTP_BUFFER_DUMP_FUNC);
#endif
}

#ifdef REG_TEST
static inline void PrintGTPSize(void)
{
    _dpd.logMsg("\nGTP Session Size: %lu\n", (long unsigned int)sizeof(GTPData));
}
#endif

/* Initializes the GTP preprocessor module and registers
 * it in the preprocessor list.
 *
 * PARAMETERS:
 *
 * argp:        Pointer to argument string to process for config data.
 *
 * RETURNS:     Nothing.
 */
static void GTPInit(struct _SnortConfig *sc, char *argp)
{
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    GTPConfig *pDefaultPolicyConfig = NULL;
    GTPConfig *pPolicyConfig = NULL;


    if (gtp_config == NULL)
    {
        /*create a context*/
        gtp_config = sfPolicyConfigCreate();
        if (gtp_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                    "for GTP config.\n");
        }

        _dpd.addPreprocConfCheck(sc, GTPCheckConfig);
        _dpd.registerPreprocStats(GTP_NAME, GTP_PrintStats);
        _dpd.addPreprocExit(GTPCleanExit, NULL, PRIORITY_LAST, PP_GTP);

#ifdef PERF_PROFILING
        _dpd.addPreprocProfileFunc("gtp", (void *)&gtpPerfStats, 0, _dpd.totalPerfStats, NULL);
#endif

#ifdef TARGET_BASED
        gtp_app_id = _dpd.findProtocolReference("gtp");
        if (gtp_app_id == SFTARGET_UNKNOWN_PROTOCOL)
            gtp_app_id = _dpd.addProtocolReference("gtp");

        // register with session to handle applications
        _dpd.sessionAPI->register_service_handler( PP_GTP, gtp_app_id );
#endif
    }

#ifdef REG_TEST
    PrintGTPSize();
#endif
    sfPolicyUserPolicySet (gtp_config, policy_id);
    pDefaultPolicyConfig = (GTPConfig *)sfPolicyUserDataGetDefault(gtp_config);
    pPolicyConfig = (GTPConfig *)sfPolicyUserDataGetCurrent(gtp_config);
    if ((pPolicyConfig != NULL) && (pDefaultPolicyConfig == NULL))
    {
        DynamicPreprocessorFatalMessage("GTP preprocessor can only be "
                "configured once.\n");
    }

    pPolicyConfig = (GTPConfig *)calloc(1, sizeof(GTPConfig));
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                "GTP preprocessor configuration.\n");
    }

    sfPolicyUserDataSetCurrent(gtp_config, pPolicyConfig);

    GTP_RegRuleOptions(sc);

    ParseGTPArgs(pPolicyConfig, (u_char *)argp);


    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("SetupGTP(): The Stream preprocessor must be enabled.\n");
    }

    _dpd.addPreproc( sc, GTPmain, PRIORITY_APPLICATION, PP_GTP, PROTO_BIT__UDP );

    // register ports with session and stream
    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStreamFilter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

/*********************************************************************
 * Main entry point for GTP processing.
 *
 * Arguments:
 *  SFSnortPacket * - pointer to packet structure
 *
 * Returns:
 *  int - 	GTP_SUCCESS
 *		    GTP_FAILURE
 *
 *********************************************************************/
static inline int GTP_Process(SFSnortPacket *p, GTPData* sessp)
{
    int status;
    const uint8_t* gtp_buff =  p->payload;
    static uint32_t msgId = 0;

    GTP_Roptions *pRopts;
    GTPMsg gtpMsg;

    pRopts = &(sessp->ropts);

    memset(&gtpMsg, 0, GTPMSG_ZERO_LEN);

    /* msg_id is used to associate message with information elements
     * If msg_id matches, the information element in the info_elements
     * belongs to the message
     * Using msg_id avoids initializing info_elements for every message
     * Tabled based info_elements improves information element search performance */

    /* To avoid id overlap, clean table when msgId resets*/
    if ( msgId == 0)
        gtp_cleanInfoElements();
    gtpMsg.msg_id = ++msgId;


    status = gtp_parse(&gtpMsg, gtp_buff, p->payload_size);

    /*Update the session data*/
    pRopts->gtp_type = gtpMsg.msg_type;
    pRopts->gtp_version = gtpMsg.version;
    pRopts->gtp_infoElements = gtpMsg.info_elements;
    pRopts->gtp_header = gtpMsg.gtp_header;
    pRopts->msg_id = gtpMsg.msg_id;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "GTP message version: %d\n",
            gtpMsg.version));
    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "GTP message type: %d\n",
            gtpMsg.msg_type));

    return status;
}
/* Main runtime entry point for GTP preprocessor.
 * Analyzes GTP packets for anomalies/exploits.
 *
 * PARAMETERS:
 *
 * packetp:    Pointer to current packet to process.
 * contextp:    Pointer to context block, not used.
 *
 * RETURNS:     Nothing.
 */
static void GTPmain( void* ipacketp, void* contextp )
{
    GTPData* sessp = NULL;
    uint8_t source = 0;
    uint8_t dest = 0;

    SFSnortPacket* packetp;
#ifdef TARGET_BASED
    int16_t app_id = SFTARGET_UNKNOWN_PROTOCOL;
#endif
    tSfPolicyId policy_id = _dpd.getNapRuntimePolicy();
    PROFILE_VARS;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "%s\n", GTP_DEBUG__START_MSG));

    packetp = (SFSnortPacket*) ipacketp;
    sfPolicyUserPolicySet (gtp_config, policy_id);

#ifdef DUMP_BUFFER
    dumpBufferInit();
    dumpBuffer(PAYLOAD_DUMP,packetp->payload,packetp->payload_size);
#endif


    // precoditions - what we registered for
    assert(IsUDP(packetp) && packetp->payload && packetp->payload_size);

    PREPROC_PROFILE_START(gtpPerfStats);

    gtp_eval_config = sfPolicyUserDataGetCurrent(gtp_config);

    /* Attempt to get a previously allocated GTP block. */
    sessp = _dpd.sessionAPI->get_application_data(packetp->stream_session, PP_GTP);
    if (sessp != NULL)
    {
        gtp_eval_config = sfPolicyUserDataGet(sessp->config, sessp->policy_id);

    }

    if (sessp == NULL)
    {
        /* If not doing autodetection, check the ports to make sure this is
         * running on an GTP port, otherwise no need to examine the traffic.
         */
#ifdef TARGET_BASED
        app_id = _dpd.sessionAPI->get_application_protocol_id(packetp->stream_session);
        if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Unknown protocol - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "%s\n", GTP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(gtpPerfStats);
            return;
        }

        else if (app_id && (app_id != gtp_app_id))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Not GTP - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "%s\n", GTP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(gtpPerfStats);
            return;
        }

        else if (!app_id)
        {
#endif
            source = (uint8_t)CheckGTPPort( packetp->src_port );
            dest = (uint8_t)CheckGTPPort( packetp->dst_port );

            if ( !source && !dest )
            {
                /* Not one of the ports we care about. */
                DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Not GTP ports - not inspecting.\n"));
                DEBUG_WRAP(DebugMessage(DEBUG_GTP, "%s\n", GTP_DEBUG__END_MSG));
                PREPROC_PROFILE_END(gtpPerfStats);
                return;
            }
#ifdef TARGET_BASED
        }
#endif
        /* Check the stream session. If it does not currently
         * have our GTP data-block attached, create one.
         */
        sessp = GTPGetNewSession(packetp, policy_id);

        if ( !sessp )
        {
            /* Could not get/create the session data for this packet. */
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Create session error - not inspecting.\n"));
            DEBUG_WRAP(DebugMessage(DEBUG_GTP, "%s\n", GTP_DEBUG__END_MSG));
            PREPROC_PROFILE_END(gtpPerfStats);
            return;
        }

    }


    /* We're interested in this session. Turn on stream reassembly. */
    if ( !(sessp->state_flags & GTP_FLG_REASSEMBLY_SET ))
    {
        _dpd.streamAPI->set_reassembly(packetp->stream_session,
                STREAM_FLPOLICY_FOOTPRINT, SSN_DIR_BOTH, STREAM_FLPOLICY_SET_ABSOLUTE);
        sessp->state_flags |= GTP_FLG_REASSEMBLY_SET;
    }
    /*
     * Start process PAYLOAD
     */
    GTP_Process(packetp,sessp);

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "%s\n", GTP_DEBUG__END_MSG));
    PREPROC_PROFILE_END(gtpPerfStats);

}

/**********************************************************************
 *  Retrieves the GTP data block registered with the stream
 * session associated w/ the current packet. If none exists,
 * allocates it and registers it with the stream API.
 *
 * Arguments:
 *
 * packetp:	Pointer to the packet from which/in which to
 * 		retrieve/store the GTP data block.
 *
 * RETURNS:	Pointer to an GTP data block, upon success.
 *		NULL, upon failure.
 **********************************************************************/
GTPData * GTPGetNewSession(SFSnortPacket *packetp, tSfPolicyId policy_id)
{
    GTPData* datap = NULL;

    /* Sanity check(s) */
    assert( packetp );
    if ( !packetp->stream_session )
    {
        return NULL;
    }

    datap = (GTPData *)calloc(1, sizeof(GTPData));

    if ( !datap )
        return NULL;

    /*Register the new GTP data block in the stream session. */
    _dpd.sessionAPI->set_application_data(
            packetp->stream_session,
            PP_GTP, datap, FreeGTPData );

    datap->policy_id = policy_id;
    datap->config = gtp_config;
    ((GTPConfig *)sfPolicyUserDataGetCurrent(gtp_config))->ref_count++;

    gtp_stats.sessions++;
    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Number of sessions created: %u\n", gtp_stats.sessions));

    return datap;
}


/***********************************************************************
 * Registered as a callback with our GTP data blocks when
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
static void FreeGTPData( void* idatap )
{
    GTPData *ssn = (GTPData *)idatap;
    GTPConfig *config = NULL;

    if (ssn == NULL)
        return;
    if (numSessions > 0)
        numSessions--;

    /*Clean the configuration data*/
    if (ssn->config != NULL)
    {
        config = (GTPConfig *)sfPolicyUserDataGet(ssn->config, ssn->policy_id);
    }

    if (config == NULL)
    {
        free(ssn);
        return;
    }

    config->ref_count--;
    if ((config->ref_count == 0) &&	(ssn->config != gtp_config))
    {
        sfPolicyUserDataClear (ssn->config, ssn->policy_id);
        free(config);

        if (sfPolicyUserPolicyGetActive(ssn->config) == 0)
        {
            /* No more outstanding configs - free the config array */
            GTPFreeConfig(ssn->config);
        }

    }

    free(ssn);
}
/* **********************************************************************
 * Validates given port as an GTP server port.
 *
 * PARAMETERS:
 *
 * port:	Port to validate.
 *
 * RETURNS:	GTP_TRUE, if the port is indeed an GTP server port.
 *		    GTP_FALSE, otherwise.
 ***********************************************************************/
static inline int CheckGTPPort( uint16_t port )
{
    if ( gtp_eval_config->ports[ PORT_INDEX(port) ] & CONV_PORT( port ) )
    {
        return GTP_TRUE;
    }

    return GTP_FALSE;
}

static void registerPortsForDispatch( struct _SnortConfig *sc, GTPConfig *policy )
{
    int port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.sessionAPI->enable_preproc_for_port( sc, PP_GTP, PROTO_BIT__UDP, port ); 
    }
}

static void registerPortsForReassembly( GTPConfig *policy, int direction )
{
    uint32_t port;

    for ( port = 0; port < MAXPORTS; port++ )
    {
        if( isPortEnabled( policy->ports, port ) )
            _dpd.streamAPI->register_reassembly_port( NULL, port, direction );
    }
}

/* **********************************************************************
 * Add ports in the configuration to stream5 filter.
 *
 * PARAMETERS:
 *
 * GTPConfig:   configuration to be used.
 * tSfPolicyId: policy ID
 *
 * RETURNS: None
 ***********************************************************************/

static void _addPortsToStreamFilter(struct _SnortConfig *sc, GTPConfig *config, tSfPolicyId policy_id)
{
    int portNum;

    assert(config);
    assert(_dpd.streamAPI);

    for (portNum = 0; portNum < MAXPORTS; portNum++)
    {
        if(config->ports[(portNum/8)] & (1<<(portNum%8)))
        {
            //Add port the port
            _dpd.streamAPI->set_port_filter_status(sc, IPPROTO_UDP, (uint16_t)portNum, PORT_MONITOR_SESSION, policy_id, 1);
        }
    }

}
#ifdef TARGET_BASED

static void _addServicesToStreamFilter(struct _SnortConfig *sc, tSfPolicyId policy_id)
{
    _dpd.streamAPI->set_service_filter_status(sc, gtp_app_id, PORT_MONITOR_SESSION, policy_id, 1);
}
#endif
static int GTPCheckPolicyConfig(struct _SnortConfig *sc, tSfPolicyUserContextId config, tSfPolicyId policyId, void* pData)
{
    _dpd.setParserPolicy(sc, policyId);

    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("GTPCheckPolicyConfig(): The Stream preprocessor must be enabled.\n");
        return -1;
    }
    return 0;
}
int GTPCheckConfig(struct _SnortConfig *sc)
{
    int rval;

    if ((rval = sfPolicyUserDataIterate (sc, gtp_config, GTPCheckPolicyConfig)))
        return rval;

    return 0;
}


static void GTPCleanExit(int signal, void *data)
{
    if (gtp_config != NULL)
    {
        GTPFreeConfig(gtp_config);
        gtp_config = NULL;
    }
}

static int GTPFreeConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
)
{
    GTPConfig *pPolicyConfig = (GTPConfig *)pData;

    //do any housekeeping before freeing GTPConfig

    sfPolicyUserDataClear (config, policyId);

    free(pPolicyConfig);

    return 0;
}

void GTPFreeConfig(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, GTPFreeConfigPolicy);
    sfPolicyConfigDelete(config);
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
static void GTP_PrintStats(int exiting)
{
    int i, j;
    _dpd.logMsg("GTP Preprocessor Statistics\n");
    _dpd.logMsg("  Total sessions: "STDu64"\n", gtp_stats.sessions);
    if (gtp_stats.sessions < 1)
        return;

    if (gtp_stats.events > 0)
        _dpd.logMsg("  Preprocessor events: "STDu64"\n", gtp_stats.events);

    _dpd.logMsg("  Total reserved messages: "STDu64"\n", gtp_stats.unknownTypes);
    _dpd.logMsg("  Packets with reserved information elements: "STDu64"\n", gtp_stats.unknownIEs);

    for (i = 0; i < MAX_GTP_VERSION_CODE + 1; i++ )
    {
        uint64_t total_msgs = 0;
        DEBUG_WRAP(_dpd.logMsg("  Messages of version %d:\n", i););
        for(j = 0; j < MAX_GTP_TYPE_CODE + 1; j++)
        {
            GTP_MsgType *msg = gtp_stats.msgTypeTable[i][j];
            if ( msg && msg->name)
            {
                DEBUG_WRAP(_dpd.logMsg("%39s:   "STDu64"\n", msg->name, gtp_stats.messages[i][j]););
            }
            total_msgs += gtp_stats.messages[i][j];

        }
        if (total_msgs > 0)
            _dpd.logMsg("  Total messages of version %d: %u\n", i, total_msgs);
    }
}
#ifdef SNORT_RELOAD
static void GTPReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId gtp_swap_config = (tSfPolicyUserContextId)*new_config;
    tSfPolicyId policy_id = _dpd.getParserPolicy(sc);
    GTPConfig * pPolicyConfig = NULL;

    if (gtp_swap_config == NULL)
    {
        //create a context
        gtp_swap_config = sfPolicyConfigCreate();
        if (gtp_swap_config == NULL)
        {
            DynamicPreprocessorFatalMessage("Failed to allocate memory "
                    "for GTP config.\n");
        }
        *new_config = (void *)gtp_swap_config;
    }

    sfPolicyUserPolicySet (gtp_swap_config, policy_id);
    pPolicyConfig = (GTPConfig *)sfPolicyUserDataGetCurrent(gtp_swap_config);
    if (pPolicyConfig != NULL)
    {
        DynamicPreprocessorFatalMessage("GTP preprocessor can only be "
                "configured once.\n");
    }

    pPolicyConfig = (GTPConfig *)calloc(1, sizeof(GTPConfig));
    if (!pPolicyConfig)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for "
                "GTP preprocessor configuration.\n");
    }
    sfPolicyUserDataSetCurrent(gtp_swap_config, pPolicyConfig);

    GTP_RegRuleOptions(sc);

    ParseGTPArgs(pPolicyConfig, (u_char *)args);

    if (_dpd.streamAPI == NULL)
    {
        DynamicPreprocessorFatalMessage("SetupGTP(): The Stream preprocessor must be enabled.\n");
    }

    _dpd.addPreproc( sc, GTPmain, PRIORITY_APPLICATION, PP_GTP, PROTO_BIT__UDP );

    // register ports with session and stream
    registerPortsForDispatch( sc, pPolicyConfig );
    registerPortsForReassembly( pPolicyConfig, SSN_DIR_FROM_SERVER | SSN_DIR_FROM_CLIENT );
    _addPortsToStreamFilter(sc, pPolicyConfig, policy_id);

#ifdef TARGET_BASED
    _addServicesToStreamFilter(sc, policy_id);
#endif
}

static int GTPReloadVerify(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId gtp_swap_config = (tSfPolicyUserContextId)swap_config;
    GTPConfig * pPolicyConfig = NULL;
    GTPConfig * pCurrentConfig = NULL;

    if (gtp_swap_config == NULL)
        return 0;

    pPolicyConfig = (GTPConfig *)sfPolicyUserDataGet(gtp_swap_config, _dpd.getDefaultPolicy());

    if (!pPolicyConfig)
        return 0;


    if (!_dpd.isPreprocEnabled(sc, PP_STREAM))
    {
        _dpd.errMsg("SetupGTP(): The Stream preprocessor must be enabled.\n");
        return -1;
    }

    if (gtp_config != NULL)
    {
        pCurrentConfig = (GTPConfig *)sfPolicyUserDataGet(gtp_config, _dpd.getDefaultPolicy());
    }

    if (!pCurrentConfig)
        return 0;

    return 0;
}

static int GTPFreeUnusedConfigPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
)
{
    GTPConfig *pPolicyConfig = (GTPConfig *)pData;

    //do any housekeeping before freeing GTPConfig
    if (pPolicyConfig->ref_count == 0)
    {
        sfPolicyUserDataClear (config, policyId);
        free(pPolicyConfig);
    }
    return 0;
}

static void * GTPReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId gtp_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = gtp_config;

    if (gtp_swap_config == NULL)
        return NULL;

    gtp_config = gtp_swap_config;

    sfPolicyUserDataFreeIterate (old_config, GTPFreeUnusedConfigPolicy);

    if (sfPolicyUserPolicyGetActive(old_config) == 0)
    {
        /* No more outstanding configs - free the config array */
        return (void *)old_config;
    }

    return NULL;
}

static void GTPReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    GTPFreeConfig((tSfPolicyUserContextId)data);
}
#endif
