/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */
#ifndef __PLUGBASE_H__
#define __PLUGBASE_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "bitop_funcs.h"
#include "rules.h"
#include "treenodes.h"
#include "sf_types.h"
#include "debug.h"

#ifndef WIN32
# include <sys/ioctl.h>
#endif  /* !WIN32 */

#ifdef ENABLE_SSL
# ifdef Free
/* Free macro in radix.h if defined, will conflict with OpenSSL definition */
#  undef Free
# endif
#endif

#ifndef WIN32
# include <net/route.h>
#endif /* !WIN32 */

#ifdef ENABLE_SSL
# undef Free
#endif

#if defined(SOLARIS) || defined(FREEBSD) || defined(OPENBSD)
# include <sys/param.h>
#endif

#if defined(FREEBSD) || defined(OPENBSD) || defined(NETBSD) || defined(OSF1)
# include <sys/mbuf.h>
#endif

#ifndef IFNAMSIZ /* IFNAMSIZ is defined in all platforms I checked.. */
# include <net/if.h>
#endif

#include "preprocids.h"


/* Macros *********************************************************************/
#define SMALLBUFFER 32

#define DETECTION_KEYWORD 0
#define RESPONSE_KEYWORD  1

#define ENCODING_HEX     0
#define ENCODING_BASE64  1
#define ENCODING_ASCII   2

#define DETAIL_FAST  0
#define DETAIL_FULL  1


/**************************** Rule Option Plugin API **************************/
typedef enum _RuleOptType
{
	OPT_TYPE_ACTION = 0,
	OPT_TYPE_LOGGING,
	OPT_TYPE_DETECTION,
	OPT_TYPE_MAX

} RuleOptType;

typedef void (*RuleOptConfigFunc)(char *, OptTreeNode *, int);
typedef void (*RuleOptOtnHandler)(OptTreeNode *);
typedef void (*RuleOptOverrideFunc)(char *, char *, char *, OptTreeNode *, int);
typedef void (*RuleOptOverrideInitFunc)(char *, char *, RuleOptOverrideFunc);
typedef int (*RuleOptEvalFunc)(void *, Packet *);
typedef int (*ResponseFunc)(Packet*, void*);
typedef void (*PluginSignalFunc)(int, void *);
typedef void (*RuleOptParseCleanupFunc)(void);
typedef int (*RuleOptByteOrderFunc)(void *, int32_t);

#define func fptr.fptr
#define vfunc fptr.void_fptr

typedef struct _RuleOptConfigFuncNode
{
    char *keyword;
    RuleOptType type;
    union {
        RuleOptConfigFunc fptr;
        void *void_fptr;
    } fptr;
    RuleOptOtnHandler otn_handler;
    struct _RuleOptConfigFuncNode *next;

} RuleOptConfigFuncNode;

typedef struct _RuleOptOverrideInitFuncNode
{
    char *keyword;
    RuleOptType type;
    union {
        RuleOptOverrideInitFunc fptr;
        void *void_fptr;
    } fptr;
    RuleOptOtnHandler otn_handler;
    struct _RuleOptOverrideInitFuncNode *next;

} RuleOptOverrideInitFuncNode;

typedef struct _RuleOptParseCleanupNode
{
    union {
        RuleOptParseCleanupFunc fptr;
        void *void_fptr;
    } fptr;
    struct _RuleOptParseCleanupNode *next;

} RuleOptParseCleanupNode;

typedef struct _RuleOptByteOrderFuncNode
{
    char *keyword;
    union {
        RuleOptByteOrderFunc fptr;
        void *void_fptr;
    } fptr;
    struct _RuleOptByteOrderFuncNode *next;
} RuleOptByteOrderFuncNode;

void RegisterRuleOptions(void);
void RegisterRuleOption(char *, RuleOptConfigFunc, RuleOptOverrideInitFunc, RuleOptType, RuleOptOtnHandler);
void RegisterOverrideKeyword(char *, char *, RuleOptOverrideFunc);
void RegisterByteOrderKeyword(char *, RuleOptByteOrderFunc);
void DumpRuleOptions(void);
OptFpList * AddOptFuncToList(RuleOptEvalFunc, OptTreeNode *);
void AddRspFuncToList(ResponseFunc, OptTreeNode *, void *);
void FreeRuleOptConfigFuncs(RuleOptConfigFuncNode *);
void FreeRuleOptOverrideInitFuncs(RuleOptOverrideInitFuncNode *);
void AddFuncToRuleOptParseCleanupList(RuleOptParseCleanupFunc);
void RuleOptParseCleanup(void);
void FreeRuleOptParseCleanupList(RuleOptParseCleanupNode *);

