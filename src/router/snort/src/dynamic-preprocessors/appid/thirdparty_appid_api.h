/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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

#ifndef _THIRDPARTY_APPID_API_H_
#define _THIRDPARTY_APPID_API_H_

#include <stdint.h>

#include "appId.h"
#include "appIdApi.h"
#include "sf_snort_packet.h"
#include "thirdparty_appid_types.h"

#define THIRD_PARTY_APP_ID_API_VERSION 1

#define TP_PATH_MAX 4096

struct ThirdPartyConfig
{
    unsigned chp_body_collection_max;
    unsigned ftp_userid_disabled:1;
    unsigned chp_body_collection_disabled:1;
    unsigned tp_allow_probes:1;
    unsigned http_upgrade_reporting_enabled:1;
    char     tp_config_path[TP_PATH_MAX];
    int      numXffFields;
    char**   xffFields;
    int      oldNumXffFields;
    char**   oldXffFields;
};

struct ThirdPartyUtils
{
    void (*logMsg)(const char *, ...);
    uint32_t (*getSnortInstance)(void);
};

typedef int (*ThirdPartyAppIDModInit)(struct ThirdPartyConfig* config,
                                      struct ThirdPartyUtils* utils);
typedef int (*ThirdPartyAppIDModReconfigure)(struct ThirdPartyConfig* config);
typedef int (*ThirdPartyAppIDModFini)(void);
typedef void* (*ThirdPartyAppIDSessionCreate)(void);
typedef int (*ThirdPartyAppIDSessionDelete)(void* tpsession, int just_reset_state);
typedef int (*ThirdPartyAppIDSessionProcess)(void* tpsession,                                   // in
                                             SFSnortPacket* pkt,                                // in
                                             int direction,                                     // in
                                             tAppId* appId,                                     // out
                                             int* confidence,                                   // out
                                             tAppId** proto_list,                               // out
                                             ThirdPartyAppIDAttributeData** attribute_data);    // out
typedef int (*ThirdPartyAppIDPrintStats)(void);
typedef int (*ThirdPartyAppIDResetStats)(void);
typedef int (*ThirdPartyAppIDDisableFlags)(void* tpsession, uint32_t session_flags);

typedef TPState (*ThirdPartyAppIDSessionStateGet)(void* tpsession);
typedef void (*ThirdPartyAppIDSessionStateSet)(void* tpsession, TPState);
typedef void (*ThirdPartyAppIDSessionAttrSet)(void* tpsession, TPSessionAttr attr);
typedef void (*ThirdPartyAppIDSessionAttrClear)(void* tpsession, TPSessionAttr attr);
typedef unsigned (*ThirdPartyAppIDSessionAttrGet)(void* tpsession, TPSessionAttr attr);
typedef tAppId (*ThirdPartyAppIDSessionCurrentAppIdGet)(void* tpsession);

// SF_SO_PUBLIC const ThirdPartyAppIDModule thirdparty_appid_impl_module
typedef struct _ThirdPartyAppIDModule
{
    const uint32_t api_version;
    const char* module_name;
    ThirdPartyAppIDModInit init;
    ThirdPartyAppIDModReconfigure reconfigure;
    ThirdPartyAppIDModFini fini;
    ThirdPartyAppIDSessionCreate session_create;
    ThirdPartyAppIDSessionDelete session_delete;
    ThirdPartyAppIDSessionProcess session_process;
    ThirdPartyAppIDPrintStats print_stats;
    ThirdPartyAppIDResetStats reset_stats;
    ThirdPartyAppIDDisableFlags disable_flags;

    ThirdPartyAppIDSessionStateGet session_state_get;
    ThirdPartyAppIDSessionStateSet session_state_set;
    ThirdPartyAppIDSessionAttrSet session_attr_set;
    ThirdPartyAppIDSessionAttrClear session_attr_clear;
    ThirdPartyAppIDSessionAttrGet session_attr_get;
    ThirdPartyAppIDSessionCurrentAppIdGet session_appid_get;
} ThirdPartyAppIDModule;

#endif
