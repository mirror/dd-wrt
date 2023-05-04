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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_debug.h"
#include "detect.h"
#include "plugbase.h"
#include "mstring.h"
#include "sfxhash.h"
#include "util.h"
#include "decode.h"
#include "memory_stats.h"

#include "spp_session.h"
#include "session_api.h"
#include "snort_session.h"

#include "stream_common.h"
#include "stream_api.h"
#include "session_expect.h"
#include "snort_stream_udp.h"

#include "plugin_enum.h"
#include "rules.h"
#include "treenodes.h"
#include "snort.h"
#include "active.h"

#include "portscan.h" /* To know when to create sessions for all UDP */

#include "dynamic-plugins/sp_dynamic.h"

#include "profiler.h"
#include "sfPolicy.h"
#include "stream5_ha.h"

#include "reg_test.h"

#ifdef PERF_PROFILING
PreprocStats s5UdpPerfStats;
#endif

/*  M A C R O S  **************************************************/
/* actions */
#define ACTION_NOTHING                  0x00000000

/* sender/responder ip/port dereference */
#define udp_sender_ip lwSsn->client_ip
#define udp_sender_port lwSsn->client_port
#define udp_responder_ip lwSsn->server_ip
#define udp_responder_port lwSsn->server_port

/*  D A T A  S T R U C T U R E S  ***********************************/
typedef struct _UdpSession
{
    SessionControlBlock *lwSsn;

    struct timeval ssn_time;

    //uint8_t    c_ttl;
    //uint8_t    s_ttl;

} UdpSession;

/*  G L O B A L S  **************************************************/
static SessionCache* udp_lws_cache = NULL;

/*  P R O T O T Y P E S  ********************************************/
static void StreamParseUdpArgs(StreamUdpConfig *, char *, StreamUdpPolicy *);
static void StreamPrintUdpConfig(StreamUdpPolicy *);
static int ProcessUdp(SessionControlBlock *, Packet *, StreamUdpPolicy *, SFXHASH_NODE *);
static int ProcessUdpCreate (Packet *);

#ifdef ENABLE_HA

//-------------------------------------------------------------------------
// udp ha stuff
// TBD there may be some refactoring possible once tcp, icmp, and udp
// are complete

static SessionControlBlock *StreamUDPCreateSession(const SessionKey *key)
{
    setNapRuntimePolicy(getDefaultPolicy());

    SessionControlBlock *scb = session_api->create_session(udp_lws_cache, NULL, key );
    if (scb)
        s5stats.active_udp_sessions++;

    return scb;
}

static int StreamUDPDeleteSession(const SessionKey *key)
{
     SessionControlBlock *scb = session_api->get_session_by_key(udp_lws_cache, key);

    if (scb)
    {
        if( StreamSetRuntimeConfiguration( scb, scb->protocol ) == 0 )
        {
            session_api->delete_session( udp_lws_cache, scb, "ha sync", false );
            s5stats.active_udp_sessions--;
        }
        else
            WarningMessage(" WARNING: Attempt to delete a UDP Session when no valid runtime configuration.\n" );
     }

    return 0;
}


static HA_Api ha_udp_api = {
    /*.get_lws = */ GetLWUdpSession,

    /*.create_session = */ StreamUDPCreateSession,
    /*.deactivate_session = */ NULL,
    /*.delete_session = */ StreamUDPDeleteSession,
};

#endif

//-------------------------------------------------------------------------

void StreamInitUdp( void )
{
    /* Now UDP */
    if (udp_lws_cache == NULL)
    {
         udp_lws_cache = session_api->init_session_cache( SESSION_PROTO_UDP,
                                                          sizeof( UdpSession ),
                                                          &UdpSessionCleanup);
    }

#ifdef ENABLE_HA
    ha_set_api(IPPROTO_UDP, &ha_udp_api);
#endif
}

