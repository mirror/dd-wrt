/* $Id$ */
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

/**
 * @file    snort_stream5_tcp.c
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

#include "perf.h"
#include "debug.h"
#include "detect.h"
#include "plugbase.h"
#include "mstring.h"
#include "sfxhash.h"
#include "util.h"
#include "sflsq.h"
#include "bounds.h"
#include "generators.h"
#include "event_queue.h"
#include "snort.h"
#include "parser/IpAddrSet.h"

#include "decode.h"
#include "encode.h"
#include "log.h"
#include "active.h"
#include "spp_normalize.h"

#include "stream5_common.h"
#include "snort_stream5_tcp.h"
#include "stream_api.h"
#include "snort_stream5_session.h"
#include "stream_ignore.h"

#include <errno.h>

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

extern tSfPolicyUserContextId s5_config;
extern Stream5GlobalConfig *s5_global_eval_config;
extern Stream5TcpConfig *s5_tcp_eval_config;
extern uint32_t mem_in_use;
extern tSfActionQueueId decoderActionQ;

#ifdef PERF_PROFILING
PreprocStats s5TcpPerfStats;
PreprocStats s5TcpNewSessPerfStats;
PreprocStats s5TcpStatePerfStats;
PreprocStats s5TcpDataPerfStats;
PreprocStats s5TcpInsertPerfStats;
PreprocStats s5TcpFlushPerfStats;
PreprocStats s5TcpBuildPacketPerfStats;
PreprocStats s5TcpProcessRebuiltPerfStats;

PreprocStats streamSizePerfStats;
PreprocStats streamReassembleRuleOptionPerfStats;
extern PreprocStats preprocRuleOptionPerfStats;
#endif

#ifdef SNORT_RELOAD
extern tSfPolicyUserContextId s5_swap_config;
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
#define PAWS_24DAYS         2073600         /* 24 days in seconds */

/* for state transition queuing */
#define CHK_SEQ         0
#define NO_CHK_SEQ      1

#define S5_UNALIGNED       0
#define S5_ALIGNED         1

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
#define ACTION_LWSSN_DELETED            0x00000800

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

#define TF_NONE                     0x00
#define TF_WSCALE                   0x01
#define TF_TSTAMP                   0x02
#define TF_TSTAMP_ZERO              0x04
#define TF_MSS                      0x08
#define TF_FORCE_FLUSH              0x10
#define TF_MISSING_PKT              0x20
#define TF_PKT_MISSED               0x40
#define TF_MISSING_PREV_PKT         0x80
#define TF_ALL                      0xFF

#define STREAM_INSERT_OK            0
#define STREAM_INSERT_ANOMALY       1
#define STREAM_INSERT_TIMEOUT       2
#define STREAM_INSERT_FAILED        3

#define S5_DEFAULT_TCP_PACKET_MEMCAP  8388608  /* 8MB */
#define S5_MIN_OVERLAP_LIMIT 0
#define S5_MAX_OVERLAP_LIMIT 255
#define S5_MAX_FLUSH_FACTOR 2048

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
#define REASSEMBLY_POLICY_DEFAULT   REASSEMBLY_POLICY_BSD

#define STREAM_MAX_PACKET (IP_MAXPACKET - (IP_HEADER_LEN + TCP_HEADER_LEN))
#define STREAM_MAX_PACKET6 (IP_MAXPACKET - (IP6_HDR_LEN + TCP_HEADER_LEN))

//#define DEBUG_STREAM5
#define SUB_SYN_SENT  0x01
#define SUB_ACK_SENT  0x02
#define SUB_SETUP_OK  0x03
#define SUB_RST_SENT  0x04
#define SUB_FIN_SENT  0x08

#ifdef DEBUG_STREAM5
#define STREAM5_DEBUG_WRAP(x) DEBUG_WRAP(x)
#else
#define STREAM5_DEBUG_WRAP(x)
#endif

/* client/server ip/port dereference */
#define tcp_client_ip lwssn->client_ip
#define tcp_client_port lwssn->client_port
#define tcp_server_ip lwssn->server_ip
#define tcp_server_port lwssn->server_port
/*  D A T A  S T R U C T U R E S  ***********************************/
typedef struct _TcpDataBlock
{
    snort_ip        sip;
    snort_ip        dip;
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

typedef struct _CustomFlushPolicy
{
    FlushMgr client;
    FlushMgr server;
    IpAddrSet *bound_addrs;
} CustomFlushPolicy;

//-----------------------------------------------------------------
// we make a lot of StreamSegments, StreamTrackers, and TcpSessions
// so they are organized by member size/alignment requirements to 
// minimize unused space in the structs.
//-----------------------------------------------------------------

typedef struct _StreamSegment
{
    DAQ_PktHdr_t pkth;

    uint8_t    *pktOrig;
    uint8_t    *pkt;
    uint8_t    *data;
    uint8_t    *payload;

    struct _StreamSegment *prev;
    struct _StreamSegment *next;

#ifdef DEBUG
    int ordinal;
#endif
    uint32_t   cksum;
    uint32_t   caplen;  // FIXTHIS can pkth->caplen be used instead??
    uint32_t   ts;
    uint32_t   seq;

    uint16_t   size;
    uint16_t   urg_offset;

    uint8_t    buffered;
    uint8_t    blocked;

} StreamSegment;

typedef struct _StreamTracker
{
    StateMgr  s_mgr;        /* state tracking goodies */
    FlushMgr  flush_mgr;    /* please flush twice, it's a long way to
                             * the bitbucket... */

    Stream5AlertInfo alerts[MAX_SESSION_ALERTS]; /* history of alerts */

    Stream5TcpPolicy *tcp_policy;
    StreamSegment *seglist;       /* first queued segment */
    StreamSegment *seglist_tail;  /* last queued segment */
    // FIXTHIS should move out of here since only used per packet
    // or use as "flush cursor" into seglist for all modes (ips and non-ips)
    StreamSegment* seglist_next;  /* next queued segment to flush */

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
    uint32_t gap_seq;      /* sequence of next packet after a gap */
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

    uint16_t os_policy;
    uint16_t reassembly_policy;

    uint16_t wscale;       /* window scale setting */
    uint16_t mss;          /* max segment size */

    uint8_t  mac_addr[6];
    uint8_t  flags;        /* bitmap flags (TF_xxx) */

    uint8_t  alert_count;  /* number alerts stored (up to MAX_SESSION_ALERTS) */

} StreamTracker;

typedef struct _TcpSession
{
    StreamTracker client;
    StreamTracker server; 
    Stream5LWSession *lwssn;

#ifdef DEBUG
    struct timeval ssn_time;
#endif

    uint8_t ecn;

} TcpSession;

#define SL_BUF_FLUSHED 1
#define SL_BUF_DUMPED  2

static INLINE uint32_t SegsToFlush (const StreamTracker* st)
{
    uint32_t n = 0;
    StreamSegment* s;

    if ( st->seg_count > st->flush_count )
        n = st->seg_count - st->flush_count;

    if ( n < 2
#ifdef NORMALIZER
        || Normalize_IsEnabled(snort_conf, NORM_TCP_IPS)
#endif
    )
        return n;

    n = 0;
    s = st->seglist;

    while ( s )
    {
        if ( !s->buffered && SEQ_LT(s->seq, st->r_win_base) ) n++;
        s = s->next;
    }
    return n;
}

int default_ports[] =
{
    21, 23, 25, 42, 53, 80, 110, 111, 135, 136, 137, 139, 143, 445,
    513, 514, 1433, 1521, 2401, 3306
};

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
static void Stream5ParseTcpArgs(Stream5TcpConfig *, char *, Stream5TcpPolicy *);
static void Stream5PrintTcpConfig(Stream5TcpPolicy *);

static void Stream5InitPacket();
static INLINE void SetupTcpDataBlock(TcpDataBlock *, Packet *);
static int ProcessTcp(Stream5LWSession *, Packet *, TcpDataBlock *,
        Stream5TcpPolicy *);
#if OLD_CODE_NOLONGER_USED_DEPENDS_ON_CURRENT_STATE
static INLINE void QueueState(uint8_t, StreamTracker*, uint8_t,
        uint32_t, uint8_t);
static INLINE int EvalStateQueue(StreamTracker *, uint8_t, uint32_t);
#endif
static int CheckFlushPolicy(TcpSession *, StreamTracker *, StreamTracker *,
                TcpDataBlock *, Packet *);
static void Stream5SeglistAddNode(StreamTracker *, StreamSegment *,
                StreamSegment *);
static int Stream5SeglistDeleteNode(StreamTracker *, StreamSegment *);
static int AddStreamNode(StreamTracker *st, Packet *p,
                  TcpDataBlock*,
                  TcpSession *tcpssn,
                  int16_t len,
                  uint32_t slide,
                  uint32_t trunc,
                  uint32_t seq,
                  StreamSegment *left,
                  StreamSegment **retSeg);

static uint32_t Stream5GetWscale(Packet *, uint16_t *);
static uint32_t Stream5PacketHasWscale(Packet *);
static uint32_t Stream5GetMss(Packet *, uint16_t *);
static uint32_t Stream5GetTcpTimestamp(Packet *, uint32_t *, int strip);
static int FlushStream(
    StreamTracker *st, uint32_t toSeq, uint8_t *flushbuf, const uint8_t *flushbuf_end);
void TcpSessionCleanup(Stream5LWSession *ssn);

int s5TcpStreamSizeInit(char *name, char *parameters, void **dataPtr);
int s5TcpStreamSizeEval(void *p, const uint8_t **cursor, void *dataPtr);
void s5TcpStreamSizeCleanup(void *dataPtr);
int s5TcpStreamReassembleRuleOptionInit(char *name, char *parameters, void **dataPtr);
int s5TcpStreamReassembleRuleOptionEval(void *p, const uint8_t **cursor, void *dataPtr);
void s5TcpStreamReassembleRuleOptionCleanup(void *dataPtr);
static INLINE void ResetFlushMgrs(void);
static void targetPolicyIterate(void (*callback)(int));
static void policyDecoderFlagsSaveNClear(int policyId);
static void policyDecoderFlagsRestore(int policyId);

/*  G L O B A L S  **************************************************/
Stream5SessionCache *tcp_lws_cache = NULL;
static MemPool tcp_session_mempool;
static Packet *s5_pkt = NULL;
static const uint8_t *s5_pkt_end = NULL;
static char midstream_allowed = 0;

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

#ifdef DEBUG_STREAM5
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
    "Consumed",
    "Ignore"};

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
static INLINE uint32_t GenerateFlushPoint(FlushPointList *flush_point_list)
{
    return (rand() % flush_point_list->flush_range) + flush_point_list->flush_base;
}

