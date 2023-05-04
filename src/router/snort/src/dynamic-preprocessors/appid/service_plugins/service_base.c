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


/**
 * @file   service_base.c
 * @author Ron Dempster <Ron.Dempster@sourcefire.com>
 *
 */

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common_util.h"
#include "service_base.h"
#include "service_state.h"
#include "service_api.h"
#include "service_bgp.h"
#include "service_bootp.h"
#include "detector_cip.h"
#include "service_dcerpc.h"
#include "service_flap.h"
#include "service_ftp.h"
#include "service_irc.h"
#include "service_lpr.h"
#include "service_mysql.h"
#include "service_netbios.h"
#include "service_nntp.h"
#include "service_ntp.h"
#include "service_radius.h"
#include "service_rexec.h"
#include "service_rfb.h"
#include "service_rlogin.h"
#include "service_rpc.h"
#include "service_rshell.h"
#include "service_rsync.h"
#include "service_rtmp.h"
#include "service_snmp.h"
#include "service_ssh.h"
#include "service_ssl.h"
#include "service_telnet.h"
#include "service_tftp.h"
#include "detector_dns.h"
#include "detector_sip.h"
#include "service_direct_connect.h"
#include "service_battle_field.h"
#include "service_MDNS.h"
#include "detector_pattern.h"
#include "luaDetectorModule.h"
#include "cpuclock.h"
#include "commonAppMatcher.h"
#include "fw_appid.h"
#include "flow.h"
#include "appIdConfig.h"
#include "ip_funcs.h"
#include "luaDetectorApi.h"

/*#define SERVICE_DEBUG 1 */
/*#define SERVICE_DEBUG_PORT  0 */

#define BUFSIZE         512

#define STATE_ID_INCONCLUSIVE_SERVICE_WEIGHT 3
#define STATE_ID_INVALID_CLIENT_THRESHOLD    9
#define STATE_ID_MAX_VALID_COUNT             5
#define STATE_ID_NEEDED_DUPE_DETRACT_COUNT   3


static void *service_flowdata_get(tAppIdData *flow, unsigned service_id);
static int service_flowdata_add(tAppIdData *flow, void *data, unsigned service_id, AppIdFreeFCN fcn);
static void AppIdAddHostInfo(tAppIdData *flow, SERVICE_HOST_INFO_CODE code, const void *info);
static int AppIdAddDHCP(tAppIdData *flowp, unsigned op55_len, const uint8_t *op55, unsigned op60_len, const uint8_t *op60, const uint8_t *mac);
static void AppIdAddHostIP(tAppIdData *flow, const uint8_t *mac, uint32_t ip4,
                                      int32_t zone, uint32_t subnetmask, uint32_t leaseSecs, uint32_t router);
static void AppIdAddSMBData(tAppIdData *flow, unsigned major, unsigned minor, uint32_t flags);
static void AppIdServiceAddMisc(tAppIdData* flow, tAppId miscId);

const ServiceApi serviceapi =
{
    .data_get = &service_flowdata_get,
    .data_add = &service_flowdata_add,
    .flow_new = &AppIdEarlySessionCreate,
    .data_add_id = &AppIdFlowdataAddId,
    .data_add_dhcp = &AppIdAddDHCP,
    .dhcpNewLease = &AppIdAddHostIP,
    .analyzefp = &AppIdAddSMBData,
    .add_service = &AppIdServiceAddService,
    .fail_service = &AppIdServiceFailService,
    .service_inprocess = &AppIdServiceInProcess,
    .incompatible_data = &AppIdServiceIncompatibleData,
    .add_host_info = &AppIdAddHostInfo,
    .add_payload = &AppIdAddPayload,
    .add_multipayload = &AppIdAddMultiPayload,
    .add_user = &AppIdAddUser,
    .add_service_consume_subtype = &AppIdServiceAddServiceSubtype,
    .add_misc = &AppIdServiceAddMisc,
    .add_dns_query_info = &AppIdAddDnsQueryInfo,
    .add_dns_response_info = &AppIdAddDnsResponseInfo,
    .reset_dns_info = &AppIdResetDnsInfo,
};

#ifdef SERVICE_DEBUG
static const char *serviceIdStateName[] =
{
    "NEW",
    "VALID",
    "PORT",
    "PATTERN",
    "BRUTE_FORCE"
};
#endif

static tRNAServiceElement *ftp_service = NULL;

static tServicePatternData *free_pattern_data;

/*C service API */
static void ServiceRegisterPattern(RNAServiceValidationFCN fcn,
                                   u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                   int position, struct _Detector *userdata, int provides_user,
                                   const char *name, tServiceConfig *pServiceConfig);
static void CServiceRegisterPattern(RNAServiceValidationFCN fcn, uint8_t proto,
                                   const uint8_t *pattern, unsigned size,
                                   int position, const char *name, tAppIdConfig *pConfig);
static void ServiceRegisterPatternUser(RNAServiceValidationFCN fcn, uint8_t proto,
                                       const uint8_t *pattern, unsigned size,
                                       int position, const char *name, tAppIdConfig *pConfig);
static int CServiceAddPort(RNAServiceValidationPort *pp, tRNAServiceValidationModule *svm, tAppIdConfig *pConfig);
static void CServiceRemovePorts(RNAServiceValidationFCN validate, tAppIdConfig *pConfig);

static InitServiceAPI svc_init_api =
{
    .RegisterPattern = &CServiceRegisterPattern,
    .AddPort = &CServiceAddPort,
    .RemovePorts = CServiceRemovePorts,
    .RegisterPatternUser = &ServiceRegisterPatternUser,
    .RegisterAppId = &appSetServiceValidator,
    .RegisterDetectorCallback = &appSetServiceDetectorCallback,
};

static CleanServiceAPI svc_clean_api =
{
};

extern tRNAServiceValidationModule timbuktu_service_mod;
extern tRNAServiceValidationModule bit_service_mod;
extern tRNAServiceValidationModule tns_service_mod;
extern tRNAServiceValidationModule http_service_mod;

static tRNAServiceValidationModule *static_service_list[] =
{
    &bgp_service_mod,
    &bootp_service_mod,
    &dcerpc_service_mod,
    &cip_service_mod,
    &dns_service_mod,
    &enip_service_mod,
    &flap_service_mod,
    &ftp_service_mod,
    &irc_service_mod,
    &lpr_service_mod,
    &mysql_service_mod,
    &netbios_service_mod,
    &nntp_service_mod,
    &ntp_service_mod,
    &radius_service_mod,
    &rexec_service_mod,
    &rfb_service_mod,
    &rlogin_service_mod,
    &rpc_service_mod,
    &rshell_service_mod,
    &rsync_service_mod,
    &rtmp_service_mod,
    &snmp_service_mod,
    &ssh_service_mod,
    &ssl_service_mod,
    &telnet_service_mod,
    &tftp_service_mod,
    &sip_service_mod,
    &directconnect_service_mod,
    &battlefield_service_mod,
    &mdns_service_mod,
    &timbuktu_service_mod,
    &bit_service_mod,
    &tns_service_mod,
    &pattern_service_mod,
    &http_service_mod
};

typedef struct _SERVICE_MATCH
{
    struct _SERVICE_MATCH *next;
    unsigned count;
    unsigned size;
    tRNAServiceElement *svc;
} ServiceMatch;

static DHCPInfo *dhcp_info_free_list;
static FpSMBData *smb_data_free_list;
static unsigned smOrderedListSize = 0;
static ServiceMatch **smOrderedList = NULL;
static ServiceMatch *free_service_match;
static const uint8_t zeromac[6] = {0, 0, 0, 0, 0, 0};

/**free ServiceMatch List.
 */
void AppIdFreeServiceMatchList(ServiceMatch* sm)
{
    ServiceMatch *tmpSm;

    if (!sm)
        return;

    for (tmpSm = sm; tmpSm->next; tmpSm = tmpSm->next);
    tmpSm->next = free_service_match;
    free_service_match = sm;
}

void cleanupFreeServiceMatch(void)
{
    ServiceMatch *match;
    while ((match=free_service_match) != NULL)
    {
        free_service_match = match->next;
        free(match);
    }
}

int AddFTPServiceState(tAppIdData *fp)
{
    if (!ftp_service)
        return -1;
    return AppIdFlowdataAddId(fp, 21, ftp_service);
}

/**allocate one ServiceMatch element.
 */
static inline ServiceMatch* allocServiceMatch(void)
{
    ServiceMatch *sm;

    if ((sm = free_service_match))
    {
        free_service_match = sm->next;
        memset(sm, 0, sizeof(*sm));
        return sm;
    }
    return (ServiceMatch *)calloc(1, sizeof(ServiceMatch));
}

static int pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    ServiceMatch **matches = (ServiceMatch **)data;
    tServicePatternData *pd = (tServicePatternData *)id;
    ServiceMatch *sm;

    if (pd->position >= 0 && pd->position != index)
        return 0;

    for (sm=*matches; sm; sm=sm->next)
        if (sm->svc == pd->svc)
            break;
    if (sm)
        sm->count++;
    else
    {
        if ((sm=allocServiceMatch()) == NULL)
        {
            _dpd.errMsg( "Error allocating a service match");
            return 0;
        }
        sm->count++;
        sm->svc = pd->svc;
        sm->size = pd->size;
        sm->next = *matches;
        *matches = sm;
    }
    return 0;
}

tAppId getPortServiceId(uint8_t proto, uint16_t port, const tAppIdConfig *pConfig)
{
    tAppId appId;

    if (proto == IPPROTO_TCP)
        appId = pConfig->tcp_port_only[port];
    else
        appId = pConfig->udp_port_only[port];

    checkSandboxDetection(appId);

    return appId;
}

tAppId getProtocolServiceId(uint8_t proto, const tAppIdConfig *pConfig)
{
    tAppId appId;

    appId = pConfig->ip_protocol[proto];

    checkSandboxDetection(appId);

    return appId;
}

