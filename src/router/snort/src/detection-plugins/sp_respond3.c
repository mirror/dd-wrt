/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License Version 2 as published by
 * the Free Software Foundation.  You may not use, modify or distribute this
 * program under any other version of the GNU General Public License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 ****************************************************************************/
/* Snort sp_resp3 Detection Plugin
 *
 * Perform flexible response on packets matching conditions specified in Snort
 * rules.
 *
 * Shutdown hostile network connections by injecting TCP resets or ICMP
 * unreachable packets.
 *
 * flexresp3 is derived from flexresp and flexresp2.  It includes all
 * configuration options from those modules and has these differences:
 *
 * - injects packets with correct encapsulations (doesn't assume
 * eth+ip+icmp/tcp).
 *
 * - uses the wire packet as a prototype, not the packet generating the alert
 * (which may be reassembled or otherwise generated internally with only the
 * headers required for logging).
 *
 * - queues the injection action so that it is taken only once after detection
 * regardless of multiple resp3 rules firing.
 *
 * - uses the same encoding and injection mechanism as active_response and/or
 * reject actions.
 *
 * - bypasses sequence strafing in inline mode.
 *
 * - if a resp3 rule is also a drop rule, the drop processing takes precedence.
 */

// @file    sp_respond3.c
// @author  Russ Combs <rcombs@sourcefire.com>

#ifdef ENABLE_RESPONSE3

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_bounds.h"
#include "checksum.h"
#include "snort_debug.h"
#include "decode.h"
#include "encode.h"
#include "detection_options.h"
#include "log.h"
#include "mstring.h"
#include "parser.h"
#include "plugbase.h"
#include "plugin_enum.h"
#include "profiler.h"
#include "active.h"
#include "rules.h"
#include "sfhashfcn.h"
#include "sfxhash.h"
#include "snort.h"
#include "sp_respond.h"
#include "util.h"

#define MOD_NAME "sp_resp3"     /* plugin name */

#define RESP_RST_SND  0x01
#define RESP_RST_RCV  0x02
#define RESP_UNR_NET  0x04
#define RESP_UNR_HOST 0x08
#define RESP_UNR_PORT 0x10

#define RESP_RST (RESP_RST_SND|RESP_RST_RCV)
#define RESP_UNR (RESP_UNR_NET|RESP_UNR_HOST|RESP_UNR_PORT)

#ifdef PERF_PROFILING
static PreprocStats resp3PerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

// instance data
typedef struct {
    uint32_t mask;
    uint32_t flags;
} Resp3_Data;

static int s_init = 1;

// callback functions
static void Resp3_Init(struct _SnortConfig *, char* data, OptTreeNode*, int protocol);
static void Resp3_Cleanup(int signal, void* data);

// core functions
static int Resp3_Parse(char* type);
static int Resp3_Queue(Packet*, void*);
static void Resp3_Send(Packet*, void*);

//--------------------------------------------------------------------
// public functions
// here we use the non '_' versions for consistency ...
uint32_t RespondHash(void* d)
{
    uint32_t a,b,c;
    Resp3_Data* data = (Resp3_Data*)d;

    a = data->mask;
    b = RULE_OPTION_TYPE_RESPOND;
    c = 0;

    final(a,b,c);

    return c;
}

int RespondCompare(void* l, void* r)
{
    Resp3_Data* left = (Resp3_Data*)l;
    Resp3_Data* right = (Resp3_Data*)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->mask == right->mask)
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