static INLINE void InitFlushPointList(FlushPointList *flush_point_list, uint32_t value, uint32_t range, int use_static)
{
    uint32_t i;
    uint32_t flush_range = range;
    uint32_t flush_base = value - range/2;

    if (!flush_point_list)
        return;

    if (!flush_point_list->initialized)
    {
        flush_point_list->flush_range = flush_range;
        flush_point_list->flush_base = flush_base;
#ifndef DYNAMIC_RANDOM_FLUSH_POINTS
        flush_point_list->current = 0;

        flush_point_list->flush_points = SnortAlloc(sizeof(uint32_t) * RAND_FLUSH_POINTS);
        for (i=0;i<RAND_FLUSH_POINTS;i++)
        {
            if (snort_conf->run_flags & RUN_FLAG__STATIC_HASH)
            {
                if ( i == 0 )
                    LogMessage("Warning:  using constant flush points!\n");
                flush_point_list->flush_points[i] = 192;
            }
            else if (use_static)
            {
                if ( i == 0 )
                    LogMessage("Warning:  using static flush points!\n");
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

static INLINE void UpdateFlushMgr(FlushMgr *mgr, FlushPointList *flush_point_list)
{
    switch (mgr->flush_policy)
    {
        case STREAM_FLPOLICY_FOOTPRINT:
        case STREAM_FLPOLICY_LOGICAL:
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
            mgr->last_size = 0;
            mgr->last_count = 0;
            break;

        default:
            break;
    }
}

static INLINE void InitFlushMgr(FlushMgr *mgr, FlushPointList *flush_point_list, uint8_t policy)
{
    mgr->flush_policy = policy;
    if ((policy == STREAM_FLPOLICY_FOOTPRINT) ||
        (policy == STREAM_FLPOLICY_LOGICAL))
    {
        UpdateFlushMgr(mgr, flush_point_list);
    }
}

static int ResetFlushMgrsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    int i;
    Stream5Config *pPolicyConfig = (Stream5Config *)pData;

    //do any housekeeping before freeing Stream5Config
    if (pPolicyConfig->tcp_config == NULL)
        return 0;

    for (i = 0; i < pPolicyConfig->tcp_config->num_policies; i++)
    {
        int j;
        Stream5TcpPolicy *policy = pPolicyConfig->tcp_config->policy_list[i];
        FlushPointList *fpl = &policy->flush_point_list;
        FlushMgr *client, *server;
        uint8_t flush_policy;

        fpl->current = 0;

        for (j = 0; j < MAX_PORTS; j++)
        {
            client = &policy->flush_config[j].client;
            flush_policy = policy->flush_config[j].client.flush_policy;
            InitFlushMgr(client, fpl, flush_policy);

            server = &policy->flush_config[j].server;
            flush_policy = policy->flush_config[j].server.flush_policy;
            InitFlushMgr(server, fpl, flush_policy);
        }
#ifdef TARGET_BASED
        /* protocol 0 is the unknown case. skip it */
        for (j = 1; j < MAX_PROTOCOL_ORDINAL; j++)
        {
            client = &policy->flush_config_protocol[j].client;
            flush_policy = policy->flush_config_protocol[j].client.flush_policy;
            InitFlushMgr(client, fpl, flush_policy);

            server = &policy->flush_config_protocol[j].server;
            flush_policy = policy->flush_config_protocol[j].server.flush_policy;
            InitFlushMgr(server, fpl, flush_policy);
        }
#endif
    }

    return 0;
}

static INLINE void ResetFlushMgrs(void)
{
    if (s5_config == NULL)
        return;

    sfPolicyUserDataIterate (s5_config, ResetFlushMgrsPolicy);
}

void Stream5UpdatePerfBaseState(SFBASE *sf_base,
                                Stream5LWSession *lwssn,
                                char newState)
{
    if (!lwssn)
    {
        return;
    }

    switch (newState)
    {
    case TCP_STATE_SYN_SENT:
        if (!(lwssn->session_flags & SSNFLAG_COUNTED_INITIALIZE))
        {
            sf_base->iSessionsInitializing++;
            lwssn->session_flags |= SSNFLAG_COUNTED_INITIALIZE;
        }
        break;
    case TCP_STATE_ESTABLISHED:
        if (!(lwssn->session_flags & SSNFLAG_COUNTED_ESTABLISH))
        {
            sf_base->iSessionsEstablished++;
            if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
                UpdateFlowIPState(&sfFlow, IP_ARG(lwssn->client_ip), IP_ARG(lwssn->server_ip), SFS_STATE_TCP_ESTABLISHED); 
            lwssn->session_flags |= SSNFLAG_COUNTED_ESTABLISH;
            if (lwssn->session_flags & SSNFLAG_COUNTED_INITIALIZE)
            {
                sf_base->iSessionsInitializing--;
            }
        }
        break;
    case TCP_STATE_CLOSING:
        if (!(lwssn->session_flags & SSNFLAG_COUNTED_CLOSING))
        {
            sf_base->iSessionsClosing++;
            lwssn->session_flags |= SSNFLAG_COUNTED_CLOSING;
            if (lwssn->session_flags & SSNFLAG_COUNTED_ESTABLISH)
            {
                sf_base->iSessionsEstablished--;
                if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
                    UpdateFlowIPState(&sfFlow, IP_ARG(lwssn->client_ip), IP_ARG(lwssn->server_ip), SFS_STATE_TCP_CLOSED); 
            }
            else if (lwssn->session_flags & SSNFLAG_COUNTED_INITIALIZE)
            {
                sf_base->iSessionsInitializing--;
            }
        }
        break;
    case TCP_STATE_CLOSED:
        if (lwssn->session_flags & SSNFLAG_COUNTED_CLOSING)
        {
            sf_base->iSessionsClosing--;
        }
        else if (lwssn->session_flags & SSNFLAG_COUNTED_ESTABLISH)
        {
            sf_base->iSessionsEstablished--;
            if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
                UpdateFlowIPState(&sfFlow, IP_ARG(lwssn->client_ip), IP_ARG(lwssn->server_ip), SFS_STATE_TCP_CLOSED); 
        }
        else if (lwssn->session_flags & SSNFLAG_COUNTED_INITIALIZE)
        {
            sf_base->iSessionsInitializing--;
        }
        break;
    default:
        break;
    }
}

void Stream5InitTcp(Stream5GlobalConfig *gconfig)
{
    if (gconfig == NULL)
        return;

    if((tcp_lws_cache == NULL) && gconfig->track_tcp_sessions)
    {
        tcp_lws_cache = InitLWSessionCache(gconfig->max_tcp_sessions,
                                           30, 5, 0, &TcpSessionCleanup);

        if(!tcp_lws_cache)
        {
            FatalError("Unable to init stream5 TCP session cache, no TCP "
                       "stream inspection!\n");
        }

        if (mempool_init(&tcp_session_mempool,
                    gconfig->max_tcp_sessions, sizeof(TcpSession)) != 0)
        {
            FatalError("%s(%d) Could not initialize tcp session memory pool.\n",
                    __FILE__, __LINE__);
        }

        Stream5TcpRegisterPreprocProfiles();
    }
}

void Stream5TcpRegisterPreprocProfiles(void)
{
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("s5TcpNewSess", &s5TcpNewSessPerfStats, 2, &s5TcpPerfStats);
    RegisterPreprocessorProfile("s5TcpState", &s5TcpStatePerfStats, 2, &s5TcpPerfStats);
    RegisterPreprocessorProfile("s5TcpData", &s5TcpDataPerfStats, 3, &s5TcpStatePerfStats);
    RegisterPreprocessorProfile("s5TcpPktInsert", &s5TcpInsertPerfStats, 4, &s5TcpDataPerfStats);
    RegisterPreprocessorProfile("s5TcpFlush", &s5TcpFlushPerfStats, 3, &s5TcpStatePerfStats);
    RegisterPreprocessorProfile("s5TcpBuildPacket", &s5TcpBuildPacketPerfStats, 4, &s5TcpFlushPerfStats);
    RegisterPreprocessorProfile("s5TcpProcessRebuilt",
                                &s5TcpProcessRebuiltPerfStats, 4, &s5TcpFlushPerfStats);
#endif
}

void Stream5TcpRegisterRuleOptions(void)
{
#ifdef DYNAMIC_PLUGIN
    /* Register the 'stream_size' rule option */
    RegisterPreprocessorRuleOption("stream_size", &s5TcpStreamSizeInit,
                                   &s5TcpStreamSizeEval, &s5TcpStreamSizeCleanup,
                                   NULL, NULL, NULL, NULL);

    RegisterPreprocessorRuleOption("stream_reassemble", &s5TcpStreamReassembleRuleOptionInit,
                                   &s5TcpStreamReassembleRuleOptionEval, &s5TcpStreamReassembleRuleOptionCleanup,
                                   NULL, NULL, NULL, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("stream_size", &streamSizePerfStats, 4, &preprocRuleOptionPerfStats);
    RegisterPreprocessorProfile("reassemble", &streamReassembleRuleOptionPerfStats, 4, &preprocRuleOptionPerfStats);
#endif
#endif

}

void Stream5TcpInitFlushPoints(void)
{
    int i;

    /* Seed the flushpoint random generator */
    srand( (unsigned int) sizeof(default_ports) + (unsigned int) time(NULL) );

    /* Default is to ignore, for all ports */
    for(i=0;i<MAX_PORTS;i++)
    {
        ignore_flush_policy[i].client.flush_policy = STREAM_FLPOLICY_IGNORE;
        ignore_flush_policy[i].server.flush_policy = STREAM_FLPOLICY_IGNORE;
    }
#ifdef TARGET_BASED
    for(i=0;i<MAX_PROTOCOL_ORDINAL;i++)
    {
        ignore_flush_policy_protocol[i].client.flush_policy = STREAM_FLPOLICY_IGNORE;
        ignore_flush_policy_protocol[i].server.flush_policy = STREAM_FLPOLICY_IGNORE;
    }
#endif
}

void Stream5TcpPolicyInit(Stream5TcpConfig *config, char *args)
{
    Stream5TcpPolicy *s5TcpPolicy;

    if (config == NULL)
        return;

    s5TcpPolicy = (Stream5TcpPolicy *) SnortAlloc(sizeof(Stream5TcpPolicy));

    /* Initialize flush policy to Ignore */
    memcpy(&s5TcpPolicy->flush_config, ignore_flush_policy,
            sizeof(FlushConfig) * MAX_PORTS);
#ifdef TARGET_BASED
    memcpy(&s5TcpPolicy->flush_config_protocol, ignore_flush_policy_protocol,
            sizeof(FlushConfig) * MAX_PROTOCOL_ORDINAL);
#endif

    Stream5ParseTcpArgs(config, args, s5TcpPolicy);

    config->num_policies++;

    /* Now add this context to the internal list */
    if (config->policy_list == NULL)
    {
        config->policy_list =
            (Stream5TcpPolicy **)SnortAlloc(sizeof (Stream5TcpPolicy *));
    }
    else
    {
        Stream5TcpPolicy **tmpPolicyList =
            (Stream5TcpPolicy **)SnortAlloc(sizeof (Stream5TcpPolicy *) * (config->num_policies));

        memcpy(tmpPolicyList, config->policy_list,
               sizeof(Stream5TcpPolicy *) * (config->num_policies - 1));

        free(config->policy_list);
        
        config->policy_list = tmpPolicyList;
    }

    config->policy_list[config->num_policies - 1] = s5TcpPolicy;

    Stream5PrintTcpConfig(s5TcpPolicy);
}

static INLINE uint16_t StreamPolicyIdFromName(char *name)
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

static INLINE uint16_t GetTcpReassemblyPolicy(int os_policy)
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
        default:
            return REASSEMBLY_POLICY_DEFAULT;
    }
}

#define STATIC_FP ((s5TcpPolicy->flags & STREAM5_CONFIG_STATIC_FLUSHPOINTS)?1:0)

static void Stream5ParseTcpArgs(Stream5TcpConfig *config, char *args, Stream5TcpPolicy *s5TcpPolicy)
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
    int reassembly_direction = SSN_DIR_CLIENT;
    int32_t long_val = 0;

    s5TcpPolicy->policy = STREAM_POLICY_DEFAULT;
    s5TcpPolicy->reassembly_policy = REASSEMBLY_POLICY_DEFAULT;
    s5TcpPolicy->session_timeout = S5_DEFAULT_SSN_TIMEOUT;
    s5TcpPolicy->max_window = 0;
    s5TcpPolicy->flags = 0;
    s5TcpPolicy->max_queued_bytes = S5_DEFAULT_MAX_QUEUED_BYTES;
    s5TcpPolicy->max_queued_segs = S5_DEFAULT_MAX_QUEUED_SEGS;

    s5TcpPolicy->max_consec_small_segs = S5_DEFAULT_CONSEC_SMALL_SEGS;
    s5TcpPolicy->max_consec_small_seg_size = S5_DEFAULT_MAX_SMALL_SEG_SIZE;

    if(args != NULL && strlen(args) != 0)
    {
        toks = mSplit(args, ",", 0, &num_toks, 0);

        for (i = 0; i < num_toks; i++)
        {
            if(!strcasecmp(toks[i], "use_static_footprint_sizes"))
                s5TcpPolicy->flags |= STREAM5_CONFIG_STATIC_FLUSHPOINTS;
        }

        for (i = 0; i < num_toks; i++)
        {
            stoks = mSplit(toks[i], " ", 3, &s_toks, 0);

            if (s_toks == 0)
            {
                FatalError("%s(%d) => Missing parameter in Stream5 TCP config.\n",
                    file_name, file_line);
            }

            if(!strcasecmp(stoks[0], "timeout"))
            {
                if(stoks[1])
                {
                    s5TcpPolicy->session_timeout = strtoul(stoks[1], &endPtr, 10);
                }
                
                if (!stoks[1] || (endPtr == &stoks[1][0]))
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  "
                        "Integer parameter required.\n",
                        file_name, file_line);
                }

                if ((s5TcpPolicy->session_timeout > S5_MAX_SSN_TIMEOUT) ||
                    (s5TcpPolicy->session_timeout < S5_MIN_SSN_TIMEOUT))
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  "
                        "Must be between %d and %d\n",
                        file_name, file_line,
                        S5_MIN_SSN_TIMEOUT, S5_MAX_SSN_TIMEOUT);
                }
                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
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

                if ((long_val > S5_MAX_OVERLAP_LIMIT) ||
                    (long_val < S5_MIN_OVERLAP_LIMIT))
                {
                    FatalError("%s(%d) => Invalid overlap limit in config file."
                        "  Must be between %d and %d\n",
                        file_name, file_line,
                        S5_MIN_OVERLAP_LIMIT, S5_MAX_OVERLAP_LIMIT);
                }
                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "detect_anomalies"))
            {
                s5TcpPolicy->flags |=  STREAM5_CONFIG_ENABLE_ALERTS;
                if (s_toks > 1)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "policy"))
            {
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

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "require_3whs"))
            {
                s5TcpPolicy->flags |= STREAM5_CONFIG_REQUIRE_3WHS;

                if (s_toks > 1)
                {
                    s5TcpPolicy->hs_timeout = SnortStrtoul(stoks[1], &endPtr, 10);

                    if ((endPtr == &stoks[1][0]) || (*endPtr != '\0') || (errno == ERANGE))
                    {
                        FatalError("%s(%d) => Invalid 3Way Handshake allowable.  Integer parameter required.\n",
                                   file_name, file_line);
                    }

                    if (s5TcpPolicy->hs_timeout > S5_MAX_SSN_TIMEOUT)
                    {
                        FatalError("%s(%d) => Invalid handshake timeout in "
                                   "config file.  Must be between %d and %d\n",
                                   file_name, file_line,
                                   S5_MIN_ALT_HS_TIMEOUT, S5_MAX_SSN_TIMEOUT);
                    }
                }

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "bind_to"))
            {
                if (s_toks < 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option - "
                               "\"bind_to\" option requires an argument.\n",
                               file_name, file_line);
                }

                if(strstr(stoks[1], "["))
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  IP lists are not allowed.\n",
                        file_name, file_line);
                }

                s5TcpPolicy->bound_addrs = IpAddrSetParse(stoks[1]);

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
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

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
                if ((long_val > S5_MAX_MAX_WINDOW) ||
                    (long_val < S5_MIN_MAX_WINDOW))
                {
                    FatalError("%s(%d) => Invalid Max Window size."
                        "  Must be between %d and %d\n",
                        file_name, file_line,
                        S5_MIN_MAX_WINDOW, S5_MAX_MAX_WINDOW);
                }
            }
            else if(!strcasecmp(stoks[0], "use_static_footprint_sizes"))
            {
                // we already handled this one above
            }
            else if(!strcasecmp(stoks[0], "flush_factor"))
            {
                if (stoks[1])
                {
                    s5TcpPolicy->flush_factor = (uint8_t)SnortStrtoulRange(
                        stoks[1], &endPtr, 10, 0, S5_MAX_FLUSH_FACTOR);
                }
                if (
                    (!stoks[1] || (endPtr == &stoks[1][0])) ||
                    (s5TcpPolicy->flush_factor > S5_MAX_FLUSH_FACTOR))
                {
                    FatalError("%s(%d) => 'flush_factor %d' invalid: "
                        "value must be between 0 and %d segments.\n",
                        file_name, file_line, s5TcpPolicy->flush_factor,
                        S5_MAX_FLUSH_FACTOR);
                }
            }
            else if(!strcasecmp(stoks[0], "dont_store_large_packets"))
            {
                s5TcpPolicy->flags |= STREAM5_CONFIG_PERFORMANCE;
                if (s_toks > 1)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "check_session_hijacking"))
            {
                s5TcpPolicy->flags |= STREAM5_CONFIG_CHECK_SESSION_HIJACKING;
                if (s_toks > 1)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "ignore_any_rules"))
            {
                s5TcpPolicy->flags |= STREAM5_CONFIG_IGNORE_ANY;
                if (s_toks > 1)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                            file_name, file_line);
                }
            }
            else if(!strcasecmp(stoks[0], "dont_reassemble_async"))
            {
                s5TcpPolicy->flags |= STREAM5_CONFIG_NO_ASYNC_REASSEMBLY;
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

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
                if (((long_val > S5_MAX_MAX_QUEUED_BYTES) ||
                     (long_val < S5_MIN_MAX_QUEUED_BYTES)) &&
                    (long_val != 0))
                {
                    FatalError("%s(%d) => Invalid Max Queued Bytes."
                        "  Must be 0 (disabled) or between %d and %d\n",
                        file_name, file_line,
                        S5_MIN_MAX_QUEUED_BYTES, S5_MAX_MAX_QUEUED_BYTES);
                }
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

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream5 TCP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
                if (((long_val > S5_MAX_MAX_QUEUED_SEGS) ||
                     (long_val < S5_MIN_MAX_QUEUED_SEGS)) &&
                    (long_val != 0))
                {
                    FatalError("%s(%d) => Invalid Max Queued Bytes."
                        "  Must be 0 (disabled) or between %d and %d\n",
                        file_name, file_line,
                        S5_MIN_MAX_QUEUED_SEGS, S5_MAX_MAX_QUEUED_SEGS);
                }
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
                
                if ((long_val > S5_MAX_CONSEC_SMALL_SEGS) ||
                    (long_val < S5_MIN_CONSEC_SMALL_SEGS))
                {
                    FatalError("%s(%d) => Invalid Small Segments."
                        "  Must be integer between %d and %d, inclusive\n",
                        file_name, file_line,
                        S5_MIN_CONSEC_SMALL_SEGS, S5_MAX_CONSEC_SMALL_SEGS);
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
                
                if ((long_val > S5_MAX_MAX_SMALL_SEG_SIZE) ||
                    (long_val < S5_MIN_MAX_SMALL_SEG_SIZE))
                {
                    FatalError("%s(%d) => Invalid Small Segments bytes."
                        "  Must be integer between %d and %d, inclusive\n",
                        file_name, file_line,
                        S5_MIN_MAX_SMALL_SEG_SIZE, S5_MAX_MAX_SMALL_SEG_SIZE);
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
                mSplitFree(&ptoks, num_ptoks);
            }
            else if (!strcasecmp(stoks[0], "ports"))
            {
                if (s_toks > 1)
                {
                    if(!strcasecmp(stoks[1], "client"))
                    {
                        reassembly_direction = SSN_DIR_CLIENT;
                    }
                    else if(!strcasecmp(stoks[1], "server"))
                    {
                        reassembly_direction = SSN_DIR_SERVER;
                    }
                    else
                    {
                        reassembly_direction = SSN_DIR_BOTH;
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
                            if (reassembly_direction & SSN_DIR_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                            if (reassembly_direction & SSN_DIR_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                        }
                    }
                    else if (!strcasecmp(stoks[2], "none"))
                    {
                        for (j=0; j<MAX_PORTS; j++)
                        {
                            if (reassembly_direction & SSN_DIR_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].client;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                            if (reassembly_direction & SSN_DIR_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[j].server;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                        }
                    }
                    else
                    {
                        ptoks = mSplit(stoks[2], " ", MAX_PORTS, &num_ptoks, 0);

                        for (j=0;j<num_ptoks;j++)
                        {
                            if (ptoks[j])
                            {
                                long_port = strtol(ptoks[j], &endPtr, 10);
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

                            if (reassembly_direction & SSN_DIR_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[port].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                            if (reassembly_direction & SSN_DIR_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[port].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                        }
                        mSplitFree(&ptoks, num_ptoks);
                    }
                    set_flush_policy = 1;
                }
            }
#ifdef TARGET_BASED
            else if (!strcasecmp(stoks[0], "protocol"))
            {
                if (s_toks > 1)
                {
                    if(!strcasecmp(stoks[1], "client"))
                    {
                        reassembly_direction = SSN_DIR_CLIENT;
                    }
                    else if(!strcasecmp(stoks[1], "server"))
                    {
                        reassembly_direction = SSN_DIR_SERVER;
                    }
                    else
                    {
                        reassembly_direction = SSN_DIR_BOTH;
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
                            if (reassembly_direction & SSN_DIR_CLIENT) 
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                            if (reassembly_direction & SSN_DIR_SERVER) 
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                            s5TcpPolicy->flush_config_protocol[j].configured = 1;
                        }
                    }
                    else if (!strcasecmp(stoks[2], "none"))
                    {
                        for (j=1; j<MAX_PROTOCOL_ORDINAL; j++)
                        {
                            if (reassembly_direction & SSN_DIR_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[j].client;
                                flush_mgr->flush_policy = STREAM_FLPOLICY_IGNORE;
                            }
                            if (reassembly_direction & SSN_DIR_SERVER)
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

                            if (reassembly_direction & SSN_DIR_CLIENT)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[proto_ordinal].client;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                            if (reassembly_direction & SSN_DIR_SERVER)
                            {
                                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[proto_ordinal].server;
                                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
                            }
                            s5TcpPolicy->flush_config_protocol[proto_ordinal].configured = 1;
                        }
                        mSplitFree(&ptoks, num_ptoks);
                    }
                    set_target_flush_policy = 1;
                }
            }
#endif
            else
            {
                FatalError("%s(%d) => Invalid Stream5 TCP policy option\n", 
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
            FatalError("%s(%d) => Default Stream5 TCP Policy already set. "
                "This policy must be bound to a specific host or "
                "network.\n", file_name, file_line);
        }

        config->default_policy = s5TcpPolicy;
    }
    else
    {
        if (s5TcpPolicy->flags & STREAM5_CONFIG_IGNORE_ANY)
        {
            FatalError("%s(%d) => \"ignore_any_rules\" option can be used only"
                   " with Default Stream5 TCP Policy\n", file_name, file_line);
        }
    }

    if (!set_flush_policy)
    {
        /* Initialize it if not already... */
        InitFlushPointList(&s5TcpPolicy->flush_point_list, 192, 128, STATIC_FP);
        for (i=0;i<(int)(sizeof(default_ports)/sizeof(int)); i++)
        {
            if (reassembly_direction & SSN_DIR_CLIENT)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[default_ports[i]].client;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
            }
            if (reassembly_direction & SSN_DIR_SERVER)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config[default_ports[i]].server;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
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
            /* Set flush managers. */
            if (reassembly_direction & SSN_DIR_CLIENT)
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[app_id].client;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
            }
            if (reassembly_direction & SSN_DIR_SERVER) 
            {
                FlushMgr *flush_mgr = &s5TcpPolicy->flush_config_protocol[app_id].server;
                FlushPointList *flush_point_list = &s5TcpPolicy->flush_point_list;
                InitFlushMgr(flush_mgr, flush_point_list, STREAM_FLPOLICY_FOOTPRINT);
            }
            s5TcpPolicy->flush_config_protocol[app_id].configured = 1;
        }
    }
#endif
}

static void Stream5PrintTcpConfig(Stream5TcpPolicy *s5TcpPolicy)
{
    int i=0, j=0;
    LogMessage("Stream5 TCP Policy config:\n");
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
        if (s5TcpPolicy->flags & STREAM5_CONFIG_REQUIRE_3WHS)
        {
            LogMessage("        Require 3-Way Handshake: YES\n");
            if (s5TcpPolicy->hs_timeout != 0)
            {
                LogMessage("        3-Way Handshake Timeout: %d\n",
                    s5TcpPolicy->hs_timeout);
            }
        }
        if (s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS)
        {
            LogMessage("        Detect Anomalies: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM5_CONFIG_STATIC_FLUSHPOINTS)
        {
            LogMessage("        Static Flushpoint Sizes: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM5_CONFIG_PERFORMANCE)
        {
            LogMessage("        Don't Queue Large Packets for Reassembly: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM5_CONFIG_CHECK_SESSION_HIJACKING)
        {
            LogMessage("        Check for TCP Session Hijacking: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM5_CONFIG_IGNORE_ANY)
        {
            LogMessage("        Ignore Any -> Any Rules: YES\n");
        }
        if (s5TcpPolicy->flags & STREAM5_CONFIG_NO_ASYNC_REASSEMBLY)
        {
            LogMessage("        Don't queue packets on one-sided sessions: YES\n");
        }
    }
    LogMessage("    Reassembly Ports:\n");
    for (i=0; i<MAX_PORTS && j<20; i++)
    {
        int direction = 0;
        int client_flushpolicy = s5TcpPolicy->flush_config[i].client.flush_policy;
        int server_flushpolicy = s5TcpPolicy->flush_config[i].server.flush_policy;
        char client_policy_str[STD_BUF];
        char server_policy_str[STD_BUF];
        client_policy_str[0] = server_policy_str[0] = '\0';

        if (client_flushpolicy != STREAM_FLPOLICY_IGNORE)
        {
            direction |= SSN_DIR_CLIENT;

            if (client_flushpolicy <= STREAM_FLPOLICY_MAX)
                SnortSnprintf(client_policy_str, STD_BUF, "client (%s)",
                              flush_policy_names[client_flushpolicy]);
        }
        if (server_flushpolicy != STREAM_FLPOLICY_IGNORE)
        {
            direction |= SSN_DIR_SERVER;

            if (server_flushpolicy <= STREAM_FLPOLICY_MAX)
                SnortSnprintf(server_policy_str, STD_BUF, "server (%s)",
                              flush_policy_names[server_flushpolicy]);
        }
        if (direction)
        {
            LogMessage("      %d %s %s\n", i,
                client_policy_str, server_policy_str);
#ifndef DEBUG
            /* Don't increment, so we print the full list in
             * debug mode. */
            j++;
#endif
        }
    }

#ifdef SUP_IP6
// XXX-IPv6 var printing
#else
    IpAddrSetPrint("    Bound Addresses:", s5TcpPolicy->bound_addrs);
#endif

}

#ifdef TARGET_BASED
int StreamPolicyIdFromHostAttributeEntry(HostAttributeEntry *host_entry)
{
    if (!host_entry || !host_entry->hostInfo.streamPolicyName)
        return 0;

    host_entry->hostInfo.streamPolicy = StreamPolicyIdFromName(host_entry->hostInfo.streamPolicyName);
    host_entry->hostInfo.streamPolicySet = 1;

    STREAM5_DEBUG_WRAP(
        DebugMessage(DEBUG_STREAM_STATE,
            "STREAM5 INIT: %s(%d) for Entry %s:%s:%s (%s)\n",
            reassembly_policy_names[host_entry->hostInfo.streamPolicy],
            host_entry->hostInfo.streamPolicy,
            host_entry->hostInfo.operatingSystem.value.s_value,
            host_entry->hostInfo.vendor.value.s_value,
            host_entry->hostInfo.version.value.s_value,
            host_entry->hostInfo.streamPolicyName););
    return 0;
}
#endif

void s5TcpPrintPortFilter();

/** 
 * Stream5VerifyTcpConfig is is called after all preprocs (static & dynamic)
 * are inited.
 */
int Stream5VerifyTcpConfig(Stream5TcpConfig *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return -1;

    if (!tcp_lws_cache)
    {
        LogMessage("WARNING: Stream5 TCP Session Cache not initialized\n");
        return -1;
    }

    if (config->num_policies == 0)
    {
        LogMessage("WARNING: Stream5 TCP no policies specified in configuration\n");
        return -1;
    }

    if (config->default_policy == NULL)
    {
        LogMessage("WARNING: Stream5 TCP default policy not specified in configuration\n");
        return -1;
    }

    /* Do this now
     * verify config is called after all preprocs (static & dynamic)
     * are inited.  Gives us the correct number of bits for
     * p->preprocessor_bits
     */
    if (!s5_pkt)
        Stream5InitPacket();

#ifdef TARGET_BASED
    SFAT_SetPolicyIds(StreamPolicyIdFromHostAttributeEntry, policy_id);
#endif

    /* Post-process TCP rules to establish TCP ports to inspect. */
    setPortFilterList(config->port_filter, IPPROTO_TCP, 
            (config->default_policy->flags & STREAM5_CONFIG_IGNORE_ANY), policy_id);

    //printf ("TCP Ports with Inspection/Monitoring\n");
    //s5PrintPortFilter(config->tcpPortFilter);
    return 0;
}


uint32_t Stream5GetTcpPrunes(void)
{
    return tcp_lws_cache ? tcp_lws_cache->prunes : s5stats.tcp_prunes;
}

void Stream5ResetTcpPrunes(void)
{
    if ( tcp_lws_cache )
        tcp_lws_cache->prunes = 0;
}

void Stream5ResetTcp(void)
{
    if (snort_conf == NULL)
    {
        ErrorMessage("Snort configuration is NULL.\n");
        return;
    }

    /* Unset decoder flags for the purge */
    targetPolicyIterate(policyDecoderFlagsSaveNClear);

    s5_tcp_cleanup = 1;
    PurgeLWSessionCache(tcp_lws_cache);
    s5_tcp_cleanup = 0;
    mempool_clean(&tcp_session_mempool);

    /* Set decoder flags back to original */
    targetPolicyIterate(policyDecoderFlagsRestore);

    ResetFlushMgrs();
}


void Stream5CleanTcp(void)
{
    if (snort_conf == NULL)
    {
        ErrorMessage("Snort configuration is NULL.\n");
        return;
    }

    if ( tcp_lws_cache )
        s5stats.tcp_prunes = tcp_lws_cache->prunes;

    /* Turn off decoder alerts since we're decoding stored
     * packets that we already alerted on. */
    targetPolicyIterate(policyDecoderFlagsSaveNClear);

    /* Set s5_tcp_cleanup to force a flush of all queued data */
    s5_tcp_cleanup = 1;
    /* Clean up hash table -- delete all sessions */
    DeleteLWSessionCache(tcp_lws_cache);
    tcp_lws_cache = NULL;

    /* Cleanup the rebuilt packet */
    if (s5_pkt)
    {
        Encode_Delete(s5_pkt);
        s5_pkt = NULL;
    }

    /* Reset this */
    s5_tcp_cleanup = 0;

    mempool_destroy(&tcp_session_mempool);

    /* And turn decoder alerts back on (or whatever they were set to) */
    targetPolicyIterate(policyDecoderFlagsRestore);
}

void Stream5TcpConfigFree(Stream5TcpConfig *config)
{
    int i;

    if (config == NULL)
        return;

    /* Cleanup TCP Policies and the list */
    for (i = 0; i < config->num_policies; i++)
    {
        Stream5TcpPolicy *policy = config->policy_list[i];

        free(policy->flush_point_list.flush_points);

        if (policy->bound_addrs != NULL)
#ifdef SUP_IP6
            sfvar_free(policy->bound_addrs);
#else
        {
            IpAddrSetDestroy(policy->bound_addrs);
            free(policy->bound_addrs);
        }
#endif
        free(policy);
    }

    free(config->policy_list);
    free(config);
}

#ifdef DEBUG_STREAM5
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
    LogMessage("    ts_last:            %lu\n", s->ts_last);
    LogMessage("    wscale:             %lu\n", s->wscale);
    LogMessage("    mss:                0x%08X\n", s->mss);
    LogMessage("    l_unackd:           %X\n", s->l_unackd);
    LogMessage("    l_nxt_seq:          %X\n", s->l_nxt_seq);
    LogMessage("    l_window:           %lu\n", s->l_window);
    LogMessage("    r_nxt_ack:          %X\n", s->r_nxt_ack);
    LogMessage("    r_win_base:         %X\n", s->r_win_base);
    LogMessage("    seglist_base_seq:   %X\n", s->seglist_base_seq);
    LogMessage("    seglist:            %p\n", s->seglist);
    LogMessage("    seglist_tail:       %p\n", s->seglist_tail);
    LogMessage("    seg_count:          %d\n", s->seg_count);
    LogMessage("    seg_bytes_total:    %d\n", s->seg_bytes_total);
    LogMessage("    seg_bytes_logical:  %d\n", s->seg_bytes_logical);

    PrintStateMgr(&s->s_mgr);
}

static void PrintTcpSession(TcpSession *ts)
{
    LogMessage("TcpSession:\n");
#ifdef DEBUG
    LogMessage("    ssn_time:           %lu\n", ts->ssn_time.tv_sec);
#endif
    LogMessage("    server IP:          0x%08X\n", ts->tcp_server_ip);
    LogMessage("    client IP:          0x%08X\n", ts->tcp_client_ip);
    LogMessage("    server port:        %d\n", ts->tcp_server_port);
    LogMessage("    client port:        %d\n", ts->tcp_client_port);

    LogMessage("    flags:              0x%X\n", ts->lwssn->session_flags);

    LogMessage("Client Tracker:\n");
    PrintStreamTracker(&ts->client);
    LogMessage("Server Tracker:\n");
    PrintStreamTracker(&ts->server);
}

static void PrintTcpDataBlock(TcpDataBlock *tdb)
{
    LogMessage("TcpDataBlock:\n");
    LogMessage("    sip:    0x%08X\n", tdb->sip);
    LogMessage("    dip:    0x%08X\n", tdb->dip);
    LogMessage("    seq:    0x%08X\n", tdb->seq);
    LogMessage("    ack:    0x%08X\n", tdb->ack);
    LogMessage("    win:    %d\n", tdb->win);
    LogMessage("    end:    0x%08X\n", tdb->end_seq);
}

#ifdef DEBUG
static void PrintFlushMgr(FlushMgr *fm)
{
    if(fm == NULL)
        return;

    switch(fm->flush_policy)
    {
        case STREAM_FLPOLICY_NONE: 
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    NONE\n"););
            break;
        case STREAM_FLPOLICY_FOOTPRINT:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    FOOTPRINT %d\n", fm->flush_pt););
            break;
        case STREAM_FLPOLICY_LOGICAL:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    LOGICAL %d\n", fm->flush_pt););
            break;
        case STREAM_FLPOLICY_RESPONSE:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    RESPONSE\n"););
            break;
        case STREAM_FLPOLICY_SLIDING_WINDOW:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    SLIDING_WINDOW %d\n", fm->flush_pt););
            break;
#if 0
        case STREAM_FLPOLICY_CONSUMED:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "          CONSUMED %d\n", fm->flush_pt););
            break;
#endif
        case STREAM_FLPOLICY_IGNORE:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "    IGNORE\n"););
            break;
    }
}
#endif  // DEBUG
#endif  // DEBUG_STREAM5

static INLINE void EventSynOnEst(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_SYN_ON_EST,                 /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_SYN_ON_EST_STR,             /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventExcessiveOverlap(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_EXCESSIVE_TCP_OVERLAPS,     /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_EXCESSIVE_TCP_OVERLAPS_STR, /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventBadTimestamp(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_BAD_TIMESTAMP,              /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_BAD_TIMESTAMP_STR,          /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventWindowTooLarge(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_WINDOW_TOO_LARGE,           /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_WINDOW_TOO_LARGE_STR,       /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventDataOnSyn(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_DATA_ON_SYN,                /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_DATA_ON_SYN_STR,            /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventDataOnClosed(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_DATA_ON_CLOSED,             /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_DATA_ON_CLOSED_STR,         /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventDataAfterReset(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_DATA_AFTER_RESET,           /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_DATA_AFTER_RESET_STR,       /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventBadSegment(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_BAD_SEGMENT,                /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_BAD_SEGMENT_STR,            /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventSessionHijackedClient(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_SESSION_HIJACKED_CLIENT,    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_SESSION_HIJACKED_CLIENT_STR, /* event msg */
            NULL);                              /* rule info ptr */
}
static INLINE void EventSessionHijackedServer(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_SESSION_HIJACKED_SERVER,    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_SESSION_HIJACKED_SERVER_STR, /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventDataWithoutFlags(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_DATA_WITHOUT_FLAGS,         /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_DATA_WITHOUT_FLAGS_STR,     /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventMaxSmallSegsExceeded(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_SMALL_SEGMENT,              /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_SMALL_SEGMENT_STR,          /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void Event4whs(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_4WAY_HANDSHAKE,             /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_4WAY_HANDSHAKE_STR,         /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventNoTimestamp(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_NO_TIMESTAMP,               /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_NO_TIMESTAMP_STR,           /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventBadReset(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_BAD_RST,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_BAD_RST_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventBadFin(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_BAD_FIN,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_BAD_FIN_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventBadAck(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_BAD_ACK,                    /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_BAD_ACK_STR,                /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventDataAfterRstRcvd(Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_DATA_AFTER_RST_RCVD,        /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_DATA_AFTER_RST_RCVD_STR,    /* event msg */
            NULL);                              /* rule info ptr */
}

static INLINE void EventInternal (uint32_t eventSid)
{
    if ( !InternalEventIsEnabled(snort_conf->rate_filter_config, eventSid) )
        return;

    s5stats.internalEvents++;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
        "Stream5 raised internal event %d\n", eventSid););

    SnortEventqAdd(GENERATOR_INTERNAL,          /* GID */
            eventSid,                           /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_INTERNAL_EVENT_STR,         /* event msg*/
            NULL);                              /* rule info ptr */
}

static INLINE void EventWindowSlam (Stream5TcpPolicy *s5TcpPolicy)
{
    if(!(s5TcpPolicy->flags & STREAM5_CONFIG_ENABLE_ALERTS))
        return;

    s5stats.events++;

    SnortEventqAdd(GENERATOR_SPP_STREAM5,       /* GID */
            STREAM5_WINDOW_SLAM,                /* SID */
            1,                                  /* rev */
            0,                                  /* class */
            3,                                  /* priority */
            STREAM5_WINDOW_SLAM_STR,            /* event msg */
            NULL);                              /* rule info ptr */
}

/*
 *  Utility functions for TCP stuff
 */
#ifdef NORMALIZER
typedef enum {
    PC_TCP_TRIM,
    PC_TCP_ECN_SSN,
    PC_TCP_TS_NOP,
    PC_TCP_IPS_DATA,
    PC_TCP_BLOCK,
    PC_MAX
} PegCounts;

static PegCount normStats[PC_MAX];

static const char* pegName[PC_MAX] = { 
    "tcp::trim",
    "tcp::ecn_ssn",
    "tcp::ts_nop",
    "tcp::ips_data",
    "tcp::block",
};

void Stream_PrintNormalizationStats (void)
{
    int i;
    // header output by normalizer

    for ( i = 0; i < PC_MAX; i++ )
    {
        // same alignment as in Norm_PrintStats()
        LogMessage("%23s: %lu\n", pegName[i], normStats[i]);
    }
}

void Stream_ResetNormalizationStats (void)
{
    memset(normStats, 0, sizeof(normStats));
}

//-----------------------------------------------------------------------
// instead of centralizing all these normalizations so that
// Normalize_IsEnabled() is called only once, the checks and
// normalizations are localized.  this should lead to many
// fewer total checks.  however, it is best to minimize
// configuration checks on a per packet basis so there is
// still room for improvement.
static INLINE void NormalDropPacket (Packet* p)
{
    Active_DropPacket();
}

static INLINE int NormalDropPacketIf (Packet* p, NormFlags f)
{
    if ( Normalize_IsEnabled(snort_conf, f) )
    {
        NormalDropPacket(p);
        normStats[PC_TCP_BLOCK]++;
        sfBase.iPegs[PERF_COUNT_TCP_BLOCK]++;
        return 1;
    }
    return 0;
}

static INLINE void NormalStripTimeStamp (Packet* p, int i)
{
    uint8_t* opt;

    if ( i < 0 )
    {
        for ( i = 0; i < p->tcp_option_count; i++ )
        {
            if ( p->tcp_options[i].code == TCPOPT_TIMESTAMP )
                break;
        }
        if ( i == p->tcp_option_count )
            return;
    }
    // first set raw option bytes to nops
    opt = (uint8_t*)p->tcp_options[i].data - 2;
    memset(opt, TCPOPT_NOP, TCPOLEN_TIMESTAMP);

    // then nop decoded option code only
    p->tcp_options[i].code = TCPOPT_NOP;

    p->packet_flags |= PKT_MODIFIED;
    normStats[PC_TCP_TS_NOP]++;
    sfBase.iPegs[PERF_COUNT_TCP_TS_NOP]++;
}

static INLINE void NormalTrimPayload (
    Packet* p, uint16_t max, TcpDataBlock* tdb
) {
    if ( p->dsize > max )
    {
        uint16_t fat = p->dsize - max;
        p->dsize = max;
        p->packet_flags |= (PKT_MODIFIED|PKT_RESIZED);
        tdb->end_seq -= fat;
        normStats[PC_TCP_TRIM]++;
        sfBase.iPegs[PERF_COUNT_TCP_TRIM]++;
    }
}

static INLINE int NormalTrimPayloadIf (
    Packet* p, NormFlags f, uint16_t max, TcpDataBlock* tdb
) {
    if ( 
        Normalize_IsEnabled(snort_conf, f) &&
        p->dsize > max )
    {
        NormalTrimPayload(p, max, tdb);
        return 1;
    }
    return 0;
}

static INLINE void NormalTrackECN (TcpSession* s, TCPHdr* tcph, int req3way)
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

static INLINE void NormalCheckECN (TcpSession* s, Packet* p)
{
    if ( !s->ecn && (p->tcph->th_flags & (TH_ECE|TH_CWR)) )
    {
        ((TCPHdr*)p->tcph)->th_flags &= ~(TH_ECE|TH_CWR);
        p->packet_flags |= PKT_MODIFIED;
        normStats[PC_TCP_ECN_SSN]++;
        sfBase.iPegs[PERF_COUNT_TCP_ECN_SSN]++;
    }
}
#else
#define NormalDropPacket(p)
#define NormalDropPacketIf(p, f)
#define NormalTrimPayloadIf(p, f, m, t)
#endif

static INLINE int IsBetween(uint32_t low, uint32_t high, uint32_t cur)
{
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "(%X, %X, %X) = (low, high, cur)\n", low,high,cur););

    /* If we haven't seen anything, ie, low & high are 0, return true */
    if ((low == 0) && (low == high))
        return 1;

    return (SEQ_GEQ(cur, low) && SEQ_LEQ(cur, high));
}           

static INLINE uint32_t Stream5GetWindow(StreamTracker *st)
{
    int32_t window;

    /* If we're in readback mode and haven't seen the other
     * side yet, window is r_next_ack - r_win_base.
     */
    if ((st->l_window == 0) && ScReadMode())
    {
        window = st->r_nxt_ack - st->r_win_base + 1;
    }
    else
    {
        //window = st->r_win_base + (st->l_window - st->r_nxt_ack);
        window = st->l_window;
    }

    if(window <  0)
        return 0;
    else
        return (uint32_t) window;
}

// ack number must ack syn
static INLINE int ValidRstSynSent(StreamTracker *st, TcpDataBlock *tdb)
{
    return tdb->ack == st->l_unackd;
}

// per rfc 793 a rst is valid if the seq number is in window
// for all states but syn-sent (handled above).  however, we
// validate here based on how various implementations actually
// handle a rst.
static INLINE int ValidRst(StreamTracker *st, TcpDataBlock *tdb)
{
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking end_seq (%X) > r_win_base (%X) && "
                "seq (%X) < r_nxt_ack(%X)\n", 
                tdb->end_seq, st->r_win_base, tdb->seq, 
                st->r_nxt_ack+Stream5GetWindow(st)););

    switch (st->os_policy)
    {
        case STREAM_POLICY_HPUX11:
            if (SEQ_GEQ(tdb->seq, st->r_nxt_ack))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "rst is valid seq (>= next seq)!\n"););
                return 1;
            }
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "rst is not valid seq (>= next seq)!\n"););
            return 0;
            break;
        case STREAM_POLICY_FIRST:
        case STREAM_POLICY_LAST:
        /* Uh, who knows */
        case STREAM_POLICY_BSD:
        case STREAM_POLICY_MACOS:
        case STREAM_POLICY_WINDOWS:
        case STREAM_POLICY_VISTA:
        case STREAM_POLICY_WINDOWS2K3:
        case STREAM_POLICY_HPUX10:
        case STREAM_POLICY_IRIX:
            if (SEQ_EQ(tdb->seq, st->r_nxt_ack))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "rst is valid seq (next seq)!\n"););
                return 1;
            }
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "rst is not valid seq (next seq)!\n"););
            return 0;
            break;
        case STREAM_POLICY_LINUX:
        case STREAM_POLICY_OLD_LINUX:
        case STREAM_POLICY_SOLARIS:
            if(SEQ_GEQ(tdb->end_seq, st->r_win_base))
            {
                if(SEQ_LT(tdb->seq, st->r_nxt_ack+Stream5GetWindow(st)))
                {
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "rst is valid seq (within window)!\n"););
                return 1;
                }
            }
            
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "rst is not valid seq (within window)!\n"););
            return 0;
            break;
    }

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
        "rst is not valid!\n"););
    return 0;
}