void setUdpDirectionAndPorts( Packet *p, SessionControlBlock *scb )
{
    scb->ha_state.direction = FROM_SENDER;
    IP_COPY_VALUE(scb->client_ip, GET_SRC_IP(p));
    scb->client_port = p->udph->uh_sport;
    IP_COPY_VALUE(scb->server_ip, GET_DST_IP(p));
    scb->server_port = p->udph->uh_dport;
}

void StreamUdpPolicyInit(StreamUdpConfig *config, char *args)
{
    StreamUdpPolicy *s5UdpPolicy;

    if (config == NULL)
        return;

    s5UdpPolicy = (StreamUdpPolicy *)SnortPreprocAlloc(1,
                         sizeof(StreamUdpPolicy), PP_STREAM, PP_MEM_CATEGORY_CONFIG);

    StreamParseUdpArgs(config, args, s5UdpPolicy);

    config->num_policies++;

    /* Now add this context to the internal list */
    if (config->policy_list == NULL)
    {
        config->policy_list =
            (StreamUdpPolicy **)SnortPreprocAlloc(1, sizeof(StreamUdpPolicy *),
                                      PP_STREAM, PP_MEM_CATEGORY_CONFIG);
    }
    else
    {
        StreamUdpPolicy **tmpPolicyList =
            (StreamUdpPolicy **)SnortPreprocAlloc(config->num_policies,
                                      sizeof(StreamUdpPolicy *), PP_STREAM,
                                      PP_MEM_CATEGORY_CONFIG);

        memcpy(tmpPolicyList, config->policy_list,
               sizeof(StreamUdpPolicy *) * (config->num_policies - 1));

        SnortPreprocFree(config->policy_list, (config->num_policies - 1) *
                         sizeof(StreamUdpPolicy *), PP_STREAM, PP_MEM_CATEGORY_CONFIG);

        config->policy_list = tmpPolicyList;
    }

    config->policy_list[config->num_policies - 1] = s5UdpPolicy;

    // register callback with Session that determines direction and client/server ports
    registerDirectionPortCallback( SESSION_PROTO_UDP, setUdpDirectionAndPorts );

    StreamPrintUdpConfig(s5UdpPolicy);

#ifdef REG_TEST
    LogMessage("    UDP Session Size: %lu\n", (long unsigned int)sizeof(UdpSession));
#endif
}

