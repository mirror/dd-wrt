/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 * @file    spp_session.c
 * @author  davis mcpherson <dmcpherson@sourcefire.com>
 * @date    22 Feb 2013
 *
 * @brief  sessions? we don't got no sessions...
 */

/*  I N C L U D E S  ************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>

#ifndef WIN32
#include <sys/time.h>       /* struct timeval */
#endif
#include <sys/types.h>      /* u_int*_t */

#include "snort.h"
#include "snort_bounds.h"
#include "util.h"
#include "snort_debug.h"
#include "mempool.h"
#include "mstring.h"
#include "memory_stats.h"
#include "detect.h"
#include "active.h"
#include "sp_flowbits.h"
#include "packet_time.h"

#include "session_expect.h"
#include "snort_session.h"
#include "session_api.h"
#include "spp_session.h"
#include "reload.h"
#include "snort_stream_tcp.h"
#include "snort_stream_udp.h"
#include "snort_stream_ip.h"
#include "snort_stream_icmp.h"

#include "reg_test.h"

#ifdef ENABLE_HA
#include "stream5_ha.h"
#endif

#ifdef PERF_PROFILING
PreprocStats sessionPerfStats;
# ifdef ENABLE_HA
extern PreprocStats sessionHAPerfStats;
# endif
#endif

extern OptTreeNode *otn_tmp;

/*  M A C R O S  **************************************************/

/*  G L O B A L S  **************************************************/
SessionStatistics session_stats;
uint32_t firstPacketTime = 0;
uint32_t session_mem_in_use = 0;

SessionConfiguration *session_configuration = NULL;
static SessionConfiguration *session_reload_configuration = NULL;
static GetHttpXffPrecedenceFunc getHttpXffPrecedenceFunc = NULL;

SessionCache *proto_session_caches[ SESSION_PROTO_MAX ];

MemPool sessionFlowMempool;

static PreprocEnableMask appHandlerDispatchMask[ INT16_MAX ];

static sfaddr_t fixed_addr = {{{{0xFF,0,0,0}}},0}; /* Used in lieu of ICMPv6/ICMP multicast/broadcast address
* to generate a matching key in between router solicitation and advertisement to associate them in one session.
* In future, we can extend this approach to other types of services (may need support in snort's decode):
*  - Multicast Listener Query (MLD) types 130-131-132 (typically multicast dest),
*  - ICMP Node Information Query types 139-140 (unicast or multicast dest),
*  - Neighbor Solicitation Message Format types 135-136 (unicast or multicast dest),
*  - Echo Request/Reply types 128-129 ping6 and 8-0 ping (unicast/multicast/broadcast dest), etc.
*/

/*  P R O T O T Y P E S  ********************************************/
void initializeSessionPreproc(struct _SnortConfig *, char *);
static void parseSessionConfiguration(SessionConfiguration *, char *);
static void exitSessionCleanly(int, void *);
static void resetSessionState(int, void *);
static void resetSessionStatistics(int, void *);
static int  verifySessionConfig(struct _SnortConfig *);
static void printSessionConfiguration(SessionConfiguration *);
static void printSessionStatistics(int);
static void sessionPacketProcessor(Packet *p, void *context);

#ifdef SNORT_RELOAD
static void reloadSessionConfiguration( struct _SnortConfig *, char *, void ** );
static int verifyReloadedSessionConfiguration( struct _SnortConfig *, void * );
static void *activateSessionConfiguration( struct _SnortConfig *sc, void *data );
static void freeSessionConfiguration( void * );
#endif

/*  S E S S I O N A P I **********************************************/
#if 0
static void PrintSessionKey(SessionKey *);
#endif

#ifdef MPLS
static void initMplsHeaders(SessionControlBlock*);
static void freeMplsHeaders(SessionControlBlock*);
#endif

static SessionCache* initSessionCache(uint32_t session_type, uint32_t protocol_scb_size, SessionCleanup clean_fcn);
static void *getSessionControlBlock(SessionCache*, Packet *, SessionKey *);
static void *checkSessionControlBlock(void *, Packet *, SessionKey *);
static void updateSessionControlBlockTime(SessionControlBlock *, Packet *);
static void populateSessionKey(Packet *p, SessionKey *key);
static int initSessionKeyFromPktHeader( sfaddr_t* srcIP, uint16_t srcPort, sfaddr_t* dstIP,
                                        uint16_t dstPort, char proto, uint16_t vlan,
                                        uint32_t mplsId, 
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
                                        uint16_t address_space_id_src,
                                        uint16_t address_space_id_dst,
#else
                                        uint16_t addressSpaceId,
#endif        
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                        uint32_t carrierId,
#endif
                                        SessionKey *key);
static void *getSessionControlBlockFromKey(SessionCache*, const SessionKey *);
static void *getSessionHandle(const SessionKey *key);
static void *createSession(SessionCache*, Packet *, const SessionKey * );
static bool isSessionVerified( void * );
static void removeSessionFromProtoOneWayList(uint32_t proto, void *scb);
static int deleteSession(SessionCache*, void *, char *reason, bool close_sync);
static int deleteSessionByKey(void *, char *reason);
static void printSessionCache(SessionCache*);
static int deleteSessionCache(uint32_t protocol);
static int purgeSessionCache(SessionCache*);
static int pruneSessionCache(SessionCache*, uint32_t thetime, void *save_me, int memcheck);
static void cleanProtocolSessionsPool( uint32_t  );
static void freeProtocolSessionsPool( uint32_t, void * );
static void *allocateProtocolSession( uint32_t );
static int getActiveSessionCount(SessionCache*);
static uint32_t getSessionPruneCount( uint32_t );
static void resetSessionPruneCount( uint32_t );
static void checkSessionTimeout( uint32_t, time_t );
static void setPacketDirectionFlag(Packet *p, void *scb);
static void freeSessionApplicationData(void *scb);

static int isProtocolTrackingEnabled( IpProto proto );
static uint32_t getPacketDirection( Packet *p );
static void disableInspection(void *scbptr, Packet *p);
static void stopInspection( void * scbptr, Packet *p, char dir, int32_t bytes, int response );
struct _ExpectNode;
static int ignoreChannel( const Packet *ctrlPkt, sfaddr_t* srcIP, uint16_t srcPort, sfaddr_t* dstIP,
        uint16_t dstPort, uint8_t protocol, uint32_t preprocId, char direction, char flags,
        struct _ExpectNode** packetExpectedNode);
static int getIgnoreDirection( void *scbptr );
static int setIgnoreDirection( void *scbptr, int );
static void resumeInspection( void *scbptr, char dir );
static void dropTraffic( Packet*, void *scbptr, char dir );
static int setApplicationData( void *scbptr, uint32_t protocol, void *data, StreamAppDataFree free_func );
static void *getApplicationData( void *scbptr, uint32_t protocol );
static StreamSessionKey * getSessionKeyFromPacket( Packet *p );
static void * getApplicationDataFromSessionKey( const StreamSessionKey *key, uint32_t protocol);
static void *getApplicationDataFromIpPort( sfaddr_t* srcIP, uint16_t srcPort, 
                                           sfaddr_t* dstIP, uint16_t dstPort,
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
                                           uint16_t address_space_id_src,
                                           uint16_t address_space_id_dst,
#else        
                                           uint16_t addressSpaceId,
#endif        
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                           uint32_t carrierId,
#endif
                                           char ip_protocol, uint16_t vlan,
                                           uint32_t mplsId, uint32_t protocol );
static void setSessionExpirationTime(Packet *p, void *scbptr, uint32_t timeout);
static int getSessionExpirationTime(Packet *p, void *scbptr);
static uint32_t setSessionFlags( void *scbptr, uint32_t flags );
static uint32_t getSessionFlags( void *scbptr );
static tSfPolicyId getSessionPolicy(void *scbptr, int policy_type);
static void setSessionPolicy(void *scbptr, int policy_type, tSfPolicyId id);
static StreamFlowData *getFlowData( Packet *p );
static void setSessionDeletionDelayed( void *scbptr, bool delay_session_deletion_flag);
static bool isSessionDeletionDelayed( void *scbptr );
static int setAppProtocolIdExpected( const Packet *ctrlPkt, sfaddr_t* srcIP, uint16_t srcPort, sfaddr_t* dstIP,
        uint16_t dstPort, uint8_t protocol, int16_t protoId, uint32_t preprocId,
        void* protoData, void (*protoDataFreeFn)(void*),
        struct _ExpectNode** packetExpectedNode);
#ifdef TARGET_BASED
static void registerApplicationHandler( uint32_t preproc_id, int16_t app_id );
static int16_t getAppProtocolId( void *scbptr );
static int16_t setAppProtocolId( void *scbptr, int16_t id );
static sfaddr_t* getSessionIpAddress( void *scbptr, uint32_t direction );
static void getSessionPorts( void *scbptr, uint16_t *client_port, uint16_t *server_port );
#endif

static uint16_t getPreprocessorStatusBit( void );
static void getMaxSessions(tSfPolicyId policyId, StreamSessionLimits* limits);
static void *getSessionHandleFromIpPort( sfaddr_t* srcIP, uint16_t srcPort,
                                         sfaddr_t* dstIP, uint16_t dstPort, 
                                         char ip_protocol, uint16_t vlan,
                                         uint32_t mplsId,
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                                         uint16_t address_space_id_src,
                                         uint16_t address_space_id_dst
#else
                                         uint16_t addressSpaceId
#endif 
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                         , uint32_t carrierId
#endif
                                        );
static const StreamSessionKey *getKeyFromSession(const void *scbptr);

#ifdef TARGET_BASED
#ifdef ACTIVE_RESPONSE
static void initActiveResponse( Packet*, void* scbptr );
#endif

static uint8_t getHopLimit ( void* scbptr, char dir, int outer );
#endif
static void deleteSessionIfClosed( Packet* );
static void disablePreprocForSession( void *scbptr, uint32_t preproc_id );
static void enablePreprocForPort( SnortConfig *sc, uint32_t preproc_id, uint32_t proto, uint16_t port );
static void enablePreprocAllPorts( SnortConfig *sc, uint32_t preproc_id, uint32_t proto );
static void enablePreprocAllPortsAllPolicies( SnortConfig *sc, uint32_t preproc_id, uint32_t proto );
static bool isPreprocEnabledForPort( uint32_t preproc_id, uint16_t port );
static void registerNapSelector( nap_selector nap_selector_func );
static void registerGetHttpXffPrecedence(GetHttpXffPrecedenceFunc fn);
static char** getHttpXffPrecedence(void* ssn, uint32_t flags, int* nFields);
static void setReputationUpdateCount (void *ssn, uint8_t count);

SessionAPI session_api_dispatch_table = {
    /* .version = */ SESSION_API_VERSION1,
    /* .init_session_cache =  */  initSessionCache,
    /* .get_session = */ getSessionControlBlock,
    /* .populate_session_key = */ populateSessionKey,
    /* .get_session_key_by_ip_port = */ initSessionKeyFromPktHeader,
    /* .get_session_by_key = */ getSessionControlBlockFromKey,
    /* .get_session_handle = */ getSessionHandle,
    /* .create_session = */ createSession,
    /* .is_session_verified = */ isSessionVerified,
    /* .remove_session_from_oneway_list = */ removeSessionFromProtoOneWayList,
    /* .delete_session = */ deleteSession,
    /* .delete_session_by_key = */ deleteSessionByKey,
    /* .print_session_cache = */ printSessionCache,
    /* .delete_session_cache = */ deleteSessionCache,
    /* .purge_session_cache = */ purgeSessionCache,
    /* .prune_session_cache = */ pruneSessionCache,
    /* .clean_protocol_session_pool = */ cleanProtocolSessionsPool,
    /* .free_protocol_session_pool = */ freeProtocolSessionsPool,
    /* .alloc_protocol_session = */ allocateProtocolSession,
    /* .get_session_count = */ getActiveSessionCount,
    /* .get_session_prune_count = */ getSessionPruneCount,
    /* .reset_session_prune_count = */ resetSessionPruneCount,
    /* .check_session_timeout = */ checkSessionTimeout,
    /* .protocol_tracking_enabled = */ isProtocolTrackingEnabled,
    /* .set_packet_direction_flag = */ setPacketDirectionFlag,
    /* .free_application_data = */ freeSessionApplicationData,
    /* .get_packet_direction = */ getPacketDirection,
    /* .disable_inspection = */ disableInspection,
    /* .stop_inspection = */ stopInspection,
    /* .ignore_session = */ ignoreChannel,
    /* .get_ignore_direction = */ getIgnoreDirection,
    /* .resume_inspection = */ resumeInspection,
    /* .drop_traffic = */ dropTraffic,
    /* .set_application_data = */ setApplicationData,
    /* .get_application_data = */ getApplicationData,
    /* .set_expire_timer = */ setSessionExpirationTime,
    /* .get_expire_timer = */ getSessionExpirationTime,
    /* .set_session_flags = */ setSessionFlags,
    /* .get_session_flags = */ getSessionFlags,
    /* .get_runtime_policy = */ getSessionPolicy,
    /* .set_runtime_policy = */ setSessionPolicy,
    /* .get_flow_data = */ getFlowData,
    /* .set_session_deletion_delayed = */ setSessionDeletionDelayed,
    /* .is_session_deletion_delayed = */ isSessionDeletionDelayed,
#ifdef TARGET_BASED
    /* .register_service_handler  */ registerApplicationHandler,
    /* .get_application_protocol_id = */ getAppProtocolId,
    /* .set_application_protocol_id = */ setAppProtocolId,
    /* .get_session_ip_address = */ getSessionIpAddress,
    /* .get_session_ports = */ getSessionPorts,
#endif
    /* .get_preprocessor_status_bit = */ getPreprocessorStatusBit,
#ifdef TARGET_BASED
#ifdef ACTIVE_RESPONSE
    /* .init_active_response = */ initActiveResponse,
#endif
    /* .get_session_ttl = */ getHopLimit,
#endif
    /* .set_application_protocol_id_expected = */ setAppProtocolIdExpected,
#ifdef ENABLE_HA
    /* .register_ha_funcs = */ RegisterSessionHAFuncs,
    /* .unregister_ha_funcs = */ UnregisterSessionHAFuncs,
    /* .set_ha_pending_bit = */ SessionSetHAPendingBit,
    /* .process_ha = */ SessionProcessHA,
#endif
    /* .get_max_session_limits = */ getMaxSessions,
    /* .set_ignore_direction = */ setIgnoreDirection,
    /* .get_session_ptr_from_ip_port = */ getSessionHandleFromIpPort,
    /* .get_key_from_session_ptr = */ getKeyFromSession,
    /* .check_session_closed = */ deleteSessionIfClosed,
    /* .get_session_key = */ getSessionKeyFromPacket,
    /* .get_application_data_from_key = */ getApplicationDataFromSessionKey,
    /* .get_application_data_from_ip_port = */ getApplicationDataFromIpPort,
    /* .disable_preproc_for_session = */ disablePreprocForSession,
    /* .enable_preproc_for_port = */ enablePreprocForPort,
    /* .enable_preproc_all_ports = */ enablePreprocAllPorts,
    /* .enable_preproc_all_ports_all_policies = */ enablePreprocAllPortsAllPolicies,
    /* .is_preproc_enabled_for_port = */  isPreprocEnabledForPort,
    /* .register_nap_selector =  */   registerNapSelector,
    /* .register_mandatory_early_session_creator = */ registerMandatoryEarlySessionCreator,
    /* .get_application_data_from_expected_node = */ getApplicationDataFromExpectedNode,
    /* .add_application_data_to_expected_node = */ addApplicationDataToExpectedNode,
    /* .register_get_http_xff_precedence = */ registerGetHttpXffPrecedence,
    /* .get_http_xff_precedence = */ getHttpXffPrecedence,
    /* .get_next_expected_node = */ getNextExpectedNode,
    /* .set_reputation_update_counter = */ setReputationUpdateCount
};


void SetupSessionManager(void)
{
#ifndef SNORT_RELOAD
    RegisterPreprocessor("stream5_global", initializeSessionPreproc);
# ifdef ENABLE_HA
    RegisterPreprocessor("stream5_ha", StreamHAInit);
# endif
#else
    RegisterPreprocessor("stream5_global",
            initializeSessionPreproc,
            reloadSessionConfiguration,
            verifyReloadedSessionConfiguration,
            activateSessionConfiguration,
            freeSessionConfiguration);

# ifdef ENABLE_HA
    RegisterPreprocessor("stream5_ha",
            SessionHAInit,
            SessionHAReload,
            SessionVerifyHAConfig,
            SessionHASwapReload,
            SessionHAConfigFree);
#endif
#endif

    // init the pointer to session api dispatch table
    session_api = &session_api_dispatch_table;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Preprocessor Session is setup\n"););
}

static void selectRuntimePolicy( Packet *p, bool client_packet )
{
    tSfPolicyId id;
    int vlanId = ( p->vh ) ? VTH_VLAN( p->vh ) : -1;

    sfaddr_t* srcIp = ( p->iph ) ? GET_SRC_IP( p ) : NULL;
    sfaddr_t* dstIp = ( p->iph ) ? GET_DST_IP( p ) : NULL;

    //set policy id for this packet
    id = sfGetApplicablePolicyId( snort_conf->policy_config, vlanId, srcIp, dstIp );
    setNapRuntimePolicy( id );
    setIpsRuntimePolicy( id );
    p->configPolicyId = snort_conf->targeted_policies[ id ]->configPolicyId;
    p->ips_os_selected = true;
}

SessionConfiguration *getSessionConfiguration( bool reload_config )
{
    if( reload_config )
        return session_reload_configuration;
    else
        return session_configuration;
}

static SessionConfiguration *initSessionConfiguration( void )
{
    SessionConfiguration *sessionConfig =
        (SessionConfiguration *)SnortPreprocAlloc(1, sizeof(SessionConfiguration),
                                      PP_STREAM, PP_MEM_CATEGORY_CONFIG );

    sessionConfig->track_tcp_sessions = STREAM_TRACK_YES;
    sessionConfig->max_tcp_sessions = STREAM_DEFAULT_MAX_TCP_SESSIONS;
    sessionConfig->tcp_cache_pruning_timeout = STREAM_DEFAULT_TCP_CACHE_PRUNING_TIMEOUT;
    sessionConfig->tcp_cache_nominal_timeout = STREAM_DEFAULT_TCP_CACHE_NOMINAL_TIMEOUT;
    sessionConfig->track_udp_sessions = STREAM_TRACK_YES;
    sessionConfig->max_udp_sessions = STREAM_DEFAULT_MAX_UDP_SESSIONS;
    sessionConfig->udp_cache_pruning_timeout = STREAM_DEFAULT_UDP_CACHE_PRUNING_TIMEOUT;
    sessionConfig->udp_cache_nominal_timeout = STREAM_DEFAULT_UDP_CACHE_NOMINAL_TIMEOUT;
    sessionConfig->track_icmp_sessions = STREAM_TRACK_NO;
    sessionConfig->max_icmp_sessions = STREAM_DEFAULT_MAX_ICMP_SESSIONS;
    sessionConfig->track_ip_sessions = STREAM_TRACK_NO;
    sessionConfig->max_ip_sessions = STREAM_DEFAULT_MAX_IP_SESSIONS;
    sessionConfig->memcap = STREAM_DEFAULT_MEMCAP;
    sessionConfig->prune_log_max = STREAM_DEFAULT_PRUNE_LOG_MAX;
#ifdef ACTIVE_RESPONSE
    sessionConfig->max_active_responses = STREAM_DEFAULT_MAX_ACTIVE_RESPONSES;
    sessionConfig->min_response_seconds = STREAM_DEFAULT_MIN_RESPONSE_SECONDS;
#endif

#ifdef ENABLE_HA
    sessionConfig->enable_ha = 0;
#endif

    return sessionConfig;
}

