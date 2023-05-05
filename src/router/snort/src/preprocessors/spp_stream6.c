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
 * @file    spp_stream6.c
 * @author  Martin Roesch <roesch@sourcefire.com>
 *          Steven Sturges <ssturges@sourcefire.com>
 *          davis mcpherson <dmcpherson@sourcefire.com>
 * @date    19 Apr 2005
 *
 * @brief   You can never have too many stream reassemblers...
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
#include "plugbase.h"
#include "session_api.h"
#include "spp_stream6.h"
#include "stream_api.h"
#include "stream_paf.h"
#include "stream_common.h"
#include "session_common.h"
#include "snort_stream_tcp.h"
#include "snort_stream_udp.h"
#include "snort_stream_icmp.h"
#include "snort_stream_ip.h"
#include "snort_session.h"
#include "checksum.h"
#include "mstring.h"
#include "parser/IpAddrSet.h"
#include "decode.h"
#include "detect.h"
#include "generators.h"
#include "event_queue.h"
#include "session_expect.h"
#include "perf.h"
#include "active.h"
#include "sfdaq.h"
#include "ipv6_port.h"
#include "sfPolicy.h"
#include "sp_flowbits.h"
#include "stream5_ha.h"
#include "memory_stats.h"
#include "mempool.h"
#include "control/sfcontrol_funcs.h"

#ifdef TARGET_BASED
#include "sftarget_protocol_reference.h"
#include "sftarget_hostentry.h"
#endif

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats s5PerfStats;
extern PreprocStats s5TcpPerfStats;
extern PreprocStats s5UdpPerfStats;
extern PreprocStats s5IcmpPerfStats;
extern PreprocStats s5IpPerfStats;
#endif

extern SessionCache *proto_session_caches[SESSION_PROTO_MAX];
extern MemPool sessionFlowMempool;

extern OptTreeNode *otn_tmp;

extern FlushConfig ignore_flush_policy[MAX_PORTS];
#ifdef TARGET_BASED
extern FlushConfig ignore_flush_policy_protocol[MAX_PROTOCOL_ORDINAL];
#endif


/*  M A C R O S  **************************************************/
#define PP_STREAM6_PRIORITY  PRIORITY_CORE + PP_CORE_ORDER_STREAM

/*  G L O B A L S  **************************************************/
tSfPolicyUserContextId stream_parsing_config = NULL;
tSfPolicyUserContextId stream_online_config = NULL;

SessionConfiguration *stream_session_config = NULL;

StreamStats s5stats;

uint32_t xtradata_func_count = 0;
LogFunction xtradata_map[LOG_FUNC_MAX];
LogExtraData extra_data_log = NULL;
void *extra_data_config = NULL;

static bool old_config_freed = false;

/*  P R O T O T Y P E S  ********************************************/
#ifdef MPLS
static void updateMplsHeaders(Packet *, SessionControlBlock *);
#endif
int stream_print_mem_stats(FILE *, char *, PreprocMemInfo *);
static void StreamPolicyInitTcp(struct _SnortConfig *, char *);
static void StreamPolicyInitUdp(struct _SnortConfig *, char *);
static void StreamPolicyInitIcmp(struct _SnortConfig *, char *);
static void StreamPolicyInitIp(struct _SnortConfig *, char *);
static void StreamCleanExit(int, void *);
static void StreamReset(int, void *);
static void StreamResetStats(int, void *);
static int StreamVerifyConfig(struct _SnortConfig *);
static void StreamPrintSessionConfig(SessionConfiguration *);
static void StreamPrintStats(int);
static void DisplayStreamStatistics (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);
static void StreamProcess(Packet *p, void *context);
static inline int IsEligible(Packet *p);
#ifdef TARGET_BASED
static void initServiceFilterStatus(struct _SnortConfig *sc);
#endif

#ifdef SNORT_RELOAD
static void StreamTcpReload(struct _SnortConfig *, char *, void **);
static void StreamUdpReload(struct _SnortConfig *, char *, void **);
static void StreamIcmpReload(struct _SnortConfig *, char *, void **);
static void StreamIpReload(struct _SnortConfig *, char *, void **);
static int StreamReloadVerify(struct _SnortConfig *, void *);
static void * StreamReloadSwap(struct _SnortConfig *, void *);
static void StreamReloadSwapFree(void *);
#endif

/*  S T R E A M  A P I **********************************************/
static int StreamMidStreamDropAlert(void);
static int StreamAlertFlushStream(Packet *p);
static int StreamRequestFlushStream(Packet *p);
static int StreamResponseFlushStream(Packet *p);
static int StreamAddSessionAlert(void *ssnptr, Packet *p, uint32_t gid, uint32_t sid);
static int StreamCheckSessionAlert(void *ssnptr, Packet *p, uint32_t gid, uint32_t sid);
static int StreamUpdateSessionAlert(void *ssnptr, Packet *p, uint32_t gid, uint32_t sid,
        uint32_t event_id, uint32_t event_second);
static char StreamSetReassembly(void *ssnptr, uint8_t flush_policy, char dir, char flags);
static void StreamUpdateDirection( void * scbptr, char dir, sfaddr_t* ip, uint16_t port );
static char StreamGetReassemblyDirection(void *ssnptr);
static char StreamGetReassemblyFlushPolicy(void *ssnptr, char dir);
static char StreamIsStreamSequenced(void *ssnptr, char dir);
static int StreamMissingInReassembled(void *ssnptr, char dir);
static char StreamPacketsMissing(void *ssnptr, char dir);
static void StreamDropPacket( Packet *p );
static int StreamGetRebuiltPackets( Packet *p, PacketIterator callback, void *userdata);
static int StreamGetStreamSegments( Packet *p, StreamSegmentIterator callback, void *userdata);
static uint32_t StreamGetFlushPoint(void *ssnptr, char dir);
static void StreamSetFlushPoint(void *ssnptr, char dir, uint32_t flush_point);
static uint8_t StreamRegisterPAFPort(
        struct _SnortConfig *, tSfPolicyId,
        uint16_t server_port, bool toServer,
        PAF_Callback, bool autoEnable);
static uint8_t StreamRegisterPAFService(
        struct _SnortConfig *, tSfPolicyId,
        uint16_t service, bool toServer,
        PAF_Callback, bool autoEnable);
static void** StreamGetPAFUserData(void* ssnptr, bool to_server, uint8_t id);
static bool StreamIsPafActive(void* ssnptr, bool to_server);
static bool StreamActivatePaf(void* ssnptr, int dir, int16_t service, uint8_t type);

static uint32_t StreamRegisterXtraData(LogFunction );
static uint32_t StreamGetXtraDataMap(LogFunction **);
static void StreamRegisterXtraDataLog(LogExtraData, void * );
static void StreamSetExtraData(void* ssn, Packet*, uint32_t);
static void StreamClearExtraData(void* ssn, Packet*, uint32_t);
static void s5SetPortFilterStatus( struct _SnortConfig *, IpProto protocol, uint16_t port, uint16_t status,
        tSfPolicyId policyId, int parsing );
static void s5UnsetPortFilterStatus( struct _SnortConfig *, IpProto protocol, uint16_t port, uint16_t status,
        tSfPolicyId policyId, int parsing );
#ifdef TARGET_BASED
static void setServiceFilterStatus( struct _SnortConfig *sc, int service, int status, tSfPolicyId policyId, int parsing );
#endif
static void StreamForceSessionExpiration(void *ssnptr);
static void StreamForceDeleteSession(void *ssnptr );
static void registerReassemblyPort( char *network, uint16_t port, int reassembly_direction );
static void unregisterReassemblyPort( char *network, uint16_t port, int reassembly_direction );
static unsigned StreamRegisterHandler(Stream_Callback);
static bool StreamSetHandler(void* ssnptr, unsigned id, Stream_Event);
static uint32_t StreamGetPreprocFlags( void *ssnptr);
static void StreamResetPolicy(void* ssnptr, int dir, uint16_t policy, uint16_t mss);
static void StreamSetSessionDecrypted(void* ssnptr, bool enable);
static bool StreamIsSessionDecrypted(void* ssnptr);
struct _ExpectNode;
static int StreamSetApplicationProtocolIdExpectedPreassignCallbackId( const Packet *ctrlPkt, sfaddr_t* srcIP,
        uint16_t srcPort, sfaddr_t* dstIP, uint16_t dstPort, uint8_t protocol, int16_t protoId,
        uint32_t preprocId, void *protoData, void (*protoDataFreeFn)(void*), unsigned cbId, Stream_Event se,
        struct _ExpectNode** packetExpectedNode);

#if defined(FEAT_OPEN_APPID)
static void SetApplicationId(void* ssnptr, int16_t serviceAppId, int16_t clientAppId,
        int16_t payloadAppId, int16_t miscAppId);
static void GetApplicationId(void* ssnptr, int16_t *serviceAppId, int16_t *clientAppId,
        int16_t *payloadAppId, int16_t *miscAppId);
static int RegisterHttpHeaderCallback (Http_Processor_Callback cb);
#endif /* defined(FEAT_OPEN_APPID) */

static bool serviceEventPublish(unsigned int preprocId, void *ssnptr, ServiceEventType eventType, void * eventData);
static bool serviceEventSubscribe(unsigned int preprocId, ServiceEventType eventType, ServiceEventNotifierFunc cb);
static void StreamRegisterPAFFree(uint8_t id, PAF_Free_Callback cb);
static Packet* getWirePacket();
static uint8_t getFlushPolicyDir();
static bool StreamIsSessionHttp2(void* ssnptr);
static void StreamSetSessionHttp2(void* ssnptr);
static bool StreamShowRebuiltPackets();
static bool StreamIsSessionHttp2Upg(void* ssnptr);
static void StreamSetSessionHttp2Upg(void* ssnptr);
static int RegisterFTPFlushCallback (FTP_Processor_Flush_Callback cb);
static void setFtpFilePosition (void *scbptr,bool flush);
#ifdef HAVE_DAQ_DECRYPTED_SSL
static int StreamSimulateTcpAck(void *ssnptr, uint8_t dir, uint32_t len);
#endif

