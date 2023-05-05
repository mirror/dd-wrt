/* $Id$ */
/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2010-2013 Sourcefire, Inc.
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
 */

#ifdef NORMALIZER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "active.h"
#include "mstring.h"
#include "normalize.h"
#include "parser.h"
#include "plugbase.h"
#include "profiler.h"
#include "sf_types.h"
#include "sfPolicy.h"
#include "snort.h"
#include "spp_normalize.h"
#include "snort_stream_tcp.h"

// Priority for Normalize preproc
#define PP_NORMALIZE_PRIORITY PRIORITY_CORE + PP_CORE_ORDER_NORML

static tSfPolicyUserContextId base_set = NULL;

#ifdef PERF_PROFILING
PreprocStats norm_perf_stats;
#endif

static void Preproc_Execute(Packet* , void*);
static void Preproc_CleanExit(int, void*);
static void Preproc_Reset(int, void*);
static void Preproc_PostConfigInit(struct _SnortConfig *, void*);
static int Preproc_CheckConfig (struct _SnortConfig *);
static void Preproc_ResetStats(int, void*);
static void Preproc_PrintStats(int);
static void Preproc_Install(struct _SnortConfig *);

static void Init_IP4(struct _SnortConfig *, char*);
static void Init_ICMP4(struct _SnortConfig *, char*);
static void Init_IP6(struct _SnortConfig *, char*);
static void Init_ICMP6(struct _SnortConfig *, char*);
static void Init_TCP(struct _SnortConfig *, char*);

static void Parse_IP4(struct _SnortConfig *, NormalizerContext*, char*);
static void Parse_ICMP4(NormalizerContext*, char*);
static void Parse_IP6(struct _SnortConfig *, NormalizerContext*, char*);
static void Parse_ICMP6(NormalizerContext*, char*);
static void Parse_TCP(NormalizerContext*, char*);

static void Print_IP4(struct _SnortConfig *, const NormalizerContext*);
static void Print_ICMP4(const NormalizerContext*);
static void Print_IP6(struct _SnortConfig *, const NormalizerContext*);
static void Print_ICMP6(const NormalizerContext*);
static void Print_TCP(const NormalizerContext*);

#ifdef SNORT_RELOAD
static void Reload_IP4(struct _SnortConfig *, char*, void **);
static void Reload_ICMP4(struct _SnortConfig *, char*, void **);
static void Reload_IP6(struct _SnortConfig *, char*, void **);
static void Reload_ICMP6(struct _SnortConfig *, char*, void **);
static void Reload_TCP(struct _SnortConfig *, char*, void **);

static int Reload_Verify(struct _SnortConfig *, void *);
static void* Reload_Swap(struct _SnortConfig *, void *);
static void Reload_Free(void*);
#endif

#ifdef SNORT_RELOAD
#define NORM_FUNCS(p) Init_ ## p, Reload_ ## p, Reload_Verify, Reload_Swap, Reload_Free
#else
#define NORM_FUNCS(p) Init_ ## p
#endif

void SetupNormalizer (void)
{
    RegisterPreprocessor("normalize_ip4", NORM_FUNCS(IP4));
    RegisterPreprocessor("normalize_icmp4", NORM_FUNCS(ICMP4));
    RegisterPreprocessor("normalize_ip6", NORM_FUNCS(IP6));
    RegisterPreprocessor("normalize_icmp6", NORM_FUNCS(ICMP6));
    RegisterPreprocessor("normalize_tcp", NORM_FUNCS(TCP));
}

//-------------------------------------------------------------------------
// basic initialization stuff
//-------------------------------------------------------------------------

#define PROTO_BITS (PROTO_BIT__IP|PROTO_BIT__ICMP|PROTO_BIT__TCP)

static NormalizerContext* Init_GetContext (struct _SnortConfig *sc)
{
    NormalizerContext* pc = NULL;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if ( ScNapPassiveModeNewConf(sc) )
        return NULL;

    if ( !base_set )
    {
        base_set = sfPolicyConfigCreate();
        Preproc_Install(sc);
    }

    sc->normalizer_set  = true;

    sfPolicyUserPolicySet(base_set, policy_id);
    pc = sfPolicyUserDataGetCurrent(base_set);

    if ( !pc )
    {
        pc = (NormalizerContext* )SnortAlloc(sizeof(NormalizerContext));
        sfPolicyUserDataSetCurrent(base_set, pc);
        if( pc->regFunc != reload)
        {
            AddFuncToPreprocList( sc, Preproc_Execute, PP_NORMALIZE_PRIORITY,  PP_NORMALIZE, PROTO_BITS);
            pc->regFunc = init;
        }
        session_api->enable_preproc_all_ports( sc, PP_NORMALIZE, PROTO_BITS );          
    }
    pc->normMode = ScNapInlineTestModeNewConf(sc) ? NORM_MODE_WOULDA : NORM_MODE_ON;

    return pc;
}

