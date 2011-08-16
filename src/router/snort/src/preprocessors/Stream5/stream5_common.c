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
 
#include "debug.h"
#include "decode.h"
#include "log.h"
#include "util.h"
#include "generators.h"
#include "event_queue.h"
#include "snort.h"
#include "sf_types.h"

#include "snort_stream5_session.h"
#include "stream5_common.h"
#include "portscan.h"
#include "sftarget_protocol_reference.h"
#include "sp_dynamic.h" 
#include "snort_stream5_tcp.h"
#include "snort_stream5_udp.h"
#include "snort_stream5_icmp.h"
#include "parser.h" 
#include "active.h" 

extern tSfPolicyUserContextId s5_config;

static void printIgnoredRules(
        IgnoredRuleList *pIgnoredRuleList,
        int any_any_flow
        );
static void addRuleToIgnoreList(
        IgnoredRuleList **ppIgnoredRuleList, 
        OptTreeNode *otn);

/*  M A C R O S  **************************************************/
static INLINE uint64_t CalcJiffies(Packet *p)
{
    uint64_t ret = 0;
    uint64_t sec = (p->pkth->ts.tv_sec * TCP_HZ);
    uint64_t usec = (p->pkth->ts.tv_usec / (1000000UL/TCP_HZ));

    ret = sec + usec;

    return ret;
    //return (p->pkth->ts.tv_sec * TCP_HZ) + 
    //       (p->pkth->ts.tv_usec / (1000000UL/TCP_HZ));
}

int Stream5Expire(Packet *p, Stream5LWSession *lwssn)
{
    uint64_t pkttime = CalcJiffies(p);

    if (lwssn->expire_time == 0)
    {
        /* Not yet set, not expired */
        return 0;
    }
    
    if((int)(pkttime - lwssn->expire_time) > 0)
    {
        sfBase.iStreamTimeouts++;
        lwssn->session_flags |= SSNFLAG_TIMEDOUT;
        lwssn->session_state |= STREAM5_STATE_TIMEDOUT;

        switch (lwssn->protocol)
        {
            case IPPROTO_TCP:
                s5stats.tcp_timeouts++;
                //DeleteLWSession(tcp_lws_cache, lwssn);
                break;
            case IPPROTO_UDP:
                s5stats.udp_timeouts++;
                //DeleteLWSession(udp_lws_cache, lwssn);
                break;
            case IPPROTO_ICMP:
                s5stats.icmp_timeouts++;
                //DeleteLWSession(icmp_lws_cache, lwssn);
                break;
        }
        return 1;
    }

    return 0;
}

void Stream5SetExpire(Packet *p, 
        Stream5LWSession *lwssn, uint32_t timeout)
{
    lwssn->expire_time = CalcJiffies(p) + (timeout * TCP_HZ);
    return;
}

#ifdef ACTIVE_RESPONSE
int Stream5GetExpire(Packet *p, Stream5LWSession *lwssn)
{
    return ( CalcJiffies(p) > lwssn->expire_time );
}

// *DROP* flags are set to mark the direction(s) for which traffic was
// seen since last reset and then cleared after sending new attempt so
// that we only send in the still active direction(s).
void Stream5ActiveResponse(Packet* p, Stream5LWSession *lwssn)
{
    Stream5Config *config = sfPolicyUserDataGet(s5_config, getRuntimePolicy());
    uint8_t max = config->global_config->max_active_responses;

    if ( p->packet_flags & PKT_FROM_CLIENT )
        lwssn->session_state |= STREAM5_STATE_DROP_CLIENT;
    else
        lwssn->session_state |= STREAM5_STATE_DROP_SERVER;

    if ( (lwssn->response_count < max) && Stream5GetExpire(p, lwssn) )
    {
        uint32_t delay = config->global_config->min_response_seconds;
        EncodeFlags flags =
            ( (lwssn->session_state & STREAM5_STATE_DROP_CLIENT) &&
              (lwssn->session_state & STREAM5_STATE_DROP_SERVER) ) ?
            ENC_FLAG_FWD : 0;  // reverse dir is always true
   
        Active_KillSession(p, &flags);
        ++lwssn->response_count;
        Stream5SetExpire(p, lwssn, delay);

        lwssn->session_state &= ~(STREAM5_STATE_DROP_CLIENT|STREAM5_STATE_DROP_SERVER);
    }
}