StreamAPI s5api = {
    /* .version = */ STREAM_API_VERSION5,
    /* .alert_inline_midstream_drops = */ StreamMidStreamDropAlert,
    /* .alert_flush_stream = */ StreamAlertFlushStream,
    /* .request_flush_stream = */ StreamRequestFlushStream,
    /* .response_flush_stream = */ StreamResponseFlushStream,
    /* .traverse_reassembled = */ StreamGetRebuiltPackets,
    /* .traverse_stream_segments = */ StreamGetStreamSegments,
    /* .add_session_alert = */ StreamAddSessionAlert,
    /* .check_session_alerted = */ StreamCheckSessionAlert,
    /* .update_session_alert = */ StreamUpdateSessionAlert,
    /* .set_reassembly = */ StreamSetReassembly,
    /* .update_direction = */ StreamUpdateDirection,
    /* .get_reassembly_direction = */ StreamGetReassemblyDirection,
    /* .get_reassembly_flush_policy = */ StreamGetReassemblyFlushPolicy,
    /* .is_stream_sequenced = */ StreamIsStreamSequenced,
    /* .missing_in_reassembled = */ StreamMissingInReassembled,
    /* .missed_packets = */ StreamPacketsMissing,
    /* .drop_packet = */ StreamDropPacket,
    /* .get_flush_point = */ StreamGetFlushPoint,
    /* .set_flush_point = */ StreamSetFlushPoint,
    /* .register_paf_port = */ StreamRegisterPAFPort,
    /* .get_paf_user_data = */ StreamGetPAFUserData,
    /* .is_paf_active = */ StreamIsPafActive,
    /* .activate_paf = */ StreamActivatePaf,
    /* .set_tcp_syn_session_status = */ s5TcpSetSynSessionStatus,
    /* .unset_tcp_syn_session_status = */ s5TcpUnsetSynSessionStatus,
    /* .reg_xtra_data_cb = */ StreamRegisterXtraData,
    /* .reg_xtra_data_log = */ StreamRegisterXtraDataLog,
    /* .get_xtra_data_map = */ StreamGetXtraDataMap,
    /* .register_paf_service = */ StreamRegisterPAFService,
    /* .set_extra_data = */ StreamSetExtraData,
    /* .clear_extra_data = */ StreamClearExtraData,

    // The methods below may move to Session
    /* .set_port_filter_status = */ s5SetPortFilterStatus,
    /* .unset_port_filter_status = */ s5UnsetPortFilterStatus,
#ifdef TARGET_BASED
    /* .set_service_filter_status = */ setServiceFilterStatus,
#endif
    /* .register_reassembly_port = */ registerReassemblyPort,
    /* .register_reassembly_port = */ unregisterReassemblyPort,
    /* .expire_session = */ StreamForceSessionExpiration,
    /* .force_delete_session = */ StreamForceDeleteSession,    
    /* .register_event_handler = */ StreamRegisterHandler,
    /* .set_event_handler = */ StreamSetHandler,
    /* .set_reset_policy = */ StreamResetPolicy,
    /* .set_session_decrypted = */ StreamSetSessionDecrypted,
    /* .is_session_decrypted = */ StreamIsSessionDecrypted,
    /* .set_application_protocol_id_expected_preassign_callback = */ StreamSetApplicationProtocolIdExpectedPreassignCallbackId,
    /* .print_normalization_stats = */ Stream_PrintNormalizationStats,
    /* .reset_normalization_stats = */ Stream_ResetNormalizationStats,
#if defined(FEAT_OPEN_APPID)
    /* .set_application_id = */ SetApplicationId,
    /* .get_application_id = */ GetApplicationId,
    /* .register_http_header_callback = */ RegisterHttpHeaderCallback,
#endif /* defined(FEAT_OPEN_APPID) */
    /* .service_event_publish */ serviceEventPublish,
    /* .service_event_subscribe */ serviceEventSubscribe,
    /* .register_paf_free */ StreamRegisterPAFFree,
    /* .get_wire_packet */ getWirePacket,
    /* .get_flush_policy_dir */ getFlushPolicyDir,
    /* .is_session_http2 */ StreamIsSessionHttp2,
    /* .set_session_http2 */ StreamSetSessionHttp2,
    /* .is_show_rebuilt_packets_enabled */ StreamShowRebuiltPackets,
    /* .is_session_http2_upg */ StreamIsSessionHttp2Upg,
    /* .set_session_http2_upg */ StreamSetSessionHttp2Upg,
    /* .get_preproc_flags */ StreamGetPreprocFlags,
    /* .register_ftp_flush_cb */ RegisterFTPFlushCallback,
    /* .set_ftp_file_position */ setFtpFilePosition
#ifdef HAVE_DAQ_DECRYPTED_SSL
    ,
    /* .simulate_tcp_ack_in_peer_stream_tracker = */ StreamSimulateTcpAck
#endif
};

void SetupStream6(void)
{
    RegisterMemoryStatsFunction(PP_STREAM, stream_print_mem_stats);
#ifndef SNORT_RELOAD
    RegisterPreprocessor("stream5_tcp", StreamPolicyInitTcp);
    RegisterPreprocessor("stream5_udp", StreamPolicyInitUdp);
    RegisterPreprocessor("stream5_icmp", StreamPolicyInitIcmp);
    RegisterPreprocessor("stream5_ip", StreamPolicyInitIp);
#else
    RegisterPreprocessor("stream5_tcp", StreamPolicyInitTcp, StreamTcpReload,
            StreamReloadVerify, StreamReloadSwap, StreamReloadSwapFree);
    RegisterPreprocessor("stream5_udp", StreamPolicyInitUdp, StreamUdpReload,
            StreamReloadVerify, StreamReloadSwap, StreamReloadSwapFree);
    RegisterPreprocessor("stream5_icmp", StreamPolicyInitIcmp, StreamIcmpReload,
            StreamReloadVerify, StreamReloadSwap, StreamReloadSwapFree);
    RegisterPreprocessor("stream5_ip", StreamPolicyInitIp, StreamIpReload,
            StreamReloadVerify, StreamReloadSwap, StreamReloadSwapFree);
#endif

    // init pointer to stream api dispatch table...
    stream_api = &s5api;
	
    /* Registering for SFR CLI */
	ControlSocketRegisterHandler(CS_TYPE_STREAM_STATS, NULL, NULL, &DisplayStreamStatistics);
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Stream preprocessor setup complete.\n"););
}

// Initialize the configuration object for a stream preprocessor policy. If this is the first stream configuration
// being parsed for this NAP policy then allocate the config context object that holds the config settings for all
// the possible stream protocols. This function is called before each protocol specific configuration string is
// processed for each NAP policy defined.
static StreamConfig *initStreamPolicyConfig( struct _SnortConfig *sc, bool reload_config )
{
    tSfPolicyId policy_id = getParserPolicy( sc );
    StreamConfig *pCurrentPolicyConfig = NULL;

    if( stream_parsing_config == NULL )
    {
        // we are parsing the first stream conf file, create a context and do all stream
        // one time initialization functions.
        //
        stream_parsing_config = sfPolicyConfigCreate();

        if( !reload_config )
        {
#ifdef PERF_PROFILING
            RegisterPreprocessorProfile( "s5", &s5PerfStats, 0, &totalPerfStats , NULL);
            RegisterPreprocessorProfile( "s5tcp", &s5TcpPerfStats, 1, &s5PerfStats , NULL);
            RegisterPreprocessorProfile( "s5udp", &s5UdpPerfStats, 1, &s5PerfStats , NULL);
            RegisterPreprocessorProfile( "s5icmp", &s5IcmpPerfStats, 1, &s5PerfStats , NULL);
            RegisterPreprocessorProfile( "s5ip", &s5IpPerfStats, 1, &s5PerfStats , NULL);
#endif

            AddFuncToPreprocCleanExitList( StreamCleanExit, NULL, PP_STREAM6_PRIORITY, PP_STREAM );
            AddFuncToPreprocResetList( StreamReset, NULL, PP_STREAM6_PRIORITY, PP_STREAM );
            AddFuncToPreprocResetStatsList( StreamResetStats, NULL, PP_STREAM6_PRIORITY, PP_STREAM );
            AddFuncToConfigCheckList( sc, StreamVerifyConfig );
            RegisterPreprocStats( "stream5", StreamPrintStats );
        }
        else
            old_config_freed = false;

    }

    // set this policy id as current and get pointer to the struct of pointers to the
    // protocol specific configuration pointers for this policy...
    // if this pointer is NULL then this is the first stream protocol conf file we are
    // parsing for this policy, so allocate required memory
    sfPolicyUserPolicySet( stream_parsing_config, policy_id );
    pCurrentPolicyConfig = ( StreamConfig * ) sfPolicyUserDataGetCurrent( stream_parsing_config );
    if( pCurrentPolicyConfig == NULL )
    {
        pCurrentPolicyConfig = ( StreamConfig * ) SnortPreprocAlloc( 1,
                                            sizeof( StreamConfig ), PP_STREAM,
                                            PP_MEM_CATEGORY_CONFIG );
        sfPolicyUserDataSetCurrent( stream_parsing_config, pCurrentPolicyConfig );
        // get pointer to the session configuration...if it's NULL bad news, session not
        // configured so Fatal Error...
        pCurrentPolicyConfig->session_config = getSessionConfiguration( reload_config );
        if( pCurrentPolicyConfig->session_config == NULL )
        {
            FatalError( "%s(%d) - Session Must Be Configured Before Stream!\n", file_name, file_line );
        }

        // stream registers to run for all ports
        session_api->enable_preproc_all_ports( sc, PP_STREAM, PROTO_BIT__ALL );
        pCurrentPolicyConfig->verified = false;
        pCurrentPolicyConfig->swapped = false;
        pCurrentPolicyConfig->reload_config = reload_config;
        StreamPrintSessionConfig( pCurrentPolicyConfig->session_config );
    }

    return pCurrentPolicyConfig;
}

// return pointer to configuration context object for all stream polices. If parsing is
// true return pointer to current parsing context object (NULL if parsing not in progress)
// otherwise pointer to the current active config context object
static inline tSfPolicyUserContextId getStreamConfigContext( bool parsing )
{
    if( parsing )
        return stream_parsing_config;
    else
        return stream_online_config;
}

// return pointer to Stream configuration for the specified policy.  If parsing is
// true return pointer to config struct the policy is being parsed into, otherwise pointer
// to the currently active config
StreamConfig *getStreamPolicyConfig( tSfPolicyId policy_id, bool parsing )
{
    tSfPolicyUserContextId ctx;

    if( parsing )
        ctx = ( stream_parsing_config != NULL ) ? stream_parsing_config : stream_online_config;
    else
        ctx = stream_online_config;

    if( ctx != NULL )
        return ( StreamConfig * ) sfPolicyUserDataGet( ctx, policy_id );
    else
        return NULL;
}