static inline uint16_t sslPortRemap(
        uint16_t port
        )
{
    switch (port)
    {
    case 465:
        return 25;
    case 563:
        return 119;
    case 585:
    case 993:
        return 143;
    case 990:
        return 21;
    case 992:
        return 23;
    case 994:
        return 6667;
    case 995:
        return 110;
    default:
        return 0;
    }
}

static inline tRNAServiceElement *AppIdGetNextServiceByPort(
        uint8_t protocol,
        uint16_t port,
        const tRNAServiceElement * const lastService,
        tAppIdData *rnaData,
        const tAppIdConfig *pConfig
        )
{
    tRNAServiceElement *service = NULL;
    SF_LIST *list = NULL;

    if (AppIdServiceDetectionLevel(rnaData))
    {
        unsigned remappedPort = sslPortRemap(port);
        if (remappedPort)
            list = pConfig->serviceConfig.tcp_services[remappedPort];
    }
    else if (protocol == IPPROTO_TCP)
    {
        list = pConfig->serviceConfig.tcp_services[port];
    }
    else
    {
        list = pConfig->serviceConfig.udp_services[port];
    }

    if (list)
    {
        service = sflist_first(list);

        if (lastService)
        {
            while ( service && ((service->validate != lastService->validate) || (service->userdata != lastService->userdata)))
                service = sflist_next(list);
            if (service)
                service = sflist_next(list);
        }
    }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Port service for protocol %u port %u, service %s\n",
                (unsigned)protocol, (unsigned)port, (service && service->name) ? service->name:"UNKNOWN");
#endif

    return service;
}

static inline tRNAServiceElement *AppIdNextServiceByPattern(struct _SERVICE_MATCH **currentService
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                           , uint16_t port
#endif
#endif
                                                           )
{
    tRNAServiceElement *service = NULL;

    while (*currentService)
    {
        *currentService = (*currentService)->next;
        if (*currentService && (*currentService)->svc->current_ref_count)
        {
            service = (*currentService)->svc;
            break;
        }
    }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Next pattern service %s\n",
                (service && service->name) ? service->name:"UNKNOWN");
#endif

    return service;
}

tRNAServiceElement *ServiceGetServiceElement(RNAServiceValidationFCN fcn, struct _Detector *userdata,
                                                  tAppIdConfig *pConfig)
{
    tRNAServiceElement *li;

    for (li=pConfig->serviceConfig.tcp_service_list; li; li=li->next)
    {
        if ((li->validate == fcn) && (li->userdata == userdata))
            return li;
    }

    for (li=pConfig->serviceConfig.udp_service_list; li; li=li->next)
    {
        if ((li->validate == fcn) && (li->userdata == userdata))
            return li;
    }
    return NULL;
}

static void ServiceRegisterPattern(RNAServiceValidationFCN fcn,
                                   u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                   int position, struct _Detector *userdata, int provides_user,
                                   const char *name, tServiceConfig *pServiceConfig)
{
    void **patterns;
    tServicePatternData **pd_list;
    int *count;
    tServicePatternData *pd;
    tRNAServiceElement * *list;
    tRNAServiceElement *li;

    if (proto == IPPROTO_TCP)
    {
        patterns = &pServiceConfig->tcp_patterns;
        pd_list = &pServiceConfig->tcp_pattern_data;

        count = &pServiceConfig->tcp_pattern_count;
        list = &pServiceConfig->tcp_service_list;
    }
    else if (proto == IPPROTO_UDP)
    {
        patterns = &pServiceConfig->udp_patterns;
        pd_list = &pServiceConfig->udp_pattern_data;

        count = &pServiceConfig->udp_pattern_count;
        list = &pServiceConfig->udp_service_list;
    }
    else
    {
        _dpd.errMsg("Invalid protocol when registering a pattern: %u\n",(unsigned)proto);
        return;
    }

    for (li=*list; li; li=li->next)
    {
        if ((li->validate == fcn) && (li->userdata == userdata))
            break;
    }
    if (!li)
    {
        if (!(li = calloc(1, sizeof(*li))))
        {
            _dpd.errMsg( "Could not allocate a service list element");
            return;
        }
        li->next = *list;
        *list = li;
        li->validate = fcn;
        li->userdata = userdata;
        li->detectorType = UINT_MAX;
        li->provides_user = provides_user;
        li->name = name;
    }

    if (!(*patterns))
    {
        *patterns = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF);
        if (!(*patterns))
        {
            _dpd.errMsg("Error initializing the pattern table for protocol %u\n",(unsigned)proto);
            return;
        }
    }

    if (free_pattern_data)
    {
        pd = free_pattern_data;
        free_pattern_data = pd->next;
        memset(pd, 0, sizeof(*pd));
    }
    else if ((pd=(tServicePatternData *)calloc(1, sizeof(*pd))) == NULL)
    {
        _dpd.errMsg( "Error allocating pattern data");
        return;
    }
    pd->svc = li;
    pd->size = size;
    pd->position = position;
    _dpd.searchAPI->search_instance_add_ex(*patterns, (void *)pattern, size, pd, STR_SEARCH_CASE_SENSITIVE);
    (*count)++;
    pd->next = *pd_list;
    *pd_list = pd;
    li->ref_count++;
}

void ServiceRegisterPatternDetector(RNAServiceValidationFCN fcn,
                                    u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                    int position, struct _Detector *userdata, const char *name)
{
    ServiceRegisterPattern(fcn, proto, pattern, size, position, userdata, 0, name, &userdata->pAppidNewConfig->serviceConfig);
}

static void ServiceRegisterPatternUser(RNAServiceValidationFCN fcn, uint8_t proto,
                                       const uint8_t *pattern, unsigned size,
                                       int position, const char *name, tAppIdConfig *pConfig)
{
    ServiceRegisterPattern(fcn, proto, pattern, size, position, NULL, 1, name, &pConfig->serviceConfig);
}

static void CServiceRegisterPattern(RNAServiceValidationFCN fcn, uint8_t proto,
                                    const uint8_t *pattern, unsigned size,
                                    int position, const char *name,
                                    tAppIdConfig *pConfig)
{
    ServiceRegisterPattern(fcn, proto, pattern, size, position, NULL, 0, name, &pConfig->serviceConfig);
}

static void RemoveServicePortsByType(RNAServiceValidationFCN validate,
                                     SF_LIST **services,
                                     tRNAServiceElement *list,
                                     struct _Detector* userdata)
{
    tRNAServiceElement *li, *liTmp;
    SF_LNODE *node;
    SF_LNODE *nextNode;
    unsigned i;
    SF_LIST *listTmp;

    for (li=list; li; li=li->next)
    {
        if (li->validate == validate && li->userdata == userdata)
            break;
    }
    if (li == NULL)
        return;

    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if ((listTmp = services[i]))
        {
            node = sflist_first_node(listTmp);
            while (node)
            {
                liTmp = (tRNAServiceElement *)SFLIST_NODE_TO_DATA(node);
                if (liTmp == li)
                {
                    nextNode = node->next;
                    li->ref_count--;
                    sflist_remove_node(listTmp, node);
                    node = nextNode;
                    continue;
                }

                node = node->next;
            }
        }
    }
}

/**
 * \brief Remove all ports registered for all services
 *
 * This function takes care of removing ports for all services including C service modules,
 * Lua detector modules and services associated with C detector modules.
 *
 * @param pServiceConfig - Service configuration from which all ports need to be removed
 * @return void
 */
static void RemoveAllServicePorts(tServiceConfig *pServiceConfig)
{
    int i;

    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if (pServiceConfig->tcp_services[i])
        {
            sflist_free(pServiceConfig->tcp_services[i]);
            pServiceConfig->tcp_services[i] = NULL;
        }
    }
    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if (pServiceConfig->udp_services[i])
        {
            sflist_free(pServiceConfig->udp_services[i]);
            pServiceConfig->udp_services[i] = NULL;
        }
    }
    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if (pServiceConfig->udp_reversed_services[i])
        {
            sflist_free(pServiceConfig->udp_reversed_services[i]);
            pServiceConfig->udp_reversed_services[i] = NULL;
        }
    }
}

void ServiceRemovePorts(RNAServiceValidationFCN validate, struct _Detector* userdata, tAppIdConfig *pConfig)
{
    RemoveServicePortsByType(validate, pConfig->serviceConfig.tcp_services, pConfig->serviceConfig.tcp_service_list, userdata);
    RemoveServicePortsByType(validate, pConfig->serviceConfig.udp_services, pConfig->serviceConfig.udp_service_list, userdata);
    RemoveServicePortsByType(validate, pConfig->serviceConfig.udp_reversed_services, pConfig->serviceConfig.udp_reversed_service_list, userdata);
}

static void CServiceRemovePorts(RNAServiceValidationFCN validate, tAppIdConfig *pConfig)
{
    ServiceRemovePorts(validate, NULL, pConfig);
}

int ServiceAddPort(RNAServiceValidationPort *pp, tRNAServiceValidationModule *svm,
                   struct _Detector* userdata, tAppIdConfig *pConfig)
{
    SF_LIST **services;
    tRNAServiceElement * *list = NULL;
    tRNAServiceElement *li;
    tRNAServiceElement *serviceElement;
    uint8_t isAllocated = 0;

    _dpd.debugMsg(DEBUG_LOG, "Adding service %s for protocol %u on port %u, %p",
            svm->name, (unsigned)pp->proto, (unsigned)pp->port, pp->validate);
    if (pp->proto == IPPROTO_TCP)
    {
        services = pConfig->serviceConfig.tcp_services;
        list = &pConfig->serviceConfig.tcp_service_list;
    }
    else if (pp->proto == IPPROTO_UDP)
    {
        if (!pp->reversed_validation)
        {
            services = pConfig->serviceConfig.udp_services;
            list = &pConfig->serviceConfig.udp_service_list;
        }
        else
        {
            services = pConfig->serviceConfig.udp_reversed_services;
            list = &pConfig->serviceConfig.udp_reversed_service_list;
        }
    }
    else
    {
        _dpd.errMsg( "Service %s did not have a valid protocol (%u)",
               svm->name, (unsigned)pp->proto);
        return 0;
    }