static void StreamParseUdpArgs(StreamUdpConfig *config, char *args, StreamUdpPolicy *s5UdpPolicy)
{
    char **toks;
    int num_toks;
    int i;
    char *index;
    char **stoks = NULL;
    int s_toks;
    char *endPtr = NULL;

    if (s5UdpPolicy == NULL)
        return;

    s5UdpPolicy->session_timeout = STREAM_DEFAULT_SSN_TIMEOUT;
    s5UdpPolicy->flags = 0;

    if(args != NULL && strlen(args) != 0)
    {
        toks = mSplit(args, ",", 6, &num_toks, 0);

        i=0;

        while(i < num_toks)
        {
            index = toks[i];

            while(isspace((int)*index)) index++;

            stoks = mSplit(index, " ", 3, &s_toks, 0);

            if (s_toks == 0)
            {
                FatalError("%s(%d) => Missing parameter in Stream UDP config.\n",
                    file_name, file_line);
            }

            if(!strcasecmp(stoks[0], "timeout"))
            {
                if(stoks[1])
                {
                    s5UdpPolicy->session_timeout = strtoul(stoks[1], &endPtr, 10);
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  Integer parameter required.\n",
                            file_name, file_line);
                }

                if ((s5UdpPolicy->session_timeout > STREAM_MAX_SSN_TIMEOUT) ||
                    (s5UdpPolicy->session_timeout < STREAM_MIN_SSN_TIMEOUT))
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  "
                        "Must be between %d and %d\n",
                        file_name, file_line,
                        STREAM_MIN_SSN_TIMEOUT, STREAM_MAX_SSN_TIMEOUT);
                }

                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream UDP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else if (!strcasecmp(stoks[0], "ignore_any_rules"))
            {
                s5UdpPolicy->flags |= STREAM_CONFIG_IGNORE_ANY;

                if (s_toks > 1)
                {
                    FatalError("%s(%d) => Invalid Stream UDP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else
            {
                FatalError("%s(%d) => Invalid Stream UDP Policy option\n",
                            file_name, file_line);
            }

            mSplitFree(&stoks, s_toks);
            i++;
        }

        mSplitFree(&toks, num_toks);
    }

    if (s5UdpPolicy->bound_addrs == NULL)
    {
        if (config->default_policy != NULL)
        {
            FatalError("%s(%d) => Default Stream UDP Policy already set. "
                       "This policy must be bound to a specific host or "
                       "network.\n", file_name, file_line);
        }

        config->default_policy = s5UdpPolicy;
    }
    else
    {
        if (s5UdpPolicy->flags & STREAM_CONFIG_IGNORE_ANY)
        {
            FatalError("%s(%d) => \"ignore_any_rules\" option can be used only"
                       " with Default Stream UDP Policy\n", file_name, file_line);
        }
    }
}

static void StreamPrintUdpConfig(StreamUdpPolicy *s5UdpPolicy)
{
    LogMessage("Stream UDP Policy config:\n");
    LogMessage("    Timeout: %d seconds\n", s5UdpPolicy->session_timeout);
    if (s5UdpPolicy->flags)
    {
        LogMessage("    Options:\n");
        if (s5UdpPolicy->flags & STREAM_CONFIG_IGNORE_ANY)
        {
            LogMessage("        Ignore Any -> Any Rules: YES\n");
        }
    }
    //IpAddrSetPrint("    Bound Addresses:", s5UdpPolicy->bound_addrs);
}


int StreamVerifyUdpConfig(struct _SnortConfig *sc, StreamUdpConfig *config, tSfPolicyId policyId)
{
    if (config == NULL)
        return -1;

    if (!udp_lws_cache)
        return -1;

    if (config->num_policies == 0)
        return -1;

    /* Post-process UDP rules to establish UDP ports to inspect. */
    setPortFilterList(sc, config->port_filter, IPPROTO_UDP,
            (config->default_policy->flags & STREAM_CONFIG_IGNORE_ANY), policyId);

    //printf ("UDP Ports with Inspection/Monitoring\n");
    //s5PrintPortFilter(config->port_filter);
    return 0;
}

#ifdef STREAM_DEBUG_ENABLED
static void PrintUdpSession(UdpSession *us)
{
    LogMessage("UdpSession:\n");
    LogMessage("    ssn_time:           %lu\n", us->ssn_time.tv_sec);
    LogMessage("    sender IP:          0x%08X\n", us->udp_sender_ip);
    LogMessage("    responder IP:          0x%08X\n", us->udp_responder_ip);
    LogMessage("    sender port:        %d\n", us->udp_sender_port);
    LogMessage("    responder port:        %d\n", us->udp_responder_port);

    LogMessage("    flags:              0x%X\n", us->lwSsn->session_flags);
}
#endif

SessionControlBlock *GetLWUdpSession( const SessionKey *key )
{
    return session_api->get_session_by_key(udp_lws_cache, key);
}

void UdpSessionCleanup(void *ssn)
{
    SessionControlBlock *scb = ( SessionControlBlock * ) ssn;

    UdpSession *udpssn = NULL;

    if (scb->ha_state.session_flags & SSNFLAG_PRUNED)
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_PRUNED);
    }
    else if (scb->ha_state.session_flags & SSNFLAG_TIMEDOUT)
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_TIMEDOUT);
    }
    else
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_NORMALLY);
    }

    if (scb->proto_specific_data)
        udpssn = (UdpSession *)scb->proto_specific_data->data;

    if (!udpssn)
    {
        /* Huh? */
        return;
    }

    /* Cleanup the proto specific data */
    session_api->free_protocol_session_pool( SESSION_PROTO_UDP, scb );
    scb->proto_specific_data = NULL;
    scb->session_state = STREAM_STATE_NONE;
    scb->ha_state.session_flags = SSNFLAG_NONE;
    scb->expire_time = 0;
    scb->ha_state.ignore_direction = 0;

    StreamResetFlowBits(scb);
    session_api->free_application_data(scb);

    s5stats.udp_sessions_released++;
    s5stats.active_udp_sessions--;

    RemoveUDPSession(&sfBase);
}

