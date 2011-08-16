/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
#ifndef STREAM5_COMMON_H_
#define STREAM5_COMMON_H_

#include <sys/types.h>
#ifndef WIN32
#include <netinet/in.h>
#endif

#include "sfutil/bitop_funcs.h"
#include "parser/IpAddrSet.h"

#include "stream_api.h"
#include "mempool.h"
#include "sf_types.h"

#ifdef TARGET_BASED
#include "target-based/sftarget_hostentry.h"
#endif

#include "sfPolicy.h"
#include "sfPolicyUserData.h"

//#define DEBUG_STREAM5 DEBUG

/* Only track a maximum number of alerts per session */
#define MAX_SESSION_ALERTS 8

/* defaults and limits */
#define S5_DEFAULT_SSN_TIMEOUT  30        /* seconds to timeout a session */
#define S5_MAX_SSN_TIMEOUT      3600*24   /* max timeout (approx 1 day) */
#define S5_MIN_SSN_TIMEOUT      1         /* min timeout (1 second) */
#define S5_MIN_ALT_HS_TIMEOUT   0         /* min timeout (0 seconds) */
#define S5_TRACK_YES            1
#define S5_TRACK_NO             0
#define S5_MAX_MAX_WINDOW       0x3FFFc000 /* max window allowed by TCP */
                                           /* 65535 << 14 (max wscale) */
#define S5_MIN_MAX_WINDOW       0

#define S5_DEFAULT_MAX_QUEUED_BYTES 1048576 /* 1 MB */
#define S5_MIN_MAX_QUEUED_BYTES 1024       /* Don't let this go below 1024 */
#define S5_MAX_MAX_QUEUED_BYTES 0x40000000 /* 1 GB, most we could reach within
                                            * largest window scale */
#define AVG_PKT_SIZE            400
#define S5_DEFAULT_MAX_QUEUED_SEGS (S5_DEFAULT_MAX_QUEUED_BYTES/AVG_PKT_SIZE)
#define S5_MIN_MAX_QUEUED_SEGS  2          /* Don't let this go below 2 */
#define S5_MAX_MAX_QUEUED_SEGS  0x40000000 /* 1 GB worth of one-byte segments */

#define S5_DEFAULT_MAX_SMALL_SEG_SIZE 0    /* disabled */
#define S5_MAX_MAX_SMALL_SEG_SIZE 2048     /* 2048 bytes in single packet, uh, not small */
#define S5_MIN_MAX_SMALL_SEG_SIZE 0        /* 0 means disabled */

#define S5_DEFAULT_CONSEC_SMALL_SEGS 0     /* disabled */
#define S5_MAX_CONSEC_SMALL_SEGS 2048      /* 2048 single byte packets without acks is alot */
#define S5_MIN_CONSEC_SMALL_SEGS 0         /* 0 means disabled */

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
#define STREAM_POLICY_DEFAULT   STREAM_POLICY_BSD

#define STREAM5_CONFIG_STATEFUL_INSPECTION      0x00000001
#define STREAM5_CONFIG_ENABLE_ALERTS            0x00000002
#define STREAM5_CONFIG_LOG_STREAMS              0x00000004
#define STREAM5_CONFIG_REASS_CLIENT             0x00000008
#define STREAM5_CONFIG_REASS_SERVER             0x00000010
#define STREAM5_CONFIG_ASYNC                    0x00000020
#define STREAM5_CONFIG_SHOW_PACKETS             0x00000040
#define STREAM5_CONFIG_FLUSH_ON_ALERT           0x00000080
#define STREAM5_CONFIG_REQUIRE_3WHS             0x00000100
#define STREAM5_CONFIG_MIDSTREAM_DROP_NOALERT   0x00000200
#define STREAM5_CONFIG_IGNORE_ANY               0x00000400
#define STREAM5_CONFIG_PERFORMANCE              0x00000800
#define STREAM5_CONFIG_STATIC_FLUSHPOINTS       0x00001000
#define STREAM5_CONFIG_IPS                      0x00002000
#define STREAM5_CONFIG_CHECK_SESSION_HIJACKING  0x00004000
#define STREAM5_CONFIG_NO_ASYNC_REASSEMBLY      0x00008000

