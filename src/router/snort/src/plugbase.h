/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include "snort_debug.h"
#include "preprocids.h"

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

typedef void (*RuleOptConfigFunc)(struct _SnortConfig *, char *, OptTreeNode *, int);
typedef void (*RuleOptOtnHandler)(struct _SnortConfig *, OptTreeNode *);
typedef void (*RuleOptOverrideFunc)(struct _SnortConfig *, char *, char *, char *, OptTreeNode *, int);
typedef void (*RuleOptOverrideInitFunc)(char *, char *, RuleOptOverrideFunc);
typedef int (*RuleOptEvalFunc)(void *, Packet *);
typedef int (*ResponseFunc)(Packet*, void*);
typedef void (*PluginSignalFunc)(int, void *);
typedef void (*PluginSignalFuncWithSnortConfig)(struct _SnortConfig *, int, void *);
typedef void (*PostConfigFunc)(struct _SnortConfig *, int, void *);
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

/***************************** Buffer Dump API ********************************/

#ifdef DUMP_BUFFER
void RegisterBufferTracer(TraceBuffer * (*)(), BUFFER_DUMP_FUNC);
#endif

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

DetectionEvalFuncNode * AddFuncToDetectionList(struct _SnortConfig *, DetectionEvalFunc, uint16_t, uint32_t, uint32_t);
void FreeDetectionEvalFuncs(DetectionEvalFuncNode *);

/***************************** Preprocessor API *******************************/
typedef void (*PreprocConfigFunc)(struct _SnortConfig *, char *);
typedef void (*PreprocStatsFunc)(int);
typedef void (*PreprocEvalFunc)(Packet *, void *);
typedef int (*PreprocCheckConfigFunc)(struct _SnortConfig *);
typedef void (*PreprocSignalFunc)(int, void *);
typedef void (*PreprocPostConfigFunc)(struct _SnortConfig *, void *);
typedef void (*PreprocMetaEvalFunc)(int, const uint8_t *);

typedef void (*PeriodicFunc)(int, void *);

#ifdef SNORT_RELOAD
struct _PreprocConfigFuncNode;
typedef struct _PreprocessorSwapData
{
    struct _PreprocConfigFuncNode *preprocNode;
    void *data;
    struct _PreprocessorSwapData *next;
} PreprocessorSwapData;

typedef void (*PreprocReloadFunc)(struct _SnortConfig *, char *, void **);
typedef int (*PreprocReloadVerifyFunc)(struct _SnortConfig *, void *);
typedef void * (*PreprocReloadSwapFunc)(struct _SnortConfig *, void *);
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
    PreprocEnableMask preproc_bit;
    uint32_t proto_mask;
    union
    {
        PreprocEvalFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocEvalFuncNode *next;

} PreprocEvalFuncNode;