#define NOT_INLINE "WARNING: %s normalizations disabled because not inline.\n"

static void Init_IP4 (struct _SnortConfig *sc, char* args)
{
    NormalizerContext* pc = Init_GetContext(sc);

    if ( pc )
        Parse_IP4(sc, pc, args);
    else
        LogMessage(NOT_INLINE, "ip4");
}

static void Init_ICMP4 (struct _SnortConfig *sc, char* args)
{
    NormalizerContext* pc = Init_GetContext(sc);

    if ( pc )
        Parse_ICMP4(pc, args);
    else
        LogMessage(NOT_INLINE, "icmp4");
}

static void Init_IP6 (struct _SnortConfig *sc, char* args)
{
    NormalizerContext* pc = Init_GetContext(sc);

    if ( pc )
        Parse_IP6(sc, pc, args);
    else
        LogMessage(NOT_INLINE, "ip6");
}

static void Init_ICMP6 (struct _SnortConfig *sc, char* args)
{
    NormalizerContext* pc = Init_GetContext(sc);

    if ( pc )
        Parse_ICMP6(pc, args);
    else
        LogMessage(NOT_INLINE, "icmp6");
}

static void Init_TCP (struct _SnortConfig *sc, char* args)
{
    NormalizerContext* pc = Init_GetContext(sc);

    if ( pc )
        Parse_TCP(pc, args);
    else
        LogMessage(NOT_INLINE, "tcp");
}

//-------------------------------------------------------------------------
// parsing stuff
//-------------------------------------------------------------------------

// options may appear in any order separated by ',':
// preprocessor normalize_ip4: [id] [df] [rf] [tos] [trim]
static void Parse_IP4 (struct _SnortConfig *sc, NormalizerContext* pc, char* args)
{
    char** toks;
    int num_toks;
    int i;

    Norm_Enable(pc, NORM_IP4);

    if ( !args )
        args = "";

    toks = mSplit(args, ", ", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
#if 0
        if ( !strcasecmp(toks[i], "id") )
        {
            Norm_Enable(pc, NORM_IP4_ID);
        }
        else
#endif
        if ( !strcasecmp(toks[i], "df") )
        {
            Norm_Enable(pc, NORM_IP4_DF);
        }
        else if ( !strcasecmp(toks[i], "rf") )
        {
            Norm_Enable(pc, NORM_IP4_RF);
        }
        else if ( !strcasecmp(toks[i], "tos") )
        {
            Norm_Enable(pc, NORM_IP4_TOS);
        }
        else if ( !strcasecmp(toks[i], "trim") )
        {
            Norm_Enable(pc, NORM_IP4_TRIM);
        }
        else
        {
            ParseError("Invalid preprocessor normalize_ip4 option '%s'", toks[i]);
        }
    }
    {
        tSfPolicyId pid = getParserPolicy(sc);
        SnortPolicy* policy = sc->targeted_policies[pid];

        if ( (policy->new_ttl > 1) && (policy->new_ttl >= policy->min_ttl) )
        {
            Norm_Enable(pc, NORM_IP4_TTL);
        }
    }
    mSplitFree(&toks, num_toks);
    Print_IP4(sc, pc);
}

// preprocessor normalize_icmp4
static void Parse_ICMP4 (NormalizerContext* pc, char* args)
{
    Norm_Enable(pc, NORM_ICMP4);
    Print_ICMP4(pc);
}

// preprocessor normalize_ip6
static void Parse_IP6 (struct _SnortConfig *sc, NormalizerContext* pc, char* args)
{
    Norm_Enable(pc, NORM_IP6);
    {
        tSfPolicyId pid = getParserPolicy(sc);
        SnortPolicy* policy = sc->targeted_policies[pid];

        if ( (policy->new_ttl > 1) && (policy->new_ttl >= policy->min_ttl) )
        {
            Norm_Enable(pc, NORM_IP6_TTL);
        }
    }
    Print_IP6(sc, pc);
}