static void StreamPrintSessionConfig( SessionConfiguration *config )
{
    LogMessage("Stream global config:\n");
    LogMessage("    Track TCP sessions: %s\n", config->track_tcp_sessions == STREAM_TRACK_YES ?
            "ACTIVE" : "INACTIVE");
    if( config->track_tcp_sessions == STREAM_TRACK_YES )
    {
        LogMessage("    Max TCP sessions: %u\n", config->max_tcp_sessions);
        LogMessage("    TCP cache pruning timeout: %u seconds\n", config->tcp_cache_pruning_timeout);
        LogMessage("    TCP cache nominal timeout: %u seconds\n", config->tcp_cache_nominal_timeout);
    }

    LogMessage("    Memcap (for reassembly packet storage): %d\n", config->memcap);
    LogMessage("    Track UDP sessions: %s\n", config->track_udp_sessions == STREAM_TRACK_YES ?
            "ACTIVE" : "INACTIVE");
    if( config->track_udp_sessions == STREAM_TRACK_YES )
    {
        LogMessage("    Max UDP sessions: %u\n", config->max_udp_sessions);
        LogMessage("    UDP cache pruning timeout: %u seconds\n", config->udp_cache_pruning_timeout);
        LogMessage("    UDP cache nominal timeout: %u seconds\n", config->udp_cache_nominal_timeout);
    }

    LogMessage("    Track ICMP sessions: %s\n", config->track_icmp_sessions == STREAM_TRACK_YES ?
            "ACTIVE" : "INACTIVE");
    if( config->track_icmp_sessions == STREAM_TRACK_YES )
        LogMessage("    Max ICMP sessions: %u\n", config->max_icmp_sessions);

    LogMessage("    Track IP sessions: %s\n", config->track_ip_sessions == STREAM_TRACK_YES ?
            "ACTIVE" : "INACTIVE");
    if( config->track_ip_sessions == STREAM_TRACK_YES )
        LogMessage("    Max IP sessions: %u\n", config->max_ip_sessions);
    if( config->prune_log_max )
        LogMessage("    Log info if session memory consumption exceeds %d\n", config->prune_log_max);
#ifdef ACTIVE_RESPONSE
    LogMessage("    Send up to %d active responses\n", config->max_active_responses);

    if( config->max_active_responses > 1 )
    {
        LogMessage("    Wait at least %d seconds between responses\n",
                config->min_response_seconds);
    }
#endif
    LogMessage("    Protocol Aware Flushing: %s\n", ScPafEnabled() ? "ACTIVE" : "INACTIVE");
    LogMessage("        Maximum Flush Point: %u\n", ScPafMax());
#ifdef ENABLE_HA
    LogMessage("    High Availability: %s\n", config->enable_ha ? "ENABLED" : "DISABLED");
#endif

#ifdef REG_TEST
    LogMessage("    Session Control Block Size: %lu\n", (long unsigned int)sizeof(SessionControlBlock));
#endif

}

static void StreamPolicyInitTcp( struct _SnortConfig *sc, char *args )
{
    StreamConfig *config = NULL;

    config = initStreamPolicyConfig( sc, false );
    if ( !config->session_config->track_tcp_sessions )
        return;

    if( config->tcp_config == NULL )
    {
        config->tcp_config = ( StreamTcpConfig * ) SnortPreprocAlloc( 1,
                                            sizeof( StreamTcpConfig ), PP_STREAM,
                                            PP_MEM_CATEGORY_CONFIG );
        StreamInitTcp( );
        StreamTcpInitFlushPoints( );
        StreamTcpRegisterRuleOptions( sc );
        AddFuncToPreprocPostConfigList( sc, StreamPostConfigTcp, config->tcp_config );
    }

    /* Call the protocol specific initializer */
    StreamTcpPolicyInit( sc, config->tcp_config, args );
}

static void StreamPolicyInitUdp( struct _SnortConfig *sc, char *args )
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, false );
    if( !config->session_config->track_udp_sessions )
        return;

    if( config->udp_config == NULL )
    {
        config->udp_config = ( StreamUdpConfig * ) SnortPreprocAlloc(1, 
                                            sizeof( StreamUdpConfig ), PP_STREAM,
                                            PP_MEM_CATEGORY_CONFIG);
        StreamInitUdp( );
    }

    /* Call the protocol specific initializer */
    StreamUdpPolicyInit( config->udp_config, args );
}

static void StreamPolicyInitIcmp( struct _SnortConfig *sc, char *args )
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, false );
    if( !config->session_config->track_icmp_sessions )
        return;

    if( config->icmp_config == NULL )
    {
        config->icmp_config = ( StreamIcmpConfig * ) SnortPreprocAlloc( 1, 
                                            sizeof( StreamIcmpConfig ), PP_STREAM,
                                            PP_MEM_CATEGORY_CONFIG );
        StreamInitIcmp( );
    }

    /* Call the protocol specific initializer */
    StreamIcmpPolicyInit( config->icmp_config, args );
}

static void StreamPolicyInitIp( struct _SnortConfig *sc, char *args )
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, false );
    if( !config->session_config->track_ip_sessions )
        return;

    if( config->ip_config == NULL )
    {
        config->ip_config = ( StreamIpConfig * ) SnortPreprocAlloc( 1, 
                                            sizeof( StreamIpConfig ), PP_STREAM,
                                            PP_MEM_CATEGORY_CONFIG );
        StreamInitIp( );
    }

    /* Call the protocol specific initializer */
    StreamIpPolicyInit( config->ip_config, args );
}

int StreamVerifyProtocolConfigs( struct _SnortConfig *sc, StreamConfig *s5c,
        tSfPolicyId policyId, int *proto_flags )
{
    int tcpNotConfigured = 0;
    int udpNotConfigured = 0;
    int icmpNotConfigured = 0;
    int ipNotConfigured = 0;

    if( s5c->tcp_config )
    {
        tcpNotConfigured = StreamVerifyTcpConfig( sc, s5c->tcp_config, policyId );
        if( tcpNotConfigured )
            WarningMessage("WARNING: Stream TCP misconfigured.\n");
        else
            *proto_flags |= PROTO_BIT__TCP;
    }

    if( s5c->udp_config )
    {
        udpNotConfigured = StreamVerifyUdpConfig( sc, s5c->udp_config, policyId );
        if( udpNotConfigured )
            WarningMessage("WARNING: Stream UDP misconfigured.\n");
        else
            *proto_flags |= PROTO_BIT__UDP;
    }

    if( s5c->icmp_config )
    {
        icmpNotConfigured = StreamVerifyIcmpConfig( s5c->icmp_config, policyId );
        if( icmpNotConfigured )
            WarningMessage("WARNING: Stream ICMP misconfigured.\n");
        else
            *proto_flags |= PROTO_BIT__ICMP;
    }

    if( s5c->ip_config )
    {
        ipNotConfigured = StreamVerifyIpConfig( s5c->ip_config, policyId );
        if( ipNotConfigured )
            WarningMessage("WARNING: Stream IP misconfigured.\n");
        else
            *proto_flags |= PROTO_BIT__IP;
    }

    return( tcpNotConfigured || udpNotConfigured || icmpNotConfigured || ipNotConfigured );
}

static int StreamVerifyConfigPolicy( struct _SnortConfig *sc, tSfPolicyUserContextId config,
                                     tSfPolicyId policyId, void* pData )
{
    int configNotValid = 0;
    int proto_flags = 0;
    tSfPolicyId tmp_policy_id = getParserPolicy( sc );
    StreamConfig *stream_conf = ( StreamConfig * ) pData;

    if( stream_conf->verified )
        return 0;

    // verify that session is configured.
    if ( stream_conf->session_config == NULL )
    {
        FatalError("%s(%d) No Stream session configuration...exiting.\n", __FILE__, __LINE__);
    }

    configNotValid = StreamVerifyProtocolConfigs( sc, stream_conf, policyId, &proto_flags );
    if ( configNotValid )
    {
        FatalError("%s(%d) Stream not properly configured... exiting\n", __FILE__, __LINE__);
    }

    stream_conf->verified = true;
    setParserPolicy( sc, policyId );
    AddFuncToPreprocList( sc, StreamProcess, PP_STREAM6_PRIORITY, PP_STREAM, proto_flags );
    setParserPolicy( sc, tmp_policy_id );

    return 0;
}

static int StreamVerifyConfig(struct _SnortConfig *sc)
{
    int rval = sfPolicyUserDataIterate( sc, stream_parsing_config, StreamVerifyConfigPolicy );
    if( rval )
        return rval;

    if (!stream_online_config)
    {
        stream_online_config = stream_parsing_config;
        stream_parsing_config = NULL;
    }

#ifdef TARGET_BASED
    initServiceFilterStatus( sc );
#endif

    return 0;
}

static void StreamReset(int signal, void *foo)
{
    if (stream_online_config == NULL)
        return;

    StreamResetTcp();
    StreamResetUdp();
    StreamResetIcmp();
    StreamResetIp();
}

static void StreamResetStats(int signal, void *foo)
{
    memset(&s5stats, 0, sizeof(s5stats));
    StreamResetTcpPrunes();
    StreamResetUdpPrunes();
    StreamResetIcmpPrunes();
    StreamResetIpPrunes();
}

static void StreamCleanExit(int signal, void *foo)
{
#ifdef ENABLE_QUICK_EXIT
    LogMessage("Snort quick exit enabled\n");
    return;
#else
    /* Protocol specific cleanup actions */
    StreamCleanTcp();
    StreamCleanUdp();
    StreamCleanIcmp();
    StreamCleanIp();

    StreamFreeConfigs(stream_online_config);
    stream_online_config = NULL;
#endif
}

