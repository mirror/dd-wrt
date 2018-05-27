/*
** Copyright (C) 2014-2017 Cisco and/or its affiliates. All rights reserved.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
** DESCRIPTION:
** Enables packet tracing to log verdicts from preprocessors.
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "sf_types.h"
#include "decode.h"
#include "util.h"
#include "session_api.h"
#include "active.h"
#include "pkt_tracer.h"

Verdict_Reason verdict_reason = VERDICT_REASON_NO_BLOCK; // preproc# causing a packet drop
volatile int pkt_trace_cli_flag; // set by message socket
bool pkt_trace_enabled; // set by pktTracerDebugCheck
char trace_line[MAX_TRACE_LINE];    // used by preproc to write a trace

#define MAX_TRACE_SIZE 2048
#define DEBUG_SESSION_ID_SIZE (39+1+5+4+39+1+5+1+3+1+1+1+2+1+10+1+1+1+10+1)
typedef struct _DebugSessionConstraints
{
    uint16_t trace_version;
    struct in6_addr sip;
    int sip_flag;
    struct in6_addr dip;
    int dip_flag;
    uint16_t sport;
    uint16_t dport;
    uint8_t protocol;
} DebugSessionConstraints;

static bool alreadySentTrace;
static bool pkt_trace_enabled_by_lina;
static bool pkt_trace_enabled_by_clish;
static DebugSessionConstraints pkt_tracer_debug_info;
static char pkt_tracer_debug_session[DEBUG_SESSION_ID_SIZE];
static char pktTraceData[MAX_TRACE_SIZE];
static uint32_t pktTraceDataLen = 0;
static bool first_trace_line = true;
static bool trace_line_info_received = false;
static bool trace_line_appid_received = false;

// Handling the special case when SSL blocks a packet before snort gets it
static bool snort_received_packet = false;
static bool ssl_callback_before_snort = false;

static inline void debugParse(const char *desc, const uint8_t *data, uint32_t length,
                          volatile int *debug_flag, DebugSessionConstraints *info)
{
    *debug_flag = 0;
    memset(info, 0, sizeof(*info));
    do
    {
        if (length >= sizeof(info->trace_version))
        {
            memcpy(&info->trace_version, data, sizeof(info->trace_version));
            length -= sizeof(info->trace_version);
            data += sizeof(info->trace_version);
            if (info->trace_version != 1) // 'system support trace' version 1 takes five tuples as input
                break;
        }
        else
            break;

        if (length >= sizeof(info->protocol))
        {
            info->protocol = *data;
            length -= sizeof(info->protocol);
            data += sizeof(info->protocol);
        }
        else
            break;

        if (length >= sizeof(info->sip))
        {

            memcpy(&info->sip, data, sizeof(info->sip));
            if (info->sip.s6_addr32[1] || info->sip.s6_addr32[2] || info->sip.s6_addr32[3])
                info->sip_flag = 1;
            else if (info->sip.s6_addr32[0])
            {
                info->sip.s6_addr32[3] = info->sip.s6_addr32[0];
                info->sip.s6_addr32[0] = 0;
                info->sip.s6_addr16[5] = 0xFFFF;
                info->sip_flag = 1;
            }
            length -= sizeof(info->sip);
            data += sizeof(info->sip);
        }
        else
            break;

        if (length >= sizeof(info->sport))
        {
            memcpy(&info->sport, data, sizeof(info->sport));
            length -= sizeof(info->sport);
            data += sizeof(info->sport);
        }
        else
            break;

        if (length >= sizeof(info->dip))
        {
            memcpy(&info->dip, data, sizeof(info->dip));
            if (info->dip.s6_addr32[1] || info->dip.s6_addr32[2] || info->dip.s6_addr32[3])
                info->dip_flag = 1;
            else if (info->dip.s6_addr32[0])
            {
                info->dip.s6_addr32[3] = info->dip.s6_addr32[0];
                info->dip.s6_addr32[0] = 0;
                info->dip.s6_addr16[5] = 0xFFFF;
                info->dip_flag = 1;
            }
            length -= sizeof(info->dip);
            data += sizeof(info->dip);
        }
        else
            break;

        if (length >= sizeof(info->dport))
        {
            memcpy(&info->dport, data, sizeof(info->dport));
            length -= sizeof(info->dport);
            data += sizeof(info->dport);
        }
        else
            break;
    } while (0);

    if (info->protocol || info->sip_flag || info->sport || info->dip_flag || info->dport)
    {
        int saf;
        int daf;
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        if (!info->sip.s6_addr32[0] && !info->sip.s6_addr32[0] && !info->sip.s6_addr16[4] &&
            info->sip.s6_addr16[5] == 0xFFFF)
        {
            saf = AF_INET;
        }
        else
            saf = AF_INET6;
        if (!info->dip.s6_addr32[0] && !info->dip.s6_addr32[0] && !info->dip.s6_addr16[4] &&
            info->dip.s6_addr16[5] == 0xFFFF)
        {
            daf = AF_INET;
        }
        else
            daf = AF_INET6;
        if (!info->sip_flag)
            saf = daf;
        if (!info->dip_flag)
            daf = saf;
        sipstr[0] = 0;
        inet_ntop(saf, saf == AF_INET ? &info->sip.s6_addr32[3] : info->sip.s6_addr32, sipstr, sizeof(sipstr));
        dipstr[0] = 0;
        inet_ntop(daf, daf == AF_INET ? &info->dip.s6_addr32[3] : info->dip.s6_addr32, dipstr, sizeof(dipstr));
        LogMessage("Debugging %s with %s-%u and %s-%u %u\n", desc,
                    sipstr, (unsigned)info->sport,
                    dipstr, (unsigned)info->dport,
                    (unsigned)info->protocol);
        *debug_flag = 1;
    }
    else
        LogMessage("Debugging %s disabled\n", desc);
}

int DebugPktTracer(uint16_t type, const uint8_t *data, uint32_t length, void **new_context,
               char* statusBuf, int statusBuf_len)
{
    debugParse("packet-tracer", data, length, &pkt_trace_cli_flag, &pkt_tracer_debug_info);
    return 0;
}

bool pktTracerDebugCheck(Packet* p)
{
#if defined(HAVE_DAQ_EXT_MODFLOW) && defined(HAVE_DAQ_PKT_TRACE)
    if (p && p->pkth && p->pkth->flags & DAQ_PKT_FLAG_TRACE_ENABLED)
    {
        pkt_trace_enabled_by_lina = true;
        if (first_trace_line)
        {
            first_trace_line = false;
            LogMessage("PktTracerDbg \n"); // empty line
        }
        LogMessage("PktTracerDbg Tracing enabled by Lina\n"); // troubleshooting message
    }
    else
        pkt_trace_enabled_by_lina = false;
#else
    pkt_trace_enabled_by_lina = false;
#endif
    pkt_trace_enabled_by_clish = false;

    if (!pkt_trace_cli_flag || !p || !(p->iph_api))
        return pkt_trace_enabled_by_lina;

    StreamSessionKey local_key;
    memset(&local_key, 0, sizeof(local_key));
    sfaddr_t *src = GET_SRC_IP(p);
    sfaddr_t *dst = GET_DST_IP(p);
    uint16_t srcPort = p->sp;
    uint16_t dstPort = p->dp;
    uint16_t sport, dport;

    StreamSessionKey *key = &local_key;
    key->protocol = (uint8_t) GET_IPH_PROTO(p);
    switch (key->protocol)
    {
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            sport = srcPort;
            dport = dstPort;
            break;
        case IPPROTO_ICMP:
            if (srcPort == ICMP_ECHOREPLY)
            {
                dport = ICMP_ECHO; /* Treat ICMP echo reply the same as request */
                sport = 0;
            }
            else /* otherwise, every ICMP type gets different key */
            {
                sport = srcPort;
                dport = 0;
            }
            break;
        case IPPROTO_ICMPV6:
            if (srcPort == ICMP6_REPLY)
            {
                dport = ICMP6_ECHO; /* Treat ICMPv6 echo reply the same as request */
                sport = 0;
            }
            else /* otherwise, every ICMP type gets different key */
            {
                sport = srcPort;
                dport = 0;
            }
            break;
        default:
            sport = dport = 0;
            break;
    }

    if (sfip_fast_lt6(src, dst))
    {
        COPY4(key->ip_l, sfaddr_get_ip6_ptr(src));
        key->port_l = sport;
        COPY4(key->ip_h, sfaddr_get_ip6_ptr(dst));
        key->port_h = dport;
    }
    else if (sfip_fast_eq6(src, dst))
    {
        COPY4(key->ip_l, sfaddr_get_ip6_ptr(src));
        COPY4(key->ip_h, sfaddr_get_ip6_ptr(dst));
        if (sport < dport)
        {
            key->port_l = sport;
            key->port_h = dport;
        }
        else
        {
            key->port_l = dport;
            key->port_h = sport;
        }
    }
    else
    {
        COPY4(key->ip_l, sfaddr_get_ip6_ptr(dst));
        key->port_l = dport;
        COPY4(key->ip_h, sfaddr_get_ip6_ptr(src));
        key->port_h = sport;
    }

    if ((!pkt_tracer_debug_info.protocol || pkt_tracer_debug_info.protocol == key->protocol) &&
        (((!pkt_tracer_debug_info.sport || pkt_tracer_debug_info.sport == key->port_l) &&
          (!pkt_tracer_debug_info.sip_flag || memcmp(&pkt_tracer_debug_info.sip, key->ip_l, sizeof(pkt_tracer_debug_info.sip)) == 0) &&
          (!pkt_tracer_debug_info.dport || pkt_tracer_debug_info.dport == key->port_h) &&
          (!pkt_tracer_debug_info.dip_flag || memcmp(&pkt_tracer_debug_info.dip, key->ip_h, sizeof(pkt_tracer_debug_info.dip)) == 0)) ||
         ((!pkt_tracer_debug_info.sport || pkt_tracer_debug_info.sport == key->port_h) &&
           (!pkt_tracer_debug_info.sip_flag || memcmp(&pkt_tracer_debug_info.sip, key->ip_h, sizeof(pkt_tracer_debug_info.sip)) == 0) &&
           (!pkt_tracer_debug_info.dport || pkt_tracer_debug_info.dport == key->port_l) &&
           (!pkt_tracer_debug_info.dip_flag || memcmp(&pkt_tracer_debug_info.dip, key->ip_l, sizeof(pkt_tracer_debug_info.dip)) == 0))))
    {
        int af;
        const struct in6_addr* sip;
        const struct in6_addr* dip;
        unsigned offset;
        uint16_t sport;
        uint16_t dport;
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        if (p->packet_flags & PKT_FROM_CLIENT)
        {
            sip = (const struct in6_addr*)key->ip_l;
            dip = (const struct in6_addr*)key->ip_h;
            sport = key->port_l;
            dport = key->port_h;
        }
        else
        {
            sip = (const struct in6_addr*)key->ip_h;
            dip = (const struct in6_addr*)key->ip_l;
            sport = key->port_h;
            dport = key->port_l;
        }

        sipstr[0] = 0;
        if (sip->s6_addr32[0] || sip->s6_addr32[1] || sip->s6_addr16[4] || (sip->s6_addr16[5] && sip->s6_addr16[5] != 0xFFFF))
        {
            af = AF_INET6;
            offset = 0;
        }
        else
        {
            af = AF_INET;
            offset = 12;
        }
        inet_ntop(af, &sip->s6_addr[offset], sipstr, sizeof(sipstr));
        dipstr[0] = 0;
        if (dip->s6_addr32[0] || dip->s6_addr32[1] || dip->s6_addr16[4] || (dip->s6_addr16[5] && dip->s6_addr16[5] != 0xFFFF))
        {
            af = AF_INET6;
            offset = 0;
        }
        else
        {
            af = AF_INET;
            offset = 12;
        }
        inet_ntop(af, &dip->s6_addr[offset], dipstr, sizeof(dipstr));
        snprintf(pkt_tracer_debug_session, DEBUG_SESSION_ID_SIZE, "%s-%u - %s-%u %u",
            sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol);
        pkt_trace_enabled_by_clish = true;
        return true;
    }
    return pkt_trace_enabled_by_lina;
}

