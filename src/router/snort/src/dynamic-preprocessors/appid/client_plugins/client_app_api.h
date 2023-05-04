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


#ifndef __CLIENT_APP_API_H__
#define __CLIENT_APP_API_H__

#include "flow.h"
#include "commonAppMatcher.h"

struct _Detector;

// Forward declaration for AppId config. Cannot include appIdConfig.h because of
// circular dependency
struct appIdConfig_;

typedef enum {
    CLIENT_APP_SUCCESS = 0,
    CLIENT_APP_INPROCESS = 10,
    CLIENT_APP_ENULL = -10,
    CLIENT_APP_EINVALID = -11,
    CLIENT_APP_ENOMEM = -12
} CLIENT_APP_RETCODE;

typedef struct
{
    const char *name;
    SF_LIST items;
} RNAClientAppModuleConfig;

typedef struct
{
    const char *name;
    const char *value;
} RNAClientAppModuleConfigItem;

typedef CLIENT_APP_RETCODE (*RNAClientAppFCN)(const uint8_t *, uint16_t, const int,
                                              tAppIdData *session, SFSnortPacket *pkt, struct _Detector *userData,
                                              const struct appIdConfig_ *pConfig);

typedef int (*RNAClientAppCallbackFCN)(const uint8_t *, uint16_t, const int, tAppIdData *session,
                                       const SFSnortPacket *pkt, struct _Detector *userData,
                                       const struct appIdConfig_ *pConfig);

typedef struct _INIT_CLIENT_API
{
    void (*RegisterPattern)(RNAClientAppFCN fcn, uint8_t proto,
                            const uint8_t * const pattern, unsigned size, int position, struct appIdConfig_ *pConfig);
    void (*RegisterPatternEx)(RNAClientAppFCN fcn, uint8_t proto,
                            const uint8_t * const pattern, unsigned size, int position,
			      struct _Detector *userdata);
    void (*RegisterPatternNoCase)(RNAClientAppFCN fcn, uint8_t proto,
                                  const uint8_t * const pattern, unsigned size, int position,
                                  struct appIdConfig_ *pConfig);
    void (*RegisterAppId)(RNAClientAppFCN fcn, tAppId appId, uint32_t additionalInfo, struct appIdConfig_ *pConfig);
    void (*RegisterDetectorCallback)(RNAClientAppCallbackFCN fcn, tAppId appId, struct _Detector *userdata, struct appIdConfig_ *pConfig);
    int debug;
    uint32_t instance_id;
    struct appIdConfig_ *pAppidConfig;  ///< AppId context for which this API should be used
} InitClientAppAPI;

typedef struct _CLEAN_CLIENT_API
{
    struct appIdConfig_ *pAppidConfig;  ///< AppId context for which this API should be used
} CleanClientAppAPI;


typedef struct _FINALIZE_CLIENT_API
{
    void* data;
} FinalizeClientAppAPI;

typedef CLIENT_APP_RETCODE (*RNAClientAppInitFCN)(const InitClientAppAPI * const, SF_LIST *config);
typedef CLIENT_APP_RETCODE (*RNAClientAppFinalizeFCN)(const FinalizeClientAppAPI * const);
typedef void (*RNAClientAppCleanFCN)(const CleanClientAppAPI * const);

typedef void *(*ClientAppFlowdataGet)(tAppIdData *, unsigned);
typedef int (*ClientAppFlowdataAdd)(tAppIdData *, void *, unsigned, AppIdFreeFCN);
typedef void (*ClientAppAddApp)(SFSnortPacket *p, int direction, const struct appIdConfig_ *pConfig, tAppIdData *, tAppId, tAppId, const char *);
typedef void (*ClientAppAddInfo)(tAppIdData *, const char *);
typedef void (*ClientAppAddUser)(tAppIdData *, const char *, tAppId, int);
typedef void (*ClientAppAddPayload) (tAppIdData *, tAppId);

typedef struct _CLIENT_APP_API
{
    ClientAppFlowdataGet data_get;
    ClientAppFlowdataAdd data_add;
    ClientAppAddApp add_app;
    ClientAppAddInfo add_info;
    ClientAppAddUser add_user;
    ClientAppAddPayload add_payload;
} ClientAppApi;

typedef struct RNAClientAppRecord_
{
    struct RNAClientAppRecord_ *next;
    struct RNAClientAppModule *module;
} RNAClientAppRecord;

typedef struct RNAClientAppModule
{
    const char *name;
    uint8_t proto;
    RNAClientAppInitFCN init;
    RNAClientAppCleanFCN clean;
    RNAClientAppFCN validate;
    RNAClientAppCallbackFCN detectorCallback;
    bool detectorContext;
    unsigned minimum_matches;
    const ClientAppApi *api;
    struct _Detector*  userData;

    /**precedence of this detector.*/
    unsigned int precedence;
    RNAClientAppFinalizeFCN finalize;

    int provides_user;
    unsigned flow_data_index;
} tRNAClientAppModule;

typedef struct _RNA_CLIENT_APP_APPID_SESSION_STATE
{
    struct _RNA_CLIENT_APP_APPID_SESSION_STATE *next;
    const tRNAClientAppModule *ca;
} RNAClientAppFlowState;

#endif /* __CLIENT_APP_API_H__ */