static void DisplayStreamStatistics (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int len = 0;
    int total_sessions = s5stats.total_tcp_sessions + s5stats.total_udp_sessions +
        s5stats.total_icmp_sessions + s5stats.total_ip_sessions;

    if (total_sessions) {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "Stream statistics:\n"
                "            Total sessions: %u\n"
                "              TCP sessions: %u\n"
                "              UDP sessions: %u\n"
                "             ICMP sessions: %u\n"
                "               IP sessions: %u\n"
                "                TCP Prunes: %u\n"
                "                UDP Prunes: %u\n"
                "               ICMP Prunes: %u\n"
                "                 IP Prunes: %u\n"
                "TCP StreamTrackers Created: %u\n"
                "TCP StreamTrackers Deleted: %u\n"
                "              TCP Timeouts: %u\n"
                "              TCP Overlaps: %u\n"
                "       TCP Segments Queued: %u\n"
                "     TCP Segments Released: %u\n"
                "       TCP Rebuilt Packets: %u\n"
                "         TCP Segments Used: %u\n"
                "              TCP Discards: %u\n"
                "                  TCP Gaps: %u\n"
                "      UDP Sessions Created: %u\n"
                "      UDP Sessions Deleted: %u\n"
                "              UDP Timeouts: %u\n"
                "              UDP Discards: %u\n"
                "                    Events: %u\n"
                "           Internal Events: %u\n"
                "           TCP Port Filter\n"
                "                  Filtered: %u\n"
                "                 Inspected: %u\n"
                "                   Tracked: %u\n"
                "           UDP Port Filter\n"
                "                  Filtered: %u\n"
                "                 Inspected: %u\n"
                "                   Tracked: %u\n"
                , total_sessions
                , s5stats.total_tcp_sessions
                , s5stats.total_udp_sessions
                , s5stats.total_icmp_sessions
                , s5stats.total_ip_sessions
                , StreamGetTcpPrunes()
                , StreamGetUdpPrunes()
                , StreamGetIcmpPrunes()
                , StreamGetIpPrunes()
                , s5stats.tcp_streamtrackers_created
                , s5stats.tcp_streamtrackers_released
                , s5stats.tcp_timeouts
                , s5stats.tcp_overlaps
                , s5stats.tcp_streamsegs_created
                , s5stats.tcp_streamsegs_released
                , s5stats.tcp_rebuilt_packets
                , s5stats.tcp_rebuilt_seqs_used
                , s5stats.tcp_discards
                , s5stats.tcp_gaps
                , s5stats.udp_sessions_created
                , s5stats.udp_sessions_released
                , s5stats.udp_timeouts
                , s5stats.udp_discards
                , s5stats.events
                , s5stats.internalEvents
                , s5stats.tcp_port_filter.filtered
                , s5stats.tcp_port_filter.inspected
                , s5stats.tcp_port_filter.session_tracked
                , s5stats.udp_port_filter.filtered
                , s5stats.udp_port_filter.inspected
                , s5stats.udp_port_filter.session_tracked);
    } else {
        len  = snprintf(buffer, CS_STATS_BUF_SIZE, "Stream statistics not available\n Total sessions: %u", total_sessions);
    }

    if (-1 == f(te, (const uint8_t *)buffer, len)) {
        LogMessage("Unable to send data to the frontend\n");
    }
}

static void StreamPrintStats(int exiting)
{
    LogMessage("Stream statistics:\n");
    LogMessage("            Total sessions: %u\n", s5stats.total_tcp_sessions +
            s5stats.total_udp_sessions +
            s5stats.total_icmp_sessions +
            s5stats.total_ip_sessions);

    LogMessage("              TCP sessions: %u\n", s5stats.total_tcp_sessions);
    LogMessage("       Active TCP sessions: %u\n", s5stats.active_tcp_sessions);
    LogMessage("  Non mempool TCP sess mem: %u\n", session_mem_in_use);
    LogMessage("          TCP mempool used: %"PRIu64"\n", get_tcp_used_mempool());
    LogMessage("              UDP sessions: %u\n", s5stats.total_udp_sessions);
    LogMessage("       Active UDP sessions: %u\n", s5stats.active_udp_sessions);
    LogMessage("          UDP mempool used: %"PRIu64"\n", get_udp_used_mempool());
    LogMessage("             ICMP sessions: %u\n", s5stats.total_icmp_sessions);
    LogMessage("      Active ICMP sessions: %u\n", s5stats.active_icmp_sessions);
    LogMessage("         ICMP mempool used: %"PRIu64"\n", get_icmp_used_mempool());
    LogMessage("               IP sessions: %u\n", s5stats.total_ip_sessions);
    LogMessage("        Active IP sessions: %u\n", s5stats.active_ip_sessions);
    LogMessage("           IP mempool used: %"PRIu64"\n", get_ip_used_mempool());

    LogMessage("                TCP Prunes: %u\n", StreamGetTcpPrunes());
    LogMessage("                UDP Prunes: %u\n", StreamGetUdpPrunes());
    LogMessage("               ICMP Prunes: %u\n", StreamGetIcmpPrunes());
    LogMessage("                 IP Prunes: %u\n", StreamGetIpPrunes());
    LogMessage("TCP StreamTrackers Created: %u\n", s5stats.tcp_streamtrackers_created);
    LogMessage("TCP StreamTrackers Deleted: %u\n", s5stats.tcp_streamtrackers_released);
    LogMessage("              TCP Timeouts: %u\n", s5stats.tcp_timeouts);
    LogMessage("              TCP Overlaps: %u\n", s5stats.tcp_overlaps);
    LogMessage("       TCP Segments Queued: %u\n", s5stats.tcp_streamsegs_created);
    LogMessage("     TCP Segments Released: %u\n", s5stats.tcp_streamsegs_released);
    LogMessage("       TCP Rebuilt Packets: %u\n", s5stats.tcp_rebuilt_packets);
    LogMessage("         TCP Segments Used: %u\n", s5stats.tcp_rebuilt_seqs_used);
    LogMessage("              TCP Discards: %u\n", s5stats.tcp_discards);
    LogMessage("                  TCP Gaps: %u\n", s5stats.tcp_gaps);
    LogMessage("      UDP Sessions Created: %u\n", s5stats.udp_sessions_created);
    LogMessage("      UDP Sessions Deleted: %u\n", s5stats.udp_sessions_released);
    LogMessage("              UDP Timeouts: %u\n", s5stats.udp_timeouts);
    LogMessage("              UDP Discards: %u\n", s5stats.udp_discards);
    LogMessage("     ICMP Dest unreachable: %u\n", s5stats.icmp_unreachable);
    LogMessage(" ICMP Fragmentation needed: %u\n", s5stats.icmp_unreachable_code4);
    LogMessage("                    Events: %u\n", s5stats.events);
    LogMessage("           Internal Events: %u\n", s5stats.internalEvents);
    LogMessage("           TCP Port Filter\n");
    LogMessage("                  Filtered: %u\n", s5stats.tcp_port_filter.filtered);
    LogMessage("                 Inspected: %u\n", s5stats.tcp_port_filter.inspected);
    LogMessage("                   Tracked: %u\n", s5stats.tcp_port_filter.session_tracked);
    LogMessage("           UDP Port Filter\n");
    LogMessage("                  Filtered: %u\n", s5stats.udp_port_filter.filtered);
    LogMessage("                 Inspected: %u\n", s5stats.udp_port_filter.inspected);
    LogMessage("                   Tracked: %u\n", s5stats.udp_port_filter.session_tracked);

    // TBD-EDM move to session will need to fix reg tests?
#ifdef ENABLE_HA
    SessionPrintHAStats();
#endif

}

#define GET_MEMPOOL(protocol) \
    proto_session_caches[protocol] ? \
        proto_session_caches[protocol]->protocol_session_pool : NULL

size_t stream_get_free_mempool(MemPool *mempool)
{
    if (mempool)
        return mempool->max_memory - mempool->used_memory;

    return 0;
}

size_t stream_get_used_mempool(MemPool *mempool)
{
    if (mempool)
        return mempool->used_memory;

    return 0;
}

size_t stream_get_max_mempool(MemPool *mempool)
{   
    if (mempool)
        return mempool->max_memory;
    
    return 0;
}

int stream_print_mem_stats(FILE *fd, char* buffer, PreprocMemInfo *meminfo)
{
    time_t curr_time;
    int len = 0;
    uint32_t total_sessions = s5stats.total_tcp_sessions + s5stats.total_udp_sessions +
          s5stats.total_icmp_sessions + s5stats.total_ip_sessions;

    if (fd)
    {
        size_t total_heap_memory = meminfo[PP_MEM_CATEGORY_SESSION].used_memory
                              + meminfo[PP_MEM_CATEGORY_CONFIG].used_memory;
        size_t total_pool_memory = stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_TCP))
                 + stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_UDP))
                 + stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP))
                 + stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_IP))
                 + stream_get_max_mempool(&sessionFlowMempool);
           
        len = fprintf(fd, ",%u,%u,%u"
               ",%u,%u,%u"
               ",%u,%u,%u"
               ",%lu,%u,%u"
               ",%lu,%u,%u,%lu,%lu"
               , total_sessions
               , s5stats.total_tcp_sessions
               , s5stats.active_tcp_sessions
               , s5stats.total_udp_sessions
               , s5stats.active_udp_sessions
               , s5stats.total_icmp_sessions
               , s5stats.active_icmp_sessions
               , s5stats.total_ip_sessions
               , s5stats.active_ip_sessions
               , meminfo[PP_MEM_CATEGORY_SESSION].used_memory
               , meminfo[PP_MEM_CATEGORY_SESSION].num_of_alloc
               , meminfo[PP_MEM_CATEGORY_SESSION].num_of_free
               , meminfo[PP_MEM_CATEGORY_CONFIG].used_memory
               , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_alloc
               , meminfo[PP_MEM_CATEGORY_CONFIG].num_of_free
               , total_pool_memory
               , total_pool_memory + total_heap_memory);

        return len;
    }

    curr_time = time(NULL);

    if (buffer)
    {
        len = snprintf(buffer, CS_STATS_BUF_SIZE, "\n\nMemory Statistics of Stream on: %s\n"
             "Stream Session Statistics:\n"
             "            Total sessions: %u\n"
             "              TCP sessions: %u\n"
             "       Active TCP sessions: %u\n"
             "              UDP sessions: %u\n"
             "       Active UDP sessions: %u\n"
             "             ICMP sessions: %u\n"
             "      Active ICMP sessions: %u\n"
             "               IP sessions: %u\n"
             "        Active IP sessions: %u\n"
             "\n   TCP Memory Pool:\n"
             "        Free Memory:        %14zu bytes\n"
             "        Used Memory:        %14zu bytes\n"
             "        Max Memory :        %14zu bytes\n"
             "\n   UDP Memory Pool:\n"
             "        Free Memory:        %14zu bytes\n"
             "        Used Memory:        %14zu bytes\n"
             "        Max Memory :        %14zu bytes\n"
             "\n   ICMP Memory Pool:\n"
             "        Free Memory:        %14zu bytes\n"
             "        Used Memory:        %14zu bytes\n"
             "        Max Memory :        %14zu bytes\n"
             "\n   IP Memory Pool:\n"
             "        Free Memory:        %14zu bytes\n"
             "        Used Memory:        %14zu bytes\n"
             "        Max Memory :        %14zu bytes\n"
             "\n   Session Flow Memory Pool:\n"
             "        Free Memory:        %14zu bytes\n"
             "        Used Memory:        %14zu bytes\n"
             "        Max Memory :        %14zu bytes\n"
             , ctime(&curr_time)
             , total_sessions
             , s5stats.total_tcp_sessions
             , s5stats.active_tcp_sessions
             , s5stats.total_udp_sessions
             , s5stats.active_udp_sessions
             , s5stats.total_icmp_sessions
             , s5stats.active_icmp_sessions
             , s5stats.total_ip_sessions
             , s5stats.active_ip_sessions
             , stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_TCP)) 
             , stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_TCP))
             , stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_TCP))
             , stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_UDP))                                 
             , stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_UDP))
             , stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_UDP))
             , stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP))                                 
             , stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP))
             , stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP))
             , stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_IP))                                 
             , stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_IP))
             , stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_IP))
             , stream_get_free_mempool(&sessionFlowMempool)
             , stream_get_used_mempool(&sessionFlowMempool)
             , stream_get_max_mempool(&sessionFlowMempool));
    } else {
        LogMessage("\n");
        LogMessage("Memory Statistics of Stream on: %s\n",ctime(&curr_time));
        LogMessage("Stream Session Statistics:\n");
        LogMessage("            Total sessions: %u\n", total_sessions);
        LogMessage("              TCP sessions: %u\n", s5stats.total_tcp_sessions);
        LogMessage("       Active TCP sessions: %u\n", s5stats.active_tcp_sessions);
        LogMessage("              UDP sessions: %u\n", s5stats.total_udp_sessions);
        LogMessage("       Active UDP sessions: %u\n", s5stats.active_udp_sessions);
        LogMessage("             ICMP sessions: %u\n", s5stats.total_icmp_sessions);
        LogMessage("      Active ICMP sessions: %u\n", s5stats.active_icmp_sessions);
        LogMessage("               IP sessions: %u\n", s5stats.total_ip_sessions);
        LogMessage("        Active IP sessions: %u\n", s5stats.active_ip_sessions);
        LogMessage("   TCP Memory Pool:\n");
        LogMessage("        Free Memory:        %14zu bytes\n",
                   stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_TCP)));
        LogMessage("        Used Memory:        %14zu bytes\n",
                   stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_TCP)));
        LogMessage("        Max Memory :        %14zu bytes\n",
                   stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_TCP)));
        LogMessage("   UDP Memory Pool:\n");
        LogMessage("        Free Memory:        %14zu bytes\n",
                   stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_UDP)));
        LogMessage("        Used Memory:        %14zu bytes\n",
                   stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_UDP)));
        LogMessage("        Max Memory :        %14zu bytes\n",
                   stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_UDP)));
        LogMessage("   ICMP Memory Pool:\n");
        LogMessage("        Free Memory:        %14zu bytes\n",
                   stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP)));
        LogMessage("        Used Memory:        %14zu bytes\n",
                   stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP)));
        LogMessage("        Max Memory :        %14zu bytes\n",
                   stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_ICMP)));
        LogMessage("   IP Memory Pool:\n");
        LogMessage("        Free Memory:        %14zu bytes\n",
                   stream_get_free_mempool(GET_MEMPOOL(SESSION_PROTO_IP)));
        LogMessage("        Used Memory:        %14zu bytes\n",
                   stream_get_used_mempool(GET_MEMPOOL(SESSION_PROTO_IP)));
        LogMessage("        Max Memory :        %14zu bytes\n",
                   stream_get_max_mempool(GET_MEMPOOL(SESSION_PROTO_IP)));
        LogMessage("   Session Flow Memory Pool:\n");
        LogMessage("        Free Memory:        %14zu bytes\n",
                   stream_get_free_mempool(&sessionFlowMempool));
        LogMessage("        Used Memory:        %14zu bytes\n",
                   stream_get_used_mempool(&sessionFlowMempool));
        LogMessage("        Max Memory :        %14zu bytes\n",
                   stream_get_max_mempool(&sessionFlowMempool));
    }

    return len;
}