    for (li=*list; li; li=li->next)
    {
        if (li->validate == pp->validate && li->userdata == userdata)
            break;
    }
    if (!li)
    {
        if (!(li = calloc(1, sizeof(*li))))
        {
            _dpd.errMsg( "Could not allocate a service list element");
            return -1;
        }

        isAllocated = 1;
        li->next = *list;
        *list = li;
        li->validate = pp->validate;
        li->provides_user = svm->provides_user;
        li->userdata = userdata;
        li->detectorType = UINT_MAX;
        li->name = svm->name;
    }

    if (pp->proto == IPPROTO_TCP && pp->port == 21 && !ftp_service)
    {
        ftp_service = li;
        li->ref_count++;
    }

    /*allocate a new list if this is first detector for this port. */
    if (!services[pp->port])
    {
        if (!(services[pp->port] = malloc(sizeof(SF_LIST))))
        {
            if (isAllocated)
            {
                *list = li->next;
                free(li);
            }
            _dpd.errMsg( "Could not allocate a service list");
            return -1;
        }
        sflist_init(services[pp->port]);
    }

    /*search and add if not present. */
    for (serviceElement = sflist_first(services[pp->port]);
            serviceElement && (serviceElement != li);
            serviceElement = sflist_next(services[pp->port]));

    if (!serviceElement)
    {
        if (sflist_add_tail(services[pp->port], li))
        {
            _dpd.errMsg( "Could not add %s, service for protocol %u on port %u", svm->name,
                   (unsigned)pp->proto, (unsigned)pp->port);
            if (isAllocated)
            {
                *list = li->next;
                free(li);
            }
            return -1;
        }
    }

    li->ref_count++;
    return 0;
}

static int CServiceAddPort(RNAServiceValidationPort *pp, tRNAServiceValidationModule *svm, tAppIdConfig *pConfig)
{
    return ServiceAddPort(pp, svm, NULL, pConfig);
}

int serviceLoadForConfigCallback(void *symbol, tAppIdConfig *pConfig)
{
    static unsigned service_module_index = 0;
    tRNAServiceValidationModule *svm = (tRNAServiceValidationModule *)symbol;
    RNAServiceValidationPort *pp;

    if (service_module_index >= 65536)
    {
        _dpd.errMsg( "Maximum number of service modules exceeded");
        return -1;
    }

    svm->api = &serviceapi;
    pp = svm->pp;
    for (pp=svm->pp; pp && pp->validate; pp++)
    {
        if (CServiceAddPort(pp, svm, pConfig))
            return -1;
    }

    if (svm->init(&svc_init_api))
    {
        _dpd.errMsg("Error initializing service %s\n",svm->name);
    }

    svm->next = pConfig->serviceConfig.active_service_list;
    pConfig->serviceConfig.active_service_list = svm;

    svm->flow_data_index = service_module_index | APPID_SESSION_DATA_SERVICE_MODSTATE_BIT;
    service_module_index++;

    return 0;
}

int serviceLoadCallback(void *symbol)
{
    return serviceLoadForConfigCallback(symbol, pAppidActiveConfig);
}

int LoadServiceModules(const char **dir_list, uint32_t instance_id, tAppIdConfig *pConfig)
{
    unsigned i;

    svc_init_api.instance_id = instance_id;
    svc_init_api.debug = appidStaticConfig->app_id_debug;
    svc_init_api.dpd = &_dpd;
    svc_init_api.pAppidConfig = pConfig;

    for (i=0; i<sizeof(static_service_list)/sizeof(*static_service_list); i++)
    {
        if (serviceLoadForConfigCallback(static_service_list[i], pConfig))
            return -1;
    }

    return 0;
}

int ReloadServiceModules(tAppIdConfig *pConfig)
{
    tRNAServiceValidationModule  *svm;
    RNAServiceValidationPort    *pp;

    svc_init_api.debug = app_id_debug;
    svc_init_api.pAppidConfig = pConfig;

    // active_service_list contains both service modules and services associated with
    // detector modules
    for (svm=pConfig->serviceConfig.active_service_list; svm; svm=svm->next)
    {
        // processing only non-lua service detectors.
        if (svm->init)
        {
            pp = svm->pp;
            for (pp=svm->pp; pp && pp->validate; pp++)
            {
                if (CServiceAddPort(pp, svm, pConfig))
                    return -1;
            }
        }
    }

    return 0;
}

void ServiceInit(tAppIdConfig *pConfig)
{
    luaModuleInitAllServices();
}

void ServiceFinalize(tAppIdConfig *pConfig)
{
    if (pConfig->serviceConfig.tcp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(pConfig->serviceConfig.tcp_patterns);
    }
    if (pConfig->serviceConfig.udp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(pConfig->serviceConfig.udp_patterns);
    }
}

void UnconfigureServices(tAppIdConfig *pConfig)
{
    tRNAServiceElement *li;
    tServicePatternData *pd;
    tRNAServiceValidationModule *svm;

    svc_clean_api.pAppidConfig = pConfig;

    if (pConfig->serviceConfig.tcp_patterns)
    {
        _dpd.searchAPI->search_instance_free(pConfig->serviceConfig.tcp_patterns);
        pConfig->serviceConfig.tcp_patterns = NULL;
    }
    // Do not free memory for the pattern; this can be later reclaimed when a
    // new pattern needs to be created. Memory for these patterns will be freed
    // on exit.
    while (pConfig->serviceConfig.tcp_pattern_data)
    {
        pd = pConfig->serviceConfig.tcp_pattern_data;
        if ((li = pd->svc) != NULL)
            li->ref_count--;
        pConfig->serviceConfig.tcp_pattern_data = pd->next;
        pd->next = free_pattern_data;
        free_pattern_data = pd;
    }
    if (pConfig->serviceConfig.udp_patterns)
    {
        _dpd.searchAPI->search_instance_free(pConfig->serviceConfig.udp_patterns);
        pConfig->serviceConfig.udp_patterns = NULL;
    }
    while (pConfig->serviceConfig.udp_pattern_data)
    {
        pd = pConfig->serviceConfig.udp_pattern_data;
        if ((li = pd->svc) != NULL)
            li->ref_count--;
        pConfig->serviceConfig.udp_pattern_data = pd->next;
        pd->next = free_pattern_data;
        free_pattern_data = pd;
    }

    RemoveAllServicePorts(&pConfig->serviceConfig);

    for (svm=pConfig->serviceConfig.active_service_list; svm; svm=svm->next)
    {
        if (svm->clean)
            svm->clean(&svc_clean_api);
    }

    CleanServicePortPatternList(pConfig);
}

void ReconfigureServices(tAppIdConfig *pConfig)
{
    tRNAServiceValidationModule *svm;

    for (svm=pConfig->serviceConfig.active_service_list; svm; svm=svm->next)
    {
        /*processing only non-lua service detectors. */
        if (svm->init)
        {
            if (svm->init(&svc_init_api))
            {
                _dpd.errMsg("Error initializing service %s\n",svm->name);
            }
            else
            {
                _dpd.debugMsg(DEBUG_LOG,"Initialized service %s\n",svm->name);
            }
        }
    }

    ServiceInit(pConfig);
}

void CleanupServices(tAppIdConfig *pConfig)
{
#ifdef APPID_FULL_CLEANUP
    tServicePatternData *pattern;
    tRNAServiceElement *se;
    ServiceMatch *sm;
    tRNAServiceValidationModule *svm;
    FpSMBData *sd;
    DHCPInfo *info;

    svc_clean_api.pAppidConfig = pConfig;

    if (pConfig->serviceConfig.tcp_patterns)
    {
        _dpd.searchAPI->search_instance_free(pConfig->serviceConfig.tcp_patterns);
        pConfig->serviceConfig.tcp_patterns = NULL;
    }
    if (pConfig->serviceConfig.udp_patterns)
    {
        _dpd.searchAPI->search_instance_free(pConfig->serviceConfig.udp_patterns);
        pConfig->serviceConfig.udp_patterns = NULL;
    }
    while ((pattern=pConfig->serviceConfig.tcp_pattern_data))
    {
        pConfig->serviceConfig.tcp_pattern_data = pattern->next;
        free(pattern);
    }
    while ((pattern=pConfig->serviceConfig.udp_pattern_data))
    {
        pConfig->serviceConfig.udp_pattern_data = pattern->next;
        free(pattern);
    }
    while ((pattern=free_pattern_data))
    {
        free_pattern_data = pattern->next;
        free(pattern);
    }
    while ((se=pConfig->serviceConfig.tcp_service_list))
    {
        pConfig->serviceConfig.tcp_service_list = se->next;
        free(se);
    }
    while ((se=pConfig->serviceConfig.udp_service_list))
    {
        pConfig->serviceConfig.udp_service_list = se->next;
        free(se);
    }
    while ((se=pConfig->serviceConfig.udp_reversed_service_list))
    {
        pConfig->serviceConfig.udp_reversed_service_list = se->next;
        free(se);
    }
    while ((sd = smb_data_free_list))
    {
        smb_data_free_list = sd->next;
        free(sd);
    }
    while ((info = dhcp_info_free_list))
    {
        dhcp_info_free_list = info->next;
        free(info);
    }
    while ((sm = free_service_match))
    {
        free_service_match = sm->next;
        free(sm);
    }
    if (smOrderedList)
     {
        free(smOrderedList);
        smOrderedListSize = 0;
     }

    RemoveAllServicePorts(&pConfig->serviceConfig);

    for (svm=pConfig->serviceConfig.active_service_list; svm; svm=svm->next)
    {
        if (svm->clean)
            svm->clean(&svc_clean_api);
    }

    CleanServicePortPatternList(pConfig);
#endif
}

static int AppIdPatternPrecedence(const void *a, const void *b)
{
    const ServiceMatch *sm1 = (ServiceMatch*)a;
    const ServiceMatch *sm2 = (ServiceMatch*)b;

    /*higher precedence should be before lower precedence */
    if (sm1->count != sm2->count)
        return (sm2->count - sm1->count);
    else
        return (sm2->size - sm1->size);
}

