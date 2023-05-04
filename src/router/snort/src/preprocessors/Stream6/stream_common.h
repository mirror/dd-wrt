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

#ifndef STREAM_COMMON_H_
#define STREAM_COMMON_H_

#include <sys/types.h>
#ifndef WIN32
#include <netinet/in.h>
#endif

#include "sfutil/bitop_funcs.h"
#include "sfutil/sfActionQueue.h"
#include "parser/IpAddrSet.h"

#include "session_common.h"
#include "stream_api.h"
#include "mempool.h"
#include "sf_types.h"

#ifdef TARGET_BASED
#include "target-based/sftarget_hostentry.h"
#endif

#include "sfPolicy.h"
#include "sfPolicyUserData.h"

//#define STREAM_DEBUG_ENABLED DEBUG


/* defaults and limits */
#define STREAM_MAX_MAX_WINDOW       0x3FFFc000  /* max window allowed by TCP */
                                                /* 65535 << 14 (max wscale) */
#define STREAM_MIN_MAX_WINDOW       0
#define MAX_PORTS_TO_PRINT          20

#define STREAM_DEFAULT_MAX_QUEUED_BYTES 1048576 /* 1 MB */
#define STREAM_MIN_MAX_QUEUED_BYTES 1024       /* Don't let this go below 1024 */
#define STREAM_MAX_MAX_QUEUED_BYTES 0x40000000 /* 1 GB, most we could reach within
                                            * largest window scale */
#define AVG_PKT_SIZE            400
#define STREAM_DEFAULT_MAX_QUEUED_SEGS (STREAM_DEFAULT_MAX_QUEUED_BYTES/AVG_PKT_SIZE)
#define STREAM_MIN_MAX_QUEUED_SEGS  2          /* Don't let this go below 2 */
#define STREAM_MAX_MAX_QUEUED_SEGS  0x40000000 /* 1 GB worth of one-byte segments */

#define STREAM_DEFAULT_MAX_SMALL_SEG_SIZE 0    /* disabled */
#define STREAM_MAX_MAX_SMALL_SEG_SIZE 2048     /* 2048 bytes in single packet, uh, not small */
#define STREAM_MIN_MAX_SMALL_SEG_SIZE 0        /* 0 means disabled */

#define STREAM_DEFAULT_CONSEC_SMALL_SEGS 0     /* disabled */
#define STREAM_MAX_CONSEC_SMALL_SEGS 2048      /* 2048 single byte packets without acks is alot */
#define STREAM_MIN_CONSEC_SMALL_SEGS 0         /* 0 means disabled */

#if defined(FEAT_OPEN_APPID)
#define MAX_APP_PROTOCOL_ID  4
#endif /* defined(FEAT_OPEN_APPID) */

/* target-based policy types */
#define STREAM_POLICY_FIRST     1
#define STREAM_POLICY_LINUX     2
#define STREAM_POLICY_BSD       3
#define STREAM_POLICY_OLD_LINUX 4
#define STREAM_POLICY_LAST      5
#define STREAM_POLICY_WINDOWS   6
#define STREAM_POLICY_SOLARIS   7
#define STREAM_POLICY_HPUX11    8
#define STREAM_POLICY_IRIX      9
#define STREAM_POLICY_MACOS     10
#define STREAM_POLICY_HPUX10    11
#define STREAM_POLICY_VISTA     12
#define STREAM_POLICY_WINDOWS2K3 13
#define STREAM_POLICY_IPS       14
#define STREAM_POLICY_NOACK     15
#define STREAM_POLICY_DEFAULT   STREAM_POLICY_BSD

#define STREAM_CONFIG_STATEFUL_INSPECTION      0x00000001
#define STREAM_CONFIG_ENABLE_ALERTS            0x00000002
#define STREAM_CONFIG_LOG_STREAMS              0x00000004
#define STREAM_CONFIG_REASS_CLIENT             0x00000008
#define STREAM_CONFIG_REASS_SERVER             0x00000010
#define STREAM_CONFIG_ASYNC                    0x00000020
#define STREAM_CONFIG_SHOW_PACKETS             0x00000040
#define STREAM_CONFIG_FLUSH_ON_ALERT           0x00000080
#define STREAM_CONFIG_REQUIRE_3WHS             0x00000100
#define STREAM_CONFIG_MIDSTREAM_DROP_NOALERT   0x00000200
#define STREAM_CONFIG_IGNORE_ANY               0x00000400
#define STREAM_CONFIG_PERFORMANCE              0x00000800
#define STREAM_CONFIG_STATIC_FLUSHPOINTS       0x00001000
#define STREAM_CONFIG_IPS                      0x00002000
#define STREAM_CONFIG_CHECK_SESSION_HIJACKING  0x00004000
#define STREAM_CONFIG_NO_ASYNC_REASSEMBLY      0x00008000