static void checkOnewayStatus( uint32_t protocol, SessionControlBlock *scb )
{
    if( scb->in_oneway_list && scb->session_established )
        session_api->remove_session_from_oneway_list( protocol, scb );
}

static void updateMplsHeaders(Packet *p, SessionControlBlock *scb )
{
    uint8_t layerIndex;
    uint32_t direction = session_api->get_packet_direction(p);

    if(direction == PKT_FROM_CLIENT && !(scb->clientMplsHeader->start))
    {
        for(layerIndex=0; layerIndex < p->next_layer; layerIndex++)
        {
             if( p->layers[layerIndex].proto == PROTO_MPLS && p->layers[layerIndex].start != NULL )
             {
                   scb->clientMplsHeader->length = p->layers[layerIndex].length;
                   scb->clientMplsHeader->start  = (uint8_t*)SnortPreprocAlloc(1, scb->clientMplsHeader->length,
                                                                    PP_STREAM, PP_MEM_CATEGORY_SESSION);
                   memcpy(scb->clientMplsHeader->start,p->layers[layerIndex].start,scb->clientMplsHeader->length);
                   break;
             }
        }
    }
    else if ( direction == PKT_FROM_SERVER && !(scb->serverMplsHeader->start))
    {
        for(layerIndex=0; layerIndex < p->next_layer; layerIndex++)
        {
             if( p->layers[layerIndex].proto == PROTO_MPLS && p->layers[layerIndex].start != NULL )
             {
                   scb->serverMplsHeader->length = p->layers[layerIndex].length;
                   scb->serverMplsHeader->start  = (uint8_t*)SnortPreprocAlloc(1, scb->serverMplsHeader->length,
                                                                    PP_STREAM, 0);
                   memcpy(scb->serverMplsHeader->start,p->layers[layerIndex].start, scb->serverMplsHeader->length);
                   break;
             }
        }
    }
    else
        return;
}


/*
 * MAIN ENTRY POINT
 */
void StreamProcess(Packet *p, void *context)
{
    SessionKey key;
    SessionControlBlock *scb;
    PROFILE_VARS;

    if (!firstPacketTime)
        firstPacketTime = p->pkth->ts.tv_sec;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"););
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, "In Stream!\n"););

    // get the session and if NULL, bail we can't do anything with that...
    scb = p->ssnptr;
    if( scb == NULL )
    {
        DEBUG_WRAP( DebugMessage( DEBUG_STREAM_STATE,
                   "Stream Processing called with NULL pointer to session control block\n" ) );
        return;
    }

    stream_session_config = scb->session_config;
    if( scb->stream_config_stale || scb->stream_config == NULL )
    {
        scb->stream_config = sfPolicyUserDataGet( stream_online_config, getNapRuntimePolicy() );
        if( scb->stream_config == NULL )
        {
            ErrorMessage("Stream Configuration is NULL, Stream Packet Processing Terminated.\n");
            return;
        }
        else
        {
            scb->proto_policy = NULL;
            scb->stream_config_stale = false;
        }
    }

    if(!IsEligible(p))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, "Is not eligible!\n"););
        return;
    }

#ifdef MPLS
   if(scb->clientMplsHeader != NULL && scb->serverMplsHeader != NULL )
   {
        if(!(scb->clientMplsHeader->start) || !(scb->serverMplsHeader->start))
            updateMplsHeaders(p,scb);
   }
#endif

    PREPROC_PROFILE_START(s5PerfStats);
     /* Call individual TCP/UDP/ICMP/IP processing, per GET_IPH_PROTO(p) */
    switch( GET_IPH_PROTO( p ) )
    {
        case IPPROTO_TCP:
            if( session_api->protocol_tracking_enabled( SESSION_PROTO_TCP ) )
            {
                StreamProcessTcp( p, scb, scb->proto_policy, &key );
                checkOnewayStatus( SESSION_PROTO_TCP, scb );
            }
            break;

        case IPPROTO_UDP:
            if (session_api->protocol_tracking_enabled( SESSION_PROTO_UDP ) )
            {
                StreamProcessUdp(p, scb, scb->proto_policy, &key);
                checkOnewayStatus( SESSION_PROTO_UDP, scb );
            }
            break;

        case IPPROTO_ICMP:
            if (session_api->protocol_tracking_enabled( SESSION_PROTO_ICMP ) )
            {
                StreamProcessIcmp(p);
                checkOnewayStatus( SESSION_PROTO_ICMP, scb );
                break;
            }
            // fall thru ...

        default:
            if (session_api->protocol_tracking_enabled( SESSION_PROTO_IP ) )
            {
                StreamProcessIp(p, scb, &key);
                checkOnewayStatus( SESSION_PROTO_IP, scb );
            }
            break;
    }

    PREPROC_PROFILE_END(s5PerfStats);
}

static inline int IsEligible(Packet *p)
{
    if ((p->frag_flag) || (p->error_flags & PKT_ERR_CKSUM_IP))
        return 0;

    if (p->packet_flags & PKT_REBUILT_STREAM)
        return 0;

    if (!IPH_IS_VALID(p))
        return 0;

    switch(GET_IPH_PROTO(p))
    {
        case IPPROTO_TCP:
            {
                if(p->tcph == NULL)
                    return 0;

                if (p->error_flags & PKT_ERR_CKSUM_TCP)
                    return 0;
            }
            break;
        case IPPROTO_UDP:
            {
                if(p->udph == NULL)
                    return 0;

                if (p->error_flags & PKT_ERR_CKSUM_UDP)
                    return 0;
            }
            break;
        case IPPROTO_ICMP:
        case IPPROTO_ICMPV6:
            {
                if(p->icmph == NULL)
                    return 0;

                if (p->error_flags & PKT_ERR_CKSUM_ICMP)
                    return 0;
            }
            break;
        default:
            if(p->iph == NULL)
                return 0;
            break;
    }

    return 1;
}

/*************************** API Implementations *******************/

static int StreamMidStreamDropAlert(void)
{
    StreamConfig *config = sfPolicyUserDataGet(stream_online_config, getNapRuntimePolicy());

    if (config == NULL)
        return 1;

    return (config->session_config->flags &
            STREAM_CONFIG_MIDSTREAM_DROP_NOALERT) ? 0 : 1;
}

static inline bool StreamOkToFlush(Packet *p)
{
    SessionControlBlock *ssn;

    if ((p == NULL) || (p->ssnptr == NULL))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Don't flush NULL packet or session\n"););
        return false;
    }

    ssn = p->ssnptr;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return false;

    if ((ssn->protocol != IPPROTO_TCP) ||
            (p->packet_flags & PKT_REBUILT_STREAM))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Don't flush on rebuilt packets\n"););
        return false;
    }
    return true;
}

static int StreamAlertFlushStream(Packet *p)
{

    if (!StreamOkToFlush(p))
        return 0;

    if (!(stream_session_config->flags & STREAM_CONFIG_FLUSH_ON_ALERT))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Don't flush on alert from individual packet\n"););
        return 0;
    }

    /* Flush the listener queue -- this is the same side that
     * the packet gets inserted into */
    StreamFlushListener(p, p->ssnptr);

    return 0;
}

