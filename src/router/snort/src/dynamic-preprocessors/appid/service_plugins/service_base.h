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


#ifndef __SERVICE_BASE_H__
#define __SERVICE_BASE_H__

#include "appIdApi.h"
#include "service_api.h"
#include "commonAppMatcher.h"
#include "flow.h"
#include "serviceConfig.h"
#include "appIdConfig.h"
#include "thirdparty_appid_utils.h"
struct _SERVICE_MATCH;

void CleanupServices(tAppIdConfig *pConfig);
void ReconfigureServices(tAppIdConfig *pConfig);
void UnconfigureServices(tAppIdConfig *pConfig);
void ServiceInit(tAppIdConfig *pConfig);
void ServiceFinalize(tAppIdConfig *pConfig);
void FailInProcessService(tAppIdData *flowp, const tAppIdConfig *pConfig);
int LoadServiceModules(const char **dir_list, uint32_t instance_id, tAppIdConfig *pConfig);

/**
 * \brief Reload C service modules
 *
 * This function is called during reload/reconfiguration. It registers service ports in the given
 * AppId configuration. This function also takes care of services associated with detector modules.
 *
 * @param pConfig - AppId config in which services' ports get registered
 * @return 0 on success, -1 on failure
 */
int ReloadServiceModules(tAppIdConfig *pConfig);
int serviceLoadCallback(void *symbol);
int serviceLoadForConfigCallback(void *symbol, tAppIdConfig *pConfig);
int ServiceAddPort(RNAServiceValidationPort *pp, tRNAServiceValidationModule *svm,
                   struct _Detector* userdata, tAppIdConfig *pConfig);
void ServiceRemovePorts(RNAServiceValidationFCN validate, struct _Detector* userdata, tAppIdConfig *pConfig);
void ServiceRegisterPatternDetector(RNAServiceValidationFCN fcn,
                                    u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                    int position, struct _Detector *userdata,
                                    const char *name);
int AppIdDiscoverService(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *rnaData, const tAppIdConfig *pConfig);
tAppId getPortServiceId(uint8_t proto, uint16_t port, const tAppIdConfig *pConfig);
tAppId getProtocolServiceId(uint8_t proto, const tAppIdConfig *pConfig);

void AppIdFreeServiceIDState(AppIdServiceIDState *id_state);

int AppIdServiceAddService(tAppIdData*flow, const SFSnortPacket *pkt, int dir,
                           const tRNAServiceElement *svc_element,
                           tAppId appId, const char *vendor, const char *version,
                           const RNAServiceSubtype *subtype, AppIdServiceIDState *id_state);
int AppIdServiceAddServiceSubtype(tAppIdData*flow, const SFSnortPacket *pkt, int dir,
                                  const tRNAServiceElement *svc_element,
                                  tAppId appId, const char *vendor, const char *version,
                                  RNAServiceSubtype *subtype, AppIdServiceIDState *id_state);
int AppIdServiceInProcess(tAppIdData*flow, const SFSnortPacket *pkt, int dir,
                          const tRNAServiceElement *svc_element, AppIdServiceIDState *id_state);
int AppIdServiceIncompatibleData(tAppIdData*flow, const SFSnortPacket *pkt, int dir,
                                 const tRNAServiceElement *svc_element, unsigned flow_data_index, const tAppIdConfig *pConfig, AppIdServiceIDState *id_state);
int AppIdServiceFailService(tAppIdData*flow, const SFSnortPacket *pkt, int dir,
                            const tRNAServiceElement *svc_element, unsigned flow_data_index, const tAppIdConfig *pConfig, AppIdServiceIDState *id_state);
int AddFTPServiceState(tAppIdData *fp);
void AppIdFreeDhcpInfo(DHCPInfo *dd);
void AppIdFreeSMBData(FpSMBData *sd);
void AppIdFreeDhcpData(DhcpFPData *dd);

void dumpPorts(FILE *stream, const tAppIdConfig *pConfig);

tRNAServiceElement *ServiceGetServiceElement(RNAServiceValidationFCN fcn, struct _Detector *userdata, tAppIdConfig *pConfig);

extern tRNAServiceValidationModule *active_service_list;

extern uint32_t app_id_instance_id;
void cleanupFreeServiceMatch(void);
void AppIdFreeServiceMatchList(struct _SERVICE_MATCH* sm);

static inline bool compareServiceElements(const tRNAServiceElement *first, const tRNAServiceElement *second)
{
    if (first == second)
        return 0;
    if (first == NULL || second == NULL)
        return 1;
    return (first->validate != second->validate || first->userdata != second->userdata);
}

// change UNIT_TESTING and UNIT_TEST_FIRST_DECRYPTED_PACKET in appIdApi.h
static inline uint32_t AppIdServiceDetectionLevel(tAppIdData * session)
{
    if (getAppIdFlag(session, APPID_SESSION_DECRYPTED)) return 1;
#ifdef UNIT_TESTING
    if (session->session_packet_count >= UNIT_TEST_FIRST_DECRYPTED_PACKET)
        return 1;
#endif
    return 0;
}

static inline void PopulateExpectedFlow(tAppIdData* parent, tAppIdData* expected, uint64_t flags, APPID_SESSION_DIRECTION dir)
{
    if (dir == APP_ID_FROM_INITIATOR)
    {
        setAppIdFlag(expected, flags |
                     getAppIdFlag(parent,
                                  APPID_SESSION_RESPONDER_MONITORED |
                                  APPID_SESSION_INITIATOR_MONITORED |
                                  APPID_SESSION_SPECIAL_MONITORED |
                                  APPID_SESSION_RESPONDER_CHECKED |
                                  APPID_SESSION_INITIATOR_CHECKED |
                                  APPID_SESSION_DISCOVER_APP |
                                  APPID_SESSION_DISCOVER_USER));
    }
    else if (dir == APP_ID_FROM_RESPONDER)
    {
        if (getAppIdFlag(parent, APPID_SESSION_INITIATOR_MONITORED))
           flags |= APPID_SESSION_RESPONDER_MONITORED;
        if (getAppIdFlag(parent, APPID_SESSION_INITIATOR_CHECKED))
           flags |= APPID_SESSION_RESPONDER_CHECKED;
        if (getAppIdFlag(parent, APPID_SESSION_RESPONDER_MONITORED))
           flags |= APPID_SESSION_INITIATOR_MONITORED;
        if (getAppIdFlag(parent, APPID_SESSION_RESPONDER_CHECKED))
           flags |= APPID_SESSION_INITIATOR_CHECKED;
        setAppIdFlag(expected, flags |
                     getAppIdFlag(parent, 
                                  APPID_SESSION_SPECIAL_MONITORED |
                                  APPID_SESSION_DISCOVER_APP |
                                  APPID_SESSION_DISCOVER_USER));
    }
    expected->rnaServiceState = RNA_STATE_FINISHED;
    expected->rnaClientState = RNA_STATE_FINISHED;
    if (thirdparty_appid_module)
        thirdparty_appid_module->session_state_set(expected->tpsession, TP_STATE_TERMINATED);
}

#endif /* __SERVICE_BASE_H__ */

