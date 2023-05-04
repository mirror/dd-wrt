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
 * @file    snort_stream_tcp.c
 * @author  Martin Roesch <roesch@sourcefire.com>
 * @author  Steven Sturges <ssturges@sourcefire.com>
 *
 */

/*
 * TODOs:
 * - midstream ssn pickup (done, SAS 10/14/2005)
 * - syn flood protection (done, SAS 9/27/2005)
 *
 * - review policy anomaly detection
 *   + URG pointer (TODO)
 *   + data on SYN (done, SAS 10/12/2005)
 *   + data on FIN (done, SAS 10/12/2005)
 *   + data after FIN (done, SAS 10/13/2005)
 *   + window scaling/window size max (done, SAS 10/13/2005)
 *   + PAWS, TCP Timestamps (done, SAS 10/12/2005)
 *
 * - session shutdown/Reset handling (done, SAS)
 * - flush policy for Window/Consumed
 * - limit on number of overlapping packets (done, SAS)
 */

#include <errno.h>
#include <assert.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "perf.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "detect.h"
#include "plugbase.h"
#include "mstring.h"
#include "sfxhash.h"
#include "util.h"
#include "sflsq.h"
#include "snort_bounds.h"
#include "generators.h"
#include "event_queue.h"
#include "snort.h"
#include "memory_stats.h"
#include "parser/IpAddrSet.h"

#include "decode.h"
#include "encode.h"
#include "log.h"
#include "active.h"
#include "spp_normalize.h"

#include "sf_sdlist.h"

#include "spp_session.h"
#include "session_api.h"
#include "snort_session.h"

#include "stream_common.h"
#include "snort_stream_tcp.h"
#include "stream_api.h"
#include "session_expect.h"
#include "stream_paf.h"
#include "stream5_ha.h"

#ifdef TARGET_BASED
#include "sftarget_protocol_reference.h"
#include "sftarget_hostentry.h"
#endif

#include "profiler.h"

#include "ipv6_port.h"
#include "sf_iph.h"

#include "sp_preprocopt.h"
#include "sfPolicy.h"
#include "sfActionQueue.h"
#include "detection_util.h"
#include "file_api.h"

#ifdef REG_TEST
#include "reg_test.h"
#include <stdio.h>
#endif

// port reassembly registration utils
static void StreamCreateReassemblyPortList( void );
static void StreamDestoryReassemblyPortList( void );

#ifdef PERF_PROFILING
PreprocStats s5TcpPerfStats;
PreprocStats s5TcpNewSessPerfStats;
PreprocStats s5TcpStatePerfStats;
PreprocStats s5TcpDataPerfStats;
PreprocStats s5TcpInsertPerfStats;
PreprocStats s5TcpPAFPerfStats;
PreprocStats s5TcpFlushPerfStats;
PreprocStats s5TcpBuildPacketPerfStats;
PreprocStats s5TcpProcessRebuiltPerfStats;

PreprocStats streamSizePerfStats;
PreprocStats streamReassembleRuleOptionPerfStats;
extern PreprocStats preprocRuleOptionPerfStats;

#endif

/*  M A C R O S  **************************************************/

/* TCP flags */
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_NORESERVED (TH_FIN|TH_SYN|TH_RST|TH_PUSH|TH_ACK|TH_URG)

/* TCP states */
#define TCP_STATE_NONE         0
#define TCP_STATE_LISTEN       1
#define TCP_STATE_SYN_RCVD     2
#define TCP_STATE_SYN_SENT     3
#define TCP_STATE_ESTABLISHED  4
#define TCP_STATE_CLOSE_WAIT   5
#define TCP_STATE_LAST_ACK     6
#define TCP_STATE_FIN_WAIT_1   7
#define TCP_STATE_CLOSING      8
#define TCP_STATE_FIN_WAIT_2   9
#define TCP_STATE_TIME_WAIT   10
#define TCP_STATE_CLOSED      11

#ifndef MIN
# define MIN(a,b)  (((a)<(b)) ? (a):(b))
#endif
#ifndef MAX
# define MAX(a,b)  (((a)>(b)) ? (a):(b))
#endif

#define PAWS_WINDOW         60
#define PAWS_24DAYS         2147483647  /*  2^ 31 -1 - approx 24 days in milli secs*/

/* for state transition queuing */
#define CHK_SEQ         0
#define NO_CHK_SEQ      1

#define STREAM_UNALIGNED       0
#define STREAM_ALIGNED         1

/* actions */
#define ACTION_NOTHING                  0x00000000
#define ACTION_FLUSH_SENDER_STREAM      0x00000001
#define ACTION_FLUSH_RECEIVER_STREAM    0x00000002
#define ACTION_DROP_SESSION             0x00000004
#define ACTION_ACK_SENDER_DATA          0x00000008
#define ACTION_ACK_RECEIVER_DATA        0x00000010
#define ACTION_SET_SSN                  0x00000040
#define ACTION_COMPLETE_TWH             0x00000080
#define ACTION_RST                      0x00000100
#define ACTION_BAD_SEQ                  0x00000200
#define ACTION_BAD_PKT                  0x00000400
#define ACTION_LWSSN_CLOSED             0x00000800
#define ACTION_DISABLE_INSPECTION       0x00001000

/* events */
#define EVENT_SYN_ON_EST                0x00000001
#define EVENT_DATA_ON_SYN               0x00000002
#define EVENT_DATA_ON_CLOSED            0x00000004
#define EVENT_BAD_TIMESTAMP             0x00000008
#define EVENT_BAD_SEGMENT               0x00000010
#define EVENT_WINDOW_TOO_LARGE          0x00000020
#define EVENT_EXCESSIVE_TCP_OVERLAPS    0x00000040
#define EVENT_DATA_AFTER_RESET          0x00000080
#define EVENT_SESSION_HIJACK_CLIENT     0x00000100
#define EVENT_SESSION_HIJACK_SERVER     0x00000200
#define EVENT_DATA_WITHOUT_FLAGS        0x00000400
#define EVENT_4WHS                      0x00000800
#define EVENT_NO_TIMESTAMP              0x00001000
#define EVENT_BAD_RST                   0x00002000
#define EVENT_BAD_FIN                   0x00004000
#define EVENT_BAD_ACK                   0x00008000
#define EVENT_DATA_AFTER_RST_RCVD       0x00010000
#define EVENT_WINDOW_SLAM               0x00020000
#define EVENT_WIN_SZ_0_TCP_FIN_WAIT_1   0x00040000

#define TF_NONE                     0x0000
#define TF_WSCALE                   0x0001
#define TF_TSTAMP                   0x0002
#define TF_TSTAMP_ZERO              0x0004
#define TF_MSS                      0x0008
#define TF_FORCE_FLUSH              0x0010
#define TF_PKT_MISSED               0x0020  // sticky
#define TF_MISSING_PKT              0x0040  // used internally
#define TF_MISSING_PREV_PKT         0x0080  // reset for each reassembled
#define TF_FIRST_PKT_MISSING        0x0100
#define TF_ALL                      0xFFFF

#define STREAM_INSERT_OK            0
#define STREAM_INSERT_ANOMALY       1
#define STREAM_INSERT_TIMEOUT       2
#define STREAM_INSERT_FAILED        3

#define STREAM_DEFAULT_TCP_PACKET_MEMCAP  8388608  /* 8MB */
#define STREAM_MIN_OVERLAP_LIMIT 0
#define STREAM_MAX_OVERLAP_LIMIT 255
#define STREAM_MAX_FLUSH_FACTOR 2048

#define REASSEMBLY_POLICY_FIRST     1
#define REASSEMBLY_POLICY_LINUX     2
#define REASSEMBLY_POLICY_BSD       3
#define REASSEMBLY_POLICY_OLD_LINUX 4
#define REASSEMBLY_POLICY_LAST      5
#define REASSEMBLY_POLICY_WINDOWS   6
#define REASSEMBLY_POLICY_SOLARIS   7
#define REASSEMBLY_POLICY_HPUX11    8
#define REASSEMBLY_POLICY_IRIX      9
#define REASSEMBLY_POLICY_MACOS     10
#define REASSEMBLY_POLICY_HPUX10    11
#define REASSEMBLY_POLICY_VISTA     12
#define REASSEMBLY_POLICY_WINDOWS2K3 13
#define REASSEMBLY_POLICY_NOACK      14
#define REASSEMBLY_POLICY_DEFAULT   REASSEMBLY_POLICY_BSD

#define SUB_SYN_SENT  0x01
#define SUB_ACK_SENT  0x02
#define SUB_SETUP_OK  0x03
#define SUB_RST_SENT  0x04
#define SUB_FIN_SENT  0x08

// flush types
#define STREAM_FT_INTERNAL  0  // normal s5 "footprint"
#define STREAM_FT_EXTERNAL  1  // set by other preprocessor
#define STREAM_FT_PAF_MAX   2  // paf_max + footprint fp

#define SLAM_MAX 4

/* Only track a maximum number of alerts per session */
#define MAX_SESSION_ALERTS 8

//#define STREAM_DEBUG_ENABLED
#ifdef STREAM_DEBUG_ENABLED
#define STREAM_DEBUG_WRAP(x) DEBUG_WRAP(x)
#else
#define STREAM_DEBUG_WRAP(x)
#endif

/* client/server ip/port dereference */
#define tcp_client_ip scb->client_ip
#define tcp_client_port scb->client_port
#define tcp_server_ip scb->server_ip
#define tcp_server_port scb->server_port

static uint32_t pkt_snaplen = 0;
#define PKT_SNAPLEN 1514

/*  D A T A  S T R U C T U R E S  ***********************************/
typedef struct _TcpDataBlock
{
    uint32_t   seq;
    uint32_t   ack;
    uint32_t   win;
    uint32_t   end_seq;
    uint32_t   ts;
} TcpDataBlock;

typedef struct _StateMgr
{
    uint8_t    state;
    uint8_t    sub_state;
    uint8_t    state_queue;
    uint8_t    expected_flags;
    uint32_t   transition_seq;
    uint32_t   stq_get_seq;
} StateMgr;

#define RAND_FLUSH_POINTS 64

//-------------------------------------------------------------------------
// extra, extra - read all about it!
// -- u2 is the only output plugin that currently supports extra data
// -- extra data may be captured before or after alerts
// -- extra data may be per packet or persistent (saved on session)
//
// -- per packet extra data is logged iff we alert on the packet
//    containing the extra data - u2 drives this
// -- an extra data mask is added to Packet to indicate when per packet
//    extra data is available
//
// -- persistent extra data must be logged exactly once for each alert
//    regardless of capture/alert ordering - s5 purge_alerts drives this
// -- an extra data mask is added to the session trackers to indicate that
//    persistent extra data is available
//
// -- event id and second are added to the session alert trackers so that
//    the extra data can be correlated with events
// -- event id and second are not available when StreamAddSessionAlertTcp
//    is called; u2 calls StreamUpdateSessionAlertTcp as events are logged
//    to set these fields
//-------------------------------------------------------------------------

typedef struct _StreamAlertInfo
{
    /* For storing alerts that have already been seen on the session */
    uint32_t sid;
    uint32_t gid;
    uint32_t seq;
    // if we log extra data, event_* is used to correlate with alert
    uint32_t event_id;
    uint32_t event_second;
} StreamAlertInfo;

//-----------------------------------------------------------------
// we make a lot of StreamSegments, StreamTrackers, and TcpSessions
// so they are organized by member size/alignment requirements to
// minimize unused space in the structs.
// ... however, use of padding below is critical, adjust if needed
//-----------------------------------------------------------------

typedef struct _StreamSegment
{
    uint8_t    *data;
    uint8_t    *payload;

    struct _StreamSegment *prev;
    struct _StreamSegment *next;

#ifdef DEBUG
    int ordinal;
#endif
    struct timeval tv;
    uint32_t caplen;
    uint32_t pktlen;

    uint32_t   ts;
    uint32_t   seq;

    uint16_t   orig_dsize;
    uint16_t   size;

    uint16_t   urg_offset;

    uint8_t    buffered;

    // this sequence ensures 4-byte alignment of iph in pkt
    // (only significant if we call the grinder)
    uint8_t    pad1;
    uint16_t   pad2;
    uint8_t    pkt[1];  // variable length

} StreamSegment;

typedef struct _StreamTracker
{
    StateMgr  s_mgr;        /* state tracking goodies */
    FlushMgr  flush_mgr;    /* please flush twice, it's a long way to
                             * the bitbucket... */

    // this is intended to be private to s5_paf but is included
    // directly to avoid the need for allocation; do not directly
    // manipulate within this module.
    PAF_State paf_state;    // for tracking protocol aware flushing

    StreamAlertInfo alerts[MAX_SESSION_ALERTS]; /* history of alerts */

    StreamTcpPolicy *tcp_policy;
    StreamSegment *seglist;       /* first queued segment */
    StreamSegment *seglist_tail;  /* last queued segment */

    // TBD move out of here since only used per packet?
    StreamSegment* seglist_next;  /* next queued segment to flush */

    StreamSegment *held_segment;  /* blocked segment of http connection */
#ifdef DEBUG
    int segment_ordinal;
#endif
    /* Local in the context of these variables means the local part
     * of the connection.  For example, if this particular StreamTracker
     * was tracking the client side of a connection, the l_unackd value
     * would represent the client side of the connection's last unacked
     * sequence number
     */
    uint32_t l_unackd;     /* local unack'd seq number */
    uint32_t l_nxt_seq;    /* local next expected sequence */
    uint32_t l_window;     /* local receive window */

    uint32_t r_nxt_ack;    /* next expected ack from remote side */
    uint32_t r_win_base;   /* remote side window base sequence number
                            * (i.e. the last ack we got) */
    uint32_t isn;          /* initial sequence number */
    uint32_t ts_last;      /* last timestamp (for PAWS) */
    uint32_t ts_last_pkt;  /* last packet timestamp we got */

    uint32_t seglist_base_seq;   /* seq of first queued segment */
    uint32_t seg_count;          /* number of current queued segments */
    uint32_t seg_bytes_total;    /* total bytes currently queued */
    uint32_t seg_bytes_logical;  /* logical bytes queued (total - overlaps) */
    uint32_t total_bytes_queued; /* total bytes queued (life of session) */
    uint32_t total_segs_queued;  /* number of segments queued (life) */
    uint32_t overlap_count;      /* overlaps encountered */
    uint32_t small_seg_count;
    uint32_t flush_count;        /* number of flushed queued segments */
    uint32_t xtradata_mask;      /* extra data available to log */

    uint16_t os_policy;
    uint16_t reassembly_policy;

    uint16_t wscale;       /* window scale setting */
    uint16_t mss;          /* max segment size */
    uint16_t  flags;        /* bitmap flags (TF_xxx) */

    uint8_t  mac_addr[6];

    uint8_t  alert_count;  /* number alerts stored (up to MAX_SESSION_ALERTS) */

} StreamTracker;

typedef struct _TcpSession
{
    StreamTracker client;
    StreamTracker server;
    SessionControlBlock *scb;

#ifdef DEBUG
    struct timeval ssn_time;
#endif

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    int32_t ingress_index;  /* Index of the inbound interface. */
    int32_t egress_index;   /* Index of the outbound interface. */
    int32_t ingress_group;  /* Index of the inbound group. */
    int32_t egress_group;   /* Index of the outbound group. */
    uint32_t daq_flags;     /* Flags for the packet (DAQ_PKT_FLAG_*) */
    void *priv_ptr;         /* private data pointer. used in pinhole */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t address_space_id_src;
    uint16_t address_space_id_dst;
#else
    uint16_t address_space_id;
#endif
#ifdef HAVE_DAQ_FLOW_ID
    uint32_t daq_flow_id;
#endif
#endif
    uint8_t ecn;
    bool session_decrypted;
    uint8_t pp_flags;
} TcpSession;

 #ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    TcpSession initialization ={
                                 .ingress_index = DAQ_PKTHDR_UNKNOWN,
                                 .egress_index = DAQ_PKTHDR_UNKNOWN,
                                 .ingress_group = DAQ_PKTHDR_UNKNOWN,
                                 .egress_group = DAQ_PKTHDR_UNKNOWN 
                               };
  
 #endif

#define SL_BUF_FLUSHED 1

static inline int SetupOK (const StreamTracker* st)
{
    return ( (st->s_mgr.sub_state & SUB_SETUP_OK) == SUB_SETUP_OK );
}

static inline int Stream_NormGetMode(uint16_t reassembly_policy, const SnortConfig* sc, NormFlags nf)
{
    if ( reassembly_policy == REASSEMBLY_POLICY_NOACK )
        return NORM_MODE_OFF;
    return Normalize_GetMode(sc, nf);
}

static inline uint32_t SegsToFlush (const StreamTracker* st, unsigned max)
{
    uint32_t n = st->seg_count - st->flush_count;
    StreamSegment* s;

    if ( !n || max == 1 )
        return n;

    n = 0;
    s = st->seglist;

    while ( s )
    {
        if ( !s->buffered && SEQ_LT(s->seq, st->r_win_base) )
            n++;

        if ( max && n == max )
            return n;

        s = s->next;
    }
    return n;
}

static inline bool DataToFlush (const StreamTracker* st)
{
    if ( (st->flush_mgr.flush_policy == STREAM_FLPOLICY_PROTOCOL)
#ifdef NORMALIZER
            || (st->flush_mgr.flush_policy == STREAM_FLPOLICY_PROTOCOL_IPS)
#endif
            || st->flush_mgr.flush_policy == STREAM_FLPOLICY_PROTOCOL_NOACK
            || st->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_NOACK
       )
        return ( SegsToFlush(st, 1) > 0 );

#ifdef NORMALIZER
    if ((st->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS) || (st->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS_FTP))
        return ( SegsToFlush(st, 1) > 1 );
#endif

    return ( SegsToFlush(st, 2) > 1 );
}

#ifdef REG_TEST
int default_ports[] =
{
    21, 23, 25, 42, 53, 80, 110, 111, 135, 136, 137, 139, 143, 445,
    513, 514, 1433, 1521, 2401, 3306
};
#define DEFAULT_PORTS_SIZE (int)(sizeof(default_ports)/sizeof(int))
#else
int *default_ports = 0;
#define DEFAULT_PORTS_SIZE 0
#endif

#ifdef TARGET_BASED
char *default_protocols[] =
{
    "ftp", "telnet", "smtp", "nameserver", "dns", "http", "pop3", "sunrpc",
    "dcerpc", "netbios-ssn", "imap", "login", "shell", "mssql", "oracle", "cvs",
    "mysql"
};
#endif

FlushConfig ignore_flush_policy[MAX_PORTS];
#ifdef TARGET_BASED
FlushConfig ignore_flush_policy_protocol[MAX_PROTOCOL_ORDINAL];
#endif

/*  P R O T O T Y P E S  ********************************************/
static void StreamParseTcpArgs(struct _SnortConfig *, StreamTcpConfig *, char *, StreamTcpPolicy *);
static void StreamPrintTcpConfig(StreamTcpPolicy *);

static void StreamInitPacket();
static inline void SetupTcpDataBlock(TcpDataBlock *, Packet *);
static int ProcessTcp(SessionControlBlock *, Packet *, TcpDataBlock *,
        StreamTcpPolicy *, SFXHASH_NODE *);
#if OLD_CODE_NOLONGER_USED_DEPENDS_ON_CURRENT_STATE
static inline void QueueState(uint8_t, StreamTracker*, uint8_t,
        uint32_t, uint8_t);
static inline int EvalStateQueue(StreamTracker *, uint8_t, uint32_t);
#endif
static inline int CheckFlushPolicyOnData(
        StreamTcpConfig *config, TcpSession *, StreamTracker *,
        StreamTracker *, TcpDataBlock *, Packet *);
static inline int CheckFlushPolicyOnAck(
        StreamTcpConfig *config, TcpSession *, StreamTracker *,
        StreamTracker *, TcpDataBlock *, Packet *);
static void StreamSeglistAddNode(StreamTracker *, StreamSegment *,
        StreamSegment *);
static int StreamSeglistDeleteNode(StreamTracker *, StreamSegment *);
static int StreamSeglistDeleteNodeTrim(StreamTracker*, StreamSegment*, uint32_t flush_seq);
static int AddStreamNode(StreamTracker *st, Packet *p,
        TcpDataBlock*,
        TcpSession *tcpssn,
        uint16_t len,
        uint32_t slide,
        uint32_t trunc,
        uint32_t seq,
        StreamSegment *left,
        StreamSegment **retSeg);
static int DupStreamNode(
        Packet*,
        StreamTracker*,
        StreamSegment* left,
        StreamSegment** retSeg);

static uint32_t StreamGetWscale(Packet *, uint16_t *);
static uint32_t StreamPacketHasWscale(Packet *);
static uint32_t StreamGetMss(Packet *, uint16_t *);
static uint32_t StreamGetTcpTimestamp(Packet *, uint32_t *, int strip);
static int FlushStream(
        Packet*, StreamTracker *st, uint32_t toSeq, uint8_t *flushbuf,
        const uint8_t *flushbuf_end);
static void TcpSessionCleanup(SessionControlBlock *ssn, int freeApplicationData);
static void TcpSessionCleanupWithFreeApplicationData(void *ssn);
static void FlushQueuedSegs(SessionControlBlock *ssn, TcpSession *tcpssn);

int s5TcpStreamSizeInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr);
int s5TcpStreamSizeEval(void *p, const uint8_t **cursor, void *dataPtr);
void s5TcpStreamSizeCleanup(void *dataPtr);
int s5TcpStreamReassembleRuleOptionInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr);
int s5TcpStreamReassembleRuleOptionEval(void *p, const uint8_t **cursor, void *dataPtr);
void s5TcpStreamReassembleRuleOptionCleanup(void *dataPtr);
static inline void ResetFlushMgrs(void);
static void targetPolicyIterate(void (*callback)(int));
static void policyDecoderFlagsSaveNClear(int policyId);
static void policyDecoderFlagsRestore(int policyId);
static void policyChecksumFlagsSaveNClear(int policyId);
static void policyChecksumFlagsRestore(int policyId);
static inline uint16_t GetTcpReassemblyPolicy(int os_policy);

/*  G L O B A L S  **************************************************/
static SessionCache* tcp_lws_cache = NULL;
static Packet *s5_pkt = NULL;
static Packet *tcp_cleanup_pkt = NULL;
static const uint8_t *s5_pkt_end = NULL;
static char midstream_allowed = 0;
static Packet *wire_packet = NULL;
static uint8_t flush_policy_for_dir = 0;

/* enum for policy names */
static char *reassembly_policy_names[] = {
    "no policy!",
    "FIRST",
    "LINUX",
    "BSD",
    "OLD LINUX",
    "LAST",
    "WINDOWS",
    "SOLARIS",
    "HPUX11",
    "IRIX",
    "MACOS",
    "HPUX10",
    "WINDOWS VISTA",
    "WINDOWS 2003"
        "IPS"
};

#ifdef STREAM_DEBUG_ENABLED
static char *state_names[] = {
    "NONE",
    "LISTEN",
    "SYN_RCVD",
    "SYN_SENT",
    "ESTABLISHED",
    "CLOSE_WAIT",
    "LAST_ACK",
    "FIN_WAIT_1",
    "CLOSING",
    "FIN_WAIT_2",
    "TIME_WAIT",
    "CLOSED"
};
#endif

static char *flush_policy_names[] = {
    "None",
    "Footprint",
    "Logical",
    "Response",
    "Sliding Window",
#if 0
    "Consumed",
#endif
    "Ignore",
    "Protocol",
    "Footprint-IPS",
    "Protocol-IPS",
    "Footprint-NOACK",
    "Protocol-NOACK",
    "Footprint-IPS-FTP",
    "Disabled"
};

static int s5_tcp_cleanup = 0;

static uint32_t g_static_points[RAND_FLUSH_POINTS] =
{ 128, 217, 189, 130, 240, 221, 134, 129,
    250, 232, 141, 131, 144, 177, 201, 130,
    230, 190, 177, 142, 130, 200, 173, 129,
    250, 244, 174, 151, 201, 190, 180, 198,
    220, 201, 142, 185, 219, 129, 194, 140,
    145, 191, 197, 183, 199, 220, 231, 245,
    233, 135, 143, 158, 174, 194, 200, 180,
    201, 142, 153, 187, 173, 199, 143, 201 };

/*  F U N C T I O N S  **********************************************/
static inline uint32_t GenerateFlushPoint(FlushPointList *flush_point_list)
{
    return (rand() % flush_point_list->flush_range) + flush_point_list->flush_base;
}

static inline void InitFlushPointList(FlushPointList *flush_point_list, uint32_t value, uint32_t range, int use_static)
{
    uint32_t i;
    uint32_t flush_range = range;
    uint32_t flush_base = value - range/2;

    if (!flush_point_list)
        return;

    if (!flush_point_list->initialized)
    {
#ifdef REG_TEST
        const char* sfp = getenv("S5_FPT");
        // no error checking required - atoi() is sufficient
        uint32_t cfp = sfp ? atoi(sfp) : 0;
        if ( cfp < 128 || cfp > 255 ) cfp = 192;
#else
        const uint32_t cfp = 192;
#endif

        flush_point_list->flush_range = flush_range;
        flush_point_list->flush_base = flush_base;
#ifndef DYNAMIC_RANDOM_FLUSH_POINTS
        flush_point_list->current = 0;

        flush_point_list->flush_points = SnortPreprocAlloc(RAND_FLUSH_POINTS, sizeof(uint32_t),
                                                 PP_STREAM, PP_MEM_CATEGORY_CONFIG);
        for (i=0;i<RAND_FLUSH_POINTS;i++)
        {
            if (snort_conf->run_flags & RUN_FLAG__STATIC_HASH)
            {
                if ( i == 0 )
                    LogMessage("WARNING:  using constant flush point = %u!\n", cfp);
                flush_point_list->flush_points[i] = cfp;

            }
            else if (use_static)
            {
                if ( i == 0 )
                    LogMessage("WARNING: using static flush points.\n");
                flush_point_list->flush_points[i] = g_static_points[i];
            }
            else
            {
                flush_point_list->flush_points[i] = GenerateFlushPoint(flush_point_list);
            }
        }
#endif
        flush_point_list->initialized = 1;
    }
}

static inline void UpdateFlushMgr( SnortConfig *sc,
        FlushMgr *mgr, FlushPointList *flush_point_list, uint32_t flags)
{
    if ( mgr->flush_type == STREAM_FT_EXTERNAL )
        return;

    switch (mgr->flush_policy)
    {
        case STREAM_FLPOLICY_FOOTPRINT:
        case STREAM_FLPOLICY_LOGICAL:
            break;

        case STREAM_FLPOLICY_PROTOCOL:
            if ( flags & TF_PKT_MISSED )
            {
                mgr->flush_policy = STREAM_FLPOLICY_FOOTPRINT;
                mgr->flush_type = STREAM_FT_PAF_MAX;
            }
            break;

#ifdef NORMALIZER
        case STREAM_FLPOLICY_FOOTPRINT_IPS:
        case STREAM_FLPOLICY_FOOTPRINT_IPS_FTP:
            break;

        case STREAM_FLPOLICY_PROTOCOL_IPS:
            if ( flags & TF_PKT_MISSED )
            {
                mgr->flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS;
                mgr->flush_type = STREAM_FT_PAF_MAX;
            }
            break;
#endif
        case STREAM_FLPOLICY_FOOTPRINT_NOACK:
            break;
        case STREAM_FLPOLICY_PROTOCOL_NOACK:
            if ( flags & TF_PKT_MISSED )
            {
                mgr->flush_policy = STREAM_FLPOLICY_FOOTPRINT_NOACK;
                mgr->flush_type = STREAM_FT_PAF_MAX;
            }
            break;
        default:
            return;
    }
    /* Ideally, we would call rand() each time, but that
     * is a performance headache waiting to happen. */
#ifdef DYNAMIC_RANDOM_FLUSH_POINTS
    mgr->flush_pt = GenerateFlushPoint();
#else
    if (flush_point_list)
    {
        /* Handle case where it wasn't initialized... */
        if (flush_point_list->initialized == 0)
        {
            InitFlushPointList(flush_point_list, 192, 128, 0);
        }
        mgr->flush_pt = flush_point_list->flush_points[flush_point_list->current];
        flush_point_list->current = (flush_point_list->current+1) % RAND_FLUSH_POINTS;
    }
#endif
    //Do not reset the last_size for FTP flush policy
    if(mgr->flush_policy != STREAM_FLPOLICY_FOOTPRINT_IPS_FTP)
    {
        mgr->last_size = 0;
        mgr->last_count = 0;
    }

    if ( mgr->flush_type == STREAM_FT_PAF_MAX )
        mgr->flush_pt += ScPafMaxNewConf(sc);
}

static inline void InitFlushMgr( SnortConfig *sc, FlushMgr *mgr, FlushPointList *flush_point_list,
        uint8_t policy, uint8_t auto_disable)
{
    // if policy is to disable reassembly just set policy in flush manager and
    // return
    if( policy == STREAM_FLPOLICY_DISABLED )
    {
        mgr->flush_policy = policy;
        return;
    }
    else if ( mgr->flush_policy == STREAM_FLPOLICY_DISABLED )
    {
        WarningMessage( "Stream policy has disabled reassembly on this port." );
        return;
    }

    mgr->flush_policy = policy;
    mgr->flush_type = STREAM_FT_INTERNAL;
    mgr->auto_disable = auto_disable;

    UpdateFlushMgr(sc, mgr, flush_point_list, 0);

#ifdef NORMALIZER
    if ( Normalize_GetMode(sc, NORM_TCP_IPS) == NORM_MODE_ON)
    {
        if ( policy == STREAM_FLPOLICY_FOOTPRINT )
            mgr->flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS;

        else if ( policy == STREAM_FLPOLICY_PROTOCOL )
            mgr->flush_policy = STREAM_FLPOLICY_PROTOCOL_IPS;
    }
#endif
}

static inline void InitFlushMgrByPort (
        SessionControlBlock* scb, StreamTracker* pst,
        uint16_t port, bool c2s, uint8_t flush_policy, bool all_services)
{
    uint16_t registration, auto_disable = 0;
    bool flush = (flush_policy != STREAM_FLPOLICY_IGNORE);

#if 0
    // this check required if PAF doesn't abort
    if ( scb->session_state & STREAM_STATE_MIDSTREAM )
        registration = 0;
    else
#endif
        if(all_services)
        {
            registration = s5_paf_port_registration_all(
                    ( ( StreamConfig * ) scb->stream_config )->tcp_config->paf_config, port, c2s, flush);
        }
        else
        {
            registration = s5_paf_port_registration(
                    ( ( StreamConfig * ) scb->stream_config )->tcp_config->paf_config, port, c2s, flush);
        }

    if ( registration )
    {
        if( flush_policy == STREAM_FLPOLICY_FOOTPRINT_NOACK )
            flush_policy = STREAM_FLPOLICY_PROTOCOL_NOACK;
        else
            flush_policy = STREAM_FLPOLICY_PROTOCOL;
        s5_paf_setup(&pst->paf_state, registration);
        auto_disable = !flush;
    }
    InitFlushMgr(snort_conf, &pst->flush_mgr,
            &pst->tcp_policy->flush_point_list, flush_policy, auto_disable);
}

static inline void InitFlushMgrByService (
        SessionControlBlock* scb, StreamTracker* pst,
        int16_t service, bool c2s, uint8_t flush_policy)
{
    uint16_t registration, auto_disable = 0;
    bool flush = (flush_policy != STREAM_FLPOLICY_IGNORE);

#if 0
    // this check required if PAF doesn't abort
    if ( scb->session_state & STREAM_STATE_MIDSTREAM )
        registration = 0;
    else
#endif
        registration = s5_paf_service_registration(
                ( ( StreamConfig * ) scb->stream_config )->tcp_config->paf_config, service, c2s, flush);

    if ( registration )
    {
        if( flush_policy == STREAM_FLPOLICY_FOOTPRINT_NOACK )
            flush_policy = STREAM_FLPOLICY_PROTOCOL_NOACK;
        else
            flush_policy = STREAM_FLPOLICY_PROTOCOL;
        s5_paf_setup(&pst->paf_state, registration);
        auto_disable = !flush;
    }
    InitFlushMgr(snort_conf, &pst->flush_mgr,
            &pst->tcp_policy->flush_point_list, flush_policy, auto_disable);
}

static int ResetFlushMgrsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    int i;
    StreamConfig *pPolicyConfig = (StreamConfig *)pData;

    //do any housekeeping before freeing StreamConfig
    if (pPolicyConfig->tcp_config == NULL)
        return 0;

    for (i = 0; i < pPolicyConfig->tcp_config->num_policies; i++)
    {
        int j;
        StreamTcpPolicy *policy = pPolicyConfig->tcp_config->policy_list[i];
        FlushPointList *fpl = &policy->flush_point_list;
        FlushMgr *client, *server;
        uint8_t flush_policy;

        fpl->current = 0;

        for (j = 0; j < MAX_PORTS; j++)
        {
            client = &policy->flush_config[j].client;
            flush_policy = policy->flush_config[j].client.flush_policy;
            InitFlushMgr(snort_conf, client, fpl, flush_policy, 0);

            server = &policy->flush_config[j].server;
            flush_policy = policy->flush_config[j].server.flush_policy;
            InitFlushMgr(snort_conf, server, fpl, flush_policy, 0);
        }
#ifdef TARGET_BASED
        /* protocol 0 is the unknown case. skip it */
        for (j = 1; j < MAX_PROTOCOL_ORDINAL; j++)
        {
            client = &policy->flush_config_protocol[j].client;
            flush_policy = policy->flush_config_protocol[j].client.flush_policy;
            InitFlushMgr(snort_conf, client, fpl, flush_policy, 0);

            server = &policy->flush_config_protocol[j].server;
            flush_policy = policy->flush_config_protocol[j].server.flush_policy;
            InitFlushMgr(snort_conf, server, fpl, flush_policy, 0);
        }
#endif
    }

    return 0;
}

static inline void ResetFlushMgrs( void )
{
    if( stream_online_config == NULL )
        return;

    sfPolicyUserDataFreeIterate( stream_online_config, ResetFlushMgrsPolicy );
}

void **StreamGetPAFUserDataTcp( SessionControlBlock* scb, bool to_server, uint8_t id )
{
    TcpSession* tcpssn;
    PAF_State *ps;
    int8_t i;

    if(!id)
       return NULL;

    if( !scb->proto_specific_data )
        return NULL;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if ( !tcpssn )
        return NULL;

    ps = ( to_server ) ? &tcpssn->server.paf_state
       :
       &tcpssn->client.paf_state;

    i = s5_paf_get_user_data_index( ps, id );

    if( i >= 0 )
       return &ps->user[i];
    else
       return NULL;
}

bool StreamIsPafActiveTcp( SessionControlBlock *scb, bool to_server )
{
    TcpSession *tcpssn;
    FlushMgr *fm;

    if( !scb->proto_specific_data )
        return false;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if( !tcpssn )
        return false;

    fm = ( to_server ) ? &tcpssn->server.flush_mgr : &tcpssn->client.flush_mgr;

    return ( ( fm->flush_policy == STREAM_FLPOLICY_PROTOCOL )
#ifdef NORMALIZER
            || ( fm->flush_policy == STREAM_FLPOLICY_PROTOCOL_IPS )
#endif
            || (fm->flush_policy == STREAM_FLPOLICY_PROTOCOL_NOACK)
           );
}

bool StreamActivatePafTcp (SessionControlBlock *scb, int dir, int16_t service_port, uint8_t type)
{
    TcpSession *tcpssn;
    StreamTracker *trk;
    FlushMgr *fm;
    uint8_t flush_policy;

    if( !scb->proto_specific_data )
        return false;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if( !tcpssn || !ScPafEnabled() )
        return false;

    // must have valid stream config for this session, may need to reset after a reload
    if( scb->stream_config == NULL )
    {
        WarningMessage("Stream Configuration NULL For In Progress Session, Reinitializing.\n");
        scb->stream_config = sfPolicyUserDataGet( stream_online_config, getNapRuntimePolicy() );
        if( scb->stream_config == NULL )
        {
            ErrorMessage("No Valid Stream Configurations, Stream Processing Terminated For This Packet.\n");
            return false;
        }
    }

    switch ( dir )
    {
        case SSN_DIR_TO_CLIENT:
            {
                trk = &tcpssn->client;
                fm = &tcpssn->client.flush_mgr;
                flush_policy = fm->flush_policy;
                if ( flush_policy == STREAM_FLPOLICY_IGNORE )
                    flush_policy = STREAM_FLPOLICY_PROTOCOL;

                if ( type == PAF_TYPE_SERVICE )
                {
                    InitFlushMgrByService(scb, trk, service_port, false, flush_policy);
                }
                else
                {
                    InitFlushMgrByPort(scb, trk, service_port, false, flush_policy, true);
                }
            }
            break;
        case SSN_DIR_TO_SERVER:
            {
                trk = &tcpssn->server;
                fm = &tcpssn->server.flush_mgr;
                flush_policy = fm->flush_policy;
                if ( flush_policy == STREAM_FLPOLICY_IGNORE )
                    flush_policy = STREAM_FLPOLICY_PROTOCOL;

                if ( type == PAF_TYPE_SERVICE )
                {
                    InitFlushMgrByService(scb, trk, service_port, true, flush_policy);
                }
                else
                {
                    InitFlushMgrByPort(scb, trk, service_port, true, flush_policy, true );
                }
            }
            break;
        case SSN_DIR_BOTH:
            {
                trk = &tcpssn->server;
                fm = &tcpssn->server.flush_mgr;
                flush_policy = fm->flush_policy;
                if ( flush_policy == STREAM_FLPOLICY_IGNORE )
                    flush_policy = STREAM_FLPOLICY_PROTOCOL;

                if ( type == PAF_TYPE_SERVICE )
                {
                    InitFlushMgrByService(scb, trk, service_port, true, flush_policy);
                }
                else
                {
                    InitFlushMgrByPort(scb, trk, service_port, true, flush_policy, true );
                }
            }
            {
                trk = &tcpssn->client;
                fm = &tcpssn->client.flush_mgr;
                flush_policy = fm->flush_policy;
                if ( flush_policy == STREAM_FLPOLICY_IGNORE )
                    flush_policy = STREAM_FLPOLICY_PROTOCOL;

                if ( type == PAF_TYPE_SERVICE )
                {
                    InitFlushMgrByService(scb, trk, service_port, false, flush_policy);
                }
                else
                {
                    InitFlushMgrByPort(scb, trk, service_port, false, flush_policy, true);
                }
            }
            break;

        default:
            return false;
    }

    return true;
}

static inline void StreamResetPolicyPerTrk( StreamTracker* trk, uint16_t policy, uint16_t mss )
{
    trk->tcp_policy->policy = trk->os_policy = policy;
    trk->reassembly_policy = GetTcpReassemblyPolicy( policy );
    trk->mss = mss;
}

void StreamResetPolicyTcp ( SessionControlBlock *scb, int dir, uint16_t policy, uint16_t mss)
{
    TcpSession *tcpssn;

    if( !scb->proto_specific_data )
        return;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if( !tcpssn )
        return;

    switch( dir )
    {
        case SSN_DIR_TO_CLIENT:
            StreamResetPolicyPerTrk( &tcpssn->client, policy, mss );
            break;

        case SSN_DIR_TO_SERVER:
            StreamResetPolicyPerTrk( &tcpssn->server, policy, mss );
            break;

        case SSN_DIR_BOTH:
            StreamResetPolicyPerTrk( &tcpssn->server, policy, mss );
            StreamResetPolicyPerTrk( &tcpssn->client, policy, mss );
            break;

        default:
            break;
    }
}

void StreamSetSessionDecryptedTcp( SessionControlBlock *scb, bool enable )
{
    TcpSession *tcpssn;

    if( !scb->proto_specific_data )
        return;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if ( !tcpssn )
        return;

    if((SegsToFlush(&tcpssn->server, 1) > 0) || (SegsToFlush(&tcpssn->client, 1) > 0))
    {
        FlushQueuedSegs(scb, tcpssn);
        Active_Reset();
    }

    tcpssn->session_decrypted = enable;
}

bool StreamIsSessionDecryptedTcp( SessionControlBlock *scb )
{
    TcpSession *tcpssn;

    if( !scb->proto_specific_data )
        return false;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if ( !tcpssn )
        return false;

    return ( tcpssn->session_decrypted == true );
}

uint32_t StreamGetPreprocFlagsTcp(SessionControlBlock *scb)
{
    TcpSession *tcpssn;

    if( !scb->proto_specific_data )
        return 0;

    tcpssn = ( TcpSession * ) scb->proto_specific_data->data;

    if ( !tcpssn )
        return 0;

    return tcpssn->pp_flags;
}

//-------------------------------------------------------------------------
// tcp ha stuff

#ifdef ENABLE_HA
static SessionControlBlock *StreamTCPCreateSession( const SessionKey *key )
{
    setNapRuntimePolicy( getDefaultPolicy() );

    /* TODO: Set the TCP policy to something here? */
    return session_api->create_session( tcp_lws_cache, NULL, key );
}

static void StreamTCPDeactivateSession( SessionControlBlock *scb )
{
    if( scb->proto_specific_data )
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Cleaning up the TCP session associated with the session being put into standby.\n"););
        TcpSessionCleanup( scb, 0 );
    }

    scb->session_state &= ~( STREAM_STATE_SYN | STREAM_STATE_SYN_ACK |
            STREAM_STATE_ACK | STREAM_STATE_ESTABLISHED );
    scb->ha_state.session_flags &= ~( SSNFLAG_SEEN_CLIENT | SSNFLAG_SEEN_SERVER );
}

static int StreamTCPDeleteSession( const SessionKey *key )
{
    SessionControlBlock *scb = session_api->get_session_by_key( tcp_lws_cache, key );

    if( scb != NULL )
    {
        if( StreamSetRuntimeConfiguration( scb, scb->protocol ) == 0 )
        {
            if (!session_api->delete_session( tcp_lws_cache, scb, "ha sync", false ))
               s5stats.active_tcp_sessions--;
        }
        else
            WarningMessage(" WARNING: Attempt to delete a TCP Session when no valid runtime configuration.\n" );
    }

    return 0;
}
#endif

SessionControlBlock *GetLWTcpSession( const SessionKey *key )
{
    return session_api->get_session_by_key( tcp_lws_cache, key );
}

#ifdef ENABLE_HA

static HA_Api ha_tcp_api = {
    /*.get_lws = */ GetLWTcpSession,

    /*.create_session = */ StreamTCPCreateSession,
    /*.deactivate_session = */ StreamTCPDeactivateSession,
    /*.delete_session = */ StreamTCPDeleteSession,
};

#endif

void setTcpDirectionAndPorts( Packet *p, SessionControlBlock *scb )
{
    if( TCP_ISFLAGSET( p->tcph, TH_SYN ) && !TCP_ISFLAGSET( p->tcph, TH_ACK ) )
    {
        STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                    "Stream SYN PACKET, establishing lightweight session direction.\n"););
        /* SYN packet from client */
        scb->ha_state.direction = FROM_CLIENT;
        IP_COPY_VALUE( scb->client_ip, GET_SRC_IP( p ) );
        scb->client_port = p->tcph->th_sport;
        IP_COPY_VALUE( scb->server_ip, GET_DST_IP( p ) );
        scb->server_port = p->tcph->th_dport;
    }
    else if( TCP_ISFLAGSET( p->tcph, ( TH_SYN | TH_ACK ) ) )
    {
        scb->ha_state.direction = FROM_SERVER;
        IP_COPY_VALUE( scb->client_ip, GET_DST_IP( p ) );
        scb->client_port = p->tcph->th_dport;
        IP_COPY_VALUE( scb->server_ip, GET_SRC_IP( p ) );
        scb->server_port = p->tcph->th_sport;
    }
    else
    {
        /* Assume from client, can update later */
        if( p->sp > p->dp )
        {
            scb->ha_state.direction = FROM_CLIENT;
            IP_COPY_VALUE( scb->client_ip, GET_SRC_IP( p ) );
            scb->client_port = p->tcph->th_sport;
            IP_COPY_VALUE( scb->server_ip, GET_DST_IP( p ) );
            scb->server_port = p->tcph->th_dport;
        }
        else
        {
            scb->ha_state.direction = FROM_SERVER;
            IP_COPY_VALUE( scb->client_ip, GET_DST_IP( p ) );
            scb->client_port = p->tcph->th_dport;
            IP_COPY_VALUE( scb->server_ip, GET_SRC_IP( p ) );
            scb->server_port = p->tcph->th_sport;
        }
    }
}
void StreamInitTcp( void )
{
    if( tcp_lws_cache == NULL )
    {
        tcp_lws_cache = session_api->init_session_cache( SESSION_PROTO_TCP,
                sizeof( TcpSession ),
                &TcpSessionCleanupWithFreeApplicationData );
        StreamTcpRegisterPreprocProfiles();
    }
#ifdef ENABLE_HA
    ha_set_api( IPPROTO_TCP, &ha_tcp_api );
#endif

    pkt_snaplen = (snort_conf->pkt_snaplen > 0) ? snort_conf->pkt_snaplen : PKT_SNAPLEN;
}

void StreamTcpRegisterPreprocProfiles( void )
{
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile( "s5TcpNewSess", &s5TcpNewSessPerfStats, 2, &s5TcpPerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpState", &s5TcpStatePerfStats, 2, &s5TcpPerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpData", &s5TcpDataPerfStats, 3, &s5TcpStatePerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpPktInsert", &s5TcpInsertPerfStats, 4, &s5TcpDataPerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpPAF", &s5TcpPAFPerfStats, 3, &s5TcpStatePerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpFlush", &s5TcpFlushPerfStats, 3, &s5TcpStatePerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpBuildPacket", &s5TcpBuildPacketPerfStats, 4, &s5TcpFlushPerfStats , NULL);
    RegisterPreprocessorProfile( "s5TcpProcessRebuilt", &s5TcpProcessRebuiltPerfStats,
            4, &s5TcpFlushPerfStats, NULL );
#endif
}

void StreamTcpRegisterRuleOptions( struct _SnortConfig *sc )
{
    /* Register the 'stream_size' rule option */
    RegisterPreprocessorRuleOption( sc, "stream_size", &s5TcpStreamSizeInit,
            &s5TcpStreamSizeEval, &s5TcpStreamSizeCleanup,
            NULL, NULL, NULL, NULL );

    RegisterPreprocessorRuleOption( sc, "stream_reassemble", &s5TcpStreamReassembleRuleOptionInit,
            &s5TcpStreamReassembleRuleOptionEval,
            &s5TcpStreamReassembleRuleOptionCleanup,
            NULL, NULL, NULL, NULL );
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile( "stream_size", &streamSizePerfStats, 4, &preprocRuleOptionPerfStats , NULL);
    RegisterPreprocessorProfile( "reassemble", &streamReassembleRuleOptionPerfStats,
            4, &preprocRuleOptionPerfStats, NULL);
#endif
}

void StreamTcpInitFlushPoints( void )
{
    int i;

    /* Seed the flushpoint random generator */
    srand( ( unsigned int ) sizeof( default_ports ) + ( unsigned int ) time( NULL ) );

    /* Default is to ignore, for all ports */
    for( i = 0; i < MAX_PORTS; i++ )
    {
        ignore_flush_policy[i].client.flush_policy = STREAM_FLPOLICY_IGNORE;
        ignore_flush_policy[i].server.flush_policy = STREAM_FLPOLICY_IGNORE;
    }
#ifdef TARGET_BASED
    for( i = 0; i < MAX_PROTOCOL_ORDINAL; i++)
    {
        ignore_flush_policy_protocol[i].client.flush_policy = STREAM_FLPOLICY_IGNORE;
        ignore_flush_policy_protocol[i].server.flush_policy = STREAM_FLPOLICY_IGNORE;
    }
#endif
}

static void addStreamTcpPolicyToList( StreamTcpConfig *config, StreamTcpPolicy *policy )
{
    config->num_policies++;

    /* Now add this context to the internal list */
    if( config->policy_list == NULL )
    {
        config->policy_list = ( StreamTcpPolicy ** ) SnortPreprocAlloc(1,
                             sizeof( StreamTcpPolicy * ), PP_STREAM,
                             PP_MEM_CATEGORY_CONFIG);
    }
    else
    {
        uint32_t policyListSize = sizeof( StreamTcpPolicy * ) * ( config->num_policies );
        StreamTcpPolicy **tmpPolicyList = ( StreamTcpPolicy ** ) SnortPreprocAlloc(1, 
                                              policyListSize, PP_STREAM,
                                              PP_MEM_CATEGORY_CONFIG);

        // copy the existing policies to new list, free old list and point to new...
        memcpy( tmpPolicyList, config->policy_list, policyListSize - sizeof( StreamTcpPolicy * ) );
        SnortPreprocFree( config->policy_list, sizeof( StreamTcpPolicy * ), PP_STREAM,
                         PP_MEM_CATEGORY_CONFIG );
        config->policy_list = tmpPolicyList;
    }

    // add new policy to end of the list
    config->policy_list[config->num_policies - 1] = policy;
}

StreamTcpPolicy *initTcpPolicyInstance( void )
{
    StreamTcpPolicy *tcp_policy = (StreamTcpPolicy *) SnortPreprocAlloc(1,
                                          sizeof(StreamTcpPolicy), PP_STREAM, 
                                          PP_MEM_CATEGORY_CONFIG);

    /* Initialize flush policy to Ignore */
    memcpy(&tcp_policy->flush_config, ignore_flush_policy,
            sizeof(FlushConfig) * MAX_PORTS);
#ifdef TARGET_BASED
    memcpy(&tcp_policy->flush_config_protocol, ignore_flush_policy_protocol,
            sizeof(FlushConfig) * MAX_PROTOCOL_ORDINAL);
#endif

    return tcp_policy;
}

StreamTcpPolicy *StreamTcpPolicyClone( StreamTcpPolicy *master, StreamConfig *config )
{
    StreamTcpPolicy *clone = initTcpPolicyInstance( );

    clone->policy = master->policy;
    clone->reassembly_policy = master->reassembly_policy;
    clone->flags = master->flags;
    clone->flush_factor = master->flush_factor;
    clone->session_timeout = master->session_timeout;
    clone->max_window = master->max_window;
    clone->overlap_limit = master->overlap_limit;
    clone->hs_timeout = master->hs_timeout;
    clone->max_queued_bytes = master->max_queued_bytes;
    clone->max_queued_segs = master->max_queued_segs;
    clone->max_consec_small_segs = master->max_consec_small_segs;
    clone->max_consec_small_seg_size = master->max_consec_small_seg_size;
    memcpy(clone->small_seg_ignore, master->small_seg_ignore, sizeof(master->small_seg_ignore));

    addStreamTcpPolicyToList( config->tcp_config, clone );

    return clone;
}

void StreamTcpPolicyInit(struct _SnortConfig *sc, StreamTcpConfig *config, char *args)
{
    StreamTcpPolicy *s5TcpPolicy;

    if (config == NULL)
        return;

    // create list for caching port reassembly requests...
    StreamCreateReassemblyPortList( );

    s5TcpPolicy = initTcpPolicyInstance( );
    StreamParseTcpArgs(sc, config, args, s5TcpPolicy);
    addStreamTcpPolicyToList( config, s5TcpPolicy );

    if ( ScPafEnabledNewConf(sc) && !config->paf_config )
        config->paf_config = s5_paf_new();

    // register callback with Session that determines direction and client/server ports
    registerDirectionPortCallback( SESSION_PROTO_TCP, setTcpDirectionAndPorts );

    StreamPrintTcpConfig(s5TcpPolicy);

#ifdef REG_TEST
    LogMessage("    TCP Session Size: %lu\n", (long unsigned int)sizeof(TcpSession));
#endif
}

static inline uint16_t StreamPolicyIdFromName(char *name)
{
    if (!name)
    {
        return STREAM_POLICY_DEFAULT;
    }

    if(!strcasecmp(name, "bsd"))
    {
        return STREAM_POLICY_BSD;
    }
    else if(!strcasecmp(name, "old-linux"))
    {
        return STREAM_POLICY_OLD_LINUX;
    }
    else if(!strcasecmp(name, "linux"))
    {
        return STREAM_POLICY_LINUX;
    }
    else if(!strcasecmp(name, "first"))
    {
        return STREAM_POLICY_FIRST;
    }
    else if(!strcasecmp(name, "noack"))
    {
        return STREAM_POLICY_NOACK;
    }
    else if(!strcasecmp(name, "last"))
    {
        return STREAM_POLICY_LAST;
    }
    else if(!strcasecmp(name, "windows"))
    {
        return STREAM_POLICY_WINDOWS;
    }
    else if(!strcasecmp(name, "solaris"))
    {
        return STREAM_POLICY_SOLARIS;
    }
    else if(!strcasecmp(name, "win2003") ||
            !strcasecmp(name, "win2k3"))
    {
        return STREAM_POLICY_WINDOWS2K3;
    }
    else if(!strcasecmp(name, "vista"))
    {
        return STREAM_POLICY_VISTA;
    }
    else if(!strcasecmp(name, "hpux") ||
            !strcasecmp(name, "hpux11"))
    {
        return STREAM_POLICY_HPUX11;
    }
    else if(!strcasecmp(name, "hpux10"))
    {
        return STREAM_POLICY_HPUX10;
    }
    else if(!strcasecmp(name, "irix"))
    {
        return STREAM_POLICY_IRIX;
    }
    else if(!strcasecmp(name, "macos") ||
            !strcasecmp(name, "grannysmith"))
    {
        return STREAM_POLICY_MACOS;
    }
    return STREAM_POLICY_DEFAULT; /* BSD is the default */
}

static inline uint16_t GetTcpReassemblyPolicy(int os_policy)
{
    switch (os_policy)
    {
        case STREAM_POLICY_FIRST:
            return REASSEMBLY_POLICY_FIRST;
        case STREAM_POLICY_LINUX:
            return REASSEMBLY_POLICY_LINUX;
        case STREAM_POLICY_BSD:
            return REASSEMBLY_POLICY_BSD;
        case STREAM_POLICY_OLD_LINUX:
            return REASSEMBLY_POLICY_OLD_LINUX;
        case STREAM_POLICY_LAST:
            return REASSEMBLY_POLICY_LAST;
        case STREAM_POLICY_WINDOWS:
            return REASSEMBLY_POLICY_WINDOWS;
        case STREAM_POLICY_SOLARIS:
            return REASSEMBLY_POLICY_SOLARIS;
        case STREAM_POLICY_WINDOWS2K3:
            return REASSEMBLY_POLICY_WINDOWS2K3;
        case STREAM_POLICY_VISTA:
            return REASSEMBLY_POLICY_VISTA;
        case STREAM_POLICY_HPUX11:
            return REASSEMBLY_POLICY_HPUX11;
        case STREAM_POLICY_HPUX10:
            return REASSEMBLY_POLICY_HPUX10;
        case STREAM_POLICY_IRIX:
            return REASSEMBLY_POLICY_IRIX;
        case STREAM_POLICY_MACOS:
            return REASSEMBLY_POLICY_MACOS;
        case STREAM_POLICY_NOACK:
            return REASSEMBLY_POLICY_NOACK;
        default:
            return REASSEMBLY_POLICY_DEFAULT;
    }
}

#define STATIC_FP ((s5TcpPolicy->flags & STREAM_CONFIG_STATIC_FLUSHPOINTS)?1:0)

static void StreamParseTcpArgs(struct _SnortConfig *sc, StreamTcpConfig *config, char *args, StreamTcpPolicy *s5TcpPolicy)
{
    char **toks;
    int num_toks;
    int i;
    char **stoks = NULL;
    int s_toks;
    char *endPtr = NULL;
    char set_flush_policy = 0;
#ifdef TARGET_BASED
    char set_target_flush_policy = 0;
#endif
    int reassembly_direction = SSN_DIR_FROM_CLIENT;
    int32_t long_val = 0;

    s5TcpPolicy->policy = STREAM_POLICY_DEFAULT;
    s5TcpPolicy->reassembly_policy = REASSEMBLY_POLICY_DEFAULT;
    s5TcpPolicy->session_timeout = STREAM_DEFAULT_SSN_TIMEOUT;
    s5TcpPolicy->max_window = 0;
    s5TcpPolicy->flags = 0;
    s5TcpPolicy->max_queued_bytes = STREAM_DEFAULT_MAX_QUEUED_BYTES;
    s5TcpPolicy->max_queued_segs = STREAM_DEFAULT_MAX_QUEUED_SEGS;

    s5TcpPolicy->max_consec_small_segs = STREAM_DEFAULT_CONSEC_SMALL_SEGS;
    s5TcpPolicy->max_consec_small_seg_size = STREAM_DEFAULT_MAX_SMALL_SEG_SIZE;
    s5TcpPolicy->log_asymmetric_traffic = false;

    if(args != NULL && strlen(args) != 0)
    {
        toks = mSplit(args, ",", 0, &num_toks, 0);

        for (i = 0; i < num_toks; i++)
        {
            if(!strcasecmp(toks[i], "use_static_footprint_sizes"))
                s5TcpPolicy->flags |= STREAM_CONFIG_STATIC_FLUSHPOINTS;
        }

        for (i = 0; i < num_toks; i++)
        {
            int max_s_toks = 1;  // set to 0 to disable check
            stoks = mSplit(toks[i], " ", 3, &s_toks, 0);

            if (s_toks == 0)
            {
                FatalError("%s(%d) => Missing parameter in Stream TCP config.\n",
                        file_name, file_line);
            }

            if(!strcasecmp(stoks[0], "timeout"))
            {
                if(stoks[1])
                {
                    s5TcpPolicy->session_timeout = strtoul(stoks[1], &endPtr, 10);
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  "
                            "Integer parameter required.\n",
                            file_name, file_line);
                }

                if ((s5TcpPolicy->session_timeout > STREAM_MAX_SSN_TIMEOUT) ||
                        (s5TcpPolicy->session_timeout < STREAM_MIN_SSN_TIMEOUT))
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  "
                            "Must be between %d and %d\n",
                            file_name, file_line,
                            STREAM_MIN_SSN_TIMEOUT, STREAM_MAX_SSN_TIMEOUT);
                }
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "log_asymmetric_traffic"))
            {
                if(stoks[1])
                {
                    if(!strcasecmp(stoks[1], "no"))
                           s5TcpPolicy->log_asymmetric_traffic = false;
                    else if(!strcasecmp(stoks[1], "yes"))
                           s5TcpPolicy->log_asymmetric_traffic = true;
                    else
                           FatalError("%s(%d) => invalid: value must be 'yes' or 'no'\n", file_name, file_line);
                }
                else
                {
                    FatalError("%s(%d) => 'log_asymmetric_traffic' missing option\n", file_name, file_line);
                }
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "overlap_limit"))
            {
                if(stoks[1])
                {
                    long_val = SnortStrtol(stoks[1], &endPtr, 10);
                    if (errno == ERANGE)
                    {
                        errno = 0;
                        long_val = -1;
                    }
                    s5TcpPolicy->overlap_limit = (uint8_t)long_val;
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]))
                {
                    FatalError("%s(%d) => Invalid overlap limit in config file."
                            "Integer parameter required\n",
                            file_name, file_line);
                }

                if ((long_val > STREAM_MAX_OVERLAP_LIMIT) ||
                        (long_val < STREAM_MIN_OVERLAP_LIMIT))
                {
                    FatalError("%s(%d) => Invalid overlap limit in config file."
                            "  Must be between %d and %d\n",
                            file_name, file_line,
                            STREAM_MIN_OVERLAP_LIMIT, STREAM_MAX_OVERLAP_LIMIT);
                }
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "detect_anomalies"))
            {
                s5TcpPolicy->flags |=  STREAM_CONFIG_ENABLE_ALERTS;
            }
            else if(!strcasecmp(stoks[0], "policy"))
            {
                if(s_toks != 2)
                    ParseError("Invalid Stream TCP policy option");
                s5TcpPolicy->policy = StreamPolicyIdFromName(stoks[1]);

                if ((s5TcpPolicy->policy == STREAM_POLICY_DEFAULT) &&
                        (strcasecmp(stoks[1], "bsd")))
                {
                    /* Default is BSD.  If we don't have "bsd", its
                     * the default and invalid.
                     */
                    FatalError("%s(%d) => Bad policy name \"%s\"\n",
                            file_name, file_line, stoks[1]);
                }
                s5TcpPolicy->reassembly_policy =
                    GetTcpReassemblyPolicy(s5TcpPolicy->policy);

                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "require_3whs"))
            {
                s5TcpPolicy->flags |= STREAM_CONFIG_REQUIRE_3WHS;

                if (s_toks > 1)
                {
                    s5TcpPolicy->hs_timeout = SnortStrtoul(stoks[1], &endPtr, 10);

                    if ((endPtr == &stoks[1][0]) || (*endPtr != '\0') || (errno == ERANGE))
                    {
                        FatalError("%s(%d) => Invalid 3Way Handshake allowable.  Integer parameter required.\n",
                                file_name, file_line);
                    }

                    if (s5TcpPolicy->hs_timeout > STREAM_MAX_SSN_TIMEOUT)
                    {
                        FatalError("%s(%d) => Invalid handshake timeout in "
                                "config file.  Must be between %d and %d\n",
                                file_name, file_line,
                                STREAM_MIN_ALT_HS_TIMEOUT, STREAM_MAX_SSN_TIMEOUT);
                    }
                }

                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "bind_to"))
            {
                if (s_toks < 2)
                {
                    FatalError("%s(%d) => Invalid Stream TCP Policy option - "
                            "\"bind_to\" option requires an argument.\n",
                            file_name, file_line);
                }

                if(strstr(stoks[1], "["))
                {
                    FatalError("%s(%d) => Invalid Stream TCP Policy option.  IP lists are not allowed.\n",
                            file_name, file_line);
                }

                s5TcpPolicy->bound_addrs = IpAddrSetParse(sc, stoks[1]);
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "max_window"))
            {
                if(stoks[1])
                {
                    long_val = SnortStrtol(stoks[1], &endPtr, 10);
                    if (errno == ERANGE)
                    {
                        errno = 0;
                        FatalError("%s(%d) => Invalid Max Window size.  Integer parameter required.\n",
                                file_name, file_line);
                    }
                    s5TcpPolicy->max_window = (uint32_t)long_val;
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]))
                {
                    FatalError("%s(%d) => Invalid Max Window size.  Integer parameter required.\n",
                            file_name, file_line);
                }

                if ((long_val > STREAM_MAX_MAX_WINDOW) ||
                        (long_val < STREAM_MIN_MAX_WINDOW))
                {
                    FatalError("%s(%d) => Invalid Max Window size."
                            "  Must be between %d and %d\n",
                            file_name, file_line,
                            STREAM_MIN_MAX_WINDOW, STREAM_MAX_MAX_WINDOW);
                }
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "use_static_footprint_sizes"))
            {
                // we already handled this one above
            }
            else if(!strcasecmp(stoks[0], "flush_factor"))
            {
                if (stoks[1])
                {
                    s5TcpPolicy->flush_factor = (uint16_t)SnortStrtoulRange(
                            stoks[1], &endPtr, 10, 0, STREAM_MAX_FLUSH_FACTOR);
                }
                if (
                        (!stoks[1] || (endPtr == &stoks[1][0])) ||
                        (s5TcpPolicy->flush_factor > STREAM_MAX_FLUSH_FACTOR))
                {
                    FatalError("%s(%d) => 'flush_factor %d' invalid: "
                            "value must be between 0 and %d segments.\n",
                            file_name, file_line, s5TcpPolicy->flush_factor,
                            STREAM_MAX_FLUSH_FACTOR);
                }
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "dont_store_large_packets"))
            {
                s5TcpPolicy->flags |= STREAM_CONFIG_PERFORMANCE;
            }
            else if(!strcasecmp(stoks[0], "check_session_hijacking"))
            {
                s5TcpPolicy->flags |= STREAM_CONFIG_CHECK_SESSION_HIJACKING;
            }
            else if(!strcasecmp(stoks[0], "ignore_any_rules"))
            {
                s5TcpPolicy->flags |= STREAM_CONFIG_IGNORE_ANY;
            }
            else if(!strcasecmp(stoks[0], "dont_reassemble_async"))
            {
                s5TcpPolicy->flags |= STREAM_CONFIG_NO_ASYNC_REASSEMBLY;
            }
            else if(!strcasecmp(stoks[0], "max_queued_bytes"))
            {
                if(stoks[1])
                {
                    long_val = SnortStrtol(stoks[1], &endPtr, 10);
                    if (errno == ERANGE)
                    {
                        errno = 0;
                        FatalError("%s(%d) => Invalid Max Queued Bytes.  Integer parameter required.\n",
                                file_name, file_line);
                    }
                    s5TcpPolicy->max_queued_bytes = (uint32_t)long_val;
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]))
                {
                    FatalError("%s(%d) => Invalid Max Queued Bytes.  Integer parameter required.\n",
                            file_name, file_line);
                }
                if (((long_val > STREAM_MAX_MAX_QUEUED_BYTES) ||
                            (long_val < STREAM_MIN_MAX_QUEUED_BYTES)) &&
                        (long_val != 0))
                {
                    FatalError("%s(%d) => Invalid Max Queued Bytes."
                            "  Must be 0 (disabled) or between %d and %d\n",
                            file_name, file_line,
                            STREAM_MIN_MAX_QUEUED_BYTES, STREAM_MAX_MAX_QUEUED_BYTES);
                }
                max_s_toks = 2;
            }
            else if(!strcasecmp(stoks[0], "max_queued_segs"))
            {
                if(stoks[1])
                {
                    long_val = SnortStrtol(stoks[1], &endPtr, 10);
                    if (errno == ERANGE)
                    {
                        errno = 0;
                        FatalError("%s(%d) => Invalid Max Queued Bytes.  Integer parameter required.\n",
                                file_name, file_line);
                    }
                    s5TcpPolicy->max_queued_segs = (uint32_t)long_val;
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]))
                {
                    FatalError("%s(%d) => Invalid Max Queued Bytes.  Integer parameter required.\n",
                            file_name, file_line);
                }

                if (((long_val > STREAM_MAX_MAX_QUEUED_SEGS) ||
                            (long_val < STREAM_MIN_MAX_QUEUED_SEGS)) &&
                        (long_val != 0))
                {
                    FatalError("%s(%d) => Invalid Max Queued Bytes."
                            "  Must be 0 (disabled) or between %d and %d\n",
                            file_name, file_line,
                            STREAM_MIN_MAX_QUEUED_SEGS, STREAM_MAX_MAX_QUEUED_SEGS);
                }
                max_s_toks = 2;
            }
            else if (!strcasecmp(stoks[0], "small_segments"))
            {
                char **ptoks;
                int num_ptoks;

                /* Small segments takes at least 3 parameters... */
                if (s_toks < 3)
                {
                    FatalError("%s(%d) => Insufficient parameters to small "
                            "segments configuration.  Syntax is: "
                            "<number> bytes <number> ignore_ports p1 p2, "
                            "with ignore_ports being an optional parameter\n",
                            file_name, file_line);
                }

                /* first the number of consecutive segments */
                long_val = SnortStrtol(stoks[1], &endPtr, 10);
                if (errno == ERANGE)
                {
                    errno = 0;
                    FatalError("%s(%d) => Invalid Small Segment number.  Integer parameter required.\n",
                            file_name, file_line);
                }
                s5TcpPolicy->max_consec_small_segs = (uint32_t)long_val;

                if ((long_val > STREAM_MAX_CONSEC_SMALL_SEGS) ||
                        (long_val < STREAM_MIN_CONSEC_SMALL_SEGS))
                {
                    FatalError("%s(%d) => Invalid Small Segments."
                            "  Must be integer between %d and %d, inclusive\n",
                            file_name, file_line,
                            STREAM_MIN_CONSEC_SMALL_SEGS, STREAM_MAX_CONSEC_SMALL_SEGS);
                }

                ptoks = mSplit(stoks[2], " ", MAX_PORTS + 3, &num_ptoks, 0);

                /* the bytes keyword */
                if (strcasecmp(ptoks[0], "bytes") || (num_ptoks < 2))
                {
                    FatalError("%s(%d) => Insufficient parameters to small "
                            "segments configuration.  Syntax is: "
                            "<number> bytes <number> ignore_ports p1 p2, "
                            "with ignore_ports being an optional parameter\n",
                            file_name, file_line);
                }

                /* the minimum bytes for a segment to be considered "small" */
                long_val = SnortStrtol(ptoks[1], &endPtr, 10);
                if (errno == ERANGE)
                {
                    errno = 0;
                    FatalError("%s(%d) => Invalid Small Segment bytes.  Integer parameter required.\n",
                            file_name, file_line);
                }
                s5TcpPolicy->max_consec_small_seg_size = (uint32_t)long_val;

                if ((long_val > STREAM_MAX_MAX_SMALL_SEG_SIZE) ||
                        (long_val < STREAM_MIN_MAX_SMALL_SEG_SIZE))
                {
                    FatalError("%s(%d) => Invalid Small Segments bytes."
                            "  Must be integer between %d and %d, inclusive\n",
                            file_name, file_line,
                            STREAM_MIN_MAX_SMALL_SEG_SIZE, STREAM_MAX_MAX_SMALL_SEG_SIZE);
                }

                /* and the optional ignore_ports */
                if (num_ptoks > 2)
                {
                    int j;
                    unsigned short port = 0;
                    long long_port = 0;
                    if (strcasecmp(ptoks[2], "ignore_ports") || (num_ptoks < 4))
                    {
                        FatalError("%s(%d) => Insufficient parameters to small "
                                "segments configuration.  Syntax is: "
                                "<number> bytes <number> ignore_ports p1 p2, "
                                "with ignore_ports being an optional parameter\n",
                                file_name, file_line);
                    }

                    for (j=3; j<num_ptoks;j++)
                    {
                        if (ptoks[j])
                        {
                            long_port = strtol(ptoks[j], &endPtr, 10);
                        }
                        if (!ptoks[j] || (endPtr == &ptoks[j][0]))
                        {
                            FatalError("%s(%d) => Invalid Port for small segments ignore_ports parameter.  Integer parameter required.\n",
                                    file_name, file_line);
                        }

                        if ((long_port < 0) || (long_port > MAX_PORTS-1))
                        {
                            FatalError(
                                    "%s(%d) => Invalid port %ld for small segments ignore_ports "
                                    "parameter, must be between 0 and %d, inclusive\n",
                                    file_name, file_line, long_port, MAX_PORTS-1);
                        }
                        port = (unsigned short)long_port;

                        s5TcpPolicy->small_seg_ignore[port/8] |= (1 << (port %8));
                    }
                }
                max_s_toks = 0; // we already checked all tokens
                mSplitFree(&ptoks, num_ptoks);
            }
            else if (!strcasecmp(stoks[0], "ports"))
            {
                if (s_toks > 1)
                {
                    if(!strcasecmp(stoks[1], "client"))
                    {
                        reassembly_direction = SSN_DIR_FROM_CLIENT;
                    }
                    else if(!strcasecmp(stoks[1], "server"))
                    {
                        reassembly_direction = SSN_DIR_FROM_SERVER;
                    }
                    else if(!strcasecmp(stoks[1], "both"))
                    {
                        reassembly_direction = SSN_DIR_BOTH;
                    }
                    else
                    {
                        FatalError(
                                "%s(%d) => Invalid direction for reassembly "
                                "configuration \"%s\".  Valid values are 'client', "
                                "'server', or 'both'.\n",
                                file_name, file_line, stoks[1]);
                    }
                }

                if (s_toks > 2)
                {
                    char **ptoks;
                    int num_ptoks;
                    int j;
                    unsigned short port = 0;
                    long long_port = 0;

                    /* Initialize it if not already... */
                    InitFlushPointList(&s5TcpPolicy->flush_point_list, 192, 128, STATIC_FP);

                    if (!strcasecmp(stoks[2], "all"))
                    {
                        for (j=0; j<MAX_PORTS; j++)
                        {
                            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
                            }
                            if (reassembly_direction & SSN_DIR_FROM_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
                            }
                        }
                    }
                    else if (!strcasecmp(stoks[2], "none"))
                    {
                        for (j=0; j<MAX_PORTS; j++)
                        {
                            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].client;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                            if (reassembly_direction & SSN_DIR_FROM_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].server;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                        }
                    }
                    else
                    {
                        ptoks = mSplit(stoks[2], " ", MAX_PORTS, &num_ptoks, 0);

                        for (j=0; j < num_ptoks; j++)
                        {
                            FlushPolicy flush_policy = STREAM_FLPOLICY_FOOTPRINT;

                            if (ptoks[j])
                            {
                                // check for '!' not port syntax used to disable reassembly on a port
                                // even if requested later by a preproc...
                                if ( ptoks[j][0] == '!' )
                                {
                                    flush_policy = STREAM_FLPOLICY_DISABLED;
                                    long_port = strtol(&ptoks[j][1], &endPtr, 10);
                                }
                                else
                                {
                                    long_port = strtol(ptoks[j], &endPtr, 10);
                                }
                            }

                            if (!ptoks[j] || (endPtr == &ptoks[j][0]))
                            {
                                FatalError("%s(%d) => Invalid Port list.  Integer parameter required.\n",
                                        file_name, file_line);
                            }

                            if ((long_port < 0) || (long_port > MAX_PORTS-1))
                            {
                                FatalError(
                                        "%s(%d) => Invalid port %ld, must be between 0 and %d, "
                                        "inclusive\n",
                                        file_name, file_line, long_port, MAX_PORTS-1);
                            }
                            port = (unsigned short)long_port;

                            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[port].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, flush_policy, 0);
                            }
                            if (reassembly_direction & SSN_DIR_FROM_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[port].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, flush_policy, 0);
                            }
                        }
                        mSplitFree(&ptoks, num_ptoks);
                    }
                    set_flush_policy = 1;
                }
                max_s_toks = 0;  // we already checked all tokens
            }
#ifdef TARGET_BASED
            else if (!strcasecmp(stoks[0], "protocol"))
            {
                if (s_toks > 1)
                {
                    if(!strcasecmp(stoks[1], "client"))
                    {
                        reassembly_direction = SSN_DIR_FROM_CLIENT;
                    }
                    else if(!strcasecmp(stoks[1], "server"))
                    {
                        reassembly_direction = SSN_DIR_FROM_SERVER;
                    }
                    else if(!strcasecmp(stoks[1], "both"))
                    {
                        reassembly_direction = SSN_DIR_BOTH;
                    }
                    else
                    {
                        FatalError(
                                "%s(%d) => Invalid direction for reassembly "
                                "configuration \"%s\".  Valid values are 'client', "
                                "'server', or 'both'.\n",
                                file_name, file_line, stoks[1]);
                    }
                }

                if (s_toks > 2)
                {
                    char **ptoks;
                    int num_ptoks;
                    int j;

                    /* Initialize it if not already... */
                    InitFlushPointList(&s5TcpPolicy->flush_point_list, 192, 128, STATIC_FP);

                    if (!strcasecmp(stoks[2], "all"))
                    {
                        for (j=1; j<MAX_PROTOCOL_ORDINAL; j++)
                        {
                            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
                            }
                            if (reassembly_direction & SSN_DIR_FROM_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
                            }
                            s5TcpPolicy->flush_config_protocol[j].configured = 1;
                        }
                    }
                    else if (!strcasecmp(stoks[2], "none"))
                    {
                        for (j=1; j<MAX_PROTOCOL_ORDINAL; j++)
                        {
                            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].client;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                            if (reassembly_direction & SSN_DIR_FROM_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].server;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                            s5TcpPolicy->flush_config_protocol[j].configured = 1;
                        }
                    }
                    else
                    {
                        ptoks = mSplit(stoks[2], " ", MAX_PROTOCOL_ORDINAL, &num_ptoks, 0);

                        for (j=0;j<num_ptoks;j++)
                        {
                            int16_t proto_ordinal;
                            if (!ptoks[j])
                            {
                                FatalError("%s(%d) => Invalid Protocol Name.  Protocol name must be specified.\n",
                                        file_name, file_line);
                            }
                            /* First look it up */
                            proto_ordinal = FindProtocolReference(ptoks[j]);
                            if (proto_ordinal == SFTARGET_UNKNOWN_PROTOCOL)
                            {
                                /* Not known -- add it */
                                proto_ordinal = AddProtocolReference(ptoks[j]);
                                if (proto_ordinal == SFTARGET_UNKNOWN_PROTOCOL)
                                {
                                    FatalError("%s(%d) => Failed to find protocol reference for '%s'\n",
                                            file_name, file_line, ptoks[j]);
                                }
                            }

                            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[proto_ordinal].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
                            }
                            if (reassembly_direction & SSN_DIR_FROM_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[proto_ordinal].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
                            }
                            s5TcpPolicy->flush_config_protocol[proto_ordinal].configured = 1;
                        }
                        mSplitFree(&ptoks, num_ptoks);
                    }
                    set_target_flush_policy = 1;
                }
                max_s_toks = 0;  // we already checked all tokens
            }
#endif
            else
            {
                FatalError("%s(%d) => Invalid Stream TCP policy option\n",
                        file_name, file_line);
            }

            if ( max_s_toks && (s_toks > max_s_toks) )
            {
                FatalError("%s(%d) => Invalid Stream TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
            }
            mSplitFree(&stoks, s_toks);
        }

        mSplitFree(&toks, num_toks);
    }

    if (s5TcpPolicy->bound_addrs == NULL)
    {
        if (config->default_policy != NULL)
        {
            FatalError("%s(%d) => Default Stream TCP Policy already set. "
                    "This policy must be bound to a specific host or "
                    "network.\n", file_name, file_line);
        }

        config->default_policy = s5TcpPolicy;
    }
    else
    {
        if (s5TcpPolicy->flags & STREAM_CONFIG_IGNORE_ANY)
        {
            FatalError("%s(%d) => \"ignore_any_rules\" option can be used only"
                    " with Default Stream TCP Policy\n", file_name, file_line);
        }
    }

    if (!set_flush_policy)
    {
        /* Initialize it if not already... */
        InitFlushPointList(&s5TcpPolicy->flush_point_list, 192, 128, STATIC_FP);
        for (i=0; i<DEFAULT_PORTS_SIZE; i++)
        {
            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[default_ports[i]].client;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
            }
            if (reassembly_direction & SSN_DIR_FROM_SERVER)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[default_ports[i]].server;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
            }
        }
    }

#ifdef TARGET_BASED
    if (!set_target_flush_policy)
    {
        int app_id;
        /* Initialize it if not already... */
        InitFlushPointList(&s5TcpPolicy->flush_point_list, 192, 128, STATIC_FP);
        for (i=0; i<(int)(sizeof(default_protocols)/sizeof(char *)); i++)
        {
            /* Look up the protocol by name. Add it if it doesn't exist. */
            app_id = FindProtocolReference(default_protocols[i]);
            if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
            {
                app_id = AddProtocolReference(default_protocols[i]);
            }

            /* While this should never happen, I don't feel guilty adding this
             * logic as it executes at parse time. */
            if (app_id == SFTARGET_UNKNOWN_PROTOCOL)
                continue;

            /* Set flush managers. */
            if (reassembly_direction & SSN_DIR_FROM_CLIENT)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[app_id].client;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
            }
            if (reassembly_direction & SSN_DIR_FROM_SERVER)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[app_id].server;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(sc, flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT, 0);
            }
            s5TcpPolicy->flush_config_protocol[app_id].configured = 1;
        }
    }
#endif
}

static void StreamPrintTcpConfig(StreamTcpPolicy *s5TcpPolicy)
{
    int i=0, j=0;
    LogMessage("Stream TCP Policy config:\n");
    if (s5TcpPolicy->bound_addrs != NULL)
    {
        IpAddrSetPrint("    Bound Addresses: ", s5TcpPolicy->bound_addrs);
    }
    else
    {
        LogMessage("    Bound Address: default\n");
    }
    LogMessage("    Reassembly Policy: %s\n",
            reassembly_policy_names[s5TcpPolicy->reassembly_policy]);
    LogMessage("    Timeout: %d seconds\n", s5TcpPolicy->session_timeout);
    if (s5TcpPolicy->max_window != 0)
        LogMessage("    Max TCP Window: %u\n", s5TcpPolicy->max_window);
    if (s5TcpPolicy->overlap_limit)
        LogMessage("    Limit on TCP Overlaps: %d\n", s5TcpPolicy->overlap_limit);
    if (s5TcpPolicy->max_queued_bytes != 0)
    {
        LogMessage("    Maximum number of bytes to queue per session: %d\n",
                s5TcpPolicy->max_queued_bytes);
    }
    if (s5TcpPolicy->max_queued_segs != 0)
    {
        LogMessage("    Maximum number of segs to queue per session: %d\n",
                s5TcpPolicy->max_queued_segs);
    }
    if (s5TcpPolicy->flags)
    {
        LogMessage("    Options:\n");
        if (s5TcpPolicy->flags & STREAM_CONFIG_REQUIRE_3WHS)
        {
            LogMessage("        Require 3-Way Handshake: YES\n");
            if (s5TcpPolicy->hs_timeout != 0)
            {
                LogMessage("        3-Way Handshake Timeout: %d\n",
                        s5TcpPolicy->hs_timeout);
            }
        }
        if (s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS)
        {
            LogMessage("        Detect Anomalies: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM_CONFIG_STATIC_FLUSHPOINTS)
        {
            LogMessage("        Static Flushpoint Sizes: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM_CONFIG_PERFORMANCE)
        {
            LogMessage("        Don't Queue Large Packets for Reassembly: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM_CONFIG_CHECK_SESSION_HIJACKING)
        {
            LogMessage("        Check for TCP Session Hijacking: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM_CONFIG_IGNORE_ANY)
        {
            LogMessage("        Ignore Any -> Any Rules: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM_CONFIG_NO_ASYNC_REASSEMBLY)
        {
            LogMessage("        Don't queue packets on one-sided sessions: YES\n");
        }
    }
    LogMessage("    Reassembly Ports:\n");
    for (i=0; i<MAX_PORTS; i++)
    {
        int direction = 0;
        int client_flushpolicy = s5TcpPolicy->flush_config[i].client.flush_policy;
        int server_flushpolicy = s5TcpPolicy->flush_config[i].server.flush_policy;
        char client_policy_str[STD_BUF];
        char server_policy_str[STD_BUF];
        client_policy_str[0] = server_policy_str[0] = '\0';

        if (client_flushpolicy != STREAM_FLPOLICY_IGNORE)
        {
            direction |= SSN_DIR_FROM_CLIENT;

            if (client_flushpolicy < STREAM_FLPOLICY_MAX)
                SnortSnprintf(client_policy_str, STD_BUF, "client (%s)",
                        flush_policy_names[client_flushpolicy]);
        }
        if (server_flushpolicy != STREAM_FLPOLICY_IGNORE)
        {
            direction |= SSN_DIR_FROM_SERVER;

            if (server_flushpolicy < STREAM_FLPOLICY_MAX)
                SnortSnprintf(server_policy_str, STD_BUF, "server (%s)",
                        flush_policy_names[server_flushpolicy]);
        }
        if (direction)
        {
            if (j<MAX_PORTS_TO_PRINT)
            {
                LogMessage("      %d %s %s\n", i,
                        client_policy_str, server_policy_str);
            }
            j++;
        }
    }

    if (j > MAX_PORTS_TO_PRINT)
    {
        LogMessage("      additional ports configured but not printed.\n");
    }
}

#ifdef TARGET_BASED
int StreamPolicyIdFromHostAttributeEntry(HostAttributeEntry *host_entry)
{
    if (!host_entry)
        return 0;

    host_entry->hostInfo.streamPolicy = StreamPolicyIdFromName(host_entry->hostInfo.streamPolicyName);
    host_entry->hostInfo.streamPolicySet = 1;

    STREAM_DEBUG_WRAP(
            DebugMessage(DEBUG_STREAM_STATE,
                "STREAM INIT: %s(%d) for Entry %s\n",
                reassembly_policy_names[host_entry->hostInfo.streamPolicy],
                host_entry->hostInfo.streamPolicy,
                host_entry->hostInfo.streamPolicyName););
    return 0;
}
#endif

void StreamPostConfigTcp (struct _SnortConfig *sc, void* pv)
{
    // enable all ports registered by preprocessors for reassembly
    enableRegisteredPortsForReassembly( sc );
}

void s5TcpPrintPortFilter();

/**
 * StreamVerifyTcpConfig is is called after all preprocs (static & dynamic)
 * are inited.
 */
int StreamVerifyTcpConfig(struct _SnortConfig *sc, StreamTcpConfig *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return -1;

    if (!tcp_lws_cache)
    {
        LogMessage("WARNING: Stream TCP Session Cache not initialized.\n");
        return -1;
    }

    if (config->num_policies == 0)
    {
        LogMessage("WARNING: Stream TCP no policies specified in configuration.\n");
        return -1;
    }

    if (config->default_policy == NULL)
    {
        LogMessage("WARNING: Stream TCP default policy not specified in configuration.\n");
        return -1;
    }

    /* Do this now
     * verify config is called after all preprocs (static & dynamic)
     * are inited.  Gives us the correct number of bits for
     * p->preprocessor_bits
     */
    if (!s5_pkt)
        StreamInitPacket();

#ifdef TARGET_BASED
    SFAT_SetPolicyIds(StreamPolicyIdFromHostAttributeEntry, policy_id);
#endif

    /* Post-process TCP rules to establish TCP ports to inspect. */
    setPortFilterList(sc, config->port_filter, IPPROTO_TCP,
            (config->default_policy->flags & STREAM_CONFIG_IGNORE_ANY), policy_id);

    //printf ("TCP Ports with Inspection/Monitoring\n");
    //s5PrintPortFilter(config->tcpPortFilter);
    return 0;
}


uint32_t StreamGetTcpPrunes(void)
{
    if( tcp_lws_cache )
        return session_api->get_session_prune_count( SESSION_PROTO_TCP );
    else
        return s5stats.tcp_prunes;
}

void StreamResetTcpPrunes(void)
{
    session_api->reset_session_prune_count( SESSION_PROTO_TCP );
}

void StreamResetTcp(void)
{
    if (snort_conf == NULL)
    {
        ErrorMessage("Snort configuration is NULL.\n");
        return;
    }

    /* Unset decoder flags for the purge */
    targetPolicyIterate(policyDecoderFlagsSaveNClear);

    s5_tcp_cleanup = 1;
    session_api->purge_session_cache(tcp_lws_cache);
    s5_tcp_cleanup = 0;
    session_api->clean_protocol_session_pool( SESSION_PROTO_TCP );

    /* Set decoder flags back to original */
    targetPolicyIterate(policyDecoderFlagsRestore);

    ResetFlushMgrs();
}


void StreamCleanTcp(void)
{
    if (snort_conf == NULL)
    {
        ErrorMessage("Snort configuration is NULL.\n");
        return;
    }

    s5stats.tcp_prunes = session_api->get_session_prune_count( SESSION_PROTO_TCP );

    /* Turn off decoder alerts since we're decoding stored
     * packets that we already alerted on. */
    targetPolicyIterate(policyDecoderFlagsSaveNClear);

    /* Set s5_tcp_cleanup to force a flush of all queued data */
    s5_tcp_cleanup = 1;
    /* Clean up session cache */
    session_api->delete_session_cache( SESSION_PROTO_TCP );
    tcp_lws_cache = NULL;

    /* Cleanup the rebuilt packet */
    if (s5_pkt)
    {
        Encode_Delete(s5_pkt);
        s5_pkt = NULL;
    }

    /* Cleanup TCP session cleanup packet */
    if (tcp_cleanup_pkt)
    {
        DeleteGrinderPkt(tcp_cleanup_pkt);
        tcp_cleanup_pkt = NULL;
    }

    /* cleanup is over... */
    s5_tcp_cleanup = 0;

    /* And turn decoder alerts back on (or whatever they were set to) */
    targetPolicyIterate(policyDecoderFlagsRestore);
}

void StreamTcpConfigFree(StreamTcpConfig *config)
{
    int i;

    if (config == NULL)
        return;

    /* Cleanup TCP Policies and the list */
    for (i = 0; i < config->num_policies; i++)
    {
        StreamTcpPolicy *policy = config->policy_list[i];

        SnortPreprocFree(policy->flush_point_list.flush_points,
                 sizeof(uint32_t) * RAND_FLUSH_POINTS, PP_STREAM,
                 PP_MEM_CATEGORY_CONFIG);

        if (policy->bound_addrs != NULL)
            sfvar_free(policy->bound_addrs);
        SnortPreprocFree(policy, sizeof( StreamTcpPolicy * ), PP_STREAM,
              PP_MEM_CATEGORY_CONFIG);
    }

    if ( config->paf_config )
        s5_paf_delete(config->paf_config);

    SnortPreprocFree(config->policy_list, 
           sizeof(StreamTcpPolicy *) * (config->num_policies), PP_STREAM,
           PP_MEM_CATEGORY_CONFIG);
    SnortPreprocFree(config, sizeof(StreamTcpConfig), PP_STREAM, PP_MEM_CATEGORY_CONFIG);
}

#ifdef STREAM_DEBUG_ENABLED
static void PrintStateMgr(StateMgr *s)
{
    LogMessage("StateMgr:\n");
    LogMessage("    state:          %s\n", state_names[s->state]);
    LogMessage("    state_queue:    %s\n", state_names[s->state_queue]);
    LogMessage("    expected_flags: 0x%X\n", s->expected_flags);
    LogMessage("    transition_seq: 0x%X\n", s->transition_seq);
    LogMessage("    stq_get_seq:    %d\n", s->stq_get_seq);
}

static void PrintStreamTracker(StreamTracker *s)
{
    LogMessage(" + StreamTracker +\n");
    LogMessage("    isn:                0x%X\n", s->isn);
    LogMessage("    ts_last:            %u\n", s->ts_last);
    LogMessage("    wscale:             %u\n", s->wscale);
    LogMessage("    mss:                0x%08X\n", s->mss);
    LogMessage("    l_unackd:           %X\n", s->l_unackd);
    LogMessage("    l_nxt_seq:          %X\n", s->l_nxt_seq);
    LogMessage("    l_window:           %u\n", s->l_window);
    LogMessage("    r_nxt_ack:          %X\n", s->r_nxt_ack);
    LogMessage("    r_win_base:         %X\n", s->r_win_base);
    LogMessage("    seglist_base_seq:   %X\n", s->seglist_base_seq);
    LogMessage("    seglist:            %p\n", (void*)s->seglist);
    LogMessage("    seglist_tail:       %p\n", (void*)s->seglist_tail);
    LogMessage("    seg_count:          %d\n", s->seg_count);
    LogMessage("    seg_bytes_total:    %d\n", s->seg_bytes_total);
    LogMessage("    seg_bytes_logical:  %d\n", s->seg_bytes_logical);

    PrintStateMgr(&s->s_mgr);
}

static void PrintTcpSession(TcpSession *ts)
{
    char buf[64];

    LogMessage("TcpSession:\n");
#ifdef DEBUG
    LogMessage("    ssn_time:           %lu\n", ts->ssn_time.tv_sec);
#endif
    sfip_ntop(&ts->tcp_server_ip, buf, sizeof(buf));
    LogMessage("    server IP:          %s\n", buf);
    sfip_ntop(&ts->tcp_client_ip, buf, sizeof(buf));
    LogMessage("    client IP:          %s\n", buf);

    LogMessage("    server port:        %d\n", ts->tcp_server_port);
    LogMessage("    client port:        %d\n", ts->tcp_client_port);

    LogMessage("    flags:              0x%X\n", ts->scb->ha_state.session_flags);

    LogMessage("Client Tracker:\n");
    PrintStreamTracker(&ts->client);
    LogMessage("Server Tracker:\n");
    PrintStreamTracker(&ts->server);
}

static void PrintTcpDataBlock(TcpDataBlock *tdb)
{
    LogMessage("TcpDataBlock:\n");
    LogMessage("    seq:    0x%08X\n", tdb->seq);
    LogMessage("    ack:    0x%08X\n", tdb->ack);
    LogMessage("    win:    %d\n", tdb->win);
    LogMessage("    end:    0x%08X\n", tdb->end_seq);
}

#ifdef STREAM_DEBUG_ENABLED
static void PrintFlushMgr(FlushMgr *fm)
{
    if(fm == NULL)
        return;

    switch(fm->flush_policy)
    {
        case STREAM_FLPOLICY_NONE:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    NONE\n"););
            break;
        case STREAM_FLPOLICY_FOOTPRINT:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    FOOTPRINT %d\n", fm->flush_pt););
            break;
        case STREAM_FLPOLICY_LOGICAL:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    LOGICAL %d\n", fm->flush_pt););
            break;
        case STREAM_FLPOLICY_RESPONSE:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    RESPONSE\n"););
            break;
        case STREAM_FLPOLICY_SLIDING_WINDOW:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    SLIDING_WINDOW %d\n", fm->flush_pt););
            break;
#if 0
        case STREAM_FLPOLICY_CONSUMED:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "          CONSUMED %d\n", fm->flush_pt););
            break;
#endif
        case STREAM_FLPOLICY_IGNORE:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    IGNORE\n"););
            break;

        case STREAM_FLPOLICY_PROTOCOL:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    PROTOCOL\n"););
            break;
    }
}
#endif  // DEBUG
#endif  // STREAM_DEBUG_ENABLED

static inline void Discard ()
{
    s5stats.tcp_discards++;
}
static inline void EventSynOnEst(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_SYN_ON_EST,                 /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_SYN_ON_EST_STR,             /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventExcessiveOverlap(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_EXCESSIVE_TCP_OVERLAPS,     /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_EXCESSIVE_TCP_OVERLAPS_STR, /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventBadTimestamp(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_BAD_TIMESTAMP,              /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_BAD_TIMESTAMP_STR,          /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventWindowTooLarge(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_WINDOW_TOO_LARGE,           /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_WINDOW_TOO_LARGE_STR,       /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventDataOnSyn(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_DATA_ON_SYN,                /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_DATA_ON_SYN_STR,            /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventDataOnClosed(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_DATA_ON_CLOSED,             /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_DATA_ON_CLOSED_STR,         /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventDataAfterReset(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_DATA_AFTER_RESET,           /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_DATA_AFTER_RESET_STR,       /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventBadSegment(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_BAD_SEGMENT,                /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_BAD_SEGMENT_STR,            /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventSessionHijackedClient(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_SESSION_HIJACKED_CLIENT,    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_SESSION_HIJACKED_CLIENT_STR, /* event msg */
            NULL);                              /* rule info ptr */
}
static inline void EventSessionHijackedServer(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_SESSION_HIJACKED_SERVER,    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_SESSION_HIJACKED_SERVER_STR, /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventDataWithoutFlags(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_DATA_WITHOUT_FLAGS,         /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_DATA_WITHOUT_FLAGS_STR,     /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventMaxSmallSegsExceeded(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_SMALL_SEGMENT,              /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_SMALL_SEGMENT_STR,          /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void Event4whs(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_4WAY_HANDSHAKE,             /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_4WAY_HANDSHAKE_STR,         /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventNoTimestamp(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_NO_TIMESTAMP,               /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_NO_TIMESTAMP_STR,           /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventBadReset(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_BAD_RST,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_BAD_RST_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventBadFin(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_BAD_FIN,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_BAD_FIN_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventBadAck(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_BAD_ACK,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_BAD_ACK_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventDataAfterRstRcvd(StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_DATA_AFTER_RST_RCVD,        /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_DATA_AFTER_RST_RCVD_STR,    /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventInternal (uint32_t eventSid)
{
    if ( !InternalEventIsEnabled(snort_conf->rate_filter_config, eventSid) )
        return;

    s5stats.internalEvents++;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Stream raised internal event %d\n", eventSid););

    SnortEventqAdd(
        GENERATOR_INTERNAL,             /* GID */
        eventSid,                       /* SID */
        1,                              /* rev */
        0,                              /* class */
        3,                              /* priority */
        STREAM_INTERNAL_EVENT_STR,      /* event msg*/
        NULL                            /* rule info ptr */
        );
}

static inline void EventWindowSlam (StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_WINDOW_SLAM,                /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_WINDOW_SLAM_STR,            /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventWindowZeroAfterFinAck (StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,        /* GID */
            STREAM_WIN_SZ_0_TCP_FIN_WAIT_1,     /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_WIN_SZ_0_TCP_FIN_WAIT_1_STR, /* event msg */
            NULL);                              /* rule info ptr */
}

static inline void EventNo3whs (StreamTcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM,       /* GID */
            STREAM_NO_3WHS,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM_NO_3WHS_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

/*
 *  Utility functions for TCP stuff
 */
#ifdef NORMALIZER
typedef enum {
    PC_TCP_ECN_SSN,
    PC_TCP_TS_NOP,
    PC_TCP_IPS_DATA,
    PC_TCP_BLOCK,
    PC_TCP_TRIM_SYN,
    PC_TCP_TRIM_RST,
    PC_TCP_TRIM_WIN,
    PC_TCP_TRIM_MSS,
    PC_MAX
} PegCounts;

static uint64_t normStats[PC_MAX][NORM_MODE_MAX];

static const char* pegName[PC_MAX] = {
    "tcp::ecn_ssn",
    "tcp::ts_nop",
    "tcp::ips_data",
    "tcp::block",
    "tcp::trim_syn",
    "tcp::trim_rst",
    "tcp::trim_win",
    "tcp::trim_mss",
};

void Stream_PrintNormalizationStats (void)
{
    int i;
    // header output by normalizer

    for ( i = 0; i < PC_MAX; i++ )
    {
        // same alignment as in Norm_PrintStats()
        LogMessage("%23s: " STDu64 "\n", pegName[i], normStats[i][NORM_MODE_ON]);
        LogMessage("Would %17s: " STDu64 "\n", pegName[i], normStats[i][NORM_MODE_WOULDA]);
    }
}

void Stream_ResetNormalizationStats (void)
{
    memset(normStats, 0, sizeof(normStats));
}

//-----------------------------------------------------------------------
// instead of centralizing all these normalizations so that
// Normalize_GetMode() is called only once, the checks and
// normalizations are localized.  this should lead to many
// fewer total checks.  however, it is best to minimize
// configuration checks on a per packet basis so there is
// still room for improvement.

static inline bool NormalDropPacket (Packet* p)
{
    NormMode mode = Normalize_GetMode(snort_conf, NORM_TCP_BLOCK);
    if ( mode != NORM_MODE_OFF )
    {
            normStats[PC_TCP_BLOCK][mode]++;
            sfBase.iPegs[PERF_COUNT_TCP_BLOCK][mode]++;
            if ( mode == NORM_MODE_ON )
            {
                Active_NapDropPacket(p);
                if (pkt_trace_enabled)
                {
                    addPktTraceData(VERDICT_REASON_STREAM, snprintf(trace_line, MAX_TRACE_LINE,
                       "Stream: TCP normalization error in timestamp, window, seq, ack, fin, flags, or unexpected data, %s\n",
                       getPktTraceActMsg()));
                }
                else addPktTraceData(VERDICT_REASON_STREAM, 0);
                return true;
            }
    }
    return false;
}

static inline bool NormalStripTimeStamp (Packet* p, int i)
{
    uint8_t* opt;
    NormMode mode =  Normalize_GetMode(snort_conf, NORM_TCP_OPT);

    if ( mode != NORM_MODE_OFF )
    {
        if ( i < 0 )
        {
            for ( i = 0; i < p->tcp_option_count; i++ )
                if ( p->tcp_options[i].code == TCPOPT_TIMESTAMP )
                    break;
            if ( i == p->tcp_option_count )
                return false;
        }

        normStats[PC_TCP_TS_NOP][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_TS_NOP][mode]++;
        if ( mode == NORM_MODE_ON )
        {
            // first set raw option bytes to nops
            opt = (uint8_t*)p->tcp_options[i].data - 2;
            memset(opt, TCPOPT_NOP, TCPOLEN_TIMESTAMP);

            // then nop decoded option code only
            p->tcp_options[i].code = TCPOPT_NOP;

            p->packet_flags |= PKT_MODIFIED;
            return true;
        }
    }
    return false;
}

static inline void NormalTrimPayload (Packet* p, uint32_t max, TcpDataBlock* tdb)
{
    uint16_t fat = p->dsize - max;
    p->dsize = max;
    p->packet_flags |= (PKT_MODIFIED|PKT_RESIZED);
    tdb->end_seq -= fat;
}

static inline bool NormalTrimPayloadIfSyn ( Packet *p, uint32_t max, TcpDataBlock *tdb )
{
    NormMode mode = Normalize_GetMode(snort_conf, NORM_TCP_TRIM_SYN);
    if ( mode != NORM_MODE_OFF && p->dsize > max )
    {
        normStats[PC_TCP_TRIM_SYN][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_TRIM_SYN][mode]++;
        if( mode == NORM_MODE_ON )
        {
            NormalTrimPayload(p, max, tdb);
            return true;
        }
    }
    return false;
}

static inline bool NormalTrimPayloadIfRst ( Packet *p, uint32_t max, TcpDataBlock *tdb )
{
    NormMode mode = Normalize_GetMode(snort_conf, NORM_TCP_TRIM_RST);
    if ( mode != NORM_MODE_OFF && p->dsize > max )
    {
        normStats[PC_TCP_TRIM_RST][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_TRIM_RST][mode]++;
        if( mode == NORM_MODE_ON )
        {
            NormalTrimPayload(p, max, tdb);
            return true;
        }
    }
    return false;
}

static inline bool NormalTrimPayloadIfWin ( Packet *p, uint32_t max, TcpDataBlock *tdb )
{
    NormMode mode = Normalize_GetMode(snort_conf, NORM_TCP_TRIM_WIN);
    if ( mode != NORM_MODE_OFF && p->dsize > max )
    {
        normStats[PC_TCP_TRIM_WIN][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_TRIM_WIN][mode]++;
        if( mode == NORM_MODE_ON )
        {
            NormalTrimPayload(p, max, tdb);
            return true;
        }
    }
    return false;
}

static inline bool NormalTrimPayloadIfMss ( Packet *p, uint32_t max, TcpDataBlock *tdb )
{
    NormMode mode = Normalize_GetMode(snort_conf, NORM_TCP_TRIM_MSS);
    if ( mode != NORM_MODE_OFF && p->dsize > max )
    {
        normStats[PC_TCP_TRIM_MSS][mode]++;
        sfBase.iPegs[PERF_COUNT_TCP_TRIM_MSS][mode]++;
        if( mode == NORM_MODE_ON )
        {
            NormalTrimPayload(p, max, tdb);
            return true;
        }
    }
    return false;
}

static inline void NormalTrackECN (TcpSession* s, const TCPHdr* tcph, int req3way)
{
    if ( !s )
        return;

    if ( TCP_ISFLAGSET(tcph, TH_SYN|TH_ACK) )
    {
        if ( !req3way || s->ecn )
            s->ecn = ((tcph->th_flags & (TH_ECE|TH_CWR)) == TH_ECE);
    }
    else if ( TCP_ISFLAGSET(tcph, TH_SYN) )
        s->ecn = TCP_ISFLAGSET(tcph, (TH_ECE|TH_CWR));
}

static inline void NormalCheckECN (TcpSession* s, Packet* p)
{
    NormMode mode = Normalize_GetMode(snort_conf, NORM_TCP_ECN_STR);
    if ( mode != NORM_MODE_OFF )
    {
        if ( !s->ecn && (p->tcph->th_flags & (TH_ECE|TH_CWR)) )
        {
            normStats[PC_TCP_ECN_SSN][mode]++;
            sfBase.iPegs[PERF_COUNT_TCP_ECN_SSN][mode]++;
            if ( mode == NORM_MODE_ON )
            {
                ((TCPHdr*)p->tcph)->th_flags &= ~(TH_ECE|TH_CWR);
                p->packet_flags |= PKT_MODIFIED;
            }
        }
    }
}
#else
#define NormalDropPacket(p)
#define NormalTrackECN(s, h, r)
#endif

void StreamUpdatePerfBaseState(SFBASE *sf_base,
        SessionControlBlock *scb,
        char newState)
{
    if (!scb)
    {
        return;
    }

    switch (newState)
    {
        case TCP_STATE_SYN_SENT:
            if (!(scb->ha_state.session_flags & SSNFLAG_COUNTED_INITIALIZE))
            {
                sf_base->iSessionsInitializing++;
                scb->ha_state.session_flags |= SSNFLAG_COUNTED_INITIALIZE;
            }
            break;
        case TCP_STATE_ESTABLISHED:
            if (!(scb->ha_state.session_flags & SSNFLAG_COUNTED_ESTABLISH))
            {
                sf_base->iSessionsEstablished++;
                EventInternal(INTERNAL_EVENT_SESSION_ADD);

                if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
                    UpdateFlowIPState(&sfFlow, IP_ARG(scb->client_ip), IP_ARG(scb->server_ip), SFS_STATE_TCP_ESTABLISHED);

                scb->ha_state.session_flags |= SSNFLAG_COUNTED_ESTABLISH;

                if ((scb->ha_state.session_flags & SSNFLAG_COUNTED_INITIALIZE) &&
                        !(scb->ha_state.session_flags & SSNFLAG_COUNTED_CLOSING))
                {
                    assert(sf_base->iSessionsInitializing);
                    sf_base->iSessionsInitializing--;
                }
            }
            break;
        case TCP_STATE_CLOSING:
            if (!(scb->ha_state.session_flags & SSNFLAG_COUNTED_CLOSING))
            {
                sf_base->iSessionsClosing++;
                scb->ha_state.session_flags |= SSNFLAG_COUNTED_CLOSING;
                if (scb->ha_state.session_flags & SSNFLAG_COUNTED_ESTABLISH)
                {
                    assert(sf_base->iSessionsEstablished);
                    sf_base->iSessionsEstablished--;

                    if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
                        UpdateFlowIPState(&sfFlow, IP_ARG(scb->client_ip), IP_ARG(scb->server_ip), SFS_STATE_TCP_CLOSED);
                }
                else if (scb->ha_state.session_flags & SSNFLAG_COUNTED_INITIALIZE)
                {
                    assert(sf_base->iSessionsInitializing);
                    sf_base->iSessionsInitializing--;
                }
            }
            break;
        case TCP_STATE_CLOSED:
            if (scb->ha_state.session_flags & SSNFLAG_COUNTED_CLOSING)
            {
                assert(sf_base->iSessionsClosing);
                sf_base->iSessionsClosing--;
            }
            else if (scb->ha_state.session_flags & SSNFLAG_COUNTED_ESTABLISH)
            {
                assert(sf_base->iSessionsEstablished);
                sf_base->iSessionsEstablished--;

                if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
                    UpdateFlowIPState(&sfFlow, IP_ARG(scb->client_ip), IP_ARG(scb->server_ip), SFS_STATE_TCP_CLOSED);
            }
            else if (scb->ha_state.session_flags & SSNFLAG_COUNTED_INITIALIZE)
            {
                assert(sf_base->iSessionsInitializing);
                sf_base->iSessionsInitializing--;
            }
            break;
        default:
            break;
    }
    sf_base->stream5_mem_in_use = session_mem_in_use;
}

//-------------------------------------------------------------------------
// ssn ingress is client; ssn egress is server

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
static inline void SetPacketHeaderFoo (TcpSession* tcpssn, const Packet* p)
{
    if ((( p->packet_flags & PKT_FROM_CLIENT ) || (p->pkth->egress_index == DAQ_PKTHDR_UNKNOWN))
          && (p->pkth->egress_group == DAQ_PKTHDR_UNKNOWN ))
    {
        tcpssn->ingress_index = p->pkth->ingress_index;
        tcpssn->ingress_group = p->pkth->ingress_group;
        // ssn egress may be unknown, but will be correct
        tcpssn->egress_index = p->pkth->egress_index;
        tcpssn->egress_group = p->pkth->egress_group;
    }
    else
    {
        if ( p->pkth->egress_index != DAQ_PKTHDR_FLOOD )
        {
            tcpssn->ingress_index = p->pkth->egress_index;
            tcpssn->ingress_group = p->pkth->egress_group;
        }
        tcpssn->egress_index = p->pkth->ingress_index;
        tcpssn->egress_group = p->pkth->ingress_group;
        tcpssn->ingress_index = p->pkth->egress_index;
        tcpssn->ingress_group = p->pkth->egress_group;
    }
#ifdef HAVE_DAQ_FLOW_ID
    tcpssn->daq_flow_id = p->pkth->flow_id;
#endif
    tcpssn->daq_flags = p->pkth->flags;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    tcpssn->address_space_id_dst = p->pkth->address_space_id_dst;
    tcpssn->address_space_id_src = p->pkth->address_space_id_src;
#else
    tcpssn->address_space_id = p->pkth->address_space_id;
#endif
}

static inline void GetPacketHeaderFoo (
        const TcpSession* tcpssn, const DAQ_PktHdr_t* phdr, EncodeFlags fwd,
        DAQ_PktHdr_t* pkth, uint32_t dir)
{
    if (((dir & PKT_FROM_CLIENT) || (tcpssn->egress_index == DAQ_PKTHDR_UNKNOWN)) 
        && (tcpssn->egress_group == DAQ_PKTHDR_UNKNOWN))
    {
        pkth->ingress_index = tcpssn->ingress_index;
        pkth->ingress_group = tcpssn->ingress_group;
        pkth->egress_index = tcpssn->egress_index;
        pkth->egress_group = tcpssn->egress_group;
    }
    else
    {
        pkth->ingress_index = tcpssn->egress_index;
        pkth->ingress_group = tcpssn->egress_group;
        pkth->egress_index = tcpssn->ingress_index;
        pkth->egress_group = tcpssn->ingress_group;
    }
#ifdef HAVE_DAQ_FLOW_ID
    pkth->flow_id = tcpssn->daq_flow_id;
#endif
    pkth->flags = tcpssn->daq_flags;
    pkth->priv_ptr = tcpssn->priv_ptr;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    pkth->address_space_id_dst = tcpssn->address_space_id_dst;
    pkth->address_space_id_src = tcpssn->address_space_id_src;
#else
    pkth->address_space_id = tcpssn->address_space_id;
#endif
#if defined(DAQ_VERSION) && DAQ_VERSION > 8
    pkth->proto = phdr->proto;
#endif

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID) && defined(DAQ_VERSION) && DAQ_VERSION > 10
    pkth->carrier_id = phdr->carrier_id;
#endif

#ifdef HAVE_DAQ_REAL_ADDRESSES
    if (phdr->flags & DAQ_PKT_FLAG_REAL_ADDRESSES)
    {
        pkth->flags &= ~(DAQ_PKT_FLAG_REAL_SIP_V6 | DAQ_PKT_FLAG_REAL_DIP_V6);
        if (fwd)
        {
            pkth->flags |= phdr->flags & (DAQ_PKT_FLAG_REAL_SIP_V6 | DAQ_PKT_FLAG_REAL_DIP_V6);
            pkth->n_real_sPort = phdr->n_real_sPort;
            pkth->n_real_dPort = phdr->n_real_dPort;
            pkth->real_sIP = phdr->real_sIP;
            pkth->real_dIP = phdr->real_dIP;
        }
        else
        {
            if (phdr->flags & DAQ_PKT_FLAG_REAL_SIP_V6)
                pkth->flags |= DAQ_PKT_FLAG_REAL_DIP_V6;
            if (phdr->flags & DAQ_PKT_FLAG_REAL_DIP_V6)
                pkth->flags |= DAQ_PKT_FLAG_REAL_SIP_V6;
            pkth->n_real_sPort = phdr->n_real_dPort;
            pkth->n_real_dPort = phdr->n_real_sPort;
            pkth->real_sIP = phdr->real_dIP;
            pkth->real_dIP = phdr->real_sIP;
        }
    }
#endif
}

static inline void SwapPacketHeaderFoo (TcpSession* tcpssn)
{
    if ( tcpssn->egress_index != DAQ_PKTHDR_UNKNOWN )
    {
        int32_t ingress_index;
        int32_t ingress_group;

        ingress_index = tcpssn->ingress_index;
        ingress_group = tcpssn->ingress_group;
        tcpssn->ingress_index = tcpssn->egress_index;
        tcpssn->ingress_group = tcpssn->egress_group;
        tcpssn->egress_index = ingress_index;
        tcpssn->egress_group = ingress_group;
    }
}
#endif


//-------------------------------------------------------------------------

static inline int IsBetween(uint32_t low, uint32_t high, uint32_t cur)
{
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "(%X, %X, %X) = (low, high, cur)\n", low,high,cur););

    /* If we haven't seen anything, ie, low & high are 0, return true */
    if ((low == 0) && (low == high))
        return 1;

    return (SEQ_GEQ(cur, low) && SEQ_LEQ(cur, high));
}

static inline bool TwoWayTraffic (SessionControlBlock *scb)
{
    return ( (scb->ha_state.session_flags & SSNFLAG_SEEN_BOTH) == SSNFLAG_SEEN_BOTH );
}

static inline uint32_t StreamGetWindow(
        SessionControlBlock *scb, StreamTracker* st, TcpDataBlock* tdb)
{
    int32_t window;

    if ( st->l_window )
    {
        // don't use the window if we may have missed scaling
        if ( !(scb->session_state & STREAM_STATE_MIDSTREAM) )
            return st->l_window;
    }
    // one way zero window is unitialized
    // two way zero window is actually closed (regardless of scaling)
    else if ( TwoWayTraffic(scb) )
        return st->l_window;

    // ensure the data is in the window
    window = tdb->end_seq - st->r_win_base;

    if(window <  0)
        window = 0;

    return (uint32_t) window;
}

// ack number must ack syn
static inline int ValidRstSynSent(StreamTracker *st, TcpDataBlock *tdb)
{
    return tdb->ack == st->l_unackd;
}

// per rfc 793 a rst is valid if the seq number is in window
// for all states but syn-sent (handled above).  however, we
// validate here based on how various implementations actually
// handle a rst.
static inline int ValidRst(
        SessionControlBlock *scb, StreamTracker *st, TcpDataBlock *tdb)
{
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking end_seq (%X) > r_win_base (%X) && "
                "seq (%X) < r_nxt_ack(%X)\n",
                tdb->end_seq, st->r_win_base, tdb->seq,
                st->r_nxt_ack+StreamGetWindow(scb, st, tdb)););

    switch (st->os_policy)
    {
        case STREAM_POLICY_HPUX11:
            if (SEQ_GEQ(tdb->seq, st->r_nxt_ack))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "rst is valid seq (>= next seq)!\n"););
                return 1;
            }
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "rst is not valid seq (>= next seq)!\n"););
            return 0;
            break;
        case STREAM_POLICY_FIRST:
        case STREAM_POLICY_NOACK:
        case STREAM_POLICY_LAST:
        case STREAM_POLICY_MACOS:
        case STREAM_POLICY_WINDOWS:
        case STREAM_POLICY_VISTA:
        case STREAM_POLICY_WINDOWS2K3:
        case STREAM_POLICY_HPUX10:
        case STREAM_POLICY_IRIX:
            if (SEQ_EQ(tdb->seq, st->r_nxt_ack))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "rst is valid seq (next seq)!\n"););
                return 1;
            }
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "rst is not valid seq (next seq)!\n"););
            return 0;
            break;
        case STREAM_POLICY_BSD:
        case STREAM_POLICY_LINUX:
        case STREAM_POLICY_OLD_LINUX:
        case STREAM_POLICY_SOLARIS:
            if(SEQ_GEQ(tdb->end_seq, st->r_win_base))
            {
                // reset must be admitted when window closed
                if ( SEQ_LEQ(tdb->seq, st->r_win_base+StreamGetWindow(scb, st, tdb)) )
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "rst is valid seq (within window)!\n"););
                    return 1;
                }
            }

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "rst is not valid seq (within window)!\n"););
            return 0;
            break;
    }

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "rst is not valid!\n"););
    return 0;
}

static inline int ValidTimestamp(StreamTracker *talker, StreamTracker *listener, TcpDataBlock *tdb,
        Packet *p, int *eventcode, int *got_ts)
{
    if((p->tcph->th_flags & TH_RST) || (listener->tcp_policy->policy == STREAM_POLICY_NOACK))
        return ACTION_NOTHING;

#ifdef NORMALIZER
#if 0
    if ( p->tcph->th_flags & TH_ACK &&
            Normalize_GetMode(snort_conf, NORM_TCP_OPT) != NORM_MODE_OFF)
    {
        // FIXTHIS validate tsecr here (check that it was previously sent)
        // checking for the most recent ts is easy enough must check if
        // ts are up to date in retransmitted packets
    }
#endif
#endif
    /*
     * check PAWS
     */
    if((talker->flags & TF_TSTAMP) && (listener->flags & TF_TSTAMP))
    {
        char validate_timestamp = 1;
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Checking timestamps for PAWS\n"););

        *got_ts = StreamGetTcpTimestamp(p, &tdb->ts, 0);

        if (*got_ts)
        {
            if (listener->tcp_policy->policy == STREAM_POLICY_HPUX11)
            {
                /* HPUX 11 ignores timestamps for out of order segments */
                if ((listener->flags & TF_PKT_MISSED) ||
                        !SEQ_EQ(listener->r_nxt_ack, tdb->seq))
                {
                    validate_timestamp = 0;
                }
            }

            if (talker->flags & TF_TSTAMP_ZERO)
            {
                /* Handle the case where the 3whs used a 0 timestamp.  Next packet
                 * from that endpoint should have a valid timestamp... */
                if ((listener->tcp_policy->policy == STREAM_POLICY_LINUX) ||
                        (listener->tcp_policy->policy == STREAM_POLICY_WINDOWS2K3))
                {
                    /* Linux, Win2k3 et al.  do not support timestamps if
                     * the 3whs used a 0 timestamp. */
                    talker->flags &= ~TF_TSTAMP;
                    listener->flags &= ~TF_TSTAMP;
                    validate_timestamp = 0;
                }
                else if ((listener->tcp_policy->policy == STREAM_POLICY_OLD_LINUX) ||
                        (listener->tcp_policy->policy == STREAM_POLICY_WINDOWS) ||
                        (listener->tcp_policy->policy == STREAM_POLICY_VISTA))
                {
                    /* Older Linux (2.2 kernel & earlier), Win32 (non 2K3)
                     * allow the 3whs to use a 0 timestamp. */
                    talker->flags &= ~TF_TSTAMP_ZERO;
                    if(SEQ_EQ(listener->r_nxt_ack, tdb->seq))
                    {
                        talker->ts_last = tdb->ts;
                        validate_timestamp = 0; /* Ignore the timestamp for this
                                                 * first packet, next one will
                                                 * checked. */
                    }
                }
            }

            if (validate_timestamp)
            {
                int result = 0;
                if (listener->tcp_policy->policy == STREAM_POLICY_LINUX)
                {
                    /* Linux 2.6 accepts timestamp values that are off
                     * by one. */
                    result = (int)((tdb->ts - talker->ts_last) + 1);
                }
                else
                {
                    result = (int)(tdb->ts - talker->ts_last);
                }

#ifdef DAQ_PKT_FLAG_RETRY_PACKET
                if(result < 0 && (p->pkth->flags & DAQ_PKT_FLAG_RETRY_PACKET))
                {
                    //  Retry packets can legitimately have old timestamps
                    //  in TCP options (if a re-transmit comes in before
                    //  the retry) so don't consider it an error.
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Retry packet had old timestamp.  Reseting to last timestamp seen.\n"););
                    tdb->ts = talker->ts_last;
                }
                else
#endif
                if( (talker->ts_last != 0) && result < 0)
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Packet outside PAWS window, dropping\n"););
                    /* bail, we've got a packet outside the PAWS window! */
                    //Discard();
                    *eventcode |= EVENT_BAD_TIMESTAMP;
                    NormalDropPacket(p);
                    return ACTION_BAD_PKT;
                }
                else if ((talker->ts_last != 0) &&
                        ((uint32_t)p->pkth->ts.tv_sec > talker->ts_last_pkt+PAWS_24DAYS))
                {
                    /* this packet is from way too far into the future */
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "packet PAWS timestamp way too far ahead of"
                                "last packet %d %d...\n", p->pkth->ts.tv_sec,
                                talker->ts_last_pkt););
                    //Discard();
                    *eventcode |= EVENT_BAD_TIMESTAMP;
                    NormalDropPacket(p);
                    return ACTION_BAD_PKT;
                }
                else
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "packet PAWS ok...\n"););
                }
            }
        }
        else
        {
            /* we've got a packet with no timestamp, but 3whs indicated talker
             * was doing timestamps.  This breaks protocol, however, some servers
             * still ack the packet with the missing timestamp.  Log an alert,
             * but continue to process the packet
             */
            *eventcode |= EVENT_NO_TIMESTAMP;
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "packet no timestamp, had one earlier from this side...ok for now...\n"););

            if (listener->tcp_policy->policy == STREAM_POLICY_SOLARIS)
            {
                /* Solaris stops using timestamps if it receives a packet
                 * without a timestamp and there were timestamps in use.
                 */
                listener->flags &= ~TF_TSTAMP;
            }
            NormalDropPacket(p);
        }
    }
    else if ( TCP_ISFLAGSET(p->tcph, TH_SYN) &&
            !TCP_ISFLAGSET(p->tcph, TH_ACK) )
    {
        *got_ts = StreamGetTcpTimestamp(p, &tdb->ts, 0);
        if ( *got_ts ) {
            talker->flags |= TF_TSTAMP;
            // In case of SYN, there is no ts_last is 0
            // set it to the current value, so that computation of 
            // ts_last is correct for subsequent packets.
            talker->ts_last = tdb->ts;
            talker->ts_last_pkt = p->pkth->ts.tv_sec;
        }
    }
    else
    {
#ifdef NORMALIZER
        // if we are not handling timestamps, and this isn't a syn
        // (only), and we have seen a valid 3way setup, then we strip
        // (nop) the timestamp option.  this includes the cases where
        // we disable timestamp handling.
        int strip = ( SetupOK(talker) && SetupOK(listener) );
#else
        int strip = 0;
#endif
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "listener not doing timestamps...\n"););
        *got_ts = StreamGetTcpTimestamp(p, &tdb->ts, strip);

        if (*got_ts)
        {
            if (!(talker->flags & TF_TSTAMP))
            {
                /* Since we skipped the SYN, may have missed the talker's
                 * timestamp there, so set it now.
                 */
                talker->flags |= TF_TSTAMP;
                if (tdb->ts == 0)
                {
                    talker->flags |= TF_TSTAMP_ZERO;
                }
            }

            /* Only valid to test this if listener is using timestamps.
             * Otherwise, timestamp in this packet is not used, regardless
             * of its value. */
            if ((tdb->ts == 0) && (listener->flags & TF_TSTAMP))
            {
                switch (listener->os_policy)
                {
                    case STREAM_POLICY_WINDOWS:
                    case STREAM_POLICY_VISTA:
                    case STREAM_POLICY_WINDOWS2K3:
                    case STREAM_POLICY_OLD_LINUX:
                    case STREAM_POLICY_SOLARIS:
                        /* Old Linux & Windows allows a 0 timestamp value. */
                        break;
                    default:
                        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "Packet with 0 timestamp, dropping\n"););
                        //Discard();
                        /* bail */
                        *eventcode |= EVENT_BAD_TIMESTAMP;
                        return ACTION_BAD_PKT;
                }
            }
        }
    }
    return ACTION_NOTHING;
}

#ifdef S5_PEDANTIC
// From RFC 793:
//
//    Segment Receive  Test
//    Length  Window
//    ------- -------  -------------------------------------------
//
//       0       0     SEG.SEQ = RCV.NXT
//
//       0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
//
//      >0       0     not acceptable
//
//      >0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
//                     or RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND
//
static inline int ValidSeq(
        const Packet* p, SessionControlBlock *scb, StreamTracker *st, TcpDataBlock *tdb, bool *before_win_base)
{
    uint32_t win = StreamGetWindow(scb, st, tdb);

    if ( !p->dsize )
    {
        if ( !win )
        {
            return ( tdb->seq == st->r_win_base );
        }
        return SEQ_LEQ(st->r_win_base, tdb->seq) &&
            SEQ_LT(tdb->seq, st->r_win_base+win);
    }
    if ( !win )
        return 0;

    if ( SEQ_LEQ(st->r_win_base, tdb->seq) &&
            SEQ_LT(tdb->seq, st->r_win_base+win) )
        return 1;

    return SEQ_LEQ(st->r_win_base, tdb->end_seq) &&
        SEQ_LT(tdb->end_seq, st->r_win_base+win);
}
#else
static inline int ValidSeq(
        const Packet* p, SessionControlBlock *scb, StreamTracker *st, TcpDataBlock *tdb, bool *before_win_base)
{
    int right_ok;
    uint32_t left_seq;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking end_seq (%X) > r_win_base (%X) && "
                "seq (%X) < r_nxt_ack(%X)\n",
                tdb->end_seq, st->r_win_base, tdb->seq,
                st->r_nxt_ack+StreamGetWindow(scb, st, tdb)););

    if ( SEQ_LT(st->r_nxt_ack, st->r_win_base) )
        left_seq = st->r_nxt_ack;
    else
        left_seq = st->r_win_base;

    if ( p->dsize )
        right_ok = SEQ_GT(tdb->end_seq, left_seq);
    else
        right_ok = SEQ_GEQ(tdb->end_seq, left_seq);

    if ( right_ok )
    {
        uint32_t win = StreamGetWindow(scb, st, tdb);

        if( SEQ_LEQ(tdb->seq, st->r_win_base+win) )
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "seq is within window!\n"););
            return 1;
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "seq is past the end of the window!\n"););
        }
    }
    else
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "end_seq is before win_base\n"););
        *before_win_base = true;
    }
    return 0;
}
#endif

static inline void UpdateSsn(Packet* p, StreamTracker *rcv, StreamTracker *snd, TcpDataBlock *tdb)
{
#if 0
#ifdef NORMALIZER
    if (
            // FIXTHIS these checks are a hack to avoid off by one normalization
            // due to FIN ... if last segment filled a hole, r_nxt_ack is not at
            // end of data, FIN is ignored so sequence isn't bumped, and this
            // forces seq-- on ACK of FIN.  :(
        rcv->s_mgr.state == TCP_STATE_ESTABLISHED &&
            rcv->s_mgr.state_queue == TCP_STATE_NONE &&
            Normalize_GetMode(snort_conf, NORM_TCP_IPS) )
            {
            // walk the seglist until a gap or tdb->ack whichever is first
            // if a gap exists prior to ack, move ack back to start of gap
            StreamSegment* seg = snd->seglist;

            // FIXTHIS must check ack oob with empty seglist
            // FIXTHIS add lower gap bound to tracker for efficiency?
            while ( seg )
            {
            uint32_t seq = seg->seq + seg->size;
            if ( SEQ_LEQ(tdb->ack, seq) )
            break;

            seg = seg->next;

            if ( !seg || seg->seq > seq )
            {
                // normalize here
                tdb->ack = seq;
                ((TCPHdr*)p->tcph)->th_ack = htonl(seq);
                p->packet_flags |= PKT_MODIFIED;
                break;
            }
            }
            }
#endif
#endif
    // ** if we don't see a segment, we can't track seq at ** below
    // so we update the seq by the ack if it is beyond next expected
    if(SEQ_GT(tdb->ack, rcv->l_unackd))
        rcv->l_unackd = tdb->ack;

    // ** this is how we track the last seq number sent
    // as is l_unackd is the "last left" seq recvd
    snd->l_unackd = tdb->seq;

    if (SEQ_GT(tdb->end_seq, snd->l_nxt_seq))
        snd->l_nxt_seq = tdb->end_seq;

#ifdef S5_PEDANTIC
    if ( SEQ_GT(tdb->ack, snd->r_win_base) &&
            SEQ_LEQ(tdb->ack, snd->r_nxt_ack) )
#else
        if ( SEQ_GT(tdb->ack, snd->r_win_base) )
#endif
            snd->r_win_base = tdb->ack;

    snd->l_window = tdb->win;
}

static void StreamInitPacket(void)
{
    s5_pkt = Encode_New();
}

static inline void SetupTcpDataBlock(TcpDataBlock *tdb, Packet *p)
{
    tdb->seq = ntohl(p->tcph->th_seq);
    tdb->ack = ntohl(p->tcph->th_ack);
    tdb->win = ntohs(p->tcph->th_win);
    tdb->end_seq = tdb->seq + (uint32_t) p->dsize;
    tdb->ts = 0;

    if(p->tcph->th_flags & TH_SYN)
    {
        tdb->end_seq++;
    }
    // don't bump end_seq for fin here
    // we will bump if/when fin is processed

    return;
}

static void SegmentFree (StreamSegment *seg)
{
    unsigned dropped = sizeof(StreamSegment);

    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "Dumping segment at seq %X, size %d, caplen %d\n",
                seg->seq, seg->size, seg->caplen););

    if ( seg->caplen > 0 )
        dropped += seg->caplen - 1;  // seg contains 1st byte

    session_mem_in_use -= dropped;
    SnortPreprocFree(seg, dropped, PP_STREAM, PP_MEM_CATEGORY_SESSION);
    s5stats.tcp_streamsegs_released++;

    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "SegmentFree dropped %d bytes\n", dropped););
}

static void DeleteSeglist(StreamSegment *listhead)
{
    StreamSegment *idx = listhead;
    StreamSegment *dump_me;
    int i = 0;

    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "In DeleteSeglist\n"););
    while(idx)
    {
        i++;
        dump_me = idx;
        idx = idx->next;
        SegmentFree(dump_me);
    }

    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "Dropped %d segments\n", i););
}

static inline int purge_alerts(StreamTracker *st, uint32_t flush_seq, void *ssnptr)
{
    int i;
    int new_count = 0;

    for (i=0;i<st->alert_count;i++)
    {
        StreamAlertInfo* ai = st->alerts + i;

        if (SEQ_LT(ai->seq, flush_seq) )
        {
            if(st->xtradata_mask && extra_data_log)
            {
                extra_data_log(
                        ssnptr, extra_data_config, xtradata_map,
                        xtradata_func_count, st->xtradata_mask,
                        ai->event_id, ai->event_second);
            }
            memset(ai, 0, sizeof(*ai));
        }
        else
        {
            if (new_count != i)
            {
                st->alerts[new_count] = st->alerts[i];
            }
            new_count++;
        }
    }
    st->alert_count = new_count;

    return new_count;
}

static inline int purge_to_seq(TcpSession *tcpssn, StreamTracker *st, uint32_t flush_seq)
{
    StreamSegment *ss = NULL;
    StreamSegment *dump_me = NULL;
    int purged_bytes = 0;
    uint32_t last_ts = 0;

    if(st->seglist == NULL)
    {
        if ( SEQ_LT(st->seglist_base_seq, flush_seq) )
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "setting st->seglist_base_seq to 0x%X\n", flush_seq););
            st->seglist_base_seq = flush_seq;
        }
        return 0;
    }

    ss = st->seglist;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In purge_to_seq, start seq = 0x%X end seq = 0x%X delta %d\n",
                ss->seq, flush_seq, flush_seq-ss->seq););
    while(ss)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "s: %X  sz: %d\n", ss->seq, ss->size););
        dump_me = ss;

        ss = ss->next;
        if(SEQ_LT(dump_me->seq, flush_seq))
        {
            if (dump_me->ts > last_ts)
            {
                last_ts = dump_me->ts;
            }
            purged_bytes += StreamSeglistDeleteNodeTrim(st, dump_me, flush_seq);
        }
        else
            break;
    }

    if ( SEQ_LT(st->seglist_base_seq, flush_seq) )
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "setting st->seglist_base_seq to 0x%X\n", flush_seq););
        st->seglist_base_seq = flush_seq;
    }
    if ( SEQ_LT(st->r_nxt_ack, flush_seq) )
        st->r_nxt_ack = flush_seq;

    purge_alerts(st, flush_seq, (void *)tcpssn->scb);

    if (st->seglist == NULL)
    {
        st->seglist_tail = NULL;
    }

    /* Update the "last" time stamp seen from the other side
     * to be the most recent timestamp (largest) that was removed
     * from the queue.  This will ensure that as we go forward,
     * last timestamp is the highest one that we had stored and
     * purged and handle the case when packets arrive out of order,
     * such as:
     * P1: seq 10, length 10, timestamp 10
     * P3: seq 30, length 10, timestamp 30
     * P2: seq 20, length 10, timestamp 20
     *
     * Without doing it this way, the timestamp would be 20.  With
     * the next packet to arrive (P4, seq 40), the ts_last value
     * wouldn't be updated for the talker in ProcessTcp() since that
     * code specificially looks for the NEXT sequence number.
     */
    if ( !last_ts )
        return purged_bytes;

    if (st == &tcpssn->client)
    {
        int32_t delta = last_ts - tcpssn->server.ts_last;
        if (delta > 0)
            tcpssn->server.ts_last = last_ts;
    }
    else if (st == &tcpssn->server)
    {
        int32_t delta = last_ts - tcpssn->client.ts_last;
        if (delta > 0)
            tcpssn->client.ts_last = last_ts;
    }

    return purged_bytes;
}

static inline void purge_all (StreamTracker *st)
{
    DeleteSeglist(st->seglist);
    st->seglist = st->seglist_tail = st->seglist_next = NULL;
    st->seg_count = st->flush_count = 0;
    st->seg_bytes_total = st->seg_bytes_logical = 0;
}

// purge_flushed_ackd():
// * must only purge flushed and acked bytes
// * we may flush partial segments
// * must adjust seq->seq and seg->size when a flush gets only the
//   initial part of a segment
// * FIXTHIS need flag to mark any reassembled packets that have a gap
//   (if we reassemble such)
static inline int purge_flushed_ackd (TcpSession *tcpssn, StreamTracker *st)
{
    StreamSegment* seg = st->seglist;
    uint32_t seq;

    if ( !st->seglist )
        return 0;

    seq = st->seglist->seq;

    while ( seg && seg->buffered )
    {
        uint32_t end = seg->seq + seg->size;

        if ( SEQ_GT(end, st->r_win_base) )
        {
            seq = st->r_win_base;
            break;
        }
        seq = end;
        seg = seg->next;
    }
    if ( seq != st->seglist->seq )
        return purge_to_seq(tcpssn, st, seq);

    return 0;
}

static void ShowRebuiltPacket (Packet* p)
{
    if(stream_session_config->flags & STREAM_CONFIG_SHOW_PACKETS)
    {
        //ClearDumpBuf();
        printf("+++++++++++++++++++Stream Packet+++++++++++++++++++++\n");
        PrintIPPkt(stdout, IPPROTO_TCP, p);
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        //ClearDumpBuf();
    }
}

static inline int _flush_to_seq_noack( TcpSession *tcpssn, StreamTracker *st, uint32_t bytes,
        Packet *p, sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir )
{
    uint32_t stop_seq;
    uint32_t footprint = 0;
    uint32_t bytes_processed = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpFlushPerfStats);

    if(p->dsize == 0)
        return bytes_processed;

    // if not specified, set bytes to flush to what was acked
    if ( !bytes && SEQ_GT(st->r_win_base, st->seglist_base_seq) )
        bytes = st->r_win_base - st->seglist_base_seq;

    stop_seq = st->seglist_base_seq + bytes;

    {
        footprint = stop_seq - st->seglist_base_seq;

        if(footprint == 0)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Negative footprint, bailing %d (0x%X - 0x%X)\n",
                        footprint, stop_seq, st->seglist_base_seq););
            PREPROC_PROFILE_END(s5TcpFlushPerfStats);

            return bytes_processed;
        }

#if 0
        //Might not need this as we are not buffering the packets??
#ifdef DEBUG_STREAM
        if(footprint < st->seg_bytes_logical)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Footprint less than queued bytes, "
                        "win_base: 0x%X base_seq: 0x%X\n",
                        stop_seq, st->seglist_base_seq););
        }
#endif
#endif

#if 0
        //Won't need this as we are not buffering the packets
        if(footprint > p->max_dsize)
        {
            /* this is as much as we can pack into a stream buffer */
            footprint = p->max_dsize;
            stop_seq = st->seglist_base_seq + footprint;
        }
#endif

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attempting to flush %lu bytes\n", footprint););

        st->seglist_base_seq = stop_seq;
        st->seglist_next->buffered = SL_BUF_FLUSHED;
        st->flush_count++;
        p->packet_flags &= ~PKT_STREAM_INSERT;
        p->packet_flags |= (PKT_REBUILT_STREAM|PKT_STREAM_EST);
        bytes_processed = p->dsize;

        sfBase.iStreamFlushes++;
        ShowRebuiltPacket(p);
        s5stats.tcp_rebuilt_packets++;
        UpdateStreamReassStats(&sfBase, bytes_processed);

    }

    if ( st->tcp_policy )
        UpdateFlushMgr(snort_conf, &st->flush_mgr, &st->tcp_policy->flush_point_list, st->flags);

    /* tell them how many bytes we processed */
    PREPROC_PROFILE_END(s5TcpFlushPerfStats);
    return bytes_processed;
}

static inline unsigned int getSegmentFlushSize(
        StreamTracker* st,
        StreamSegment *ss,
        uint32_t to_seq,
        unsigned int flushBufSize
        )
{
    unsigned int flushSize = ss->size;

    //copy only till flush buffer gets full
    if ( flushSize > flushBufSize )
        flushSize = flushBufSize;

    // copy only to flush point
    if ( s5_paf_active(&st->paf_state) && SEQ_GT(ss->seq + flushSize, to_seq) )
        flushSize = to_seq - ss->seq;

    return flushSize;
}

static int PseudoFlushStream(
   Packet* p, StreamTracker *st, uint32_t toSeq,
   uint8_t *flushbuf, const uint8_t *flushbuf_end)
{
    StreamSegment *ss = NULL, *seglist;
    uint16_t bytes_flushed = 0;
    uint32_t flushbuf_size, bytes_to_copy;
    int ret;
    PROFILE_VARS;

    if ( st->seglist == NULL || st->seglist_tail == NULL )
        return -1;

    PREPROC_PROFILE_START(s5TcpBuildPacketPerfStats);

    // skip over previously pseudo flushed segments
    seglist = st->seglist_next;

    for(ss = seglist; ss && SEQ_LT(ss->seq,  toSeq); ss = ss->next)
    {
        flushbuf_size = flushbuf_end - flushbuf;
        bytes_to_copy = getSegmentFlushSize(st, ss, toSeq, flushbuf_size);

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                   "Copying %u bytes for pseudo flushing from %X\n",
                   bytes_to_copy, ss->seq););

        ret = SafeMemcpy(flushbuf, ss->payload,
                    bytes_to_copy, flushbuf, flushbuf_end);

        if (ret == SAFEMEM_ERROR)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "ERROR writing flushbuf while pseudo flushing."
                            "Attempting to write flushbuf out of range!\n"););
        }
        else
        {
            flushbuf += bytes_to_copy;
        }

        bytes_flushed += bytes_to_copy;
        if (ss->next)
        {
            st->seglist_next = ss->next;
        }

        if ( flushbuf >= flushbuf_end )
            break;

        if ( SEQ_EQ(ss->seq + bytes_to_copy,  toSeq) )
            break;

    }

    PREPROC_PROFILE_END(s5TcpBuildPacketPerfStats);
    return bytes_flushed;
}

static inline void pseudo_flush(
TcpSession *tcpssn, StreamTracker *st, uint32_t bytes, Packet *p,
        sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t start_seq;
    uint32_t stop_seq;
    uint32_t footprint = 0;
    uint32_t bytes_processed = 0;
    int32_t flushed_bytes;
    StreamSegment *ss = NULL;

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    DAQ_PktHdr_t pkth;
#endif
    EncodeFlags enc_flags = 0;
    PROFILE_VARS;

    if (!bytes)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
               "No bytes to pseudo flush\n"););
        return;
    }
    PREPROC_PROFILE_START(s5TcpFlushPerfStats);

    if ( !p->packet_flags || (dir & p->packet_flags) )
        enc_flags = ENC_FLAG_FWD;

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    GetPacketHeaderFoo(tcpssn, p->pkth, enc_flags, &pkth, dir);
    Encode_Format_With_DAQ_Info(enc_flags, p, s5_pkt, PSEUDO_PKT_TCP, &pkth, 0);
#elif defined(HAVE_DAQ_ACQUIRE_WITH_META)
    Encode_Format_With_DAQ_Info(enc_flags, p, s5_pkt, PSEUDO_PKT_TCP, 0);
#else
    Encode_Format(enc_flags, p, s5_pkt, PSEUDO_PKT_TCP);
#endif

    s5_pkt_end = s5_pkt->data + s5_pkt->max_dsize;

    ss = st->seglist_next;
    stop_seq = st->seglist_next->seq + bytes;
    do
    {
        footprint = bytes;
        if(footprint > s5_pkt->max_dsize)
        {
            /* this is as much as we can pack into a stream buffer */
            footprint = s5_pkt->max_dsize;
            stop_seq = st->seglist_next->seq + footprint;
        }

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attempting to pseudo flush %lu bytes\n", footprint););
        /* Capture the seq of the first octet before flush changes the sequence numbers */
        start_seq = htonl(st->seglist_next->seq);

        /* setup the pseudopacket payload */
        flushed_bytes = PseudoFlushStream(p, st, stop_seq, (uint8_t *)s5_pkt->data, s5_pkt_end);

        if(flushed_bytes == -1)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Failed to pseudo flush the stream\n"););
            return;
        }
        if (flushed_bytes == 0)
        {
            /* No more ACK'd data... bail */
            break;
        }

        bytes_processed += flushed_bytes;

        ((TCPHdr *)s5_pkt->tcph)->th_seq = start_seq;
        s5_pkt->packet_flags |= (PKT_REBUILT_STREAM|PKT_STREAM_EST);
        s5_pkt->dsize = (uint16_t)flushed_bytes;

        s5_pkt->packet_flags |= PKT_PDU_TAIL;
        s5_pkt->packet_flags |= PKT_PSEUDO_FLUSH;

        Encode_Update(s5_pkt);

        if(IS_IP4(s5_pkt))
        {
            s5_pkt->inner_ip4h.ip_len = s5_pkt->iph->ip_len;
        }
        else
        {
            IP6RawHdr* ip6h = (IP6RawHdr*)s5_pkt->raw_ip6h;
            if ( ip6h ) s5_pkt->inner_ip6h.len = ip6h->ip6plen;
        }

        ((DAQ_PktHdr_t*)s5_pkt->pkth)->ts.tv_sec = st->seglist_next->tv.tv_sec;
        ((DAQ_PktHdr_t*)s5_pkt->pkth)->ts.tv_usec = st->seglist_next->tv.tv_usec;

        s5_pkt->packet_flags |= dir;
        s5_pkt->ssnptr = (void *) tcpssn->scb;
#ifdef TARGET_BASED
        s5_pkt->application_protocol_ordinal = p->application_protocol_ordinal;
#endif
        PREPROC_PROFILE_TMPEND(s5TcpFlushPerfStats);
        {
            int tmp_do_detect, tmp_do_detect_content;
            PROFILE_VARS;

            PREPROC_PROFILE_START(s5TcpProcessRebuiltPerfStats);
            tmp_do_detect = do_detect;
            tmp_do_detect_content = do_detect_content;

            SnortEventqPush();
            Preprocess(s5_pkt);
            SnortEventqPop();
            DetectReset(s5_pkt->data, s5_pkt->dsize);

            do_detect = tmp_do_detect;
            do_detect_content = tmp_do_detect_content;
            PREPROC_PROFILE_END(s5TcpProcessRebuiltPerfStats);
        }
        PREPROC_PROFILE_TMPSTART(s5TcpFlushPerfStats);
    } while(bytes_processed < bytes);

    st->seglist_next = ss;
    PREPROC_PROFILE_END(s5TcpFlushPerfStats);
}

static inline int _flush_to_seq (

        TcpSession *tcpssn, StreamTracker *st, uint32_t bytes, Packet *p,
        sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t start_seq;
    uint32_t stop_seq;
    uint32_t footprint = 0;
    uint32_t bytes_processed = 0;
    int32_t flushed_bytes;

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    DAQ_PktHdr_t pkth;
#endif
    EncodeFlags enc_flags = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpFlushPerfStats);

    if(st->paf_state.fpt_eoh)
        tcpssn->pp_flags |= PP_HTTPINSPECT_PAF_FLUSH_POST_HDR;
    else
        tcpssn->pp_flags &= ~PP_HTTPINSPECT_PAF_FLUSH_POST_HDR;

    if ( !p->packet_flags || (dir & p->packet_flags) )
        enc_flags = ENC_FLAG_FWD;

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    GetPacketHeaderFoo(tcpssn, p->pkth, enc_flags, &pkth, dir);
    Encode_Format_With_DAQ_Info(enc_flags, p, s5_pkt, PSEUDO_PKT_TCP, &pkth, 0);
#elif defined(HAVE_DAQ_ACQUIRE_WITH_META)
    Encode_Format_With_DAQ_Info(enc_flags, p, s5_pkt, PSEUDO_PKT_TCP, 0);
#else
    Encode_Format(enc_flags, p, s5_pkt, PSEUDO_PKT_TCP);
#endif

    s5_pkt_end = s5_pkt->data + s5_pkt->max_dsize;

    // TBD in ips mode, these should be coming from current packet (tdb)
    ((TCPHdr *)s5_pkt->tcph)->th_ack = htonl(st->l_unackd);
    ((TCPHdr *)s5_pkt->tcph)->th_win = htons((uint16_t)st->l_window);

    // if not specified, set bytes to flush to what was acked
    if ( !bytes && SEQ_GT(st->r_win_base, st->seglist_base_seq) )
        bytes = st->r_win_base - st->seglist_base_seq;

    stop_seq = st->seglist_base_seq + bytes;

    do
    {
        footprint = stop_seq - st->seglist_base_seq;

        if(footprint == 0)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Negative footprint, bailing %d (0x%X - 0x%X)\n",
                        footprint, stop_seq, st->seglist_base_seq););
            PREPROC_PROFILE_END(s5TcpFlushPerfStats);

            return bytes_processed;
        }

#ifdef STREAM_DEBUG_ENABLED
        if(footprint < st->seg_bytes_logical)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Footprint less than queued bytes, "
                        "win_base: 0x%X base_seq: 0x%X\n",
                        stop_seq, st->seglist_base_seq););
        }
#endif

        if(footprint > s5_pkt->max_dsize)
        {
            /* this is as much as we can pack into a stream buffer */
            footprint = s5_pkt->max_dsize;
            stop_seq = st->seglist_base_seq + footprint;
        }

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attempting to flush %lu bytes\n", footprint););

        /* Capture the seq of the first octet before flush changes the sequence numbers */
        start_seq = htonl(st->seglist_next->seq);

        /* setup the pseudopacket payload */
        flushed_bytes = FlushStream(p, st, stop_seq, (uint8_t *)s5_pkt->data, s5_pkt_end);

        if(flushed_bytes == -1)
        {
            /* couldn't put a stream together for whatever reason
             * should probably clean the seglist and bail...
             */
            if(st->seglist)
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "dumping entire seglist!\n"););
                purge_all(st);
            }

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "setting st->seglist_base_seq to 0x%X\n", stop_seq););
            st->seglist_base_seq = stop_seq;

            PREPROC_PROFILE_END(s5TcpFlushPerfStats);
            return bytes_processed;
        }

        if (flushed_bytes == 0)
        {
            /* No more ACK'd data... bail */
            break;
        }

        ((TCPHdr *)s5_pkt->tcph)->th_seq = start_seq;
        s5_pkt->packet_flags |= (PKT_REBUILT_STREAM|PKT_STREAM_EST);
        s5_pkt->dsize = (uint16_t)flushed_bytes;

        if ((p->packet_flags & PKT_PDU_TAIL))
            s5_pkt->packet_flags |= PKT_PDU_TAIL;
        
        /* Rebuilt packet with only and complete http-post header is set with PKT_EVAL_DROP, 
         * it will be used to evalute drop/allow packet by preprocs  
         */
        if (tcpssn->pp_flags & PP_HTTPINSPECT_PAF_FLUSH_POST_HDR)
            s5_pkt->packet_flags |= PKT_EVAL_DROP;

        Encode_Update(s5_pkt);

        if(IS_IP4(s5_pkt))
        {
            s5_pkt->inner_ip4h.ip_len = s5_pkt->iph->ip_len;
        }
        else
        {
            IP6RawHdr* ip6h = (IP6RawHdr*)s5_pkt->raw_ip6h;
            if ( ip6h ) s5_pkt->inner_ip6h.len = ip6h->ip6plen;
        }

        ((DAQ_PktHdr_t*)s5_pkt->pkth)->ts.tv_sec = st->seglist_next->tv.tv_sec;
        ((DAQ_PktHdr_t*)s5_pkt->pkth)->ts.tv_usec = st->seglist_next->tv.tv_usec;

        sfBase.iStreamFlushes++;
        bytes_processed += s5_pkt->dsize;

        s5_pkt->packet_flags |= dir;
        s5_pkt->ssnptr = (void *) tcpssn->scb;
#ifdef TARGET_BASED
        s5_pkt->application_protocol_ordinal = p->application_protocol_ordinal;
#endif
        ShowRebuiltPacket(s5_pkt);
        s5stats.tcp_rebuilt_packets++;
        UpdateStreamReassStats(&sfBase, flushed_bytes);

        PREPROC_PROFILE_TMPEND(s5TcpFlushPerfStats);
        {
            int tmp_do_detect, tmp_do_detect_content;
            PROFILE_VARS;

            PREPROC_PROFILE_START(s5TcpProcessRebuiltPerfStats);
            tmp_do_detect = do_detect;
            tmp_do_detect_content = do_detect_content;

            SnortEventqPush();
            Preprocess(s5_pkt);
            SnortEventqPop();
            DetectReset(s5_pkt->data, s5_pkt->dsize);

            do_detect = tmp_do_detect;
            do_detect_content = tmp_do_detect_content;
            PREPROC_PROFILE_END(s5TcpProcessRebuiltPerfStats);
        }
        PREPROC_PROFILE_TMPSTART(s5TcpFlushPerfStats);

        // TBD abort should be by PAF callback only since
        // recovery may be possible in some cases
        if ( st->flags & TF_MISSING_PKT )
        {
            st->flags |= TF_MISSING_PREV_PKT;
            st->flags |= TF_PKT_MISSED;
            st->flags &= ~TF_MISSING_PKT;
            s5stats.tcp_gaps++;
        }
        else
        {
            st->flags &= ~TF_MISSING_PREV_PKT;
        }
    } while ( DataToFlush(st) );

    if ( st->tcp_policy )
        UpdateFlushMgr(snort_conf, &st->flush_mgr, &st->tcp_policy->flush_point_list, st->flags);

    /* tell them how many bytes we processed */
    PREPROC_PROFILE_END(s5TcpFlushPerfStats);
    return bytes_processed;
}

static inline int flush_to_seq_noack( TcpSession *tcpssn, StreamTracker *st, uint32_t bytes,
        Packet *p, sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir )
{
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In flush_to_seq_noack()\n"););

    if ( !bytes )
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bailing, no data\n"););
        return 0;
    }

    if ( !st->seglist_next )
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bailing, bad seglist ptr\n"););
        return 0;
    }

#if 0
    //Might not need this. Check with Russ.
    if (!DataToFlush(st) && !(st->flags & TF_FORCE_FLUSH))
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "only 1 packet in seglist no need to flush\n"););
        return 0;
    }
#endif

    st->flags &= ~TF_MISSING_PKT;
    st->flags &= ~TF_MISSING_PREV_PKT;

    /* This will set this flag on the first reassembly
     * if reassembly for this direction was set midstream */
    if ( SEQ_LT(st->seglist_base_seq, st->seglist_next->seq) )
    {
        uint32_t missed = st->seglist_next->seq - st->seglist_base_seq;

        if ( missed <= bytes )
            bytes -= missed;

        st->flags |= TF_MISSING_PREV_PKT;
        st->flags |= TF_PKT_MISSED;
        s5stats.tcp_gaps++;
        st->seglist_base_seq = st->seglist_next->seq;

        if ( !bytes )
            return 0;
    }

    return _flush_to_seq_noack(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);

}
/*
 * flush a seglist up to the given point, generate a pseudopacket,
 * and fire it thru the system.
 */
static inline int flush_to_seq(
        TcpSession *tcpssn, StreamTracker *st, uint32_t bytes, Packet *p,
        sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In flush_to_seq()\n"););

    if ( !bytes )
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bailing, no data\n"););
        return 0;
    }

    if ( !st->seglist_next )
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bailing, bad seglist ptr\n"););
        return 0;
    }

    if (!DataToFlush(st) && !(st->flags & TF_FORCE_FLUSH))
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "only 1 packet in seglist no need to flush\n"););
        return 0;
    }

    st->flags &= ~TF_MISSING_PREV_PKT;

    /* This will set this flag on the first reassembly
     * if reassembly for this direction was set midstream */
    if ( SEQ_LT(st->seglist_base_seq, st->seglist_next->seq) &&
            !(st->flags & TF_FIRST_PKT_MISSING) )
    {
        uint32_t missed = st->seglist_next->seq - st->seglist_base_seq;

        if ( missed <= bytes )
            bytes -= missed;

        st->flags |= TF_MISSING_PREV_PKT;
        st->flags |= TF_PKT_MISSED;
        s5stats.tcp_gaps++;
        st->seglist_base_seq = st->seglist_next->seq;

        if ( !bytes )
            return 0;
    }
    st->flags &= ~TF_FIRST_PKT_MISSING;

    if (p->packet_flags & PKT_PSEUDO_FLUSH)
    {
         pseudo_flush(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
         return 0;
    }
 
    return _flush_to_seq(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);

}

/*
 * get the footprint for the current seglist, the difference
 * between our base sequence and the last ack'd sequence we
 * received
 */
static inline uint32_t get_q_footprint(StreamTracker *st)
{
    uint32_t fp;

    if (st == NULL)
    {
        return 0;
    }

    fp = st->r_win_base - st->seglist_base_seq;

    if(fp <= 0)
        return 0;

    st->seglist_next = st->seglist;
    return fp;
}

static bool two_way_traffic=true;
// FIXTHIS get_q_sequenced() performance could possibly be
// boosted by tracking sequenced bytes as seglist is updated
// to avoid the while loop, etc. below.
static inline uint32_t get_q_sequenced(StreamTracker *st)
{
    uint32_t len;
    StreamSegment* seg = st ? st->seglist : NULL;
    StreamSegment* base = NULL;

    if ( !seg )
        return 0;

    if ( two_way_traffic && SEQ_LT(st->r_win_base, seg->seq) )
        return 0;

    while ( seg->next && (seg->next->seq == seg->seq + seg->size) )
    {
        if ( !seg->buffered && !base )
            base = seg;
        seg = seg->next;
    }
    if ( !seg->buffered && !base )
        base = seg;

    if ( !base )
        return 0;

    st->seglist_next = base;
    st->seglist_base_seq = base->seq;
    len = seg->seq + seg->size - base->seq;

    return ( len > 0 ) ? len : 0;
}

static inline int flush_ackd(
        TcpSession *tcpssn, StreamTracker *st, Packet *p,
        sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t bytes = get_q_footprint(st);
    return flush_to_seq(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
}

// FIXTHIS flush_stream() calls should be replaced with calls to
// CheckFlushPolicyOn*() with the exception that for the *OnAck() case,
// any available ackd data must be flushed in both directions.
static inline int flush_stream(
        TcpSession *tcpssn, StreamTracker *st, Packet *p,
        sfaddr_t* sip, sfaddr_t* dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    FlushMgr* fm;
#ifdef NORMALIZER
    if ( Stream_NormGetMode(st->reassembly_policy, snort_conf, NORM_TCP_IPS) == NORM_MODE_ON )
    {
        uint32_t bytes = get_q_sequenced(st);
        return flush_to_seq(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
    }
#endif
    fm = &tcpssn->server.flush_mgr;
    if(fm->flush_policy == STREAM_FLPOLICY_PROTOCOL_NOACK)
    {
        uint32_t bytes = get_q_sequenced(st);
        return flush_to_seq(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
    }
    else if (fm->flush_policy == STREAM_FLPOLICY_FOOTPRINT_NOACK)
    {
        uint32_t bytes = get_q_sequenced(st);
        return flush_to_seq_noack(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
    }
    return flush_ackd(tcpssn, st, p, sip, dip, sp, dp, dir);
}

static int FlushStream(
        Packet* p, StreamTracker *st, uint32_t toSeq, uint8_t *flushbuf,
        const uint8_t *flushbuf_end)
{
    StreamSegment *ss = NULL, *seglist, *sr;
    uint16_t bytes_flushed = 0;
    uint16_t bytes_skipped = 0;
    uint32_t bytes_queued = st->seg_bytes_logical;
    uint32_t segs = 0;
    int ret;
    PROFILE_VARS;

    if ( st->seglist == NULL || st->seglist_tail == NULL )
        return -1;

    PREPROC_PROFILE_START(s5TcpBuildPacketPerfStats);

    // skip over previously flushed segments
    seglist = st->seglist_next;

    for(ss = seglist; ss && SEQ_LT(ss->seq,  toSeq); ss = ss->next)
    {
        unsigned int flushbuf_size = flushbuf_end - flushbuf;
        unsigned int bytes_to_copy = getSegmentFlushSize(st, ss, toSeq, flushbuf_size);

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Flushing %u bytes from %X\n", bytes_to_copy, ss->seq));

        if(ss->urg_offset >= 1)
        {
            /* if urg_offset is set, seq + urg_offset is seq # of octet
             * in stream following the last urgent octet.  all preceding
             * octets in segment are considered urgent.  this code will
             * skip over the urgent data when flushing.
             */

            unsigned int non_urgent_bytes =
                ss->urg_offset < bytes_to_copy ? (bytes_to_copy - ss->urg_offset) : 0;

            if ( non_urgent_bytes )
            {
                ret = SafeMemcpy(flushbuf, ss->payload+ss->urg_offset,
                        non_urgent_bytes, flushbuf, flushbuf_end);

                if (ret == SAFEMEM_ERROR)
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "ERROR writing flushbuf attempting to "
                                "write flushbuf out of range!\n"););
                }
                else
                    flushbuf += non_urgent_bytes;

                bytes_skipped += ss->urg_offset;
            }
            else
            {
                ss->urg_offset = 0;
            }
        }
        else
        {
            ret = SafeMemcpy(flushbuf, ss->payload,
                    bytes_to_copy, flushbuf, flushbuf_end);

            if (ret == SAFEMEM_ERROR)
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "ERROR writing flushbuf attempting to "
                            "write flushbuf out of range!\n"););
            }
            else
                flushbuf += bytes_to_copy;
        }

        if ( !st->paf_state.fpt_eoh &&
                bytes_to_copy < ss->size &&
                DupStreamNode(NULL, st, ss, &sr) == STREAM_INSERT_OK )
        {
            ss->size = bytes_to_copy;
            sr->seq += bytes_to_copy;
            sr->size -= bytes_to_copy;
            sr->payload += bytes_to_copy + (ss->payload - ss->data);
        }

        bytes_flushed += bytes_to_copy;
        if(!st->paf_state.fpt_eoh){
            ss->buffered = SL_BUF_FLUSHED;
            st->flush_count++;
            segs++;
        }

        if ( flushbuf >= flushbuf_end )
            break;

        if ( SEQ_EQ(ss->seq + bytes_to_copy,  toSeq) )
            break;

        /* Check for a gap/missing packet */
        // FIXTHIS PAF should account for missing data and resume
        // scanning at the start of next PDU instead of aborting.
        // FIXTHIS FIN may be in toSeq causing bogus gap counts.
        if ( ((ss->next && (ss->seq + ss->size != ss->next->seq)) ||
                    (!ss->next && (ss->seq + ss->size < toSeq))) &&
                !(st->flags & TF_FIRST_PKT_MISSING) )
        {
            if ( ss->next )
            {
                toSeq = ss->next->seq;
                st->seglist_next = ss->next;
            }
            st->flags |= TF_MISSING_PKT;
            break;
        }
    }

    st->seglist_base_seq = toSeq;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "setting st->seglist_base_seq to 0x%X\n", st->seglist_base_seq););

    bytes_queued -= bytes_flushed;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "flushed %d bytes / %d segs on stream, "
                "skipped %d bytes, %d still queued\n",
                bytes_flushed, segs, bytes_skipped, bytes_queued););

    PREPROC_PROFILE_END(s5TcpBuildPacketPerfStats);
    return bytes_flushed - bytes_skipped;
}

int StreamFlushServer(Packet *p, SessionControlBlock *scb)
{
    int flushed;
    TcpSession *tcpssn = NULL;
    StreamTracker *flushTracker = NULL;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return 0;

    flushTracker = &tcpssn->server;

    flushTracker->flags |= TF_FORCE_FLUSH;

    /* If this is a rebuilt packet, don't flush now because we'll
     * overwrite the packet being processed.
     */
    if (p->packet_flags & PKT_REBUILT_STREAM)
    {
        /* We'll check & clear the TF_FORCE_FLUSH next time through */
        return 0;
    }

    /* Need to convert the addresses to network order */
    flushed = flush_stream(tcpssn, flushTracker, p,
            &tcpssn->tcp_server_ip,
            &tcpssn->tcp_client_ip,
            tcpssn->tcp_server_port,
            tcpssn->tcp_client_port,
            PKT_FROM_SERVER);
    if (flushed)
        purge_flushed_ackd(tcpssn, flushTracker);

    flushTracker->flags &= ~TF_FORCE_FLUSH;

    return flushed;
}

int StreamFlushClient(Packet *p, SessionControlBlock *scb)
{
    int flushed;
    TcpSession *tcpssn = NULL;
    StreamTracker *flushTracker = NULL;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return 0;

    flushTracker = &tcpssn->client;

    flushTracker->flags |= TF_FORCE_FLUSH;

    /* If this is a rebuilt packet, don't flush now because we'll
     * overwrite the packet being processed.
     */
    if (p->packet_flags & PKT_REBUILT_STREAM)
    {
        /* We'll check & clear the TF_FORCE_FLUSH next time through */
        return 0;
    }

    /* Need to convert the addresses to network order */
    flushed = flush_stream(tcpssn, flushTracker, p,
            &tcpssn->tcp_client_ip,
            &tcpssn->tcp_server_ip,
            tcpssn->tcp_client_port,
            tcpssn->tcp_server_port,
            PKT_FROM_CLIENT);
    if (flushed)
        purge_flushed_ackd(tcpssn, flushTracker);

    flushTracker->flags &= ~TF_FORCE_FLUSH;

    return flushed;
}

int StreamFlushListener(Packet *p, SessionControlBlock *scb)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *listener = NULL;
    int dir = 0;
    int flushed = 0;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return 0;

    /* figure out direction of this packet -- we should've already
     * looked at it, so the packet_flags are already set. */
    if(p->packet_flags & PKT_FROM_SERVER)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Flushing listener on packet from server\n"););
        listener = &tcpssn->client;
        /* dir of flush is the data from the opposite side */
        dir = PKT_FROM_SERVER;
    }
    else if (p->packet_flags & PKT_FROM_CLIENT)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Flushing listener on packet from client\n"););
        listener = &tcpssn->server;
        /* dir of flush is the data from the opposite side */
        dir = PKT_FROM_CLIENT;
    }

    if (dir != 0)
    {
        listener->flags |= TF_FORCE_FLUSH;
        flushed = flush_stream(tcpssn, listener, p,
                GET_SRC_IP(p), GET_DST_IP(p),
                p->tcph->th_sport, p->tcph->th_dport, dir);
        if (flushed)
            purge_flushed_ackd(tcpssn, listener);

        listener->flags &= ~TF_FORCE_FLUSH;
    }

    return flushed;
}

int StreamFlushTalker(Packet *p, SessionControlBlock *scb)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *talker = NULL;
    int dir = 0;
    int flushed = 0;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

    /* figure out direction of this packet -- we should've already
     * looked at it, so the packet_flags are already set. */
    if(p->packet_flags & PKT_FROM_SERVER)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Flushing talker on packet from server\n"););
        talker = &tcpssn->server;
        /* dir of flush is the data from the opposite side */
        dir = PKT_FROM_CLIENT;
    }
    else if (p->packet_flags & PKT_FROM_CLIENT)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Flushing talker on packet from client\n"););
        talker = &tcpssn->client;
        /* dir of flush is the data from the opposite side */
        dir = PKT_FROM_SERVER;
    }

    if (dir != 0)
    {
        talker->flags |= TF_FORCE_FLUSH;
        flushed = flush_stream(tcpssn, talker, p,
                GET_DST_IP(p), GET_SRC_IP(p),
                p->tcph->th_dport, p->tcph->th_sport, dir);
        if (flushed)
            purge_flushed_ackd(tcpssn, talker);

        talker->flags &= ~TF_FORCE_FLUSH;
    }

    return flushed;
}

static void TcpSessionClear (SessionControlBlock* scb, TcpSession* tcpssn, int freeApplicationData)
{
    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "In TcpSessionClear, %lu bytes in use\n", session_mem_in_use););
    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "client has %d segs queued\n", tcpssn->client.seg_count););
    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "server has %d segs queued\n", tcpssn->server.seg_count););

    // update stats
    s5stats.tcp_streamtrackers_released++;
    if (s5stats.active_tcp_sessions > 0 )
        s5stats.active_tcp_sessions--;
    StreamUpdatePerfBaseState(&sfBase, tcpssn->scb, TCP_STATE_CLOSED);
    RemoveStreamSession(&sfBase);

    if (scb->ha_state.session_flags & SSNFLAG_PRUNED)
        CloseStreamSession(&sfBase, SESSION_CLOSED_PRUNED);
    else if (scb->ha_state.session_flags & SSNFLAG_TIMEDOUT)
        CloseStreamSession(&sfBase, SESSION_CLOSED_PRUNED | SESSION_CLOSED_TIMEDOUT);
    else
        CloseStreamSession(&sfBase, SESSION_CLOSED_NORMALLY);


    // release external state
    if (freeApplicationData)
        session_api->free_application_data(scb);
    StreamResetFlowBits(scb);

    // release internal protocol specific state
    purge_all(&tcpssn->client);
    purge_all(&tcpssn->server);

    s5_paf_clear(&tcpssn->client.paf_state);
    s5_paf_clear(&tcpssn->server.paf_state);

    session_api->free_protocol_session_pool( SESSION_PROTO_TCP, scb );
    scb->proto_specific_data = NULL;

    // update light-weight state
    scb->ha_state.session_flags = SSNFLAG_NONE;
    scb->session_state = STREAM_STATE_NONE;
    scb->expire_time = 0;
    scb->ha_state.ignore_direction = 0;

    // generate event for rate filtering
    EventInternal(INTERNAL_EVENT_SESSION_DEL);

    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "After cleaning, %lu bytes in use\n", session_mem_in_use););
}
static StreamTcpPolicy *StreamSearchTcpConfigForBoundPolicy(StreamTcpConfig *tcp_config, sfaddr_t *ip)
{
    int policyIndex;
    StreamTcpPolicy *policy = NULL;

    for (policyIndex = 0; policyIndex < tcp_config->num_policies; policyIndex++)
    {
        policy = tcp_config->policy_list[policyIndex];

        if (policy->bound_addrs == NULL)
            continue;

        /*
         * Does this policy handle packets to this IP address?
         */
        if(sfvar_ip_in(policy->bound_addrs, ip))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                        "[Stream] Found tcp policy in IpAddrSet\n"););
            break;
        }
    }

    if (policyIndex == tcp_config->num_policies)
        policy = tcp_config->default_policy;

    return policy;
}

static inline StreamTcpPolicy *StreamPolicyLookup( SessionControlBlock *scb, sfaddr_t *ip)
{
    StreamTcpConfig *tcp_config = ( ( StreamConfig * ) scb->stream_config )->tcp_config;

    if( tcp_config != NULL )
        return StreamSearchTcpConfigForBoundPolicy( tcp_config, ip );
    else
        return NULL;
}

static void FlushQueuedSegs(SessionControlBlock *scb, TcpSession *tcpssn)
{
    DAQ_PktHdr_t tmp_pcap_hdr;

#ifdef REG_TEST
    if (getRegTestFlags() & REG_TEST_FLAG_STREAM_DECODE)
        printf("\nFlushQueuedSegs | ");
#endif

    /* Flush ack'd data on both sides as necessary */
    {
        Packet *p = NewGrinderPkt(tcp_cleanup_pkt, NULL, NULL);

        int flushed;

        tcp_cleanup_pkt = p;

        if (!s5_tcp_cleanup)
        {
            /* Turn off decoder alerts since we're decoding stored
             * packets that we already alerted on. */
            policyDecoderFlagsSaveNClear(scb->napPolicyId);
        }
        policyChecksumFlagsSaveNClear(scb->napPolicyId);

        /* Flush the client */
        if (tcpssn->client.seglist && !(scb->ha_state.ignore_direction & SSN_DIR_FROM_SERVER) )
        {
            pc.s5tcp1++;
            /* Do each field individually because of size differences on 64bit OS */
            memset(&tmp_pcap_hdr, 0, sizeof(tmp_pcap_hdr));
            tmp_pcap_hdr.ts.tv_sec = tcpssn->client.seglist->tv.tv_sec;
            tmp_pcap_hdr.ts.tv_usec = tcpssn->client.seglist->tv.tv_usec;
            tmp_pcap_hdr.caplen = tcpssn->client.seglist->caplen;
            tmp_pcap_hdr.pktlen = tcpssn->client.seglist->pktlen;

            SnortEventqPush();
            (*grinder)(p, &tmp_pcap_hdr, tcpssn->client.seglist->pkt);

            p->ssnptr = scb;

            //set policy id for this packet
            {
#ifdef HAVE_DAQ_REAL_ADDRESSES
                sfaddr_t* srcIp = (p->iph)? GET_SRC_IP((p)) : NULL;
                sfaddr_t* dstIp = (p->iph)? GET_DST_IP((p)) : NULL;

                if (srcIp && dstIp)
                {
                    if (srcIp->family == AF_INET6)
                    {
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_sIP).s6_addr), &(srcIp)->ia32, sizeof(struct in6_addr));
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_dIP).s6_addr), &(dstIp)->ia32, sizeof(struct in6_addr));
                        ((DAQ_PktHdr_t*)p->pkth)->flags |= (DAQ_PKT_FLAG_REAL_SIP_V6 | DAQ_PKT_FLAG_REAL_DIP_V6);
                    }
                    else
                    {
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_sIP).s6_addr), &(srcIp)->ia32[3], sizeof(uint32_t));
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_dIP).s6_addr), &(dstIp)->ia32[3], sizeof(uint32_t));
                    }
                    ((DAQ_PktHdr_t*)p->pkth)->flags |= DAQ_PKT_FLAG_REAL_ADDRESSES;

                    if( tcpssn->daq_flags & DAQ_PKT_FLAG_IGNORE_VLAN )
                        ((DAQ_PktHdr_t*)p->pkth)->flags |= DAQ_PKT_FLAG_IGNORE_VLAN;
                }
#endif

                if( scb->session_config != session_configuration )
                {
                    getSessionPlugins()->select_session_nap(p, true );
                    scb->napPolicyId = getNapRuntimePolicy();
                    scb->ipsPolicyId = getDefaultPolicy();
                    p->configPolicyId = snort_conf->targeted_policies[scb->ipsPolicyId]->configPolicyId;
                    scb->stream_config = sfPolicyUserDataGet( stream_online_config, scb->napPolicyId );
                    tcpssn->client.tcp_policy = StreamPolicyLookup(scb, GET_DST_IP(p));
                    tcpssn->server.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
                }
                else
                {
                     setNapRuntimePolicy(scb->napPolicyId);
                     setIpsRuntimePolicy(scb->ipsPolicyId);
                     p->configPolicyId = snort_conf->targeted_policies[scb->ipsPolicyId]->configPolicyId;
                     scb->stream_config = sfPolicyUserDataGet( stream_online_config, scb->napPolicyId );
                }

                //actions are queued only for IDS case
                sfActionQueueExecAll(decoderActionQ);
            }
            SnortEventqPop();
            tcpssn->client.flags |= TF_FORCE_FLUSH;

            if ( !p->tcph )
            {
                flushed = 0;
            }
            else
            {
                flushed = flush_stream(tcpssn, &tcpssn->client, p,
                        p->iph_api->iph_ret_src(p), p->iph_api->iph_ret_dst(p),
                        p->tcph->th_sport, p->tcph->th_dport,
                        PKT_FROM_SERVER);
            }
            if (flushed)
                purge_flushed_ackd(tcpssn, &tcpssn->client);
            else
            {
                SnortEventqPush();
                SnortEventqLog(snort_conf->event_queue, p);
                SnortEventqReset();
                SnortEventqPop();
            }
            tcpssn->client.flags &= ~TF_FORCE_FLUSH;
        }

        /* Flush the server */
        if (tcpssn->server.seglist && !(scb->ha_state.ignore_direction & SSN_DIR_FROM_CLIENT) )
        {
            pc.s5tcp2++;
            /* Do each field individually because of size differences on 64bit OS */
            memset(&tmp_pcap_hdr, 0, sizeof(tmp_pcap_hdr));
            tmp_pcap_hdr.ts.tv_sec = tcpssn->server.seglist->tv.tv_sec;
            tmp_pcap_hdr.ts.tv_usec = tcpssn->server.seglist->tv.tv_usec;
            tmp_pcap_hdr.caplen = tcpssn->server.seglist->caplen;
            tmp_pcap_hdr.pktlen = tcpssn->server.seglist->pktlen;

            SnortEventqPush();
            (*grinder)(p, &tmp_pcap_hdr, tcpssn->server.seglist->pkt);

            p->ssnptr = scb;

            //set policy id for this packet
            {
#ifdef HAVE_DAQ_REAL_ADDRESSES
                sfaddr_t* srcIp = (p->iph)? GET_SRC_IP((p)) : NULL;
                sfaddr_t* dstIp = (p->iph)? GET_DST_IP((p)) : NULL;

                if (srcIp && dstIp)
                {
                    if (srcIp->family == AF_INET6)
                    {
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_sIP).s6_addr), &(srcIp)->ia32, sizeof(struct in6_addr));
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_dIP).s6_addr), &(dstIp)->ia32, sizeof(struct in6_addr));
                        ((DAQ_PktHdr_t*)p->pkth)->flags |= (DAQ_PKT_FLAG_REAL_SIP_V6 | DAQ_PKT_FLAG_REAL_DIP_V6);
                    }
                    else
                    {
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_sIP).s6_addr), &(srcIp)->ia32[3], sizeof(uint32_t));
                        memcpy(&((((DAQ_PktHdr_t*)p->pkth)->real_dIP).s6_addr), &(dstIp)->ia32[3], sizeof(uint32_t));
                    }
                    ((DAQ_PktHdr_t*)p->pkth)->flags |= DAQ_PKT_FLAG_REAL_ADDRESSES;

                    if( tcpssn->daq_flags & DAQ_PKT_FLAG_IGNORE_VLAN )
                        ((DAQ_PktHdr_t*)p->pkth)->flags |= DAQ_PKT_FLAG_IGNORE_VLAN;
                }
#endif

                if( scb->session_config != session_configuration )
                {
                    getSessionPlugins()->select_session_nap(p, false );
                    scb->ipsPolicyId = getDefaultPolicy();
                    scb->napPolicyId = getNapRuntimePolicy();
                    p->configPolicyId = snort_conf->targeted_policies[scb->ipsPolicyId]->configPolicyId;
                    scb->stream_config = sfPolicyUserDataGet( stream_online_config, scb->napPolicyId );
                    tcpssn->client.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
                    tcpssn->server.tcp_policy = StreamPolicyLookup(scb, GET_DST_IP(p));
                }
                else
                {
                     setNapRuntimePolicy(scb->napPolicyId);
                     setIpsRuntimePolicy(scb->ipsPolicyId);
                     p->configPolicyId = snort_conf->targeted_policies[scb->ipsPolicyId]->configPolicyId;
                     scb->stream_config = sfPolicyUserDataGet( stream_online_config, scb->napPolicyId );
                }
                
		//actions are queued only for IDS case
                sfActionQueueExecAll(decoderActionQ);
            }
            SnortEventqPop();
            tcpssn->server.flags |= TF_FORCE_FLUSH;

            if ( !p->tcph )
            {
                flushed = 0;
            }
            else
            {
                flushed = flush_stream(tcpssn, &tcpssn->server, p,
                        p->iph_api->iph_ret_src(p), p->iph_api->iph_ret_dst(p),
                        p->tcph->th_sport, p->tcph->th_dport,
                        PKT_FROM_CLIENT);
            }
            if (flushed)
                purge_flushed_ackd(tcpssn, &tcpssn->server);
            else
            {
                SnortEventqPush();
                SnortEventqLog(snort_conf->event_queue, p);
                SnortEventqReset();
                SnortEventqPop();
            }
            tcpssn->server.flags &= ~TF_FORCE_FLUSH;
        }

        if (!s5_tcp_cleanup)
        {
            /* And turn decoder alerts back on (or whatever they were set to) */
            policyDecoderFlagsRestore(scb->napPolicyId);
        }
        policyChecksumFlagsRestore(scb->napPolicyId);

    }

}

static void TcpSessionCleanup(SessionControlBlock *scb, int freeApplicationData)
{
    TcpSession *tcpssn = NULL;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
    {
        /* Huh? */
        StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_CLOSED);
        return;
    }

    stream_session_config = scb->session_config;

    FlushQueuedSegs(scb, tcpssn);

    TcpSessionClear(scb, tcpssn, freeApplicationData);
}

static void TcpSessionCleanupWithFreeApplicationData(void *scb)
{
    TcpSessionCleanup( ( SessionControlBlock * ) scb, 1);
}
 
struct session_state_cleanup_cache
{
    uint32_t old_mem_in_use;
    sfaddr_t client_ip;
    sfaddr_t server_ip;

    uint16_t client_port;
    uint16_t server_port;
    uint16_t lw_session_state;
    uint32_t lw_session_flags;
    int16_t app_proto_id;
};

static inline void cleanup_cache_session_state( SessionControlBlock *scb, struct session_state_cleanup_cache *sscc )
{
    sscc->old_mem_in_use = session_mem_in_use;

    sscc->client_port = scb->client_port;
    sscc->server_port = scb->server_port;
    sscc->lw_session_state = scb->session_state;
    sscc->lw_session_flags = scb->ha_state.session_flags;

#ifdef TARGET_BASED
    sscc->app_proto_id = scb->ha_state.application_protocol;
#else
    sscc->app_proto_id = 0;
#endif

    sfip_set_ip(&sscc->client_ip, &scb->client_ip);
    sfip_set_ip(&sscc->server_ip, &scb->server_ip);
}

static inline void cleanup_log_session_state( char *delete_reason, struct session_state_cleanup_cache *sscc )
{
    if (stream_session_config->prune_log_max &&
            (sscc->old_mem_in_use - session_mem_in_use) > stream_session_config->prune_log_max)
    {
        char *client_ip_str, *server_ip_str;
        client_ip_str = SnortStrdup(inet_ntoa(&sscc->client_ip));
        server_ip_str = SnortStrdup(inet_ntoa(&sscc->server_ip));

        LogMessage("S5: Pruned session from cache that was "
                "using %d bytes (%s). %s %d --> %s %d (%d) : "
                "LWstate 0x%x LWFlags 0x%x\n",
                sscc->old_mem_in_use - session_mem_in_use,
                delete_reason,
                client_ip_str, sscc->client_port,
                server_ip_str, sscc->server_port,
                sscc->app_proto_id, sscc->lw_session_state, sscc->lw_session_flags);

        free(client_ip_str);
        free(server_ip_str);
    }

}


#ifdef SEG_TEST
static void CheckSegments (const StreamTracker* a)
{
    StreamSegment* ss = a->seglist;
    uint32_t sx = ss ? ss->seq : 0;

    while ( ss )
    {
        if ( SEQ_GT(sx, ss->seq) )
        {
            const int SEGBORK = 0;
            assert(SEGBORK);
        }
        sx = ss->seq + ss->size;
        ss = ss->next;
    }
}
#endif

#ifdef REG_TEST
#define LCL(p, x)    (p->x - p->isn)
#define RMT(p, x, q) (p->x - (q ? q->isn : 0))

static int s5_trace_enabled = -1;

static void TraceEvent (
        const Packet* p, TcpDataBlock* tdb, uint32_t txd, uint32_t rxd
        ) {
    int i;
    char flags[7] = "UAPRSF";
    const TCPHdr* h = p->tcph;
    const char* order = "";

    if ( !h )
        return;

    for ( i = 0; i < 6; i++)
        if ( !((1<<(5-i)) & h->th_flags) ) flags[i] = '-';

    // force relative ack to zero if not conveyed
    if ( flags[1] != 'A' ) rxd = ntohl(h->th_ack);

    if ( p->packet_flags & PKT_STREAM_ORDER_OK )
        order = " (ins)";

    else if ( p->packet_flags & PKT_STREAM_ORDER_BAD )
        order = " (oos)";

    fprintf(stdout,
            "\n" FMTu64("-3") " %s=0x%02x Seq=%-4u Ack=%-4u Win=%-4u Len=%-4u%s\n",
            //"\n" FMTu64("-3") " %s=0x%02x Seq=%-4u Ack=%-4u Win=%-4u Len=%-4u End=%-4u%s\n",
            pc.total_from_daq, flags, h->th_flags,
            ntohl(h->th_seq)-txd, ntohl(h->th_ack)-rxd,
            ntohs(h->th_win), p->dsize, order
            //ntohs(h->th_win), p->dsize,  tdb->end_seq-txd, order
           );
}

static void TraceSession (const SessionControlBlock* lws)
{
    fprintf(stdout, "    LWS: ST=0x%x SF=0x%x CP=%u SP=%u\n",
            (unsigned)lws->session_state, lws->ha_state.session_flags,
            (unsigned)ntohs(lws->client_port), (unsigned)ntohs(lws->server_port));
}

static const char* statext[] = {
    "NON", "LST", "SYR", "SYS", "EST", "CLW",
    "LAK", "FW1", "CLG", "FW2", "TWT", "CLD"
};

static const char* flushxt[] = {
    "NON", "FPR", "LOG", "RSP", "SLW",
#if 0
    "CON",
#endif
    "IGN", "PRO",
#ifdef NORMALIZER
    "PRE", "PAF"
#endif
};

static void TraceSegments (const StreamTracker* a)
{
    StreamSegment* ss = a->seglist;
    uint32_t sx = a->r_win_base;
    unsigned segs = 0, bytes = 0;

    while ( ss )
    {
        if ( SEQ_LT(sx, ss->seq) )
            fprintf(stdout, " +%u", ss->seq-sx);
        else if ( SEQ_GT(sx, ss->seq) )
            fprintf(stdout, " -%u", sx-ss->seq);

        fprintf(stdout, " %u", ss->size);

        segs++;
        bytes += ss->size;

        sx = ss->seq + ss->size;
        ss = ss->next;
    }
    assert(a->seg_count == segs);
    assert(a->seg_bytes_logical == bytes);
}

static void TraceState (const StreamTracker* a, const StreamTracker* b, const char* s)
{
    uint32_t why = a->l_nxt_seq ? LCL(a, l_nxt_seq) : 0;

    fprintf(stdout,
            "    %s ST=%s:%02x   UA=%-4u NS=%-4u LW=%-5u RN=%-4u RW=%-4u ",
            s, statext[a->s_mgr.state], a->s_mgr.sub_state,
            LCL(a, l_unackd), why, a->l_window,
            RMT(a, r_nxt_ack, b), RMT(a, r_win_base, b)
           );
    if ( a->s_mgr.state_queue )
        fprintf(stdout,
                "QS=%s QC=0x%02x QA=%-4u",
                statext[a->s_mgr.state_queue], a->s_mgr.expected_flags,
                RMT(a, s_mgr.transition_seq, b)
               );
    fprintf(stdout, "\n");
    fprintf(stdout,
            "         FP=%s:%-4u SC=%-4u FL=%-4u SL=%-5u BS=%-4u",
            flushxt[a->flush_mgr.flush_policy], a->flush_mgr.flush_pt,
            a->seg_count, a->flush_count, a->seg_bytes_logical,
            a->seglist_base_seq - b->isn
           );
    if ( s5_trace_enabled == 2 )
        TraceSegments(a);

    fprintf(stdout, "\n");
}

static void TraceTCP (
        const Packet* p, const SessionControlBlock* lws, TcpDataBlock* tdb, int event
        ) {
    const TcpSession* ssn = (lws && lws->proto_specific_data) ?
        (TcpSession*)lws->proto_specific_data->data : NULL;
    const StreamTracker* srv = ssn ? &ssn->server : NULL;
    const StreamTracker* cli = ssn ? &ssn->client : NULL;

    const char* cdir = "?", *sdir = "?";
    uint32_t txd = 0, rxd = 0;

    if ( p->packet_flags & PKT_FROM_SERVER )
    {
        sdir = "SRV>";
        cdir = "CLI<";
        if ( srv ) txd = srv->isn;
        if ( cli ) rxd = cli->isn;
    }
    else if ( p->packet_flags & PKT_FROM_CLIENT )
    {
        sdir = "SRV<";
        cdir = "CLI>";
        if ( cli ) txd = cli->isn;
        if ( srv ) rxd = srv->isn;
    }
    TraceEvent(p, tdb, txd, rxd);

    if ( lws && lws->session_established ) TraceSession(lws);

    if ( !event )
    {
        if ( cli ) TraceState(cli, srv, cdir);
        if ( srv ) TraceState(srv, cli, sdir);
    }
}

static inline void S5TraceTCP (
        const Packet* p, const SessionControlBlock* lws, TcpDataBlock* tdb, int event
        ) {
    if ( !s5_trace_enabled )
        return;

    if ( s5_trace_enabled < 0 )
    {
        const char* s5t = getenv("S5_TRACE");

        if ( !s5t ) {
            s5_trace_enabled = 0;
            return;
        }
        // no error checking required - atoi() is sufficient
        s5_trace_enabled = atoi(s5t);
    }
    TraceTCP(p, lws, tdb, event);
}
#endif  // REG_TEST

/*
 * Main entry point for TCP
 */
int StreamProcessTcp(Packet *p, SessionControlBlock *scb, StreamTcpPolicy *s5TcpPolicy, SessionKey *skey)
{
    TcpDataBlock tdb;
    SFXHASH_NODE *hash_node = NULL;
    int rc, status;
    PROFILE_VARS;
#if defined(DAQ_CAPA_CST_TIMEOUT)
    uint64_t timeout;
#endif

    STREAM_DEBUG_WRAP(
            char flagbuf[9];
            CreateTCPFlagString(p, flagbuf);
            DebugMessage((DEBUG_STREAM|DEBUG_STREAM_STATE),
                "Got TCP Packet 0x%X:%d ->  0x%X:%d %s\nseq: 0x%X   ack:0x%X  "
                "dsize: %u\n"
                "active sessions: %u\n",
                GET_SRC_IP(p),
                p->sp,
                GET_DST_IP(p),
                p->dp,
                flagbuf,
                ntohl(p->tcph->th_seq), ntohl(p->tcph->th_ack), p->dsize,
                sfxhash_count(tcp_lws_cache->hashTable));
            );

    PREPROC_PROFILE_START(s5TcpPerfStats);

    if( scb->ha_state.session_flags & ( SSNFLAG_DROP_CLIENT | SSNFLAG_DROP_SERVER ) )
    {
        /* Got a packet on a session that was dropped (by a rule). */
        session_api->set_packet_direction_flag(p, scb);

        /* Drop this packet */
        if( ( ( p->packet_flags & PKT_FROM_SERVER) &&
              ( scb->ha_state.session_flags & SSNFLAG_DROP_SERVER ) )
              ||
              ( ( p->packet_flags & PKT_FROM_CLIENT ) &&
                ( scb->ha_state.session_flags & SSNFLAG_DROP_CLIENT ) ) )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Blocking %s packet as session was blocked\n",
                        p->packet_flags & PKT_FROM_SERVER ?
                        "server" : "client"););
            DisableAllDetect( p );

            if( scb->ha_state.session_flags & SSNFLAG_FORCE_BLOCK )
                Active_ForceDropSessionWithoutReset();
            else
                Active_DropSessionWithoutReset(p);

#ifdef ACTIVE_RESPONSE
            StreamActiveResponse(p, scb);
#endif
            if (pkt_trace_enabled)
                addPktTraceData(VERDICT_REASON_STREAM, snprintf(trace_line, MAX_TRACE_LINE,
                    "Stream: TCP session was already blocked, %s\n", getPktTraceActMsg()));
            else addPktTraceData(VERDICT_REASON_STREAM, 0);

            PREPROC_PROFILE_END(s5TcpPerfStats);
            return ACTION_NOTHING;
        }
    }

    if( s5TcpPolicy == NULL )
    {
        /* Find an Tcp policy for this packet */
        s5TcpPolicy = StreamPolicyLookup( scb, GET_DST_IP( p ) );
        if( !s5TcpPolicy )
        {
            STREAM_DEBUG_WRAP( DebugMessage( DEBUG_STREAM,
                               "[Stream] Could not find Tcp Policy context "
                               "for IP %s\n", inet_ntoa( GET_DST_ADDR( p ) ) ); );
            PREPROC_PROFILE_END( s5TcpPerfStats );
            return 0;
        }
#if defined(DAQ_CAPA_CST_TIMEOUT)
        if (Daq_Capa_Timeout)
        {
           GetTimeout(p,&timeout);
           s5TcpPolicy->session_timeout = timeout + STREAM_DELAY_TIMEOUT_AFTER_CONNECTION_ENDED; 
        }
#endif
    }

    rc = isPacketFilterDiscard( p,
            ( ( ( StreamConfig * ) scb->stream_config )->tcp_config->default_policy->flags & STREAM_CONFIG_IGNORE_ANY ) );
    if( rc == PORT_MONITOR_PACKET_DISCARD )
    {
        //ignore the packet
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                    "[Stream] %s:%d -> %s:%d Packet discarded due to port filtering\n",
                    inet_ntoa(GET_SRC_ADDR(p)),p->sp,inet_ntoa(GET_DST_ADDR(p)),p->dp););

        UpdateFilteredPacketStats(&sfBase, IPPROTO_TCP);
        PREPROC_PROFILE_END(s5TcpPerfStats);
        return 0;
    }

    SetupTcpDataBlock( &tdb, p );

#ifdef STREAM_DEBUG_ENABLED
    PrintTcpDataBlock( &tdb );
#endif

    if( !scb->session_established )
    {
        if( s5TcpPolicy->flags & STREAM_CONFIG_REQUIRE_3WHS )
        {
            /* if require 3WHS, set session state to SYN rx'ed if we got one */
            if( TCP_ISFLAGSET( p->tcph, TH_SYN ) && !TCP_ISFLAGSET( p->tcph, TH_ACK ) )
            {
                /* SYN only */
                scb->session_state = STREAM_STATE_SYN;
                scb->session_established = true;
                scb->proto_policy = s5TcpPolicy;
                s5stats.total_tcp_sessions++;
                s5stats.active_tcp_sessions++;
            }
            else
            {
                /* If we're within the "startup" window, try to handle
                 * this packet as midstream pickup -- allows for
                 * connections that already existed before snort started.
                 */
                if (p->pkth->ts.tv_sec - firstPacketTime < s5TcpPolicy->hs_timeout)
                {
                    midstream_allowed = 1;
                    goto midstream_pickup_allowed;
                }
                /*
                 * Do nothing with this packet since we require a 3-way.
                 * Wow that just sounds cool... Require a 3-way.  Hehe.
                 */
                DEBUG_WRAP(
                        DebugMessage(DEBUG_STREAM_STATE, "Stream: Requiring 3-way "
                            "Handshake, but failed to retrieve session object "
                            "for non SYN packet.\n"););

                midstream_allowed = 0;
                EventNo3whs(s5TcpPolicy);
                PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
                S5TraceTCP(p, scb, &tdb, 1);
#endif
                return 0;
            }
        }
        else
        {
midstream_pickup_allowed:
            if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
            {
                /* If we have a SYN/ACK */
                scb->session_established = true;
                scb->proto_policy = s5TcpPolicy;
                s5stats.total_tcp_sessions++;
                s5stats.active_tcp_sessions++;
            }
            else if (p->dsize > 0)
            {
                /* If we have data -- missed the SYN/ACK
                 * somehow -- maybe just an incomplete PCAP.
                 * This handles data on SYN situations
                 */
                scb->session_established = true;
                scb->proto_policy = s5TcpPolicy;
                s5stats.total_tcp_sessions++;
                s5stats.active_tcp_sessions++;
            }
            else if( ( ( ( ( StreamConfig * ) scb->stream_config )->tcp_config->session_on_syn ||
                        ( StreamPacketHasWscale(p) & TF_WSCALE ) )
                    && TCP_ISFLAGSET(p->tcph, TH_SYN) ) || StreamExpectIsExpected( p, &hash_node ) )
            {
                /* If we have a wscale option or this is an expected connection, need to save the
                 * option if its the first SYN from client or continue to get the expected session
                 * data. */
                scb->session_state = STREAM_STATE_SYN;
                scb->session_established = true;
                scb->proto_policy = s5TcpPolicy;
                s5stats.total_tcp_sessions++;
                s5stats.active_tcp_sessions++;
            }
            else
            {
                /* No data, no need to create session yet */
                /* This is done to handle SYN flood DoS attacks */
#ifdef STREAM_DEBUG_ENABLED
                if (TCP_ISFLAGSET(p->tcph, TH_SYN))
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Stream: no data in packet (SYN only), no need to"
                                "create lightweight session.\n"););
                }
                else
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Stream: no data in packet (non SYN/keep alive "
                                "ACK?), no need to create lightweight session.\n"););
                }
#endif

                if(TCP_ISFLAGSET(p->tcph, TH_SYN))
                    scb->session_state |= STREAM_STATE_SYN;
                PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
                S5TraceTCP(p, scb, &tdb, 1);
#endif
                return 0;
            }
        }
    }

    p->ssnptr = scb;

    /*
     * Check if the session is expired.
     * Should be done before we do something with the packet...
     * ie, Insert a packet, or handle state change SYN, FIN, RST, etc.
     */
    if( ( scb->session_state & STREAM_STATE_TIMEDOUT) || StreamExpire( p, scb ) )
    {
        scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;

#ifdef ENABLE_HA
        /* Notify the HA peer of the session cleanup/reset by way of a deletion notification. */
        PREPROC_PROFILE_TMPEND(s5TcpPerfStats);
        SessionHANotifyDeletion(scb);
        PREPROC_PROFILE_TMPSTART(s5TcpPerfStats);
        scb->ha_flags = HA_FLAG_NEW | HA_FLAG_MODIFIED;
#endif

        /* If this one has been reset, delete the TCP portion, and start a new. */
        if( scb->ha_state.session_flags & SSNFLAG_RESET )
        {
            struct session_state_cleanup_cache sscc;

            cleanup_cache_session_state( scb, &sscc );
            TcpSessionCleanupWithFreeApplicationData(scb);
            cleanup_log_session_state( "new data/reset", &sscc );
            status = ProcessTcp( scb, p, &tdb, s5TcpPolicy, hash_node );

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Finished Stream TCP cleanly!\n"
                        "---------------------------------------------------\n"););
        }
        else
        {
            /* Not reset, simply time'd out.  Clean it up */
            struct session_state_cleanup_cache sscc;

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, "Stream TCP session timedout!\n"););

            cleanup_cache_session_state( scb, &sscc );
            TcpSessionCleanupWithFreeApplicationData( scb );
            cleanup_log_session_state( "new data/timedout", &sscc );
            status = ProcessTcp(scb, p, &tdb, s5TcpPolicy, hash_node);
        }
    }
    else
    {
        status = ProcessTcp( scb, p, &tdb, s5TcpPolicy, hash_node );
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Finished Stream TCP cleanly!\n"
                    "---------------------------------------------------\n"););
    }

    if( !( status & ACTION_LWSSN_CLOSED ) )
    {
        MarkupPacketFlags( p, scb );
        /* Receiving valid RST for a ongoing/hanged tcp session,
         * override configured session timeout to non-configurable
         * lower timeout of 180 seconds to clean up the session.
         */
        if((status & ACTION_RST) &&
            (scb->expire_time - (((uint64_t)p->pkth->ts.tv_sec ) * TCP_HZ)) > (STREAM_SSN_RST_TIMEOUT * TCP_HZ))
        {
            session_api->set_expire_timer(p, scb, STREAM_SSN_RST_TIMEOUT);
        }     
        else
        {   
            session_api->set_expire_timer( p, scb, s5TcpPolicy->session_timeout );
        }
    }
#ifdef ENABLE_HA
    else
    {
        /* TCP Session closed, so send an HA deletion event for the session. */
        SessionHANotifyDeletion( scb );
    }
#endif
    if( status & ACTION_DISABLE_INSPECTION )
    {
        session_api->disable_inspection( scb, p );

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream Ignoring packet from %d. Session marked as ignore\n",
                    p->packet_flags & PKT_FROM_SERVER? "server" : "client"););
    }

    PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
    S5TraceTCP( p, scb, &tdb, 0 );
#endif
    return 0;
}

static uint32_t StreamGetTcpTimestamp( Packet *p, uint32_t *ts, int strip )
{
    unsigned int i = 0;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Getting timestamp...\n"););
    while(i < p->tcp_option_count && i < TCP_OPTLENMAX)
    {
        if(p->tcp_options[i].code == TCPOPT_TIMESTAMP)
        {
#ifdef NORMALIZER
            if ( strip && Normalize_GetMode(snort_conf, NORM_TCP_OPT) == NORM_MODE_ON )
            {
                NormalStripTimeStamp(p, i);
            }
            else
            if ( !strip || !NormalStripTimeStamp(p, i) )
#endif
            {
                *ts = EXTRACT_32BITS(p->tcp_options[i].data);
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Found timestamp %lu\n", *ts););

                return TF_TSTAMP;
            }
        }
        i++;
    }
    *ts = 0;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "No timestamp...\n"););

    return TF_NONE;
}

static uint32_t StreamGetMss(Packet *p, uint16_t *value)
{
    unsigned int i = 0;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Getting MSS...\n"););
    while(i < p->tcp_option_count && i < TCP_OPTLENMAX)
    {
        if(p->tcp_options[i].code == TCPOPT_MAXSEG)
        {
            *value = EXTRACT_16BITS(p->tcp_options[i].data);
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Found MSS %u\n", *value););
            return TF_MSS;
        }

        i++;
    }

    *value = 0;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "No MSS...\n"););
    return TF_NONE;
}

static uint32_t StreamGetWscale(Packet *p, uint16_t *value)
{
    unsigned int i = 0;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Getting wscale...\n"););
    while(i < p->tcp_option_count && i < TCP_OPTLENMAX)
    {
        if(p->tcp_options[i].code == TCPOPT_WSCALE)
        {
            *value = (uint16_t) p->tcp_options[i].data[0];
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Found wscale %d\n", *value););

            /* If scale specified in option is larger than 14,
             * use 14 because of limitation in the math of
             * shifting a 32bit value (max scaled window is 2^30th).
             *
             * See RFC 1323 for details.
             */
            if (*value > 14)
            {
                *value = 14;
            }

            return TF_WSCALE;
        }

        i++;
    }

    *value = 0;
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "No wscale...\n"););
    return TF_NONE;
}

static uint32_t StreamPacketHasWscale(Packet *p)
{
    uint16_t wscale;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking for wscale...\n"););
    return StreamGetWscale(p, &wscale);
}

static inline int IsWellFormed(Packet *p, StreamTracker *ts)
{
    return ( !ts->mss || (p->dsize <= ts->mss) );
}

static void FinishServerInit(Packet *p, TcpDataBlock *tdb, TcpSession *ssn)
{
    StreamTracker *server;
    StreamTracker *client;

    if (!ssn)
    {
        return;
    }

    server = &ssn->server;
    client = &ssn->client;

    server->l_window = tdb->win;              /* set initial server window */
    server->l_unackd = tdb->end_seq;
    server->l_nxt_seq = server->l_unackd;
    server->isn = tdb->seq;

    client->r_nxt_ack = tdb->seq + 1;

    if ( p->tcph->th_flags & TH_FIN )
        server->l_nxt_seq--;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "seglist_base_seq = %X\n", client->seglist_base_seq););

    if (!(ssn->scb->session_state & STREAM_STATE_MIDSTREAM))
    {
        server->s_mgr.state = TCP_STATE_SYN_RCVD;
        client->seglist_base_seq = server->l_unackd;

        if ( tdb->seq == tdb->end_seq )
            client->r_win_base = tdb->end_seq;
        else
            client->r_win_base = tdb->seq + 1;
    }
    else
    {
        client->seglist_base_seq = tdb->seq;
        client->r_win_base = tdb->seq;
    }
    server->flags |= StreamGetTcpTimestamp(p, &server->ts_last, 0);
    if (server->ts_last == 0)
        server->flags |= TF_TSTAMP_ZERO;
    else
        server->ts_last_pkt = p->pkth->ts.tv_sec;
    server->flags |= StreamGetMss(p, &server->mss);
    server->flags |= StreamGetWscale(p, &server->wscale);

#ifdef STREAM_DEBUG_ENABLED
    PrintTcpSession(ssn);
#endif
}

#ifdef OLD_CODE_NOLONGER_USED_DEPENDS_ON_CURRENT_STATE
static inline void QueueState(uint8_t transition, StreamTracker *st,
        uint8_t expected_flags, uint32_t seq_num, uint8_t get_seq)
{
    StateMgr *smgr = &st->s_mgr;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "[^^] Queing transition to %s, flag 0x%X, seq: 0x%X\n",
                state_names[transition], expected_flags, seq_num););

    smgr->state_queue = transition;
    smgr->expected_flags = expected_flags;
    smgr->stq_get_seq = get_seq;
    smgr->transition_seq = seq_num;

#ifdef STREAM_DEBUG_ENABLED
    PrintStateMgr(smgr);
#endif
    return;
}

static inline int EvalStateQueue(StreamTracker *sptr, uint8_t flags,
        uint32_t ack)
{
    StateMgr *smgr = &sptr->s_mgr;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Evaluating state queue!\n"););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "StreamTracker %p, flags 0x%X ack: 0x%X\n", sptr, flags, ack);
            PrintStateMgr(smgr););

    if(smgr->expected_flags != 0)
    {
        if((flags & smgr->expected_flags) != 0)
        {
            if(smgr->stq_get_seq && (SEQ_GEQ(ack, smgr->transition_seq)))
            {

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "[^^] Accepting %s state transition\n",
                            state_names[smgr->state_queue]););
                smgr->state = smgr->state_queue;
                smgr->expected_flags = 0;
                smgr->transition_seq = 0;
                return 1;
            }
            else if(!smgr->stq_get_seq)
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "[^^] Accepting %s state transition\n",
                            state_names[smgr->state_queue]););
                smgr->state = smgr->state_queue;
                smgr->expected_flags = 0;
                smgr->transition_seq = 0;
                return 1;

            }
            else
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "[!!] sptr->stq_get_seq: %d  "
                            "[ack: 0x%X expected: 0x%X]\n", smgr->stq_get_seq,
                            ack, smgr->transition_seq););
            }
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "[!!] flags: 0x%X  expected: 0x%X, bitwise: 0x%X\n",
                        flags, smgr->expected_flags,
                        (flags & smgr->expected_flags)););
        }
    }
    else
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "No transition queued, returning\n"););
    }

    return 0;
}
#endif

static inline int IgnoreLargePkt(StreamTracker *st, Packet *p, TcpDataBlock *tdb)
{
    if((st->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT) &&
            (st->tcp_policy->flags & STREAM_CONFIG_PERFORMANCE))
    {
        if ((p->dsize > st->flush_mgr.flush_pt * 2) &&
                (st->seg_count == 0))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "WARNING: Data larger than twice flushpoint.  Not "
                        "inserting for reassembly: seq: %d, size %d!\n"
                        "This is a tradeoff of performance versus the remote "
                        "possibility of catching an exploit that spans two or "
                        "more consecuvitve large packets.\n",
                        tdb->seq, p->dsize););
            return 1;
        }
    }
    return 0;
}

static void NewQueue(StreamTracker *st, Packet *p, TcpDataBlock *tdb, TcpSession *tcpssn)
{
    StreamSegment *ss = NULL;
    uint32_t overlap = 0;
    PROFILE_VARS;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In NewQueue\n"););

    PREPROC_PROFILE_START(s5TcpInsertPerfStats);

    if(st->flush_mgr.flush_policy != STREAM_FLPOLICY_IGNORE)
    {
        uint32_t seq = tdb->seq;

        /* Check if we should not insert a large packet */
        if (IgnoreLargePkt(st, p, tdb))
        {
            return;
        }

        if ( p->tcph->th_flags & TH_SYN )
            seq++;

        /* new packet seq is below the last ack... */
        if ( SEQ_GT(st->r_win_base, seq) )
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "segment overlaps ack'd data...\n"););
            overlap = st->r_win_base - tdb->seq;
            if (overlap >= p->dsize)
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "full overlap on ack'd data, dropping segment\n"););
                PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                return;
            }
        }

        AddStreamNode(st, p, tdb, tcpssn, p->dsize, overlap, 0, tdb->seq+overlap, NULL, &ss);

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attached new queue to seglist, %d bytes queued, "
                    "base_seq 0x%X\n",
                    ss->size, st->seglist_base_seq););
    }

    PREPROC_PROFILE_END(s5TcpInsertPerfStats);
    return;
}

#if 0
static inline StreamSegment *FindSegment(StreamTracker *st, uint32_t pkt_seq)
{
    int32_t dist_head;
    int32_t dist_tail;
    StreamSegment *ss;

    if (!st->seglist)
        return NULL;

    dist_head = pkt_seq - st->seglist->seq;
    dist_tail = pkt_seq - st->seglist_tail->seq;

    if (dist_head <= dist_tail)
    {
        /* Start iterating at the head (left) */
        for (ss = st->seglist; ss; ss = ss->next)
        {
            if (SEQ_EQ(ss->seq, pkt_seq))
                return ss;

            if (SEQ_GEQ(ss->seq, pkt_seq))
                break;
        }
    }
    else
    {
        /* Start iterating at the tail (right) */
        for (ss = st->seglist_tail; ss; ss = ss->prev)
        {
            if (SEQ_EQ(ss->seq, pkt_seq))
                return ss;

            if (SEQ_LT(ss->seq, pkt_seq))
                break;
        }
    }
    return NULL;
}
#endif

void StreamTcpSessionClear(Packet *p)
{
    SessionControlBlock *scb;
    TcpSession *ssn;

    if ((!p) || (!p->ssnptr))
        return;

    scb = (SessionControlBlock *)p->ssnptr;

    if (!scb->proto_specific_data)
        return;

    ssn = (TcpSession *)scb->proto_specific_data->data;

    if (!ssn)
        return;

    TcpSessionClear(scb, ssn, 1);
}

static inline int SegmentFastTrack(StreamSegment *tail, TcpDataBlock *tdb)
{
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking seq for fast track: %X > %X\n", tdb->seq,
                tail->seq + tail->size););

    if(SEQ_EQ(tdb->seq, tail->seq + tail->size))
        return 1;

    return 0;
}

static inline StreamSegment* SegmentAlloc (
        Packet* p, const struct timeval* tv, uint32_t caplen, uint32_t pktlen, const uint8_t* pkt)
{
    StreamSegment* ss;
    unsigned size = sizeof(*ss);

    if ( caplen > 0 )
        size += caplen - 1;  // ss contains 1st byte

    session_mem_in_use += size;

    if (session_mem_in_use > stream_session_config->memcap)
    {
        pc.str_mem_faults++;
        sfBase.iStreamFaults++;

        if ( !p )
        {
            session_mem_in_use -= size;
            return NULL;
        }

        /* Smack the older time'd out sessions */
        if ( !session_api->prune_session_cache( tcp_lws_cache, p->pkth->ts.tv_sec,
                    ( SessionControlBlock * ) p->ssnptr, 0 ) )
        {
            /* Try the memcap - last parameter (1) specifies check
             * based on memory cap. */
            session_api->prune_session_cache(tcp_lws_cache, 0, (SessionControlBlock*) p->ssnptr, 1);
        }
    }
    if (session_mem_in_use > stream_session_config->memcap)
    {
        session_mem_in_use -= size;
        return NULL;
    }

    ss = SnortPreprocAlloc(1, size, PP_STREAM, PP_MEM_CATEGORY_SESSION);

    ss->tv.tv_sec = tv->tv_sec;
    ss->tv.tv_usec = tv->tv_usec;
    ss->caplen = caplen;
    ss->pktlen = pktlen;

    memcpy(ss->pkt, pkt, caplen);

    return ss;
}

static int AddStreamNode(StreamTracker *st, Packet *p,
        TcpDataBlock* tdb,
        TcpSession *tcpssn,
        uint16_t len,
        uint32_t slide,
        uint32_t trunc,
        uint32_t seq,
        StreamSegment *left,
        StreamSegment **retSeg)
{
    StreamSegment *ss = NULL;
    uint16_t reassembly_policy;
    int32_t newSize = len - slide - trunc;
    reassembly_policy = st->reassembly_policy;

    if(st->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT)
    {
        //If FIN is already queued, when adding a segment, take target-based approach
        switch(reassembly_policy)
        {
            case REASSEMBLY_POLICY_LAST:
                st->s_mgr.state_queue = TCP_STATE_NONE;
                break;
            case REASSEMBLY_POLICY_FIRST:
            default:
                if(SEQ_GT(seq + newSize, st->s_mgr.transition_seq - 1))
                {
                    uint32_t delta = seq + newSize - (st->s_mgr.transition_seq - 1);
                    newSize = newSize - delta;
                }
                break;
        }
    }

    if (newSize <= 0)
    {
        /*
         * zero size data because of trimming.  Don't
         * insert it
         */
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "zero size TCP data after left & right trimming "
                    "(len: %d slide: %d trunc: %d)\n",
                    len, slide, trunc););
        Discard();
        NormalTrimPayloadIfWin(p, 0, tdb);

#ifdef STREAM_DEBUG_ENABLED
        {
            StreamSegment *idx = st->seglist;
            unsigned long i = 0;
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Dumping seglist, %d segments\n", st->seg_count););
            while (idx)
            {
                i++;
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "%d  ptr: %p  seq: 0x%X  size: %d nxt: %p prv: %p\n",
                            i, idx, idx->seq, idx->size, idx->next, idx->prev););

                if(st->seg_count < i)
                    FatalError("Circular list, WTF?\n");

                idx = idx->next;
            }
        }
#endif
        return STREAM_INSERT_ANOMALY;
    }

    ss = SegmentAlloc(p, &p->pkth->ts, p->pkth->caplen, p->pkth->pktlen, p->pkt);
    if (!ss)
      return STREAM_INSERT_ANOMALY;

    ss->data = ss->pkt + (p->data - p->pkt);
    ss->orig_dsize = p->dsize;

    ss->payload = ss->data + slide;
    ss->size = (uint16_t)newSize;
    ss->seq = seq;
    ss->ts = tdb->ts;

    /* handle the urg ptr */
    if(p->tcph->th_flags & TH_URG)
    {
        if(ntohs(p->tcph->th_urp) < p->dsize)
        {
            switch(st->os_policy)
            {
                case STREAM_POLICY_LINUX:
                case STREAM_POLICY_OLD_LINUX:
                    /* Linux, Old linux discard data from urgent pointer */
                    /* If urg pointer is 0, it's treated as a 1 */
                    ss->urg_offset = ntohs(p->tcph->th_urp);
                    if (ss->urg_offset == 0)
                    {
                        ss->urg_offset = 1;
                    }
                    break;
                case STREAM_POLICY_FIRST:
                case STREAM_POLICY_NOACK:
                case STREAM_POLICY_LAST:
                case STREAM_POLICY_BSD:
                case STREAM_POLICY_MACOS:
                case STREAM_POLICY_SOLARIS:
                case STREAM_POLICY_WINDOWS:
                case STREAM_POLICY_WINDOWS2K3:
                case STREAM_POLICY_VISTA:
                case STREAM_POLICY_HPUX11:
                case STREAM_POLICY_HPUX10:
                case STREAM_POLICY_IRIX:
                    /* Others discard data from urgent pointer */
                    /* If urg pointer is beyond this packet, it's treated as a 0 */
                    ss->urg_offset = ntohs(p->tcph->th_urp);
                    if (ss->urg_offset > p->dsize)
                    {
                        ss->urg_offset = 0;
                    }
                    break;
            }
        }
    }

    StreamSeglistAddNode(st, left, ss);
    st->seg_bytes_logical += ss->size;
    st->seg_bytes_total += ss->caplen;  /* Includes protocol headers and payload */
    st->total_segs_queued++;
    st->total_bytes_queued += ss->size;

    p->packet_flags |= PKT_STREAM_INSERT;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "added %d bytes on segment list @ seq: 0x%X, total %lu, "
                "%d segments queued\n", ss->size, ss->seq,
                st->seg_bytes_logical, SegsToFlush(st, 0)););

    *retSeg = ss;
#ifdef SEG_TEST
    CheckSegments(st);
#endif
    return STREAM_INSERT_OK;
}

static int DupStreamNode(Packet *p,
        StreamTracker *st,
        StreamSegment *left,
        StreamSegment **retSeg)
{
    StreamSegment* ss = SegmentAlloc(p, &left->tv, left->caplen, left->pktlen, left->pkt);

    if ( !ss )
        return STREAM_INSERT_FAILED;

    ss->data = ss->pkt + (left->data - left->pkt);
    ss->orig_dsize = left->orig_dsize;

    /* twiddle the values for overlaps */
    ss->payload = ss->data;
    ss->size = left->size;
    ss->seq = left->seq;

    StreamSeglistAddNode(st, left, ss);
    st->seg_bytes_total += ss->caplen;
    st->total_segs_queued++;
    //st->total_bytes_queued += ss->size;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "added %d bytes on segment list @ seq: 0x%X, total %lu, "
                "%d segments queued\n", ss->size, ss->seq,
                st->seg_bytes_logical, SegsToFlush(st, 0)););

    *retSeg = ss;
    return STREAM_INSERT_OK;

}

static inline bool IsRetransmit(StreamSegment *seg, const uint8_t *rdata,
        uint16_t rsize, uint32_t rseq, bool check_frt, bool *full_retransmit)
{
    // If seg->orig_size == seg->size, then it's sequence number wasn't adjusted
    // so can just do a straight compare of the sequence numbers.
    // Don't want to count as a retransmit if segment's size/sequence number
    // has been adjusted.
    *full_retransmit = false;
    if (SEQ_EQ(seg->seq, rseq) && (seg->orig_dsize == seg->size))
    {
        if (((seg->size <= rsize) && (memcmp(seg->data, rdata, seg->size) == 0))
                || ((seg->size > rsize) && (memcmp(seg->data, rdata, rsize) == 0)))
            return true;
    }
    //Checking for a possible split of segment in which case
    //we compare complete data of the segment to find a retransmission
    else if(check_frt && SEQ_EQ(seg->seq, rseq) && seg->orig_dsize == rsize )
    {
        if(memcmp(seg->data, rdata, rsize) == 0)
        {
            *full_retransmit = true;
            return true;
        }
    }
    return false;
}

static inline void RetransmitProcess(Packet *p, TcpSession *tcpssn)
{
    // Data has already been analyzed so don't bother looking at it again.
    DisableDetect( p );

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Allowing retransmitted data "
                "-- not blocked previously\n"););
}

static inline void RetransmitHandle(Packet *p, TcpSession *tcpssn)
{

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Calling SE_REXMIT Handler\n"););

    if ( tcpssn->scb->handler[SE_REXMIT] )
        StreamCallHandler(p, tcpssn->scb->handler[SE_REXMIT]);
}

static inline void EndOfFileHandle(Packet *p, TcpSession *tcpssn)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Calling SE_EOF Handler\n"););

    if ( tcpssn->scb->handler[SE_EOF] )
        StreamCallHandler(p, tcpssn->scb->handler[SE_EOF]);
}

static inline bool IsRetransmitOfHeldSegment(Packet *p, TcpDataBlock *tdb,
      StreamTracker *listener)
{
    StreamSegment *held_seg = listener->held_segment;

    if (held_seg == NULL)
        return FALSE;

    if (held_seg->seq == tdb->seq)
        return TRUE;

    return FALSE;
}

static bool IsTCPFastOpenPkt (Packet *p)
{
    uint8_t opt_count = 0;
 
    while (opt_count < p->tcp_option_count)
    {
        if (p->tcp_options[opt_count].code == TCPOPT_TFO)
        {
            return TRUE;
        }
        opt_count++;
    }
    return FALSE; 
}

static int StreamQueue(StreamTracker *st, Packet *p, TcpDataBlock *tdb,
        TcpSession *tcpssn)
{
    StreamSegment *ss = NULL;
    StreamSegment *left = NULL;
    StreamSegment *right = NULL;
    StreamSegment *dump_me = NULL;
    uint32_t seq = tdb->seq;
    uint32_t seq_end = tdb->end_seq;
    uint16_t len = p->dsize;
    int trunc = 0;
    int overlap = 0;
    int slide = 0;
    int ret = STREAM_INSERT_OK;
    char done = 0;
    char addthis = 1;
    int32_t dist_head;
    int32_t dist_tail;
    uint16_t reassembly_policy;
#ifdef NORMALIZER
    NormMode ips_data;
#endif
    // To check for retransmitted data
    const uint8_t *rdata = p->data;
    uint16_t rsize = p->dsize;
    uint32_t rseq = tdb->seq;
    PROFILE_VARS;
    STREAM_DEBUG_WRAP(
            StreamSegment *lastptr = NULL;
            uint32_t base_seq = st->seglist_base_seq;
            int last = 0;
            );

#ifdef NORMALIZER
    ips_data = Stream_NormGetMode(st->reassembly_policy, snort_conf, NORM_TCP_IPS);
    if ( ips_data == NORM_MODE_ON )
        reassembly_policy = REASSEMBLY_POLICY_FIRST;
    else
#endif
        reassembly_policy = st->reassembly_policy;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Queuing %d bytes on stream!\n"
                "base_seq: %X seq: %X  seq_end: %X\n",
                seq_end - seq, base_seq, seq, seq_end););

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "%d segments on seglist\n", SegsToFlush(st, 0)););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););

    PREPROC_PROFILE_START(s5TcpInsertPerfStats);

    /* Check if we should not insert a large packet */
    if (IgnoreLargePkt(st, p, tdb))
    {
        // NORM should ignore large pkt be disabled for stream normalization?
        // if not, how to normalize ignored large packets?
        return ret;
    }

    // NORM fast tracks are in sequence - no norms
    if(st->seglist_tail && SegmentFastTrack(st->seglist_tail, tdb))
    {
        /* segment fit cleanly at the end of the segment list */
        left = st->seglist_tail;
        right = NULL;

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Fast tracking segment! (tail_seq %X size %d)\n",
                    st->seglist_tail->seq, st->seglist_tail->size););

        ret = AddStreamNode(st, p, tdb, tcpssn, len,
                slide /* 0 */, trunc /* 0 */, seq, left /* tail */,
                &ss);

        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
        return ret;
    }

    if (st->seglist && st->seglist_tail)
    {
        if (SEQ_GT(tdb->seq, st->seglist->seq))
        {
            dist_head = tdb->seq - st->seglist->seq;
        }
        else
        {
            dist_head = st->seglist->seq - tdb->seq;
        }

        if (SEQ_GT(tdb->seq, st->seglist_tail->seq))
        {
            dist_tail = tdb->seq - st->seglist_tail->seq;
        }
        else
        {
            dist_tail = st->seglist_tail->seq - tdb->seq;
        }
    }
    else
    {
        dist_head = dist_tail = 0;
    }

    if (SEQ_LEQ(dist_head, dist_tail))
    {
        /* Start iterating at the head (left) */
        for(ss = st->seglist; ss; ss = ss->next)
        {
            STREAM_DEBUG_WRAP(
                    DebugMessage(DEBUG_STREAM_STATE,
                        "ss: %p  seq: 0x%X  size: %lu delta: %d\n",
                        ss, ss->seq, ss->size, (ss->seq-base_seq) - last);
                    last = ss->seq-base_seq;
                    lastptr = ss;

                    DebugMessage(DEBUG_STREAM_STATE,
                        "   lastptr: %p ss->next: %p ss->prev: %p\n",
                        lastptr, ss->next, ss->prev);
                    );

            right = ss;

            if(SEQ_GEQ(right->seq, seq))
                break;

            left = right;
        }

        if(ss == NULL)
            right = NULL;
    }
    else
    {
        /* Start iterating at the tail (right) */
        for(ss = st->seglist_tail; ss; ss = ss->prev)
        {
            STREAM_DEBUG_WRAP(
                    DebugMessage(DEBUG_STREAM_STATE,
                        "ss: %p  seq: 0x%X  size: %lu delta: %d\n",
                        ss, ss->seq, ss->size, (ss->seq-base_seq) - last);
                    last = ss->seq-base_seq;
                    lastptr = ss;

                    DebugMessage(DEBUG_STREAM_STATE,
                        "   lastptr: %p ss->next: %p ss->prev: %p\n",
                        lastptr, ss->next, ss->prev);
                    );

            left = ss;

            if(SEQ_LT(left->seq, seq))
                break;

            right = left;
        }

        if(ss == NULL)
            left = NULL;
    }

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "left: %p:0x%X  right: %p:0x%X\n", left,
                left?left->seq:0, right, right?right->seq:0););

    /*
     * handle left overlaps
     */
    if(left)
    {
        // NOTE that left->seq is always less than seq, otherwise it would
        // be a right based on the above determination of left and right

        /* check if the new segment overlaps on the left side */
        overlap = left->seq + left->size - seq;

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "left overlap %d\n", overlap););

        if(overlap > 0)
        {
            // NOTE that overlap will always be less than left->size since
            // seq is always greater than left->seq
            s5stats.tcp_overlaps++;
            st->overlap_count++;

            switch(reassembly_policy)
            {
                case REASSEMBLY_POLICY_FIRST:
                case REASSEMBLY_POLICY_NOACK:
                case REASSEMBLY_POLICY_LINUX:
                case REASSEMBLY_POLICY_BSD:
                case REASSEMBLY_POLICY_WINDOWS:
                case REASSEMBLY_POLICY_WINDOWS2K3:
                case REASSEMBLY_POLICY_VISTA:
                case REASSEMBLY_POLICY_HPUX10:
                case REASSEMBLY_POLICY_IRIX:
                case REASSEMBLY_POLICY_OLD_LINUX:
                case REASSEMBLY_POLICY_MACOS:
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "left overlap, honoring old data\n"););
#ifdef NORMALIZER
                    if ( ips_data != NORM_MODE_OFF )
                    {
                        if (SEQ_LT(left->seq,tdb->seq) && SEQ_GT(left->seq + left->size, tdb->seq + p->dsize))
                        {
                            if ( ips_data == NORM_MODE_ON )
                            {
                                unsigned offset = tdb->seq - left->seq;
                                memcpy((uint8_t*)p->data, left->payload+offset, p->dsize);
                                p->packet_flags |= PKT_MODIFIED;
                            }
                            normStats[PC_TCP_IPS_DATA][ips_data]++;
                            sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA][ips_data]++;
                        }
                        else if (SEQ_LT(left->seq, tdb->seq))
                        {
                            if ( ips_data == NORM_MODE_ON )
                            {
                                unsigned offset = tdb->seq - left->seq;
                                unsigned length = left->seq + left->size - tdb->seq;
                                memcpy((uint8_t*)p->data, left->payload+offset, length);
                                p->packet_flags |= PKT_MODIFIED;
                            }
                            normStats[PC_TCP_IPS_DATA][ips_data]++;
                            sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA][ips_data]++;
                        }
                    }
#endif
                    seq += overlap;
                    slide = overlap;
                    if(SEQ_LEQ(seq_end, seq))
                    {
                        /*
                         * houston, we have a problem
                         */
                        /* flag an anomaly */
                        EventBadSegment(st->tcp_policy);
                        Discard();
                        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                        return STREAM_INSERT_ANOMALY;
                    }
                    break;

                case REASSEMBLY_POLICY_SOLARIS:
                case REASSEMBLY_POLICY_HPUX11:
                    if (SEQ_LT(left->seq, seq) && SEQ_GEQ(left->seq + left->size, seq + len))
                    {
                        /* New packet is entirely overlapped by an
                         * existing packet on both sides.  Drop the
                         * new data. */
                        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "left overlap, honoring old data\n"););
                        seq += overlap;
                        slide = overlap;
                        if(SEQ_LEQ(seq_end, seq))
                        {
                            /*
                             * houston, we have a problem
                             */
                            /* flag an anomaly */
                            EventBadSegment(st->tcp_policy);
                            Discard();
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return STREAM_INSERT_ANOMALY;
                        }
                    }

                    /* Otherwise, trim the old data accordingly */
                    left->size -= (int16_t)overlap;
                    st->seg_bytes_logical -= overlap;
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "left overlap, honoring new data\n"););
                    break;
                case REASSEMBLY_POLICY_LAST:
                    /* True "Last" policy" */
                    if (SEQ_LT(left->seq, seq) && SEQ_GT(left->seq + left->size, seq + len))
                    {
                        /* New data is overlapped on both sides by
                         * existing data.  Existing data needs to be
                         * split and the new data inserted in the
                         * middle.
                         *
                         * Need to duplicate left.  Adjust that
                         * seq by + (seq + len) and
                         * size by - (seq + len - left->seq).
                         */
                        ret = DupStreamNode(p, st, left, &right);
                        if (ret != STREAM_INSERT_OK)
                        {
                            /* No warning,
                             * its done in StreamSeglistAddNode */
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return ret;
                        }
                        left->size -= (int16_t)overlap;
                        st->seg_bytes_logical -= overlap;

                        right->seq = seq + len;
                        right->size -= (int16_t)(seq + len - left->seq);
                        right->payload += (seq + len - left->seq);
                        st->seg_bytes_logical -= (seq + len - left->seq);
                    }
                    else
                    {
                        left->size -= (int16_t)overlap;
                        st->seg_bytes_logical -= overlap;
                    }

                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "left overlap, honoring new data\n"););
                    break;
            }

            if(SEQ_LEQ(seq_end, seq))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "seq_end < seq"););
                /*
                 * houston, we have a problem
                 */
                /* flag an anomaly */
                EventBadSegment(st->tcp_policy);
                Discard();
                PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                return STREAM_INSERT_ANOMALY;
            }
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "No left overlap\n"););
        }
    }

    //(seq_end > right->seq) && (seq_end <= (right->seq+right->size))))
    while(right && !done && SEQ_LT(right->seq, seq_end))
    {
        bool full_retransmit = false;
        trunc = 0;
        overlap = (int)(seq_end - right->seq);
        //overlap = right->size - (right->seq - seq);
        //right->seq + right->size - seq_end;

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "right overlap(%d): len: %d right->seq: 0x%X seq: 0x%X\n",
                    overlap, len, right->seq, seq););
        /* Treat sequence number overlap as a retransmission
         * Only check right side since left side happens rarely
         */
        if((st->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS_FTP) && (overlap == right->size) && right->next && (right->next->size > 0))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "ftp-data daq retry or rexmit, do not call RetransmitHandle as \
                        data beyond this packet is already received\n"););
        }
        else
        {
            RetransmitHandle(p, tcpssn);
        }

        if(overlap < right->size)
        {
            if (IsRetransmit(right, rdata, rsize, rseq, false, &full_retransmit))
            {
#ifdef DAQ_PKT_FLAG_RETRY_PACKET
		 /* If packet is identified as re-transmitted
		  * and it is set as retry already then block the packet  */
		if (!(p->pkth->flags & DAQ_PKT_FLAG_RETRY_PACKET))
			p->packet_flags |= PKT_RETRANSMIT;
#endif
                // All data was retransmitted
                if (IsRetransmitOfHeldSegment(p, tdb, st))
                {
                    st->held_segment = NULL;
                    p->packet_flags |= PKT_PSEUDO_FLUSH;
                }
                else
                {
                    RetransmitProcess(p, tcpssn);
                }
                addthis = 0;
                break;
            }
            s5stats.tcp_overlaps++;
            st->overlap_count++;

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Got partial right overlap\n"););

            switch(reassembly_policy)
            {
                /* truncate existing data */
                case REASSEMBLY_POLICY_LAST:
                case REASSEMBLY_POLICY_LINUX:
                case REASSEMBLY_POLICY_OLD_LINUX:
                case REASSEMBLY_POLICY_BSD:
                case REASSEMBLY_POLICY_WINDOWS:
                case REASSEMBLY_POLICY_WINDOWS2K3:
                case REASSEMBLY_POLICY_IRIX:
                case REASSEMBLY_POLICY_HPUX10:
                case REASSEMBLY_POLICY_MACOS:
                    if (SEQ_EQ(right->seq, seq) &&
                            (reassembly_policy != REASSEMBLY_POLICY_LAST))
                    {
                        slide = (right->seq + right->size - seq);
                        seq += slide;
                    }
                    else
                    {
                        /* partial overlap */
                        right->seq += overlap;
                        right->payload += overlap;
                        right->size -= (int16_t)overlap;
                        st->seg_bytes_logical -= overlap;
                        st->total_bytes_queued -= overlap;
                    }

                    // right->size always > 0 since overlap < right->size

                    break;

                case REASSEMBLY_POLICY_FIRST:
                case REASSEMBLY_POLICY_VISTA:
                case REASSEMBLY_POLICY_SOLARIS:
                case REASSEMBLY_POLICY_HPUX11:
#ifdef NORMALIZER
                    if ( ips_data == NORM_MODE_ON )
                    {
                        unsigned offset = right->seq - tdb->seq;
                        unsigned length = tdb->seq + p->dsize - right->seq;
                        memcpy((uint8_t*)p->data+offset, right->payload, length);
                        p->packet_flags |= PKT_MODIFIED;
                    }
                    if ( ips_data != NORM_MODE_OFF )
                    {
                        normStats[PC_TCP_IPS_DATA][ips_data]++;
                        sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA][ips_data]++;
                    }
#endif
                    trunc = overlap;
                    break;

                case REASSEMBLY_POLICY_NOACK:
                    // Don't normalize
                    trunc = overlap;
                    break;
            }

            /* all done, keep me out of the loop */
            done = 1;
        }
        else  // Full overlap
        {
            // Don't want to count retransmits as overlaps or do anything
            // else with them.  Account for retransmits of multiple PDUs
            // in one segment.
            if (IsRetransmit(right, rdata, rsize, rseq, (rseq == tdb->seq), &full_retransmit))
            {
                if( !full_retransmit )
                {
                    rdata += right->size;
                    rsize -= right->size;
                    rseq += right->size;

                    seq += right->size;
                    left = right;
                    right = right->next;
                }
                else
                {
                    rsize = 0;
                    done = 1;
                }

                if (rsize == 0)
                {
#ifdef DAQ_PKT_FLAG_RETRY_PACKET
                /* If packet is identified as re-transmitted
		 * and it is set as retry already then block the packet  */
		    if (!(p->pkth->flags & DAQ_PKT_FLAG_RETRY_PACKET))
			    p->packet_flags |= PKT_RETRANSMIT;
#endif
                    // All data was retransmitted
                    if (IsRetransmitOfHeldSegment(p, tdb, st))
                    {
                        st->held_segment = NULL;
                        p->packet_flags |= PKT_PSEUDO_FLUSH;
                    }
                    else
                    {
                        RetransmitProcess(p, tcpssn);
                    }

                    addthis = 0;
                }
                continue;
            }

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Got full right overlap\n"););

            s5stats.tcp_overlaps++;
            st->overlap_count++;

            switch(reassembly_policy)
            {
                case REASSEMBLY_POLICY_BSD:
                case REASSEMBLY_POLICY_LINUX:
                case REASSEMBLY_POLICY_WINDOWS:
                case REASSEMBLY_POLICY_WINDOWS2K3:
                case REASSEMBLY_POLICY_HPUX10:
                case REASSEMBLY_POLICY_IRIX:
                case REASSEMBLY_POLICY_MACOS:
                    if (SEQ_GEQ(seq_end, right->seq + right->size) &&
                            SEQ_LT(seq, right->seq))
                    {
                        dump_me = right;

                        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "retrans, dropping old data at seq %d, size %d\n",
                                    right->seq, right->size););
                        right = right->next;
                        StreamSeglistDeleteNode(st, dump_me);
                        if (left == dump_me)
                            left = NULL;
                        break;
                    }
                    else
                    {
                        switch (reassembly_policy)
                        {
                            case REASSEMBLY_POLICY_WINDOWS:
                            case REASSEMBLY_POLICY_WINDOWS2K3:
                            case REASSEMBLY_POLICY_BSD:
                            case REASSEMBLY_POLICY_MACOS:
                                /* BSD/MacOS & Windows follow a FIRST policy in the
                                 * case below... */
                                break;
                            default:
                                /* All others follow a LAST policy */
                                if (SEQ_GT(seq_end, right->seq + right->size) &&
                                        SEQ_EQ(seq, right->seq))
                                {
                                    /* When existing data is fully overlapped by new
                                     * and sequence numbers are the same, most OSs
                                     * follow a LAST policy.
                                     */
                                    goto right_overlap_last;
                                }
                                break;
                        }
                    }
                    /* Fall through */
                case REASSEMBLY_POLICY_FIRST:
                case REASSEMBLY_POLICY_VISTA:
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Got full right overlap, truncating new\n"););
#ifdef NORMALIZER
                    if ( ips_data == NORM_MODE_ON )
                    {
                        unsigned offset = right->seq - tdb->seq;
                        memcpy((uint8_t*)p->data+offset, right->payload, right->size);
                        p->packet_flags |= PKT_MODIFIED;
                    }
                    if ( ips_data != NORM_MODE_OFF )
                    {
                        normStats[PC_TCP_IPS_DATA][ips_data]++;
                        sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA][ips_data]++;
                    }
#endif
                    if (SEQ_EQ(right->seq, seq))
                    {
                        /* Overlap is greater than or equal to right->size
                         * slide gets set before insertion */
                        seq += right->size;
                        left = right;
                        right = right->next;

                        /* Adjusted seq is fully overlapped */
                        if (SEQ_EQ(seq, seq_end))
                        {
                            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                        "StreamQueue got full right overlap with "
                                        "resulting seq too high, bad segment "
                                        "(seq: %X  seq_end: %X overlap: %lu\n",
                                        seq, seq_end, overlap););
                            EventBadSegment(st->tcp_policy);
                            Discard();
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return STREAM_INSERT_ANOMALY;
                        }

                        /* No data to add on the left of right, so continue
                         * since some of the other non-first targets may have
                         * fallen into this case */
                        continue;
                    }

                    /* seq is less than right->seq */

                    /* trunc is reset to 0 at beginning of loop */
                    trunc = overlap;

                    /* insert this one, and see if we need to chunk it up */
                    /* Adjust slide so that is correct relative to orig seq */
                    slide = seq - tdb->seq;
                    ret = AddStreamNode(st, p, tdb, tcpssn, len, slide, trunc, seq, left, &ss);
                    if (ret != STREAM_INSERT_OK)
                    {
                        /* no warning, already done above */
                        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                        return ret;
                    }

                    /* Set seq to end of right since overlap was greater than
                     * or equal to right->size and inserted seq has been
                     * truncated to beginning of right
                     * And reset trunc to 0 since we may fall out of loop if
                     * next right is NULL */
                    seq = right->seq + right->size;
                    left = right;
                    right = right->next;
                    trunc = 0;

                    /* Keep looping since in IPS we may need to copy old
                     * data into packet */

                    break;

                case REASSEMBLY_POLICY_NOACK:
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Got full right overlap, truncating new\n"););
                    if (SEQ_EQ(right->seq, seq))
                    {
                        /* Overlap is greater than or equal to right->size
                         * slide gets set before insertion */
                        seq += right->size;
                        left = right;
                        right = right->next;

                        /* Adjusted seq is fully overlapped */
                        if (SEQ_EQ(seq, seq_end))
                        {
                            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                        "StreamQueue got full right overlap with "
                                        "resulting seq too high, bad segment "
                                        "(seq: %X  seq_end: %X overlap: %lu\n",
                                        seq, seq_end, overlap););
                            EventBadSegment(st->tcp_policy);
                            s5stats.tcp_discards++;
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return STREAM_INSERT_ANOMALY;
                        }

                        /* No data to add on the left of right, so continue
                         * since some of the other non-first targets may have
                         * fallen into this case */
                        continue;
                    }

                    /* seq is less than right->seq */

                    /* trunc is reset to 0 at beginning of loop */
                    trunc = overlap;

                    /* insert this one, and see if we need to chunk it up */
                    /* Adjust slide so that is correct relative to orig seq */
                    slide = seq - tdb->seq;
                    ret = AddStreamNode(st, p, tdb, tcpssn, len, slide, trunc, seq, left, &ss);
                    if (ret != STREAM_INSERT_OK)
                    {
                        /* no warning, already done above */
                        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                        return ret;
                    }

                    /* Set seq to end of right since overlap was greater than
                     * or equal to right->size and inserted seq has been
                     * truncated to beginning of right
                     * And reset trunc to 0 since we may fall out of loop if
                     * next right is NULL */
                    seq = right->seq + right->size;
                    left = right;
                    right = right->next;
                    trunc = 0;

                    /* Keep looping since in IPS we may need to copy old
                     * data into packet */

                    break;

                case REASSEMBLY_POLICY_HPUX11:
                case REASSEMBLY_POLICY_SOLARIS:
                    /* If this packet is wholly overlapping and the same size
                     * as a previous one and we have not received the one
                     * immediately preceeding, we take the FIRST. */
                    if (SEQ_EQ(right->seq, seq) && (right->size == len) &&
                            (left && !SEQ_EQ(left->seq + left->size, seq)))
                    {
                        trunc += overlap;
                        if(SEQ_LEQ((int)(seq_end - trunc), seq))
                        {
                            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                        "StreamQueue got full right overlap with "
                                        "resulting seq too high, bad segment "
                                        "(seq: %X  seq_end: %X overlap: %lu\n",
                                        seq, seq_end, overlap););
                            EventBadSegment(st->tcp_policy);
                            Discard();
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return STREAM_INSERT_ANOMALY;
                        }
                        break;
                    }
                    /* Fall through */
                case REASSEMBLY_POLICY_OLD_LINUX:
                case REASSEMBLY_POLICY_LAST:
right_overlap_last:
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Got full right overlap of old, dropping old\n"););
                    dump_me = right;
                    right = right->next;
                    StreamSeglistDeleteNode(st, dump_me);
                    if (left == dump_me)
                        left = NULL;
                    break;
            }
        }
    }

    if (addthis)
    {
        /* Adjust slide so that is correct relative to orig seq */
        slide = seq - tdb->seq;
        ret = AddStreamNode(st, p, tdb, tcpssn, len,
                slide, trunc, seq, left, &ss);
    }
    else
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Fully truncated right overlap\n"););
    }

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "StreamQueue returning normally\n"););

    PREPROC_PROFILE_END(s5TcpInsertPerfStats);
    return ret;
}


static void ProcessTcpStream(StreamTracker *rcv, TcpSession *tcpssn,
        Packet *p, TcpDataBlock *tdb,
        StreamTcpPolicy *s5TcpPolicy)
{

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In ProcessTcpStream(), %d bytes to queue\n", p->dsize););

    if ( p->packet_flags & PKT_IGNORE )
        return;

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    SetPacketHeaderFoo(tcpssn, p);
#endif

    if ((s5TcpPolicy->flags & STREAM_CONFIG_NO_ASYNC_REASSEMBLY) &&
            !TwoWayTraffic(tcpssn->scb))
    {
        return;
    }

    if ((s5TcpPolicy->max_consec_small_segs) &&
        /* check ignore_ports */
        !(s5TcpPolicy->small_seg_ignore[p->dp/8] & (1 << (p->dp %8))))
    {
        if (p->dsize >= s5TcpPolicy->max_consec_small_seg_size)
        {
            rcv->small_seg_count = 0;
        }
        else 
        {
            if (++rcv->small_seg_count == s5TcpPolicy->max_consec_small_segs)
            {
                /* Above threshold, log it... requires detect_anomalies be
                 * on in this TCP policy, action controlled by preprocessor
                 * rule. */
                EventMaxSmallSegsExceeded(s5TcpPolicy);
            }
        }
    }

    if (s5TcpPolicy->max_queued_bytes &&
            (rcv->seg_bytes_total > s5TcpPolicy->max_queued_bytes))
    {
        if (stream_session_config->prune_log_max && (TwoWayTraffic(tcpssn->scb) || s5TcpPolicy->log_asymmetric_traffic) && !(tcpssn->scb->ha_state.session_flags & SSNFLAG_LOGGED_QUEUE_FULL))
        {
            char bufc[INET6_ADDRSTRLEN], bufs[INET6_ADDRSTRLEN];
            sfip_ntop(&tcpssn->scb->client_ip, bufc, sizeof(bufc));
            sfip_ntop(&tcpssn->scb->server_ip, bufs, sizeof(bufs));

            LogMessage("S5: Session exceeded configured max bytes to queue %d "
                    "using %d bytes (%s). %s %d --> %s %d "
#ifdef TARGET_BASED
                    "(%d) "
#endif
                    ": LWstate 0x%x LWFlags 0x%x\n",
                    s5TcpPolicy->max_queued_bytes, rcv->seg_bytes_total,
                    (rcv == &tcpssn->client) ? "client queue" : "server queue",
                    bufc,
                    ntohs(tcpssn->scb->client_port),
                    bufs,
                    ntohs(tcpssn->scb->server_port),
#ifdef TARGET_BASED
                    tcpssn->scb->ha_state.application_protocol,
#endif
                    tcpssn->scb->session_state,
                    tcpssn->scb->ha_state.session_flags);

            /* only log this one per session */
            tcpssn->scb->ha_state.session_flags |= SSNFLAG_LOGGED_QUEUE_FULL;
        }
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Ignoring segment due to too many bytes queued\n"););
        return;
    }

    if (s5TcpPolicy->max_queued_segs &&
            (rcv->seg_count+1 > s5TcpPolicy->max_queued_segs))
    {
        if (stream_session_config->prune_log_max && (TwoWayTraffic(tcpssn->scb) || s5TcpPolicy->log_asymmetric_traffic) && !(tcpssn->scb->ha_state.session_flags & SSNFLAG_LOGGED_QUEUE_FULL))
        {
            char bufc[INET6_ADDRSTRLEN], bufs[INET6_ADDRSTRLEN];
            sfip_ntop(&tcpssn->scb->client_ip, bufc, sizeof(bufc));
            sfip_ntop(&tcpssn->scb->server_ip, bufs, sizeof(bufs));

            LogMessage("S5: Session exceeded configured max segs to queue %d "
                    "using %d segs (%s). %s %d --> %s %d "
#ifdef TARGET_BASED
                    "(%d) "
#endif
                    ": LWstate 0x%x LWFlags 0x%x\n",
                    s5TcpPolicy->max_queued_segs, rcv->seg_count,
                    (rcv == &tcpssn->client) ? "client queue" : "server queue",
                    bufc,
                    ntohs(tcpssn->scb->client_port),
                    bufs,
                    ntohs(tcpssn->scb->server_port),
#ifdef TARGET_BASED
                    tcpssn->scb->ha_state.application_protocol,
#endif
                    tcpssn->scb->session_state, tcpssn->scb->ha_state.session_flags);

            /* only log this one per session */
            tcpssn->scb->ha_state.session_flags |= SSNFLAG_LOGGED_QUEUE_FULL;
        }
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Ignoring segment due to too many bytes queued\n"););
        return;
    }

    if(rcv->seg_count != 0)
    {
        if(rcv->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Ignoring segment due to IGNORE flush_policy\n"););
            return;
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "queuing segment\n"););

            if ( SEQ_GT(rcv->r_win_base, tdb->seq) )
            {
                uint32_t offset = rcv->r_win_base - tdb->seq;
                
                if ( offset < p->dsize )
                {
                    tdb->seq += offset;
                    p->data += offset;
                    p->dsize -= (uint16_t)offset;

                    StreamQueue(rcv, p, tdb, tcpssn);

                    p->dsize += (uint16_t)offset;
                    p->data -= offset;
                    tdb->seq -= offset;
                }
            }
            else
                StreamQueue(rcv, p, tdb, tcpssn);

            if ((rcv->tcp_policy->overlap_limit) &&
                    (rcv->overlap_count > rcv->tcp_policy->overlap_limit))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Reached the overlap limit.  Flush the data "
                            "and kill the session if configured\n"););
                if (p->packet_flags & PKT_FROM_CLIENT)
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Flushing data on packet from the client\n"););
                    flush_stream(tcpssn, rcv, p,
                            GET_SRC_IP(p), GET_DST_IP(p),
                            p->tcph->th_sport, p->tcph->th_dport,
                            PKT_FROM_CLIENT);

                    flush_stream(tcpssn, &tcpssn->server, p,
                            GET_DST_IP(p), GET_SRC_IP(p),
                            p->tcph->th_dport, p->tcph->th_sport,
                            PKT_FROM_SERVER);
                }
                else
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Flushing data on packet from the server\n"););
                    flush_stream(tcpssn, rcv, p,
                            GET_SRC_IP(p), GET_DST_IP(p),
                            p->tcph->th_sport, p->tcph->th_dport,
                            PKT_FROM_SERVER);

                    flush_stream(tcpssn, &tcpssn->client, p,
                            GET_DST_IP(p), GET_SRC_IP(p),
                            p->tcph->th_dport, p->tcph->th_sport,
                            PKT_FROM_CLIENT);
                }
                purge_all(&tcpssn->client);
                purge_all(&tcpssn->server);

                /* Alert on overlap limit and reset counter */
                EventExcessiveOverlap(rcv->tcp_policy);
                rcv->overlap_count = 0;
            }
        }
    }
    else
    {
        if(rcv->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Ignoring segment due to IGNORE flush_policy\n"););
            return;
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "queuing segment\n"););
            NewQueue(rcv, p, tdb, tcpssn);
        }
    }

    return;
}

static int ProcessTcpData(Packet *p, StreamTracker *listener, TcpSession *tcpssn,
        TcpDataBlock *tdb, StreamTcpPolicy *s5TcpPolicy)
{
    PROFILE_VARS;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In ProcessTcpData()\n"););

    PREPROC_PROFILE_START(s5TcpDataPerfStats);
    if (!IsTCPFastOpenPkt(p) && (p->tcph->th_flags & TH_SYN) && (listener->os_policy != STREAM_POLICY_MACOS))
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Bailing, data on SYN, not MAC Policy!\n"););
        NormalTrimPayloadIfSyn(p, 0, tdb);
        PREPROC_PROFILE_END(s5TcpDataPerfStats);
        return STREAM_UNALIGNED;
    }

    /* we're aligned, so that's nice anyway */
    if((tdb->seq == listener->r_nxt_ack) || (p->tcph->th_flags & TH_SYN))
    {
        if ( p->tcph->th_flags & TH_SYN )
            ++tdb->seq;

        /* check if we're in the window */
        if( s5TcpPolicy->policy != STREAM_POLICY_NOACK )
        {
            if(StreamGetWindow(tcpssn->scb, listener, tdb) == 0)
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Bailing, we're out of the window!\n"););
                NormalTrimPayloadIfWin(p, 0, tdb);
                PREPROC_PROFILE_END(s5TcpDataPerfStats);
                return STREAM_UNALIGNED;
            }
        }

        /* move the ack boundry up, this is the only way we'll accept data */
        // update r_nxt_ack for IGNORE Flush policy here. Else update in StreamSeglistAddNode
        if((listener->s_mgr.state_queue == TCP_STATE_NONE) &&
                (listener->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE))
            listener->r_nxt_ack = tdb->end_seq;

        //For IPS mode trim packet if it overwrites OOO FIN in state_queue
        if((listener->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT) &&
                (Stream_NormGetMode(listener->reassembly_policy, snort_conf, NORM_TCP_IPS) == NORM_MODE_ON))
        {
            if(SEQ_GT(tdb->end_seq, listener->s_mgr.transition_seq - 1))
            {
                uint32_t delta = tdb->end_seq - (listener->s_mgr.transition_seq - 1);
                if (p->dsize > delta)
                    NormalTrimPayload(p, delta, tdb);
            }
        }

        if(p->dsize != 0)
        {
            if (listener->flags &  TF_MISSING_PKT)
                tcpssn->scb->ha_state.session_flags |= SSNFLAG_STREAM_ORDER_BAD;

            if (tcpssn->scb->ha_state.session_flags & SSNFLAG_STREAM_ORDER_BAD)
                p->packet_flags |= PKT_STREAM_ORDER_BAD;
            else
                p->packet_flags |= PKT_STREAM_ORDER_OK;

            ProcessTcpStream(listener, tcpssn, p, tdb, s5TcpPolicy);

            if (SEQ_GT(listener->r_nxt_ack,tdb->end_seq))
            {
                tcpssn->scb->ha_state.session_flags |= SSNFLAG_STREAM_ORDER_BAD;
            }

            PREPROC_PROFILE_END(s5TcpDataPerfStats);
            return STREAM_ALIGNED;
        }
    }
    else
    {
        /* pkt is out of order, do some target-based shizzle here */

        /* NO, we don't want to simply bail.  Some platforms
         * favor unack'd dup data over the original data.
         * Let the reassembly policy decide how to handle
         * the overlapping data.
         *
         * See HP, Solaris, et al. for those that favor
         * duplicate data over the original in some cases.
         */
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "out of order segment (tdb->seq: 0x%X "
                    "l->r_nxt_ack: 0x%X!\n", tdb->seq, listener->r_nxt_ack););

        if (listener->s_mgr.state_queue == TCP_STATE_NONE)
        {
            /* check if we're in the window */
            if( s5TcpPolicy->policy != STREAM_POLICY_NOACK )
            {
                if(StreamGetWindow(tcpssn->scb, listener, tdb) == 0)
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Bailing, we're out of the window!\n"););
                    NormalTrimPayloadIfWin(p, 0, tdb);
                    PREPROC_PROFILE_END(s5TcpDataPerfStats);
                    return STREAM_UNALIGNED;
                }
            }

            if ((listener->s_mgr.state == TCP_STATE_ESTABLISHED) &&
                    (listener->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE))
            {
                if ( SEQ_GT(tdb->end_seq, listener->r_nxt_ack))
                {
                    /* set next ack so we are within the window going forward on
                     * this side. */
                    // FIXTHIS for ips, must move all the way to first hole or right end
                    listener->r_nxt_ack = tdb->end_seq;
                }
            }
        }
        //For IPS mode trim packet if it overwrites OOO FIN in state_queue
        else if((listener->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT) &&
                (Stream_NormGetMode(listener->reassembly_policy, snort_conf, NORM_TCP_IPS) == NORM_MODE_ON))
        {
            if(SEQ_GT(tdb->end_seq, listener->s_mgr.transition_seq - 1))
            {
                uint32_t delta = tdb->end_seq - (listener->s_mgr.transition_seq - 1);
                if (p->dsize > delta)
                    NormalTrimPayload(p, delta, tdb);
            }

        }

        if(p->dsize != 0)
        {
            uint32_t r_nxt_ack_old = listener->r_nxt_ack;
            if (SEQ_GT(listener->r_win_base, listener->r_nxt_ack) ||
                    (listener->flags & TF_MISSING_PKT))
                tcpssn->scb->ha_state.session_flags |= SSNFLAG_STREAM_ORDER_BAD;

            if (tcpssn->scb->ha_state.session_flags & SSNFLAG_STREAM_ORDER_BAD)
                p->packet_flags |= PKT_STREAM_ORDER_BAD;

            ProcessTcpStream(listener, tcpssn, p, tdb, s5TcpPolicy);

            /* If we have moved the expected seq, overlapped segment, set BAD flag*/
            if (SEQ_GT(listener->r_nxt_ack, r_nxt_ack_old))
            {
                tcpssn->scb->ha_state.session_flags |= SSNFLAG_STREAM_ORDER_BAD;
                p->packet_flags |= PKT_STREAM_ORDER_BAD;
            }
        }
    }

    PREPROC_PROFILE_END(s5TcpDataPerfStats);
    return STREAM_UNALIGNED;
}

uint16_t StreamGetPolicy(SessionControlBlock *scb, StreamTcpPolicy *s5TcpPolicy, int direction)
{
#ifdef TARGET_BASED
    uint16_t policy_id;
    /* Not caching this host_entry in the frag tracker so we can
     * swap the table out after processing this packet if we need
     * to.  */
    HostAttributeEntry *host_entry = NULL;
    int ssn_dir;

    if (!IsAdaptiveConfigured())
        return s5TcpPolicy->policy;

    if (direction == FROM_CLIENT)
    {
        host_entry = SFAT_LookupHostEntryByIP(&scb->server_ip);
        ssn_dir = SSN_DIR_FROM_SERVER;
    }
    else
    {
        host_entry = SFAT_LookupHostEntryByIP(&scb->client_ip);
        ssn_dir = SSN_DIR_FROM_CLIENT;
    }
    if (host_entry && (isStreamPolicySet(host_entry) == POLICY_SET))
    {
        policy_id = getStreamPolicy(host_entry);

        if (policy_id != SFAT_UNKNOWN_STREAM_POLICY)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "StreamGetPolicy: Policy Map Entry: %d(%s)\n",
                        policy_id, reassembly_policy_names[policy_id]););

            /* Since we've already done the lookup, try to get the
             * application protocol id with that host_entry. */
            setAppProtocolIdFromHostEntry(scb, host_entry, ssn_dir);
            return policy_id;
        }
    }
#endif

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "StreamGetPolicy: Using configured default %d(%s)\n",
                s5TcpPolicy->policy, reassembly_policy_names[s5TcpPolicy->policy]););

    return s5TcpPolicy->policy;
}

void SetTcpReassemblyPolicy(StreamTracker *st)
{
    st->reassembly_policy = GetTcpReassemblyPolicy(st->os_policy);
}

static void SetOSPolicy(TcpSession *tcpssn)
{
    if (tcpssn->client.os_policy == 0)
    {
        tcpssn->client.os_policy = StreamGetPolicy(tcpssn->scb, tcpssn->client.tcp_policy, FROM_SERVER);
        SetTcpReassemblyPolicy(&tcpssn->client);
    }

    if (tcpssn->server.os_policy == 0)
    {
        tcpssn->server.os_policy = StreamGetPolicy(tcpssn->scb, tcpssn->server.tcp_policy, FROM_CLIENT);
        SetTcpReassemblyPolicy(&tcpssn->server);
    }
}

static inline int ValidMacAddress(StreamTracker *talker,
        StreamTracker *listener,
        Packet *p)
{
    int i;
    int ret = 0;

    if (p->eh == NULL)
        return 0;

    /* Use a for loop and byte comparison, which has proven to be
     * faster on pipelined architectures compared to a memcmp (setup
     * for memcmp is slow).  Not using a 4 byte and 2 byte long because
     * there is no guaranttee of memory alignment (and thus performance
     * issues similar to memcmp). */
    for (i=0;i<6;i++)
    {
        if ((talker->mac_addr[i] != p->eh->ether_src[i]))
        {
            if (p->packet_flags & PKT_FROM_CLIENT)
                ret |= EVENT_SESSION_HIJACK_CLIENT;
            else
                ret |= EVENT_SESSION_HIJACK_SERVER;
        }

        if (listener->mac_addr[i] != p->eh->ether_dst[i])
        {
            if (p->packet_flags & PKT_FROM_CLIENT)
                ret |= EVENT_SESSION_HIJACK_SERVER;
            else
                ret |= EVENT_SESSION_HIJACK_CLIENT;
        }
    }
    return ret;
}

static inline void CopyMacAddr(Packet *p,
        TcpSession *tcpssn,
        int dir)
{
    int i;

    /* Not ethernet based, nothing to do */
    if (p->eh == NULL)
        return;

    if (dir == FROM_CLIENT)
    {
        /* Client is SRC */
        for (i=0;i<6;i++)
        {
            tcpssn->client.mac_addr[i] = p->eh->ether_src[i];
            tcpssn->server.mac_addr[i] = p->eh->ether_dst[i];
        }
    }
    else
    {
        /* Server is SRC */
        for (i=0;i<6;i++)
        {
            tcpssn->server.mac_addr[i] = p->eh->ether_src[i];
            tcpssn->client.mac_addr[i] = p->eh->ether_dst[i];
        }
    }
}

static int NewTcpSession(Packet *p, SessionControlBlock *scb, TcpDataBlock *tdb,
                         StreamTcpPolicy *dstPolicy)
{
    MemBucket *tmpBucket = NULL;
    TcpSession *tmp = NULL;
    uint16_t server_port = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpNewSessPerfStats);

    if (TCP_ISFLAGSET(p->tcph, TH_SYN) &&
            !TCP_ISFLAGSET(p->tcph, TH_ACK))
    {
        /******************************************************************
         * start new sessions on proper SYN packets
         *****************************************************************/
        tmpBucket = session_api->alloc_protocol_session( SESSION_PROTO_TCP );
        if(!tmpBucket)
        {
            PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
            return -1;
        }
        tmp = tmpBucket->data;
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Creating new session tracker on SYN!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif
        scb->ha_state.session_flags |= SSNFLAG_SEEN_CLIENT;

        if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
        {
            scb->ha_state.session_flags |= SSNFLAG_ECN_CLIENT_QUERY;
        }

        /* setup the stream trackers */
        tmp->client.s_mgr.state = TCP_STATE_SYN_SENT;
        tmp->client.isn = tdb->seq;
        tmp->client.l_unackd = tdb->seq + 1;
        tmp->client.l_nxt_seq = tmp->client.l_unackd;

        if ( tdb->seq != tdb->end_seq )
            tmp->client.l_nxt_seq += (tdb->end_seq - tdb->seq - 1);

        tmp->client.l_window = tdb->win;
        tmp->client.ts_last_pkt = p->pkth->ts.tv_sec;

        tmp->server.seglist_base_seq = tmp->client.l_unackd;
        tmp->server.r_nxt_ack = tmp->client.l_unackd;
        tmp->server.r_win_base = tdb->seq+1;

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "seglist_base_seq = %X\n", tmp->server.seglist_base_seq););
        tmp->server.s_mgr.state = TCP_STATE_LISTEN;

        tmp->client.flags |= StreamGetTcpTimestamp(p, &tmp->client.ts_last, 0);
        if (tmp->client.ts_last == 0)
            tmp->client.flags |= TF_TSTAMP_ZERO;
        tmp->client.flags |= StreamGetMss(p, &tmp->client.mss);
        tmp->client.flags |= StreamGetWscale(p, &tmp->client.wscale);


        /* Set the StreamTcpPolicy for each direction (pkt from client) */
        tmp->client.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
        tmp->server.tcp_policy = dstPolicy;

        /* Server is destination */
        server_port = p->dp;

        CopyMacAddr(p, tmp, FROM_CLIENT);
    }
    else if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
    {
        /******************************************************************
         * start new sessions on SYN/ACK from server
         *****************************************************************/
        tmpBucket = session_api->alloc_protocol_session( SESSION_PROTO_TCP );
        if(!tmpBucket)
        {
            PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
            return -1;
        }
        tmp = tmpBucket->data;
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Creating new session tracker on SYN_ACK!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif
        scb->ha_state.session_flags |= SSNFLAG_SEEN_SERVER;

        if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
        {
            scb->ha_state.session_flags |= SSNFLAG_ECN_SERVER_REPLY;
        }

        /* setup the stream trackers */
        tmp->server.s_mgr.state = TCP_STATE_SYN_RCVD;
        tmp->server.isn = tdb->seq;
        tmp->server.l_unackd = tdb->seq + 1;
        tmp->server.l_nxt_seq = tmp->server.l_unackd;
        tmp->server.l_window = tdb->win;

        tmp->server.seglist_base_seq = tdb->ack;
        tmp->server.r_win_base = tdb->ack;
        tmp->server.r_nxt_ack = tdb->ack;
        tmp->server.ts_last_pkt = p->pkth->ts.tv_sec;

        tmp->client.seglist_base_seq = tmp->server.l_unackd;
        tmp->client.r_nxt_ack = tmp->server.l_unackd;
        tmp->client.r_win_base = tdb->seq+1;
        tmp->client.l_nxt_seq = tdb->ack;
        tmp->client.isn = tdb->ack-1;

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "seglist_base_seq = %X\n", tmp->client.seglist_base_seq););
        tmp->client.s_mgr.state = TCP_STATE_SYN_SENT;

        tmp->server.flags |= StreamGetTcpTimestamp(p, &tmp->server.ts_last, 0);
        if (tmp->server.ts_last == 0)
            tmp->server.flags |= TF_TSTAMP_ZERO;
        tmp->server.flags |= StreamGetMss(p, &tmp->server.mss);
        tmp->server.flags |= StreamGetWscale(p, &tmp->server.wscale);

        /* Set the StreamTcpPolicy for each direction (pkt from server) */
        tmp->server.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
        tmp->client.tcp_policy = dstPolicy;
        scb->proto_policy = tmp->server.tcp_policy;

        /* Client is destination */
        server_port = p->sp;

        CopyMacAddr(p, tmp, FROM_SERVER);
    }
    else if ((p->tcph->th_flags & TH_ACK) &&
            !(p->tcph->th_flags & TH_RST) &&
            (scb->session_state & STREAM_STATE_ESTABLISHED))
    {
        /******************************************************************
         * start new sessions on completion of 3-way (ACK only, no data)
         *****************************************************************/
        tmpBucket = session_api->alloc_protocol_session( SESSION_PROTO_TCP );
        if(!tmpBucket)
        {
            PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
            return -1;
        }
        tmp = tmpBucket->data;
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Creating new session tracker on ACK!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif
        scb->ha_state.session_flags |= SSNFLAG_SEEN_CLIENT;

        if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
        {
            scb->ha_state.session_flags |= SSNFLAG_ECN_CLIENT_QUERY;
        }

        /* setup the stream trackers */
        tmp->client.s_mgr.state = TCP_STATE_ESTABLISHED;
        tmp->client.isn = tdb->seq;
        tmp->client.l_unackd = tdb->seq + 1;
        tmp->client.l_nxt_seq = tmp->client.l_unackd;
        tmp->client.l_window = tdb->win;

        tmp->client.ts_last_pkt = p->pkth->ts.tv_sec;

        tmp->server.seglist_base_seq = tmp->client.l_unackd;
        tmp->server.r_nxt_ack = tmp->client.l_unackd;
        tmp->server.r_win_base = tdb->seq+1;

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "seglist_base_seq = %X\n", tmp->server.seglist_base_seq););
        tmp->server.s_mgr.state = TCP_STATE_ESTABLISHED;

        tmp->client.flags |= StreamGetTcpTimestamp(p, &tmp->client.ts_last, 0);
        if (tmp->client.ts_last == 0)
            tmp->client.flags |= TF_TSTAMP_ZERO;
        tmp->client.flags |= StreamGetMss(p, &tmp->client.mss);
        tmp->client.flags |= StreamGetWscale(p, &tmp->client.wscale);

        /* Set the StreamTcpPolicy for each direction (pkt from client) */
        tmp->client.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
        tmp->server.tcp_policy = dstPolicy;

        /* Server is destination */
        server_port = p->dp;

        CopyMacAddr(p, tmp, FROM_CLIENT);
    }
    else if (p->dsize != 0)
    {
        /******************************************************************
         * start new sessions on data in packet
         *****************************************************************/
        tmpBucket = session_api->alloc_protocol_session( SESSION_PROTO_TCP );
        if(!tmpBucket)
        {
            PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
            return -1;
        }
        tmp = tmpBucket->data;
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Creating new session tracker on data packet (ACK|PSH)!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif

        if (scb->ha_state.direction == FROM_CLIENT)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Session direction is FROM_CLIENT\n"););

            /* Sender is client (src port is higher) */
            scb->ha_state.session_flags |= SSNFLAG_SEEN_CLIENT;

            if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
            {
                scb->ha_state.session_flags |= SSNFLAG_ECN_CLIENT_QUERY;
            }

            /* setup the stream trackers */
            tmp->client.s_mgr.state = TCP_STATE_ESTABLISHED;
            tmp->client.isn = tdb->seq;
            tmp->client.l_unackd = tdb->seq;
            tmp->client.l_nxt_seq = tmp->client.l_unackd;
            tmp->client.l_window = tdb->win;
            tmp->client.r_nxt_ack = tdb->ack;
            tmp->client.r_win_base = tdb->ack;

            tmp->client.ts_last_pkt = p->pkth->ts.tv_sec;

            tmp->server.seglist_base_seq = tmp->client.l_unackd;
            tmp->server.r_nxt_ack = tmp->client.l_unackd;
            tmp->server.r_win_base = tdb->seq;
            tmp->server.l_window = 0; /* reset later */

            /* Next server packet is what was ACKd */
            tmp->server.l_nxt_seq = tdb->ack;
            tmp->server.l_unackd = tdb->ack - 1;

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "seglist_base_seq = %X\n", tmp->server.seglist_base_seq););
            tmp->server.s_mgr.state = TCP_STATE_ESTABLISHED;

            tmp->client.flags |= StreamGetTcpTimestamp(p, &tmp->client.ts_last, 0);
            if (tmp->client.ts_last == 0)
                tmp->client.flags |= TF_TSTAMP_ZERO;
            tmp->client.flags |= StreamGetMss(p, &tmp->client.mss);
            tmp->client.flags |= StreamGetWscale(p, &tmp->client.wscale);

            /* Set the StreamTcpPolicy for each direction (pkt from client) */
            tmp->client.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
            tmp->server.tcp_policy = dstPolicy;

            /* Server is destination */
            server_port = p->dp;

            CopyMacAddr(p, tmp, FROM_CLIENT);
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Session direction is FROM_SERVER\n"););

            /* Sender is server (src port is lower) */
            scb->ha_state.session_flags |= SSNFLAG_SEEN_SERVER;

            /* setup the stream trackers */
            tmp->server.s_mgr.state = TCP_STATE_ESTABLISHED;
            tmp->server.isn = tdb->seq;
            tmp->server.l_unackd = tdb->seq;
            tmp->server.l_nxt_seq = tmp->server.l_unackd;
            tmp->server.l_window = tdb->win;

            tmp->server.seglist_base_seq = tdb->ack;
            tmp->server.r_win_base = tdb->ack;
            tmp->server.r_nxt_ack = tdb->ack;
            tmp->server.ts_last_pkt = p->pkth->ts.tv_sec;

            tmp->client.seglist_base_seq = tmp->server.l_unackd;
            tmp->client.r_nxt_ack = tmp->server.l_unackd;
            tmp->client.r_win_base = tdb->seq;
            tmp->client.l_window = 0; /* reset later */
            tmp->client.isn = tdb->ack-1;
            tmp->client.l_nxt_seq = tdb->ack;

            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "seglist_base_seq = %X\n", tmp->client.seglist_base_seq););
            tmp->client.s_mgr.state = TCP_STATE_ESTABLISHED;

            tmp->server.flags |= StreamGetTcpTimestamp(p, &tmp->server.ts_last, 0);
            if (tmp->server.ts_last == 0)
                tmp->server.flags |= TF_TSTAMP_ZERO;
            tmp->server.flags |= StreamGetMss(p, &tmp->server.mss);
            tmp->server.flags |= StreamGetWscale(p, &tmp->server.wscale);

            /* Set the StreamTcpPolicy for each direction (pkt from server) */
            tmp->server.tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
            tmp->client.tcp_policy = dstPolicy;
            scb->proto_policy = tmp->server.tcp_policy;

            /* Client is destination */
            server_port = p->sp;

            CopyMacAddr(p, tmp, FROM_SERVER);
        }
    }

    if (tmp)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "adding TcpSession to lightweight session\n"););
        scb->proto_specific_data = tmpBucket;
        scb->protocol = GET_IPH_PROTO(p);
        tmp->scb = scb;

        /* New session, previous was marked as reset.  Clear the
         * reset flag. */
        if (scb->ha_state.session_flags & SSNFLAG_RESET)
            scb->ha_state.session_flags &= ~SSNFLAG_RESET;

        SetOSPolicy(tmp);

        if ( (scb->ha_state.session_flags & SSNFLAG_CLIENT_SWAP) &&
                !(scb->ha_state.session_flags & SSNFLAG_CLIENT_SWAPPED) )
        {
            StreamTracker trk = tmp->client;
            sfaddr_t ip = scb->client_ip;
            uint16_t port = scb->client_port;

            tmp->client = tmp->server;
            tmp->server = trk;

            scb->client_ip = scb->server_ip;
            scb->server_ip = ip;

            scb->client_port = scb->server_port;
            scb->server_port = port;

            if ( !TwoWayTraffic(scb) )
            {
                if ( scb->ha_state.session_flags & SSNFLAG_SEEN_CLIENT )
                {
                    scb->ha_state.session_flags ^= SSNFLAG_SEEN_CLIENT;
                    scb->ha_state.session_flags |= SSNFLAG_SEEN_SERVER;
                }
                else if ( scb->ha_state.session_flags & SSNFLAG_SEEN_SERVER )
                {
                    scb->ha_state.session_flags ^= SSNFLAG_SEEN_SERVER;
                    scb->ha_state.session_flags |= SSNFLAG_SEEN_CLIENT;
                }
            }
            scb->ha_state.session_flags |= SSNFLAG_CLIENT_SWAPPED;
        }
        /* Set up the flush behaviour, based on the configured info
         * for the server and client ports.
         */
        /* Yes, the server flush manager gets the info from the
         * policy's server port's the flush policy from the client
         * and visa-versa.
         *
         * For example, when policy said 'ports client 80', that means
         * reassemble packets from the client side (stored in the server's
         * flush buffer in the session) destined for port 80.  Port 80 is
         * the server port and we're reassembling the client side.
         * That should make this almost as clear as opaque mud!
         */
#ifdef TARGET_BASED
        if (tmp->client.tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].configured == 1)
        {
            StreamTracker* pst = &tmp->server;
            uint8_t flush_policy =
                pst->tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].client.flush_policy;
            InitFlushMgrByService(scb, pst, scb->ha_state.application_protocol, true, flush_policy);
        }
        else
#endif
        {
            StreamTracker* pst = &tmp->server;
            uint8_t flush_policy =
                pst->tcp_policy->flush_config[server_port].client.flush_policy;
            InitFlushMgrByPort(scb, pst, server_port, true, flush_policy, false);
        }

#ifdef TARGET_BASED
        if (tmp->server.tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].configured == 1)
        {
            StreamTracker* pst = &tmp->client;
            uint8_t flush_policy =
                pst->tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].server.flush_policy;
            InitFlushMgrByService(scb, pst, scb->ha_state.application_protocol, false, flush_policy);
        }
        else
#endif
        {
            StreamTracker* pst = &tmp->client;
            uint8_t flush_policy =
                pst->tcp_policy->flush_config[server_port].server.flush_policy;
            InitFlushMgrByPort(scb, pst, server_port, false, flush_policy, false);
        }


#ifdef STREAM_DEBUG_ENABLED
        PrintTcpSession(tmp);
#endif

        session_api->set_expire_timer(p, scb, dstPolicy->session_timeout);

        s5stats.tcp_streamtrackers_created++;

        AddStreamSession(&sfBase, scb->session_state & STREAM_STATE_MIDSTREAM ? SSNFLAG_MIDSTREAM : 0);

        StreamUpdatePerfBaseState(&sfBase, tmp->scb, TCP_STATE_SYN_SENT);

#ifdef NORMALIZER
        tmp->ecn = 0;
#endif
        tmp->session_decrypted = false;
        PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
        return 1;
    }

    PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
    return 0;
}

/* set_service_based_flush_policy
 *
 * Once appid detects the protocol, calling this api
 * to set the respective paf apis. 
 * If paf function pointer already pointed to the
 * correct paf api, not going to set again.
 */ 
void set_service_based_flush_policy(SessionControlBlock *scb)
{
    TcpSession *tcp_session = NULL;
    StreamTracker* pst;
    uint8_t flush_policy;
    int ret;

#ifdef TARGET_BASED
    if (scb->proto_specific_data)
        tcp_session = scb->proto_specific_data->data;

    if (tcp_session == NULL)
        return;

    if (tcp_session->client.tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].configured == 1)
    {
        pst = &tcp_session->server;
        ret = cb_mask_cmp(((StreamConfig *)scb->stream_config)->tcp_config->paf_config,
                          scb->ha_state.application_protocol, true, pst->paf_state.cb_mask);
        if (ret <= 0)
        {   
            return;
        }

        flush_policy =
            pst->tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].client.flush_policy;
        InitFlushMgrByService(scb, pst, scb->ha_state.application_protocol, true, flush_policy);
    }

    if (tcp_session->server.tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].configured == 1)
    {
        pst = &tcp_session->client;
        ret = cb_mask_cmp(((StreamConfig *)scb->stream_config)->tcp_config->paf_config,
                          scb->ha_state.application_protocol, false, pst->paf_state.cb_mask);
        if (ret <= 0)
        {
            return;
        }

        flush_policy =
            pst->tcp_policy->flush_config_protocol[scb->ha_state.application_protocol].server.flush_policy;
        InitFlushMgrByService(scb, pst, scb->ha_state.application_protocol, false, flush_policy);
    }
#endif
}


static int RepeatedSyn(
        StreamTracker *listener, StreamTracker *talker,
        TcpDataBlock *tdb, TcpSession *tcpssn)
{
    switch (listener->os_policy)
    {
        case STREAM_POLICY_WINDOWS:
        case STREAM_POLICY_WINDOWS2K3:
        case STREAM_POLICY_VISTA:
            /* Windows has some strange behaviour here.  If the
             * sequence of the reset is the next expected sequence,
             * it Resets.  Otherwise it ignores the 2nd SYN.
             */
            if (SEQ_EQ(tdb->seq, listener->r_nxt_ack))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Got syn on established windows ssn, which causes Reset,"
                            "bailing\n"););
                tcpssn->scb->ha_state.session_flags |= SSNFLAG_RESET;
                talker->s_mgr.state = TCP_STATE_CLOSED;
                return ACTION_RST;
            }
            else
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Got syn on established windows ssn, not causing Reset,"
                            "bailing\n"););
                Discard();
                return ACTION_NOTHING;
            }
            break;
        case STREAM_POLICY_MACOS:
            /* MACOS ignores a 2nd SYN, regardless of the sequence number. */
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Got syn on established macos ssn, not causing Reset,"
                        "bailing\n"););
            Discard();
            return ACTION_NOTHING;
            break;
        case STREAM_POLICY_FIRST:
        case STREAM_POLICY_NOACK:
        case STREAM_POLICY_LAST:
        case STREAM_POLICY_LINUX:
        case STREAM_POLICY_OLD_LINUX:
        case STREAM_POLICY_BSD:
        case STREAM_POLICY_SOLARIS:
        case STREAM_POLICY_HPUX11:
        case STREAM_POLICY_HPUX10:
        case STREAM_POLICY_IRIX:
            /* If its not a retransmission of the actual SYN... RESET */
            if(!SEQ_EQ(tdb->seq,talker->isn))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Got syn on established ssn, which causes Reset, bailing\n"););
                tcpssn->scb->ha_state.session_flags |= SSNFLAG_RESET;
                talker->s_mgr.state = TCP_STATE_CLOSED;
                return ACTION_RST;
            }
            else
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Got syn on established ssn, not causing Reset,"
                            "bailing\n"););
                Discard();
                return ACTION_NOTHING;
            }
            break;
    }
    return ACTION_NOTHING;
}

static void LogTcpEvents(StreamTcpPolicy *s5TcpPolicy, int eventcode)
{
    if ( !eventcode )
        return;

    if (eventcode & EVENT_SYN_ON_EST)
        EventSynOnEst(s5TcpPolicy);

    if (eventcode & EVENT_DATA_ON_SYN)
        EventDataOnSyn(s5TcpPolicy);

    if (eventcode & EVENT_DATA_ON_CLOSED)
        EventDataOnClosed(s5TcpPolicy);

    if (eventcode & EVENT_BAD_TIMESTAMP)
        EventBadTimestamp(s5TcpPolicy);

    if (eventcode & EVENT_BAD_SEGMENT)
        EventBadSegment(s5TcpPolicy);

    if (eventcode & EVENT_WINDOW_TOO_LARGE)
        EventWindowTooLarge(s5TcpPolicy);

    if (eventcode & EVENT_EXCESSIVE_TCP_OVERLAPS)
        EventExcessiveOverlap(s5TcpPolicy);

    if (eventcode & EVENT_DATA_AFTER_RESET)
        EventDataAfterReset(s5TcpPolicy);

    if (eventcode & EVENT_SESSION_HIJACK_CLIENT)
        EventSessionHijackedClient(s5TcpPolicy);

    if (eventcode & EVENT_SESSION_HIJACK_SERVER)
        EventSessionHijackedServer(s5TcpPolicy);

    if (eventcode & EVENT_DATA_WITHOUT_FLAGS)
        EventDataWithoutFlags(s5TcpPolicy);

    if (eventcode & EVENT_4WHS)
        Event4whs(s5TcpPolicy);

    if (eventcode & EVENT_NO_TIMESTAMP)
        EventNoTimestamp(s5TcpPolicy);

    if (eventcode & EVENT_BAD_RST)
        EventBadReset(s5TcpPolicy);

    if (eventcode & EVENT_BAD_FIN)
        EventBadFin(s5TcpPolicy);

    if (eventcode & EVENT_BAD_ACK)
        EventBadAck(s5TcpPolicy);

    if (eventcode & EVENT_DATA_AFTER_RST_RCVD)
        EventDataAfterRstRcvd(s5TcpPolicy);

    if (eventcode & EVENT_WINDOW_SLAM)
        EventWindowSlam(s5TcpPolicy);

    if (eventcode & EVENT_WIN_SZ_0_TCP_FIN_WAIT_1)
        EventWindowZeroAfterFinAck(s5TcpPolicy);
}

static inline void DisableInspection (SessionControlBlock *scb, Packet* p, char ignore)
{
    /* Set the directions to ignore... */
    scb->ha_state.ignore_direction = ignore;
    StreamSetReassemblyTcp(scb, STREAM_FLPOLICY_IGNORE, ignore, STREAM_FLPOLICY_SET_ABSOLUTE);
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Stream: Ignoring packet from %d. Marking session marked as ignore.\n",
                p->packet_flags & PKT_FROM_SERVER? "server" : "client"););

    session_api->disable_inspection(scb, p);
}

static inline bool checkFINTransitionStatus(const Packet* p, StreamTracker *listener)
{
    if((p->dsize != 0) && (listener->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT) &&
            ((listener->s_mgr.transition_seq - 1) == listener->r_nxt_ack))
        return true;
    return false;
}

static int ProcessTcp(SessionControlBlock *scb, Packet *p, TcpDataBlock *tdb,
                      StreamTcpPolicy *s5TcpPolicy, SFXHASH_NODE *hash_node)
{
    int retcode = ACTION_NOTHING;
    int eventcode = 0;
    char ignore;
    int got_ts = 0;
    int new_ssn = 0;
    int ts_action = ACTION_NOTHING;
    TcpSession *tcpssn = NULL;
    StreamTracker *talker = NULL;
    StreamTracker *listener = NULL;
    uint32_t require3Way = (s5TcpPolicy->flags & STREAM_CONFIG_REQUIRE_3WHS);
    bool process_fin = false;
    STREAM_DEBUG_WRAP(char *t = NULL; char *l = NULL;)
        PROFILE_VARS;

    if (scb->protocol != IPPROTO_TCP)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Lightweight session not TCP on TCP packet\n"););
        return retcode;
    }

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    PREPROC_PROFILE_START(s5TcpStatePerfStats);
            
    if (tcpssn == NULL)
    {
        if ( ScPafEnabled() )
        {
            /* Check if the session is to be ignored */
            if (hash_node)
                ignore = StreamExpectProcessNode(p, scb, hash_node);
            else
                ignore = StreamExpectCheck(p, scb);
            if (ignore)
            {
                DisableInspection(scb, p, ignore);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode;
            }
        }

        if (TCP_ISFLAGSET(p->tcph, TH_SYN) && !TCP_ISFLAGSET(p->tcph, TH_ACK))
        {
            scb->session_state |= STREAM_STATE_SYN;

            if( require3Way || ( StreamPacketHasWscale( p ) & TF_WSCALE ) ||
                    ( ( p->dsize > 0 ) &&
                      ( StreamGetPolicy( scb, s5TcpPolicy, FROM_CLIENT ) == STREAM_POLICY_MACOS ) ) )
            {
                /* Create TCP session if we
                 * 1) require 3-WAY HS, OR
                 * 2) client sent wscale option, OR
                 * 3) have data and its a MAC OS policy -- MAC
                 *    is the only one that accepts data on SYN
                 *    (and thus requires a TCP session at this point)
                 */
                if(NewTcpSession(p, scb, tdb, s5TcpPolicy)== -1 )
                {
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return retcode;
                }
                tcpssn = (TcpSession *)scb->proto_specific_data->data;
                new_ssn = 1;
                NormalTrackECN(tcpssn, p->tcph, require3Way);
            }

            /* Nothing left todo here */
        }
        else if( TCP_ISFLAGSET( p->tcph, ( TH_SYN | TH_ACK ) ) )
        {
            scb->session_state |= STREAM_STATE_SYN_ACK;
            if (!require3Way || midstream_allowed)
            {
                if(NewTcpSession(p, scb, tdb, s5TcpPolicy)== -1 )
                {
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return retcode;
                }
                tcpssn = (TcpSession *)scb->proto_specific_data->data;
                new_ssn = 1;
            }
            NormalTrackECN(tcpssn, p->tcph, require3Way);
            /* Nothing left todo here */
        }
        else if( TCP_ISFLAGSET( p->tcph, TH_ACK ) && !TCP_ISFLAGSET( p->tcph, TH_RST )
                &&
                ( scb->session_state & STREAM_STATE_SYN_ACK ) )
        {
            /* TODO: do we need to verify the ACK field is >= the seq of the SYN-ACK? */

            /* 3-way Handshake complete, create TCP session */
            scb->session_state |= STREAM_STATE_ACK | STREAM_STATE_ESTABLISHED;
            if(NewTcpSession(p, scb, tdb, s5TcpPolicy)== -1 )
            {
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode;
            }
            tcpssn = (TcpSession *)scb->proto_specific_data->data;
            new_ssn = 1;

            NormalTrackECN(tcpssn, p->tcph, require3Way);
            StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_ESTABLISHED);
        }
        else if ((p->dsize > 0) && (!require3Way || midstream_allowed))
        {
            // TBD-EDM not sure we need this here...
            //
            /*  need to figure out direction, etc
                Assume from client, can update later */
            if (p->sp > p->dp)
            {
                scb->ha_state.direction = FROM_CLIENT;
                IP_COPY_VALUE(scb->client_ip, GET_SRC_IP(p));
                scb->client_port = p->tcph->th_sport;
                IP_COPY_VALUE(scb->server_ip, GET_DST_IP(p));
                scb->server_port = p->tcph->th_dport;
            }
            else
            {
                scb->ha_state.direction = FROM_SERVER;
                IP_COPY_VALUE(scb->client_ip, GET_DST_IP(p));
                scb->client_port = p->tcph->th_dport;
                IP_COPY_VALUE(scb->server_ip, GET_SRC_IP(p));
                scb->server_port = p->tcph->th_sport;
            }
            scb->session_state |= STREAM_STATE_MIDSTREAM;
            scb->ha_state.session_flags |= SSNFLAG_MIDSTREAM;


#ifdef STREAM_DEBUG_ENABLED
            if (ScReadMode())
            {
                /* If we're in readback mode... may only have one packet.
                 * That being packet with the exploit being tested, so
                 * mark this session as established, so rule option
                 * 'flow:established' works correctly.
                 */
                STREAM_DEBUG_WRAP(
                        char timestamp[TIMEBUF_SIZE];
                        char src_addr[17];
                        char dst_addr[17];
                        memset((char *)timestamp, 0, TIMEBUF_SIZE);
                        ts_print((struct timeval *) &p->pkth->ts, timestamp);
                        SnortSnprintf(src_addr, 17, "%s",
                            inet_ntoa(GET_SRC_ADDR(p)));
                        SnortSnprintf(dst_addr, 17, "%s",
                            inet_ntoa(GET_DST_ADDR(p)));
                        DebugMessage(DEBUG_STREAM_STATE, "Session not established"
                            "on midstream-pickup of data packet.  Will be marked"
                            "as established when other side is seen. Packet Info:"
                            "Time: %s\tSrc: %s:%d\tDst: %s:%d\n",
                            timestamp, src_addr, p->sp, dst_addr, p->dp);

                        );
#ifdef MIMIC_STREAM4_MIDSTREAM_BEHAVIOUR
                scb->session_state |= STREAM_STATE_ESTABLISHED;
                scb->ha_state.session_flags |= SSNFLAG_ESTABLISHED;
#endif
            }
#endif
            if(NewTcpSession(p, scb, tdb, s5TcpPolicy)== -1 )
            {
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode;
            }
            tcpssn = (TcpSession *)scb->proto_specific_data->data;
            new_ssn = 1;
            NormalTrackECN(tcpssn, p->tcph, require3Way);

            if (scb->session_state & STREAM_STATE_ESTABLISHED)
                StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_ESTABLISHED);
        }
        else if (p->dsize == 0)
        {
            /* Already have a scb, but no tcp session.
             * Probably just an ACK of already sent data (that
             * we missed).
             */
            /* Do nothing. */
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return retcode;
        }


        /* This flag is set in session_expect when the event handler is set.
         * It is assumed that if we need to register the event handler before
         * analyzing the packet, we will also want to begin packet reassembly
         * immediately.  If a preprocessor does not want to set reassembly in
         * both directory, it will call set_reassembly again with the correct
         * parameters.
         */
        if (p->packet_flags & PKT_EARLY_REASSEMBLY)
        {
            p->packet_flags &= ~PKT_EARLY_REASSEMBLY;
            StreamSetReassemblyTcp(scb, STREAM_FLPOLICY_FOOTPRINT,
                   SSN_DIR_BOTH, STREAM_FLPOLICY_SET_ABSOLUTE);
        }
    }
    else
    {

        /* If session is already marked as established */
        if (!(scb->session_state & STREAM_STATE_ESTABLISHED))
        {
            if (hash_node)
                ignore = StreamExpectProcessNode(p, scb, hash_node);
            else
                ignore = StreamExpectCheck(p, scb);
            if (p->packet_flags & PKT_EARLY_REASSEMBLY)
            {
                p->packet_flags &= ~PKT_EARLY_REASSEMBLY;
                StreamSetReassemblyTcp(scb, STREAM_FLPOLICY_FOOTPRINT,
                    ~(ignore & SSN_DIR_BOTH) & SSN_DIR_BOTH, //Directions to not ignore
                    STREAM_FLPOLICY_SET_ABSOLUTE);
            }

            if(!require3Way || midstream_allowed)
            {
                /* If not requiring 3-way Handshake... */

                /* TCP session created on TH_SYN above,
                 * or maybe on SYN-ACK, or anything else */

                /* Need to update Lightweight session state */
                if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
                {
                    /* SYN-ACK from server */
                    if (scb->session_state != STREAM_STATE_NONE)
                    {
                        scb->session_state |= STREAM_STATE_SYN_ACK;
                    }
                }
                else if (TCP_ISFLAGSET(p->tcph, TH_ACK) &&
                        (scb->session_state & STREAM_STATE_SYN_ACK))
                {
                    scb->session_state |= STREAM_STATE_ACK | STREAM_STATE_ESTABLISHED;
                    StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_ESTABLISHED);
                }

            }
        }

#ifdef NORMALIZER
        if (TCP_ISFLAGSET(p->tcph, TH_SYN))
            NormalTrackECN(tcpssn, p->tcph, require3Way);
#endif
    }

    two_way_traffic = TwoWayTraffic(scb);
    /* figure out direction of this packet */
    session_api->set_packet_direction_flag(p, scb);

    if(p->packet_flags & PKT_FROM_SERVER)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream: Updating on packet from server\n"););
        scb->ha_state.session_flags |= SSNFLAG_SEEN_SERVER;
        if (tcpssn)
        {
            talker = &tcpssn->server;
            listener = &tcpssn->client;
        }

        STREAM_DEBUG_WRAP(
                t = "Server";
                l = "Client");

        if ( talker && talker->s_mgr.state == TCP_STATE_LISTEN &&
                ((p->tcph->th_flags & (TH_SYN|TH_ACK)) == TH_SYN) )
        {
            eventcode |= EVENT_4WHS;
        }
        /* If we picked this guy up midstream, finish the initialization */
        if ((scb->session_state & STREAM_STATE_MIDSTREAM) &&
                !(scb->session_state & STREAM_STATE_ESTABLISHED))
        {
            FinishServerInit(p, tdb, tcpssn);
            if((p->tcph->th_flags & TH_ECE) &&
                    scb->ha_state.session_flags & SSNFLAG_ECN_CLIENT_QUERY)
            {
                scb->ha_state.session_flags |= SSNFLAG_ECN_SERVER_REPLY;
            }

            if (scb->ha_state.session_flags & SSNFLAG_SEEN_CLIENT)
            {
                // should TCP state go to established too?
                scb->session_state |= STREAM_STATE_ESTABLISHED;
                scb->ha_state.session_flags |= SSNFLAG_ESTABLISHED;
                StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_ESTABLISHED);
            }
        }
#ifdef ACTIVE_RESPONSE
        if ( !scb->inner_server_ttl )
            SetTTL(scb, p, 0);
#endif
    }
    else
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream: Updating on packet from client\n"););
        /* if we got here we had to see the SYN already... */
        scb->ha_state.session_flags |= SSNFLAG_SEEN_CLIENT;
        if (tcpssn)
        {
            talker = &tcpssn->client;
            listener = &tcpssn->server;
        }

        STREAM_DEBUG_WRAP(
                t = "Client";
                l = "Server";);

        if ((scb->session_state & STREAM_STATE_MIDSTREAM) &&
                !(scb->session_state & STREAM_STATE_ESTABLISHED))
        {
            /* Midstream and seen server. */
            if (scb->ha_state.session_flags & SSNFLAG_SEEN_SERVER)
            {
                scb->session_state |= STREAM_STATE_ESTABLISHED;
                scb->ha_state.session_flags |= SSNFLAG_ESTABLISHED;
            }
        }
#ifdef ACTIVE_RESPONSE
        if ( !scb->inner_client_ttl )
            SetTTL(scb, p, 1);
#endif
    }
    /* Check if there is a reload and policy is changed */
    if( listener && talker && listener->tcp_policy != s5TcpPolicy && talker->tcp_policy != s5TcpPolicy)
    {
        listener->tcp_policy = s5TcpPolicy;
        talker->tcp_policy = StreamPolicyLookup(scb, GET_SRC_IP(p));
    }
    /*
     * check for SYN on reset session
     */
    if ((scb->ha_state.session_flags & SSNFLAG_RESET) &&
            (p->tcph->th_flags & TH_SYN))
    {
        if ((!tcpssn) ||
                ((listener->s_mgr.state == TCP_STATE_CLOSED) ||
                 (talker->s_mgr.state == TCP_STATE_CLOSED)))
        {
            /* Listener previously issued a reset */
            /* Talker is re-SYN-ing */
            struct session_state_cleanup_cache sscc;

            cleanup_cache_session_state( scb, &sscc );
            TcpSessionCleanupWithFreeApplicationData(scb);
            cleanup_log_session_state( "SYN on RST ssn", &sscc );

            if (p->tcph->th_flags & TH_RST)
            {
                /* Got SYN/RST.  We're done. */
                NormalTrimPayloadIfSyn(p, 0, tdb);
                NormalTrimPayloadIfRst(p, 0, tdb);
                tcpssn = NULL;
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode | ACTION_RST;
            }
            else if (TCP_ISFLAGSET(p->tcph, TH_SYN) &&
                    !TCP_ISFLAGSET(p->tcph, TH_ACK))
            {
                scb->ha_state.direction = FROM_CLIENT;
                IP_COPY_VALUE(scb->client_ip, GET_SRC_IP(p));
                scb->client_port = p->tcph->th_sport;
                IP_COPY_VALUE(scb->server_ip, GET_DST_IP(p));
                scb->server_port = p->tcph->th_dport;
                scb->session_state = STREAM_STATE_SYN;
#ifdef ACTIVE_RESPONSE
                SetTTL(scb, p, 1);
#endif
                if(NewTcpSession(p, scb, tdb, s5TcpPolicy)== -1 )
                {
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return retcode;
                }

                tcpssn = (TcpSession *)scb->proto_specific_data->data;
                new_ssn = 1;
                NormalTrackECN(tcpssn, p->tcph, require3Way);

                if (tcpssn)
                {
                    listener = &tcpssn->server;
                    talker = &tcpssn->client;
                }
                scb->ha_state.session_flags = SSNFLAG_SEEN_CLIENT;
            }
            else if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
            {
                scb->ha_state.direction = FROM_SERVER;
                IP_COPY_VALUE(scb->client_ip, GET_DST_IP(p));
                scb->client_port = p->tcph->th_dport;
                IP_COPY_VALUE(scb->server_ip, GET_SRC_IP(p));
                scb->server_port = p->tcph->th_sport;
                scb->session_state = STREAM_STATE_SYN_ACK;
#ifdef ACTIVE_RESPONSE
                SetTTL(scb, p, 0);
#endif
                if(NewTcpSession(p, scb, tdb, s5TcpPolicy)== -1 )
                {
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return retcode;
                }
                tcpssn = (TcpSession *)scb->proto_specific_data->data;
                new_ssn = 1;
                NormalTrackECN(tcpssn, p->tcph, require3Way);

                if (tcpssn)
                {
                    listener = &tcpssn->client;
                    talker = &tcpssn->server;
                }
                scb->ha_state.session_flags = SSNFLAG_SEEN_SERVER;
            }
        }
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Got SYN pkt on reset ssn, re-SYN-ing\n"););
    }

    if (((p->packet_flags & PKT_FROM_SERVER) && (scb->ha_state.ignore_direction & SSN_DIR_FROM_CLIENT)) ||
            ((p->packet_flags & PKT_FROM_CLIENT) && (scb->ha_state.ignore_direction & SSN_DIR_FROM_SERVER)))
    {
        if (talker && (talker->flags & TF_FORCE_FLUSH))
        {
            StreamFlushTalker(p, scb);
            talker->flags &= ~TF_FORCE_FLUSH;
        }

        if (listener && (listener->flags & TF_FORCE_FLUSH))
        {
            StreamFlushListener(p, scb);
            listener->flags &= ~TF_FORCE_FLUSH;
        }
        p->packet_flags |= PKT_IGNORE;
        retcode |= ACTION_DISABLE_INSPECTION;
    }

    /* Check if the session is to be ignored */
    if ( !ScPafEnabled() )
    {
        if (hash_node)
            ignore = StreamExpectProcessNode(p, scb, hash_node);
        else
            ignore = StreamExpectCheck(p, scb);
        if (ignore)
        {
            DisableInspection(scb, p, ignore);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return retcode;
        }
    }

    /* Handle data on SYN */
    if (!IsTCPFastOpenPkt(p) && p->dsize && TCP_ISFLAGSET(p->tcph, TH_SYN))
    {
        /* MacOS accepts data on SYN, so don't alert if policy is MACOS */
        if (StreamGetPolicy(scb, s5TcpPolicy, FROM_CLIENT) !=
                STREAM_POLICY_MACOS)
        {
#ifdef NORMALIZER
            NormalTrimPayloadIfSyn(p, 0, tdb); // remove data on SYN
            if ( Normalize_GetMode(snort_conf, NORM_TCP_TRIM_SYN) == NORM_MODE_OFF)
#endif
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Got data on SYN packet, not processing it\n"););
                //EventDataOnSyn(s5TcpPolicy);
                eventcode |= EVENT_DATA_ON_SYN;
                retcode |= ACTION_BAD_PKT;
            }
        }
    }

    if (!tcpssn)
    {
        LogTcpEvents(s5TcpPolicy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return retcode;
    }

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "   %s [talker] state: %s\n", t,
                state_names[talker->s_mgr.state]););
    STREAM_DEBUG_WRAP(PrintFlushMgr(&talker->flush_mgr););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "   %s state: %s(%d)\n", l,
                state_names[listener->s_mgr.state],
                listener->s_mgr.state););
    STREAM_DEBUG_WRAP(PrintFlushMgr(&listener->flush_mgr););

    // may find better placement to eliminate redundant flag checks
    if(p->tcph->th_flags & TH_SYN)
        talker->s_mgr.sub_state |= SUB_SYN_SENT;
    if(p->tcph->th_flags & TH_ACK)
        talker->s_mgr.sub_state |= SUB_ACK_SENT;

    /*
     * process SYN ACK on unestablished sessions
     */
    if( (TCP_STATE_SYN_SENT == listener->s_mgr.state) &&
            (TCP_STATE_LISTEN == talker->s_mgr.state) )
    {
        if(p->tcph->th_flags & TH_ACK)
        {
            /*
             * make sure we've got a valid segment
             */
            if(!IsBetween(listener->l_unackd, listener->l_nxt_seq, tdb->ack))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Pkt ack is out of bounds, bailing!\n"););
                Discard();
                NormalTrimPayloadIfWin(p, 0, tdb);
                LogTcpEvents(listener->tcp_policy, eventcode);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode | ACTION_BAD_PKT;
            }
        }

        talker->flags |= StreamGetTcpTimestamp(p, &tdb->ts, 0);
        if (tdb->ts == 0)
            talker->flags |= TF_TSTAMP_ZERO;

        /*
         * catch resets sent by server
         */
        if(p->tcph->th_flags & TH_RST)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "got RST\n"););

            NormalTrimPayloadIfRst(p, 0, tdb);

            /* Reset is valid when in SYN_SENT if the
             * ack field ACKs the SYN.
             */
            if(ValidRstSynSent(listener, tdb))
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "got RST, closing talker\n"););
                /* Reset is valid */
                /* Mark session as reset... Leave it around so that any
                 * additional data sent from one side or the other isn't
                 * processed (and is dropped in inline mode).
                 */
                scb->ha_state.session_flags |= SSNFLAG_RESET;
                talker->s_mgr.state = TCP_STATE_CLOSED;
                StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_CLOSING);
                /* Leave listener open, data may be in transit */
                LogTcpEvents(listener->tcp_policy, eventcode);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode | ACTION_RST;
            }
            /* Reset not valid. */
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "bad sequence number, bailing\n"););
            Discard();
            eventcode |= EVENT_BAD_RST;
            NormalDropPacket(p);
            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return retcode;
        }

        /*
         * finish up server init
         */
        if(p->tcph->th_flags & TH_SYN)
        {
            FinishServerInit(p, tdb, tcpssn);
            if (talker->flags & TF_TSTAMP)
            {
                talker->ts_last_pkt = p->pkth->ts.tv_sec;
                talker->ts_last = tdb->ts;
            }
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Finish server init got called!\n"););
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Finish server init didn't get called!\n"););
        }

        if((p->tcph->th_flags & TH_ECE) &&
                scb->ha_state.session_flags & SSNFLAG_ECN_CLIENT_QUERY)
        {
            scb->ha_state.session_flags |= SSNFLAG_ECN_SERVER_REPLY;
        }

        /*
         * explicitly set the state
         */
        listener->s_mgr.state = TCP_STATE_SYN_SENT;
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Accepted SYN ACK\n"););
    }

    /*
     * scale the window.  Only if BOTH client and server specified
     * wscale option as part of 3-way handshake.
     * This is per RFC 1323.
     */
    if ((talker->flags & TF_WSCALE) && (listener->flags & TF_WSCALE))
    {
        tdb->win <<= talker->wscale;
    }

    /* Check for session hijacking -- compare mac address to the ones
     * that were recorded at session startup.
     */
#ifdef DAQ_PKT_FLAG_PRE_ROUTING
    if (!(p->pkth->flags & DAQ_PKT_FLAG_PRE_ROUTING) &&
            listener->tcp_policy->flags & STREAM_CONFIG_CHECK_SESSION_HIJACKING)
#else
        if (listener->tcp_policy->flags & STREAM_CONFIG_CHECK_SESSION_HIJACKING)
#endif
        {
            eventcode |= ValidMacAddress(talker, listener, p);
        }

    /* Check timestamps */
    ts_action = ValidTimestamp(talker, listener, tdb, p, &eventcode, &got_ts);

    /*
     * check RST validity
     */
    if(p->tcph->th_flags & TH_RST)
    {
        NormalTrimPayloadIfRst(p, 0, tdb);

        if(ValidRst(scb, listener, tdb))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Got RST, bailing\n"););

            if (
                    listener->s_mgr.state == TCP_STATE_FIN_WAIT_1 ||
                    listener->s_mgr.state == TCP_STATE_FIN_WAIT_2 ||
                    listener->s_mgr.state == TCP_STATE_CLOSE_WAIT ||
                    listener->s_mgr.state == TCP_STATE_LAST_ACK   ||
                    listener->s_mgr.state == TCP_STATE_CLOSING
               ) {
                StreamFlushTalker(p, scb);
                StreamFlushListener(p, scb);
                scb->ha_state.session_flags |= SSNFLAG_FREE_APP_DATA;
            }
            scb->ha_state.session_flags |= SSNFLAG_RESET;
            talker->s_mgr.state = TCP_STATE_CLOSED;
            talker->s_mgr.sub_state |= SUB_RST_SENT;
            StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_CLOSING);

#ifdef NORMALIZER
            if ( Normalize_GetMode(snort_conf, NORM_TCP_IPS) == NORM_MODE_ON )
                listener->s_mgr.state = TCP_STATE_CLOSED;
            /* else for ids:
               leave listener open, data may be in transit */
#endif

            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return retcode | ACTION_RST;
        }
        /* Reset not valid. */
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bad sequence number, bailing\n"););
        Discard();
        eventcode |= EVENT_BAD_RST;
        NormalDropPacket(p);
        LogTcpEvents(listener->tcp_policy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return retcode | ts_action;
    }
    else
    {
        /* check for valid seqeuence/retrans */
        bool before_win_base = false;
        if( s5TcpPolicy->policy != STREAM_POLICY_NOACK )
        {
            if ( (listener->s_mgr.state >= TCP_STATE_ESTABLISHED) &&
                    !ValidSeq(p, scb, listener, tdb, &before_win_base) )
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "bad sequence number, bailing\n"););
                Discard();
                if(before_win_base)
                    DisableAppPreprocessors(p);
                NormalTrimPayloadIfWin(p, 0, tdb);
                LogTcpEvents(listener->tcp_policy, eventcode);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode | ts_action;
            }
        }
    }

    if (ts_action != ACTION_NOTHING)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bad timestamp, bailing\n"););
        Discard();
        // this packet was normalized elsewhere
        LogTcpEvents(listener->tcp_policy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return retcode | ts_action;
    }

    /*
     * update PAWS timestamps
     */
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "PAWS update tdb->seq %lu > listener->r_win_base %lu\n",
                tdb->seq, listener->r_win_base););
    if(got_ts && SEQ_EQ(listener->r_win_base, tdb->seq))
    {
        if((int32_t)(tdb->ts - talker->ts_last) >= 0 ||
                (uint32_t)p->pkth->ts.tv_sec >= talker->ts_last_pkt+PAWS_24DAYS)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "updating timestamps...\n"););
            talker->ts_last = tdb->ts;
            talker->ts_last_pkt = p->pkth->ts.tv_sec;
        }
    }
    else
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "not updating timestamps...\n"););
    }

    /*
     * check for repeat SYNs
     */
    if ( !new_ssn &&
            ((p->tcph->th_flags & (TH_SYN|TH_ACK)) == TH_SYN) )
    {
        int action;
#ifdef NORMALIZER
        if ( !SEQ_EQ(tdb->seq, talker->isn) &&
                NormalDropPacket(p) )
            action = ACTION_BAD_PKT;
        else
#endif
            if ( talker->s_mgr.state >= TCP_STATE_ESTABLISHED )
                action = RepeatedSyn(listener, talker, tdb, tcpssn);
            else
                action = ACTION_NOTHING;

        if (action != ACTION_NOTHING)
        {
            /* got a bad SYN on the session, alert! */
            eventcode |= EVENT_SYN_ON_EST;
            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return retcode | action;
        }
    }

    /*
     * Check that the window is within the limits
     */
    if ( s5TcpPolicy->policy != STREAM_POLICY_NOACK )
    {
        if (listener->tcp_policy->max_window && (tdb->win > listener->tcp_policy->max_window))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Got window that was beyond the allowed policy value, bailing\n"););
            /* got a window too large, alert! */
            eventcode |= EVENT_WINDOW_TOO_LARGE;
            Discard();
            NormalDropPacket(p);
            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return retcode | ACTION_BAD_PKT;
        }
        else if ((p->packet_flags & PKT_FROM_CLIENT)
                && (tdb->win <= SLAM_MAX) && (tdb->ack == listener->isn + 1)
                && !(p->tcph->th_flags & (TH_FIN|TH_RST))
                && !(scb->ha_state.session_flags & SSNFLAG_MIDSTREAM))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Window slammed shut!\n"););
            /* got a window slam alert! */
            eventcode |= EVENT_WINDOW_SLAM;
            Discard();

#ifdef NORMALIZER
            if ( NormalDropPacket(p) )
            {
                LogTcpEvents(listener->tcp_policy, eventcode);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return retcode | ACTION_BAD_PKT;
            }
#endif
        }
    }

    if(talker->s_mgr.state_queue != TCP_STATE_NONE)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Found queued state transition on ack 0x%X, "
                    "current 0x%X!\n", talker->s_mgr.transition_seq,
                    tdb->ack););
        if(tdb->ack == talker->s_mgr.transition_seq)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "accepting transition!\n"););
            talker->s_mgr.state = talker->s_mgr.state_queue;
            talker->s_mgr.state_queue = TCP_STATE_NONE;
            //If ACK is received for Queued FIN before previous data is received. Process FIN transition
            if(listener->s_mgr.state == TCP_STATE_ESTABLISHED)
            {
                listener->l_nxt_seq++;
                talker->r_nxt_ack = tdb->ack;
                listener->s_mgr.sub_state |= SUB_FIN_SENT;
                listener->s_mgr.state = TCP_STATE_FIN_WAIT_1;

                StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_CLOSING);
            }
        }
    }

    /*
     * process ACK flags
     */
    if(p->tcph->th_flags & TH_ACK)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Got an ACK...\n"););
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "   %s [listener] state: %s\n", l,
                    state_names[listener->s_mgr.state]););

        switch(listener->s_mgr.state)
        {
            case TCP_STATE_SYN_SENT:
                if ( !require3Way || midstream_allowed )
                    break;
                // fall thru ...
            case TCP_STATE_SYN_RCVD:
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "listener state is SYN_SENT...\n"););
                if(IsBetween(listener->l_unackd, listener->l_nxt_seq, tdb->ack) &&
                        ((!require3Way || midstream_allowed) ||
                         ((talker->s_mgr.sub_state == SUB_SETUP_OK) &&
                          (listener->s_mgr.sub_state == SUB_SETUP_OK)) ))
                {
                    UpdateSsn(p, listener, talker, tdb);
                    scb->ha_state.session_flags |= SSNFLAG_ESTABLISHED;
                    scb->session_state |= STREAM_STATE_ESTABLISHED;
                    listener->s_mgr.state = TCP_STATE_ESTABLISHED;
                    talker->s_mgr.state = TCP_STATE_ESTABLISHED;
                    StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_ESTABLISHED);
                    /* Indicate this packet completes 3-way handshake */
                    p->packet_flags |= PKT_STREAM_TWH;
                }

                talker->flags |= got_ts;
                if(got_ts && SEQ_EQ(listener->r_nxt_ack, tdb->seq))
                {
                    talker->ts_last_pkt = p->pkth->ts.tv_sec;
                    talker->ts_last = tdb->ts;
                }

                break;

            case TCP_STATE_ESTABLISHED:
                /* Handle out-of-order/OOO ACK */
                if ((Normalize_GetMode(snort_conf, NORM_TCP_IPS) == NORM_MODE_ON) &&
                    (SEQ_GT(tdb->ack, listener->l_nxt_seq)))
                {
                    NormalDropPacket(p);
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return retcode | ACTION_BAD_PKT;
                }
                UpdateSsn(p, listener, talker, tdb);
                break;

            case TCP_STATE_CLOSE_WAIT:
                UpdateSsn(p, listener, talker, tdb);
                break;

            case TCP_STATE_FIN_WAIT_1:
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "tdb->ack %X >= talker->r_nxt_ack %X\n",
                            tdb->ack, talker->r_nxt_ack););

                if ( SEQ_EQ(tdb->ack, listener->l_nxt_seq) )
                {
#ifdef NORMALIZER
                    if ( (listener->os_policy == STREAM_POLICY_WINDOWS) && (tdb->win == 0) )
                    {
                        eventcode |= EVENT_WIN_SZ_0_TCP_FIN_WAIT_1;
                    }
#endif
                    UpdateSsn(p, listener, talker, tdb);
                    listener->s_mgr.state = TCP_STATE_FIN_WAIT_2;

                    if ( (p->tcph->th_flags & TH_FIN) )
                    {
                        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "seq ok, setting state!\n"););

                        if (talker->s_mgr.state_queue == TCP_STATE_NONE)
                        {
                            talker->s_mgr.state = TCP_STATE_LAST_ACK;
                            EndOfFileHandle(p, (TcpSession *)scb->proto_specific_data->data);
                        }
                        if ( scb->ha_state.session_flags & SSNFLAG_MIDSTREAM )
                        {
                            // FIXTHIS this should be handled below in fin section
                            // but midstream sessions fail the seq test
                            listener->s_mgr.state_queue = TCP_STATE_TIME_WAIT;
                            listener->s_mgr.transition_seq = tdb->end_seq;
                            listener->s_mgr.expected_flags = TH_ACK;
                        }
                    }
                    else if (listener->s_mgr.state_queue == TCP_STATE_CLOSING)
                    {
                        listener->s_mgr.state_queue = TCP_STATE_TIME_WAIT;
                        listener->s_mgr.transition_seq = tdb->end_seq;
                        listener->s_mgr.expected_flags = TH_ACK;
                    }
                }
                else
                {
                    UpdateSsn(p, listener, talker, tdb);
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "bad ack!\n"););
                }
                break;

            case TCP_STATE_FIN_WAIT_2:
                if ( SEQ_GT(tdb->ack, listener->l_nxt_seq) )
                {
                    eventcode |= EVENT_BAD_ACK;
                    LogTcpEvents(talker->tcp_policy, eventcode);
                    NormalDropPacket(p);
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return retcode | ACTION_BAD_PKT;
                }
                UpdateSsn(p, listener, talker, tdb);
                break;

            case TCP_STATE_CLOSING:
                UpdateSsn(p, listener, talker, tdb);
                if(SEQ_GEQ(tdb->end_seq, listener->r_nxt_ack))
                {
                    listener->s_mgr.state = TCP_STATE_TIME_WAIT;
                }
                break;

            case TCP_STATE_LAST_ACK:
                UpdateSsn(p, listener, talker, tdb);

                if ( SEQ_EQ(tdb->ack, listener->l_nxt_seq) )
                {
                    listener->s_mgr.state = TCP_STATE_CLOSED;
                }
                break;

            default:
                // FIXTHIS safe to ignore when inline?
                break;
        }
    }

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
            tcpssn->priv_ptr = p->pkth->priv_ptr;
#endif
    /*
     * handle data in the segment
     */
    if(p->dsize)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "   %s state: %s(%d) getting data\n", l,
                    state_names[listener->s_mgr.state],
                    listener->s_mgr.state););

        // FIN means only that sender is done talking,
        // other side may continue yapping.
        if(TCP_STATE_FIN_WAIT_2 == talker->s_mgr.state ||
                TCP_STATE_TIME_WAIT == talker->s_mgr.state)
        {
            /* data on a segment when we're not accepting data any more */
            /* alert! */
            //EventDataOnClosed(talker->tcp_policy);
            eventcode |= EVENT_DATA_ON_CLOSED;
            retcode |= ACTION_BAD_PKT;
            NormalDropPacket(p);
        }
        else if (TCP_STATE_CLOSED == talker->s_mgr.state)
        {
            /* data on a segment when we're not accepting data any more */
            /* alert! */
            if (scb->ha_state.session_flags & SSNFLAG_RESET)
            {
                // no need to run detection PPs on this packet
                DisableDetect( p );

                //EventDataAfterReset(listener->tcp_policy);
                if ( talker->s_mgr.sub_state & SUB_RST_SENT )
                    eventcode |= EVENT_DATA_AFTER_RESET;
                else
                    eventcode |= EVENT_DATA_AFTER_RST_RCVD;
            }
            else
            {
                //EventDataOnClosed(listener->tcp_policy);
                eventcode |= EVENT_DATA_ON_CLOSED;
            }
            retcode |= ACTION_BAD_PKT;
            NormalDropPacket(p);
        }
        else if ((TCP_STATE_FIN_WAIT_1 == talker->s_mgr.state) &&
                 SEQ_GEQ(tdb->seq, (listener->s_mgr.transition_seq - 1)))
        {   
            /* Alert! : Data on a segment when we're not accepting data any more */
            eventcode |= EVENT_DATA_ON_CLOSED;
            retcode |= ACTION_BAD_PKT;
            NormalDropPacket(p);
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Queuing data on listener, t %s, l %s...\n",
                        flush_policy_names[talker->flush_mgr.flush_policy],
                        flush_policy_names[listener->flush_mgr.flush_policy]););

#ifdef NORMALIZER
            if( s5TcpPolicy->policy != STREAM_POLICY_NOACK )
            {
                // these normalizations can't be done if we missed setup. and
                // window is zero in one direction until we've seen both sides.
                // Avoid this normalization for Asymmetric traffic
                if (( !(scb->ha_state.session_flags & SSNFLAG_MIDSTREAM) ) && TwoWayTraffic(scb))
                {
                    // sender of syn w/mss limits payloads from peer
                    // since we store mss on sender side, use listener mss
                    // same reasoning for window size
                    StreamTracker* st = listener;

                    // trim to fit in window and mss as needed
                    NormalTrimPayloadIfWin(p, (st->r_win_base + st->l_window) - st->r_nxt_ack, tdb);
                    if ( st->mss )
                        NormalTrimPayloadIfMss(p, st->mss, tdb);

                    NormalCheckECN(tcpssn, p);
                }
            }
#endif
            /*
             * dunno if this is RFC but fragroute testing expects it
             * for the record, I've seen FTP data sessions that send
             * data packets with no tcp flags set
             */
            if ((p->tcph->th_flags != 0) || (s5TcpPolicy->policy == STREAM_POLICY_LINUX) || (s5TcpPolicy->policy == STREAM_POLICY_NOACK))
            {
                ProcessTcpData(p, listener, tcpssn, tdb, s5TcpPolicy);
                //Check if all segments are received. Process FIN transition
                if(checkFINTransitionStatus(p, listener))
                    process_fin = true;
            }
            else
            {
                eventcode |= EVENT_DATA_WITHOUT_FLAGS;
                NormalDropPacket(p);
            }
        }
    }

    if((p->tcph->th_flags & TH_FIN) || process_fin) //FIN is received or process Queued FIN
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Got an FIN...\n"););
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "   %s state: %s(%d)\n", l,
                    state_names[talker->s_mgr.state],
                    talker->s_mgr.state););

        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "checking ack (0x%X) vs nxt_ack (0x%X)\n",
                    tdb->end_seq, listener->r_win_base););

        if(SEQ_LT(tdb->end_seq,listener->r_win_base))
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "FIN inside r_win_base, bailing\n"););
            goto dupfin;
        }
        else
        {
            //FIN is in order or we need to process FIN from state_queue
            if((tdb->end_seq == listener->r_nxt_ack) ||  process_fin ||
                (talker->s_mgr.state > TCP_STATE_ESTABLISHED) ||
                (listener->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE))
            {
                // need substate since we don't change state immediately
                if ( (talker->s_mgr.state >= TCP_STATE_ESTABLISHED) &&
                        !(talker->s_mgr.sub_state & SUB_FIN_SENT) )
                {
                    talker->l_nxt_seq++;
                    listener->r_nxt_ack++;
                    talker->s_mgr.sub_state |= SUB_FIN_SENT;

#ifdef NORMALIZER
                    if ((listener->flush_mgr.flush_policy != STREAM_FLPOLICY_PROTOCOL) &&
                            (listener->flush_mgr.flush_policy != STREAM_FLPOLICY_PROTOCOL_IPS) &&
                            (listener->flush_mgr.flush_policy != STREAM_FLPOLICY_PROTOCOL_NOACK) &&
                            Normalize_GetMode(snort_conf, NORM_TCP_IPS) == NORM_MODE_ON)
                    {
                        p->packet_flags |= PKT_PDU_TAIL;
                    }
#endif
                }
                switch(talker->s_mgr.state)
                {
                    case TCP_STATE_SYN_RCVD:
                    case TCP_STATE_ESTABLISHED:
                        if (talker->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT)
                        {
                            talker->s_mgr.state_queue = TCP_STATE_CLOSING;
                        }
                        talker->s_mgr.state = TCP_STATE_FIN_WAIT_1;
                        EndOfFileHandle(p, (TcpSession *) scb->proto_specific_data->data);
#ifdef NORMALIZER
                        if ( !p->dsize )
                            CheckFlushPolicyOnData( ( ( StreamConfig * ) scb->stream_config )->tcp_config,
                                    tcpssn, talker, listener, tdb, p);
#endif
                        StreamUpdatePerfBaseState(&sfBase, scb, TCP_STATE_CLOSING);
                        break;

                    case TCP_STATE_CLOSE_WAIT:
                        talker->s_mgr.state = TCP_STATE_LAST_ACK;
                        break;

                    case TCP_STATE_FIN_WAIT_1:
                        if (!p->dsize)
                            RetransmitHandle(p, tcpssn);
                        break;

                    default:
                        /* all other states stay where they are */
                        break;
                }

                if (!process_fin && ((talker->s_mgr.state == TCP_STATE_FIN_WAIT_1) ||
                        (talker->s_mgr.state == TCP_STATE_LAST_ACK)))
                {
                    uint32_t end_seq = ( scb->ha_state.session_flags & SSNFLAG_MIDSTREAM ) ?
                        tdb->end_seq-1 : tdb->end_seq;

                    if ( (listener->s_mgr.expected_flags == TH_ACK) &&
                            SEQ_GEQ(end_seq, listener->s_mgr.transition_seq) )
                    {
                        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "FIN beyond previous, ignoring\n"););
                        eventcode |= EVENT_BAD_FIN;
                        LogTcpEvents(talker->tcp_policy, eventcode);
                        NormalDropPacket(p);
                        PREPROC_PROFILE_END(s5TcpStatePerfStats);
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
                        tcpssn->priv_ptr = NULL;
#endif
                        return retcode | ACTION_BAD_PKT;
                    }
                }

                if(!process_fin)
                {
                    switch ( listener->s_mgr.state )
                    {
                        case TCP_STATE_ESTABLISHED:
                            listener->s_mgr.state_queue = TCP_STATE_CLOSE_WAIT;
                            listener->s_mgr.transition_seq = tdb->end_seq + 1;
                            listener->s_mgr.expected_flags = TH_ACK;
                            break;

                        case TCP_STATE_FIN_WAIT_1:
                            listener->s_mgr.state_queue = TCP_STATE_CLOSING;
                            listener->s_mgr.transition_seq = tdb->end_seq + 1;
                            listener->s_mgr.expected_flags = TH_ACK;
                            break;

                        case TCP_STATE_FIN_WAIT_2:
                            listener->s_mgr.state_queue = TCP_STATE_TIME_WAIT;
                            listener->s_mgr.transition_seq = tdb->end_seq + 1;
                            listener->s_mgr.expected_flags = TH_ACK;
                            break;

                        case TCP_STATE_CLOSED:
                            listener->s_mgr.transition_seq = tdb->end_seq + 1;
                            break; 
                    }
                }
            }
            else
            {
                //OOO FIN received
                if((listener->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT) && SEQ_LT(tdb->end_seq,listener->s_mgr.transition_seq))
                {
                    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "FIN inside transition_seq, bailing\n"););
                    goto dupfin;
                }
                if ((listener->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT) ||
                        (talker->s_mgr.state == TCP_STATE_FIN_WAIT_1) ||
                            (talker->s_mgr.state == TCP_STATE_LAST_ACK))
                {
                    uint32_t end_seq = ( scb->ha_state.session_flags & SSNFLAG_MIDSTREAM ) ?
                        tdb->end_seq-1 : tdb->end_seq;

                    if ( (listener->s_mgr.expected_flags == TH_ACK) &&
                            SEQ_GEQ(end_seq, listener->s_mgr.transition_seq) )
                    {
                        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "FIN beyond previous, ignoring\n"););
                        eventcode |= EVENT_BAD_FIN;
                        LogTcpEvents(talker->tcp_policy, eventcode);
                        NormalDropPacket(p);
                        PREPROC_PROFILE_END(s5TcpStatePerfStats);
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
                        tcpssn->priv_ptr = NULL;
#endif
                        return retcode | ACTION_BAD_PKT;
                    }
                }
                switch ( listener->s_mgr.state )
                {
                    case TCP_STATE_ESTABLISHED:
                        listener->s_mgr.state_queue = TCP_STATE_CLOSE_WAIT;
                        listener->s_mgr.transition_seq = tdb->end_seq + 1;
                        listener->s_mgr.expected_flags = TH_ACK;
                        break;

                    case TCP_STATE_CLOSED:
                        listener->s_mgr.transition_seq = tdb->end_seq + 1;
                        break;
                }
            }
        }
    }

dupfin:

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "   %s [talker] state: %s\n", t,
                state_names[talker->s_mgr.state]););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "   %s state: %s(%d)\n", l,
                state_names[listener->s_mgr.state],
                listener->s_mgr.state););

    /*
     * handle TIME_WAIT timer stuff
     */

    /* Handle Asymmetric connection closing, we will see only one
     * direction pkts. FIN_ACK handling.
     * Do not set state to closed prematurely when handling retry packets
     * to avoid retry loop - CSCvc08844.
     */
    if((!TwoWayTraffic(scb) &&
#ifdef DAQ_PKT_FLAG_RETRY_PACKET
        !(p->pkth->flags & DAQ_PKT_FLAG_RETRY_PACKET) &&
#endif
        (talker->s_mgr.state >= TCP_STATE_FIN_WAIT_1 || listener->s_mgr.state >= TCP_STATE_FIN_WAIT_1)))
    {
        if (TCP_ISFLAGSET(p->tcph, (TH_FIN|TH_ACK)))
        {
            if(talker->s_mgr.state >= TCP_STATE_FIN_WAIT_1)
                talker->s_mgr.state = TCP_STATE_CLOSED;
            if(listener->s_mgr.state >= TCP_STATE_FIN_WAIT_1)
                listener->s_mgr.state = TCP_STATE_CLOSED;
            listener->flags |= TF_FORCE_FLUSH;
        }
    }

    if((talker->s_mgr.state == TCP_STATE_TIME_WAIT && listener->s_mgr.state == TCP_STATE_CLOSED) ||
            (listener->s_mgr.state == TCP_STATE_TIME_WAIT && talker->s_mgr.state == TCP_STATE_CLOSED) ||
            (listener->s_mgr.state == TCP_STATE_TIME_WAIT && talker->s_mgr.state == TCP_STATE_TIME_WAIT)||
            (!TwoWayTraffic(scb)&& (talker->s_mgr.state == TCP_STATE_CLOSED || listener->s_mgr.state == TCP_STATE_CLOSED)))
    {
        //dropssn:
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Session terminating, flushing session buffers\n"););

#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
        tcpssn->priv_ptr = NULL;
#endif
        if ( p->tcph->th_flags & TH_ACK )
            CheckFlushPolicyOnAck( ( ( StreamConfig * ) scb->stream_config )->tcp_config,
                       tcpssn, talker, listener, tdb, p);

        if(p->packet_flags & PKT_FROM_SERVER)
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "flushing FROM_SERVER\n"););
            if(talker->seg_bytes_logical)
            {
                uint32_t flushed = flush_stream(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport,
                        PKT_FROM_CLIENT);

                if(flushed)
                {
                    // FIXTHIS - these calls redundant?
                    purge_alerts(talker, talker->r_win_base, (void *)tcpssn->scb);
                    purge_to_seq(tcpssn, talker, talker->seglist->seq + flushed);
                }
            }

            if(listener->seg_bytes_logical)
            {
                uint32_t flushed = flush_stream(tcpssn, listener, p,
                        GET_SRC_IP(p), GET_DST_IP(p),
                        p->tcph->th_sport, p->tcph->th_dport,
                        PKT_FROM_SERVER);

                if(flushed)
                {
                    purge_alerts(listener, listener->r_win_base, (void *)tcpssn->scb);
                    purge_to_seq(tcpssn, listener, listener->seglist->seq + flushed);
                }
            }
        }
        else
        {
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "flushing FROM_CLIENT\n"););
            if(listener->seg_bytes_logical)
            {
                uint32_t flushed = flush_stream(tcpssn, listener, p,
                        GET_SRC_IP(p), GET_DST_IP(p),
                        p->tcph->th_sport, p->tcph->th_dport,
                        PKT_FROM_CLIENT);

                if(flushed)
                {
                    purge_alerts(listener, listener->r_win_base, (void *)tcpssn->scb);
                    purge_to_seq(tcpssn, listener, listener->seglist->seq + flushed);
                }
            }

            if(talker->seg_bytes_logical)
            {
                uint32_t flushed = flush_stream(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport,
                        PKT_FROM_SERVER);

                if(flushed)
                {
                    purge_alerts(talker, talker->r_win_base,(void *)tcpssn->scb);
                    purge_to_seq(tcpssn, talker, talker->seglist->seq + flushed);
                }
            }
        }
        LogTcpEvents(listener->tcp_policy, eventcode);
        /* The last ACK is a part of the session.  Delete the session after processing is complete. */
        TcpSessionCleanup(scb, 0);
        scb->session_state |= STREAM_STATE_CLOSED;
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return retcode | ACTION_LWSSN_CLOSED;
    }
    else if(listener->s_mgr.state == TCP_STATE_CLOSED && talker->s_mgr.state == TCP_STATE_SYN_SENT)
    {
        if(p->tcph->th_flags & TH_SYN &&
                !(p->tcph->th_flags & TH_ACK) &&
                !(p->tcph->th_flags & TH_RST))
        {
          session_api->set_expire_timer(p, scb, s5TcpPolicy->session_timeout);
        }
    }

    if((listener->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_NOACK)
            || (listener->flush_mgr.flush_policy == STREAM_FLPOLICY_PROTOCOL_NOACK))
    {
        uint32_t flushed = 0;
        flushed = CheckFlushPolicyOnData( ( ( StreamConfig * ) scb->stream_config )->tcp_config,
                                tcpssn, talker, listener, tdb, p);
        if (flushed)
        {
             if(listener->xtradata_mask && extra_data_log)
                  purge_alerts(listener, listener->seglist->seq + flushed, (void *)tcpssn->scb);
        }
    }
#ifdef NORMALIZER
    else if ( p->dsize > 0 )
    {
        /* in case of Asymmetric traffic, enfore flush_policy from STREAM_FLPOLICY_FOOTPRINT to STREAM_FLPOLICY_FOOTPRINT_IPS
         * This will be applied on passive mode sessions, which has policy mode as STREAM_FLPOLICY_FOOTPRINT only
         */
        uint32_t flushed = 0;
        if (!TwoWayTraffic(scb) &&
                (scb->session_state & ( STREAM_STATE_SYN | STREAM_STATE_SYN_ACK)) &&
                !TCP_ISFLAGSET(p->tcph, TH_SYN))
        {
            if(listener->flush_mgr.flush_policy == STREAM_FLPOLICY_PROTOCOL)
                listener->flush_mgr.flush_policy = STREAM_FLPOLICY_PROTOCOL_IPS;
            else if(listener->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT)
                listener->flush_mgr.flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS;
        }

        flushed = CheckFlushPolicyOnData( ( ( StreamConfig * ) scb->stream_config )->tcp_config,
                                tcpssn, talker, listener, tdb, p);
        if (!TwoWayTraffic(scb) && flushed &&
                (scb->session_state & ( STREAM_STATE_SYN | STREAM_STATE_SYN_ACK)) &&
                !TCP_ISFLAGSET(p->tcph, TH_SYN))
        {
            purge_to_seq(tcpssn, listener, listener->seglist->seq + flushed);
        }
        else if(flushed)
        {
            if(listener->xtradata_mask && extra_data_log)
                 purge_alerts(listener, listener->seglist->seq + flushed, (void *)tcpssn->scb);
        }
    }
#endif

    if ( p->tcph->th_flags & TH_ACK )
        CheckFlushPolicyOnAck( ( ( StreamConfig * ) scb->stream_config )->tcp_config,
                               tcpssn, talker, listener, tdb, p);

    LogTcpEvents(listener->tcp_policy, eventcode);
    PREPROC_PROFILE_END(s5TcpStatePerfStats);
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    tcpssn->priv_ptr = NULL;
#endif
    return retcode;
}

// this is for post-ack flushing
static inline uint32_t GetReverseDir (const Packet* p)
{
    /* Remember, one side's packets are stored in the
     * other side's queue.  So when talker ACKs data,
     * we need to check if we're ready to flush.
     *
     * If we do decide to flush, the flush IP & port info
     * is the opposite of the packet -- again because this
     * is the ACK from the talker and we're flushing packets
     * that actually came from the listener.
     */
    if ( p->packet_flags & PKT_FROM_SERVER )
        return PKT_FROM_CLIENT;

    else if ( p->packet_flags & PKT_FROM_CLIENT )
        return PKT_FROM_SERVER;

    return 0;
}

#ifdef NORMALIZER
static inline uint32_t GetForwardDir (const Packet* p)
{
    if ( p->packet_flags & PKT_FROM_SERVER )
        return PKT_FROM_SERVER;

    else if ( p->packet_flags & PKT_FROM_CLIENT )
        return PKT_FROM_CLIENT;

    return 0;
}

static inline int CheckFlushCoercion (
        Packet* p, FlushMgr* fm, uint16_t flush_factor
        ) {
    if ( !flush_factor )
        return 0;

    if (
            p->dsize &&
            (p->dsize < fm->last_size) &&
            (fm->last_count >= flush_factor) )
    {
        fm->last_size = 0;
        fm->last_count = 0;
        return 1;
    }
    if ( p->dsize > fm->last_size )
        fm->last_size = p->dsize;

    fm->last_count++;
    return 0;
}

static inline int CheckFTPFlushCoercion (Packet* p, FlushMgr* fm)
{
#ifdef HAVE_DAQ_DECRYPTED_SSL
    if (p->pkth->flags & DAQ_PKT_FLAG_DECRYPTED_SSL)
    {
      if (fm->flush)
        return 1;
      else
        return 0;
    }
#endif
    if( p->dsize && p->dsize != fm->last_size )
    {
        fm->last_size = p->dsize;
        return 1;
    }
    return 0;
}
#endif

static inline bool AutoDisable (StreamTracker* a, StreamTracker* b)
{
    if ( !a->flush_mgr.auto_disable )
        return false;

    a->flush_mgr.flush_policy = STREAM_FLPOLICY_IGNORE;
    purge_all(a);

    if ( b->flush_mgr.auto_disable )
    {
        b->flush_mgr.flush_policy = STREAM_FLPOLICY_IGNORE;
        purge_all(b);
    }
    return true;
}

#ifdef NORMALIZER
/*
 * In case of Pre-ACK mode, check if we need to
 * block packet so that the payload deos not reach endpoint without
 * inspection.
 */
static inline bool check_to_hold_packet(Packet *pkt, PAF_Status paf_state,
            StreamTracker *trk)
{
    if (((paf_state == PAF_PSEUDO_FLUSH_SEARCH) ||
         (paf_state == PAF_PSEUDO_FLUSH_SKIP)) &&
        !(pkt->packet_flags & PKT_PSEUDO_FLUSH))
        {
            if(trk->held_segment != NULL)
                return FALSE;

            return TRUE;
        }
        return FALSE;
}

static inline void hold_packet(Packet *pkt, StreamTracker *trk,
     StreamSegment *seg)
{
    trk->held_segment = seg;
    pkt->packet_flags |= PKT_FAST_BLOCK;
    Active_DropPacket(pkt);
}

// see flush_pdu_ackd() for details
// the key difference is that we operate on forward moving data
// because we don't wait until it is acknowledged
static inline uint32_t flush_pdu_ips ( StreamTcpConfig *config, TcpSession *ssn,
                                       StreamTracker *trk, Packet *pkt, uint64_t *flags )
{
    bool to_srv = ( *flags == PKT_FROM_CLIENT );
    uint16_t srv_port = ( to_srv ? pkt->dp : pkt->sp );
    uint32_t total = 0, avail;
    StreamSegment* seg;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpPAFPerfStats);
    avail = get_q_sequenced(trk);
    seg = trk->seglist_next;

    // * must stop if gap (checked in s5_paf_check)
    while ( seg && *flags && (total < avail) )
    {
        uint32_t flush_pt;
        uint32_t size = seg->size;
        uint32_t end = seg->seq + seg->size;
        uint32_t pos = s5_paf_position(&trk->paf_state);
	
        total += size;

        if ( s5_paf_initialized(&trk->paf_state) && SEQ_LEQ(end, pos) )
        {
            if (!seg->next && pkt->packet_flags & PKT_PSEUDO_FLUSH)
            {
                *flags |= PKT_PSEUDO_FLUSH;
            } else {
                seg = seg->next;
                continue;
            }
        }
        //Used by h2_paf
        wire_packet = pkt;
        flush_policy_for_dir = trk->flush_mgr.flush_policy;
        flush_pt = s5_paf_check( config->paf_config, &trk->paf_state, ssn->scb,
                                 seg->payload, size, total, seg->seq, srv_port,
                                 flags, trk->flush_mgr.flush_pt);

        if (check_to_hold_packet(pkt, trk->paf_state.paf, trk))
        {
                hold_packet(pkt, trk, seg);
        }

        if (*flags & PKT_PURGE)
        {     
            //For HTTP/2 case where stream doesnt flush the data
            //Set the seglist value to SL_BUF_FLUSHED
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "flush_pdu_ips and purge\n"););
            seg->buffered = SL_BUF_FLUSHED;
            return 0;

        }
        if ( flush_pt > 0 )
        {
            PREPROC_PROFILE_END(s5TcpPAFPerfStats);
            return flush_pt;
        }
        seg = seg->next;
    }

    PREPROC_PROFILE_END(s5TcpPAFPerfStats);
    return 0;
}

static inline int CheckFlushPolicyOnData( StreamTcpConfig *config, TcpSession *tcpssn,
                                          StreamTracker *talker, StreamTracker *listener,
                                          TcpDataBlock *tdb, Packet *p)
{
    uint32_t flushed = 0;
    uint32_t avail;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In CheckFlushPolicyOnData\n"););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Talker flush policy: %s\n",
                flush_policy_names[talker->flush_mgr.flush_policy]););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Listener flush policy: %s\n",
                flush_policy_names[listener->flush_mgr.flush_policy]););

    switch(listener->flush_mgr.flush_policy)
    {
        case STREAM_FLPOLICY_IGNORE:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "STREAM_FLPOLICY_IGNORE\n"););
            return 0;

        case STREAM_FLPOLICY_FOOTPRINT_IPS_FTP:
            {
                int coerce = 0;
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "STREAM_FLPOLICY_FOOTPRINT-IPS_FTP\n"););

#ifdef NORMALIZER
                if ( Normalize_GetMode(snort_conf, NORM_TCP_IPS) == NORM_MODE_ON )
                {
#ifdef DAQ_PKT_FLAG_SSL_DETECTED
                    //Do not do FTP-Normalization for FTPS traffic
                    if((p->pkth->flags & DAQ_PKT_FLAG_SSL_DETECTED) || (p->pkth->flags & DAQ_PKT_FLAG_SSL_SHELLO))
                    {
                        tcpssn->pp_flags |= PP_FTPTELNET_FTPS;
                        listener->flush_mgr.last_size = 0;
                    }
#endif
                    if(!(tcpssn->pp_flags & PP_FTPTELNET_FTPS))
                    {
                        if(!listener->flush_mgr.last_size)
                        {
                            //Rely on mss if it exists, else use snaplen
                            if((tcpssn->client.mss > 0) && (tcpssn->server.mss > 0))
                            {
                                uint32_t ftp_data_conn_mss = (tcpssn->client.mss < tcpssn->server.mss) ? tcpssn->client.mss : tcpssn->server.mss;
                                listener->flush_mgr.last_size = ftp_data_conn_mss - p->tcp_options_len;
                            }
                            else
                                listener->flush_mgr.last_size = pkt_snaplen - (p->data - p->pkt);
                        }
                        coerce = CheckFTPFlushCoercion(p, &listener->flush_mgr);
                        if(coerce)
                            CallFTPFlushProcessor(p);
                        listener->flush_mgr.flush = false;
                        talker->flush_mgr.flush = false;
                    }
                    if(!coerce)
                    {
                        if(tcpssn->pp_flags & PP_FTPTELNET_FTPS)
                            coerce = CheckFlushCoercion(p, &listener->flush_mgr, listener->tcp_policy->flush_factor);
                        avail = get_q_sequenced(listener);

                        if (
                                (avail > 0) &&
                                (coerce || (avail >= listener->flush_mgr.flush_pt) ||
                                 (avail && talker->s_mgr.state == TCP_STATE_FIN_WAIT_1))
                           ) {
                            uint32_t dir = GetForwardDir(p);

                            if ( talker->s_mgr.state == TCP_STATE_FIN_WAIT_1 )
                                listener->flags |= TF_FORCE_FLUSH;

                            flushed = flush_to_seq(
                                    tcpssn, listener, avail, p,
                                    GET_SRC_IP(p), GET_DST_IP(p),
                                    p->tcph->th_sport, p->tcph->th_dport, dir);
                        }
                    }
                }
#endif
            }
            break;

        case STREAM_FLPOLICY_FOOTPRINT_IPS:
            {
                int coerce;
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "STREAM_FLPOLICY_FOOTPRINT-IPS\n"););

                avail = get_q_sequenced(listener);
                coerce = CheckFlushCoercion(
                        p, &listener->flush_mgr, listener->tcp_policy->flush_factor);

                if (
                        (avail > 0) &&
                        (coerce || (avail >= listener->flush_mgr.flush_pt) ||
                         (avail && talker->s_mgr.state == TCP_STATE_FIN_WAIT_1))
                   ) {
                    uint32_t dir = GetForwardDir(p);

                    if ( talker->s_mgr.state == TCP_STATE_FIN_WAIT_1 )
                        listener->flags |= TF_FORCE_FLUSH;

                    flushed = flush_to_seq(
                            tcpssn, listener, avail, p,
                            GET_SRC_IP(p), GET_DST_IP(p),
                            p->tcph->th_sport, p->tcph->th_dport, dir);
                }
            }
            break;

        case STREAM_FLPOLICY_FOOTPRINT_NOACK:
            {
                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "STREAM_FLPOLICY_FOOTPRINT-NOACK\n"););

                avail = get_q_sequenced(listener);
                CheckFlushCoercion(
                        p, &listener->flush_mgr, listener->tcp_policy->flush_factor);

                if ( avail > 0)
                {
                    uint32_t dir = GetForwardDir(p);

                    if ( talker->s_mgr.state == TCP_STATE_FIN_WAIT_1 )
                        listener->flags |= TF_FORCE_FLUSH;

                    flushed = flush_to_seq_noack(
                            tcpssn, listener, avail, p,
                            GET_SRC_IP(p), GET_DST_IP(p),
                            p->tcph->th_sport, p->tcph->th_dport, dir);
                }
            }
            break;

        case STREAM_FLPOLICY_PROTOCOL_IPS:
        case STREAM_FLPOLICY_PROTOCOL_NOACK:
            {
                uint64_t flags = GetForwardDir(p);
                uint32_t flush_amt = flush_pdu_ips( config, tcpssn, listener, p, &flags);
                uint32_t this_flush;

                if (flush_amt == 0 && (flags & PKT_PURGE))
                {
                    listener->seglist_next->buffered = SL_BUF_FLUSHED;
                    break;
                }
                while ( flush_amt > 0 )
                {
                    if( flags & PKT_IGNORE )
                    {
                        StreamSegment *curseg = listener->seglist_next;
                        uint32_t size_to_flush = 0;
                        //set these ignored segments to FLUSHED
                        //so that these gets purged on ack
                        while ( curseg )
                        {
                            if( !curseg->buffered )
                            {
                                size_to_flush += curseg->size;
                                if( size_to_flush == flush_amt )
                                {
                                    curseg->buffered = SL_BUF_FLUSHED;
                                    break;
                                }
                                else if( size_to_flush > flush_amt )
                                {
                                    unsigned int  bytes_to_copy = curseg->size - (  size_to_flush - flush_amt);
                                    StreamSegment *newseg = NULL;

                                    if ( DupStreamNode(NULL, listener, curseg, &newseg) == STREAM_INSERT_OK )
                                    {
                                        curseg->size = bytes_to_copy;
                                        newseg->seq += bytes_to_copy;
                                        newseg->size -= bytes_to_copy;
                                        newseg->payload += bytes_to_copy + (curseg->payload - curseg->data);
                                        curseg->buffered = SL_BUF_FLUSHED;
                                    }
                                    break;
                                }
                                curseg->buffered = SL_BUF_FLUSHED;
                            }
                            curseg = curseg->next;
                        }

                        this_flush = flush_amt;
                        flags &= ~PKT_IGNORE;
                    }
                    // if this payload is exactly one pdu, don't
                    // actually flush, just use the raw packet

                    /*
                     * For pseudo flush case, fall back to the regular
                     * flush routine which takes care of handling
                     * pseudo flush case.
                     */
                    else if (!(p->packet_flags & PKT_PSEUDO_FLUSH) &&
                         listener->seglist_next &&
                        (tdb->seq == listener->seglist_next->seq) &&
                        (flush_amt == listener->seglist_next->size) &&
                        (flush_amt == p->dsize) )
                    {
                        this_flush = flush_amt;
                        if(!listener->paf_state.fpt_eoh){
                            listener->seglist_next->buffered = SL_BUF_FLUSHED;
                            listener->flush_count++;
                        }
                        p->packet_flags |=  flags & PKT_PDU_FULL;
                        /* Raw packet with only and complete http-post header is set with PKT_EVAL_DROP, 
                         * it will be used to evalute drop/allow packet by preprocs  
                         */
                        if (listener->paf_state.fpt_eoh) 
                        {
                            p->packet_flags |= PKT_EVAL_DROP;
                        }
                        ShowRebuiltPacket(p);
                    }
                    else
                    {
                        this_flush = flush_to_seq(
                                tcpssn, listener, flush_amt, p,
                                GET_SRC_IP(p), GET_DST_IP(p),
                                p->tcph->th_sport, p->tcph->th_dport, flags);
                    }
                    // if we didn't flush as expected, bail
                    // (we can flush less than max dsize)
                    if ( !this_flush )
                        break;

                    if(listener->paf_state.fpt_eoh)
                    {
                        listener->paf_state.fpt_eoh = 0;
                        tcpssn->pp_flags &= ~(PP_HTTPINSPECT_PAF_FLUSH_POST_HDR);
                    }
                    else
                        flushed += this_flush;
                    flags = GetForwardDir(p);
                    flush_amt = flush_pdu_ips( config, tcpssn, listener, p, &flags);
                }
                if ( !flags )
                {
                    if ( AutoDisable(listener, talker) )
                        return 0;

                    if ( listener->flush_mgr.flush_policy == STREAM_FLPOLICY_PROTOCOL_NOACK )
                        listener->flush_mgr.flush_policy = STREAM_FLPOLICY_FOOTPRINT_NOACK;
                    else
                        listener->flush_mgr.flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS;
                    listener->flush_mgr.flush_pt += ScPafMax();
                    listener->flush_mgr.flush_type = STREAM_FT_PAF_MAX;

                    return CheckFlushPolicyOnData( config, tcpssn, talker, listener, tdb, p);
                }
            }
            break;
    }
    listener->paf_state.fpt_eoh = 0;
    tcpssn->pp_flags &= ~(PP_HTTPINSPECT_PAF_FLUSH_POST_HDR);
    return flushed;
}
#endif

// iterate over seglist and scan all new acked bytes
// - new means not yet scanned
// - must use seglist data (not packet) since this packet may plug a
//   hole and enable paf scanning of following segments
// - if we reach a flush point
//   - return bytes to flush if data available (must be acked)
//   - return zero if not yet received or received but not acked
// - if we reach a skip point
//   - jump ahead and resume scanning any available data
// - must stop if we reach a gap
// - one segment may lead to multiple checks since
//   it may contain multiple encapsulated PDUs
// - if we partially scan a segment we must save state so we
//   know where we left off and can resume scanning the remainder

static inline uint32_t flush_pdu_ackd ( StreamTcpConfig *config, TcpSession* ssn,
                                        StreamTracker* trk, Packet* pkt, uint64_t* flags)
{
    bool to_srv = ( *flags == PKT_FROM_CLIENT );
    uint16_t srv_port = ( to_srv ? pkt->sp : pkt->dp );
    uint32_t total = 0;
    StreamSegment* seg;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpPAFPerfStats);
    seg = SEQ_LT(trk->seglist_base_seq, trk->r_win_base) ? trk->seglist : NULL;

    // * must stop if not acked
    // * must use adjusted size of seg if not fully acked
    // * must stop if gap (checked in s5_paf_check)
    while ( seg && *flags && SEQ_LT(seg->seq, trk->r_win_base) )
    {
        uint32_t flush_pt;
        uint32_t size = seg->size;
        uint32_t end = seg->seq + seg->size;
        uint32_t pos = s5_paf_position(&trk->paf_state);

        if ( s5_paf_initialized(&trk->paf_state) && SEQ_LEQ(end, pos) )
        {
            total += size;
            seg = seg->next;
            continue;
        }
        if ( SEQ_GT(end, trk->r_win_base) )
            size = trk->r_win_base - seg->seq;

        total += size;

        wire_packet = pkt;
        flush_policy_for_dir = trk->flush_mgr.flush_policy;
        flush_pt = s5_paf_check( config->paf_config, &trk->paf_state, ssn->scb,
                                 seg->payload, size, total, seg->seq, srv_port,
                                 flags, trk->flush_mgr.flush_pt);
        
        if (*flags & PKT_PURGE)
        {
            //For HTTP 2.0 case where stream doesnt flush the data
            //Set the seglist value to SL_BUF_FLUSHED
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                 "Flag PKT_PURGE is set"););
            seg->buffered = SL_BUF_FLUSHED;
            return 0;
        }
        if ( flush_pt > 0 )
        {
            PREPROC_PROFILE_END(s5TcpPAFPerfStats);
            return flush_pt;
        }
        seg = seg->next;
    }

    PREPROC_PROFILE_END(s5TcpPAFPerfStats);
    return 0;
}

static int CheckFlushPolicyOnAck( StreamTcpConfig *config, TcpSession *tcpssn,
                                  StreamTracker *talker, StreamTracker *listener,
                                  TcpDataBlock *tdb, Packet *p)
{
    uint32_t flushed = 0;

    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In CheckFlushPolicyOnAck\n"););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Talker flush policy: %s\n",
                flush_policy_names[talker->flush_mgr.flush_policy]););
    STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Listener flush policy: %s\n",
                flush_policy_names[listener->flush_mgr.flush_policy]););

    switch(talker->flush_mgr.flush_policy)
    {
        case STREAM_FLPOLICY_IGNORE:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "STREAM_FLPOLICY_IGNORE\n"););
            return 0;

        case STREAM_FLPOLICY_FOOTPRINT:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "STREAM_FLPOLICY_FOOTPRINT\n"););
            {
                if(get_q_footprint(talker) >= talker->flush_mgr.flush_pt)
                {
                    uint32_t dir = GetReverseDir(p);

                    flushed = flush_ackd(tcpssn, talker, p,
                            GET_DST_IP(p), GET_SRC_IP(p),
                            p->tcph->th_dport, p->tcph->th_sport, dir);

                    if(flushed)
                        purge_flushed_ackd(tcpssn, talker);
                }
            }
            break;

        case STREAM_FLPOLICY_LOGICAL:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "STREAM_FLPOLICY_LOGICAL\n"););
            if(talker->seg_bytes_logical > talker->flush_mgr.flush_pt)
            {
                uint32_t dir = GetReverseDir(p);

                flushed = flush_ackd(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                if(flushed)
                    purge_flushed_ackd(tcpssn, talker);
            }
            break;

        case STREAM_FLPOLICY_RESPONSE:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Running FLPOLICY_RESPONSE\n"););
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "checking l.r_win_base (0x%X) > "
                        "t.seglist_base_seq (0x%X)\n",
                        talker->r_win_base, talker->seglist_base_seq););

            if(SEQ_GT(talker->r_win_base, talker->seglist_base_seq) &&
                    IsWellFormed(p, talker))
            {
                uint32_t dir = GetReverseDir(p);

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "flushing talker, t->sbl: %d\n",
                            talker->seg_bytes_logical););
                //PrintStreamTracker(talker);
                //PrintStreamTracker(talker);

                flushed = flush_ackd(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "bye bye data...\n"););

                if(flushed)
                    purge_flushed_ackd(tcpssn, talker);
            }
            break;

        case STREAM_FLPOLICY_SLIDING_WINDOW:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "STREAM_FLPOLICY_SLIDING_WINDOW\n"););
            if(get_q_footprint(talker) >= talker->flush_mgr.flush_pt)
            {
                uint32_t dir = GetReverseDir(p);

                flushed = flush_ackd(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Deleting head node for sliding window...\n"););

                /* Base sequence for next window'd flush is the end
                 * of the first packet. */
                talker->seglist_base_seq = talker->seglist->seq + talker->seglist->size;
                StreamSeglistDeleteNode(talker, talker->seglist);

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "setting talker->seglist_base_seq to 0x%X\n",
                            talker->seglist->seq););

            }
            break;

#if 0
        case STREAM_FLPOLICY_CONSUMED:
            STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "STREAM_FLPOLICY_CONSUMED\n"););
            if(get_q_footprint(talker) >= talker->flush_mgr.flush_pt)
            {
                uint32_t dir = GetReverseDir(p);

                flushed = flush_ackd(tcpssn, talker, p,
                        p->iph->ip_dst.s_addr, p->iph->ip_src.s_addr,
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Deleting head node for sliding window...\n"););

                talker->seglist_base_seq = talker->seglist->seq + talker->seglist->size;
                /* TODO: Delete up to the consumed bytes */
                StreamSeglistDeleteNode(talker, talker->seglist);

                STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "setting talker->seglist_base_seq to 0x%X\n",
                            talker->seglist->seq););

            }
            break;
#endif
        case STREAM_FLPOLICY_PROTOCOL:
            {
                uint64_t flags = GetReverseDir(p);
                uint32_t flush_amt = flush_pdu_ackd( config, tcpssn, talker, p, &flags);
                
                while (flush_amt == 0 && (flags & PKT_PURGE))
                {
                    purge_flushed_ackd(tcpssn, talker);
                    // Need to take care of other acked segments
                    flags = GetReverseDir(p);
                    flush_amt = flush_pdu_ackd( config, tcpssn, talker, p, &flags);
                }


                while ( flush_amt > 0 )
                {
                    if( flags & PKT_IGNORE )
                    {
                        flushed = flush_amt;
                        flags &= ~PKT_IGNORE;
                    }
                    else if(talker->paf_state.fpt_eoh)
                    {
                        talker->paf_state.fpt_eoh = 0;
                        tcpssn->pp_flags &= ~(PP_HTTPINSPECT_PAF_FLUSH_POST_HDR);
                        flags = GetReverseDir(p);
                        flush_amt = flush_pdu_ackd( config, tcpssn, talker, p, &flags);
                        continue;
                    }
                    else
                    {
                        talker->seglist_next = talker->seglist;
                        talker->seglist_base_seq = talker->seglist->seq;

                        // for consistency with other cases, should return total
                        // but that breaks flushing pipelined pdus
                        flushed = flush_to_seq(
                                tcpssn, talker, flush_amt, p,
                                GET_DST_IP(p), GET_SRC_IP(p),
                                p->tcph->th_dport, p->tcph->th_sport, flags);
                    }

                    // ideally we would purge just once after this loop
                    // but that throws off base
                    purge_to_seq(tcpssn, talker, talker->seglist->seq + flushed);

                    // if we didn't flush as expected, bail
                    // (we can flush less than max dsize)
                    if ( !flushed )
                        break;

                    flags = GetReverseDir(p);
                    flush_amt = flush_pdu_ackd( config, tcpssn, talker, p, &flags);
                }
                if ( !flags )
                {
                    if ( AutoDisable(talker, listener) )
                        return 0;

                    talker->flush_mgr.flush_policy = STREAM_FLPOLICY_FOOTPRINT;
                    talker->flush_mgr.flush_pt += ScPafMax();
                    talker->flush_mgr.flush_type = STREAM_FT_PAF_MAX;

                    return CheckFlushPolicyOnAck( config, tcpssn, talker, listener, tdb, p );
                }
            }
            break;

#ifdef NORMALIZER
        case STREAM_FLPOLICY_FOOTPRINT_IPS:
        case STREAM_FLPOLICY_FOOTPRINT_IPS_FTP:
        case STREAM_FLPOLICY_PROTOCOL_IPS:
            purge_flushed_ackd(tcpssn, talker);
            break;
#endif
        case STREAM_FLPOLICY_FOOTPRINT_NOACK:
        case STREAM_FLPOLICY_PROTOCOL_NOACK:
            purge_flushed_ackd(tcpssn, talker);
            break;
    }

    return flushed;
}

static void StreamSeglistAddNode(StreamTracker *st, StreamSegment *prev,
        StreamSegment *new)
{
    s5stats.tcp_streamsegs_created++;
    if(prev)
    {
        new->next = prev->next;
        new->prev = prev;
        prev->next = new;
        if (new->next)
            new->next->prev = new;
        else
            st->seglist_tail = new;
    }
    else
    {
        new->next = st->seglist;
        if(new->next)
            new->next->prev = new;
        else
            st->seglist_tail = new;
        st->seglist = new;
    }
    st->seg_count++;
    //Calculate r_nxt_ack by walking the seglist to reach first hole or right end
    if(SEQ_LEQ(new->seq, st->r_nxt_ack))
    {
        while ( new->next && (new->next->seq == new->seq + new->size) )
        {
            new = new->next;
        }
        if(SEQ_LT(st->r_nxt_ack, new->seq + new->size))
          st->r_nxt_ack = new->seq + new->size;
    }
#ifdef DEBUG
    new->ordinal = st->segment_ordinal++;
    if (new->next && (new->next->seq == new->seq))
    {
        LogMessage("Same seq to right, check me\n");
    }
#endif
}

static int StreamSeglistDeleteNode (StreamTracker* st, StreamSegment* seg)
{
    int ret;

    assert(st && seg);

    STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                "Dropping segment at seq %X, len %d\n",
                seg->seq, seg->size););

    if(seg->prev)
        seg->prev->next = seg->next;
    else
        st->seglist = seg->next;

    if(seg->next)
        seg->next->prev = seg->prev;
    else
        st->seglist_tail = seg->prev;

    st->seg_bytes_logical -= seg->size;
    st->seg_bytes_total -= seg->caplen;

    ret = seg->caplen;

    if (seg->buffered)
    {
        s5stats.tcp_rebuilt_seqs_used++;
        st->flush_count--;
    }

    if ( st->seglist_next == seg )
        st->seglist_next = NULL;

    SegmentFree(seg);
    st->seg_count--;

    return ret;
}

static int StreamSeglistDeleteNodeTrim (
        StreamTracker* st, StreamSegment* seg, uint32_t flush_seq)
{
    assert(st && seg);

    /*urgent data is never sent flushed to preprocessors, hence avoid trimming segment having urg_offset >=1*/
    if( (seg->seq + seg->size - seg->urg_offset) > flush_seq )
    {
        /* If PAF is applicable and paf_state is set, we need to trim the segment when
         * we receive ACK for data less than the segment size.
         */
        if( s5_paf_active(&st->paf_state) )
        {
             uint32_t delta = flush_seq - seg->seq;

             if ( delta < seg->size )
             {
                  STREAM_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,
                             "Left-Trimming segment at seq %X, len %d, delta %u\n",
                             seg->seq, seg->size, delta););

                  seg->seq = flush_seq;
                  seg->size -= (uint16_t)delta;
                  seg->payload += delta;
                  st->seg_bytes_logical -= delta;
                  return 0;
             }
        }
        /* If PAF is not applicable return without trim and delete when ACK is received for data
         * less than segment size.
         */
        else
        {
            /* If paf_state is PAF_ABORT and we receive ACK for bytes less than the segment size,
             * do not trim and do not delete from queue till we recive the ACK for remaining bytes.
             * example - FTP Data Session is always in default state which is PAF_ABORT, because PAF
             * is not applicable for FTP DATA, in this case if we receive ACK for data less than the
             * segment size, we should not trim OR delete this node till we get the ACK for complete
             * segment. */
             return 0;
        }
    }
    return StreamSeglistDeleteNode(st, seg);
}

void TcpUpdateDirection(SessionControlBlock *ssn, char dir,
        sfaddr_t* ip, uint16_t port)
{
    TcpSession *tcpssn = (TcpSession *)ssn->proto_specific_data->data;
    sfaddr_t tmpIp;
    uint16_t tmpPort;
    StreamTracker tmpTracker;

    if (IP_EQUALITY(&tcpssn->tcp_client_ip, ip) && (tcpssn->tcp_client_port == port))
    {
        if ((dir == SSN_DIR_FROM_CLIENT) && (ssn->ha_state.direction == SSN_DIR_FROM_CLIENT))
        {
            /* Direction already set as client */
            return;
        }
    }
    else if (IP_EQUALITY(&tcpssn->tcp_server_ip, ip) && (tcpssn->tcp_server_port == port))
    {
        if ((dir == SSN_DIR_FROM_SERVER) && (ssn->ha_state.direction == SSN_DIR_FROM_SERVER))
        {
            /* Direction already set as server */
            return;
        }
    }

    /* Swap them -- leave ssn->ha_state.direction the same */

    /* XXX: Gotta be a more efficient way to do this without the memcpy */
    tmpIp = tcpssn->tcp_client_ip;
    tmpPort = tcpssn->tcp_client_port;
    tcpssn->tcp_client_ip = tcpssn->tcp_server_ip;
    tcpssn->tcp_client_port = tcpssn->tcp_server_port;
    tcpssn->tcp_server_ip = tmpIp;
    tcpssn->tcp_server_port = tmpPort;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
    SwapPacketHeaderFoo(tcpssn);
#endif
    memcpy(&tmpTracker, &tcpssn->client, sizeof(StreamTracker));
    memcpy(&tcpssn->client, &tcpssn->server, sizeof(StreamTracker));
    memcpy(&tcpssn->server, &tmpTracker, sizeof(StreamTracker));

}

/* Iterates through the packets that were reassembled for
 * logging of tagged packets.
 */
int GetTcpRebuiltPackets(Packet *p, SessionControlBlock *ssn, PacketIterator callback, void *userdata)
{
    int packets = 0;
    TcpSession *tcpssn = NULL;
    StreamTracker *st;
    StreamSegment *ss;
    uint32_t start_seq = ntohl(p->tcph->th_seq);
    uint32_t end_seq = start_seq + p->dsize;

    if (!ssn || !ssn->proto_specific_data || !ssn->proto_specific_data->data)
    {
        return packets;
    }

    tcpssn = (TcpSession *)ssn->proto_specific_data->data;

    /* StreamTracker is the opposite of the ip of the reassembled
     * packet --> it came out the queue for the other side */
    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
    {
        st = &tcpssn->server;
    }
    else
    {
        st = &tcpssn->client;
    }

    // skip over segments not covered by this reassembled packet
    for (ss = st->seglist; ss && SEQ_LT(ss->seq, start_seq); ss = ss->next);

    // return flushed segments only
    // For HTTP Post request End-of-Header Flush, if alert is raised, return the segments even though they are not buffered.
    for (; ss && ((ss->buffered == SL_BUF_FLUSHED) || (tcpssn->pp_flags & PP_HTTPINSPECT_PAF_FLUSH_POST_HDR)); ss = ss->next)
    {
        if (SEQ_GEQ(ss->seq,start_seq) && SEQ_LT(ss->seq, end_seq))
        {
            DAQ_PktHdr_t pkth;
            pkth.ts.tv_sec = ss->tv.tv_sec;
            pkth.ts.tv_usec = ss->tv.tv_usec;
            pkth.caplen = ss->caplen;
            pkth.pktlen = ss->pktlen;

            callback(&pkth, ss->pkt, userdata);
            packets++;
        }
        else
            break;
    }

    return packets;
}

/* Iterates through the packets that were reassembled for
 * logging of tagged packets.
 */
int GetTcpStreamSegments(Packet *p, SessionControlBlock *ssn, StreamSegmentIterator callback,
        void *userdata)
{
    int packets = 0;
    TcpSession *tcpssn = NULL;
    StreamTracker *st;
    StreamSegment *ss;
    uint32_t start_seq = ntohl(p->tcph->th_seq);
    uint32_t end_seq = start_seq + p->dsize;

    if (!ssn || !ssn->proto_specific_data || !ssn->proto_specific_data->data)
        return -1;

    tcpssn = (TcpSession *)ssn->proto_specific_data->data;

    /* StreamTracker is the opposite of the ip of the reassembled
     * packet --> it came out the queue for the other side */
    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
        st = &tcpssn->server;
    else
        st = &tcpssn->client;

    // skip over segments not covered by this reassembled packet
    for (ss = st->seglist; ss && SEQ_LT(ss->seq, start_seq); ss = ss->next);

    // return flushed segments only
    // For HTTP Post request End-of-Header Flush, if alert is raised, return the segments even though they are not buffered.
    for (; ss && ((ss->buffered == SL_BUF_FLUSHED) || (tcpssn->pp_flags & PP_HTTPINSPECT_PAF_FLUSH_POST_HDR)); ss = ss->next)
    {
        if (SEQ_GEQ(ss->seq,start_seq) && SEQ_LT(ss->seq, end_seq))
        {
            DAQ_PktHdr_t pkth;
            uint32_t ajust_seq = ss->seq - (uint32_t)(ss->payload - ss->data);
            pkth.ts.tv_sec = ss->tv.tv_sec;
            pkth.ts.tv_usec = ss->tv.tv_usec;
            pkth.caplen = ss->caplen;
            pkth.pktlen = ss->pktlen;

            if (callback(&pkth, ss->pkt, ss->data, ajust_seq, userdata) != 0)
                return -1;

            packets++;
        }
        else
            break;
    }

    return packets;
}

int StreamAddSessionAlertTcp( SessionControlBlock* scb, Packet* p, uint32_t gid, uint32_t sid)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;
    StreamAlertInfo* ai;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

    if (IP_EQUALITY(GET_SRC_IP(p),&tcpssn->tcp_client_ip))
    {
        st = &tcpssn->server;
    }
    else
    {
        st = &tcpssn->client;
    }

    if (st->alert_count >= MAX_SESSION_ALERTS)
        return 0;

    ai = st->alerts + st->alert_count;
    ai->gid = gid;
    ai->sid = sid;
    ai->seq = GET_PKT_SEQ(p);

    if ( p->tcph->th_flags & TH_FIN )
        ai->seq--;

    st->alert_count++;

    return 0;
}

int StreamCheckSessionAlertTcp(SessionControlBlock *scb, Packet *p, uint32_t gid, uint32_t sid)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;
    int i;
    int iRet = 0;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

    /* If this is not a rebuilt packet, no need to check further */
    if (!(p->packet_flags & PKT_REBUILT_STREAM))
    {
        return 0;
    }

    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
    {
        st = &tcpssn->server;
    }
    else
    {
        st = &tcpssn->client;
    }

    for (i=0;i<st->alert_count;i++)
    {
        /*  This is a rebuilt packet and if we've seen this alert before,
         *  return that we have previously alerted on original packet.
         */
        if ( st->alerts[i].gid == gid &&
                st->alerts[i].sid == sid )
        {
            return -1;
        }
    }

    return iRet;
}

int StreamUpdateSessionAlertTcp(SessionControlBlock *scb, Packet *p, uint32_t gid, uint32_t sid,
        uint32_t event_id, uint32_t event_second)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;
    int i;
    uint32_t seq_num;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
    {
        st = &tcpssn->server;
    }
    else
    {
        st = &tcpssn->client;
    }

    seq_num = GET_PKT_SEQ(p);

    if ( p->tcph->th_flags & TH_FIN )
        seq_num--;

    for (i=0;i<st->alert_count;i++)
    {
        StreamAlertInfo* ai = st->alerts + i;

        if ( ai->gid == gid &&
                ai->sid == sid && SEQ_EQ(ai->seq, seq_num))
        {
            ai->event_id = event_id;
            ai->event_second = event_second;
            return 0;
        }
    }

    return -1;
}

void StreamSetExtraDataTcp (SessionControlBlock* scb, Packet* p, uint32_t xid)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return;

    if (IP_EQUALITY(GET_SRC_IP(p),&tcpssn->tcp_client_ip))
        st = &tcpssn->server;
    else
        st = &tcpssn->client;

    st->xtradata_mask |= BIT(xid);
    p->xtradata_mask |= BIT(xid);
}

void StreamClearExtraDataTcp (SessionControlBlock* scb, Packet* p, uint32_t xid)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return;

    if (IP_EQUALITY(GET_SRC_IP(p),&tcpssn->tcp_client_ip))
        st = &tcpssn->server;
    else
        st = &tcpssn->client;

    if ( xid )
        st->xtradata_mask &= ~BIT(xid);
    else
        st->xtradata_mask = 0;
}

char StreamGetReassemblyDirectionTcp(SessionControlBlock *scb)
{
    TcpSession *tcpssn = NULL;
    char dir = SSN_DIR_NONE;

    if (!scb)
        return SSN_DIR_NONE;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return SSN_DIR_NONE;

    if ((tcpssn->server.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE) &&
            (tcpssn->server.flush_mgr.flush_policy != STREAM_FLPOLICY_IGNORE))
    {
        dir |= SSN_DIR_TO_SERVER;
    }

    if ((tcpssn->client.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE) &&
            (tcpssn->client.flush_mgr.flush_policy != STREAM_FLPOLICY_IGNORE))
    {
        dir |= SSN_DIR_TO_CLIENT;
    }

    return dir;
}

Packet* getWirePacketTcp()
{
    return wire_packet;
}

uint8_t getFlushPolicyDirTcp()
{
    if (flush_policy_for_dir == STREAM_FLPOLICY_FOOTPRINT_IPS   || 
        flush_policy_for_dir == STREAM_FLPOLICY_FOOTPRINT_NOACK ||
        flush_policy_for_dir == STREAM_FLPOLICY_PROTOCOL_IPS    ||
        flush_policy_for_dir == STREAM_FLPOLICY_PROTOCOL_NOACK)
    {
        return 1;
    }
    return 0;
}

uint32_t StreamGetFlushPointTcp(SessionControlBlock *scb, char dir)
{
    TcpSession *tcpssn = NULL;

    if (scb == NULL)
        return 0;

    if (scb->proto_specific_data != NULL)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (tcpssn == NULL)
        return 0;

    if (dir & SSN_DIR_TO_SERVER)
        return tcpssn->server.flush_mgr.flush_pt;
    else if (dir & SSN_DIR_TO_CLIENT)
        return tcpssn->client.flush_mgr.flush_pt;

    return 0;
}

void StreamSetFlushPointTcp(SessionControlBlock *scb, char dir, uint32_t flush_point)
{
    TcpSession *tcpssn = NULL;

    if (scb == NULL)
        return;

    if (scb->proto_specific_data != NULL)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (tcpssn == NULL)
        return;

    if (flush_point == 0)
        return;

    if (dir & SSN_DIR_TO_SERVER)
    {
        tcpssn->server.flush_mgr.flush_pt = flush_point;
        tcpssn->server.flush_mgr.last_size = 0;
        tcpssn->server.flush_mgr.last_count = 0;
        tcpssn->server.flush_mgr.flush_type = STREAM_FT_EXTERNAL;
    }
    else if (dir & SSN_DIR_TO_CLIENT)
    {
        tcpssn->client.flush_mgr.flush_pt = flush_point;
        tcpssn->client.flush_mgr.last_size = 0;
        tcpssn->client.flush_mgr.last_count = 0;
        tcpssn->client.flush_mgr.flush_type = STREAM_FT_EXTERNAL;
    }
}

char StreamSetReassemblyTcp(SessionControlBlock *scb, uint8_t flush_policy, char dir, char flags)
{
    TcpSession *tcpssn = NULL;
    uint16_t tmp_flags;

    if (!scb)
        return SSN_DIR_NONE;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return SSN_DIR_NONE;

    tmp_flags = (flush_policy == STREAM_FLPOLICY_IGNORE)? 0: TF_FIRST_PKT_MISSING;
    if (flags & STREAM_FLPOLICY_SET_APPEND)
    {
        if (dir & SSN_DIR_TO_SERVER)
        {
            if (tcpssn->server.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE)
            {
                /* Changing policy with APPEND, Bad */
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Stream: Changing client flush policy using "
                            "append is asking for trouble.  Ignored\n"););
            }
            else
            {
                InitFlushMgr(snort_conf, &tcpssn->server.flush_mgr,
                        &tcpssn->server.tcp_policy->flush_point_list,
                        flush_policy, 0);
                tcpssn->server.flags |= tmp_flags;
            }
        }

        if (dir & SSN_DIR_TO_CLIENT)
        {
            if (tcpssn->client.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE)
            {
                /* Changing policy with APPEND, Bad */
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Stream: Changing server flush policy using "
                            "append is asking for trouble.  Ignored\n"););
            }
            else
            {
                InitFlushMgr(snort_conf, &tcpssn->client.flush_mgr,
                        &tcpssn->client.tcp_policy->flush_point_list,
                        flush_policy, 0);
                tcpssn->client.flags |= tmp_flags;
            }
        }

    }
    else if (flags & STREAM_FLPOLICY_SET_ABSOLUTE)
    {
        if (dir & SSN_DIR_TO_SERVER)
        {
            //If the flush policy is already set to STREAM_FLPOLICY_FOOTPRINT_IPS_FTP, ssl preproc shouldn't reset the flush policy for FTPS.
            //Unset the pp_flags because SSL will send decrypted FTP traffic from here on
            if((tcpssn->pp_flags & PP_FTPTELNET_FTPS) && (tcpssn->server.flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS_FTP))
            {
                flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS_FTP;
                tcpssn->pp_flags &= ~PP_FTPTELNET_FTPS;
            }

            InitFlushMgr(snort_conf, &tcpssn->server.flush_mgr,
                    &tcpssn->server.tcp_policy->flush_point_list,
                    flush_policy, 0);
            tcpssn->server.flags |= tmp_flags;
            if(tcpssn->server.flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS)
            {
                //set_application_data is done for ftp-data connection during TCPSession setup. Change the Flush policy for ftp-data only.
                if(session_api->get_application_data(scb, PP_FTPTELNET))
                {
                    tcpssn->server.flush_mgr.flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS_FTP;
                    tcpssn->server.flush_mgr.last_size = 0;
                }
            }
        }

        if (dir & SSN_DIR_TO_CLIENT)
        {
            //If the flush policy is already set to STREAM_FLPOLICY_FOOTPRINT_IPS_FTP, ssl preproc shouldn't reset the flush policy for FTPS.
            //Unset the pp_flags because SSL will send decrypted FTP traffic from here on
            if((tcpssn->pp_flags & PP_FTPTELNET_FTPS) && (tcpssn->client.flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS_FTP))
            {
                flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS_FTP;
                tcpssn->pp_flags &= ~PP_FTPTELNET_FTPS;
            }

            InitFlushMgr(snort_conf, &tcpssn->client.flush_mgr,
                    &tcpssn->client.tcp_policy->flush_point_list,
                    flush_policy, 0);
            tcpssn->client.flags |= tmp_flags;
            if(tcpssn->client.flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT_IPS)
            {
                //set_application_data is done for ftp-data connection during TCPSession setup. Change the Flush policy for ftp-data only.
                if(session_api->get_application_data(scb, PP_FTPTELNET))
                {
                    tcpssn->client.flush_mgr.flush_policy = STREAM_FLPOLICY_FOOTPRINT_IPS_FTP;
                    tcpssn->client.flush_mgr.last_size = 0;
                }
            }
        }
    }

    return StreamGetReassemblyDirectionTcp(scb);
}

char StreamGetReassemblyFlushPolicyTcp(SessionControlBlock *scb, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!scb)
        return STREAM_FLPOLICY_NONE;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return STREAM_FLPOLICY_NONE;

    if (dir & SSN_DIR_TO_SERVER)
    {
        return (char)tcpssn->server.flush_mgr.flush_policy;
    }

    if (dir & SSN_DIR_TO_CLIENT)
    {
        return (char)tcpssn->client.flush_mgr.flush_policy;
    }
    return STREAM_FLPOLICY_NONE;
}

char StreamIsStreamSequencedTcp(SessionControlBlock *scb, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!scb)
        return 1;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return 1;

    if (dir & SSN_DIR_FROM_CLIENT)
    {
        if ( tcpssn->server.flags & TF_MISSING_PREV_PKT )
            return 0;
    }

    if (dir & SSN_DIR_FROM_SERVER)
    {
        if ( tcpssn->client.flags & TF_MISSING_PREV_PKT )
            return 0;
    }

    return 1;
}

/* This will falsly return SSN_MISSING_BEFORE on the first reassembed
 * packet if reassembly for this direction was set mid-session */
int StreamMissingInReassembledTcp(SessionControlBlock *scb, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!scb)
        return SSN_MISSING_NONE;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return SSN_MISSING_NONE;

    if (dir & SSN_DIR_FROM_CLIENT)
    {
        if (tcpssn->server.flags & TF_MISSING_PREV_PKT)
            return SSN_MISSING_BEFORE;
    }
    else if (dir & SSN_DIR_FROM_SERVER)
    {
        if (tcpssn->client.flags & TF_MISSING_PREV_PKT)
            return SSN_MISSING_BEFORE;
    }

    return SSN_MISSING_NONE;
}

char StreamPacketsMissingTcp(SessionControlBlock *scb, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!scb)
        return 0;

    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (!tcpssn)
        return 0;

    if (dir & SSN_DIR_FROM_CLIENT)
    {
        if (tcpssn->server.flags & TF_PKT_MISSED)
            return 1;
    }

    if (dir & SSN_DIR_FROM_SERVER)
    {
        if (tcpssn->client.flags & TF_PKT_MISSED)
            return 1;
    }

    return 0;
}

#define SSOD_LESS_THAN 1
#define SSOD_GREATER_THAN 2
#define SSOD_EQUALS 3
#define SSOD_LESS_THAN_OR_EQUALS 4
#define SSOD_GREATER_THAN_OR_EQUALS 5
#define SSOD_NOT_EQUALS 6

#define SSOD_MATCH 1
#define SSOD_NOMATCH 0
typedef struct _StreamSizeOptionData
{
    char operator;
    uint32_t size;
    char direction;
} StreamSizeOptionData;

int s5TcpStreamSizeInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr)
{
    char **toks;
    int num_toks;
    char *endp;
    StreamSizeOptionData *ssod = NULL;
    toks = mSplit(parameters, ",", 4, &num_toks, 0);

    if (num_toks != 3)
    {
        FatalError("%s(%d): Invalid parameters for %s option\n",
                file_name, file_line, name);
    }

    ssod = SnortPreprocAlloc(1, sizeof(StreamSizeOptionData), PP_STREAM,
                    PP_MEM_CATEGORY_CONFIG);

    if (!ssod)
    {
        FatalError("%s(%d): Failed to allocate data for %s option\n",
                file_name, file_line, name);
    }

    /* Parse the direction.
     * Can be: client, server, both, either
     */
    if (!strcasecmp(toks[0], "client"))
    {
        ssod->direction = SSN_DIR_FROM_CLIENT;
    }
    else if (!strcasecmp(toks[0], "server"))
    {
        ssod->direction = SSN_DIR_FROM_SERVER;
    }
    else if (!strcasecmp(toks[0], "both"))
    {
        ssod->direction = SSN_DIR_BOTH;
    }
    else if (!strcasecmp(toks[0], "either"))
    {
        ssod->direction = SSN_DIR_NONE;
    }
    else
    {
        FatalError("%s(%d): Invalid direction: %s for option %s\n",
                file_name, file_line, toks[0], name);
    }

    /* Parse the operator.
     * Can be: =, <, > , !=, <=, >=
     */
    if (!strcasecmp(toks[1], "="))
    {
        ssod->operator = SSOD_EQUALS;
    }
    else if (!strcasecmp(toks[1], "<"))
    {
        ssod->operator = SSOD_LESS_THAN;
    }
    else if (!strcasecmp(toks[1], ">"))
    {
        ssod->operator = SSOD_GREATER_THAN;
    }
    else if (!strcasecmp(toks[1], "!="))
    {
        ssod->operator = SSOD_NOT_EQUALS;
    }
    else if (!strcasecmp(toks[1], "<="))
    {
        ssod->operator = SSOD_LESS_THAN_OR_EQUALS;
    }
    else if (!strcasecmp(toks[1], ">="))
    {
        ssod->operator = SSOD_GREATER_THAN_OR_EQUALS;
    }
    else
    {
        FatalError("%s(%d): Invalid operator: %s for option %s\n",
                file_name, file_line, toks[1], name);
    }

    ssod->size = SnortStrtoul(toks[2], &endp, 0);
    if ((endp == toks[2]) || (errno == ERANGE))
    {
        FatalError("%s(%d): Invalid size: %s for option %s\n",
                file_name, file_line, toks[2], name);
    }

    *dataPtr = ssod;
    mSplitFree(&toks, num_toks);

    return 1;
}

static inline int s5TcpStreamSizeCompare(uint32_t size1, uint32_t size2, char operator)
{
    int retval = 0;
    switch (operator)
    {
        case SSOD_EQUALS:
            if (size1 == size2)
                retval = 1;
            break;
        case SSOD_LESS_THAN:
            if (size1 < size2)
                retval = 1;
            break;
        case SSOD_GREATER_THAN:
            if (size1 > size2)
                retval = 1;
            break;
        case SSOD_NOT_EQUALS:
            if (size1 != size2)
                retval = 1;
            break;
        case SSOD_LESS_THAN_OR_EQUALS:
            if (size1 <= size2)
                retval = 1;
            break;
        case SSOD_GREATER_THAN_OR_EQUALS:
            if (size1 >= size2)
                retval = 1;
            break;
        default:
            break;
    }
    return retval;
}

int s5TcpStreamSizeEval(void *p, const uint8_t **cursor, void *dataPtr)
{
    Packet *pkt = p;
    SessionControlBlock *scb = NULL;
    TcpSession *tcpssn = NULL;
    StreamSizeOptionData *ssod = (StreamSizeOptionData *)dataPtr;
    uint32_t client_size;
    uint32_t server_size;
    PROFILE_VARS;

    if (!pkt || !pkt->ssnptr || !ssod || !pkt->tcph)
        return DETECTION_OPTION_NO_MATCH;

    scb = pkt->ssnptr;
    if (!scb->proto_specific_data)
        return DETECTION_OPTION_NO_MATCH;

    PREPROC_PROFILE_START(streamSizePerfStats);

    tcpssn = (TcpSession *)scb->proto_specific_data->data;

    if (tcpssn->client.l_nxt_seq > tcpssn->client.isn)
    {
        /* the normal case... */
        client_size = tcpssn->client.l_nxt_seq - tcpssn->client.isn;
    }
    else
    {
        /* the seq num wrapping case... */
        client_size = tcpssn->client.isn - tcpssn->client.l_nxt_seq;
    }
    if (tcpssn->server.l_nxt_seq > tcpssn->server.isn)
    {
        /* the normal case... */
        server_size = tcpssn->server.l_nxt_seq - tcpssn->server.isn;
    }
    else
    {
        /* the seq num wrapping case... */
        server_size = tcpssn->server.isn - tcpssn->server.l_nxt_seq;
    }

    switch (ssod->direction)
    {
        case SSN_DIR_FROM_CLIENT:
            if (s5TcpStreamSizeCompare(client_size, ssod->size, ssod->operator)
                    == SSOD_MATCH)
            {
                PREPROC_PROFILE_END(streamSizePerfStats);
                return DETECTION_OPTION_MATCH;
            }
            break;
        case SSN_DIR_FROM_SERVER:
            if (s5TcpStreamSizeCompare(server_size, ssod->size, ssod->operator)
                    == SSOD_MATCH)
            {
                PREPROC_PROFILE_END(streamSizePerfStats);
                return DETECTION_OPTION_MATCH;
            }
            break;
        case SSN_DIR_NONE: /* overloaded.  really, its an 'either' */
            if ((s5TcpStreamSizeCompare(client_size, ssod->size, ssod->operator)
                        == SSOD_MATCH) ||
                    (s5TcpStreamSizeCompare(server_size, ssod->size, ssod->operator)
                     == SSOD_MATCH))
            {
                PREPROC_PROFILE_END(streamSizePerfStats);
                return DETECTION_OPTION_MATCH;
            }
            break;
        case SSN_DIR_BOTH:
            if ((s5TcpStreamSizeCompare(client_size, ssod->size, ssod->operator)
                        == SSOD_MATCH) &&
                    (s5TcpStreamSizeCompare(server_size, ssod->size, ssod->operator)
                     == SSOD_MATCH))
            {
                PREPROC_PROFILE_END(streamSizePerfStats);
                return DETECTION_OPTION_MATCH;
            }
            break;
        default:
            break;
    }
    PREPROC_PROFILE_END(streamSizePerfStats);
    return DETECTION_OPTION_NO_MATCH;
}

void s5TcpStreamSizeCleanup(void *dataPtr)
{
    StreamSizeOptionData *ssod = dataPtr;
    if (ssod)
    {
        SnortPreprocFree(ssod, sizeof(StreamSizeOptionData), PP_STREAM,
                         PP_MEM_CATEGORY_CONFIG);
    }
}

typedef struct _StreamReassembleRuleOptionData
{
    char enable;
    char alert;
    char direction;
    char fastpath;
} StreamReassembleRuleOptionData;

int s5TcpStreamReassembleRuleOptionInit(struct _SnortConfig *sc, char *name, char *parameters, void **dataPtr)
{
    char **toks;
    int num_toks;
    StreamReassembleRuleOptionData *srod = NULL;
    toks = mSplit(parameters, ",", 4, &num_toks, 0);

    if (num_toks < 2)
    {
        FatalError("%s(%d): Invalid parameters for %s option\n",
                file_name, file_line, name);
    }

    srod = SnortPreprocAlloc(1, sizeof(StreamReassembleRuleOptionData), PP_STREAM,
                        PP_MEM_CATEGORY_CONFIG);

    if (!srod)
    {
        FatalError("%s(%d): Failed to allocate data for %s option\n",
                file_name, file_line, name);
    }

    /* Parse the action.
     * Can be: enable or disable
     */
    if (!strcasecmp(toks[0], "enable"))
    {
        srod->enable = 1;
    }
    else if (!strcasecmp(toks[0], "disable"))
    {
        srod->enable = 0;
    }
    else
    {
        FatalError("%s(%d): Invalid action: %s for option %s.  Valid "
                "parameters are 'enable' or 'disable'\n",
                file_name, file_line, toks[0], name);
    }

    /* Parse the direction.
     * Can be: client, server, both
     */
    /* Need to these around, so they match the ones specified via the stream5_tcp ports
     * option, ie, stream5_tcp: ports client enables reassembly on client-sourced traffic. */
    if (!strcasecmp(toks[1], "client"))
    {
        srod->direction = SSN_DIR_FROM_SERVER;
    }
    else if (!strcasecmp(toks[1], "server"))
    {
        srod->direction = SSN_DIR_FROM_CLIENT;
    }
    else if (!strcasecmp(toks[1], "both"))
    {
        srod->direction = SSN_DIR_BOTH;
    }
    else
    {
        FatalError("%s(%d): Invalid direction: %s for option %s\n",
                file_name, file_line, toks[1], name);
    }

    /* Parse the optional parameters:
     * noalert flag, fastpath flag
     */
    srod->alert = 1;
    if (num_toks > 2)
    {
        int i = 2;
        for (; i< num_toks; i++)
        {
            if (!strcasecmp(toks[i], "noalert"))
            {
                srod->alert = 0;
            }
            else if (!strcasecmp(toks[i], "fastpath"))
            {
                srod->fastpath = 1;
                if (srod->enable)
                {
                    FatalError("%s(%d): Using 'fastpath' with 'enable' is "
                            "not valid for %s\n", file_name, file_line, name);
                }
            }
            else
            {
                FatalError("%s(%d): Invalid optional parameter: %s for option %s\n",
                        file_name, file_line, toks[i], name);
            }
        }
    }

    *dataPtr = srod;
    mSplitFree(&toks, num_toks);

    return 1;
}

int s5TcpStreamReassembleRuleOptionEval(void *p, const uint8_t **cursor, void *dataPtr)
{
    Packet *pkt = p;
    SessionControlBlock *scb = NULL;
    StreamReassembleRuleOptionData *srod = (StreamReassembleRuleOptionData *)dataPtr;
    PROFILE_VARS;

    if (!pkt || !pkt->ssnptr || !srod || !pkt->tcph)
        return 0;

    PREPROC_PROFILE_START(streamReassembleRuleOptionPerfStats);
    scb = pkt->ssnptr;

    if (!srod->enable) /* Turn it off */
        StreamSetReassemblyTcp(scb, STREAM_FLPOLICY_IGNORE, srod->direction, STREAM_FLPOLICY_SET_ABSOLUTE);
    else
        StreamSetReassemblyTcp(scb, STREAM_FLPOLICY_FOOTPRINT, srod->direction, STREAM_FLPOLICY_SET_ABSOLUTE);

    if (srod->fastpath)
    {
        /* Turn off inspection */
        scb->ha_state.ignore_direction |= srod->direction;
        session_api->disable_inspection(scb, pkt);

        /* TBD: Set TF_FORCE_FLUSH ? */
    }

    if (srod->alert)
    {
        PREPROC_PROFILE_END(streamReassembleRuleOptionPerfStats);
        return DETECTION_OPTION_MATCH;
    }

    PREPROC_PROFILE_END(streamReassembleRuleOptionPerfStats);
    return DETECTION_OPTION_NO_ALERT;
}

void s5TcpStreamReassembleRuleOptionCleanup(void *dataPtr)
{
    StreamReassembleRuleOptionData *srod = dataPtr;
    if (srod)
    {
        SnortPreprocFree(srod, sizeof(StreamReassembleRuleOptionData),
                 PP_STREAM, PP_MEM_CATEGORY_CONFIG);
    }
}

void s5TcpSetPortFilterStatus(struct _SnortConfig *sc, unsigned short port,
        uint16_t status, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->tcp_config != NULL ) )
        config->tcp_config->port_filter[ port ] |= status;
}

void s5TcpUnsetPortFilterStatus( struct _SnortConfig *sc, unsigned short port, uint16_t status,
        tSfPolicyId policyId, int parsing )
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->tcp_config != NULL ) )
        config->tcp_config->port_filter[ port ] &= ~status;
}

int s5TcpGetPortFilterStatus(struct _SnortConfig *sc, unsigned short port, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->tcp_config != NULL ) )
        return ( int ) config->tcp_config->port_filter[ port ];
    else
        return PORT_MONITOR_NONE;
}

void s5TcpSetSynSessionStatus(struct _SnortConfig *sc, uint16_t status, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    if (status <= PORT_MONITOR_SESSION)
        return;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->tcp_config != NULL ) )
        config->tcp_config->session_on_syn |= status;
}

void s5TcpUnsetSynSessionStatus(struct _SnortConfig *sc, uint16_t status, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    if (status <= PORT_MONITOR_SESSION)
        return;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->tcp_config != NULL ) )
        config->tcp_config->session_on_syn &= ~status;
}

static void targetPolicyIterate(void (*callback)(int))
{
    unsigned int i;

    for (i = 0; i < snort_conf->num_policies_allocated; i++)
    {
        if (snort_conf->targeted_policies[i] != NULL)
        {
            callback(i);
        }
    }
}

static void policyDecoderFlagsSaveNClear(int policyId)
{
    SnortPolicy *pPolicy = snort_conf->targeted_policies[policyId];

    if (pPolicy)
    {
        pPolicy->decoder_alert_flags_saved = pPolicy->decoder_alert_flags;
        pPolicy->decoder_drop_flags_saved  = pPolicy->decoder_drop_flags;

        pPolicy->decoder_alert_flags = 0;
        pPolicy->decoder_drop_flags = 0;
    }
}

static void policyDecoderFlagsRestore(int policyId)
{
    SnortPolicy *pPolicy = snort_conf->targeted_policies[policyId];

    if (pPolicy)
    {
        pPolicy->decoder_alert_flags = pPolicy->decoder_alert_flags_saved;
        pPolicy->decoder_drop_flags =  pPolicy->decoder_drop_flags_saved;

        pPolicy->decoder_alert_flags_saved = 0;
        pPolicy->decoder_drop_flags_saved = 0;
    }
}
static void policyChecksumFlagsSaveNClear(int policyId)
{
    SnortPolicy *pPolicy = snort_conf->targeted_policies[policyId];

    if (pPolicy)
    {
        pPolicy->checksum_flags_saved = pPolicy->checksum_flags;
        pPolicy->checksum_flags = 0;
    }
}

static void policyChecksumFlagsRestore(int policyId)
{
    SnortPolicy *pPolicy = snort_conf->targeted_policies[policyId];

    if (pPolicy)
    {
        pPolicy->checksum_flags = pPolicy->checksum_flags_saved;
    }
}

typedef struct network_reassembly_ports
{
    sfcidr_t *ip;
    uint8_t s_ports[MAXPORTS_STORAGE];
    uint8_t  c_ports[MAXPORTS_STORAGE];
} NetworkReassemblyPorts;

// single instance for saving ports registered with default policy...
NetworkReassemblyPorts  default_nrps;

// List head for list of networks and ports within that preprocs have requested for port
// reassembly
sfSDList targeted_nrps;

static void freeNrpNode( void *data )
{
    if( data )
    {
        NetworkReassemblyPorts *nrp = ( NetworkReassemblyPorts * ) data;
        if( nrp->ip != NULL )
            free( nrp->ip );
        SnortPreprocFree( data, sizeof( NetworkReassemblyPorts ),
               PP_STREAM, PP_MEM_CATEGORY_CONFIG );
    }
}

static void StreamCreateReassemblyPortList( void )
{
    //In SnortInit, we call ConfigurePreprocessors twice. The allocations done first need to be freed
    if(targeted_nrps.size > 0)
        StreamDestoryReassemblyPortList();

    sf_sdlist_init( &targeted_nrps, &freeNrpNode );
}

static void StreamDestoryReassemblyPortList( void )
{
    sf_sdlist_purge( &targeted_nrps );
}

static NetworkReassemblyPorts *initNetworkPortReassembly( sfcidr_t *ip, unsigned short port, int ra_dir )
{
    NetworkReassemblyPorts *nrp = SnortPreprocAlloc( 1, sizeof( NetworkReassemblyPorts ),
                                     PP_STREAM, PP_MEM_CATEGORY_CONFIG );

    if( nrp == NULL )
    {
        ErrorMessage( " ERROR: Unable to allocate memory for initializing Network Reassembly Post List\n");
        return NULL;
    }

    nrp->ip = ip;
    if( ra_dir & SSN_DIR_FROM_SERVER )
        enablePort( nrp->s_ports, port );

    if( ra_dir & SSN_DIR_FROM_CLIENT )
        enablePort( nrp->c_ports, port );

    return nrp;
}

void registerPortForReassembly( char *network, uint16_t port, int ra_dir )
{
    SFIP_RET status;
    SDListItem *net_node;

    // if network is NULL then this is a registration for the default policy
    if( network == NULL )
    {
        if( ra_dir & SSN_DIR_FROM_SERVER )
            enablePort( default_nrps.s_ports, port );
        if( ra_dir & SSN_DIR_FROM_CLIENT )
            enablePort( default_nrps.c_ports, port );

        return;
    }
    else
    {
        sfcidr_t *ip = sfip_alloc( network, &status );
        if ( status != SFIP_SUCCESS )
        {
            WarningMessage(
                    "Invalid network ( %s ) in reassembly registration request.", network );
            // free ip struct allocated above...
            if( ip != NULL )
                sfip_free( ip );
            return;
        }

        net_node = sf_sdlist_head( &targeted_nrps );
        while( net_node != NULL )
        {
            SFIP_RET rc;

            NetworkReassemblyPorts *net_data = sf_sdlist_data( net_node );
            rc = sfip_compare( &net_data->ip->addr, &ip->addr );
            if( rc == SFIP_EQUAL )
            {
                // already have this network in list, add the new port...
                if( ra_dir & SSN_DIR_FROM_SERVER )
                    enablePort( net_data->s_ports, port );
                if( ra_dir & SSN_DIR_FROM_CLIENT )
                    enablePort( net_data->c_ports, port );
               break;
            }
            else if( rc == SFIP_GREATER )
            {
                // current network in list is 'greater' than network in registration request,
                // this means request network is not in the list, so we add it as node before
                // the current node...
                // ptr to ip struct allocated above is stored in net data so set to NULL so
                // we don't free it below...
                NetworkReassemblyPorts *new_net = initNetworkPortReassembly( ip, port, ra_dir);
                if( new_net != NULL)
                {
                    sf_sdlist_ins_prev( &targeted_nrps, net_node, new_net );
                    ip = NULL;
                }
                break;
            }
            else
            {
                // current network in list is 'less' than network in registration request...
                // move to next entry
                net_node = sf_sdlist_next( net_node );
            }
        }

        // finished searching list for matching network... if net_node is NULL then we did not
        // find a match AND all networks in the list are 'less than' or more general than the
        // request network...
        if( net_node == NULL )
        {
            // at list end and found no match, add new network to end of list
            // ptr to ip struct allocated above is stored in net data so set to NULL so
            // we don't free it below...
            NetworkReassemblyPorts *new_net = initNetworkPortReassembly( ip, port, ra_dir);
            if( new_net != NULL)
            {
                sf_sdlist_ins_next( &targeted_nrps, sf_sdlist_tail( &targeted_nrps ), new_net );
                ip = NULL;
            }
        }

        // free ip struct allocated above if not still in use...
        if( ip != NULL )
            sfip_free( ip );
    }
}

static void enablePortsForReassembly( NetworkReassemblyPorts *nrp_list, StreamTcpPolicy *policy )
{
    uint32_t port;

    for( port = 0; port < MAXPORTS; port++)
    {
        if( isPortEnabled( nrp_list->s_ports, port ) )
        {
#if 0
// TBD-EDM
            InitFlushMgr(&policy->flush_config[port].server,
                    &policy->flush_point_list,
                    STREAM_FLPOLICY_FOOTPRINT, 0);
#endif
        }

        if( isPortEnabled( nrp_list->c_ports, port ) )
        {
#if 0
// TBD-EDM
            InitFlushMgr(&policy->flush_config[port].client,
                    &policy->flush_point_list,
                    STREAM_FLPOLICY_FOOTPRINT, 0);
#endif
        }
    }
}

void enableTargetedPolicyReassemblyPorts( StreamConfig *config )
{
    SDListItem *prev_node;
    SDListItem *net_node = sf_sdlist_head( &targeted_nrps );
    while( net_node != NULL )
    {
        NetworkReassemblyPorts *nrp_list = sf_sdlist_data( net_node );
        StreamTcpPolicy *policy = StreamSearchTcpConfigForBoundPolicy( config->tcp_config,
                                                                       &nrp_list->ip->addr );
        if( policy == config->tcp_config->default_policy )
        {
            // no existing policy for this network...create a new one by cloning
            // the default policy
            policy = StreamTcpPolicyClone( config->tcp_config->default_policy, 
                                           config );
        }

        // init the ports registered specifically for this network...
        enablePortsForReassembly( nrp_list, policy );

        // now look back up the list of networks and register ports from all networks
        // that contain the current network...
        prev_node = sf_sdlist_prev( net_node );
        while( prev_node != NULL )
        {
            NetworkReassemblyPorts *prev_nrp_list = sf_sdlist_data( prev_node );

            if( sfip_contains( prev_nrp_list->ip, &nrp_list->ip->addr ) == SFIP_CONTAINS )
                enablePortsForReassembly( prev_nrp_list, policy );
            prev_node = sf_sdlist_prev( prev_node );
        }

        net_node = sf_sdlist_next( net_node );
    }
}

void enableRegisteredPortsForReassembly( struct _SnortConfig *sc )
{
    int idx;
    tSfPolicyUserContextId stream_cfg = (stream_parsing_config)? stream_parsing_config : stream_online_config;
    StreamConfig *config = (StreamConfig *) sfPolicyUserDataGet(stream_cfg, getParserPolicy(sc));

    // Since stream configs are inherited from the default,
    // we should attempt to fetch the default policy config
    // if the non-default lookup failed
    if (!config && getParserPolicy(sc) != getDefaultPolicy())
        config = (StreamConfig *) sfPolicyUserDataGet(
            stream_cfg,
            getDefaultPolicy()
        );

    if (!config)
    {
        WarningMessage(
            "Stream TCP not enabled in default configuration.\n"
        );

        return;
    }


    // iterate thru list of networks, init'ing all ports requested from reassembly for each
    enableTargetedPolicyReassemblyPorts( config );

    // ... now init ports registered with the default policy, these ports also get
    // registered with all targeted policies...
    enablePortsForReassembly( &default_nrps, config->tcp_config->default_policy );

    // enable ports in all target policies...
    for (idx = 0; idx < config->tcp_config->num_policies; idx++)
        enablePortsForReassembly( &default_nrps,  config->tcp_config->policy_list[ idx ] );

    // all ports registered for reassembly have been enabled in all appropriate networks,
    // free memory used to cache the requests...
    StreamDestoryReassemblyPortList( );
}


void SetFTPFileLocation(void *scbptr ,bool flush)
{
  SessionControlBlock *scb = ( SessionControlBlock * ) scbptr;
  TcpSession * tcpssn;

  if ( scb  && scb->proto_specific_data && scb->proto_specific_data->data)
  {
    tcpssn = (TcpSession *)scb->proto_specific_data->data;
    tcpssn->client.flush_mgr.flush = flush;
    tcpssn->server.flush_mgr.flush = flush;
  }
}


void unregisterPortForReassembly( char *network, uint16_t port, int ra_dir )
{

    // TBD may not need this capability...
    WarningMessage( "Unregistering ports for reassembly not currently supported" );
    return;
}

bool StreamIsSessionHttp2Tcp( SessionControlBlock *scb )
{
    if ( scb->ha_state.session_flags & SSNFLAG_HTTP_2)
    {
        return true;
    }
    return false;
}

void StreamSetSessionHttp2Tcp( SessionControlBlock *scb )
{
    scb->ha_state.session_flags |= SSNFLAG_HTTP_2;
}

bool StreamIsSessionHttp2UpgTcp( SessionControlBlock *scb )
{
    if ( scb->ha_state.session_flags & SSNFLAG_HTTP_2_UPG)
    {
        return true;
    }
    return false;
}

void StreamSetSessionHttp2UpgTcp( SessionControlBlock *scb )
{
    scb->ha_state.session_flags |= SSNFLAG_HTTP_2_UPG;
}

#ifdef SNORT_RELOAD
void SessionTCPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout)
{
    SessionReload(tcp_lws_cache, max_sessions, pruningTimeout, nominalTimeout
#ifdef REG_TEST
                  , "TCP"
#endif
                  );
}

unsigned SessionTCPReloadAdjust(unsigned maxWork)
{
    return SessionProtocolReloadAdjust(tcp_lws_cache, session_configuration->max_tcp_sessions,
                                       maxWork, session_configuration->memcap
#ifdef REG_TEST
                                       , "TCP"
#endif
                                       );
}
#endif

#ifdef HAVE_DAQ_DECRYPTED_SSL
int StreamSimulatePeerTcpAckp( SessionControlBlock *scb, uint8_t dir, uint32_t tcp_payload_len )
{

    StreamTracker *talker = NULL;
    StreamTracker *listener = NULL;
    TcpSession *tcpssn = NULL;

    if ((tcp_payload_len == 0) || !scb)
    {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                          "Stream: no SCB %p or zero length %d \n", scb, tcp_payload_len););
        return -1;
    }
    if (scb->proto_specific_data)
        tcpssn = (TcpSession *)scb->proto_specific_data->data;
    if (!tcpssn) {
        STREAM_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                          "Stream: no tcpssn \n"););
        return -1;
    }

    if (dir == 0)
    {
        // Packet from Server. (p->packet_flags & PKT_FROM_SERVER) is TRUE.
        talker = &tcpssn->server;
        listener = &tcpssn->client;
    } else
    {
        talker = &tcpssn->client;
        listener = &tcpssn->server;
    }
    talker->l_unackd += tcp_payload_len;
    listener->r_win_base += tcp_payload_len;
    purge_flushed_ackd(tcpssn, listener);

    return 0;
}
#endif

size_t get_tcp_used_mempool()
{
    if (tcp_lws_cache && tcp_lws_cache->protocol_session_pool)
        return tcp_lws_cache->protocol_session_pool->used_memory;
    
    return 0;
}