// Get name-string from DAQ_Verdict (must sync with DAQ_Verdict defined at daq_common.h)
static char* getVerdictStr(DAQ_Verdict vd)
{
    static char* vName[] = {"PASS", "BLOCK", "REPLACE", "WHITELIST", "BLACKLIST",
                            "IGNORE", "RETRY", "Error in reading verdict!"};

    if (vd > MAX_DAQ_VERDICT)
        vd = MAX_DAQ_VERDICT;
    return vName[vd];
}

// Get name-string from Verdict_Reason (must sync with Verdict_Reason defined at sf_dynamic_common.h)
static char* getModuleStr(Verdict_Reason module)
{
    static char* mName[] = {"Packet Information", "Session String", "None", "a module", "DAQ Retry", "Snort",
        "AppID", "SSL", "Firewall", "Captive Portal", "Safe Search", "SI", "Prefilter", "FTP",
        "Stream", "Session", "Defragmentation", "Snort React", "Snort Response", "SI/Reputation",
        "X-Link2State", "Back Orifice", "SMB", "File Process", "IPS"};

    if (module >= MAX_VERDICT_REASON)
        module = VERDICT_REASON_UNKNOWN;
    return mName[module];
}

// If enabled by Lina, add trace_line to pktTraceData buffer, else print to /var/log/message
void addPktTraceData(int reason, int traceLen)
{
    Verdict_Reason module = (Verdict_Reason) reason;
    if (!snort_received_packet && module == VERDICT_REASON_SFSSL)
        ssl_callback_before_snort = true; // used in addPktTraceInfo on receiving packet by snort
    else if (module == VERDICT_REASON_INFO)
    {
        if (trace_line_info_received) return; // records only one packet info trace
        else trace_line_info_received = true;
    }
    else if (module == VERDICT_REASON_APPID)
    {
        if (trace_line_appid_received) return; // records only one appid trace
        else trace_line_appid_received = true;
    }
    else if (module > VERDICT_REASON_NO_BLOCK &&
             (verdict_reason == VERDICT_REASON_NO_BLOCK || module == VERDICT_REASON_SI))
        verdict_reason = module; // records the first blocking module and any subsequent blocking from SI

    if (traceLen <= 0) return;

    if (module == VERDICT_REASON_SSNSTR) // update debug session string
    {
        strncpy(pkt_tracer_debug_session, trace_line,
            (traceLen >= DEBUG_SESSION_ID_SIZE)? DEBUG_SESSION_ID_SIZE-1 : traceLen);
        pkt_tracer_debug_session[DEBUG_SESSION_ID_SIZE-1] = '\0';
        return;
    }

    if (pkt_trace_enabled_by_clish) // tracing enabled by 'system support tracer'
    {
        if (first_trace_line)
        {
            first_trace_line = false;
            LogMessage("PktTracerDbg \n"); // empty line
        }
        LogMessage("PktTracerDbg %s %s\n", pkt_tracer_debug_session, trace_line);
    }
    if (pkt_trace_enabled_by_lina) // tracing enabled by Lina flag
    {
        if (pktTraceDataLen + traceLen >= MAX_TRACE_SIZE)
        {
            LogMessage("Packet trace buffer is full; logging skipped!\n");
            return;
        }
        strncat(pktTraceData, trace_line, traceLen);
        pktTraceDataLen += traceLen;
        pktTraceData[MAX_TRACE_SIZE-1] = '\0';
    }
}

