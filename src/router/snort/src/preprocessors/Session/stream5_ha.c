/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
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
 *****************************************************************************/

/**************************************************************************
 *
 * stream5_ha.c
 *
 * Authors: Michael Altizer <maltizer@sourcefire.com>, Russ Combs <rcombs@sourcefire.com>
 *
 * Description:
 *
 * Stream high availability support.
 *
 **************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "active.h"
#include "mstring.h"
#include "packet_time.h"
#include "parser.h"
#include "sfcontrol_funcs.h"
#ifdef SIDE_CHANNEL
#include "sidechannel.h"
#endif
#include "stream5_ha.h"
#include "util.h"

/*
 * Stream HA messages will have the following format:
 *
 * <message>  ::= <header> <has-rec> <psd-rec>
 * <header>   ::= <event> version <message length> <key>
 * <event>    ::= HA_EVENT_UPDATE | HA_EVENT_DELETE
 * <key>      ::= <ipv4-key> | <ipv6-key>
 * <ipv4-key> ::= HA_TYPE_KEY sizeof(ipv4-key) ipv4-key
 * <ipv6-key> ::= HA_TYPE_KEY sizeof(ipv6-key) ipv6-key
 * <has-rec>  ::= HA_TYPE_HAS sizeof(has-rec) has-rec | (null)
 * <psd-rec>  ::= HA_TYPE_PSD sizeof(psd-rec) psd-preprocid psd-subcode psd-rec <psd-rec> | (null)
 */

typedef struct _StreamHAFuncsNode
{
    uint16_t id;
    uint16_t mask;
    uint8_t preproc_id;
    uint8_t subcode;
    uint8_t size;
    StreamHAProducerFunc produce;
    StreamHAConsumerFunc consume;
    uint32_t produced;
    uint32_t consumed;
} StreamHAFuncsNode;

typedef enum
{
    HA_TYPE_KEY,    // Lightweight Session Key
    HA_TYPE_HAS,    // Lightweight Session Data
    HA_TYPE_PSD,    // Preprocessor-specific Data
    HA_TYPE_MAX
} HA_Type;

typedef struct _MsgHeader
{
    uint8_t event;
    uint8_t version;
    uint16_t total_length;
    uint8_t key_type;
    uint8_t key_size;
} MsgHeader;

typedef struct _RecordHeader
{
    uint8_t type;
    uint8_t length;
} RecordHeader;

typedef struct _PreprocDataHeader
{
    uint8_t preproc_id;
    uint8_t subcode;
} PreprocDataHeader;

/* Something more will probably be added to this structure in the future... */
#define HA_SESSION_FLAG_LOW     0x01     // client address / port is low in key
#define HA_SESSION_FLAG_IP6     0x02     // key addresses are ip6
typedef struct
{
    StreamHAState ha_state;
    uint8_t flags;
} StreamHASession;

typedef struct
{
    uint32_t update_messages_received;
    uint32_t update_messages_received_no_session;
    uint32_t delete_messages_received;
    uint32_t update_messages_sent_immediately;
    uint32_t update_messages_sent_normally;
    uint32_t delete_messages_sent;
    uint32_t delete_messages_not_sent;
} StreamHAStats;

typedef struct _HADebugSessionConstraints
{
    int sip_flag;
    struct in6_addr sip;
    int dip_flag;
    struct in6_addr dip;
    uint16_t sport;
    uint16_t dport;
    uint8_t protocol;
} HADebugSessionConstraints;

#define MAX_STREAM_HA_FUNCS 8  // depends on sizeof(SessionControlBlock.ha_pending_mask)
#define HA_MESSAGE_VERSION  0x82
static StreamHAFuncsNode *stream_ha_funcs[MAX_STREAM_HA_FUNCS];
static int n_stream_ha_funcs = 0;
static int runtime_output_fd = -1;
#ifdef REG_TEST
static int  runtime_input_fd = -1;
#endif
static uint8_t file_io_buffer[UINT16_MAX];
static StreamHAStats s5ha_stats;

/* Runtime debugging stuff. */
#define HA_DEBUG_SESSION_ID_SIZE    (39+1+5+5+39+1+5+1+3+1) /* "<IPv6 address>:<port> <-> <IPv6 address>:<port> <ipproto>\0" */
static HADebugSessionConstraints s5_ha_debug_info;
static volatile int s5_ha_debug_flag = 0;
static char s5_ha_debug_session[HA_DEBUG_SESSION_ID_SIZE];
uint32_t HA_CRITICAL_SESSION_FLAGS = ( SSNFLAG_DROP_CLIENT | SSNFLAG_DROP_SERVER | SSNFLAG_RESET );

#define IP6_SESSION_KEY_SIZE sizeof(SessionKey)
#define IP4_SESSION_KEY_SIZE (IP6_SESSION_KEY_SIZE - 24)

#ifdef PERF_PROFILING
PreprocStats sessionHAPerfStats;
PreprocStats sessionHAConsumePerfStats;
PreprocStats sessionHAProducePerfStats;
#endif

static int ConsumeHAMessage(const uint8_t *msg, uint32_t msglen);

//--------------------------------------------------------------------
//  Runtime debugging support.
//--------------------------------------------------------------------
static inline bool StreamHADebugCheck(const SessionKey *key, volatile int debug_flag,
                                        HADebugSessionConstraints *info, char *debug_session, size_t debug_session_len)
{
#ifndef REG_TEST
    if (debug_flag)
    {
        if ((!info->protocol || info->protocol == key->protocol) &&
                (((!info->sport || info->sport == key->port_l) &&
                  (!info->sip_flag || memcmp(&info->sip, key->ip_l, sizeof(info->sip)) == 0) &&
                  (!info->dport || info->dport == key->port_h) &&
                  (!info->dip_flag || memcmp(&info->dip, key->ip_h, sizeof(info->dip)) == 0)) ||
                 ((!info->sport || info->sport == key->port_h) &&
                  (!info->sip_flag || memcmp(&info->sip, key->ip_h, sizeof(info->sip)) == 0) &&
                  (!info->dport || info->dport == key->port_l) &&
                  (!info->dip_flag || memcmp(&info->dip, key->ip_l, sizeof(info->dip)) == 0))))
        {
#endif
            char lipstr[INET6_ADDRSTRLEN];
            char hipstr[INET6_ADDRSTRLEN];

            lipstr[0] = '\0';
            hipstr[0] = '\0';
            if (key->ip_l[0] || key->ip_l[1] || key->ip_l[2] != htonl(0xFFFF) ||
                key->ip_h[0] || key->ip_h[1] || key->ip_h[2] != htonl(0xFFFF))
            {
                inet_ntop(AF_INET6, key->ip_l, lipstr, sizeof(lipstr));
                inet_ntop(AF_INET6, key->ip_h, hipstr, sizeof(hipstr));
            }
            else
            {
                inet_ntop(AF_INET, &key->ip_l[3], lipstr, sizeof(lipstr));
                inet_ntop(AF_INET, &key->ip_h[3], hipstr, sizeof(hipstr));
            }
            snprintf(debug_session, debug_session_len, "%s:%hu <-> %s:%hu %hhu",
                    lipstr, key->port_l, hipstr, key->port_h, key->protocol);
            return true;
#ifndef REG_TEST
        }
    }

    return false;
#endif
}