uint32_t StreamGetUdpPrunes(void)
{
    if( udp_lws_cache)
        return session_api->get_session_prune_count( SESSION_PROTO_UDP );
    else
        return s5stats.udp_prunes;
}

void StreamResetUdpPrunes(void)
{
    session_api->reset_session_prune_count( SESSION_PROTO_UDP );
}

void StreamResetUdp(void)
{
    session_api->purge_session_cache(udp_lws_cache);
    session_api->clean_protocol_session_pool( SESSION_PROTO_UDP );
}

void StreamCleanUdp(void)
{
    if ( udp_lws_cache )
        s5stats.udp_prunes = session_api->get_session_prune_count( SESSION_PROTO_UDP );

    /* Clean up session cache */
    session_api->delete_session_cache( SESSION_PROTO_UDP );
    udp_lws_cache = NULL;

}

static int NewUdpSession(Packet *p,
                         SessionControlBlock *scb,
                         StreamUdpPolicy *s5UdpPolicy)
{
    UdpSession *tmp;
    MemBucket *tmpBucket;
    /******************************************************************
     * create new sessions
     *****************************************************************/
    tmpBucket = session_api->alloc_protocol_session( SESSION_PROTO_UDP );
    if (tmpBucket == NULL)
        return -1;

    tmp = tmpBucket->data;
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Creating new session tracker!\n"););

    tmp->ssn_time.tv_sec = p->pkth->ts.tv_sec;
    tmp->ssn_time.tv_usec = p->pkth->ts.tv_usec;
    scb->ha_state.session_flags |= SSNFLAG_SEEN_SENDER;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "adding UdpSession to lightweight session\n"););
    scb->proto_specific_data = tmpBucket;
    scb->protocol = GET_IPH_PROTO(p);
    scb->ha_state.direction = FROM_SENDER;
    tmp->lwSsn = scb;

#ifdef STREAM_DEBUG_ENABLED
    PrintUdpSession(tmp);
#endif
    session_api->set_expire_timer(p, scb, s5UdpPolicy->session_timeout);

    s5stats.udp_sessions_created++;
    s5stats.active_udp_sessions++;

    AddUDPSession(&sfBase);
    if (perfmon_config && (perfmon_config->perf_flags & SFPERF_FLOWIP))
        UpdateFlowIPState(&sfFlow, IP_ARG(scb->client_ip), IP_ARG(scb->server_ip), SFS_STATE_UDP_CREATED);

    return 0;
}

//-------------------------------------------------------------------------
/*
 * Main entry point for UDP
 */