// preprocessor normalize_icmp6
static void Parse_ICMP6 (NormalizerContext* pc, char* args)
{
    Norm_Enable(pc, NORM_ICMP6);
    Print_ICMP6(pc);
}

// options may appear in any order separated by ',':
// preprocessor normalize_tcp: [ecn packet|stream] [urp] [opts] [allow <opt>+]
//
// <opt> ::= sack|echo|partial_order|alt_checksum|md5|#
// where 2 <= # <= 255.
static void Parse_TCP (NormalizerContext* pc, char* args)
{
    char **toks;
    int num_toks;
    int i, state = 0, opts = 0;

    if ( !args ) args = "";
    toks = mSplit(args, ", ", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        switch ( state ) {
        case 0:
            if ( !strcasecmp(toks[i], "ecn") )
            {
                state = 1;
            }
            else if ( !strcasecmp(toks[i], "block") )
            {
                Norm_Enable(pc, NORM_TCP_BLOCK);
            }
            else if ( !strcasecmp(toks[i], "rsv") )
            {
                Norm_Enable(pc, NORM_TCP_RSV);
            }
            else if ( !strcasecmp(toks[i], "pad") )
            {
                Norm_Enable(pc, NORM_TCP_PAD);
            }
            else if ( !strcasecmp(toks[i], "req_urg") )
            {
                Norm_Enable(pc, NORM_TCP_REQ_URG);
            }
            else if ( !strcasecmp(toks[i], "req_pay") )
            {
                Norm_Enable(pc, NORM_TCP_REQ_PAY);
            }
            else if ( !strcasecmp(toks[i], "req_urp") )
            {
                Norm_Enable(pc, NORM_TCP_URP);
            }
            else if ( !strcasecmp(toks[i], "urp") )
            {
                Norm_Enable(pc, NORM_TCP_URP);
            }
            else if ( !strcasecmp(toks[i], "opts") )
            {
                Norm_Enable(pc, NORM_TCP_OPT);
            }
            else if ( !strcasecmp(toks[i], "allow") )
            {
                state = 2;
                opts = 0;
            }
            else if ( !strcasecmp(toks[i], "ips") )
            {
                Norm_Enable(pc, NORM_TCP_IPS);
            }
            else if ( !strcasecmp(toks[i], "trim_syn") )
            {
                Norm_Enable(pc, NORM_TCP_TRIM_SYN);
            }
            else if ( !strcasecmp(toks[i], "trim_rst") )
            {
                Norm_Enable(pc, NORM_TCP_TRIM_RST);
            }
            else if ( !strcasecmp(toks[i], "trim_win") )
            {
                Norm_Enable(pc, NORM_TCP_TRIM_WIN);
            }
            else if ( !strcasecmp(toks[i], "trim_mss") )
            {
                Norm_Enable(pc, NORM_TCP_TRIM_MSS);
            }
            else if ( !strcasecmp(toks[i], "trim") )
            {
                //Catch-all / backwards compatible
                Norm_Enable(pc, NORM_TCP_TRIM_SYN);
                Norm_Enable(pc, NORM_TCP_TRIM_RST);
                Norm_Enable(pc, NORM_TCP_TRIM_WIN);
                Norm_Enable(pc, NORM_TCP_TRIM_MSS);
            }
            else
            {
                ParseError("Invalid preprocessor normalize_tcp option '%s'", toks[i]);
            }
            break;

        case 1:
            if ( !strcasecmp(toks[i], "stream") )
            {
                Norm_Enable(pc, NORM_TCP_ECN_STR);
                state = 0;
            }
            else if ( !strcasecmp(toks[i], "packet") )
            {
                Norm_Enable(pc, NORM_TCP_ECN_PKT);
                state = 0;
            }
            else
            {
                ParseError("Unknown ecn argument '%s'"
                    " Need packet|stream", toks[i]);
            }
            break;

        case 2:
            if ( !strcasecmp(toks[i], "sack") )
            {
                Norm_TcpPassOption(pc, 4);
                Norm_TcpPassOption(pc, 5);
                opts++;
            }
            else if ( !strcasecmp(toks[i], "echo") )
            {
                Norm_TcpPassOption(pc, 6);
                Norm_TcpPassOption(pc, 7);
                opts++;
            }
            else if ( !strcasecmp(toks[i], "partial_order") )
            {
                Norm_TcpPassOption(pc, 9);
                Norm_TcpPassOption(pc, 10);
                opts++;
            }
            else if ( !strcasecmp(toks[i], "conn_count") )
            {
                Norm_TcpPassOption(pc, 11);
                Norm_TcpPassOption(pc, 12);
                Norm_TcpPassOption(pc, 13);
                opts++;
            }
            else if ( !strcasecmp(toks[i], "alt_checksum") )
            {
                Norm_TcpPassOption(pc, 14);
                Norm_TcpPassOption(pc, 15);
                opts++;
            }
            else if ( !strcasecmp(toks[i], "md5") )
            {
                Norm_TcpPassOption(pc, 19);
                opts++;
            }
            else if ( isdigit(*toks[i]) )
            {
                int opt = atoi(toks[i]);
                if ( 1 < opt && opt < 256 )
                {
                    Norm_TcpPassOption(pc, (uint8_t)opt);
                    opts++;
                }
                else
                {
                    ParseError("Bad TCP option number '%s'; must be"
                        " between 2 and 255 inclusive", toks[i]);
                }
            }
            else if ( opts > 0 )
            {
                i--;
                state = 0;
            }
            else
            {
                ParseError("Bad TCP option '%s'; must be"
                    " sack|echo|partial_order|conn_count|alt_checksum|md5|#", toks[i]);
            }
            break;
        }
    }
    if ( state == 1 )
    {
        ParseError("Missing argument for '%s'", toks[i-1]);
    }
    mSplitFree(&toks, num_toks);
    Print_TCP(pc);
}