// and here we use the functional name for consistency ...
void SetupRespond(void)
{
    RegisterRuleOption("resp", Resp3_Init, NULL, OPT_TYPE_ACTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("resp3", &resp3PerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
}

//--------------------------------------------------------------------
// callback functions

static void Resp3_Init(struct _SnortConfig *sc, char* data, OptTreeNode* otn, int protocol)
{
    Resp3_Data* rd = NULL;
    void* idx_dup;

    if ( otn->ds_list[PLUGIN_RESPONSE] )
        FatalError("%s(%d): Multiple response options in rule\n",
            file_name, file_line);

    if ( s_init )
    {
        AddFuncToCleanExitList(Resp3_Cleanup, NULL);

        Active_SetEnabled(1);
        s_init = 0;
    }

    rd = (Resp3_Data*)SnortAlloc(sizeof(*rd));
    rd->mask = Resp3_Parse(data);

    if ( add_detection_option(sc, RULE_OPTION_TYPE_RESPOND, rd, &idx_dup)
        == DETECTION_OPTION_EQUAL)
    {
        free(rd);
        rd = idx_dup;
    }
    // this prevents multiple response options in rule
    otn->ds_list[PLUGIN_RESPONSE] = rd;
    AddRspFuncToList(Resp3_Queue, otn, rd);
}

static void Resp3_Cleanup(int signal, void* data)
{
    s_init = 1;
}

//--------------------------------------------------------------------
// core functions
// TBD conf parsing should be registered and distributed as well as rule
// option parsing

static int Resp3_Parse(char* type)
{
    char* *toks;
    uint32_t flags = 0;
    int num_toks, i;

    if ( type )
        toks = mSplit(type, ",", 6, &num_toks, 0);
    else
        FatalError("%s: %s(%d): missing resp modifier",
            MOD_NAME, file_name, file_line);

    i = 0;
    while (i < num_toks)
    {
        if ( !strcasecmp(toks[i], "reset_source") ||
             !strcasecmp(toks[i], "rst_snd") )
        {
            flags |= RESP_RST_SND;
            i++;
        }
        else if ( !strcasecmp(toks[i], "reset_dest") ||
                  !strcasecmp(toks[i], "rst_rcv") )
        {
            flags |= RESP_RST_RCV;
            i++;
        }
        else if ( !strcasecmp(toks[i], "reset_both") ||
                  !strcasecmp(toks[i], "rst_all") )
        {
            flags |= (RESP_RST_RCV | RESP_RST_SND);
            i++;
        }
        else if (!strcasecmp(toks[i], "icmp_net"))
        {
            flags |= RESP_UNR_NET;
            i++;
        }
        else if (!strcasecmp(toks[i], "icmp_host"))
        {
            flags |= RESP_UNR_HOST;
            i++;
        }
        else if (!strcasecmp(toks[i], "icmp_port"))
        {
            flags |= RESP_UNR_PORT;
            i++;
        }
        else if (!strcasecmp(toks[i], "icmp_all"))
        {
            flags |= (RESP_UNR_NET | RESP_UNR_HOST | RESP_UNR_PORT);
            i++;
        }
        else
            FatalError("%s: %s(%d): invalid resp modifier: %s\n",
                MOD_NAME, file_name, file_line, toks[i]);
    }

    mSplitFree(&toks, num_toks);

    if ( !flags )
        FatalError("%s: %s(%d): invalid resp configuration: %s\n",
            MOD_NAME, file_name, file_line, "no response specified");

    return flags;
}

//--------------------------------------------------------------------

static int Resp3_Queue (Packet* p, void* pv)
{
    Resp3_Data* rd = (Resp3_Data*)pv;
    PROFILE_VARS;

    PREPROC_PROFILE_START(resp3PerfStats);
    rd->flags = 0;

    if ( Active_IsRSTCandidate(p) )
        rd->flags |= (rd->mask & RESP_RST);

    if ( Active_IsUNRCandidate(p) )
        rd->flags |= (rd->mask & RESP_UNR);

    if ( rd->flags )
        Active_QueueResponse(Resp3_Send, rd);

    PREPROC_PROFILE_END(resp3PerfStats);
    return 0;
}

//--------------------------------------------------------------------

static void Resp3_Send (Packet* p, void* pv)
{
    Resp3_Data* rd = (Resp3_Data*)pv;
    PROFILE_VARS;

    PREPROC_PROFILE_START(resp3PerfStats);
    Active_IgnoreSession(p);

    if ( rd->flags & RESP_RST_SND )
        Active_SendReset(p, 0);

    if ( rd->flags & RESP_RST_RCV )
        Active_SendReset(p, ENC_FLAG_FWD);

    if ( rd->flags & RESP_UNR_NET )
        Active_SendUnreach(p, ENC_UNR_NET);

    if ( rd->flags & RESP_UNR_HOST )
        Active_SendUnreach(p, ENC_UNR_HOST);

    if ( rd->flags & RESP_UNR_PORT )
        Active_SendUnreach(p, ENC_UNR_PORT);

    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_RESPONSE, snprintf(trace_line, MAX_TRACE_LINE,
            "Snort Response: ignoring session, %s\n", getPktTraceActMsg()));
    else addPktTraceData(VERDICT_REASON_RESPONSE, 0);

    PREPROC_PROFILE_END(resp3PerfStats);
}

#endif // ENABLE_RESPONSE3