static void StreamHADebugParse(const char *desc, const uint8_t *data, uint32_t length,
        volatile int *debug_flag, HADebugSessionConstraints *info)
{
    *debug_flag = 0;
    memset(info, 0, sizeof(*info));
    do
    {
        if (length >= sizeof(info->protocol))
        {
            info->protocol = *(uint8_t *)data;
            length -= sizeof(info->protocol);
            data += sizeof(info->protocol);
        }
        else
            break;

        if (length >= sizeof(info->sip))
        {
            if (memcmp(data + 4, info->sip.s6_addr + 4, 12) == 0)
            {
                if (memcmp(data, info->sip.s6_addr, 4) != 0)
                {
                    info->dip_flag = 1;
                    info->sip.s6_addr32[0] = info->sip.s6_addr32[1] = info->sip.s6_addr16[4] = 0;
                    info->sip.s6_addr16[5] = 0xFFFF;
                    info->sip.s6_addr32[3] = *(uint32_t*)data;
                }
            }
            else if (memcmp(data, info->sip.s6_addr, 4) != 0)
            {
                info->dip_flag = 1;
                memcpy(&info->sip, data, sizeof(info->sip));
            }
            length -= sizeof(info->sip);
            data += sizeof(info->sip);
        }
        else
            break;

        if (length >= sizeof(info->sport))
        {
            info->sport = *(uint16_t *)data;
            length -= sizeof(info->sport);
            data += sizeof(info->sport);
        }
        else
            break;

        if (length >= sizeof(info->dip))
        {
            if (memcmp(data + 4, info->dip.s6_addr + 4, 12) == 0)
            {
                if (memcmp(data, info->dip.s6_addr, 4) != 0)
                {
                    info->dip_flag = 1;
                    info->dip.s6_addr32[0] = info->dip.s6_addr32[1] = info->dip.s6_addr16[4] = 0;
                    info->dip.s6_addr16[5] = 0xFFFF;
                    info->dip.s6_addr32[3] = *(uint32_t*)data;
                }
            }
            else if (memcmp(data, info->dip.s6_addr, 4) != 0)
            {
                info->dip_flag = 1;
                memcpy(&info->dip, data, sizeof(info->dip));
            }
            length -= sizeof(info->dip);
            data += sizeof(info->dip);
        }
        else
            break;

        if (length >= sizeof(info->dport))
        {
            info->dport = *(uint16_t *)data;
            length -= sizeof(info->dport);
            data += sizeof(info->dport);
        }
        else
            break;
    } while (0);

    if (info->protocol || info->sip_flag || info->sport || info->dip_flag || info->dport)
    {
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        sipstr[0] = '\0';
        if (info->sip_flag)
            inet_ntop(AF_INET6, &info->sip, sipstr, sizeof(sipstr));
        else
            snprintf(sipstr, sizeof(sipstr), "any");

        dipstr[0] = '\0';
        if (info->dip_flag)
            inet_ntop(AF_INET6, &info->dip, dipstr, sizeof(dipstr));
        else
            snprintf(dipstr, sizeof(dipstr), "any");

        LogMessage("Debugging %s with %s-%hu and %s-%hu %hhu\n", desc,
                    sipstr, info->sport, dipstr, info->dport, info->protocol);
        *debug_flag = 1;
    }
    else
        LogMessage("Debugging %s disabled\n", desc);
}

static int StreamDebugHA(uint16_t type, const uint8_t *data, uint32_t length, void **new_context, char *statusBuf, int statusBuf_len)
{
    StreamHADebugParse("S5HA", data, length, &s5_ha_debug_flag, &s5_ha_debug_info);

    return 0;
}

//--------------------------------------------------------------------
// Protocol-specific HA API
// could use an array here (and an enum instead of IPPROTO_*)
//--------------------------------------------------------------------

static const HA_Api *s_tcp = NULL;
static const HA_Api *s_udp = NULL;
static const HA_Api *s_ip = NULL;

int ha_set_api(unsigned proto, const HA_Api *api)
{
    switch (proto)
    {
        case IPPROTO_TCP:
            s_tcp = api;
            break;
        case IPPROTO_UDP:
            s_udp = api;
            break;
        case IPPROTO_IP:
            s_ip = api;
            break;
        default:
            return -1;
    }
    return 0;
}

static inline const HA_Api *ha_get_api(unsigned proto)
{
    switch (proto)
    {
        case IPPROTO_TCP:
            return s_tcp;
        case IPPROTO_UDP:
            return s_udp;
        case IPPROTO_ICMP:
        case IPPROTO_IP:
        default:
            return s_ip;
    }
    return NULL;
}

static inline void ha_print_key(const SessionKey *key, int rx, uint8_t event)
{
#if STREAM_DEBUG_ENABLED
    char ipA[INET6_ADDRSTRLEN], ipB[INET6_ADDRSTRLEN];

    if (key->ip_l[1] || key->ip_l[2] || key->ip_l[3] || key->ip_h[1] || key->ip_h[2] || key->ip_h[3])
    {
        sfip_raw_ntop(AF_INET6, key->ip_l, ipA, sizeof(ipA));
        sfip_raw_ntop(AF_INET6, key->ip_h, ipB, sizeof(ipB));
    }
    else
    {
        sfip_raw_ntop(AF_INET, key->ip_l, ipA, sizeof(ipA));
        sfip_raw_ntop(AF_INET, key->ip_h, ipB, sizeof(ipB));
    }
    LogMessage("%s flow %s message for %s:%hu <-> %s:%hu, Protocol %hhu\n",
               rx ? "Receiving" : "Sending", (event == HA_EVENT_DELETE) ? "deletion" : "update",
               ipA, key->port_l, ipB, key->port_h, key->protocol);
#endif
}

int RegisterSessionHAFuncs(uint32_t preproc_id, uint8_t subcode, uint8_t size,
                            StreamHAProducerFunc produce, StreamHAConsumerFunc consume)
{
    StreamHAFuncsNode *node;
    int i, idx;

    if (produce == NULL || consume == NULL)
    {
        FatalError("One must be both a producer and a consumer to participate in Stream HA!\n");
    }

    if (preproc_id > UINT8_MAX)
    {
        FatalError("Preprocessor ID must be between 0 and %d to participate in Stream HA!\n", UINT8_MAX);
    }

    idx = n_stream_ha_funcs;
    for (i = 0; i < n_stream_ha_funcs; i++)
    {
        node = stream_ha_funcs[i];
        if (node)
        {
            if (preproc_id == node->preproc_id && subcode == node->subcode)
            {
                FatalError("Duplicate Stream HA registration attempt for preprocessor %hu with subcode %hu\n",
                           (unsigned short)node->preproc_id, (unsigned short)node->subcode);
            }
        }
        else if (idx == n_stream_ha_funcs)
            idx = i;
    }

    if (idx == MAX_STREAM_HA_FUNCS)
    {
        FatalError("Attempted to register more than %d Stream HA types!\n", MAX_STREAM_HA_FUNCS);
    }

    if (idx == n_stream_ha_funcs)
        n_stream_ha_funcs++;

    node = (StreamHAFuncsNode *) SnortAlloc(sizeof(StreamHAFuncsNode));
    node->id = idx;
    node->mask = (1 << idx);
    node->preproc_id = (uint8_t) preproc_id;
    node->subcode = subcode;
    node->size = size;
    node->produce = produce;
    node->consume = consume;

    stream_ha_funcs[idx] = node;

    LogMessage("StreamHA: Registered node %hu for preprocessor ID %hhu with subcode %hhu (size %hhu)\n",
                node->id, node->preproc_id, node->subcode, node->size);

    return idx;
}

void UnregisterSessionHAFuncs(uint32_t preproc_id, uint8_t subcode)
{
    StreamHAFuncsNode *node;
    int i;

    for (i = 0; i < n_stream_ha_funcs; i++)
    {
        node = stream_ha_funcs[i];
        if (node && preproc_id == node->preproc_id && subcode == node->subcode)
        {
            stream_ha_funcs[i] = NULL;
            free(node);
            break;
        }
    }

    if ((i + 1) == n_stream_ha_funcs)
        n_stream_ha_funcs--;
}

void SessionSetHAPendingBit(void *ssnptr, int bit)
{
    SessionControlBlock *scb = (SessionControlBlock*) ssnptr;

    if (!scb)
        return;

    if (bit >= n_stream_ha_funcs || !stream_ha_funcs[bit])
    {
        FatalError("Attempted to set illegal HA pending bit %d!\n", bit);
    }

    scb->ha_pending_mask |= (1 << bit);
}