void initializeMaxExpectedFlows( SessionConfiguration *sessionConfig )
{
    // initialize max expected flows...
    uint32_t max = sessionConfig->max_tcp_sessions + sessionConfig->max_udp_sessions;

    if( ( max >>= 9 ) == 0 )
        max = 2;

    StreamExpectInit(max);
    LogMessage("      Max Expected Streams: %u\n", max);
}

void initializeSessionPreproc(struct _SnortConfig *sc, char *args)
{
    if (session_configuration == NULL)
    {
        session_configuration = initSessionConfiguration( );

#ifdef PERF_PROFILING
        RegisterPreprocessorProfile("session_manager", &sessionPerfStats, 0, &totalPerfStats, NULL);
# ifdef ENABLE_HA
        RegisterPreprocessorProfile("session_ha", &sessionHAPerfStats, 1, &sessionPerfStats, NULL);
# endif
#endif

        // register these to run last so other preprocs have chance to release all session
        // related resources before session exits
        AddFuncToPreprocCleanExitList(exitSessionCleanly, NULL, PRIORITY_LAST, PP_SESSION);
        AddFuncToPreprocResetList(resetSessionState, NULL, PRIORITY_LAST, PP_SESSION);
        AddFuncToPreprocResetStatsList(resetSessionStatistics, NULL, PP_SESSION_PRIORITY, PP_SESSION);
        AddFuncToConfigCheckList(sc, verifySessionConfig);
        RegisterPreprocStats("session_manager", printSessionStatistics);

        session_api = &session_api_dispatch_table;

#ifdef ENABLE_HA
        AddFuncToPostConfigList(sc, SessionHAPostConfigInit, NULL);
#endif

        parseSessionConfiguration(session_configuration, args);
        if( ( !session_configuration->disabled) &&
                (  session_configuration->track_tcp_sessions == STREAM_TRACK_NO) &&
                (  session_configuration->track_udp_sessions == STREAM_TRACK_NO) &&
                (  session_configuration->track_icmp_sessions == STREAM_TRACK_NO) &&
                (  session_configuration->track_ip_sessions == STREAM_TRACK_NO))
        {
            FatalError("%s(%d) ==> Session enabled, but not configured to track "
                    "TCP, UDP, ICMP, or IP.\n", file_name, file_line);
        }

        enablePreprocAllPorts( sc, PP_SESSION, PROTO_BIT__ALL );
        // TBD-EDM - fix reg tests when this is enabled
        // printSessionConfiguration(session_configuration);

        // register default NAP selector, nap rule engine will overide when it is configured
        registerNapSelector( &selectRuntimePolicy );
        memset( appHandlerDispatchMask, 0, sizeof( appHandlerDispatchMask ) );
        AddFuncToPreprocList(sc, sessionPacketProcessor, PP_SESSION_PRIORITY, PP_SESSION, PROTO_BIT__ALL);

        initializeMaxExpectedFlows( session_configuration );
        sc->run_flags |= RUN_FLAG__STATEFUL;
    }
    else
    {
#if 0
        // EDM-TBD
        FatalError("stream5_global must only be configured once.\n");
#else
        WarningMessage("stream5_global must only be configured once. Ignoring this configuration element\n");
#endif
    }
}

