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


#ifndef __CLIENT_APP_BASE_H__
#define __CLIENT_APP_BASE_H__

#include "client_app_api.h"
#include "commonAppMatcher.h"
#include "httpCommon.h"
#include "flow.h"
#include "appIdConfig.h"

#define GENERIC_APP_OFFSET 2000000000

void ClientAppInit(tAppidStaticConfig* appidSC, tAppIdConfig *pConfig);
void ClientAppFinalize(tAppIdConfig *pConfig);
void UnconfigureClientApp(tAppIdConfig *pConfig);
void CleanupClientApp(tAppIdConfig *pConfig);
int clientAppLoadCallback(void *symbol);
int clientAppLoadForConfigCallback(void *symbol, tClientAppConfig *pConfig);
int LoadClientAppModules(const char **dir_list, tAppIdConfig *pConfig);
void ClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                              const uint8_t * const pattern, unsigned size,
                              int position, unsigned nocase, struct _Detector *userData, tClientAppConfig *pConfig);
const ClientAppApi *getClientApi(void);
RNAClientAppModuleConfig * getClientAppModuleConfig(const char *moduleName, tClientAppConfig *pConfig);
int AppIdDiscoverClientApp(SFSnortPacket *p, int direction, tAppIdData *rnaData, const tAppIdConfig *pConfig);
void AppIdAddClientApp(SFSnortPacket *p, int direction, const tAppIdConfig *pConfig, tAppIdData *flowp, tAppId service_id, tAppId id, const char *version);

DetectorAppUrlList *getAppUrlList(tAppIdConfig *pConfig);
tRNAClientAppModule *ClientAppGetClientAppModule(RNAClientAppFCN fcn, struct _Detector *userdata,
                                                      tClientAppConfig *pConfig);

int sipUaPatternAdd( 
        tAppId clientAppId,
        const char* clientVersion,
        const char* uaPattern,
        tDetectorSipConfig *pSipConfig
        );
int sipServerPatternAdd( 
        tAppId clientAppId,
        const char* clientVersion,
        const char* uaPattern,
        tDetectorSipConfig *pSipConfig
        );
int sipUaFinalize(tDetectorSipConfig *pSipConfig);
int sipServerFinalize(void);
int portPatternFinalize(tAppIdConfig *pConfig);

#endif /* __CLIENT_APP_BASE_H__ */