static INLINE int ValidTimestamp(StreamTracker *talker,
                                 StreamTracker *listener,
                                 TcpDataBlock *tdb,
                                 Packet *p,
                                 int *eventcode,
                                 int *got_ts)
{
    if (!eventcode || !got_ts)
        return ACTION_NOTHING;

    if(p->tcph->th_flags & TH_RST)
        return ACTION_NOTHING;

#ifdef NORMALIZER
#if 0
    if ( p->tcph->th_flags & TH_ACK &&
        Normalize_IsEnabled(snort_conf, NORM_TCP_OPT) )
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
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Checking timestamps for PAWS\n"););

        *got_ts = Stream5GetTcpTimestamp(p, &tdb->ts, 0);

        if (*got_ts)
        {
            if (listener->tcp_policy->policy == STREAM_POLICY_HPUX11)
            {
                /* HPUX 11 ignores timestamps for out of order segments */
                if ((listener->flags & TF_MISSING_PKT) ||
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

                if(result < 0)
                {
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                                "Packet outside PAWS window, dropping\n"););
                    /* bail, we've got a packet outside the PAWS window! */
                    //s5stats.tcp_discards++;
                    *eventcode |= EVENT_BAD_TIMESTAMP;
                    NormalDropPacketIf(p, NORM_TCP_OPT);
                    return ACTION_BAD_PKT;
                }
                else if ((talker->ts_last != 0) &&
                        ((uint32_t)p->pkth->ts.tv_sec > talker->ts_last_pkt+PAWS_24DAYS))
                {
                    /* this packet is from way too far into the future */
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                                "packet PAWS timestamp way too far ahead of"
                                "last packet %d %d...\n", p->pkth->ts.tv_sec,
                                talker->ts_last_pkt););
                    //s5stats.tcp_discards++;
                    *eventcode |= EVENT_BAD_TIMESTAMP;
                    NormalDropPacketIf(p, NORM_TCP_OPT);
                    return ACTION_BAD_PKT;
                }
                else
                {
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "packet no timestamp, had one earlier from this side...ok for now...\n"););

            if (listener->tcp_policy->policy == STREAM_POLICY_SOLARIS)
            {
                /* Solaris stops using timestamps if it receives a packet
                 * without a timestamp and there were timestamps in use.
                 */
                listener->flags &= ~TF_TSTAMP;
            }
            NormalDropPacketIf(p, NORM_TCP_OPT);
        }
    }
    else if ( TCP_ISFLAGSET(p->tcph, TH_SYN) &&
             !TCP_ISFLAGSET(p->tcph, TH_ACK) )
    {
        *got_ts = Stream5GetTcpTimestamp(p, &tdb->ts, 0);
        if ( *got_ts )
            talker->flags |= TF_TSTAMP;
    }
    else
    {
#ifdef NORMALIZER
        // if we are not handling timestamps, and this isn't a syn
        // (only), and we have seen a valid 3way setup, then we strip
        // (nop) the timestamp option.  this includes the cases where
        // we disable timestamp handling.
        int strip = ( (talker->s_mgr.sub_state == SUB_SETUP_OK) &&
                    (listener->s_mgr.sub_state == SUB_SETUP_OK) );
#else
        int strip = 0;
#endif
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "listener not doing timestamps...\n"););
        *got_ts = Stream5GetTcpTimestamp(p, &tdb->ts, strip);

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
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Packet with 0 timestamp, dropping\n"););
                    //s5stats.tcp_discards++;
                    /* bail */
                    *eventcode |= EVENT_BAD_TIMESTAMP;
                    return ACTION_BAD_PKT;
                }
            }
        }
    }
    return ACTION_NOTHING;
}

static INLINE int ValidSeq(StreamTracker *st, TcpDataBlock *tdb)
{
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking end_seq (%X) > r_win_base (%X) && "
                "seq (%X) < r_nxt_ack(%X)\n", 
                tdb->end_seq, st->r_win_base, tdb->seq, 
                st->r_nxt_ack+Stream5GetWindow(st)););

    if(SEQ_GEQ(tdb->end_seq, st->r_win_base))
    {
        if(SEQ_LT(tdb->seq, st->r_nxt_ack+Stream5GetWindow(st)))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "seq is within window!\n"););
            return 1;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "seq is past the end of the window!\n"););
        }
    }
    else
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "end_seq is before win_base\n"););
    }
    return 0;
}

static INLINE void UpdateSsn(
    Packet* p, StreamTracker *rcv, StreamTracker *snd, TcpDataBlock *tdb)
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
         Normalize_IsEnabled(snort_conf, NORM_TCP_IPS) )
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
    // FIXTHIS first test below is implied by second; verify test suite 
    // doesn't break
    if(SEQ_GT(tdb->ack, rcv->l_unackd) && SEQ_GT(tdb->ack, rcv->l_nxt_seq))
        rcv->l_unackd = tdb->ack;

    // ** this is how we track the last seq number sent
    // as is l_unackd is the "last left" seq recvd
    snd->l_unackd = tdb->seq;

    if (SEQ_GT(tdb->end_seq, snd->l_nxt_seq))
        snd->l_nxt_seq = tdb->end_seq;

    if (!SEQ_EQ(snd->r_win_base, tdb->ack))
    {
        snd->small_seg_count = 0;
    }
    snd->r_win_base = tdb->ack;
    snd->l_window = tdb->win;
}

static void Stream5InitPacket(void)
{
    s5_pkt = Encode_New();
}

static INLINE void SetupTcpDataBlock(TcpDataBlock *tdb, Packet *p)
{
#ifdef SUP_IP6
    tdb->sip = *GET_SRC_IP(p);
    tdb->dip = *GET_DST_IP(p);
    if(IS_IP4(p))
    {
        *tdb->sip.ip32 = ntohl(*tdb->sip.ip32);
        *tdb->dip.ip32 = ntohl(*tdb->dip.ip32);
    } 
#else
    tdb->sip = ntohl(p->iph->ip_src.s_addr);
    tdb->dip = ntohl(p->iph->ip_dst.s_addr);
#endif
    tdb->seq = ntohl(p->tcph->th_seq);
    tdb->ack = ntohl(p->tcph->th_ack);
    tdb->win = ntohs(p->tcph->th_win);
    tdb->end_seq = tdb->seq + (uint32_t) p->dsize;
    tdb->ts = 0;

    if(p->tcph->th_flags & TH_SYN)
    {
        tdb->end_seq++;
        if(!(p->tcph->th_flags & TH_ACK))
            EventInternal(INTERNAL_EVENT_SYN_RECEIVED);
    }
    // don't bump end_seq for fin here
    // we will bump if/when fin is processed

    return;
}

static void Stream5DropSegment(StreamSegment *seg)
{
    int dropped = 0;

    if(seg != NULL)
    {
        STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                        "Dumping segment at seq %X, size %d, caplen %d\n", 
                        seg->seq, seg->size, seg->caplen););

        if(seg->pktOrig != NULL)
        {
            mem_in_use -= seg->caplen;
            dropped += seg->caplen;
            free(seg->pktOrig);
            seg->pktOrig = NULL;
        }

        mem_in_use -= sizeof(StreamSegment);
        dropped += sizeof(StreamSegment);
        free(seg);
        s5stats.tcp_streamsegs_released++;
    }

    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "Stream5DropSegment dropped %d bytes\n", dropped););
}

static void DeleteSeglist(StreamSegment *listhead)
{
    StreamSegment *idx = listhead;
    StreamSegment *dump_me;
    int i = 0;

    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "In DeleteSeglist\n"););
    while(idx)
    {
        i++;
        dump_me = idx;
        idx = idx->next;
        Stream5DropSegment(dump_me);
    }

    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "Dropped %d segments\n", i););
}

static INLINE int purge_alerts(StreamTracker *st, uint32_t flush_seq)
{
    int i;
    int new_count = 0;

    for (i=0;i<st->alert_count;i++)
    {
        uint32_t alert_seq = ntohl(st->alerts[i].seq);
        if (SEQ_LT(alert_seq,flush_seq) )
        {
            st->alerts[i].sid = 0;
            st->alerts[i].gid = 0;
            st->alerts[i].seq = 0;
        }
        else
        {
            if (new_count != i)
            {
                st->alerts[new_count].sid = st->alerts[i].sid;
                st->alerts[new_count].gid = st->alerts[i].gid;
                st->alerts[new_count].seq = st->alerts[i].seq;
            }
            new_count++;
        }
    }
    st->alert_count = new_count;

    return new_count;
}

static INLINE int purge_to_seq(TcpSession *tcpssn, StreamTracker *st, uint32_t flush_seq)
{
    StreamSegment *ss = NULL;
    StreamSegment *dump_me = NULL;
    int purged_bytes = 0;
    uint32_t last_ts = 0;

    if(st->seglist == NULL)
    {
        if ( SEQ_LT(st->seglist_base_seq, flush_seq) )
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "setting st->seglist_base_seq to 0x%X\n", flush_seq););
            st->seglist_base_seq = flush_seq;
        }
        return 0;
    }

    ss = st->seglist;
    
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "In purge_to_seq, start seq = 0x%X end seq = 0x%X delta %d\n", 
                ss->seq, flush_seq, flush_seq-ss->seq););
    while(ss)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "s: %X  sz: %d\n", ss->seq, ss->size););
        dump_me = ss;

        ss = ss->next;
        if(SEQ_LT(dump_me->seq, flush_seq))
        {
            if (dump_me->ts > last_ts)
            {
                last_ts = dump_me->ts;
            }
            purged_bytes += Stream5SeglistDeleteNode(st, dump_me);
        }
        else
            break;
    }

    if ( SEQ_LT(st->seglist_base_seq, flush_seq) )
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
            "setting st->seglist_base_seq to 0x%X\n", flush_seq););
        st->seglist_base_seq = flush_seq;
    }
#ifdef NORMALIZER
    if ( !Normalize_IsEnabled(snort_conf, NORM_TCP_IPS) )
#endif
        st->r_nxt_ack = flush_seq;

    purge_alerts(st, flush_seq);

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

static INLINE void purge_all (StreamTracker *st)
{
    DeleteSeglist(st->seglist);
    st->seglist = st->seglist_tail = NULL;
    st->seg_count = st->flush_count = 0;
}

/*
 * purge a seglist up to the the last ack received
 */
static INLINE int purge_ackd(TcpSession *tcpssn, StreamTracker *st)
{
    return purge_to_seq(tcpssn, st, st->r_win_base);
}

#ifdef SUP_IP6
static INLINE int _flush_to_seq_4(
    TcpSession *tcpssn, StreamTracker *st, uint32_t bytes, Packet *p, 
    snort_ip_p sip, snort_ip_p dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t stop_seq;
    uint32_t footprint = 0;
    uint32_t bytes_processed = 0;
    int32_t flushed_bytes;
    EncodeFlags enc_flags = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpFlushPerfStats);

    if ( htons(sp) == p->sp ) enc_flags |= ENC_FLAG_FWD;
    Encode_Format(enc_flags, p, s5_pkt);

    s5_pkt_end = s5_pkt->data + s5_pkt->max_dsize;

    if ( st->seglist_next )
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist_next->seq);
    else if ( st->seglist )
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist->seq);
    else
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist_base_seq);

    ((TCPHdr *)s5_pkt->tcph)->th_ack = htonl(st->l_unackd);
    ((TCPHdr *)s5_pkt->tcph)->th_win = htons((uint16_t)st->l_window);

    // if not specified, set bytes to flush to what was acked
    if ( !bytes ) bytes = st->r_win_base - st->seglist_base_seq;
    stop_seq = st->seglist_base_seq + bytes;

    do
    {
        footprint = stop_seq - st->seglist_base_seq;

        if(footprint == 0) 
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Negative footprint, bailing %d (0x%X - 0x%X)\n", 
                        footprint, stop_seq, st->seglist_base_seq););
            PREPROC_PROFILE_END(s5TcpFlushPerfStats);

            return bytes_processed;
        }

#ifdef DEBUG
        if(footprint < st->seg_bytes_logical)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Footprint less than queued bytes, "
                        "win_base: 0x%X base_seq: 0x%X\n", 
                        stop_seq, st->seglist_base_seq););
        }
#endif

        if(footprint > STREAM_MAX_PACKET)
        {
            /* this is as much as we can pack into a stream buffer */
            footprint = STREAM_MAX_PACKET;
        }

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attempting to flush %lu bytes\n", footprint););

        /* setup the pseudopacket payload */
        flushed_bytes = FlushStream(st, stop_seq, (uint8_t *)s5_pkt->data, s5_pkt_end);

        if(flushed_bytes == -1)
        {
            /* couldn't put a stream together for whatever reason
             * should probably clean the seglist and bail...
             */
            if(st->seglist)
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "dumping entire seglist!\n"););
                purge_all(st);
            }

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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

        s5_pkt->packet_flags = (PKT_REBUILT_STREAM|PKT_STREAM_EST);
        s5_pkt->dsize = (uint16_t)flushed_bytes;
        
        Encode_Update(s5_pkt);
        s5_pkt->inner_ip4h.ip_len = s5_pkt->iph->ip_len;

        if (st->seglist_next != NULL)
        {
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_sec = st->seglist_next->pkth.ts.tv_sec;
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_usec = st->seglist_next->pkth.ts.tv_usec;
        }
        else if (st->seglist != NULL)
        {
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_sec = st->seglist->pkth.ts.tv_sec;
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_usec = st->seglist->pkth.ts.tv_usec;
        }

        sfBase.iStreamFlushes++;

        bytes_processed += s5_pkt->dsize;

        s5_pkt->packet_flags |= dir;
        s5_pkt->ssnptr = (void *) tcpssn->lwssn;
#ifdef TARGET_BASED
        s5_pkt->application_protocol_ordinal = p->application_protocol_ordinal;
#endif
        //s5_pkt->streamptr = (void *) st;

        if (st->flags & TF_MISSING_PKT)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "setting st->seglist_base_seq to 0x%X (gap)\n", st->gap_seq););
            st->seglist_base_seq = st->gap_seq;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "setting st->seglist_base_seq to 0x%X\n", stop_seq););
            st->seglist_base_seq = stop_seq;
        }

        if(s5_global_eval_config->flags & STREAM5_CONFIG_SHOW_PACKETS)
        {
            //ClearDumpBuf();
            printf("+++++++++++++++++++Stream Packet+++++++++++++++++++++\n");
            PrintIPPkt(stdout, IPPROTO_TCP, s5_pkt);
            printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            //ClearDumpBuf();
        }

        s5stats.tcp_rebuilt_packets++; 

        PREPROC_PROFILE_TMPEND(s5TcpFlushPerfStats);
        {
            int event, tmp_do_detect, tmp_do_detect_content;
            PROFILE_VARS;
            
            PREPROC_PROFILE_START(s5TcpProcessRebuiltPerfStats);
            tmp_do_detect = do_detect;
            tmp_do_detect_content = do_detect_content;

            SnortEventqPush();
            event = Preprocess(s5_pkt);
            SnortEventqPop();
            DetectReset();

            do_detect = tmp_do_detect;
            do_detect_content = tmp_do_detect_content;
            PREPROC_PROFILE_END(s5TcpProcessRebuiltPerfStats);

            if(event)
            {
                //LogStream(s);
            }
        }
        PREPROC_PROFILE_TMPSTART(s5TcpFlushPerfStats);

        /* Reset alert tracking after flushing rebuilt packet */

    } while (!(st->flags & TF_MISSING_PKT) && (SegsToFlush(st) > 1));

    /* Grab the next random flush point */
    // FIXTHIS should we get next random flush pt per flush or per session?
    //UpdateFlushMgr(&st->flush_mgr);

    /* tell them how many bytes we processed */
    PREPROC_PROFILE_END(s5TcpFlushPerfStats);
    return bytes_processed;
}


static INLINE int _flush_to_seq_6(
    TcpSession *tcpssn, StreamTracker *st, uint32_t bytes, Packet *p, 
    snort_ip_p sip, snort_ip_p dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t base_seq;
    uint32_t stop_seq;
    uint32_t footprint = 0;
    uint32_t bytes_processed = 0;
    int32_t flushed_bytes;
    EncodeFlags enc_flags = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpFlushPerfStats);
    
    if ( htons(sp) == p->sp ) enc_flags |= ENC_FLAG_FWD;
    Encode_Format(enc_flags, p, s5_pkt);

    s5_pkt_end = s5_pkt->data + s5_pkt->max_dsize;

    if ( st->seglist_next )
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist_next->seq);
    else if ( st->seglist )
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist->seq);
    else
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist_base_seq);

    ((TCPHdr *)s5_pkt->tcph)->th_ack = htonl(st->l_unackd);
    ((TCPHdr *)s5_pkt->tcph)->th_win = htons((uint16_t)st->l_window);

    // for now, we allow bytes == 0 for flushing acked
    // otherwise, bytes is how many to ack
    if ( !bytes ) bytes = st->r_win_base - st->seglist_base_seq;
    stop_seq = st->seglist_base_seq + bytes;

    do
    {
        base_seq = st->seglist_base_seq; 

        footprint = stop_seq - base_seq;

        if(footprint <= 0) 
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Negative footprint, bailing %d (0x%X - 0x%X)\n", 
                        footprint, stop_seq, base_seq););
            PREPROC_PROFILE_END(s5TcpFlushPerfStats);

            return bytes_processed;
        }

#ifdef DEBUG
        if(footprint < st->seg_bytes_logical)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Footprint less than queued bytes, "
                        "win_base: 0x%X base_seq: 0x%X\n", 
                        stop_seq, base_seq););
        }
#endif

        if(footprint > STREAM_MAX_PACKET6)
        {
            /* this is as much as we can pack into a stream buffer */
            footprint = STREAM_MAX_PACKET6;
        }

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attempting to flush %lu bytes\n", footprint););

        /* setup the pseudopacket payload */
        flushed_bytes = FlushStream(st, stop_seq, (uint8_t *)s5_pkt->data, s5_pkt_end);

        if(flushed_bytes == -1)
        {
            /* couldn't put a stream together for whatever reason
             * should probably clean the seglist and bail...
             */
            if(st->seglist)
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "dumping entire seglist!\n"););
                purge_all(st);
            }

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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

        s5_pkt->packet_flags = (PKT_REBUILT_STREAM|PKT_STREAM_EST);
        s5_pkt->dsize = (uint16_t)flushed_bytes;

        Encode_Update(s5_pkt);
        {
            IP6RawHdr* ip6h = (IP6RawHdr*)s5_pkt->raw_ip6h;
            if ( ip6h ) s5_pkt->inner_ip6h.len = ip6h->ip6plen;
        }
        if (st->seglist_next != NULL)
        {
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_sec = st->seglist_next->pkth.ts.tv_sec;
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_usec = st->seglist_next->pkth.ts.tv_usec;
        }
        else if (st->seglist != NULL)
        {
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_sec = st->seglist->pkth.ts.tv_sec;
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_usec = st->seglist->pkth.ts.tv_usec;
        }

        sfBase.iStreamFlushes++;

        bytes_processed += s5_pkt->dsize;

        s5_pkt->packet_flags |= dir;
        s5_pkt->ssnptr = (void *) tcpssn->lwssn;
#ifdef TARGET_BASED
        s5_pkt->application_protocol_ordinal = p->application_protocol_ordinal;
#endif
        //s5_pkt->streamptr = (void *) st;

        if (st->flags & TF_MISSING_PKT)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "setting st->seglist_base_seq to 0x%X (gap)\n", st->gap_seq););
            st->seglist_base_seq = st->gap_seq;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "setting st->seglist_base_seq to 0x%X\n", stop_seq););
            st->seglist_base_seq = stop_seq;
        }

        if(s5_global_eval_config->flags & STREAM5_CONFIG_SHOW_PACKETS)
        {
            //ClearDumpBuf();
            printf("+++++++++++++++++++Stream Packet+++++++++++++++++++++\n");
            PrintIPPkt(stdout, IPPROTO_TCP, s5_pkt);
            printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            //ClearDumpBuf();
        }

        s5stats.tcp_rebuilt_packets++; 

        PREPROC_PROFILE_TMPEND(s5TcpFlushPerfStats);
        {
            int event, tmp_do_detect, tmp_do_detect_content;
            PROFILE_VARS;
            
            PREPROC_PROFILE_START(s5TcpProcessRebuiltPerfStats);
            tmp_do_detect = do_detect;
            tmp_do_detect_content = do_detect_content;

            SnortEventqPush();
            event = Preprocess(s5_pkt);
            SnortEventqPop();
            DetectReset();

            do_detect = tmp_do_detect;
            do_detect_content = tmp_do_detect_content;
            PREPROC_PROFILE_END(s5TcpProcessRebuiltPerfStats);

            if(event)
            {
                //LogStream(s);
            }
        }
        PREPROC_PROFILE_TMPSTART(s5TcpFlushPerfStats);

        /* Reset alert tracking after flushing rebuilt packet */

    } while (!(st->flags & TF_MISSING_PKT) && (SegsToFlush(st) > 1));

    /* Grab the next random flush point */
    // FIXTHIS should we get next random flush pt per flush or per session?
    //UpdateFlushMgr(&st->flush_mgr);

    /* tell them how many bytes we processed */
    PREPROC_PROFILE_END(s5TcpFlushPerfStats);
    return bytes_processed;
}
#endif /* SUP_IP6 */

/*
 * get the footprint for the current seglist, the difference
 * between our base sequence and the last ack'd sequence we
 * received
 */
static INLINE uint32_t get_q_footprint(StreamTracker *st)
{
    uint32_t fp;

    if (st == NULL)
    {
        return 0;
    }

    fp = st->r_win_base - st->seglist_base_seq;

    if(fp <= 0)
        return 0;

    return fp;
}

// FIXTHIS get_q_sequenced() performance could possibly be
// boosted by tracking sequenced bytes as seglist is updated
// to avoid the while loop, etc. below.
static INLINE uint32_t get_q_sequenced(StreamTracker *st)
{
    uint32_t len;
    StreamSegment* seg = st ? st->seglist : NULL;
    StreamSegment* base = NULL;

    if ( !seg )
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

/*
 * flush a seglist up to the given point, generate a pseudopacket,
 * and fire it thru the system.
 */
static INLINE int flush_to_seq(
    TcpSession *tcpssn, StreamTracker *st, uint32_t bytes, Packet *p, 
    snort_ip_p sip, snort_ip_p dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t bytes_processed = 0;

#ifndef SUP_IP6
    uint32_t base_seq;
    uint32_t stop_seq;
    uint32_t footprint = 0;
    int32_t flushed_bytes;
    EncodeFlags enc_flags = 0;
    PROFILE_VARS;
#endif

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In flush_to_seq()\n"););

    if(st->seg_bytes_logical == 0)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bailing, no data\n"););
        return bytes_processed;
    }

    if(st->seglist == NULL || st->seglist_tail == NULL)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "bailing, bad seglist ptr\n"););
        return bytes_processed;
    }

    if ((SegsToFlush(st) < 2) && !(st->flags & TF_FORCE_FLUSH))
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "only 1 packet in seglist no need to flush\n"););
        return bytes_processed;
    }

    //PrintSeglist(st);

#ifdef SUP_IP6
    if(sfip_family(sip) == AF_INET) 
    {
        return _flush_to_seq_4(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
    }

    return _flush_to_seq_6(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
    
#else
    PREPROC_PROFILE_START(s5TcpFlushPerfStats);

    if ( htons(sp) == p->sp ) enc_flags |= ENC_FLAG_FWD;
    Encode_Format(enc_flags, p, s5_pkt);

    s5_pkt_end = s5_pkt->data + s5_pkt->max_dsize;

    if ( st->seglist_next )
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist_next->seq);
    else if ( st->seglist )
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist->seq);
    else
        ((TCPHdr *)s5_pkt->tcph)->th_seq = htonl(st->seglist_base_seq);

    ((TCPHdr *)s5_pkt->tcph)->th_ack = htonl(st->l_unackd);
    ((TCPHdr *)s5_pkt->tcph)->th_win = htons((uint16_t)st->l_window);

    stop_seq = st->seglist_base_seq + bytes;

    do
    {
        base_seq = st->seglist_base_seq; 

        footprint = stop_seq - base_seq;

        // FIXTHIS is this check redundant?  should delete
        // cuz FlushStream() should catch any issues
        if(footprint <= 0) 
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Negative footprint, bailing %d (0x%X - 0x%X)\n", 
                        footprint, stop_seq, base_seq););
            PREPROC_PROFILE_END(s5TcpFlushPerfStats);

            return bytes_processed;
        }

#ifdef DEBUG
        if(footprint < st->seg_bytes_logical)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Footprint less than queued bytes, "
                        "win_base: 0x%X base_seq: 0x%X\n", 
                        stop_seq, base_seq););
        }
#endif

        if(footprint > STREAM_MAX_PACKET)
        {
            /* this is as much as we can pack into a stream buffer */
            footprint = STREAM_MAX_PACKET;
        }

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attempting to flush %lu bytes\n", footprint););

        /* setup the pseudopacket payload */
        flushed_bytes = FlushStream(st, stop_seq, (uint8_t *)s5_pkt->data, s5_pkt_end);

        if(flushed_bytes == -1)
        {
            /* couldn't put a stream together for whatever reason
             * should probably clean the seglist and bail...
             */
            if(st->seglist)
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "dumping entire seglist!\n"););
                purge_all(st);
            }

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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

        s5_pkt->packet_flags = (PKT_REBUILT_STREAM|PKT_STREAM_EST);
        s5_pkt->dsize = (uint16_t)flushed_bytes;

        Encode_Update(s5_pkt);

        if (st->seglist_next != NULL)
        {
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_sec = st->seglist_next->pkth.ts.tv_sec;
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_usec = st->seglist_next->pkth.ts.tv_usec;
        }
        else if (st->seglist != NULL)
        {
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_sec = st->seglist->pkth.ts.tv_sec;
            ((DAQ_PktHdr_t *)s5_pkt->pkth)->ts.tv_usec = st->seglist->pkth.ts.tv_usec;
        }

        sfBase.iStreamFlushes++;
        bytes_processed += s5_pkt->dsize;

        s5_pkt->packet_flags |= dir;
        s5_pkt->ssnptr = (void *) tcpssn->lwssn;
#ifdef TARGET_BASED
        s5_pkt->application_protocol_ordinal = p->application_protocol_ordinal;
#endif
        //s5_pkt->streamptr = (void *) st;

        if (st->flags & TF_MISSING_PKT)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "setting st->seglist_base_seq to 0x%X (gap)\n", st->gap_seq););
            st->seglist_base_seq = st->gap_seq;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "setting st->seglist_base_seq to 0x%X\n", stop_seq););
            st->seglist_base_seq = stop_seq;
        }

        if(s5_global_eval_config->flags & STREAM5_CONFIG_SHOW_PACKETS)
        {
            //ClearDumpBuf();
            printf("+++++++++++++++++++Stream Packet+++++++++++++++++++++\n");
            PrintIPPkt(stdout, IPPROTO_TCP, s5_pkt);
            printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            //ClearDumpBuf();
        }

        s5stats.tcp_rebuilt_packets++; 

        PREPROC_PROFILE_TMPEND(s5TcpFlushPerfStats);
        {
            int event, tmp_do_detect, tmp_do_detect_content;
            PROFILE_VARS;
            
            PREPROC_PROFILE_START(s5TcpProcessRebuiltPerfStats);
            tmp_do_detect = do_detect;
            tmp_do_detect_content = do_detect_content;

            SnortEventqPush();
            event = Preprocess(s5_pkt);
            SnortEventqPop();
            DetectReset();

            do_detect = tmp_do_detect;
            do_detect_content = tmp_do_detect_content;
            PREPROC_PROFILE_END(s5TcpProcessRebuiltPerfStats);

            if(event)
            {
                //LogStream(s);
            }
        }
        PREPROC_PROFILE_TMPSTART(s5TcpFlushPerfStats);

        /* Reset alert tracking after flushing rebuilt packet */

    } while (!(st->flags & TF_MISSING_PKT) && (SegsToFlush(st) > 1));

    /* Grab the next random flush point */
    // FIXTHIS should we get next random flush pt per flush or per session?
    //UpdateFlushMgr(&st->flush_mgr);

    /* tell them how many bytes we processed */
    PREPROC_PROFILE_END(s5TcpFlushPerfStats);
    return bytes_processed;