static void parseSessionConfiguration( SessionConfiguration *config, char *args )
{
    char **toks;
    int num_toks;
    int i;
    char **stoks;
    int s_toks;
    char *endPtr = NULL;

    if (config == NULL)
        return;

    if ((args == NULL) || (strlen(args) == 0))
        return;

    toks = mSplit(args, ",", 0, &num_toks, 0);
    i = 0;

    for (i = 0; i < num_toks; i++)
    {
        stoks = mSplit(toks[i], " ", 4, &s_toks, 0);

        if (s_toks == 0)
        {
            FatalError("%s(%d) => Missing parameter in Session config.\n",
                    file_name, file_line);
        }
        if (s_toks > 2) //Trailing parameters
        {
            FatalError("%s(%d) => Too many parameters for option in Session config.\n",
                    file_name, file_line);
        }

        if(!strcasecmp(stoks[0], "memcap"))
        {
            if (stoks[1])
            {
                config->memcap = strtoul(stoks[1], &endPtr, 10);
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid memcap in config file.  Requires integer parameter.\n",
                        file_name, file_line);
            }

            if ((config->memcap > STREAM_RIDICULOUS_HI_MEMCAP) ||
                    (config->memcap < STREAM_RIDICULOUS_LOW_MEMCAP))
            {
                FatalError("%s(%d) => 'memcap %s' invalid: value must be "
                        "between %d and %d bytes\n",
                        file_name, file_line,
                        stoks[1], STREAM_RIDICULOUS_LOW_MEMCAP,
                        STREAM_RIDICULOUS_HI_MEMCAP);
            }
        }
        else if(!strcasecmp(stoks[0], "max_tcp"))
        {
            if (stoks[1])
            {
                config->max_tcp_sessions = strtoul(stoks[1], &endPtr, 10);
                if (config->track_tcp_sessions == STREAM_TRACK_YES)
                {
                    if ((config->max_tcp_sessions > STREAM_RIDICULOUS_MAX_SESSIONS) ||
                            (config->max_tcp_sessions == 0))
                    {
                        FatalError("%s(%d) => 'max_tcp %d' invalid: value must be "
                                "between 1 and %d sessions\n",
                                file_name, file_line,
                                config->max_tcp_sessions,
                                STREAM_RIDICULOUS_MAX_SESSIONS);
                    }
                }
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid max_tcp in config file.  Requires integer parameter.\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "tcp_cache_pruning_timeout"))
        {
            if (stoks[1])
            {
                unsigned long timeout = strtoul(stoks[1], &endPtr, 10);

                if (config->track_tcp_sessions == STREAM_TRACK_YES)
                {
                    if ( !timeout || (timeout > STREAM_MAX_CACHE_TIMEOUT) )
                    {
                        FatalError(
                                "%s(%d) => '%s %lu' invalid: value must be between 1 and %d seconds\n",
                                file_name, file_line, stoks[0], timeout, STREAM_MAX_CACHE_TIMEOUT);
                    }
                }
                config->tcp_cache_pruning_timeout = (uint16_t)timeout;
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid %s in config file.  Requires integer parameter.\n",
                        file_name, file_line, stoks[0]);
            }
        }
        else if(!strcasecmp(stoks[0], "tcp_cache_nominal_timeout"))
        {
            if (stoks[1])
            {
                unsigned long timeout = strtoul(stoks[1], &endPtr, 10);

                if (config->track_tcp_sessions == STREAM_TRACK_YES)
                {
                    if ( !timeout || (timeout > STREAM_MAX_CACHE_TIMEOUT) )
                    {
                        FatalError(
                                "%s(%d) => '%s %lu' invalid: value must be between 1 and %d seconds\n",
                                file_name, file_line, stoks[0], timeout, STREAM_MAX_CACHE_TIMEOUT);
                    }
                }
                config->tcp_cache_nominal_timeout = (uint16_t)timeout;
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid %s in config file.  Requires integer parameter.\n",
                        file_name, file_line, stoks[0]);
            }
        }
        else if(!strcasecmp(stoks[0], "track_tcp"))
        {
            if (stoks[1])
            {
                if(!strcasecmp(stoks[1], "no"))
                    config->track_tcp_sessions = STREAM_TRACK_NO;
                else if(!strcasecmp(stoks[1], "yes"))
                    config->track_tcp_sessions = STREAM_TRACK_YES;
                else
                    FatalError("%s(%d) => invalid: value must be 'yes' or 'no'\n",
                        file_name, file_line);
            }
            else
            {
                FatalError("%s(%d) => 'track_tcp' missing option\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "max_udp"))
        {
            if (stoks[1])
            {
                config->max_udp_sessions = strtoul(stoks[1], &endPtr, 10);
                if (config->track_udp_sessions == STREAM_TRACK_YES)
                {
                    if ((config->max_udp_sessions > STREAM_RIDICULOUS_MAX_SESSIONS) ||
                            (config->max_udp_sessions == 0))
                    {
                        FatalError("%s(%d) => 'max_udp %d' invalid: value must be "
                                "between 1 and %d sessions\n",
                                file_name, file_line,
                                config->max_udp_sessions,
                                STREAM_RIDICULOUS_MAX_SESSIONS);
                    }
                }
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid max_udp in config file.  Requires integer parameter.\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "udp_cache_pruning_timeout"))
        {
            if (stoks[1])
            {
                unsigned long timeout = strtoul(stoks[1], &endPtr, 10);

                if (config->track_udp_sessions == STREAM_TRACK_YES)
                {
                    if ( !timeout || (timeout > STREAM_MAX_CACHE_TIMEOUT) )
                    {
                        FatalError(
                                "%s(%d) => '%s %lu' invalid: value must be between 1 and %d seconds\n",
                                file_name, file_line, stoks[0], timeout, STREAM_MAX_CACHE_TIMEOUT);
                    }
                }
                config->udp_cache_pruning_timeout = (uint16_t)timeout;
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid %s in config file.  Requires integer parameter.\n",
                        file_name, file_line, stoks[0]);
            }
        }
        else if(!strcasecmp(stoks[0], "udp_cache_nominal_timeout"))
        {
            if (stoks[1])
            {
                unsigned long timeout = strtoul(stoks[1], &endPtr, 10);

                if (config->track_udp_sessions == STREAM_TRACK_YES)
                {
                    if ( !timeout || (timeout > STREAM_MAX_CACHE_TIMEOUT) )
                    {
                        FatalError(
                                "%s(%d) => '%s %lu' invalid: value must be between 1 and %d seconds\n",
                                file_name, file_line, stoks[0], timeout, STREAM_MAX_CACHE_TIMEOUT);
                    }
                }
                config->udp_cache_nominal_timeout = (uint16_t)timeout;
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid %s in config file.  Requires integer parameter.\n",
                        file_name, file_line, stoks[0]);
            }
        }
        else if(!strcasecmp(stoks[0], "track_udp"))
        {
            if (stoks[1])
            {
                if(!strcasecmp(stoks[1], "no"))
                    config->track_udp_sessions = STREAM_TRACK_NO;
                else if(!strcasecmp(stoks[1], "yes"))
                    config->track_udp_sessions = STREAM_TRACK_YES;
                else
                    FatalError("%s(%d) => invalid: value must be 'yes' or 'no'\n",
                        file_name, file_line);
            }
            else
            {
                FatalError("%s(%d) => 'track_udp' missing option\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "max_icmp"))
        {
            if (stoks[1])
            {
                config->max_icmp_sessions = strtoul(stoks[1], &endPtr, 10);

                if (config->track_icmp_sessions == STREAM_TRACK_YES)
                {
                    if ((config->max_icmp_sessions > STREAM_RIDICULOUS_MAX_SESSIONS) ||
                            (config->max_icmp_sessions == 0))
                    {
                        FatalError("%s(%d) => 'max_icmp %d' invalid: value must be "
                                "between 1 and %d sessions\n",
                                file_name, file_line,
                                config->max_icmp_sessions,
                                STREAM_RIDICULOUS_MAX_SESSIONS);
                    }
                }
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid max_icmp in config file.  Requires integer parameter.\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "track_icmp"))
        {
            if (stoks[1])
            {
                if(!strcasecmp(stoks[1], "no"))
                    config->track_icmp_sessions = STREAM_TRACK_NO;
                else if(!strcasecmp(stoks[1], "yes"))
                    config->track_icmp_sessions = STREAM_TRACK_YES;
                else
                    FatalError("%s(%d) => invalid: value must be 'yes' or 'no'\n",
                        file_name, file_line);
            }
            else
            {
                FatalError("%s(%d) => 'track_icmp' missing option\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "max_ip"))
        {
            if (stoks[1])
            {
                config->max_ip_sessions = strtoul(stoks[1], &endPtr, 10);

                if (config->track_ip_sessions == STREAM_TRACK_YES)
                {
                    if ((config->max_ip_sessions > STREAM_RIDICULOUS_MAX_SESSIONS) ||
                            (config->max_ip_sessions == 0))
                    {
                        FatalError("%s(%d) => 'max_ip %d' invalid: value must be "
                                "between 1 and %d sessions\n",
                                file_name, file_line,
                                config->max_ip_sessions,
                                STREAM_RIDICULOUS_MAX_SESSIONS);
                    }
                }
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid max_ip in config file.  Requires integer parameter.\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "track_ip"))
        {
            if (stoks[1])
            {
                if(!strcasecmp(stoks[1], "no"))
                    config->track_ip_sessions = STREAM_TRACK_NO;
                else if(!strcasecmp(stoks[1], "yes"))
                    config->track_ip_sessions = STREAM_TRACK_YES;
                else
                    FatalError("%s(%d) => invalid: value must be 'yes' or 'no'\n",
                        file_name, file_line);
            }
            else
            {
                FatalError("%s(%d) => 'track_ip' missing option\n",
                        file_name, file_line);
            }
        }
        else if(!strcasecmp(stoks[0], "flush_on_alert"))
        {
            if (s_toks > 1) //Trailing parameters
            {
                FatalError("%s(%d) => Too many parameters for option in Session config.\n",
                        file_name, file_line);
            }
            config->flags |= STREAM_CONFIG_FLUSH_ON_ALERT;
        }
        else if(!strcasecmp(stoks[0], "show_rebuilt_packets"))
        {
            if (s_toks > 1) //Trailing parameters
            {
                FatalError("%s(%d) => Too many parameters for option in Session config.\n",
                        file_name, file_line);
            }
            config->flags |= STREAM_CONFIG_SHOW_PACKETS;
        }
        else if(!strcasecmp(stoks[0], "prune_log_max"))
        {
            if (stoks[1])
            {
                config->prune_log_max = strtoul(stoks[1], &endPtr, 10);
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid prune_log_max in config file.  Requires integer parameter.\n",
                        file_name, file_line);
            }

            if (((config->prune_log_max > STREAM_MAX_PRUNE_LOG_MAX) ||
                        (config->prune_log_max < STREAM_MIN_PRUNE_LOG_MAX)) &&
                    (config->prune_log_max != 0))
            {
                FatalError("%s(%d) => Invalid Prune Log Max."
                        "  Must be 0 (disabled) or between %d and %d\n",
                        file_name, file_line,
                        STREAM_MIN_PRUNE_LOG_MAX, STREAM_MAX_PRUNE_LOG_MAX);
            }
        }
#ifdef TBD
        else if(!strcasecmp(stoks[0], "no_midstream_drop_alerts"))
        {
            /*
             * FIXTHIS: Do we want to not alert on drops for sessions picked
             * up midstream ?  If we're inline, and get a session midstream,
             * its because it was picked up during startup.  In inline
             * mode, we should ALWAYS be requiring TCP 3WHS.
             */
            if (s_toks > 1) //Trailing parameters
            {
                FatalError("%s(%d) => Too many parameters for option in Session config.\n",
                        file_name, file_line);
            }
            config->flags |= STREAM_CONFIG_MIDSTREAM_DROP_NOALERT;
        }
#endif
#ifdef ACTIVE_RESPONSE
        else if(!strcasecmp(stoks[0], "max_active_responses"))
        {
            if (stoks[1])
            {
                config->max_active_responses = (uint8_t)SnortStrtoulRange(stoks[1], &endPtr, 10, 0, STREAM_MAX_ACTIVE_RESPONSES_MAX);
            }
            if ((!stoks[1] || (endPtr == &stoks[1][0])) || *endPtr ||
                    (config->max_active_responses > STREAM_MAX_ACTIVE_RESPONSES_MAX))
            {
                FatalError("%s(%d) => 'max_active_responses %d' invalid: "
                        "value must be between 0 and %d responses.\n",
                        file_name, file_line, config->max_active_responses,
                        STREAM_MAX_ACTIVE_RESPONSES_MAX);
            }
            if ( config->max_active_responses > 0 )
            {
                Active_SetEnabled(2);
            }
        }
        else if(!strcasecmp(stoks[0], "min_response_seconds"))
        {
            if (stoks[1])
            {
                config->min_response_seconds = strtoul(stoks[1], &endPtr, 10);
            }
            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid min_response_seconds in config file. "
                        " Requires integer parameter.\n", file_name, file_line);
            }
            else if ( ( config->min_response_seconds > STREAM_MIN_RESPONSE_SECONDS_MAX ) ||
                    ( config->min_response_seconds < 1 ) )
            {
                FatalError("%s(%d) => 'min_response_seconds %d' invalid: "
                        "value must be between 1 and %d seconds.\n",
                        file_name, file_line, config->min_response_seconds,
                        STREAM_MIN_RESPONSE_SECONDS_MAX);
            }
        }
#endif
#ifdef ENABLE_HA
        else if (!strcasecmp(stoks[0], "enable_ha"))
        {
            if (s_toks > 1) //Trailing parameters
            {
                FatalError("%s(%d) => Too many parameters for option in Session config.\n",
                        file_name, file_line);
            }
            config->enable_ha = 1;
        }
#endif /* ENABLE_HA */
        else if(!strcasecmp(stoks[0], "disabled"))
        {
            if (s_toks > 1) //Trailing parameters
            {
                FatalError("%s(%d) => Too many parameters for option in Session config.\n",
                        file_name, file_line);
            }
            config->disabled = 1;
#if 0
            // TBD-EDM - breaks reg tests, enable after merge and fix tests
            WarningMessage("%s(%d) Session disable option is set...supported option but nothing useful can come of this.\n",
                    file_name, file_line);
#endif
        }
        else
        {
            FatalError("%s(%d) => Unknown Session global option (%s)\n",
                    file_name, file_line, toks[i]);
        }

        mSplitFree(&stoks, s_toks);
    }

    mSplitFree(&toks, num_toks);
}

static void printSessionConfiguration(SessionConfiguration *config)
{
// TBD-EDM
#if 0
    if (config == NULL)
        return;

    LogMessage("Session global config:\n");
    if (config->track_tcp_sessions == STREAM_TRACK_YES)
        LogMessage("    Max TCP sessions: %u\n", config->max_tcp_sessions);
    if (config->track_udp_sessions == STREAM_TRACK_YES)
        LogMessage("    Max UDP sessions: %u\n", config->max_udp_sessions);
    if (config->track_icmp_sessions == STREAM_TRACK_YES)
        LogMessage("    Max ICMP sessions: %u\n", config->max_icmp_sessions);
    if (config->track_ip_sessions == STREAM_TRACK_YES)
        LogMessage("    Max IP sessions: %u\n", config->max_ip_sessions);
    if (config->prune_log_max)
    {
        LogMessage("    Log info if session memory consumption exceeds %d\n",
                config->prune_log_max);
    }
#ifdef ACTIVE_RESPONSE
    LogMessage("    Send up to %d active responses\n", config->max_active_responses);

    if (config->max_active_responses > 1)
    {
        LogMessage("    Wait at least %d seconds between responses\n", config->min_response_seconds);
    }
#endif
#ifdef ENABLE_HA
    LogMessage("    High Availability: %s\n", config->enable_ha ? "ENABLED" : "DISABLED");
#endif
#endif
}

static void resetSessionState( int signal, void *foo )
{
    if ( session_configuration == NULL )
        return;

    mempool_clean( &sessionFlowMempool );
}

static void resetSessionStatistics( int signal, void *foo )
{
    memset( &session_stats, 0, sizeof( session_stats ) );
#ifdef ENABLE_HA
    SessionResetHAStats();
#endif
}

static void exitSessionCleanly( int signal, void *foo )
{
#ifdef ENABLE_HA
    SessionCleanHA();
#endif

    /* free expected session data structures... */
    StreamExpectCleanup();

    mempool_destroy( &sessionFlowMempool );
    freeSessionPlugins( );
    SessionFreeConfig( session_configuration );
    session_configuration = NULL;
}

static int verifySessionConfig( struct _SnortConfig *sc )
{
    int obj_size = 0;
    unsigned total_sessions = 0;

    if( session_configuration == NULL )
    {
        FatalError("%s(%d) Session config is NULL.\n", __FILE__, __LINE__);
    }

#ifdef ENABLE_HA
    if( session_configuration->enable_ha )
    {
        if( ( SessionVerifyHAConfig( sc, session_configuration->ha_config ) != 0 ) )
        {
            FatalError( "Session HA misconfigured.  Session preprocessor exiting\n" );
        }
    }
#endif

    if(session_configuration->track_tcp_sessions)
       total_sessions += session_configuration->max_tcp_sessions;
    else
    {
        session_configuration->memcap = 0; 
        session_configuration->max_tcp_sessions = 0;
    }
    if(session_configuration->track_udp_sessions)
       total_sessions += session_configuration->max_udp_sessions;
    else
        session_configuration->max_udp_sessions = 0;
    if(session_configuration->track_icmp_sessions)
       total_sessions += session_configuration->max_icmp_sessions;
    else
        session_configuration->max_icmp_sessions = 0;
    if(session_configuration->track_ip_sessions)
       total_sessions += session_configuration->max_ip_sessions;
    else
        session_configuration->max_ip_sessions = 0;

    if( total_sessions == 0 && !session_configuration->disabled)
    {
        FatalError( "%s(%d) All protocols configured with 0 as max session count.\n", __FILE__, __LINE__ );
    }

    /* Initialize the memory pool for Flowbits Data */
    /* use giFlowbitSize - 1, since there is already 1 byte in the
     * StreamFlowData structure */
    obj_size = sizeof( StreamFlowData ) + getFlowbitSizeInBytes( ) - 1;

    if( ( obj_size % sizeof( long ) ) != 0 )
    {
        /* Increase obj_size by sizeof(long) to force sizeof(long) byte
         * alignment for each object in the mempool.  Without this,
         * the mempool data buffer was not aligned. Overlaying the
         * StreamFlowData structure caused problems on some Solaris
         * platforms. */
        obj_size += ( sizeof( long ) - ( obj_size % sizeof( long ) ) );
    }

    if (total_sessions != 0)
    {
        if( mempool_init( &sessionFlowMempool, total_sessions, obj_size ) != 0 )
        {
            FatalError( "%s(%d) Could not initialize flow bits memory pool.\n", __FILE__, __LINE__ );
        }
    }

    session_configuration->max_sessions = total_sessions;
    session_configuration->numSnortPolicies = sc->num_policies_allocated;
    session_configuration->policy_ref_count = SnortPreprocAlloc( sc->num_policies_allocated,
                                                sizeof( uint32_t ), PP_STREAM, 
                                                PP_MEM_CATEGORY_CONFIG );
    if( !session_configuration->policy_ref_count )
    {
        FatalError( "%s(%d) Could not allocate policy ref count.\n", __FILE__, __LINE__ );
    }

    return 0;
}

static void printSessionStatistics( int exiting )
{
#if 0
    LogMessage("Session statistics:\n");
    LogMessage("            Total sessions: %u\n", session_stats.total_tcp_sessions +
            session_stats.total_udp_sessions +
            session_stats.total_icmp_sessions +
            session_stats.total_ip_sessions);
    LogMessage("              TCP sessions: %u\n", session_stats.total_tcp_sessions);
    LogMessage("              UDP sessions: %u\n", session_stats.total_udp_sessions);
    LogMessage("             ICMP sessions: %u\n", session_stats.total_icmp_sessions);
    LogMessage("               IP sessions: %u\n", session_stats.total_ip_sessions);

    LogMessage("                    Events: %u\n", session_stats.events);
    LogMessage("           Internal Events: %u\n", session_stats.internalEvents);
    LogMessage("           TCP Port Filter\n");
    LogMessage("                   Dropped: %u\n", session_stats.tcp_port_filter.filtered);
    LogMessage("                 Inspected: %u\n", session_stats.tcp_port_filter.inspected);
    LogMessage("                   Tracked: %u\n", session_stats.tcp_port_filter.session_tracked);
    LogMessage("           UDP Port Filter\n");
    LogMessage("                   Dropped: %u\n", session_stats.udp_port_filter.filtered);
    LogMessage("                 Inspected: %u\n", session_stats.udp_port_filter.inspected);
    LogMessage("                   Tracked: %u\n", session_stats.udp_port_filter.session_tracked);
#ifdef ENABLE_HA
    SessionPrintHAStats();
#endif
#endif
}

static void initPreprocDispatchList(  Packet *p )
{
    PreprocEvalFuncNode *ppn;
    SnortPolicy *policy = snort_conf->targeted_policies[ getNapRuntimePolicy() ];

    // set initial post session preproc enabled for the policy selected for
    // this session
    ppn = policy->preproc_eval_funcs;
    while( ppn != NULL && ppn->priority <= PP_SESSION_PRIORITY )
        ppn = ppn->next;

    p->cur_pp = ppn;
}

static inline int isPacketEligible( Packet *p )
{
    if( ( p->frag_flag ) || ( p->error_flags & PKT_ERR_CKSUM_IP ) )
        return 0;

    if( p->packet_flags & PKT_REBUILT_STREAM )
        return 0;

    if( !IPH_IS_VALID( p ) )
        return 0;

    switch( GET_IPH_PROTO( p ) )
    {
        case IPPROTO_TCP:
            {
                if( p->tcph == NULL )
                    return 0;

                if( p->error_flags & PKT_ERR_CKSUM_TCP )
                    return 0;
            }
            break;

        case IPPROTO_UDP:
            {
                if( p->udph == NULL )
                    return 0;

                if( p->error_flags & PKT_ERR_CKSUM_UDP )
                    return 0;
            }
            break;

        case IPPROTO_ICMP:
        case IPPROTO_ICMPV6:
            {
                if( p->icmph == NULL )
                    return 0;

                if( p->error_flags & PKT_ERR_CKSUM_ICMP )
                    return 0;
            }
            break;

        default:
            if( p->iph == NULL )
                return 0;
            break;
    }

    return 1;
}

void insertIntoOneWaySessionList( SessionCache *session_cache, SessionControlBlock *scb )
{

    if( session_cache->ows_list.head == NULL )
    {
        // list is empty link head & tail pointers to this scb
        session_cache->ows_list.head = scb;
        session_cache->ows_list.tail = scb;
        scb->ows_next = NULL;
        scb->ows_prev = NULL;
    }
    else
    {
        // list is not empty, add this node to the end
        session_cache->ows_list.tail->ows_next = scb;
        scb->ows_prev = session_cache->ows_list.tail;
        scb->ows_next = NULL;
        session_cache->ows_list.tail = scb;
    }

    // set scb state to in oneway list and increment count of sessions
    // in the oneway list
    scb->in_oneway_list = true;
    session_cache->ows_list.num_sessions++;
}

void removeFromOneWaySessionList( SessionCache *session_cache, SessionControlBlock *scb )
{
    if( scb->ows_prev == NULL )
    {
        // removing first node
        if( scb->ows_next == NULL )
        {
            // only node in list
            session_cache->ows_list.head = NULL;
            session_cache->ows_list.tail = NULL;
        }
        else
        {
            session_cache->ows_list.head = scb->ows_next;
            scb->ows_next->ows_prev = NULL;
        }
    }
    else if ( scb->ows_next == NULL )
    {
        // removing last node ( list with one node detected above so must be >1 nodes here )
        session_cache->ows_list.tail = scb->ows_prev;
        scb->ows_prev->ows_next = NULL;
    }
    else
    {
        // removing node from middle of list
        scb->ows_prev->ows_next = scb->ows_next;
        scb->ows_next->ows_prev = scb->ows_prev;
    }

    scb->ows_next = NULL;
    scb->ows_prev = NULL;

    // set scb state to not in oneway list and decrement count of sessions
    // in the oneway list
    scb->in_oneway_list = false;
    session_cache->ows_list.num_sessions--;
}

static void decrementPolicySessionRefCount( SessionControlBlock *scb )
{

#ifdef ENABLE_HA
    // if in standby no policy bound to this session
    if( scb->ha_flags & HA_FLAG_STANDBY )
        return;
#endif

    if( scb->napPolicyId < scb->session_config->numSnortPolicies )
    {
        scb->session_config->policy_ref_count[ scb->napPolicyId ]--;

#ifdef SNORT_RELOAD
        if( scb->session_config != session_configuration ||
                scb->session_config->no_ref_cb )
#else
        if( scb->session_config != session_configuration )
#endif
        {
            uint32_t i;
            bool no_refs = true;

            for( i = 0; i < scb->session_config->numSnortPolicies; i++ )
                if( scb->session_config->policy_ref_count[ i ] > 0 )
                {
                    no_refs = false;
                   break;
                }

           if( no_refs )
           {
#ifdef SNORT_RELOAD
               if( scb->session_config->no_ref_cb )
               {
                    scb->session_config->no_ref_cb(scb->session_config->no_ref_cb_data);
                    scb->session_config->no_ref_cb = NULL;
                    scb->session_config->no_ref_cb_data = NULL;
               }
#endif
               if( scb->session_config != session_configuration )
                   SessionFreeConfig( scb->session_config );
           }
        }
    }
    else
    {
        WarningMessage("%s(%d) NAP Policy ID is UNBOUND or not valid: %u", file_name, file_line, scb->napPolicyId );
    }
}

void initializePacketPolicy( Packet *p, SessionControlBlock *scb )
{
    SnortPolicy *policy;

    // opensource policy selector will set to true, product does not
    p->ips_os_selected = false;

    if( scb != NULL )
    {
        // if config pointer is same as online we are good to go...
        if( scb->session_config != session_configuration )
        {
            // session config has been reloaded, reevaluate packet against NAP rules
            decrementPolicySessionRefCount( scb );
            scb->initial_pp = NULL;
            scb->stream_config_stale = true;
            scb->napPolicyId = SF_POLICY_UNBOUND;
            scb->ipsPolicyId = SF_POLICY_UNBOUND;
            scb->session_config = session_configuration;
        }

        /* Select the policy for this session...Then initialize list of
         * additional preprocs to be dispatched for this session
         */
        if ( scb->napPolicyId == SF_POLICY_UNBOUND )
        {
            if( isNapRuntimePolicyDefault())
                getSessionPlugins()->select_session_nap( p,
                        ( getPacketDirection( p ) & PKT_FROM_CLIENT ) ? true : false );
            scb->napPolicyId = getNapRuntimePolicy();
            session_configuration->policy_ref_count[ scb->napPolicyId ]++;
            scb->ipsPolicyId = getIpsRuntimePolicy();

            // set the preproc enable mask for pps registered to handle traffic on this port
            policy = snort_conf->targeted_policies[ scb->napPolicyId ];
            scb->enabled_pps = policy->pp_enabled[ ntohs(scb->server_port) ];
            if( scb->port_guess )
                scb->enabled_pps |= policy->pp_enabled[ ntohs(scb->client_port) ];
            scb->ips_os_selected = p->ips_os_selected;
        }
        else
        {
            // set runtime policy from session setting, session already has preproc list
            setNapRuntimePolicy( scb->napPolicyId );
            setIpsRuntimePolicy( scb->ipsPolicyId );
            p->ips_os_selected = scb->ips_os_selected;
        }

        initPreprocDispatchList( p );
        EnablePreprocessors( p, scb->enabled_pps );

        // save current ha state for ha processing after packet processing has completed
        scb->cached_ha_state = scb->ha_state;
    }
    else
    {
        // no session, select policy from packet network parameters
        getSessionPlugins()->select_session_nap( p,
                                                 ( getPacketDirection( p ) & PKT_FROM_CLIENT ) ? true : false );
        initPreprocDispatchList( p );
        policy = snort_conf->targeted_policies[ getNapRuntimePolicy() ];
        EnablePreprocessors( p, policy->pp_enabled[ p->dp ] | policy->pp_enabled[ p->sp ] );
    }
}

static inline SessionControlBlock *findPacketSessionControlBlock(SessionCache *sessionCache, Packet *p, SessionKey *key)
{
    SessionControlBlock *scb = NULL;
#if defined(DAQ_CAPA_CST_TIMEOUT)
        uint64_t timeout;
#endif

    if (!sessionCache)
        return NULL;
    scb = checkSessionControlBlock(sessionCache, p, key);
    if (scb && getSessionExpirationTime(p, scb))
    {
        // We retrieved a scb which has already expired but not deleted
        // Delete the previous scb and recover  it from HA data
        deleteSessionByKey (scb, "expired and not cleaned") ;
        scb = p->ssnptr = NULL;
    }
#if defined(DAQ_CAPA_CST_TIMEOUT)
    else if (scb && (p->pkth->flags & DAQ_PKT_FLAG_NEW_FLOW))
    {
        //We retrieved a scb but this is a new flow, which means this is stale scb, Delete it
        deleteSessionByKey (scb, "stale and not cleaned") ;
        scb = p->ssnptr = NULL;
    }
#endif
    else
    {
        // Update the time of last data seen in the SCB
        updateSessionControlBlockTime(scb, p);
    }

#if defined(ENABLE_HA) && defined(HAVE_DAQ_QUERYFLOW)
    if ((!scb || !scb->appDataList) && session_configuration->enable_ha && session_configuration->ha_config->use_daq &&
#ifndef REG_TEST
            (p->pkth->flags & DAQ_PKT_FLAG_HA_STATE_AVAIL) &&
#endif
             SessionHAQueryDAQState(p->pkth) == 0)
    {

        scb = getSessionControlBlock(sessionCache, p, key);
#if defined(DAQ_CAPA_CST_TIMEOUT)
        if (scb && Daq_Capa_Timeout)
        {
          GetTimeout(p,&timeout);
          sessionCache->timeoutNominal = timeout;
          setSessionExpirationTime(p,scb,timeout);
    }
#endif
    }
#endif

#ifdef HAVE_DAQ_PKT_TRACE
    if (pkt_trace_enabled && (p->pkth->flags & DAQ_PKT_FLAG_SIMULATED) && scb)
    {
        addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
            "Session: simulated packet matches existing snort session\n"));
    }
#endif

    return scb;
}

// Return true if a preprocessor is enabled for a session
static inline bool isPreprocEnabledForSession( SessionControlBlock *scb, uint32_t preproc_id )
{
	return  ( scb && scb->enabled_pps & (UINT64_C(1) << preproc_id) ) ? true : false;
}

// Turn ON a preprocessor for a session
static inline void enablePreprocForSession( SessionControlBlock *scb, uint32_t preproc_id)
{
    if( scb != NULL )
        scb->enabled_pps |= ( UINT64_C(1) << preproc_id );
}

#if defined (HAVE_DAQ_QUERYFLOW) && defined (DAQ_QUERYFLOW_TYPE_IS_CONN_META_VALID) 
int SessionConnMetaQuery(const DAQ_PktHdr_t *pkthdr)
{
    DAQ_QueryFlow_t query;
    int rval;

    query.type = DAQ_QUERYFLOW_TYPE_IS_CONN_META_VALID;
    query.length = 0;
    query.value = 0;

    rval = DAQ_QueryFlow(pkthdr, &query);
    if (rval == DAQ_SUCCESS ||
        rval == DAQ_ERROR_NOTSUP)
        return 1;
    else
        return 0;
}
#endif

static void sessionPacketProcessor(Packet *p, void *context)
{
    SessionControlBlock *scb = NULL;
    SessionKey key;
    uint32_t flags;
#if defined(DAQ_CAPA_CST_TIMEOUT)
    uint64_t timeout;
#endif

    PROFILE_VARS;

    if (!firstPacketTime)
        firstPacketTime = p->pkth->ts.tv_sec;

    if( !isPacketEligible( p ) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, "Is not eligible!\n"););
        initializePacketPolicy( p, p->ssnptr );
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, "++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, "In Session!\n"););

    PREPROC_PROFILE_START(sessionPerfStats);

    if( p->ssnptr == NULL )
    {
        switch( GET_IPH_PROTO( p ) )
        {
            case IPPROTO_TCP:
                scb = findPacketSessionControlBlock( proto_session_caches[ SESSION_PROTO_TCP ], p, &key );
#if defined(DAQ_CAPA_CST_TIMEOUT)
                /* we will do syn re-evaluation when the we recive SYN for stale scssions
                 * we will delete session when we recive SYN on session 
                 * which is going to be exired in this time STREAM_DELAY_TIMEOUT_AFTER_CONNECTION_ENDED*/
                if ( scb != NULL  && 
                  Daq_Capa_Timeout &&  (p->tcph  && p->tcph->th_flags & (TH_SYN)) && 
                  (scb->expire_time - (((uint64_t)p->pkth->ts.tv_sec ) * TCP_HZ) <= STREAM_DELAY_TIMEOUT_AFTER_CONNECTION_ENDED))
                {
                  scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;
                  deleteSession(proto_session_caches[ SESSION_PROTO_TCP ], scb, "stale/timeout",true);
                  scb = NULL;
                }
#endif
                if ( ( scb == NULL ) && SessionTrackingEnabled( session_configuration, SESSION_PROTO_TCP ) )
                {
#if defined(HAVE_DAQ_QUERYFLOW) && defined (DAQ_QUERYFLOW_TYPE_IS_CONN_META_VALID)
                    /* It is observed for non-IP packets, conn meta check fails. In case of conn meta check failure,
                     * check if non_ip_pkt is set to allow flow creation. 
                     */
                    if (SessionConnMetaQuery(p->pkth) || p->non_ip_pkt) {
                        scb = createSession( proto_session_caches[ SESSION_PROTO_TCP ], p, &key );
                    } else {
                        if (pkt_trace_enabled)
                            addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
                                        "Conn meta is not available. So not creating TCP session\n"));
                        DisablePacketAnalysis( p );
                        PREPROC_PROFILE_END(sessionPerfStats);
                        return;
                    }
#else
                    scb = createSession( proto_session_caches[ SESSION_PROTO_TCP ], p, &key );
#endif
#if defined(DAQ_CAPA_CST_TIMEOUT)
                    if (Daq_Capa_Timeout && scb != NULL)
                    {

                      GetTimeout(p,&timeout);
                      timeout = timeout + STREAM_DELAY_TIMEOUT_AFTER_CONNECTION_ENDED;
                      proto_session_caches[ SESSION_PROTO_TCP ]->timeoutNominal = timeout;
                      setSessionExpirationTime(p,scb,timeout);
                    }
#endif 
                }

                if( ( scb != NULL ) && !scb->session_established && ( getSessionPlugins()->set_tcp_dir_ports != NULL ) )
                    getSessionPlugins()->set_tcp_dir_ports( p, scb );

                break;

            case IPPROTO_UDP:
                scb = findPacketSessionControlBlock( proto_session_caches[ SESSION_PROTO_UDP ], p, &key );
				if( ( scb == NULL ) &&  SessionTrackingEnabled( session_configuration, SESSION_PROTO_UDP ) )
				{
#if defined(HAVE_DAQ_QUERYFLOW) && defined (DAQ_QUERYFLOW_TYPE_IS_CONN_META_VALID)
                    /* It is observed for non-IP packets, conn meta check fails. In case of conn meta check failure,
                     * check if non_ip_pkt is set to allow flow creation. 
                     */
                    if (SessionConnMetaQuery(p->pkth) || p->non_ip_pkt) {
                        scb = createSession( proto_session_caches[ SESSION_PROTO_UDP ], p, &key );
                    } else {
                        if (pkt_trace_enabled)
                            addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
                                        "Conn meta is not available. So not creating UDP session\n"));
                        DisablePacketAnalysis( p );
                        PREPROC_PROFILE_END(sessionPerfStats);
                        return;
                    }