/**Perform pattern match of a packet and construct a list of services sorted in order of
 * precedence criteria. Criteria is count and then size. The first service in the list is
 * returned. The list itself is saved in AppIdServiceIDState. If
 * appId is already identified, then use it instead of searching again. RNA will capability
 * to try out other inferior matches. If appId is unknown i.e. searched and not found by FRE then
 * dont do any pattern match. This is a way degrades RNA detector selection if FRE is running on
 * this sensor.
*/
static inline tRNAServiceElement *AppIdGetServiceByPattern(const SFSnortPacket *pkt, uint8_t proto,
                                                          const int dir, const tServiceConfig *pServiceConfig,
                                                          struct _SERVICE_MATCH **serviceList, struct _SERVICE_MATCH **currentService)
{
    void *patterns = NULL;
    ServiceMatch *match_list;
    ServiceMatch *sm;
    uint32_t count;
    uint32_t i;
    tRNAServiceElement *service = NULL;

    if (proto == IPPROTO_TCP)
        patterns = pServiceConfig->tcp_patterns;
    else
        patterns = pServiceConfig->udp_patterns;

    if (!patterns)
    {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Pattern bailing due to no patterns\n");
#endif
        return NULL;
    }

    if (!smOrderedList)
    {
        smOrderedListSize = 32;
        if (!(smOrderedList = calloc(smOrderedListSize, sizeof(*smOrderedList))))
        {
            _dpd.errMsg( "Pattern bailing due to failed allocation");
            return NULL;
        }
    }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
    {
#endif
        fprintf(SF_DEBUG_FILE, "Matching\n");
        DumpHex(SF_DEBUG_FILE, pkt->payload, pkt->payload_size);
#if SERVICE_DEBUG_PORT
    }
#endif
#endif
    /*FRE didn't search */
    match_list = NULL;
    _dpd.searchAPI->search_instance_find_all(patterns, (char *)pkt->payload,
            pkt->payload_size, 0, &pattern_match, (void*)&match_list);

    count = 0;
    for (sm=match_list; sm; sm=sm->next)
    {
        if (count >= smOrderedListSize)
        {
            ServiceMatch **tmp;
            smOrderedListSize *= 2;
            tmp = realloc(smOrderedList, smOrderedListSize * sizeof(*smOrderedList));
            if (!tmp)
            {
                /*log realloc failure */
                _dpd.errMsg("Realloc failure %u\n",smOrderedListSize);
                smOrderedListSize /= 2;

                /*free the remaining elements. */
                AppIdFreeServiceMatchList(sm);

                break;
            }
            _dpd.errMsg("Realloc %u\n",smOrderedListSize);

            smOrderedList = tmp;
        }

        smOrderedList[count++] = sm;
    }

    if (!count)
        return NULL;

    qsort(smOrderedList, count, sizeof(*smOrderedList), AppIdPatternPrecedence);

    /*rearrange the matchlist now */
    for (i = 0; i < (count-1); i++)
        smOrderedList[i]->next = smOrderedList[i+1];
    smOrderedList[i]->next = NULL;

    service = smOrderedList[0]->svc;
    if (*serviceList)
        AppIdFreeServiceMatchList(*serviceList);
    *serviceList = smOrderedList[0];
    *currentService = smOrderedList[0];

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Pattern service for protocol %u (%u->%u), %s\n",
                (unsigned)proto, (unsigned)pkt->src_port, (unsigned)pkt->dst_port,
                (service && service->name) ? service->name:"UNKNOWN");
#endif
    return service;
}

static inline tRNAServiceElement * AppIdGetServiceByBruteForce(
        uint32_t protocol,
        const tRNAServiceElement *lastService,
        const tAppIdConfig *pConfig
        )
{
    tRNAServiceElement *service;

    if (lastService)
        service = lastService->next;
    else
        service = ((protocol == IPPROTO_TCP) ? pConfig->serviceConfig.tcp_service_list:pConfig->serviceConfig.udp_service_list);

    while (service && !service->current_ref_count)
        service = service->next;

    return service;
}

static void AppIdAddHostInfo(tAppIdData *flow, SERVICE_HOST_INFO_CODE code, const void *info)
{
    if (code == SERVICE_HOST_INFO_NETBIOS_NAME)
    {
        if (flow->netbios_name)
        {
            if (strcmp(flow->netbios_name,(char *)info) == 0)
                return;
            free(flow->netbios_name);
        }
        flow->netbios_name = strdup((char *)info);
    }
}

void AppIdFreeDhcpData(DhcpFPData *dd)
{
    free(dd);
}

static int AppIdAddDHCP(tAppIdData *flowp, unsigned op55_len, const uint8_t *op55, unsigned op60_len, const uint8_t *op60, const uint8_t *mac)


{
    if(op55_len && op55_len <= DHCP_OPTION55_LEN_MAX && !getAppIdFlag(flowp, APPID_SESSION_HAS_DHCP_FP))
    {
        DhcpFPData *rdd;

        rdd = malloc(sizeof(*rdd));
        if (!rdd)
            return -1;

        if (AppIdFlowdataAdd(flowp, rdd, APPID_SESSION_DATA_DHCP_FP_DATA, (AppIdFreeFCN)AppIdFreeDhcpData))
        {
            AppIdFreeDhcpData(rdd);
            return -1;
        }

        setAppIdFlag(flowp, APPID_SESSION_HAS_DHCP_FP);
        rdd->op55_len = (op55_len > DHCP_OP55_MAX_SIZE) ? DHCP_OP55_MAX_SIZE:op55_len;
        memcpy(rdd->op55, op55, rdd->op55_len);
        rdd->op60_len =  (op60_len > DHCP_OP60_MAX_SIZE) ? DHCP_OP60_MAX_SIZE:op60_len;
        if(op60_len)
            memcpy(rdd->op60, op60, rdd->op60_len);
        memcpy(rdd->mac, mac, sizeof(rdd->mac));
    }
    return 0;
}

void AppIdFreeDhcpInfo(DHCPInfo *dd)
{
    if (dd)
    {
        dd->next = dhcp_info_free_list;
        dhcp_info_free_list = dd;
    }
}

static void AppIdAddHostIP(tAppIdData *flow, const uint8_t *mac, uint32_t ip, int32_t zone,
                                      uint32_t subnetmask, uint32_t leaseSecs, uint32_t router)
{
    DHCPInfo *info;
    unsigned flags;

    if (memcmp(mac, zeromac, 6) == 0 || ip == 0)
         return;

    if (!getAppIdFlag(flow, APPID_SESSION_DO_RNA) || getAppIdFlag(flow, APPID_SESSION_HAS_DHCP_INFO))
        return;

    flags = isIPv4HostMonitored(ntohl(ip), zone);
    if (!(flags & IPFUNCS_HOSTS_IP)) return;


    if (dhcp_info_free_list)
    {
        info = dhcp_info_free_list;
        dhcp_info_free_list = info->next;
    }
    else if (!(info = malloc(sizeof(*info))))
        return;

    if (AppIdFlowdataAdd(flow, info, APPID_SESSION_DATA_DHCP_INFO, (AppIdFreeFCN)AppIdFreeDhcpInfo))
    {
        AppIdFreeDhcpInfo(info);
        return;
    }
    setAppIdFlag(flow, APPID_SESSION_HAS_DHCP_INFO);
    info->ipAddr = ip;
    memcpy(info->macAddr, mac, sizeof(info->macAddr));
    info->subnetmask = subnetmask;
    info->leaseSecs = leaseSecs;
    info->router = router;
}

void AppIdFreeSMBData(FpSMBData *sd)
{
    if (sd)
    {
        sd->next = smb_data_free_list;
        smb_data_free_list = sd;
    }
}

static void AppIdAddSMBData(tAppIdData *flow, unsigned major, unsigned minor, uint32_t flags)
{
    FpSMBData *sd;

    if (flags & FINGERPRINT_UDP_FLAGS_XENIX)
        return;

    if (getAppIdFlag(flow, APPID_SESSION_HAS_SMB_INFO))
        return;

    if (smb_data_free_list)
    {
        sd = smb_data_free_list;
        smb_data_free_list = sd->next;
    }
    else
        sd = malloc(sizeof(*sd));
    if (!sd)
        return;

    if (AppIdFlowdataAdd(flow, sd, APPID_SESSION_DATA_SMB_DATA, (AppIdFreeFCN)AppIdFreeSMBData))
    {
        AppIdFreeSMBData(sd);
        return;
    }

    setAppIdFlag(flow, APPID_SESSION_HAS_SMB_INFO);
    sd->major = major;
    sd->minor = minor;
    sd->flags = flags & FINGERPRINT_UDP_FLAGS_MASK;
}

static int AppIdServiceAddServiceEx(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                                    const tRNAServiceElement *svc_element,
                                    tAppId appId, const char *vendor, const char *version, AppIdServiceIDState *id_state)
{
    uint16_t port;
    sfaddr_t *ip;

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid = GET_SFOUTER_IPH_PROTOID(pkt, pkt_header);
#endif

    if (!flow || !pkt || !svc_element)
    {
        _dpd.errMsg("Invalid arguments to absinthe_add_appId");
        return SERVICE_EINVALID;
    }

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t asId; 
#endif

    tAppId tmpServiceAppId = flow->serviceAppId;
    flow->serviceData = svc_element;

    if (vendor)
    {
        if (flow->serviceVendor)
            free(flow->serviceVendor);
        flow->serviceVendor = strdup(vendor);
        if (!flow->serviceVendor)
            _dpd.errMsg("failed to allocate service vendor name");
    }
    if (version)
    {
        if (flow->serviceVersion)
            free(flow->serviceVersion);
        flow->serviceVersion = strdup(version);
        if (!flow->serviceVersion)
            _dpd.errMsg("failed to allocate service version");
    }
    setAppIdFlag(flow, APPID_SESSION_SERVICE_DETECTED);
    flow->serviceAppId = appId;

    checkSandboxDetection(appId);

    if (appId > APP_ID_NONE && tmpServiceAppId != appId)
        CheckDetectorCallback(pkt, flow, (APPID_SESSION_DIRECTION) dir, appId, pAppidActiveConfig);

    if (getAppIdFlag(flow, APPID_SESSION_IGNORE_HOST))
        return SERVICE_SUCCESS;

    if (!getAppIdFlag(flow, APPID_SESSION_UDP_REVERSED))
    {
        if (dir == APP_ID_FROM_INITIATOR)
        {
            ip = GET_DST_IP(pkt);
            port = pkt->dst_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            asId = pkt->pkt_header->address_space_id_dst;
#endif
        }
        else
        {
            ip = GET_SRC_IP(pkt);
            port = pkt->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            asId = pkt->pkt_header->address_space_id_src;
#endif
        }
        if (flow->service_port)
            port = flow->service_port;
    }
    else
    {
        if (dir == APP_ID_FROM_INITIATOR)
        {
            ip = GET_SRC_IP(pkt);
            port = pkt->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            asId = pkt->pkt_header->address_space_id_src;
#endif
        }
        else
        {
            ip = GET_DST_IP(pkt);
            port = pkt->dst_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            asId = pkt->pkt_header->address_space_id_dst;
#endif
        }
    }
    if (!id_state)
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        id_state = AppIdGetServiceIDState(ip, flow->proto, port,
                                          AppIdServiceDetectionLevel(flow),
                                          asId, cid);
#else
        id_state = AppIdGetServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow), cid);
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        id_state = AppIdGetServiceIDState(ip, flow->proto, port,
                                          AppIdServiceDetectionLevel(flow),
                                          asId);
