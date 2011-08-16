/* $Id$ */
/*
 ** Copyright (C) 2010-2011 Sourcefire, Inc.
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
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include "sfPolicy.h"
#include "snort.h"
#include "spp_normalize.h"
#include "snort_stream5_tcp.h"

static tSfPolicyUserContextId base_set = NULL;
#ifdef SNORT_RELOAD
static tSfPolicyUserContextId swap_set = NULL;
#endif

#ifdef PERF_PROFILING
PreprocStats norm_perf_stats;
#endif

static void Preproc_Execute(Packet* , void*);
static void Preproc_CleanExit(int, void*);
static void Preproc_Reset(int, void*);
static void Preproc_PostConfigInit(void*);
static void Preproc_CheckConfig (void);
static void Preproc_ResetStats(int, void*);
static void Preproc_PrintStats(int);
static void Preproc_Install(void);

static void Init_IP4(char*);
static void Init_ICMP4(char*);
#ifdef SUP_IP6
static void Init_IP6(char*);
static void Init_ICMP6(char*);
#endif
static void Init_TCP(char*);

static void Parse_IP4(NormalizerContext*, char*);
static void Parse_ICMP4(NormalizerContext*, char*);
#ifdef SUP_IP6
static void Parse_IP6(NormalizerContext*, char*);
static void Parse_ICMP6(NormalizerContext*, char*);
#endif
static void Parse_TCP(NormalizerContext*, char*);

static void Print_IP4(const NormalizerContext*);
static void Print_ICMP4(const NormalizerContext*);
#ifdef SUP_IP6
static void Print_IP6(const NormalizerContext*);
static void Print_ICMP6(const NormalizerContext*);
#endif
static void Print_TCP(const NormalizerContext*);

#ifdef SNORT_RELOAD
static void Reload_IP4(char*);
static void Reload_ICMP4(char*);
#ifdef SUP_IP6
static void Reload_IP6(char*);
static void Reload_ICMP6(char*);
#endif
static void Reload_TCP(char*);

static int Reload_Verify(void);
static void* Reload_Swap(void);
static void Reload_Free(void*);
#endif

#ifdef SNORT_RELOAD
#define NORM_FUNCS(p) Init_ ## p, Reload_ ## p, Reload_Swap, Reload_Free
#else
#define NORM_FUNCS(p) Init_ ## p
#endif

void SetupNormalizer (void)
{
    RegisterPreprocessor("normalize_ip4", NORM_FUNCS(IP4));
    RegisterPreprocessor("normalize_icmp4", NORM_FUNCS(ICMP4));
#ifdef SUP_IP6
    RegisterPreprocessor("normalize_ip6", NORM_FUNCS(IP6));
    RegisterPreprocessor("normalize_icmp6", NORM_FUNCS(ICMP6));
#endif
    RegisterPreprocessor("normalize_tcp", NORM_FUNCS(TCP));
}

//-------------------------------------------------------------------------
// basic initialization stuff
//-------------------------------------------------------------------------

#define PROTO_BITS (PROTO_BIT__IP|PROTO_BIT__ICMP|PROTO_BIT__TCP)

static NormalizerContext* Init_GetContext ()
{
    NormalizerContext* pc = NULL;
    tSfPolicyId policy_id = getParserPolicy();

    if ( !base_set )
    {
        base_set = sfPolicyConfigCreate();

        Preproc_Install();
    }
    sfPolicyUserPolicySet(base_set, policy_id);
    pc = sfPolicyUserDataGetCurrent(base_set);

    if ( !pc )
    {
        pc = (NormalizerContext* )SnortAlloc(sizeof(NormalizerContext));
        sfPolicyUserDataSetCurrent(base_set, pc);

        AddFuncToPreprocList(
            Preproc_Execute, PRIORITY_NETWORK, PP_NORMALIZE, PROTO_BITS);
    }
    return pc;
}

#define NOT_INLINE "WARNING: %s normalizations disabled because not inline"

static void Init_IP4 (char* args)
{
    NormalizerContext* pc = Init_GetContext();
    if ( pc && ScInlineMode() )
        Parse_IP4(pc, args);
    else
         LogMessage(NOT_INLINE, "ip4");
}

static void Init_ICMP4 (char* args)
{
    NormalizerContext* pc = Init_GetContext();
    if ( pc && ScInlineMode() )
        Parse_ICMP4(pc, args);
    else
         LogMessage(NOT_INLINE, "icmp4");
}

#ifdef SUP_IP6
static void Init_IP6 (char* args)
{
    NormalizerContext* pc = Init_GetContext();
    if ( pc && ScInlineMode() )
        Parse_IP6(pc, args);
    else
         LogMessage(NOT_INLINE, "ip6");
}

static void Init_ICMP6 (char* args)
{
    NormalizerContext* pc = Init_GetContext();
    if ( pc && ScInlineMode() )
        Parse_ICMP6(pc, args);
    else
         LogMessage(NOT_INLINE, "icmp6");
}
#endif

static void Init_TCP (char* args)
{
    NormalizerContext* pc = Init_GetContext();
    if ( pc && ScInlineMode() )
        Parse_TCP(pc, args);
    else
         LogMessage(NOT_INLINE, "tcp");
}

//-------------------------------------------------------------------------
// parsing stuff
//-------------------------------------------------------------------------

// options may appear in any order separated by ',':
// preprocessor normalize_ip4: [id] [df] [rf] [tos] [trim]
static void Parse_IP4 (NormalizerContext* pc, char* args)
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
        tSfPolicyId pid = getParserPolicy();
        SnortPolicy* policy = snort_conf_for_parsing->targeted_policies[pid];

        if ( (policy->new_ttl > 1) && (policy->new_ttl >= policy->min_ttl) )
        {
            Norm_Enable(pc, NORM_IP4_TTL);
        }
    }
    mSplitFree(&toks, num_toks);
    Print_IP4(pc);
}

// preprocessor normalize_icmp4
static void Parse_ICMP4 (NormalizerContext* pc, char* args)
{
    Norm_Enable(pc, NORM_ICMP4);
    Print_ICMP4(pc);
}

#ifdef SUP_IP6
// preprocessor normalize_ip6
static void Parse_IP6 (NormalizerContext* pc, char* args)
{
    Norm_Enable(pc, NORM_IP6);
    {
        tSfPolicyId pid = getParserPolicy();
        SnortPolicy* policy = snort_conf_for_parsing->targeted_policies[pid];

        if ( (policy->new_ttl > 1) && (policy->new_ttl >= policy->min_ttl) )
        {
            Norm_Enable(pc, NORM_IP6_TTL);
        }
    }
    Print_IP6(pc);
}

// preprocessor normalize_icmp6
static void Parse_ICMP6 (NormalizerContext* pc, char* args)
{
    Norm_Enable(pc, NORM_ICMP6);
    Print_ICMP6(pc);
}
#endif

// options may appear in any order separated by ',':
// preprocessor normalize_tcp: [ecn packet|stream] [urp] [opts] [allow <opt>+]
//
// <opt> ::= sack|echo|partial_order|alt_checksum|md5|#
// where 2 <= # <= 255.
static void Parse_TCP (NormalizerContext* pc, char* args)
{
    char **toks;
    int num_toks;
    int i, state = 0, opts;

    if ( !args ) args = "";
    toks = mSplit(args, ", ", 0, &num_toks, 0);

    // urp is backwards from the rest: on with group and disabled separately
    Norm_Enable(pc, NORM_TCP_URP);

    for (i = 0; i < num_toks; i++)
    {
        switch ( state ) {
        case 0:
            if ( !strcasecmp(toks[i], "ecn") )
            {
                state = 1;
            }
            else if ( !strcasecmp(toks[i], "urp") )
            {
                Norm_Disable(pc, NORM_TCP_URP);
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
    Norm_Enable(pc, NORM_TCP);
    Print_TCP(pc);
}

//-------------------------------------------------------------------------
// printing stuff
//-------------------------------------------------------------------------

#define ON "on"
#define OFF "off"

static INLINE void LogConf (const char* p, const char* s)
{
    LogMessage("%12s: %s\n", p, s);
}

static INLINE void LogFlag (
    const char* p, const NormalizerContext* nc, NormFlags nf)
{
    const char* s = Norm_IsEnabled(nc, nf) ? ON : OFF;
    LogConf(p, s);
}

static void Print_IP4 (const NormalizerContext* nc)
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
            tSfPolicyId pid = getParserPolicy();
            int min = snort_conf_for_parsing->targeted_policies[pid]->min_ttl;
            int new = snort_conf_for_parsing->targeted_policies[pid]->new_ttl;
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

#ifdef SUP_IP6
static void Print_IP6 (const NormalizerContext* nc)
{
    LogMessage("Normalizer config:\n");
    LogFlag("ip6", nc, NORM_IP6);

    if ( Norm_IsEnabled(nc, NORM_IP6) )
    {
        if ( Norm_IsEnabled(nc, NORM_IP6_TTL) )
        {
            tSfPolicyId pid = getParserPolicy();
            int min = snort_conf_for_parsing->targeted_policies[pid]->min_ttl;
            int new = snort_conf_for_parsing->targeted_policies[pid]->new_ttl;
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
#endif

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
    }
}

//-------------------------------------------------------------------------
// preproc (main) stuff
//-------------------------------------------------------------------------

static void Preproc_Install (void)
{
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile(
        "normalize", &norm_perf_stats, 0, &totalPerfStats);
#endif
    AddFuncToPreprocCleanExitList(
        Preproc_CleanExit, NULL, PRIORITY_LAST, PP_NORMALIZE);

    AddFuncToPreprocResetList(
        Preproc_Reset, NULL, PRIORITY_FIRST, PP_NORMALIZE);

    AddFuncToPreprocResetStatsList(
        Preproc_ResetStats, NULL, PRIORITY_FIRST, PP_NORMALIZE);

    AddFuncToConfigCheckList(Preproc_CheckConfig );
    AddFuncToPreprocPostConfigList(Preproc_PostConfigInit, NULL);
    RegisterPreprocStats("normalize", Preproc_PrintStats);
}

//-------------------------------------------------------------------------

static int Preproc_CheckPolicy (
    tSfPolicyUserContextId set,
    tSfPolicyId pid, 
    void* pv)
{
    //NormalizerContext* pc = (NormalizerContext*)pv;
    return 0;
}

static void Preproc_CheckConfig (void)
{
    if ( !base_set )
        return;

    sfPolicyUserDataIterate(base_set, Preproc_CheckPolicy);
}

static int Preproc_PostInit (
    tSfPolicyUserContextId set,
    tSfPolicyId pid, 
    void* pv)
{
    NormalizerContext *pc = (NormalizerContext *)pv;
    SnortPolicy* policy = snort_conf_for_parsing->targeted_policies[pid];

    if ( policy->new_ttl && policy->new_ttl < policy->min_ttl )
    {
        policy->new_ttl = policy->min_ttl;
    }
    Norm_SetConfig(pc);
    return 0;
}

static void Preproc_PostConfigInit (void* pv)
{
    sfPolicyUserDataIterate(base_set, Preproc_PostInit);
}

//-------------------------------------------------------------------------

static void Preproc_Execute (Packet *p, void *context)
{
    tSfPolicyId pid = getRuntimePolicy();
    NormalizerContext* pc = (NormalizerContext*)sfPolicyUserDataGet(base_set, pid);
    PROFILE_VARS;

    if ( !pc )
        return;

    PREPROC_PROFILE_START(norm_perf_stats);

    if ( !Active_PacketWasDropped() )
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

    sfPolicyUserDataIterate(set, Preproc_FreePolicy);
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
    if(ScInlineMode())
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
static NormalizerContext* Reload_GetContext ()
{
    NormalizerContext* pc = NULL;
    tSfPolicyId policy_id = getParserPolicy();

    if ( !swap_set )
    {
        swap_set = sfPolicyConfigCreate();
        AddFuncToPreprocReloadVerifyList(Reload_Verify);
    }
    sfPolicyUserPolicySet(swap_set, policy_id);
    pc = sfPolicyUserDataGetCurrent(swap_set);

    if ( !pc )
    {
        pc = (NormalizerContext* )SnortAlloc(sizeof(NormalizerContext));
        sfPolicyUserDataSetCurrent(swap_set, pc);

        AddFuncToPreprocList(
            Preproc_Execute, PRIORITY_NETWORK, PP_NORMALIZE, PROTO_BITS);
    }
    return pc;
}

static void Reload_IP4 (char* args)
{
    NormalizerContext* pc = Reload_GetContext();
    if ( pc )
        Parse_IP4(pc, args);
}

static void Reload_ICMP4 (char* args)
{
    NormalizerContext* pc = Reload_GetContext();
    if ( pc )
        Parse_ICMP4(pc, args);
}

#ifdef SUP_IP6
static void Reload_IP6 (char* args)
{
    NormalizerContext* pc = Reload_GetContext();
    if ( pc )
        Parse_IP6(pc, args);
}

static void Reload_ICMP6 (char* args)
{
    NormalizerContext* pc = Reload_GetContext();
    if ( pc )
        Parse_ICMP6(pc, args);
}
#endif

static void Reload_TCP (char* args)
{
    NormalizerContext* pc = Reload_GetContext();
    if ( pc )
        Parse_TCP(pc, args);
}

//-------------------------------------------------------------------------

static int Reload_VerifyPolicy (
    tSfPolicyUserContextId set,
    tSfPolicyId pid, 
    void* pv
) {
    //NormalizerContext* pc = (NormalizerContext*)pv;
    SnortPolicy* policy = snort_conf_for_parsing->targeted_policies[pid];
    if ( policy->new_ttl && policy->new_ttl < policy->min_ttl )
    {
        policy->new_ttl = policy->min_ttl;
    }
    return 0;
}

static int Reload_Verify(void)
{
    if ( !swap_set )
        return -1;

    sfPolicyUserDataIterate (swap_set, Reload_VerifyPolicy);
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

static void* Reload_Swap (void)
{
    tSfPolicyUserContextId old_set = base_set;

    if ( !swap_set )
        return NULL;

    base_set = swap_set;
    swap_set = NULL;

    sfPolicyUserDataIterate(base_set, Preproc_PostInit);
    sfPolicyUserDataIterate(old_set, Reload_SwapPolicy);

    if ( !sfPolicyUserPolicyGetActive(old_set) )
        return (void*)old_set;

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

int Normalize_IsEnabled (const SnortConfig* sc, NormFlags nf)
{
    tSfPolicyId pid;
    NormalizerContext* pc;

    if ( !base_set ) return 0;

    pid = getRuntimePolicy();
    pc = sfPolicyUserDataGet(base_set, pid);

    if ( !pc ) return 0;

    return Norm_IsEnabled(pc, nf);
}

#endif  // NORMALIZER