static int StreamRequestFlushStream(Packet *p)
{
    if (!StreamOkToFlush(p))
        return 0;

    /* Flush the talker queue -- this is the opposite side that
     * the packet gets inserted into */
    StreamFlushListener(p, p->ssnptr);

    return 0;
}

static int StreamResponseFlushStream(Packet *p)
{
    if (!StreamOkToFlush(p))
            return 0;

    /* Flush the talker queue -- this is the opposite side that
     * the packet gets inserted into */
    StreamFlushTalker(p, p->ssnptr);

    return 0;
}

static int StreamAddSessionAlert(
        void *ssnptr,
        Packet *p,
        uint32_t gid,
        uint32_t sid)
{
    SessionControlBlock *ssn;

    if ( !ssnptr )
        return 0;

    ssn = (SessionControlBlock *)ssnptr;
    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 0;

    /* Don't need to do this for other protos because they don't
       do any reassembly. */
    if ( GET_IPH_PROTO(p) != IPPROTO_TCP )
        return 0;

    return StreamAddSessionAlertTcp(ssn, p, gid, sid);
}

/* return non-zero if gid/sid have already been seen */
static int StreamCheckSessionAlert(
        void *ssnptr,
        Packet *p,
        uint32_t gid,
        uint32_t sid)
{
    SessionControlBlock *ssn;

    if ( !ssnptr )
        return 0;

    ssn = (SessionControlBlock *)ssnptr;
    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 0;

    /* Don't need to do this for other protos because they don't
       do any reassembly. */
    if ( GET_IPH_PROTO(p) != IPPROTO_TCP )
        return 0;

    return StreamCheckSessionAlertTcp(ssn, p, gid, sid);
}

static int StreamUpdateSessionAlert(
        void *ssnptr,
        Packet *p,
        uint32_t gid,
        uint32_t sid,
        uint32_t event_id,
        uint32_t event_second)
{
    SessionControlBlock *ssn;

    if ( !ssnptr )
        return 0;

    ssn = (SessionControlBlock *)ssnptr;
    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 0;

    /* Don't need to do this for other protos because they don't
       do any reassembly. */
    if ( GET_IPH_PROTO(p) != IPPROTO_TCP )
        return 0;

    return StreamUpdateSessionAlertTcp(ssn, p, gid, sid, event_id, event_second);
}

static void StreamSetExtraData (void* pv, Packet* p, uint32_t flag)
{
    SessionControlBlock* ssn = pv;

    if ( !ssn )
        return;

    StreamSetExtraDataTcp(ssn, p, flag);
}

// FIXTHIS get pv/ssn from packet directly?
static void StreamClearExtraData (void* pv, Packet* p, uint32_t flag)
{
    SessionControlBlock* ssn = pv;

    if ( !ssn )
        return;

    StreamClearExtraDataTcp(ssn, p, flag);
}

static int StreamGetRebuiltPackets(
        Packet *p,
        PacketIterator callback,
        void *userdata)
{
    SessionControlBlock *ssn = (SessionControlBlock*)p->ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return 0;

    /* Only if this is a rebuilt packet */
    if (!(p->packet_flags & PKT_REBUILT_STREAM))
        return 0;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 0;

    return GetTcpRebuiltPackets(p, ssn, callback, userdata);
}

static int StreamGetStreamSegments(
        Packet *p,
        StreamSegmentIterator callback,
        void *userdata)
{
    SessionControlBlock *ssn = (SessionControlBlock*)p->ssnptr;

    if ((ssn == NULL) || (ssn->protocol != IPPROTO_TCP))
        return -1;

    /* Only if this is a rebuilt packet */
    if (!(p->packet_flags & PKT_REBUILT_STREAM))
        return -1;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return -1;

    return GetTcpStreamSegments(p, ssn, callback, userdata);
}

static void StreamUpdateDirection( void * scbptr, char dir, sfaddr_t* ip, uint16_t port )
{
    SessionControlBlock *scb = (SessionControlBlock *)scbptr;

    if (!scb)
        return;

    if (StreamSetRuntimeConfiguration(scb, scb->protocol) == -1)
        return;

    switch (scb->protocol)
    {
        case IPPROTO_TCP:
            TcpUpdateDirection(scb, dir, ip, port);
            break;
        case IPPROTO_UDP:
            UdpUpdateDirection(scb, dir, ip, port);
            break;
        case IPPROTO_ICMP:
            //IcmpUpdateDirection(scb, dir, ip, port);
            break;
    }
}

static char StreamGetReassemblyDirection(void *ssnptr)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return SSN_DIR_NONE;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return SSN_DIR_NONE;

    return StreamGetReassemblyDirectionTcp(ssn);
}

static uint32_t StreamGetFlushPoint(void *ssnptr, char dir)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if ((ssn == NULL) || (ssn->protocol != IPPROTO_TCP))
        return 0;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 0;

    return StreamGetFlushPointTcp(ssn, dir);
}

static void StreamSetFlushPoint(void *ssnptr, char dir, uint32_t flush_point)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if ((ssn == NULL) || (ssn->protocol != IPPROTO_TCP))
        return;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return;

    StreamSetFlushPointTcp(ssn, dir, flush_point);
}

static char StreamSetReassembly(void *ssnptr,
        uint8_t flush_policy,
        char dir,
        char flags)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return 0;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 0;

    return StreamSetReassemblyTcp(ssn, flush_policy, dir, flags);
}

#ifdef HAVE_DAQ_DECRYPTED_SSL
static int StreamSimulateTcpAck(void *ssnptr,
        uint8_t dir,
        uint32_t len)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return -1;

    stream_session_config = ssn->session_config;

    return StreamSimulatePeerTcpAckp(ssn, dir, len);
}
#endif

static char StreamGetReassemblyFlushPolicy(void *ssnptr, char dir)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return STREAM_FLPOLICY_NONE;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return STREAM_FLPOLICY_NONE;

    return StreamGetReassemblyFlushPolicyTcp(ssn, dir);
}

static char StreamIsStreamSequenced(void *ssnptr, char dir)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return 1;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 1;

    return StreamIsStreamSequencedTcp(ssn, dir);
}

static int StreamMissingInReassembled(void *ssnptr, char dir)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return SSN_MISSING_NONE;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return SSN_MISSING_NONE;

    return StreamMissingInReassembledTcp(ssn, dir);
}

static void StreamDropPacket( Packet *p )
{
    SessionControlBlock* scb = (SessionControlBlock*)p->ssnptr;

    if ( !scb )
        return;

    switch (scb->protocol)
    {
        case IPPROTO_TCP:
            StreamTcpSessionClear(p);
            break;
        case IPPROTO_UDP:
            UdpSessionCleanup(scb);
            break;
        case IPPROTO_IP:
            IpSessionCleanup(scb);
            break;
        case IPPROTO_ICMP:
            IcmpSessionCleanup(scb);
            break;
        default:
            break;
    }

    if (!(p->packet_flags & PKT_STATELESS))
        session_api->drop_traffic(p, p->ssnptr, SSN_DIR_BOTH);
}

static char StreamPacketsMissing(void *ssnptr, char dir)
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return 1;

    if (StreamSetRuntimeConfiguration(ssn, ssn->protocol) == -1)
        return 1;

    return StreamPacketsMissingTcp(ssn, dir);
}

#ifdef TARGET_BASED
static void initServiceFilterStatus( struct _SnortConfig *sc )
{
    SFGHASH_NODE *hashNode;
    tSfPolicyId policyId = 0;

    if( sc == NULL )
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n", __FILE__, __LINE__);
    }

    for( hashNode = sfghash_findfirst( sc->otn_map );
            hashNode;
            hashNode = sfghash_findnext( sc->otn_map ) )
    {
        OptTreeNode *otn = ( OptTreeNode * ) hashNode->data;

        for( policyId = 0; policyId < otn->proto_node_num; policyId++ )
        {
            RuleTreeNode *rtn = getRtnFromOtn( otn, policyId );
            if( rtn && ( rtn->proto == IPPROTO_TCP ) )
            {
                unsigned int svc_idx;
                for( svc_idx = 0; svc_idx < otn->sigInfo.num_services; svc_idx++ )
                    if( otn->sigInfo.services[svc_idx].service_ordinal )
                        setServiceFilterStatus( sc, otn->sigInfo.services[svc_idx].service_ordinal,
                                PORT_MONITOR_SESSION, policyId, 1 );
            }
        }
    }
}

static void setServiceFilterStatus( struct _SnortConfig *sc, int service, int status, tSfPolicyId policyId, int parsing )
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( config != NULL )
        config->service_filter[ service ] = status;
}

static int getServiceFilterStatus( struct _SnortConfig *sc, int service, tSfPolicyId policyId, int parsing )
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( config != NULL )
        return config->service_filter[ service ];
    else
        return PORT_MONITOR_NONE;
}
#endif

int isPacketFilterDiscard( Packet *p, int ignore_any_rules )
{
    uint8_t  action = 0;
    tPortFilterStats   *pPortFilterStats = NULL;
    tSfPolicyId policy_id = getNapRuntimePolicy();
#ifdef TARGET_BASED
    int protocolId = GetProtocolReference(p);
#endif

#ifdef TARGET_BASED
    if( ( protocolId > 0 ) && getServiceFilterStatus( NULL, protocolId, policy_id, 0 ) )
    {
        return PORT_MONITOR_PACKET_PROCESS;
    }
#endif

    switch( GET_IPH_PROTO( p ) )
    {
        case IPPROTO_TCP:
            if( session_api->protocol_tracking_enabled( SESSION_PROTO_TCP ) )
            {
                action = s5TcpGetPortFilterStatus( NULL, p->sp, policy_id, 0 )
                    |
                    s5TcpGetPortFilterStatus( NULL, p->dp, policy_id, 0 );
            }

            pPortFilterStats = &s5stats.tcp_port_filter;
            break;

        case IPPROTO_UDP:
            if( session_api->protocol_tracking_enabled( SESSION_PROTO_UDP ) )
            {
                action = s5UdpGetPortFilterStatus( NULL, p->sp, policy_id, 0 )
                    |
                    s5UdpGetPortFilterStatus( NULL, p->dp, policy_id, 0 );
            }

            pPortFilterStats = &s5stats.udp_port_filter;
            break;

        default:
            return PORT_MONITOR_PACKET_PROCESS;
    }

    if( !( action & PORT_MONITOR_SESSION_BITS ) )
    {
        if( !( action & PORT_MONITOR_INSPECT ) && ignore_any_rules )
        {
            /* Ignore this TCP packet entirely */
            DisableDetect( p );
            //otn_tmp = NULL;
            pPortFilterStats->filtered++;
        }
        else
        {
            pPortFilterStats->inspected++;
        }

        return PORT_MONITOR_PACKET_DISCARD;
    }

    pPortFilterStats->session_tracked++;
    return PORT_MONITOR_PACKET_PROCESS;
}