// Writes pktTraceData buffer into PDTS or /var/log/message
void writePktTraceData(DAQ_Verdict verdict, unsigned int napId, unsigned int ipsId, const Packet* p)
{
    alreadySentTrace = false;
    if (pkt_trace_enabled_by_clish)
    {
        LogMessage("PktTracerDbg %s NAP id %u, IPS id %u, Verdict %s\n", pkt_tracer_debug_session,
            napId, ipsId, getVerdictStr(verdict));
        if (verdict_reason != VERDICT_REASON_NO_BLOCK)
            LogMessage("PktTracerDbg %s ===> Blocked by %s\n", pkt_tracer_debug_session, getModuleStr(verdict_reason));
    }
#if defined(HAVE_DAQ_EXT_MODFLOW) && defined(HAVE_DAQ_PKT_TRACE)
    if (pkt_trace_enabled_by_lina)
    {
        int len;
        if (pktTraceDataLen >= MAX_TRACE_SIZE)
            len = 0;
        else if (verdict_reason == VERDICT_REASON_NO_BLOCK)
            len = snprintf(pktTraceData+pktTraceDataLen, MAX_TRACE_SIZE - pktTraceDataLen, "NAP id %u, IPS id %u, Verdict %s\n",
                    napId, ipsId, getVerdictStr(verdict));
        else
            len = snprintf(pktTraceData+pktTraceDataLen, MAX_TRACE_SIZE - pktTraceDataLen, "NAP id %u, IPS id %u, Verdict %s, %s %s\n", napId, ipsId,
                getVerdictStr(verdict), "Blocked by", getModuleStr(verdict_reason));
        if (len > 0)
        {
            pktTraceDataLen += len;
            pktTraceData[MAX_TRACE_SIZE-1] = '\0';
        }
        // Send to DAQ
        DAQ_ModFlow_t mod;
        DAQ_ModFlowPktTrace_t mod_tr;
        mod_tr.vreason = (uint8_t) verdict_reason;
        mod_tr.pkt_trace_data_len = pktTraceDataLen+1; // accounting '\0'
        mod_tr.pkt_trace_data = (uint8_t *) pktTraceData;
        mod.type = DAQ_MODFLOW_TYPE_PKT_TRACE;
        mod.length = sizeof(DAQ_ModFlowPktTrace_t);
        mod.value = (void *) &mod_tr;
        DAQ_ModifyFlow(p->pkth, &mod);
        if (pkt_trace_enabled_by_clish) // troubleshooting message in CLISH
            LogMessage("PktTracerDbg Trace buffer and verdict reason are sent to DAQ's PDTS\n");
        if (verdict_reason != VERDICT_REASON_NO_BLOCK)
            alreadySentTrace = true;
    }
#endif

    // re-initialize after writing each packet trace
    pktTraceData[0] = '\0';
    pktTraceDataLen = 0;
    trace_line_info_received = false;
    trace_line_appid_received = false;
    first_trace_line = true;
    snort_received_packet = false;
    ssl_callback_before_snort = false;
}