void RegisterByteOrderKeyword(char *, RuleOptByteOrderFunc);
RuleOptByteOrderFunc GetByteOrderFunc(char *);
void FreeRuleOptByteOrderFuncs(RuleOptByteOrderFuncNode *);

/***************************** Non Rule Detection API *************************/
typedef void (*DetectionEvalFunc)(Packet *, void *);
typedef struct _DetectionEvalFuncNode
{
    void *context;
    uint16_t priority;
    uint32_t detect_id;
    //uint32_t detect_bit;
    uint32_t proto_mask;
    union
    {
        DetectionEvalFunc fptr;
        void *void_fptr;
    } fptr;
    struct _DetectionEvalFuncNode *next;

} DetectionEvalFuncNode;

DetectionEvalFuncNode * AddFuncToDetectionList(DetectionEvalFunc, uint16_t, uint32_t, uint32_t);
void FreeDetectionEvalFuncs(DetectionEvalFuncNode *);

/***************************** Preprocessor API *******************************/
typedef void (*PreprocConfigFunc)(char *);
typedef void (*PreprocStatsFunc)(int);
typedef void (*PreprocEvalFunc)(Packet *, void *);
typedef void (*PreprocCheckConfigFunc)(void);
typedef void (*PreprocSignalFunc)(int, void *);
typedef void * (*PreprocReassemblyPktFunc)(void);
typedef void (*PreprocPostConfigFunc)(void *);

#ifdef SNORT_RELOAD
typedef void (*PreprocReloadFunc)(char *);
typedef int (*PreprocReloadVerifyFunc)(void);
typedef void * (*PreprocReloadSwapFunc)(void);
typedef void (*PreprocReloadSwapFreeFunc)(void *);
#endif

#define config_func cfptr.fptr
#define config_vfunc cfptr.void_fptr
typedef struct _PreprocConfigFuncNode
{
    char *keyword;
    union {
        PreprocConfigFunc fptr;
        void *void_fptr;
    } cfptr;

#ifdef SNORT_RELOAD
    /* Tells whether we call the config func or reload func */
    int initialized;
    void *swap_free_data;
    PreprocReloadFunc reload_func;
    PreprocReloadVerifyFunc reload_verify_func;
    PreprocReloadSwapFunc reload_swap_func;
    PreprocReloadSwapFreeFunc reload_swap_free_func;
#endif

    struct _PreprocConfigFuncNode *next;

} PreprocConfigFuncNode;