/* traffic direction identification */
#define FROM_SERVER     0
#define FROM_RESPONDER  0
#define FROM_CLIENT     1
#define FROM_SENDER     1

#define STREAM_STATE_NONE                  0x0000
#define STREAM_STATE_SYN                   0x0001
#define STREAM_STATE_SYN_ACK               0x0002
#define STREAM_STATE_ACK                   0x0004
#define STREAM_STATE_ESTABLISHED           0x0008
#define STREAM_STATE_DROP_CLIENT           0x0010
#define STREAM_STATE_DROP_SERVER           0x0020
#define STREAM_STATE_MIDSTREAM             0x0040
#define STREAM_STATE_TIMEDOUT              0x0080
#define STREAM_STATE_UNREACH               0x0100
#define STREAM_STATE_PORT_INSPECT          0x0200
#define STREAM_STATE_CLOSED                0x0800

/*  D A T A   S T R U C T U R E S  **********************************/
typedef struct _FlushMgr
{
    uint32_t   flush_pt;
    uint16_t   last_count;
    uint16_t   last_size;
    uint8_t    flush_policy;
    uint8_t    flush_type;
    uint8_t    auto_disable;
    bool       flush;
    //uint8_t    spare;

} FlushMgr;

typedef struct _FlushConfig
{
    FlushMgr client;
    FlushMgr server;
    //SF_LIST *dynamic_policy;
#ifdef TARGET_BASED
    uint8_t configured;
#endif
} FlushConfig;

#ifndef DYNAMIC_RANDOM_FLUSH_POINTS
typedef struct _FlushPointList
{
    uint8_t    current;
    uint8_t    initialized;

    uint32_t   flush_range;
    uint32_t   flush_base;  /* Set as value - range/2 */
    /* flush_pt is split evently on either side of flush_value, within
     * the flush_range.  flush_pt can be from:
     * (flush_value - flush_range/2) to (flush_value + flush_range/2)
     *
     * For example:
     * flush_value = 192
     * flush_range = 128
     * flush_pt will vary from 128 to 256
     */
    uint32_t *flush_points;

} FlushPointList;
#endif

/**list of ignored rules.
 */
typedef struct _IgnoredRuleList
{
    OptTreeNode *otn;
    struct _IgnoredRuleList *next;
} IgnoredRuleList;

typedef struct _StreamTcpPolicy
{
    uint16_t   policy;
    uint16_t   reassembly_policy;
    uint16_t   flags;
    uint16_t   flush_factor;
    uint32_t   session_timeout;
    uint32_t   max_window;
    uint32_t   overlap_limit;
    uint32_t   hs_timeout;
    IpAddrSet   *bound_addrs;
    FlushConfig flush_config[MAX_PORTS];
#ifdef TARGET_BASED
    FlushConfig flush_config_protocol[MAX_PROTOCOL_ORDINAL];
#endif
#ifndef DYNAMIC_RANDOM_FLUSH_POINTS
    FlushPointList flush_point_list;
#endif
    uint32_t   max_queued_bytes;
    uint32_t   max_queued_segs;

    uint32_t   max_consec_small_segs;
    uint32_t   max_consec_small_seg_size;
    char       small_seg_ignore[MAX_PORTS/8];
    bool       log_asymmetric_traffic;

} StreamTcpPolicy;

typedef struct _StreamTcpConfig
{
    StreamTcpPolicy *default_policy;
    StreamTcpPolicy **policy_list;

    void* paf_config;

    uint8_t num_policies;
    uint16_t session_on_syn;
    uint16_t port_filter[MAX_PORTS + 1];

} StreamTcpConfig;

typedef struct _StreamUdpPolicy
{
    uint32_t   session_timeout;
    uint16_t   flags;
    IpAddrSet   *bound_addrs;

} StreamUdpPolicy;

typedef struct _StreamUdpConfig
{
    StreamUdpPolicy *default_policy;
    StreamUdpPolicy **policy_list;
    uint8_t num_policies;
    uint8_t dummy;  /* For alignment */
    uint16_t port_filter[MAX_PORTS + 1];

} StreamUdpConfig;

typedef struct _StreamIcmpPolicy
{
    uint32_t   session_timeout;
    //uint16_t   flags;

} StreamIcmpPolicy;

typedef struct _StreamIcmpConfig
{
    StreamIcmpPolicy default_policy;
    uint8_t num_policies;

} StreamIcmpConfig;

typedef struct _StreamIpPolicy
{
    uint32_t   session_timeout;

} StreamIpPolicy;

typedef struct _StreamIpConfig
{
    StreamIpPolicy default_policy;

} StreamIpConfig;