int StreamProcessUdp( Packet *p, SessionControlBlock *scb,
                      StreamUdpPolicy *s5UdpPolicy, SessionKey *skey)
{
    SFXHASH_NODE *hash_node = NULL;
    PROFILE_VARS;

// XXX-IPv6 StreamProcessUDP debugging

    PREPROC_PROFILE_START(s5UdpPerfStats);

    if( s5UdpPolicy == NULL )
    {
        int policyIndex;
        StreamUdpConfig *udp_config = ( ( StreamConfig * ) scb->stream_config )->udp_config;
        if( udp_config == NULL )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                                    "[Stream] Could not find Udp Policy context "
                                    "for IP %s\n", inet_ntoa(GET_DST_ADDR(p))););
            PREPROC_PROFILE_END( s5UdpPerfStats );
            return 0;
        }

        /* Find an Udp policy for this packet */
        for( policyIndex = 0; policyIndex < udp_config->num_policies; policyIndex++ )
        {
            s5UdpPolicy = udp_config->policy_list[ policyIndex ];

            if( s5UdpPolicy->bound_addrs == NULL )
                continue;

            /*
             * Does this policy handle packets to this IP address?
             */
            if( sfvar_ip_in( s5UdpPolicy->bound_addrs, GET_DST_IP( p ) ) )
            {
                DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                                        "[Stream] Found udp policy in IpAddrSet\n"););
                break;
            }
        }

        if( policyIndex == udp_config->num_policies )
            s5UdpPolicy = udp_config->default_policy;

        if( s5UdpPolicy == NULL )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                                    "[Stream] Could not find Udp Policy context "
                                    "for IP %s\n", inet_ntoa(GET_DST_ADDR(p))););
            PREPROC_PROFILE_END(s5UdpPerfStats);
            return 0;
        }
   }

    /* UDP Sessions required */
    if ( !scb->session_established )
    {
        int rc;
#if defined(DAQ_CAPA_CST_TIMEOUT)
        uint64_t timeout;
        if (Daq_Capa_Timeout)
        {
          GetTimeout(p,&timeout);
          s5UdpPolicy->session_timeout = timeout;
        }
#endif
        scb->proto_policy = s5UdpPolicy;

        rc = isPacketFilterDiscard( p, s5UdpPolicy->flags & STREAM_CONFIG_IGNORE_ANY );
        if( ( rc == PORT_MONITOR_PACKET_DISCARD ) && !StreamExpectIsExpected( p, &hash_node ) )
        {
            //ignore the packet
            scb->session_state &= ~STREAM_STATE_PORT_INSPECT;
            UpdateFilteredPacketStats(&sfBase, IPPROTO_UDP);
            session_api->set_expire_timer(p, scb, s5UdpPolicy->session_timeout);
            PREPROC_PROFILE_END(s5UdpPerfStats);
            return 0;
        }
 
        scb->session_state |= STREAM_STATE_PORT_INSPECT;
        scb->session_established = true;
        s5stats.total_udp_sessions++;
        s5stats.active_udp_sessions++;
    }

    p->ssnptr = scb;

    /*
     * Check if the session is expired.
     * Should be done before we do something with the packet...
     * ie, Insert a packet, or handle state change SYN, FIN, RST, etc.
     */
    if( ( scb->session_state & STREAM_STATE_TIMEDOUT )
          ||
          StreamExpire( p, scb ) )
    {
        scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;

        /* Session is timed out */
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream UDP session timedout!\n"););

#ifdef ENABLE_HA
        /* Notify the HA peer of the session cleanup/reset by way of a deletion notification. */
        PREPROC_PROFILE_TMPEND(s5UdpPerfStats);
        SessionHANotifyDeletion(scb);
        PREPROC_PROFILE_TMPSTART(s5UdpPerfStats);
        scb->ha_flags = (HA_FLAG_NEW | HA_FLAG_MODIFIED | HA_FLAG_MAJOR_CHANGE);
#endif

        /* Clean it up */
        UdpSessionCleanup(scb);

        ProcessUdp(scb, p, s5UdpPolicy, hash_node);
    }
    else
    {
        ProcessUdp(scb, p, s5UdpPolicy, hash_node);
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Finished Stream UDP cleanly!\n"
                    "---------------------------------------------------\n"););
    }
    MarkupPacketFlags(p, scb);
    session_api->set_expire_timer(p, scb, s5UdpPolicy->session_timeout);
    PREPROC_PROFILE_END(s5UdpPerfStats);

    return 0;
}