int isPacketFilterDiscardUdp ( Packet *p, int ignore_any_rules )
{
    uint8_t action_ips = 0, action_nap = 0;
    tPortFilterStats *pPortFilterStats = NULL;
    SessionControlBlock *scb;
    tSfPolicyId policy_id_ips = getIpsRuntimePolicy();
    tSfPolicyId policy_id_nap = getNapRuntimePolicy();
    SnortPolicy *policy;
    PreprocEnableMask enabled_pps;   
    bool nap_inspect = false;

    scb = p->ssnptr;
    if ( !scb ) {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Session control block of packet is NULL.\n"););
        return PORT_MONITOR_PACKET_DISCARD;
    }
    
    if ( session_api->protocol_tracking_enabled( SESSION_PROTO_UDP ) &&
        ( snort_conf->udp_ips_port_filter_list ) ) {
        action_ips = s5UdpGetIPSPortFilterStatus (snort_conf, p->sp, p->dp, policy_id_ips);
    }

    pPortFilterStats = &s5stats.udp_port_filter;

    // Check if NAP has marked it as inspect/filter. 
    action_nap = s5UdpGetPortFilterStatus (NULL, p->sp, policy_id_nap, 0) | 
                 s5UdpGetPortFilterStatus (NULL, p->dp, policy_id_nap, 0);
    if ( !( action_nap & PORT_MONITOR_SESSION_BITS ) && ( action_nap & PORT_MONITOR_INSPECT ) && ignore_any_rules ) {
        nap_inspect = true ;
    }

    if ( !( action_ips & PORT_MONITOR_SESSION_BITS ) ) {
        if ( !( action_ips & PORT_MONITOR_INSPECT ) && ignore_any_rules ) {
            // Port not present in IPS port list too, disable detection. 
            DisableDetect( p );
        } else {
            /*
             * If nap_inspect is true it implies NAP marked it for inspect, now IPS too marking for inspect,
             * so no change in counter.
             * If nap_inspect is false ie: NAP marked for filter, now IPS marks it to inspect undo the NAP counter. 
             */ 
            if ( !nap_inspect ) {
                sfBase.total_udp_filtered_packets--;
                pPortFilterStats->filtered--;
                pPortFilterStats->inspected++;
            }
        }
        return PORT_MONITOR_PACKET_DISCARD;
    }
    // Undo NAPs increment and enable detection 
    if ( nap_inspect )
       pPortFilterStats->inspected--;
    else 
       pPortFilterStats->filtered--;

    pPortFilterStats->session_tracked++;
    policy = snort_conf->targeted_policies[ getNapRuntimePolicy() ];
    enabled_pps = policy->pp_enabled[ p->dp ] | policy->pp_enabled[ p->sp ];
    EnableContentPreprocDetection (p,enabled_pps);

    return PORT_MONITOR_PACKET_PROCESS;
}

static uint8_t StreamRegisterPAFPort( struct _SnortConfig *sc, tSfPolicyId id, uint16_t server_port,
        bool to_server, PAF_Callback cb, bool autoEnable)
{
    return s5_paf_register_port( sc, id, server_port, to_server, cb, autoEnable );
}

static uint8_t StreamRegisterPAFService( struct _SnortConfig *sc, tSfPolicyId id, uint16_t service,
        bool to_server, PAF_Callback cb, bool autoEnable)
{
    return s5_paf_register_service( sc, id, service, to_server, cb, autoEnable );
}

static uint32_t StreamRegisterXtraData( LogFunction f )
{
    uint32_t i = 0;
    while( i < xtradata_func_count )
    {
        if( xtradata_map[i++] == f )
        {
            return i;
        }
    }
    if( xtradata_func_count == LOG_FUNC_MAX )
        return 0;
    xtradata_map[xtradata_func_count++] = f;
    return xtradata_func_count;
}

static uint32_t StreamGetXtraDataMap( LogFunction **f )
{
    if( f )
    {
        *f = xtradata_map;
        return xtradata_func_count;
    }
    else
        return 0;
}

static void StreamRegisterXtraDataLog( LogExtraData f, void *config )
{
    extra_data_log = f;
    extra_data_config = config;
}

void** StreamGetPAFUserData( void* ssnptr, bool to_server, uint8_t id )
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return NULL;

    return StreamGetPAFUserDataTcp( ( SessionControlBlock * ) ssnptr, to_server, id );
}

static bool StreamIsPafActive( void* ssnptr, bool to_server )
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return false;

    return StreamIsPafActiveTcp( ( SessionControlBlock * ) ssnptr, to_server );
}

static bool StreamActivatePaf( void* ssnptr, int dir, int16_t service, uint8_t type )
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if (!ssn || ssn->protocol != IPPROTO_TCP)
        return false;

    return StreamActivatePafTcp( ( SessionControlBlock *) ssnptr, dir, service, type );
}

static void StreamResetPolicy( void *ssnptr, int dir, uint16_t policy, uint16_t mss )
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if( ssn )
    {
        if (ssn->protocol != IPPROTO_TCP )
            return;

        StreamResetPolicyTcp( ssnptr, dir, policy, mss );
    }

    return;
}

static void StreamSetSessionDecrypted( void *ssnptr, bool enable )
{
    SessionControlBlock *ssn = (SessionControlBlock *)ssnptr;

    if( ssn )
    {
        if (ssn->protocol != IPPROTO_TCP )
            return;

        StreamSetSessionDecryptedTcp( ssnptr, enable );
    }

    return;
}

static bool StreamIsSessionDecrypted( void *ssnptr )
{
    SessionControlBlock *ssn;

    if(ssnptr)
    {
        ssn = (SessionControlBlock *)ssnptr;
        if (ssn->protocol == IPPROTO_TCP )
            return StreamIsSessionDecryptedTcp( ssnptr );
        else
            return false;
    }
    else
        return false;
}

static void s5SetPortFilterStatus( struct _SnortConfig *sc, IpProto protocol, uint16_t port, uint16_t status,
        tSfPolicyId policyId, int parsing )
{
    switch( protocol )
    {
        case IPPROTO_TCP:
            s5TcpSetPortFilterStatus( sc, port, status, policyId, parsing );
            break;

        case IPPROTO_UDP:
            s5UdpSetPortFilterStatus( sc, port, status, policyId, parsing );
            break;

        case IPPROTO_ICMP:
            break;

        default:
            break;
    }
}

static void s5UnsetPortFilterStatus( struct _SnortConfig *sc, IpProto protocol, uint16_t port, uint16_t status,
        tSfPolicyId policyId, int parsing )
{
    if( status <= PORT_MONITOR_SESSION )
        return;

    switch( protocol )
    {
        case IPPROTO_TCP:
            s5TcpUnsetPortFilterStatus( sc, port, status, policyId, parsing );
            break;

        case IPPROTO_UDP:
            s5UdpUnsetPortFilterStatus( sc, port, status, policyId, parsing );
            break;

        case IPPROTO_ICMP:
            break;

        default:
            break;
    }
}

static void StreamForceSessionExpiration( void *ssnptr )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) ssnptr;

    if( StreamExpireSession( scb ) )
    {
#ifdef ENABLE_HA
        SessionHANotifyDeletion( scb );
#endif
    }
}

static void StreamForceDeleteSession(void *ssnptr )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) ssnptr;

    if(scb)
        StreamDeleteSession( scb );
}

static void registerReassemblyPort( char *network, uint16_t port, int reassembly_direction )
{
    registerPortForReassembly( network, port, reassembly_direction );
}

static void unregisterReassemblyPort( char *network, uint16_t port, int reassembly_direction )
{
    unregisterPortForReassembly( network, port, reassembly_direction );
}

#define CB_MAX 32
static Stream_Callback stream_cb[ CB_MAX ];
static unsigned stream_cb_idx = 1;

static unsigned StreamRegisterHandler( Stream_Callback cb )
{
    unsigned id;

    for ( id = 1; id < stream_cb_idx; id++ )
    {
        if ( stream_cb[id] == cb )
            break;
    }
    if ( id == CB_MAX )
        return 0;

    if ( id == stream_cb_idx )
        stream_cb[stream_cb_idx++] = cb;

    return id;
}

static bool StreamSetHandler( void* ssnptr, unsigned id, Stream_Event se )
{
    SessionControlBlock *scb = ( SessionControlBlock * ) ssnptr;

    if ( se >= SE_MAX || scb->handler[ se ] )
        return false;

    scb->handler[ se ] = id;
    return true;
}

static uint32_t StreamGetPreprocFlags( void *ssnptr)
{
    SessionControlBlock *scb = ( SessionControlBlock * ) ssnptr;
    if( scb )
        return StreamGetPreprocFlagsTcp(ssnptr);

    return 0;

}

#if defined(FEAT_OPEN_APPID)
static void SetApplicationId(void* ssnptr, int16_t serviceAppId, int16_t clientAppId,
        int16_t payloadAppId, int16_t miscAppId)
{
    SessionControlBlock *scb = (SessionControlBlock *)ssnptr;

    scb->app_protocol_id[APP_PROTOID_SERVICE] = serviceAppId;
    scb->app_protocol_id[APP_PROTOID_CLIENT] = clientAppId;
    scb->app_protocol_id[APP_PROTOID_PAYLOAD] = payloadAppId;
    scb->app_protocol_id[APP_PROTOID_MISC] = miscAppId;
}

static void GetApplicationId(void* ssnptr, int16_t *serviceAppId, int16_t *clientAppId,
        int16_t *payloadAppId, int16_t *miscAppId)
{
    SessionControlBlock *scb = (SessionControlBlock *)ssnptr;

    *serviceAppId = scb->app_protocol_id[APP_PROTOID_SERVICE];
    *clientAppId = scb->app_protocol_id[APP_PROTOID_CLIENT];
    *payloadAppId = scb->app_protocol_id[APP_PROTOID_PAYLOAD];
    *miscAppId   = scb->app_protocol_id[APP_PROTOID_MISC];
}

#define HTTP_HEADER_PROCESSOR_MAX 10
static Http_Processor_Callback http_header_processor_cb[HTTP_HEADER_PROCESSOR_MAX];
static unsigned http_header_processor_cb_idx = 1;