#else
        id_state = AppIdGetServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow));
#endif
#endif
    } 

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)   
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    if (!id_state && !(id_state = AppIdAddServiceIDState(ip, flow->proto, port,
                                                         AppIdServiceDetectionLevel(flow), 
                                                         asId, cid)))
#else
    if (!id_state && !(id_state = AppIdAddServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow), cid)))
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
    if (!id_state && !(id_state = AppIdAddServiceIDState(ip, flow->proto, port,
                                                         AppIdServiceDetectionLevel(flow),
                                                         asId)))
#else
    if (!id_state && !(id_state = AppIdAddServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow))))
#endif
#endif
    {
        _dpd.errMsg("Add service failed to create state");
        return SERVICE_ENOMEM;
    }
    flow->service_ip = *ip;
    flow->service_port = port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    flow->serviceAsId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    flow->carrierId = cid;
#endif

    id_state->reset_time = 0;
    if (id_state->state != SERVICE_ID_VALID)
    {
        id_state->state = SERVICE_ID_VALID;
        id_state->valid_count = 0;
        id_state->detract_count = 0;
        IP_CLEAR(id_state->last_detract);
        id_state->invalid_client_count = 0;
        IP_CLEAR(id_state->last_invalid_client);
    }
    id_state->svc = svc_element;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
    {
        char ipstr[INET6_ADDRSTRLEN];

        ipstr[0] = 0;
        inet_ntop(sfaddr_family(&flow->service_ip), (void *)sfaddr_get_ptr(&flow->service_ip), ipstr, sizeof(ipstr));
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        fprintf(SF_DEBUG_FILE, "Valid: %s:%u:%u AS %u CID:%u %p %d\n", ipstr,
                (unsigned)flow->proto, (unsigned)flow->service_port,
                asId, (unsigned)cid, id_state, (int)id_state->state);
#else
        fprintf(SF_DEBUG_FILE, "Valid: %s:%u:%u CID:%u %p %d\n", ipstr, (unsigned)flow->proto,
                (unsigned)flow->service_port, (unsigned)cid, id_state, (int)id_state->state);
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        fprintf(SF_DEBUG_FILE, "Valid: %s:%u:%u AS %u %p %d\n", ipstr,
                (unsigned)flow->proto, (unsigned)flow->service_port,
                asId, id_state, (int)id_state->state);
#else
        fprintf(SF_DEBUG_FILE, "Valid: %s:%u:%u %p %d\n", ipstr, (unsigned)flow->proto,
                (unsigned)flow->service_port, id_state, (int)id_state->state);
#endif
#endif
    }
#endif

    if (!id_state->valid_count)
    {
        id_state->valid_count++;
        id_state->invalid_client_count = 0;
        IP_CLEAR(id_state->last_invalid_client);
        id_state->detract_count = 0;
        IP_CLEAR(id_state->last_detract);
    }
    else if (id_state->valid_count < STATE_ID_MAX_VALID_COUNT)
        id_state->valid_count++;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) AS %u CID %u is valid\n",
                (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, asId, (unsigned) cid);
#else
        fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) CID %u is valid\n",
                (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, (unsigned) cid);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) AS %u is valid\n",
                (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, asId);
#else
        fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid\n",
                (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
#endif
#endif
    }
#endif
    return SERVICE_SUCCESS;
}

int AppIdServiceAddServiceSubtype(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                                  const tRNAServiceElement *svc_element,
                                  tAppId appId, const char *vendor, const char *version,
                                  RNAServiceSubtype *subtype, AppIdServiceIDState *id_state)
{
    flow->subtype = subtype;
    if (!svc_element->current_ref_count)
    {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) AS %u CID %u is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, flow->serviceAsId, (unsigned)flow->carrierId); 
#else
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) CID %u is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, (unsigned)flow->carrierId);
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) AS %u is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, flow->serviceAsId);
#else
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
#endif
#endif
        }
#endif
        return SERVICE_SUCCESS;
    }
    return AppIdServiceAddServiceEx(flow, pkt, dir, svc_element, appId, vendor, version, id_state);
}

int AppIdServiceAddService(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                           const tRNAServiceElement *svc_element,
                           tAppId appId, const char *vendor, const char *version,
                           const RNAServiceSubtype *subtype, AppIdServiceIDState *id_state)
{
    RNAServiceSubtype *new_subtype = NULL;
    RNAServiceSubtype *tmp_subtype;

    if (!svc_element->current_ref_count)
    {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) AS %u CID %u is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, flow->serviceAsId, (unsigned)flow->carrierId);
#else
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) CID %u is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, (unsigned)flow->carrierId);
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) AS %u is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, flow->serviceAsId);
#else
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
#endif
#endif
        }
#endif
        return SERVICE_SUCCESS;
    }

    for ( ; subtype; subtype = subtype->next)
    {
        tmp_subtype = calloc(1, sizeof(*tmp_subtype));
        if (tmp_subtype)
        {
            if (subtype->service)
            {
                tmp_subtype->service = strdup(subtype->service);
                if (!tmp_subtype->service)
                    _dpd.errMsg("failed to allocate service subtype");
            }
            if (subtype->vendor)
            {
                tmp_subtype->vendor = strdup(subtype->vendor);
                if (!tmp_subtype->vendor)
                    _dpd.errMsg("failed to allocate service subtype vendor");
            }
            if (subtype->version)
            {
                tmp_subtype->version = strdup(subtype->version);
                if (!tmp_subtype->version)
                    _dpd.errMsg("failed to allocate service version");
            }
            tmp_subtype->next = new_subtype;
            new_subtype = tmp_subtype;
        }
    }
    flow->subtype = new_subtype;
    return AppIdServiceAddServiceEx(flow, pkt, dir, svc_element, appId, vendor, version, id_state);
}

int AppIdServiceInProcess(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                          const tRNAServiceElement *svc_element,
                          AppIdServiceIDState *id_state)
{
    if (!flow || !pkt)
    {
        _dpd.errMsg( "Invalid arguments to service_in_process");
        return SERVICE_EINVALID;
    }

    if (dir == APP_ID_FROM_INITIATOR || getAppIdFlag(flow, APPID_SESSION_IGNORE_HOST|APPID_SESSION_UDP_REVERSED))
        return SERVICE_SUCCESS;

    if (!sfaddr_is_set(&flow->service_ip))
    {
        sfaddr_t *ip;

        ip = GET_SRC_IP(pkt);
        flow->service_ip = *ip;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        flow->serviceAsId = pkt->pkt_header->address_space_id_src;
#endif
        if (!flow->service_port)
            flow->service_port = pkt->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        flow->carrierId = GET_SFOUTER_IPH_PROTOID(pkt, pkt_header);
#endif
    }
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        fprintf(SF_DEBUG_FILE, "Service for protocol %u on port %u is in process (%u->%u) AS %u CID %u, %p %s",
                (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port,
                (unsigned)pkt->dst_port, flow->serviceAsId, (unsigned)flow->carrierId, 
                svc_element->validate, svc_element->name ? :"UNKNOWN");
#else
        fprintf(SF_DEBUG_FILE, "Service for protocol %u on port %u is in process (%u->%u) CID %u, %p %s",
                (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port,
                (unsigned)flow->carrierId, svc_element->validate, svc_element->name ? :"UNKNOWN");
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        fprintf(SF_DEBUG_FILE, "Service for protocol %u on port %u is in process (%u->%u) AS %u, %p %s",
                (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port,
                (unsigned)pkt->dst_port, flow->serviceAsId, 
                svc_element->validate, svc_element->name ? :"UNKNOWN");
#else
        fprintf(SF_DEBUG_FILE, "Service for protocol %u on port %u is in process (%u->%u), %p %s",
                (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port,
                svc_element->validate, svc_element->name ? :"UNKNOWN");
#endif
#endif

    }
#endif

    return SERVICE_SUCCESS;
}

/**Called when service can not be identified on a flow but the checks failed on client request
 * rather than server response. When client request fails a check, it may be specific to a client
 * therefore we should not fail the service right away. If the same behavior is seen from the same
 * client ultimately we will have to fail the service. If the same behavior is seen from different
 * clients going to same service then this most likely the service is something else.
 */
int AppIdServiceIncompatibleData(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                                 const tRNAServiceElement *svc_element, unsigned flow_data_index,
                                 const tAppIdConfig *pConfig, AppIdServiceIDState *id_state)
{
    if (!flow || !pkt)
    {
        _dpd.errMsg("Invalid arguments to service_incompatible_data");
        return SERVICE_EINVALID;
    }

    if (flow_data_index != APPID_SESSION_DATA_NONE)
        AppIdFlowdataDelete(flow, flow_data_index);

    /* If we're still working on a port/pattern list of detectors, then ignore
     * individual fails until we're done looking at everything. */
    if (    (flow->serviceData == NULL)                                                /* we're working on a list of detectors, and... */
         && (flow->candidate_service_list != NULL))
    {
        if (sflist_count(flow->candidate_service_list) != 0)                           /*     it's not empty */
        {
            return SERVICE_SUCCESS;
        }
    }

    setAppIdFlag(flow, APPID_SESSION_SERVICE_DETECTED);
    clearAppIdFlag(flow, APPID_SESSION_CONTINUE);

    flow->serviceAppId = APP_ID_NONE;

    if (getAppIdFlag(flow, APPID_SESSION_IGNORE_HOST|APPID_SESSION_UDP_REVERSED) || (svc_element && !svc_element->current_ref_count))
        return SERVICE_SUCCESS;

    if (dir == APP_ID_FROM_INITIATOR)
    {
        setAppIdFlag(flow, APPID_SESSION_INCOMPATIBLE);
        return SERVICE_SUCCESS;
    }

    uint16_t port;
    sfaddr_t *ip;
    ip = GET_SRC_IP(pkt);
    port = flow->service_port ? flow->service_port : pkt->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t asId = pkt->pkt_header->address_space_id_src;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid = GET_SFOUTER_IPH_PROTOID(pkt, pkt_header);
#endif

    if (!id_state)
    {    
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        id_state = AppIdGetServiceIDState(ip, flow->proto, port,
                                          AppIdServiceDetectionLevel(flow),
                                          asId, cid);
#else
        id_state = AppIdGetServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow), cid);
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        id_state = AppIdGetServiceIDState(ip, flow->proto, port,
                                          AppIdServiceDetectionLevel(flow),
                                          asId);
#else
        id_state = AppIdGetServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow));
#endif
#endif
    }   

    if (!id_state)
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port,
                                                AppIdServiceDetectionLevel(flow), 
                                                asId, cid)))
