/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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

#ifndef SESSION_COMMON_H_
#define SESSION_COMMON_H_

#include <sys/types.h>
#ifndef WIN32
#include <netinet/in.h>
#endif

#include "sfutil/bitop_funcs.h"
#include "sfutil/sfActionQueue.h"
#include "parser/IpAddrSet.h"

#include "session_api.h"
#include "mempool.h"
#include "sf_types.h"
#include "plugbase.h"

#ifdef TARGET_BASED
#include "target-based/sftarget_hostentry.h"
#endif

#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/* defaults and limits */
#define STREAM_DEFAULT_SSN_TIMEOUT  30        /* seconds to timeout a session */
#define STREAM_MAX_SSN_TIMEOUT      3600 * 24 /* max timeout (approx 1 day) */
#define STREAM_MIN_SSN_TIMEOUT      1         /* min timeout (1 second) */
#define STREAM_MIN_ALT_HS_TIMEOUT   0         /* min timeout (0 seconds) */
/* Lower timeout value in seconds to clean up the session
 * for receiving valid RST for a ongoing/hanged tcp session.
 */
#define STREAM_SSN_RST_TIMEOUT      180
#define STREAM_TRACK_YES            1
#define STREAM_TRACK_NO             0

#define STREAM_MIN_MAX_WINDOW       0
#define MAX_PORTS_TO_PRINT      20

/* traffic direction identification */
#define FROM_SERVER     0
#define FROM_RESPONDER  0
#define FROM_CLIENT     1
#define FROM_SENDER     1

#if defined(FEAT_OPEN_APPID)
#define MAX_APP_PROTOCOL_ID  4
#endif /* defined(FEAT_OPEN_APPID) */

/*  Control Socket types */
#define CS_TYPE_DEBUG_STREAM_HA     ((PP_STREAM << 7) + 0)     // 0x680 / 1664

/*  D A T A   S T R U C T U R E S  **********************************/

typedef void (*NoRefCallback)( void *data );

#ifdef ENABLE_HA
typedef struct _SessionHAConfig
{
    struct timeval min_session_lifetime;
    struct timeval min_sync_interval;
    char *startup_input_file;
    char *runtime_output_file;
    char *shutdown_output_file;
#ifdef REG_TEST
    char *runtime_input_file;
# endif
# ifdef SIDE_CHANNEL
    uint8_t use_side_channel;
# endif
    uint8_t use_daq;
} SessionHAConfig;
#endif

typedef struct _SessionConfiguration
{
    char       disabled;
    char       track_tcp_sessions;
    char       track_udp_sessions;
    char       track_icmp_sessions;
    char       track_ip_sessions;
#ifdef ENABLE_HA
    char       enable_ha;
#endif
    uint32_t   max_sessions;
    uint32_t   max_tcp_sessions;
    uint32_t   max_udp_sessions;
    uint32_t   max_icmp_sessions;
    uint32_t   max_ip_sessions;
    uint16_t   tcp_cache_pruning_timeout;
    uint16_t   tcp_cache_nominal_timeout;
    uint16_t   udp_cache_pruning_timeout;
    uint16_t   udp_cache_nominal_timeout;
    uint32_t   memcap;
    uint32_t   prune_log_max;
    uint32_t   flags;

#ifdef ACTIVE_RESPONSE
    uint32_t   min_response_seconds;
    uint8_t    max_active_responses;
#endif

#ifdef ENABLE_HA
    SessionHAConfig *ha_config;
#endif
    uint32_t  numSnortPolicies;
    uint32_t  *policy_ref_count;
#ifdef SNORT_RELOAD
    NoRefCallback no_ref_cb;
    void         *no_ref_cb_data;
#endif
} SessionConfiguration;

#ifdef MPLS
typedef struct _MPLS_Hdr
{
    uint16_t length;
    uint8_t* start;
}MPLS_Hdr;
#endif

