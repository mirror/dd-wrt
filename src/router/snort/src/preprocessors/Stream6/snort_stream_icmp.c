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
#include "decode.h"
#include "mstring.h"
#include "sfxhash.h"
#include "util.h"
#include "memory_stats.h"

#include "spp_session.h"
#include "session_api.h"
#include "snort_session.h"

#include "stream_common.h"
#include "snort_stream_tcp.h"
#include "snort_stream_udp.h"
#include "snort_stream_icmp.h"

#include "parser.h"

#include "reg_test.h"

#include "profiler.h"
#include "sfPolicy.h"
#ifdef PERF_PROFILING
PreprocStats s5IcmpPerfStats;
#endif

/* client/server ip/port dereference */
#define icmp_sender_ip lwSsn->client_ip
#define icmp_responder_ip lwSsn->server_ip

/*  D A T A  S T R U C T U R E S  ***********************************/
typedef struct _IcmpSession
{
    SessionControlBlock *lwSsn;

    uint32_t   echo_count;

    struct timeval ssn_time;

} IcmpSession;


/*  G L O B A L S  **************************************************/
static SessionCache* icmp_lws_cache = NULL;

/*  P R O T O T Y P E S  ********************************************/
static void StreamParseIcmpArgs(char *, StreamIcmpPolicy *);
static void StreamPrintIcmpConfig(StreamIcmpPolicy *);
static int ProcessIcmpUnreach(Packet *p);
static int ProcessIcmpEcho(Packet *p);

void StreamInitIcmp( void )
{
    /* Finally ICMP */
    if(icmp_lws_cache == NULL)
    {
        icmp_lws_cache = session_api->init_session_cache( SESSION_PROTO_ICMP,
                                                          sizeof( IcmpSession ),
                                                          NULL );
    }
}

void StreamIcmpPolicyInit(StreamIcmpConfig *config, char *args)
{
    if (config == NULL)
        return;

    config->num_policies++;

    StreamParseIcmpArgs(args, &config->default_policy);

    StreamPrintIcmpConfig(&config->default_policy);
}

static void StreamParseIcmpArgs(char *args, StreamIcmpPolicy *s5IcmpPolicy)
{
    char **toks;
    int num_toks;
    int i;
    char **stoks = NULL;
    int s_toks;
    char *endPtr = NULL;

    s5IcmpPolicy->session_timeout = STREAM_DEFAULT_SSN_TIMEOUT;
    //s5IcmpPolicy->flags = 0;

    if(args != NULL && strlen(args) != 0)
    {
        toks = mSplit(args, ",", 0, &num_toks, 0);

        for (i = 0; i < num_toks; i++)
        {
            stoks = mSplit(toks[i], " ", 2, &s_toks, 0);

            if (s_toks == 0)
            {
                FatalError("%s(%d) => Missing parameter in Stream ICMP config.\n",
                    file_name, file_line);
            }

            if(!strcasecmp(stoks[0], "timeout"))
            {
                if(stoks[1])
                {
                    s5IcmpPolicy->session_timeout = strtoul(stoks[1], &endPtr, 10);
                }

                if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  Integer parameter required.\n",
                            file_name, file_line);
                }

                if ((s5IcmpPolicy->session_timeout > STREAM_MAX_SSN_TIMEOUT) ||
                    (s5IcmpPolicy->session_timeout < STREAM_MIN_SSN_TIMEOUT))
                {
                    FatalError("%s(%d) => Invalid timeout in config file.  "
                        "Must be between %d and %d\n",
                        file_name, file_line,
                        STREAM_MIN_SSN_TIMEOUT, STREAM_MAX_SSN_TIMEOUT);
                }
                if (s_toks > 2)
                {
                    FatalError("%s(%d) => Invalid Stream ICMP Policy option.  Missing comma?\n",
                        file_name, file_line);
                }
            }
            else
            {
                FatalError("%s(%d) => Invalid Stream ICMP policy option\n",
                            file_name, file_line);
            }

            mSplitFree(&stoks, s_toks);
        }

        mSplitFree(&toks, num_toks);
    }
}