#else
                    scb = createSession( proto_session_caches[ SESSION_PROTO_UDP ], p, &key );
#endif
#if defined(DAQ_CAPA_CST_TIMEOUT)
                    if (Daq_Capa_Timeout && scb != NULL)
                    {
                        GetTimeout(p,&timeout);
                        proto_session_caches[ SESSION_PROTO_UDP ]->timeoutNominal = timeout;
                        setSessionExpirationTime(p,scb,timeout);
                }
#endif
                }

                if( scb && !scb->session_established && ( getSessionPlugins()->set_udp_dir_ports != NULL ) )
                    getSessionPlugins()->set_udp_dir_ports( p, scb );

                break;

            case IPPROTO_ICMP:
                // new flow allocate an scb, if not tracking ICMP, then fall thru and treat packet as
                // an IP protocol packet
                if ( SessionTrackingEnabled( session_configuration, SESSION_PROTO_ICMP ) )
                {
                    scb = findPacketSessionControlBlock( proto_session_caches[ SESSION_PROTO_ICMP ], p, &key );
                    if( scb != NULL )
                    break;
    
                    scb = createSession( proto_session_caches[ SESSION_PROTO_ICMP ], p, &key );
#if defined(DAQ_CAPA_CST_TIMEOUT)
                    if (Daq_Capa_Timeout && scb != NULL)
                    {

                      GetTimeout(p,&timeout);
                      proto_session_caches[ SESSION_PROTO_ICMP ]->timeoutNominal = timeout;
                      setSessionExpirationTime(p,scb,timeout);
                    }
#endif
                                                        
                    break;
                }
                // fall thru, not tracking ICMP, treat as IP packet...

            case IPPROTO_IP:
            default:
                scb = findPacketSessionControlBlock( proto_session_caches[ SESSION_PROTO_IP ], p, &key );
                if( ( scb == NULL ) && SessionTrackingEnabled( session_configuration, SESSION_PROTO_IP ) )
                {
                    scb = createSession( proto_session_caches[ SESSION_PROTO_IP ], p, &key );
#if defined(DAQ_CAPA_CST_TIMEOUT)
                    if (Daq_Capa_Timeout && scb != NULL)
                    {

                      GetTimeout(p,&timeout);
                      proto_session_caches[ SESSION_PROTO_IP ]->timeoutNominal = timeout;
                      setSessionExpirationTime(p,scb,timeout);
                    }
#endif
                }
                break;
                
        }
        // assign allocated SCB to the Packet structure
        p->ssnptr = scb;
    }
    else
    {
        scb = p->ssnptr;
    }

    initializePacketPolicy( p, scb );

    flags = getSessionFlags( scb );

    if( ( scb && (getIPRepUpdateCount() != scb->iprep_update_counter ) )
        && ( !(isPreprocEnabledForSession(scb,PP_REPUTATION)) && !(flags & (SSNFLAG_DETECTION_DISABLED|SSNFLAG_FORCE_BLOCK)) ) )
    {
	// If preproc was disabled or flow was whitelisted earlier, re-enable reputation
        EnablePreprocessor(p, PP_REPUTATION);
        enablePreprocForSession(scb, PP_REPUTATION);
        scb->ha_state.session_flags &= ~SSNFLAG_DETECTION_DISABLED;
        scb->ha_state.session_flags &= ~SSNFLAG_FORCE_BLOCK;
    }
    else if( flags & SSNFLAG_FORCE_BLOCK )
    {
        DisablePacketAnalysis( p );
        /* Detect will turn on the perfmonitor preprocessor when this function returns */
        scb->enabled_pps = PP_PERFMONITOR;
        Active_ForceDropSession();
        if (pkt_trace_enabled)
            addPktTraceData(VERDICT_REASON_SESSION, snprintf(trace_line, MAX_TRACE_LINE,
                "Session: blocked session flag is true, %s\n", getPktTraceActMsg()));
        else addPktTraceData(VERDICT_REASON_SESSION, 0);
    }
    else if( flags & SSNFLAG_DETECTION_DISABLED )
    {
        DisablePacketAnalysis( p );
        /* Detect will turn on the perfmonitor preprocessor when this function returns */
        scb->enabled_pps = PP_PERFMONITOR;
    }

    PREPROC_PROFILE_END(sessionPerfStats);
}

/*************************** API Implementations *******************/
#define SESSION_CACHE_FLAG_PURGING  0x01
#define SESSION_CACHE_FLAG_PRUNING  0x02

#if 0

void PrintSessionKey(SessionKey *skey)
{
    LogMessage("SessionKey:\n");
    LogMessage("      ip_l     = 0x%08X\n", skey->ip_l);
    LogMessage("      ip_h     = 0x%08X\n", skey->ip_h);
    LogMessage("      prt_l    = %d\n", skey->port_l);
    LogMessage("      prt_h    = %d\n", skey->port_h);
    LogMessage("      vlan_tag = %d\n", skey->vlan_tag);
#ifdef MPLS
    LogMessage("    mpls label = 0x%08X\n", skey->mplsLabel);
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    LogMessage(" addr space id = %d\n", skey->addressSpaceId);
#endif
}
#endif

static int getActiveSessionCount(SessionCache* session_cache)
{
    if (session_cache &&session_cache->hashTable)
        return session_cache->hashTable->count;
    else
        return 0;
}

static uint32_t getSessionPruneCount( uint32_t protocol )
{

    if( protocol < SESSION_PROTO_MAX )
    {
        if( proto_session_caches[ protocol ] )
            return proto_session_caches[ protocol ]->prunes;
        else
            return 0;
    }
    else
    {
        WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, protocol);
        return 0;
    }
}

static void resetSessionPruneCount( uint32_t protocol )
{
    if( protocol < SESSION_PROTO_MAX )
    {
        if( proto_session_caches[protocol] )
            proto_session_caches[protocol]->prunes = 0;
    }
    else
    {
        WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, protocol);
    }
}

static int initSessionKeyFromPktHeader( sfaddr_t* srcIP,
        uint16_t srcPort,
        sfaddr_t* dstIP,
        uint16_t dstPort,
        char proto,
        uint16_t vlan,
        uint32_t mplsId,
#if !defined(DAQ_CAPA_VRF) || defined(SFLINUX)
        uint16_t addressSpaceId,
#else
        uint16_t address_space_id_src,
        uint16_t address_space_id_dst,
#endif  
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)    
        uint32_t carrierId, 
#endif 
        SessionKey *key )
    
{
    uint16_t sport;
    uint16_t dport;
    sfaddr_t *src;
    sfaddr_t *dst;
    /* Because the key is going to be used for hash lookups,
     * the lower of the values of the IP address field is
     * stored in the key->ip_l and the port for that ip is
     * stored in key->port_l.
     */

    if (!key)
        return 0;


    switch (proto)
    {
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            sport = srcPort;
            dport = dstPort;
            break;
        case IPPROTO_ICMP:
            if (srcPort == ICMP_ECHOREPLY)
            {
                dport = ICMP_ECHO; /* Treat ICMP echo reply the same as request */
                sport = 0;
            }
            else if (srcPort == ICMP_ROUTER_ADVERTISE)
            {
                dport = ICMP_ROUTER_SOLICIT; /* Treat ICMP router advertisement the same as solicitation */
                sport = 0;
                srcIP = &fixed_addr; /* Matching src address to solicit dest address */
            }
            else /* otherwise, every ICMP type gets different key */
            {
                sport = srcPort;
                dport = 0;
                if (srcPort == ICMP_ROUTER_SOLICIT)
                    dstIP = &fixed_addr; /* To get unique key, don't use multicast/broadcast addr (RFC 1256) */
            }
            break;
        case IPPROTO_ICMPV6:
            if (srcPort == ICMP6_REPLY)
            {
                dport = ICMP6_ECHO; /* Treat ICMPv6 echo reply the same as request */
                sport = 0;
            }
            else if (srcPort == ICMP6_ADVERTISEMENT)
            {
                dport = ICMP6_SOLICITATION; /* Treat ICMPv6 router advertisement the same as solicitation */
                sport = 0;
                srcIP = &fixed_addr; /* Matching src address to solicit dest address */
            }
            else /* otherwise, every ICMP type gets different key */
            {
                sport = srcPort;
                dport = 0;
                if (srcPort == ICMP6_SOLICITATION)
                    dstIP = &fixed_addr; /* To get unique key, don't use multicast addr (RFC 4861) */
            }
            break;
        default:
            sport = dport = 0;
            break;
    }

    src = srcIP;
    dst = dstIP;

    if (sfip_fast_lt6(src, dst))
    {
        COPY4(key->ip_l, sfaddr_get_ip6_ptr(src));
        key->port_l = sport;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
        key->addressSpaceId_l = address_space_id_src;
        key->addressSpaceId_h = address_space_id_dst;
#endif        
        COPY4(key->ip_h, sfaddr_get_ip6_ptr(dst));
        key->port_h = dport;
    }
    else if (sfip_fast_eq6(src, dst))
    {
        COPY4(key->ip_l, sfaddr_get_ip6_ptr(src));
        COPY4(key->ip_h, sfaddr_get_ip6_ptr(dst));
        if (sport < dport)
        {
            key->port_l = sport;
            key->port_h = dport;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
            key->addressSpaceId_l = address_space_id_src;
            key->addressSpaceId_h = address_space_id_dst;
#endif            
        }
        else
        {
            key->port_l = dport;
            key->port_h = sport;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
            key->addressSpaceId_l = address_space_id_dst;
            key->addressSpaceId_h = address_space_id_src;
#endif            
        }
    }
    else
    {
        COPY4(key->ip_l, sfaddr_get_ip6_ptr(dst));
        key->port_l = dport;
        COPY4(key->ip_h, sfaddr_get_ip6_ptr(src));
        key->port_h = sport;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
        key->addressSpaceId_l = address_space_id_dst;
        key->addressSpaceId_h = address_space_id_src;
#endif            
    }
# ifdef MPLS
    if (ScMplsOverlappingIp())
    {
        key->mplsLabel = mplsId;
    }
    else
    {
        key->mplsLabel = 0;
    }
# else
    key->mplsLabel = 0;
# endif

    key->protocol = proto;

    if (!ScVlanAgnostic())
        key->vlan_tag = vlan;
    else
        key->vlan_tag = 0;

    key->pad = 0;
#if !defined(DAQ_CAPA_VRF) || defined(SFLINUX)
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    if (!ScAddressSpaceAgnostic())
        key->addressSpaceId = addressSpaceId;
    else
        key->addressSpaceId = 0;
#else
    key->addressSpaceId = 0;
#endif
    key->addressSpaceIdPad1 = 0;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        key->carrierId = carrierId;
#endif
    return 1;
}

static int getSessionKey(Packet *p, SessionKey *key)
{
    char proto = GET_IPH_PROTO(p);
    uint32_t mplsId = 0;
    uint16_t vlanId = 0;
    uint16_t sport = p->sp;
# ifdef MPLS
    if (ScMplsOverlappingIp() && (p->mpls != NULL))
    {
        mplsId = p->mplsHdr.label;
    }
#endif

    if (p->vh && !ScVlanAgnostic())
        vlanId = (uint16_t)VTH_VLAN(p->vh);
    if ((proto == IPPROTO_ICMP) || (proto == IPPROTO_ICMPV6))
    {
        /* ICMP */
        sport = p->icmph->type;
    }
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
   uint32_t cid = GET_OUTER_IPH_PROTOID(p, pkth);

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t sAsId = DAQ_GetSourceAddressSpaceID(p->pkth);
    uint16_t dAsId = DAQ_GetDestinationAddressSpaceID(p->pkth);
    
    return initSessionKeyFromPktHeader(GET_SRC_IP(p), sport, GET_DST_IP(p), p->dp,
                                       proto, vlanId, mplsId, sAsId, dAsId, cid, key);
#else    
    uint16_t addressSpaceId = 0;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    addressSpaceId = DAQ_GetAddressSpaceID(p->pkth);
#endif
    return initSessionKeyFromPktHeader(GET_SRC_IP(p), sport, GET_DST_IP(p), p->dp,
                                       proto, vlanId, mplsId, addressSpaceId, cid, key);
#endif    
#else /* No CarrierId support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t sAsId = DAQ_GetSourceAddressSpaceID(p->pkth);
    uint16_t dAsId = DAQ_GetDestinationAddressSpaceID(p->pkth);

    return initSessionKeyFromPktHeader(GET_SRC_IP(p), sport, GET_DST_IP(p), p->dp,
                                       proto, vlanId, mplsId, sAsId, dAsId, key);
#else
    uint16_t addressSpaceId = 0;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    addressSpaceId = DAQ_GetAddressSpaceID(p->pkth);
#endif
    return initSessionKeyFromPktHeader(GET_SRC_IP(p), sport, GET_DST_IP(p), p->dp,
                                       proto, vlanId, mplsId, addressSpaceId, key);
#endif
#endif
}

static inline void determinePacketDirection( Packet *p, uint16_t p_port, uint16_t scb_port, int is_sport )
{
    if ( is_sport )
        p->packet_flags |= ( p_port == scb_port ) ? PKT_FROM_CLIENT : PKT_FROM_SERVER;
    else
        p->packet_flags |= ( p_port == scb_port ) ? PKT_FROM_SERVER : PKT_FROM_CLIENT;
}

static void setPacketDirectionFlag(Packet *p, void *session)
{
    SessionControlBlock *scb = ( SessionControlBlock  *) session;

    if(IS_IP4(p))
    {
        if (sfip_fast_eq4(&p->ip4h->ip_addrs->ip_src, &scb->client_ip))
        {
            if (GET_IPH_PROTO(p) == IPPROTO_TCP)
                determinePacketDirection(p, p->tcph->th_sport, scb->client_port, true);
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
                determinePacketDirection(p, p->udph->uh_sport, scb->client_port, true);
            else
                p->packet_flags |= PKT_FROM_CLIENT;
        }
        else if (sfip_fast_eq4(&p->ip4h->ip_addrs->ip_dst, &scb->client_ip))
        {
            if (GET_IPH_PROTO(p) == IPPROTO_TCP)
                determinePacketDirection(p, p->tcph->th_dport, scb->client_port, false);
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
                determinePacketDirection(p, p->udph->uh_dport, scb->client_port, false);
            else
                p->packet_flags |= PKT_FROM_SERVER;
        }
    }
    else /* IS_IP6(p) */
    {
        if (sfip_fast_eq6(&p->ip6h->ip_addrs->ip_src, &scb->client_ip))
        {
            if (GET_IPH_PROTO(p) == IPPROTO_TCP)
                determinePacketDirection(p, p->tcph->th_sport, scb->client_port, true);
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
                determinePacketDirection(p, p->udph->uh_sport, scb->client_port, true);
            else
                p->packet_flags |= PKT_FROM_CLIENT;
        }
        else if (sfip_fast_eq6(&p->ip6h->ip_addrs->ip_dst, &scb->client_ip))
        {
            if (GET_IPH_PROTO(p) == IPPROTO_TCP)
                determinePacketDirection(p, p->tcph->th_dport, scb->client_port, false);
            else if (GET_IPH_PROTO(p) == IPPROTO_UDP)
                determinePacketDirection(p, p->udph->uh_dport, scb->client_port, false);
            else
                p->packet_flags |= PKT_FROM_SERVER;
        }
    }
}

static void *getSessionControlBlock( SessionCache* sessionCache, Packet *p, SessionKey *key )
{
    SessionControlBlock *scb = NULL;
    scb = ( SessionControlBlock *) checkSessionControlBlock ( sessionCache, p, key );
    updateSessionControlBlockTime ( scb, p );
    return scb;
}

static void *checkSessionControlBlock( void *sessionCache, Packet *p, SessionKey *key )
{
    // Retrieve the SCB without updating the last_data_seen time on the SCB
    SessionControlBlock *scb = NULL;

    if( getSessionKey( p, key ) )
    {
        scb = getSessionControlBlockFromKey( sessionCache, key );
    }
    return scb;
}

static void updateSessionControlBlockTime(SessionControlBlock *scb, Packet *p)
{
    if( scb != NULL )
    {
       if( scb->last_data_seen < p->pkth->ts.tv_sec )
          scb->last_data_seen = p->pkth->ts.tv_sec;
    }
}