// this struct is organized by member size for compactness
typedef struct _SessionControlBlock
{
    SessionKey *key;

    MemBucket  *proto_specific_data;
    StreamAppData *appDataList;

    MemBucket *flowdata; /* add flowbits */

    long       last_data_seen;
    uint64_t   expire_time;

    tSfPolicyId napPolicyId;
    tSfPolicyId ipsPolicyId;
    SessionConfiguration *session_config;
    void *stream_config;
    void *proto_policy;

    PreprocEvalFuncNode *initial_pp;
    PreprocEnableMask enabled_pps;

    uint16_t    session_state;
    uint8_t     handler[SE_MAX];

    sfaddr_t    client_ip; // FIXTHIS family and bits should be changed to uint16_t
    sfaddr_t    server_ip; // or uint8_t to reduce sizeof from 24 to 20

    uint16_t    client_port;
    uint16_t    server_port;
    bool        port_guess;
    bool        stream_config_stale;

    uint8_t     protocol;

#ifdef ACTIVE_RESPONSE
    uint8_t     response_count;
#endif

    uint8_t  inner_client_ttl;
    uint8_t  inner_server_ttl;
    uint8_t  outer_client_ttl;
    uint8_t  outer_server_ttl;

    StreamHAState ha_state;
    StreamHAState cached_ha_state;

#ifdef ENABLE_HA
    struct timeval  ha_next_update;
    uint8_t         ha_pending_mask;
    uint8_t         ha_flags;
#endif

    bool    ips_os_selected;
    bool    session_established;
    bool    new_session;
    bool    in_oneway_list;
    bool    is_session_deletion_delayed;
    uint8_t iprep_update_counter;

    // pointers for linking into list of oneway sessions
    struct _SessionControlBlock *ows_prev;
    struct _SessionControlBlock *ows_next;

#if defined(FEAT_OPEN_APPID)
    int16_t     app_protocol_id[MAX_APP_PROTOCOL_ID];
#endif /* defined(FEAT_OPEN_APPID) */

#ifdef MPLS
   MPLS_Hdr *clientMplsHeader;
   MPLS_Hdr *serverMplsHeader;
#endif
} SessionControlBlock;


/**Common statistics for tcp and udp packets, maintained by port filtering.
 */
typedef struct {
    /* packets filtered without further processing by any preprocessor or
     * detection engine.
     */
    uint32_t  filtered;

    /* packets inspected and but processed futher by stream5 preprocessor.
     */
    uint32_t  inspected;

    /* packets session tracked by stream5 preprocessor.
     */
    uint32_t  session_tracked;
} tPortFilterStats;

typedef struct _SessionStatistics
{
    uint32_t   total_tcp_sessions;
    uint32_t   total_udp_sessions;
    uint32_t   total_icmp_sessions;
    uint32_t   total_ip_sessions;
    uint32_t   tcp_prunes;
    uint32_t   udp_prunes;
    uint32_t   icmp_prunes;
    uint32_t   ip_prunes;
    uint32_t   events;
    uint32_t   internalEvents;
    tPortFilterStats  tcp_port_filter;
    tPortFilterStats  udp_port_filter;
} SessionStatistics;

SessionConfiguration *getSessionConfiguration( bool reload_config );
int SessionTrackingEnabled( SessionConfiguration *config, uint32_t protocol );
uint32_t GetSessionPruneLogMax( void );
uint32_t GetSessionMemCap( void );
void SessionFreeConfig( SessionConfiguration * );
int isPacketFilterDiscard( Packet *p, int ignore_any_rules );
int isPacketFilterDiscardUdp( Packet *p, int ignore_any_rules );

typedef void ( *set_dir_ports_cb )( Packet *p, SessionControlBlock *scb );
typedef int ( *flush_stream_cb )( Packet *p, SessionControlBlock *scb );

void registerDirectionPortCallback( uint8_t proto, set_dir_ports_cb cb_func );
void registerFlushStreamCallback( bool client_to_server, flush_stream_cb cb_func );

#ifdef SNORT_RELOAD
void register_no_ref_policy_callback(SessionConfiguration *session_conf, NoRefCallback cb, void *data);
#endif

struct session_plugins
{
    set_dir_ports_cb set_tcp_dir_ports;
    set_dir_ports_cb set_udp_dir_ports;
    flush_stream_cb  flush_client_stream;
    flush_stream_cb  flush_server_stream;
    nap_selector select_session_nap;
};

struct session_plugins *getSessionPlugins( void );
void freeSessionPlugins( void );

// shared session state
extern SessionStatistics session_stats;
extern uint32_t firstPacketTime;
extern SessionConfiguration *session_configuration;

extern uint32_t session_mem_in_use;
extern tSfActionQueueId decoderActionQ;


static inline uint64_t CalcJiffies(Packet *p)
{
    uint64_t ret = 0;
    uint64_t sec = ( uint64_t ) p->pkth->ts.tv_sec * TCP_HZ;
    uint64_t usec = ( p->pkth->ts.tv_usec / ( 1000000UL / TCP_HZ ) );

    ret = sec + usec;

    return ret;
}


#endif /* SESSION_COMMON_H_ */