static void StreamPrintIcmpConfig(StreamIcmpPolicy *s5IcmpPolicy)
{
    LogMessage("Stream ICMP Policy config:\n");
    LogMessage("    Timeout: %d seconds\n", s5IcmpPolicy->session_timeout);
    //LogMessage("    Flags: 0x%X\n", s5UdpPolicy->flags);
    //IpAddrSetPrint("    Bound Addresses:", s5UdpPolicy->bound_addrs);

}

void IcmpSessionCleanup(SessionControlBlock *ssn)
{
    IcmpSession *icmpssn = NULL;

    if (ssn->ha_state.session_flags & SSNFLAG_PRUNED)
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_PRUNED);
    }
    else if (ssn->ha_state.session_flags & SSNFLAG_TIMEDOUT)
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_TIMEDOUT);
    }
    else
    {
        CloseStreamSession(&sfBase, SESSION_CLOSED_NORMALLY);
    }

    if (ssn->proto_specific_data)
        icmpssn = ssn->proto_specific_data->data;

    if (!icmpssn)
    {
        /* Huh? */
        return;
    }

    /* Cleanup the proto specific data */
    session_api->free_protocol_session_pool( SESSION_PROTO_ICMP, ssn );
    ssn->proto_specific_data = NULL;

    StreamResetFlowBits(ssn);
    session_api->free_application_data(ssn);

    s5stats.icmp_sessions_released++;
    s5stats.active_icmp_sessions--;
}

uint32_t StreamGetIcmpPrunes(void)
{
    if( icmp_lws_cache )
        return session_api->get_session_prune_count( SESSION_PROTO_ICMP );
    else
        return s5stats.icmp_prunes;
}

void StreamResetIcmpPrunes(void)
{
    session_api->reset_session_prune_count( SESSION_PROTO_ICMP );
}

void StreamResetIcmp(void)
{
    session_api->purge_session_cache(icmp_lws_cache);
    session_api->clean_protocol_session_pool( SESSION_PROTO_ICMP );
}

void StreamCleanIcmp(void)
{
    if ( icmp_lws_cache )
        s5stats.icmp_prunes = session_api->get_session_prune_count( SESSION_PROTO_ICMP );

    /* Clean up session cache */
    session_api->delete_session_cache( SESSION_PROTO_ICMP );
    icmp_lws_cache = NULL;
}

void StreamIcmpConfigFree(StreamIcmpConfig *config)
{
    if (config == NULL)
        return;

    SnortPreprocFree(config, sizeof(StreamIcmpConfig), 
                     PP_STREAM, PP_MEM_CATEGORY_CONFIG);
}

int StreamVerifyIcmpConfig(StreamIcmpConfig *config, tSfPolicyId policy_id)
{
    if (config == NULL)
        return -1;

    if (!icmp_lws_cache)
        return -1;

    if (config->num_policies == 0)
        return -1;

    return 0;
}

int StreamProcessIcmp(Packet *p)
{
    int status = 0;

    switch (p->icmph->type)
    {
    case ICMP_DEST_UNREACH:
        s5stats.icmp_unreachable++;
        if (p->icmph->code != ICMP_FRAG_NEEDED)
        {
            status = ProcessIcmpUnreach(p);
        }
        else
        {
            s5stats.icmp_unreachable_code4++;
        }
        break;

    case ICMP_ECHO:
    case ICMP_ECHOREPLY:
        status = ProcessIcmpEcho(p);
        break;

    default:
        /* We only handle the above ICMP messages with stream5 */
        break;
    }

    return status;
}