//-------------------------------------------------------------------------
// printing stuff
//-------------------------------------------------------------------------

#define ON "on"
#define OFF "off"

static inline void LogConf (const char* p, const char* s)
{
    LogMessage("%12s: %s\n", p, s);
}

static inline void LogFlag (
    const char* p, const NormalizerContext* nc, NormFlags nf)
{
    const char* s = Norm_IsEnabled(nc, nf) ? ON : OFF;
    LogConf(p, s);
}

static void Print_IP4 (struct _SnortConfig *sc, const NormalizerContext* nc)
{
    LogMessage("Normalizer config:\n");
    LogFlag("ip4", nc, NORM_IP4);

    if ( Norm_IsEnabled(nc, NORM_IP4) )
    {
        //LogFlag("ip4::id", nc, NORM_IP4_ID);
        LogFlag("ip4::df", nc, NORM_IP4_DF);
        LogFlag("ip4::rf", nc, NORM_IP4_RF);
        LogFlag("ip4::tos", nc, NORM_IP4_TOS);
        LogFlag("ip4::trim", nc, NORM_IP4_TRIM);

        if ( Norm_IsEnabled(nc, NORM_IP4_TTL) )
        {
            tSfPolicyId pid = getParserPolicy(sc);
            int min = sc->targeted_policies[pid]->min_ttl;
            int new = sc->targeted_policies[pid]->new_ttl;
            LogMessage("%12s: %s (min=%d, new=%d)\n", "ip4::ttl", ON, min, new);
        }
        else
            LogConf("ip4::ttl", OFF);
    }
}

static void Print_ICMP4 (const NormalizerContext* nc)
{
    LogMessage("Normalizer config:\n");
    LogFlag("icmp4", nc, NORM_ICMP4);
}

static void Print_IP6 (struct _SnortConfig *sc, const NormalizerContext* nc)
{
    LogMessage("Normalizer config:\n");
    LogFlag("ip6", nc, NORM_IP6);

    if ( Norm_IsEnabled(nc, NORM_IP6) )
    {
        if ( Norm_IsEnabled(nc, NORM_IP6_TTL) )
        {
            tSfPolicyId pid = getParserPolicy(sc);
            int min = sc->targeted_policies[pid]->min_ttl;
            int new = sc->targeted_policies[pid]->new_ttl;
            LogMessage("%12s: %s (min=%d, new=%d)\n", "ip6::hops", ON, min, new);
        }
        else
            LogConf("ip6::hops", OFF);
    }
}

