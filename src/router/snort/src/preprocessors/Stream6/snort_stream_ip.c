/****************************************************************************
*
* Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
*  Copyright (C) 2005-2013 Sourcefire, Inc.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License Version 2 as
*  published by the Free Software Foundation.  You may not use, modify or
*  distribute this program under any other version of the GNU General
*  Public License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
* ***************************************************************************/

/*
 * @file    snort_stream_ip.c
 * @author  Russ Combs <rcombs@sourcefire.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "active.h"
#include "decode.h"
#include "detect.h"
#include "mstring.h"
#include "parser.h"
#include "profiler.h"
#include "sfPolicy.h"
#include "sfxhash.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "memory_stats.h"

#include "spp_session.h"
#include "session_api.h"
#include "snort_session.h"

#include "snort_stream_ip.h"
#include "session_expect.h"
#include "stream5_ha.h"
#include "util.h"

#include "reg_test.h"

#ifdef PERF_PROFILING
PreprocStats s5IpPerfStats;
#endif

static SessionCache* ip_lws_cache = NULL;

//-------------------------------------------------------------------------
// private methods
//-------------------------------------------------------------------------

static void StreamPrintIpConfig (StreamIpPolicy* policy)
{
    LogMessage("Stream IP Policy config:\n");
    LogMessage("    Timeout: %d seconds\n", policy->session_timeout);

}

static void StreamParseIpArgs (char* args, StreamIpPolicy* policy)
{
    char* *toks;
    int num_toks;
    int i;

    policy->session_timeout = STREAM_DEFAULT_SSN_TIMEOUT;

    if ( !args || !*args )
        return;

    toks = mSplit(args, ",", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        int s_toks;
        char* *stoks = mSplit(toks[i], " ", 2, &s_toks, 0);

        if (s_toks == 0)
        {
            ParseError("Missing parameter in Stream IP config.\n");
        }

        if(!strcasecmp(stoks[0], "timeout"))
        {
            char* endPtr = NULL;

            if(stoks[1])
            {
                policy->session_timeout = strtoul(stoks[1], &endPtr, 10);
            }

            if (!stoks[1] || (endPtr == &stoks[1][0]))
            {
                ParseError("Invalid timeout in config file.  Integer parameter required.\n");
            }

            if ((policy->session_timeout > STREAM_MAX_SSN_TIMEOUT) ||
                (policy->session_timeout < STREAM_MIN_SSN_TIMEOUT))
            {
                ParseError("Invalid timeout in config file.  Must be between %d and %d\n",
                    STREAM_MIN_SSN_TIMEOUT, STREAM_MAX_SSN_TIMEOUT);
            }
            if (s_toks > 2)
            {
                ParseError("Invalid Stream IP Policy option.  Missing comma?\n");
            }
        }
        else
        {
            ParseError("Invalid Stream IP policy option\n");
        }

        mSplitFree(&stoks, s_toks);
    }

    mSplitFree(&toks, num_toks);
}

void IpSessionCleanup (void* ssn)
{
    SessionControlBlock *scb = ( SessionControlBlock * ) ssn;
    
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

    StreamResetFlowBits(scb);
    session_api->free_application_data(scb);

    scb->ha_state.session_flags = SSNFLAG_NONE;
    scb->session_state = STREAM_STATE_NONE;

    scb->expire_time = 0;
    scb->ha_state.ignore_direction = 0;
    s5stats.active_ip_sessions--;
}


#ifdef ENABLE_HA

//-------------------------------------------------------------------------
// ip ha stuff
//-------------------------------------------------------------------------

SessionControlBlock *GetLWIpSession (const SessionKey *key)
{
    return session_api->get_session_by_key(ip_lws_cache, key);
}

static SessionControlBlock *StreamIPCreateSession (const SessionKey *key)
{
    setNapRuntimePolicy(getDefaultPolicy());

    SessionControlBlock *scb = session_api->create_session(ip_lws_cache, NULL, key);
    if (scb)
        s5stats.active_ip_sessions++;

    return scb;
}

static int StreamIPDeleteSession (const SessionKey *key)
{
    SessionControlBlock *scb = session_api->get_session_by_key(ip_lws_cache, key);

    if( scb != NULL )
    {
        if( StreamSetRuntimeConfiguration(scb, scb->protocol) == 0 )
        {
            session_api->delete_session(ip_lws_cache, scb, "ha sync", false);
            s5stats.active_ip_sessions--;
        }
        else
            WarningMessage(" WARNING: Attempt to delete an IP Session when no valid runtime configuration\n" );
    }

    return 0;
}


static HA_Api ha_ip_api = {
    /*.get_lws = */ GetLWIpSession,

    /*.create_session = */ StreamIPCreateSession,
    /*.deactivate_session = */ NULL,
    /*.delete_session = */ StreamIPDeleteSession,
};