static void StreamParseHAArgs(SnortConfig *sc, SessionHAConfig *config, char *args)
{
    char **toks;
    int num_toks;
    int i;
    char **stoks = NULL;
    int s_toks;
    char *endPtr = NULL;
    unsigned long int value;

    if (config == NULL)
        return;

    if ((args == NULL) || (strlen(args) == 0))
        return;

    toks = mSplit(args, ",", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        stoks = mSplit(toks[i], " ", 2, &s_toks, 0);

        if (s_toks == 0)
        {
            FatalError("%s(%d) => Missing parameter in Stream HA config.\n",
                    file_name, file_line);
        }

        if (!strcmp(stoks[0], "min_session_lifetime"))
        {
            if (stoks[1])
                value = strtoul(stoks[1], &endPtr, 10);
            else
                value = 0;

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid '%s' in config file. Requires integer parameter.\n",
                           file_name, file_line, stoks[0]);
            }

            if (value > UINT16_MAX)
            {
                FatalError("%s(%d) => '%s %lu' invalid: value must be between 0 and %d milliseconds.\n",
                           file_name, file_line, stoks[0], value, UINT16_MAX);
            }

            config->min_session_lifetime.tv_sec = 0;
            while (value >= 1000)
            {
                config->min_session_lifetime.tv_sec++;
                value -= 1000;
            }
            config->min_session_lifetime.tv_usec = value * 1000;
        }
        else if (!strcmp(stoks[0], "min_sync_interval"))
        {
            if (stoks[1])
                value = strtoul(stoks[1], &endPtr, 10);
            else
                value = 0;

            if (!stoks[1] || (endPtr == &stoks[1][0]) || *endPtr)
            {
                FatalError("%s(%d) => Invalid '%s' in config file. Requires integer parameter.\n",
                           file_name, file_line, stoks[0]);
            }

            if (value > UINT16_MAX)
            {
                FatalError("%s(%d) => '%s %lu' invalid: value must be between 0 and %d milliseconds.\n",
                           file_name, file_line, stoks[0], value, UINT16_MAX);
            }

            config->min_sync_interval.tv_sec = 0;
            while (value >= 1000)
            {
                config->min_sync_interval.tv_sec++;
                value -= 1000;
            }
            config->min_sync_interval.tv_usec = value * 1000;
        }
        else if (!strcmp(stoks[0], "startup_input_file"))
        {
            if (!stoks[1])
            {
                FatalError("%s(%d) => '%s' missing an argument\n", file_name, file_line, stoks[0]);
            }
            if (config->startup_input_file)
            {
                FatalError("%s(%d) => '%s' specified multiple times\n", file_name, file_line, stoks[0]);
            }
            config->startup_input_file = SnortStrdup(stoks[1]);
        }
        else if (!strcmp(stoks[0], "runtime_output_file"))
        {
            if (!stoks[1])
            {
                FatalError("%s(%d) => '%s' missing an argument\n", file_name, file_line, stoks[0]);
            }
            if (config->runtime_output_file)
            {
                FatalError("%s(%d) => '%s' specified multiple times\n", file_name, file_line, stoks[0]);
            }
            config->runtime_output_file = SnortStrdup(stoks[1]);
        }
        else if (!strcmp(stoks[0], "shutdown_output_file"))
        {
            if (!stoks[1])
            {
                FatalError("%s(%d) => '%s' missing an argument\n", file_name, file_line, stoks[0]);
            }
            if (config->shutdown_output_file)
            {
                FatalError("%s(%d) => '%s' specified multiple times\n", file_name, file_line, stoks[0]);
            }
            config->shutdown_output_file = SnortStrdup(stoks[1]);
        }
        else if (!strcmp(stoks[0], "use_side_channel"))
        {
#ifdef SIDE_CHANNEL
            if (!sc->side_channel_config.enabled)
            {
                FatalError("%s(%d) => '%s' cannot be specified without enabling the Snort side channel.\n",
                            file_name, file_line, stoks[0]);
            }
            config->use_side_channel = 1;
#else
            FatalError("%s(%d) => Snort has been compiled without Side Channel support.\n", file_name, file_line);
#endif
        }
        else if (!strcmp(stoks[0], "use_daq"))
        {
#ifdef HAVE_DAQ_EXT_MODFLOW
            config->use_daq = 1;
#else
            FatalError("%s(%d) => Snort has been compiled against a LibDAQ version without extended flow modifier support.\n",
                        file_name, file_line);
#endif
        }
        else
        {
            FatalError("%s(%d) => Invalid Stream HA config option '%s'\n",
                    file_name, file_line, stoks[0]);
        }

        mSplitFree(&stoks, s_toks);
    }

    mSplitFree(&toks, num_toks);
#ifdef REG_TEST
    if(sc->ha_out)
    {
        if(config->runtime_output_file)
            free(config->runtime_output_file);
        config->runtime_output_file = SnortStrdup(sc->ha_out);
	}
    if(sc->ha_in)
    {
        if(config->startup_input_file)
            free(config->startup_input_file);
        config->startup_input_file = SnortStrdup(sc->ha_in);
    }
   if(sc->ha_pdts_in)
   {
       if(config->runtime_input_file)
           free(config->runtime_input_file);
       config->runtime_input_file = SnortStrdup(sc->ha_pdts_in);
   }
#endif

}

static void StreamPrintHAConfig(SessionHAConfig *config)
{
    if (config == NULL)
        return;

    LogMessage("Stream HA config:\n");
    LogMessage("    Minimum Session Lifetime: %lu milliseconds\n",
                config->min_session_lifetime.tv_sec * 1000 + config->min_session_lifetime.tv_usec / 1000);
    LogMessage("    Minimum Sync Interval: %lu milliseconds\n",
                config->min_sync_interval.tv_sec * 1000 + config->min_sync_interval.tv_usec / 1000);
    if (config->startup_input_file)
        LogMessage("    Startup Input File:    %s\n", config->startup_input_file);
    if (config->runtime_output_file)
        LogMessage("    Runtime Output File:   %s\n", config->runtime_output_file);
    if (config->shutdown_output_file)
        LogMessage("    Shutdown Output File:  %s\n", config->shutdown_output_file);
#ifdef SIDE_CHANNEL
    LogMessage("    Using Side Channel:    %s\n", config->use_side_channel ? "Yes" : "No");
#endif
    LogMessage("    Using DAQ:             %s\n", config->use_daq ? "Yes" : "No");
#ifdef REG_TEST
    LogMessage("    Stream LWS HA Data Size: %zu\n", sizeof(StreamHASession));
#endif
}

void SessionPrintHAStats(void)
{
    StreamHAFuncsNode *node;
    int i;

    LogMessage("  High Availability\n");
    LogMessage("          Updates Received: %u\n", s5ha_stats.update_messages_received);
    LogMessage("Updates Received (No Session): %u\n", s5ha_stats.update_messages_received_no_session);
    LogMessage("        Deletions Received: %u\n", s5ha_stats.delete_messages_received);
    LogMessage("  Updates Sent Immediately: %u\n", s5ha_stats.update_messages_sent_immediately);
    LogMessage("     Updates Sent Normally: %u\n", s5ha_stats.update_messages_sent_normally);
    LogMessage("            Deletions Sent: %u\n", s5ha_stats.delete_messages_sent);
    LogMessage("        Deletions Not Sent: %u\n", s5ha_stats.delete_messages_not_sent);
    for (i = 0; i < n_stream_ha_funcs; i++)
    {
        node = stream_ha_funcs[i];
        if (!node)
            continue;
        LogMessage("        Node %hhu/%hhu: %u produced, %u consumed\n",
                    node->preproc_id, node->subcode, node->produced, node->consumed);
    }
}

void SessionResetHAStats(void)
{
    memset(&s5ha_stats, 0, sizeof(s5ha_stats));
}

#ifdef HAVE_DAQ_EXT_MODFLOW
static void SessionHAMetaEval(int type, const uint8_t *data)
{
    DAQ_HA_State_Data_t *daqState;

    if (type != DAQ_METAHDR_TYPE_HA_STATE)
        return;

    daqState = (DAQ_HA_State_Data_t *) data;

    if (daqState->length == 0 || daqState->length >= UINT16_MAX || !daqState->data)
        return;

    /* Ignore the return value from consuming the message - it will print out
        errors and there's nothing we can do about it. */
    ConsumeHAMessage(daqState->data, daqState->length);
}
#endif

