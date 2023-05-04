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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sf_types.h"
#include "output.h"
#include "output_api.h"
#include "output_lib.h"
#include "active.h"
#include "log_text.h"
#include "parser.h"
#include "sfdaq.h"
#include "snort.h"
#include "util.h"
#include "mstring.h"
#include "decode.h"
#include "plugbase.h"

extern char *file_name;
extern int file_line;


char *GetLogDir(void)
{
   return snort_conf->log_dir;
}

char *GetAlertFile(void)
{
   return snort_conf->alert_file;
}

int GetLogIPv6Extra(void)
{
   return snort_conf->log_ipv6_extra;
}

const char *GetDAQInterface(void)
{
   return (PRINT_INTERFACE(DAQ_GetInterfaceSpec()));
}

const char *GetProtocolName(void* p)
{
    Packet *packet = (Packet *)p;
   return (protocol_names[GET_IPH_PROTO(packet)]);
}

int GetVlanId(void* p)
{
   Packet *packet = (Packet *)p;
   return (VTH_VLAN(packet->vh));
}

tSfPolicyId GetParserPolicy(struct _SnortConfig *sc)
{
    return getParserPolicy(sc);
}

tSfPolicyId GetNapRuntimePolicy(void)
{
    return getNapRuntimePolicy();
}

tSfPolicyId GetIpsRuntimePolicy(void)
{
    return getIpsRuntimePolicy();
}

tSfPolicyId GetDefaultPolicy(void)
{
    return getDefaultPolicy();
}

const char *GetBasePolicyVersion(void)
{
    return snort_conf->base_version;
}

const char *GetTargetPolicyVersion(tSfPolicyId policy_id)
{
    SnortPolicy *policy = snort_conf->targeted_policies[policy_id];
    if(policy)
        return (policy->policy_version);
    else
        return NULL;
}

int initOutputPlugin(void *outputInit)
{
    DynamicOutputData outputData;

    outputData.majorVersion = OUTPUT_DATA_MAJOR_VERSION;
    outputData.minorVersion = OUTPUT_DATA_MINOR_VERSION;
    outputData.size = sizeof(DynamicOutputData);

    outputData.logTimeStamp = (LogFunc)LogTimeStamp;
    outputData.getLogDirectory = &GetLogDir;
    outputData.getAlertFile = &GetAlertFile;
    outputData.getTestMode = &ScTestMode;
    outputData.getLogIPv6Extra = &GetLogIPv6Extra;
    outputData.logPriorityData = (LogDataFunc)&LogPriorityData;
    outputData.logXrefs = (LogDataFunc) &LogXrefs;
    outputData.logIPPkt= (LogIPFunc) &LogIPPkt;
    outputData.logTimeStamp = (LogFunc)&LogTimeStamp;
#ifndef NO_NON_ETHER_DECODER
    outputData.logTrHeader = (LogFunc)&LogTrHeader;
    outputData.logArpHeader = (LogFunc)&LogArpHeader;
#endif
    outputData.log2ndHeader = (LogFunc)&Log2ndHeader;
    outputData.logIpAddrs = (LogFunc)&LogIpAddrs;
    outputData.logIPHeader = (LogFunc) &LogIPHeader;
    outputData.logTCPHeader = (LogFunc)&LogTCPHeader;
    outputData.logUDPHeader = (LogFunc)&LogUDPHeader;
    outputData.logICMPHeader = (LogFunc)&LogICMPHeader;

    outputData.textLog_Init = (TextLog_InitFunc)&TextLog_Init;
    outputData.textLog_Term = (TextLog_TermFunc)&TextLog_Term;
    outputData.textLog_Putc = (TextLog_PutcFunc)&TextLog_Putc;
    outputData.textLog_Quote = (TextLog_PutsFunc)&TextLog_Quote;
    outputData.textLog_Write = (TextLog_WriteFunc)&TextLog_Write;
    outputData.textLog_Print = (TextLog_PrintFunc)&TextLog_Print;
    outputData.textLog_PrintUnicode = (TextLog_PrintUnicodeFunc)&TextLog_PrintUnicode;
    outputData.textLog_Flush = (TextLog_FlushFunc)&TextLog_Flush;
    outputData.textLog_NewLine = (TextLog_NewLineFunc)&TextLog_NewLine;
    outputData.textLog_Puts = (TextLog_PutsFunc)&TextLog_Puts;

    outputData.active_PacketWasDropped = &Active_PacketWasDropped;
    outputData.active_PacketWouldBeDropped = &Active_PacketWouldBeDropped ;
    outputData.ScOutputAppData = &ScOutputAppData;
    outputData.ScAlertInterface = &ScAlertInterface;
    outputData.ScNoOutputTimestamp = &ScNoOutputTimestamp;

    outputData.addOutputModule = (AddFuncToOutputListFunc)&AddFuncToOutputList;
    outputData.addCleanExit = &AddFuncToCleanExitList;
#ifdef SNORT_RELOAD
    outputData.addReload = &AddFuncToReloadList;
#endif
    outputData.addPostconfig = &AddFuncToPostConfigList;

    outputData.config_file = &file_name;
    outputData.config_line = &file_line;
    outputData.logMsg = &LogMessage;
    outputData.errMsg = &ErrorMessage;
    outputData.fatalMsg = &FatalError;
    outputData.debugMsg = &DebugMessageFunc;
    outputData.SnortStrdup = &SnortStrdup;
    outputData.SnortSnprintf = &SnortSnprintf;
    outputData.getNapRuntimePolicy = GetNapRuntimePolicy;
    outputData.getIpsRuntimePolicy = GetIpsRuntimePolicy;
    outputData.getParserPolicy = GetParserPolicy;
    outputData.getDefaultPolicy = GetDefaultPolicy;
    outputData.getBasePolicyVersion = &GetBasePolicyVersion;
    outputData.getTargetPolicyVersion = &GetTargetPolicyVersion;
#ifdef WIN32
    outputData.print_interface = &print_interface;
#endif
    outputData.getDAQinterface = &GetDAQInterface;
    outputData.getDAQBaseProtocol = &DAQ_GetBaseProtocol;
    outputData.getProtocolName = &GetProtocolName;
    outputData.getVlanId = &GetVlanId;
    outputData.mSplit = &mSplit;
    outputData.mSplitFree = &mSplitFree;
    outputData.obApi = obApi;
    outputData.streamAPI = &stream_api;
    outputData.getDAQInterfaceMode = &DAQ_GetInterfaceMode;
    return(((OutputInitFunc)outputInit)(&outputData));

}