typedef struct _StreamConfig
{
    SessionConfiguration *session_config;
    StreamTcpConfig *tcp_config;
    StreamUdpConfig *udp_config;
    StreamIcmpConfig *icmp_config;
    StreamIpConfig *ip_config;

#ifdef TARGET_BASED
    uint8_t service_filter[MAX_PROTOCOL_ORDINAL];
#endif

    bool verified;
    bool swapped;
    bool reload_config;

} StreamConfig;

typedef struct _StreamStats
{
    uint32_t   total_tcp_sessions;
    uint32_t   total_udp_sessions;
    uint32_t   total_icmp_sessions;
    uint32_t   total_ip_sessions;
    uint32_t   tcp_prunes;
    uint32_t   udp_prunes;
    uint32_t   icmp_prunes;
    uint32_t   ip_prunes;
    uint32_t   tcp_timeouts;
    uint32_t   tcp_streamtrackers_created;
    uint32_t   tcp_streamtrackers_released;
    uint32_t   tcp_streamsegs_created;
    uint32_t   tcp_streamsegs_released;
    uint32_t   tcp_rebuilt_packets;
    uint32_t   tcp_rebuilt_seqs_used;
    uint32_t   tcp_overlaps;
    uint32_t   tcp_discards;
    uint32_t   tcp_gaps;
    uint32_t   udp_timeouts;
    uint32_t   udp_sessions_created;
    uint32_t   udp_sessions_released;
    uint32_t   udp_discards;
    uint32_t   icmp_timeouts;
    uint32_t   icmp_sessions_created;
    uint32_t   icmp_sessions_released;
    uint32_t   ip_timeouts;
    uint32_t   events;
    uint32_t   internalEvents;
    uint32_t   active_tcp_sessions;
    uint64_t   active_tcp_memory;
    uint32_t   active_udp_sessions;
    uint32_t   active_icmp_sessions;
    uint32_t   active_ip_sessions;
    uint32_t   icmp_unreachable;
    uint32_t   icmp_unreachable_code4;
    tPortFilterStats  tcp_port_filter;
    tPortFilterStats  udp_port_filter;
} StreamStats;

/**Whether incoming packets should be ignored or processed.
 */
typedef enum {
    /**Ignore the packet. */
    PORT_MONITOR_PACKET_PROCESS = 0,

    /**Process the packet. */
    PORT_MONITOR_PACKET_DISCARD

} PortMonitorPacketStates;

void StreamDisableInspection(SessionControlBlock *scb, Packet *p);

int StreamExpireSession(SessionControlBlock *scb);
int StreamExpire(Packet *p, SessionControlBlock *scb);

#ifdef ACTIVE_RESPONSE
void StreamActiveResponse(Packet*, SessionControlBlock*);
void SetTTL (SessionControlBlock*, Packet*, int client);
#endif

void MarkupPacketFlags(Packet *p, SessionControlBlock *ssn);

#ifdef TARGET_BASED
void setAppProtocolIdFromHostEntry(SessionControlBlock *scb,
                                   HostAttributeEntry *host_entry,
                                   int direction);
#endif
StreamConfig *getStreamPolicyConfig( tSfPolicyId policy_id, bool parsing );
void StreamFreeConfig(StreamConfig *);
void StreamFreeConfigs(tSfPolicyUserContextId);
void StreamCallHandler(Packet*, unsigned id);
void CallFTPFlushProcessor(Packet *);

static inline void StreamResetFlowBits( SessionControlBlock *scb )
{
    StreamFlowData *flowdata;

    if( ( scb == NULL ) || ( scb->flowdata == NULL ) )
        return;

    flowdata = ( StreamFlowData * ) scb->flowdata->data;
    boResetBITOP( &( flowdata->boFlowbits ) );
}


void setPortFilterList( struct _SnortConfig *sc, uint16_t *portList, IpProto protocol,
                        int ignoreAnyAnyRules, tSfPolicyId policyId );
int StreamAnyAnyFlow( uint16_t *portList, OptTreeNode *otn, RuleTreeNode *rtn, int any_any_flow,
                      IgnoredRuleList **ppIgnoredRuleList, int ignoreAnyAnyRules );
void s5PrintPortFilter( uint16_t portList[] );
int StreamSetRuntimeConfiguration( SessionControlBlock *scb, uint8_t protocol );
bool getStreamIgnoreAnyConfig (struct _SnortConfig *sc, IpProto protocol);

// shared stream state
extern StreamStats s5stats;
extern uint32_t firstPacketTime;
extern MemPool s5FlowMempool;

extern uint32_t session_mem_in_use;
extern SessionConfiguration *stream_session_config;
extern tSfPolicyUserContextId stream_online_config;
extern tSfPolicyUserContextId stream_parsing_config;
extern tSfActionQueueId decoderActionQ;

void StreamDeleteSession(SessionControlBlock *scb);

#endif /* STREAM_COMMON_H_ */