#endif
}

static INLINE int flush_ackd(
    TcpSession *tcpssn, StreamTracker *st, Packet *p, 
    snort_ip_p sip, snort_ip_p dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
    uint32_t bytes = get_q_footprint(st);
    return flush_to_seq(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
}
 
static INLINE int flush_stream(
    TcpSession *tcpssn, StreamTracker *st, Packet *p, 
    snort_ip_p sip, snort_ip_p dip, uint16_t sp, uint16_t dp, uint32_t dir)
{
#ifdef NORMALIZER
    if ( Normalize_IsEnabled(snort_conf, NORM_TCP_IPS) )
    {
        uint32_t bytes = get_q_sequenced(st);
        return flush_to_seq(tcpssn, st, bytes, p, sip, dip, sp, dp, dir);
    }
#endif
    return flush_ackd(tcpssn, st, p, sip, dip, sp, dp, dir);
}
 
static INLINE unsigned int getSegmentFlushSize(
        StreamSegment *ss,
        uint32_t base_seq,
        unsigned int flushBufSize
        )
{
    unsigned int flushSize = ss->size;

    //copy only till flush buffer gets full
    if(((ss->seq - base_seq) + flushSize) > flushBufSize)
        flushSize = flushBufSize - (ss->seq - base_seq);

#if 0
    printf("FLUSHSZ=%u, base=%u, seq=%u, diff=%d\n",
        flushSize, base_seq, ss->seq, (int)ss->seq - (int)base_seq);
#endif

    return flushSize;
}

/*
 * flush the client seglist up to the most recently acked segment
 */
static int FlushStream(
    StreamTracker *st, uint32_t toSeq, uint8_t *flushbuf, const uint8_t *flushbuf_end)
{
    StreamSegment *ss = NULL, *seglist;
    uint32_t base_seq;
    uint16_t bytes_flushed = 0;
    uint16_t bytes_skipped = 0;
    uint32_t bytes_queued = st->seg_bytes_logical;
    uint32_t segs = 0;
    uint32_t flushbuf_size = flushbuf_end - flushbuf;
    int ret;
    PROFILE_VARS;

    if(SegsToFlush(st) == 0 || st->seglist == NULL || st->seglist_tail == NULL)
        return -1;

    PREPROC_PROFILE_START(s5TcpBuildPacketPerfStats);

    // skip over previously flushed segments
    for(seglist = st->seglist; seglist && seglist->buffered; seglist = seglist->next);
    if ( !seglist )
        return 0;

    base_seq = seglist->seq;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Flushing stream, starting seq %X, toSeq: %X "
                "base_seq: %X seglist: %p seglist->seq: %X\n", 
                base_seq, toSeq, base_seq, seglist, seglist->seq););

    st->flags &= ~TF_MISSING_PKT;
    st->flags &= ~TF_MISSING_PREV_PKT;
    st->gap_seq = 0;

    /* This will falsely set this flag on the first reassembly
     * if reassembly for this direction was set midstream */
    // FIXTHIS is missing prev pkt condition valid when seglist != st->seglist?
    if ( st->seglist == seglist && st->seglist_base_seq < seglist->seq)
        st->flags |= TF_MISSING_PREV_PKT;

    for(ss = seglist; ss && SEQ_LT(ss->seq,  toSeq); ss = ss->next)
    {
        unsigned int  bytes_to_copy;

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "seq: 0x%X  offset: %d  size: %d\n", ss->seq, 
                    ss->seq-base_seq, ss->size););

        bytes_to_copy = getSegmentFlushSize(ss, base_seq, flushbuf_size);

        /* Check for a gap/missing packet */
        if ((ss->next && (ss->seq + ss->size != ss->next->seq)) ||
            (!ss->next && (ss->seq + ss->size < toSeq)))
        {
            st->flags |= TF_MISSING_PKT;
            st->flags |= TF_PKT_MISSED;

            if (ss->next)
                st->gap_seq = ss->next->seq;
            else
                st->gap_seq = toSeq;

            s5stats.tcp_gaps++;
        }

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "s: %p e: %p dlt: %lu off: %d d: %p sz: %d\n", 
                    flushbuf, flushbuf_end, flushbuf_size, 
                    ss->seq-base_seq, ss->payload, ss->size););

        if(ss->urg_offset == 1)
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
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "ERROR writing flushbuf attempting to "
                                "write flushbuf out of range!\n"););
                }
                else
                    flushbuf += non_urgent_bytes;

                bytes_skipped += ss->urg_offset;
            }
        }
        else
        {
            ret = SafeMemcpy(flushbuf, ss->payload, 
                      bytes_to_copy, flushbuf, flushbuf_end);

            if (ret == SAFEMEM_ERROR)
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "ERROR writing flushbuf attempting to "
                            "write flushbuf out of range!\n"););
            }
            else
                flushbuf += bytes_to_copy;
        }

        if (bytes_to_copy < ss->size) 
        {
            bytes_flushed += bytes_to_copy;
            ss->seq = toSeq;
            ss->size -= bytes_to_copy;
            ss->payload += bytes_to_copy;
        }
        else
        {
            bytes_flushed += ss->size;
            ss->buffered = SL_BUF_FLUSHED;
            st->flush_count++;
        }
        segs++;

        /* Was there a gap following this segment? */
        if (st->flags & TF_MISSING_PKT)
            break;

        if(((ss->seq - base_seq) + bytes_to_copy) >= flushbuf_size)
            break;
    }

    bytes_queued -= bytes_flushed;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
        "flushed %d bytes / %d segs on stream, "
        "skipped %d bytes, %d still queued\n",
        bytes_flushed, segs, bytes_skipped, bytes_queued););

    PREPROC_PROFILE_END(s5TcpBuildPacketPerfStats);
    return bytes_flushed - bytes_skipped;
}

int Stream5FlushServer(Packet *p, Stream5LWSession *lwssn)
{
    int flushed;
    TcpSession *tcpssn = NULL;
    StreamTracker *flushTracker = NULL;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

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
#ifdef SUP_IP6
                            &tcpssn->tcp_server_ip,
                            &tcpssn->tcp_client_ip,
#else
                            tcpssn->tcp_server_ip,
                            tcpssn->tcp_client_ip,
#endif
                            tcpssn->tcp_server_port,
                            tcpssn->tcp_client_port,
                            PKT_FROM_SERVER);
    if (flushed)
        purge_ackd(tcpssn, flushTracker);

    flushTracker->flags &= ~TF_FORCE_FLUSH;

    return flushed;
}

int Stream5FlushClient(Packet *p, Stream5LWSession *lwssn)
{
    int flushed;
    TcpSession *tcpssn = NULL;
    StreamTracker *flushTracker = NULL;
   
    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

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
#ifdef SUP_IP6
                            &tcpssn->tcp_client_ip,
                            &tcpssn->tcp_server_ip,
#else
                            tcpssn->tcp_client_ip,
                            tcpssn->tcp_server_ip,
#endif
                            tcpssn->tcp_client_port,
                            tcpssn->tcp_server_port,
                            PKT_FROM_CLIENT);
    if (flushed)
        purge_ackd(tcpssn, flushTracker);

    flushTracker->flags &= ~TF_FORCE_FLUSH;

    return flushed;
}

int Stream5FlushListener(Packet *p, Stream5LWSession *lwssn)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *listener = NULL;
    int dir = 0;
    int flushed = 0;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return 0;

    /* figure out direction of this packet -- we should've already
     * looked at it, so the packet_flags are already set. */
    if(p->packet_flags & PKT_FROM_SERVER)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Flushing listener on packet from server\n"););
        listener = &tcpssn->client;
        /* dir of flush is the data from the opposite side */
        dir = PKT_FROM_SERVER;
    }
    else if (p->packet_flags & PKT_FROM_CLIENT)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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
            purge_ackd(tcpssn, listener);
        listener->flags &= ~TF_FORCE_FLUSH;
    }

    return flushed;
}

int Stream5FlushTalker(Packet *p, Stream5LWSession *lwssn)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *talker = NULL;
    int dir = 0;
    int flushed = 0;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

    /* figure out direction of this packet -- we should've already
     * looked at it, so the packet_flags are already set. */
    if(p->packet_flags & PKT_FROM_SERVER)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Flushing talker on packet from server\n"););
        talker = &tcpssn->server;
        /* dir of flush is the data from the opposite side */
        dir = PKT_FROM_CLIENT;
    }
    else if (p->packet_flags & PKT_FROM_CLIENT)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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
            purge_ackd(tcpssn, talker);

        talker->flags &= ~TF_FORCE_FLUSH;
    }

    return flushed;
}

Stream5LWSession *GetLWTcpSession(SessionKey *key)
{
    return GetLWSessionFromKey(tcp_lws_cache, key);
}

void TcpSessionCleanup(Stream5LWSession *lwssn)
{
    DAQ_PktHdr_t tmp_pcap_hdr;
    TcpSession *tcpssn = NULL;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
    {
        /* Huh? */
        Stream5UpdatePerfBaseState(&sfBase, lwssn, TCP_STATE_CLOSED);
        return;
    }

    /* Flush ack'd data on both sides as necessary */
    {
        Packet p;
        int flushed;

        if (!s5_tcp_cleanup)
        {
            /* Turn off decoder alerts since we're decoding stored
             * packets that we already alerted on. */
            policyDecoderFlagsSaveNClear(lwssn->policy_id);
        }

        /* Flush the client */
        if (tcpssn->client.seglist && !(lwssn->ignore_direction & SSN_DIR_SERVER) )
        {
            pc.s5tcp1++;
            /* Do each field individually because of size differences on 64bit OS */
            tmp_pcap_hdr.ts.tv_sec = tcpssn->client.seglist->pkth.ts.tv_sec;
            tmp_pcap_hdr.ts.tv_usec = tcpssn->client.seglist->pkth.ts.tv_usec;
            tmp_pcap_hdr.caplen = tcpssn->client.seglist->pkth.caplen - SPARC_TWIDDLE;
            tmp_pcap_hdr.pktlen = tcpssn->client.seglist->pkth.pktlen - SPARC_TWIDDLE;

            SnortEventqPush();
            (*grinder)(&p, &tmp_pcap_hdr,
                       tcpssn->client.seglist->pktOrig + SPARC_TWIDDLE);
            p.ssnptr = lwssn;

            //set policy id for this packet
            {
                int vlanId = (p.vh)? VTH_VLAN(p.vh) : -1;
                snort_ip_p srcIp = (p.iph)? GET_SRC_IP((&p)) : (snort_ip_p)0;
                snort_ip_p dstIp = (p.iph)? GET_DST_IP((&p)) : (snort_ip_p)0;

                setRuntimePolicy(sfGetApplicablePolicyId(
                                    snort_conf->policy_config, vlanId, srcIp, dstIp));
                p.configPolicyId =
                    snort_conf->targeted_policies[getRuntimePolicy()]->configPolicyId;

                //actions are queued only for IDS case
                sfActionQueueExecAll(decoderActionQ);
            }
            SnortEventqPop();

            tcpssn->client.flags |= TF_FORCE_FLUSH;

#ifdef SUP_IP6
            flushed = flush_stream(tcpssn, &tcpssn->client, &p,
                            p.iph_api->iph_ret_src(&p), p.iph_api->iph_ret_dst(&p), 
                            p.tcph->th_sport, p.tcph->th_dport,
                            PKT_FROM_SERVER);
#else
            flushed = flush_stream(tcpssn, &tcpssn->client, &p,
                            p.iph->ip_src.s_addr, p.iph->ip_dst.s_addr,
                            p.tcph->th_sport, p.tcph->th_dport,
                            PKT_FROM_SERVER);
#endif
            if (flushed)
                purge_ackd(tcpssn, &tcpssn->client);
            else
            {
                SnortEventqPush();
                SnortEventqLog(snort_conf->event_queue, &p);
                SnortEventqReset();
                SnortEventqPop();
            }
            tcpssn->client.flags &= ~TF_FORCE_FLUSH;
        }

        /* Flush the server */
        if (tcpssn->server.seglist && !(lwssn->ignore_direction & SSN_DIR_CLIENT) )
        {
            pc.s5tcp2++;
            /* Do each field individually because of size differences on 64bit OS */
            tmp_pcap_hdr.ts.tv_sec = tcpssn->server.seglist->pkth.ts.tv_sec;
            tmp_pcap_hdr.ts.tv_usec = tcpssn->server.seglist->pkth.ts.tv_usec;
            tmp_pcap_hdr.caplen = tcpssn->server.seglist->pkth.caplen - SPARC_TWIDDLE;
            tmp_pcap_hdr.pktlen = tcpssn->server.seglist->pkth.pktlen - SPARC_TWIDDLE;

            SnortEventqPush();
            (*grinder)(&p, &tmp_pcap_hdr,
                       tcpssn->server.seglist->pktOrig + SPARC_TWIDDLE);

            //set policy id for this packet
            {
                int vlanId = (p.vh)? VTH_VLAN(p.vh) : -1;
                snort_ip_p srcIp = (p.iph)? GET_SRC_IP((&p)) : (snort_ip_p)0;
                snort_ip_p dstIp = (p.iph)? GET_DST_IP((&p)) : (snort_ip_p)0;

                setRuntimePolicy(sfGetApplicablePolicyId(
                                    snort_conf->policy_config, vlanId, srcIp, dstIp));
                p.configPolicyId =
                    snort_conf->targeted_policies[getRuntimePolicy()]->configPolicyId;

                //actions are queued only for IDS case
                sfActionQueueExecAll(decoderActionQ);
            }
            SnortEventqPop();

            p.ssnptr = lwssn;

            tcpssn->server.flags |= TF_FORCE_FLUSH;
#ifdef SUP_IP6
            flushed = flush_stream(tcpssn, &tcpssn->server, &p,
                            p.iph_api->iph_ret_src(&p), p.iph_api->iph_ret_dst(&p), 
                            p.tcph->th_sport, p.tcph->th_dport,
                            PKT_FROM_CLIENT);
#else
            flushed = flush_stream(tcpssn, &tcpssn->server, &p,
                            p.iph->ip_src.s_addr, p.iph->ip_dst.s_addr,
                            p.tcph->th_sport, p.tcph->th_dport,
                            PKT_FROM_CLIENT);
#endif
            if (flushed)
                purge_ackd(tcpssn, &tcpssn->server);
            else
            {
                SnortEventqPush();
                SnortEventqLog(snort_conf->event_queue, &p);
                SnortEventqReset();
                SnortEventqPop();
            }
            tcpssn->server.flags &= ~TF_FORCE_FLUSH;
        }

        if (!s5_tcp_cleanup)
        {
            /* And turn decoder alerts back on (or whatever they were set to) */
            policyDecoderFlagsRestore(lwssn->policy_id);
        }
    }

    /* Purge the sequence lists */
    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "In TcpSessionCleanup, %lu bytes in use\n", mem_in_use););
    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "client has %d segs queued\n", tcpssn->client.seg_count););
    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "server has %d segs queued\n", tcpssn->server.seg_count););

    purge_all(&tcpssn->client);
    purge_all(&tcpssn->server);

    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
                "After cleaning, %lu bytes in use\n", mem_in_use););

    Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_CLOSED);
    if (lwssn->session_flags & SSNFLAG_PRUNED)
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_PRUNED);
    }
    else if (lwssn->session_flags & SSNFLAG_TIMEDOUT)
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_TIMEDOUT);
    }
    else
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_NORMALLY);
    }

    /* Cleanup the proto specific data */
    mempool_free(&tcp_session_mempool, lwssn->proto_specific_data);
    lwssn->proto_specific_data = NULL;
    lwssn->session_flags = SSNFLAG_NONE;
    lwssn->session_state = STREAM5_STATE_NONE;
    lwssn->expire_time = 0;
    lwssn->ignore_direction = 0;

    Stream5ResetFlowBits(lwssn);

    FreeLWApplicationData(lwssn);

    s5stats.tcp_streamtrackers_released++;
    RemoveStreamSession(&sfBase);

    EventInternal(INTERNAL_EVENT_SESSION_DEL);
}

#ifdef SUP_IP6
Stream5TcpPolicy *Stream5PolicyLookup(sfip_t *ip)
#else
Stream5TcpPolicy *Stream5PolicyLookup(struct in_addr ip)
#endif
{
    int policyIndex;
    Stream5TcpPolicy *policy = NULL;

    for (policyIndex = 0; policyIndex < s5_tcp_eval_config->num_policies; policyIndex++)
    {
        policy = s5_tcp_eval_config->policy_list[policyIndex];

        if (policy->bound_addrs == NULL)
            continue;

        /*
         * Does this policy handle packets to this IP address?
         */
#ifdef SUP_IP6
        if(sfvar_ip_in(policy->bound_addrs, ip))
#else
        if(IpAddrSetContains(policy->bound_addrs, ip))
#endif
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                "[Stream5] Found tcp policy in IpAddrSet\n"););
            break;
        }
    }

    if (policyIndex == s5_tcp_eval_config->num_policies)
        policy = s5_tcp_eval_config->default_policy;

    return policy;
}

#ifdef TARGET_BASED 
#define PRE_SESSION_CLEANUP_TARGET(lwssn) \
    app_proto_id = lwssn->application_protocol;
#else
#define PRE_SESSION_CLEANUP_TARGET(lwssn)
#endif

#ifdef SUP_IP6
#define PRE_SESSION_CLEANUP(lwssn) \
    uint32_t old_mem_in_use = mem_in_use; \
    sfip_t client_ip; \
    sfip_t server_ip; \
    uint16_t client_port = lwssn->client_port; \
    uint16_t server_port = lwssn->server_port; \
    uint16_t lw_session_state = lwssn->session_state; \
    uint32_t lw_session_flags = lwssn->session_flags; \
    int16_t app_proto_id = 0; \
    sfip_set_ip(&client_ip, &lwssn->client_ip); \
    sfip_set_ip(&server_ip, &lwssn->server_ip);

#define POST_SESSION_CLEANUP(delete_reason) \
    if ((old_mem_in_use - mem_in_use ) > s5_global_eval_config->prune_log_max) \
    { \
        char *client_ip_str, *server_ip_str; \
        client_ip_str = SnortStrdup(inet_ntoa(&client_ip)); \
        server_ip_str = SnortStrdup(inet_ntoa(&server_ip)); \
        LogMessage("S5: Pruned session from cache that was " \
                   "using %d bytes (%s). %s %d --> %s %d (%d) : " \
                   "LWstate 0x%x LWFlags 0x%x\n", \
                   old_mem_in_use - mem_in_use, \
                   delete_reason, \
                   client_ip_str, client_port, \
                   server_ip_str, server_port, \
                   app_proto_id, lw_session_state, lw_session_flags); \
        free(client_ip_str); \
        free(server_ip_str); \
    }

#else /* SUP_IP6 */
#define PRE_SESSION_CLEANUP(lwssn) \
    uint32_t old_mem_in_use = mem_in_use; \
    struct in_addr client_ip; \
    struct in_addr server_ip; \
    uint16_t client_port = lwssn->client_port; \
    uint16_t server_port = lwssn->server_port; \
    uint16_t lw_session_state = lwssn->session_state; \
    uint32_t lw_session_flags = lwssn->session_flags; \
    int16_t app_proto_id = 0; \
    client_ip.s_addr = lwssn->client_ip; \
    server_ip.s_addr = lwssn->server_ip;

#define POST_SESSION_CLEANUP(delete_reason) \
    if ((old_mem_in_use - mem_in_use ) > s5_global_eval_config->prune_log_max) \
    { \
        char *client_ip_str, *server_ip_str; \
        client_ip_str = SnortStrdup(inet_ntoa(client_ip)); \
        server_ip_str = SnortStrdup(inet_ntoa(server_ip)); \
        LogMessage("S5: Pruned session from cache that was " \
                   "using %d bytes (%s). %s %d --> %s %d (%d) : " \
                   "LWstate 0x%x LWFlags 0x%x\n", \
                   old_mem_in_use - mem_in_use, \
                   delete_reason, \
                   client_ip_str, client_port, \
                   server_ip_str, server_port, \
                   app_proto_id, lw_session_state, lw_session_flags); \
        free(client_ip_str); \
        free(server_ip_str); \
    }
#endif /* SUP_IP6 */

#ifdef REG_TEST
#define LCL(p, x)    (p->x - p->isn)
#define RMT(p, x, q) (p->x - (q ? q->isn : 0))

static void TraceEvent (
    const TCPHdr* h, TcpDataBlock* tdb, uint32_t txd, uint32_t rxd, uint32_t len
) {
    static unsigned pkt = 1; 

    int i;
    char flags[7] = "UAPRSF";

    for ( i = 0; i < 6; i++) 
        if ( !((1<<(5-i)) & h->th_flags) ) flags[i] = '-'; 

    // force relative ack to zero if not conveyed
    if ( flags[1] != 'A' ) rxd = ntohl(h->th_ack);

    fprintf(stdout,
        "\n%-3u %s=0x%02x Seq=%-4u Ack=%-4u Win=%-4u Len=%-4u\n",
        //"\n%-3u %s=0x%02x Seq=%-4u Ack=%-4u Win=%-4u Len=%-4u End=%-4u\n",
        pkt++, flags, h->th_flags,
        ntohl(h->th_seq)-txd, ntohl(h->th_ack)-rxd,
        ntohs(h->th_win), len
        //ntohs(h->th_win), len,  tdb->end_seq-txd
    );   
}

static void TraceSession (const Stream5LWSession* lws) 
{
    fprintf(stdout, "    LWS: ST=0x%x SF=0x%x CP=%u SP=%u\n",
        (unsigned)lws->session_state, lws->session_flags,
        (unsigned)ntohs(lws->client_port), (unsigned)ntohs(lws->server_port)
    );   
}

static const char* statext[] = {
    "NON", "LST", "SYR", "SYS", "EST", "CLW",
    "LAK", "FW1", "CLG", "FW2", "TWT", "CLD"
};

static const char* flushxt[] = {
    "NON", "FPR", "LOG", "RSP", "SLW", "CON", "IGN"
};

static void TraceState (
    const StreamTracker* a, const StreamTracker* b, const char* s)
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
    fprintf(stdout, "\n");
}

static void TraceTCP (
    const Packet* p, const Stream5LWSession* lws, TcpDataBlock* tdb, int event
) {
    const TCPHdr* h = p ? p->tcph : NULL;
    const TcpSession* ssn = (lws && lws->proto_specific_data) ?
         (TcpSession*)lws->proto_specific_data->data : NULL;
    const StreamTracker* srv = ssn ? &ssn->server : NULL;
    const StreamTracker* cli = ssn ? &ssn->client : NULL;

    const char* cdir = "?", *sdir = "?";
    uint32_t txd = 0, rxd = 0;

    if ( !h ) return;

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
    TraceEvent(h, tdb, txd, rxd, p->dsize);

    if ( lws ) TraceSession(lws);

    if ( !event )
    {
        if ( cli ) TraceState(cli, srv, cdir);
        if ( srv ) TraceState(srv, cli, sdir);
    }
}

static int s5_trace_enabled = -1;

static INLINE void S5TraceTCP (
    const Packet* p, const Stream5LWSession* lws, TcpDataBlock* tdb, int event
) {
    if ( !s5_trace_enabled ) 
        return;

    if ( s5_trace_enabled < 0 )
    {
        if ( !getenv("S5_TRACE") ) {
            s5_trace_enabled = 0;
            return;
        }
        s5_trace_enabled = 1;
    }
    TraceTCP(p, lws, tdb, event);
}
#endif  // REG_TEST

/*
 * Main entry point for TCP
 */
int Stream5ProcessTcp(Packet *p, Stream5LWSession *lwssn,
                      Stream5TcpPolicy *s5TcpPolicy, SessionKey *skey)
{
    TcpDataBlock tdb;
    int status;
    PROFILE_VARS;

    STREAM5_DEBUG_WRAP(
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

    if (s5TcpPolicy == NULL)
    {
        /* Find an Tcp policy for this packet */
#ifdef SUP_IP6
        s5TcpPolicy = Stream5PolicyLookup(GET_DST_IP(p));
#else
        s5TcpPolicy = Stream5PolicyLookup(p->iph->ip_dst);
#endif

        if (!s5TcpPolicy)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM, 
                                            "[Stream5] Could not find Tcp Policy context "
                                            "for IP %s\n", inet_ntoa(GET_DST_ADDR(p))););
            PREPROC_PROFILE_END(s5TcpPerfStats);
            return 0;
        }
    }

    if (isPacketFilterDiscard(p, (s5_tcp_eval_config->default_policy->flags &
                                  STREAM5_CONFIG_IGNORE_ANY)) == PORT_MONITOR_PACKET_DISCARD)
    {
        //ignore the packet
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM, 
                                        "[Stream5] %s:%d -> %s:%d Packet discarded due to port filtering\n",
                                        inet_ntoa(GET_SRC_ADDR(p)),p->sp,inet_ntoa(GET_DST_ADDR(p)),p->dp););

        UpdateFilteredPacketStats(&sfBase, IPPROTO_TCP);
        PREPROC_PROFILE_END(s5TcpPerfStats);
        return 0;
    }

    SetupTcpDataBlock(&tdb, p);

#ifdef DEBUG_STREAM5
    PrintTcpDataBlock(&tdb);
#endif

    if (lwssn == NULL)
    {
        /* if require 3WHS, create Lightweight Session on SYN */
        if (s5TcpPolicy->flags & STREAM5_CONFIG_REQUIRE_3WHS)
        {
            if (TCP_ISFLAGSET(p->tcph, TH_SYN) &&
                !TCP_ISFLAGSET(p->tcph, TH_ACK))
            {
                /* SYN only */
                lwssn = NewLWSession(tcp_lws_cache, p, skey, (void *)s5TcpPolicy);
                lwssn->session_state = STREAM5_STATE_SYN;
                s5stats.total_tcp_sessions++;
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
                else
                {
                    midstream_allowed = 0;
                }

                /* TODO: maybe look at drop stats before printing this
                 * warning -- or make this a configurable alert when
                 * requiring 3WAY. */
                DEBUG_WRAP(
                    DebugMessage(DEBUG_STREAM_STATE, "Stream5: Requiring 3-way "
                        "Handshake, but failed to retrieve session object "
                        "for non SYN packet.  Dropped SYN or hacker?\n"););

                /* 
                 * Do nothing with this packet since we require a 3-way.
                 * Wow that just sounds cool... Require a 3-way.  Hehe.
                 */
#ifdef REG_TEST
                S5TraceTCP(p, lwssn, &tdb, 1);
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
                lwssn = NewLWSession(tcp_lws_cache, p, skey, (void *)s5TcpPolicy);
                s5stats.total_tcp_sessions++;
            }
            else if (p->dsize > 0)
            {
                /* If we have data -- missed the SYN/ACK
                 * somehow -- maybe just an incomplete PCAP.  */
                /* This handles data on SYN situations */
                lwssn = NewLWSession(tcp_lws_cache, p, skey, (void *)s5TcpPolicy);
                s5stats.total_tcp_sessions++;
            }
            else if ((Stream5PacketHasWscale(p) & TF_WSCALE) &&
                     TCP_ISFLAGSET(p->tcph, TH_SYN))
            {
                /* If we have a wscale option, need to save the
                 * option if its the first SYN from client. */
                lwssn = NewLWSession(tcp_lws_cache, p, skey, (void *)s5TcpPolicy);
                lwssn->session_state = STREAM5_STATE_SYN;
                s5stats.total_tcp_sessions++;
            }
            else
            {
                /* No data, no need to create session yet */
                /* This is done to handle SYN flood DoS attacks */
#ifdef DEBUG
                    if (TCP_ISFLAGSET(p->tcph, TH_SYN))
                    {
                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Stream5: no data in packet (SYN only), no need to"
                            "create lightweight session.\n"););
                    }
                    else
                    {
                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Stream5: no data in packet (non SYN/keep alive "
                            "ACK?), no need to create lightweight session.\n"););
                    }
#endif

                PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
                S5TraceTCP(p, lwssn, &tdb, 1);
