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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_debug.h"
#include "decode.h"
#include "log.h"
#include "util.h"
#include "generators.h"
#include "event_queue.h"
#include "snort.h"
#include "memory_stats.h"

#include "session_api.h"

#include "stream_common.h"
#include "portscan.h"
#include "sftarget_protocol_reference.h"
#include "sp_dynamic.h"
#include "snort_stream_tcp.h"
#include "snort_stream_udp.h"
#include "snort_stream_icmp.h"
#include "snort_stream_ip.h"
#include "parser.h"
#include "active.h"
#ifdef ENABLE_HA
#include "stream5_ha.h"
#endif

static void printIgnoredRules(
        IgnoredRuleList *pIgnoredRuleList,
        int any_any_flow
        );
static void addRuleToIgnoreList(
        IgnoredRuleList **ppIgnoredRuleList,
        OptTreeNode *otn);

/*  M A C R O S  **************************************************/

int StreamExpireSession(SessionControlBlock *scb)
{
#ifdef ENABLE_HA
    /* Sessions in standby cannot be locally expired. */
    if (scb->ha_flags & HA_FLAG_STANDBY)
        return 0;
#endif

    sfBase.iStreamTimeouts++;
    scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;
    scb->session_state |= STREAM_STATE_TIMEDOUT;

    switch (scb->protocol)
    {
        case IPPROTO_TCP:
            s5stats.tcp_timeouts++;
            break;
        case IPPROTO_UDP:
            s5stats.udp_timeouts++;
            break;
        case IPPROTO_ICMP:
            s5stats.icmp_timeouts++;
            break;
    }

    return 1;
}

void StreamDeleteSession(SessionControlBlock *scb)
{
    sfBase.iStreamTimeouts++;
    scb->ha_state.session_flags |= SSNFLAG_TIMEDOUT;

    switch (scb->protocol)
    {
        case IPPROTO_TCP:
            s5stats.tcp_timeouts++;
            break;
        case IPPROTO_UDP:
            s5stats.udp_timeouts++;
            break;
        case IPPROTO_ICMP:
            s5stats.icmp_timeouts++;
        case IPPROTO_IP:
            s5stats.ip_timeouts++;
            break;
    }

    if ( session_api->delete_session_by_key(scb, "timeout") != SFXHASH_OK )
    {
        LogMessage("WARNING: failed to delete session. \n");
    }
}

int StreamExpire(Packet *p, SessionControlBlock *scb)
{
    uint64_t pkttime = CalcJiffies(p);

    if (scb->expire_time == 0)
    {
        /* Not yet set, not expired */
        return 0;
    }

    if((int)(pkttime - scb->expire_time) > 0)
    {
        /* Expiration time has passed. */
        return StreamExpireSession(scb);
    }

    return 0;
}

#ifdef ACTIVE_RESPONSE
int StreamGetExpire(Packet *p, SessionControlBlock *scb)
{
    return ( CalcJiffies(p) > scb->expire_time );
}

// *DROP* flags are set to mark the direction(s) for which traffic was
// seen since last reset and then cleared after sending new attempt so
// that we only send in the still active direction(s).
void StreamActiveResponse(Packet* p, SessionControlBlock *scb)
{
    StreamConfig *config = sfPolicyUserDataGet(stream_online_config, getNapRuntimePolicy());
    uint8_t max = config->session_config->max_active_responses;

    if ( p->packet_flags & PKT_FROM_CLIENT )
        scb->session_state |= STREAM_STATE_DROP_CLIENT;
    else
        scb->session_state |= STREAM_STATE_DROP_SERVER;

    if ( (scb->response_count < max) && session_api->get_expire_timer(p, scb) )
    {
        uint32_t delay = config->session_config->min_response_seconds;
        EncodeFlags flags =
            ( (scb->session_state & STREAM_STATE_DROP_CLIENT) &&
              (scb->session_state & STREAM_STATE_DROP_SERVER) ) ?
            ENC_FLAG_FWD : 0;  // reverse dir is always true

        Active_KillSession(p, &flags);
        ++scb->response_count;
#if defined(DAQ_CAPA_CST_TIMEOUT)
        if (!Daq_Capa_Timeout)
#endif
           session_api->set_expire_timer(p, scb, delay);

        scb->session_state &= ~(STREAM_STATE_DROP_CLIENT|STREAM_STATE_DROP_SERVER);
    }
}

void SetTTL (SessionControlBlock* ssn, Packet* p, int client)
{
    uint8_t inner_ttl = 0, outer_ttl = 0;
    if ( p->outer_iph_api )
        outer_ttl = p->outer_iph_api->iph_ret_ttl(p);

    if ( p->iph_api )
        inner_ttl = p->iph_api->iph_ret_ttl(p);
    if ( client )
    {
        ssn->outer_client_ttl = outer_ttl;
        ssn->inner_client_ttl = inner_ttl;
    }
    else
    {
        ssn->outer_server_ttl = outer_ttl;
        ssn->inner_server_ttl = inner_ttl;
    }
}
#endif