#endif

//-------------------------------------------------------------------------
// public methods
//-------------------------------------------------------------------------

void StreamInitIp( void )
{
    if(ip_lws_cache == NULL)
    {
        ip_lws_cache = session_api->init_session_cache( SESSION_PROTO_IP,
                                                        0,             // NO session control blocks for IP
                                                        IpSessionCleanup);
    }

#ifdef ENABLE_HA
    ha_set_api(IPPROTO_IP, &ha_ip_api);
#endif
}

void StreamResetIp (void)
{
    session_api->purge_session_cache(ip_lws_cache);
}

void StreamCleanIp (void)
{
    if ( ip_lws_cache )
        s5stats.ip_prunes = session_api->get_session_prune_count( SESSION_PROTO_IP );

    /* Clean up hash table -- delete all sessions */
    session_api->delete_session_cache( SESSION_PROTO_IP );
    ip_lws_cache = NULL;
}

//-------------------------------------------------------------------------
// public config methods
//-------------------------------------------------------------------------

void StreamIpPolicyInit (StreamIpConfig* config, char* args)
{
    if (config == NULL)
        return;

    StreamParseIpArgs(args, &config->default_policy);
    StreamPrintIpConfig(&config->default_policy);
}

void StreamIpConfigFree (StreamIpConfig* config)
{
    if (config == NULL)
        return;

    SnortPreprocFree(config, sizeof(StreamIpConfig),
                      PP_STREAM, PP_MEM_CATEGORY_CONFIG);
}

int StreamVerifyIpConfig (StreamIpConfig* config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return -1;

    if (!ip_lws_cache)
        return -1;

    return 0;
}

//-------------------------------------------------------------------------
// public access methods
//-------------------------------------------------------------------------

uint32_t StreamGetIpPrunes (void)
{
    if( ip_lws_cache )
        return session_api->get_session_prune_count( SESSION_PROTO_IP );
    else
        return s5stats.ip_prunes;
}

void StreamResetIpPrunes (void)
{
    session_api->reset_session_prune_count( SESSION_PROTO_IP );
}

//-------------------------------------------------------------------------
// private packet processing methods
//-------------------------------------------------------------------------

static inline void InitSession (Packet* p, SessionControlBlock *scb)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM,
        "Stream IP session initialized!\n"););

    s5stats.total_ip_sessions++;
    s5stats.active_ip_sessions++;
    IP_COPY_VALUE(scb->client_ip, GET_SRC_IP(p));
    IP_COPY_VALUE(scb->server_ip, GET_DST_IP(p));
}

static inline int BlockedSession (Packet* p, SessionControlBlock *scb)
{
    if ( !(scb->ha_state.session_flags & (SSNFLAG_DROP_CLIENT|SSNFLAG_DROP_SERVER)) )
        return 0;

    if (
        ((p->packet_flags & PKT_FROM_SERVER) && (scb->ha_state.session_flags & SSNFLAG_DROP_SERVER)) ||
        ((p->packet_flags & PKT_FROM_CLIENT) && (scb->ha_state.session_flags & SSNFLAG_DROP_CLIENT)) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
            "Blocking %s packet as session was blocked\n",
            p->packet_flags & PKT_FROM_SERVER ?  "server" : "client"););

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
        return 1;
    }
    return 0;
}

static inline int IgnoreSession (Packet* p, SessionControlBlock *scb)
{
    if (
        ((p->packet_flags & PKT_FROM_SERVER) && (scb->ha_state.ignore_direction & SSN_DIR_FROM_CLIENT)) ||
        ((p->packet_flags & PKT_FROM_CLIENT) && (scb->ha_state.ignore_direction & SSN_DIR_FROM_SERVER)) )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
            "Stream Ignoring packet from %d. Session marked as ignore\n",
            p->packet_flags & PKT_FROM_CLIENT? "sender" : "responder"););

        session_api->disable_inspection(scb, p);
        return 1;
    }

    return 0;
}

#ifdef ENABLE_EXPECTED_IP
static inline int CheckExpectedSession (Packet* p, SessionControlBlock *scb)
{
    int ignore;

    ignore = StreamExpectCheck(p, scb);

    if (ignore)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
            "Stream: Ignoring packet from %d. Marking session marked as ignore.\n",
            p->packet_flags & PKT_FROM_CLIENT? "sender" : "responder"););

        scb->ha_state.ignore_direction = ignore;
        session_api->disable_inspection(scb, p);
        return 1;
    }

    return 0;
}
#endif