#endif
                return 0;
            }
        }

        if (!lwssn)
        {
            LogMessage("Stream5: Failed to retrieve session object.  Out of memory?\n");
            PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
            S5TraceTCP(p, lwssn, &tdb, 1);
#endif

            return -1;
        }
    }
    else
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
            "Stream5: Retrieved existing session object.\n"););
    }

    if (!lwssn)
    {
        LogMessage("Stream5: Failed to retrieve session object.  Out of memory?\n");
        PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
        S5TraceTCP(p, lwssn, &tdb, 1);
#endif
        return -1;
    }

    p->ssnptr = lwssn;

    /*
     * Check if the session is expired.
     * Should be done before we do something with the packet...
     * ie, Insert a packet, or handle state change SYN, FIN, RST, etc.
     */
    if ((lwssn->session_state & STREAM5_STATE_TIMEDOUT) ||
        Stream5Expire(p, lwssn))
    {
        lwssn->session_flags |= SSNFLAG_TIMEDOUT;
        /* Session is timed out */
        if (lwssn->session_flags & SSNFLAG_RESET)
        {
            PRE_SESSION_CLEANUP(lwssn);
            PRE_SESSION_CLEANUP_TARGET(lwssn);

            /* If this one has been reset, delete the TCP
             * portion, and start a new. */
            TcpSessionCleanup(lwssn);

            POST_SESSION_CLEANUP("new data/reset");

            status = ProcessTcp(lwssn, p, &tdb, s5TcpPolicy);

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Finished Stream5 TCP cleanly!\n"
                    "---------------------------------------------------\n"););
        }
        else
        {
            PRE_SESSION_CLEANUP(lwssn);
            PRE_SESSION_CLEANUP_TARGET(lwssn);

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream5 TCP session timedout!\n"););

            /* Not reset, simply time'd out.  Clean it up */
            TcpSessionCleanup(lwssn);

            POST_SESSION_CLEANUP("new data/timedout");

            status = ProcessTcp(lwssn, p, &tdb, s5TcpPolicy);
        }
    }
    else
    {
        status = ProcessTcp(lwssn, p, &tdb, s5TcpPolicy);
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Finished Stream5 TCP cleanly!\n"
                    "---------------------------------------------------\n"););
    }

    if (status != ACTION_LWSSN_DELETED)
    {
        MarkupPacketFlags(p, lwssn);
        Stream5SetExpire(p, lwssn, s5TcpPolicy->session_timeout);
    }

    PREPROC_PROFILE_END(s5TcpPerfStats);
#ifdef REG_TEST
    S5TraceTCP(p, lwssn, &tdb, 0);
#endif
    return 0;
}

static uint32_t Stream5GetTcpTimestamp(Packet *p, uint32_t *ts, int strip)
{
    unsigned int i = 0;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Getting timestamp...\n"););
    while(i < p->tcp_option_count && i < TCP_OPTLENMAX)
    {
        if(p->tcp_options[i].code == TCPOPT_TIMESTAMP)
        {
#ifdef NORMALIZER
            if ( strip && Normalize_IsEnabled(snort_conf, NORM_TCP_OPT) )
            {
                NormalStripTimeStamp(p, i);
            }
            else
#endif
            {
                *ts = EXTRACT_32BITS(p->tcp_options[i].data);
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Found timestamp %lu\n", *ts););

                return TF_TSTAMP;
            }
        }
        i++;
    }
    *ts = 0;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "No timestamp...\n"););

    return TF_NONE;
}

static uint32_t Stream5GetMss(Packet *p, uint16_t *value)
{
    unsigned int i = 0;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Getting MSS...\n"););
    while(i < p->tcp_option_count && i < TCP_OPTLENMAX)
    {
        if(p->tcp_options[i].code == TCPOPT_MAXSEG)
        {
            *value = EXTRACT_16BITS(p->tcp_options[i].data);
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Found MSS %u\n", *value););
            return TF_MSS;
        }

        i++;
    }

    *value = 0;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "No MSS...\n"););
    return TF_NONE;
}

static uint32_t Stream5GetWscale(Packet *p, uint16_t *value)
{
    unsigned int i = 0;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Getting wscale...\n"););
    while(i < p->tcp_option_count && i < TCP_OPTLENMAX)
    {
        if(p->tcp_options[i].code == TCPOPT_WSCALE)
        {
            *value = (uint16_t) p->tcp_options[i].data[0];
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "No wscale...\n"););
    return TF_NONE;
}

static uint32_t Stream5PacketHasWscale(Packet *p)
{
    uint16_t wscale;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Checking for wscale...\n"););
    return Stream5GetWscale(p, &wscale);
}

static INLINE int IsWellFormed(Packet *p, StreamTracker *ts)
{
    return ( p->dsize <= ts->mss );
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
    server->l_unackd = tdb->seq + 1;
    server->l_nxt_seq = server->l_unackd;
    server->isn = tdb->seq;

    client->r_nxt_ack = tdb->end_seq;
    client->r_win_base = tdb->end_seq;
    client->seglist_base_seq = server->l_unackd;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
               "seglist_base_seq = %X\n", client->seglist_base_seq););

    if (!(ssn->lwssn->session_state & STREAM5_STATE_MIDSTREAM))
        server->s_mgr.state = TCP_STATE_SYN_RCVD;

    server->flags |= Stream5GetTcpTimestamp(p, &server->ts_last, 0);
    if (server->ts_last == 0)
        server->flags |= TF_TSTAMP_ZERO;
    else
        server->ts_last_pkt = p->pkth->ts.tv_sec;
    server->flags |= Stream5GetMss(p, &server->mss);
    server->flags |= Stream5GetWscale(p, &server->wscale);

#ifdef DEBUG_STREAM5
    PrintTcpSession(ssn);
#endif
}

#ifdef OLD_CODE_NOLONGER_USED_DEPENDS_ON_CURRENT_STATE
static INLINE void QueueState(uint8_t transition, StreamTracker *st,
        uint8_t expected_flags, uint32_t seq_num, uint8_t get_seq)
{
    StateMgr *smgr = &st->s_mgr;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "[^^] Queing transition to %s, flag 0x%X, seq: 0x%X\n",
                state_names[transition], expected_flags, seq_num););

    smgr->state_queue = transition;
    smgr->expected_flags = expected_flags;
    smgr->stq_get_seq = get_seq;
    smgr->transition_seq = seq_num;

#ifdef DEBUG_STREAM5
    PrintStateMgr(smgr);
#endif
    return;
}

static INLINE int EvalStateQueue(StreamTracker *sptr, uint8_t flags, 
        uint32_t ack)
{
    StateMgr *smgr = &sptr->s_mgr;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Evaluating state queue!\n"););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "StreamTracker %p, flags 0x%X ack: 0x%X\n", sptr, flags, ack);
            PrintStateMgr(smgr););

    if(smgr->expected_flags != 0)
    {
        if((flags & smgr->expected_flags) != 0)
        {
            if(smgr->stq_get_seq && (SEQ_GEQ(ack, smgr->transition_seq)))
            {

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "[^^] Accepting %s state transition\n",
                            state_names[smgr->state_queue]););
                smgr->state = smgr->state_queue;
                smgr->expected_flags = 0;
                smgr->transition_seq = 0;
                return 1;
            }
            else if(!smgr->stq_get_seq)
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "[^^] Accepting %s state transition\n",
                            state_names[smgr->state_queue]););
                smgr->state = smgr->state_queue;
                smgr->expected_flags = 0;
                smgr->transition_seq = 0;
                return 1;

            }
            else
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "[!!] sptr->stq_get_seq: %d  "
                            "[ack: 0x%X expected: 0x%X]\n", smgr->stq_get_seq,
                            ack, smgr->transition_seq););
            }
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "[!!] flags: 0x%X  expected: 0x%X, bitwise: 0x%X\n",
                        flags, smgr->expected_flags,
                        (flags & smgr->expected_flags)););
        }
    }
    else
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "No transition queued, returning\n"););
    }

    return 0;
}
#endif

static INLINE int IgnoreLargePkt(StreamTracker *st, Packet *p, TcpDataBlock *tdb)
{
    if((st->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT) &&
       (st->tcp_policy->flags & STREAM5_CONFIG_PERFORMANCE))
    {
        if ((p->dsize > st->flush_mgr.flush_pt * 2) &&
            (st->seg_count == 0))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In NewQueue\n"););
    
    PREPROC_PROFILE_START(s5TcpInsertPerfStats);

    if(st->flush_mgr.flush_policy != STREAM_FLPOLICY_IGNORE)
    {
        /* Check if we should not insert a large packet */
        if (IgnoreLargePkt(st, p, tdb))
        {
            return;
        }

        /* new packet seq is below the last ack... */
        if(SEQ_GT(st->seglist_base_seq, tdb->seq))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "segment overlaps ack'd data...\n"););
            overlap = st->seglist_base_seq - tdb->seq;
            if(overlap >= p->dsize) 
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "full overlap on ack'd data, dropping segment\n"););
                PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                return;
            }
        }

        AddStreamNode(st, p, tdb, tcpssn, p->dsize, 0, 0, tdb->seq, NULL, &ss);

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Attached new queue to seglist, %d bytes queued, "
                    "base_seq 0x%X\n", 
                    ss->size, st->seglist_base_seq););
    }

    PREPROC_PROFILE_END(s5TcpInsertPerfStats);
    return;
}

static INLINE StreamSegment *FindSegment(StreamTracker *st, uint32_t pkt_seq)
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

void Stream5TcpBlockPacket(Packet *p)
{
    StreamSegment *ss;
    StreamTracker *tracker;
    Stream5LWSession *lwssn;
    TcpSession *ssn;
    uint32_t seq;

    if ((!p) || (!p->ssnptr))
        return;

    if (p->packet_flags & PKT_REBUILT_STREAM)
        return;

    if (!(p->packet_flags & PKT_STREAM_INSERT))
        return;

    lwssn = (Stream5LWSession *)p->ssnptr;

    ssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!ssn)
        return;

    seq = ntohl(p->tcph->th_seq);

    if (p->packet_flags & PKT_FROM_SERVER)
    {
        tracker = &ssn->client;
    }
    else
    {
        tracker = &ssn->server;
    }
    
    ss = FindSegment(tracker, seq);

    if (ss)
    {
        ss->blocked = 1;
    }
}

static INLINE int SegmentFastTrack(StreamSegment *tail, TcpDataBlock *tdb)
{
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Checking seq for fast track: %X > %X\n", tdb->seq, 
                tail->seq + tail->size););

    if(SEQ_EQ(tdb->seq, tail->seq + tail->size))
        return 1;

    return 0;
}

static void *SegmentAlloc(uint32_t size, Packet *p)
{
    void *tmp;

    mem_in_use += size;

    if (mem_in_use > s5_global_eval_config->memcap)
    {
        pc.str_mem_faults++;
        sfBase.iStreamFaults++;

        /* Smack the older time'd out sessions */
        if (!PruneLWSessionCache(tcp_lws_cache, p->pkth->ts.tv_sec,
                    (Stream5LWSession*)p->ssnptr, 0))
        {
            /* Try the memcap - last parameter (1) specifies check
             * based on memory cap. */
            PruneLWSessionCache(tcp_lws_cache, 0,
                    (Stream5LWSession*)p->ssnptr, 1);
        }
    }

    tmp = SnortAlloc(size);

    return tmp;
}

static int AddStreamNode(StreamTracker *st, Packet *p,
                  TcpDataBlock* tdb,
                  TcpSession *tcpssn,
                  int16_t len,
                  uint32_t slide,
                  uint32_t trunc,
                  uint32_t seq,
                  StreamSegment *left,
                  StreamSegment **retSeg)
{
    StreamSegment *ss = NULL;
    int32_t newSize = len - slide - trunc;

    if (newSize <= 0)
    {
        /*
         * zero size data because of trimming.  Don't
         * insert it
         */
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "zero size TCP data after left & right trimming "
                    "(len: %d slide: %d trunc: %d)\n",
                    len, slide, trunc););
        s5stats.tcp_discards++;
        NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);

#ifdef DEBUG_STREAM
        {
            StreamSegment *idx = st->seglist;
            unsigned long i = 0;
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Dumping seglist, %d segments\n", st->seg_count););
            while (idx)
            {
                i++;
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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

#if 0
    /* If we've got lots of data queued up on this session,
     * only seeing one side of the trafffic, etc, etc, etc.
     */
    if (0 /* */)
    {

        return STREAM_INSERT_ANOMALY;
    }
#endif
    
    ss = (StreamSegment *) SegmentAlloc(sizeof(StreamSegment), p);
    ss->pktOrig = ss->pkt = (uint8_t *) SegmentAlloc(p->pkth->caplen + SPARC_TWIDDLE, p);

    ss->caplen = p->pkth->caplen + SPARC_TWIDDLE;
    ss->pkt += SPARC_TWIDDLE;

    memcpy(ss->pkt, p->pkt, p->pkth->caplen);
    //memcpy(&ss->pkth, p->pkth, sizeof(DAQ_PktHdr_t));
    /* Do each field individually because of size differences on 64bit OS */
    ss->pkth.ts.tv_sec = p->pkth->ts.tv_sec;
    ss->pkth.ts.tv_usec = p->pkth->ts.tv_usec;
    ss->pkth.caplen = p->pkth->caplen;
    ss->pkth.pktlen = p->pkth->pktlen;

    ss->data = ss->pkt + (p->data - p->pkt);

    ss->payload = ss->data + slide;
    ss->size = (uint16_t)newSize;
    ss->seq = seq;
    ss->ts = tdb->ts;
    ss->cksum = p->tcph->th_sum;

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
            case STREAM_POLICY_LAST:
                /* Uh, who knows */
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

    Stream5SeglistAddNode(st, left, ss);
    st->seg_bytes_logical += ss->size;
    st->seg_bytes_total += ss->caplen;  /* Includes protocol headers and payload */
    st->total_segs_queued++;
    st->total_bytes_queued += ss->size;

    p->packet_flags |= PKT_STREAM_INSERT;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "added %d bytes on segment list @ seq: 0x%X, total %lu, "
                "%d segments queued\n", ss->size, ss->seq,
                st->seg_bytes_logical, SegsToFlush(st)););

    *retSeg = ss;
    return STREAM_INSERT_OK;
}

static int DupStreamNode(Packet *p,
        StreamTracker *st,
        StreamSegment *left,
        StreamSegment **retSeg)
{
    StreamSegment *ss = NULL;

    /*
     * get a new node
     */
    ss = (StreamSegment *) SegmentAlloc(sizeof(StreamSegment), p);
    /* caplen includes SPARC_TWIDDLE HERE */
    ss->pktOrig = ss->pkt = (uint8_t *) SegmentAlloc(left->caplen, p);

    /* caplen includes SPARC_TWIDDLE HERE */
    ss->caplen = left->caplen;
    memcpy(ss->pktOrig, left->pktOrig, left->caplen);
    memcpy(&ss->pkth, &left->pkth, sizeof(DAQ_PktHdr_t));

    ss->pkt += SPARC_TWIDDLE;
    ss->data = ss->pkt + (left->data - left->pkt);
    /*
     * twiddle the values for overlaps
     */
    ss->payload = ss->data;
    ss->size = left->size;
    ss->seq = left->seq;
    ss->cksum = left->cksum;

    Stream5SeglistAddNode(st, left, ss);
    //st->seg_bytes_logical += ss->size;
    //st->seg_bytes_total += p->dsize;
    st->total_segs_queued++;
    //st->total_bytes_queued += ss->size;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "added %d bytes on segment list @ seq: 0x%X, total %lu, "
                "%d segments queued\n", ss->size, ss->seq,
                st->seg_bytes_logical, SegsToFlush(st)););

    *retSeg = ss;
    return STREAM_INSERT_OK;

}

static int StreamQueue(StreamTracker *st, Packet *p, TcpDataBlock *tdb, 
        TcpSession *tcpssn)
{
    StreamSegment *ss = NULL;
    StreamSegment *left = NULL;
    StreamSegment *right = NULL;
    StreamSegment *dump_me = NULL;
    uint32_t orig_seq, seq = tdb->seq;
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
    int ips_data;
#endif
    PROFILE_VARS;
    STREAM5_DEBUG_WRAP(
        StreamSegment *lastptr = NULL;
        uint32_t base_seq = st->seglist_base_seq;
        int last = 0;
    );

#ifdef NORMALIZER
    ips_data = Normalize_IsEnabled(snort_conf, NORM_TCP_IPS);
    if ( ips_data )
        reassembly_policy = REASSEMBLY_POLICY_FIRST;
    else
#endif
        reassembly_policy = st->reassembly_policy;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Queuing %d bytes on stream!\n"
                "base_seq: %X seq: %X  seq_end: %X\n", 
                seq_end - seq, base_seq, seq, seq_end););

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "%d segments on seglist\n", SegsToFlush(st)););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););

    orig_seq = seq;

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

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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
        if (tdb->seq > st->seglist->seq)
        {
            dist_head = tdb->seq - st->seglist->seq;
        }
        else
        {
            dist_head = st->seglist->seq - tdb->seq;
        }

        if (tdb->seq > st->seglist_tail->seq)
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
    
    if (dist_head <= dist_tail)
    {
        /* Start iterating at the head (left) */
        for(ss = st->seglist; ss; ss = ss->next)
        {
            STREAM5_DEBUG_WRAP(
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
            STREAM5_DEBUG_WRAP(
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

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+\n"););

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "left: %p:0x%X  right: %p:0x%X\n", left, 
                left?left->seq:0, right, right?right->seq:0););

    /*
     * handle left overlaps
     */
    if(left)
    {
        /* 
         * check if the new segment overlaps on the left side
         */
        overlap = left->seq + left->size - seq;

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "left overlap %d\n", overlap););

        if(overlap > 0)
        {
            s5stats.tcp_overlaps++;
            st->overlap_count++;
            switch(reassembly_policy)
            {
                case REASSEMBLY_POLICY_FIRST:
                case REASSEMBLY_POLICY_LINUX:
                case REASSEMBLY_POLICY_BSD:
                case REASSEMBLY_POLICY_WINDOWS:
                case REASSEMBLY_POLICY_WINDOWS2K3:
                case REASSEMBLY_POLICY_VISTA:
                case REASSEMBLY_POLICY_HPUX10:
                case REASSEMBLY_POLICY_IRIX:
                case REASSEMBLY_POLICY_OLD_LINUX:
                case REASSEMBLY_POLICY_MACOS:
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "left overlap, honoring old data\n"););
#ifdef NORMALIZER
                    if ( ips_data )
                    {
                        if ((left->seq < seq) && (left->seq + left->size > seq + len))
                        {
                            unsigned offset = seq - left->seq;
                            memcpy((uint8_t*)p->data, left->payload+offset, len);
                            p->packet_flags |= PKT_MODIFIED;
                            normStats[PC_TCP_IPS_DATA]++;
                            sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA]++;
                        }
                        else if ( left->seq < seq )
                        {
                            unsigned offset = seq - left->seq;
                            unsigned length = left->seq + left->size - seq;
                            memcpy((uint8_t*)p->data, left->payload+offset, length);
                            p->packet_flags |= PKT_MODIFIED;
                            normStats[PC_TCP_IPS_DATA]++;
                            sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA]++;
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
                        s5stats.tcp_discards++;
                        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                        return STREAM_INSERT_ANOMALY;
                    }
                    break;

                case REASSEMBLY_POLICY_SOLARIS:
                case REASSEMBLY_POLICY_HPUX11:
                    if ((left->seq < seq) && (left->seq + left->size >= seq + len))
                    {
                        /* New packet is entirely overlapped by an
                         * existing packet on both sides.  Drop the
                         * new data. */
                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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
                            s5stats.tcp_discards++;
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return STREAM_INSERT_ANOMALY;
                        }
                    }
                    /* Otherwise, trim the old data accordingly */
                    left->size -= (int16_t)overlap;
                    st->seg_bytes_logical -= overlap;
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "left overlap, honoring new data\n"););
                    if (left->size <= 0)
                    {
                        dump_me = left;

                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "retrans, dumping old TCP data (seq: %d "
                                "overlap: %d)\n", dump_me->seq, overlap););

                        left = left->prev;
                        Stream5SeglistDeleteNode(st, dump_me);
                    }
                    break;
                case REASSEMBLY_POLICY_LAST:
                    /* True "Last" policy" */
                    if ((left->seq < seq) && (left->seq + left->size > seq + len))
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
                        //st->seg_bytes_total -= overlap;
                        
                        right->seq = seq + len;
                        right->size -= (int16_t)(seq + len - left->seq);
                        right->payload += (seq + len - left->seq);
                        st->seg_bytes_logical -= (seq + len - left->seq);
                        //st->seg_bytes_total -= (seq + len - left->seq);
                    }
                    else
                    {
                        left->size -= (int16_t)overlap;
                        st->seg_bytes_logical -= overlap;
                    }
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "left overlap, honoring new data\n"););
                    if (left->size <= 0)
                    {
                        dump_me = left;

                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "retrans, dumping old TCP data (seq: %d "
                                "overlap: %d)\n", dump_me->seq, overlap););

                        left = left->prev;
                        Stream5SeglistDeleteNode(st, dump_me);
                    }
                    break;
            }

            if(SEQ_LEQ(seq_end, seq))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "seq_end < seq"););
                /*
                 * houston, we have a problem
                 */
                /* flag an anomaly */
                EventBadSegment(st->tcp_policy);
                s5stats.tcp_discards++;
                PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                return STREAM_INSERT_ANOMALY;
            }
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "No left overlap\n"););
        }
    }

    //(seq_end > right->seq) && (seq_end <= (right->seq+right->size))))
    while(right && !done && SEQ_LT(right->seq, seq_end))
    {
        trunc = 0;
        overlap = (int)(seq_end - right->seq);
        //overlap = right->size - (right->seq - seq);
        //right->seq + right->size - seq_end;

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "right overlap(%d): len: %d right->seq: 0x%X seq: 0x%X\n",
                    overlap, len, right->seq, seq););

        if(overlap < right->size)
        {
            s5stats.tcp_overlaps++;
            st->overlap_count++;

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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
                    if ((right->seq == seq) &&
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

                    if (right->size <= 0)
                    {
                        dump_me = right;

                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_FRAG, "retrans, "
                                "dumping old data (seq: %d overlap: %d)\n", 
                                dump_me->seq, overlap););

                        right = right->next;

                        Stream5SeglistDeleteNode(st, dump_me);
 
                    }
                    break;

                case REASSEMBLY_POLICY_FIRST:
                case REASSEMBLY_POLICY_VISTA:
                case REASSEMBLY_POLICY_SOLARIS:
                case REASSEMBLY_POLICY_HPUX11:
#ifdef NORMALIZER
                    if ( ips_data )
                    {
                        unsigned offset = right->seq - tdb->seq;
                        unsigned length = seq + len - right->seq;
                        memcpy((uint8_t*)p->data+offset, right->payload, length);
                        p->packet_flags |= PKT_MODIFIED;
                        normStats[PC_TCP_IPS_DATA]++;
                        sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA]++;
                    }
#endif
                    trunc = overlap;
                    break;
            }

            /* all done, keep me out of the loop */
            done = 1;
        }
        else
        {
            s5stats.tcp_overlaps++;
            st->overlap_count++;

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Got full right overlap\n"););

#if 0
            if ( ips )
            {
                // FIXTHIS csum checks are evasion opportunities
                // also get here on a single segment that was normalized above
                // can't not inspect because may have more data (seq may have
                // been adjusted above)
            } else
#endif
            if ((right->seq == seq) && (right->cksum == p->tcph->th_sum))
            {
                /* RETRANSMISSION */
                /* Packet was analyzed the first time.
                 * Don't bother looking at it again.
                 */
                DisableDetect(p);

                /* Still want to count this in Perfmon */
                SetPreprocBit(p, PP_PERFMONITOR);

                /* Examined previously, was it blocked? */
                if (right->blocked)
                {
                    /* Previously blocked, block it again */
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                                "Dropping retransmitted packet -- "
                                "blocked previously\n"););
                    NormalDropPacket(p); // already alerted
                }
                else
                {
                    /* Previously not blocked, let it through */
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                                "Allowing retransmitted packet -- "
                                "not blocked previously\n"););
                    // NORM must overwrite payload
                }
                addthis = 0;
                done = 1;
                break;
            }
            else if ((right->seq == seq) &&
                     (right->size >= p->dsize))
            {
                /* Strange -- different size data.  New is same or smaller.  */
                /* Log Evasion attempt -- NO because this is handled by
                 * the policy-based code below. */
            }

            switch(reassembly_policy)
            {
                case REASSEMBLY_POLICY_BSD:
                case REASSEMBLY_POLICY_LINUX:
                case REASSEMBLY_POLICY_WINDOWS:
                case REASSEMBLY_POLICY_WINDOWS2K3:
                case REASSEMBLY_POLICY_HPUX10:
                case REASSEMBLY_POLICY_IRIX:
                case REASSEMBLY_POLICY_MACOS:
                    if ((seq_end >= right->seq + right->size) &&
                       (seq < right->seq))
                    {
                        dump_me = right;
                        st->seg_bytes_logical -= right->size;
                        //st->seg_bytes_total -= right->size;

                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "retrans, dropping old data at seq %d, size %d\n",
                                    right->seq, right->size););
                        right = right->next;
                        Stream5SeglistDeleteNode(st, dump_me);
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
                            if ((seq_end > right->seq + right->size) &&
                                (seq == right->seq))
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
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Got full right overlap, truncating new\n"););
#ifdef NORMALIZER
                    if ( ips_data )
                    {
                        unsigned offset = right->seq - tdb->seq;
                        memcpy((uint8_t*)p->data+offset, right->payload, right->size);
                        p->packet_flags |= PKT_MODIFIED;
                        normStats[PC_TCP_IPS_DATA]++;
                        sfBase.iPegs[PERF_COUNT_TCP_IPS_DATA]++;
                    }
#endif
                    /* full overlap */
                    if (right->seq == seq)
                    {
                        slide = (int32_t)(right->seq + right->size - seq);
                        seq += slide;
                        left = right;
                        right = right->next;

                        if (right && (SEQ_EQ(seq,right->seq)))
                        {
                            /* same seq as next packet don't insert yet...
                             * keep going.
                             */
                            continue;
                        }
                    }
                    else
                    {
                        trunc += overlap;
                    }
                    if(SEQ_LEQ((int)(seq_end - trunc), seq))
                    {
                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                    "StreamQueue got full right overlap with "
                                    "resulting seq too high, bad segment "
                                    "(seq: %X  seq_end: %X overlap: %lu\n", 
                                    seq, seq_end, overlap););
                        EventBadSegment(st->tcp_policy);
                        s5stats.tcp_discards++;
                        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                        return STREAM_INSERT_ANOMALY;
                    }

                    /* insert this one, and see if we need to chunk it up */
                    /* Adjust slide so that is correct relative to orig_seq */
                    slide = seq - orig_seq;
                    ret = AddStreamNode(st, p, tdb, tcpssn, len, slide, trunc, seq, left, &ss);
                    if (ret != STREAM_INSERT_OK)
                    {
                        /* no warning, already done above */
                        PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                        return ret;
                    }

                    {
                        uint32_t curr_end = ss->seq + ss->size;

                        while (right &&
                             (curr_end <= right->seq) &&
                             (right->seq < seq_end))
                        {
                            curr_end = right->seq + right->size;
                            left = right;
                            right = right->next;
                        }

                        if (right && (right->seq < seq_end))
                        {
                            /* Adjust seq to end of 'right' */
                            seq = right->seq + right->size;

                            /* Not overlapping on the left any more */
                            slide = 0;

                            left = right;

                            /*
                             * Reset trunc, in case the next one kicks us
                             * out of the loop.  This packet will become the
                             * right-most entry so far.  Don't truncate any
                             * further.
                             */
                            trunc = 0;
                            if (right->next)
                                continue;
                        }

                        if (curr_end < seq_end)
                        {
                            /* Insert this guy in his proper spot,
                             * adjust offset to the right-most endpoint
                             * we saw.
                             */
                            slide = left->seq + left->size - seq;
                            seq = curr_end;
                            trunc = 0;
                        }
                        else
                        {
                            addthis = 0;
                        }
                    }

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
                            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "StreamQueue got full right overlap with "
                                "resulting seq too high, bad segment "
                                "(seq: %X  seq_end: %X overlap: %lu\n", 
                                seq, seq_end, overlap););
                            EventBadSegment(st->tcp_policy);
                            s5stats.tcp_discards++;
                            PREPROC_PROFILE_END(s5TcpInsertPerfStats);
                            return STREAM_INSERT_ANOMALY;
                        }
                        break;
                    }
                /* Fall through */
                case REASSEMBLY_POLICY_OLD_LINUX:
                case REASSEMBLY_POLICY_LAST:
right_overlap_last:
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "Got full right overlap of old, dropping old\n"););
                    dump_me = right;
                    right = right->next;
                    Stream5SeglistDeleteNode(st, dump_me);
                    break;
            }
        }
    }

    if (addthis)
    {
        /* Adjust slide so that is correct relative to orig_seq */
        slide = seq - orig_seq;
        ret = AddStreamNode(st, p, tdb, tcpssn, len,
                slide, trunc, seq, left, &ss);
    }
    else
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Fully truncated right overlap\n"););
    }

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "StreamQueue returning normally\n"););

    PREPROC_PROFILE_END(s5TcpInsertPerfStats);
    return ret;
}


static void ProcessTcpStream(StreamTracker *rcv, TcpSession *tcpssn,
                             Packet *p, TcpDataBlock *tdb,
                             Stream5TcpPolicy *s5TcpPolicy)
{

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In ProcessTcpStream(), %d bytes to queue\n", p->dsize););

    if ((s5TcpPolicy->flags & STREAM5_CONFIG_NO_ASYNC_REASSEMBLY) &&
        ((tcpssn->lwssn->session_flags & (SSNFLAG_SEEN_SERVER | SSNFLAG_SEEN_CLIENT))
        != (SSNFLAG_SEEN_SERVER | SSNFLAG_SEEN_CLIENT)))
    {
        return;
    }

    if (s5TcpPolicy->max_consec_small_segs)
    {
        if (p->dsize < s5TcpPolicy->max_consec_small_seg_size)
        {
            /* check ignore_ports */
            if (!(s5TcpPolicy->small_seg_ignore[p->dp/8] & (1 << (p->dp %8))))
            {
                rcv->small_seg_count++;

                if (rcv->small_seg_count > s5TcpPolicy->max_consec_small_segs)
                {
                    /* Above threshold, log it... requires detect_anomalies be
                    * on in this TCP policy, action controlled by preprocessor
                    * rule. */
                    EventMaxSmallSegsExceeded(s5TcpPolicy);
    
                    /* Reset counter, so we're not too noisy */
                    rcv->small_seg_count = 0;
                }
            }
        }
    }

    if (s5TcpPolicy->max_queued_bytes &&
        (rcv->seg_bytes_total > s5TcpPolicy->max_queued_bytes))
    {
        if (!(tcpssn->lwssn->session_flags & SSNFLAG_LOGGED_QUEUE_FULL))
        {
            char *client_ip_str, *server_ip_str;
#ifdef SUP_IP6
            sfip_t client_ip;
            sfip_t server_ip;
            sfip_set_ip(&client_ip, &tcpssn->lwssn->client_ip);
            sfip_set_ip(&server_ip, &tcpssn->lwssn->server_ip);
            client_ip_str = SnortStrdup(inet_ntoa(&client_ip));
            server_ip_str = SnortStrdup(inet_ntoa(&server_ip));
#else
            struct in_addr client_ip;
            struct in_addr server_ip;
            client_ip.s_addr = tcpssn->lwssn->client_ip;
            server_ip.s_addr = tcpssn->lwssn->server_ip;
            client_ip_str = SnortStrdup(inet_ntoa(client_ip));
            server_ip_str = SnortStrdup(inet_ntoa(server_ip));
#endif
            LogMessage("S5: Session exceeded configured max bytes to queue %d "
                   "using %d bytes (%s). %s %d --> %s %d "
#ifdef TARGET_BASED
                   "(%d) "
#endif
                   ": LWstate 0x%x LWFlags 0x%x\n",
                   s5TcpPolicy->max_queued_bytes, rcv->seg_bytes_total,
                   (rcv == &tcpssn->client) ? "client queue" : "server queue", 
                   client_ip_str, ntohs(tcpssn->lwssn->client_port),
                   server_ip_str, ntohs(tcpssn->lwssn->server_port),
#ifdef TARGET_BASED
                   tcpssn->lwssn->application_protocol,
#endif
                   tcpssn->lwssn->session_state,
                   tcpssn->lwssn->session_flags);

            free(client_ip_str);  // FIXTHIS eliminate strdup and free
            free(server_ip_str);

            /* only log this one per session */
            tcpssn->lwssn->session_flags |= SSNFLAG_LOGGED_QUEUE_FULL;
        }
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Ignoring segment due to too many bytes queued\n"););
        return;
    }

    if (s5TcpPolicy->max_queued_segs &&
        (rcv->seg_count+1 > s5TcpPolicy->max_queued_segs))
    {
        if (!(tcpssn->lwssn->session_flags & SSNFLAG_LOGGED_QUEUE_FULL))
        {
            char *client_ip_str, *server_ip_str;
#ifdef SUP_IP6
            sfip_t client_ip;
            sfip_t server_ip;
            sfip_set_ip(&client_ip, &tcpssn->lwssn->client_ip);
            sfip_set_ip(&server_ip, &tcpssn->lwssn->server_ip);
            client_ip_str = SnortStrdup(inet_ntoa(&client_ip));
            server_ip_str = SnortStrdup(inet_ntoa(&server_ip));
#else
            struct in_addr client_ip;
            struct in_addr server_ip;
            client_ip.s_addr = tcpssn->lwssn->client_ip;
            server_ip.s_addr = tcpssn->lwssn->server_ip;
            client_ip_str = SnortStrdup(inet_ntoa(client_ip));
            server_ip_str = SnortStrdup(inet_ntoa(server_ip));
#endif
            LogMessage("S5: Session exceeded configured max segs to queue %d "
                   "using %d segs (%s). %s %d --> %s %d "
#ifdef TARGET_BASED
                   "(%d) "
#endif
                   ": LWstate 0x%x LWFlags 0x%x\n",
                   s5TcpPolicy->max_queued_segs, rcv->seg_count,
                   (rcv == &tcpssn->client) ? "client queue" : "server queue", 
                   client_ip_str, ntohs(tcpssn->lwssn->client_port),
                   server_ip_str, ntohs(tcpssn->lwssn->server_port),
#ifdef TARGET_BASED
                   tcpssn->lwssn->application_protocol,
#endif
                   tcpssn->lwssn->session_state, tcpssn->lwssn->session_flags);

            free(client_ip_str);  // FIXTHIS eliminate strdup and free
            free(server_ip_str);

            /* only log this one per session */
            tcpssn->lwssn->session_flags |= SSNFLAG_LOGGED_QUEUE_FULL;
        }
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Ignoring segment due to too many bytes queued\n"););
        return;
    }

    if(rcv->seg_count != 0)
    {
        if(rcv->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Ignoring segment due to IGNORE flush_policy\n"););
            return;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "queuing segment\n"););
            StreamQueue(rcv, p, tdb, tcpssn);

            if ((rcv->tcp_policy->overlap_limit) && 
                (rcv->overlap_count > rcv->tcp_policy->overlap_limit))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Reached the overlap limit.  Flush the data "
                        "and kill the session if configured\n"););
                if (p->packet_flags & PKT_FROM_CLIENT)
                {
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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

                /* Alert on overlap limit */
                /* FYI: Mark session as dead/drop remaining packets */
                /* FYI: Issue Drop/Reset packets for this session */
                EventExcessiveOverlap(rcv->tcp_policy);
            }
        }
    }
    else
    {
        if(rcv->flush_mgr.flush_policy == STREAM_FLPOLICY_IGNORE)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Ignoring segment due to IGNORE flush_policy\n"););
            return;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                                "queuing segment\n"););
            NewQueue(rcv, p, tdb, tcpssn);
        }
    }

    return;
}

static int ProcessTcpData(Packet *p, StreamTracker *listener, TcpSession *tcpssn,
        TcpDataBlock *tdb, Stream5TcpPolicy *s5TcpPolicy)
{
    PROFILE_VARS;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "In ProcessTcpData()\n"););

    PREPROC_PROFILE_START(s5TcpDataPerfStats);
    if ((p->tcph->th_flags & TH_SYN) && (listener->os_policy != STREAM_POLICY_MACOS))
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Bailing, data on SYN, not MAC Policy!\n"););
        NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);
        PREPROC_PROFILE_END(s5TcpDataPerfStats);
        return S5_UNALIGNED;
    }

    /* we're aligned, so that's nice anyway */
    if(tdb->seq == listener->r_nxt_ack)
    {
        /* check if we're in the window */
        if(Stream5GetWindow(listener) == 0)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Bailing, we're out of the window!\n"););
            NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);
            PREPROC_PROFILE_END(s5TcpDataPerfStats);
            return S5_UNALIGNED;
        }

        /* move the ack boundry up, this is the only way we'll accept data */
        // FIXTHIS for ips, must move all the way to first hole or right end
        if (listener->s_mgr.state_queue == TCP_STATE_NONE)
            listener->r_nxt_ack = tdb->end_seq;

        if(p->dsize != 0)
        {
            ProcessTcpStream(listener, tcpssn, p, tdb, s5TcpPolicy);
            /* set flags to session flags */

            PREPROC_PROFILE_END(s5TcpDataPerfStats);
            return S5_ALIGNED;
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
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "out of order segment (tdb->seq: 0x%X "
                    "l->r_nxt_ack: 0x%X!\n", tdb->seq, listener->r_nxt_ack);); 

        if (listener->s_mgr.state_queue == TCP_STATE_NONE)
        {
            /* check if we're in the window */
            if(Stream5GetWindow(listener) == 0)
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                            "Bailing, we're out of the window!\n"););
                NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);
                PREPROC_PROFILE_END(s5TcpDataPerfStats);
                return S5_UNALIGNED;
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

        if(p->dsize != 0)
        {
            ProcessTcpStream(listener, tcpssn, p, tdb, s5TcpPolicy);
        }
    }

    PREPROC_PROFILE_END(s5TcpDataPerfStats);
    return S5_UNALIGNED;
}

uint16_t StreamGetPolicy(Stream5LWSession *lwssn, Stream5TcpPolicy *s5TcpPolicy,
              int direction)
{
#ifdef TARGET_BASED
    uint16_t policy_id;
    /* Not caching this host_entry in the frag tracker so we can
     * swap the table out after processing this packet if we need
     * to.  */
    HostAttributeEntry *host_entry = NULL;
    int ssn_dir;

    if (!IsAdaptiveConfigured(getRuntimePolicy(), 0))
        return s5TcpPolicy->policy;

#ifdef SUP_IP6
    if (direction == FROM_CLIENT)
    {
        host_entry = SFAT_LookupHostEntryByIP(&lwssn->server_ip);
        ssn_dir = SSN_DIR_SERVER;
    }
    else
    {
        host_entry = SFAT_LookupHostEntryByIP(&lwssn->client_ip);
        ssn_dir = SSN_DIR_CLIENT;
    }
#else
    if (direction == FROM_CLIENT)
    {
        host_entry = SFAT_LookupHostEntryByIp4Addr(ntohl(lwssn->server_ip));
        ssn_dir = SSN_DIR_SERVER;
    }
    else
    {
        host_entry = SFAT_LookupHostEntryByIp4Addr(ntohl(lwssn->client_ip));
        ssn_dir = SSN_DIR_CLIENT;
    }
#endif
    if (host_entry && (isStreamPolicySet(host_entry) == POLICY_SET))
    {
        policy_id = getStreamPolicy(host_entry);

        if (policy_id != SFAT_UNKNOWN_STREAM_POLICY)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "StreamGetPolicy: Policy Map Entry: %s %s %s %d(%s)\n",
                host_entry->hostInfo.vendor.value.s_value,
                host_entry->hostInfo.operatingSystem.value.s_value,
                host_entry->hostInfo.version.value.s_value,
                policy_id, reassembly_policy_names[policy_id]););

            /* Since we've already done the lookup, try to get the
             * application protocol id with that host_entry. */
            Stream5SetApplicationProtocolIdFromHostEntry(lwssn, host_entry, ssn_dir);
            return policy_id;
        }
    }
#endif

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
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
        tcpssn->client.os_policy = StreamGetPolicy(tcpssn->lwssn, tcpssn->client.tcp_policy, FROM_SERVER);
        SetTcpReassemblyPolicy(&tcpssn->client);
    }

    if (tcpssn->server.os_policy == 0)
    {
        tcpssn->server.os_policy = StreamGetPolicy(tcpssn->lwssn, tcpssn->server.tcp_policy, FROM_CLIENT);
        SetTcpReassemblyPolicy(&tcpssn->server);
    }
}

static INLINE int ValidMacAddress(StreamTracker *talker,
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

static INLINE void CopyMacAddr(Packet *p,
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

static int NewTcpSession(Packet *p,
                         Stream5LWSession *lwssn,
                         TcpDataBlock *tdb,
                         Stream5TcpPolicy *dstPolicy)
{
    MemBucket *tmpBucket = NULL;
    TcpSession *tmp = NULL;
    uint16_t client_port = 0, server_port = 0;
    PROFILE_VARS;

    PREPROC_PROFILE_START(s5TcpNewSessPerfStats);

    if (TCP_ISFLAGSET(p->tcph, TH_SYN) &&
        !TCP_ISFLAGSET(p->tcph, TH_ACK))
    {
        /******************************************************************
         * start new sessions on proper SYN packets
         *****************************************************************/
        tmpBucket = mempool_alloc(&tcp_session_mempool);
        tmp = tmpBucket->data;
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Creating new session tracker on SYN!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif
        lwssn->session_flags |= SSNFLAG_SEEN_CLIENT;

        if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
        {
            lwssn->session_flags |= SSNFLAG_ECN_CLIENT_QUERY;
        }

        /* setup the stream trackers */
        tmp->client.s_mgr.state = TCP_STATE_SYN_SENT;
        tmp->client.isn = tdb->seq;
        tmp->client.l_unackd = tdb->seq + 1;
        tmp->client.l_nxt_seq = tmp->client.l_unackd;
        tmp->client.l_window = tdb->win;

        tmp->client.ts_last_pkt = p->pkth->ts.tv_sec;

        tmp->server.seglist_base_seq = tmp->client.l_unackd;
        tmp->server.r_nxt_ack = tmp->client.l_unackd;
        tmp->server.r_win_base = tdb->seq+1;

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "seglist_base_seq = %X\n", tmp->server.seglist_base_seq););
        tmp->server.s_mgr.state = TCP_STATE_LISTEN;

        tmp->client.flags |= Stream5GetTcpTimestamp(p, &tmp->client.ts_last, 0);
        if (tmp->client.ts_last == 0)
            tmp->client.flags |= TF_TSTAMP_ZERO;
        tmp->client.flags |= Stream5GetMss(p, &tmp->client.mss);
        tmp->client.flags |= Stream5GetWscale(p, &tmp->client.wscale);


        /* Set the Stream5TcpPolicy for each direction (pkt from client) */
#ifdef SUP_IP6
        tmp->client.tcp_policy = Stream5PolicyLookup(GET_SRC_IP(p));
#else
        tmp->client.tcp_policy = Stream5PolicyLookup(p->iph->ip_src);
#endif
        tmp->server.tcp_policy = dstPolicy;

        /* Server is destination */
        server_port = p->dp;
        client_port = p->sp;

        CopyMacAddr(p, tmp, FROM_CLIENT);
    }
    else if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
    {
        /******************************************************************
         * start new sessions on SYN/ACK from server
         *****************************************************************/
        tmpBucket = mempool_alloc(&tcp_session_mempool);
        tmp = tmpBucket->data;
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Creating new session tracker on SYN_ACK!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif
        lwssn->session_flags |= SSNFLAG_SEEN_SERVER;

        if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
        {
            lwssn->session_flags |= SSNFLAG_ECN_SERVER_REPLY;
        }

        /* setup the stream trackers */
        tmp->server.s_mgr.state = TCP_STATE_SYN_RCVD;
        tmp->server.isn = tdb->seq;
        tmp->server.l_unackd = tdb->seq + 1;
        tmp->server.l_nxt_seq = tmp->server.l_unackd;
        tmp->server.l_window = tdb->win;

        /* Double check these two -- should be ACK -1? */
        tmp->server.seglist_base_seq = tdb->ack;
        tmp->server.r_win_base = tdb->ack;
        tmp->server.r_nxt_ack = tdb->ack;

        tmp->server.ts_last_pkt = p->pkth->ts.tv_sec;

        tmp->client.seglist_base_seq = tmp->server.l_unackd;
        tmp->client.r_nxt_ack = tmp->server.l_unackd;
        tmp->client.r_win_base = tdb->seq+1;
        tmp->client.l_nxt_seq = tdb->ack;

        /* Double check this -- should be ACK -1? */
        tmp->client.isn = tdb->ack-1;

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "seglist_base_seq = %X\n", tmp->client.seglist_base_seq););
        tmp->client.s_mgr.state = TCP_STATE_SYN_SENT;

        tmp->server.flags |= Stream5GetTcpTimestamp(p, &tmp->server.ts_last, 0);
        if (tmp->server.ts_last == 0)
            tmp->server.flags |= TF_TSTAMP_ZERO;
        tmp->server.flags |= Stream5GetMss(p, &tmp->server.mss);
        tmp->server.flags |= Stream5GetWscale(p, &tmp->server.wscale);

        /* Set the Stream5TcpPolicy for each direction (pkt from server) */
#ifdef SUP_IP6
        tmp->server.tcp_policy = Stream5PolicyLookup(GET_SRC_IP(p));
#else
        tmp->server.tcp_policy = Stream5PolicyLookup(p->iph->ip_src);
#endif
        tmp->client.tcp_policy = dstPolicy;

        /* Client is destination */
        server_port = p->sp;
        client_port = p->dp;

        CopyMacAddr(p, tmp, FROM_SERVER);
    }
    else if ((p->tcph->th_flags & TH_ACK) &&
             !(p->tcph->th_flags & TH_RST) &&
             (lwssn->session_state & STREAM5_STATE_ESTABLISHED))
    {
        /******************************************************************
         * start new sessions on completion of 3-way (ACK only, no data)
         *****************************************************************/
        tmpBucket = mempool_alloc(&tcp_session_mempool);
        tmp = tmpBucket->data;
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Creating new session tracker on ACK!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif
        lwssn->session_flags |= SSNFLAG_SEEN_CLIENT;

        if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
        {
            lwssn->session_flags |= SSNFLAG_ECN_CLIENT_QUERY;
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

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "seglist_base_seq = %X\n", tmp->server.seglist_base_seq);); tmp->server.s_mgr.state = TCP_STATE_ESTABLISHED;

        tmp->client.flags |= Stream5GetTcpTimestamp(p, &tmp->client.ts_last, 0);
        if (tmp->client.ts_last == 0)
            tmp->client.flags |= TF_TSTAMP_ZERO;
        tmp->client.flags |= Stream5GetMss(p, &tmp->client.mss);
        tmp->client.flags |= Stream5GetWscale(p, &tmp->client.wscale);

        /* Set the Stream5TcpPolicy for each direction (pkt from client) */
#ifdef SUP_IP6
        tmp->client.tcp_policy = Stream5PolicyLookup(GET_SRC_IP(p));
#else
        tmp->client.tcp_policy = Stream5PolicyLookup(p->iph->ip_src);
#endif

        tmp->server.tcp_policy = dstPolicy;

        /* Server is destination */
        server_port = p->dp;
        client_port = p->sp;

        CopyMacAddr(p, tmp, FROM_CLIENT);
    }
    else if (p->dsize != 0)
    {
        /******************************************************************
         * start new sessions on data in packet
         *****************************************************************/
        tmpBucket = mempool_alloc(&tcp_session_mempool);
        tmp = tmpBucket->data;
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Creating new session tracker on data packet (ACK|PSH)!\n"););

#ifdef DEBUG
        tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
        tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
#endif

        if (lwssn->direction == FROM_CLIENT)
        {
            /* Sender is client (src port is higher) */
            lwssn->session_flags |= SSNFLAG_SEEN_CLIENT;

            if((p->tcph->th_flags & (TH_CWR|TH_ECE)) == (TH_CWR|TH_ECE))
            {
                lwssn->session_flags |= SSNFLAG_ECN_CLIENT_QUERY;
            }

            /* setup the stream trackers */
            tmp->client.s_mgr.state = TCP_STATE_ESTABLISHED;
            tmp->client.isn = tdb->seq;
            tmp->client.l_unackd = tdb->seq;
            tmp->client.l_nxt_seq = tmp->client.l_unackd;
            tmp->client.l_window = tdb->win;

            tmp->client.ts_last_pkt = p->pkth->ts.tv_sec;

            tmp->server.seglist_base_seq = tmp->client.l_unackd;
            tmp->server.r_nxt_ack = tmp->client.l_unackd;
            tmp->server.r_win_base = tdb->seq;
            tmp->server.l_window = 0; /* reset later */

            /* Next server packet is what was ACKd */
            //tmp->server.l_nxt_seq = tdb->ack + 1;
            tmp->server.l_unackd = tdb->ack - 1;

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "seglist_base_seq = %X\n", tmp->server.seglist_base_seq););
            tmp->server.s_mgr.state = TCP_STATE_ESTABLISHED;

            tmp->client.flags |= Stream5GetTcpTimestamp(p, &tmp->client.ts_last, 0);
            if (tmp->client.ts_last == 0)
                tmp->client.flags |= TF_TSTAMP_ZERO;
            tmp->client.flags |= Stream5GetMss(p, &tmp->client.mss);
            tmp->client.flags |= Stream5GetWscale(p, &tmp->client.wscale);

            /* Set the Stream5TcpPolicy for each direction (pkt from client) */
#ifdef SUP_IP6
            tmp->client.tcp_policy = Stream5PolicyLookup(GET_SRC_IP(p));
#else
            tmp->client.tcp_policy = Stream5PolicyLookup(p->iph->ip_src);
#endif
            tmp->server.tcp_policy = dstPolicy;

            /* Server is destination */
            server_port = p->dp;
            client_port = p->sp;

            CopyMacAddr(p, tmp, FROM_CLIENT);
        }
        else
        {
            /* Sender is server (src port is lower) */
            lwssn->session_flags |= SSNFLAG_SEEN_SERVER;

            /* setup the stream trackers */
            tmp->server.s_mgr.state = TCP_STATE_ESTABLISHED;
            tmp->server.isn = tdb->seq;
            tmp->server.l_unackd = tdb->seq;
            tmp->server.l_nxt_seq = tmp->server.l_unackd;
            tmp->server.l_window = tdb->win;

            /* Double check these two -- should be ACK -1? */
            tmp->server.seglist_base_seq = tdb->ack -1;
            tmp->server.r_win_base = tdb->ack;
            tmp->server.r_nxt_ack = tdb->ack;

            tmp->server.ts_last_pkt = p->pkth->ts.tv_sec;

            tmp->client.seglist_base_seq = tmp->server.l_unackd;
            tmp->client.r_nxt_ack = tmp->server.l_unackd;
            tmp->client.r_win_base = tdb->seq;
            tmp->client.l_window = 0; /* reset later */

            /* Double check this -- should be ACK -1? */
            tmp->client.isn = tdb->ack-1;

            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "seglist_base_seq = %X\n", tmp->client.seglist_base_seq););
            tmp->client.s_mgr.state = TCP_STATE_ESTABLISHED;

            tmp->server.flags |= Stream5GetTcpTimestamp(p, &tmp->server.ts_last, 0);
            if (tmp->server.ts_last == 0)
                tmp->server.flags |= TF_TSTAMP_ZERO;
            tmp->server.flags |= Stream5GetMss(p, &tmp->server.mss);
            tmp->server.flags |= Stream5GetWscale(p, &tmp->server.wscale);

            /* Set the Stream5TcpPolicy for each direction (pkt from server) */
#ifdef SUP_IP6
            tmp->server.tcp_policy = Stream5PolicyLookup(GET_SRC_IP(p));
#else
            tmp->server.tcp_policy = Stream5PolicyLookup(p->iph->ip_src);
#endif
            tmp->client.tcp_policy = dstPolicy;

            /* Client is destination */
            server_port = p->sp;
            client_port = p->dp;

            CopyMacAddr(p, tmp, FROM_SERVER);
        }
    }

    if (tmp)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "adding TcpSession to lightweight session\n"););
        lwssn->proto_specific_data = tmpBucket;
        lwssn->protocol = GET_IPH_PROTO(p);
        tmp->lwssn = lwssn;

        /* New session, previous was marked as reset.  Clear the
         * reset flag. */
        if (lwssn->session_flags & SSNFLAG_RESET)
            lwssn->session_flags &= ~SSNFLAG_RESET;

        SetOSPolicy(tmp);
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
        if (tmp->server.tcp_policy->flush_config_protocol[lwssn->application_protocol].configured == 1)
        {
            InitFlushMgr(&tmp->server.flush_mgr,
                     &tmp->server.tcp_policy->flush_point_list,
                     tmp->server.tcp_policy->flush_config_protocol[lwssn->application_protocol].client.flush_policy);
        }
        else
        {
#endif
            InitFlushMgr(&tmp->server.flush_mgr,
                     &tmp->server.tcp_policy->flush_point_list,
                     tmp->server.tcp_policy->flush_config[server_port].client.flush_policy);
#ifdef TARGET_BASED
        }
#endif

#ifdef TARGET_BASED
        if (tmp->client.tcp_policy->flush_config_protocol[lwssn->application_protocol].configured == 1)
        {
            InitFlushMgr(&tmp->client.flush_mgr,
                     &tmp->client.tcp_policy->flush_point_list,
                     tmp->client.tcp_policy->flush_config_protocol[lwssn->application_protocol].server.flush_policy);
        }
        else
        {
#endif
            InitFlushMgr(&tmp->client.flush_mgr,
                     &tmp->client.tcp_policy->flush_point_list,
                     tmp->client.tcp_policy->flush_config[server_port].server.flush_policy);
#ifdef TARGET_BASED
        }
#endif

#ifdef DEBUG_STREAM5
        PrintTcpSession(tmp);
#endif
        Stream5SetExpire(p, lwssn, dstPolicy->session_timeout);

        s5stats.tcp_streamtrackers_created++;

        AddStreamSession(&sfBase, lwssn->session_state & STREAM5_STATE_MIDSTREAM ? SSNFLAG_MIDSTREAM : 0);

        Stream5UpdatePerfBaseState(&sfBase, tmp->lwssn, TCP_STATE_SYN_SENT);

        EventInternal(INTERNAL_EVENT_SESSION_ADD);

#ifdef NORMALIZER
        tmp->ecn = 0;
#endif
        PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
        return 1;
    }

    PREPROC_PROFILE_END(s5TcpNewSessPerfStats);
    return 0;
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
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Got syn on established windows ssn, which causes Reset,"
                "bailing\n"););
            tcpssn->lwssn->session_flags |= SSNFLAG_RESET;
            talker->s_mgr.state = TCP_STATE_CLOSED;
            return ACTION_RST;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Got syn on established windows ssn, not causing Reset,"
                "bailing\n"););
            s5stats.tcp_discards++;
            return ACTION_NOTHING;
        }
        break;
    case STREAM_POLICY_MACOS:
        /* MACOS ignores a 2nd SYN, regardless of the sequence number. */
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
            "Got syn on established macos ssn, not causing Reset,"
            "bailing\n"););
        s5stats.tcp_discards++;
        return ACTION_NOTHING;
        break;
    case STREAM_POLICY_FIRST:
    case STREAM_POLICY_LAST:
    /* Uh, who knows */
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
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Got syn on established ssn, which causes Reset, bailing\n"););
            tcpssn->lwssn->session_flags |= SSNFLAG_RESET;
            talker->s_mgr.state = TCP_STATE_CLOSED;
            return ACTION_RST;
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Got syn on established ssn, not causing Reset,"
                "bailing\n"););
            s5stats.tcp_discards++;
            return ACTION_NOTHING;
        }
        break;
    }
    return ACTION_NOTHING;
}

static void LogTcpEvents(Stream5TcpPolicy *s5TcpPolicy, int eventcode)
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
}

