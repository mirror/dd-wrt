/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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
                                              tAppIdData *session, const SFSnortPacket *pkt, struct _Detector *userData);

typedef struct _INIT_CLIENT_API
{
    void (*RegisterPattern)(RNAClientAppFCN fcn, uint8_t proto,
                            const uint8_t * const pattern, unsigned size, int position);
    void (*RegisterPatternEx)(RNAClientAppFCN fcn, uint8_t proto,
                            const uint8_t * const pattern, unsigned size, int position,
                            struct _Detector *userdata);
    void (*RegisterPatternNoCase)(RNAClientAppFCN fcn, uint8_t proto,
                                  const uint8_t * const pattern, unsigned size, int position);
    void (*RegisterAppId)(RNAClientAppFCN fcn, tAppId appId, uint32_t additionalInfo, struct _Detector *userData);
    int debug;
} InitClientAppAPI;

typedef struct _CLEAN_CLIENT_API
{
} CleanClientAppAPI;


typedef struct _FINALIZE_CLIENT_API
{
    void* data;
} FinalizeClientAppAPI;

typedef CLIENT_APP_RETCODE (*RNAClientAppInitFCN)(const InitClientAppAPI * const, SF_LIST *config);
typedef CLIENT_APP_RETCODE (*RNAClientAppFinalizeFCN)(const FinalizeClientAppAPI * const);
typedef void (*RNAClientAppCleanFCN)(const CleanClientAppAPI * const);

typedef void *(*ClientAppFlowdataGet)(FLOW *);
typedef int (*ClientAppFlowdataAdd)(FLOW *, void *, AppIdFreeFCN);
typedef void (*ClientAppAddApp)(FLOW *, tAppId, tAppId, const char *);
typedef void (*ClientAppAddInfo)(FLOW *, const char *);
typedef void (*ClientAppAddUser)(FLOW *, const char *, tAppId, int);
typedef void (*ClientAppAddPayload) (FLOW *, tAppId);

typedef struct _CLIENT_APP_API
{
    ClientAppFlowdataGet data_get;
    ClientAppFlowdataAdd data_add;
    ClientAppAddApp add_app;
    ClientAppAddInfo add_info;
    ClientAppAddUser add_user;
    ClientAppAddPayload add_payload;
} ClientAppApi;

struct _RNA_CLIENT_APP_MODULE;

typedef struct _RNA_CLIENT_APP_RECORD
{
    struct _RNA_CLIENT_APP_RECORD *next;
    const struct _RNA_CLIENT_APP_MODULE *module;
} RNAClientAppRecord;

typedef struct _RNA_CLIENT_APP_MODULE
{
    const char *name;
    uint8_t proto;
    RNAClientAppInitFCN init;
    RNAClientAppCleanFCN clean;
    RNAClientAppFCN validate;
    unsigned minimum_matches;
    const ClientAppApi *api;
    struct _Detector*  userData;

    /**precedence of this detector.*/
    unsigned int precedence;
    RNAClientAppFinalizeFCN finalize;

    int provides_user;
} RNAClientAppModule;

typedef struct _RNA_CLIENT_APP_FLOW_STATE
{
    struct _RNA_CLIENT_APP_FLOW_STATE *next;
    const RNAClientAppModule *ca;
} RNAClientAppFlowState;

#endif /* __CLIENT_APP_API_H__ */