static int RegisterHttpHeaderCallback(Http_Processor_Callback cb)
{
    unsigned id;

    for ( id = 1; id < http_header_processor_cb_idx; id++ )
    {
        if ( http_header_processor_cb[id] == cb )
            break;
    }
    if ( id == HTTP_HEADER_PROCESSOR_MAX )
        return -1;

    if ( id == http_header_processor_cb_idx )
        http_header_processor_cb[http_header_processor_cb_idx++] = cb;

    return 0;
}

void CallHttpHeaderProcessors(Packet* p, HttpParsedHeaders * const headers)
{
    unsigned id;

    for ( id = 1; id < http_header_processor_cb_idx; id++ )
    {
        http_header_processor_cb[id](p, headers);
    }
}

bool IsAnybodyRegisteredForHttpHeader(void)
{
    return ((http_header_processor_cb_idx - 1) > 0);
}
#endif /* defined(FEAT_OPEN_APPID) */

#define SERVICE_EVENT_SUBSCRIBER_MAX 10
#define SERVICE_EVENT_TYPE_MAX 10
static ServiceEventNotifierFunc serviceEventRegistry[PP_MAX][SERVICE_EVENT_TYPE_MAX][SERVICE_EVENT_SUBSCRIBER_MAX];

static bool serviceEventSubscribe(unsigned int preprocId, ServiceEventType eventType, ServiceEventNotifierFunc cb)
{
    unsigned i;
    ServiceEventNotifierFunc *notifierPtr;

    if (preprocId >= PP_MAX || eventType >= SERVICE_EVENT_TYPE_MAX)
        return false;

    notifierPtr = serviceEventRegistry[preprocId][eventType];

    for ( i = 0; i < SERVICE_EVENT_SUBSCRIBER_MAX; i++ , notifierPtr++)
    {
        if ( *notifierPtr == cb )
            return true;
        if (!(*notifierPtr))
        {
            *notifierPtr = cb;
            return true;
        }
    }
    return false;
}

static bool serviceEventPublish(unsigned int preprocId, void *ssnptr, ServiceEventType eventType, void * eventData)
{
    unsigned i;
    ServiceEventNotifierFunc *notifierPtr;

    if (preprocId >= PP_MAX || eventType >= SERVICE_EVENT_TYPE_MAX)
        return false;

    notifierPtr = serviceEventRegistry[preprocId][eventType];

    for ( i = 0; i < SERVICE_EVENT_SUBSCRIBER_MAX; i++ , notifierPtr++)
    {
        if (*notifierPtr)
            (*notifierPtr)(ssnptr, eventType, eventData);
        else
            break;
    }

    return true;
}

void StreamCallHandler( Packet* p, unsigned id )
{
    assert( id && id < stream_cb_idx && stream_cb[ id ] );
    stream_cb[ id ]( p );
}

static void setFtpFilePosition (void *scbptr,bool flush)
{
  SetFTPFileLocation(scbptr,flush);
  return;
}


static int StreamSetApplicationProtocolIdExpectedPreassignCallbackId( const Packet *ctrlPkt,
        sfaddr_t* srcIP, uint16_t srcPort, sfaddr_t* dstIP, uint16_t dstPort,
        uint8_t protocol, int16_t protoId, uint32_t preprocId, void *protoData,
        void ( *protoDataFreeFn )( void * ), unsigned cbId, Stream_Event se,
        struct _ExpectNode** packetExpectedNode)
{
    return StreamExpectAddChannelPreassignCallback(ctrlPkt, srcIP, srcPort, dstIP, dstPort,
            SSN_DIR_BOTH, 0, protocol, STREAM_EXPECTED_CHANNEL_TIMEOUT,
            protoId, preprocId, protoData, protoDataFreeFn, cbId, se,
            packetExpectedNode);
}

#define FTP_PROCESSOR_MAX 2
static FTP_Processor_Flush_Callback ftp_processor_flush_cb[FTP_PROCESSOR_MAX];
static unsigned ftp_processor_flush_cb_idx = 1;

static int RegisterFTPFlushCallback(FTP_Processor_Flush_Callback cb)
{
    unsigned id;

    for ( id = 1; id < ftp_processor_flush_cb_idx; id++ )
    {
        if ( ftp_processor_flush_cb[id] == cb )
            break;
    }
    if ( id == FTP_PROCESSOR_MAX)
        return -1;

    if ( id == ftp_processor_flush_cb_idx)
        ftp_processor_flush_cb[ftp_processor_flush_cb_idx++] = cb;

    return 0;
}

void CallFTPFlushProcessor(Packet* p)
{
    unsigned id;

    for ( id = 1; id < ftp_processor_flush_cb_idx; id++ )
    {
        ftp_processor_flush_cb[id](p);
    }
}

#ifdef SNORT_RELOAD
static void StreamTcpReload( struct _SnortConfig *sc, char *args, void **new_config )
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, true );
    if( !config->session_config->track_tcp_sessions )
        return;

    if( config->tcp_config == NULL )
    {
        config->tcp_config = ( StreamTcpConfig * ) SnortPreprocAlloc( 1, 
                                               sizeof( StreamTcpConfig ),
                                               PP_STREAM, PP_MEM_CATEGORY_CONFIG );

        StreamTcpInitFlushPoints();
        StreamTcpRegisterRuleOptions( sc );
        AddFuncToPreprocPostConfigList( sc, StreamPostConfigTcp, config->tcp_config );
    }

    /* Call the protocol specific initializer */
    StreamTcpPolicyInit( sc, config->tcp_config, args );

    *new_config = getStreamConfigContext( true );
}

static void StreamUdpReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, true );
    if( !config->session_config->track_udp_sessions )
        return;

    if( config->udp_config == NULL )
        config->udp_config = ( StreamUdpConfig * ) SnortPreprocAlloc( 1, 
                                                 sizeof( StreamUdpConfig ),
                                                 PP_STREAM, PP_MEM_CATEGORY_CONFIG );

    /* Call the protocol specific initializer */
    StreamUdpPolicyInit( config->udp_config, args );

    *new_config = getStreamConfigContext( true );
}

static void StreamIcmpReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, true );
    if( !config->session_config->track_icmp_sessions )
        return;

    if( config->icmp_config == NULL )
        config->icmp_config = ( StreamIcmpConfig * ) SnortPreprocAlloc(1, sizeof( StreamIcmpConfig ),
                                                      PP_STREAM, PP_MEM_CATEGORY_CONFIG);

    /* Call the protocol specific initializer */
    StreamIcmpPolicyInit( config->icmp_config, args );

    *new_config = getStreamConfigContext( true );
}

static void StreamIpReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    StreamConfig *config;

    config = initStreamPolicyConfig( sc, true );
    if( !config->session_config->track_ip_sessions )
        return;

    if( config->ip_config == NULL )
        config->ip_config = ( StreamIpConfig * ) SnortPreprocAlloc( 1,
                                                      sizeof( *config->ip_config ),
                                                      PP_STREAM, PP_MEM_CATEGORY_CONFIG );

    /* Call the protocol specific initializer */
    StreamIpPolicyInit( config->ip_config, args );

    *new_config = getStreamConfigContext( true );
}

static int StreamReloadVerify( struct _SnortConfig *sc, void *swap_config )
{
    tSfPolicyUserContextId ssc = ( tSfPolicyUserContextId ) swap_config;

    if( ( ssc == NULL ) || ( stream_online_config == NULL ) )
        return 0;

    if( sfPolicyUserDataIterate( sc, ssc, StreamVerifyConfigPolicy ) != 0 )
        return -1;

#ifdef TARGET_BASED
    initServiceFilterStatus( sc );
#endif

    return 0;
}
static void StreamFreeOldConfig( void *data )
{
    if( data == NULL )
        return;

    StreamFreeConfigs( ( tSfPolicyUserContextId ) data );
}

static int StreamReloadSwapPolicy( struct _SnortConfig *sc, tSfPolicyUserContextId config,
                                   tSfPolicyId policyId, void *pData )
{
    StreamConfig *stream_config = ( StreamConfig * ) pData;

    if( stream_config->session_config->policy_ref_count[ policyId ] == 0 )
    {
        sfPolicyUserDataClear( config, policyId );
        StreamFreeConfig( stream_config );
    }
    else
       register_no_ref_policy_callback(stream_config->session_config, StreamFreeOldConfig, (void *)config);

    return 0;
}

static void *StreamReloadSwap( struct _SnortConfig *sc, void *swap_config )
{
   tSfPolicyUserContextId new_config = ( tSfPolicyUserContextId ) swap_config;
   tSfPolicyUserContextId old_config = stream_online_config;


    if( ( new_config == stream_online_config ) || new_config == NULL )
        return NULL;

    stream_online_config = new_config;
    stream_parsing_config = NULL;
    // free memory for all configs with no refs
    sfPolicyUserDataIterate( sc, old_config, StreamReloadSwapPolicy );
    if( sfPolicyUserPolicyGetActive( old_config ) == 0 )
        return old_config;

    // still some active sessions with ref to old config...
    return NULL;
}

static void StreamReloadSwapFree( void *data )
{
    if( data == NULL )
        return;

    if( !old_config_freed )
    {
        StreamFreeConfigs( ( tSfPolicyUserContextId ) data );
        old_config_freed = true;
    }
}

#endif

static void StreamRegisterPAFFree(uint8_t id, PAF_Free_Callback cb)
{
    s5_paf_register_free(id, cb);
}

static Packet* getWirePacket()
{
    return getWirePacketTcp();
}

static uint8_t getFlushPolicyDir()
{
    return getFlushPolicyDirTcp();
}

static bool StreamIsSessionHttp2( void *ssnptr )
{
    SessionControlBlock *ssn;

    if(ssnptr)
    {    
        ssn = (SessionControlBlock *)ssnptr;
        if (ssn->protocol == IPPROTO_TCP )
            return StreamIsSessionHttp2Tcp( ssnptr );
        else 
            return false;
    }    
    else 
        return false;
}
static void StreamSetSessionHttp2( void *ssnptr)
{
    if( ssnptr )
        StreamSetSessionHttp2Tcp( ssnptr );

    return;
}

static bool StreamShowRebuiltPackets()
{
    return (stream_session_config->flags & STREAM_CONFIG_SHOW_PACKETS);
}

static bool StreamIsSessionHttp2Upg( void *ssnptr )
{
    SessionControlBlock *ssn;

    if(ssnptr)
    {
        ssn = (SessionControlBlock *)ssnptr;
        if (ssn->protocol == IPPROTO_TCP )
            return StreamIsSessionHttp2UpgTcp( ssnptr );
        else
            return false;
    }
    else
        return false;
}
static void StreamSetSessionHttp2Upg( void *ssnptr)
{
    if( ssnptr )
        StreamSetSessionHttp2UpgTcp( ssnptr );

    return;
}