static int ProcessTcp(Stream5LWSession *lwssn, Packet *p, TcpDataBlock *tdb, 
        Stream5TcpPolicy *s5TcpPolicy)
{
    int retcode = 0;
    int eventcode = 0;
    char ignore = 0;
    int got_ts = 0;
    int new_ssn = 0;
    int ts_action = ACTION_NOTHING;
    int aligned = S5_UNALIGNED;
    TcpSession *tcpssn = NULL;
    StreamTracker *talker = NULL;
    StreamTracker *listener = NULL;
    uint32_t require3Way = (s5TcpPolicy->flags & STREAM5_CONFIG_REQUIRE_3WHS);
    int16_t protoId = 0;
    STREAM5_DEBUG_WRAP(char *t = NULL; char *l = NULL;)
    PROFILE_VARS;

    if (lwssn->protocol != IPPROTO_TCP)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Lightweight session not TCP on TCP packet\n"););
        return ACTION_NOTHING;
    }

    if (
        (lwssn->session_flags & (SSNFLAG_DROP_CLIENT|SSNFLAG_DROP_SERVER)))
    {
        /* Got a packet on a session that was dropped (by a rule). */
        GetLWPacketDirection(p, lwssn);

        /* Drop this packet */
        if (((p->packet_flags & PKT_FROM_SERVER) &&
             (lwssn->session_flags & SSNFLAG_DROP_SERVER)) ||
            ((p->packet_flags & PKT_FROM_CLIENT) &&
             (lwssn->session_flags & SSNFLAG_DROP_CLIENT)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Blocking %s packet as session was blocked\n",
                        p->packet_flags & PKT_FROM_SERVER ?
                        "server" : "client"););
            DisableDetect(p);
            /* Still want to add this number of bytes to totals */
            SetPreprocBit(p, PP_PERFMONITOR);
            NormalDropPacket(p);  // already alerted
#ifdef ACTIVE_RESPONSE
            Stream5ActiveResponse(p, lwssn);
#endif
            return ACTION_NOTHING;
        }
    }

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    PREPROC_PROFILE_START(s5TcpStatePerfStats);

    if (tcpssn == NULL)
    {
        if (TCP_ISFLAGSET(p->tcph, TH_SYN) &&
            !TCP_ISFLAGSET(p->tcph, TH_ACK))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Stream5 SYN PACKET, establishing lightweight"
                    "session direction.\n"););
            /* SYN packet from client */
            lwssn->direction = FROM_CLIENT;
            IP_COPY_VALUE(lwssn->client_ip, GET_SRC_IP(p));
            lwssn->client_port = p->tcph->th_sport;
            IP_COPY_VALUE(lwssn->server_ip, GET_DST_IP(p));
            lwssn->server_port = p->tcph->th_dport;
            lwssn->session_state |= STREAM5_STATE_SYN;
#ifdef ACTIVE_RESPONSE
            SetTTL(lwssn, p, 1);
#endif
            if (require3Way || (Stream5PacketHasWscale(p) & TF_WSCALE) ||
                ((p->dsize > 0) &&
                 (StreamGetPolicy(lwssn, s5TcpPolicy, FROM_CLIENT) ==
                     STREAM_POLICY_MACOS)))
            {
                /* Create TCP session if we 
                 * 1) require 3-WAY HS, OR
                 * 2) client sent wscale option, OR
                 * 3) have data and its a MAC OS policy -- MAC
                 *    is the only one that accepts data on SYN
                 *    (and thus requires a TCP session at this point)
                 */
                NewTcpSession(p, lwssn, tdb, s5TcpPolicy);
                tcpssn = (TcpSession *)lwssn->proto_specific_data->data;
                new_ssn = 1;
#ifdef NORMALIZER
                NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
            }

            /* Nothing left todo here */
        }
        else if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
        {
            /* SYN-ACK from server */
            if ((lwssn->session_state == STREAM5_STATE_NONE) ||
                (lwssn->session_flags & SSNFLAG_RESET))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "Stream5 SYN|ACK PACKET, establishing lightweight"
                        "session direction.\n"););
                lwssn->direction = FROM_SERVER;
                IP_COPY_VALUE(lwssn->client_ip, GET_DST_IP(p));
                lwssn->client_port = p->tcph->th_dport;
                IP_COPY_VALUE(lwssn->server_ip, GET_SRC_IP(p));
                lwssn->server_port = p->tcph->th_sport;
#ifdef ACTIVE_RESPONSE
                SetTTL(lwssn, p, 0);
#endif
            }
            lwssn->session_state |= STREAM5_STATE_SYN_ACK;
            if (!require3Way)
            {
                NewTcpSession(p, lwssn, tdb, s5TcpPolicy);
                tcpssn = (TcpSession *)lwssn->proto_specific_data->data;
                new_ssn = 1;
            }

#ifdef NORMALIZER
            NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
            /* Nothing left todo here */
        }
        else if (TCP_ISFLAGSET(p->tcph, TH_ACK) &&
                !TCP_ISFLAGSET(p->tcph, TH_RST) &&
                 (lwssn->session_state & STREAM5_STATE_SYN_ACK))
        {
            /* TODO: do we need to verify the ACK field is >= the seq of the SYN-ACK? */

            /* 3-way Handshake complete, create TCP session */
            lwssn->session_state |= STREAM5_STATE_ACK | STREAM5_STATE_ESTABLISHED;
            NewTcpSession(p, lwssn, tdb, s5TcpPolicy);
            tcpssn = (TcpSession *)lwssn->proto_specific_data->data;
            new_ssn = 1;

#ifdef NORMALIZER
            NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
            Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_ESTABLISHED);
        }
        else if ((p->dsize > 0) && (!require3Way || midstream_allowed))
        {
            /* create session on data, need to figure out direction, etc */
            /* Assume from client, can update later */
            if (p->sp > p->dp)
            {
                lwssn->direction = FROM_CLIENT;
                IP_COPY_VALUE(lwssn->client_ip, GET_SRC_IP(p));
                lwssn->client_port = p->tcph->th_sport;
                IP_COPY_VALUE(lwssn->server_ip, GET_DST_IP(p));
                lwssn->server_port = p->tcph->th_dport;
#ifdef ACTIVE_RESPONSE
                SetTTL(lwssn, p, 1);
#endif
            }
            else
            {
                lwssn->direction = FROM_SERVER;
                IP_COPY_VALUE(lwssn->client_ip, GET_DST_IP(p));
                lwssn->client_port = p->tcph->th_dport;
                IP_COPY_VALUE(lwssn->server_ip, GET_SRC_IP(p));
                lwssn->server_port = p->tcph->th_sport;
#ifdef ACTIVE_RESPONSE
                SetTTL(lwssn, p, 0);
#endif
            }
            lwssn->session_state |= STREAM5_STATE_MIDSTREAM;
            lwssn->session_flags |= SSNFLAG_MIDSTREAM;

#ifdef DEBUG
            if (ScReadMode())
            {
                /* If we're in readback mode... may only have one packet.
                 * That being packet with the exploit being tested, so
                 * mark this session as established, so rule option
                 * 'flow:established' works correctly.
                 */
                STREAM5_DEBUG_WRAP(
                    char timestamp[TIMEBUF_SIZE];
                    char src_addr[17];
                    char dst_addr[17];
                    bzero((char *)timestamp, TIMEBUF_SIZE);
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
                lwssn->session_state |= STREAM5_STATE_ESTABLISHED;
                lwssn->session_flags |= SSNFLAG_ESTABLISHED;
#endif
            }
#endif
            NewTcpSession(p, lwssn, tdb, s5TcpPolicy);
            tcpssn = (TcpSession *)lwssn->proto_specific_data->data;
            new_ssn = 1;

#ifdef NORMALIZER
            NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
            if (lwssn->session_state & STREAM5_STATE_ESTABLISHED)
                Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_ESTABLISHED);
        }
        else if (p->dsize == 0)
        {
            /* Already have a lwssn, but no tcp session.
             * Probably just an ACK of already sent data (that
             * we missed).
             */
            /* Do nothing. */
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return ACTION_NOTHING | retcode;
        }
    }
    else
    {
        /* If session is already marked as established */
        if (!(lwssn->session_state & STREAM5_STATE_ESTABLISHED) && !require3Way)
        {
            /* If not requiring 3-way Handshake... */
    
            /* TCP session created on TH_SYN above,
             * or maybe on SYN-ACK, or anything else */
    
            /* Need to update Lightweight session state */
            if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
            {
                /* SYN-ACK from server */
                if (lwssn->session_state != STREAM5_STATE_NONE)
                {
                    lwssn->session_state |= STREAM5_STATE_SYN_ACK;
                }
            }
            else if (TCP_ISFLAGSET(p->tcph, TH_ACK) &&
                     (lwssn->session_state & STREAM5_STATE_SYN_ACK))
            {
                lwssn->session_state |= STREAM5_STATE_ACK | STREAM5_STATE_ESTABLISHED;
                Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_ESTABLISHED);
            }
        }
#ifdef NORMALIZER
        if (TCP_ISFLAGSET(p->tcph, TH_SYN))
            NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
    }

    /* figure out direction of this packet */
    GetLWPacketDirection(p, lwssn);

    if(p->packet_flags & PKT_FROM_SERVER)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Stream5: Updating on packet from server\n"););
        lwssn->session_flags |= SSNFLAG_SEEN_SERVER;
        if (tcpssn)
        {
            talker = &tcpssn->server;
            listener = &tcpssn->client;
        }

        STREAM5_DEBUG_WRAP(
                t = "Server";
                l = "Client");

        if ( talker && talker->s_mgr.state == TCP_STATE_LISTEN &&
            ((p->tcph->th_flags & (TH_SYN|TH_ACK)) == TH_SYN) )
        {
            eventcode |= EVENT_4WHS;
        }
        /* If we picked this guy up midstream, finish the initialization */
        if ((lwssn->session_state & STREAM5_STATE_MIDSTREAM) &&
            !(lwssn->session_state & STREAM5_STATE_ESTABLISHED))
        {
            FinishServerInit(p, tdb, tcpssn);
            if((p->tcph->th_flags & TH_ECE) && 
                lwssn->session_flags & SSNFLAG_ECN_CLIENT_QUERY)
            {
                lwssn->session_flags |= SSNFLAG_ECN_SERVER_REPLY;
            }
            
            if (lwssn->session_flags & SSNFLAG_SEEN_CLIENT)
            {
                // should TCP state go to established too?
                lwssn->session_state |= STREAM5_STATE_ESTABLISHED;
                lwssn->session_flags |= SSNFLAG_ESTABLISHED;
                Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_ESTABLISHED);
            }
        }
    }
    else
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Stream5: Updating on packet from client\n"););
        /* if we got here we had to see the SYN already... */
        lwssn->session_flags |= SSNFLAG_SEEN_CLIENT;
        if (tcpssn)
        {
            talker = &tcpssn->client;
            listener = &tcpssn->server;
        }

        STREAM5_DEBUG_WRAP(
                t = "Client";
                l = "Server";);

        if ((lwssn->session_state & STREAM5_STATE_MIDSTREAM) &&
            !(lwssn->session_state & STREAM5_STATE_ESTABLISHED))
        {
            /* Midstream and seen server. */
            if (lwssn->session_flags & SSNFLAG_SEEN_SERVER)
            {
                lwssn->session_state |= STREAM5_STATE_ESTABLISHED;
                lwssn->session_flags |= SSNFLAG_ESTABLISHED;
            }
        }
    }

    /*
     * check for SYN on reset session
     */
    if ((lwssn->session_flags & SSNFLAG_RESET) &&
        (p->tcph->th_flags & TH_SYN))
    {
        if ((!tcpssn) ||
            ((listener->s_mgr.state == TCP_STATE_CLOSED) ||
             (talker->s_mgr.state == TCP_STATE_CLOSED)))
        {
            PRE_SESSION_CLEANUP(lwssn);
            PRE_SESSION_CLEANUP_TARGET(lwssn);

            /* Listener previously issued a reset */
            /* Talker is re-SYN-ing */

            TcpSessionCleanup(lwssn);

            POST_SESSION_CLEANUP("SYN on RST ssn");

            if (p->tcph->th_flags & TH_RST)
            {
                /* Got SYN/RST.  We're done. */
                NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);
                tcpssn = NULL;
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return ACTION_RST;
            }
            else if (TCP_ISFLAGSET(p->tcph, TH_SYN) &&
                     !TCP_ISFLAGSET(p->tcph, TH_ACK))
            {
                lwssn->direction = FROM_CLIENT;
                IP_COPY_VALUE(lwssn->client_ip, GET_SRC_IP(p));
                lwssn->client_port = p->tcph->th_sport;
                IP_COPY_VALUE(lwssn->server_ip, GET_DST_IP(p));
                lwssn->server_port = p->tcph->th_dport;
                lwssn->session_state = STREAM5_STATE_SYN;
#ifdef ACTIVE_RESPONSE
                SetTTL(lwssn, p, 1);
#endif
                NewTcpSession(p, lwssn, tdb, s5TcpPolicy);

                tcpssn = (TcpSession *)lwssn->proto_specific_data->data;
                new_ssn = 1;

#ifdef NORMALIZER
                NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
                if (tcpssn)
                {
                    listener = &tcpssn->server;
                    talker = &tcpssn->client;
                }
                lwssn->session_flags = SSNFLAG_SEEN_CLIENT;
            }
            else if (TCP_ISFLAGSET(p->tcph, (TH_SYN|TH_ACK)))
            {
                lwssn->direction = FROM_SERVER;
                IP_COPY_VALUE(lwssn->client_ip, GET_DST_IP(p));
                lwssn->client_port = p->tcph->th_dport;
                IP_COPY_VALUE(lwssn->server_ip, GET_SRC_IP(p));
                lwssn->server_port = p->tcph->th_sport;
                lwssn->session_state = STREAM5_STATE_SYN_ACK;
#ifdef ACTIVE_RESPONSE
                SetTTL(lwssn, p, 0);
#endif
                NewTcpSession(p, lwssn, tdb, s5TcpPolicy);

#ifdef NORMALIZER
                NormalTrackECN(tcpssn, (TCPHdr*)p->tcph, require3Way);
#endif
                tcpssn = (TcpSession *)lwssn->proto_specific_data->data;
                new_ssn = 1;

                if (tcpssn)
                {
                    listener = &tcpssn->client;
                    talker = &tcpssn->server;
                }
                lwssn->session_flags = SSNFLAG_SEEN_SERVER;
            }
        }
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Got SYN pkt on reset ssn, re-SYN-ing\n"););
    }

    if (((p->packet_flags & PKT_FROM_SERVER) && (lwssn->ignore_direction & SSN_DIR_CLIENT)) ||
        ((p->packet_flags & PKT_FROM_CLIENT) && (lwssn->ignore_direction & SSN_DIR_SERVER)))
    {
        if (talker && (talker->flags & TF_FORCE_FLUSH))
        {
            Stream5FlushTalker(p, lwssn);
            talker->flags &= ~TF_FORCE_FLUSH;
        }

        if (listener && (listener->flags & TF_FORCE_FLUSH))
        {
            Stream5FlushListener(p, lwssn);
            listener->flags &= ~TF_FORCE_FLUSH;
        }

        Stream5DisableInspection(lwssn, p);
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Stream5 Ignoring packet from %d. "
                    "Session marked as ignore\n",
                    p->packet_flags & PKT_FROM_SERVER? "server" : "client"););
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ACTION_NOTHING;
    }

    /* Check if the session is to be ignored */
    ignore = CheckIgnoreChannel(p, &protoId);
    if (ignore)
    {
        /* Set the directions to ignore... */
        lwssn->ignore_direction = ignore;
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Stream5: Ignoring packet from %d. "
                    "Marking session marked as ignore.\n",
                    p->packet_flags & PKT_FROM_SERVER? "server" : "client"););
        Stream5DisableInspection(lwssn, p);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ACTION_NOTHING;
    }
    else if (protoId != 0)
    {
#ifdef TARGET_BASED
        if (IsAdaptiveConfigured(getRuntimePolicy(), 0))
            lwssn->application_protocol = protoId;
#endif
    }

    /* Handle data on SYN */
    if ((p->dsize) && TCP_ISFLAGSET(p->tcph, TH_SYN))
    {
        /* MacOS accepts data on SYN, so don't alert if policy is MACOS */
        if (StreamGetPolicy(lwssn, s5TcpPolicy, FROM_CLIENT) !=
            STREAM_POLICY_MACOS)
        {
#ifdef NORMALIZER
            if ( Normalize_IsEnabled(snort_conf, NORM_TCP) )
            {
                NormalTrimPayload(p, 0, tdb); // remove data on SYN
            }
            else
#endif
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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
        return ACTION_NOTHING | retcode;
    }

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "   %s [talker] state: %s\n", t, 
                state_names[talker->s_mgr.state]););
    STREAM5_DEBUG_WRAP(PrintFlushMgr(&talker->flush_mgr););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "   %s state: %s(%d)\n", l, 
                state_names[listener->s_mgr.state], 
                listener->s_mgr.state););
    STREAM5_DEBUG_WRAP(PrintFlushMgr(&listener->flush_mgr););

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
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "Pkt ack is out of bounds, bailing!\n"););
                s5stats.tcp_discards++;
                NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);
                LogTcpEvents(listener->tcp_policy, eventcode);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return ACTION_BAD_PKT;
            }
        }

        talker->flags |= Stream5GetTcpTimestamp(p, &tdb->ts, 0);
        if (tdb->ts == 0)
            talker->flags |= TF_TSTAMP_ZERO;

        /*
         * catch resets sent by server 
         */
        if(p->tcph->th_flags & TH_RST)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "got RST\n"););

            NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);

            /* Reset is valid when in SYN_SENT if the
             * ack field ACKs the SYN.
             */
            if(ValidRstSynSent(listener, tdb))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "got RST, closing talker\n"););
                /* Reset is valid */
                /* Mark session as reset... Leave it around so that any
                 * additional data sent from one side or the other isn't
                 * processed (and is dropped in inline mode).
                 */
                lwssn->session_flags |= SSNFLAG_RESET;
                talker->s_mgr.state = TCP_STATE_CLOSED;
                Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_CLOSING);
                /* Leave listener open, data may be in transit */
                LogTcpEvents(listener->tcp_policy, eventcode);
                PREPROC_PROFILE_END(s5TcpStatePerfStats);
                return ACTION_RST;
            }
            /* Reset not valid. */
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "bad sequence number, bailing\n"););
            s5stats.tcp_discards++;
            eventcode |= EVENT_BAD_RST;
            NormalDropPacketIf(p, NORM_TCP);
            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return ACTION_NOTHING;
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
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "Finish server init got called!\n"););
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "Finish server init didn't get called!\n"););
        }

        if((p->tcph->th_flags & TH_ECE) && 
            lwssn->session_flags & SSNFLAG_ECN_CLIENT_QUERY)
        {
            lwssn->session_flags |= SSNFLAG_ECN_SERVER_REPLY;
        }

        /*
         * explicitly set the state
         */
        listener->s_mgr.state = TCP_STATE_SYN_SENT;
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Accepted SYN ACK\n"););
        LogTcpEvents(listener->tcp_policy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ACTION_NOTHING;
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
    if (listener->tcp_policy->flags & STREAM5_CONFIG_CHECK_SESSION_HIJACKING)
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
        NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);

        if(ValidRst(listener, tdb))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Got RST, bailing\n"););
            lwssn->session_flags |= SSNFLAG_RESET;
            talker->s_mgr.state = TCP_STATE_CLOSED;
            talker->s_mgr.sub_state |= SUB_RST_SENT;
            Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_CLOSING);

#ifdef NORMALIZER
            if ( Normalize_IsEnabled(snort_conf, NORM_TCP_IPS) )
                listener->s_mgr.state = TCP_STATE_CLOSED;
            /* else for ids:
                leave listener open, data may be in transit */
#endif

            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return ACTION_RST;
        }
        /* Reset not valid. */
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "bad sequence number, bailing\n"););
        s5stats.tcp_discards++;
        eventcode |= EVENT_BAD_RST;
        NormalDropPacketIf(p, NORM_TCP);
        LogTcpEvents(listener->tcp_policy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ACTION_NOTHING | ts_action;
    }
    else
    {
        /* check for valid seqeuence/retrans */
        if(!ValidSeq(listener, tdb))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "bad sequence number, bailing\n"););
            s5stats.tcp_discards++;
            NormalTrimPayloadIf(p, NORM_TCP, 0, tdb);
            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return ACTION_NOTHING | ts_action;
        }
    }

    if (ts_action != ACTION_NOTHING)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "bad timestamp, bailing\n"););
        s5stats.tcp_discards++;
        NormalDropPacketIf(p, NORM_TCP);
        LogTcpEvents(listener->tcp_policy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ts_action;
    }

    /*
     * update PAWS timestamps
     */
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "PAWS update tdb->seq %lu > listener->r_win_base %lu\n",
                tdb->seq, listener->r_win_base););
    if(got_ts && SEQ_EQ(listener->r_nxt_ack, tdb->seq))
    {
        if((int32_t)(tdb->ts - talker->ts_last) >= 0 ||
           (uint32_t)p->pkth->ts.tv_sec >= talker->ts_last_pkt+PAWS_24DAYS)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "updating timestamps...\n"););
            talker->ts_last = tdb->ts;
            talker->ts_last_pkt = p->pkth->ts.tv_sec;
        }
    }
    else
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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
        if ( NormalDropPacketIf(p, NORM_TCP) )
            action = ACTION_BAD_PKT;
        else
#endif
            action = RepeatedSyn(listener, talker, tdb, tcpssn);

        if (action != ACTION_NOTHING)
        {
            /* got a bad SYN on the session, alert! */
            eventcode |= EVENT_SYN_ON_EST;
            LogTcpEvents(listener->tcp_policy, eventcode);
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return action;
        }
    }

    /*
     * Check that the window is within the limits
     */
    if (listener->tcp_policy->max_window && (tdb->win > listener->tcp_policy->max_window))
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Got window that was beyond the allowed policy value, bailing\n"););
        /* got a window too large, alert! */
        eventcode |= EVENT_WINDOW_TOO_LARGE;
        s5stats.tcp_discards++;
        NormalDropPacketIf(p, NORM_TCP);
        LogTcpEvents(listener->tcp_policy, eventcode);
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ACTION_NOTHING | ACTION_BAD_PKT;
    }
    else if ( !tdb->win && !talker->total_bytes_queued &&
        !(p->tcph->th_flags & (TH_FIN|TH_RST)) )
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
            "Window slammed shut!\n"););
        /* got a window slam alert! */
        eventcode |= EVENT_WINDOW_SLAM;
        s5stats.tcp_discards++;
        LogTcpEvents(listener->tcp_policy, eventcode);

#ifdef NORMALIZER
        if ( NormalDropPacketIf(p, NORM_TCP) )
        {
            PREPROC_PROFILE_END(s5TcpStatePerfStats);
            return ACTION_NOTHING | ACTION_BAD_PKT;
        }
#endif
    }

    if(talker->s_mgr.state_queue != TCP_STATE_NONE)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Found queued state transition on ack 0x%X, "
                    "current 0x%X!\n", talker->s_mgr.transition_seq, 
                    tdb->ack););
        if(tdb->ack == talker->s_mgr.transition_seq)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "accepting transition!\n"););
            talker->s_mgr.state = talker->s_mgr.state_queue;
            talker->s_mgr.state_queue = TCP_STATE_NONE;
        }
    }
    
    /* 
     * process ACK flags
     */
    if(p->tcph->th_flags & TH_ACK)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Got an ACK...\n"););
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "   %s [listener] state: %s\n", l, 
                    state_names[listener->s_mgr.state]););

        switch(listener->s_mgr.state)
        {
            case TCP_STATE_SYN_SENT:
                if ( !require3Way )
                    break;
                // fall thru ...
            case TCP_STATE_SYN_RCVD:
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "listener state is SYN_SENT...\n"););
                if(IsBetween(listener->l_unackd, listener->l_nxt_seq, tdb->ack) &&
                    ( !require3Way ||
                        ((talker->s_mgr.sub_state == SUB_SETUP_OK) &&
                        (listener->s_mgr.sub_state == SUB_SETUP_OK)) )) 
                {
                    UpdateSsn(p, listener, talker, tdb);
                    lwssn->session_flags |= SSNFLAG_ESTABLISHED;
                    lwssn->session_state |= STREAM5_STATE_ESTABLISHED;
                    listener->s_mgr.state = TCP_STATE_ESTABLISHED;
                    talker->s_mgr.state = TCP_STATE_ESTABLISHED;
                    Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_ESTABLISHED);
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
            case TCP_STATE_CLOSE_WAIT:
                UpdateSsn(p, listener, talker, tdb);
                break;

            case TCP_STATE_FIN_WAIT_1:
                UpdateSsn(p, listener, talker, tdb);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "tdb->ack %X >= talker->r_nxt_ack %X\n",
                            tdb->ack, talker->r_nxt_ack););
                if ( SEQ_EQ(tdb->ack, listener->l_nxt_seq) )
                {
                    listener->s_mgr.state = TCP_STATE_FIN_WAIT_2;

                    if ( (p->tcph->th_flags & TH_FIN) )
                    {
                        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                                "seq ok, setting state!\n"););

                        if (talker->s_mgr.state_queue == TCP_STATE_NONE)
                        {
                            talker->s_mgr.state = TCP_STATE_LAST_ACK;
                        }                  
                        // FIXTHIS this should be handled below in fin section
                        // but midstream sessions fail the seq test
                        listener->s_mgr.state_queue = TCP_STATE_TIME_WAIT;
                        listener->s_mgr.transition_seq = tdb->end_seq;
                        listener->s_mgr.expected_flags = TH_ACK;
                    }
                }
                else
                {
                    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                                "bad ack!\n"););
                }
                break;

            case TCP_STATE_FIN_WAIT_2:
                UpdateSsn(p, listener, talker, tdb);
                if ( SEQ_GT(tdb->ack, listener->l_nxt_seq) )
                {
                    eventcode |= EVENT_BAD_ACK;
                    LogTcpEvents(talker->tcp_policy, eventcode);
                    NormalDropPacketIf(p, NORM_TCP);
                    PREPROC_PROFILE_END(s5TcpStatePerfStats);
                    return ACTION_NOTHING | ACTION_BAD_PKT;
                }
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
                // FIXTHIS this special case should be eliminated
                if ( lwssn->session_flags & SSNFLAG_MIDSTREAM )
                {
                    if ( SEQ_EQ(tdb->ack+1, listener->l_nxt_seq) )
                        listener->s_mgr.state = TCP_STATE_CLOSED;
                }
                else if ( SEQ_EQ(tdb->ack, listener->l_nxt_seq) )
                {
                    listener->s_mgr.state = TCP_STATE_CLOSED;
                }
                break;

            default:
                // FIXTHIS safe to ignore when inline?
                break;
        }
    }

    /*
     * handle data in the segment
     */
    if(p->dsize)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
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
            NormalDropPacketIf(p, NORM_TCP);
        }
        else if (TCP_STATE_CLOSED == talker->s_mgr.state)
        {
            /* data on a segment when we're not accepting data any more */
            /* alert! */
            if (lwssn->session_flags & SSNFLAG_RESET)
            {
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
            NormalDropPacketIf(p, NORM_TCP);
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "Queuing data on listener, t %s, l %s...\n",
                        flush_policy_names[talker->flush_mgr.flush_policy],
                        flush_policy_names[listener->flush_mgr.flush_policy]););

#ifdef NORMALIZER
            // sender of syn w/mss limits payloads from peer
            // since we store mss on sender side, use listener mss
            // same reasoning for window size
            if ( Normalize_IsEnabled(snort_conf, NORM_TCP) )
            {
                // get the current window size
                StreamTracker* st = listener;
                uint32_t max = (st->r_win_base + st->l_window) - st->r_nxt_ack;

                // get lesser of current window or mss but
                // if mss is zero it is unset so don't use it
                if ( st->mss && st->mss < max )
                    max = st->mss;

                NormalTrimPayload(p, max, tdb);

                if ( Normalize_IsEnabled(snort_conf, NORM_TCP_ECN_STR) )
                    NormalCheckECN(tcpssn, p);
            }