void addPktTraceInfo(void *packet)
{
    Packet* p = (Packet *) packet;

    snort_received_packet = true;
    if (ssl_callback_before_snort) // special case
    {
        verdict_reason = VERDICT_REASON_SFSSL;
        ssl_callback_before_snort = false;
    }

    if (IsTCP(p))
    {
        if (p->tcph->th_flags & TH_ACK)
        {
            addPktTraceData(VERDICT_REASON_INFO, snprintf(trace_line, MAX_TRACE_LINE,
                "Packet: TCP%s%s, ACK%s%s, seq %lu, ack %lu\n",
                PacketIsRebuilt(p)? " rebuilt" : "",
                p->tcph->th_flags & TH_SYN? ", SYN" : "",
                p->tcph->th_flags & TH_RST? ", RST" : "",
                p->tcph->th_flags & TH_FIN? ", FIN" : "",
                (unsigned long) ntohl(p->tcph->th_seq),
                (unsigned long) ntohl(p->tcph->th_ack)));
        }
        else
        {
            addPktTraceData(VERDICT_REASON_INFO, snprintf(trace_line, MAX_TRACE_LINE,
                "Packet: TCP%s%s%s%s, seq %lu\n",
                PacketIsRebuilt(p)? " rebuilt" : "",
                p->tcph->th_flags & TH_SYN? ", SYN" : "",
                p->tcph->th_flags & TH_RST? ", RST" : "",
                p->tcph->th_flags & TH_FIN? ", FIN" : "",
                (unsigned long) ntohl(p->tcph->th_seq)));
        }
    }
    else if (IsUDP(p))
    {
        addPktTraceData(VERDICT_REASON_INFO, snprintf(trace_line, MAX_TRACE_LINE,
            "Packet: UDP\n"));
    }
    else if (IsICMP(p))
    {
            addPktTraceData(VERDICT_REASON_INFO, snprintf(trace_line, MAX_TRACE_LINE,
                "Packet: ICMP\n"));
    }
    else if (IsIP(p))
    {
            addPktTraceData(VERDICT_REASON_INFO, snprintf(trace_line, MAX_TRACE_LINE,
                "Packet: IP\n"));
    }
    else
    {
        addPktTraceData(VERDICT_REASON_INFO, snprintf(trace_line, MAX_TRACE_LINE,
            "Packet: not IP/TCP/UDP/ICMP\n"));
    }
}

const char* getPktTraceActMsg()
{
    static const char* msg[] = {"drop", "would drop", "can't drop"};

    if (Active_PacketWasDropped())
        return msg[0];
    else if (Active_PacketWouldBeDropped())
        return msg[1];
    else return msg[2];
}

#if defined(HAVE_DAQ_EXT_MODFLOW) && defined(HAVE_DAQ_VERDICT_REASON)
void sendReason(const Packet* p)
{
    if (alreadySentTrace) // already sent verdict reason with trace data
    {
        alreadySentTrace = false;
        return;
    }
    DAQ_ModFlow_t mod;
    mod.type = DAQ_MODFLOW_TYPE_VER_REASON;
    mod.length = sizeof(uint8_t);
    mod.value = (void*)&verdict_reason;
    DAQ_ModifyFlow(p->pkth, &mod);
    if (pkt_trace_enabled_by_clish) // troubleshooting message in CLISH
        LogMessage("PktTracerDbg Verdict reason is sent to DAQ's PDTS\n");
}
#endif
