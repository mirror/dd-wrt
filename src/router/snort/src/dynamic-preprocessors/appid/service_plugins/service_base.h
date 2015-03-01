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


#ifndef __SERVICE_BASE_H__
#define __SERVICE_BASE_H__

#include "service_api.h"
#include "commonAppMatcher.h"
#include "flow.h"
struct _SERVICE_MATCH;

void CleanupServices(void);
void ReconfigureServices(void);
void ServiceInit(void);
void FailInProcessService(tAppIdData *flowp);
int LoadServiceModules(const char **dir_list, uint32_t instance_id);
int serviceLoadCallback(void *symbol);
int ServiceAddPort(RNAServiceValidationPort *pp, RNAServiceValidationModule *svm,
                   struct _Detector* userdata);
void ServiceRemovePorts(RNAServiceValidationFCN validate, struct _Detector* userdata);
void ServiceRegisterPatternDetector(RNAServiceValidationFCN fcn,
                                    u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                    int position, struct _Detector *userdata,
                                    const char *name);
int AppIdDiscoverService(const SFSnortPacket *p, int direction, tAppIdData *rnaData);
tAppId getPortServiceId(uint8_t proto, uint16_t port);

void AppIdFreeServiceIDState(AppIdServiceIDState *id_state);

int AppIdServiceAddService(FLOW *flow, const SFSnortPacket *pkt, int dir,
                           const RNAServiceElement *svc_element,
                           tAppId appId, const char *vendor, const char *version,
                           const RNAServiceSubtype *subtype);
int AppIdServiceAddServiceSubtype(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                  const RNAServiceElement *svc_element,
                                  tAppId appId, const char *vendor, const char *version,
                                  RNAServiceSubtype *subtype);
int AppIdServiceInProcess(FLOW *flow, const SFSnortPacket *pkt, int dir,
                          const RNAServiceElement *svc_element);
int AppIdServiceIncompatibleData(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                 const RNAServiceElement *svc_element);
int AppIdServiceFailService(FLOW *flow, const SFSnortPacket *pkt, int dir,
                            const RNAServiceElement *svc_element);
int AddFTPServiceState(tAppIdData *fp);

void AppIdFreeDhcpInfo(DHCPInfo *dd);
void AppIdFreeSMBData(FpSMBData *sd);
void AppIdFreeDhcpData(DhcpFPData *dd);

void dumpPorts(FILE *stream);

const RNAServiceElement *ServiceGetServiceElement(RNAServiceValidationFCN fcn, struct _Detector *userdata);

extern RNAServiceValidationModule *active_service_list;

extern tAppId app_id_service_instance;
void cleanupFreeServiceMatch(void);
void AppIdFreeServiceMatchList(struct _SERVICE_MATCH* sm);

static inline bool compareServiceElements(const RNAServiceElement *first, const RNAServiceElement *second)
{
    if (first == second)
        return 0;
    if (first == NULL || second == NULL)
        return 1;
    return (first->validate != second->validate || first->userdata != second->userdata);
}

#endif /* __SERVICE_BASE_H__ */