static void populateSessionKey( Packet *p, SessionKey *key )
{

    if (!key || !p)
        return;
     
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid = GET_OUTER_IPH_PROTOID(p, pkth);

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
    uint16_t sAsId = DAQ_GetSourceAddressSpaceID(p->pkth);
    uint16_t dAsId = DAQ_GetDestinationAddressSpaceID(p->pkth);

    initSessionKeyFromPktHeader( GET_SRC_IP( p ), p->sp, GET_DST_IP( p ), p->dp,
                                 GET_IPH_PROTO( p ), p->vh ? VTH_VLAN( p->vh ) : 0,
                                 p->mplsHdr.label, sAsId, dAsId, cid, key);
#else
    uint16_t addressSpaceId = 0;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    addressSpaceId = DAQ_GetAddressSpaceID( p->pkth );
#endif

    initSessionKeyFromPktHeader( GET_SRC_IP( p ), p->sp, GET_DST_IP( p ), p->dp,
                                 GET_IPH_PROTO( p ), p->vh ? VTH_VLAN( p->vh ) : 0,
                                 p->mplsHdr.label, addressSpaceId, cid, key);
#endif  
#else /* No Carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t sAsId = DAQ_GetSourceAddressSpaceID(p->pkth);
    uint16_t dAsId = DAQ_GetDestinationAddressSpaceID(p->pkth);

    initSessionKeyFromPktHeader( GET_SRC_IP( p ), p->sp, GET_DST_IP( p ), p->dp,
                                 GET_IPH_PROTO( p ), p->vh ? VTH_VLAN( p->vh ) : 0,
                                 p->mplsHdr.label, sAsId, dAsId, key);
#else
    uint16_t addressSpaceId = 0;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    addressSpaceId = DAQ_GetAddressSpaceID( p->pkth );
#endif

    initSessionKeyFromPktHeader( GET_SRC_IP( p ), p->sp, GET_DST_IP( p ), p->dp,
                                 GET_IPH_PROTO( p ), p->vh ? VTH_VLAN( p->vh ) : 0,
                                 p->mplsHdr.label, addressSpaceId, key);
#endif   
#endif  
}

static void *getSessionControlBlockFromKey( SessionCache* session_cache, const SessionKey *key )
{
    SessionControlBlock *scb = NULL;
    SFXHASH_NODE *hnode;

    if( !session_cache )
        return NULL;

    hnode = sfxhash_find_node( session_cache->hashTable, key );

    if( hnode && hnode->data )
    {
        /* This is a unique hnode, since the sfxhash finds the
         * same key before returning this node.
         */
        scb = ( SessionControlBlock * ) hnode->data;
    }

    return scb;
}

static void freeSessionApplicationData(void *session)
{
    SessionControlBlock *scb = ( SessionControlBlock  *) session;

    StreamAppData *tmpData, *appData = scb->appDataList;
    while( appData )
    {
        if( appData->freeFunc && appData->dataPointer )
            appData->freeFunc( appData->dataPointer );

        tmpData = appData->next;
        SnortPreprocFree(appData, sizeof(StreamAppData), PP_STREAM,
                    PP_MEM_CATEGORY_SESSION);
        appData = tmpData;
        scb->appDataList = appData;
    }
}

static int removeSession(SessionCache *session_cache, SessionControlBlock *scb )
{
    SFXHASH_NODE *hnode;

    decrementPolicySessionRefCount( scb );

    mempool_free(&sessionFlowMempool, scb->flowdata);
    scb->flowdata = NULL;

    hnode = sfxhash_find_node(session_cache->hashTable, scb->key);
    if (!hnode)
        return SFXHASH_ERR;
    if (session_cache->nextTimeoutEvalNode == hnode)
        session_cache->nextTimeoutEvalNode = NULL;

    return sfxhash_free_node(session_cache->hashTable, hnode);
}

static int deleteSessionByKey(void *session, char *delete_reason)
{
    SessionCache *session_cache;
    SessionControlBlock *scb = ( SessionControlBlock  *) session;

    assert( ( NULL != scb ) && ( NULL != scb->key ) );

    switch(scb->key->protocol)
    {
        case IPPROTO_TCP:
            session_cache = proto_session_caches[SESSION_PROTO_TCP];
            break;
        case IPPROTO_UDP:
            session_cache = proto_session_caches[SESSION_PROTO_UDP];
            break;
        case IPPROTO_ICMP:
            session_cache = proto_session_caches[SESSION_PROTO_ICMP];
            if (session_cache) break;
        default:
            session_cache = proto_session_caches[SESSION_PROTO_IP];
            break;
    }

    return deleteSession(session_cache, session, delete_reason, false);
}

static int deleteSession(SessionCache* session_cache, void *session, char *delete_reason, bool close_sync)
{
    sfaddr_t client_ip;
    sfaddr_t server_ip;
    uint16_t client_port;
    uint16_t server_port;
    uint16_t lw_session_state;
    uint16_t lw_session_flags;
    int16_t app_proto_id;
    uint32_t prune_log_max;
    uint32_t old_mem_in_use;
    int ret;
    bool saved_pkt_trace = false;

    SessionControlBlock *scb = ( SessionControlBlock  *) session;

    assert( ( NULL != session_cache ) && ( NULL != scb ) );

    if(!close_sync && pkt_trace_enabled)
    {
        SavePktTrace(); 
        saved_pkt_trace = true;
    }

    if(!close_sync)
        pkt_trace_enabled = pktTracerDebugCheckSsn((void *)scb); 

    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
                    "Session: deleting snort session, reason: %s\n",delete_reason ?delete_reason:"unknown"));

    /* Save the current mem in use before pruning */
    old_mem_in_use = session_mem_in_use;

    /* And save some info on that session */
    client_port = ntohs(scb->client_port);
    server_port = ntohs(scb->server_port);
    lw_session_state = scb->session_state;
    lw_session_flags = scb->ha_state.session_flags;
#ifdef TARGET_BASED
    app_proto_id = scb->ha_state.application_protocol;
#endif

    sfip_set_ip(&client_ip, &scb->client_ip);
    sfip_set_ip(&server_ip, &scb->server_ip);

#ifdef MPLS
    if( scb->clientMplsHeader != NULL && scb->serverMplsHeader != NULL )
        freeMplsHeaders(scb);
#endif

#ifdef ENABLE_HA
    if ( !(session_cache->flags & SESSION_CACHE_FLAG_PURGING) )
        SessionHANotifyDeletion(scb);
#endif

    /*
     * Call callback to cleanup the protocol (TCP/UDP/ICMP)
     * specific session details
     */
    if ( session_cache->cleanup_fcn )
        session_cache->cleanup_fcn(scb);

    freeSessionApplicationData(scb);

    /* Need to save this off since the global config might be from an
     * older session - because of a reload - and that config might
     * get freed after removing the session */
    prune_log_max = GetSessionPruneLogMax( );

    // if sessions is in the one-way list then remove it...
    if( scb->in_oneway_list )
        removeFromOneWaySessionList( session_cache, scb );

    ret = removeSession(session_cache, scb);

    /* If we're pruning and we clobbered some large amount, log a
     * message about that session. */
    if ( prune_log_max
            && ((old_mem_in_use - session_mem_in_use ) > prune_log_max) )
    {
      char *client_ip_str, *server_ip_str;
      client_ip_str = SnortStrdup(inet_ntoa(&client_ip));
      server_ip_str = SnortStrdup(inet_ntoa(&server_ip));
      LogMessage("S5: Pruned session from cache that was "
          "using %d bytes (%s). %s %d --> %s %d "
#ifdef TARGET_BASED
          "(%d) "
#endif
          ": LWstate 0x%x LWFlags 0x%x\n",
          old_mem_in_use - session_mem_in_use,
          delete_reason?delete_reason:"Unknown",
          client_ip_str, client_port,
          server_ip_str, server_port,
#ifdef TARGET_BASED
          app_proto_id,
#endif
          lw_session_state, lw_session_flags);
	  free(client_ip_str);
	  free(server_ip_str);
    }
    
    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
                    "Session: deleted snort session using %d bytes; "
#ifdef TARGET_BASED
                    "protocol id:(%d) "
#endif
                    ": LWstate 0x%x LWFlags 0x%x\n",
                    old_mem_in_use - session_mem_in_use,
#ifdef TARGET_BASED
                    app_proto_id,
#endif
                    lw_session_state, lw_session_flags));

    if(saved_pkt_trace)
    {
        pkt_trace_enabled = true;
        RestorePktTrace();
    } 
    else if(!close_sync)
        pkt_trace_enabled = false;

    return ret;
}

static int purgeSessionCache(SessionCache* session_cache)
{
    int retCount = 0;
    SessionControlBlock *idx;
    SFXHASH_NODE *hnode;

    if (!session_cache)
        return 0;

    session_cache->flags |= SESSION_CACHE_FLAG_PURGING;

    /* Remove all sessions from the hash table. */
    hnode = sfxhash_mru_node(session_cache->hashTable);
    while (hnode)
    {
        idx = (SessionControlBlock *)hnode->data;
        if (!idx)
        {
            sfxhash_free_node(session_cache->hashTable, hnode);
        }
        else
        {
            idx->ha_state.session_flags |= SSNFLAG_PRUNED;
            deleteSession(session_cache, idx, "purge whole cache", false);
        }
        hnode = sfxhash_mru_node(session_cache->hashTable);
        retCount++;
    }

    session_cache->flags &= ~SESSION_CACHE_FLAG_PURGING;

    return retCount;
}

static int deleteSessionCache( uint32_t protocol )
{
    int retCount = 0;
    SessionCache *session_cache = NULL;

    if( protocol < SESSION_PROTO_MAX )
        session_cache = proto_session_caches[ protocol ];

    if( session_cache )
    {
        retCount = purgeSessionCache(session_cache);

        // release memory allocated for protocol specific session data
        mempool_destroy( session_cache->protocol_session_pool );
        free( session_cache->protocol_session_pool );

        sfxhash_delete( session_cache->hashTable );
        free( session_cache );
        proto_session_caches[ protocol ] = NULL;
    }

    return retCount;
}

static inline bool isSessionBlocked (SessionControlBlock* session)
{
    SessionControlBlock *scb = ( SessionControlBlock  *) session;

    return ( scb->ha_state.session_flags & ( SSNFLAG_DROP_CLIENT | SSNFLAG_DROP_SERVER ) ) != 0;
}

static int pruneOneWaySessions( SessionCache *session_cache )
{
    unsigned num_pruned = 0;

    while( session_cache->ows_list.num_sessions > session_cache->ows_list.prune_threshold )
    {
        SessionControlBlock *scb = session_cache->ows_list.head;
        if( scb != NULL )
        {
            removeFromOneWaySessionList( session_cache, scb );
            deleteSession(session_cache, scb, "oneway", false);
            if( ++num_pruned > session_cache->ows_list.prune_max )
                break;
        }
        else
        {
            WarningMessage("%s(%d) One Way Session Count Non-zero but list  head is NULL\n",
                    file_name, file_line );
            break;
        }
    }

    session_cache->prunes += num_pruned;
    return num_pruned;
}

static bool prune_more_sessions( SessionCache *session_cache, uint32_t num_pruned,
        uint32_t prune_stop_threshold, int memCheck )
{
    unsigned int session_count = sfxhash_count(session_cache->hashTable);

    if( session_count < 1 )
        return false;

    if( !memCheck )
        return ( ( session_count > prune_stop_threshold ) || ( num_pruned == 0 ) );
    else
        return session_mem_in_use > GetSessionMemCap();
}

static void moveHashNodeToFront( SessionCache *session_cache )
{
    SFXHASH_NODE *lastNode;

    lastNode = sfxhash_lru_node(session_cache->hashTable);
    if(lastNode)
    	sfxhash_gmovetofront(session_cache->hashTable, lastNode);
}
static ThrottleInfo error_throttleInfo = {0,60,0};

static int pruneSessionCache( SessionCache* session_cache, uint32_t thetime, void *save_me_session, int memCheck )
{
    SessionControlBlock *save_me = ( SessionControlBlock  * ) save_me_session;
    SessionControlBlock *scb;
    uint32_t pruned = 0;

    Active_Suspend();

    if( thetime != 0 )
    {
        /* Pruning, look for sessions that have time'd out */
        bool got_one;
        scb = ( SessionControlBlock * ) sfxhash_lru( session_cache->hashTable );

        if( scb == NULL )
        {
            Active_Resume();
            return 0;
        }

        do
        {
            got_one = false;
            if( scb == save_me )
            {
                SFXHASH_NODE *lastNode = sfxhash_lru_node( session_cache->hashTable );
                sfxhash_gmovetofront( session_cache->hashTable, lastNode );
                lastNode = sfxhash_lru_node( session_cache->hashTable );
                if( ( lastNode ) && ( lastNode->data != scb ) )
                {
                    scb = ( SessionControlBlock * ) lastNode->data;
                    got_one = true;
                    continue;
                }
                else
                {
                    session_cache->prunes += pruned;
                    Active_Resume();
                    return pruned;
                }
            }

            if((scb->last_data_seen + session_cache->timeoutAggressive) < thetime)
            {
                SessionControlBlock *savscb = scb;

                if(sfxhash_count(session_cache->hashTable) > 1)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "pruning stale session\n"););
                    savscb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;
                    deleteSession(session_cache, savscb, "stale/timeout", false);

                    scb = (SessionControlBlock *) sfxhash_lru(session_cache->hashTable);
                    pruned++;
                    got_one = true;
                }
                else
                {
                    savscb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;
                    deleteSession(session_cache, savscb, "stale/timeout/last scb", false);
                    pruned++;
                    session_cache->prunes += pruned;
                    Active_Resume();
                    return pruned;
                }
            }
            else
            {
                session_cache->prunes += pruned;
                Active_Resume();
                return pruned;
            }

            if (pruned > session_cache->cleanup_sessions)
            {
                /* Don't bother cleaning more than 'n' at a time */
                break;
            }
        } while( ( scb != NULL ) && got_one );

        session_cache->prunes += pruned;
        Active_Resume();
        return pruned;
    }
    else
    {
        /* Free up to 'n' sessions at a time until we get under the memcap or free
         * enough sessions to be able to create new ones.
         */
        uint32_t prune_stop_threshold = session_cache->max_sessions - session_cache->cleanup_sessions;
        while( prune_more_sessions( session_cache, pruned, prune_stop_threshold, memCheck ) )
        {
            unsigned int blocks = 0;
            DEBUG_WRAP( DebugMessage(DEBUG_STREAM,
                        "S5: Pruning session cache by %d scbs for %s: %d/%d\n",
                        session_cache->cleanup_sessions,
                        memCheck ? "memcap" : "hash limit",
                        session_mem_in_use,
                        GetSessionMemCap() ););

            scb = (SessionControlBlock *) sfxhash_lru(session_cache->hashTable);

            if( scb == NULL )
                break;

            if( scb == save_me )
            {
                if(sfxhash_count(session_cache->hashTable) == 1)
                    break;
                moveHashNodeToFront( session_cache );
                continue;
            }

            if( memCheck )
            {
                if ( isSessionBlocked( scb ) )
                {
                    if( ++blocks >= sfxhash_count( session_cache->hashTable ) )
                        break;

                    moveHashNodeToFront( session_cache );
                    continue;
                }
                else
                {
                    scb->ha_state.session_flags |= SSNFLAG_PRUNED;
                    deleteSession( session_cache, scb, "memcap/check", false );
                    pruned++;
                }
            }
            else
            {
                scb->ha_state.session_flags |= SSNFLAG_PRUNED;
                deleteSession( session_cache, scb, "memcap/stale", false );
                pruned++;
            }

            if ( pruned >= session_cache->cleanup_sessions )
                break;
        }
    }

    if( memCheck && pruned )
    {
	ErrorMessageThrottled(&error_throttleInfo,"S5: Pruned %d sessions from cache for memcap. %d scbs remain. memcap: %d/%d\n",
                    pruned, sfxhash_count( session_cache->hashTable ),
                    session_mem_in_use,
                    GetSessionMemCap() );
        DEBUG_WRAP( if( sfxhash_count(session_cache->hashTable) == 1 )
                    {
                        DebugMessage(DEBUG_STREAM, "S5: Pruned, one session remains\n");
                    } );
    }
    session_cache->prunes += pruned;
    Active_Resume();
    return pruned;
}

static void freeMplsHeaders(SessionControlBlock *scb)
{
    if ( scb->clientMplsHeader->start != NULL )
    {
         SnortPreprocFree(scb->clientMplsHeader->start,
                          scb->clientMplsHeader->length, PP_STREAM,
                          PP_MEM_CATEGORY_SESSION);
         scb->clientMplsHeader->start = NULL;
    }
    SnortPreprocFree(scb->clientMplsHeader, sizeof(MPLS_Hdr), PP_STREAM,
                     PP_MEM_CATEGORY_SESSION);
    scb->clientMplsHeader = NULL;
    if (scb->serverMplsHeader->start != NULL )
    {
         SnortPreprocFree(scb->serverMplsHeader->start,
                     scb->serverMplsHeader->length, PP_STREAM,
                     PP_MEM_CATEGORY_SESSION);
         scb->serverMplsHeader->start = NULL;
    }
    SnortPreprocFree(scb->serverMplsHeader, sizeof(MPLS_Hdr),
                    PP_STREAM, 0);
    scb->serverMplsHeader = NULL;
}

static void initMplsHeaders(SessionControlBlock *scb)
{
    scb->clientMplsHeader = (MPLS_Hdr*)SnortPreprocAlloc(1, sizeof(MPLS_Hdr),
                                         PP_STREAM, PP_MEM_CATEGORY_CONFIG);
    scb->serverMplsHeader = (MPLS_Hdr*)SnortPreprocAlloc(1, sizeof(MPLS_Hdr),
                                         PP_STREAM, PP_MEM_CATEGORY_CONFIG);
}


static void *createSession(SessionCache* session_cache, Packet *p, const SessionKey *key )
{
    SessionControlBlock *scb = NULL;
    SFXHASH_NODE *hnode;
    StreamFlowData *flowdata;
    time_t timestamp = p ? p->pkth->ts.tv_sec : packet_time();

    if( session_cache == NULL )
        return NULL;

    hnode = sfxhash_get_node(session_cache->hashTable, key);
    if (!hnode)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "HashTable full, clean One Way Sessions.\n"););
        if( pruneOneWaySessions( session_cache ) == 0 )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "No One Way Sessions, clean timedout sessions.\n"););
            if( pruneSessionCache(session_cache, timestamp, NULL, 0) == 0 )
            {
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "No timedout sessions, clean least recently used.\n"););
                pruneSessionCache(session_cache, 0, NULL, 0);
            }
        }

        /* Should have some freed nodes now */
        hnode = sfxhash_get_node(session_cache->hashTable, key);
#ifdef DEBUG_MSGS
        if (!hnode)
            LogMessage("%s(%d) Problem, no freed nodes\n", __FILE__, __LINE__);