static void Print_ICMP6 (const NormalizerContext* nc)
{
    LogMessage("Normalizer config:\n");
    LogFlag("icmp6", nc, NORM_ICMP6);
}

static void Print_TCP (const NormalizerContext* nc)
{
    LogMessage("Normalizer config:\n");
    LogFlag("tcp", nc, NORM_TCP);

    if ( Norm_IsEnabled(nc, NORM_TCP) )
    {
        const char* s;

        if ( Norm_IsEnabled(nc, NORM_TCP_ECN_PKT) )
            s = "packet";
        else if ( Norm_IsEnabled(nc, NORM_TCP_ECN_STR) )
            s = "stream";
        else
            s = OFF;

        LogConf("tcp::ecn", s);
        LogFlag("tcp::block", nc, NORM_TCP_BLOCK);
        LogFlag("tcp::rsv", nc, NORM_TCP_RSV);
        LogFlag("tcp::pad", nc, NORM_TCP_PAD);
        LogFlag("tcp::req_urg", nc, NORM_TCP_REQ_URG);
        LogFlag("tcp::req_pay", nc, NORM_TCP_REQ_PAY);
        LogFlag("tcp::req_urp", nc, NORM_TCP_REQ_URP);
        LogFlag("tcp::urp", nc, NORM_TCP_URP);

        if ( Norm_IsEnabled(nc, NORM_TCP_OPT) )
        {
            char buf[1024] = "";
            char* p = buf;
            int opt;
            size_t min;

            p += snprintf(p, buf+sizeof(buf)-p, "%s", "(allow ");
            min = strlen(buf);

            // TBD translate options to keywords allowed by parser
            for ( opt = 2; opt < 256; opt++ )
            {
                const char* fmt = (strlen(buf) > min) ? ",%d" : "%d";
                if ( Norm_TcpIsOptional(nc, opt) )
                    p += snprintf(p, buf+sizeof(buf)-p, fmt, opt);
            }
            if ( strlen(buf) > min )
            {
                snprintf(p, buf+sizeof(buf)-p, "%c", ')');
                buf[sizeof(buf)-1] = '\0';
            }
            LogMessage("%12s: %s %s\n", "tcp::opt", ON, buf);
        }
        else
            LogConf("tcp::opt", OFF);

        LogFlag("tcp::ips", nc, NORM_TCP_IPS);
        LogFlag("tcp::trim_syn", nc, NORM_TCP_TRIM_SYN);
        LogFlag("tcp::trim_rst", nc, NORM_TCP_TRIM_RST);
        LogFlag("tcp::trim_win", nc, NORM_TCP_TRIM_WIN);
        LogFlag("tcp::trim_mss", nc, NORM_TCP_TRIM_MSS);
    }
}

//-------------------------------------------------------------------------
// preproc (main) stuff
//-------------------------------------------------------------------------

static void Preproc_Install (struct _SnortConfig *sc)
{
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile(
        "normalize", &norm_perf_stats, 0, &totalPerfStats, NULL);
#endif
    AddFuncToPreprocCleanExitList(
        Preproc_CleanExit, NULL, PRIORITY_LAST, PP_NORMALIZE);

    AddFuncToPreprocResetList(
        Preproc_Reset, NULL, PP_NORMALIZE_PRIORITY, PP_NORMALIZE);

    AddFuncToPreprocResetStatsList(
        Preproc_ResetStats, NULL, PP_NORMALIZE_PRIORITY, PP_NORMALIZE);

    AddFuncToConfigCheckList(sc, Preproc_CheckConfig);
    AddFuncToPreprocPostConfigList(sc, Preproc_PostConfigInit, NULL);
    RegisterPreprocStats("normalize", Preproc_PrintStats);
}

//-------------------------------------------------------------------------

static int Preproc_CheckPolicy (
    struct _SnortConfig *sc,
    tSfPolicyUserContextId set,
    tSfPolicyId pid,
    void* pv)
{
    //NormalizerContext* pc = (NormalizerContext*)pv;
    return 0;
}

static int Preproc_CheckConfig (struct _SnortConfig *sc)
{
    int rval;

    if ( !base_set )
        return 0;

    if ((rval = sfPolicyUserDataIterate(sc, base_set, Preproc_CheckPolicy)))
        return rval;

    return 0;
}