void SetTTL (Stream5LWSession* ssn, Packet* p, int client)
{
    Layer* inner = NULL, *outer = NULL;
    int i;

    for ( i = 0; i < p->next_layer; i++ )
    {
        if ( p->layers[i].proto == PROTO_IP4 
            // || p->layers[i].proto == PROTO_IP6
        ) {
            outer = inner;
            inner = p->layers + i;
        }
    }
    if ( outer )
    {
        if ( outer->proto == PROTO_IP4 )
        {
            uint8_t ttl = ((IP4Hdr*)outer->start)->ip_ttl;
            if ( client ) ssn->outer_client_ttl = ttl;
            else ssn->outer_server_ttl = ttl;
        }
    }
    if ( inner )
    {
        if ( inner->proto == PROTO_IP4 )
        {
            uint8_t ttl = ((IP4Hdr*)inner->start)->ip_ttl;
            if ( client ) ssn->inner_client_ttl = ttl;
            else ssn->inner_server_ttl = ttl;
        }
    }
}
#endif

void MarkupPacketFlags(Packet *p, Stream5LWSession *lwssn)
{
    if(!lwssn)
        return;

    if((lwssn->session_flags & SSNFLAG_ESTABLISHED) != SSNFLAG_ESTABLISHED)
    {
        if((lwssn->session_flags & (SSNFLAG_SEEN_SERVER|SSNFLAG_SEEN_CLIENT)) !=
            (SSNFLAG_SEEN_SERVER|SSNFLAG_SEEN_CLIENT))
        {
            p->packet_flags |= PKT_STREAM_UNEST_UNI;
        }
    }
    else
    {
        p->packet_flags |= PKT_STREAM_EST;
        if(p->packet_flags & PKT_STREAM_UNEST_UNI)
        {
            p->packet_flags ^= PKT_STREAM_UNEST_UNI;
        }
    }
}

#if 0
/** Get rule list for a specific protocol
 *
 * @param rule  
 * @param ptocool protocol type 
 * @returns RuleTreeNode* rule list for specific protocol
 */
static INLINE RuleTreeNode * protocolRuleList(RuleListNode *rule, int protocol)
{
    switch (protocol)
    {
        case IPPROTO_TCP:
            return rule->RuleList->TcpList;
        case IPPROTO_UDP:
            return rule->RuleList->UdpList;
        case IPPROTO_ICMP:
            break;
        default:
            break;
    }
    return NULL;
}
#endif
static INLINE char * getProtocolName (int protocol)
{
    static char *protocolName[] = {"TCP", "UDP", "ICMP"};
    switch (protocol)
    {
        case IPPROTO_TCP:
            return protocolName[0];
        case IPPROTO_UDP:
            return protocolName[1];
        case IPPROTO_ICMP:
            return protocolName[2];
            break;
        default:
            break;
    }
    return NULL;
}

/**check whether a flow bit is set for an option node.
 *
 * @param otn Option Tree Node
 * @returns 0 - no flow bit is set, 1 otherwise
 */
int Stream5OtnHasFlowOrFlowbit(OptTreeNode *otn)
{
    if (otn->ds_list[PLUGIN_CLIENTSERVER] ||
#ifdef DYNAMIC_PLUGIN
        DynamicHasFlow(otn) ||
        DynamicHasFlowbit(otn) ||
#endif
        otn->ds_list[PLUGIN_FLOWBIT])
    {
        return 1;
    }
    return 0;
}

/**initialize given port list from the given ruleset, for a given policy
 * @param portList pointer to array of MAX_PORTS+1 uint8_t. This array content 
 * is changed by walking through the rulesets.
 * @param protocol - protocol type
 */