static int ProcessUdp( SessionControlBlock *scb, Packet *p, StreamUdpPolicy *s5UdpPolicy,
                       SFXHASH_NODE *hash_node )
{
    char ignore;
    UdpSession *udpssn = NULL;

    if (scb->protocol != IPPROTO_UDP)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Lightweight session not UDP on UDP packet\n"););
        return ACTION_NOTHING;
    }

    if (scb->ha_state.session_flags & (SSNFLAG_DROP_CLIENT|SSNFLAG_DROP_SERVER))
    {
        /* Got a packet on a session that was dropped (by a rule). */
        session_api->set_packet_direction_flag(p, scb);

        /* Drop this packet */
        if (((p->packet_flags & PKT_FROM_SERVER) &&
             (scb->ha_state.session_flags & SSNFLAG_DROP_SERVER)) ||
            ((p->packet_flags & PKT_FROM_CLIENT) &&
             (scb->ha_state.session_flags & SSNFLAG_DROP_CLIENT)))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Blocking %s packet as session was blocked\n",
                        p->packet_flags & PKT_FROM_SERVER ?
                        "server" : "client"););
            DisableDetect( p );
            if ( scb->ha_state.session_flags & SSNFLAG_FORCE_BLOCK )
                Active_ForceDropSessionWithoutReset();
            else
                Active_DropSessionWithoutReset(p);
#ifdef ACTIVE_RESPONSE
            StreamActiveResponse(p, scb);
#endif
            if (pkt_trace_enabled)
                addPktTraceData(VERDICT_REASON_STREAM, snprintf(trace_line, MAX_TRACE_LINE,
                    "Stream: session was already blocked, %s\n", getPktTraceActMsg()));
            else addPktTraceData(VERDICT_REASON_STREAM, 0);
            return ACTION_NOTHING;
        }
    }

    if (scb->proto_specific_data != NULL)
        udpssn = (UdpSession *)scb->proto_specific_data->data;

    if (udpssn == NULL)
    {
        if (NewUdpSession(p, scb, s5UdpPolicy) == -1)
            return ACTION_NOTHING;
        udpssn = (UdpSession *)scb->proto_specific_data->data;

        /* Check if the session is to be ignored */
        if (hash_node)
            ignore = StreamExpectProcessNode(p, scb, hash_node);
        else
            ignore = StreamExpectCheck(p, scb);
        if (ignore)
        {
            /* Set the directions to ignore... */
            scb->ha_state.ignore_direction = ignore;
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                        "Stream: Ignoring packet from %d. "
                        "Marking session marked as ignore.\n",
                        p->packet_flags & PKT_FROM_CLIENT? "sender" : "responder"););
            session_api->disable_inspection(scb, p);
            return ACTION_NOTHING;
        }
    }

    /* figure out direction of this packet */
    session_api->set_packet_direction_flag(p, scb);

    if (((p->packet_flags & PKT_FROM_SERVER) && (scb->ha_state.ignore_direction & SSN_DIR_FROM_CLIENT)) ||
        ((p->packet_flags & PKT_FROM_CLIENT) && (scb->ha_state.ignore_direction & SSN_DIR_FROM_SERVER)))
    {
        session_api->disable_inspection(scb, p);
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream Ignoring packet from %d. "
                    "Session marked as ignore\n",
                    p->packet_flags & PKT_FROM_CLIENT? "sender" : "responder"););
        return ACTION_NOTHING;
    }

    /* if both seen, mark established */
    if(p->packet_flags & PKT_FROM_SERVER)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream: Updating on packet from responder\n"););
        scb->ha_state.session_flags |= SSNFLAG_SEEN_RESPONDER;
#ifdef ACTIVE_RESPONSE
        SetTTL(scb, p, 0);
#endif
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream: Updating on packet from client\n"););
        /* if we got here we had to see the SYN already... */
        scb->ha_state.session_flags |= SSNFLAG_SEEN_SENDER;
#ifdef ACTIVE_RESPONSE
        SetTTL(scb, p, 1);
#endif
    }

    if (!(scb->ha_state.session_flags & SSNFLAG_ESTABLISHED))
    {
        if ((scb->ha_state.session_flags & SSNFLAG_SEEN_SENDER) &&
            (scb->ha_state.session_flags & SSNFLAG_SEEN_RESPONDER))
        {
            scb->ha_state.session_flags |= SSNFLAG_ESTABLISHED;
        }
    }

    return ACTION_NOTHING;
}