#else
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow), cid)))
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port,
                                                AppIdServiceDetectionLevel(flow),
                                                asId)))
#else
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow))))
#endif
#endif
        {
            _dpd.errMsg("Incompatible service failed to create state");
            return SERVICE_ENOMEM;
        }
        id_state->svc = svc_element;
    }
    else
    {
        id_state->reset_time = 0;
    }
    flow->service_ip = *ip;
    flow->service_port = port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    flow->serviceAsId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    flow->carrierId = cid;
#endif
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
       fprintf(SF_DEBUG_FILE, "service_IC: State %s for protocol %u on port %u (%u->%u) AS %u CID %u, count %u, %s\n",
               serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
               (unsigned)pkt->src_port, (unsigned)pkt->dst_port, asId, (unsigned)flow->carrierId,
               id_state->invalid_client_count,
               (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#else
        fprintf(SF_DEBUG_FILE, "service_IC: State %s for protocol %u on port %u (%u->%u) CID %u, count %u, %s\n",
                serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, (unsigned) flow->carrierId, id_state->invalid_client_count,
                (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif
#else /* No carrier id */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
       fprintf(SF_DEBUG_FILE, "service_IC: State %s for protocol %u on port %u (%u->%u) AS %u, count %u, %s\n",
               serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
               (unsigned)pkt->src_port, (unsigned)pkt->dst_port, asId, 
               id_state->invalid_client_count,
               (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#else
        fprintf(SF_DEBUG_FILE, "service_IC: State %s for protocol %u on port %u (%u->%u), count %u, %s\n",
                serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_client_count,
                (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif
#endif
    }
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(sfaddr_family(&flow->service_ip), (void *)sfaddr_get_ptr(&flow->service_ip), ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Incompat: %s:%u:%u %p %d %s\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state,
            (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
}
#endif

    return SERVICE_SUCCESS;
}

int AppIdServiceFailService(tAppIdData* flow, const SFSnortPacket *pkt, int dir,
                            const tRNAServiceElement *svc_element, unsigned flow_data_index,
                            const tAppIdConfig *pConfig, AppIdServiceIDState *id_state)
{
    if (flow_data_index != APPID_SESSION_DATA_NONE)
        AppIdFlowdataDelete(flow, flow_data_index);

    /* If we're still working on a port/pattern list of detectors, then ignore
     * individual fails until we're done looking at everything. */
    if (    (flow->serviceData == NULL)                                                /* we're working on a list of detectors, and... */
         && (flow->candidate_service_list != NULL))
    {
        if (sflist_count(flow->candidate_service_list) != 0)                           /*     it's not empty */
        {
            return SERVICE_SUCCESS;
        }
    }

    flow->serviceAppId = APP_ID_NONE;

    setAppIdFlag(flow, APPID_SESSION_SERVICE_DETECTED);
    clearAppIdFlag(flow, APPID_SESSION_CONTINUE);

    /* detectors should be careful in marking flow UDP_REVERSED otherwise the same detector
     * gets all future flows. UDP_REVERSE should be marked only when detector positively
     * matches opposite direction patterns. */

    if (getAppIdFlag(flow, APPID_SESSION_IGNORE_HOST|APPID_SESSION_UDP_REVERSED) || (svc_element && !svc_element->current_ref_count))
        return SERVICE_SUCCESS;

    /* For subsequent packets, avoid marking service failed on client packet,
     * otherwise the service will show up on client side. */
    if (dir == APP_ID_FROM_INITIATOR)
    {
        setAppIdFlag(flow, APPID_SESSION_INCOMPATIBLE);
        return SERVICE_SUCCESS;
    }

    uint16_t port;
    sfaddr_t *ip;
    ip = GET_SRC_IP(pkt);
    port = flow->service_port ? flow->service_port : pkt->src_port;
    flow->service_ip = *ip;
    flow->service_port = port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    flow->serviceAsId = pkt->pkt_header->address_space_id_src;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    flow->carrierId = GET_SFOUTER_IPH_PROTOID(pkt, pkt_header);
#endif
    if (!id_state)
    { 
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)   
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
       id_state = AppIdGetServiceIDState(ip, flow->proto, port,
                                         AppIdServiceDetectionLevel(flow),
                                         flow->serviceAsId, flow->carrierId);
#else
       id_state = AppIdGetServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow), flow->carrierId);
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
       id_state = AppIdGetServiceIDState(ip, flow->proto, port,
                                         AppIdServiceDetectionLevel(flow),
                                         flow->serviceAsId);
#else
       id_state = AppIdGetServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow));
#endif
#endif
    }   

    if (!id_state)
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port,
                                                AppIdServiceDetectionLevel(flow),
                                                flow->serviceAsId, flow->carrierId)))
#else
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow), flow->carrierId)))
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port,
                                                AppIdServiceDetectionLevel(flow),
                                                flow->serviceAsId)))
#else
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port, AppIdServiceDetectionLevel(flow))))
#endif
#endif
        {
            _dpd.errMsg("Fail service failed to create state");
            return SERVICE_ENOMEM;
        }
        id_state->svc = svc_element;
    }
    id_state->reset_time = 0;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
       fprintf(SF_DEBUG_FILE, "service_fail: State %s for protocol %u on port %u (%u->%u) AS %u CID %u, count %u, valid count %u, currSvc %s\n",
               serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
               (unsigned)pkt->src_port, (unsigned)pkt->dst_port, flow->serviceAsId, (unsigned)flow->carrierId,
               id_state->invalid_client_count, id_state->valid_count,
               (svc_element && svc_element->name) ? svc_element->name:"UNKNOWN");
#else
        fprintf(SF_DEBUG_FILE, "service_fail: State %s for protocol %u on port %u (%u->%u) CID %u, count %u, valid count %u, currSvc %s\n",
                serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, (unsigned)flow->carrierId, id_state->invalid_client_count, id_state->valid_count,
                (svc_element && svc_element->name) ? svc_element->name:"UNKNOWN");
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
       fprintf(SF_DEBUG_FILE, "service_fail: State %s for protocol %u on port %u (%u->%u) AS %u, count %u, valid count %u, currSvc %s\n",
               serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
               (unsigned)pkt->src_port, (unsigned)pkt->dst_port, flow->serviceAsId, 
               id_state->invalid_client_count, id_state->valid_count,
               (svc_element && svc_element->name) ? svc_element->name:"UNKNOWN");
#else
        fprintf(SF_DEBUG_FILE, "service_fail: State %s for protocol %u on port %u (%u->%u), count %u, valid count %u, currSvc %s\n",
                serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_client_count, id_state->valid_count,
                (svc_element && svc_element->name) ? svc_element->name:"UNKNOWN");
#endif
#endif
    }
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(sfaddr_family(&flow->service_ip), (void *)sfaddr_get_ptr(&flow->service_ip), ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Fail: %s:%u:%u %p %d %s\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state,
            (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
}
#endif

    return SERVICE_SUCCESS;
}

/* Handle some exception cases on failure:
 *  - valid_count: If we have a detector that should be valid, but it keeps
 *    failing, consider restarting the detector search.
 *  - invalid_client_count: If our service detector search had trouble
 *    simply because of unrecognized client data, then consider retrying
 *    the search again. */