#ifdef HAVE_DAQ_QUERYFLOW
#ifdef REG_TEST
int SessionHAQueryDAQState( DAQ_PktHdr_t *pkthdr)
#else
int SessionHAQueryDAQState(const DAQ_PktHdr_t *pkthdr)
#endif
{
    DAQ_QueryFlow_t query;
    DAQ_HA_State_Data_t daqHAState;
    int rval;

    query.type = DAQ_QUERYFLOW_TYPE_HA_STATE;
    query.length = sizeof(daqHAState);
    query.value = &daqHAState;

#ifdef REG_TEST
    pkthdr->priv_ptr = &runtime_input_fd;
#endif

    if ((rval = DAQ_QueryFlow(pkthdr, &query)) == DAQ_SUCCESS)
    {
        if (daqHAState.length == 0 || daqHAState.length >= UINT16_MAX || !daqHAState.data)
            return -EINVAL;
        rval = ConsumeHAMessage(daqHAState.data, daqHAState.length);
    }

    return rval;
}
#endif

void SessionHAInit( struct _SnortConfig *sc, char *args )
{
    if( session_configuration == NULL )
        FatalError("Tried to config Session HA policy without core Session config!\n");

    if( session_configuration->ha_config != NULL )
        FatalError("%s(%d) ==> Cannot duplicate Sesssion HA configuration\n", file_name, file_line);

    // if HA not enabled for session, then we are out of here...
    if( !session_configuration->enable_ha )
        return;

    session_configuration->ha_config = ( SessionHAConfig * ) SnortAlloc( sizeof( SessionHAConfig ) );
    StreamParseHAArgs(sc, session_configuration->ha_config, args);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("sessionHAProduce", &sessionHAProducePerfStats, 2, &sessionHAPerfStats, NULL);
    RegisterPreprocessorProfile("sessionHAConsume", &sessionHAConsumePerfStats, 0, &totalPerfStats, NULL);
#endif

    ControlSocketRegisterHandler(CS_TYPE_DEBUG_STREAM_HA, StreamDebugHA, NULL, NULL);
    LogMessage("Registered StreamHA Debug control socket message type (0x%x)\n", CS_TYPE_DEBUG_STREAM_HA);

#ifdef HAVE_DAQ_EXT_MODFLOW
    if (session_configuration->ha_config->use_daq)
    {
        AddFuncToPreprocMetaEvalList(sc, SessionHAMetaEval, PP_SESSION_PRIORITY, PP_SESSION);
        //Do not send delete notification on RST when there is a underlying data plane.
        HA_CRITICAL_SESSION_FLAGS &= ~SSNFLAG_RESET;
    }    
#endif

    StreamPrintHAConfig(session_configuration->ha_config);
}

#if defined(SNORT_RELOAD)
void SessionHAReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    SessionHAConfig *session_ha_config = ( SessionHAConfig * ) *new_config;

    // session ha config is only in default policy... just return if not default
    if( getParserPolicy( sc ) != getDefaultPolicy( ) )
        return;

    // if HA not enabled for session, then we are out of here...
    if( !session_configuration->enable_ha )
        return;

    if ( session_ha_config == NULL )
    {
        session_ha_config = ( SessionHAConfig * ) SnortAlloc( sizeof( SessionHAConfig ) );
        if ( session_ha_config == NULL )
            FatalError("Failed to allocate storage for Session HA configuration.\n");

        StreamParseHAArgs( sc, session_ha_config, args );
        StreamPrintHAConfig( session_ha_config );
       *new_config = session_ha_config;

    }
    else
    {
        FatalError("%s(%d) ==> Cannot duplicate Sesssion HA configuration\n", file_name, file_line);
    }
}
#endif

int SessionVerifyHAConfig(struct _SnortConfig *sc, void *swap_config)
{
    if (swap_config == NULL)
        return -1;

    return 0;
}

void *SessionHASwapReload( struct _SnortConfig *sc, void *data )
{
    session_configuration->ha_config = ( SessionHAConfig * ) data;
    return NULL;
}


void SessionHAConfigFree( void *data )
{
    SessionHAConfig *config = ( SessionHAConfig * ) data;

    if (config == NULL)
        return;

    if (config->startup_input_file)
        free(config->startup_input_file);

    if (config->runtime_output_file)
        free(config->runtime_output_file);

    if (config->shutdown_output_file)
        free(config->shutdown_output_file);
#ifdef REG_TEST
    if (config->runtime_input_file)
        free(config->runtime_input_file);
#endif

    free(config);
}

// This MUST have the exact same logic as createSessionKeyFromPktHeader()
static inline bool IsClientLower(const sfaddr_t *cltIP, uint16_t cltPort,
                                 const sfaddr_t *srvIP, uint16_t srvPort, char proto)
{
    if (sfip_fast_lt6(cltIP, srvIP))
        return true;

    if (sfip_fast_lt6(srvIP, cltIP))
        return false;

    switch (proto)
    {
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            if (cltPort < srvPort)
                return true;
    }
    return false;
}

static SessionControlBlock *DeserializeHASession(const SessionKey *key,
                                                 const StreamHASession *has,
                                                 SessionControlBlock *scb)
{
    SessionControlBlock *retSsn;
    int family;

    if (!scb)
    {
        const HA_Api *api;

        api = ha_get_api(key->protocol);
        retSsn = api->create_session(key);

        retSsn->ha_flags &= ~HA_FLAG_NEW;
        retSsn->ha_flags |= HA_FLAG_STANDBY;

        family = (has->flags & HA_SESSION_FLAG_IP6) ? AF_INET6 : AF_INET;
        if (has->flags & HA_SESSION_FLAG_LOW)
        {
            sfip_set_raw(&retSsn->server_ip, retSsn->key->ip_l, family);
            sfip_set_raw(&retSsn->client_ip, retSsn->key->ip_h, family);
            retSsn->server_port = retSsn->key->port_l;
            retSsn->client_port = retSsn->key->port_h;
        }
        else
        {
            sfip_set_raw(&retSsn->client_ip, retSsn->key->ip_l, family);
            sfip_set_raw(&retSsn->server_ip, retSsn->key->ip_h, family);
            retSsn->client_port = retSsn->key->port_l;
            retSsn->server_port = retSsn->key->port_h;
        }
    }
    else
        retSsn = scb;

    retSsn->ha_state = has->ha_state;

    return retSsn;
}

static inline int DeserializePreprocData(uint8_t event, SessionControlBlock *scb, uint8_t preproc_id,
                                         uint8_t subcode, const uint8_t *data, uint8_t length)
{
    StreamHAFuncsNode *node;
    int i;

    for (i = 0; i < n_stream_ha_funcs; i++)
    {
        node = stream_ha_funcs[i];
        if (node && preproc_id == node->preproc_id && subcode == node->subcode)
        {
            if (node->size < length)
            {
                ErrorMessage("Stream HA preprocessor data record's length exceeds expected size! (%u vs %u)\n",
                        length, node->size);
                return -1;
            }
            node->consumed++;
            return node->consume(scb, data, length);
        }
    }

    ErrorMessage("Stream HA preprocessor data record received with unrecognized preprocessor ID/subcode! (%hhu:%hhu)\n",
            preproc_id, subcode);
    return -1;
}