int ProcessUdpCreate (Packet *p) 
{
   SFXHASH_NODE *hash_node = NULL;
   SessionControlBlock *scb;
   StreamUdpPolicy *s5UdpPolicy;
   PROFILE_VARS;

   scb = p->ssnptr;
   if (!scb) {
       DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "[Stream] Could not find Udp session Control block "));
       return 0;
   }

   s5UdpPolicy = scb->proto_policy;
   if (s5UdpPolicy == NULL) {
       DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                               "[Stream] Could not find Udp Policy context "
                               "for IP %s\n", inet_ntoa(GET_DST_ADDR(p))););
       return 0; 
    }

    PREPROC_PROFILE_START(s5UdpPerfStats);
    scb->session_established = true;
    s5stats.total_udp_sessions++;

    /*
     * Check if the session is expired.
     */
    if (( scb->session_state & STREAM_STATE_TIMEDOUT )
        || StreamExpire( p, scb )) {

        scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;

        /* Session is timed out */
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Stream UDP session timedout!\n"););

#ifdef ENABLE_HA
        /* Notify the HA peer of the session cleanup/reset by way of a deletion notification. */
        PREPROC_PROFILE_TMPEND(s5UdpPerfStats);
        SessionHANotifyDeletion(scb);
        PREPROC_PROFILE_TMPSTART(s5UdpPerfStats);
        scb->ha_flags = (HA_FLAG_NEW | HA_FLAG_MODIFIED | HA_FLAG_MAJOR_CHANGE);
#endif

        /* Clean it up */
        UdpSessionCleanup(scb);
        ProcessUdp(scb, p, s5UdpPolicy, hash_node);

    } else {
        ProcessUdp(scb, p, s5UdpPolicy, hash_node);
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                    "Finished Stream UDP cleanly!\n"
                    "---------------------------------------------------\n"););
    }
    MarkupPacketFlags(p, scb);
    session_api->set_expire_timer(p, scb, s5UdpPolicy->session_timeout);
    PREPROC_PROFILE_END(s5UdpPerfStats);

    return 0;
}

void InspectPortFilterUdp (Packet *p)
{
    int rc;
    SessionControlBlock *scb; 
    StreamUdpPolicy *s5UdpPolicy;

    scb = p->ssnptr;
    if (!scb) {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM,"[Stream] Sesssion control does not exist"));
        return;
    }

    s5UdpPolicy = scb->proto_policy;
    if (s5UdpPolicy == NULL) {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
                                "[Stream] Could not find Udp Policy context "
                                "for IP %s\n", inet_ntoa(GET_DST_ADDR(p))););
        return;
    }
    // If NAP had set port to be filtered, now check IPS portlist. 
    if (!(scb->session_state & STREAM_STATE_PORT_INSPECT)) { 

        rc = isPacketFilterDiscardUdp(p, s5UdpPolicy->flags & STREAM_CONFIG_IGNORE_ANY);
        if (rc == PORT_MONITOR_PACKET_PROCESS) {
            // Port is not present in NAP, but present in IPS portlist, flow will be tracked.  
            scb->session_state |= STREAM_STATE_PORT_INSPECT;
            // Complete UDP session/flow creation as it needs to tracked. 
            ProcessUdpCreate(p);
        }
        /*
         * If return value from isPacketFilterDiscardUdp() was PORT_MONITOR_PACKET_DISCARD, 
         * packet is marked either inspected/filtered, based NAP/IPS portlist flag evaluation.
         */
    } 
    return;
}