static void HandleFailure(tAppIdData *flowp,
                          AppIdServiceIDState *id_state,
                          sfaddr_t *client_ip,
                          SFSnortPacket *p)
{
    if (!id_state)
        return;

    /* If we had a valid detector, check for too many fails.  If so, start
     * search sequence again. */
    if (id_state->state == SERVICE_ID_VALID)
    {
        /* Too many invalid clients?  If so, count it as an invalid detect. */
        if (id_state->invalid_client_count >= STATE_ID_INVALID_CLIENT_THRESHOLD)
        {
            if (id_state->valid_count <= 1)
            {
                id_state->state = SERVICE_ID_NEW;
                id_state->invalid_client_count = 0;
                IP_CLEAR(id_state->last_invalid_client);
                id_state->valid_count = 0;
                id_state->detract_count = 0;
                IP_CLEAR(id_state->last_detract);
                id_state->svc = NULL;
            }
            else
            {
                id_state->valid_count--;
                id_state->last_invalid_client = *client_ip;
                id_state->invalid_client_count = 0;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                id_state->asId = flowp->serviceAsId;
#endif
            }
        }
        /* Just a plain old fail.  If too many of these happen, start
         * search process over. */
        else if (id_state->invalid_client_count == 0)
        {
            if (sfip_fast_eq6(&id_state->last_detract, client_ip))
                id_state->detract_count++;
            else 
            {
                id_state->last_detract = *client_ip;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                id_state->asId = flowp->serviceAsId;
#endif
            }

            if (id_state->detract_count >= STATE_ID_NEEDED_DUPE_DETRACT_COUNT)
            {
                if (id_state->valid_count <= 1)
                {
                    id_state->state = SERVICE_ID_NEW;
                    id_state->invalid_client_count = 0;
                    IP_CLEAR(id_state->last_invalid_client);
                    id_state->valid_count = 0;
                    id_state->detract_count = 0;
                    IP_CLEAR(id_state->last_detract);
                    id_state->svc = NULL;
                }
                else
                    id_state->valid_count--;
            }
        }
    }
    /* In SERVICE_ID_NEW, if port/pattern fails and not in a mid-stream, go to brute force. */
    else if (id_state->state == SERVICE_ID_NEW &&
            flowp->search_state == SERVICE_ID_PENDING &&
            (sflist_count(flowp->candidate_service_list) == 0) &&
            p && !(_dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM))
    {
        id_state->state = SERVICE_ID_BRUTE_FORCE;
    }
}

/**Changes in_process service state to failed state when a flow is terminated.
 *
 * RNA used to repeat the same service detector if the detector remained in process till the flow terminated. Thus RNA
 * got stuck on this one detector and never tried another service detector. This function will treat such a detector
 * as returning incompatibleData when the flow is terminated. The intent here to make RNA try other service detectors but
 * unlike incompatibleData status, we dont want to undermine confidence in the service.
 *
 * @note SFSnortPacket may be NULL when this function is called upon session timeout.
 */
void FailInProcessService(tAppIdData *flowp, const tAppIdConfig *pConfig)
{
    AppIdServiceIDState *id_state;
    sfaddr_t *tmp_ip;

    if (getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED|APPID_SESSION_UDP_REVERSED))
        return;
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    id_state = AppIdGetServiceIDState(&flowp->service_ip, flowp->proto, 
                                      flowp->service_port, 
                                      AppIdServiceDetectionLevel(flowp),
                                      flowp->serviceAsId, flowp->carrierId);
#else
    id_state = AppIdGetServiceIDState(&flowp->service_ip, flowp->proto,
                                      flowp->service_port, 
                                      AppIdServiceDetectionLevel(flowp), flowp->carrierId);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
    id_state = AppIdGetServiceIDState(&flowp->service_ip, flowp->proto,
                                      flowp->service_port, 
                                      AppIdServiceDetectionLevel(flowp),
                                      flowp->serviceAsId);
#else
    id_state = AppIdGetServiceIDState(&flowp->service_ip, flowp->proto,
                                      flowp->service_port, 
                                      AppIdServiceDetectionLevel(flowp));
#endif
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (flowp->service_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "FailInProcess %" PRIx64 ", %08X:%u proto %u\n",
                flowp->common.flags, sfaddr_get_ip4_value(&flowp->common.initiator_ip),
                (unsigned)flowp->service_port, (unsigned)flowp->proto);
#endif

    if (!id_state || (id_state->svc && !id_state->svc->current_ref_count))
        return;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (flowp->service_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "FailInProcess: State %s for protocol %u on port %u, count %u, %s\n",
                serviceIdStateName[id_state->state], (unsigned)flowp->proto, (unsigned)flowp->service_port,
                id_state->invalid_client_count, (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif

    id_state->invalid_client_count += STATE_ID_INCONCLUSIVE_SERVICE_WEIGHT;

#ifdef TARGET_BASED
    tmp_ip = _dpd.sessionAPI->get_session_ip_address(flowp->ssn, SSN_DIR_FROM_SERVER);
    if (sfip_fast_eq6(tmp_ip, &flowp->service_ip))
        tmp_ip = _dpd.sessionAPI->get_session_ip_address(flowp->ssn, SSN_DIR_FROM_CLIENT);
#endif

    HandleFailure(flowp, id_state, tmp_ip, 0);

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (flowp->service_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "FailInProcess: Changed State to %s for protocol %u on port %u, count %u, %s\n",
                serviceIdStateName[id_state->state], (unsigned)flowp->proto, (unsigned)flowp->service_port,
                id_state->invalid_client_count, (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif
}

/* This function should be called to find the next service detector to try when
 * we have not yet found a valid detector in the host tracker.  It will try
 * both port and/or pattern (but not brute force - that should be done outside
 * of this function).  This includes UDP reversed services.  A valid id_state
 * (even if just initialized to the NEW state) should exist before calling this
 * function.  The state coming out of this function will reflect the state in
 * which the next detector was found.  If nothing is found, it'll indicate that
 * brute force should be tried next as a state (and return NULL).  This
 * function can be called once or multiple times (to run multiple detectors in
 * parallel) per flow.  Do not call this function if a detector has already
 * been specified (serviceData).  Basically, this function handles going
 * through the main port/pattern search (and returning which detector to add
 * next to the list of detectors to try (even if only 1)). */
static const tRNAServiceElement * AppIdGetNextService(const SFSnortPacket *p,
                                                      const int dir,
                                                      tAppIdData *rnaData,
                                                      const tAppIdConfig *pConfig,
                                                      const tRNAServiceElement *lastService,
                                                      struct _SERVICE_MATCH **serviceList,
                                                      struct _SERVICE_MATCH **currentService)
{
    tRNAServiceElement *svc = NULL;
    uint8_t proto;

    proto = rnaData->proto;

    /* See if there are any port detectors to try.  If not, move onto patterns. */
    if (rnaData->search_state == SERVICE_ID_PORT)
    {
        svc = AppIdGetNextServiceByPort(proto, (uint16_t)((dir == APP_ID_FROM_RESPONDER) ? p->src_port : p->dst_port), lastService, rnaData, pConfig);
        if (svc)
            return svc;
        else
            rnaData->search_state = SERVICE_ID_PATTERN;
    }

    if (rnaData->search_state == SERVICE_ID_PATTERN)
    {
        /* If we haven't found anything yet, try to see if we get any hits
         * first with UDP reversed services before moving onto pattern matches. */
        if (dir == APP_ID_FROM_INITIATOR)
        {
            if (!getAppIdFlag(rnaData, APPID_SESSION_ADDITIONAL_PACKET) && (proto == IPPROTO_UDP)
                 && !rnaData->tried_reverse_service )
            {
                AppIdServiceIDState * reverse_id_state;
                const tRNAServiceElement * reverse_service = NULL;
                sfaddr_t * reverse_ip = GET_SRC_IP(p);
                rnaData->tried_reverse_service = true;
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                uint32_t cid = GET_SFOUTER_IPH_PROTOID(p, pkt_header);
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                if((reverse_id_state = AppIdGetServiceIDState(reverse_ip, proto, p->src_port, 
                                                              AppIdServiceDetectionLevel(rnaData), 
                                                              p->pkt_header->address_space_id_src, cid)))
#else
                if ((reverse_id_state = AppIdGetServiceIDState(reverse_ip, proto, p->src_port, AppIdServiceDetectionLevel(rnaData),
                                                               cid)))
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
                if((reverse_id_state = AppIdGetServiceIDState(reverse_ip, proto, p->src_port,
                                                              AppIdServiceDetectionLevel(rnaData),
                                                              p->pkt_header->address_space_id_src)))
#else
                if ((reverse_id_state = AppIdGetServiceIDState(reverse_ip, proto, p->src_port, AppIdServiceDetectionLevel(rnaData))))
#endif
#endif
                {
                    reverse_service = reverse_id_state->svc;
                }
                if (    reverse_service
                     || (pConfig->serviceConfig.udp_reversed_services[p->src_port] && (reverse_service = sflist_first(pConfig->serviceConfig.udp_reversed_services[p->src_port])))
                     || (p->payload_size && (reverse_service = AppIdGetServiceByPattern(p, proto, dir, &pConfig->serviceConfig, serviceList, currentService))) )
                {
                    return reverse_service;
                }
            }
            return NULL;
        }
        /* Try pattern match detectors. */
        else    /* APP_ID_FROM_RESPONDER */
        {
            if (*serviceList == NULL) // no list yet (need to make one)
                svc = AppIdGetServiceByPattern(p, proto, dir, &pConfig->serviceConfig, serviceList, currentService);
            else /* already have a pattern service list (just use it) */
            {
                svc = AppIdNextServiceByPattern(currentService
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                , flow->service_port
#endif
#endif
                                                );
            }
            if (svc)
                return svc;
            else
                rnaData->search_state = SERVICE_ID_PENDING; // We are done pattern matching
        }
    }

    /* If it was in VALID or BRUTE FORCE or no service found */
    return NULL;
}
int AppIdDiscoverService(SFSnortPacket *p, const APPID_SESSION_DIRECTION dir, tAppIdData *rnaData, const tAppIdConfig *pConfig)
{
    int ret = SERVICE_NOMATCH;
    const tRNAServiceElement *service = NULL;
    AppIdServiceIDState *id_state = NULL;
    uint8_t proto = rnaData->proto;
    SF_LNODE *node;
    ServiceValidationArgs args;
    bool bruteForceDone = false;
    bool appIdFailServiceDone = false;
    sfaddr_t *ip;
    uint16_t port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
    uint16_t asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    uint32_t cid = 0;
#endif

    /* Get packet info. */
    if (sfaddr_is_set(&rnaData->service_ip))
    {
        ip   = &rnaData->service_ip;
        port = rnaData->service_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        asId = rnaData->serviceAsId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        cid = rnaData->carrierId;
#endif
    }
    else
    {
        if (dir == APP_ID_FROM_RESPONDER)
        {
            ip   = GET_SRC_IP(p);
            port = p->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            asId = p->pkt_header->address_space_id_src;
#endif
        }
        else
        {
            ip   = GET_DST_IP(p);
            port = p->dst_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            asId = p->pkt_header->address_space_id_dst;
#endif
        }
        rnaData->service_ip = *ip;
        rnaData->service_port = port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        rnaData->serviceAsId = asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        rnaData->carrierId = GET_SFOUTER_IPH_PROTOID(p, pkt_header);
#endif
    }

    /* When a new flow was initialized, rnaData->search_state = 0 (SERVICE_ID_START) */
    if (rnaData->search_state == SERVICE_ID_START)
    {
        rnaData->search_state = SERVICE_ID_PORT; // Also ensures this block to be executed once per flow
        /* Get host tracker state. */
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
        id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                          asId, cid);
#else
        id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData), cid);
#endif        
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                          asId);
#else
        id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData));
