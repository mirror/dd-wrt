/*
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steven Sturges
 *
 * Dynamic Library Loading for Snort
 *
 */
#ifndef _SF_DYNAMIC_PREPROCESSOR_H_
#define _SF_DYNAMIC_PREPROCESSOR_H_

#include <ctype.h>
#ifdef SF_WCHAR
#include <wchar.h>
#endif
#include "sf_dynamic_meta.h"
#include "ipv6_port.h"
#include "obfuscation.h"
#include "memory_stats.h"

/* specifies that a function does not return
 * used for quieting Visual Studio warnings
 */
#ifdef WIN32
#if _MSC_VER >= 1400
#define NORETURN __declspec(noreturn)
#else
#define NORETURN
#endif
#else
#define NORETURN
#endif

#ifdef PERF_PROFILING
#ifndef PROFILE_PREPROCS_NOREDEF /* Don't redefine this from the main area */
#ifdef PROFILING_PREPROCS
#undef PROFILING_PREPROCS
#endif
#define PROFILING_PREPROCS _dpd.profilingPreprocsFunc()
#endif
#endif

#define PREPROCESSOR_DATA_VERSION 29

#include "sf_dynamic_common.h"
#include "sf_dynamic_engine.h"
#include "session_api.h"
#include "stream_api.h"
#include "str_search.h"
#include "obfuscation.h"
/*#include "sfportobject.h" */
#include "sfcontrol.h"
#ifdef SIDE_CHANNEL
#include "sidechannel_define.h"
#endif
#include "idle_processing.h"
#include "file_api.h"
#include "reload_api.h"
#include "smtp_api.h"

struct _PreprocStats;

#define MINIMUM_DYNAMIC_PREPROC_ID 10000
typedef void (*PreprocessorInitFunc)(struct _SnortConfig *, char *);
typedef void * (*AddPreprocFunc)(struct _SnortConfig *, void (*pp_func)(void *, void *), uint16_t, uint32_t, uint32_t);
typedef void * (*AddMetaEvalFunc)(struct _SnortConfig *, void (*meta_eval_func)(int, const uint8_t *),
                                  uint16_t priority, uint32_t preproc_id);