void setPortFilterList(
        uint8_t *portList, 
        int protocol,
        int ignoreAnyAnyRules,
        tSfPolicyId policyId
        )
{
    char *port_array = NULL;
    int num_ports = 0;
    int i;
    RuleTreeNode *rtn;
    OptTreeNode *otn;
    int inspectSrc, inspectDst;
    char any_any_flow = 0;
    IgnoredRuleList *pIgnoredRuleList = NULL;     ///list of ignored rules
    char *protocolName;
    SFGHASH_NODE *hashNode;
    int flowBitIsSet = 0;
    SnortConfig *sc = snort_conf_for_parsing;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    if ((protocol == IPPROTO_TCP) && (ignoreAnyAnyRules == 0))
    {
        int j;
        for (j=0; j<MAX_PORTS; j++)
        {
            portList[j] |= PORT_MONITOR_SESSION | PORT_MONITOR_INSPECT;
        }
        return;
    }

    protocolName = getProtocolName(protocol);

    /* Post-process TCP rules to establish TCP ports to inspect. */
    for (hashNode = sfghash_findfirst(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        flowBitIsSet = Stream5OtnHasFlowOrFlowbit(otn);

        rtn = getRtnFromOtn(otn, policyId);

        if (!rtn)
        {
            continue;
        }

        if (rtn->proto == protocol)
        { 
            //do operation
            inspectSrc = inspectDst = 0;
            if (PortObjectHasAny(rtn->src_portobject))
            {
                inspectSrc = -1;
            }
            else
            {
                port_array = PortObjectCharPortArray(port_array, rtn->src_portobject, &num_ports);
                if (port_array && num_ports != 0)
                {
                    inspectSrc = 1;
                    for (i=0;i<SFPO_MAX_PORTS;i++)
                    {
                        if (port_array[i])
                        {
                            portList[i] |= PORT_MONITOR_INSPECT;
                            /* port specific rule */
                                /* Look for an OTN with flow or flowbits keyword */
                                if (flowBitIsSet)
                                {
                                    portList[i] |= PORT_MONITOR_SESSION;
                                }
                        }
                    }
                }
                if ( port_array )
                {
                    free(port_array);
                    port_array = NULL;
                }
            }
            if (PortObjectHasAny(rtn->dst_portobject))
            {
                inspectDst = -1;
            }
            else
            {
                port_array = PortObjectCharPortArray(port_array, rtn->dst_portobject, &num_ports);
                if (port_array && num_ports != 0)
                {
                    inspectDst = 1;
                    for (i=0;i<SFPO_MAX_PORTS;i++)
                    {
                        if (port_array[i])
                        {
                            portList[i] |= PORT_MONITOR_INSPECT;
                            /* port specific rule */
                                if (flowBitIsSet)
                                {
                                    portList[i] |= PORT_MONITOR_SESSION;
                                }
                        }
                    }
                }
                if ( port_array )
                {
                    free(port_array);
                    port_array = NULL;
                }
            }
            if ((inspectSrc == -1) && (inspectDst == -1))
            {
                /* any -> any rule */
                if (any_any_flow == 0)
                {
                    any_any_flow = Stream5AnyAnyFlow(portList, otn, rtn, any_any_flow,
                            &pIgnoredRuleList, ignoreAnyAnyRules);
                }
            }
        }
    }

    /* If portscan is tracking TCP/UDP, need to create
     * sessions for all ports */
    if (((protocol == IPPROTO_UDP) && (ps_get_protocols(policyId) & PS_PROTO_UDP))
     || ((protocol == IPPROTO_TCP) && (ps_get_protocols(policyId) & PS_PROTO_TCP)))
    {
        int j;
        for (j=0; j<MAX_PORTS; j++)
        {
            portList[j] |= PORT_MONITOR_SESSION;
        }
    }

    if (any_any_flow == 1)
    {
        LogMessage("Warning: 'ignore_any_rules' option for Stream5 %s "
            "disabled because of %s rule with flow or flowbits option\n", 
            protocolName, protocolName);
    }

    else if (pIgnoredRuleList)
    {
        LogMessage("Warning: Rules (GID:SID) effectively ignored because of "
            "'ignore_any_rules' option for Stream5 %s:\n", protocolName);
    }
    // free list; print iff any_any_flow
    printIgnoredRules(pIgnoredRuleList, any_any_flow);

}

/**Determines whether any_any_flow should be ignored or not.
 *
 * Dont ignore any_any_flows if flow bit is set on an any_any_flow, 
 * or ignoreAnyAnyRules is not set.
 * @param portList port list
 * @param rtn Rule tree node
 * @param any_any_flow - set if any_any_flow is ignored,0 otherwise
 * @param ppIgnoredRuleList
 * @param ignoreAnyAnyRules
 * @returns
 */
int Stream5AnyAnyFlow(
        uint8_t *portList, 
        OptTreeNode *otn,
        RuleTreeNode *rtn, 
        int any_any_flow,
        IgnoredRuleList **ppIgnoredRuleList,
        int ignoreAnyAnyRules
        )
{
    /**if any_any_flow is set then following code has no effect.*/
    if (any_any_flow)
    {
        return any_any_flow;
    }

    /* Look for an OTN with flow or flowbits keyword */
    if (Stream5OtnHasFlowOrFlowbit(otn))
    {
        int i;

        for (i=1;i<=MAX_PORTS;i++)
        {
            /* track sessions for ALL ports becuase
             * of any -> any with flow/flowbits */
            portList[i] |= PORT_MONITOR_SESSION;
        }
        return 1;
    }

    if (ignoreAnyAnyRules)
    {

        /* if not, then ignore the content/pcre/etc */
        if (otn->ds_list[PLUGIN_PATTERN_MATCH] ||
                otn->ds_list[PLUGIN_PATTERN_MATCH_OR] ||
                otn->ds_list[PLUGIN_PATTERN_MATCH_URI] ||
#ifdef DYNAMIC_PLUGIN
                DynamicHasContent(otn) ||
                DynamicHasByteTest(otn) ||
                DynamicHasPCRE(otn) ||
#endif
                otn->ds_list[PLUGIN_BYTE_TEST] ||
                otn->ds_list[PLUGIN_PCRE])
        {
            /* Ignoring this rule.... */
            addRuleToIgnoreList(ppIgnoredRuleList, otn);
        }
    }

    return 0;
}

/**add rule to the ignore rule list.
 */
static void addRuleToIgnoreList(IgnoredRuleList **ppIgnoredRuleList, OptTreeNode *otn)
{
    IgnoredRuleList *ignored_rule;

    ignored_rule = SnortAlloc(sizeof(*ignored_rule));
    ignored_rule->otn = otn;
    ignored_rule->next = *ppIgnoredRuleList;
    *ppIgnoredRuleList = ignored_rule;
}


/**print the ignored rule list.
 */
static void printIgnoredRules(
        IgnoredRuleList *pIgnoredRuleList,
        int any_any_flow
        )
{
    char six_sids = 0;
    int sids_ignored = 0;
    char buf[STD_BUF];
    IgnoredRuleList *ignored_rule;
    IgnoredRuleList *next_ignored_rule;

    buf[0] = '\0';

    for (ignored_rule = pIgnoredRuleList; ignored_rule != NULL; )
    {
        if (any_any_flow == 0)
        {
            if (six_sids == 1)
            {
                SnortSnprintfAppend(buf, STD_BUF-1, "\n");
                LogMessage("%s", buf);
                six_sids = 0;
            }

            if (sids_ignored == 0)
            {
                SnortSnprintf(buf, STD_BUF-1, "    %d:%d",
                        ignored_rule->otn->sigInfo.generator,
                        ignored_rule->otn->sigInfo.id);
            }
            else
            {
                SnortSnprintfAppend(buf, STD_BUF-1, ", %d:%d", 
                        ignored_rule->otn->sigInfo.generator,
                        ignored_rule->otn->sigInfo.id);
            }
            sids_ignored++;
            if (sids_ignored %6 == 0)
            {
                /* Have it print next time through */
                six_sids = 1;
                sids_ignored = 0;
            }
        }
        next_ignored_rule = ignored_rule->next;
        free(ignored_rule);
        ignored_rule = next_ignored_rule;
    }

    if (sids_ignored || six_sids)
    {
        SnortSnprintfAppend(buf, STD_BUF-1, "\n");
        LogMessage("%s", buf);
    }
}

static int Stream5FreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId, 
        void* pData
        )
{
    Stream5Config *pPolicyConfig = (Stream5Config *)pData;

    //do any housekeeping before freeing Stream5Config
    sfPolicyUserDataClear (config, policyId);
    Stream5FreeConfig(pPolicyConfig);

    return 0;
}

void Stream5FreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataIterate (config, Stream5FreeConfigsPolicy);

    sfPolicyConfigDelete(config);
}

void Stream5FreeConfig(Stream5Config *config)
{
    if (config == NULL)
        return;

    if (config->global_config != NULL)
    {
        free(config->global_config);
        config->global_config = NULL;
    }

    if (config->tcp_config != NULL)
    {
        Stream5TcpConfigFree(config->tcp_config);
        config->tcp_config = NULL;
    }

    if (config->udp_config != NULL)
    {
        Stream5UdpConfigFree(config->udp_config);
        config->udp_config = NULL;
    }

    if (config->icmp_config != NULL)
    {
        Stream5IcmpConfigFree(config->icmp_config);
        config->icmp_config = NULL;
    }

    free(config);
}