/* traffic direction identification */
#define FROM_SERVER     0
#define FROM_RESPONDER  0
#define FROM_CLIENT     1
#define FROM_SENDER     1

#define STREAM5_STATE_NONE                  0x0000
#define STREAM5_STATE_SYN                   0x0001
#define STREAM5_STATE_SYN_ACK               0x0002
#define STREAM5_STATE_ACK                   0x0004
#define STREAM5_STATE_ESTABLISHED           0x0008
#define STREAM5_STATE_DROP_CLIENT           0x0010
#define STREAM5_STATE_DROP_SERVER           0x0020
#define STREAM5_STATE_MIDSTREAM             0x0040
#define STREAM5_STATE_TIMEDOUT              0x0080
#define STREAM5_STATE_UNREACH               0x0100
#define STREAM5_STATE_SENDER_SEEN           0x0200
#define STREAM5_STATE_RECEIVER_SEEN         0x0400
#define STREAM5_STATE_CLOSED                0x0800

#define TCP_HZ          100

/*  D A T A   S T R U C T U R E S  **********************************/
typedef struct _SessionKey
{
/* XXX If this data structure changes size, HashKeyCmp must be updated! */
#ifdef SUP_IP6
    uint32_t   ip_l[4]; /* Low IP */
    uint32_t   ip_h[4]; /* High IP */
#else
    uint32_t   ip_l; /* Low IP */
    uint32_t   ip_h; /* High IP */
#endif
    uint16_t   port_l; /* Low Port - 0 if ICMP */
    uint16_t   port_h; /* High Port - 0 if ICMP */
    uint16_t   vlan_tag;
    char        protocol;
    char        pad;
#ifdef MPLS
    uint32_t   mplsLabel; /* MPLS label */
    uint32_t   mplsPad;
#endif
/* XXX If this data structure changes size, HashKeyCmp must be updated! */
} SessionKey;

typedef struct _Stream5AppData
{
    uint32_t   protocol;
    void        *dataPointer;
    struct _Stream5AppData *next;
    struct _Stream5AppData *prev;
    StreamAppDataFree freeFunc;
} Stream5AppData;

typedef struct _Stream5AlertInfo
{
    /* For storing alerts that have already been seen on the session */
    uint32_t sid;
    uint32_t gid;
    uint32_t seq;
} Stream5AlertInfo;

// this struct is organized by member size for compactness
typedef struct _Stream5LWSession
{
    SessionKey key;

    snort_ip   client_ip;
    snort_ip   server_ip;

    MemBucket  *proto_specific_data;
    Stream5AppData *appDataList;

    MemBucket *flowdata; /* add flowbits */

    void *policy;

    long       last_data_seen;
    uint64_t   expire_time;

    tSfPolicyUserContextId config;
    tSfPolicyId policy_id;

    uint32_t   session_flags;

    uint16_t   session_state;

    uint16_t   client_port;
    uint16_t   server_port;

#ifdef TARGET_BASED
    int16_t    ipprotocol;
    int16_t    application_protocol;
#endif

    char       protocol;
    char       direction;
    char       ignore_direction; /* flag to ignore traffic on this session */

#ifdef ACTIVE_RESPONSE
    uint8_t    response_count;
#endif

    uint8_t    inner_client_ttl, inner_server_ttl;
    uint8_t    outer_client_ttl, outer_server_ttl;

} Stream5LWSession;

typedef struct _Stream5GlobalConfig
{
    char       disabled;
    char       track_tcp_sessions;
    char       track_udp_sessions;
    char       track_icmp_sessions;
    uint32_t   max_tcp_sessions;
    uint32_t   max_udp_sessions;
    uint32_t   max_icmp_sessions;
    uint32_t   memcap;
    uint32_t   prune_log_max;
    uint32_t   flags;

#ifdef ACTIVE_RESPONSE
    uint32_t   min_response_seconds;
    uint8_t    max_active_responses;
#endif

} Stream5GlobalConfig;

typedef struct _FlushMgr
{
    uint32_t   flush_pt;
    uint16_t   last_count;
    uint16_t   last_size;
    uint8_t    flush_policy;

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

typedef struct _Stream5TcpPolicy
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

} Stream5TcpPolicy;