static int ConsumeHAMessage(const uint8_t *msg, uint32_t msglen)
{
    const HA_Api *api;
    StreamHASession *has;
    SessionControlBlock *scb;
    SessionKey key;
    MsgHeader *msg_hdr;
    RecordHeader *rec_hdr;
    PreprocDataHeader *psd_hdr;
    uint32_t offset;
    bool debug_flag;
    int rval = 1;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sessionHAConsumePerfStats);

    /* Read the message header */
    if (msglen < sizeof(*msg_hdr))
    {
        ErrorMessage("Stream HA message length shorter than header length! (%u)\n", msglen);
        goto consume_exit;
    }
    msg_hdr = (MsgHeader *) msg;
    offset = sizeof(*msg_hdr);

    if (msg_hdr->total_length != msglen)
    {
        ErrorMessage("Stream HA message header's total length does not match actual length! (%u vs %u)\n",
                msg_hdr->total_length, msglen);
        goto consume_exit;
    }

    if (msg_hdr->version != HA_MESSAGE_VERSION)
    {
        ErrorMessage("Stream HA message has unrecognized version: %hhu!\n", msg_hdr->version);
        goto consume_exit;
    }

    if (msg_hdr->event != HA_EVENT_UPDATE && msg_hdr->event != HA_EVENT_DELETE)
    {
        ErrorMessage("Stream HA message has unknown event type: %hhu!\n", msg_hdr->event);
        goto consume_exit;
    }

    /* Read the key */
    if (msg_hdr->key_size == IP4_SESSION_KEY_SIZE) /* IPv4, miniature key */
    {
        /* Lower IPv4 address */
        memcpy(&key.ip_l[3], msg + offset, 4);
        key.ip_l[0] = key.ip_l[1] = 0;
        key.ip_l[2] = htonl(0xFFFF);
        offset += 4;
        /* Higher IPv4 address */
        memcpy(&key.ip_h[3], msg + offset, 4);
        key.ip_h[0] = key.ip_h[1] = 0;
        key.ip_h[2] = htonl(0xFFFF);
        offset += 4;
        /* The remainder of the key */
        memcpy(((uint8_t *) &key) + 32, msg + offset, IP4_SESSION_KEY_SIZE - 8);
        offset += IP4_SESSION_KEY_SIZE - 8;
    }
    else if (msg_hdr->key_size == IP6_SESSION_KEY_SIZE) /* IPv6, full-size key */
    {
        memcpy(&key, msg + offset, IP6_SESSION_KEY_SIZE);
        offset += IP6_SESSION_KEY_SIZE;
    }
    else
    {
        ErrorMessage("Stream HA message has unrecognized key size: %hhu!\n", msg_hdr->key_size);
        goto consume_exit;
    }

    debug_flag = StreamHADebugCheck(&key, s5_ha_debug_flag, &s5_ha_debug_info, s5_ha_debug_session, sizeof(s5_ha_debug_session));

    api = ha_get_api(key.protocol);
    if (!api)
    {
        ErrorMessage("Stream HA message has unhandled protocol: %u!\n", key.protocol);
        goto consume_exit;
    }

    if (msg_hdr->event == HA_EVENT_DELETE)
    {
        if (debug_flag)
            LogMessage("S5HADbg Consuming deletion message for %s\n", s5_ha_debug_session);
        if (offset != msglen)
        {
            ErrorMessage("Stream HA deletion message contains extraneous data! (%u bytes)\n", msglen - offset);
            goto consume_exit;
        }
        s5ha_stats.delete_messages_received++;
        rval = api->delete_session(&key);
        if (debug_flag)
        {
            if (!rval)
                LogMessage("S5HADbg Deleted LWSession for %s\n", s5_ha_debug_session);
            else
                LogMessage("S5HADbg Could not delete LWSession for %s\n", s5_ha_debug_session);
        }
        goto consume_exit;
    }

    if (debug_flag)
        LogMessage("S5HADbg Consuming update message for %s\n", s5_ha_debug_session);

    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_NO_BLOCK, snprintf(trace_line, MAX_TRACE_LINE,
                     "Recovered session \n"));

    scb = api->get_lws(&key);

    /* Read any/all records. */
    while (offset < msglen)
    {
        if (sizeof(*rec_hdr) > (msglen - offset))
        {
            ErrorMessage("Stream HA message contains a truncated record header! (%zu vs %u)\n",
                    sizeof(*rec_hdr), msglen - offset);
            goto consume_exit;
        }
        rec_hdr = (RecordHeader *) (msg + offset);
        offset += sizeof(*rec_hdr);

        switch (rec_hdr->type)
        {
            case HA_TYPE_HAS:
                if (rec_hdr->length != sizeof(*has))
                {
                    ErrorMessage("Stream HA message contains incorrectly size HA Session record! (%u vs %zu)\n",
                            rec_hdr->length, sizeof(*has));
                    goto consume_exit;
                }
                if (rec_hdr->length > (msglen - offset))
                {
                    ErrorMessage("Stream HA message contains truncated HA Session record data! (%u vs %u)\n",
                            rec_hdr->length, msglen - offset);
                    goto consume_exit;
                }
                has = (StreamHASession *) (msg + offset);
                offset += rec_hdr->length;
                if (debug_flag)
                {
#ifdef TARGET_BASED
                    LogMessage("S5HADbg %s Session for %s - SF=0x%x IPP=0x%hx AP=0x%hx DIR=%hhu IDIR=%hhu\n",
                                (scb) ? "Updating" : "Creating", s5_ha_debug_session, has->ha_state.session_flags,
                                has->ha_state.ipprotocol, has->ha_state.application_protocol,
                                has->ha_state.direction, has->ha_state.ignore_direction);
#else
                    LogMessage("S5HADbg %s LWSession for %s - SF=0x%x DIR=%hhu IDIR=%hhu\n",
                                (lwssn) ? "Updating" : "Creating", s5_ha_debug_session, has->ha_state.session_flags,
                                has->ha_state.direction, has->ha_state.ignore_direction);
#endif
                }
                if(!scb)
                {
                    if (has->ha_state.session_flags & SSNFLAG_COUNTED_CLOSING)
                        sfBase.iSessionsClosing++;
                    else if (has->ha_state.session_flags & SSNFLAG_COUNTED_ESTABLISH)
                        sfBase.iSessionsEstablished++;
                    else if (has->ha_state.session_flags & SSNFLAG_COUNTED_INITIALIZE)
                        sfBase.iSessionsInitializing++;
                }
                scb = DeserializeHASession(&key, has, scb);
                scb->session_established = true;
                break;

            case HA_TYPE_PSD:
                if (!scb)
                {
                    //ErrorMessage("Stream HA message with preprocessor data record received for non-existent session!\n");
                    s5ha_stats.update_messages_received_no_session++;
                    goto consume_exit;
                }
                if (sizeof(*psd_hdr) > (msglen - offset))
                {
                    ErrorMessage("Stream HA message contains a truncated preprocessor data record header! (%zu vs %u)\n",
                            sizeof(*psd_hdr), msglen - offset);
                    goto consume_exit;
                }
                psd_hdr = (PreprocDataHeader *) (msg + offset);
                offset += sizeof(*psd_hdr);
                if (rec_hdr->length > (msglen - offset))
                {
                    ErrorMessage("Stream HA message contains truncated preprocessor data record data! (%u vs %u)\n",
                            rec_hdr->length, msglen - offset);
                    goto consume_exit;
                }
                if (debug_flag)
                {
                    LogMessage("S5HADbg Consuming %hhu byte preprocessor data record for %s with PPID=%hhu and SC=%hhu\n",
                                rec_hdr->length, s5_ha_debug_session, psd_hdr->preproc_id, psd_hdr->subcode);
                }
                if (DeserializePreprocData(msg_hdr->event, scb, psd_hdr->preproc_id, psd_hdr->subcode,
                                            msg + offset, rec_hdr->length) != 0)
                {
                    ErrorMessage("Stream HA message contained invalid preprocessor data record!\n");
                    goto consume_exit;
                }
                offset += rec_hdr->length;
                break;

            default:
                ErrorMessage("Stream HA message contains unrecognized record type: %hhu!\n", rec_hdr->type);
                goto consume_exit;
        }
    }
    /* Mark the session as being in standby mode since we just received an update. */
    if (scb && !(scb->ha_flags & HA_FLAG_STANDBY))
    {
        if (api->deactivate_session)
            api->deactivate_session(scb);
        scb->ha_flags |= HA_FLAG_STANDBY;
    }

    s5ha_stats.update_messages_received++;
    rval = 0;

consume_exit:
    PREPROC_PROFILE_END(sessionHAConsumePerfStats);
    return rval;
}

/*
 * File I/O
 */