static inline void UpdateSession (Packet* p, SessionControlBlock* scb)
{
    MarkupPacketFlags(p, scb);

    if ( !(scb->ha_state.session_flags & SSNFLAG_ESTABLISHED) )
    {

        if ( p->packet_flags & PKT_FROM_CLIENT )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Stream: Updating on packet from client\n"););

            scb->ha_state.session_flags |= SSNFLAG_SEEN_CLIENT;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Stream: Updating on packet from server\n"););

            scb->ha_state.session_flags |= SSNFLAG_SEEN_SERVER;
        }

        if ( (scb->ha_state.session_flags & SSNFLAG_SEEN_CLIENT) &&
             (scb->ha_state.session_flags & SSNFLAG_SEEN_SERVER) )
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
                "Stream: session established!\n"););

            scb->ha_state.session_flags |= SSNFLAG_ESTABLISHED;

#ifdef ACTIVE_RESPONSE
            SetTTL(scb, p, 0);
#endif
        }
    }

    // Reset the session timeout.
    {
        StreamIpPolicy* policy;
        policy = (StreamIpPolicy*)scb->proto_policy;

        session_api->set_expire_timer(p, scb, policy->session_timeout);
    }
}

//-------------------------------------------------------------------------
// public packet processing method
//-------------------------------------------------------------------------

int StreamProcessIp( Packet *p, SessionControlBlock *scb, SessionKey *skey )
{
    PROFILE_VARS;

    PREPROC_PROFILE_START( s5IpPerfStats );

    if( scb->proto_policy == NULL )
    {
        scb->proto_policy = ( ( StreamConfig * ) scb->stream_config )->ip_config;
#if defined(DAQ_CAPA_CST_TIMEOUT)
        if (Daq_Capa_Timeout)
        {
          StreamIpPolicy* policy;
          uint64_t timeout;
          policy = (StreamIpPolicy*)scb->proto_policy;
          GetTimeout(p,&timeout);
          policy->session_timeout = timeout;
        }
#endif
    }
    if( !scb->session_established )
    {
        scb->session_established = true;
        InitSession(p, scb);

#ifdef ENABLE_EXPECTED_IP
        if( CheckExpectedSession( p, scb ) )
        {
            PREPROC_PROFILE_END( s5IpPerfStats );
            return 0;
        }
#endif
    }
    else
    {
        if( ( scb->session_state & STREAM_STATE_TIMEDOUT ) || StreamExpire( p, scb ) )
        {
            scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;

            /* Session is timed out */
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM, "Stream IP session timeout!\n"););

#ifdef ENABLE_HA
            /* Notify the HA peer of the session cleanup/reset by way of a deletion notification. */
            PREPROC_PROFILE_TMPEND( s5IpPerfStats );
            SessionHANotifyDeletion( scb );
            scb->ha_flags = ( HA_FLAG_NEW | HA_FLAG_MODIFIED | HA_FLAG_MAJOR_CHANGE );
            PREPROC_PROFILE_TMPSTART( s5IpPerfStats );
#endif

            /* Clean it up */
            IpSessionCleanup( scb );

#ifdef ENABLE_EXPECTED_IP
            if( CheckExpectedSession( p, scb ) )
            {
                PREPROC_PROFILE_END( s5IpPerfStats );
                return 0;
            }
#endif
        }
   }

    session_api->set_packet_direction_flag( p, scb );
    p->ssnptr = scb;

    if( BlockedSession( p, scb ) || IgnoreSession( p, scb ) )
    {
        PREPROC_PROFILE_END( s5IpPerfStats );
        return 0;
    }

    UpdateSession( p, scb );

    PREPROC_PROFILE_END( s5IpPerfStats );

    return 0;
}

#ifdef SNORT_RELOAD
void SessionIPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout)
{
    SessionReload(ip_lws_cache, max_sessions, pruningTimeout, nominalTimeout
#ifdef REG_TEST
                  , "IP"
#endif
                  );
}

unsigned SessionIPReloadAdjust(unsigned maxWork)
{
    return SessionProtocolReloadAdjust(ip_lws_cache, session_configuration->max_ip_sessions, 
                                       maxWork, 0
#ifdef REG_TEST
                                       , "IP"
#endif
                                       );
}
#endif

size_t get_ip_used_mempool()
{
    if (ip_lws_cache && ip_lws_cache->protocol_session_pool)
        return ip_lws_cache->protocol_session_pool->used_memory;

    return 0;
}