typedef struct _Stream5TcpConfig
{
    Stream5TcpPolicy *default_policy;
    Stream5TcpPolicy **policy_list;
    uint8_t num_policies;
    uint8_t port_filter[MAX_PORTS + 1];

} Stream5TcpConfig;

typedef struct _Stream5UdpPolicy
{
    uint32_t   session_timeout;
    uint16_t   flags;
    IpAddrSet   *bound_addrs;

} Stream5UdpPolicy;

typedef struct _Stream5UdpConfig
{
    Stream5UdpPolicy *default_policy;
    Stream5UdpPolicy **policy_list;
    uint8_t num_policies;
    uint8_t port_filter[MAX_PORTS + 1];

} Stream5UdpConfig;

typedef struct _Stream5IcmpPolicy
{
    uint32_t   session_timeout;
    //uint16_t   flags;

} Stream5IcmpPolicy;

typedef struct _Stream5IcmpConfig
{
    Stream5IcmpPolicy default_policy;
    uint8_t num_policies;

} Stream5IcmpConfig;

typedef struct _Stream5Config
{
    Stream5GlobalConfig *global_config;
    Stream5TcpConfig *tcp_config;
    Stream5UdpConfig *udp_config;
    Stream5IcmpConfig *icmp_config;

#ifdef TARGET_BASED
    uint8_t service_filter[MAX_PROTOCOL_ORDINAL];
#endif

    uint32_t ref_count;

} Stream5Config;

/**Common statistics for tcp and udp packets, maintained by port filtering.
 */
typedef struct {
    /**packets dropped without further processing by any preprocessor or
     * detection engine.
     */
    uint32_t  dropped;

    /**packets inspected and but processed futher by stream5 preprocessor.
     */
    uint32_t  inspected;

    /**packets session tracked by stream5 preprocessor.
     */
    uint32_t  session_tracked;
} tPortFilterStats;

typedef struct _Stream5Stats
{
    uint32_t   total_tcp_sessions;
    uint32_t   total_udp_sessions;
    uint32_t   total_icmp_sessions;
    uint32_t   tcp_prunes;
    uint32_t   udp_prunes;
    uint32_t   icmp_prunes;
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
    uint32_t   events;
    uint32_t   internalEvents;
    tPortFilterStats  tcp_port_filter;
    tPortFilterStats  udp_port_filter;
} Stream5Stats;

/**Whether incoming packets should be ignored or processed.
 */
typedef enum { 
    /**Ignore the packet. */
    PORT_MONITOR_PACKET_PROCESS = 0,

    /**Process the packet. */
    PORT_MONITOR_PACKET_DISCARD

} PortMonitorPacketStates;

extern Stream5Stats s5stats;
extern uint32_t firstPacketTime;
extern MemPool s5FlowMempool;

void Stream5DisableInspection(Stream5LWSession *lwssn, Packet *p);

int Stream5Expire(Packet *p, Stream5LWSession *ssn);
void Stream5SetExpire(Packet *p, Stream5LWSession *ssn, uint32_t timeout);

#ifdef ACTIVE_RESPONSE
int Stream5GetExpire(Packet*, Stream5LWSession*);
void Stream5ActiveResponse(Packet*, Stream5LWSession*);
void SetTTL (Stream5LWSession*, Packet*, int client);
#endif

void MarkupPacketFlags(Packet *p, Stream5LWSession *ssn);

#ifdef TARGET_BASED
void Stream5SetApplicationProtocolIdFromHostEntry(Stream5LWSession *lwssn,
                                           HostAttributeEntry *host_entry,
                                           int direction);
#endif
void Stream5FreeConfig(Stream5Config *);
void Stream5FreeConfigs(tSfPolicyUserContextId);

int isPacketFilterDiscard(
        Packet *p,
        int ignore_any_rules
        );

static INLINE void Stream5ResetFlowBits(Stream5LWSession *lwssn)
{
    StreamFlowData *flowdata;

    if ((lwssn == NULL) || (lwssn->flowdata == NULL))
        return;

    flowdata = (StreamFlowData *)lwssn->flowdata->data;
    boResetBITOP(&(flowdata->boFlowbits));
}

#endif /* STREAM5_COMMON_H_ */