static inline ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t n;
    errno = 0;

    while ((n = read(fd, buf, count)) <= (ssize_t) count)
    {
        if (n == (ssize_t) count)
            return 0;

        if (n > 0)
        {
            buf = (uint8_t *) buf + n;
            count -= n;
        }
        else if (n == 0)
            break;
        else if (errno != EINTR)
        {
            ErrorMessage("Error reading from Stream HA message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }
    return -1;
}

static int ReadHAMessagesFromFile(const char *filename)
{
    MsgHeader *msg_header;
    uint8_t *msg;
    int rval, fd;

    fd = open(filename, O_RDONLY, 0664);
    if (fd < 0)
    {
        FatalError("Could not open %s for reading HA messages from: %s (%d)\n", filename, strerror(errno), errno);
    }

    LogMessage("Reading Stream HA messages from '%s'...\n", filename);
    msg = file_io_buffer;
    while ((rval = Read(fd, msg, sizeof(*msg_header))) == 0)
    {
        msg_header = (MsgHeader *) msg;
        if (msg_header->total_length < sizeof(*msg_header))
        {
            ErrorMessage("Stream HA Message total length (%hu) is way too short!\n", msg_header->total_length);
            close(fd);
            return -1;
        }
        else if (msg_header->total_length > (UINT16_MAX - sizeof(*msg_header)))
        {
            ErrorMessage("Stream HA Message total length (%hu) is too long!\n", msg_header->total_length);
            close(fd);
            return -1;
        }
        else if (msg_header->total_length > sizeof(*msg_header))
        {
        	if ((rval = Read(fd, msg + sizeof(*msg_header), msg_header->total_length - sizeof(*msg_header))) != 0)
        	{
            	ErrorMessage("Error reading the remaining %zu bytes of an HA message from file: %s (%d)\n",
                    msg_header->total_length - sizeof(*msg_header), strerror(errno), errno);
            	close(fd);
            	return rval;
            }
        }

        if ((rval = ConsumeHAMessage(msg, msg_header->total_length)) != 0)
        {
            close(fd);
            return rval;
        }
    }
    close(fd);

    return 0;
}

static inline ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t n;
    errno = 0;

    while ((n = write(fd, buf, count)) <= (ssize_t) count)
    {
        if (n == (ssize_t) count)
            return 0;

        if (n > 0)
            count -= n;
        else if (errno != EINTR)
        {
            ErrorMessage("Error writing to Stream HA message file: %s (%d)\n", strerror(errno), errno);
            break;
        }
    }

    return -1;
}

static uint32_t WriteHAMessageHeader(uint8_t event, uint16_t msglen, const SessionKey *key, uint8_t *msg)
{
    MsgHeader *msg_hdr;
    uint32_t offset;

    msg_hdr = (MsgHeader *) msg;
    offset = sizeof(*msg_hdr);
    msg_hdr->event = event;
    msg_hdr->version = HA_MESSAGE_VERSION;
    msg_hdr->total_length = msglen;
    msg_hdr->key_type = HA_TYPE_KEY;

    if (key->ip_l[0] || key->ip_l[1] || key->ip_l[2] != htonl(0xFFFF) ||
        key->ip_h[0] || key->ip_h[1] || key->ip_h[2] != htonl(0xFFFF))
    {
        msg_hdr->key_size = IP6_SESSION_KEY_SIZE;
        memcpy(msg + offset, key, IP6_SESSION_KEY_SIZE);
        offset += IP6_SESSION_KEY_SIZE;
    }
    else
    {
        msg_hdr->key_size = IP4_SESSION_KEY_SIZE;
        memcpy(msg + offset, &key->ip_l[3], sizeof(key->ip_l[3]));
        offset += sizeof(key->ip_l[3]);
        memcpy(msg + offset, &key->ip_h[3], sizeof(key->ip_h[3]));
        offset += sizeof(key->ip_h[3]);
        memcpy(msg + offset, ((uint8_t *) key) + 32, IP4_SESSION_KEY_SIZE - 8);
        offset += IP4_SESSION_KEY_SIZE - 8;
    }
    return offset;
}

static void UpdateHAMessageHeaderLength(uint8_t *msg, uint16_t msglen)
{
    MsgHeader *msg_hdr;

    msg_hdr = (MsgHeader *) msg;
    msg_hdr->total_length = msglen;
}

static uint32_t WriteHASession(SessionControlBlock *scb, uint8_t *msg)
{
    StreamHASession *has;
    RecordHeader *rec_hdr;
    uint32_t offset;

    rec_hdr = (RecordHeader *) msg;
    offset = sizeof(*rec_hdr);
    rec_hdr->type = HA_TYPE_HAS;
    rec_hdr->length = sizeof(*has);

    has = (StreamHASession *) (msg + offset);
    offset += sizeof(*has);
    has->ha_state = scb->ha_state;

    if (!IsClientLower(&scb->client_ip, scb->client_port, &scb->server_ip, scb->server_port, scb->key->protocol))
        has->flags |= HA_SESSION_FLAG_LOW;

    has->flags |= HA_SESSION_FLAG_IP6;

    return offset;
}

static uint32_t WritePreprocDataRecord(SessionControlBlock *scb, StreamHAFuncsNode *node, uint8_t *msg, bool forced)
{
    RecordHeader *rec_hdr;
    PreprocDataHeader *psd_hdr;
    uint32_t offset;

    rec_hdr = (RecordHeader *) msg;
    offset = sizeof(*rec_hdr);
    rec_hdr->type = HA_TYPE_PSD;

    psd_hdr = (PreprocDataHeader *) (msg + offset);
    offset += sizeof(*psd_hdr);
    psd_hdr->preproc_id = node->preproc_id;
    psd_hdr->subcode = node->subcode;

    rec_hdr->length = node->produce(scb, msg + offset);
    /* If this was a forced record generation (the preprocessor did not indicate pending HA state data), as in the case
        of a full HA record generation for session pickling, return a 0 offset if there is no data so that space is not
        wasted on the PSD header. */
    if (rec_hdr->length == 0 && forced)
        return 0;
    offset += rec_hdr->length;
    node->produced++;

    return offset;
}

static uint32_t CalculateHAMessageSize(uint8_t event, SessionControlBlock *scb)
{
    StreamHAFuncsNode *node;
    SessionKey *key;
    uint32_t msg_size;
    int idx;

    key = scb->key;

    /* Header (including the key).  IPv4 keys are miniaturized. */
    msg_size = sizeof(MsgHeader);
    if (key->ip_l[0] || key->ip_l[1] || key->ip_l[2] != htonl(0xFFFF) ||
        key->ip_h[0] || key->ip_h[1] || key->ip_h[2] != htonl(0xFFFF))
        msg_size += IP6_SESSION_KEY_SIZE;
    else
        msg_size += IP4_SESSION_KEY_SIZE;

    if (event == HA_EVENT_UPDATE)
    {
        /* HA Session record */
        //if (scb->ha_flags & HA_FLAG_MODIFIED)
            msg_size += sizeof(RecordHeader) + sizeof(StreamHASession);

        /* Preprocessor data records */
        for (idx = 0; idx < n_stream_ha_funcs; idx++)
        {
            if (scb->ha_pending_mask & (1 << idx))
            {
                node = stream_ha_funcs[idx];
                if (!node)
                    continue;
                msg_size += sizeof(RecordHeader) + sizeof(PreprocDataHeader) + node->size;
            }
        }
    }

#ifdef DEBUG_HA_PRINT
    printf("Calculated msg_size = %u (%zu, %zu, %zu, %zu, %zu, %zu, %zu)\n", msg_size, IP4_SESSION_KEY_SIZE, IP6_SESSION_KEY_SIZE,
            sizeof(MsgHeader), sizeof(RecordHeader), sizeof(StreamHASession), sizeof(StreamHAState), sizeof(bool));
#endif
    return msg_size;
}

static uint32_t GenerateHADeletionMessage(uint8_t *msg, uint32_t msg_size, SessionControlBlock *scb)
{
    uint32_t msglen;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sessionHAProducePerfStats);

    msglen = WriteHAMessageHeader(HA_EVENT_DELETE, msg_size, scb->key, msg);

    PREPROC_PROFILE_END(sessionHAProducePerfStats);

    return msglen;
}