typedef void (*AddPreprocExit)(void (*pp_exit_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocUnused)(void (*pp_unused_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocConfCheck)(struct _SnortConfig *, int (*pp_conf_chk_func) (struct _SnortConfig *));
typedef void (*AddToPostConfList)(struct _SnortConfig *sc, void (*post_config_func)(struct _SnortConfig *, int , void *), void *arg);
typedef int (*AlertQueueAdd)(uint32_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, const char *, void *);
typedef uint32_t (*GenSnortEvent)(Packet *p, uint32_t gid, uint32_t sid, uint32_t rev,
                                  uint32_t classification, uint32_t priority, const char *msg);
#ifdef SNORT_RELOAD
typedef void (*PreprocessorReloadFunc)(struct _SnortConfig *, char *, void **);
typedef int (*PreprocessorReloadVerifyFunc)(struct _SnortConfig *, void *);
typedef void * (*PreprocessorReloadSwapFunc)(struct _SnortConfig *, void *);
typedef void (*PreprocessorReloadSwapFreeFunc)(void *);
#endif

#ifndef SNORT_RELOAD
typedef void (*PreprocRegisterFunc)(const char *, PreprocessorInitFunc);
#else
typedef void (*PreprocRegisterFunc)(const char *, PreprocessorInitFunc,
                                    PreprocessorReloadFunc,
                                    PreprocessorReloadVerifyFunc,
                                    PreprocessorReloadSwapFunc,
                                    PreprocessorReloadSwapFreeFunc);
typedef void *(*GetRelatedReloadDataFunc)(struct _SnortConfig *, const char *);
#endif
typedef int (*ThresholdCheckFunc)(unsigned int, unsigned int, sfaddr_t*, sfaddr_t*, long);
typedef void (*InlineDropFunc)(void *);
typedef bool (*ActivePacketWasDroppedFunc)(void);
typedef bool (*InlineRetryFunc)(void *);
typedef void (*ActiveEnableFunc)(int);
typedef void (*DisableDetectFunc)(void *);
typedef void (*EnableDetectFunc)(void );
typedef int (*EnablePreprocessorFunc)(void *, uint32_t);
typedef int (*DetectFunc)(void *);
typedef void *(*GetRuleInfoByNameFunc)(char *);
typedef void *(*GetRuleInfoByIdFunc)(int);
typedef int (*printfappendfunc)(char *, int, const char *, ...);
typedef char ** (*TokenSplitFunc)(const char *, const char *, const int, int *, const char);
typedef void (*TokenFreeFunc)(char ***, int);
typedef void (*PreprocStatsNodeFreeFunc)(struct _PreprocStats *stats);
typedef void (*AddPreprocProfileFunc)(const char *, void *, int, void *, PreprocStatsNodeFreeFunc freefn);
typedef int (*ProfilingFunc)(void);
typedef int (*PreprocessFunc)(void *);
#ifdef DUMP_BUFFER
typedef void (*BufferDumpRegisterFunc)(TraceBuffer * (*)(), unsigned int);
#endif
typedef void (*PreprocStatsRegisterFunc)(const char *, void (*pp_stats_func)(int));
typedef void (*AddPreprocReset)(void (*pp_rst_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocResetStats)(void (*pp_rst_stats_func) (int, void *), void *arg, uint16_t, uint32_t);
typedef void (*AddPreprocReassemblyPktFunc)(void * (*pp_reass_pkt_func)(void), uint32_t);
typedef int (*SetPreprocReassemblyPktBitFunc)(void *, uint32_t);
typedef void (*DisablePreprocessorsFunc)(void *);
typedef char** (*DynamicGetHttpXffFieldsFunc)(int* nFields);
#ifdef TARGET_BASED
typedef int16_t (*FindProtocolReferenceFunc)(const char *);
typedef int16_t (*AddProtocolReferenceFunc)(const char *);
#if defined(FEAT_OPEN_APPID)
typedef const char * (*FindProtocolNameFunc)(int16_t);
#endif /* defined(FEAT_OPEN_APPID) */
typedef int (*IsAdaptiveConfiguredFunc)(void);
typedef int (*IsAdaptiveConfiguredForSnortConfigFunc)(struct _SnortConfig *);
#endif
typedef void (*IP6BuildFunc)(void *, const void *, int);
#define SET_CALLBACK_IP 0
#define SET_CALLBACK_ICMP_ORIG 1
typedef void (*IP6SetCallbacksFunc)(void *, int, char);
typedef void (*AddKeywordOverrideFunc)(struct _SnortConfig *, char *, char *, PreprocOptionInit,
        PreprocOptionEval, PreprocOptionCleanup, PreprocOptionHash,
        PreprocOptionKeyCompare, PreprocOptionOtnHandler,
        PreprocOptionFastPatternFunc);
typedef void (*AddKeywordByteOrderFunc)(char *, PreprocOptionByteOrderFunc);

typedef int (*IsPreprocEnabledFunc)(struct _SnortConfig *, uint32_t);

typedef char * (*PortArrayFunc)(char *, PortObject *, int *);

typedef int (*AlertQueueLog)(void *);
typedef void (*AlertQueueControl)(void);  /* reset, push, and pop */
typedef void (*SetPolicyFunc)(struct _SnortConfig *, tSfPolicyId);
typedef tSfPolicyId (*GetPolicyFromIdFunc)(uint16_t );
typedef void (*ChangePolicyFunc)(tSfPolicyId, void *p);
typedef void (*SetFileDataPtrFunc)(uint8_t *,uint16_t );
typedef void (*DetectResetFunc)(uint8_t *,uint16_t );
typedef void (*SetAltDecodeFunc)(uint16_t );
typedef void (*DetectFlagEnableFunc)(SFDetectFlagType);
typedef long (*DynamicStrtol)(const char *, char **, int);
typedef unsigned long(*DynamicStrtoul)(const char *, char **, int);
typedef const char* (*DynamicStrnStr)(const char *, int, const char *);
typedef const char* (*DynamicStrcasestr)(const char *, int, const char *);
typedef int (*DynamicStrncpy)(char *, const char *, size_t );
typedef const char* (*DynamicStrnPbrk)(const char *, int , const char *);

typedef int (*EvalRTNFunc)(void *rtn, void *p, int check_ports);

typedef void* (*EncodeNew)(void);
typedef void (*EncodeDelete)(void*);
typedef void (*EncodeUpdate)(void*);
typedef int (*EncodeFormat)(uint32_t, const void*, void*, int);

typedef void* (*NewGrinderPktPtr)(void *, void *, uint8_t *);
typedef void (*DeleteGrinderPktPtr)(void*);
typedef bool (*PafEnabledFunc)(void);
typedef time_t (*SCPacketTimeFunc)(void);
typedef void (*SCGetPktTimeOfDay)(struct timeval *tv);

#ifdef SIDE_CHANNEL
typedef bool (*SCEnabledFunc)(void);
typedef int (*SCRegisterRXHandlerFunc)(uint16_t type, SCMProcessMsgFunc processMsgFunc, void *data);
typedef int (*SCPreallocMessageTXFunc)(uint32_t length, SCMsgHdr **hdr, uint8_t **msg_ptr, void **msg_handle);
typedef int (*SCEnqueueMessageTXFunc)(SCMsgHdr *hdr, const uint8_t *msg, uint32_t length, void *msg_handle, SCMQMsgFreeFunc msgFreeFunc);
#endif



typedef char* (*GetLogDirectory)(void);

typedef int (*ControlSocketRegisterHandlerFunc)(uint16_t, OOBPreControlFunc, IBControlFunc,
                                                OOBPostControlFunc);

typedef int (*RegisterIdleHandler)(IdleProcessingHandler);
#ifdef ACTIVE_RESPONSE
#define SND_BLK_RESP_FLAG_DO_CLIENT 1
#define SND_BLK_RESP_FLAG_DO_SERVER 2
typedef void (*DynamicSendBlockResponse)(void *packet, const uint8_t* buffer, uint32_t buffer_len, unsigned flags);
typedef void (*ActiveInjectDataFunc)(void *, uint32_t, const uint8_t *, uint32_t);
typedef void (*ActiveSendForwardResetFunc)(void *);
typedef void (*ActiveResponseFunc )(void *, const uint8_t *, uint32_t , uint32_t);
// NOTE: DynamicActive_ResponseFunc must match func ptr def Active_ResponseFunc in active.h
typedef void (*DynamicActive_ResponseFunc)(Packet *packet, void* data);
typedef int  (*ActiveQueueResponseFunc )(DynamicActive_ResponseFunc cb, void *);
#endif
typedef int (*DynamicSetFlowId)(const void* p, uint32_t id);
#ifdef HAVE_DAQ_EXT_MODFLOW
typedef int (*DynamicModifyFlow)(const DAQ_PktHdr_t *hdr, const DAQ_ModFlow_t* mod);
#endif
#ifdef HAVE_DAQ_QUERYFLOW
typedef int (*DynamicQueryFlow)(const DAQ_PktHdr_t *hdr, DAQ_QueryFlow_t* query);
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 8
typedef void (*DynamicDebugPkt)(uint8_t moduleId, uint8_t logLevel, const DAQ_Debug_Packet_Params_t *params, const char *msg, ...);
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
typedef int (*DynamicIoctl)(unsigned int type, char *buffer, size_t *len);
#endif

typedef int (*DynamicIsStrEmpty)(const char * );
typedef void (*AddPeriodicCheck)(void (*pp_check_func) (int, void *), void *arg, uint16_t, uint32_t, uint32_t);
typedef void (*AddPostConfigFuncs)(struct _SnortConfig *, void (*pp_post_config_func) (struct _SnortConfig *, void *), void *arg);
typedef int (*AddOutPutModule)(const char *filename);
typedef int (*CanWhitelist)(void);

#if defined(DAQ_CAPA_CST_TIMEOUT)
typedef bool (*CanGetTimeout)(void);
typedef void (*GetDaqCapaTimeOutFunc)(bool);
typedef void (*RegisterGetDaqCapaTimeoutFunc)(GetDaqCapaTimeOutFunc);
GetDaqCapaTimeOutFunc getDaqCapaTimeoutFnPtr;
#endif

typedef uint32_t (*GetCapability)(void);
typedef void (*DisableAllPoliciesFunc)(struct _SnortConfig *);
typedef int (*ReenablePreprocBitFunc)(struct _SnortConfig *, unsigned int preproc_id);
typedef int (*DynamicCheckValueInRangeFunc)(const char *, char *,
        unsigned long lo, unsigned long hi, unsigned long *value);
typedef bool (*DynamicReadyForProcessFunc) (void* pkt);
typedef int (*SslAppIdLookupFunc)(void * ssnptr, const char * serverName, const char * commonName, int32_t *serviceAppId, int32_t *clientAppId, int32_t *payloadAppId);
typedef void (*RegisterSslAppIdLookupFunc)(SslAppIdLookupFunc);

typedef int32_t (*GetAppIdFunc)(void *ssnptr);
typedef void (*RegisterGetAppIdFunc)(GetAppIdFunc);

typedef struct urlQueryContext* (*UrlQueryCreateFunc)(const char *url);
typedef void (*UrlQueryDestroyFunc)(struct urlQueryContext *context);
typedef int  (*UrlQueryMatchFunc)(void *ssnptr, struct urlQueryContext *context, uint16_t inUrlCat, uint16_t inUrlMinRep, uint16_t inUrlMaxRep);
typedef void (*RegisterUrlQueryFunc)(UrlQueryCreateFunc, UrlQueryDestroyFunc,UrlQueryMatchFunc);

typedef int (*UserGroupIdGetFunc)(void *ssnptr, uint32_t *userId, uint32_t *realmId, unsigned *groupIdArray, unsigned groupIdArrayLen);
typedef void (*RegisterUserGroupIdGetFunc)(UserGroupIdGetFunc);

typedef int (*GeoIpAddressLookupFunc)(const sfaddr_t *snortIp, uint16_t *geo);
typedef void (*RegisterGeoIpAddressLookupFunc)(GeoIpAddressLookupFunc);

typedef void (*UpdateSSLSSnLogDataFunc)(void *ssnptr, uint8_t logging_on, uint8_t action_is_block, const char *ssl_cert_fingerprint,
            uint32_t ssl_cert_fingerprint_len, uint32_t ssl_cert_status, uint8_t *ssl_policy_id,
            uint32_t ssl_policy_id_len, uint32_t ssl_rule_id, uint16_t ssl_cipher_suite, uint8_t ssl_version,
            uint16_t ssl_actual_action, uint16_t ssl_expected_action, uint32_t ssl_url_category,
            uint16_t ssl_flow_status, uint32_t ssl_flow_error, uint32_t ssl_flow_messages,
            uint64_t ssl_flow_flags, char *ssl_server_name, uint8_t *ssl_session_id, uint8_t session_id_len,
            uint8_t *ssl_ticket_id, uint8_t ticket_id_len);
typedef void (*RegisterUpdateSSLSSnLogDataFunc)(UpdateSSLSSnLogDataFunc);

typedef void (*EndSSLSSnLogDataFunc)(void *ssnptr, uint32_t ssl_flow_messages, uint64_t ssl_flow_flags) ;
typedef void (*RegisterEndSSLSSnLogDataFunc)(EndSSLSSnLogDataFunc);

typedef int (*GetSSLActualActionFunc)(void *ssnptr, uint16_t *action);
typedef void (*RegisterGetSSLActualActionFunc)(GetSSLActualActionFunc);

typedef void (*GetIntfDataFunc)(void *ssnptr,int32_t *ingressIntfIndex, int32_t *egressIntfIndex,
                int32_t *ingressZoneIndex, int32_t *egressZoneIndex) ;
typedef void (*RegisterGetIntfDataFunc)(GetIntfDataFunc);

typedef void (*SetTlsHostAppIdFunc)(void *ssnptr, const char *serverName, const char *commonName,
            const char *orgName, const char *subjectAltName, bool isSniMismatch,
            int32_t *serviceAppId, int32_t *clientAppId, int32_t *payloadAppId);
typedef void (*RegisterSetTlsHostAppIdFunc)(SetTlsHostAppIdFunc);

//
// SSL Callbacks
//
typedef bool (*DynamicIsSSLPolicyEnabledFunc)(struct _SnortConfig *sc);
typedef void (*DynamicSetSSLPolicyEnabledFunc)(struct _SnortConfig *sc, tSfPolicyId policy, bool value);
typedef void (*SetSSLCallbackFunc)(void *);
typedef void* (*GetSSLCallbackFunc)(void);

typedef int (*_LoadLibraryFunc)(struct _SnortConfig *sc, const char * const path, int indent);
typedef void (*LoadAllLibsFunc)(struct _SnortConfig *sc, const char * const path, _LoadLibraryFunc loadFunc);
typedef void * _PluginHandle;
typedef _PluginHandle (*OpenDynamicLibraryFunc)(const char * const library_name, int useGlobal);
typedef void (*_dlsym_func)(void);
typedef _dlsym_func (*GetSymbolFunc)(_PluginHandle handle, char * symbol, DynamicPluginMeta * meta, int fatal);
typedef void (*CloseDynamicLibraryFunc)(_PluginHandle handle);

#if defined(FEAT_OPEN_APPID)
typedef bool (*IsAppIdRequiredFunc)(void);
typedef void (*RegisterIsAppIdRequiredFunc)(IsAppIdRequiredFunc);
typedef void (*UnregisterIsAppIdRequiredFunc)(IsAppIdRequiredFunc);
struct AppIdApi;
#endif /* defined(FEAT_OPEN_APPID) */

typedef bool (*ReadModeFunc)(void);

typedef int (*GetPerfIndicatorsFunc)(void *Request);

typedef uint32_t (*GetSnortPacketLatencyFunc)(void);

typedef double (*GetSnortPacketDropPortionFunc)(void);

typedef bool (*IsTestModeFunc)(void);

typedef struct _SnortConfig* (*GetCurrentSnortConfigFunc)(void);

typedef void (*AddPktTraceDataFunc)(int module, int traceLen);

typedef const char* (*GetPktTraceActionMsgFunc)();

#ifdef SNORT_RELOAD
typedef int (*ReloadAdjustRegisterFunc)(struct _SnortConfig* sc, const char* raName,
                                        tSfPolicyId raPolicyId, ReloadAdjustFunc raFunc,
                                        void *raUserData, ReloadAdjustUserFreeFunc raUserFreeFunc); 
#endif

typedef int (*DynamicSetPreserveFlow)(const void* p);

// IPrep Last update count
typedef void (*IprepUpdateCountFunc)(uint8_t);

typedef int (*RegisterMemoryStatsFunc)(uint preproc,
                                       int (*MemoryStatsDisplayFunc)(FILE *fd,
                                                                     char *buffer,
                                                                     PreprocMemInfo *meminfo));

typedef void* (*SnortAllocFunc)(int num, unsigned long size, uint32_t preproc, uint32_t data);

typedef void (*SnortFreeFunc)(void * ptr, uint32_t size, uint32_t preproc, uint32_t data);
typedef bool (*ReputationProcessExternalIpFunc)(void *p, sfaddr_t* ip);
typedef void (*RegisterReputationProcessExternalFunc)(ReputationProcessExternalIpFunc);
typedef int (*ReputationGetEntryCountFunc)(void);
typedef void (*RegisterReputationGetEntryCountFunc)(ReputationGetEntryCountFunc);
/* FTP data transfer mode */
typedef bool (*ftpGetModefunc)(void *ssnptr);
typedef void (*RegisterFtpQueryModefunc)(ftpGetModefunc);
typedef void (*LogMsgThrottled)(void*, const char *, ...);

#define ENC_DYN_FWD 0x80000000
#define ENC_DYN_NET 0x10000000

/* Info Data passed to dynamic preprocessor plugin must include:
 * version
 * Pointer to AltDecodeBuffer
 * Pointer to HTTP URI Buffers
 * Pointer to functions to log Messages, Errors, Fatal Errors
 * Pointer to function to add preprocessor to list of configure Preprocs
 * Pointer to function to regsiter preprocessor configuration keyword
 * Pointer to function to create preprocessor alert
 */
typedef struct _DynamicPreprocessorData
{
    int version;
    int size;

    SFDataBuffer *altBuffer;
    SFDataPointer *altDetect;
    SFDataPointer *fileDataBuf;

    LogMsgFunc logMsg;
    LogMsgFunc errMsg;
    LogMsgFunc fatalMsg;
    DebugMsgFunc debugMsg;
    LogMsgThrottled errMsgThrottled;

    PreprocRegisterFunc registerPreproc;
#ifdef SNORT_RELOAD
    GetRelatedReloadDataFunc getRelatedReloadData;
#endif
    AddPreprocFunc addPreproc;
    AddPreprocFunc addPreprocAllPolicies;
    GetSnortInstance getSnortInstance;
    AddPreprocExit addPreprocExit;
    AddPreprocConfCheck addPreprocConfCheck;
    RegisterPreprocRuleOpt preprocOptRegister;
    AddPreprocProfileFunc addPreprocProfileFunc;
    ProfilingFunc profilingPreprocsFunc;
    void *totalPerfStats;

    AlertQueueAdd alertAdd;
    GenSnortEvent genSnortEvent;
    ThresholdCheckFunc thresholdCheck;
#ifdef ACTIVE_RESPONSE
    ActiveEnableFunc activeSetEnabled;
#endif

    DetectFunc detect;
    DisableDetectFunc disableDetect;
    DisableDetectFunc disableAllDetect;
    DisableDetectFunc disablePacketAnalysis;
    EnableDetectFunc  enableContentDetect;
    EnablePreprocessorFunc enablePreprocessor;

    SessionAPI *sessionAPI;
    StreamAPI *streamAPI;
    SearchAPI *searchAPI;

    char **config_file;
    int *config_line;
    printfappendfunc printfappend;
    TokenSplitFunc tokenSplit;
    TokenFreeFunc tokenFree;

    GetRuleInfoByNameFunc getRuleInfoByName;
    GetRuleInfoByIdFunc getRuleInfoById;
#ifdef SF_WCHAR
    DebugWideMsgFunc debugWideMsg;
#endif

    PreprocessFunc preprocess;
#ifdef DUMP_BUFFER
    BufferDumpRegisterFunc registerBufferTracer;
#endif
    char **debugMsgFile;
    int *debugMsgLine;

    PreprocStatsRegisterFunc registerPreprocStats;
    AddPreprocReset addPreprocReset;
    AddPreprocResetStats addPreprocResetStats;
    DisablePreprocessorsFunc disablePreprocessors;

    IP6BuildFunc ip6Build;
    IP6SetCallbacksFunc ip6SetCallbacks;

    AlertQueueLog logAlerts;
    AlertQueueControl resetAlerts;
    AlertQueueControl pushAlerts;
    AlertQueueControl popAlerts;

#ifdef TARGET_BASED
    FindProtocolReferenceFunc findProtocolReference;
    AddProtocolReferenceFunc addProtocolReference;
    IsAdaptiveConfiguredFunc isAdaptiveConfigured;
    IsAdaptiveConfiguredForSnortConfigFunc isAdaptiveConfiguredForSnortConfig;
#endif

    AddKeywordOverrideFunc preprocOptOverrideKeyword;
    AddKeywordByteOrderFunc preprocOptByteOrderKeyword;
    IsPreprocEnabledFunc isPreprocEnabled;

    PortArrayFunc portObjectCharPortArray;

    GetPolicyFunc getNapRuntimePolicy;
    GetPolicyFunc getIpsRuntimePolicy;
    GetParserPolicyFunc getParserPolicy;
    GetPolicyFunc getDefaultPolicy;
    SetPolicyFunc setParserPolicy;
    SetFileDataPtrFunc setFileDataPtr;
    DetectResetFunc DetectReset;
    SetAltDecodeFunc SetAltDecode;
    GetAltDetectFunc GetAltDetect;
    SetAltDetectFunc SetAltDetect;
    IsDetectFlagFunc Is_DetectFlag;
    DetectFlagDisableFunc DetectFlag_Disable;
    DynamicStrtol SnortStrtol;
    DynamicStrtoul SnortStrtoul;
    DynamicStrnStr SnortStrnStr;
    DynamicStrncpy SnortStrncpy;
    DynamicStrnPbrk SnortStrnPbrk;
    DynamicStrcasestr SnortStrcasestr;
    EvalRTNFunc fpEvalRTN;

    ObfuscationApi *obApi;

    EncodeNew encodeNew;
    EncodeDelete encodeDelete;
    EncodeFormat encodeFormat;
    EncodeUpdate encodeUpdate;

    NewGrinderPktPtr newGrinderPkt;
    DeleteGrinderPktPtr deleteGrinderPkt;

    AddPreprocFunc addDetect;
    PafEnabledFunc isPafEnabled;
    SCPacketTimeFunc pktTime;
    SCGetPktTimeOfDay getPktTimeOfDay;
#ifdef SIDE_CHANNEL
    SCEnabledFunc isSCEnabled;
    SCRegisterRXHandlerFunc scRegisterRXHandler;
    SCPreallocMessageTXFunc scAllocMessageTX;
    SCEnqueueMessageTXFunc scEnqueueMessageTX;
#endif

    GetLogDirectory getLogDirectory;

    ControlSocketRegisterHandlerFunc controlSocketRegisterHandler;
    RegisterIdleHandler registerIdleHandler;

    GetPolicyFromIdFunc getPolicyFromId;
    ChangePolicyFunc changeNapRuntimePolicy;
    ChangePolicyFunc changeIpsRuntimePolicy;
    InlineDropFunc inlineDropPacket;
    InlineDropFunc inlineForceDropPacket;
    InlineDropFunc inlineDropSessionAndReset;
    InlineDropFunc inlineForceDropSession;
    InlineDropFunc inlineForceDropSessionAndReset;
    ActivePacketWasDroppedFunc active_PacketWasDropped;
    InlineRetryFunc inlineRetryPacket;
    DynamicIsStrEmpty SnortIsStrEmpty;
    AddMetaEvalFunc addMetaEval;
#ifdef ACTIVE_RESPONSE
    DynamicSendBlockResponse dynamicSendBlockResponse;
#endif
    DynamicSetFlowId dynamicSetFlowId;
#ifdef HAVE_DAQ_EXT_MODFLOW
    DynamicModifyFlow dynamicModifyFlow;
#endif
#ifdef HAVE_DAQ_QUERYFLOW
    DynamicQueryFlow dynamicQueryFlow;
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 8
    DynamicDebugPkt dynamicDebugPkt;
#endif

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
    DynamicIoctl dynamicIoctl;
#endif

    AddPeriodicCheck addPeriodicCheck;
    AddPostConfigFuncs addPostConfigFunc;
    AddToPostConfList addFuncToPostConfigList;
    char **snort_conf_dir;
    AddOutPutModule addOutputModule;
    CanWhitelist canWhitelist;
    FileAPI *fileAPI;
    DisableAllPoliciesFunc disableAllPolicies;
    ReenablePreprocBitFunc reenablePreprocBit;
    DynamicCheckValueInRangeFunc checkValueInRange;

    SetHttpBufferFunc setHttpBuffer;
    GetHttpBufferFunc getHttpBuffer;

#ifdef ACTIVE_RESPONSE
    ActiveInjectDataFunc activeInjectData;
    ActiveResponseFunc activeSendResponse;
    ActiveSendForwardResetFunc activeSendForwardReset;
    ActiveQueueResponseFunc activeQueueResponse;
#endif
    GetSSLCallbackFunc getSSLCallback;
    SetSSLCallbackFunc setSSLCallback;
    SslAppIdLookupFunc         sslAppIdLookup;
    RegisterSslAppIdLookupFunc registerSslAppIdLookup;

    GetAppIdFunc getAppId;
    RegisterGetAppIdFunc registerGetAppId;

    UrlQueryCreateFunc urlQueryCreate;
    UrlQueryDestroyFunc urlQueryDestroy;
    UrlQueryMatchFunc urlQueryMatch;
    RegisterUrlQueryFunc registerUrlQuery;

    UserGroupIdGetFunc userGroupIdGet;
    RegisterUserGroupIdGetFunc registerUserGroupIdGet;

    GeoIpAddressLookupFunc geoIpAddressLookup;
    RegisterGeoIpAddressLookupFunc registerGeoIpAddressLookup;

    UpdateSSLSSnLogDataFunc updateSSLSSnLogData;
    RegisterUpdateSSLSSnLogDataFunc registerUpdateSSLSSnLogData;

    EndSSLSSnLogDataFunc endSSLSSnLogData;
    RegisterEndSSLSSnLogDataFunc registerEndSSLSSnLogData;

    GetSSLActualActionFunc getSSLActualAction;
    RegisterGetSSLActualActionFunc registerGetSSLActualAction;

    GetIntfDataFunc getIntfData;
    RegisterGetIntfDataFunc registerGetIntfData;
    DynamicReadyForProcessFunc readyForProcess;
    DynamicIsSSLPolicyEnabledFunc isSSLPolicyEnabled;
    DynamicSetSSLPolicyEnabledFunc setSSLPolicyEnabled;

    /* Preproc's fetch Snort performance indicators.  Used by IAB. */
    GetPerfIndicatorsFunc getPerfIndicators;
    GetSnortPacketLatencyFunc getPacketLatency;
    GetSnortPacketDropPortionFunc getPacketDropPortion;

    LoadAllLibsFunc loadAllLibs;
    OpenDynamicLibraryFunc openDynamicLibrary;
    GetSymbolFunc getSymbol;
    CloseDynamicLibraryFunc closeDynamicLibrary;

    DynamicGetHttpXffFieldsFunc getHttpXffFields;

#if defined(FEAT_OPEN_APPID)
    struct AppIdApi *appIdApi;
    RegisterIsAppIdRequiredFunc registerIsAppIdRequired;
    UnregisterIsAppIdRequiredFunc unregisterIsAppIdRequired;
    IsAppIdRequiredFunc isAppIdRequired;
#endif /* defined(FEAT_OPEN_APPID) */
    ReadModeFunc isReadMode;
    IsTestModeFunc isTestMode;
    GetCurrentSnortConfigFunc getCurrentSnortConfig;
    bool *pkt_tracer_enabled;
    char *trace;
    uint32_t traceMax;
    AddPktTraceDataFunc addPktTrace;
    GetPktTraceActionMsgFunc getPktTraceActionMsg;

#ifdef SNORT_RELOAD
    ReloadAdjustRegisterFunc reloadAdjustRegister;
#endif

#ifdef DAQ_MODFLOW_TYPE_PRESERVE_FLOW
    DynamicSetPreserveFlow setPreserveFlow;
#endif
    IprepUpdateCountFunc setIPRepUpdateCount;
    RegisterMemoryStatsFunc registerMemoryStatsFunc;
    SnortAllocFunc snortAlloc;
    SnortFreeFunc snortFree;
#if defined(DAQ_CAPA_CST_TIMEOUT)
    CanGetTimeout canGetTimeout;
    RegisterGetDaqCapaTimeoutFunc registerGetDaqCapaTimeout;
#endif
    GetCapability getCapability;

    ReputationGetEntryCountFunc reputation_get_entry_count;
    RegisterReputationGetEntryCountFunc registerReputationGetEntryCount;
    ReputationProcessExternalIpFunc reputation_process_external_ip;
    RegisterReputationProcessExternalFunc registerReputationProcessExternal;
    RegisterFtpQueryModefunc registerFtpmodeQuery;
    ftpGetModefunc ftpGetMode;
    
    SetTlsHostAppIdFunc setTlsHostAppId;
    RegisterSetTlsHostAppIdFunc registerSetTlsHostAppId;
    SmtpAPI *smtpApi;
} DynamicPreprocessorData;

/* Function prototypes for Dynamic Preprocessor Plugins */
void CloseDynamicPreprocessorLibs(void);
int LoadDynamicPreprocessor(struct _SnortConfig *sc, const char * const library_name, int indent);
void LoadAllDynamicPreprocessors(struct _SnortConfig *sc, const char * const path);
typedef int (*InitPreprocessorLibFunc)(DynamicPreprocessorData *);

int InitDynamicPreprocessors(void);
void RemoveDuplicatePreprocessorPlugins(void);

/* This was necessary because of static code analysis not recognizing that
 * fatalMsg did not return - use instead of fatalMsg
 */
NORETURN void DynamicPreprocessorFatalMessage(const char *format, ...);

extern DynamicPreprocessorData _dpd;
#endif /* _SF_DYNAMIC_PREPROCESSOR_H_ */