typedef struct _PreprocStatsFuncNode
{
    char *keyword;
    union
    {
        PreprocStatsFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocStatsFuncNode *next;

} PreprocStatsFuncNode;

typedef struct _PreprocEvalFuncNode
{
    void *context;
    uint16_t priority;
    uint32_t preproc_id;
    uint32_t preproc_bit;
    uint32_t proto_mask;
    union
    {
        PreprocEvalFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocEvalFuncNode *next;

} PreprocEvalFuncNode;

typedef struct _PreprocCheckConfigFuncNode
{
    union
    {
        PreprocCheckConfigFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocCheckConfigFuncNode *next;

} PreprocCheckConfigFuncNode;

typedef struct _PreprocSignalFuncNode
{
    void *arg;
    uint16_t priority;
    uint32_t preproc_id;
    union
    {
        PreprocSignalFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocSignalFuncNode *next;

} PreprocSignalFuncNode;

typedef struct _PreprocReassemblyPktFuncNode
{
    unsigned int preproc_id;
    union
    {
        PreprocReassemblyPktFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocReassemblyPktFuncNode *next;

} PreprocReassemblyPktFuncNode;

typedef struct _PreprocPostConfigFuncNode
{
    void *data;
    union
    {
        PreprocPostConfigFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocPostConfigFuncNode *next;

} PreprocPostConfigFuncNode;

#ifdef SNORT_RELOAD
typedef struct _PreprocReloadVerifyFuncNode
{
    union
    {
        PreprocReloadVerifyFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocReloadVerifyFuncNode *next;

} PreprocReloadVerifyFuncNode;
#endif


struct _SnortConfig;

void RegisterPreprocessors(void);
#ifndef SNORT_RELOAD
void RegisterPreprocessor(char *, PreprocConfigFunc);
#else
void RegisterPreprocessor(char *, PreprocConfigFunc, PreprocReloadFunc,
                          PreprocReloadSwapFunc, PreprocReloadSwapFreeFunc);
#endif
PreprocConfigFuncNode * GetPreprocConfig(char *);
PreprocConfigFunc GetPreprocConfigFunc(char *);
void RegisterPreprocStats(char *, PreprocStatsFunc);
void DumpPreprocessors(void);
void AddFuncToConfigCheckList(PreprocCheckConfigFunc);
void AddFuncToPreprocPostConfigList(PreprocPostConfigFunc, void *);
void CheckPreprocessorsConfig(struct _SnortConfig *);
PreprocEvalFuncNode * AddFuncToPreprocList(PreprocEvalFunc, uint16_t, uint32_t, uint32_t);
void AddFuncToPreprocCleanExitList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocShutdownList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocResetList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocResetStatsList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocReassemblyPktList(PreprocReassemblyPktFunc, uint32_t);
int IsPreprocEnabled(uint32_t);
void FreePreprocConfigFuncs(void);
void FreePreprocCheckConfigFuncs(PreprocCheckConfigFuncNode *);
void FreePreprocStatsFuncs(PreprocStatsFuncNode *);
void FreePreprocEvalFuncs(PreprocEvalFuncNode *);
void FreePreprocReassemblyPktFuncs(PreprocReassemblyPktFuncNode *);
void FreePreprocSigFuncs(PreprocSignalFuncNode *);
void FreePreprocPostConfigFuncs(PreprocPostConfigFuncNode *);
void PostConfigPreprocessors(struct _SnortConfig *);

#ifdef SNORT_RELOAD
void AddFuncToPreprocReloadVerifyList(PreprocReloadVerifyFunc);
void FreePreprocReloadVerifyFuncs(PreprocReloadVerifyFuncNode *);
int VerifyReloadedPreprocessors(struct _SnortConfig *);
void SwapPreprocConfigurations(void);
void FreeSwappedPreprocConfigurations(void);
void FreePreprocReloadVerifyFuncList(PreprocReloadVerifyFuncNode *);
#endif

static INLINE void DisablePreprocessors(Packet *p) 
{
    p->preprocessor_bits = PP_ALL_OFF;
}

static INLINE void EnablePreprocessors(Packet *p) 
{
    p->preprocessor_bits = PP_ALL_ON;
}

static INLINE int IsPreprocBitSet(Packet *p, unsigned int preproc_bit)
{
    return (p->preprocessor_bits & preproc_bit);
}

static INLINE int SetPreprocBit(Packet *p, unsigned int preproc_id)
{
    p->preprocessor_bits |= (1 << preproc_id);
    return 0;
}

static INLINE int SetAllPreprocBits(Packet *p)
{
    SetPreprocBit(p, PP_SFPORTSCAN);
    SetPreprocBit(p, PP_PERFMONITOR);
    SetPreprocBit(p, PP_STREAM5);
    SetPreprocBit(p, PP_SDF);
    return 0;
}

static INLINE int IsPreprocReassemblyPktBitSet(Packet *p, unsigned int preproc_id)
{
    return (p->preproc_reassembly_pkt_bits & (1 << preproc_id)) != 0;
}

static INLINE int SetPreprocReassemblyPktBit(Packet *p, unsigned int preproc_id)
{
    p->preproc_reassembly_pkt_bits |= (1 << preproc_id);
    p->packet_flags |= PKT_PREPROC_RPKT;
    return 0;
}

/************************** Miscellaneous Functions  **************************/

typedef struct _PluginSignalFuncNode
{
    void *arg;
    union
    {
        PluginSignalFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PluginSignalFuncNode *next;

} PluginSignalFuncNode;

/* Used for both rule options and output.  Preprocessors have their own */
void AddFuncToRestartList(PluginSignalFunc, void *);
void AddFuncToCleanExitList(PluginSignalFunc, void *);
void AddFuncToShutdownList(PluginSignalFunc, void *);
void AddFuncToPostConfigList(PluginSignalFunc, void *);
void AddFuncToSignalList(PluginSignalFunc, void *, PluginSignalFuncNode **);
void PostConfigInitPlugins(PluginSignalFuncNode *);
void FreePluginSigFuncs(PluginSignalFuncNode *);


#endif /* __PLUGBASE_H__ */