#endif
    }

    if (hnode && hnode->data)
    {
        scb = hnode->data;

        /* Zero everything out */
        memset(scb, 0, sizeof(SessionControlBlock));

        /* Save the session key for future use */
        scb->key = hnode->key;
        scb->session_state = STREAM_STATE_NONE;
        scb->session_established = false;
        scb->protocol = key->protocol;
        scb->last_data_seen = timestamp;
        scb->flowdata = mempool_alloc(&sessionFlowMempool);
        if( scb->flowdata )
        {
            flowdata = scb->flowdata->data;
            boInitStaticBITOP(&(flowdata->boFlowbits), getFlowbitSizeInBytes(), flowdata->flowb);
        }

        scb->stream_config_stale = true;
        scb->stream_config = NULL;
        scb->proto_policy = NULL;
        scb->napPolicyId = SF_POLICY_UNBOUND;
        scb->ipsPolicyId = SF_POLICY_UNBOUND;
        scb->session_config = session_configuration;

        scb->port_guess = true;

#ifdef MPLS
   if( p != NULL )
   {
        uint8_t layerIndex;
        for(layerIndex=0; layerIndex < p->next_layer; layerIndex++)
        {
             if( p->layers[layerIndex].proto == PROTO_MPLS && p->layers[layerIndex].start != NULL )
             {
                    initMplsHeaders(scb);
                    break;
             }
        }
   }
#endif

#ifdef ENABLE_HA
        if (session_configuration->enable_ha)
        {
            scb->ha_flags |= HA_FLAG_NEW;
            /* Calculate the threshold time for the first HA update message. */
            packet_gettimeofday(&scb->ha_next_update);
            if (session_configuration->ha_config)
            {
                scb->ha_next_update.tv_usec += session_configuration->ha_config->min_session_lifetime.tv_usec;
                if (scb->ha_next_update.tv_usec > 1000000)
                {
                    scb->ha_next_update.tv_usec -= 1000000;
                    scb->ha_next_update.tv_sec++;
                }
                scb->ha_next_update.tv_sec += session_configuration->ha_config->min_session_lifetime.tv_sec;
            }

            memset( &scb->ha_state, '\0', sizeof( StreamHAState ) );
            scb->cached_ha_state = scb->ha_state;
            scb->new_session = true;
        }
#endif

        // all sessions are one-way when created so add to oneway session list...
        insertIntoOneWaySessionList( session_cache, scb );

        if (pkt_trace_enabled)
            addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
                "Session: new snort session\n"));
    }

    return scb;
}

static bool isSessionVerified( void *ssn )
{
    if ( ssn != NULL )
        return ( ( SessionControlBlock * ) ssn )->session_established;
    else
        return false;
}

static void removeSessionFromProtoOneWayList( uint32_t proto, void *scb )
{
    if( proto < SESSION_PROTO_MAX )
    {
        removeFromOneWaySessionList( proto_session_caches[ proto ], ( SessionControlBlock * ) scb );
    }
    else
    {
        WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, proto);
    }
}

static void cleanProtocolSessionsPool( uint32_t protocol )
{
    if( protocol < SESSION_PROTO_MAX )
    {
        if( proto_session_caches[protocol] )
            mempool_clean( proto_session_caches[protocol]->protocol_session_pool );
    }
    else
    {
        WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, protocol);
    }
}

static void freeProtocolSessionsPool( uint32_t protocol, void *scb )
{
    if( protocol < SESSION_PROTO_MAX )
    {
        if( proto_session_caches[protocol] )
            mempool_free( proto_session_caches[protocol]->protocol_session_pool,
                    ( ( SessionControlBlock * ) scb )->proto_specific_data );
    }
    else
    {
        WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, protocol);
    }
}

static void *allocateProtocolSession( uint32_t protocol )
{
    if( protocol < SESSION_PROTO_MAX )
    {
        if( proto_session_caches[protocol] )
            return mempool_force_alloc( proto_session_caches[protocol]->protocol_session_pool );
    }
    else
    {
        WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, protocol);
    }

    return NULL;
}


static uint32_t HashFunc(SFHASHFCN *p, unsigned char *d, int n)
{
    uint32_t a,b,c;
    uint32_t offset = 0;
#ifdef MPLS
    uint32_t tmp = 0;
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    uint32_t tmp2 = 0;
#endif

    a = *(uint32_t*)d;         /* IPv6 lo[0] */
    b = *(uint32_t*)(d+4);     /* IPv6 lo[1] */
    c = *(uint32_t*)(d+8);     /* IPv6 lo[2] */

    mix(a,b,c);

    a += *(uint32_t*)(d+12);   /* IPv6 lo[3] */
    b += *(uint32_t*)(d+16);   /* IPv6 hi[0] */
    c += *(uint32_t*)(d+20);   /* IPv6 hi[1] */

    mix(a,b,c);

    a += *(uint32_t*)(d+24);   /* IPv6 hi[2] */
    b += *(uint32_t*)(d+28);   /* IPv6 hi[3] */
    c += *(uint32_t*)(d+32);   /* port lo & port hi */

    mix(a,b,c);

    offset=36;
    a += *(uint32_t*)(d+offset);   /* vlan, protocol, & pad */
#ifdef MPLS
    tmp = *(uint32_t *)(d+offset+4);
    if( tmp )
    {
        b += tmp;   /* mpls label */
    }
    offset += 8;    /* skip past vlan/proto/ipver & mpls label */
#else
    offset += 4;    /* skip past vlan/proto/ipver */
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    tmp2 = *(uint32_t*)(d+offset); /* after offset that has been moved */
    c += tmp2; /* address space id and 16bits of zero'd pad */
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    mix(a,b,c);
    a += *(uint32_t*)(d+offset+4);
#endif

    final(a,b,c);

    return c;
}

static int HashKeyCmp(const void *s1, const void *s2, size_t n)
{
#ifndef SPARCV9 /* ie, everything else, use 64bit comparisons */
    uint64_t *a,*b;

    a = (uint64_t*)s1;
    b = (uint64_t*)s2;
    if(*a - *b) return 1;       /* Compares IPv4 lo/hi */
    /* Compares IPv6 low[0,1] */

    a++;
    b++;
    if(*a - *b) return 1;       /* Compares port lo/hi, vlan, protocol, pad */
    /* Compares IPv6 low[2,3] */

    a++;
    b++;
    if(*a - *b) return 1;       /* Compares IPv6 hi[0,1] */

    a++;
    b++;
    if(*a - *b) return 1;       /* Compares IPv6 hi[2,3] */

    a++;
    b++;
    if(*a - *b) return 1;       /* Compares port lo/hi, vlan, protocol, pad */

#ifdef MPLS
    a++;
    b++;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    if (*a - *b) return 1;      /* Compares MPLS label, AddressSpace ID and 16bit pad */
#else
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares mpls label, no pad */
    }
#endif
#else
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    a++;
    b++;
    {
        uint16_t *x, *y;
        x = (uint16_t *)a;
        y = (uint16_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares addressSpaceID, no pad */
    }
#endif
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
   a++;
   b++;
   {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        if (*x - *y) return 1; /* Compares carrierID */
   }
#endif

#else /* SPARCV9 */
    uint32_t *a,*b;

    a = (uint32_t*)s1;
    b = (uint32_t*)s2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv4 lo/hi */
    /* Compares IPv6 low[0,1] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares port lo/hi, vlan, protocol, pad */
    /* Compares IPv6 low[2,3] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv6 hi[0,1] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares IPv6 hi[2,3] */

    a+=2;
    b+=2;
    if ((*a - *b) || (*(a+1) - *(b+1))) return 1;       /* Compares port lo/hi, vlan, protocol, pad */

#ifdef MPLS
    a+=2;
    b+=2;
    {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares mpls label */
    }
#endif
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#ifdef MPLS
    a++;
    b++;
#else
    a+=2;
    b+=2;
#endif
    {
        uint16_t *x, *y;
        x = (uint16_t *)a;
        y = (uint16_t *)b;
        //x++;
        //y++;
        if (*x - *y) return 1;  /* Compares addressSpaceID, no pad */
    }
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
   a++;
   b++;
   {
        uint32_t *x, *y;
        x = (uint32_t *)a;
        y = (uint32_t *)b;
        if (*x - *y) return 1; /* Compares carrierID */
   }
#endif
#endif /* SPARCV9 */

    return 0;
}

static SessionCache* initSessionCache(uint32_t session_type, uint32_t protocol_scb_size, SessionCleanup cleanup_fcn)
{
    int hashTableSize;
    SessionCache *sessionCache = NULL;
    uint32_t max_sessions = 0, session_timeout_min = 0, session_timeout_max = 0;
    uint32_t cleanup_sessions = 5;

    switch ( session_type )
    {
        case SESSION_PROTO_TCP:
            if( session_configuration->track_tcp_sessions == STREAM_TRACK_YES )
            {
                max_sessions = session_configuration->max_tcp_sessions;
                if (session_configuration->memcap > (max_sessions * protocol_scb_size))
                    session_configuration->memcap = session_configuration->memcap
                        - (max_sessions * protocol_scb_size);
                session_timeout_min = session_configuration->tcp_cache_pruning_timeout;
                session_timeout_max = session_configuration->tcp_cache_nominal_timeout;
            }
            break;

        case SESSION_PROTO_UDP:
            if( session_configuration->track_udp_sessions == STREAM_TRACK_YES )
            {
                max_sessions = session_configuration->max_udp_sessions;
                session_timeout_min = session_configuration->udp_cache_pruning_timeout;
                session_timeout_max = session_configuration->udp_cache_nominal_timeout;
            }
            break;

        case SESSION_PROTO_ICMP:
            if( session_configuration->track_icmp_sessions == STREAM_TRACK_YES )
            {
                max_sessions = session_configuration->max_icmp_sessions;
                session_timeout_min = 30;
                session_timeout_max = 3 * session_timeout_min;
            }
            break;

        case SESSION_PROTO_IP:
            if( session_configuration->track_ip_sessions == STREAM_TRACK_YES )
            {
                max_sessions = session_configuration->max_ip_sessions;
                session_timeout_min = 30;
                session_timeout_max = 3 * session_timeout_min;
            }
            break;

        default:
            WarningMessage("%s(%d) Invalid session protocol type: %d", file_name, file_line, session_type);
            return NULL;
            break;
    }

    // only create a case for session controls for this protocol if tracking is enabled
    if( max_sessions > 0 )
    {
        // set hash table size to max sessions value...adjust up to avoid collisions????
        hashTableSize = max_sessions;

        sessionCache = SnortPreprocAlloc( 1, sizeof( SessionCache ), PP_STREAM,
                              PP_MEM_CATEGORY_CONFIG );
        if( sessionCache )
        {
            sessionCache->timeoutAggressive = session_timeout_min;
            sessionCache->timeoutNominal = session_timeout_max;
            sessionCache->max_sessions = max_sessions;
            if( cleanup_sessions < ( 2 * max_sessions ) )
                sessionCache->cleanup_sessions = cleanup_sessions;
            else
                sessionCache->cleanup_sessions = ( max_sessions / 2 );
            if( sessionCache->cleanup_sessions == 0 )
                sessionCache->cleanup_sessions = 1;

            sessionCache->cleanup_fcn = cleanup_fcn;

            // init one way sessions list control block...
            sessionCache->ows_list.head = NULL;
            sessionCache->ows_list.tail = NULL;
            sessionCache->ows_list.num_sessions = 0;
            // tune these values for managing pruning of one-way sessions
            sessionCache->ows_list.prune_threshold = max_sessions * 0.05;
            if( sessionCache->ows_list.prune_threshold == 0 )
                sessionCache->ows_list.prune_threshold = 10;
            sessionCache->ows_list.prune_max = sessionCache->ows_list.prune_threshold * 0.25;

            /* Okay, now create the table */
            sessionCache->hashTable = sfxhash_new( hashTableSize, sizeof(SessionKey), sizeof(SessionControlBlock),
                    //maxSessionMem + tableMem, 0, NULL, NULL, 1);
                0, 0, NULL, NULL, 1 );

            sfxhash_set_max_nodes( sessionCache->hashTable, max_sessions );
            sfxhash_set_keyops( sessionCache->hashTable, HashFunc, HashKeyCmp );

            // now alloc and initial memory for protocol specific session blocks
            if( protocol_scb_size > 0 )
            {
                sessionCache->protocol_session_pool = SnortAlloc(sizeof(MemPool));
                if( mempool_init( sessionCache->protocol_session_pool, max_sessions, protocol_scb_size ) != 0 )
                {
                    FatalError( "%s(%d) Could not initialize protocol session memory pool.\n",
                            __FILE__, __LINE__ );
                }
            }
        }
        else
        {
            FatalError( "%s(%d) Unable to create a stream session cache for protocol type: %d.\n",
                    file_name, file_line, session_type);
        }
    }
    else
    {
        WarningMessage("%s(%d) Protocol tracking disabled for protocol type: %d\n",
                file_name, file_line, session_type);
    }

    proto_session_caches[ session_type ] = sessionCache;
    return sessionCache;
}

static void printSessionCache(SessionCache* sessionCache)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "%lu sessions active\n",
                sfxhash_count( sessionCache->hashTable ) ););
}

static void checkCacheFlowTimeout(uint32_t flowCount, time_t cur_time, SessionCache *cache)
{
    uint32_t flowRetiredCount = 0, flowExaminedCount = 0;
    SessionControlBlock *scb;
    SFXHASH_NODE *hnode, *hnode_prev;

    if( !cache )
        return;

    hnode_prev = cache->nextTimeoutEvalNode;
    while( flowRetiredCount < flowCount && flowExaminedCount < ( 2 * flowCount ) )
    {
        if( !( hnode = hnode_prev ) && !( hnode = sfxhash_lru_node( cache->hashTable ) ) )
            break;

        scb = ( SessionControlBlock * ) hnode->data;
        if( ( time_t ) ( scb->last_data_seen + cache->timeoutNominal ) > cur_time )
        {
            uint64_t time_jiffies;
            /*  Give extra 1 second delay*/
            time_jiffies = ((uint64_t)cur_time - 1) * TCP_HZ;

            if( !( ( scb->expire_time != 0 )  && ( cur_time != 0 ) && ( scb->expire_time <= time_jiffies ) ) )
                break;
        }

        hnode_prev = hnode->gprev;
        flowExaminedCount++;

#ifdef ENABLE_HA
        if( scb->ha_flags & HA_FLAG_STANDBY )
            continue;
#endif

        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "retiring stale session\n"););
        scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;
        deleteSession(cache, scb, "stale/timeout", false);
        flowRetiredCount++;
    }

    cache->nextTimeoutEvalNode = hnode_prev;
}

/*get next flow from session cache. */
static void checkSessionTimeout(uint32_t flowCount, time_t cur_time)
{
    Active_Suspend( );
    checkCacheFlowTimeout(flowCount, cur_time, proto_session_caches[SESSION_PROTO_TCP]);
    checkCacheFlowTimeout(flowCount, cur_time, proto_session_caches[SESSION_PROTO_UDP]);
    checkCacheFlowTimeout(flowCount, cur_time, proto_session_caches[SESSION_PROTO_IP]);
    //icmp_lws_cache does not need cleaning
    Active_Resume( );
}

static int isProtocolTrackingEnabled( IpProto proto )
{
    int tracking_enabled = false;

    switch ( proto )
    {
        case SESSION_PROTO_TCP:
            tracking_enabled = session_configuration->track_tcp_sessions;
            break;

        case SESSION_PROTO_UDP:
            tracking_enabled = session_configuration->track_udp_sessions;
            break;

        case SESSION_PROTO_ICMP:
            tracking_enabled = session_configuration->track_icmp_sessions;
            break;

        case SESSION_PROTO_IP:
            tracking_enabled = session_configuration->track_ip_sessions;
            break;

        default:
            WarningMessage("%s(%d) Invalid session protocol id: %d", file_name, file_line, proto);
            break;
    }

    return ( tracking_enabled );
}

static int setApplicationData( void *scbptr, uint32_t protocol, void *data, StreamAppDataFree free_func )
{
    SessionControlBlock *scb;
    StreamAppData *appData = NULL;
    if (scbptr)
    {
        scb = ( SessionControlBlock * ) scbptr;
        appData = scb->appDataList;
        while (appData)
        {
            if (appData->protocol == protocol)
            {
                /* If changing the pointer to the data, free old one */
                if ((appData->freeFunc) && (appData->dataPointer != data))
                {
                    if ( appData->dataPointer )
                        appData->freeFunc(appData->dataPointer);
                }
                else
                {
                    /* Same pointer, same protocol.  Go away */
                    break;
                }

                appData->dataPointer = NULL;
                break;
            }

            appData = appData->next;
        }

        /* If there isn't one for this protocol, allocate */
        if (!appData)
        {
            appData = SnortPreprocAlloc(1, sizeof(StreamAppData), PP_STREAM,
                              PP_MEM_CATEGORY_SESSION);

            /* And add it to the list */
            if (scb->appDataList)
            {
                scb->appDataList->prev = appData;
            }
            appData->next = scb->appDataList;
            scb->appDataList = appData;
        }

        /* This will reset free_func if it already exists */
        appData->protocol = protocol;
        appData->freeFunc = free_func;
        appData->dataPointer = data;

        return 0;
    }
    return -1;
}

static void *getApplicationData( void *scbptr, uint32_t protocol )
{
    SessionControlBlock *scb;
    StreamAppData *appData = NULL;
    void *data = NULL;
    if (scbptr)
    {
        scb = ( SessionControlBlock* ) scbptr;
        appData = scb->appDataList;
        while (appData)
        {
            if (appData->protocol == protocol)
            {
                data = appData->dataPointer;
                break;
            }
            appData = appData->next;
        }
    }
    return data;
}

static void * getSessionHandle(const SessionKey *key)
{
    SessionControlBlock *scb;

    switch(key->protocol)
    {
        case IPPROTO_TCP:
            scb = getSessionControlBlockFromKey(proto_session_caches[SESSION_PROTO_TCP], key);
            break;
        case IPPROTO_UDP:
            scb = getSessionControlBlockFromKey(proto_session_caches[SESSION_PROTO_UDP], key);
            break;
        case IPPROTO_ICMP:
            scb = getSessionControlBlockFromKey(proto_session_caches[SESSION_PROTO_ICMP], key);
            if (scb) break;
            /* fall through */
        default:
            scb = getSessionControlBlockFromKey(proto_session_caches[SESSION_PROTO_IP], key);
            break;
    }

    return (void *)scb;
}

static void *getSessionHandleFromIpPort( sfaddr_t* srcIP, uint16_t srcPort,
                                         sfaddr_t* dstIP, uint16_t dstPort,
                                         char ip_protocol, uint16_t vlan, 
                                         uint32_t mplsId,
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                                         uint16_t address_space_id_src,
                                         uint16_t address_space_id_dst       
#else        
                                         uint16_t addressSpaceId 
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                         , uint32_t carrierId
#endif
                                        )
{
    SessionKey key;

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    initSessionKeyFromPktHeader(srcIP, srcPort, dstIP, dstPort, ip_protocol,
                                vlan, mplsId, address_space_id_src,
                                address_space_id_dst, carrierId, &key);
#else   
    initSessionKeyFromPktHeader(srcIP, srcPort, dstIP, dstPort, ip_protocol,
                                vlan, mplsId, addressSpaceId, carrierId, &key);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    initSessionKeyFromPktHeader(srcIP, srcPort, dstIP, dstPort, ip_protocol,
                                vlan, mplsId, address_space_id_src,
                                address_space_id_dst, &key);
#else
    initSessionKeyFromPktHeader(srcIP, srcPort, dstIP, dstPort, ip_protocol,
                                vlan, mplsId, addressSpaceId, &key);
#endif
#endif

    return (void*)getSessionHandle(&key);
}

static const StreamSessionKey *getKeyFromSession( const void *scbptr )
{
    return ( ( SessionControlBlock * ) scbptr)->key;
}

