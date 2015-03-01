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
#include "service_dcerpc.h"
#include "service_dns.h"
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
#include "service_smtp.h"
#include "service_snmp.h"
#include "service_ssh.h"
#include "service_ssl.h"
#include "service_telnet.h"
#include "service_tftp.h"
#include "detector_sip.h"
#include "service_direct_connect.h"
#include "service_battle_field.h"
#include "service_MDNS.h"
#include "service_pattern.h"
#include "luaDetectorModule.h"
#include "cpuclock.h"
#include "commonAppMatcher.h"
#include "fw_appid.h"
#include "flow.h"
#include "rna_flow.h"
#include "appIdConfig.h"

/*#define SERVICE_DEBUG 1 */
/*#define SERVICE_DEBUG_PORT  0 */

#define BUFSIZE         512

#define APP_ID_STATE_ID_INVALID_THRESHOLD   1
#define STATE_ID_INCONCLUSIVE_SERVICE_WEIGHT  3

#define RNA_SERVICE_MAX_PORT 65536

static RNAServiceElement *tcp_service_list = NULL;
static RNAServiceElement *udp_service_list = NULL;
static RNAServiceElement *udp_reversed_service_list = NULL;
    /*list nodes are RNAServiceElement*. */
static SF_LIST* tcp_services[RNA_SERVICE_MAX_PORT];
static SF_LIST* udp_services[RNA_SERVICE_MAX_PORT];
static SF_LIST* udp_reversed_services[RNA_SERVICE_MAX_PORT];

static void *service_flowdata_get(tAppIdData *flow);
static int service_flowdata_add(tAppIdData *flow, void *data, AppIdFreeFCN fcn);
static void AppIdAddHostInfo(FLOW *flow, SERVICE_HOST_INFO_CODE code, const void *info);
static int AppIdAddDHCP(FLOW *flowp, unsigned op55_len, const uint8_t *op55, unsigned op60_len, const uint8_t *op60, const uint8_t *mac);
static void AppIdAddHostIP(FLOW *flow, const uint8_t *mac, uint32_t ip4,
                                      int32_t zone, uint32_t subnetmask, uint32_t leaseSecs, uint32_t router);
static void AppIdAddSMBData(FLOW *flow, unsigned major, unsigned minor, uint32_t flags);
static void AppIdServiceAddMisc(FLOW* flow, tAppId miscId);

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
    .add_user = &AppIdAddUser,
    .add_service_consume_subtype = &AppIdServiceAddServiceSubtype,
    .add_misc = &AppIdServiceAddMisc,
};

typedef struct _PATTERN_DATA
{
    struct _PATTERN_DATA *next;
    int position;
    unsigned size;
    RNAServiceElement *svc;
} PatternData;

#ifdef SERVICE_DEBUG
static const char *serviceIdStateName[] =
{
    "NEW",
    "VALID",
    "PORT",
    "PATTERN",
    "BRUTE_FORCE",
    "BRUTE_FORCE_FAILED"
};
#endif

tAppId app_id_service_instance;

static RNAServiceElement *ftp_service = NULL;

void *tcp_patterns;
int tcp_pattern_count;
static PatternData *tcp_pattern_data;
void *udp_patterns;
int udp_pattern_count;
static PatternData *udp_pattern_data;

static PatternData *free_pattern_data;

/*C service API */
static void ServiceRegisterPattern(RNAServiceValidationFCN fcn,
                                   u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                   int position, struct _Detector *userdata, int provides_user,
                                   const char *name);
static void CServiceRegisterPattern(RNAServiceValidationFCN fcn, uint8_t proto,
                                   const uint8_t *pattern, unsigned size,
                                   int position, const char *name);
static void ServiceRegisterPatternUser(RNAServiceValidationFCN fcn, uint8_t proto,
                                       const uint8_t *pattern, unsigned size,
                                       int position, const char *name);
static int CServiceAddPort(RNAServiceValidationPort *pp, RNAServiceValidationModule *svm);
static void CServiceRemovePorts(RNAServiceValidationFCN validate);

static InitServiceAPI svc_init_api =
{
    .RegisterPattern = &CServiceRegisterPattern,
    .AddPort = &CServiceAddPort,
    .RemovePorts = CServiceRemovePorts,
    .RegisterPatternUser = &ServiceRegisterPatternUser,
    .RegisterAppId = &appSetServiceValidator,
};

extern RNAServiceValidationModule timbuktu_service_mod;
extern RNAServiceValidationModule bit_service_mod;
extern RNAServiceValidationModule tns_service_mod;

static RNAServiceValidationModule *static_service_list[] =
{
    &bgp_service_mod,
    &bootp_service_mod,
    &dcerpc_service_mod,
    &dns_service_mod,
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
    &smtp_service_mod,
    &snmp_service_mod,
    &ssh_service_mod,
    &ssl_service_mod,
    &telnet_service_mod,
    &tftp_service_mod,
    &sip_service_mod,
    &directconnect_service_mod,
    &battlefield_service_mod,
    &mdns_service_mod,
    &pattern_service_mod,
    &timbuktu_service_mod,
    &bit_service_mod,
    &tns_service_mod
};

RNAServiceValidationModule *active_service_list = NULL;

typedef struct _SERVICE_MATCH
{
    struct _SERVICE_MATCH *next;
    unsigned count;
    unsigned size;
    RNAServiceElement *svc;
} ServiceMatch;

static unsigned smOrderedListSize = 0;
static ServiceMatch **smOrderedList = NULL;
static ServiceMatch *free_service_match;

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
    PatternData *pd = (PatternData *)id;
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

tAppId getPortServiceId(uint8_t proto, uint16_t port)
{
    tAppId appId;

    if (proto == IPPROTO_TCP)
        appId = appIdConfig.tcp_port_only[port];
    else if (proto == IPPROTO_UDP)
        appId = appIdConfig.udp_port_only[port];
    else
        appId = appIdConfig.ip_protocol[proto];

    if (appId > APP_ID_NONE && appId == app_id_service_instance)
        fprintf(SF_DEBUG_FILE, "add service\n");

    return appId;
}