void MarkupPacketFlags(Packet *p, SessionControlBlock *scb)
{
    if(!scb)
        return;

    if((scb->ha_state.session_flags & SSNFLAG_ESTABLISHED) != SSNFLAG_ESTABLISHED)
    {
        if((scb->ha_state.session_flags & (SSNFLAG_SEEN_SERVER|SSNFLAG_SEEN_CLIENT)) !=
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
static inline RuleTreeNode * protocolRuleList(RuleListNode *rule, IpProto protocol)
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
static inline char * getProtocolName (IpProto protocol)
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
int StreamOtnHasFlowOrFlowbit(OptTreeNode *otn)
{
    if (otn->ds_list[PLUGIN_CLIENTSERVER] ||
        DynamicHasFlow(otn) ||
        DynamicHasFlowbit(otn) ||
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
        struct _SnortConfig *sc,
        uint16_t *portList,
        IpProto protocol,
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

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    if (((protocol == IPPROTO_TCP) || (protocol == IPPROTO_UDP)) && (ignoreAnyAnyRules == 0))
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
        flowBitIsSet = StreamOtnHasFlowOrFlowbit(otn);

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
                    any_any_flow = StreamAnyAnyFlow(portList, otn, rtn, any_any_flow,
                            &pIgnoredRuleList, ignoreAnyAnyRules);
                }
            }
        }
    }

    /* If portscan is tracking TCP/UDP, need to create
     * sessions for all ports */
    if (((protocol == IPPROTO_UDP) && (ps_get_protocols(sc, policyId) & PS_PROTO_UDP))
     || ((protocol == IPPROTO_TCP) && (ps_get_protocols(sc, policyId) & PS_PROTO_TCP)))
    {
        int j;
        for (j=0; j<MAX_PORTS; j++)
        {
            portList[j] |= PORT_MONITOR_SESSION;
        }
    }

    if (any_any_flow == 1)
    {
        LogMessage("WARNING: 'ignore_any_rules' option for Stream %s "
            "disabled because of %s rule with flow or flowbits option.\n",
            protocolName, protocolName);
    }

    else if (pIgnoredRuleList)
    {
        LogMessage("WARNING: Rules (GID:SID) effectively ignored because of "
            "'ignore_any_rules' option for Stream %s.\n", protocolName);
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
int StreamAnyAnyFlow(
        uint16_t *portList,
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
    if (StreamOtnHasFlowOrFlowbit(otn))
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
                DynamicHasContent(otn) ||
                DynamicHasByteTest(otn) ||
                DynamicHasPCRE(otn) ||
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

    ignored_rule = SnortPreprocAlloc(1, sizeof(*ignored_rule), PP_STREAM, 
                           PP_MEM_CATEGORY_CONFIG);
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
        SnortPreprocFree(ignored_rule, sizeof(*ignored_rule), PP_STREAM,
                  PP_MEM_CATEGORY_CONFIG);
        ignored_rule = next_ignored_rule;
    }

    if (sids_ignored || six_sids)
    {
        SnortSnprintfAppend(buf, STD_BUF-1, "\n");
        LogMessage("%s", buf);
    }
}

static int StreamFreeConfigsPolicy(
        tSfPolicyUserContextId config,
        tSfPolicyId policyId,
        void* pData
        )
{
    StreamConfig *pPolicyConfig = (StreamConfig *)pData;

    //do any housekeeping before freeing StreamConfig
    sfPolicyUserDataClear (config, policyId);
    StreamFreeConfig(pPolicyConfig);

    return 0;
}

void StreamFreeConfigs(tSfPolicyUserContextId config)
{
    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate(config, StreamFreeConfigsPolicy);

    sfPolicyConfigDelete(config);
}

void StreamFreeConfig(StreamConfig *config)
{
    if (config == NULL)
        return;

    if (config->tcp_config != NULL)
    {
        StreamTcpConfigFree(config->tcp_config);
        config->tcp_config = NULL;
    }

    if (config->udp_config != NULL)
    {
        StreamUdpConfigFree(config->udp_config);
        config->udp_config = NULL;
    }

    if (config->icmp_config != NULL)
    {
        StreamIcmpConfigFree(config->icmp_config);
        config->icmp_config = NULL;
    }

    if (config->ip_config != NULL)
    {
        StreamIpConfigFree(config->ip_config);
        config->ip_config = NULL;
    }

    SnortPreprocFree(config, sizeof(*config), PP_STREAM, PP_MEM_CATEGORY_CONFIG);
}

int StreamSetRuntimeConfiguration( SessionControlBlock *scb, uint8_t protocol )
{
   StreamConfig *pPolicyConfig;

    pPolicyConfig = ( StreamConfig * ) sfPolicyUserDataGet( stream_online_config, getNapRuntimePolicy() );
    if (pPolicyConfig == NULL)
        return -1;

    stream_session_config = pPolicyConfig->session_config;

    return 0;
}

bool getStreamIgnoreAnyConfig (struct _SnortConfig *sc, IpProto protocol)
{
    StreamConfig *config;
    tSfPolicyId policyId;

    for (policyId = 0; policyId < sfPolicyNumAllocated(sc->policy_config); policyId++) {

        if ((config = getStreamPolicyConfig(policyId, 0))) {
            switch (protocol) {
               case IPPROTO_TCP :
                   if ((config->tcp_config) && (config->tcp_config->default_policy->flags & STREAM_CONFIG_IGNORE_ANY))
                       return true;
               case IPPROTO_UDP :
                   if ((config->udp_config) && (config->udp_config->default_policy->flags & STREAM_CONFIG_IGNORE_ANY))
                       return true;
               default:
                   break;
            }
        }
    }
    return false;
}