static int Preproc_PostInit (
    struct _SnortConfig *sc,
    tSfPolicyUserContextId set,
    tSfPolicyId pid,
    void* pv)
{
    NormalizerContext *pc = (NormalizerContext *)pv;
    SnortPolicy* policy = sc->targeted_policies[pid];

    if ( policy->new_ttl && policy->new_ttl < policy->min_ttl )
    {
        policy->new_ttl = policy->min_ttl;
    }
    Norm_SetConfig(pc);
    return 0;
}

static void Preproc_PostConfigInit (struct _SnortConfig *sc, void* pv)
{
    sfPolicyUserDataIterate(sc, base_set, Preproc_PostInit);
}

//-------------------------------------------------------------------------

static void Preproc_Execute (Packet *p, void *context)
{
    tSfPolicyId pid = getNapRuntimePolicy();
    NormalizerContext* pc = (NormalizerContext*)sfPolicyUserDataGet(base_set, pid);
    PROFILE_VARS;

    if ( !pc )
        return;

    PREPROC_PROFILE_START(norm_perf_stats);

    if ( DAQ_GetInterfaceMode(p->pkth) == DAQ_MODE_INLINE )
        if ( !Active_PacketWasDropped() )
            if ( pc->normMode == NORM_MODE_ON || pc->normMode == NORM_MODE_WOULDA )
                Norm_Packet(pc, p);

    PREPROC_PROFILE_END(norm_perf_stats);
    return;
}

//-------------------------------------------------------------------------

static void Preproc_FreeContext (NormalizerContext* pc)
{
    if ( pc )
        free(pc);
}

static int Preproc_FreePolicy(
        tSfPolicyUserContextId set,
        tSfPolicyId pid,
        void* pv
        )
{
    NormalizerContext* pc = (NormalizerContext*)pv;

    sfPolicyUserDataClear(set, pid);
    Preproc_FreeContext(pc);

    return 0;
}

static void Preproc_FreeSet (tSfPolicyUserContextId set)
{
    if ( !set )
        return;

    sfPolicyUserDataFreeIterate(set, Preproc_FreePolicy);
    sfPolicyConfigDelete(set);
}

//-------------------------------------------------------------------------

static void Preproc_CleanExit (int signal, void *foo)
{
    Preproc_FreeSet(base_set);
}

static void Preproc_Reset (int signal, void *foo) { }

static void Preproc_PrintStats(int exiting)
{
    if(!ScNapPassiveMode())
    {
        Norm_PrintStats();
        Stream_PrintNormalizationStats();
    }
}

static void Preproc_ResetStats (int signal, void *foo)
{
    Norm_ResetStats();
    Stream_ResetNormalizationStats();
}

//-------------------------------------------------------------------------
// reload stuff
//-------------------------------------------------------------------------

#ifdef SNORT_RELOAD
static NormalizerContext* Reload_GetContext (struct _SnortConfig *sc, void **new_config)
{
    tSfPolicyUserContextId swap_set;
    NormalizerContext* pc = NULL;
    tSfPolicyId policy_id = getParserPolicy(sc);

    if ( ScNapPassiveModeNewConf(sc) )
        return NULL;

    if (!(swap_set = (tSfPolicyUserContextId)*new_config))
        if (!(swap_set = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "normalize_ip4")))
            if (!(swap_set = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "normalize_ip6")))
                if (!(swap_set = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "normalize_tcp")))
                    if (!(swap_set = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "normalize_icmp4")))
                        swap_set = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "normalize_icmp6");

    if ( !swap_set )
    {
        swap_set = sfPolicyConfigCreate();
        *new_config = (void *)swap_set;
    }

    sc->normalizer_set  = true;

    sfPolicyUserPolicySet(swap_set, policy_id);
    pc = sfPolicyUserDataGetCurrent(swap_set);

    if ( !pc )
    {
        pc = (NormalizerContext* )SnortAlloc(sizeof(NormalizerContext));
        sfPolicyUserDataSetCurrent(swap_set, pc);

        if( pc->regFunc != init)
        {
            AddFuncToPreprocList(
                sc, Preproc_Execute, PP_NORMALIZE_PRIORITY, PP_NORMALIZE, PROTO_BITS);
            pc->regFunc = reload;
        }
        session_api->enable_preproc_all_ports( sc, PP_NORMALIZE, PROTO_BITS );          
  
    }
    if ( sc->targeted_policies[policy_id]->nap_policy_mode == POLICY_MODE__INLINE_TEST )
        pc->normMode = NORM_MODE_WOULDA;
    else
        pc->normMode = NORM_MODE_ON;
    return pc;
}