#endif
            /* 
             * dunno if this is RFC but fragroute testing expects it
             * for the record, I've seen FTP data sessions that send
             * data packets with no tcp flags set
             */
            if ((p->tcph->th_flags != 0) || (s5TcpPolicy->policy == STREAM_POLICY_LINUX))
            {
                aligned = ProcessTcpData(p, listener, tcpssn, tdb, s5TcpPolicy);
            }
            else
            {
                eventcode |= EVENT_DATA_WITHOUT_FLAGS;
                NormalDropPacketIf(p, NORM_TCP);
            }
        }
    }

    if(p->tcph->th_flags & TH_FIN)
    {
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Got an FIN...\n"););
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "   %s state: %s(%d)\n", l, 
                    state_names[talker->s_mgr.state], 
                    talker->s_mgr.state););

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "checking ack (0x%X) vs nxt_ack (0x%X)\n",
                    tdb->end_seq, listener->r_win_base););
        if(SEQ_LT(tdb->end_seq,listener->r_win_base))
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "FIN inside r_win_base, bailing\n"););
            goto dupfin;
        }
        else
        {
            switch(talker->s_mgr.state)
            {
                case TCP_STATE_SYN_RCVD:
                case TCP_STATE_ESTABLISHED:
                    if (talker->s_mgr.state_queue == TCP_STATE_CLOSE_WAIT)
                    {
                        talker->s_mgr.state_queue = TCP_STATE_CLOSING;
                    }
                    talker->s_mgr.state = TCP_STATE_FIN_WAIT_1;

                    Stream5UpdatePerfBaseState(&sfBase, tcpssn->lwssn, TCP_STATE_CLOSING);
                    break;

                case TCP_STATE_CLOSE_WAIT:
                    talker->s_mgr.state = TCP_STATE_LAST_ACK;
                    break;

                default:
                    /* all other states stay where they are */
                    break;
            }
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
            }
            // need substate since we don't change state immediately
            if ( !(talker->s_mgr.sub_state & SUB_FIN_SENT) )
            {
                talker->l_nxt_seq++;
                listener->r_nxt_ack++;
                talker->s_mgr.sub_state |= SUB_FIN_SENT;
            }
            else if(SEQ_GEQ(tdb->end_seq,talker->l_nxt_seq))
            {
                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "FIN beyond l_nxt_seq, ignoring\n"););
                switch(talker->s_mgr.state)
                {
                    case TCP_STATE_FIN_WAIT_1:
                    case TCP_STATE_LAST_ACK:
                        eventcode |= EVENT_BAD_FIN;
                        LogTcpEvents(talker->tcp_policy, eventcode);
                        NormalDropPacketIf(p, NORM_TCP);
                        PREPROC_PROFILE_END(s5TcpStatePerfStats);
                        return ACTION_NOTHING | ACTION_BAD_PKT;
                }
            }
        }
    }

dupfin:

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "   %s [talker] state: %s\n", t, 
                state_names[talker->s_mgr.state]););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "   %s state: %s(%d)\n", l, 
                state_names[listener->s_mgr.state], 
                listener->s_mgr.state););

    /*
     * handle TIME_WAIT timer stuff
     */
    if((talker->s_mgr.state == TCP_STATE_TIME_WAIT && listener->s_mgr.state == TCP_STATE_CLOSED) ||
       (listener->s_mgr.state == TCP_STATE_TIME_WAIT && talker->s_mgr.state == TCP_STATE_CLOSED))
    {
//dropssn:
        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "Session terminating, flushing session buffers\n"););

        if(p->packet_flags & PKT_FROM_SERVER)
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "flushing FROM_SERVER\n"););
            if(talker->seg_bytes_logical)
                flush_stream(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport,
                        PKT_FROM_CLIENT);

            if(listener->seg_bytes_logical)
                flush_stream(tcpssn, listener, p,
                        GET_SRC_IP(p), GET_DST_IP(p),
                        p->tcph->th_sport, p->tcph->th_dport,
                        PKT_FROM_SERVER);
        }
        else
        {
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "flushing FROM_CLIENT\n"););
            if(listener->seg_bytes_logical)
            {
                flush_stream(tcpssn, listener, p,
                        GET_SRC_IP(p), GET_DST_IP(p),
                        p->tcph->th_sport, p->tcph->th_dport,
                        PKT_FROM_CLIENT);
            }
            if(talker->seg_bytes_logical)
            {
                flush_stream(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport,
                        PKT_FROM_SERVER);
            }
        }

        /* yoink that shit */
        LogTcpEvents(listener->tcp_policy, eventcode);
        DeleteLWSession(tcp_lws_cache, lwssn, "closed normally");
        p->ssnptr = NULL;
        PREPROC_PROFILE_END(s5TcpStatePerfStats);
        return ACTION_LWSSN_DELETED;
    }     
    else if(listener->s_mgr.state == TCP_STATE_CLOSED && talker->s_mgr.state == TCP_STATE_SYN_SENT)
    {
        if(p->tcph->th_flags & TH_SYN &&
           !(p->tcph->th_flags & TH_ACK) &&
           !(p->tcph->th_flags & TH_RST))
        {
            Stream5SetExpire(p, lwssn, s5TcpPolicy->session_timeout);
        }
    }

    CheckFlushPolicy(tcpssn, talker, listener, tdb, p);

    LogTcpEvents(listener->tcp_policy, eventcode);
    PREPROC_PROFILE_END(s5TcpStatePerfStats);
    return ACTION_NOTHING;
}

// this is for post-ack flushing
static INLINE int GetReverseDir (const Packet* p)
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
static INLINE uint32_t GetForwardDir (const Packet* p)
{
    if ( p->packet_flags & PKT_FROM_SERVER )
        return PKT_FROM_SERVER;

    else if ( p->packet_flags & PKT_FROM_CLIENT )
        return PKT_FROM_CLIENT;

    return 0;
}

static INLINE int CheckFlushCoercion (
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
#endif

int CheckFlushPolicy(TcpSession *tcpssn, StreamTracker *talker, 
        StreamTracker *listener, TcpDataBlock *tdb, Packet *p)
{
    uint32_t flushed = 0;

    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "In CheckFlushPolicy\n"););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Talker flush policy: %s\n", 
                flush_policy_names[talker->flush_mgr.flush_policy]););
    STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                "Listener flush policy: %s\n", 
                flush_policy_names[listener->flush_mgr.flush_policy]););

#ifdef NORMALIZER
    if ( (listener->flush_mgr.flush_policy == STREAM_FLPOLICY_FOOTPRINT)
          && Normalize_IsEnabled(snort_conf, NORM_TCP_IPS) )
    {
        int coerce;
        uint32_t avail;

        STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                    "STREAM_FLPOLICY_FOOTPRINT-IPS\n"););

        coerce = CheckFlushCoercion(
            p, &listener->flush_mgr, listener->tcp_policy->flush_factor);

        avail = get_q_sequenced(listener);

        if (
            (avail > 0) &&
            (coerce || (avail >= listener->flush_mgr.flush_pt) ||
            (avail && talker->s_mgr.state == TCP_STATE_FIN_WAIT_1))
        ) {
            uint32_t dir = GetForwardDir(p);

            if ( talker->s_mgr.state == TCP_STATE_FIN_WAIT_1 )
                listener->flags |= TF_FORCE_FLUSH;

            flushed = flush_to_seq(tcpssn, listener, avail, p,
                    GET_SRC_IP(p), GET_DST_IP(p),
                    p->tcph->th_sport, p->tcph->th_dport, dir);

            if ( flushed )
                purge_ackd(tcpssn, listener);
        }
        return flushed;
    }
#endif
    switch(talker->flush_mgr.flush_policy)
    {
        case STREAM_FLPOLICY_IGNORE:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "STREAM_FLPOLICY_IGNORE\n"););
            return 0;

        case STREAM_FLPOLICY_FOOTPRINT:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "STREAM_FLPOLICY_FOOTPRINT\n"););
            {
                if(get_q_footprint(talker) >= talker->flush_mgr.flush_pt)
                {
                    uint32_t dir = GetReverseDir(p);

                    flushed = flush_ackd(tcpssn, talker, p,
                            GET_DST_IP(p), GET_SRC_IP(p),
                            p->tcph->th_dport, p->tcph->th_sport, dir);

                    if(flushed)
                        purge_ackd(tcpssn, talker);
                }
            }
            break;

        case STREAM_FLPOLICY_LOGICAL:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "STREAM_FLPOLICY_LOGICAL\n"););
            if(talker->seg_bytes_logical > talker->flush_mgr.flush_pt)
            {
                uint32_t dir = GetReverseDir(p);

                flushed = flush_ackd(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                if(flushed)
                    purge_ackd(tcpssn, talker);
            }
            break;

        case STREAM_FLPOLICY_RESPONSE:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "Running FLPOLICY_RESPONSE\n"););
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "checking l.r_win_base (0x%X) > "
                        "t.seglist_base_seq (0x%X)\n", 
                        talker->r_win_base, talker->seglist_base_seq););

            if(SEQ_GT(talker->r_win_base, talker->seglist_base_seq) && 
                    IsWellFormed(p, talker))
            {
                uint32_t dir = GetReverseDir(p);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "flushing talker, t->sbl: %d\n",
                            talker->seg_bytes_logical););
                //PrintStreamTracker(talker);
                //PrintStreamTracker(talker);

                flushed = flush_ackd(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "bye bye data...\n"););

                if(flushed)
                    purge_ackd(tcpssn, talker);
            }
            break;

        case STREAM_FLPOLICY_SLIDING_WINDOW:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "STREAM_FLPOLICY_SLIDING_WINDOW\n"););
            if(get_q_footprint(talker) >= talker->flush_mgr.flush_pt)
            {
                uint32_t dir = GetReverseDir(p);

                flushed = flush_ackd(tcpssn, talker, p,
                        GET_DST_IP(p), GET_SRC_IP(p),
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "Deleting head node for sliding window...\n"););

                /* Base sequence for next window'd flush is the end
                 * of the first packet. */
                talker->seglist_base_seq = talker->seglist->seq + talker->seglist->size;
                Stream5SeglistDeleteNode(talker, talker->seglist);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "setting talker->seglist_base_seq to 0x%X\n", 
                            talker->seglist->seq););

            }
            break;

#if 0
        case STREAM_FLPOLICY_CONSUMED:
            STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                        "STREAM_FLPOLICY_CONSUMED\n"););
            if(get_q_footprint(talker) >= talker->flush_mgr.flush_pt)
            {
                uint32_t dir = GetReverseDir(p);

                flushed = flush_ackd(tcpssn, talker, p,
                        p->iph->ip_dst.s_addr, p->iph->ip_src.s_addr,
                        p->tcph->th_dport, p->tcph->th_sport, dir);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "Deleting head node for sliding window...\n"););

                talker->seglist_base_seq = talker->seglist->seq + talker->seglist->size;
                /* TODO: Delete up to the consumed bytes */
                Stream5SeglistDeleteNode(talker, talker->seglist);

                STREAM5_DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE, 
                            "setting talker->seglist_base_seq to 0x%X\n", 
                            talker->seglist->seq););

            }
            break;
#endif

    }
    
    return flushed;
}

static void Stream5SeglistAddNode(StreamTracker *st, StreamSegment *prev,
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
#ifdef DEBUG
    new->ordinal = st->segment_ordinal++;
    if (new->next && (new->next->seq == new->seq))
    {
        LogMessage("Same seq to right, check me\n");
    }
#endif
    return;
}

static int Stream5SeglistDeleteNode(StreamTracker *st, StreamSegment *seg)
{
    int ret;

    if(st == NULL || seg == NULL)
        return 0;

    STREAM5_DEBUG_WRAP( DebugMessage(DEBUG_STREAM_STATE,  
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

    Stream5DropSegment(seg);
    st->seg_count--;

    return ret;
}

void TcpUpdateDirection(Stream5LWSession *ssn, char dir,
                        snort_ip_p ip, uint16_t port)
{
    TcpSession *tcpssn = (TcpSession *)ssn->proto_specific_data->data;
    snort_ip tmpIp;
    uint16_t tmpPort;
    StreamTracker tmpTracker;

#ifdef SUP_IP6
    if (IP_EQUALITY(&tcpssn->tcp_client_ip, ip) && (tcpssn->tcp_client_port == port))
    {
        if ((dir == SSN_DIR_CLIENT) && (ssn->direction == SSN_DIR_CLIENT))
        {
            /* Direction already set as client */
            return;
        }
    }
    else if (IP_EQUALITY(&tcpssn->tcp_server_ip, ip) && (tcpssn->tcp_server_port == port))
    {
        if ((dir == SSN_DIR_SERVER) && (ssn->direction == SSN_DIR_SERVER))
        {
            /* Direction already set as server */
            return;
        }
    }
#else
    if (IP_EQUALITY(tcpssn->tcp_client_ip, ip) && (tcpssn->tcp_client_port == port))
    {
        if ((dir == SSN_DIR_CLIENT) && (ssn->direction == SSN_DIR_CLIENT))
        {
            /* Direction already set as client */
            return;
        }
    }
    else if (IP_EQUALITY(tcpssn->tcp_server_ip, ip) && (tcpssn->tcp_server_port == port))
    {
        if ((dir == SSN_DIR_SERVER) && (ssn->direction == SSN_DIR_SERVER))
        {
            /* Direction already set as server */
            return;
        }
    }
#endif

    /* Swap them -- leave ssn->direction the same */

    /* XXX: Gotta be a more efficient way to do this without the memcpy */
    tmpIp = tcpssn->tcp_client_ip;
    tmpPort = tcpssn->tcp_client_port;
    tcpssn->tcp_client_ip = tcpssn->tcp_server_ip;
    tcpssn->tcp_client_port = tcpssn->tcp_server_port;
    tcpssn->tcp_server_ip = tmpIp;
    tcpssn->tcp_server_port = tmpPort;
    memcpy(&tmpTracker, &tcpssn->client, sizeof(StreamTracker));
    memcpy(&tcpssn->client, &tcpssn->server, sizeof(StreamTracker));
    memcpy(&tcpssn->server, &tmpTracker, sizeof(StreamTracker));

}

/* Iterates through the packets that were reassembled for
 * logging of tagged packets.
 */
int GetTcpRebuiltPackets(Packet *p, Stream5LWSession *ssn,
        PacketIterator callback, void *userdata)
{
    int packets = 0;
    TcpSession *tcpssn = (TcpSession *)ssn->proto_specific_data->data;
    StreamTracker *st;
    StreamSegment *ss;

    if (!tcpssn)
    {
        return packets;
    }

    /* StreamTracker is the opposite of the ip of the reassembled
     * packet --> it came out the queue for the other side */
#ifdef SUP_IP6
    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
#else
    if (IP_EQUALITY(GET_SRC_IP(p), tcpssn->tcp_client_ip))
#endif
    {
        st = &tcpssn->server;
    }
    else
    {
        st = &tcpssn->client;
    }

    // skip over previously returned segments
    for (ss = st->seglist; ss && ss->buffered == SL_BUF_DUMPED; ss = ss->next);

    // return flushed segments only
    for (; ss && ss->buffered == SL_BUF_FLUSHED; ss = ss->next)
    {
        callback(&ss->pkth, ss->pkt, userdata);
        packets++;
        ss->buffered = SL_BUF_DUMPED;
    }

    return packets;
}

/* Iterates through the packets that were reassembled for
 * logging of tagged packets.
 */
int GetTcpStreamSegments(Packet *p, Stream5LWSession *ssn,
        StreamSegmentIterator callback, void *userdata)
{
    int packets = 0;
    TcpSession *tcpssn = (TcpSession *)ssn->proto_specific_data->data;
    StreamTracker *st;
    StreamSegment *ss;

    if (tcpssn == NULL)
        return -1;

    /* StreamTracker is the opposite of the ip of the reassembled
     * packet --> it came out the queue for the other side */
#ifdef SUP_IP6
    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
#else
    if (IP_EQUALITY(GET_SRC_IP(p), tcpssn->tcp_client_ip))
#endif
        st = &tcpssn->server;
    else
        st = &tcpssn->client;

    // skip over previously returned segments
    for (ss = st->seglist; ss && ss->buffered == SL_BUF_DUMPED; ss = ss->next);

    // return flushed segments only
    for (; ss && ss->buffered == SL_BUF_FLUSHED; ss = ss->next)
    {
        if (callback(&ss->pkth, ss->pkt, ss->data, ss->seq, userdata) != 0)
            return -1;

        packets++;
        ss->buffered = SL_BUF_DUMPED;
    }

    return packets;
}

int Stream5AddSessionAlertTcp(Stream5LWSession *lwssn, Packet *p, uint32_t gid, uint32_t sid)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

#ifdef SUP_IP6
    if (IP_EQUALITY(GET_SRC_IP(p),&tcpssn->tcp_client_ip))
#else
    if (IP_EQUALITY(GET_SRC_IP(p),tcpssn->tcp_client_ip))
#endif
    {
        st = &tcpssn->server;
    }
    else
    {
        st = &tcpssn->client;
    }

    if (st->alert_count >= MAX_SESSION_ALERTS)
        return 0;

    st->alerts[st->alert_count].gid = gid;
    st->alerts[st->alert_count].sid = sid;
    st->alerts[st->alert_count].seq = p->tcph->th_seq;
    st->alert_count++;

    return 0;
}

int Stream5CheckSessionAlertTcp(Stream5LWSession *lwssn, Packet *p, uint32_t gid, uint32_t sid)
{
    TcpSession *tcpssn = NULL;
    StreamTracker *st;
    int i;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
    {
        return 0;
    }

    /* If this is not a rebuilt packet, no need to check further */
    if (!(p->packet_flags & PKT_REBUILT_STREAM))
    {
        return 0;
    }

#ifdef SUP_IP6
    if (IP_EQUALITY(GET_SRC_IP(p), &tcpssn->tcp_client_ip))
#else
    if (p->iph->ip_src.s_addr == tcpssn->tcp_client_ip)
#endif
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

    return 0;
}

char Stream5GetReassemblyDirectionTcp(Stream5LWSession *lwssn)
{
    TcpSession *tcpssn = NULL;
    char dir = SSN_DIR_NONE;

    if (!lwssn)
        return SSN_DIR_NONE;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return SSN_DIR_NONE;

    if ((tcpssn->server.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE) &&
        (tcpssn->server.flush_mgr.flush_policy != STREAM_FLPOLICY_IGNORE))
    {
        dir |= SSN_DIR_SERVER;
    }

    if ((tcpssn->client.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE) &&
        (tcpssn->client.flush_mgr.flush_policy != STREAM_FLPOLICY_IGNORE))
    {
        dir |= SSN_DIR_CLIENT;
    }

    return dir;
}

uint32_t Stream5GetFlushPointTcp(Stream5LWSession *lwssn, char dir)
{
    TcpSession *tcpssn = NULL;

    if (lwssn == NULL)
        return 0;

    if (lwssn->proto_specific_data != NULL)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (tcpssn == NULL)
        return 0;

    if (dir & SSN_DIR_CLIENT)
        return tcpssn->client.flush_mgr.flush_pt;
    else if (dir & SSN_DIR_SERVER)
        return tcpssn->server.flush_mgr.flush_pt;

    return 0;
}

void Stream5SetFlushPointTcp(Stream5LWSession *lwssn,
        char dir, uint32_t flush_point)
{
    TcpSession *tcpssn = NULL;

    if (lwssn == NULL)
        return;

    if (lwssn->proto_specific_data != NULL)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (tcpssn == NULL)
        return;

    if (flush_point == 0)
        return;

    if (dir & SSN_DIR_CLIENT)
    {
        tcpssn->client.flush_mgr.flush_pt = flush_point;
        tcpssn->client.flush_mgr.last_size = 0;
        tcpssn->client.flush_mgr.last_count = 0;
    }
    else if (dir & SSN_DIR_SERVER)
    {
        tcpssn->server.flush_mgr.flush_pt = flush_point;
        tcpssn->server.flush_mgr.last_size = 0;
        tcpssn->server.flush_mgr.last_count = 0;
    }
}

char Stream5SetReassemblyTcp(Stream5LWSession *lwssn,
                                   uint8_t flush_policy,
                                   char dir,
                                   char flags)
{
    TcpSession *tcpssn = NULL;

    if (!lwssn)
        return SSN_DIR_NONE;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return SSN_DIR_NONE;

    if (flags & STREAM_FLPOLICY_SET_APPEND)
    {
        if (dir & SSN_DIR_CLIENT)
        {
            if (tcpssn->client.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE)
            {
                /* Changing policy with APPEND, Bad */
                DEBUG_WRAP(
                    LogMessage("Stream: Changing client flush policy using "
                            "append is asking for trouble.  Ignored\n"););
            }
            else
            {
                tcpssn->client.flush_mgr.flush_policy = flush_policy;
                /* And Initialize the Flush Mgr */
                InitFlushMgr(&tcpssn->client.flush_mgr,
                    &tcpssn->client.tcp_policy->flush_point_list,
                    flush_policy);
            }
        }

        if (dir & SSN_DIR_SERVER)
        {
            if (tcpssn->server.flush_mgr.flush_policy != STREAM_FLPOLICY_NONE)
            {
                /* Changing policy with APPEND, Bad */
                DEBUG_WRAP(
                    LogMessage("Stream: Changing server flush policy using "
                            "append is asking for trouble.  Ignored\n"););
            }
            else
            {
                tcpssn->server.flush_mgr.flush_policy = flush_policy;
                /* And Initialize the Flush Mgr */
                InitFlushMgr(&tcpssn->server.flush_mgr,
                    &tcpssn->server.tcp_policy->flush_point_list,
                    flush_policy);
            }
        }

    }
    else if (flags & STREAM_FLPOLICY_SET_ABSOLUTE)
    {
        if (dir & SSN_DIR_CLIENT)
        {
            tcpssn->client.flush_mgr.flush_policy = flush_policy;
            /* And Initialize the Flush Mgr */
            InitFlushMgr(&tcpssn->client.flush_mgr,
                &tcpssn->client.tcp_policy->flush_point_list,
                flush_policy);
        }

        if (dir & SSN_DIR_SERVER)
        {
            tcpssn->server.flush_mgr.flush_policy = flush_policy;
            /* And Initialize the Flush Mgr */
            InitFlushMgr(&tcpssn->server.flush_mgr, 
                &tcpssn->server.tcp_policy->flush_point_list,
                flush_policy);
        }
    }

    return Stream5GetReassemblyDirectionTcp(lwssn);
}

char Stream5GetReassemblyFlushPolicyTcp(Stream5LWSession *lwssn, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!lwssn)
        return STREAM_FLPOLICY_NONE;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return STREAM_FLPOLICY_NONE;

    if (dir & SSN_DIR_CLIENT)
    {
        return (char)tcpssn->client.flush_mgr.flush_policy;
    }

    if (dir & SSN_DIR_SERVER)
    {
        return (char)tcpssn->server.flush_mgr.flush_policy;
    }
    return STREAM_FLPOLICY_NONE;
}

char Stream5IsStreamSequencedTcp(Stream5LWSession *lwssn, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!lwssn)
        return 1;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return 1;

    if (dir & SSN_DIR_CLIENT)
    {
        if (tcpssn->server.flags & TF_MISSING_PKT)
            return 0;
    }

    if (dir & SSN_DIR_SERVER)
    {
        if (tcpssn->client.flags & TF_MISSING_PKT)
            return 0;
    }

    return 1;
}

/* This will falsly return SSN_MISSING_BEFORE on the first reassembed
 * packet if reassembly for this direction was set mid-session */
int Stream5MissingInReassembledTcp(Stream5LWSession *lwssn, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!lwssn)
        return SSN_MISSING_NONE;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return SSN_MISSING_NONE;

    if (dir & SSN_DIR_CLIENT)
    {
        if ((tcpssn->server.flags & TF_MISSING_PKT) &&
            (tcpssn->server.flags & TF_MISSING_PREV_PKT))
            return SSN_MISSING_BOTH;
        else if (tcpssn->server.flags & TF_MISSING_PREV_PKT)
            return SSN_MISSING_BEFORE;
        else if (tcpssn->server.flags & TF_MISSING_PKT)
            return SSN_MISSING_AFTER;
    }
    else if (dir & SSN_DIR_SERVER)
    {
        if ((tcpssn->client.flags & TF_MISSING_PKT) &&
            (tcpssn->client.flags & TF_MISSING_PREV_PKT))
            return SSN_MISSING_BOTH;
        else if (tcpssn->client.flags & TF_MISSING_PREV_PKT)
            return SSN_MISSING_BEFORE;
        else if (tcpssn->client.flags & TF_MISSING_PKT)
            return SSN_MISSING_AFTER;
    }

    return SSN_MISSING_NONE;
}

char Stream5PacketsMissingTcp(Stream5LWSession *lwssn, char dir)
{
    TcpSession *tcpssn = NULL;

    if (!lwssn)
        return 0;

    if (lwssn->proto_specific_data)
        tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

    if (!tcpssn)
        return 0;

    if (dir & SSN_DIR_CLIENT)
    {
        if (tcpssn->server.flags & TF_PKT_MISSED)
            return 1;
    }

    if (dir & SSN_DIR_SERVER)
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

int s5TcpStreamSizeInit(char *name, char *parameters, void **dataPtr)
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

    ssod = SnortAlloc(sizeof(StreamSizeOptionData));

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
        ssod->direction = SSN_DIR_CLIENT;
    }
    else if (!strcasecmp(toks[0], "server"))
    {
        ssod->direction = SSN_DIR_SERVER;
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

static INLINE int s5TcpStreamSizeCompare(uint32_t size1, uint32_t size2, char operator)
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
    Stream5LWSession *lwssn = NULL; 
    TcpSession *tcpssn = NULL;
    StreamSizeOptionData *ssod = (StreamSizeOptionData *)dataPtr;
    uint32_t client_size;
    uint32_t server_size;
    PROFILE_VARS;

    if (!pkt || !pkt->ssnptr || !ssod || !pkt->tcph)
        return DETECTION_OPTION_NO_MATCH;

    lwssn = pkt->ssnptr;
    if (!lwssn->proto_specific_data)
        return DETECTION_OPTION_NO_MATCH;

    PREPROC_PROFILE_START(streamSizePerfStats);

    tcpssn = (TcpSession *)lwssn->proto_specific_data->data;

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
        case SSN_DIR_CLIENT:
            if (s5TcpStreamSizeCompare(client_size, ssod->size, ssod->operator)
                == SSOD_MATCH)
            {
                PREPROC_PROFILE_END(streamSizePerfStats);
                return DETECTION_OPTION_MATCH;
            }
            break;
        case SSN_DIR_SERVER:
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
        free(ssod);
    }
}

typedef struct _StreamReassembleRuleOptionData
{
    char enable;
    char alert;
    char direction;
    char fastpath;
} StreamReassembleRuleOptionData;

int s5TcpStreamReassembleRuleOptionInit(char *name, char *parameters, void **dataPtr)
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

    srod = SnortAlloc(sizeof(StreamReassembleRuleOptionData));

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
        srod->direction = SSN_DIR_SERVER;
    }
    else if (!strcasecmp(toks[1], "server"))
    {
        srod->direction = SSN_DIR_CLIENT;
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
    Stream5LWSession *lwssn = NULL; 
    StreamReassembleRuleOptionData *srod = (StreamReassembleRuleOptionData *)dataPtr;
    PROFILE_VARS;

    if (!pkt || !pkt->ssnptr || !srod || !pkt->tcph)
        return 0;

    PREPROC_PROFILE_START(streamReassembleRuleOptionPerfStats);
    lwssn = pkt->ssnptr;

    if (!srod->enable) /* Turn it off */
        Stream5SetReassemblyTcp(lwssn, STREAM_FLPOLICY_IGNORE, srod->direction, STREAM_FLPOLICY_SET_ABSOLUTE);
    else
        Stream5SetReassemblyTcp(lwssn, STREAM_FLPOLICY_FOOTPRINT, srod->direction, STREAM_FLPOLICY_SET_ABSOLUTE);

    if (srod->fastpath)
    {
        /* Turn off inspection */
        lwssn->ignore_direction |= srod->direction;
        Stream5DisableInspection(lwssn, pkt);

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
        free(srod);
    }
}

void s5TcpSetPortFilterStatus(unsigned short port, int status, tSfPolicyId policyId, int parsing)
{
    Stream5Config *config;
    Stream5TcpConfig *tcp_config;

#ifdef SNORT_RELOAD
    if (parsing && (s5_swap_config != NULL))
        config = (Stream5Config *)sfPolicyUserDataGet(s5_swap_config, policyId);
    else
#endif
    config = (Stream5Config *)sfPolicyUserDataGet(s5_config, policyId);

    if (config == NULL)
        return;

    tcp_config = config->tcp_config;
    if (tcp_config == NULL)
        return;

    tcp_config->port_filter[port] |= status;
}

int s5TcpGetPortFilterStatus(unsigned short port, tSfPolicyId policyId, int parsing)
{
    Stream5Config *config;
    Stream5TcpConfig *tcp_config;

#ifdef SNORT_RELOAD
    if (parsing && (s5_swap_config != NULL))
        config = (Stream5Config *)sfPolicyUserDataGet(s5_swap_config, policyId);
    else
#endif
    config = (Stream5Config *)sfPolicyUserDataGet(s5_config, policyId);

    if (config == NULL)
        return PORT_MONITOR_NONE;

    tcp_config = config->tcp_config;
    if (tcp_config == NULL)
        return PORT_MONITOR_NONE;

    return tcp_config->port_filter[port];
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