#ifdef SIDE_CHANNEL
static void SendSCDeletionMessage(SessionControlBlock *scb, uint32_t msg_size)
{
    SCMsgHdr *sc_hdr;
    void *msg_handle;
    uint8_t *msg;
    int rval;

    /* Allocate space for the message. */
    if ((rval = SideChannelPreallocMessageTX(msg_size, &sc_hdr, &msg, &msg_handle)) != 0)
    {
        /* TODO: Error stuff goes here. */
        return;
    }

    /* Generate the message. */
    msg_size = GenerateHADeletionMessage(msg, msg_size, scb);

    /* Send the message. */
    sc_hdr->type = SC_MSG_TYPE_FLOW_STATE_TRACKING;
    sc_hdr->timestamp = packet_time();
    SideChannelEnqueueMessageTX(sc_hdr, msg, msg_size, msg_handle, NULL);
}
#endif

void SessionHANotifyDeletion(SessionControlBlock *scb)
{
    uint32_t msg_size;
    bool debug_flag;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sessionHAPerfStats);

    if ( !scb )
    {
        PREPROC_PROFILE_END(sessionHAPerfStats);
        return;
    }

    if ( !session_configuration->enable_ha )
    {
        PREPROC_PROFILE_END(sessionHAPerfStats);
        return;
    }

    /* Don't send a deletion notice if we've never sent an update for the flow, it is in standby, or we've already sent one. */
    if ( scb->ha_flags & ( HA_FLAG_NEW | HA_FLAG_STANDBY | HA_FLAG_DELETED ) )
    {
        s5ha_stats.delete_messages_not_sent++;
        PREPROC_PROFILE_END(sessionHAPerfStats);
        return;
    }

#ifdef SIDE_CHANNEL
    if (session_configuration->ha_config->use_side_channel)
    {
        debug_flag = StreamHADebugCheck(scb->key, s5_ha_debug_flag, &s5_ha_debug_info,
                                     s5_ha_debug_session, sizeof(s5_ha_debug_session));
        if (debug_flag)
            LogMessage("S5HADbg Producing deletion message for %s\n",
                       s5_ha_debug_session);
    }
#endif

    /* Calculate the size of the deletion message. */
    msg_size = CalculateHAMessageSize(HA_EVENT_DELETE, scb);

    if (runtime_output_fd >= 0)
    {
        msg_size = GenerateHADeletionMessage(file_io_buffer, msg_size, scb);
        if (Write(runtime_output_fd, file_io_buffer, msg_size) == -1)
        {
            /* TODO: Error stuff here. */
        }
    }

    if (session_configuration->ha_config->use_daq)
    {
        s5ha_stats.delete_messages_not_sent++;
        PREPROC_PROFILE_END(sessionHAPerfStats);
        return;
    }

#ifdef SIDE_CHANNEL
    if (session_configuration->ha_config->use_side_channel)
    {
        SendSCDeletionMessage(scb, msg_size);
    }
#endif
  
    s5ha_stats.delete_messages_sent++;
    scb->ha_flags |= HA_FLAG_DELETED;

    PREPROC_PROFILE_END(sessionHAPerfStats);
}

static uint32_t GenerateHAUpdateMessage(uint8_t *msg, SessionControlBlock *scb, bool full)
{
    StreamHAFuncsNode *node;
    uint32_t offset;
    int idx;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sessionHAProducePerfStats);

    /* Write HA message header with a length of 0.  It will be updated at the end. */
    offset = WriteHAMessageHeader(HA_EVENT_UPDATE, 0, scb->key, msg);
    offset += WriteHASession(scb, msg + offset);
    for (idx = 0; idx < n_stream_ha_funcs; idx++)
    {
        /* If this is the generation of a full message, try to generate state from all registered nodes. */
        if ((scb->ha_pending_mask & (1 << idx)) || full)
        {
            node = stream_ha_funcs[idx];
            if (!node)
                continue;
            offset += WritePreprocDataRecord(scb, node, msg + offset, (scb->ha_pending_mask & (1 << idx)) ? false : true);
        }
    }
    /* Update the message header's length field with the final message size. */
    UpdateHAMessageHeaderLength(msg, offset);

    PREPROC_PROFILE_END(sessionHAProducePerfStats);

#ifdef DEBUG_HA_PRINT
    printf("Generated msg_size = %u (%zu, %zu, %zu, %zu, %zu, %zu, %zu)\n", offset, IP4_SESSION_KEY_SIZE, IP6_SESSION_KEY_SIZE,
            sizeof(MsgHeader), sizeof(RecordHeader), sizeof(StreamHASession), sizeof(StreamHAState), sizeof(bool));
#endif
    return offset;
}

#ifdef SIDE_CHANNEL
static void SendSCUpdateMessage(SessionControlBlock *scb)
{
    SCMsgHdr *schdr;
    void *msg_handle;
    uint32_t msg_size;
    uint8_t *msg;
    int rval;

    /* Calculate the maximum size of the update message for preallocation. */
    msg_size = CalculateHAMessageSize(HA_EVENT_UPDATE, scb);

    /* Allocate space for the message. */
    if ((rval = SideChannelPreallocMessageTX(msg_size, &schdr, &msg, &msg_handle)) != 0)
    {
        /* TODO: Error stuff goes here. */
        return;
    }

    /* Gnerate the message. */
    msg_size = GenerateHAUpdateMessage(msg, scb, false);

    /* Send the message. */
    schdr->type = SC_MSG_TYPE_FLOW_STATE_TRACKING;
    schdr->timestamp = packet_time();
    SideChannelEnqueueMessageTX(schdr, msg, msg_size, msg_handle, NULL);
}
#endif

static inline uint8_t getHaStateDiff(SessionKey *key, const StreamHAState *old_state, StreamHAState *cur_state, bool new_session)
{
    uint32_t session_flags_diff;
    uint8_t ha_flags = 0;

    /* Session creation for non-TCP sessions is a major change.  TCP sessions
     * hold off until they are established. */
    if (new_session)
    {
        ha_flags |= HA_FLAG_MODIFIED;
        if (key->protocol != IPPROTO_TCP)
            ha_flags |= HA_FLAG_MAJOR_CHANGE;
        return ha_flags;
    }

    session_flags_diff = ( old_state->session_flags ^ cur_state->session_flags ) & ~HA_IGNORED_SESSION_FLAGS;
    if( session_flags_diff )
    {
        ha_flags |= HA_FLAG_MODIFIED;
        if( key->protocol == IPPROTO_TCP && ( session_flags_diff & HA_TCP_MAJOR_SESSION_FLAGS ) )
            ha_flags |= HA_FLAG_MAJOR_CHANGE;
        if( session_flags_diff & HA_CRITICAL_SESSION_FLAGS )
            ha_flags |= HA_FLAG_CRITICAL_CHANGE;
    }

    if( old_state->ignore_direction != cur_state->ignore_direction )
    {
        ha_flags |= HA_FLAG_MODIFIED;
        /* If we have started ignoring both directions, that means we'll probably
         * try to whitelist the session.  This is a critical change since we
         * probably won't see another packet on the session if we're using
         * a DAQ module that fully supports the WHITELIST verdict. */
        if( cur_state->ignore_direction == SSN_DIR_BOTH )
            ha_flags |= HA_FLAG_CRITICAL_CHANGE;
    }

#ifdef TARGET_BASED
    if( ( old_state->ipprotocol != cur_state->ipprotocol ) ||
        ( old_state->application_protocol != cur_state->application_protocol ) ||
        ( old_state->direction != cur_state->direction ) )
#else
    if( old_state->direction != cur_state->direction )
#endif
    {
        ha_flags |= HA_FLAG_MODIFIED;
    }

    return ha_flags;
}