void UdpUpdateDirection(SessionControlBlock *ssn, char dir, sfaddr_t* ip, uint16_t port)
{
    UdpSession *udpssn = (UdpSession *)ssn->proto_specific_data->data;
    sfaddr_t tmpIp;
    uint16_t tmpPort;

    if (IP_EQUALITY(&udpssn->udp_sender_ip, ip) && (udpssn->udp_sender_port == port))
    {
        if ((dir == SSN_DIR_FROM_SENDER) && (ssn->ha_state.direction == SSN_DIR_FROM_SENDER))
        {
            /* Direction already set as SENDER */
            return;
        }
    }
    else if (IP_EQUALITY(&udpssn->udp_responder_ip, ip) && (udpssn->udp_responder_port == port))
    {
        if ((dir == SSN_DIR_FROM_RESPONDER) && (ssn->ha_state.direction == SSN_DIR_FROM_RESPONDER))
        {
            /* Direction already set as RESPONDER */
            return;
        }
    }

    /* Swap them -- leave ssn->ha_state.direction the same */
    tmpIp = udpssn->udp_sender_ip;
    tmpPort = udpssn->udp_sender_port;
    udpssn->udp_sender_ip = udpssn->udp_responder_ip;
    udpssn->udp_sender_port = udpssn->udp_responder_port;
    udpssn->udp_responder_ip = tmpIp;
    udpssn->udp_responder_port = tmpPort;
}

void s5UdpSetPortFilterStatus(struct _SnortConfig *sc, unsigned short port, uint16_t status, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->udp_config != NULL ) )
        config->udp_config->port_filter[ port ] |= status;
}

void s5UdpUnsetPortFilterStatus(struct _SnortConfig *sc, unsigned short port, uint16_t status, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->udp_config != NULL ) )
        config->udp_config->port_filter[ port ] &= ~status;
}

int s5UdpGetPortFilterStatus( struct _SnortConfig *sc, unsigned short port, tSfPolicyId policyId, int parsing)
{
    StreamConfig *config;

    config = getStreamPolicyConfig( policyId, parsing );
    if ( ( config != NULL ) && ( config->udp_config != NULL ) )
        return ( int ) config->udp_config->port_filter[ port ];
    else
        return PORT_MONITOR_NONE;
}

int s5UdpGetIPSPortFilterStatus(struct _SnortConfig *sc, unsigned short sport, unsigned short dport, tSfPolicyId policyId)
{
    if ( sc->udp_ips_port_filter_list && sc->udp_ips_port_filter_list[policyId] )
        return ( ((int) sc->udp_ips_port_filter_list[policyId]->port_filter[ sport ]) |
             ((int) sc->udp_ips_port_filter_list[policyId]->port_filter[ dport ]) ) ;
    else
        return PORT_MONITOR_NONE;
}

void StreamUdpConfigFree(StreamUdpConfig *config)
{
    int i;

    if (config == NULL)
        return;

    /* Cleanup TCP Policies and the list */
    for (i = 0; i < config->num_policies; i++)
    {
        StreamUdpPolicy *policy = config->policy_list[i];

        if (policy->bound_addrs != NULL)
            sfvar_free(policy->bound_addrs);
        SnortPreprocFree(policy, sizeof(StreamUdpPolicy), PP_STREAM,
                         PP_MEM_CATEGORY_CONFIG);
    }

    SnortPreprocFree(config->policy_list, config->num_policies *
                     sizeof(StreamUdpPolicy *),
                     PP_STREAM, PP_MEM_CATEGORY_CONFIG);
    SnortPreprocFree(config, sizeof(StreamUdpConfig), PP_STREAM,
                     PP_MEM_CATEGORY_CONFIG);
}

#ifdef SNORT_RELOAD
void SessionUDPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout)
{
    SessionReload(udp_lws_cache, max_sessions, pruningTimeout, nominalTimeout
#ifdef REG_TEST
                  , "UDP"
#endif
                  );
}

unsigned SessionUDPReloadAdjust(unsigned maxWork)
{
    return SessionProtocolReloadAdjust(udp_lws_cache, session_configuration->max_udp_sessions,
                                       maxWork, 0
#ifdef REG_TEST
                                       , "UDP"
#endif
                                       );
}
#endif

size_t get_udp_used_mempool()
{
    if (udp_lws_cache && udp_lws_cache->protocol_session_pool)
        return udp_lws_cache->protocol_session_pool->used_memory;

    return 0;
}