static StreamSessionKey *getSessionKeyFromPacket( Packet *p )
{
    SessionKey *key = SnortPreprocAlloc(1, sizeof(*key), PP_STREAM,
                          PP_MEM_CATEGORY_SESSION);

    if (!key)
        return NULL;

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid = GET_OUTER_IPH_PROTOID(p, pkth);

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
    uint16_t sAsId = DAQ_GetSourceAddressSpaceID(p->pkth);
    uint16_t dAsId = DAQ_GetDestinationAddressSpaceID(p->pkth);

    initSessionKeyFromPktHeader(GET_SRC_IP(p), p->sp, GET_DST_IP(p), p->dp,
                                GET_IPH_PROTO(p), p->vh ? VTH_VLAN(p->vh) : 0,
                                p->mplsHdr.label, sAsId, dAsId, cid, key);
#else     
    uint16_t addressSpaceId = 0;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    addressSpaceId = DAQ_GetAddressSpaceID(p->pkth);
#endif

    initSessionKeyFromPktHeader( GET_SRC_IP(p), p->sp, GET_DST_IP(p), p->dp, GET_IPH_PROTO(p),
                                 p->vh ? VTH_VLAN(p->vh) : 0,
                                 p->mplsHdr.label, addressSpaceId, cid, key);
#endif    
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t sAsId = DAQ_GetSourceAddressSpaceID(p->pkth);
    uint16_t dAsId = DAQ_GetDestinationAddressSpaceID(p->pkth);

    initSessionKeyFromPktHeader(GET_SRC_IP(p), p->sp, GET_DST_IP(p), p->dp,
                                GET_IPH_PROTO(p), p->vh ? VTH_VLAN(p->vh) : 0,
                                p->mplsHdr.label, sAsId, dAsId, key);
#else
    uint16_t addressSpaceId = 0;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    addressSpaceId = DAQ_GetAddressSpaceID(p->pkth);
#endif

    initSessionKeyFromPktHeader( GET_SRC_IP(p), p->sp, GET_DST_IP(p), p->dp, GET_IPH_PROTO(p),
                                 p->vh ? VTH_VLAN(p->vh) : 0,
                                 p->mplsHdr.label, addressSpaceId, key);
#endif
#endif
    return key;
}

static void * getApplicationDataFromSessionKey(const StreamSessionKey *key, uint32_t protocol)
{
    SessionControlBlock *scb = getSessionHandle(key);
    return getApplicationData(scb, protocol);
}

static void *getApplicationDataFromIpPort( sfaddr_t* srcIP, uint16_t srcPort,
                                           sfaddr_t* dstIP, uint16_t dstPort,
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)   
                                           uint16_t address_space_id_src,
                                           uint16_t address_space_id_dst,
#else
                                           uint16_t addressSpaceID,
#endif       
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                                           uint32_t carrierId,
#endif 
                                           char ip_protocol, uint16_t vlan,
                                           uint32_t mplsId, uint32_t protocol
                                         )
{
    SessionControlBlock *scb;

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    scb = (SessionControlBlock *) getSessionHandleFromIpPort(srcIP, srcPort,
                                                             dstIP, dstPort,
                                                             ip_protocol, vlan,
                                                             mplsId, carrierId, address_space_id_src,
                                                             address_space_id_dst);
#else    
    scb = (SessionControlBlock *) getSessionHandleFromIpPort(srcIP, srcPort, dstIP,
                                                             dstPort, ip_protocol, vlan,
                                                             mplsId, carrierId, addressSpaceID);
#endif    
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
    scb = (SessionControlBlock *) getSessionHandleFromIpPort(srcIP, srcPort,
                                                             dstIP, dstPort,
                                                             ip_protocol, vlan,
                                                             mplsId, address_space_id_src, address_space_id_dst);
#else
    scb = (SessionControlBlock *) getSessionHandleFromIpPort(srcIP, srcPort, dstIP,
                                                             dstPort, ip_protocol, vlan,
                                                             mplsId, addressSpaceID);
#endif
#endif

    return getApplicationData(scb, protocol);
}

static void deleteSessionIfClosed( Packet* p )
{
    SessionControlBlock* scb;

    if (!p || !p->ssnptr)
        return;

    scb = ( SessionControlBlock * ) p->ssnptr;
    if( !scb->session_established )
        return;

    if (scb->session_state & STREAM_STATE_CLOSED)
    {
        if (scb->is_session_deletion_delayed) {
            setSessionExpirationTime( p, scb, STREAM_DELAY_TIMEOUT_AFTER_CONNECTION_ENDED);
            return;
        }
        switch (scb->protocol)
        {
            case IPPROTO_TCP:
                deleteSession(proto_session_caches[SESSION_PROTO_TCP], scb, "closed normally", true);
                p->ssnptr = NULL;
                break;
            case IPPROTO_UDP:
                deleteSession(proto_session_caches[SESSION_PROTO_UDP], scb, "closed normally", true);
                p->ssnptr = NULL;
                break;
            case IPPROTO_ICMP: /* matching how sessionPacketProcessor() stores ICMP sessions */
                if ( SessionTrackingEnabled( session_configuration, SESSION_PROTO_ICMP ) )
                {
                    deleteSession(proto_session_caches[SESSION_PROTO_ICMP], scb, "closed normally", true);
                    p->ssnptr = NULL;
                    break;
                }
                // fall thru, not tracking ICMP, treat as IP packet...
            case IPPROTO_IP:
            default: /* matching how sessionPacketProcessor() stores default sessions */
                deleteSession(proto_session_caches[SESSION_PROTO_IP], scb, "closed normally", true);
                p->ssnptr = NULL;
                break;
        }
    }
}

static void setSessionExpirationTime(Packet *p, void *scb, uint32_t timeout)
{
    ( ( SessionControlBlock * ) scb)->expire_time = CalcJiffies(p) + (timeout * TCP_HZ);
    return;
}

static int getSessionExpirationTime(Packet *p, void *scb)
{
    if ( ( ( SessionControlBlock * ) scb)->expire_time == 0 ) return 0;
    return ( CalcJiffies(p) > ( ( SessionControlBlock * ) scb)->expire_time );
}

#
static uint32_t setSessionFlags( void *scbptr, uint32_t flags )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    if( scb)
    {
        if( ( scb->ha_state.session_flags & flags ) != flags )
        {
#ifdef ENABLE_HA
            scb->ha_flags |= HA_FLAG_MODIFIED;
            if((scb->ha_state.session_flags & HA_CRITICAL_SESSION_FLAGS) != (flags & HA_CRITICAL_SESSION_FLAGS))
                scb->ha_flags |= HA_FLAG_CRITICAL_CHANGE;

            if( scb->protocol == IPPROTO_TCP
                    &&
                    ( scb->ha_state.session_flags & HA_TCP_MAJOR_SESSION_FLAGS )
                    != ( flags & HA_TCP_MAJOR_SESSION_FLAGS ) )
                scb->ha_flags |= HA_FLAG_MAJOR_CHANGE;

#endif
            scb->ha_state.session_flags |= flags;
        }
        return scb->ha_state.session_flags;
    }

    return 0;
}

static uint32_t getSessionFlags( void *scbptr )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    if( scb )
        return scb->ha_state.session_flags;

    return 0;
}

static tSfPolicyId getSessionPolicy( void *scbptr, int policy_type )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( policy_type == SNORT_NAP_POLICY )
        return scb->napPolicyId;
    else
        return scb->ipsPolicyId;
}

void setSessionPolicy( void *scbptr, int policy_type, tSfPolicyId id )
{
    SessionControlBlock *scb = (SessionControlBlock *) scbptr;

    if ( scb == NULL )
        return;

    if( policy_type == SNORT_NAP_POLICY )
        scb->napPolicyId = id;
    else
        scb->ipsPolicyId = id;
}

static void setSessionDeletionDelayed( void *scbptr, bool delay_session_deletion_flag)
{
    SessionControlBlock *scb = (SessionControlBlock *) scbptr;
    if ( scb == NULL )
        return;

    scb->is_session_deletion_delayed = delay_session_deletion_flag;
}

static bool isSessionDeletionDelayed( void *scbptr )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    if( scb == NULL )
        return !STREAM_DELAY_SESSION_DELETION;

    return scb->is_session_deletion_delayed;
}

static int ignoreChannel( const Packet *ctrlPkt, sfaddr_t* srcIP, uint16_t srcPort,
        sfaddr_t* dstIP, uint16_t dstPort, uint8_t protocol, uint32_t preprocId,
        char direction, char flags, struct _ExpectNode** packetExpectedNode )
{
    return StreamExpectAddChannel( ctrlPkt, srcIP, srcPort, dstIP, dstPort, direction, flags,
            protocol, STREAM_EXPECTED_CHANNEL_TIMEOUT, 0, preprocId, NULL, NULL,
            packetExpectedNode );
}

static int getIgnoreDirection( void *scbptr )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    if( scb == NULL )
        return SSN_DIR_NONE;

    return scb->ha_state.ignore_direction;
}

static int setIgnoreDirection( void *scbptr, int ignore_direction )
{
    SessionControlBlock *scb = (SessionControlBlock *)scbptr;
    if( scb == NULL )
        return 0;
    if( scb->ha_state.ignore_direction != ignore_direction )
    {
        scb->ha_state.ignore_direction = ignore_direction;
#ifdef ENABLE_HA
        ha_track_modify( scb );
#endif
    }
    return scb->ha_state.ignore_direction;
}

static void disableInspection( void *scbptr, Packet *p )
{
    /*
     * Don't want to mess up PortScan by "dropping"
     * this packet.
     *
     * Also still want the perfmon to collect the stats.
     *
     * And don't want to do any detection with rules
     */
    DisableDetect( p );
    otn_tmp = NULL;
}

static void stopInspection( void * scbptr, Packet *p, char dir, int32_t bytes, int response )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( scb == NULL )
        return;

    switch( dir )
    {
        case SSN_DIR_BOTH:
        case SSN_DIR_FROM_CLIENT:
        case SSN_DIR_FROM_SERVER:
            if( scb->ha_state.ignore_direction != dir )
            {
                scb->ha_state.ignore_direction = dir;
#ifdef ENABLE_HA
                ha_track_modify( scb );
#endif
            }
            break;
    }

    /* Flush any queued data on the client and/or server */
    if( scb->protocol == IPPROTO_TCP )
    {
        if( scb->ha_state.ignore_direction & SSN_DIR_TO_CLIENT )
            if( getSessionPlugins()->flush_client_stream != NULL )
                getSessionPlugins()->flush_client_stream( p, scb );

        if( scb->ha_state.ignore_direction & SSN_DIR_TO_SERVER )
            if( getSessionPlugins()->flush_server_stream != NULL )
                getSessionPlugins()->flush_server_stream( p, scb );
    }

    /* TODO: Handle bytes/response parameters */

    disableInspection( scb, p );
}

static void resumeInspection( void *scbptr, char dir )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( !scb )
        return;

    switch( dir )
    {
        case SSN_DIR_BOTH:
        case SSN_DIR_FROM_CLIENT:
        case SSN_DIR_FROM_SERVER:
            if( scb->ha_state.ignore_direction & dir )
            {
                scb->ha_state.ignore_direction &= ~dir;
#ifdef ENABLE_HA
                ha_track_modify( scb );
#endif
            }
            break;
    }

}

static uint32_t getPacketDirection( Packet *p )
{
    SessionControlBlock *scb;

    if( ( p == NULL ) || ( p->ssnptr == NULL ) )
        return 0;

    scb = ( SessionControlBlock * ) p->ssnptr;

    setPacketDirectionFlag( p, scb );

    return ( p->packet_flags & ( PKT_FROM_SERVER | PKT_FROM_CLIENT ) );
}

static void dropTraffic( Packet* p, void *scbptr, char dir )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( scb == NULL )
        return;

    if( ( dir & SSN_DIR_FROM_CLIENT ) && !( scb->ha_state.session_flags & SSNFLAG_DROP_CLIENT ) )
    {
        scb->ha_state.session_flags |= SSNFLAG_DROP_CLIENT;
        if ( Active_PacketForceDropped() )
            scb->ha_state.session_flags |= SSNFLAG_FORCE_BLOCK;
#ifdef ENABLE_HA
        scb->ha_flags |= ( HA_FLAG_MODIFIED | HA_FLAG_CRITICAL_CHANGE );
#endif
    }

    if( ( dir & SSN_DIR_FROM_SERVER ) && !( scb->ha_state.session_flags & SSNFLAG_DROP_SERVER ) )
    {
        scb->ha_state.session_flags |= SSNFLAG_DROP_SERVER;
        if( Active_PacketForceDropped() )
            scb->ha_state.session_flags |= SSNFLAG_FORCE_BLOCK;
#ifdef ENABLE_HA
        scb->ha_flags |= ( HA_FLAG_MODIFIED | HA_FLAG_CRITICAL_CHANGE );
#endif
    }
}

static StreamFlowData *getFlowData( Packet *p )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) p->ssnptr;

    if ( ( scb == NULL ) || ( scb->flowdata == NULL ) )
        return NULL;

    return (StreamFlowData *)scb->flowdata->data;
}

static uint16_t getPreprocessorStatusBit( void )
{
    static uint16_t preproc_filter_status_bit = PORT_MONITOR_SESSION;

    preproc_filter_status_bit <<= 1;

    return preproc_filter_status_bit;
}

static int setAppProtocolIdExpected( const Packet *ctrlPkt, sfaddr_t* srcIP, uint16_t srcPort,
        sfaddr_t* dstIP, uint16_t dstPort, uint8_t protocol, int16_t protoId, uint32_t preprocId,
        void *protoData, void (*protoDataFreeFn)(void*), struct _ExpectNode** packetExpectedNode)
{
    return StreamExpectAddChannel( ctrlPkt, srcIP, srcPort, dstIP, dstPort, SSN_DIR_BOTH, 0, protocol,
            STREAM_EXPECTED_CHANNEL_TIMEOUT, protoId, preprocId, protoData, protoDataFreeFn,
            packetExpectedNode );
}

#ifdef TARGET_BASED
void setIpProtocol( SessionControlBlock *scb )
{
    switch (scb->protocol)
    {
        case IPPROTO_TCP:
            if (scb->ha_state.ipprotocol != protocolReferenceTCP)
            {
                scb->ha_state.ipprotocol = protocolReferenceTCP;
#ifdef ENABLE_HA
                ha_track_modify(scb);
#endif
            }
            break;
        case IPPROTO_UDP:
            if (scb->ha_state.ipprotocol != protocolReferenceUDP)
            {
                scb->ha_state.ipprotocol = protocolReferenceUDP;
#ifdef ENABLE_HA
                ha_track_modify(scb);
#endif
            }
            break;
        case IPPROTO_ICMP:
            if (scb->ha_state.ipprotocol != protocolReferenceICMP)
            {
                scb->ha_state.ipprotocol = protocolReferenceICMP;
#ifdef ENABLE_HA
                ha_track_modify(scb);
#endif
            }
            break;
    }
}

void setAppProtocolIdFromHostEntry( SessionControlBlock *scb, HostAttributeEntry *host_entry, int direction )
{
    int16_t application_protocol;

    if ( ( scb == NULL ) || ( host_entry == NULL ) )
        return;

    /* Cool, its already set! */
    if( scb->ha_state.application_protocol != 0 )
        return;

    if( scb->ha_state.ipprotocol == 0 )
    {
        setIpProtocol( scb );
    }

    if( direction == SSN_DIR_FROM_SERVER )
    {
        application_protocol = getApplicationProtocolId( host_entry, scb->ha_state.ipprotocol,
                ntohs( scb->server_port ), SFAT_SERVICE );
    }
    else
    {
        application_protocol = getApplicationProtocolId( host_entry, scb->ha_state.ipprotocol,
                ntohs( scb->client_port ), SFAT_SERVICE );
    }

    if( scb->ha_state.application_protocol != application_protocol )
    {
        scb->ha_state.application_protocol = application_protocol;
#ifdef ENABLE_HA
        ha_track_modify( scb );
#endif
        // modify enabled preprocs mask to include only those always run and the ones
        // registered for this application id
        scb->enabled_pps = appHandlerDispatchMask[ application_protocol ]
                           |
                           ( scb->enabled_pps & ( PP_CLASS_NETWORK | PP_CLASS_NGFW ) );

    }
}

#ifdef TARGET_BASED
#ifdef ACTIVE_RESPONSE
static void initActiveResponse( Packet *p, void *pv )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) pv;

    if ( scb == NULL )
        return;

    scb->response_count = 1;

    if ( session_configuration->max_active_responses > 1 )
   {
#if defined(DAQ_CAPA_CST_TIMEOUT)
     if (!Daq_Capa_Timeout)
#endif 
        setSessionExpirationTime( p, scb, session_configuration->min_response_seconds );
}
}
#endif
#endif

#ifdef TARGET_BASED
static uint8_t getHopLimit( void* pv, char dir, int outer )
{
    SessionControlBlock *scb = (SessionControlBlock*)pv;

    if ( scb == NULL )
        return 255;

    if ( SSN_DIR_FROM_CLIENT == dir )
        return outer ? scb->outer_client_ttl : scb->inner_client_ttl;

    return outer ? scb->outer_server_ttl : scb->inner_server_ttl;
}
#endif


static void registerApplicationHandler( uint32_t preproc_id, int16_t app_id )
{

    if( app_id < 0 )
    {
        WarningMessage( "(%s)(%d) Invalid application id: %d.  Application handler registration failed.",
                __FILE__, __LINE__, app_id );
        return;
    }

    // set bit for this preproc in the dispatch mask
    appHandlerDispatchMask[ app_id ] |= ( UINT64_C(1) << preproc_id );
}

static int16_t getAppProtocolId( void *scbptr )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    /* Not caching the source and dest host_entry in the session so we can
     * swap the table out after processing this packet if we need
     * to.  */
    HostAttributeEntry *host_entry = NULL;
    int16_t protocol = 0;

    if( scb == NULL )
        return protocol;

    if( scb->ha_state.application_protocol == -1 )
        return 0;

    if( scb->ha_state.application_protocol != 0 )
        return scb->ha_state.application_protocol;

    if( !IsAdaptiveConfigured( ) )
        return scb->ha_state.application_protocol;

    if( scb->ha_state.ipprotocol == 0 )
    {
        setIpProtocol(scb);
    }

    host_entry = SFAT_LookupHostEntryByIP(IP_ARG(scb->server_ip));
    if( host_entry )
    {
        setAppProtocolIdFromHostEntry(scb, host_entry, SSN_DIR_FROM_SERVER);

        if( scb->ha_state.application_protocol != 0 )
        {
            return scb->ha_state.application_protocol;
        }
    }

    host_entry = SFAT_LookupHostEntryByIP( IP_ARG( scb->client_ip ) );
    if( host_entry )
    {
        setAppProtocolIdFromHostEntry( scb, host_entry, SSN_DIR_FROM_CLIENT );
        if( scb->ha_state.application_protocol != 0 )
        {
            return scb->ha_state.application_protocol;
        }
    }

    scb->ha_state.application_protocol = -1;

    return scb->ha_state.application_protocol;
}