static int ProcessIcmpUnreach(Packet *p)
{
    /* Handle ICMP unreachable */
    SessionKey skey;
    SessionControlBlock *ssn = NULL;
    uint16_t sport;
    uint16_t dport;
    sfaddr_t *src;
    sfaddr_t *dst;

    /* No "orig" IP Header */
    if (!p->orig_iph)
        return 0;

    /* Get TCP/UDP/ICMP session from original protocol/port info
     * embedded in the ICMP Unreach message.  This is already decoded
     * in p->orig_foo.  TCP/UDP ports are decoded as p->orig_sp/dp.
     */
    skey.protocol = GET_ORIG_IPH_PROTO(p);
    sport = p->orig_sp;
    dport = p->orig_dp;

    src = GET_ORIG_SRC(p);
    dst = GET_ORIG_DST(p);

    if (sfip_fast_lt6(src, dst))
    {
        COPY4(skey.ip_l, sfaddr_get_ip6_ptr(src));
        skey.port_l = sport;
        COPY4(skey.ip_h, sfaddr_get_ip6_ptr(dst));
        skey.port_h = dport;
    }
    else if (IP_EQUALITY(src, dst))
    {
        COPY4(skey.ip_l, sfaddr_get_ip6_ptr(src));
        COPY4(skey.ip_h, skey.ip_l);
        if (sport < dport)
        {
            skey.port_l = sport;
            skey.port_h = dport;
        }
        else
        {
            skey.port_l = dport;
            skey.port_h = sport;
        }
    }
    else
    {
        COPY4(skey.ip_l, sfaddr_get_ip6_ptr(dst));
        COPY4(skey.ip_h, sfaddr_get_ip6_ptr(src));
        skey.port_l = dport;
        skey.port_h = sport;
    }

    if (p->vh)
        skey.vlan_tag = (uint16_t)VTH_VLAN(p->vh);
    else
        skey.vlan_tag = 0;

    switch (skey.protocol)
    {
    case IPPROTO_TCP:
        /* Lookup a TCP session */
        ssn = GetLWTcpSession(&skey);
        break;
    case IPPROTO_UDP:
        /* Lookup a UDP session */
        ssn = GetLWUdpSession(&skey);
        break;
    case IPPROTO_ICMP:
        /* Lookup a ICMP session */
        ssn = session_api->get_session_by_key(icmp_lws_cache, &skey);
        break;
    }

    if (ssn)
    {
        /* Mark this session as dead. */
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_STATE,
            "Marking session as dead, per ICMP Unreachable!\n"););
        ssn->ha_state.session_flags |= SSNFLAG_DROP_CLIENT;
        ssn->ha_state.session_flags |= SSNFLAG_DROP_SERVER;
        ssn->session_state |= STREAM_STATE_UNREACH;
        s5stats.active_icmp_sessions++;
    }

    return 0;
}

static int ProcessIcmpEcho(Packet *p)
{
    //SessionKey skey;
    //SessionControlBlock *ssn = NULL;
    s5stats.active_icmp_sessions++;

    return 0;
}

void IcmpUpdateDirection(SessionControlBlock *ssn, char dir, sfaddr_t* ip, uint16_t port)
{
    IcmpSession *icmpssn = ssn->proto_specific_data->data;
    sfaddr_t tmpIp;

    if (!icmpssn)
    {
        /* Huh? */
        return;
    }

    if (IP_EQUALITY(&icmpssn->icmp_sender_ip, ip))
    {
        if ((dir == SSN_DIR_FROM_SENDER) && (ssn->ha_state.direction == SSN_DIR_FROM_SENDER))
        {
            /* Direction already set as SENDER */
            return;
        }
    }
    else if (IP_EQUALITY(&icmpssn->icmp_responder_ip, ip))
    {
        if ((dir == SSN_DIR_FROM_RESPONDER) && (ssn->ha_state.direction == SSN_DIR_FROM_RESPONDER))
        {
            /* Direction already set as RESPONDER */
            return;
        }
    }

    /* Swap them -- leave ssn->ha_state.direction the same */
    tmpIp = icmpssn->icmp_sender_ip;
    icmpssn->icmp_sender_ip = icmpssn->icmp_responder_ip;
    icmpssn->icmp_responder_ip = tmpIp;
}

#ifdef SNORT_RELOAD
void SessionICMPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout)
{
    SessionReload(icmp_lws_cache, max_sessions, pruningTimeout, nominalTimeout
#ifdef REG_TEST
                  , "ICMP"
#endif
                  );
}

unsigned SessionICMPReloadAdjust(unsigned maxWork)
{
    return SessionProtocolReloadAdjust(icmp_lws_cache, session_configuration->max_icmp_sessions, 
                                       maxWork, 0
#ifdef REG_TEST
                                       , "ICMP"
#endif
                                       );
}
#endif

size_t get_icmp_used_mempool()
{
    if (icmp_lws_cache && icmp_lws_cache->protocol_session_pool)
        return icmp_lws_cache->protocol_session_pool->used_memory;

    return 0;
}