static void Reload_IP4 (struct _SnortConfig *sc, char* args, void **new_config)
{
    NormalizerContext* pc = Reload_GetContext(sc, new_config);

    if ( pc )
        Parse_IP4(sc, pc, args);
    else
        LogMessage(NOT_INLINE, "ip4");
}

static void Reload_ICMP4 (struct _SnortConfig *sc, char* args, void **new_config)
{
    NormalizerContext* pc = Reload_GetContext(sc, new_config);

    if ( pc )
        Parse_ICMP4(pc, args);
    else
        LogMessage(NOT_INLINE, "icmp4");
}

static void Reload_IP6 (struct _SnortConfig *sc, char* args, void **new_config)
{
    NormalizerContext* pc = Reload_GetContext(sc, new_config);

    if ( pc )
        Parse_IP6(sc, pc, args);
    else
        LogMessage(NOT_INLINE, "ip6");
}

static void Reload_ICMP6 (struct _SnortConfig *sc, char* args, void **new_config)
{
    NormalizerContext* pc = Reload_GetContext(sc, new_config);

    if ( pc )
        Parse_ICMP6(pc, args);
    else
        LogMessage(NOT_INLINE, "icmp6");
}

static void Reload_TCP (struct _SnortConfig *sc, char* args, void **new_config)
{
    NormalizerContext* pc = Reload_GetContext(sc, new_config);

    if ( pc )
        Parse_TCP(pc, args);
    else
        LogMessage(NOT_INLINE, "tcp");
}

//-------------------------------------------------------------------------

static int Reload_VerifyPolicy (
    struct _SnortConfig *sc,
    tSfPolicyUserContextId set,
    tSfPolicyId pid,
    void* pv
) {
    //NormalizerContext* pc = (NormalizerContext*)pv;
    SnortPolicy* policy = sc->targeted_policies[pid];
    if ( policy->new_ttl && policy->new_ttl < policy->min_ttl )
    {
        policy->new_ttl = policy->min_ttl;
    }
    return 0;
}

static int Reload_Verify(struct _SnortConfig *sc, void *swap_config)
{
    int rval;
    tSfPolicyUserContextId swap_set = (tSfPolicyUserContextId)swap_config;

    if ( !swap_set )
        return -1;

    if ((rval = sfPolicyUserDataIterate (sc, swap_set, Reload_VerifyPolicy)))
        return rval;

    return 0;
}

//-------------------------------------------------------------------------

static int Reload_SwapPolicy (
    tSfPolicyUserContextId set,
    tSfPolicyId pid,
    void* pv)
{
    NormalizerContext* pc = (NormalizerContext*)pv;

    sfPolicyUserDataClear(set, pid);
    Preproc_FreeContext(pc);

    return 0;
}

static void* Reload_Swap (struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId swap_set = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_set = base_set;

    if ( !swap_set )
        return NULL;

    base_set = swap_set;

    sfPolicyUserDataIterate(sc, base_set, Preproc_PostInit);

    if ( old_set )
    {
        sfPolicyUserDataFreeIterate(old_set, Reload_SwapPolicy);

        if ( !sfPolicyUserPolicyGetActive(old_set) )
            return (void*)old_set;
    }
    return NULL;
}

//-------------------------------------------------------------------------

static void Reload_Free (void* pv)
{
    if ( !pv )
        return;

    Preproc_FreeSet((tSfPolicyUserContextId)pv);
}
#endif

//-------------------------------------------------------------------------
// public methods
//-------------------------------------------------------------------------

NormMode Normalize_GetMode (const SnortConfig* sc, NormFlags nf)
{
    tSfPolicyId pid;
    NormalizerContext* pc;

    if ( !base_set )
        return NORM_MODE_OFF;

    if (!sc->normalizer_set)
        return NORM_MODE_OFF;

    pid = getNapRuntimePolicy();
    pc = sfPolicyUserDataGet(base_set, pid);

    if ( !pc )
         return NORM_MODE_OFF;

    if ( Norm_IsEnabled(pc, nf) )
        return pc->normMode;
    return NORM_MODE_OFF;
}

#endif  // NORMALIZER