static int16_t setAppProtocolId( void *scbptr, int16_t id )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    if( scb == NULL )
        return 0;

    // modify enabled preprocs mask to include only those always run and the ones
    // registered for this application id
    scb->enabled_pps = appHandlerDispatchMask[ id ] | ( scb->enabled_pps & ( PP_CLASS_NETWORK | PP_CLASS_NGFW ) );

    if( !IsAdaptiveConfigured( ) )
        return 0;

    if( scb->ha_state.application_protocol != id )
    {
        scb->ha_state.application_protocol = id;
#ifdef ENABLE_HA
        ha_track_modify( scb );
#endif
    }

    if( !scb->ha_state.ipprotocol )
        setIpProtocol( scb );

    if( !(scb->protocol == IPPROTO_TCP) || !StreamIsSessionDecryptedTcp( scb ) )
        SFAT_UpdateApplicationProtocol( IP_ARG( scb->server_ip ), ntohs( scb->server_port ),
                scb->ha_state.ipprotocol, id );

    if( scb->protocol == IPPROTO_TCP &&
            scb->ha_state.application_protocol > 0 )
        set_service_based_flush_policy(scb);

    return id;
}

static sfaddr_t* getSessionIpAddress( void *scbptr, uint32_t direction )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( scb != NULL )
    {
        switch( direction )
        {
            case SSN_DIR_FROM_SERVER:
                return ( sfaddr_t* ) ( &( ( SessionControlBlock * ) scb)->server_ip );

            case SSN_DIR_FROM_CLIENT:
                return ( sfaddr_t* ) ( &( ( SessionControlBlock * ) scb)->client_ip );

            default:
                break;
        }
    }

    return NULL;
}

static void getSessionPorts( void *scbptr, uint16_t *client_port, uint16_t *server_port )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( scb != NULL )
    {
        if(client_port)
        {
            *client_port = ntohs(scb->client_port);
        }
        if(server_port)
        {
            *server_port = ntohs(scb->server_port);
        }
    }

    return;
}
#endif

static void getMaxSessions( tSfPolicyId policyId, StreamSessionLimits* limits )
{
    if( session_configuration )
    {
        limits->tcp_session_limit = session_configuration->track_tcp_sessions ?
            session_configuration->max_tcp_sessions : 0;

        limits->udp_session_limit = session_configuration->track_udp_sessions ?
            session_configuration->max_udp_sessions : 0;

        limits->icmp_session_limit = session_configuration->track_icmp_sessions ?
            session_configuration->max_icmp_sessions : 0;

        limits->ip_session_limit = session_configuration->track_ip_sessions ?
            session_configuration->max_ip_sessions : 0;
    }
    else
        memset( limits, 0, sizeof( *limits ) );
}

static void disablePreprocForSession( void *scbptr, uint32_t preproc_id )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;

    if( scb != NULL )
        scb->enabled_pps &= ~( UINT64_C(1) << preproc_id );
}


static void enablePreprocForPort( SnortConfig *sc, uint32_t preproc_id, uint32_t proto, uint16_t port )
{
    tSfPolicyId policy_id = getParserPolicy( sc );
    SnortPolicy *policy = sc->targeted_policies[ policy_id ];

    if( policy == NULL )
    {
        WarningMessage( "Invalid policy id: %d.  No snort policy allocated", policy_id );
        return;
    }

    policy->pp_enabled[ port ] |= ( UINT64_C(1) << preproc_id );
}

static void enablePreprocAllPorts( SnortConfig *sc, uint32_t preproc_id, uint32_t proto )
{
    tSfPolicyId policy_id = getParserPolicy( sc );
    SnortPolicy *policy = sc->targeted_policies[ policy_id ];
    uint32_t port;

    if( policy == NULL )
    {
        WarningMessage( "Invalid policy id: %d.  No snort policy allocated", policy_id );
        return;
    }

    for( port = 0; port < MAX_PORTS; port++ )
        policy->pp_enabled[ port ] |= ( UINT64_C(1) << preproc_id );
}

static void enablePreprocAllPortsAllPolicies( SnortConfig *sc, uint32_t preproc_id, uint32_t proto )
{
    uint32_t i;

    for( i = 0; i < sc->num_policies_allocated; i++ )
    if( ( sc->targeted_policies[ i ] != NULL ) && ( sc->targeted_policies[ i ]->num_preprocs > 0 ) )
    {
        setParserPolicy( sc, i );
        enablePreprocAllPorts( sc, preproc_id, proto );
    }
}

static bool isPreprocEnabledForPort( uint32_t preproc_id, uint16_t port )
{
    tSfPolicyId policy_id = getNapRuntimePolicy();
    SnortPolicy *policy = snort_conf->targeted_policies[ policy_id ];

   if( policy->pp_enabled[ port ] & ( UINT64_C(1) << preproc_id ) )
       return true;
   else
       return false;
}

static void registerNapSelector( nap_selector nap_selector_func )
{
   struct session_plugins *pfunks = getSessionPlugins( );

    if( nap_selector_func != NULL )
        pfunks->select_session_nap = nap_selector_func;
}

static void registerGetHttpXffPrecedence(GetHttpXffPrecedenceFunc fn)
{
    if (!getHttpXffPrecedenceFunc) getHttpXffPrecedenceFunc = fn;
}

static char** getHttpXffPrecedence(void* ssn, uint32_t flags, int* nFields)
{
    if (getHttpXffPrecedenceFunc) return getHttpXffPrecedenceFunc(ssn, flags, nFields);
    else return NULL;
}

static void setReputationUpdateCount (void *scbptr, uint8_t count)
{
    SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
    if ( scb )
        scb->iprep_update_counter = count;
}

#ifdef SNORT_RELOAD
static void reloadSessionConfiguration( struct _SnortConfig *sc, char *args, void **new_config )
{
    SessionConfiguration *session_config = ( SessionConfiguration * ) *new_config;

    // session config is only in default policy... fatal error if not parsing default
    if( getParserPolicy( sc ) != getDefaultPolicy( ) )
    {
        FatalError("%s(%d) Session configuration included in non-default policy.\n", __FILE__, __LINE__);
    }

    if ( session_config == NULL )
    {
        session_config = initSessionConfiguration( );
        parseSessionConfiguration( session_config, args );
        *new_config = session_config;

        session_reload_configuration = session_config;

        sc->run_flags |= RUN_FLAG__STATEFUL;
    }
    else
    {
#if 0
        // EDM-TBD
        FatalError("stream5_global must only be configured once.\n");
#else
        WarningMessage("stream5_global must only be configured once. Ignoring this configuration element\n");
        *new_config = NULL;
#endif
    }
}

void SessionReload(SessionCache* lws_cache, uint32_t max_sessions,
                   uint32_t aggressiveTimeout, uint32_t nominalTimeout
#ifdef REG_TEST
                   , const char* name
#endif
                   )
{
    if (lws_cache)
    {
#ifdef REG_TEST
        if (REG_TEST_FLAG_SESSION_RELOAD & getRegTestFlags())
        {
            const char* excessCountStr;
            excessCountStr = (sfxhash_total_count(lws_cache->hashTable) > max_sessions) ? "" : "no ";
            if (lws_cache->protocol_session_pool)
            {
                const char *excessProtocolCountStr;
                excessProtocolCountStr = (mempool_numTotalBuckets(lws_cache->protocol_session_pool) > max_sessions) ? "" : "no ";
                printf("Setting %s max sessions to %u with %sexcess sessions and %sexcess protocol entries\n",
                       name, max_sessions, excessCountStr, excessProtocolCountStr);
            }
            else
            {
                printf("Setting %s max sessions to %u with %sexcess sessions\n",
                       name, max_sessions, excessCountStr);
            }
            printf("Setting %s aggressive timeout to %u and nominal timeout to %u\n",
                   name, aggressiveTimeout, nominalTimeout);
        }
#endif
#if defined(DAQ_CAPA_CST_TIMEOUT)
        if (!Daq_Capa_Timeout)
#endif
        {
           lws_cache->timeoutNominal = nominalTimeout; 
        }
        lws_cache->timeoutAggressive = aggressiveTimeout;
        sfxhash_set_max_nodes(lws_cache->hashTable, max_sessions);
        if (lws_cache->protocol_session_pool)
            mempool_setNumObjects(lws_cache->protocol_session_pool, max_sessions);
    }
}

unsigned SessionProtocolReloadAdjust(SessionCache* lws_cache, uint32_t max_sessions,
                                     unsigned maxWork, uint32_t memcap
#ifdef REG_TEST
                                     , const char* name
#endif
                                     )
{
    if (lws_cache)
    {
        SessionControlBlock *scb;

        for (; maxWork && sfxhash_total_count(lws_cache->hashTable) > max_sessions; maxWork--)
        {
            if (sfxhash_free_anr(lws_cache->hashTable) != SFXHASH_OK)
            {
                scb = (SessionControlBlock *)sfxhash_lru(lws_cache->hashTable);
                if (scb)
                {
                    scb->ha_state.session_flags |= SSNFLAG_PRUNED;
                    deleteSession(lws_cache, scb, "reload adjust", false);
                }
                else
                    break;
            }
        }

        if (!maxWork)
            return 0;

        if (memcap)
        {
            while (maxWork && session_mem_in_use > memcap)
            {
                scb = (SessionControlBlock *)sfxhash_lru(lws_cache->hashTable);
                if (scb)
                {
                    if (isSessionBlocked(scb))
                        moveHashNodeToFront(lws_cache);
                    else
                    {
                        scb->ha_state.session_flags |= SSNFLAG_PRUNED;
                        deleteSession(lws_cache, scb, "reload adjust", false);
                        maxWork--;
                    }
                }
                else
                    break;
            }

            if (!maxWork)
                return 0;
        }

        if (lws_cache->protocol_session_pool)
        {
            for (; maxWork && mempool_numTotalBuckets(lws_cache->protocol_session_pool) > max_sessions; maxWork--)
            {
                if (mempool_free_bucket(lws_cache->protocol_session_pool))
                    break;
            }
        }
    }
    return maxWork;
}

static bool SessionReloadAdjust(bool idle, tSfPolicyId raPolicyId, void* userData)
{
    unsigned initialMaxWork = idle ? 512 : 5;
    unsigned maxWork;

    Active_Suspend();
    maxWork = SessionTCPReloadAdjust(initialMaxWork);

    if (!maxWork)
    {
        Active_Resume();
        return false;
    }

    maxWork = SessionUDPReloadAdjust(maxWork);

    if (!maxWork)
    {
        Active_Resume();
        return false;
    }

    maxWork = SessionIPReloadAdjust(maxWork);

    if (!maxWork)
    {
        Active_Resume();
        return false;
    }

    maxWork = SessionICMPReloadAdjust(maxWork);

    if (!maxWork)
    {
        Active_Resume();
        return false;
    }

    for (; maxWork && mempool_numTotalBuckets(&sessionFlowMempool) > session_configuration->max_sessions; maxWork--) 
    {
        if (mempool_free_bucket(&sessionFlowMempool))
            break;
    }

#ifdef REG_TEST
    if (REG_TEST_FLAG_SESSION_RELOAD & getRegTestFlags())
    {
        if (maxWork == initialMaxWork)
        {
            SessionCache* lws_cache;

            if ((lws_cache = proto_session_caches[SESSION_PROTO_TCP]))
            {
                printf("TCP session count %u, protocol count %u\n",
                       sfxhash_total_count(lws_cache->hashTable),
                       mempool_numTotalBuckets(lws_cache->protocol_session_pool));
                printf("TCP aggressive timeout %u, nominal timeout %u\n",
                       lws_cache->timeoutAggressive, lws_cache->timeoutNominal);
            }
            if ((lws_cache = proto_session_caches[SESSION_PROTO_UDP]))
            {
                printf("UDP session count %u, protocol count %u\n",
                       sfxhash_total_count(lws_cache->hashTable),
                       mempool_numTotalBuckets(lws_cache->protocol_session_pool));
                printf("UDP aggressive timeout %u, nominal timeout %u\n",
                       lws_cache->timeoutAggressive, lws_cache->timeoutNominal);
            }
            if ((lws_cache = proto_session_caches[SESSION_PROTO_ICMP]))
            {
                printf("ICMP session count %u, protocol count %u\n",
                       sfxhash_total_count(lws_cache->hashTable),
                       mempool_numTotalBuckets(lws_cache->protocol_session_pool));
                printf("ICMP aggressive timeout %u, nominal timeout %u\n",
                       lws_cache->timeoutAggressive, lws_cache->timeoutNominal);
            }
            if ((lws_cache = proto_session_caches[SESSION_PROTO_IP]))
            {
                printf("IP session count %u\n",
                       sfxhash_total_count(lws_cache->hashTable));
                printf("IP aggressive timeout %u, nominal timeout %u\n",
                       lws_cache->timeoutAggressive, lws_cache->timeoutNominal);
            }
            printf("Flow pool count %u\n", mempool_numTotalBuckets(&sessionFlowMempool));

            printf("Memory in use %s the memory cap\n",
                   (session_mem_in_use > session_configuration->memcap) ?
                        "greater than" : "less than or equal to");
            fflush(stdout);
        }
    }
#endif
    return (maxWork == initialMaxWork) ? true : false;
}

#ifdef ENABLE_HA
static bool verifyConfigOptionUnchanged( uint32_t new, uint32_t old, char *name, SessionConfiguration *config )
{
    if( old == new )
        return false;

    ErrorMessage("Session Reload: Changing \"%s\" requires a restart.\n", name);
    return true;
}
#endif

static int verifyReloadedSessionConfiguration( struct _SnortConfig *sc, void *swap_config )
{
    static const char* SESSION_RELOAD = "Session";
    SessionConfiguration *ssc = ( SessionConfiguration * ) swap_config;
    tSfPolicyId tmp_policy_id = getParserPolicy( sc );
    bool restart_required = false;

    if( ( session_configuration == NULL ) || ( ssc == NULL ) )
    {
        FatalError("%s(%d) Session config is NULL.\n", __FILE__, __LINE__);
    }

    if( ( ssc->track_tcp_sessions != session_configuration->track_tcp_sessions ) ||
            ( ssc->track_udp_sessions != session_configuration->track_udp_sessions ) ||
            ( ssc->track_icmp_sessions != session_configuration->track_icmp_sessions ) ||
            ( ssc->track_ip_sessions != session_configuration->track_ip_sessions ) )
    {
        ErrorMessage("Session Reload: Changing tracking of TCP, UDP ICMP, or IP "
                "sessions requires a restart.\n");
        return -1;
    }

    if (ssc->track_tcp_sessions != STREAM_TRACK_YES)
    {
        ssc->memcap = 0; 
        ssc->max_tcp_sessions = 0;
    }
    if (ssc->track_udp_sessions != STREAM_TRACK_YES)
        ssc->max_udp_sessions = 0;
    if (ssc->track_icmp_sessions != STREAM_TRACK_YES)
        ssc->max_icmp_sessions = 0;
    if (ssc->track_ip_sessions != STREAM_TRACK_YES)
        ssc->max_ip_sessions = 0;

#ifdef ENABLE_HA
    restart_required |= verifyConfigOptionUnchanged( ssc->enable_ha,
            session_configuration->enable_ha,
            "enable_ha", ssc );
#endif

    if( restart_required )
    {
        // options were changed that require a restart, let's get out of here...
        ErrorMessage("Session Reload: Verify Failed, Restart Required\n");
        return -1;
    }

    setParserPolicy( sc, getDefaultPolicy()  );
    enablePreprocAllPorts( sc, PP_SESSION, PROTO_BIT__ALL );
    AddFuncToPreprocList(sc, sessionPacketProcessor, PP_SESSION_PRIORITY, PP_SESSION, PROTO_BIT__ALL);
    setParserPolicy( sc, tmp_policy_id );

    initializeMaxExpectedFlows( session_configuration );

    ssc->numSnortPolicies = sc->num_policies_allocated;
    ssc->policy_ref_count = SnortPreprocAlloc( sc->num_policies_allocated,
                                   sizeof( uint32_t ), PP_STREAM,
                                   PP_MEM_CATEGORY_SESSION );
    if (!ssc->policy_ref_count)
        FatalError("%s(%d) policy_ref_count allocation failed.\n", __FILE__, __LINE__);

    ssc->max_sessions = ssc->max_tcp_sessions + ssc->max_udp_sessions +
                        ssc->max_icmp_sessions + ssc->max_ip_sessions;
    if (session_configuration->memcap > ssc->memcap ||
        session_configuration->max_udp_sessions > ssc->max_udp_sessions ||
        session_configuration->max_tcp_sessions > ssc->max_tcp_sessions ||
        session_configuration->max_ip_sessions > ssc->max_ip_sessions ||
        session_configuration->max_icmp_sessions > ssc->max_icmp_sessions)
    {
        ReloadAdjustSessionRegister(sc, SESSION_RELOAD, tmp_policy_id,
                                    &SessionReloadAdjust, NULL, NULL);
    }
    printSessionConfiguration(session_configuration);

    return 0;
}

static void *activateSessionConfiguration( struct _SnortConfig *sc, void *data )
{
    SessionConfiguration *old_config = NULL;
    unsigned int i;

    if( data == NULL )
        return NULL;

    old_config = session_configuration;
    session_configuration = ( SessionConfiguration * ) data;

    SessionTCPReload(session_configuration->max_tcp_sessions,
                     session_configuration->tcp_cache_pruning_timeout,
                     session_configuration->tcp_cache_nominal_timeout);
    SessionUDPReload(session_configuration->max_udp_sessions,
                     session_configuration->udp_cache_pruning_timeout,
                     session_configuration->udp_cache_nominal_timeout);
    SessionICMPReload(session_configuration->max_icmp_sessions,
                      30, 3 * 30);
    SessionIPReload(session_configuration->max_ip_sessions,
                    30, 3 * 30);
#ifdef REG_TEST
    if (REG_TEST_FLAG_SESSION_RELOAD & getRegTestFlags())
    {
        const char* excessStr;

        excessStr = (mempool_numTotalBuckets(&sessionFlowMempool) > session_configuration->max_sessions) ? "" : "no "; 
        printf("Setting flow pool max sessions to %u with %sexcess entries\n",
               session_configuration->max_sessions, excessStr);
        excessStr = (session_mem_in_use > session_configuration->memcap) ? "" : "no "; 
        printf("Setting memcap to %u with %sexcess memory\n",
               session_configuration->memcap, excessStr);
    }
#endif
    mempool_setNumObjects(&sessionFlowMempool, session_configuration->max_sessions); 
    session_reload_configuration = NULL;

#ifdef REG_TEST
    fflush(stdout);
#endif

    for( i = 0; i < old_config->numSnortPolicies; i++ )
    {
        if( old_config->policy_ref_count[ i ] > 0 )
        {
            // some sessions still using config from old policy...
            LogMessage("Session Reload: Reference Count Non-zero for old configuration.\n");
            return NULL;
        }
    }

    return old_config;
}

static void freeSessionConfiguration( void *data )
{
    if( data == NULL )
        return;

    LogMessage("Session Reload: Freeing Session Configuration Memory.\n");
    SessionFreeConfig( ( SessionConfiguration * )  data );
}

#endif

