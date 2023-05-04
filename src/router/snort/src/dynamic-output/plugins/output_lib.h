/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2012-2013 Sourcefire, Inc.
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
** Date: 01-27-2012
** Author: Hui Cao <hcao@sourcefire.com>
*/

#ifndef _OUTPUT_UTIL_H
#define _OUTPUT_UTIL_H

#include <stdio.h>
#include "output_common.h"
#include "sf_dynamic_common.h"
#include "sfPolicy.h"
#include "obfuscation.h"
#include "session_api.h"
#include "stream_api.h"
#include <daq.h>

#define OUTPUT_DATA_MAJOR_VERSION 2
#define OUTPUT_DATA_MINOR_VERSION 2

typedef int (*OutputInitFunc)(void*);

typedef void (*LogFunc)(void*, void *);
typedef void (*LogDataFunc)(void*, int);
typedef void (*LogIPFunc)(void*, int, void*);
typedef int  (*ActiveFunc)(void);
typedef char* (*getConfigFunc)(void);
typedef int (*getConfigValueFunc)(void);
typedef int (*TextLog_FlushFunc)(void*);
typedef void (*TextLog_TermFunc) (void* );
typedef int (*TextLog_PrintFunc)(void*, const char* format, ...);
typedef int (*TextLog_PrintUnicodeFunc)(void*, uint8_t *, uint32_t, uint8_t);
typedef int (*GetConfigValueFunc)(void);
typedef int (*TextLog_NewLineFunc) (void* );
typedef int (*TextLog_PutcFunc)(void*, char);
typedef int (*TextLog_PutsFunc)(void*, char*);
typedef int (*TextLog_WriteFunc)(void*, char*, int);
typedef void* (*TextLog_InitFunc)(const char* name, unsigned int maxBuf, size_t maxFile);
typedef void (*OutputExecFunc)(void *packet, char *msg, void *arg, void *event);
typedef void (*AddFuncToOutputListFunc)(struct _SnortConfig *, OutputExecFunc o_func, int type, void *arg);

typedef void (*PluginFuncWithSnortConfig)(struct _SnortConfig *, int, void *);
typedef void (*AddFuncWithSnortConfigToPluginListWithSnortConfigFunc)(struct _SnortConfig *, PluginFuncWithSnortConfig, void *arg);
typedef void (*AddFuncWithSnortConfigToPluginListFunc)(PluginFuncWithSnortConfig, void *arg);
typedef void (*PluginFunc)(int, void *);
typedef void (*AddFuncToPluginListFunc)(PluginFunc, void *arg);

typedef char *(*SnortStrdupFunc)(const char *str);
typedef int (*SnortSnprintfFunc)(char *buf, size_t buf_size, const char *format, ...);
typedef const char* (*GetBasePolicyVersionFunc) (void);
typedef const char* (*GetTargetPolicyVersionFunc) (tSfPolicyId);

typedef const char* (*GetDAQInterfaceSpecFunc)(void);
typedef DAQ_Mode (*GetDAQInterfaceMode)(const DAQ_PktHdr_t *h);
typedef const char* (*GetProtocolNameFunc)(void*);
typedef int (*GetVlanIdFunc)(void*);
typedef int (*GetDAQBaseProtocolFunc) (void);
typedef char ** (*MSplitFunc)(const char *str, const char *sep_chars, const int max_toks,
               int *num_toks, const char meta_char);
typedef void (*MSplitFreeFunc)(char ***pbuf, int num_toks);

/* Info Data passed to dynamic output plugin
 */
typedef struct _DynamicOutputData
{
    int majorVersion;
    int minorVersion;
    unsigned int size;

    getConfigFunc getLogDirectory;
    getConfigFunc getAlertFile;
    getConfigValueFunc getTestMode;
    getConfigValueFunc getLogIPv6Extra;
    LogDataFunc logPriorityData;
    LogDataFunc logXrefs;
    LogIPFunc logIPPkt;
    LogFunc logTimeStamp;
    LogFunc logTrHeader;
    LogFunc log2ndHeader;
    LogFunc logIpAddrs;
    LogFunc logIPHeader;
    LogFunc logTCPHeader;
    LogFunc logUDPHeader;
    LogFunc logICMPHeader;
    LogFunc logArpHeader;

    TextLog_InitFunc textLog_Init;
    TextLog_TermFunc textLog_Term;
    TextLog_PutcFunc textLog_Putc;
    TextLog_PutsFunc textLog_Quote;
    TextLog_WriteFunc textLog_Write;
    TextLog_PrintFunc textLog_Print;
    TextLog_PrintUnicodeFunc textLog_PrintUnicode;
    TextLog_FlushFunc textLog_Flush;
    TextLog_NewLineFunc textLog_NewLine;
    TextLog_PutsFunc textLog_Puts;


    ActiveFunc active_PacketWasDropped;
    ActiveFunc active_PacketWouldBeDropped;
    GetConfigValueFunc ScOutputAppData;
    GetConfigValueFunc ScAlertInterface;
    GetConfigValueFunc ScNoOutputTimestamp;

    AddFuncToOutputListFunc addOutputModule;
    AddFuncToPluginListFunc addCleanExit;
#ifdef SNORT_RELOAD
    AddFuncWithSnortConfigToPluginListFunc addReload;
#endif
    AddFuncWithSnortConfigToPluginListWithSnortConfigFunc addPostconfig;

    GetDAQInterfaceSpecFunc getDAQinterface;
    GetDAQBaseProtocolFunc getDAQBaseProtocol;
    GetProtocolNameFunc getProtocolName;
    GetVlanIdFunc getVlanId;
    MSplitFunc mSplit;
    MSplitFreeFunc mSplitFree;
#ifdef WIN32
    char *(*print_interface)(const char *);
#endif

    /*shared with dpd*/
    char **config_file;
    int *config_line;
    LogMsgFunc logMsg;
    LogMsgFunc errMsg;
    LogMsgFunc fatalMsg;
    DebugMsgFunc debugMsg;
    SnortStrdupFunc SnortStrdup;
    SnortSnprintfFunc SnortSnprintf;
    GetPolicyFunc getNapRuntimePolicy;
    GetPolicyFunc getIpsRuntimePolicy;
    GetParserPolicyFunc getParserPolicy;
    GetPolicyFunc getDefaultPolicy;
    GetBasePolicyVersionFunc getBasePolicyVersion;
    GetTargetPolicyVersionFunc getTargetPolicyVersion;
    ObfuscationApi *obApi;
    SessionAPI **sessionAPI;
    StreamAPI **streamAPI;
    GetDAQInterfaceMode getDAQInterfaceMode;

} DynamicOutputData;

extern DynamicOutputData _dod;


OUTPUT_SO_PUBLIC int initOutputPlugins(void *dod);

#endif /* _OUTPUT_UTIL_H */