void SessionProcessHA(void *ssnptr, const DAQ_PktHdr_t *pkthdr)
{
    struct timeval pkt_time;
    SessionControlBlock *scb = (SessionControlBlock *) ssnptr;
    uint32_t msg_size;
    bool debug_flag;
    PROFILE_VARS;

    PREPROC_PROFILE_START(sessionHAPerfStats);

    if (!scb || !session_configuration->enable_ha)
    {
        PREPROC_PROFILE_END(sessionHAPerfStats);
        return;
    }

    scb->ha_flags |= getHaStateDiff(scb->key, &scb->cached_ha_state, &scb->ha_state, scb->new_session);
    /*  Receiving traffic on a session that's in standby is a major change. */
    if (scb->ha_flags & HA_FLAG_STANDBY)
    {
        scb->ha_flags |= HA_FLAG_MODIFIED | HA_FLAG_MAJOR_CHANGE;
        scb->ha_flags &= ~HA_FLAG_STANDBY;
    }
    scb->new_session = false;

    if (!scb->ha_pending_mask && !(scb->ha_flags & HA_FLAG_MODIFIED))
    {
        PREPROC_PROFILE_END( sessionHAPerfStats );
        return;
    }

    /*
       For now, we are only generating messages for:
        (a) major and critical changes or
        (b) preprocessor changes on already synchronized sessions.
    */
    if (!(scb->ha_flags & (HA_FLAG_MAJOR_CHANGE | HA_FLAG_CRITICAL_CHANGE)) &&
         (!scb->ha_pending_mask || (scb->ha_flags & HA_FLAG_NEW)))
    {
        PREPROC_PROFILE_END(sessionHAPerfStats);
        return;
    }

    /* Ensure that a new flow has lived long enough for anyone to care about it
        and that we're not overrunning the synchronization threshold. */
    packet_gettimeofday(&pkt_time);
    if ((pkt_time.tv_sec < scb->ha_next_update.tv_sec) ||
        ((pkt_time.tv_sec == scb->ha_next_update.tv_sec) &&
          (pkt_time.tv_usec < scb->ha_next_update.tv_usec)))
    {
        /* Critical changes will be allowed to bypass the message timing restrictions. */
        if (!(scb->ha_flags & HA_FLAG_CRITICAL_CHANGE))
        {
            PREPROC_PROFILE_END( sessionHAPerfStats );
            return;
        }
        s5ha_stats.update_messages_sent_immediately++;
    }
    else
        s5ha_stats.update_messages_sent_normally++;

    debug_flag = StreamHADebugCheck(scb->key, s5_ha_debug_flag, &s5_ha_debug_info,
                                      s5_ha_debug_session, sizeof(s5_ha_debug_session));
    if (debug_flag)
#ifdef TARGET_BASED
        LogMessage("S5HADbg Producing update message for %s - "
                    "SF=0x%x IPP=0x%hx AP=0x%hx DIR=%hhu IDIR=%hhu HPM=0x%hhx HF=0x%hhx\n",
                    s5_ha_debug_session, scb->ha_state.session_flags, scb->ha_state.ipprotocol,
                    scb->ha_state.application_protocol, scb->ha_state.direction,
                    scb->ha_state.ignore_direction, scb->ha_pending_mask, scb->ha_flags);
#else
        LogMessage("S5HADbg Producing update message for %s - SF=0x%x DIR=%hhu IDIR=%hhu HPM=0x%hhx HF=0x%hhx\n",
                    s5_ha_debug_session, scb->ha_state.session_flags,
                    scb->ha_state.direction, scb->ha_state.ignore_direction,
                    scb->ha_pending_mask, scb->ha_flags);
#endif

    /* Write out to the runtime output file. */
    if (runtime_output_fd >= 0)
    {
        /* Generate the incremental update message. */
        msg_size = GenerateHAUpdateMessage(file_io_buffer, scb, false);
        if (Write(runtime_output_fd, file_io_buffer, msg_size) == -1)
        {
            /* TODO: Error stuff here. */
            WarningMessage("(%s)(%d) Error writing HA message not handled\n", __FILE__, __LINE__);
        }
    }

#ifdef HAVE_DAQ_EXT_MODFLOW
    /*
        Store via DAQ module (requires full message generation).
        Assume that if we are not setting binding DAQ verdicts because of external encapsulation-induced
          confusion that we also cannot safely set the HA state associated with this flow in the DAQ.
    */
    if (session_configuration->ha_config->use_daq && !Active_GetTunnelBypass())
    {
        /* Generate the full message. */
        msg_size = GenerateHAUpdateMessage(file_io_buffer, scb, true);
        DAQ_ModifyFlowHAState(pkthdr, file_io_buffer, msg_size);
    }
#endif

#ifdef SIDE_CHANNEL
    /* Send an update message over the side channel. */
    if (session_configuration->ha_config->use_side_channel)
    {
        SendSCUpdateMessage(scb);
    }
#endif

    /* Calculate the next update threshold. */
    scb->ha_next_update.tv_usec += session_configuration->ha_config->min_session_lifetime.tv_usec;
    if (scb->ha_next_update.tv_usec > 1000000)
    {
        scb->ha_next_update.tv_usec -= 1000000;
        scb->ha_next_update.tv_sec++;
    }
    scb->ha_next_update.tv_sec += session_configuration->ha_config->min_session_lifetime.tv_sec;

    /* Clear the modified/new flags and pending preprocessor updates. */
    scb->ha_flags &= ~(HA_FLAG_NEW | HA_FLAG_MODIFIED | HA_FLAG_MAJOR_CHANGE | HA_FLAG_CRITICAL_CHANGE);
    scb->ha_pending_mask = 0;

    PREPROC_PROFILE_END(sessionHAPerfStats);
}

#ifdef SIDE_CHANNEL
static int StreamHASCMsgHandler( SCMsgHdr *hdr, const uint8_t *msg, uint32_t msglen )
{
    int rval;
    PROFILE_VARS;

    PREPROC_PROFILE_START( sessionHAPerfStats );

    rval = ConsumeHAMessage( msg, msglen );

    PREPROC_PROFILE_END( sessionHAPerfStats );

    return rval;
}
#endif

void SessionHAPostConfigInit( struct _SnortConfig *sc, int unused, void *arg )
{
    int rval;

    if( !session_configuration->enable_ha )
        return;

    if( session_configuration->ha_config->startup_input_file )
    {
        rval = ReadHAMessagesFromFile( session_configuration->ha_config->startup_input_file);
        if( rval != 0 )
        {
            ErrorMessage( "Errors were encountered while reading HA messages from file!" );
        }
    }

    if( session_configuration->ha_config->runtime_output_file )
    {
        runtime_output_fd = open( session_configuration->ha_config->runtime_output_file,
                                 O_WRONLY | O_CREAT | O_TRUNC, 0664 );
        if( runtime_output_fd < 0 )
        {
            FatalError( "Could not open %s for writing HA messages to: %s (%d)\n",
                        session_configuration->ha_config->runtime_output_file, strerror( errno ), errno );
        }
    }

#ifdef REG_TEST
    if( session_configuration->ha_config->runtime_input_file )
    {
        runtime_input_fd = open( session_configuration->ha_config->runtime_input_file,
                                  O_RDONLY, 0664 );
        if( runtime_input_fd < 0 )
        {
            FatalError( "Could not open %s for writing HA messages to: %s (%d)\n",
                        session_configuration->ha_config->runtime_input_file, strerror( errno ), errno );
        }
    }
#endif

#ifdef SIDE_CHANNEL
    if( session_configuration->ha_config->use_side_channel )
    {
        rval = SideChannelRegisterRXHandler( SC_MSG_TYPE_FLOW_STATE_TRACKING, StreamHASCMsgHandler, NULL );
        if( rval != 0 )
        {
            /* TODO: Fatal error here or something. */
            ErrorMessage( "(%s)(%d) Errors were encountered registering Rx Side Channel Handler\n",
                    __FILE__, __LINE__ );
         }
    }
#endif
}

void SessionCleanHA( void )
{
    int i;

    for (i = 0; i < n_stream_ha_funcs; i++)
    {
        if (stream_ha_funcs[i])
        {
            free(stream_ha_funcs[i]);
            stream_ha_funcs[i] = NULL;
        }
    }
    if (runtime_output_fd >= 0)
    {
        close(runtime_output_fd);
        runtime_output_fd = -1;
    }
#ifdef REG_TEST
    if (runtime_input_fd >= 0)
    {
        close(runtime_input_fd);
        runtime_input_fd = -1;
    }
#endif

}
