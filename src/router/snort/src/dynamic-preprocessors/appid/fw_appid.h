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


#ifndef _APPID_H_
#define _APPID_H_

#include <stdint.h>
#include <attribute.h>

#include <netinet/in.h>
#include "profiler.h"
#include "commonAppMatcher.h"
#include "client_app_api.h"
#include "service_api.h"
#include "flow.h"
#include "common_util.h"
#include "sip_common.h"

#define PP_APP_ID   1

#define MIN_SFTP_PACKET_COUNT   30
#define MAX_SFTP_PACKET_COUNT   55

#define SF_APPID_MAX    30000
#define SF_APPID_DYNAMIC_MIN    1000000 

/* found this in the comp.lang.c FAQ */
#define BIT_CHUNK 8
#define BITMASK(b) (1 << ((b) % BIT_CHUNK))
#define BITSLOT(b) ((b) / BIT_CHUNK)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + BIT_CHUNK-1) / BIT_CHUNK)

extern char SSLBitList[BITNSLOTS(SF_APPID_MAX)];
extern char referredAppIdBitList[BITNSLOTS(SF_APPID_MAX)];
extern char SSLSquelchBitList[BITNSLOTS(SF_APPID_MAX)];

void fwAppIdFini(void);
void fwAppIdSearch(SFSnortPacket *p);
void httpHeaderCallback (SFSnortPacket *p, HttpParsedHeaders *const headers);
void SipSessionSnortCallback (void *ssnptr, ServiceEventType eventType, void *eventData);

void readRnaAppMappingTable(const char *path);
tAppId appGetAppFromServiceId(uint32_t serviceId);
tAppId appGetAppFromClientId(uint32_t clientId);
tAppId appGetAppFromPayloadId(uint32_t payloadId);
void appSharedDataDelete(tAppIdData * sharedData);
void AppIdAddUser(FLOW *flowp, const char *username, tAppId appId, int success);
void AppIdAddPayload(FLOW *flow, tAppId payload_id);
tAppIdData* appSharedDataAlloc(uint8_t proto, snort_ip *ip);
tAppId getOpenAppId(void *ssnptr);

void appSetServiceValidator(RNAServiceValidationFCN fcn, tAppId appId, unsigned extractsInfo, struct _Detector *data);
void appSetClientValidator(RNAClientAppFCN fcn, tAppId appId, unsigned extractsInfo, struct _Detector *data);

#ifdef FW_TRACKER_DEBUG
void logAppIdInfo(SFSnortPacket *p, char *message, tAppId id);
#endif
int AppIdDebug(uint16_t type, const uint8_t *data, uint32_t length, void **new_context,
               char* statusBuf, int statusBuf_len);

extern char app_id_debug_session[FW_DEBUG_SESSION_ID_SIZE];
extern bool app_id_debug_session_flag;

#ifdef PERF_PROFILING
extern PreprocStats httpPerfStats;
extern PreprocStats clientMatchPerfStats;
extern PreprocStats serviceMatchPerfStats;
extern PreprocStats luaClientPerfStats;
extern PreprocStats luaServerPerfStats;
#endif

extern unsigned dhcp_fp_table_size;
extern unsigned long app_id_raw_packet_count;
extern unsigned long app_id_processed_packet_count;
extern unsigned long app_id_ignored_packet_count;

static inline void bitListAddApp (tAppId app_id)
{
    if (app_id <= SF_APPID_MAX)
        BITSET(SSLBitList, app_id);
}

static inline int bitListTestApp (tAppId app_id)
{
    if (BITTEST(SSLBitList, app_id))
        return 1;
    else
        return 0;
}

static inline int testSSLAppIdForReinspect (tAppId app_id)
{
    if (app_id <= SF_APPID_MAX && (app_id == APP_ID_SSL || bitListTestApp(app_id)))
        return 1;
    else
        return 0;
}

static inline void referredAppIdAddApp (tAppId app_id)
{
    if (app_id <= SF_APPID_MAX)
        BITSET(referredAppIdBitList, app_id);
}

static inline int isReferredAppId (tAppId app_id)
{
    if (app_id <= SF_APPID_MAX && BITTEST(referredAppIdBitList, app_id))
        return 1;
    else
        return 0;
}

static inline tAppId isAppDetectionDone(tAppIdData *flow)
{
    return flow_checkflag(flow, FLOW_SERVICEDETECTED);
}

static inline tAppId pickServiceAppId(tAppIdData *flow)
{
    tAppId rval;

    if (!flow || flow->common.fsf_type.flow_type != FLOW_TYPE_NORMAL)
        return APP_ID_NONE;

    if (flow_checkflag(flow, FLOW_SERVICEDETECTED) && flow->serviceAppId > APP_ID_NONE)
        return flow->serviceAppId;
    else
        rval = APP_ID_NONE;

    if (flow->clientServiceAppId > APP_ID_NONE)
        return flow->clientServiceAppId;

    if (flow->portServiceAppId > APP_ID_NONE)
        return flow->portServiceAppId;

    return rval;
}

static inline tAppId pickOnlyServiceAppId(tAppIdData *flow)
{
    if (!flow || flow->common.fsf_type.flow_type != FLOW_TYPE_NORMAL)
        return APP_ID_NONE;

    if (flow->serviceAppId > APP_ID_NONE)
        return flow->serviceAppId;

    if (flow->serviceAppId < APP_ID_NONE)
        return APP_ID_UNKNOWN_UI;

    return APP_ID_NONE;
}

static inline tAppId pickMiscAppId(tAppIdData *flow)
{
    if (!flow || flow->common.fsf_type.flow_type != FLOW_TYPE_NORMAL)
        return APP_ID_NONE;
    if (flow->miscAppId > APP_ID_NONE)
        return flow->miscAppId;
    return APP_ID_NONE;
}

static inline tAppId pickClientAppId(tAppIdData *flow)
{
    if (!flow || flow->common.fsf_type.flow_type != FLOW_TYPE_NORMAL)
        return APP_ID_NONE;
    if (flow->clientAppId > APP_ID_NONE)
        return flow->clientAppId;
    return APP_ID_NONE;
}

static inline tAppId pickPayloadId(tAppIdData *flow)
{
    if (!flow || flow->common.fsf_type.flow_type != FLOW_TYPE_NORMAL)
        return APP_ID_NONE;
    if (flow->payloadAppId > APP_ID_NONE)
        return flow->payloadAppId;
    return APP_ID_NONE;
}

static inline tAppId pickReferredPayloadId(tAppIdData *flow)
{
    if (!flow || flow->common.fsf_type.flow_type != FLOW_TYPE_NORMAL)
        return APP_ID_NONE;
    if (flow->referredPayloadAppId > APP_ID_NONE)
        return flow->referredPayloadAppId;
    return APP_ID_NONE;
}

static inline tAppIdData* appSharedGetData(const SFSnortPacket *p)
{
    return _dpd.sessionAPI->get_application_data(p->stream_session, PP_APP_ID);
}

static inline void SSLSquelchAddApp (tAppId app_id)
{
    if (app_id > APP_ID_NONE && app_id <= SF_APPID_MAX)
        BITSET(SSLSquelchBitList, app_id);
}

static inline int SSLSquelch (tAppId app_id)
{
    if (app_id > APP_ID_NONE && app_id <= SF_APPID_MAX && BITTEST(SSLSquelchBitList, app_id))
        return 1;
    else
        return 0;
}

#endif