#endif
#endif
        /* Create it if it doesn't exist yet. */
        if (id_state == NULL)
        {
            /* New one is memset to 0, hence id_state->state = 0 (SERVICE_ID_NEW) */
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            if (!(id_state = AppIdAddServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                    asId, cid)))
#else 
            if (!(id_state = AppIdAddServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData), cid)))
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            if (!(id_state = AppIdAddServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                    asId)))
#else
            if (!(id_state = AppIdAddServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData))))
#endif
#endif
            {
                _dpd.errMsg("Discover service failed to create state");
                return SERVICE_ENOMEM;
            }
        }
        /* No more searching if brute force already walked the list unsuccessfully. */
        else if (id_state->state == SERVICE_ID_BRUTE_FORCE_FAILED)
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s brute-force failed state, no service match\n", app_id_debug_session);
            AppIdServiceFailService(rnaData, p, dir, NULL, APPID_SESSION_DATA_NONE, pConfig, id_state);
            return SERVICE_NOMATCH;
        }

        if (rnaData->serviceData == NULL)
        {
            /* If a valid service already exists in host tracker, give it a try. */
            if ((id_state->svc != NULL) && (id_state->state == SERVICE_ID_VALID))
            {
                rnaData->serviceData = id_state->svc;
            }
            /* If we've gotten to brute force, give next detector a try. */
            else if ( id_state->state == SERVICE_ID_BRUTE_FORCE &&
                      (sflist_count(rnaData->candidate_service_list) == 0) )
            {
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s brute-force state\n", app_id_debug_session);
                rnaData->serviceData = AppIdGetServiceByBruteForce(proto, id_state->svc, pConfig);
                id_state->svc = rnaData->serviceData;
                bruteForceDone = true;
                if (!rnaData->serviceData)
                    id_state->state = SERVICE_ID_BRUTE_FORCE_FAILED;
            }
        }
    }

    args.data = p->payload;
    args.size = p->payload_size;
    args.dir = dir;
    args.flowp = rnaData;
    args.pkt = p;
    args.pConfig = pConfig;
    args.app_id_debug_session_flag = app_id_debug_session_flag;
    args.app_id_debug_session = app_id_debug_session;

    /* If we already have a service to try, then try it out. */
    if (rnaData->serviceData != NULL)
    {
        service = rnaData->serviceData;
        args.userdata = service->userdata;
        ret = service->validate(&args);
        if (ret == SERVICE_NOT_COMPATIBLE)
            rnaData->got_incompatible_services = 1;
        rnaData->search_state = SERVICE_ID_PENDING;
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s %s returned %d\n", app_id_debug_session,
                        service->name ? service->name:"UNKNOWN", ret);
    }
    /* Else, try to find detector(s) to use based on ports and patterns. */
    else if (!bruteForceDone)
    {
        if (rnaData->candidate_service_list == NULL)
        {
            if (!(rnaData->candidate_service_list = malloc(sizeof(SF_LIST))))
            {
                _dpd.errMsg("Could not allocate a candidate service list.");
                return SERVICE_ENOMEM;
            }
            sflist_init(rnaData->candidate_service_list);
        }

        /* See if we've got more detector(s) to add to the candidate list. */
        if ((rnaData->search_state == SERVICE_ID_PORT) ||
            ((rnaData->search_state == SERVICE_ID_PATTERN) && (dir == APP_ID_FROM_RESPONDER)))
        {
            struct _SERVICE_MATCH *serviceList = NULL;
            struct _SERVICE_MATCH *currentService = NULL;
            const tRNAServiceElement *tmp = NULL; // Also used to remember last service in the loop
            while ((tmp = AppIdGetNextService(p, dir, rnaData, pConfig, tmp, &serviceList, &currentService)))
            {
                // Add to list if not already there
                service = sflist_first(rnaData->candidate_service_list);
                while (service && (service != tmp))
                    service = sflist_next(rnaData->candidate_service_list);
                if (service == NULL)
                    sflist_add_tail(rnaData->candidate_service_list, (void*)tmp);
            }
            if (serviceList)
                AppIdFreeServiceMatchList(serviceList);
        }

        /* Run all of the detectors that we currently have. */
        ret = SERVICE_INPROCESS;
        node = sflist_first_node(rnaData->candidate_service_list);
        service = NULL;
        while (node != NULL)
        {
            int result;
            SF_LNODE *node_tmp;

            service = (tRNAServiceElement*)SFLIST_NODE_TO_DATA(node);
            args.userdata = service->userdata;
            result = service->validate(&args);
            if (result == SERVICE_NOT_COMPATIBLE)
                rnaData->got_incompatible_services = 1;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s %s returned %d\n", app_id_debug_session,
                            service->name ? service->name:"UNKNOWN", result);

            node_tmp = node;
            node = sflist_next_node(rnaData->candidate_service_list);
            if (result == SERVICE_SUCCESS)
            {
                ret = SERVICE_SUCCESS;
                rnaData->serviceData = service;
                sflist_free(rnaData->candidate_service_list);
                rnaData->candidate_service_list = NULL;
                break;    /* done */
            }
            else if (result != SERVICE_INPROCESS)    /* fail */
            {
                sflist_remove_node(rnaData->candidate_service_list, node_tmp);
            }
        }

        /* If we tried everything and found nothing, then fail. */
        if (ret != SERVICE_SUCCESS)
        {
            if (    (sflist_count(rnaData->candidate_service_list) == 0)
                 && (rnaData->search_state == SERVICE_ID_PENDING) )
            {
                if (!id_state)
                {    
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                    id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                      asId, cid);
#else
                    id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData), cid);
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
                    id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                      asId);
#else
                    id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData));
#endif
#endif
                }                    
                AppIdServiceFailService(rnaData, p, dir, NULL, APPID_SESSION_DATA_NONE, pConfig, id_state);
                appIdFailServiceDone = true;
                ret = SERVICE_NOMATCH;
            }
        }
    }

    /* We have seen bidirectional exchange and have not identified any service */
    if (!service && (dir == APP_ID_FROM_RESPONDER))
    {
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s no RNA service detector\n", app_id_debug_session);
        if (!appIdFailServiceDone)
        {
            if (!id_state)
            {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                  asId, cid);
#else
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData), cid);
#endif
#else /* No carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                  asId);
#else
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData));
#endif
#endif
            }
            AppIdServiceFailService(rnaData, p, dir, NULL, APPID_SESSION_DATA_NONE, pConfig, id_state);
            appIdFailServiceDone = true;
            ret = SERVICE_NOMATCH;
        }
    }

    if ( ((appIdFailServiceDone && !bruteForceDone) || rnaData->got_incompatible_services) &&
         (ret != SERVICE_INPROCESS) && (ret != SERVICE_SUCCESS) )
    {
        /* Handle failure exception cases in states. */
        sfaddr_t *tmp_ip;
        if (dir == APP_ID_FROM_RESPONDER)
            tmp_ip = GET_DST_IP(p);
        else
            tmp_ip = GET_SRC_IP(p);

        if (rnaData->got_incompatible_services)
        {
            if (!id_state)
            {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                  asId, cid);
#else
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData), cid);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF) 
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData),
                                                  asId);
#else
                id_state = AppIdGetServiceIDState(ip, proto, port, AppIdServiceDetectionLevel(rnaData));
#endif
#endif
            }
            if (id_state && id_state->invalid_client_count < STATE_ID_INVALID_CLIENT_THRESHOLD)
            {
                if (sfip_fast_equals_raw(&id_state->last_invalid_client, tmp_ip))
                    id_state->invalid_client_count++;
                else
                {
                    id_state->invalid_client_count += 3;
                    id_state->last_invalid_client = *tmp_ip;
                }
            }
        }

        HandleFailure(rnaData, id_state, tmp_ip, p);
    }
    return ret;
}

static void *service_flowdata_get(tAppIdData *flow, unsigned service_id)
{
    return AppIdFlowdataGet(flow, service_id);
}

static int service_flowdata_add(tAppIdData *flow, void *data, unsigned service_id, AppIdFreeFCN fcn)
{
    return AppIdFlowdataAdd(flow, data, service_id, fcn);
}

/** GUS: 2006 09 28 10:10:54
 *  A simple function that prints the
 *  ports that have decoders registered.
 */
static void dumpServices(FILE *stream, SF_LIST *const *parray)
{
    int i,n = 0;
    for(i = 0; i < RNA_SERVICE_MAX_PORT; i++)
    {
        if (parray[i] && (sflist_count(parray[i]) != 0))
        {
            if( n !=  0)
            {
                fprintf(stream," ");
            }
            n++;
            fprintf(stream,"%d",i);
        }
    }
}

void dumpPorts(FILE *stream, const tAppIdConfig *pConfig)
{
    fprintf(stream,"(tcp ");
    dumpServices(stream,pConfig->serviceConfig.tcp_services);
    fprintf(stream,") \n");
    fprintf(stream,"(udp ");
    dumpServices(stream,pConfig->serviceConfig.udp_services);
    fprintf(stream,") \n");
}

static void AppIdServiceAddMisc(tAppIdData* flow, tAppId miscId)
{
    if(flow != NULL)
        flow->miscAppId = miscId;
}