static inline RNAServiceElement *AppIdGetNextServiceByPort(
        uint8_t protocol,
        uint16_t port,
        const RNAServiceElement * const lastService
        )
{
    RNAServiceElement *service = NULL;
    SF_LIST *list;

    if (protocol == IPPROTO_TCP)
    {
        list = tcp_services[port];
    }
    else
    {
        list = udp_services[port];
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

static inline RNAServiceElement *AppIdNextServiceByPattern(AppIdServiceIDState *id_state
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                           , uint16_t port
#endif
#endif
                                                           )
{
    RNAServiceElement *service = NULL;

    while (id_state->currentService)
    {
        id_state->currentService = id_state->currentService->next;
        if (id_state->currentService && id_state->currentService->svc->ref_count)
        {
            service = id_state->currentService->svc;
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

const RNAServiceElement *ServiceGetServiceElement(RNAServiceValidationFCN fcn, struct _Detector *userdata)
{
    RNAServiceElement *li;

    for (li=tcp_service_list; li; li=li->next)
    {
        if ((li->validate == fcn) && (li->userdata == userdata))
            return li;
    }

    for (li=udp_service_list; li; li=li->next)
    {
        if ((li->validate == fcn) && (li->userdata == userdata))
            return li;
    }
    return NULL;
}

static void ServiceRegisterPattern(RNAServiceValidationFCN fcn,
                                   u_int8_t proto, const u_int8_t *pattern, unsigned size,
                                   int position, struct _Detector *userdata, int provides_user,
                                   const char *name)
{
    void **patterns;
    PatternData **pd_list;
    int *count;
    PatternData *pd;
    RNAServiceElement * *list;
    RNAServiceElement *li;

    if (proto == IPPROTO_TCP)
    {
        patterns = &tcp_patterns;
        count = &tcp_pattern_count;
        list = &tcp_service_list;
        pd_list = &tcp_pattern_data;
    }
    else if (proto == IPPROTO_UDP)
    {
        patterns = &udp_patterns;
        count = &udp_pattern_count;
        list = &udp_service_list;
        pd_list = &udp_pattern_data;
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
    else if ((pd=(PatternData *)calloc(1, sizeof(*pd))) == NULL)
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
    ServiceRegisterPattern(fcn, proto, pattern, size, position, userdata, 0, name);
}

static void ServiceRegisterPatternUser(RNAServiceValidationFCN fcn, uint8_t proto,
                                       const uint8_t *pattern, unsigned size,
                                       int position, const char *name)
{
    ServiceRegisterPattern(fcn, proto, pattern, size, position, NULL, 1, name);
}

static void CServiceRegisterPattern(RNAServiceValidationFCN fcn, uint8_t proto,
                                   const uint8_t *pattern, unsigned size,
                                   int position, const char *name)
{
    ServiceRegisterPattern(fcn, proto, pattern, size, position, NULL, 0, name);
}

static void RemoveServicePortsByType(RNAServiceValidationFCN validate,
                                     SF_LIST **services,
                                     RNAServiceElement *list,
                                     struct _Detector* userdata)
{
    RNAServiceElement *li, *liTmp;
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
                liTmp = (RNAServiceElement *)SFLIST_NODE_TO_DATA(node);
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

void ServiceRemovePorts(RNAServiceValidationFCN validate, struct _Detector* userdata)
{
    RemoveServicePortsByType(validate, tcp_services, tcp_service_list, userdata);
    RemoveServicePortsByType(validate, udp_services, udp_service_list, userdata);
    RemoveServicePortsByType(validate, udp_reversed_services, udp_reversed_service_list, userdata);
}

static void CServiceRemovePorts(RNAServiceValidationFCN validate)
{
    ServiceRemovePorts(validate, NULL);
}

int ServiceAddPort(RNAServiceValidationPort *pp, RNAServiceValidationModule *svm,
                   struct _Detector* userdata)
{
    SF_LIST **services;
    RNAServiceElement * *list = NULL;
    RNAServiceElement *li;
    RNAServiceElement *serviceElement;
    uint8_t isAllocated = 0;

    _dpd.debugMsg(DEBUG_LOG, "Adding service %s for protocol %u on port %u, %p",
            svm->name, (unsigned)pp->proto, (unsigned)pp->port, pp->validate);
    if (pp->proto == IPPROTO_TCP)
    {
        services = tcp_services;
        list = &tcp_service_list;
    }
    else if (pp->proto == IPPROTO_UDP)
    {
        if (!pp->reversed_validation)
        {
            services = udp_services;
            list = &udp_service_list;
        }
        else
        {
            services = udp_reversed_services;
            list = &udp_reversed_service_list;
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

static int CServiceAddPort(RNAServiceValidationPort *pp, RNAServiceValidationModule *svm)
{
    return ServiceAddPort(pp, svm, NULL);
}

int serviceLoadCallback(void *symbol)
{
    RNAServiceValidationModule *svm = (RNAServiceValidationModule *)symbol;
    RNAServiceValidationPort *pp;

    svm->api = &serviceapi;
    pp = svm->pp;
    for (pp=svm->pp; pp && pp->validate; pp++)
    {
        if (CServiceAddPort(pp, svm))
            return -1;
    }

    if (svm->init(&svc_init_api))
    {
        _dpd.errMsg("Error initializing service %s\n",svm->name);
    }

    svm->next = active_service_list;
    active_service_list = svm;
    return 0;
}

int LoadServiceModules(const char **dir_list, uint32_t instance_id)
{
    unsigned i;

    svc_init_api.csd_path = appIdCommandConfig->app_id_detector_path;
    svc_init_api.instance_id = 0;
    svc_init_api.service_instance = &app_id_service_instance;
    svc_init_api.debug = appIdCommandConfig->app_id_debug;
    svc_init_api.dpd = &_dpd;

    for (i=0; i<sizeof(static_service_list)/sizeof(*static_service_list); i++)
    {
        if (serviceLoadCallback(static_service_list[i]))
            return -1;
    }

    return 0;
}

void ServiceInit(void)
{
    luaModuleInitAllServices();
    if (tcp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(tcp_patterns);
    }
    if (udp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(udp_patterns);
    }
}

void ReconfigureServices(void)
{
    RNAServiceValidationModule *svm;
    RNAServiceElement *li;
    PatternData *pd;

    if (tcp_patterns)
    {
        _dpd.searchAPI->search_instance_free(tcp_patterns);
        tcp_patterns = NULL;
    }
    while (tcp_pattern_data)
    {
        pd = tcp_pattern_data;
        if ((li = pd->svc) != NULL)
            li->ref_count--;
        tcp_pattern_data = pd->next;
        pd->next = free_pattern_data;
        free_pattern_data = pd;
    }
    if (udp_patterns)
    {
        _dpd.searchAPI->search_instance_free(udp_patterns);
        udp_patterns = NULL;
    }
    while (udp_pattern_data)
    {
        pd = udp_pattern_data;
        if ((li = pd->svc) != NULL)
            li->ref_count--;
        udp_pattern_data = pd->next;
        pd->next = free_pattern_data;
        free_pattern_data = pd;
    }

    svc_init_api.debug = appIdCommandConfig->app_id_debug;

    for (svm=active_service_list; svm; svm=svm->next)
    {
        if (svm->clean)
            svm->clean();
    }

    for (svm=active_service_list; svm; svm=svm->next)
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

    ServiceInit();
}

void CleanupServices(void)
{
#ifdef RNA_FULL_CLEANUP
    PatternData *pattern;
    RNAServiceElement *se;
    ServiceMatch *sm;
    unsigned i;
    RNAServiceValidationModule *svm;

    if (tcp_patterns)
    {
        _dpd.searchAPI->search_instance_free(tcp_patterns);
        tcp_patterns = NULL;
    }
    if (udp_patterns)
    {
        _dpd.searchAPI->search_instance_free(udp_patterns);
        udp_patterns = NULL;
    }
    while ((pattern=tcp_pattern_data) != NULL)
    {
        tcp_pattern_data = pattern->next;
        free(pattern);
    }
    while ((pattern=udp_pattern_data) != NULL)
    {
        udp_pattern_data = pattern->next;
        free(pattern);
    }
    while ((pattern=free_pattern_data) != NULL)
    {
        free_pattern_data = pattern->next;
        free(pattern);
    }
    while ((se=tcp_service_list) != NULL)
    {
        tcp_service_list = se->next;
        free(se);
    }
    while ((se=udp_service_list) != NULL)
    {
        udp_service_list = se->next;
        free(se);
    }
    while ((se=udp_reversed_service_list) != NULL)
    {
        udp_reversed_service_list = se->next;
        free(se);
    }
    while ((sm = free_service_match))
    {
        free_service_match = sm->next;
        free(sm);
    }

    cleanupFreeServiceMatch();
    if (smOrderedList)
     {
        free(smOrderedList);
        smOrderedListSize = 0;
     }

    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if (tcp_services[i])
        {
            sflist_free(tcp_services[i]);
            free(tcp_services[i]);
        }
    }
    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if (udp_services[i])
        {
            sflist_free(udp_services[i]);
            free(udp_services[i]);
        }
    }
    for (i=0; i<RNA_SERVICE_MAX_PORT; i++)
    {
        if (udp_reversed_services[i])
        {
            sflist_free(udp_reversed_services[i]);
            free(udp_reversed_services[i]);
        }
    }

    for (svm=active_service_list; svm; svm=svm->next)
    {
        if (svm->clean)
            svm->clean();
    }
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
static inline RNAServiceElement *AppIdGetServiceByPattern(const SFSnortPacket *pkt, uint8_t proto,
                                                          const int dir, AppIdServiceIDState *id_state)
{
    void *patterns = NULL;
    ServiceMatch *match_list;
    ServiceMatch *sm;
    uint32_t count;
    uint32_t i;
    RNAServiceElement *service = NULL;

    if (proto == IPPROTO_TCP)
        patterns = tcp_patterns;
    else
        patterns = udp_patterns;

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

    if (id_state)
    {
        id_state->svc = service;
        id_state->serviceList = smOrderedList[0];
        id_state->currentService = smOrderedList[0];
    }
    else
        AppIdFreeServiceMatchList(smOrderedList[0]);

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

static inline RNAServiceElement * AppIdGetServiceByBruteForce(
        uint32_t protocol,
        const RNAServiceElement *lastService
        )
{
    RNAServiceElement *service;

    if (lastService)
        service = lastService->next;
    else
        service = ((protocol == IPPROTO_TCP) ? tcp_service_list:udp_service_list);

    while (service && !service->ref_count)
        service = service->next;

    return service;
}

static void AppIdAddHostInfo(FLOW *flow, SERVICE_HOST_INFO_CODE code, const void *info)
{
}

static int AppIdAddDHCP(FLOW *flowp, unsigned op55_len, const uint8_t *op55, unsigned op60_len, const uint8_t *op60, const uint8_t *mac)
{
    return 0;
}

static void AppIdAddHostIP(FLOW *flow, const uint8_t *mac, uint32_t ip, int32_t zone,
                                      uint32_t subnetmask, uint32_t leaseSecs, uint32_t router)
{
    return;
}

static void AppIdAddSMBData(FLOW *flow, unsigned major, unsigned minor, uint32_t flags)
{
}

static int AppIdServiceAddServiceEx(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                    const RNAServiceElement *svc_element,
                                    tAppId appId, const char *vendor, const char *version)
{
    AppIdServiceIDState *id_state;

    if (!flow || !pkt || !svc_element)
    {
        _dpd.errMsg( "Invalid arguments to absinthe_add_appId");
        return SERVICE_EINVALID;
    }

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
    flow_mark(flow, FLOW_SERVICEDETECTED);
    flow->serviceAppId = appId;

    if (appId > APP_ID_NONE && appId == app_id_service_instance)
        fprintf(SF_DEBUG_FILE, "add service\n");

    if (flow_checkflag(flow, FLOW_IGNORE_HOST))
        return SERVICE_SUCCESS;

    if (!(id_state = flow->id_state))
    {
        uint16_t port;
        snort_ip *ip;

        if (!flow_checkflag(flow, FLOW_UDP_REVERSED))
        {
            if (dir == APP_ID_FROM_INITIATOR)
            {
                ip = GET_DST_IP(pkt);
                port = pkt->dst_port;
            }
            else
            {
                ip = GET_SRC_IP(pkt);
                port = pkt->src_port;
            }
        }
        else
        {
            if (dir == APP_ID_FROM_INITIATOR)
            {
                ip = GET_SRC_IP(pkt);
                port = pkt->src_port;
            }
            else
            {
                ip = GET_DST_IP(pkt);
                port = pkt->dst_port;
            }
        }
        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port)))
        {
            _dpd.errMsg( "Add service failed to create state");
            return SERVICE_ENOMEM;
        }
        flow->id_state = id_state;
        flow->service_ip = *ip;
        flow->service_port = port;
    }
    else
    {
        id_state->invalid_client_count = 0;
        IP_CLEAR(id_state->last_invalid_client);
        if (id_state->serviceList)
        {
            AppIdFreeServiceMatchList(id_state->serviceList);
            id_state->serviceList = NULL;
            id_state->currentService = NULL;
        }
        if (!flow->service_ip.family)
        {
            uint16_t port;
            snort_ip *ip;

            if (!flow_checkflag(flow, FLOW_UDP_REVERSED))
            {
                if (dir == APP_ID_FROM_INITIATOR)
                {
                    ip = GET_DST_IP(pkt);
                    port = pkt->dst_port;
                }
                else
                {
                    ip = GET_SRC_IP(pkt);
                    port = pkt->src_port;
                }
            }
            else
            {
                if (dir == APP_ID_FROM_INITIATOR)
                {
                    ip = GET_SRC_IP(pkt);
                    port = pkt->src_port;
                }
                else
                {
                    ip = GET_DST_IP(pkt);
                    port = pkt->dst_port;
                }
            }
            flow->service_ip = *ip;
            flow->service_port = port;
        }
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
#endif
    }
    id_state->reset_time = 0;
    id_state->state = SERVICE_ID_VALID;
    id_state->svc = svc_element;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(flow->service_ip.family, (void *)flow->service_ip.ip32, ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Valid: %s:%u:%u %p %d\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port, id_state, (int)id_state->state);
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
    else if (id_state->valid_count < APP_ID_MAX_VALID_COUNT)
        id_state->valid_count++;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid\n",
                (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
#endif
    return SERVICE_SUCCESS;
}

int AppIdServiceAddServiceSubtype(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                  const RNAServiceElement *svc_element,
                                  tAppId appId, const char *vendor, const char *version,
                                  RNAServiceSubtype *subtype)
{
    flow->subtype = subtype;
    if (!svc_element->ref_count)
    {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
#endif
        return SERVICE_SUCCESS;
    }
    return AppIdServiceAddServiceEx(flow, pkt, dir, svc_element, appId, vendor, version);
}

int AppIdServiceAddService(FLOW *flow, const SFSnortPacket *pkt, int dir,
                           const RNAServiceElement *svc_element,
                           tAppId appId, const char *vendor, const char *version,
                           const RNAServiceSubtype *subtype)
{
    RNAServiceSubtype *new_subtype = NULL;
    RNAServiceSubtype *tmp_subtype;

    if (!svc_element->ref_count)
    {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Service %d for protocol %u on port %u (%u->%u) is valid, but skipped\n",
                    (int)appId, (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port);
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
    return AppIdServiceAddServiceEx(flow, pkt, dir, svc_element, appId, vendor, version);
}

int AppIdServiceInProcess(FLOW *flow, const SFSnortPacket *pkt, int dir,
                          const RNAServiceElement *svc_element)
{
    AppIdServiceIDState *id_state;

    if (!flow || !pkt)
    {
        _dpd.errMsg( "Invalid arguments to service_in_process");
        return SERVICE_EINVALID;
    }

    if (dir == APP_ID_FROM_INITIATOR || flow_checkflag(flow, FLOW_IGNORE_HOST|FLOW_UDP_REVERSED))
        return SERVICE_SUCCESS;

    if (!(id_state = flow->id_state))
    {
        uint16_t port;
        snort_ip *ip;

        ip = GET_SRC_IP(pkt);
        port = pkt->src_port;

        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port)))
        {
            _dpd.errMsg( "Add service failed to create state");
            return SERVICE_ENOMEM;
        }
        flow->id_state = id_state;
        flow->service_ip = *ip;
        flow->service_port = port;
        id_state->state = SERVICE_ID_NEW;
        id_state->svc = svc_element;
    }
    else
    {
        if (!flow->service_ip.family)
        {
            snort_ip *ip = GET_SRC_IP(pkt);
            flow->service_ip = *ip;
            flow->service_port = pkt->src_port;
        }
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Service for protocol %u on port %u is in process (%u->%u), %p %s",
                    (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port,
                    svc_element->validate, svc_element->name ? :"UNKNOWN");
#endif
    }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(flow->service_ip.family, (void *)flow->service_ip.ip32, ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Inprocess: %s:%u:%u %p %d\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state);
}
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Service for protocol %u on port %u is in process (%u->%u), %s\n",
                (unsigned)flow->proto, (unsigned)flow->service_port, (unsigned)pkt->src_port, (unsigned)pkt->dst_port,
                svc_element->name ? :"UNKNOWN");
#endif

    return SERVICE_SUCCESS;
}

/**Called when service can not be identified on a flow but the checks failed on client request
 * rather than server response. When client request fails a check, it may be specific to a client
 * therefore we should not fail the service right away. If the same behavior is seen from the same
 * client ultimately we will have to fail the service. If the same behavior is seen from different
 * clients going to same service then this most likely the service is something else.
 */
int AppIdServiceIncompatibleData(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                 const RNAServiceElement *svc_element)
{
    AppIdServiceIDState *id_state;
    snort_ip *tmp_ip;

    if (!flow || !pkt)
    {
        _dpd.errMsg( "Invalid arguments to service_incompatible_data");
        return SERVICE_EINVALID;
    }

    flow_mark(flow, FLOW_SERVICEDETECTED);
    flow_clear(flow, FLOW_CONTINUE);
    AppIdFlowdataDelete(flow, FLOW_DATA_ID_SVC_FLOWSTATE);

    flow->serviceAppId = APP_ID_NONE;

    if (flow_checkflag(flow, FLOW_IGNORE_HOST|FLOW_UDP_REVERSED) || (svc_element && !svc_element->ref_count))
        return SERVICE_SUCCESS;

    if (dir == APP_ID_FROM_INITIATOR)
    {
        flow_mark(flow, FLOW_INCOMPATIBLE);
        return SERVICE_SUCCESS;
    }

    if (!(id_state = flow->id_state))
    {
        uint16_t port;
        snort_ip *ip;

        ip = GET_SRC_IP(pkt);
        port = pkt->src_port;

        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port)))
        {
            _dpd.errMsg( "Add service failed to create state");
            return SERVICE_ENOMEM;
        }
        flow->id_state = id_state;
        flow->service_ip = *ip;
        flow->service_port = port;
        id_state->state = SERVICE_ID_NEW;
        id_state->svc = svc_element;
    }
    else
    {
        if (!flow->service_ip.family)
        {
            snort_ip *ip = GET_SRC_IP(pkt);
            flow->service_ip = *ip;
            flow->service_port = pkt->src_port;
    #ifdef SERVICE_DEBUG
    #if SERVICE_DEBUG_PORT
            if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
    #endif
                fprintf(SF_DEBUG_FILE, "service_IC: Changed State to %s for protocol %u on port %u (%u->%u), count %u, %s\n",
                        serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                        (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_count,
                        (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
    #endif
        }
        id_state->reset_time = 0;
    }

    if (id_state->invalid_client_count < STATE_ID_INVALID_CLIENT_THRESHOLD)
    {
        tmp_ip = GET_DST_IP(pkt);
        if (sfip_fast_equals_raw(&id_state->last_invalid_client, tmp_ip))
            id_state->invalid_client_count++;
        else
        {
            id_state->invalid_client_count += 3;
            id_state->last_invalid_client = *tmp_ip;
        }
    }
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "service_IC: State %s for protocol %u on port %u (%u->%u), count %u, %s\n",
                serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_client_count,
                (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(flow->service_ip.family, (void *)flow->service_ip.ip32, ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Incompat: %s:%u:%u %p %d %s\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state,
            (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
}
#endif

    /*brute force phase each detector just once irrespective of failure type */
    if (id_state->invalid_client_count >= STATE_ID_INVALID_CLIENT_THRESHOLD ||
        id_state->state == SERVICE_ID_BRUTE_FORCE ||
        id_state->state == SERVICE_ID_BRUTE_FORCE_FAILED)
    {
        IP_CLEAR(id_state->last_invalid_client);
        id_state->invalid_client_count = 0;

        switch (id_state->state)
        {
        case SERVICE_ID_NEW:
            id_state->state = SERVICE_ID_PORT;
            id_state->svc = AppIdGetNextServiceByPort(flow->proto, flow->service_port, NULL);
            break;

        case SERVICE_ID_PORT:
            /*iterate over the same service n times and then move to the next */
            /*in list. At list end, move to next stage. */
            id_state->svc = AppIdGetNextServiceByPort(flow->proto, flow->service_port, svc_element);
            if (!id_state->svc)
                id_state->state = SERVICE_ID_PATTERN;
            break;

        case SERVICE_ID_PATTERN:
            /*iterate over the same service n times and then move to the next */
            /*in list. At list end, move to next stage. */
            id_state->svc = AppIdNextServiceByPattern(id_state
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                           , flow->service_port
#endif
#endif
                                                      );
            if (!id_state->svc)
            {
                id_state->state = SERVICE_ID_BRUTE_FORCE;
                id_state->svc = AppIdGetServiceByBruteForce(flow->proto, NULL);
                AppIdFreeServiceMatchList(id_state->serviceList);
                id_state->serviceList = NULL;
                id_state->currentService = NULL;
            }
            break;

        case SERVICE_ID_BRUTE_FORCE:
        case SERVICE_ID_BRUTE_FORCE_FAILED:
            /*fall into this every 3rd time */
            if (id_state->svc)
            {
                id_state->svc = AppIdGetServiceByBruteForce(flow->proto, id_state->svc);
            }
            if (!id_state->svc)
            {
                id_state->state = SERVICE_ID_BRUTE_FORCE_FAILED;
                id_state->svc = AppIdGetServiceByBruteForce(flow->proto, NULL);
                flow->serviceAppId = APP_ID_UNKNOWN;
            }
            break;

        case SERVICE_ID_VALID:
            tmp_ip = GET_DST_IP(pkt);
            id_state->last_invalid_client = *tmp_ip;
            if (id_state->valid_count <= 1)
            {
                id_state->state = SERVICE_ID_PORT;
                id_state->svc = AppIdGetNextServiceByPort(flow->proto, flow->service_port, NULL);
                id_state->invalid_count = 0;
                id_state->invalid_client_count = 0;
                IP_CLEAR(id_state->last_invalid_client);
                id_state->valid_count = 0;
                id_state->detract_count = 0;
                IP_CLEAR(id_state->last_detract);
            }
            else
                id_state->valid_count--;
            break;
        default:
            break;
        }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "service_IC: Changed State to %s for protocol %u on port %u (%u->%u), count %u, %s\n",
                    serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_count,
                    (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif
    }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(flow->service_ip.family, (void *)flow->service_ip.ip32, ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Incompat End: %s:%u:%u %p %d %s\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state, (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
}
#endif

    return SERVICE_SUCCESS;
}

int AppIdServiceFailService(FLOW* flow, const SFSnortPacket *pkt, int dir,
                            const RNAServiceElement *svc_element)
{
    AppIdServiceIDState *id_state;
    snort_ip *tmp_ip;
#ifdef SERVICE_DEBUG
    int isStateChanged = 0;
#endif

    flow->serviceAppId = APP_ID_NONE;

    flow_mark(flow, FLOW_SERVICEDETECTED);
    flow_clear(flow, FLOW_CONTINUE);
    AppIdFlowdataDelete(flow, FLOW_DATA_ID_SVC_FLOWSTATE);

    /*detectors should be careful in marking flow UDP_REVERSED otherwise the same detector */
    /*gets all future flows. UDP_REVERSE should be marked only when detector positively */
    /*matches opposite direction patterns. */

    if (flow_checkflag(flow, FLOW_IGNORE_HOST | FLOW_UDP_REVERSED) || (svc_element && !svc_element->ref_count))
        return SERVICE_SUCCESS;

    /*For subsequent packets, avoid marking service failed on client packet, */
    /*otherwise the service will show up on client side. */
    if (dir == APP_ID_FROM_INITIATOR)
    {
        flow_mark(flow, FLOW_INCOMPATIBLE);
        return SERVICE_SUCCESS;
    }

    if (!(id_state = flow->id_state))
    {
        uint16_t port;
        snort_ip *ip;

        ip = GET_SRC_IP(pkt);
        port = pkt->src_port;

        if (!(id_state = AppIdAddServiceIDState(ip, flow->proto, port)))
        {
            _dpd.errMsg( "Fail service failed to create state");
            return SERVICE_ENOMEM;
        }
        flow->id_state = id_state;
        flow->service_ip = *ip;
        flow->service_port = port;
        id_state->state = SERVICE_ID_NEW;
        id_state->svc = svc_element;
    }
    else
    {
        if (!flow->service_ip.family)
        {
            snort_ip *ip = GET_SRC_IP(pkt);
            flow->service_ip = *ip;
            flow->service_port = pkt->src_port;
        }
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "service_fail: State %s for protocol %u on port %u (%u->%u), count %u, valid count %u, currSvc %s\n",
                    serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_count, id_state->valid_count,
                    (svc_element && svc_element->name) ? svc_element->name:"UNKNOWN");
#endif
    }
    id_state->reset_time = 0;

    IP_CLEAR(id_state->last_invalid_client);
    id_state->invalid_client_count = 0;

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "service_fail: State %s for protocol %u on port %u (%u->%u), count %u, valid count %u, currSvc %s\n",
                serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_count, id_state->valid_count,
                (svc_element && svc_element->name) ? svc_element->name:"UNKNOWN");
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(flow->service_ip.family, (void *)flow->service_ip.ip32, ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Fail: %s:%u:%u %p %d %s\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state,
            (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
}
#endif

    switch (id_state->state)
    {
    case SERVICE_ID_NEW:
        id_state->state = SERVICE_ID_PORT;
        id_state->invalid_count = 0;
#ifdef SERVICE_DEBUG
        isStateChanged = 1;
#endif
    case SERVICE_ID_PORT:
        id_state->svc = AppIdGetNextServiceByPort(flow->proto, flow->service_port, svc_element);
        if (!id_state->svc)
        {
            id_state->invalid_count++;
            if (id_state->invalid_count >= APP_ID_STATE_ID_INVALID_THRESHOLD)
            {
                /*change state after failing STATE_ID_INVALID_THRESHOLD times */
                id_state->state = SERVICE_ID_PATTERN;
                id_state->svc = NULL;
                id_state->invalid_count = 0;
#ifdef SERVICE_DEBUG
                isStateChanged = 1;
#endif
            }
            else
            {
                id_state->svc = AppIdGetNextServiceByPort(flow->proto, flow->service_port, NULL);
            }
        }
        break;

    case SERVICE_ID_PATTERN:
        /*Instead of going to bruteforce, iterate over the list */
        /*Ideally, we should have a single list for port and pattern. During pattern phase, RNA */
        /*adds pattern detectors that were not tried, during brute force, append the remaining */
        /*detectors. */
        id_state->svc = AppIdNextServiceByPattern(id_state
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                  , flow->service_port
#endif
#endif
                                                  );
        if (!id_state->svc)
        {
            id_state->invalid_count++;
            if (id_state->invalid_count >= APP_ID_STATE_ID_INVALID_THRESHOLD)
            {
                /*change state after failing STATE_ID_INVALID_THRESHOLD times */
                id_state->svc = NULL;
                id_state->invalid_count = 0;
                AppIdFreeServiceMatchList(id_state->serviceList);
                id_state->serviceList = NULL;
                id_state->currentService = NULL;

                id_state->state = SERVICE_ID_BRUTE_FORCE;
                id_state->svc = AppIdGetServiceByBruteForce(flow->proto, NULL);

#ifdef SERVICE_DEBUG
                isStateChanged = 1;
#endif
            }
            else
            {
                id_state->svc = AppIdNextServiceByPattern(id_state
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                          , flow->service_port
#endif
#endif
                                                          );
            }
        }
        break;

    case SERVICE_ID_BRUTE_FORCE:
    case SERVICE_ID_BRUTE_FORCE_FAILED:
        /*Keep iterating over service list till success. */
        if (id_state->svc)
        {
            /*try next service */
            id_state->svc = AppIdGetServiceByBruteForce(flow->proto, id_state->svc);
        }
        if (!id_state->svc)
        {
            /*start over again */
            id_state->state = SERVICE_ID_BRUTE_FORCE_FAILED;
            id_state->svc = AppIdGetServiceByBruteForce(flow->proto, NULL);
            flow->serviceAppId = APP_ID_UNKNOWN;
        }
        break;

    case SERVICE_ID_VALID:
        tmp_ip = GET_DST_IP(pkt);
        if (sfip_fast_equals_raw(&id_state->last_detract, tmp_ip))
        {
            id_state->detract_count++;
            if (id_state->detract_count < APP_ID_NEEDED_DUPE_DETRACT_COUNT)
                break;
        }
        else
            id_state->last_detract = *tmp_ip;
        if (id_state->valid_count <= 1)
        {
            id_state->state = SERVICE_ID_PORT;
            id_state->invalid_count = 0;
            id_state->invalid_client_count = 0;
            IP_CLEAR(id_state->last_invalid_client);
            id_state->svc = AppIdGetNextServiceByPort(flow->proto, flow->service_port, NULL);
            /*changed to pattern */
            id_state->valid_count = 0;
            id_state->detract_count = 0;
            IP_CLEAR(id_state->last_detract);
#ifdef SERVICE_DEBUG
            isStateChanged = 1;
#endif
        }
        else
            id_state->valid_count--;
        break;
    default:
        break;
    }

#ifdef SERVICE_DEBUG
    if (isStateChanged)
    {
#if SERVICE_DEBUG_PORT
        if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "service_fail: Changed State to %s for protocol %u on port %u (%u->%u), count %u, %s\n",
                    serviceIdStateName[id_state->state], (unsigned)flow->proto, (unsigned)flow->service_port,
                    (unsigned)pkt->src_port, (unsigned)pkt->dst_port, id_state->invalid_count,
                    (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
    }
#endif

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
if (pkt->dst_port == SERVICE_DEBUG_PORT || pkt->src_port == SERVICE_DEBUG_PORT)
#endif
{
    char ipstr[INET6_ADDRSTRLEN];

    ipstr[0] = 0;
    inet_ntop(flow->service_ip.family, (void *)flow->service_ip.ip32, ipstr, sizeof(ipstr));
    fprintf(SF_DEBUG_FILE, "Fail End: %s:%u:%u %p %d %s\n", ipstr, (unsigned)flow->proto, (unsigned)flow->service_port,
            id_state, (int)id_state->state, (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
}
#endif

    return SERVICE_SUCCESS;
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
void FailInProcessService(tAppIdData *flowp)
{
    AppIdServiceIDState *id_state;
    snort_ip *tmp_ip;

    if (flow_checkflag(flowp, FLOW_SERVICEDETECTED|FLOW_UDP_REVERSED))
        return;

    id_state = AppIdGetServiceIDState(&flowp->service_ip, flowp->proto, flowp->service_port);

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (flowp->service_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "FailInProcess %08X, %08X:%u proto %u\n",
                flowp->common.flow_flags, flowp->common.initiator_ip.ip.u6_addr32[0],
                (unsigned)flowp->service_port, (unsigned)flowp->proto);
#endif

    if (!id_state || (id_state->svc && !id_state->svc->ref_count))
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

    /*brute force phase each detector just once irrespective of failure type */
    if ((id_state->invalid_client_count >= STATE_ID_INVALID_CLIENT_THRESHOLD)
        || ((id_state->state == SERVICE_ID_BRUTE_FORCE) ||
            (id_state->state == SERVICE_ID_BRUTE_FORCE_FAILED)))
    {
        id_state->invalid_client_count = 0;

        switch (id_state->state)
        {
            case SERVICE_ID_NEW:
                id_state->state = SERVICE_ID_PORT;
                id_state->svc = AppIdGetNextServiceByPort(flowp->proto, flowp->service_port, NULL);
                break;

            case SERVICE_ID_PORT:
                id_state->svc = AppIdGetNextServiceByPort(flowp->proto, flowp->service_port, flowp->serviceData);
                if (!id_state->svc)
                {
                    id_state->state = SERVICE_ID_PATTERN;
                }
                break;

            case SERVICE_ID_PATTERN:
                /*iterate over the same service n times and then move to the next */
                /*in list. At list end, move to next stage. */
                id_state->svc = AppIdNextServiceByPattern(id_state
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                                                          , flowp->service_port
#endif
#endif
                                                          );
                if (!id_state->svc)
                {
                    id_state->state = SERVICE_ID_BRUTE_FORCE;
                    id_state->svc = AppIdGetServiceByBruteForce(flowp->proto, NULL);
                    AppIdFreeServiceMatchList(id_state->serviceList);
                    id_state->serviceList = NULL;
                    id_state->currentService = NULL;
                }
                break;

            case SERVICE_ID_BRUTE_FORCE:
            case SERVICE_ID_BRUTE_FORCE_FAILED:
                /*Keep iterating over service list till success. */
                if (id_state->svc)
                {
                    /*try next service */
                    id_state->svc = AppIdGetServiceByBruteForce(flowp->proto, id_state->svc);
                }

                if (!id_state->svc)
                {
                    /*start over again */
                    id_state->state = SERVICE_ID_BRUTE_FORCE_FAILED;
                    id_state->svc = AppIdGetServiceByBruteForce(flowp->proto, NULL);
                }

                break;

            case SERVICE_ID_VALID:
                tmp_ip = _dpd.sessionAPI->get_session_ip_address(flowp->ssn, SSN_DIR_FROM_SERVER);
                if (sfip_fast_equals_raw(tmp_ip, &flowp->service_ip))
                    tmp_ip = _dpd.sessionAPI->get_session_ip_address(flowp->ssn, SSN_DIR_FROM_CLIENT);
                if (sfip_fast_equals_raw(tmp_ip, &id_state->last_detract))
                {
                    id_state->detract_count++;
                    if (id_state->detract_count < APP_ID_NEEDED_DUPE_DETRACT_COUNT)
                        break;
                }
                else
                    id_state->last_detract = *tmp_ip;
                if (id_state->valid_count <= 1)
                {
                    id_state->state = SERVICE_ID_PORT;
                    /*changed to pattern */
                    id_state->invalid_count = 0;
                    id_state->invalid_client_count = 0;
                    IP_CLEAR(id_state->last_invalid_client);
                    id_state->svc = AppIdGetNextServiceByPort(flowp->proto, flowp->service_port, NULL);
                    /*changed to pattern */
                    id_state->valid_count = 0;
                    id_state->detract_count = 0;
                    IP_CLEAR(id_state->last_detract);
                }
                else
                    id_state->valid_count--;
                break;
            default:
                break;
        }
    }

#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
    if (flowp->service_port == SERVICE_DEBUG_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "FailInProcess: Changed State to %s for protocol %u on port %u, count %u, %s\n",
                serviceIdStateName[id_state->state], (unsigned)flowp->proto, (unsigned)flowp->service_port,
                id_state->invalid_client_count, (id_state->svc && id_state->svc->name) ? id_state->svc->name:"UNKNOWN");
#endif
}

int AppIdDiscoverService(const SFSnortPacket *p, const int dir, tAppIdData *rnaData)
{
    snort_ip *ip;
    int ret = SERVICE_NOMATCH;
    const RNAServiceElement *service;
    AppIdServiceIDState *id_state;
    uint8_t proto;

    proto = rnaData->proto;

    rnaData->id_state = NULL;

    /*get service if a detector is already being evaluated for this flow. */
    service = rnaData->serviceData;
    if (rnaData->service_ip.family)
    {
        if ((id_state = AppIdGetServiceIDState(&rnaData->service_ip, proto, rnaData->service_port)))
            rnaData->id_state = id_state;
        if (!service && id_state)
            service = id_state->svc;
    }

    if (!service && !rnaData->id_state && !flow_checkflag(rnaData, FLOW_ADDITIONAL_PACKET) && proto == IPPROTO_UDP)
    {
        /*new UDP flow, first packet */
        /*From response host tracker. Previously identified service. */
        /*select service from response hosts */
        ip = GET_DST_IP(p);
        if ((id_state = AppIdGetServiceIDState(ip, proto, p->dst_port)))
        {
            rnaData->id_state = id_state;
            service = id_state->svc;
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
            if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                fprintf(SF_DEBUG_FILE, "Host state service for protocol %u port %u, %s\n", (unsigned)proto,
                        (unsigned)p->dst_port, (service && service->name) ? service->name:"UNKNOWN");
#endif
            goto server_service;
        }

        /*this section matches the first packet as if it were a response from a server. RNA basically */
        /*missed the first packet. If service is not identified then control goes down further. */
        ip = GET_SRC_IP(p);
        if ((id_state = AppIdGetServiceIDState(ip, proto, p->src_port)))
        {
            rnaData->id_state = id_state;
            service = id_state->svc;
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
            if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                fprintf(SF_DEBUG_FILE, "Host state init host service for protocol %u port %u, %s\n", (unsigned)proto,
                        (unsigned)p->src_port, (service && service->name) ? service->name:"UNKNOWN");
#endif
        }

        /*udp_reversedServices or pattern. */
        if (service
                || (udp_reversed_services[p->src_port] && (service = sflist_first(udp_reversed_services[p->src_port])))
                || (p->payload_size && (service = AppIdGetServiceByPattern(p, proto, dir, NULL))))
        {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
            if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                fprintf(SF_DEBUG_FILE, "InitHost, ReversedServices or pattern, protocol %u port %u, %s\n", (unsigned)proto,
                        (unsigned)p->src_port, service->name ? service->name:"UNKNOWN");
#endif
            /*this is a new flow so we can add flow data unconditionally? */
            ret = service->validate(p->payload, p->payload_size, dir, rnaData, p, service->userdata);
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
            if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                fprintf(SF_DEBUG_FILE, "Reversed service validate %u -> %u %u returned %d\n",
                        (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)proto, ret);
#endif
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s reversed %s returned %d\n", app_id_debug_session,
                            service->name ? service->name:"UNKNOWN", ret);
            if (ret < 0)
                return ret;
            if (ret == SERVICE_SUCCESS || ret == SERVICE_INPROCESS)
            {
                rnaData->serviceData = service;
                if (ret == SERVICE_SUCCESS)
                    flow_mark(rnaData, FLOW_SERVICEDETECTED);
                return ret;
            }
            AppIdFlowdataDelete(rnaData, FLOW_DATA_ID_SVC_FLOWSTATE);
            rnaData->id_state = NULL;
            rnaData->service_ip.family = 0;
            service = NULL;
        }
    }

server_service:
    /*UDP and TCP services. The checks above are additional checks for new UDP service. */

    if (!service && !rnaData->id_state)
    {
        if (dir == APP_ID_FROM_RESPONDER)
        {
            ip = GET_SRC_IP(p);
            id_state = AppIdGetServiceIDState(ip, proto, p->src_port);
        }
        else
        {
            ip = GET_DST_IP(p);
            id_state = AppIdGetServiceIDState(ip, proto, p->dst_port);
        }

        if (id_state)
        {
            /*Service identification is under process. service_fail is driving the next */
            /*service except when entering pattern matching phase since service_fail */
            /*can not construct the service list based on pattern matches. */

            rnaData->id_state = id_state;
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
            if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                fprintf(SF_DEBUG_FILE, "Service identification in progress TCP/UDP flow %d\n", (int)id_state->state);
#endif
            if (id_state->state == SERVICE_ID_PATTERN || id_state->state == SERVICE_ID_NEW)
            {
                if ((dir != APP_ID_FROM_RESPONDER) || (!p->payload_size))
                    return SERVICE_INPROCESS;

                if (!id_state->svc)
                {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                    if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                        fprintf(SF_DEBUG_FILE, "Service identification checking pattern\n");
#endif
                    id_state->svc = AppIdGetServiceByPattern(p, proto, dir, id_state);
                }
            }

            /*brute force */
            if (!id_state->svc)
            {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "Service identification using brute force\n");
#endif
                id_state->svc = AppIdGetServiceByBruteForce(proto, NULL);
            }

            service = id_state->svc;
        }
        else
        {
            /*First instance of a flow. Identify candidate detector in port, pattern, brute */
            /*force order. */
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
            if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                fprintf(SF_DEBUG_FILE, "First instance of TCP/UDP flow\n");
#endif

            /*get service registered for server port */
            service = AppIdGetNextServiceByPort(proto, (uint16_t)((dir == APP_ID_FROM_RESPONDER) ? p->src_port : p->dst_port), NULL);

            /*pattern matching */
            if (!service)
            {
                if (dir != APP_ID_FROM_RESPONDER)
                    return SERVICE_INPROCESS;
                if (!p->payload_size)
                    return SERVICE_INPROCESS;
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "Service identification checking pattern\n");
#endif
                service = AppIdGetServiceByPattern(p, proto, dir, id_state);
            }

            /*brute force */
            if (!service)
            {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
                if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "Service identification using brute force\n");
#endif
                service = AppIdGetServiceByBruteForce(proto, NULL);
            }
        }
    }
    else if (!rnaData->id_state)
    {
        uint16_t port;

        if (!flow_checkflag(rnaData, FLOW_ADDITIONAL_PACKET) && proto == IPPROTO_UDP)
        {
            ip = GET_DST_IP(p);
            if ((id_state = AppIdGetServiceIDState(ip, proto, p->dst_port)))
                rnaData->id_state = id_state;
            else
            {
                ip = GET_SRC_IP(p);
                if ((id_state = AppIdGetServiceIDState(ip, proto, p->src_port)))
                    rnaData->id_state = id_state;
            }
        }
        else
        {
            if (!flow_checkflag(rnaData, FLOW_UDP_REVERSED))
            {
                if (dir == APP_ID_FROM_INITIATOR)
                {
                    ip = GET_DST_IP(p);
                    port = p->dst_port;
                }
                else
                {
                    ip = GET_SRC_IP(p);
                    port = p->src_port;
                }
            }
            else
            {
                if (dir == APP_ID_FROM_INITIATOR)
                {
                    ip = GET_SRC_IP(p);
                    port = p->src_port;
                }
                else
                {
                    ip = GET_DST_IP(p);
                    port = p->dst_port;
                }
            }
            id_state = AppIdGetServiceIDState(ip, proto, port);
            rnaData->id_state = id_state;
        }
    }

    if (service)
    {
        rnaData->serviceData = service;
        if (rnaData->id_state)
            rnaData->id_state->reset_time = 0;
        ret = service->validate(p->payload, p->payload_size, dir, rnaData, p, service->userdata);
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Service validate %u -> %u %u returned %d %08X\n",
                    (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)proto, ret, rnaData->common.flow_flags);
#endif

        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s %s returned %d\n", app_id_debug_session,
                        service->name ? service->name:"UNKNOWN", ret);
        if (ret < 0)
        {
            if (!service->userdata)
                _dpd.errMsg( "Validation error: %d service %s", ret,
                       (service && service->name) ? service->name:"UNKNOWN");
            return ret;
        }
    }
    else if (dir == APP_ID_FROM_RESPONDER) /*we have seen bidirectional exchange and have not identified any service. */
    {
#ifdef SERVICE_DEBUG
#if SERVICE_DEBUG_PORT
        if (p->dst_port == SERVICE_DEBUG_PORT || p->src_port == SERVICE_DEBUG_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Validation failed: %d\n", ret);
#endif
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s no RNA service detector\n", app_id_debug_session);
        AppIdServiceFailService(rnaData, p, dir, NULL);
        return SERVICE_NOMATCH;
    }

    return ret;
}

static void *service_flowdata_get(tAppIdData *flow)
{
    return AppIdFlowdataGet(flow, FLOW_DATA_ID_SVC_FLOWSTATE);
}

static int service_flowdata_add(tAppIdData *flow, void *data, AppIdFreeFCN fcn)
{
    return AppIdFlowdataAdd(flow, data, FLOW_DATA_ID_SVC_FLOWSTATE, fcn);
}

/** GUS: 2006 09 28 10:10:54
 *  A simple function that prints the
 *  ports that have decoders registered.
 */
static void dumpServices(FILE *stream, SF_LIST **parray)
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

void dumpPorts(FILE *stream)
{
    fprintf(stream,"(tcp ");
    dumpServices(stream,tcp_services);
    fprintf(stream,") \n");
    fprintf(stream,"(udp ");
    dumpServices(stream,udp_services);
    fprintf(stream,") \n");
}

static void AppIdServiceAddMisc(FLOW* flow, tAppId miscId)
{
    if(flow != NULL)
        flow->miscAppId = miscId;
}