typedef struct _PreprocMetaEvalFuncNode
{
    uint16_t priority;
    uint32_t preproc_id;
    PreprocEnableMask preproc_bit;
    union
    {
        PreprocMetaEvalFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PreprocMetaEvalFuncNode *next;
} PreprocMetaEvalFuncNode;

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

typedef struct _PeriodicCheckFuncNode
{
    void *arg;
    uint16_t priority;
    uint32_t preproc_id;
    uint32_t period;
    uint32_t time_left;
    union
    {
        PeriodicFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PeriodicCheckFuncNode *next;

} PeriodicCheckFuncNode;


struct _SnortConfig;

void RegisterPreprocessors(void);
#ifndef SNORT_RELOAD
void RegisterPreprocessor(const char *, PreprocConfigFunc);
#else
void RegisterPreprocessor(const char *, PreprocConfigFunc, PreprocReloadFunc,
                          PreprocReloadVerifyFunc, PreprocReloadSwapFunc,
                          PreprocReloadSwapFreeFunc);
void *GetRelatedReloadData(struct _SnortConfig *, const char *);
void *GetReloadStreamConfig(struct _SnortConfig *sc);
#endif
PreprocConfigFuncNode * GetPreprocConfig(char *);
PreprocConfigFunc GetPreprocConfigFunc(char *);
void RegisterPreprocStats(const char *, PreprocStatsFunc);
void DumpPreprocessors(void);
void AddFuncToConfigCheckList(struct _SnortConfig *, PreprocCheckConfigFunc);
void AddFuncToPreprocPostConfigList(struct _SnortConfig *, PreprocPostConfigFunc, void *);
int CheckPreprocessorsConfig(struct _SnortConfig *);
PreprocEvalFuncNode * AddFuncToPreprocList(struct _SnortConfig *, PreprocEvalFunc, uint16_t, uint32_t, uint32_t);
void AddFuncToPreprocListAllNapPolicies(struct _SnortConfig *sc, PreprocEvalFunc pp_eval_func, uint16_t priority,
                                        uint32_t preproc_id, uint32_t proto_mask);
PreprocMetaEvalFuncNode * AddFuncToPreprocMetaEvalList(struct _SnortConfig *, PreprocMetaEvalFunc, uint16_t, uint32_t);
void AddFuncToPreprocCleanExitList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocShutdownList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocResetList(PreprocSignalFunc, void *, uint16_t, uint32_t);
void AddFuncToPreprocResetStatsList(PreprocSignalFunc, void *, uint16_t, uint32_t);
int IsPreprocEnabled(struct _SnortConfig *, uint32_t);
void FreePreprocConfigFuncs(void);
void FreePreprocCheckConfigFuncs(PreprocCheckConfigFuncNode *);
void FreePreprocStatsFuncs(PreprocStatsFuncNode *);
void FreePreprocEvalFuncs(PreprocEvalFuncNode *);
void FreePreprocMetaEvalFuncs(PreprocMetaEvalFuncNode *);
void FreePreprocSigFuncs(PreprocSignalFuncNode *);
void FreePreprocPostConfigFuncs(PreprocPostConfigFuncNode *);
void PostConfigPreprocessors(struct _SnortConfig *);
void FilterConfigPreprocessors(struct _SnortConfig *sc);

#ifdef SNORT_RELOAD
int VerifyReloadedPreprocessors(struct _SnortConfig *);
void SwapPreprocConfigurations(struct _SnortConfig *);
void FreeSwappedPreprocConfigurations(struct _SnortConfig *);
void FreePreprocessorReloadData(struct _SnortConfig *);
#endif

void AddFuncToPeriodicCheckList(PeriodicFunc, void *, uint16_t, uint32_t, uint32_t);
void FreePeriodicFuncs(PeriodicCheckFuncNode *head);

static inline void DisableAppPreprocessors( Packet *p )
{
    p->preprocessor_bits &= ( PP_CLASS_NETWORK | PP_CLASS_NGFW );
}

static inline void DisableAllPreprocessors( Packet *p )
{
   p->preprocessor_bits = PP_DISABLE_ALL;
}

static inline int EnablePreprocessor(Packet *p, unsigned int preproc_id)
{
    p->preprocessor_bits |= (UINT64_C(1) << preproc_id);
    return 0;
}

static inline void EnablePreprocessors(Packet *p, PreprocEnableMask enabled_pps)
{
    p->preprocessor_bits = enabled_pps;
}

static inline int IsPreprocessorEnabled(Packet *p, PreprocEnableMask preproc_bit)
{
    return ( ( p->preprocessor_bits & preproc_bit ) != 0 );
}

void DisableAllPolicies(struct _SnortConfig *);
int ReenablePreprocBit(struct _SnortConfig *, unsigned int preproc_id);

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

typedef struct _PostConfigFuncNode
{
    void *arg;
    union
    {
        PostConfigFunc fptr;
        void *void_fptr;
    } fptr;
    struct _PostConfigFuncNode *next;

} PostConfigFuncNode;

/* Used for both rule options and output.  Preprocessors have their own */
#ifdef SNORT_RELOAD
void AddFuncToReloadList(PostConfigFunc, void *);
#endif
void AddFuncToCleanExitList(PluginSignalFunc, void *);
void AddFuncToShutdownList(PluginSignalFunc, void *);
void AddFuncToPostConfigList(struct _SnortConfig *, PostConfigFunc, void *);
void AddFuncToSignalList(PluginSignalFunc, void *, PluginSignalFuncNode **);
void PostConfigInitPlugins(struct _SnortConfig *, PostConfigFuncNode *);
void FreePluginSigFuncs(PluginSignalFuncNode *);
void FreePluginPostConfigFuncs(PostConfigFuncNode *);

typedef char** (*GetHttpXffFieldsFunc)(int* nFields);
char** GetHttpXffFields(int* nFields);
void RegisterGetHttpXffFields(GetHttpXffFieldsFunc fn);

#endif /* __PLUGBASE_H__ */
