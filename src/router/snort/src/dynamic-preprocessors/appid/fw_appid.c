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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <ctype.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <syslog.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <pthread.h>
#include "appIdApi.h"
#include "fw_appid.h"
#include "profiler.h"
#include "client_app_base.h"
#include "httpCommon.h"
#include "luaDetectorApi.h"
#include "http_url_patterns.h"
#include "fw_appid.h"
#include "detector_http.h"
#include "service_ssl.h"
#include "detector_dns.h"
#include "flow.h"
#include "common_util.h"
#include "spp_appid.h"
#include "hostPortAppCache.h"
#include "lengthAppCache.h"
#include "appInfoTable.h"
#include "appIdStats.h"
#include "sf_mlmp.h"
#include "ip_funcs.h"
#include "app_forecast.h"
#include "thirdparty_appid_types.h"
#include "thirdparty_appid_utils.h"
#include "appInfoTable.h"
#include "service_base.h"

//#define DEBUG_APP_ID_SESSIONS   1
//#define DEBUG_FW_APPID  1
//#define DEBUG_FW_APPID_PORT 80

#define MAX_ATTR_LEN           1024
#define HTTP_PREFIX "http://"
#define HTTPS_PREFIX "https://"

#define MULTI_BUF_SIZE 1024
#define MAX_HOSTNAME      255

#define HTTP_PREFIX_LEN   7
#define HTTPS_PREFIX_LEN  8

#define APP_MAPPING_FILE "appMapping.data"

#ifdef RNA_DEBUG_PE
static const char *MODULE_NAME = "fw_appid";
#endif

static volatile int app_id_debug_flag;
static FWDebugSessionConstraints app_id_debug_info;
char app_id_debug_session[FW_DEBUG_SESSION_ID_SIZE];
bool app_id_debug_session_flag;

#ifdef PERF_PROFILING
PreprocStats tpPerfStats;
PreprocStats tpLibPerfStats;
PreprocStats httpPerfStats;
PreprocStats clientMatchPerfStats;
PreprocStats serviceMatchPerfStats;
#endif

#define HTTP_PATTERN_MAX_LEN    1024
#define PORT_MAX 65535

unsigned long app_id_ongoing_session = 0;
unsigned long app_id_total_alloc = 0;
unsigned long app_id_raw_packet_count = 0;
unsigned long app_id_processed_packet_count = 0;
unsigned long app_id_ignored_packet_count = 0;
unsigned long app_id_flow_data_free_list_count = 0;
unsigned long app_id_data_free_list_count = 0;
unsigned long app_id_tmp_free_list_count = 0;
unsigned long app_id_session_heap_alloc_count = 0;
unsigned long app_id_session_freelist_alloc_count = 0;

static tAppIdData *app_id_free_list;
static tTmpAppIdData *tmp_app_id_free_list;
static uint32_t snortInstance;
int app_id_debug;
static int ptype_scan_counts[NUMBER_OF_PTYPES];

static void ProcessThirdPartyResults(SFSnortPacket* p, APPID_SESSION_DIRECTION direction, tAppIdData* appIdSession, int confidence, tAppId* proto_list, ThirdPartyAppIDAttributeData* attribute_data);
static void ExamineRtmpMetadata(SFSnortPacket* p, APPID_SESSION_DIRECTION direction, tAppIdData *appIdSession);

AppIdDebugHostInfo_t AppIdDebugHostInfo;

static inline void appSharedDataFree(tAppIdData * sharedData)
{
    sharedData->next = app_id_free_list;
    app_id_free_list = sharedData;
    app_id_data_free_list_count++;
}

static inline void appTmpSharedDataFree(tTmpAppIdData * sharedData)
{
    sharedData->next = tmp_app_id_free_list;
    tmp_app_id_free_list = sharedData;
    app_id_tmp_free_list_count++;
}

static inline void appHttpFieldClear (httpSession *hsession)
{
    if (hsession == NULL) return;

    if (hsession->referer)
    {
        free(hsession->referer);
        hsession->referer = NULL;
    }
    if (hsession->cookie)
    {
        free(hsession->cookie);
        hsession->cookie = NULL;
    }
    if (hsession->url)
    {
        free(hsession->url);
        hsession->url = NULL;
    }
    if (hsession->useragent)
    {
        free(hsession->useragent);
        hsession->useragent = NULL;
    }
    if (hsession->host)
    {
        free(hsession->host);
        hsession->host = NULL;
    }
    if (hsession->uri)
    {
        free(hsession->uri);
        hsession->uri = NULL;
    }
    if (hsession->content_type)
    {
        free(hsession->content_type);
        hsession->content_type = NULL;
    }
    if (hsession->location)
    {
        free(hsession->location);
        hsession->location = NULL;
    }
    if (hsession->body)
    {
        free(hsession->body);
        hsession->body = NULL;
    }
    if (hsession->req_body)
    {
        free(hsession->req_body);
        hsession->req_body = NULL;
    }
    if (hsession->server)
    {
        free(hsession->server);
        hsession->server = NULL;
    }
    if (hsession->x_working_with)
    {
        free(hsession->x_working_with);
        hsession->x_working_with = NULL;
    }
    if (hsession->xffAddr)
    {
        sfaddr_free(hsession->xffAddr);
        hsession->xffAddr = NULL;
    }
    if (hsession->xffPrecedence)
    {
        int i;

        for (i = 0; i < hsession->numXffFields; i++)
            free(hsession->xffPrecedence[i]);
        _dpd.snortFree(hsession->xffPrecedence, hsession->numXffFields*sizeof(char*),
            PP_APP_ID, PP_MEM_CATEGORY_SESSION);
        hsession->xffPrecedence = NULL;
    }
}

static inline void appHttpSessionDataFree (httpSession *hsession)
{
    int i;

    if (hsession == NULL) return;

    appHttpFieldClear(hsession);

    if (hsession->new_field_contents)
    {
        for (i = 0; i < NUMBER_OF_PTYPES; i++)
        {
            if (NULL != hsession->new_field[i])
            {
                free(hsession->new_field[i]);
                hsession->new_field[i] = NULL;
            }
        }
    }
    if (hsession->fflow)
    {
        _dpd.snortFree(hsession->fflow, sizeof(*hsession->fflow),
             PP_APP_ID, PP_MEM_CATEGORY_SESSION);
        hsession->fflow = NULL;
    }
    if (hsession->via)
    {
        free(hsession->via);
        hsession->via = NULL;
    }
    if (hsession->content_type)
    {
        free(hsession->content_type);
        hsession->content_type = NULL;
    }
    if (hsession->response_code)
    {
        free(hsession->response_code);
        hsession->response_code = NULL;
    }
    if (hsession->tunDest)
    {
        free(hsession->tunDest);
        hsession->tunDest = NULL;
    }

     _dpd.snortFree(hsession, sizeof(*hsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
}

static inline void appDNSSessionDataFree(dnsSession *dsession)
{
    if (dsession == NULL) return;
    if (dsession->host)
    {
        free(dsession->host);
        dsession->host = NULL;
    }
    _dpd.snortFree(dsession, sizeof(*dsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
}

static inline void appTlsSessionDataFree (tlsSession *tsession)
{
    if (tsession == NULL) return;

    if (tsession->tls_host)
        free(tsession->tls_host);
    if (tsession->tls_cname)
        free(tsession->tls_cname);
    if (tsession->tls_orgUnit)
        free(tsession->tls_orgUnit);
    if (tsession->tls_first_san)
        free(tsession->tls_first_san);
    _dpd.snortFree(tsession, sizeof(*tsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
}

#ifdef REG_TEST
void appIdRegTestDumpEndOfSession(tAppIdData *appid_session)
{
    if (appid_session == NULL)
        return;

    _dpd.logMsg("AppID End of Session...\n");

    _dpd.logMsg("    pickServiceAppId(appid_session) = %d\n", pickServiceAppId(appid_session));
    _dpd.logMsg("    pickPayloadId(appid_session) = %d\n", pickPayloadId(appid_session));
    _dpd.logMsg("    pickClientAppId(appid_session) = %d\n", pickClientAppId(appid_session));
    _dpd.logMsg("    pickMiscAppId(appid_session) = %d\n", pickMiscAppId(appid_session));

    if (appid_session->dsession != NULL)
    {
        _dpd.logMsg("    appid_session->dsession->host = %s\n", appid_session->dsession->host ?: "NULL");
        _dpd.logMsg("    appid_session->dsession->options_offset = %u\n", appid_session->dsession->options_offset);
    }

    if (appid_session->tsession != NULL)
    {
        _dpd.logMsg("    appid_session->tsession->tls_host = %s\n", appid_session->tsession->tls_host ?: "NULL");
    }

    _dpd.logMsg("    Flow is %s, ignore flag is %s\n",
        getAppIdFlag(appid_session, APPID_SESSION_OOO)? "out-of-order":"in-order",
        getAppIdFlag(appid_session, APPID_SESSION_IGNORE_FLOW)? "true":"false");
}
#endif

void appSharedDataDelete(tAppIdData * sharedData)
{
    RNAServiceSubtype *subtype;

    if (sharedData)
    {
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
        if (sharedData->service_port == DEBUG_FW_APPID_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "Deleting session %p\n", sharedData);
#endif
        app_id_ongoing_session--;
#ifdef REG_TEST
        if (appidStaticConfig->appid_reg_test_mode)
            appIdRegTestDumpEndOfSession(sharedData);
#endif
        /*check daq flag */
        appIdStatsUpdate(sharedData);

        if (sharedData->ssn)
            FailInProcessService(sharedData, pAppidActiveConfig);
        AppIdFlowdataFree(sharedData);

        if (thirdparty_appid_module)
        {
            thirdparty_appid_module->session_delete(sharedData->tpsession, 0);    // we're completely done with it
            sharedData->tpsession = NULL;
        }
        free(sharedData->clientVersion);
        free(sharedData->serviceVendor);
        free(sharedData->serviceVersion);
        free(sharedData->netbios_name);
        while ((subtype = sharedData->subtype))
        {
            sharedData->subtype = subtype->next;
            free(*(void **)&subtype->service);
            free(*(void **)&subtype->vendor);
            free(*(void **)&subtype->version);
            free(subtype);
        }
        if (sharedData->candidate_service_list != NULL)
        {
            sflist_free(sharedData->candidate_service_list);
            sharedData->candidate_service_list = NULL;
        }
        if (sharedData->candidate_client_list != NULL)
        {
            sflist_free(sharedData->candidate_client_list);
            sharedData->candidate_client_list = NULL;
        }
        free(sharedData->username);
        free(sharedData->netbiosDomain);
        free(sharedData->payloadVersion);
        appHttpSessionDataFree(sharedData->hsession);
        appTlsSessionDataFree(sharedData->tsession);
        appDNSSessionDataFree(sharedData->dsession);
        sharedData->tsession = NULL;
        if (sharedData->multiPayloadList)
            sfghash_delete(sharedData->multiPayloadList);
        free(sharedData->firewallEarlyData);
        sharedData->firewallEarlyData = NULL;

        appSharedDataFree(sharedData);
    }
}
/* The snortId_for_unsynchronized value is to cheaply insure we get
   a unique value from snort's list that guarantees no other preprocessor has it in use. */

static int16_t snortId_for_unsynchronized;
static int16_t snortId_for_ftp_data;
static int16_t snortId_for_http2;

tAppIdData* appSharedDataAlloc(uint8_t proto,  const struct in6_addr *ip, uint16_t port)
{
    static uint32_t gFlowId;
    tAppIdData *data;

    app_id_ongoing_session++;
    if (app_id_free_list)
    {
        data = app_id_free_list;
        app_id_free_list = data->next;
        memset(data, 0, sizeof(*data));
        app_id_data_free_list_count--;
        app_id_session_freelist_alloc_count++;
    }
    else if (!(data = _dpd.snortAlloc(1, sizeof(*data), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
        DynamicPreprocessorFatalMessage("Could not allocate tAppIdData data");
    else
        app_id_session_heap_alloc_count++;

    app_id_total_alloc++;

    data->flowId = ++gFlowId;
    data->common.fsf_type.flow_type = APPID_SESSION_TYPE_NORMAL;
    data->proto = proto;
    data->common.initiator_ip = *ip;
    data->common.initiator_port = port;
    data->snortId = snortId_for_unsynchronized;
    data->search_support_type = SEARCH_SUPPORT_TYPE_UNKNOWN;
    return data;
}

static inline tAppIdData* appSharedCreateData(const SFSnortPacket *p, uint8_t proto, APPID_SESSION_DIRECTION direction)
{
#ifdef DEBUG_FW_APPID
    static unsigned long packet_count;
#endif
    tAppIdData *data;
    sfaddr_t *ip;

    ip = (direction == APP_ID_FROM_INITIATOR) ? GET_SRC_IP(p) : GET_DST_IP(p);
    data = appSharedDataAlloc(proto, (struct in6_addr*)sfaddr_get_ip6_ptr(ip) ,  0 );

    if ((proto == IPPROTO_TCP || proto == IPPROTO_UDP) && p->src_port != p->dst_port)
        data->common.initiator_port = (direction == APP_ID_FROM_INITIATOR) ? p->src_port : p->dst_port;
    data->ssn = p->stream_session;
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
        if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
            fprintf(SF_DEBUG_FILE, "pkt %lu : tAppIdData: Allocated %p\n", ++packet_count, data);
#endif
    data->stats.firstPktsecond = p->pkt_header->ts.tv_sec;

    _dpd.sessionAPI->set_application_data(p->stream_session, PP_APP_ID, data,
            (void (*)(void *))appSharedDataDelete);
    return data;
}

static inline void appSharedReInitData(tAppIdData* session)
{
    session->miscAppId = APP_ID_NONE;

    //payload
    if (isSslServiceAppId(session->tpAppId))
    {
        session->payloadAppId = session->referredPayloadAppId = session->tpPayloadAppId =  APP_ID_NONE;
        clearAppIdFlag(session, APPID_SESSION_CONTINUE);
        if (session->payloadVersion)
        {
            free(session->payloadVersion);
            session->payloadVersion = NULL;
        }
        if (session->hsession && session->hsession->url)
        {
            free(session->hsession->url);
            session->hsession->url = NULL;
        }
    }

    //service
    if (!getAppIdFlag(session, APPID_SESSION_STICKY_SERVICE))
    {

        session->tpAppId = session->serviceAppId = session->portServiceAppId = APP_ID_NONE;
        if (session->serviceVendor)
        {
            free(session->serviceVendor);
            session->serviceVendor = NULL;
        }
        if (session->serviceVersion)
        {
            free(session->serviceVersion);
            session->serviceVersion = NULL;
        }

        IP_CLEAR(session->service_ip);
        session->service_port = 0;
        session->rnaServiceState = RNA_STATE_NONE;
        session->serviceData = NULL;
        AppIdFlowdataDeleteAllByMask(session, APPID_SESSION_DATA_SERVICE_MODSTATE_BIT);
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        session->serviceAsId = 0xFF;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        session->carrierId = 0;
#endif
    }

    //client
    session->clientAppId = session->clientServiceAppId = APP_ID_NONE;
    if (session->clientVersion)
    {
        free(session->clientVersion);
        session->clientVersion = NULL;
    }
    session->rnaClientState = RNA_STATE_NONE;
    session->clientData = NULL;
    if (session->candidate_client_list)
    {
        sflist_free(session->candidate_client_list);
        session->candidate_client_list = NULL;
    }
    session->num_candidate_clients_tried = 0;

    AppIdFlowdataDeleteAllByMask(session, APPID_SESSION_DATA_CLIENT_MODSTATE_BIT);

    //3rd party cleaning
    if (thirdparty_appid_module)
        thirdparty_appid_module->session_delete(session->tpsession, 1);
    session->init_tpPackets = 0;
    session->resp_tpPackets = 0;

    session->scan_flags &= ~SCAN_HTTP_HOST_URL_FLAG;
    clearAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED|APPID_SESSION_CLIENT_DETECTED|APPID_SESSION_SSL_SESSION|APPID_SESSION_HTTP_SESSION|APPID_SESSION_APP_REINSPECT);
}
void fwAppIdFini(tAppIdConfig *pConfig)
{
#ifdef APPID_FULL_CLEANUP
    tAppIdData *app_id;
    tTmpAppIdData *tmp_app_id;

    while ((app_id = app_id_free_list))
    {
        app_id_free_list = app_id->next;
        app_id_data_free_list_count--;
        _dpd.snortFree(app_id, sizeof(*app_id), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
    }

    while ((tmp_app_id = tmp_app_id_free_list))
    {
        tmp_app_id_free_list = tmp_app_id->next;
        app_id_tmp_free_list_count--;
        _dpd.snortFree(tmp_app_id, sizeof(*tmp_app_id), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
    }
    AppIdFlowdataFini();
#endif

    appInfoTableFini(pConfig);
}

static inline int PENetworkMatch(const sfaddr_t *pktAddr, const PortExclusion *pe)
{
    const uint32_t* pkt = sfaddr_get_ip6_ptr(pktAddr);
    const uint32_t* nm = pe->netmask.s6_addr32;
    const uint32_t* peIP = pe->ip.s6_addr32;
    return (((pkt[0] & nm[0]) == peIP[0])
            && ((pkt[1] & nm[1]) == peIP[1])
            && ((pkt[2] & nm[2]) == peIP[2])
            && ((pkt[3] & nm[3]) == peIP[3]));
}

static inline int checkPortExclusion(const SFSnortPacket *pkt, int reversed)
{
    SF_LIST * *src_port_exclusions;
    SF_LIST * *dst_port_exclusions;
    SF_LIST *pe_list;
    PortExclusion *pe;
    sfaddr_t *s_ip;
    uint16_t port;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (IsTCP(pkt))
    {
        src_port_exclusions = pConfig->tcp_port_exclusions_src;
        dst_port_exclusions = pConfig->tcp_port_exclusions_dst;
    }
    else if (IsUDP(pkt))
    {
        src_port_exclusions = pConfig->udp_port_exclusions_src;
        dst_port_exclusions = pConfig->udp_port_exclusions_dst;
    }
    else
        return 0;

    /* check the source port */
    port = reversed ? pkt->dst_port : pkt->src_port;
    if( port && (pe_list=src_port_exclusions[port]) != NULL )
    {
        s_ip = reversed ? GET_DST_IP(pkt) : GET_SRC_IP(pkt);

        /* walk through the list of port exclusions for this port */
        for (pe=(PortExclusion *)sflist_first(pe_list);
                pe;
                pe=(PortExclusion *)sflist_next(pe_list))
        {
            if( PENetworkMatch(s_ip, pe))
            {
#ifdef RNA_DEBUG_PE
                char inetBuffer[INET6_ADDRSTRLEN];
                inetBuffer[0] = 0;
                inet_ntop(sfaddr_family(s_ip), (void *)sfaddr_get_ptr(s_ip), inetBuffer, sizeof(inetBuffer));

                SFDEBUG(MODULE_NAME, "excluding src port: %d",port);
                SFDEBUG(MODULE_NAME, "for addresses src: %s", inetBuffer);
#endif
                return 1;
            }
        }
    }

    /* check the dest port */
    port = reversed ? pkt->src_port : pkt->dst_port;
    if( port && (pe_list=dst_port_exclusions[port]) != NULL )
    {
        s_ip = reversed ? GET_SRC_IP(pkt) : GET_DST_IP(pkt);

        /* walk through the list of port exclusions for this port */
        for (pe=(PortExclusion *)sflist_first(pe_list);
                pe;
                pe=(PortExclusion *)sflist_next(pe_list))
        {
            if( PENetworkMatch(s_ip, pe))
            {
#ifdef RNA_DEBUG_PE
                char inetBuffer[INET6_ADDRSTRLEN];
                inetBuffer[0] = 0;
                inet_ntop(sfaddr_family(s_ip), (void *)sfaddr_get_ptr(s_ip), inetBuffer, sizeof(inetBuffer));
                SFDEBUG(MODULE_NAME, "excluding dst port: %d",port);
                SFDEBUG(MODULE_NAME, "for addresses dst: %s", inetBuffer);
#endif
                return 1;
            }
        }
    }

    return 0;
}

static inline bool fwAppIdDebugCheck(void *lwssn, tAppIdData *session, volatile int debug_flag,
        FWDebugSessionConstraints *info, char *debug_session, APPID_SESSION_DIRECTION direction)
{
    if (debug_flag)
    {
        const StreamSessionKey *key;

        key = _dpd.sessionAPI->get_key_from_session_ptr(lwssn);
        if ((!info->protocol || info->protocol == key->protocol) &&
            (((!info->sport || info->sport == key->port_l) &&
              (!info->sip_flag || memcmp(&info->sip, key->ip_l, sizeof(info->sip)) == 0) &&
              (!info->dport || info->dport == key->port_h) &&
              (!info->dip_flag || memcmp(&info->dip, key->ip_h, sizeof(info->dip)) == 0)) ||
             ((!info->sport || info->sport == key->port_h) &&
               (!info->sip_flag || memcmp(&info->sip, key->ip_h, sizeof(info->sip)) == 0) &&
               (!info->dport || info->dport == key->port_l) &&
               (!info->dip_flag || memcmp(&info->dip, key->ip_l, sizeof(info->dip)) == 0))))
        {
            int af;
            const struct in6_addr* sip;
            const struct in6_addr* dip;
            unsigned offset;
            uint16_t sport;
            uint16_t dport;
            char sipstr[INET6_ADDRSTRLEN];
            char dipstr[INET6_ADDRSTRLEN];
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            uint16_t sAsId; 
            uint16_t dAsId;
#endif
            if (session && session->common.fsf_type.flow_type != APPID_SESSION_TYPE_IGNORE)
            {
                if (session->common.initiator_port)
                {
                    if (session->common.initiator_port == key->port_l)
                    {
                        sip = (const struct in6_addr*)key->ip_l;
                        dip = (const struct in6_addr*)key->ip_h;
                        sport = key->port_l;
                        dport = key->port_h;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                        sAsId = key->addressSpaceId_l;
                        dAsId = key->addressSpaceId_h;
#endif
                    }
                    else
                    {
                        sip = (const struct in6_addr*)key->ip_h;
                        dip = (const struct in6_addr*)key->ip_l;
                        sport = key->port_h;
                        dport = key->port_l;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                        sAsId = key->addressSpaceId_h;
                        dAsId = key->addressSpaceId_l;
#endif
                    }
                }
                else if (memcmp(&session->common.initiator_ip, key->ip_l, sizeof(session->common.initiator_ip))==0)
                {
                    sip = (const struct in6_addr*)key->ip_l;
                    dip = (const struct in6_addr*)key->ip_h;
                    sport = key->port_l;
                    dport = key->port_h;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                    sAsId = key->addressSpaceId_l;
                    dAsId = key->addressSpaceId_h;
#endif
                }
                else
                {
                    sip = (const struct in6_addr*)key->ip_h;
                    dip = (const struct in6_addr*)key->ip_l;
                    sport = key->port_h;
                    dport = key->port_l;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                    sAsId = key->addressSpaceId_h;
                    dAsId = key->addressSpaceId_l;
#endif
                }
            }
            else
            {
                sip = (const struct in6_addr*)key->ip_l;
                dip = (const struct in6_addr*)key->ip_h;
                sport = key->port_l;
                dport = key->port_h;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                sAsId = key->addressSpaceId_l;
                dAsId = key->addressSpaceId_h;
#endif
            }
            sipstr[0] = 0;
            if (sip->s6_addr32[0] || sip->s6_addr32[1] || sip->s6_addr16[4] || (sip->s6_addr16[5] && sip->s6_addr16[5] != 0xFFFF))
            {
                af = AF_INET6;
                offset = 0;
            }
            else
            {
                af = AF_INET;
                offset = 12;
            }
            inet_ntop(af, &sip->s6_addr[offset], sipstr, sizeof(sipstr));
            dipstr[0] = 0;
            if (dip->s6_addr32[0] || dip->s6_addr32[1] || dip->s6_addr16[4] || (dip->s6_addr16[5] && dip->s6_addr16[5] != 0xFFFF))
            {
                af = AF_INET6;
                offset = 0;
            }
            else
            {
                af = AF_INET;
                offset = 12;
            }
            inet_ntop(af, &dip->s6_addr[offset], dipstr, sizeof(dipstr));
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
            uint32_t cid = key->carrierId;
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u -> %s-%u %u%s AS %u-%u I %u CID %u",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R",
                     sAsId, dAsId, (unsigned)snortInstance, (unsigned)cid);
#else
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u -> %s-%u %u%s AS %u I %u CID %u",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R",
                     (unsigned)key->addressSpaceId, (unsigned)snortInstance, (unsigned)cid);
#endif
#else
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u -> %s-%u %u%s I %u CID %u",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R", (unsigned)snortInstance,
                     (unsigned)cid );
#endif
#else /* No carrierid support */
#ifdef HAVE_DAQ_ADDRESS_SPACE_ID
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u -> %s-%u %u%s AS %u-%u I %u",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R",
                     sAsId, dAsId, (unsigned)snortInstance);
#else
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u -> %s-%u %u%s AS %u I %u",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R",
                     (unsigned)key->addressSpaceId, (unsigned)snortInstance);
#endif
#else
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u -> %s-%u %u%s I %u",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R", (unsigned)snortInstance);
#endif
#endif
            return true;
        }
    }
    return false;
}

static inline void appIdDebugParse(const char *desc, const uint8_t *data, uint32_t length,
                          volatile int *debug_flag, FWDebugSessionConstraints *info)
{
    *debug_flag = 0;
    memset(info, 0, sizeof(*info));
    do
    {
        if (length >= sizeof(info->protocol))
        {
            info->protocol = *data;
            length -= sizeof(info->protocol);
            data += sizeof(info->protocol);
        }
        else
            break;

        if (length >= sizeof(info->sip))
        {

            memcpy(&info->sip, data, sizeof(info->sip));
            if (info->sip.s6_addr32[1] || info->sip.s6_addr32[2] || info->sip.s6_addr32[3])
                info->sip_flag = 1;
            else if (info->sip.s6_addr32[0])
            {
                info->sip.s6_addr32[3] = info->sip.s6_addr32[0];
                info->sip.s6_addr32[0] = 0;
                info->sip.s6_addr16[5] = 0xFFFF;
                info->sip_flag = 1;
            }
            length -= sizeof(info->sip);
            data += sizeof(info->sip);
        }
        else
            break;

        if (length >= sizeof(info->sport))
        {
            memcpy(&info->sport, data, sizeof(info->sport));
            length -= sizeof(info->sport);
            data += sizeof(info->sport);
        }
        else
            break;

        if (length >= sizeof(info->dip))
        {
            memcpy(&info->dip, data, sizeof(info->dip));
            if (info->dip.s6_addr32[1] || info->dip.s6_addr32[2] || info->dip.s6_addr32[3])
                info->dip_flag = 1;
            else if (info->dip.s6_addr32[0])
            {
                info->dip.s6_addr32[3] = info->dip.s6_addr32[0];
                info->dip.s6_addr32[0] = 0;
                info->dip.s6_addr16[5] = 0xFFFF;
                info->dip_flag = 1;
            }
            length -= sizeof(info->dip);
            data += sizeof(info->dip);
        }
        else
            break;

        if (length >= sizeof(info->dport))
        {
            memcpy(&info->dport, data, sizeof(info->dport));
            length -= sizeof(info->dport);
            data += sizeof(info->dport);
        }
        else
            break;
    } while (0);

    if (info->protocol || info->sip_flag || info->sport || info->dip_flag || info->dport)
    {
        int saf;
        int daf;
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        if (!info->sip.s6_addr32[0] && !info->sip.s6_addr32[0] && !info->sip.s6_addr16[4] &&
            info->sip.s6_addr16[5] == 0xFFFF)
        {
            saf = AF_INET;
        }
        else
            saf = AF_INET6;
        if (!info->dip.s6_addr32[0] && !info->dip.s6_addr32[0] && !info->dip.s6_addr16[4] &&
            info->dip.s6_addr16[5] == 0xFFFF)
        {
            daf = AF_INET;
        }
        else
            daf = AF_INET6;
        if (!info->sip_flag)
            saf = daf;
        if (!info->dip_flag)
            daf = saf;
        sipstr[0] = 0;
        inet_ntop(saf, saf == AF_INET ? &info->sip.s6_addr32[3] : info->sip.s6_addr32, sipstr, sizeof(sipstr));
        dipstr[0] = 0;
        inet_ntop(daf, daf == AF_INET ? &info->dip.s6_addr32[3] : info->dip.s6_addr32, dipstr, sizeof(dipstr));
        _dpd.logMsg("Debugging %s with %s-%u and %s-%u %u\n", desc,
                    sipstr, (unsigned)info->sport,
                    dipstr, (unsigned)info->dport,
                    (unsigned)info->protocol);
        *debug_flag = 1;
    }
    else
        _dpd.logMsg("Debugging %s disabled\n", desc);
}
int AppIdDebug(uint16_t type, const uint8_t *data, uint32_t length, void **new_context,
               char* statusBuf, int statusBuf_len)
{
    appIdDebugParse("appId", data, length, &app_id_debug_flag, &app_id_debug_info);
    return 0;
}

unsigned isIPv4HostMonitored(uint32_t ip4, int32_t zone)
{
    NetworkSet *net_list;
    unsigned flags;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (zone >= 0 && zone < MAX_ZONES && pConfig->net_list_by_zone[zone])
        net_list = pConfig->net_list_by_zone[zone];
    else
        net_list = pConfig->net_list;

    NetworkSet_ContainsEx(net_list, ip4, &flags);
    return flags;
}

static inline unsigned isIPMonitored(const SFSnortPacket *p, int dst)
{
    uint32_t ipAddr;
    sfaddr_t *sf_ip;
    struct in_addr ip;
    NetworkSet *net_list;
    unsigned flags;
    int32_t zone;
    NSIPv6Addr ip6;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (!dst)
    {
        zone = p->pkt_header->ingress_group;
        sf_ip = GET_SRC_IP(p);
    }
    else
    {
        zone = (p->pkt_header->egress_index == DAQ_PKTHDR_UNKNOWN) ? p->pkt_header->ingress_group : p->pkt_header->egress_group;
        if (zone == DAQ_PKTHDR_FLOOD)
            return 0;
        sf_ip = GET_DST_IP(p);
    }
    if (zone >= 0 && zone < MAX_ZONES && pConfig->net_list_by_zone[zone])
        net_list = pConfig->net_list_by_zone[zone];
    else
        net_list = pConfig->net_list;
    if (sfaddr_family(sf_ip) == AF_INET)
    {
        ip.s_addr = sfaddr_get_ip4_value(sf_ip);
        if (ip.s_addr == 0xFFFFFFFF)
            return IPFUNCS_CHECKED;
        ipAddr = ntohl(ip.s_addr);
        NetworkSet_ContainsEx(net_list, ipAddr, &flags);
    }
    else
    {
        memcpy(&ip6, sfaddr_get_ptr(sf_ip), sizeof(ip6));
        NSIPv6AddrNtoH(&ip6);
        NetworkSet_Contains6Ex(net_list, &ip6, &flags);
    }
    return flags | IPFUNCS_CHECKED;
}

static inline int isSpecialSessionMonitored(const SFSnortPacket *p)
{
    sfaddr_t *srcAddr;

    srcAddr = GET_SRC_IP(p);
    if (sfaddr_family(srcAddr) == AF_INET)
    {
        if (IsUDP(p) && ((p->src_port == 68 && p->dst_port == 67) || (p->src_port == 67 && p->dst_port == 68)))
        {
            return 1;
        }
    }
    return 0;
}

static inline uint64_t isSessionMonitored(const SFSnortPacket *p, APPID_SESSION_DIRECTION dir, tAppIdData *session)
{
    uint64_t flags;
    uint64_t flow_flags = 0;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (pConfig == NULL)
        return flow_flags;

    if (pConfig->isAppIdAlwaysRequired == APPID_REQ_UNINITIALIZED)
        pConfig->isAppIdAlwaysRequired = _dpd.isAppIdRequired() ? APPID_REQ_YES : APPID_REQ_NO;
    if (pConfig->isAppIdAlwaysRequired == APPID_REQ_YES)
        flow_flags |= APPID_SESSION_DISCOVER_APP;

    flow_flags |= (dir == APP_ID_FROM_INITIATOR) ? APPID_SESSION_INITIATOR_SEEN : APPID_SESSION_RESPONDER_SEEN;
    if (session)
    {
        flow_flags |= session->common.flags;
        if (session->common.policyId != appIdPolicyId)
        {
            if (checkPortExclusion(p, dir == APP_ID_FROM_RESPONDER))
            {
                flow_flags |= APPID_SESSION_INITIATOR_SEEN | APPID_SESSION_RESPONDER_SEEN | APPID_SESSION_INITIATOR_CHECKED | APPID_SESSION_RESPONDER_CHECKED;
                flow_flags &= ~(APPID_SESSION_INITIATOR_MONITORED | APPID_SESSION_RESPONDER_MONITORED);
                return flow_flags;
            }
            if (dir == APP_ID_FROM_INITIATOR)
            {
                if (getAppIdFlag(session, APPID_SESSION_INITIATOR_CHECKED))
                {
                    flags = isIPMonitored(p, 0);
                    if (flags & IPFUNCS_HOSTS_IP)
                    {
                        flow_flags |= APPID_SESSION_INITIATOR_MONITORED;
                        AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_MONITOR0;
                    }
                    else
                        flow_flags &= ~APPID_SESSION_INITIATOR_MONITORED;
                }

                if (getAppIdFlag(session, APPID_SESSION_RESPONDER_CHECKED))
                {
                    flags = isIPMonitored(p, 1);
                    if (flags & IPFUNCS_HOSTS_IP)
                        flow_flags |= APPID_SESSION_RESPONDER_MONITORED;
                    else
                        flow_flags &= ~APPID_SESSION_RESPONDER_MONITORED;
                }
            }
            else
            {
                if (getAppIdFlag(session, APPID_SESSION_RESPONDER_CHECKED))
                {
                    flags = isIPMonitored(p, 0);
                    if (flags & IPFUNCS_HOSTS_IP)
                        flow_flags |= APPID_SESSION_RESPONDER_MONITORED;
                    else
                        flow_flags &= ~APPID_SESSION_RESPONDER_MONITORED;
                }

                if (getAppIdFlag(session, APPID_SESSION_INITIATOR_CHECKED))
                {
                    flags = isIPMonitored(p, 1);
                    if (flags & IPFUNCS_HOSTS_IP)
                    {
                        flow_flags |= APPID_SESSION_INITIATOR_MONITORED;
                        AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_MONITOR1;
                    }
                    else
                        flow_flags &= ~APPID_SESSION_INITIATOR_MONITORED;
                }
            }
        }

        if (getAppIdFlag(session, APPID_SESSION_BIDIRECTIONAL_CHECKED) == APPID_SESSION_BIDIRECTIONAL_CHECKED)
            return flow_flags;

        if (dir == APP_ID_FROM_INITIATOR)
        {
            if (!getAppIdFlag(session, APPID_SESSION_INITIATOR_CHECKED))
            {
                flags = isIPMonitored(p, 0);
                flow_flags |= APPID_SESSION_INITIATOR_CHECKED;
                if (flags & IPFUNCS_HOSTS_IP)
                {
                    flow_flags |= APPID_SESSION_INITIATOR_MONITORED;
                    AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_MONITOR2;
                }
                if (flags & IPFUNCS_USER_IP)
                    flow_flags |= APPID_SESSION_DISCOVER_USER;
                if (flags & IPFUNCS_APPLICATION)
                    flow_flags |= APPID_SESSION_DISCOVER_APP;

                if (isSpecialSessionMonitored(p))
                {
                    flow_flags |= APPID_SESSION_SPECIAL_MONITORED;
                }
            }
            if (!(flow_flags & APPID_SESSION_DISCOVER_APP) && !getAppIdFlag(session, APPID_SESSION_RESPONDER_CHECKED))
            {
                flags = isIPMonitored(p, 1);
                if (flags & IPFUNCS_CHECKED)
                    flow_flags |= APPID_SESSION_RESPONDER_CHECKED;
                if (flags & IPFUNCS_HOSTS_IP)
                    flow_flags |= APPID_SESSION_RESPONDER_MONITORED;
                if (flags & IPFUNCS_APPLICATION)
                    flow_flags |= APPID_SESSION_DISCOVER_APP;
                if (isSpecialSessionMonitored(p))
                {
                    flow_flags |= APPID_SESSION_SPECIAL_MONITORED;
                }
            }
        }
        else
        {
            if (!getAppIdFlag(session, APPID_SESSION_RESPONDER_CHECKED))
            {
                flags = isIPMonitored(p, 0);
                flow_flags |= APPID_SESSION_RESPONDER_CHECKED;
                if (flags & IPFUNCS_HOSTS_IP)
                    flow_flags |= APPID_SESSION_RESPONDER_MONITORED;
                if (flags & IPFUNCS_APPLICATION)
                    flow_flags |= APPID_SESSION_DISCOVER_APP;
                if (isSpecialSessionMonitored(p))
                {
                    flow_flags |= APPID_SESSION_SPECIAL_MONITORED;
                }
            }
            if (!(flow_flags & APPID_SESSION_DISCOVER_APP) && !getAppIdFlag(session, APPID_SESSION_INITIATOR_CHECKED))
            {
                flags = isIPMonitored(p, 1);
                if (flags & IPFUNCS_CHECKED)
                    flow_flags |= APPID_SESSION_INITIATOR_CHECKED;
                if (flags & IPFUNCS_HOSTS_IP)
                {
                    flow_flags |= APPID_SESSION_INITIATOR_MONITORED;
                    AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_MONITOR3;
                }
                if (flags & IPFUNCS_USER_IP)
                    flow_flags |= APPID_SESSION_DISCOVER_USER;
                if (flags & IPFUNCS_APPLICATION)
                    flow_flags |= APPID_SESSION_DISCOVER_APP;
                if (isSpecialSessionMonitored(p))
                {
                    flow_flags |= APPID_SESSION_SPECIAL_MONITORED;
                }
            }
        }
    }
    else if (checkPortExclusion(p, 0))
    {
        flow_flags |= APPID_SESSION_INITIATOR_SEEN | APPID_SESSION_RESPONDER_SEEN | APPID_SESSION_INITIATOR_CHECKED | APPID_SESSION_RESPONDER_CHECKED;
    }
    else if (dir == APP_ID_FROM_INITIATOR)
    {
        flags = isIPMonitored(p, 0);
        flow_flags |= APPID_SESSION_INITIATOR_CHECKED;
        if (flags & IPFUNCS_HOSTS_IP)
        {
            flow_flags |= APPID_SESSION_INITIATOR_MONITORED;
            AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_MONITOR4;
        }
        if (flags & IPFUNCS_USER_IP)
            flow_flags |= APPID_SESSION_DISCOVER_USER;
        if (flags & IPFUNCS_APPLICATION)
            flow_flags |= APPID_SESSION_DISCOVER_APP;
        if (!(flow_flags & APPID_SESSION_DISCOVER_APP))
        {
            flags = isIPMonitored(p, 1);
            if (flags & IPFUNCS_CHECKED)
                flow_flags |= APPID_SESSION_RESPONDER_CHECKED;
            if (flags & IPFUNCS_HOSTS_IP)
                flow_flags |= APPID_SESSION_RESPONDER_MONITORED;
            if (flags & IPFUNCS_APPLICATION)
                flow_flags |= APPID_SESSION_DISCOVER_APP;
        }
        if (isSpecialSessionMonitored(p))
        {
            flow_flags |= APPID_SESSION_SPECIAL_MONITORED;
        }
    }
    else
    {
        flags = isIPMonitored(p, 0);
        flow_flags |= APPID_SESSION_RESPONDER_CHECKED;
        if (flags & IPFUNCS_HOSTS_IP)
            flow_flags |= APPID_SESSION_RESPONDER_MONITORED;
        if (flags & IPFUNCS_APPLICATION)
            flow_flags |= APPID_SESSION_DISCOVER_APP;
        if (!(flow_flags & APPID_SESSION_DISCOVER_APP))
        {
            flags = isIPMonitored(p, 1);
            if (flags & IPFUNCS_CHECKED)
                flow_flags |= APPID_SESSION_INITIATOR_CHECKED;
            if (flags & IPFUNCS_HOSTS_IP)
            {
                flow_flags |= APPID_SESSION_INITIATOR_MONITORED;
                AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_MONITOR5;
            }
            if (flags & IPFUNCS_USER_IP)
                flow_flags |= APPID_SESSION_DISCOVER_USER;
            if (flags & IPFUNCS_APPLICATION)
                flow_flags |= APPID_SESSION_DISCOVER_APP;
        }

        if (isSpecialSessionMonitored(p))
        {
            flow_flags |= APPID_SESSION_SPECIAL_MONITORED;
        }
    }

    return flow_flags;
}

void CheckDetectorCallback(const SFSnortPacket *p, tAppIdData *session, APPID_SESSION_DIRECTION direction, tAppId appId, const tAppIdConfig *pConfig)
{
    AppInfoTableEntry *entry;
    int ret;

    if(!p || !session)
        return;

    if ((entry = appInfoEntryGet(appId, pConfig)))
    {
        if (entry->flags & APPINFO_FLAG_CLIENT_DETECTOR_CALLBACK)
        {
            if (entry->clntValidator)
            {
                if (entry->clntValidator->detectorContext)
                    return;

                entry->clntValidator->detectorContext = true;
                ret = entry->clntValidator->detectorCallback(p->payload, p->payload_size, direction,
                                                       session, p, entry->clntValidator->userData, pConfig);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s %s client detector callback returned %d\n", app_id_debug_session,
                                entry->clntValidator->name ? entry->clntValidator->name : "UNKNOWN", ret);
		entry->clntValidator->detectorContext = false;
            }
        }
        if (entry->flags & APPINFO_FLAG_SERVICE_DETECTOR_CALLBACK)
        {
            if (entry->svrValidator)
            {
                if (entry->svrValidator->detectorContext)
		    return;

		entry->svrValidator->detectorContext = true;
                ret = entry->svrValidator->detectorCallback(p->payload, p->payload_size, direction,
                                                      session, p, entry->svrValidator->userdata, pConfig);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s %s service detector callback returned %d\n", app_id_debug_session,
                                entry->svrValidator->name ? entry->svrValidator->name : "UNKNOWN", ret);
		entry->svrValidator->detectorContext = false;
            }
        }
    }
}
static inline bool svcTakingTooMuchTime(tAppIdData* session)
{
    return ((session->initiatorPcketCountWithoutReply > appidStaticConfig-> max_packet_service_fail_ignore_bytes) ||
            (session->initiatorPcketCountWithoutReply > appidStaticConfig->max_packet_before_service_fail &&
             session->initiatorBytesWithoutServerReply > appidStaticConfig->max_bytes_before_service_fail));
}

static inline void setServiceAppIdData(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *session, tAppId serviceAppId, char *vendor, char **version)
{
    if (serviceAppId <= APP_ID_NONE)
        return;

    //in drambuie, 3rd party is in INIT state after processing first GET requuest.
    if (serviceAppId == APP_ID_HTTP)
    {
        if (session->clientServiceAppId == APP_ID_NONE)
        {
            session->clientServiceAppId = serviceAppId;
        }
        return;
    }

    if (session->serviceAppId != serviceAppId)
    {
        session->serviceAppId = serviceAppId;

        CheckDetectorCallback(p, session, direction, serviceAppId, appIdActiveConfigGet());

        if (appidStaticConfig->instance_id)
            checkSandboxDetection(serviceAppId);

        /* Clear out previous values of vendor & version */
        if (session->serviceVendor)
        {
            free(session->serviceVendor);
            session->serviceVendor = NULL;
        }
        if (session->serviceVersion)
        {
            free(session->serviceVersion);
            session->serviceVersion = NULL;
        }

        if (vendor)
            session->serviceVendor = vendor;

        if (version && *version)
        {
            session->serviceVersion = *version;
            *version = NULL;
        }
    }
    else
    {
        if (vendor || version)
        {
            /* Clear previous values */
            if (session->serviceVendor)
                free(session->serviceVendor);
            if (session->serviceVersion)
                free(session->serviceVersion);

            /* set vendor */
            if (vendor)
                session->serviceVendor = vendor;
            else
                session->serviceVendor = NULL;

            /* set version */
            if (version && *version)
            {
                session->serviceVersion = *version;
                *version = NULL;
            }
            else
                session->serviceVersion = NULL;
        }
    }
}

static inline void setClientAppIdData(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *session, tAppId clientAppId, char **version)
{
    tAppIdConfig *pConfig = appIdActiveConfigGet();
    if (clientAppId <= APP_ID_NONE || clientAppId == APP_ID_HTTP)
    {
        if (version && *version)
        { 
            free(*version);
            *version = NULL;
        }
        return;
    }

    if (session->clientAppId != clientAppId)
    {
        unsigned prev_priority = appInfoEntryPriorityGet(session->clientAppId, pConfig);
        unsigned curr_priority = appInfoEntryPriorityGet(clientAppId, pConfig) ;

        if (appidStaticConfig->instance_id)
            checkSandboxDetection(clientAppId);

        if ((session->clientAppId) && (prev_priority > curr_priority ))
        {
            if (version && *version)
            {
                free(*version);
                *version = NULL;
            }
            return;
        }
        session->clientAppId = clientAppId;

	CheckDetectorCallback(p, session, direction, clientAppId, pConfig);

        if (session->clientVersion)
            free(session->clientVersion);

        if (version && *version)
        {
            session->clientVersion = *version;
            *version = NULL;
        }
        else
            session->clientVersion = NULL;
    }
    else if (version && *version)
    {
        if (session->clientVersion)
            free(session->clientVersion);
        session->clientVersion = *version;
        *version = NULL;
    }
}

static inline void setReferredPayloadAppIdData(tAppIdData *session, tAppId referredPayloadAppId)
{
    if (referredPayloadAppId <= APP_ID_NONE)
        return;

    if (session->referredPayloadAppId != referredPayloadAppId)
    {
        if (appidStaticConfig->instance_id)
            checkSandboxDetection(referredPayloadAppId);

        session->referredPayloadAppId = referredPayloadAppId;
    }
}

static inline void setPayloadAppIdData(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *session, tAppId payloadAppId, char **version)
{
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (payloadAppId <= APP_ID_NONE)
        return;

    if (session->payloadAppId != payloadAppId)
    {
        unsigned prev_priority = appInfoEntryPriorityGet(session->payloadAppId, pConfig);
        unsigned curr_priority = appInfoEntryPriorityGet(payloadAppId, pConfig);

        if (appidStaticConfig->instance_id)
            checkSandboxDetection(payloadAppId);

        if ((session->payloadAppId ) && (prev_priority > curr_priority ))
            return;

        session->payloadAppId = payloadAppId;

        CheckDetectorCallback(p, session, direction, payloadAppId, pConfig);

        if (session->payloadVersion)
            free(session->payloadVersion);

        if (version && *version)
        {
            session->payloadVersion = *version;
            *version = NULL;
        }
        else
            session->payloadVersion = NULL;
    }
    else if (version && *version)
    {
        if (session->payloadVersion)
            free(session->payloadVersion);
        session->payloadVersion = *version;
        *version = NULL;
    }
}

static inline void setTPAppIdData(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *session, tAppId tpAppId)
{
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (tpAppId <= APP_ID_NONE || !session)
        return;

    if (session->tpAppId != tpAppId)
    {
        session->tpAppId = tpAppId;
        CheckDetectorCallback(p, session, direction, tpAppId, pConfig);
    }
}

static inline void setTPPayloadAppIdData(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *session, tAppId tpPayloadAppId)
{
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (tpPayloadAppId <= APP_ID_NONE || !session)
        return;

    if (session->tpPayloadAppId != tpPayloadAppId)
    {
        session->tpPayloadAppId = tpPayloadAppId;
        CheckDetectorCallback(p, session, direction, tpPayloadAppId, pConfig);
    }
}

static inline void clearSessionAppIdData(tAppIdData *session)
{
    session->payloadAppId = APP_ID_UNKNOWN;
    session->serviceAppId = APP_ID_UNKNOWN;
    session->tpPayloadAppId = APP_ID_UNKNOWN;
    session->tpAppId = APP_ID_UNKNOWN;
    if (session->payloadVersion)
    {
        free(session->payloadVersion);
        session->payloadVersion = NULL;
    }
    if (session->serviceVendor)
    {
        free(session->serviceVendor);
        session->serviceVendor = NULL;
    }
    if (session->serviceVersion)
    {
        free(session->serviceVersion);
        session->serviceVersion = NULL;
    }
    if (session->clientVersion)
    {
        free(session->clientVersion);
        session->clientVersion = NULL;
    }
    if (session->tsession)
    {
        appTlsSessionDataFree(session->tsession);
        session->tsession = NULL;
    }
    if (session->hsession)
    {
        appHttpSessionDataFree(session->hsession);
        session->hsession = NULL;
    }
    if (session->dsession)
    {
        appDNSSessionDataFree(session->dsession);
        session->dsession = NULL;
    }
    if (thirdparty_appid_module)
        thirdparty_appid_module->session_delete(session->tpsession, 1);
}

static inline int initial_CHP_sweep (char ** chp_buffers, uint16_t * chp_buffer_lengths, MatchedCHPAction **ppmatches, tAppIdData *session, const tDetectorHttpConfig *pHttpConfig)
{
    CHPApp* cah = NULL;
    int longest = 0;
    int i;
    httpSession *hsession;
    int scanKeyFoundSomething=0;
    CHPMatchTally *pTally = NULL; // scanKeyCHP allocates a pointer, but we free it when ready

    hsession = session->hsession;

    for (i = 0; i <= MAX_KEY_PATTERN; i++)
    {
        ppmatches[i] = NULL;
        if (chp_buffers[i] && chp_buffer_lengths[i] &&
            scanKeyCHP((PatternType)i, chp_buffers[i], chp_buffer_lengths[i], &pTally, &ppmatches[i], pHttpConfig))
           scanKeyFoundSomething=1;
    }
    if (!scanKeyFoundSomething)
    {
        if (pTally) free(pTally);
        for (i = 0; i <= MAX_KEY_PATTERN; i++)
        {
            if (ppmatches[i])
            {
                FreeMatchedCHPActions(ppmatches[i]);
                ppmatches[i] = NULL;
            }
        }
        return 0;
    }

    for (i = 0; i < pTally->in_use_elements; i++)
    {
        // Only those items which have had their key_pattern_countdown field reduced to zero are a full match
        if (pTally->item[i].key_pattern_countdown)
            continue;
        if (longest < pTally->item[i].key_pattern_length_sum)
        {
            // We've found a new longest pattern set
            longest = pTally->item[i].key_pattern_length_sum;
            cah = pTally->item[i].chpapp;
        }
    }
    // either we have a candidate or we don't so we can free the tally structure either way.
    free(pTally);

    if (cah == NULL)
    {
        // We were planning to pass along the content of ppmatches to the second phase and let
        // them be freed inside scanCHP, but we have no candidate so we free here
        for (i = 0; i <= MAX_KEY_PATTERN; i++)
        {
            if (ppmatches[i])
            {
                FreeMatchedCHPActions(ppmatches[i]);
                ppmatches[i] = NULL;
            }
        }

        return 0;
    }

    /****************************************************************/
    /* candidate has been chosen and it is pointed to by cah        */
    /* we will preserve any match sets until the calls to scanCHP() */
    /****************************************************************/
    for (i = 0; i < NUMBER_OF_PTYPES; i++)
    {
        ptype_scan_counts[i] = cah->ptype_scan_counts[i];
        hsession->ptype_req_counts[i] = cah->ptype_req_counts[i];
        if (i > 3 && !cah->ptype_scan_counts[i] && !getAppIdFlag(session, APPID_SESSION_SPDY_SESSION))
        {
            clearAppIdFlag(session, APPID_SESSION_CHP_INSPECTING);
            if (thirdparty_appid_module)
                thirdparty_appid_module->session_attr_clear(session->tpsession, TP_ATTR_CONTINUE_MONITORING);
        }
    }
    hsession->chp_candidate = cah->appIdInstance;
    hsession->app_type_flags = cah->app_type_flags;
    hsession->num_matches = cah->num_matches;
    hsession->num_scans = cah->num_scans;

    if (thirdparty_appid_module)
    {
        if ((ptype_scan_counts[CONTENT_TYPE_PT]))
            thirdparty_appid_module->session_attr_set(session->tpsession, TP_ATTR_COPY_RESPONSE_CONTENT);
        else
            thirdparty_appid_module->session_attr_clear(session->tpsession, TP_ATTR_COPY_RESPONSE_CONTENT);

        if ((ptype_scan_counts[LOCATION_PT]))
            thirdparty_appid_module->session_attr_set(session->tpsession, TP_ATTR_COPY_RESPONSE_LOCATION);
        else
            thirdparty_appid_module->session_attr_clear(session->tpsession, TP_ATTR_COPY_RESPONSE_LOCATION);

        if ((ptype_scan_counts[BODY_PT]))
            thirdparty_appid_module->session_attr_set(session->tpsession, TP_ATTR_COPY_RESPONSE_BODY);
        else
            thirdparty_appid_module->session_attr_clear(session->tpsession, TP_ATTR_COPY_RESPONSE_BODY);
    }

    return 1;
}

static char *httpFieldName[ NUMBER_OF_PTYPES ] = // for use in debug messages
{
    "useragent",
    "host",
    "referer",
    "uri",
    "cookie",
    "req_body",
    "content_type",
    "location",
    "body",
};

static inline void processCHP(tAppIdData *session, char **version, SFSnortPacket *p, APPID_SESSION_DIRECTION direction, const tAppIdConfig *pConfig)
{
    int i;
    int found_in_buffer = 0;
    char *user = NULL;
    tAppId chp_final;
    tAppId ret = 0;
    httpSession *http_session = session->hsession;

    char *chp_buffers[NUMBER_OF_PTYPES] = {
        http_session->useragent,
        http_session->host,
        http_session->referer,
        http_session->uri,
        http_session->cookie,
        http_session->req_body,
        http_session->content_type,
        http_session->location,
        http_session->body,
    };

    uint16_t chp_buffer_lengths[NUMBER_OF_PTYPES] = {
        http_session->useragent_buflen,
        http_session->host_buflen,
        http_session->referer_buflen,
        http_session->uri_buflen,
        http_session->cookie_buflen,
        http_session->req_body_buflen,
        http_session->content_type_buflen,
        http_session->location_buflen,
        http_session->body_buflen,
    };

    char *chp_rewritten[NUMBER_OF_PTYPES] = {
        NULL,NULL,NULL,
        NULL,NULL,NULL,
        NULL,NULL,NULL
    };

    MatchedCHPAction *chp_matches[NUMBER_OF_PTYPES] = {
        NULL,NULL,NULL,
        NULL,NULL,NULL,
        NULL,NULL,NULL
    };

    if (http_session->chp_hold_flow)
        http_session->chp_finished = 0;

    if (!http_session->chp_candidate)
    {
        // remove artifacts from previous matches before we start again.
        if (http_session->new_field_contents)
        {
            for (i = 0; i < NUMBER_OF_PTYPES; i++)
            {
                if (http_session->new_field[i])
                {
                    free(http_session->new_field[i]);
                    http_session->new_field[i] = NULL;
                }
            }
        }

        if (!initial_CHP_sweep(chp_buffers, chp_buffer_lengths, chp_matches, session, &pConfig->detectorHttpConfig))
            http_session->chp_finished = 1; // this is a failure case.
    }
    if (!http_session->chp_finished && http_session->chp_candidate)
    {
        for (i = 0; i < NUMBER_OF_PTYPES; i++)
        {
            if (!ptype_scan_counts[i])
                continue;

            // Do scans and check results
            if (chp_buffers[i] && chp_buffer_lengths[i])
            {
                found_in_buffer = 0;
                ret = scanCHP((PatternType)i, chp_buffers[i], chp_buffer_lengths[i], chp_matches[i], version,
                        &user, &chp_rewritten[i], &found_in_buffer,
                        http_session, p, &pConfig->detectorHttpConfig);
                chp_matches[i] = NULL; // freed by scanCHP()
                http_session->total_found += found_in_buffer;
                if (!ret || found_in_buffer < http_session->ptype_req_counts[i])
                {
                    // No match at all or the required matches for the field was NOT made
                    if (!http_session->num_matches)
                    {
                        // num_matches == 0 means: all must succeed
                        // give up early
                        http_session->chp_candidate = 0;
                        break;
                    }
                }
            }
            else
            {
                // No buffer or empty
                if (!http_session->num_matches)
                {
                    // We had a pattern(s) and no buffer to look in.
                    // num_matches == 0 means: all must succeed
                    // give up early
                    http_session->chp_candidate = 0;
                    break;
                }
            }

            // Decrement the expected scan count toward 0.
            ptype_scan_counts[i] = 0;
            http_session->num_scans--;
            // if we have reached the end of the list of scans (which have something to do), then num_scans == 0
            if (http_session->num_scans == 0)
            {
                // we finished the last scan
                // either the num_matches value was zero and we failed early-on or we need to check for the min.
                if (http_session->num_matches &&
                    http_session->total_found < http_session->num_matches)
                {
                    // There was a minimum scans match count (num_matches != 0)
                    // And we did not reach that minimum
                    http_session->chp_candidate = 0;
                    break;
                }
                // All required matches were met.
                http_session->chp_finished = 1;
                break;
            }
        }
        for (i = 0; i < NUMBER_OF_PTYPES; i++)
        {
            if (chp_matches[i]) // free leftover matches
            {
                FreeMatchedCHPActions(chp_matches[i]);
                chp_matches[i] = NULL;
            }
        }
        if (!http_session->chp_candidate)
        {
            http_session->chp_finished = 1;
            if (*version)
            {
                free(*version);
                *version = NULL;
            }
            if (user)
            {
                free(user);
                user = NULL;
            }
            for (i = 0; i < NUMBER_OF_PTYPES; i++)
            {
                if (NULL != chp_rewritten[i])
                {
                    free(chp_rewritten[i]);
                    chp_rewritten[i] = NULL;
                }
            }
            memset(ptype_scan_counts, 0, 7 * sizeof(ptype_scan_counts[0]));

            // Make it possible for other detectors to run.
            http_session->skip_simple_detect = false;
            return;
        }
        if (http_session->chp_candidate && http_session->chp_finished)
        {
            chp_final = http_session->chp_alt_candidate ?
                http_session->chp_alt_candidate :
                CHP_APPIDINSTANCE_TO_ID(http_session->chp_candidate);
            if (http_session->app_type_flags & APP_TYPE_SERVICE)
            {
                setServiceAppIdData(p, direction, session, chp_final, NULL, version);
            }
            if (http_session->app_type_flags & APP_TYPE_CLIENT)
            {
                setClientAppIdData(p, direction, session, chp_final, version);
            }
            if (http_session->app_type_flags & APP_TYPE_PAYLOAD)
            {
                setPayloadAppIdData(p, direction, session, chp_final, version);
            }
            if (http_session->fflow && http_session->fflow->flow_prepared)
            {
                finalizeFflow(http_session->fflow, http_session->app_type_flags,
                              (http_session->fflow->appId ? http_session->fflow->appId : chp_final), p);
                _dpd.snortFree(http_session->fflow, sizeof(*http_session->fflow),
                    PP_APP_ID, PP_MEM_CATEGORY_SESSION);
                http_session->fflow = NULL;
            }
            if (*version)
                *version = NULL;
            if (user)
            {
                session->username = user;
                user = NULL;
                if (http_session->app_type_flags & APP_TYPE_SERVICE)
                    session->usernameService = chp_final;
                else
                    session->usernameService = session->serviceAppId;
                setAppIdFlag(session, APPID_SESSION_LOGIN_SUCCEEDED);
            }
            for (i = 0; i < NUMBER_OF_PTYPES; i++)
            {
                if (NULL != chp_rewritten[i])
                {
                    if (app_id_debug_session_flag)
                        _dpd.logMsg("AppIdDbg %s rewritten %s: %s\n", app_id_debug_session, httpFieldName[i], chp_rewritten[i]);
                    if (http_session->new_field[i])
                        free(http_session->new_field[i]);
                    http_session->new_field[i] = chp_rewritten[i];
                    http_session->new_field_contents = true;
                    chp_rewritten[i] = NULL;
                }
            }
            http_session->chp_candidate = 0;
            //if we're doing safesearch rewrites, we want to continue to hold the flow
            if (!http_session->get_offsets_from_rebuilt)
                http_session->chp_hold_flow = 0;
            session->scan_flags &= ~SCAN_HTTP_VIA_FLAG;
            session->scan_flags &= ~SCAN_HTTP_USER_AGENT_FLAG;
            session->scan_flags &= ~SCAN_HTTP_HOST_URL_FLAG;
            memset(ptype_scan_counts, 0, 7 * sizeof(ptype_scan_counts[0]));
        }
        else /* if we have a candidate, but we're not finished */
        {
            if (user)
            {
                free(user);
                user = NULL;
            }
            for (i = 0; i < NUMBER_OF_PTYPES; i++)
            {
                if (NULL != chp_rewritten[i])
                {
                    free(chp_rewritten[i]);
                    chp_rewritten[i] = NULL;
                }
            }
        }
    }
}

static inline bool payloadAppIdIsSet(tAppIdData *session)
{
    return ( session->payloadAppId || session->tpPayloadAppId );
}

static inline void clearMiscHttpFlags(tAppIdData *session)
{
    if (!getAppIdFlag(session, APPID_SESSION_SPDY_SESSION))
    {
        clearAppIdFlag(session, APPID_SESSION_CHP_INSPECTING);
        if (thirdparty_appid_module)
            thirdparty_appid_module->session_attr_clear(session->tpsession, TP_ATTR_CONTINUE_MONITORING);
    }
}

static int getHttpHostFromUri(char** host, const char* uri)
{
    char buff[MAX_HOSTNAME + 1];
    int len = 0;
    int offset = 0;

    if (!uri)
        return 0;

    memset(buff, 0, sizeof(buff));
    if (!strncmp(uri, HTTP_PREFIX, HTTP_PREFIX_LEN))
    {
        offset = HTTP_PREFIX_LEN;
    }
    else if (!strncmp(uri, HTTPS_PREFIX, HTTPS_PREFIX_LEN))
    {
        offset = HTTPS_PREFIX_LEN;
    }
    
    while(len < MAX_HOSTNAME && uri[offset] != '/' && uri[offset] != '\0')
        buff[len++] = uri[offset++];

    if (len) 
        *host = strdup(buff); 

    return len;
}

STATIC INLINE int processHTTPPacket(SFSnortPacket *p, tAppIdData *session, APPID_SESSION_DIRECTION direction, HttpParsedHeaders *const headers, const tAppIdConfig *pConfig)
{
#define RESPONSE_CODE_LENGTH 3
    HeaderMatchedPatterns hmp;
    httpSession *http_session;
    int start, end, size;
    char *version = NULL;
    char *vendorVersion = NULL;
    char *vendor = NULL;
    tAppId serviceAppId = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    tAppId referredPayloadAppId = 0;
    char *host;
    char *url;
    char *useragent;
    char *referer;
    char *via;
    AppInfoTableEntry *entry;
    PROFILE_VARS;
    PREPROC_PROFILE_START(httpPerfStats);

    http_session = session->hsession;
    if (!http_session)
    {
        clearSessionAppIdData(session);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s attempt to process HTTP packet with no HTTP data\n", app_id_debug_session);
        PREPROC_PROFILE_END(httpPerfStats);
        return 0;
    }

    // For fragmented HTTP headers, do not process if none of the fields are set.
    // These fields will get set when the HTTP header is reassembled.
    if ((!http_session->useragent) && (!http_session->host) && (!http_session->referer) && (!http_session->uri))
    {
        if (!http_session->skip_simple_detect)
            clearMiscHttpFlags(session);
        PREPROC_PROFILE_END(httpPerfStats);
        return 0;
    }

    if (direction == APP_ID_FROM_RESPONDER && !getAppIdFlag(session, APPID_SESSION_RESPONSE_CODE_CHECKED))
    {
        if (http_session->response_code)
        {
            setAppIdFlag(session, APPID_SESSION_RESPONSE_CODE_CHECKED);
            if (http_session->response_code_buflen != RESPONSE_CODE_LENGTH)
            {
                /* received bad response code. Stop processing this session */
                clearSessionAppIdData(session);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s bad http response code\n", app_id_debug_session);
                PREPROC_PROFILE_END(httpPerfStats);
                return 0;
            }
        }
#if RESPONSE_CODE_PACKET_THRESHHOLD
        else if (++(http_session->response_code_packets) == RESPONSE_CODE_PACKET_THRESHHOLD)
        {
            setAppIdFlag(session, APPID_SESSION_RESPONSE_CODE_CHECKED);
            /* didn't receive response code in first X packets. Stop processing this session */
            clearSessionAppIdData(session);
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s no response code received\n", app_id_debug_session);
            PREPROC_PROFILE_END(httpPerfStats);
            return 0;
        }
#endif
    }
    host = http_session->host;
    url = http_session->url;
    via = http_session->via;
    useragent = http_session->useragent;
    referer = http_session->referer;
    memset(&hmp, 0, sizeof(hmp));

    if (session->serviceAppId == APP_ID_NONE)
    {
        session->serviceAppId = APP_ID_HTTP;
        if (appidStaticConfig->instance_id)
            checkSandboxDetection(APP_ID_HTTP);
    }

    if (app_id_debug_session_flag)
        _dpd.logMsg("AppIdDbg %s chp_finished %d chp_hold_flow %d\n", app_id_debug_session, http_session->chp_finished, http_session->chp_hold_flow);

    if (!http_session->chp_finished || http_session->chp_hold_flow)
        processCHP(session, &version, p, direction, pConfig);

    if (!http_session->skip_simple_detect)  // false unless a match happened with a call to processCHP().
    {
        if (!getAppIdFlag(session, APPID_SESSION_APP_REINSPECT))
        {
            // Scan Server Header for Vendor & Version
            if ((thirdparty_appid_module && (session->scan_flags & SCAN_HTTP_VENDOR_FLAG) && session->hsession->server) ||
                (!thirdparty_appid_module && getHTTPHeaderLocation(p->payload, p->payload_size, HTTP_ID_SERVER, &start, &end, &hmp, &pConfig->detectorHttpConfig) == 1))
            {
                if (session->serviceAppId == APP_ID_NONE || session->serviceAppId == APP_ID_HTTP)
                {
                    RNAServiceSubtype *subtype = NULL;
                    RNAServiceSubtype **tmpSubtype;

                    if (thirdparty_appid_module)
                        getServerVendorVersion((uint8_t*)session->hsession->server, strlen(session->hsession->server), &vendorVersion, &vendor, &subtype);
                    else getServerVendorVersion(p->payload + start, end - start, &vendorVersion, &vendor, &subtype);
                    if (vendor || vendorVersion)
                    {
                        if (session->serviceVendor)
                        {
                            free(session->serviceVendor);
                            session->serviceVendor = NULL;
                        }
                        if (session->serviceVersion)
                        {
                            free(session->serviceVersion);
                            session->serviceVersion = NULL;
                        }
                        if (vendor)
                            session->serviceVendor = vendor;
                        if (vendorVersion)
                            session->serviceVersion = vendorVersion;
                        session->scan_flags &= ~SCAN_HTTP_VENDOR_FLAG;
                    }
                    if (subtype)
                    {
                        for (tmpSubtype = &session->subtype; *tmpSubtype; tmpSubtype = &(*tmpSubtype)->next);

                        *tmpSubtype = subtype;
                    }
                }
            }

            if (webdav_found(&hmp))
            {
                if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
                    _dpd.logMsg("AppIdDbg %s payload is webdav\n", app_id_debug_session);
                setPayloadAppIdData(p, direction, session, APP_ID_WEBDAV, NULL);
            }

            // Scan User-Agent for Browser types or Skype
            if ((session->scan_flags & SCAN_HTTP_USER_AGENT_FLAG) && session->clientAppId <= APP_ID_NONE && useragent && http_session->useragent_buflen)
            {
                if (version)
                {
                    free(version);
                    version = NULL;
                }
                identifyUserAgent((uint8_t *)useragent, http_session->useragent_buflen, &serviceAppId, &clientAppId, &version, &pConfig->detectorHttpConfig);
                if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                    _dpd.logMsg("AppIdDbg %s User Agent is service %d\n", app_id_debug_session, serviceAppId);
                setServiceAppIdData(p, direction, session, serviceAppId, NULL, NULL);
                if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                    _dpd.logMsg("AppIdDbg %s User Agent is client %d\n", app_id_debug_session, clientAppId);
                setClientAppIdData(p, direction, session, clientAppId, &version);
                session->scan_flags &= ~SCAN_HTTP_USER_AGENT_FLAG;
            }

            /* Scan Via Header for squid */
            if (!payloadAppIdIsSet(session) && (session->scan_flags & SCAN_HTTP_VIA_FLAG) && via && (size = strlen(via)) > 0)
            {
                if (version)
                {
                    free(version);
                    version = NULL;
                }
                payloadAppId = getAppidByViaPattern((uint8_t *)via, size, &version, &pConfig->detectorHttpConfig);
                if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
                    _dpd.logMsg("AppIdDbg %s VIA is payload %d\n", app_id_debug_session, payloadAppId);
                setPayloadAppIdData(p, direction, session, payloadAppId, NULL);
                session->scan_flags &= ~SCAN_HTTP_VIA_FLAG;
            }
        }

        /* Scan X-Working-With HTTP header */
        if ((thirdparty_appid_module && (session->scan_flags & SCAN_HTTP_XWORKINGWITH_FLAG) && session->hsession->x_working_with) ||
            (!thirdparty_appid_module && getHTTPHeaderLocation(p->payload, p->payload_size, HTTP_ID_X_WORKING_WITH, &start, &end, &hmp, &pConfig->detectorHttpConfig) == 1))
        {
            tAppId appId;

            if (thirdparty_appid_module)
                appId = scan_header_x_working_with((uint8_t*)session->hsession->x_working_with, strlen(session->hsession->x_working_with), &version);
            else appId = scan_header_x_working_with(p->payload + start, end - start, &version);

            if (appId)
            {
                if (direction == APP_ID_FROM_INITIATOR)
                {
                    if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                        _dpd.logMsg("AppIdDbg %s X is client %d\n", app_id_debug_session, appId);
                    setClientAppIdData(p, direction, session, appId, &version);
                }
                else
                {
                    if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                        _dpd.logMsg("AppIdDbg %s X is service %d\n", app_id_debug_session, appId);
                    setServiceAppIdData(p, direction, session, appId, NULL, &version);
                }
                session->scan_flags &= ~SCAN_HTTP_XWORKINGWITH_FLAG;
            }
        }

        // Scan Content-Type Header for multimedia types and scan contents
        if ((thirdparty_appid_module && (session->scan_flags & SCAN_HTTP_CONTENT_TYPE_FLAG)
             && session->hsession->content_type  && !payloadAppIdIsSet(session)) ||
            (!thirdparty_appid_module && !payloadAppIdIsSet(session) &&
             getHTTPHeaderLocation(p->payload, p->payload_size, HTTP_ID_CONTENT_TYPE, &start, &end, &hmp, &pConfig->detectorHttpConfig) == 1))
        {
            if (thirdparty_appid_module)
                payloadAppId = getAppidByContentType((uint8_t*)session->hsession->content_type, strlen(session->hsession->content_type), &pConfig->detectorHttpConfig);
            else payloadAppId = getAppidByContentType(p->payload + start, end - start, &pConfig->detectorHttpConfig);
            if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
                _dpd.logMsg("AppIdDbg %s Content-Type is payload %d\n", app_id_debug_session, payloadAppId);
            setPayloadAppIdData(p, direction, session, payloadAppId, NULL);
            session->scan_flags &= ~SCAN_HTTP_CONTENT_TYPE_FLAG;
        }

        if (session->scan_flags & SCAN_HTTP_HOST_URL_FLAG)
        {
            if (version)
            {
                free(version);
                version = NULL;
            }
            if (getAppIdFromUrl(host, url, &version, referer, &clientAppId, &serviceAppId, &payloadAppId, &referredPayloadAppId, 0, &pConfig->detectorHttpConfig) == 1)
            {
                // do not overwrite a previously-set client or service
                if (session->clientAppId <= APP_ID_NONE)
                {
                    if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                        _dpd.logMsg("AppIdDbg %s URL is client %d\n", app_id_debug_session, clientAppId);
                    setClientAppIdData(p, direction, session, clientAppId, NULL);
                }
                if (session->serviceAppId <= APP_ID_NONE)
                {
                    if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                        _dpd.logMsg("AppIdDbg %s URL is service %d\n", app_id_debug_session, serviceAppId);
                    setServiceAppIdData(p, direction, session, serviceAppId, NULL, NULL);
                }
                // DO overwrite a previously-set payload
                if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
                    _dpd.logMsg("AppIdDbg %s URL is payload %d\n", app_id_debug_session, payloadAppId);
                setPayloadAppIdData(p, direction, session, payloadAppId, &version);
                setReferredPayloadAppIdData(session, referredPayloadAppId);
            }
            session->scan_flags &= ~SCAN_HTTP_HOST_URL_FLAG;
        }

        if (session->clientAppId == APP_ID_APPLE_CORE_MEDIA)
        {
            if (session->tpPayloadAppId > APP_ID_NONE)
            {
                entry = appInfoEntryGet(session->tpPayloadAppId, pConfig);
                // only move tpPayloadAppId to client if its got a clientAppId
                if (entry && (entry->clientId > APP_ID_NONE))
                {
                    session->miscAppId = session->clientAppId;
                    session->clientAppId = session->tpPayloadAppId;
                }
            }
            else if (session->payloadAppId > APP_ID_NONE)
            {
                entry =  appInfoEntryGet(session->payloadAppId, pConfig);
                // only move payloadAppId to client if it has a clientAppid
                if (entry && (entry->clientId > APP_ID_NONE))
                {
                    session->miscAppId = session->clientAppId;
                    session->clientAppId = session->payloadAppId;
                }
            }
        }

        clearMiscHttpFlags(session);
    }  // end DON'T skip_simple_detect

    if (version) // We allocated this, but nobody used!
    {
        free(version);
        version = NULL;
    }
    PREPROC_PROFILE_END(httpPerfStats);
    return 0;
}

static inline void stopRnaServiceInspection(SFSnortPacket *p, tAppIdData* session, APPID_SESSION_DIRECTION direction)
{
    sfaddr_t *ip;
    if (direction == APP_ID_FROM_INITIATOR)
    {
        ip = GET_DST_IP(p);
        session->service_ip = *ip;
        session->service_port = p->dst_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        session->serviceAsId = p->pkt_header->address_space_id_dst;
#endif
    }
    else
    {
        ip = GET_SRC_IP(p);
        session->service_ip = *ip;
        session->service_port = p->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        session->serviceAsId = p->pkt_header->address_space_id_src;
#endif
    }
    session->rnaServiceState = RNA_STATE_FINISHED;

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
    session->carrierId = GET_SFOUTER_IPH_PROTOID(p, pkt_header);
#endif

    if ((TPIsAppIdAvailable(session->tpsession) || getAppIdFlag(session, APPID_SESSION_NO_TPI)) &&
        session->payloadAppId == APP_ID_NONE)
        session->payloadAppId = APP_ID_UNKNOWN;
    setAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED);
    clearAppIdFlag(session, APPID_SESSION_CONTINUE);
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "%u -> %u %d stopping RNA service inspection\n",
            (unsigned)p->src_port, (unsigned)p->dst_port, IsTCP(p)? IPPROTO_TCP:IPPROTO_UDP);
#endif
}

// Mock (derived?) function for _dpd.streamAPI->is_session_decrytped().
// It gets set at the beginning of fwAppIdSearch() in this file.
// Note that _dpd.streamAPI->is_session_decrypted() gets called multiple times
// in sfrna/firewall/src/spp_fw_engine.c.
// change UNIT_TESTING and UNIT_TEST_FIRST_DECRYPTED_PACKET in appIdApi.h
#ifdef UNIT_TEST_FIRST_DECRYPTED_PACKET
bool is_session_decrypted_unit_test(void * ssn)
{
    tAppIdData * session=getAppIdData(ssn);
    if (session && (session->session_packet_count >= UNIT_TEST_FIRST_DECRYPTED_PACKET))
        return 1;
    return 0;
}
#endif

static inline bool isSslDecryptionEnabled(tAppIdData *session)
{
    if (getAppIdFlag(session, APPID_SESSION_DECRYPTED))
        return 1;
    return _dpd.streamAPI->is_session_decrypted(session->ssn);
}

static inline void checkRestartSSLDetection(tAppIdData *session)
{
    if (getAppIdFlag(session, APPID_SESSION_DECRYPTED)) return;
    if (!isSslDecryptionEnabled(session)) return;

    tAppId serviceAppId = pickServiceAppId(session);
    bool isSsl = isSslServiceAppId(serviceAppId);

    // A session could either:
    // 1. Start of as SSL - captured with isSsl flag, OR
    // 2. It could start of as a non-SSL session and later change to SSL. For example, FTP->FTPS.
    //    In this case APPID_SESSION_ENCRYPTED flag is set by the protocol state machine.
    if (getAppIdFlag(session, APPID_SESSION_ENCRYPTED) || isSsl)
    {
#ifdef DEBUG_FW_APPID
        fprintf(SF_DEBUG_FILE, "SSL decryption is available, restarting app Detection\n");
#endif
        setAppIdFlag(session, APPID_SESSION_DECRYPTED);
        session->encrypted.serviceAppId = serviceAppId;
        session->encrypted.payloadAppId = pickPayloadId(session);
        session->encrypted.clientAppId = pickClientAppId(session);
        session->encrypted.miscAppId = pickMiscAppId(session);
        session->encrypted.referredAppId = pickReferredPayloadId(session);
        appSharedReInitData(session);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s SSL decryption is available, restarting app Detection\n", app_id_debug_session);

        // APPID_SESSION_ENCRYPTED is set upon receiving a command which upgrades the session to SSL.
        // Next packet after the command will have encrypted traffic.
        // In the case of a session which starts as SSL, current packet itself is encrypted. Set the special flag
        // APPID_SESSION_APP_REINSPECT_SSL which allows reinspection of this packet.
        if (isSsl)
            setAppIdFlag(session, APPID_SESSION_APP_REINSPECT_SSL);
    }
}

static inline void checkRestartTunnelDetection(tAppIdData *session)
{
    if ((session->hsession && session->hsession->is_tunnel) ||
        (session->tpPayloadAppId == APP_ID_HTTP_TUNNEL && !getAppIdFlag(session, APPID_SESSION_HTTP_TUNNEL)))
    {
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s Found HTTP Tunnel, restarting app Detection\n", app_id_debug_session);

        // Service
        if (session->serviceAppId == session->portServiceAppId)
            session->serviceAppId = APP_ID_NONE;
        session->portServiceAppId = APP_ID_NONE;
        if (session->serviceVendor)
        {
            free(session->serviceVendor);
            session->serviceVendor = NULL;
        }
        if (session->serviceVersion)
        {
            free(session->serviceVersion);
            session->serviceVersion = NULL;
        }
        IP_CLEAR(session->service_ip);
        session->service_port = 0; 
        session->rnaServiceState = RNA_STATE_NONE;
        session->serviceData = NULL;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        session->serviceAsId = 0xFF;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        session->carrierId = 0;
#endif

        AppIdFlowdataDeleteAllByMask(session, APPID_SESSION_DATA_SERVICE_MODSTATE_BIT);

        // Client
        session->rnaClientState = RNA_STATE_NONE;
        session->clientData = NULL;
        if (session->candidate_client_list)
        {
            sflist_free(session->candidate_client_list);
            session->candidate_client_list = NULL;
        }
        session->num_candidate_clients_tried = 0;
        AppIdFlowdataDeleteAllByMask(session, APPID_SESSION_DATA_CLIENT_MODSTATE_BIT);

        session->init_tpPackets = 0;
        session->resp_tpPackets = 0;

        session->scan_flags &= ~SCAN_HTTP_HOST_URL_FLAG;
        clearAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_CLIENT_DETECTED
                                | APPID_SESSION_HTTP_SESSION | APPID_SESSION_HTTP_CONNECT);
        if (session->hsession && session->hsession->is_tunnel)
        {
            session->hsession->is_tunnel = false;
            if (appidStaticConfig->http_tunnel_detect == HTTP_TUNNEL_DETECT_RESTART_AND_RESET && thirdparty_appid_module)
            {
                thirdparty_appid_module->session_delete(session->tpsession, 0);
                session->tpsession = NULL;
            }
        }

        setAppIdFlag(session, APPID_SESSION_HTTP_TUNNEL);
    }
    return;
}

static inline void checkRestartAppDetection(tAppIdData *session)
{
    checkRestartSSLDetection(session);
    checkRestartTunnelDetection(session);
}

static inline void updateEncryptedAppId( tAppIdData *session, tAppId serviceAppId)
{
    switch (serviceAppId)
    {
        case APP_ID_HTTP:
            if (session->miscAppId == APP_ID_NSIIOPS || session->miscAppId == APP_ID_DDM_SSL
                    || session->miscAppId == APP_ID_MSFT_GC_SSL || session->miscAppId == APP_ID_SF_APPLIANCE_MGMT)
            {
                break;
            }
            session->miscAppId = APP_ID_HTTPS;
            break;
        case APP_ID_SMTP:
            session->miscAppId = APP_ID_SMTPS;
            break;
        case APP_ID_NNTP:
            session->miscAppId = APP_ID_NNTPS;
            break;
        case APP_ID_IMAP:
            session->miscAppId = APP_ID_IMAPS;
            break;
        case APP_ID_SHELL:
            session->miscAppId = APP_ID_SSHELL;
            break;
        case APP_ID_LDAP:
            session->miscAppId = APP_ID_LDAPS;
            break;
        case APP_ID_FTP_DATA:
            session->miscAppId = APP_ID_FTPSDATA;
            break;
        case APP_ID_FTP:
	case APP_ID_FTP_CONTROL:
            session->miscAppId = APP_ID_FTPS;
            break;
        case APP_ID_TELNET:
            session->miscAppId = APP_ID_TELNET;
            break;
        case APP_ID_IRC:
            session->miscAppId = APP_ID_IRCS;
            break;
        case APP_ID_POP3:
            session->miscAppId = APP_ID_POP3S;
            break;
        default:
            break;
    }
}

/*
 * Desc: This function does AppId detection corresponding to the SSL params
 * The order of processing is:
 * Valid SNI:              SNI->first_SAN->CN->OU
 * No SNI/Mismatched SNI:  first_SAN->CN->OU
 */
static int scanSslParamsLookupAppId(tAppIdData *session, const char *serverName,
            bool isSniMismatch, const char *subjectAltName, const char *commonName,
            const char *orgName, tAppId *clientAppId, tAppId *payloadAppId)
{
    int ret = 0;

    if ((session->scan_flags & SCAN_SSL_HOST_FLAG) && serverName && !isSniMismatch)
    {
        ret = ssl_scan_hostname((const uint8_t *)serverName, strlen(serverName),
                clientAppId, payloadAppId, &pAppidActiveConfig->serviceSslConfig);
        session->tsession->matched_tls_type = MATCHED_TLS_HOST; 
        session->scan_flags &= ~SCAN_SSL_HOST_FLAG;
    }

    if (subjectAltName && (APP_ID_NONE == *clientAppId) && (APP_ID_NONE == *payloadAppId))
    {
        ret = ssl_scan_hostname((const uint8_t *)subjectAltName, strlen(subjectAltName),
                clientAppId, payloadAppId, &pAppidActiveConfig->serviceSslConfig);
        session->tsession->matched_tls_type = MATCHED_TLS_FIRST_SAN; 
    }

    if ((session->scan_flags & SCAN_SSL_CERTIFICATE_FLAG) && commonName &&
            (APP_ID_NONE == *clientAppId) && (APP_ID_NONE == *payloadAppId))
    {
        ret = ssl_scan_cname((const uint8_t *)commonName, strlen(commonName),
                clientAppId, payloadAppId, &pAppidActiveConfig->serviceSslConfig);
        session->tsession->matched_tls_type = MATCHED_TLS_CNAME; 
        session->scan_flags &= ~SCAN_SSL_CERTIFICATE_FLAG;
    }

    if (orgName && (APP_ID_NONE == *clientAppId) && (APP_ID_NONE == *payloadAppId))
    {
        ret = ssl_scan_cname((const uint8_t *)orgName, strlen(orgName),
                clientAppId, payloadAppId, &pAppidActiveConfig->serviceSslConfig);
        session->tsession->matched_tls_type = MATCHED_TLS_ORG_UNIT; 
    }

    if ((APP_ID_NONE == *clientAppId) && (APP_ID_NONE == *payloadAppId))
        session->tsession->matched_tls_type = MATCHED_TLS_NONE; 

    return ret;
}

static inline void ExamineSslMetadata(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *session, tAppIdConfig *pConfig)
{
    int ret = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;

    /* TLS params already scanned, skip scanning again */
    if ((session->scan_flags & SCAN_CERTVIZ_ENABLED_FLAG))
        return;

    ret = scanSslParamsLookupAppId(session, (const char*)session->tsession->tls_host,
            false, NULL, (const char*)session->tsession->tls_cname,
            (const char*)session->tsession->tls_orgUnit, &clientAppId, &payloadAppId);

    if (session->clientAppId == APP_ID_NONE ||
            session->clientAppId == APP_ID_SSL_CLIENT)
        setClientAppIdData(p, direction, session, clientAppId, NULL);
    setPayloadAppIdData(p, direction, session, payloadAppId, NULL);
    setSSLSquelch(p, ret, (ret == 1 ? payloadAppId : clientAppId));

    if (session->tsession->tls_orgUnit)
    {
        free(session->tsession->tls_orgUnit);
        session->tsession->tls_orgUnit = NULL;
    }

    if (session->tsession->tls_handshake_done &&
            session->payloadAppId == APP_ID_NONE)
    {
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s End of SSL/TLS handshake detected with no payloadAppId, so setting to unknown\n", app_id_debug_session);
        session->payloadAppId = APP_ID_UNKNOWN;
    }

}

static inline int RunClientDetectors(tAppIdData *session,
                              SFSnortPacket *p,
                              int direction,
                              tAppIdConfig *pConfig)
{
    int ret = CLIENT_APP_INPROCESS;
    const struct RNAClientAppModule *tmpClientData = session->clientData;

    if (tmpClientData != NULL)
    {
        ret = tmpClientData->validate(p->payload, p->payload_size, direction,
                                            session, p, tmpClientData->userData, pConfig);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s %s client detector returned %d\n", app_id_debug_session,
                        tmpClientData->name ? tmpClientData->name:"UNKNOWN", ret);
    }
    else
    {
        SF_LIST * tmpCandidateClientList = session->candidate_client_list;
        if (    (tmpCandidateClientList != NULL)
                  && (sflist_count(tmpCandidateClientList) > 0) )
        {
            SF_LNODE *node;
            tRNAClientAppModule *client;

            ret = CLIENT_APP_INPROCESS;
            node = sflist_first_node(tmpCandidateClientList);
            while (node != NULL)
            {
                int validator_result;
                SF_LNODE *node_tmp;

                client = (tRNAClientAppModule*)SFLIST_NODE_TO_DATA(node);
                validator_result = client->validate(p->payload, p->payload_size, direction,
                                          session, p, client->userData, pConfig);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s %s client detector returned %d\n", app_id_debug_session,
                                client->name ? client->name:"UNKNOWN", validator_result);

                if (validator_result == CLIENT_APP_SUCCESS)
                {
                    ret = CLIENT_APP_SUCCESS;
                    session->clientData = client;
                    sflist_free(tmpCandidateClientList);
                    session->candidate_client_list = NULL;
                    break;    /* done */
                }

                node_tmp = node;
                node = sflist_next_node(tmpCandidateClientList);
                if (validator_result != CLIENT_APP_INPROCESS)    /* fail */
                {
                    sflist_remove_node(tmpCandidateClientList, node_tmp);
                }
            }
        }
    }
    return ret;
}

static inline void synchAppIdWithSnortId(tAppId newAppId, SFSnortPacket *p, tAppIdData *session, tAppIdConfig *pConfig)
{
    if (newAppId  > APP_ID_NONE && newAppId < SF_APPID_MAX)
    {
        AppInfoTableEntry *entry;

        // Certain AppIds are not useful to identifying snort preprocessor choices
        switch (newAppId)
        {
            case APP_ID_FTPS:
            case APP_ID_FTPSDATA:

            // These all are variants of HTTPS
            case APP_ID_DDM_SSL:
            case APP_ID_MSFT_GC_SSL:
            case APP_ID_NSIIOPS:
            case APP_ID_SF_APPLIANCE_MGMT:
            case APP_ID_HTTPS:

            case APP_ID_IMAPS:
            case APP_ID_IRCS:
            case APP_ID_LDAPS:
            case APP_ID_NNTPS:
            case APP_ID_POP3S:
            case APP_ID_SMTPS:
            case APP_ID_SSHELL:
            case APP_ID_TELNETS:
                return;
            case APP_ID_HTTP:
                if (session->is_http2)
                    newAppId = APP_ID_HTTP2;
                break;
            default:
                break;
        }
        if ((entry = pAppidActiveConfig->AppInfoTable[newAppId]) != NULL)
        {
            register int16_t tempSnortId = entry->snortId;
            // A particular APP_ID_xxx may not be assigned a service_snort_key: value
            // in the rna_app.yaml file entry; so ignore the tempSnortId == 0 case.
            // Then if the value is different call the api.
            if ((tempSnortId != 0 || (tempSnortId = (newAppId == APP_ID_HTTP2) ? snortId_for_http2 : 0)) &&
                tempSnortId != session->snortId)
            {
                session->snortId = tempSnortId; // remember the most recent change
                // inform Snort so that other preprocessors can be turned on/off
                if (app_id_debug_session_flag)
                    if (tempSnortId == snortId_for_http2)
                        _dpd.logMsg("AppIdDbg %s Telling Snort that it's HTTP/2\n", app_id_debug_session);
#ifdef TARGET_BASED
                _dpd.sessionAPI->set_application_protocol_id(p->stream_session, tempSnortId);
#endif
                p->application_protocol_ordinal = tempSnortId;
            }
        }
    }
}

static inline void checkTerminateTpModule(uint16_t tpPktCount, tAppIdData *session)
{
    if ((tpPktCount >= appidStaticConfig->max_tp_flow_depth) ||
        (getAppIdFlag(session, APPID_SESSION_HTTP_SESSION | APPID_SESSION_APP_REINSPECT) ==
                (APPID_SESSION_HTTP_SESSION | APPID_SESSION_APP_REINSPECT) &&
         session->hsession && session->hsession->uri &&
         (!session->hsession->chp_candidate || session->hsession->chp_finished)))
    {
        if (session->tpAppId == APP_ID_NONE)
            session->tpAppId = APP_ID_UNKNOWN;
        if (session->rnaServiceState == RNA_STATE_FINISHED && session->payloadAppId == APP_ID_NONE)
            session->payloadAppId = APP_ID_UNKNOWN;
        if (thirdparty_appid_module)
            thirdparty_appid_module->session_delete(session->tpsession, 1);
    }
}

//#define DEBUG_PACKETS
#ifdef DEBUG_PACKETS
#define printSnortPacket( SFSnortPacket_ptr ) debug_printSnortPacket(SFSnortPacket_ptr)

#define CHAR_DUMP_WIDTH 60
static inline void debug_printSnortPacket (SFSnortPacket *p)
{
if (app_id_debug_flag) {
    char *tweakedPayload;
    char *hexPayload;
    _dpd.logMsg("AppIdDbg \n");
    _dpd.logMsg("AppIdDbg ------------------------------------------------\n");
    _dpd.logMsg("AppIdDbg \n");
    if (p->payload != NULL && p->payload_size)
    {
        tweakedPayload= (char *)malloc((CHAR_DUMP_WIDTH*2)+1); // room for hex
        if (tweakedPayload)
        {
            int j;
            int i;
            _dpd.logMsg("AppIdDbg payload: (%d chars per line)\n",CHAR_DUMP_WIDTH);
            for (j=0; j<p->payload_size; j+=CHAR_DUMP_WIDTH)
            {
                for (i=j; i<p->payload_size && i<(j+CHAR_DUMP_WIDTH); i++)
                {
                    if((int)p->payload[i] >= 32 && (int)p->payload[i] <=126)
                        tweakedPayload[i-j] = p->payload[i];
                    else
                        tweakedPayload[i-j] = '.';
                }
                tweakedPayload[i-j] = '\0';
                _dpd.logMsg("AppIdDbg %s\n", tweakedPayload);
            }
//#define DUMP_IN_HEX
#ifdef DUMP_IN_HEX
            _dpd.logMsg("AppIdDbg HEX payload: (%d chars per line)\n",CHAR_DUMP_WIDTH);
            for (j=0; j<p->payload_size; j+=CHAR_DUMP_WIDTH)
            {
                for (i=j; i<p->payload_size && i<(j+CHAR_DUMP_WIDTH); i++)
                {
                    sprintf(&tweakedPayload[(i-j)*2], "%02x", (p->payload)[i]);
                }
                // terminating '\0' provided by sprintf()
                _dpd.logMsg("AppIdDbg %s\n", tweakedPayload);
            }
#endif
            free(tweakedPayload); tweakedPayload = NULL;
        }
        else
        {
            DynamicPreprocessorFatalMessage("debug_printSnortPacket: "
                    "failed to allocate memory for tweakedPayload\n");
        }
    }
    if (p->stream_session)
    {
        _dpd.logMsg("AppIdDbg \nAppIdDbg for p->stream_session=%p is_session_decrypted=%d direction=%d\n",
                p->stream_session, _dpd.streamAPI->is_session_decrypted(p->stream_session),
                _dpd.sessionAPI->get_ignore_direction(p->stream_session));
    }

    _dpd.logMsg("AppIdDbg src_port: %d\n", p->src_port);
    _dpd.logMsg("AppIdDbg dst_port: %d\n", p->dst_port);
    _dpd.logMsg("AppIdDbg orig_src_port: %d\n", p->orig_src_port);
    _dpd.logMsg("AppIdDbg rig_dst_port: %d\n", p->orig_dst_port);
    _dpd.logMsg("AppIdDbg payloadsize: %d\n", p->payload_size);

    if ((p->flags) & 0x00000080)
    {
        _dpd.logMsg("AppIdDbg direction: client\n");
    }
    else if ((p->flags) & 0x00000040)
    {
        _dpd.logMsg("AppIdDbg direction: server\n");
    }
    else
    {
        _dpd.logMsg("AppIdDbg direction: unknown\n");
    }

    if ((p->flags) & 0x00000001) _dpd.logMsg("AppIdDbg A rebuilt fragment\n");
    if ((p->flags) & 0x00000002) _dpd.logMsg("AppIdDbg A rebuilt stream\n");
    if ((p->flags) & 0x00000004) _dpd.logMsg("AppIdDbg From an unestablished stream and we've only seen traffic in one direction\n");
    if ((p->flags) & 0x00000008) _dpd.logMsg("AppIdDbg From an established stream\n");
    if ((p->flags) & 0x00000010) _dpd.logMsg("AppIdDbg this packet has been queued for stream reassembly\n");
    if ((p->flags) & 0x00000020) _dpd.logMsg("AppIdDbg packet completes the 3 way handshake\n");
    if ((p->flags) & 0x00000040) _dpd.logMsg("AppIdDbg packet come from server side of a connection(tcp)\n");
    if ((p->flags) & 0x00000080) _dpd.logMsg("AppIdDbg packet come from client side of a connection(tcp)\n");
    if ((p->flags) & 0x00000100) _dpd.logMsg("AppIdDbg start of PDU\n");
    if ((p->flags) & 0x00000200) _dpd.logMsg("AppIdDbg end of PDU\n");
    if ((p->flags) & 0x00800000) _dpd.logMsg("AppIdDbg packet has new size\n");
}
}
#else
#define printSnortPacket( SFSnortPacket_ptr )
#endif

// ============================================
// Protocol and Direction Determination Section
// ============================================
// The getIPn_direction() functions are intended to return the direction for
// the protocols for the respective IP4 and IP6 headers...
// Except IPPROTO_TCP and IPPROTO_UDP which are handled within a
// Snort API call in appDetermineProtocol(), below.

static inline APPID_SESSION_DIRECTION getIP4_direction(SFSnortPacket *p)
{
    switch (p->ip4h->ip_proto)
    {
        case IPPROTO_ICMP:
            if (p->icmp_header)
            {
                switch (p->icmp_header->type)
                {
                    case 4:
                    case 5:
                    case 8:
                    case 13:
                    case 15:
                    case 17:
                        // It is a request type
                        return APP_ID_FROM_INITIATOR;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
    // no differentiation for this protocol; call it a responder
    // so that every packet will be checked.
    return APP_ID_FROM_RESPONDER;
}
static inline APPID_SESSION_DIRECTION getIP6_direction(SFSnortPacket *p)
{
    switch (p->ip6h->next)
    {
        case IPPROTO_ICMPV6:
            if (p->icmp6h)
            {
                switch (p->icmp6h->type)
                {
                    case 138:
                        // only code 0 is a request.
                        if (p->icmp6h->code == 0)
                            return APP_ID_FROM_INITIATOR;
                        break;
                    case 128:
                    case 130:
                    case 133:
                    case 135:
                        // It is a request type
                        return APP_ID_FROM_INITIATOR;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
    // no differentiation for this protocol; call it a responder
    // so that every packet will be checked.
    return APP_ID_FROM_RESPONDER;
}

static inline int appDetermineProtocol(SFSnortPacket *p, tAppIdData *session, uint8_t *protocolp, uint8_t *outer_protocol, APPID_SESSION_DIRECTION *directionp)
{
    // 'session' pointer may be NULL. In which case the packet examination is required.
    // But if 'session' is not NULL we use values saved from the initial creation
    if (session)
    {
        sfaddr_t *ip;
#ifdef DEBUG_APP_ID_SESSIONS
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
        if (session->service_port == DEBUG_FW_APPID_PORT)
#endif
        {
            char src_ip[INET6_ADDRSTRLEN];
            char dst_ip[INET6_ADDRSTRLEN];

            src_ip[0] = 0;
            ip = GET_SRC_IP(p);
            inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), src_ip, sizeof(src_ip));
            dst_ip[0] = 0;
            ip = GET_DST_IP(p);
            inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), dst_ip, sizeof(dst_ip));
            fprintf(SF_DEBUG_FILE, "AppId Session %p %p for %s-%u -> %s-%u %d\n", session, session->ssn, src_ip,
                    (unsigned)p->src_port, dst_ip, (unsigned)p->dst_port, IsTCP(p) ? IPPROTO_TCP:IPPROTO_UDP);
        }
#endif
        if (session->common.fsf_type.flow_type == APPID_SESSION_TYPE_IGNORE)
            return 0; // just ignore
        if (session->common.fsf_type.flow_type == APPID_SESSION_TYPE_NORMAL)
        {
            *protocolp = session->proto;
            session->ssn = p->stream_session;
        }
        else if (IsTCP(p))
            *protocolp = IPPROTO_TCP;
        else
            *protocolp = IPPROTO_UDP;
        ip = GET_SRC_IP(p);
        if (session->common.initiator_port)
            *directionp = (session->common.initiator_port == p->src_port) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
        else
            *directionp = (memcmp(sfaddr_get_ip6_ptr(ip), &session->common.initiator_ip, sizeof(session->common.initiator_ip))) ? APP_ID_FROM_RESPONDER: APP_ID_FROM_INITIATOR ;
    }
    else
    {
        if (IsTCP(p))
        {
            *protocolp = IPPROTO_TCP;
            *directionp = (_dpd.sessionAPI->get_packet_direction(p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
        }
        else if (IsUDP(p))
        {
            *protocolp = IPPROTO_UDP;
            *directionp = (_dpd.sessionAPI->get_packet_direction(p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
        }
        else if (IsIP(p))
        {
            *protocolp = GET_IPH_PROTO(p);
            if (p->outer_iph_api)
            {
                IP4Hdr *save_ip4h = p->ip4h;
                IP6Hdr *save_ip6h = p->ip6h;

                p->ip4h = &p->outer_ip4h;
                p->ip6h = &p->outer_ip6h;

                *outer_protocol = p->outer_iph_api->iph_ret_proto(p);

                p->ip4h = save_ip4h;
                p->ip6h = save_ip6h;
            }

            if (IS_IP4(p) && p->ip4h)
                *directionp = getIP4_direction(p);
            else if (p->ip6h)
                *directionp = getIP6_direction(p);
            else
                *directionp = APP_ID_FROM_RESPONDER;
        }
        else
        {
            // Neither IPv4 nor IPv6 - currently unsupported
            return 0;
        }
    }
    return 1;
}

static inline bool checkThirdPartyReinspect(const SFSnortPacket* p, tAppIdData* session)
{
    return getAppIdFlag(session, APPID_SESSION_HTTP_SESSION) && !getAppIdFlag(session, APPID_SESSION_NO_TPI) && TPIsAppIdDone(session->tpsession) && p->payload_size;
}

static inline int getIpPortFromHttpTunnel(char *url, int url_len, tunnelDest **tunDest)
{
    char *host = NULL, *host_start, *host_end, *url_end;
    char *portStr = NULL;
    uint16_t port = 0;
    int isIPv6 = 0, family;

    if (url_len <= 0 || !url || !tunDest)
    {
        return 1;
    }

    url_end = url + url_len - 1;
    host_start = url;

    if (url[0] == '[')
    {
        // IPv6 literal
        isIPv6 = 1;
        portStr = strchr(url, ']');
        if (portStr && portStr < url_end)
        {
            if (*(++portStr) != ':')
            {
                portStr = NULL;
            }
        }
    }
    else if(isdigit(url[0])) // Checking the first character for a possible domain name.
    {
        portStr = strrchr(url, ':');
    }
    else
    {
        return 1;
    }

    if (portStr && portStr < url_end )
    {
        host_end = portStr;
        if (*(++portStr) != '\0')
        {
            char *end = NULL;
            long ret = strtol(portStr, &end, 10);
            if (end != portStr && *end == '\0' && ret >= 1 && ret <= PORT_MAX)
            {
                port = (uint16_t)ret;
            }
        }
    }

    if (port)
    {
        if (isIPv6)
        {
            // IPv6 is enclosed in square braces. Adjusting the pointers.
            host_start++;
            host_end--;
        }

        if (host_start <= host_end)
        {
            char tmp = *host_end;
            *host_end = '\0';
            host = strdup(host_start);
            *host_end = tmp;
        }
        else
            return 1;
    }

    if (host)
    {
        tunnelDest *tDest = _dpd.snortAlloc(1, sizeof(*tDest), PP_APP_ID,
                PP_MEM_CATEGORY_SESSION);
        if (!tDest)
        {
            _dpd.errMsg("AppId: Unable to allocate memory for HTTP tunnel information\n");
            free(host);
            return 1;
        }

        if (!isIPv6)
        {
            if (inet_pton(AF_INET, host, &(tDest->ip).ip.s6_addr32[3]) <= 0)
            {
                free(host);
                _dpd.snortFree(tDest, sizeof(*tDest), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
                return 1;
            }
            (tDest->ip).ip.s6_addr32[0] = (tDest->ip).ip.s6_addr32[1] =  0;
            (tDest->ip).ip.s6_addr32[2] = ntohl(0x0000ffff);

            family = AF_INET;
        }
        else
        {
            if (inet_pton(AF_INET6, host, &(tDest->ip).ip) <= 0)
            {
                free(host);
                _dpd.snortFree(tDest, sizeof(*tDest), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
                return 1;
            }
            family = AF_INET6;
        }

        (tDest->ip).family = family;
        tDest->port = port;

        *tunDest = tDest;

        free(host);
    }
    else
    {
        return 1;
    }

    return 0;
}

static int checkHostCache(SFSnortPacket *p, tAppIdData *session, sfaddr_t *ip, uint16_t port, uint8_t protocol, tAppIdConfig *pConfig)
{
    bool checkStatic = false, checkDynamic = false;
    tHostPortVal *hv = NULL;

    if (!(session->scan_flags & SCAN_HOST_PORT_FLAG))
        checkStatic = true;

    if (isHostCacheUpdated(session->hostCacheVersion))
    {
        if ((session->session_packet_count % appidStaticConfig->host_port_app_cache_lookup_interval == 0) &&
             session->session_packet_count <= appidStaticConfig->host_port_app_cache_lookup_range &&
             appidStaticConfig->is_host_port_app_cache_runtime)
            checkDynamic = true;
    }

    if (!(checkStatic || checkDynamic))
        return 0;

    if (checkStatic)
    {
        hv = hostPortAppCacheFind(ip, port, protocol, pConfig);
        session->scan_flags |= SCAN_HOST_PORT_FLAG;
    }

    if (!hv && checkDynamic)
    {
        hv = hostPortAppCacheDynamicFind(ip, port, protocol);
        updateHostCacheVersion(&(session->hostCacheVersion));
    }

    if (hv)
    {
        switch (hv->type)
        {
            case APP_ID_TYPE_SERVICE:
                session->serviceAppId = hv->appId;
                synchAppIdWithSnortId(hv->appId, p, session, pConfig);
                session->rnaServiceState = RNA_STATE_FINISHED;
                session->rnaClientState = RNA_STATE_FINISHED;
                setAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED);
                if (thirdparty_appid_module)
                    thirdparty_appid_module->session_delete(session->tpsession, 1);
		if (session->payloadAppId == APP_ID_NONE)
                    session->payloadAppId = APP_ID_UNKNOWN;
            case APP_ID_TYPE_CLIENT:
                session->clientAppId = hv->appId;
                session->rnaClientState = RNA_STATE_FINISHED;
                break;
            case APP_ID_TYPE_PAYLOAD:
                session->payloadAppId = hv->appId;
                break;
            default:
                break;
        }
        setAppIdFlag(session, APPID_SESSION_HOST_CACHE_MATCHED);
        return 1;
    }
    return 0;
}

static inline bool isCheckHostCacheValid(tAppIdData* session, tAppId serviceAppId, tAppId clientAppId, tAppId payloadAppId, tAppId miscAppId)
{
    bool isPayloadClientNone = (payloadAppId <= APP_ID_NONE && clientAppId <= APP_ID_NONE);

    bool isAppIdNone = isPayloadClientNone && (serviceAppId <= APP_ID_NONE || serviceAppId == APP_ID_UNKNOWN_UI ||
                        (appidStaticConfig->recheck_for_portservice_appid && serviceAppId == session->portServiceAppId));

    bool isSslNone = appidStaticConfig->check_host_cache_unknown_ssl && getAppIdFlag(session, APPID_SESSION_SSL_SESSION) &&
                          !(session->tsession && session->tsession->tls_host && session->tsession->tls_cname);

    if(isAppIdNone || isSslNone || appidStaticConfig->check_host_port_app_cache)
    {
        return true;
    }
    return false;
}

void fwAppIdInit(void)
{
    /* init globals for snortId compares etc. */
#ifdef TARGET_BASED
    snortId_for_unsynchronized = _dpd.addProtocolReference("unsynchronized");
    snortId_for_ftp_data = _dpd.findProtocolReference("ftp-data");
    snortId_for_http2    = _dpd.findProtocolReference("http2");
#endif
    snortInstance = _dpd.getSnortInstance();
}

static inline tAppId processThirdParty(SFSnortPacket* p, tAppIdData* session, APPID_SESSION_DIRECTION direction, uint8_t protocol, bool* isTpAppidDiscoveryDone,
                                       tAppIdConfig *pConfig)
{
    tAppId tpAppId = session->tpAppId;
    int tp_confidence;
    tAppId* tp_proto_list;
    ThirdPartyAppIDAttributeData* tp_attribute_data;
    sfaddr_t *ip;

    /*** Start of third-party processing. ***/
    PROFILE_VARS;
    PREPROC_PROFILE_START(tpPerfStats);
    if (p->payload_size || appidStaticConfig->tp_allow_probes)
    {
        //restart inspection by 3rd party
        if (!session->tpReinspectByInitiator && (direction == APP_ID_FROM_INITIATOR) && checkThirdPartyReinspect(p, session))
        {
            session->tpReinspectByInitiator = 1;     //once per request
            setAppIdFlag(session, APPID_SESSION_APP_REINSPECT);
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s 3rd party allow reinspect http\n", app_id_debug_session);
            session->init_tpPackets = 0;
            session->resp_tpPackets = 0;
            appHttpFieldClear(session->hsession);
        }

        if (!isTPProcessingDone(session))
        {
            if (protocol != IPPROTO_TCP || (p->flags & FLAG_STREAM_ORDER_OK) || appidStaticConfig->tp_allow_probes)
            {
                PREPROC_PROFILE_START(tpLibPerfStats);
                if (!session->tpsession)
                {
                    if (!(session->tpsession = thirdparty_appid_module->session_create()))
                        DynamicPreprocessorFatalMessage("Could not allocate tAppIdData->tpsession data");
                }
                printSnortPacket(p); // debug output of packet content
                thirdparty_appid_module->session_process(session->tpsession, p, direction,
                                                         &tpAppId, &tp_confidence, &tp_proto_list, &tp_attribute_data);
                PREPROC_PROFILE_END(tpLibPerfStats);

                // First SSL decrypted packet is now being inspected. Reset the flag so that SSL decrypted traffic
                // gets processed like regular traffic from next packet onwards
                if (getAppIdFlag(session, APPID_SESSION_APP_REINSPECT_SSL))
                    clearAppIdFlag(session, APPID_SESSION_APP_REINSPECT_SSL);

                *isTpAppidDiscoveryDone = true;

                if (thirdparty_appid_module->session_state_get(session->tpsession) == TP_STATE_CLASSIFIED)
                    clearAppIdFlag(session, APPID_SESSION_APP_REINSPECT);

                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s 3rd party returned %d\n", app_id_debug_session, tpAppId);
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "%u -> %u %u 3rd party returned %d\n",
                            (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, tpAppId);
#endif

                // For now, third party can detect HTTP/2 (w/o metadata) for
                // some cases.  Treat it like HTTP w/ is_http2 flag set.
                if ((tpAppId == APP_ID_HTTP2) && (tp_confidence == 100))
                {
                    if (app_id_debug_session_flag)
                        _dpd.logMsg("AppIdDbg %s 3rd party saw HTTP/2\n", app_id_debug_session);
                    tpAppId = APP_ID_HTTP;
                    session->is_http2 = true;
                }

                // if the third-party appId must be treated as a client, do it now
                uint32_t entryFlags = appInfoEntryFlags(tpAppId, pConfig);
                if (entryFlags & APPINFO_FLAG_TP_CLIENT)
                    session->clientAppId = tpAppId;

                ProcessThirdPartyResults(p, direction, session, tp_confidence, tp_proto_list, tp_attribute_data);

                if ((entryFlags & APPINFO_FLAG_SSL_SQUELCH) &&
                    getAppIdFlag(session, APPID_SESSION_SSL_SESSION) &&
                    !(session->scan_flags & SCAN_SSL_HOST_FLAG))
                {
                    if (!(session->scan_flags & SCAN_SPOOFED_SNI_FLAG))
                    {
                        setSSLSquelch(p, 1, tpAppId);
                    }
                    else
                    {
                        if (app_id_debug_session_flag)
                            _dpd.logMsg("AppIdDbg %s Ignored 3rd party returned %d, SNI is spoofed\n",
                                app_id_debug_session, tpAppId);
                    }
                }

                if (entryFlags & APPINFO_FLAG_IGNORE)
                {
                    if (app_id_debug_session_flag)
                        _dpd.logMsg("AppIdDbg %s 3rd party ignored\n", app_id_debug_session);
                    if (getAppIdFlag(session, APPID_SESSION_HTTP_SESSION))
                        tpAppId = APP_ID_HTTP;
                    else if(getAppIdFlag(session, APPID_SESSION_SSL_SESSION))
                        tpAppId = APP_ID_SSL;
                    else
                        tpAppId = APP_ID_NONE;
                }
            }
            else
            {
                tpAppId = APP_ID_NONE;
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "%u -> %u %u Skipping ooo\n",
                            (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol);
#endif
            }

            if (thirdparty_appid_module->session_state_get(session->tpsession) == TP_STATE_MONITORING)
            {
                thirdparty_appid_module->disable_flags(session->tpsession, TP_SESSION_FLAG_ATTRIBUTE | TP_SESSION_FLAG_TUNNELING | TP_SESSION_FLAG_FUTUREFLOW);
            }
#ifdef TARGET_BASED
            if(tpAppId == APP_ID_SSL && (_dpd.sessionAPI->get_application_protocol_id(p->stream_session) == snortId_for_ftp_data))
            {
                //  If we see SSL on an FTP data channel set tpAppId back
                //  to APP_ID_NONE so the FTP preprocessor picks up the flow.
                tpAppId = APP_ID_NONE;
            }
#endif
            if (tpAppId > APP_ID_NONE && (!getAppIdFlag(session, APPID_SESSION_APP_REINSPECT) || session->payloadAppId > APP_ID_NONE))
            {
#ifdef TARGET_BASED
                tAppId snortAppId;
#endif

                // if the packet is HTTP, then search for via pattern
                if (getAppIdFlag(session, APPID_SESSION_HTTP_SESSION) && session->hsession)
                {
#ifdef TARGET_BASED
                    snortAppId = APP_ID_HTTP;
#endif
                    //payload should never be APP_ID_HTTP
                    if (tpAppId != APP_ID_HTTP)
                        setTPPayloadAppIdData(p, direction, session, tpAppId);

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                        fprintf(SF_DEBUG_FILE, "%u -> %u %u tp identified http payload %d\n",
                                (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, tpAppId);
#endif

                    session->tpAppId = APP_ID_HTTP;

                    processHTTPPacket(p, session, direction, NULL, pConfig);

                    if (TPIsAppIdAvailable(session->tpsession) && session->tpAppId == APP_ID_HTTP
                                                        && !getAppIdFlag(session, APPID_SESSION_APP_REINSPECT))
                    {
                        session->rnaClientState = RNA_STATE_FINISHED;
                        setAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED | APPID_SESSION_SERVICE_DETECTED);
                        session->rnaServiceState = RNA_STATE_FINISHED;
                        clearAppIdFlag(session, APPID_SESSION_CONTINUE);
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                        session->carrierId = GET_SFOUTER_IPH_PROTOID(p, pkt_header);
#endif
                        if (direction == APP_ID_FROM_INITIATOR)
                        {
                            ip = GET_DST_IP(p);
                            session->service_ip = *ip;
                            session->service_port = p->dst_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                            session->serviceAsId = p->pkt_header->address_space_id_dst;
#endif
                        }
                        else
                        {
                            ip = GET_SRC_IP(p);
                            session->service_ip = *ip;
                            session->service_port = p->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                            session->serviceAsId = p->pkt_header->address_space_id_src;
#endif
                        }
                    }
                }
                else if (getAppIdFlag(session, APPID_SESSION_SSL_SESSION) && session->tsession)
                {
                    ExamineSslMetadata(p, direction, session, pConfig);

                    uint16_t serverPort;
                    tAppId portAppId;

                    serverPort = (direction == APP_ID_FROM_INITIATOR)? p->dst_port:p->src_port;

                    portAppId = getSslServiceAppId(serverPort);
                    if (tpAppId == APP_ID_SSL )
                    {
                        tpAppId = portAppId;

                        //SSL policy needs to determine IMAPS/POP3S etc before appId sees first server packet
                        session->portServiceAppId = portAppId;

                        if (app_id_debug_session_flag)
                            _dpd.logMsg("AppIdDbg %s SSL is service %d, portServiceAppId %d\n",
                                    app_id_debug_session, tpAppId, session->portServiceAppId);
                    }
                    else
                    {
                        if (!(session->scan_flags & SCAN_SPOOFED_SNI_FLAG))
                        {
                            setTPPayloadAppIdData(p, direction, session, tpAppId);
                        }
                        else
                        {
                            if (app_id_debug_session_flag)
                                _dpd.logMsg("AppIdDbg %s Ignoring 3rd party returned %d, SNI is spoofed\n",
                                    app_id_debug_session, tpAppId);
                        }
                        tpAppId = portAppId;
                        if (app_id_debug_session_flag)
                            _dpd.logMsg("AppIdDbg %s SSL is %d\n", app_id_debug_session, tpAppId);
                    }
		    setTPAppIdData(p, direction, session, tpAppId);
#ifdef TARGET_BASED
                    snortAppId = APP_ID_SSL;
#endif

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                        fprintf(SF_DEBUG_FILE, "%u -> %u %u tp identified ssl service %d\n",
                                (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, tpAppId);
#endif
                }
                else
                {
                    //for non-http protocols, tp id is treated like serviceId

#ifdef TARGET_BASED
                    snortAppId = tpAppId;
#endif

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                        fprintf(SF_DEBUG_FILE, "%u -> %u %u tp identified non-http service %d\n",
                                (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, tpAppId);
#endif
                    setTPAppIdData(p, direction, session, tpAppId);
                }

#ifdef TARGET_BASED
                synchAppIdWithSnortId(snortAppId, p, session, pConfig);
#endif
            }
            else
            {
                if ((session->serviceAppId != APP_ID_ENIP && session->serviceAppId != APP_ID_CIP) &&
                   (protocol != IPPROTO_TCP || (p->flags & (FLAG_STREAM_ORDER_OK | FLAG_STREAM_ORDER_BAD))))
                {
                    if (direction == APP_ID_FROM_INITIATOR)
                    {
                        session->init_tpPackets++;
                        checkTerminateTpModule(session->init_tpPackets, session);
                    }
                    else
                    {
                        session->resp_tpPackets++;
                        checkTerminateTpModule(session->resp_tpPackets, session);
                    }
                }
            }
        }

        else if (session->hsession && (direction == APP_ID_FROM_RESPONDER) &&
            getAppIdFlag(session, APPID_SESSION_HTTP_CONNECT))
        {
            if ((p->payload_size >= 13) && !strncasecmp((char *)p->payload, "HTTP/1.1 200 ", 13))
                session->hsession->is_tunnel = true;
        }
        if (session->tpReinspectByInitiator && checkThirdPartyReinspect(p, session))
        {
            if (*isTpAppidDiscoveryDone)
                clearAppIdFlag(session, APPID_SESSION_APP_REINSPECT);
            if (direction == APP_ID_FROM_RESPONDER)
                session->tpReinspectByInitiator = 0; //toggle at OK response
        }
    }
    PREPROC_PROFILE_END(tpPerfStats);
    /*** End of third-party processing. ***/

    return tpAppId;
}

void fwAppIdSearch(SFSnortPacket *p)
{
    tAppIdData *session;
    uint8_t protocol, outer_protocol = 0;
    APPID_SESSION_DIRECTION direction;
    tAppId tpAppId = 0;
    tAppId serviceAppId = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    tAppId miscAppId = 0;
    bool isTpAppidDiscoveryDone = false;
    uint64_t flow_flags;
    sfaddr_t *ip;
    uint16_t port;
    size_t size;
#ifdef TARGET_BASED
    AppInfoTableEntry *entry;
#endif
    tAppIdConfig *pConfig = appIdActiveConfigGet();

#ifdef UNIT_TEST_FIRST_DECRYPTED_PACKET
    if(app_id_raw_packet_count==0)
        _dpd.streamAPI->is_session_decrypted=is_session_decrypted_unit_test;
#endif

    app_id_raw_packet_count++;

    if (!p->stream_session || (p->payload_size && !p->payload))
    {
        app_id_ignored_packet_count++;
        return;
    }

    SetPacketRealTime(p->pkt_header->ts.tv_sec);

    session = appSharedGetData(p);
    if (!appDetermineProtocol(p, session, &protocol, &outer_protocol, &direction))
    {
        // unsupported protocol or other ignore
        app_id_ignored_packet_count++;
        return;
    }

    if (pConfig->debugHostIp)
    {
        AppIdDebugHostInfo.session = session;
        AppIdDebugHostInfo.protocol = protocol;
        AppIdDebugHostInfo.direction = direction;
        if (session)
        {
            memcpy(&AppIdDebugHostInfo.initiatorIp, &session->common.initiator_ip, sizeof(session->common.initiator_ip));
            AppIdDebugHostInfo.initiatorPort = session->common.initiator_port;
            AppIdDebugHostInfo.family = sfaddr_family(GET_SRC_IP(p));
        }
        else
        {
            memset(&AppIdDebugHostInfo.initiatorIp, 0, sizeof(session->common.initiator_ip));
            AppIdDebugHostInfo.initiatorPort = 0;
            AppIdDebugHostInfo.family = AF_INET;
        }
        AppIdDebugHostInfo.monitorType = APPID_DEBUG_HOST_NOT_MONITORED;
    }

    app_id_debug_session_flag = fwAppIdDebugCheck(p->stream_session, session, app_id_debug_flag,
            &app_id_debug_info, app_id_debug_session, direction);

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
    {
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        sipstr[0] = 0;
        ip = GET_SRC_IP(p);
        inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), sipstr, sizeof(sipstr));
        dipstr[0] = 0;
        ip = GET_DST_IP(p);
        inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), dipstr, sizeof(dipstr));

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
        uint32_t cid = GET_SFOUTER_IPH_PROTOID(p, pkt_header);

#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        uint16_t sAsId = p->pkt_header->address_space_id_src;
        uint16_t dAsId = p->pkt_header->address_space_id_dst;

        fprintf(SF_DEBUG_FILE, "%s-%u -> %s-%u  AS %u-%u CID %u %u\n", sipstr,
                (unsigned)p->src_port, dipstr, (unsigned)p->dst_port,
                sAsId, dAsId, (unsigned)cid, (unsigned)protocol);
#else
        fprintf(SF_DEBUG_FILE, "%s-%u -> %s-%u CID %u %u\n", sipstr, (unsigned)p->src_port, dipstr, (unsigned)p->dst_port, (unsigned)cid,
                (unsigned)protocol);
#endif /* No Carrierid support*/
#else
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
        uint16_t sAsId = p->pkt_header->address_space_id_src;
        uint16_t dAsId = p->pkt_header->address_space_id_dst;
        fprintf(SF_DEBUG_FILE, "%s-%u -> %s-%u  AS %u-%u %u\n", sipstr,
                (unsigned)p->src_port, dipstr, (unsigned)p->dst_port,
                sAsId, dAsId, (unsigned)protocol);
#else
        fprintf(SF_DEBUG_FILE, "%s-%u -> %s-%u %u\n", sipstr, (unsigned)p->src_port, dipstr, (unsigned)p->dst_port, (unsigned)protocol);
#endif
#endif

        /*DumpHex(SF_DEBUG_FILE, p->payload, p->payload_size); */
    }
#endif

    if (protocol == IPPROTO_TCP) // HTTP is a subset of TCP
    {
        if (_dpd.streamAPI->is_session_http2(p->stream_session))
        {
            if (session)
                session->is_http2 = true;
            if (!(p->flags & FLAG_REBUILT_STREAM))
            {
                // For HTTP/2, we only want to look at the ones that are rebuilt from
                // Stream / HTTP Inspect as HTTP/1 packets.
                app_id_ignored_packet_count++;
                return;
            }
        }
        else    // not HTTP/2
        {
            if (p->flags & FLAG_REBUILT_STREAM && !_dpd.streamAPI->is_session_decrypted(p->stream_session))
            {
                if (direction == APP_ID_FROM_INITIATOR && session && session->hsession && session->hsession->get_offsets_from_rebuilt)
                {
                    httpGetNewOffsetsFromPacket(p, session->hsession, pConfig);
                    if (app_id_debug_session_flag)
                        _dpd.logMsg("AppIdDbg %s offsets from rebuilt packet: uri: %u-%u cookie: %u-%u\n", app_id_debug_session, session->hsession->fieldOffset[REQ_URI_FID], session->hsession->fieldEndOffset[REQ_URI_FID], session->hsession->fieldOffset[REQ_COOKIE_FID], session->hsession->fieldEndOffset[REQ_COOKIE_FID]);
                }
                app_id_ignored_packet_count++;
                return;
            }
        }
    }
    // fwAppIdSearch() is a top-level function that is called by AppIdProcess().
    // At this point, we know that we need to use the current active config -
    // pAppidActiveConfig. This function uses pAppidActiveConfig and passes it
    // to all the functions that need to look at AppId config.
    flow_flags = isSessionMonitored(p, direction, session);
    if (!(flow_flags & (APPID_SESSION_DISCOVER_APP | APPID_SESSION_SPECIAL_MONITORED)))
    {
        if (!session)
        {
            if ((flow_flags & APPID_SESSION_BIDIRECTIONAL_CHECKED) == APPID_SESSION_BIDIRECTIONAL_CHECKED)
            {
                static APPID_SESSION_STRUCT_FLAG ignore_fsf = {.flow_type = APPID_SESSION_TYPE_IGNORE};
                _dpd.sessionAPI->set_application_data(p->stream_session, PP_APP_ID, &ignore_fsf, NULL);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s not monitored\n", app_id_debug_session);
            }
            else
            {
                tTmpAppIdData *tmp_session;

                if (tmp_app_id_free_list)
                {
                    tmp_session = tmp_app_id_free_list;
                    tmp_app_id_free_list = tmp_session->next;
                    app_id_tmp_free_list_count--;
                }
                else if (!(tmp_session = _dpd.snortAlloc(1, sizeof(*tmp_session), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                    DynamicPreprocessorFatalMessage("Could not allocate tTmpAppIdData data");
                tmp_session->common.fsf_type.flow_type = APPID_SESSION_TYPE_TMP;
                tmp_session->common.flags = flow_flags;
                ip = (direction == APP_ID_FROM_INITIATOR) ? GET_SRC_IP(p) : GET_DST_IP(p);
                sfaddr_copy_to_raw(&tmp_session->common.initiator_ip, ip);
                if ((protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) && p->src_port != p->dst_port)
                    tmp_session->common.initiator_port = (direction == APP_ID_FROM_INITIATOR) ? p->src_port : p->dst_port;
                else
                    tmp_session->common.initiator_port = 0;
                tmp_session->common.policyId = appIdPolicyId;
                _dpd.sessionAPI->set_application_data(p->stream_session, PP_APP_ID,
                        tmp_session, (void (*)(void*))appTmpSharedDataFree);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s unknown monitoring\n", app_id_debug_session);
            }
        }
        else
        {
            session->common.flags = flow_flags;
            if ((flow_flags & APPID_SESSION_BIDIRECTIONAL_CHECKED) == APPID_SESSION_BIDIRECTIONAL_CHECKED)
                session->common.fsf_type.flow_type = APPID_SESSION_TYPE_IGNORE;
            session->common.policyId = appIdPolicyId;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s not monitored\n", app_id_debug_session);
        }
        return;
    }

    if (!session || session->common.fsf_type.flow_type == APPID_SESSION_TYPE_TMP)
    {
        /* This call will free the existing temporary session, if there is one */
        session = appSharedCreateData(p, protocol, direction);
        if (_dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
        {
            flow_flags |= APPID_SESSION_MID; // set once per flow
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s new mid-stream session\n", app_id_debug_session);
        }
        else if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s new session\n", app_id_debug_session);
    }

    app_id_processed_packet_count++;
    session->session_packet_count++;

    if (direction == APP_ID_FROM_INITIATOR)
    {
        session->stats.initiatorBytes += p->pkt_header->pktlen;
        if ( p->payload_size)
        {
            session->initiatorPcketCountWithoutReply ++;
            session->initiatorBytesWithoutServerReply += p->payload_size;
        }
    }
    else
    {
        session->stats.responderBytes += p->pkt_header->pktlen;
        if(p->payload_size)
        {
            session->initiatorPcketCountWithoutReply = 0;
            session->initiatorBytesWithoutServerReply = 0;
        }
    }
    session->common.flags = flow_flags;
    session->common.policyId = appIdPolicyId;

    tpAppId = session->tpAppId;

    session->common.policyId = appIdPolicyId;

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
    {
#endif
        fprintf(SF_DEBUG_FILE, "%u %u -> %u %u Begin %d %u - (%d %d %d %d %d) %u %" PRIx64 " %" PRIx64 " (%u %u %u)\n",
                (unsigned )session->session_packet_count, (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, direction,
                (unsigned)p->payload_size, session->serviceAppId, session->clientAppId, session->payloadAppId, tpAppId, session->miscAppId,
                session->rnaServiceState, session->common.flags, p->flags, thirdparty_appid_module->session_state_get(session->tpsession),
                (unsigned)session->init_tpPackets, (unsigned)session->resp_tpPackets);
        /*DumpHex(SF_DEBUG_FILE, p->payload, p->payload_size); */
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    }
#endif
#endif

    if (getAppIdFlag(session, APPID_SESSION_IGNORE_FLOW))
    {
        if (app_id_debug_session_flag && !getAppIdFlag(session, APPID_SESSION_IGNORE_FLOW_LOGGED))
        {
            setAppIdFlag(session, APPID_SESSION_IGNORE_FLOW_LOGGED);
            _dpd.logMsg("AppIdDbg %s Ignoring flow with service %d\n", app_id_debug_session, session->serviceAppId);
        }
        return;
    }

    if (p->tcp_header && !getAppIdFlag(session, APPID_SESSION_OOO))
    {
        if ((p->flags & FLAG_STREAM_ORDER_BAD) ||
            (p->payload_size && !(p->flags & (FLAG_STREAM_ORDER_OK | FLAG_RETRANSMIT | FLAG_REBUILT_STREAM))))
        {
            setAppIdFlag(session, APPID_SESSION_OOO | APPID_SESSION_OOO_CHECK_TP);
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s Packet out-of-order, %s%sflow\n", app_id_debug_session,
                    (p->flags & FLAG_STREAM_ORDER_BAD)? "bad ":"not-ok ",
                    getAppIdFlag(session, APPID_SESSION_MID)? "mid-stream ":"");

            /* Shut off service/client discoveries, since they skip not-ok data packets and
               may keep failing on subsequent data packets causing performance degradation. */
            if (!getAppIdFlag(session, APPID_SESSION_MID) || (p->src_port != 21 && p->dst_port != 21)) // exception for ftp-control
            {
                session->rnaServiceState = RNA_STATE_FINISHED;
                session->rnaClientState = RNA_STATE_FINISHED;
                if ((TPIsAppIdAvailable(session->tpsession) || getAppIdFlag(session, APPID_SESSION_NO_TPI)) &&
                    session->payloadAppId == APP_ID_NONE)
                    session->payloadAppId = APP_ID_UNKNOWN;
                setAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_CLIENT_DETECTED);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s stopped service/client discovery\n", app_id_debug_session);
            }
        }
        else
        {
            if ((p->tcp_header->flags & TCPHEADER_RST) && session->previous_tcp_flags == TCPHEADER_SYN)
            {
                AppIdServiceIDState *id_state;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                uint16_t asId;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                uint32_t cid = 0;
#endif
                setAppIdFlag(session, APPID_SESSION_SYN_RST);
                if (sfaddr_is_set(&session->service_ip))
                {
                    ip = &session->service_ip;
                    port = session->service_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                    asId = session->serviceAsId; 
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                    cid = session->carrierId;
#endif
                }
                else
                {
                    ip = GET_SRC_IP(p);
                    port = p->src_port;
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                    asId = p->pkt_header->address_space_id_src;
#endif
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
                    cid = GET_SFOUTER_IPH_PROTOID(p, pkt_header);
#endif
                }
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID) 
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                id_state = AppIdGetServiceIDState(ip, IPPROTO_TCP, port, 
                                                  AppIdServiceDetectionLevel(session), asId, cid);
#else
                id_state = AppIdGetServiceIDState(ip, IPPROTO_TCP, port, AppIdServiceDetectionLevel(session), cid);
#endif  
#else /* No Carrierid support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                id_state = AppIdGetServiceIDState(ip, IPPROTO_TCP, port,
                                                  AppIdServiceDetectionLevel(session), asId);
#else
                id_state = AppIdGetServiceIDState(ip, IPPROTO_TCP, port, AppIdServiceDetectionLevel(session));
#endif
#endif
                if (id_state)
                {
                    if (!id_state->reset_time)
                        id_state->reset_time = GetPacketRealTime;
                    else if ((GetPacketRealTime - id_state->reset_time) >= 60)
                    {
#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                        AppIdRemoveServiceIDState(ip, IPPROTO_TCP, port, 
                                                  AppIdServiceDetectionLevel(session),
                                                  asId, cid);
#else
                        AppIdRemoveServiceIDState(ip, IPPROTO_TCP, port, AppIdServiceDetectionLevel(session), cid);
#endif
#else /* No carrier id support */
#if !defined(SFLINUX) && defined(DAQ_CAPA_VRF)
                        AppIdRemoveServiceIDState(ip, IPPROTO_TCP, port,
                                                  AppIdServiceDetectionLevel(session),
                                                  asId);
#else

                        AppIdRemoveServiceIDState(ip, IPPROTO_TCP, port, AppIdServiceDetectionLevel(session));
#endif
#endif
                        setAppIdFlag(session, APPID_SESSION_SERVICE_DELETED);
                    }
                }
            }
            session->previous_tcp_flags = p->tcp_header->flags;
        }
    }

    checkRestartAppDetection(session);

    if (outer_protocol)
    {
        session->miscAppId = pConfig->ip_protocol[outer_protocol];
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s outer protocol service %d\n", app_id_debug_session, session->miscAppId);
    }

    if (protocol != IPPROTO_TCP && protocol != IPPROTO_UDP)
    {
        if (!getAppIdFlag(session, APPID_SESSION_PORT_SERVICE_DONE))
        {
            session->serviceAppId = session->portServiceAppId = getProtocolServiceId(protocol, pConfig);
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s protocol service %d\n", app_id_debug_session, session->portServiceAppId);
            setAppIdFlag(session, APPID_SESSION_PORT_SERVICE_DONE);
            session->rnaServiceState = RNA_STATE_FINISHED;
            _dpd.streamAPI->set_application_id(p->stream_session, session->serviceAppId, clientAppId, payloadAppId, session->miscAppId);
        }
        return;
    }

    if (session->tpAppId == APP_ID_SSH && session->payloadAppId != APP_ID_SFTP && session->session_packet_count >= MIN_SFTP_PACKET_COUNT && session->session_packet_count < MAX_SFTP_PACKET_COUNT)
    {
        if (GET_IPH_TOS(p) == 8)
        {
            session->payloadAppId = APP_ID_SFTP;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s payload is SFTP\n", app_id_debug_session);
        }
    }

    tpAppId = processThirdParty(p, session, direction, protocol, &isTpAppidDiscoveryDone, pConfig);

    if (!getAppIdFlag(session, APPID_SESSION_PORT_SERVICE_DONE))
    {
        switch (protocol)
        {
        case IPPROTO_TCP:
            // TCP-specific checks. No SYN/RST, and no SYN/ACK
            // we have to check for SYN/ACK here explicitly in case of TCP Fast Open
            if (getAppIdFlag(session, APPID_SESSION_SYN_RST) ||
                (p->tcp_header && (p->tcp_header->flags & TCPHEADER_SYN) &&
                (p->tcp_header->flags & TCPHEADER_ACK)))
                break;
            // fall through to next test
        case IPPROTO_UDP:
            // Both TCP and UDP need these tests to be made
            // packet must be from responder, and with a non-zero payload size
            // For all other cases the port parameter is never checked.
            if (p->payload_size < 1 || direction != APP_ID_FROM_RESPONDER)
                break;
            // fall through to all other cases
        default:
            {
                session->portServiceAppId = getPortServiceId(protocol, p->src_port, pConfig);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s port service %d\n", app_id_debug_session, session->portServiceAppId);
                setAppIdFlag(session, APPID_SESSION_PORT_SERVICE_DONE);
            }
            break;
        }
    }

    /* Length-based detectors. */
    /* Only check if:
     *  - Port service didn't find anything (and we haven't yet either).
     *  - We haven't hit the max packets allowed for detector sequence matches.
     *  - Packet has in-order data (we'll ignore 0-sized packets in sequencing). */
    if (   (p->payload_size > 0)
        && (session->portServiceAppId <= APP_ID_NONE)
        && (session->length_sequence.sequence_cnt < LENGTH_SEQUENCE_CNT_MAX)
        && !getAppIdFlag(session, APPID_SESSION_OOO))
    {
        uint8_t index = session->length_sequence.sequence_cnt;
        session->length_sequence.proto = protocol;
        session->length_sequence.sequence_cnt++;
        session->length_sequence.sequence[index].direction = direction;
        session->length_sequence.sequence[index].length    = p->payload_size;
        session->portServiceAppId = lengthAppCacheFind(&session->length_sequence, pConfig);
        if (session->portServiceAppId > APP_ID_NONE)
        {
            setAppIdFlag(session, APPID_SESSION_PORT_SERVICE_DONE);
        }
    }

    /* exceptions for rexec and any other service detector that needs to see SYN and SYN/ACK */
    if (getAppIdFlag(session, APPID_SESSION_REXEC_STDERR))
    {
        AppIdDiscoverService(p, direction, session, pConfig);

        if (getAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_CONTINUE) ==
            APPID_SESSION_SERVICE_DETECTED)
        {
            session->rnaServiceState = RNA_STATE_FINISHED;
            if (session->payloadAppId == APP_ID_NONE)
                session->payloadAppId = APP_ID_UNKNOWN;
        }
    }

    else if (protocol != IPPROTO_TCP || (p->flags & FLAG_STREAM_ORDER_OK) || getAppIdFlag(session, APPID_SESSION_MID))
    {
        /*** Start of service discovery. ***/
        if (session->rnaServiceState != RNA_STATE_FINISHED)
        {
            PROFILE_VARS;
            RNA_INSPECTION_STATE prevRnaServiceState;
            PREPROC_PROFILE_START(serviceMatchPerfStats);

            tpAppId = session->tpAppId;
            prevRnaServiceState = session->rnaServiceState;

            //decision to directly call validator or go through elaborate service_state tracking
            //is made once at the beginning of session.
            if (session->rnaServiceState == RNA_STATE_NONE && p->payload_size)
            {
                if (getAppIdFlag(session, APPID_SESSION_MID))
                {
                    // Unless it could be ftp control
                    if (protocol == IPPROTO_TCP && (p->src_port == 21 || p->dst_port == 21) &&
                            !(p->tcp_header->flags & (TCPHEADER_FIN | TCPHEADER_RST)))
                    {
                        setAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED | APPID_SESSION_NOT_A_SERVICE | APPID_SESSION_SERVICE_DETECTED);
                        if (!AddFTPServiceState(session))
                        {
                            setAppIdFlag(session, APPID_SESSION_CONTINUE);
                            if (p->dst_port != 21)
                                setAppIdFlag(session, APPID_SESSION_RESPONDER_SEEN);
                        }
                        session->rnaServiceState = RNA_STATE_STATEFUL;
                    }
                    else
                    {
                        setAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED);
                        session->rnaServiceState = RNA_STATE_FINISHED;
                        if ((TPIsAppIdAvailable(session->tpsession) || getAppIdFlag(session, APPID_SESSION_NO_TPI)) &&
                            session->payloadAppId == APP_ID_NONE)
                            session->payloadAppId = APP_ID_UNKNOWN;
                    }
                }
                else if (TPIsAppIdAvailable(session->tpsession))
                {
                    if (tpAppId > APP_ID_NONE)
                    {
                        //tp has positively identified appId, Dig deeper only if sourcefire detector
                        //identifies additional information or flow is UDP reveresed.
#ifdef TARGET_BASED
                        if ((entry = appInfoEntryGet(tpAppId, pConfig))
                                && entry->svrValidator &&
                                ((entry->flags & APPINFO_FLAG_SERVICE_ADDITIONAL) ||
                                 ((entry->flags & APPINFO_FLAG_SERVICE_UDP_REVERSED) && protocol == IPPROTO_UDP &&
                                  getAppIdFlag(session, APPID_SESSION_INITIATOR_MONITORED | APPID_SESSION_RESPONDER_MONITORED))))
                        {
                            AppIdFlowdataDeleteAllByMask(session, APPID_SESSION_DATA_SERVICE_MODSTATE_BIT);

#ifdef DEBUG_FW_APPID
                            if (session->serviceData && compareServiceElements(session->serviceData, entry->svrValidator))
                            {
                                fprintf(stderr, "Mismatched validator Original %s, new tp %s",
                                        session->serviceData->name, entry->svrValidator->name);
                            }
#endif
                            session->serviceData = entry->svrValidator;
                            session->rnaServiceState = RNA_STATE_STATEFUL;
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                            if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                                fprintf(SF_DEBUG_FILE, "%u -> %u %u RNA doing deeper inspection\n",
                                        (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol);
#endif
                        }
                        else
                        {
                            stopRnaServiceInspection(p, session, direction);
                        }
#endif
                    }
                    else
                        session->rnaServiceState = RNA_STATE_STATEFUL;
                }
                else
                    session->rnaServiceState = RNA_STATE_STATEFUL;
            }

            //stop rna inspection as soon as tp has classified a valid AppId later in the session
            if (session->rnaServiceState == RNA_STATE_STATEFUL &&
                prevRnaServiceState == RNA_STATE_STATEFUL &&
                !getAppIdFlag(session, APPID_SESSION_NO_TPI) &&
                TPIsAppIdAvailable(session->tpsession) &&
                tpAppId > APP_ID_NONE  && tpAppId < SF_APPID_MAX)
            {
#ifdef TARGET_BASED
                entry = appInfoEntryGet(tpAppId, pConfig);

                if (entry && entry->svrValidator && !(entry->flags & APPINFO_FLAG_SERVICE_ADDITIONAL))
                {
                    if (app_id_debug_session_flag)
                        _dpd.logMsg("AppIdDbg %s Stopping service detection\n", app_id_debug_session);
                    stopRnaServiceInspection(p, session, direction);
                }
#endif
            }

            // Check to see if we want to stop any detectors for SIP/RTP.
            if (session->rnaServiceState != RNA_STATE_FINISHED)
            {
                if (tpAppId == APP_ID_SIP)
                {
                    // TP needs to see its own future flows and does a better
                    // job of it than we do, so stay out of its way, and don't
                    // waste time (but we will still get the Snort callbacks
                    // for any of our own future flows).
                    //  - Shut down our detectors.
                    session->serviceAppId = APP_ID_SIP;
                    stopRnaServiceInspection(p, session, direction);
                    session->rnaClientState = RNA_STATE_FINISHED;
                }
                else if ((tpAppId == APP_ID_RTP) || (tpAppId == APP_ID_RTP_AUDIO) || (tpAppId == APP_ID_RTP_VIDEO))
                {
                    // No need for anybody to keep wasting time once we've
                    // found RTP.
                    //  - Shut down our detectors.
                    session->serviceAppId = tpAppId;
                    stopRnaServiceInspection(p, session, direction);
                    session->rnaClientState = RNA_STATE_FINISHED;
                    //  - Shut down TP.
                    thirdparty_appid_module->session_state_set(session->tpsession, TP_STATE_TERMINATED);
                    //  - Just ignore everything from now on.
                    setAppIdFlag(session, APPID_SESSION_IGNORE_FLOW);
                }
            }

            if (session->rnaServiceState == RNA_STATE_STATEFUL)
            {
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "%u -> %u %u RNA identifying service\n",
                            (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol);
#endif
                AppIdDiscoverService(p, direction, session, pConfig);
                isTpAppidDiscoveryDone = true;
                //to stop executing validator after service has been detected by RNA.
                if (getAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_CONTINUE) == APPID_SESSION_SERVICE_DETECTED)
		{
                    session->rnaServiceState = RNA_STATE_FINISHED;
                    if ((TPIsAppIdAvailable(session->tpsession) || getAppIdFlag(session, APPID_SESSION_NO_TPI)) &&
                        session->payloadAppId == APP_ID_NONE)
                        session->payloadAppId = APP_ID_UNKNOWN;
		}
                if (session->rnaServiceState == RNA_STATE_STATEFUL && session->serviceAppId == APP_ID_NONE && svcTakingTooMuchTime(session))
                {
                    stopRnaServiceInspection(p, session, direction);
                    session->serviceAppId = APP_ID_UNKNOWN;
                }
                else if(session->serviceAppId == APP_ID_DNS && appidStaticConfig->dns_host_reporting && session->dsession && session->dsession->host  )
                {
                    size = session->dsession->host_len;
                    dns_host_scan_hostname((const u_int8_t *)session->dsession->host , size, &clientAppId, &payloadAppId, &pConfig->serviceDnsConfig);
                    setClientAppIdData(p, direction, session, clientAppId, NULL);
                }
                else if (session->serviceAppId == APP_ID_RTMP)
                    ExamineRtmpMetadata(p, direction, session);
                else if (getAppIdFlag(session, APPID_SESSION_SSL_SESSION) && session->tsession)
                    ExamineSslMetadata(p, direction, session, pConfig);

#ifdef TARGET_BASED
                if (tpAppId <= APP_ID_NONE &&
                    getAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_NOT_A_SERVICE | APPID_SESSION_IGNORE_HOST) == APPID_SESSION_SERVICE_DETECTED)
                {
                    synchAppIdWithSnortId(session->serviceAppId, p, session, pConfig);
                }
#endif
            }
            PREPROC_PROFILE_END(serviceMatchPerfStats);
        }
        /*** End of service discovery. ***/

        /*** Start of client discovery. ***/
        if (session->rnaClientState != RNA_STATE_FINISHED)
        {
            PROFILE_VARS;
            PREPROC_PROFILE_START(clientMatchPerfStats);
            RNA_INSPECTION_STATE prevRnaClientState = session->rnaClientState;
            bool was_http2 = session->is_http2;
#ifdef TARGET_BASED
            bool was_service = getAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED) ? true : false;
#endif

            //decision to directly call validator or go through elaborate service_state tracking
            //is made once at the beginning of session.
            if (session->rnaClientState == RNA_STATE_NONE && p->payload_size && direction == APP_ID_FROM_INITIATOR)
            {
                if (getAppIdFlag(session, APPID_SESSION_MID))
                    session->rnaClientState = RNA_STATE_FINISHED;
                else if (TPIsAppIdAvailable(session->tpsession) && (tpAppId = session->tpAppId) > APP_ID_NONE && tpAppId < SF_APPID_MAX)
                {
#ifdef TARGET_BASED
                    if ((entry = appInfoEntryGet(tpAppId, pConfig)) && entry->clntValidator &&
                            ((entry->flags & APPINFO_FLAG_CLIENT_ADDITIONAL) ||
                             ((entry->flags & APPINFO_FLAG_CLIENT_USER) &&
                              getAppIdFlag(session, APPID_SESSION_DISCOVER_USER))))
                    {
                        //tp has positively identified appId, Dig deeper only if sourcefire detector
                        //identifies additional information
                        session->clientData = entry->clntValidator;
                        session->rnaClientState = RNA_STATE_DIRECT;
                    }
                    else
                    {
                        setAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED);
                        session->rnaClientState = RNA_STATE_FINISHED;
                    }
#endif
                }
                else if (getAppIdFlag(session, APPID_SESSION_HTTP_SESSION))
                    session->rnaClientState = RNA_STATE_FINISHED;
                else
                    session->rnaClientState = RNA_STATE_STATEFUL;
            }

            //stop rna inspection as soon as tp has classified a valid AppId later in the session
            if ((session->rnaClientState == RNA_STATE_STATEFUL ||
                 session->rnaClientState == RNA_STATE_DIRECT) &&
                session->rnaClientState == prevRnaClientState &&
                !getAppIdFlag(session, APPID_SESSION_NO_TPI) &&
                TPIsAppIdAvailable(session->tpsession) &&
                tpAppId > APP_ID_NONE  && tpAppId < SF_APPID_MAX)
            {
#ifdef TARGET_BASED
                entry = appInfoEntryGet(tpAppId, pConfig);

                if (!(entry && entry->clntValidator && entry->clntValidator == session->clientData && (entry->flags & (APPINFO_FLAG_CLIENT_ADDITIONAL|APPINFO_FLAG_CLIENT_USER))))
                {
                    session->rnaClientState = RNA_STATE_FINISHED;
                    setAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED);
                }
#endif
            }

            if (session->rnaClientState == RNA_STATE_DIRECT)
            {
                int ret = CLIENT_APP_INPROCESS;

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "%u -> %u %u RNA identifying additional client info\n",
                            (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol);
#endif
                if (direction == APP_ID_FROM_INITIATOR)
                {
                    /* get out if we've already tried to validate a client app */
                    if (!getAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED))
                    {
                        ret = RunClientDetectors(session, p, direction, pConfig);
                    }
                }
                else if (session->rnaServiceState != RNA_STATE_STATEFUL &&
                         getAppIdFlag(session, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS))
                {
                    ret = RunClientDetectors(session, p, direction, pConfig);
                }

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "%u -> %u %u direct client validate returned %d\n",
                            (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, ret);
#endif
                switch (ret)
                {
                    case CLIENT_APP_INPROCESS:
                        break;
                    default:
                        session->rnaClientState = RNA_STATE_FINISHED;
                        break;
                }
            }
            else if (session->rnaClientState == RNA_STATE_STATEFUL)
            {
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
                    fprintf(SF_DEBUG_FILE, "%u -> %u %u RNA identifying client\n",
                            (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol);
#endif
                AppIdDiscoverClientApp(p, direction, session, pConfig);
                isTpAppidDiscoveryDone = true;
                if (session->candidate_client_list != NULL)
                {
                    if (sflist_count(session->candidate_client_list) > 0)
                    {
                        int ret = 0;
                        if (direction == APP_ID_FROM_INITIATOR)
                        {
                            /* get out if we've already tried to validate a client app */
                            if (!getAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED))
                            {
                                ret = RunClientDetectors(session, p, direction, pConfig);
                            }
                        }
                        else if (session->rnaServiceState != RNA_STATE_STATEFUL &&
                                 getAppIdFlag(session, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS))
                        {
                            ret = RunClientDetectors(session, p, direction, pConfig);
                        }
                        if (ret < 0)
                        {
                            setAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED);
                            session->rnaClientState = RNA_STATE_FINISHED;
                        }
                    }
                    else
                    {
                        setAppIdFlag(session, APPID_SESSION_CLIENT_DETECTED);
                        session->rnaClientState = RNA_STATE_FINISHED;
                    }
                }
            }
            if (app_id_debug_session_flag)
                if (!was_http2 && session->is_http2)
                    _dpd.logMsg("AppIdDbg %s Got a preface for HTTP/2\n", app_id_debug_session);
#ifdef TARGET_BASED
            if (!was_service && getAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED))
                synchAppIdWithSnortId(session->serviceAppId, p, session, pConfig);
#endif
            PREPROC_PROFILE_END(clientMatchPerfStats);
        }
        /*** End of client discovery. ***/

        setAppIdFlag(session, APPID_SESSION_ADDITIONAL_PACKET);
    }
    else
    {
#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
                if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
        fprintf(SF_DEBUG_FILE, "Packet not okay\n");
#endif
    }

    serviceAppId = fwPickServiceAppId(session);
    payloadAppId = fwPickPayloadAppId(session);

    if (serviceAppId > APP_ID_NONE)
    {
        if (getAppIdFlag(session, APPID_SESSION_DECRYPTED))
        {
            if (session->miscAppId == APP_ID_NONE)
                updateEncryptedAppId(session, serviceAppId);
        }
        else if (isTpAppidDiscoveryDone && isSslServiceAppId(serviceAppId) && _dpd.isSSLPolicyEnabled(NULL))
            setAppIdFlag(session, APPID_SESSION_CONTINUE);
    }

    clientAppId = fwPickClientAppId(session);
    miscAppId = fwPickMiscAppId(session);

    if (!getAppIdFlag(session, APPID_SESSION_HOST_CACHE_MATCHED))
    {
        bool isHttpTunnel = (payloadAppId == APP_ID_HTTP_TUNNEL || payloadAppId == APP_ID_HTTP_SSL_TUNNEL) ? true : false;
        if(isCheckHostCacheValid(session, serviceAppId, clientAppId, payloadAppId, miscAppId) || isHttpTunnel)
        {
            bool isCheckHostCache =  true;
            sfaddr_t *srv_ip;
            uint16_t srv_port;

            if (isHttpTunnel)
            {
                if (session->hsession)
                {
                    if (session->scan_flags & SCAN_HTTP_URI_FLAG)
                    {
                        if (session->hsession->tunDest)
                        {
                            _dpd.snortFree(session->hsession->tunDest, sizeof(tunnelDest), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
                            session->hsession->tunDest = NULL;
                        }
                        getIpPortFromHttpTunnel(session->hsession->uri, session->hsession->uri_buflen, &session->hsession->tunDest);
                        session->scan_flags &= ~SCAN_HTTP_URI_FLAG;
                    }

                    if (session->hsession->tunDest)
                    {
                        srv_ip = &(session->hsession->tunDest->ip);
                        srv_port = session->hsession->tunDest->port;
                    }
                    else
                    {
                        isCheckHostCache =  false;
                    }
                }
                else
                {
                    isCheckHostCache =  false;
                }
            }
            else
            {
                if (direction == APP_ID_FROM_INITIATOR)
                {
                    srv_ip = GET_DST_IP(p);
                    srv_port = p->dst_port;
                }
                else
                {
                    srv_ip = GET_SRC_IP(p);
                    srv_port = p->src_port;
                }
            }

            if (isCheckHostCache && checkHostCache(p, session, srv_ip, srv_port, protocol, pConfig))
            {
                session->portServiceAppId = APP_ID_NONE; // overriding the portServiceAppId in case of a cache hit
                serviceAppId = pickServiceAppId(session);
                payloadAppId = pickPayloadId(session);
                clientAppId = pickClientAppId(session);
            }
        }
    }

    /* For OOO flow, disable third party if checkHostCache is done and appid is found.
       Not sure if it is safe to set APPID_SESSION_IGNORE_FLOW here. */
    if ((getAppIdFlag(session, APPID_SESSION_OOO_CHECK_TP | APPID_SESSION_HOST_CACHE_MATCHED) ==
            (APPID_SESSION_OOO_CHECK_TP | APPID_SESSION_HOST_CACHE_MATCHED)) &&
        (serviceAppId || payloadAppId || clientAppId) && thirdparty_appid_module)
    {
        clearAppIdFlag(session, APPID_SESSION_OOO_CHECK_TP); // don't repeat this block
        if (!TPIsAppIdDone(session->tpsession))
        {
            thirdparty_appid_module->session_state_set(session->tpsession, TP_STATE_TERMINATED);
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s stopped third party detection\n", app_id_debug_session);
        }
    }

    _dpd.streamAPI->set_application_id(p->stream_session, serviceAppId, clientAppId, payloadAppId, miscAppId);

    /* Set the field that the Firewall queries to see if we have a search engine. */
    if (session->search_support_type == SEARCH_SUPPORT_TYPE_UNKNOWN && payloadAppId > APP_ID_NONE)
    {
        uint flags = appInfoEntryFlagGet(payloadAppId, APPINFO_FLAG_SEARCH_ENGINE | APPINFO_FLAG_SUPPORTED_SEARCH, pConfig);
        session->search_support_type =
            (flags & APPINFO_FLAG_SEARCH_ENGINE) ?
                ((flags & APPINFO_FLAG_SUPPORTED_SEARCH) ? SUPPORTED_SEARCH_ENGINE : UNSUPPORTED_SEARCH_ENGINE )
                : NOT_A_SEARCH_ENGINE;
        if (app_id_debug_session_flag)
        {
            char *typeString;
            switch (session->search_support_type)
            {
                case NOT_A_SEARCH_ENGINE: typeString = "NOT_A_SEARCH_ENGINE"; break;
                case SUPPORTED_SEARCH_ENGINE: typeString = "SUPPORTED_SEARCH_ENGINE"; break;
                case UNSUPPORTED_SEARCH_ENGINE: typeString = "UNSUPPORTED_SEARCH_ENGINE"; break;
                default: break;
            }
            _dpd.logMsg("AppIdDbg %s appId: %u (safe)search_support_type=%s\n", app_id_debug_session, payloadAppId, typeString);
        }
    }

    if (serviceAppId > APP_ID_NONE)
    {
        if (session->pastIndicator != payloadAppId && payloadAppId > APP_ID_NONE)
        {
            session->pastIndicator = payloadAppId;
            checkSessionForAFIndicator(p, direction, pConfig, payloadAppId);
        }

        if (session->pastForecast != serviceAppId && session->payloadAppId == APP_ID_NONE && session->pastForecast != APP_ID_UNKNOWN)
        {
            session->pastForecast = checkSessionForAFForecast(session, p, direction, pConfig, serviceAppId);
        }
    }

    if (*_dpd.pkt_tracer_enabled)
    {
        const char *serviceName = appGetAppName(serviceAppId);
        const char *appName = appGetAppName(payloadAppId);
        _dpd.addPktTrace(VERDICT_REASON_APPID, snprintf(_dpd.trace, _dpd.traceMax, "AppID: service %s (%d), application %s (%d)%s\n",
            serviceName? serviceName : "unknown", serviceAppId, appName? appName : "unknown", payloadAppId,
            getAppIdFlag(session, APPID_SESSION_OOO)? ", out-of-order":""));
    }

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
    {
#endif
        fprintf(SF_DEBUG_FILE, "%u %u -> %u %u End %d %u - (%d %d %d %d %d) %u %" PRIx64 " %u %u %u\n", (unsigned)session->session_packet_count, (unsigned)p->src_port, (unsigned)p->dst_port,
                (unsigned)protocol, direction, (unsigned)p->payload_size,
                session->serviceAppId, session->clientAppId, session->payloadAppId,
                session->tpAppId, session->miscAppId, session->rnaServiceState, session->common.flags, thirdparty_appid_module->session_state_get(session->tpsession),
                (unsigned)session->init_tpPackets, (unsigned)session->resp_tpPackets);
        //DumpHex(SF_DEBUG_FILE, p->payload, p->payload_size);
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    }
#endif
#endif
}

STATIC INLINE void pickHttpXffAddress(SFSnortPacket* p, tAppIdData* appIdSession, ThirdPartyAppIDAttributeData* attribute_data)
{
    int i;
    static char* defaultXffPrecedence[] = {HTTP_XFF_FIELD_X_FORWARDED_FOR, HTTP_XFF_FIELD_TRUE_CLIENT_IP};

    // XFF precedence configuration cannot change for a session. Do not get it again if we already got it.
    if (!appIdSession->hsession->xffPrecedence)
    {
        char** xffPrecedence = _dpd.sessionAPI->get_http_xff_precedence(p->stream_session, p->flags, &appIdSession->hsession->numXffFields);
        int j;

        if (!xffPrecedence)
        {
            xffPrecedence = defaultXffPrecedence;
            appIdSession->hsession->numXffFields = sizeof(defaultXffPrecedence) / sizeof(defaultXffPrecedence[0]);
        }

        appIdSession->hsession->xffPrecedence = _dpd.snortAlloc(appIdSession->hsession->numXffFields,
                sizeof(char*), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
        if(!appIdSession->hsession->xffPrecedence)
        {
            DynamicPreprocessorFatalMessage("pickHttpXffAddress: "
                    "failed to allocate memory for xffPrecedence in appIdSession\n");
        }

        for (j = 0; j < appIdSession->hsession->numXffFields; j++)
            appIdSession->hsession->xffPrecedence[j] = strndup(xffPrecedence[j], UINT8_MAX);
    }

    if (app_id_debug_session_flag)
    {
        for (i = 0; i < attribute_data->numXffFields; i++)
            _dpd.logMsg("AppIdDbg %s %s : %s\n", app_id_debug_session, attribute_data->xffFieldValue[i].field,
                        attribute_data->xffFieldValue[i].value ? attribute_data->xffFieldValue[i].value : "(empty)");
    }

    // xffPrecedence array is sorted based on precedence
    for (i = 0; (i < appIdSession->hsession->numXffFields) && appIdSession->hsession->xffPrecedence[i]; i++)
    {
        int j;
        for (j = 0; j < attribute_data->numXffFields; j++)
        {
            if (appIdSession->hsession->xffAddr)
            {
                sfaddr_free(appIdSession->hsession->xffAddr);
                appIdSession->hsession->xffAddr = NULL;
            }

            if (strncasecmp(attribute_data->xffFieldValue[j].field, appIdSession->hsession->xffPrecedence[i], UINT8_MAX) == 0)
            {
                if (!attribute_data->xffFieldValue[j].value || (attribute_data->xffFieldValue[j].value[0] == '\0')) return;

                char* tmp = strrchr(attribute_data->xffFieldValue[j].value, ',');
                SFIP_RET status;

                if (!tmp)
                {
                    appIdSession->hsession->xffAddr = sfaddr_alloc(attribute_data->xffFieldValue[j].value, &status);
                }
                // For a comma-separated list of addresses, pick the last address
                else
                {
                    appIdSession->hsession->xffAddr = sfaddr_alloc(tmp + 1, &status);
                }
                break;
            }
        }
        if (appIdSession->hsession->xffAddr) break;
    }
}

static inline void ProcessThirdPartyResults(SFSnortPacket* p, APPID_SESSION_DIRECTION direction, tAppIdData* appIdSession, int confidence, tAppId* proto_list, ThirdPartyAppIDAttributeData* attribute_data)
{
    int size;
    tAppId serviceAppId = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    tAppId referredPayloadAppId = 0;
    tAppIdConfig *pConfig = appIdActiveConfigGet();
#ifdef TARGET_BASED
    AppInfoTableEntry *entry = NULL;
#endif

    if (!appIdSession->payloadAppId && ThirdPartyAppIDFoundProto(APP_ID_EXCHANGE, proto_list))
        appIdSession->payloadAppId = APP_ID_EXCHANGE;

    if (ThirdPartyAppIDFoundProto(APP_ID_HTTP, proto_list))
    {
        setAppIdFlag(appIdSession, APPID_SESSION_HTTP_SESSION);
    }
    if (ThirdPartyAppIDFoundProto(APP_ID_SPDY, proto_list))
    {
        setAppIdFlag(appIdSession, APPID_SESSION_HTTP_SESSION | APPID_SESSION_SPDY_SESSION);
    }
    if (ThirdPartyAppIDFoundProto(APP_ID_SSL, proto_list))
    {
        if (getAppIdFlag(appIdSession, APPID_SESSION_HTTP_TUNNEL))
        {
            if (!appIdSession->serviceData)
            {
#ifdef TARGET_BASED
                entry = appInfoEntryGet(APP_ID_SSL, pConfig);
                appIdSession->serviceData = entry->svrValidator;
#endif
            }
            if (getAppIdFlag(appIdSession, APPID_SESSION_HTTP_SESSION | APPID_SESSION_SPDY_SESSION))
                clearAppIdFlag(appIdSession, APPID_SESSION_HTTP_SESSION | APPID_SESSION_SPDY_SESSION);
        }
        setAppIdFlag(appIdSession, APPID_SESSION_SSL_SESSION);
    }

    if (getAppIdFlag(appIdSession, APPID_SESSION_HTTP_SESSION))
    {
        if (!appIdSession->hsession)
        {
            if (!(appIdSession->hsession = _dpd.snortAlloc(1, sizeof(*appIdSession->hsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                DynamicPreprocessorFatalMessage("Could not allocate httpSession data");
            memset(ptype_scan_counts, 0, 7 * sizeof(ptype_scan_counts[0]));
        }

        if (getAppIdFlag(appIdSession, APPID_SESSION_SPDY_SESSION))
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s flow is SPDY\n", app_id_debug_session);

            if (attribute_data->spdyRequestScheme &&
                attribute_data->spdyRequestHost &&
                attribute_data->spdyRequestPath)
            {
                static const char httpsScheme[] = "https";
                static const char httpScheme[] = "http";
                const char *scheme;

                if (appIdSession->hsession->url)
                {
                    free(appIdSession->hsession->url);
                    appIdSession->hsession->chp_finished = 0;
                }
                if (getAppIdFlag(appIdSession, APPID_SESSION_DECRYPTED)
                        && memcmp(attribute_data->spdyRequestScheme, httpScheme, sizeof(httpScheme)-1) == 0)
                {
                    scheme = httpsScheme;
                }
                else
                {
                    scheme = attribute_data->spdyRequestScheme;
                }

                size = strlen(scheme) +
                       strlen(attribute_data->spdyRequestHost) +
                       strlen(attribute_data->spdyRequestPath) +
                       sizeof("://"); // see sprintf() format
                if (NULL != (appIdSession->hsession->url = malloc(size)))
                {
                    sprintf(appIdSession->hsession->url, "%s://%s%s",
                         scheme, attribute_data->spdyRequestHost,
                         attribute_data->spdyRequestPath);
                    appIdSession->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
                }
                free(attribute_data->spdyRequestScheme);
                attribute_data->spdyRequestScheme = NULL;
            }
            else if (attribute_data->spdyRequestScheme)
            {
                free(attribute_data->spdyRequestScheme);
                attribute_data->spdyRequestScheme = NULL;
            }
            if (attribute_data->spdyRequestHost)
            {
                if (appIdSession->hsession->host)
                {
                    free(appIdSession->hsession->host);
                    appIdSession->hsession->chp_finished = 0;
                }
                appIdSession->hsession->host = attribute_data->spdyRequestHost;
                attribute_data->spdyRequestHost = NULL;
                appIdSession->hsession->fieldOffset[REQ_HOST_FID] = attribute_data->spdyRequestHostOffset;
                appIdSession->hsession->fieldEndOffset[REQ_HOST_FID] = attribute_data->spdyRequestHostEndOffset;
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s SPDY Host (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_HOST_FID], appIdSession->hsession->fieldEndOffset[REQ_HOST_FID], appIdSession->hsession->host);
                appIdSession->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
            }
            if (attribute_data->spdyRequestPath)
            {
                if (appIdSession->hsession->uri)
                {
                    free(appIdSession->hsession->uri);
                    appIdSession->hsession->chp_finished = 0;
                }
                appIdSession->hsession->uri = attribute_data->spdyRequestPath;
                attribute_data->spdyRequestPath = NULL;
                appIdSession->hsession->fieldOffset[REQ_URI_FID] = attribute_data->spdyRequestPathOffset;
                appIdSession->hsession->fieldEndOffset[REQ_URI_FID] = attribute_data->spdyRequestPathEndOffset;
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s SPDY URI (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_URI_FID], appIdSession->hsession->fieldEndOffset[REQ_URI_FID], appIdSession->hsession->uri);
            }
        }
        else
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s flow is HTTP\n", app_id_debug_session);

            if (attribute_data->httpRequestHost)
            {
                if (appIdSession->hsession->host)
                {
                    free(appIdSession->hsession->host);
                    if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                        appIdSession->hsession->chp_finished = 0;
                }
                appIdSession->hsession->host = attribute_data->httpRequestHost;
                appIdSession->hsession->host_buflen = attribute_data->httpRequestHostLen;
                appIdSession->hsession->fieldOffset[REQ_HOST_FID] = attribute_data->httpRequestHostOffset;
                appIdSession->hsession->fieldEndOffset[REQ_HOST_FID] = attribute_data->httpRequestHostEndOffset;
                attribute_data->httpRequestHost = NULL;
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s HTTP Host (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_HOST_FID], appIdSession->hsession->fieldEndOffset[REQ_HOST_FID], appIdSession->hsession->host);
                appIdSession->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
            }
            if (attribute_data->httpRequestUrl)
            {
                static const char httpScheme[] = "http://";

                if (appIdSession->hsession->url)
                {
                    free(appIdSession->hsession->url);
                    if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                        appIdSession->hsession->chp_finished = 0;
                }

                //change http to https if session was decrypted.
                if (getAppIdFlag(appIdSession, APPID_SESSION_DECRYPTED)
                        && memcmp(attribute_data->httpRequestUrl, httpScheme, sizeof(httpScheme)-1) == 0)
                {
                    appIdSession->hsession->url = malloc(strlen(attribute_data->httpRequestUrl) + 2);
                    if(!appIdSession->hsession->url)
                    {
                         DynamicPreprocessorFatalMessage("ProcessThirdPartyResults: "
                                 "Failed to allocate URL memory in AppID session\n");
                    }

                    if (appIdSession->hsession->url)
                        sprintf(appIdSession->hsession->url, "https://%s", attribute_data->httpRequestUrl + sizeof(httpScheme)-1);

                    free(attribute_data->httpRequestUrl);
                    attribute_data->httpRequestUrl = NULL;
                }
                else
                {
                    appIdSession->hsession->url = attribute_data->httpRequestUrl;
                    attribute_data->httpRequestUrl = NULL;
                }

                appIdSession->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
            }
            if (attribute_data->httpRequestUri)
            {
                if (appIdSession->hsession->uri)
                {
                    free(appIdSession->hsession->uri);
                    if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                        appIdSession->hsession->chp_finished = 0;
                }
                appIdSession->hsession->uri = attribute_data->httpRequestUri;
                appIdSession->hsession->uri_buflen = attribute_data->httpRequestUriLen;
                appIdSession->hsession->fieldOffset[REQ_URI_FID] = attribute_data->httpRequestUriOffset;
                appIdSession->hsession->fieldEndOffset[REQ_URI_FID] = attribute_data->httpRequestUriEndOffset;
                appIdSession->scan_flags |= SCAN_HTTP_URI_FLAG;

                /* Extract host from URI if it is not available */
                if (appIdSession->hsession->host == NULL)
                {
                    appIdSession->hsession->host_buflen = getHttpHostFromUri(&appIdSession->hsession->host, 
                                                                             appIdSession->hsession->uri);
                    appIdSession->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
                }
                attribute_data->httpRequestUri = NULL;
                
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s HTTP URI (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_URI_FID], appIdSession->hsession->fieldEndOffset[REQ_URI_FID], appIdSession->hsession->uri);
            }
        }
        //========================================
        // Begin common HTTP component field data
        //========================================
        if (attribute_data->httpRequestMethod)
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s HTTP request method is %s\n", app_id_debug_session, attribute_data->httpRequestMethod);
            if (!strcmp(attribute_data->httpRequestMethod, "CONNECT"))
                setAppIdFlag(appIdSession, APPID_SESSION_HTTP_CONNECT);
        }
        if (attribute_data->httpRequestVia)
        {
            if (appIdSession->hsession->via)
            {
                free(appIdSession->hsession->via);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->via = attribute_data->httpRequestVia;
            attribute_data->httpRequestVia = NULL;
            appIdSession->scan_flags |= SCAN_HTTP_VIA_FLAG;
        }
        else if (attribute_data->httpResponseVia)
        {
            if (appIdSession->hsession->via)
            {
                free(appIdSession->hsession->via);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->via = attribute_data->httpResponseVia;
            attribute_data->httpResponseVia = NULL;
            appIdSession->scan_flags |= SCAN_HTTP_VIA_FLAG;
        }
        if (attribute_data->httpRequestUserAgent)
        {
            if (appIdSession->hsession->useragent)
            {
                free(appIdSession->hsession->useragent);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->useragent = attribute_data->httpRequestUserAgent;
            appIdSession->hsession->useragent_buflen = attribute_data->httpRequestUserAgentLen;
            attribute_data->httpRequestUserAgent = NULL;
            appIdSession->hsession->fieldOffset[REQ_AGENT_FID] = attribute_data->httpRequestUserAgentOffset;
            appIdSession->hsession->fieldEndOffset[REQ_AGENT_FID] = attribute_data->httpRequestUserAgentEndOffset;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s User-Agent (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_AGENT_FID], appIdSession->hsession->fieldEndOffset[REQ_AGENT_FID], appIdSession->hsession->useragent);
            appIdSession->scan_flags |= SCAN_HTTP_USER_AGENT_FLAG;
        }
        // Check to see if third party discovered HTTP/2.
        //  - once it supports it...
        if (attribute_data->httpResponseVersion)
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s HTTP response version is %s\n", app_id_debug_session, attribute_data->httpResponseVersion);
            if (strncmp(attribute_data->httpResponseVersion, "HTTP/2", 6) == 0)
            {
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s 3rd party detected and parsed HTTP/2\n", app_id_debug_session);
                appIdSession->is_http2 = true;
            }
            free(attribute_data->httpResponseVersion);
            attribute_data->httpResponseVersion = NULL;
        }
        if (attribute_data->httpResponseCode)
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s HTTP response code is %s\n", app_id_debug_session, attribute_data->httpResponseCode);
            if (appIdSession->hsession->response_code)
            {
                free(appIdSession->hsession->response_code);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->response_code = attribute_data->httpResponseCode;
            appIdSession->hsession->response_code_buflen = attribute_data->httpResponseCodeLen;
            attribute_data->httpResponseCode = NULL;
        }
        // Check to see if we've got an upgrade to HTTP/2 (if enabled).
        //  - This covers the "without prior knowledge" case (i.e., the client
        //    asks the server to upgrade to HTTP/2).
        if (attribute_data->httpResponseUpgrade)
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s HTTP response upgrade is %s\n", app_id_debug_session, attribute_data->httpResponseUpgrade);
            if (appidStaticConfig->http2_detection_enabled)
                if (appIdSession->hsession->response_code && (strncmp(appIdSession->hsession->response_code, "101", 3) == 0))
                    if (strncmp(attribute_data->httpResponseUpgrade, "h2c", 3) == 0)
                    {
                        if (app_id_debug_session_flag)
                            _dpd.logMsg("AppIdDbg %s Got an upgrade to HTTP/2\n", app_id_debug_session);
                        appIdSession->is_http2 = true;
                    }
            free(attribute_data->httpResponseUpgrade);
            attribute_data->httpResponseUpgrade = NULL;
        }
        if (attribute_data->httpRequestReferer)
        {
            if (appIdSession->hsession->referer)
            {
                free(appIdSession->hsession->referer);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->referer = attribute_data->httpRequestReferer;
            appIdSession->hsession->referer_buflen = attribute_data->httpRequestRefererLen;
            attribute_data->httpRequestReferer = NULL;
            appIdSession->hsession->fieldOffset[REQ_REFERER_FID] = attribute_data->httpRequestRefererOffset;
            appIdSession->hsession->fieldEndOffset[REQ_REFERER_FID] = attribute_data->httpRequestRefererEndOffset;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s Referer (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_REFERER_FID], appIdSession->hsession->fieldEndOffset[REQ_REFERER_FID], appIdSession->hsession->referer);
        }
        if (attribute_data->httpRequestCookie)
        {
            if (appIdSession->hsession->cookie)
            {
                free(appIdSession->hsession->cookie);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->cookie = attribute_data->httpRequestCookie;
            appIdSession->hsession->cookie_buflen = attribute_data->httpRequestCookieLen;
            attribute_data->httpRequestCookie = NULL;
            appIdSession->hsession->fieldOffset[REQ_COOKIE_FID] = attribute_data->httpRequestCookieOffset;
            appIdSession->hsession->fieldEndOffset[REQ_COOKIE_FID] = attribute_data->httpRequestCookieEndOffset;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s Cookie (%u-%u) is %s\n", app_id_debug_session, appIdSession->hsession->fieldOffset[REQ_COOKIE_FID], appIdSession->hsession->fieldEndOffset[REQ_COOKIE_FID], appIdSession->hsession->cookie);
        }
        if (attribute_data->httpResponseContent)
        {
            if (appIdSession->hsession->content_type)
            {
                free(appIdSession->hsession->content_type);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->content_type = attribute_data->httpResponseContent;
            appIdSession->hsession->content_type_buflen = attribute_data->httpResponseContentLen;
            attribute_data->httpResponseContent = NULL;
            appIdSession->scan_flags |= SCAN_HTTP_CONTENT_TYPE_FLAG;
        }
        if (ptype_scan_counts[LOCATION_PT] && attribute_data->httpResponseLocation)
        {
            if (appIdSession->hsession->location)
            {
                free(appIdSession->hsession->location);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->location = attribute_data->httpResponseLocation;
            appIdSession->hsession->location_buflen = attribute_data->httpResponseLocationLen;
            attribute_data->httpResponseLocation = NULL;
        }
        if (attribute_data->httpRequestBody)
        {
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s got a request body %s\n", app_id_debug_session, attribute_data->httpRequestBody);
            if (appIdSession->hsession->req_body)
            {
                free(appIdSession->hsession->req_body);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->req_body = attribute_data->httpRequestBody;
            appIdSession->hsession->req_body_buflen = attribute_data->httpRequestBodyLen;
            attribute_data->httpRequestBody = NULL;
        }
        if (ptype_scan_counts[BODY_PT] && attribute_data->httpResponseBody)
        {
            if (appIdSession->hsession->body)
            {
                free(appIdSession->hsession->body);
                if (!getAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT))
                    appIdSession->hsession->chp_finished = 0;
            }
            appIdSession->hsession->body = attribute_data->httpResponseBody;
            appIdSession->hsession->body_buflen = attribute_data->httpResponseBodyLen;
            attribute_data->httpResponseBody = NULL;
        }
        if (attribute_data->numXffFields)
        {
            pickHttpXffAddress(p, appIdSession, attribute_data);
        }
        if (!appIdSession->hsession->chp_finished || appIdSession->hsession->chp_hold_flow)
        {
            setAppIdFlag(appIdSession, APPID_SESSION_CHP_INSPECTING);
            if (thirdparty_appid_module)
                thirdparty_appid_module->session_attr_set(appIdSession->tpsession, TP_ATTR_CONTINUE_MONITORING);
        }
        if (attribute_data->httpResponseServer)
        {
            if (appIdSession->hsession->server)
                free(appIdSession->hsession->server);
            appIdSession->hsession->server = attribute_data->httpResponseServer;
            attribute_data->httpResponseServer = NULL;
            appIdSession->scan_flags |= SCAN_HTTP_VENDOR_FLAG;
        }
        if (attribute_data->httpRequestXWorkingWith)
        {
            if (appIdSession->hsession->x_working_with)
                free(appIdSession->hsession->x_working_with);
            appIdSession->hsession->x_working_with = attribute_data->httpRequestXWorkingWith;
            attribute_data->httpRequestXWorkingWith = NULL;
            appIdSession->scan_flags |= SCAN_HTTP_XWORKINGWITH_FLAG;
        }
    }
    else if (ThirdPartyAppIDFoundProto(APP_ID_RTMP, proto_list) ||
             ThirdPartyAppIDFoundProto(APP_ID_RTSP, proto_list))
    {
        if (!appIdSession->hsession)
        {
            if (!(appIdSession->hsession = _dpd.snortAlloc(1, sizeof(*appIdSession->hsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                DynamicPreprocessorFatalMessage("Could not allocate httpSession data");
        }
        if (!appIdSession->hsession->url)
        {
            if (attribute_data->httpRequestUrl)
            {
                appIdSession->hsession->url = attribute_data->httpRequestUrl;
                attribute_data->httpRequestUrl = NULL;
                appIdSession->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
            }
        }

        if (!appidStaticConfig->referred_appId_disabled && !appIdSession->hsession->referer)
        {
            if (attribute_data->httpRequestReferer)
            {
                appIdSession->hsession->referer = attribute_data->httpRequestReferer;
                attribute_data->httpRequestReferer = NULL;
            }
        }

        if (!appIdSession->hsession->useragent)
        {
            if (attribute_data->httpRequestUserAgent)
            {
                appIdSession->hsession->useragent = attribute_data->httpRequestUserAgent;
                appIdSession->hsession->useragent_buflen = attribute_data->httpRequestUserAgentLen;
                attribute_data->httpRequestUserAgent = NULL;
                appIdSession->scan_flags |= SCAN_HTTP_USER_AGENT_FLAG;
            }
        }

        if ((appIdSession->scan_flags & SCAN_HTTP_USER_AGENT_FLAG) && appIdSession->clientAppId <= APP_ID_NONE && appIdSession->hsession->useragent && (size = appIdSession->hsession->useragent_buflen) > 0)
        {
            identifyUserAgent((uint8_t *)appIdSession->hsession->useragent, size, &serviceAppId, &clientAppId, NULL, &pConfig->detectorHttpConfig);
            setClientAppIdData(p, direction, appIdSession, clientAppId, NULL);
            // do not overwrite a previously-set service
            if (appIdSession->serviceAppId <= APP_ID_NONE)
                setServiceAppIdData(p, direction, appIdSession, serviceAppId, NULL, NULL);
            appIdSession->scan_flags &= ~SCAN_HTTP_USER_AGENT_FLAG;
        }

        if (appIdSession->hsession->url || (confidence == 100 && appIdSession->session_packet_count > appidStaticConfig->rtmp_max_packets))
        {
            if (appIdSession->hsession->url)
            {
                if (((getAppIdFromUrl(NULL, appIdSession->hsession->url, NULL,
                                    appIdSession->hsession->referer, &clientAppId, &serviceAppId,
                                    &payloadAppId, &referredPayloadAppId, 1, &pConfig->detectorHttpConfig)) ||
                            (getAppIdFromUrl(NULL, appIdSession->hsession->url, NULL,
                                             appIdSession->hsession->referer, &clientAppId, &serviceAppId,
                                             &payloadAppId, &referredPayloadAppId, 0, &pConfig->detectorHttpConfig))) == 1)

                {
                    // do not overwrite a previously-set client or service
                    if (appIdSession->clientAppId <= APP_ID_NONE)
                        setClientAppIdData(p, direction, appIdSession, clientAppId, NULL);
                    if (appIdSession->serviceAppId <= APP_ID_NONE)
                        setServiceAppIdData(p, direction, appIdSession, serviceAppId, NULL, NULL);

                    // DO overwrite a previously-set payload
                    setPayloadAppIdData(p, direction, appIdSession, payloadAppId, NULL);
                    setReferredPayloadAppIdData(appIdSession, referredPayloadAppId);
                }
            }

            if (thirdparty_appid_module)
            {
                thirdparty_appid_module->disable_flags(appIdSession->tpsession, TP_SESSION_FLAG_ATTRIBUTE | TP_SESSION_FLAG_TUNNELING | TP_SESSION_FLAG_FUTUREFLOW);
                thirdparty_appid_module->session_delete(appIdSession->tpsession, 1);
            }
            clearAppIdFlag(appIdSession, APPID_SESSION_APP_REINSPECT);
        }
    }
    else if (getAppIdFlag(appIdSession, APPID_SESSION_SSL_SESSION))
    {
        int reInspectSSLAppId = 0;
        tAppId tmpAppId = APP_ID_NONE;

        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s flow is SSL\n", app_id_debug_session);

        if (thirdparty_appid_module && appIdSession->tpsession)
            tmpAppId = thirdparty_appid_module->session_appid_get(appIdSession->tpsession);

        if (!appIdSession->tsession)
        {
            if (!(appIdSession->tsession = _dpd.snortAlloc(1, sizeof(*appIdSession->tsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
                DynamicPreprocessorFatalMessage("Could not allocate tlsSession data");
        }

        if (!appIdSession->clientAppId)
            setClientAppIdData(p, direction, appIdSession, APP_ID_SSL_CLIENT, NULL);

        reInspectSSLAppId = testSSLAppIdForReinspect(tmpAppId);

        if (attribute_data->tlsHost)
        {
            /* This flag is to avoid NAVL overwritting SSL provided SNI */
            if (!(appIdSession->scan_flags & SCAN_CERTVIZ_ENABLED_FLAG))
            {
                if (appIdSession->tsession->tls_host)
                    free(appIdSession->tsession->tls_host);
                appIdSession->tsession->tls_host = attribute_data->tlsHost;
                attribute_data->tlsHost = NULL;
                if (reInspectSSLAppId)
                    appIdSession->scan_flags |= SCAN_SSL_HOST_FLAG;
            }
            else
            {
                free(attribute_data->tlsHost);
                attribute_data->tlsHost = NULL;
            }
        }
        if (attribute_data->tlsCname)
        {
            /* This flag is to avoid NAVL overwritting SSL provided CN */
            if (!(appIdSession->scan_flags & SCAN_CERTVIZ_ENABLED_FLAG))
            {
                if (appIdSession->tsession->tls_cname)
                    free(appIdSession->tsession->tls_cname);
                appIdSession->tsession->tls_cname = attribute_data->tlsCname;
                attribute_data->tlsCname = NULL;
                if (reInspectSSLAppId)
                    appIdSession->scan_flags |= SCAN_SSL_CERTIFICATE_FLAG;
            }
            else
            {
                free(attribute_data->tlsCname);
                attribute_data->tlsCname = NULL;
            }
        }
        if (reInspectSSLAppId)
        {
            if (attribute_data->tlsOrgUnit)
            {
                /* This flag is to avoid NAVL overwritting SSL provided ORG */
                if (!(appIdSession->scan_flags & SCAN_CERTVIZ_ENABLED_FLAG))
                {
                    if (appIdSession->tsession->tls_orgUnit)
                        free(appIdSession->tsession->tls_orgUnit);
                    appIdSession->tsession->tls_orgUnit = attribute_data->tlsOrgUnit;
                    attribute_data->tlsOrgUnit = NULL;
                }
                else
                {
                    free(attribute_data->tlsOrgUnit);
                    attribute_data->tlsOrgUnit = NULL;
                }
            }
        }
    }
    else if (ThirdPartyAppIDFoundProto(APP_ID_FTP_CONTROL, proto_list))
    {
        if (!appidStaticConfig->ftp_userid_disabled && attribute_data->ftpCommandUser)
        {
            if (appIdSession->username)
                free(appIdSession->username);
            appIdSession->username = attribute_data->ftpCommandUser;
            attribute_data->ftpCommandUser = NULL;
            appIdSession->usernameService = APP_ID_FTP_CONTROL;
            setAppIdFlag(appIdSession, APPID_SESSION_LOGIN_SUCCEEDED);
        }
    }
}

void appSetServiceDetectorCallback(RNAServiceCallbackFCN fcn, tAppId appId, struct _Detector *userdata, tAppIdConfig *pConfig)
{
#ifdef TARGET_BASED
    AppInfoTableEntry* entry;

    if ((entry = appInfoEntryGet(appId, pConfig)))
    {
        if (entry->svrValidator)
        {
            if (entry->flags & APPINFO_FLAG_SERVICE_DETECTOR_CALLBACK)
            {
                _dpd.errMsg("AppId: Service detector callback already registerted for appid %d\n", appId);
                return;
            }
            entry->svrValidator->userdata = userdata;
            entry->svrValidator->detectorCallback = fcn;
            entry->flags |= APPINFO_FLAG_SERVICE_DETECTOR_CALLBACK;
        }
    }
#endif
    return;
}

void appSetClientDetectorCallback(RNAClientAppCallbackFCN fcn, tAppId appId, struct _Detector *userdata, tAppIdConfig *pConfig)
{
#ifdef TARGET_BASED
    AppInfoTableEntry* entry;

    if ((entry = appInfoEntryGet(appId, pConfig)))
    {
        if (entry->clntValidator)
        {
            if (entry->flags & APPINFO_FLAG_CLIENT_DETECTOR_CALLBACK)
            {
                _dpd.errMsg("AppId: Client detector callback already registerted for appid %d\n", appId);
		return;
            }
            entry->clntValidator->userData = userdata;
            entry->clntValidator->detectorCallback = fcn;
            entry->flags |= APPINFO_FLAG_CLIENT_DETECTOR_CALLBACK;
        }
    }
#endif
    return;
}

void appSetServiceValidator(RNAServiceValidationFCN fcn, tAppId appId, unsigned extractsInfo, tAppIdConfig *pConfig)
{
    AppInfoTableEntry* pEntry = appInfoEntryGet(appId, pConfig);
    if (!pEntry)
    {
        _dpd.errMsg("AppId", "Invalid direct service AppId, %d, for %p", appId, fcn);
        return;
    }
    extractsInfo &= (APPINFO_FLAG_SERVICE_ADDITIONAL | APPINFO_FLAG_SERVICE_UDP_REVERSED);
    if (!extractsInfo)
    {
        _dpd.debugMsg(DEBUG_LOG, "Ignoring direct service without info for %p with AppId %d", fcn, appId);
        return;
    }
    pEntry->svrValidator = ServiceGetServiceElement(fcn, NULL, pConfig);
    if (pEntry->svrValidator)
        pEntry->flags |= extractsInfo;
    else
        _dpd.errMsg("AppId", "Failed to find a service element for %p with AppId %d", fcn, appId);
}

void appSetLuaServiceValidator(RNAServiceValidationFCN fcn, tAppId appId, unsigned extractsInfo, struct _Detector *data)
{
#ifdef TARGET_BASED
    AppInfoTableEntry *entry;
    tAppIdConfig *pConfig = appIdNewConfigGet();

    if ((entry = appInfoEntryGet(appId, pConfig)))
    {

        entry->flags |= APPINFO_FLAG_ACTIVE;

        extractsInfo &= (APPINFO_FLAG_SERVICE_ADDITIONAL | APPINFO_FLAG_SERVICE_UDP_REVERSED);
        if (!extractsInfo)
        {
            _dpd.debugMsg(DEBUG_LOG,"Ignoring direct service without info for %p %p with AppId %d\n",fcn, data, appId);
            return;
        }

        entry->svrValidator = ServiceGetServiceElement(fcn, data, pConfig);
        if (entry->svrValidator)
            entry->flags |= extractsInfo;
        else
            _dpd.errMsg("AppId: Failed to find a service element for %p %p with AppId %d", fcn, data, appId);
    }
    else
    {
        _dpd.errMsg("Invalid direct service AppId, %d, for %p %p\n",appId, fcn, data);
    }
#endif
}

void appSetClientValidator(RNAClientAppFCN fcn, tAppId appId, unsigned extractsInfo, tAppIdConfig *pConfig)
{
    AppInfoTableEntry* pEntry = appInfoEntryGet(appId, pConfig);
    if (!pEntry)
    {
        _dpd.errMsg("AppId", "Invalid direct client application AppId, %d, for %p", appId, fcn);
        return;
    }
    extractsInfo &= (APPINFO_FLAG_CLIENT_ADDITIONAL | APPINFO_FLAG_CLIENT_USER);
    if (!extractsInfo)
    {
        _dpd.debugMsg(DEBUG_LOG, "Ignoring direct client application without info for %p with AppId %d", fcn, appId);
        return;
    }
    pEntry->clntValidator = ClientAppGetClientAppModule(fcn, NULL, &pConfig->clientAppConfig);
    if (pEntry->clntValidator)
        pEntry->flags |= extractsInfo;
    else
        _dpd.errMsg("AppId", "Failed to find a client application module for %p with AppId %d", fcn, appId);
}

void appSetLuaClientValidator(RNAClientAppFCN fcn, tAppId appId, unsigned extractsInfo, struct _Detector *data)
{
    AppInfoTableEntry* entry;
    tAppIdConfig *pConfig = appIdNewConfigGet();

    if ((entry = appInfoEntryGet(appId, pConfig)))
    {
        entry->flags |= APPINFO_FLAG_ACTIVE;
        extractsInfo &= (APPINFO_FLAG_CLIENT_ADDITIONAL | APPINFO_FLAG_CLIENT_USER);
        if (!extractsInfo)
        {
            _dpd.debugMsg(DEBUG_LOG,"Ignoring direct client application without info for %p %p with AppId %d\n",fcn, data, appId);
            return;
        }

        entry->clntValidator = ClientAppGetClientAppModule(fcn, data, &pConfig->clientAppConfig);
        if (entry->clntValidator)
            entry->flags |= extractsInfo;
        else
            _dpd.errMsg("AppId: Failed to find a client application module for %p %p with AppId %d", fcn, data, appId);
    }
    else
    {
        _dpd.errMsg("Invalid direct client application AppId, %d, for %p %p\n",appId, fcn, data);
        return;
    }
}

void AppIdAddUser(tAppIdData *flowp, const char *username, tAppId appId, int success)
{
    if (flowp->username)
        free(flowp->username);
    flowp->username = strdup(username);
    if (!flowp->username)
        DynamicPreprocessorFatalMessage("Could not allocate username data");

    flowp->usernameService = appId;
    if (success)
        setAppIdFlag(flowp, APPID_SESSION_LOGIN_SUCCEEDED);
    else
        clearAppIdFlag(flowp, APPID_SESSION_LOGIN_SUCCEEDED);
}

void AppIdAddDnsQueryInfo(tAppIdData *flow,
                          uint16_t id,
                          const uint8_t *host, uint8_t host_len, uint16_t host_offset,
                          uint16_t record_type, uint16_t options_offset, bool root_query)
{
    if (!flow->dsession)
    {
        if (!(flow->dsession = _dpd.snortAlloc(1, sizeof(*flow->dsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
            DynamicPreprocessorFatalMessage("Could not allocate dnsSession data");
    }
    else if ((flow->dsession->state != 0) && (flow->dsession->id != id))
    {
        AppIdResetDnsInfo(flow);
    }

    if (flow->dsession->state & DNS_GOT_QUERY)
        return;
    flow->dsession->state = DNS_GOT_QUERY;    // clears any response state

    flow->dsession->id          = id;
    flow->dsession->record_type = record_type;

    if (!flow->dsession->host)
    {
        if (root_query && !host_len)
        {
            flow->dsession->host_len       = 1;
            flow->dsession->host_offset    = 0;
            flow->dsession->host           = strdup(".");
            flow->dsession->options_offset = options_offset;
        }
        else if ((host != NULL) && (host_len > 0) && (host_offset > 0))
        {
            flow->dsession->host_len       = host_len;
            flow->dsession->host_offset    = host_offset;
            flow->dsession->host           = dns_parse_host(host, host_len);
            flow->dsession->options_offset = options_offset;
        }
    }
}

void AppIdAddDnsResponseInfo(tAppIdData *flow,
                             uint16_t id,
                             const uint8_t *host, uint8_t host_len, uint16_t host_offset,
                             uint8_t response_type, uint32_t ttl)
{
    if (!flow->dsession)
    {
        if (!(flow->dsession = _dpd.snortAlloc(1, sizeof(*flow->dsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
            DynamicPreprocessorFatalMessage("Could not allocate dnsSession data");
    }
    else if ((flow->dsession->state != 0) && (flow->dsession->id != id))
    {
        AppIdResetDnsInfo(flow);
    }

    if (flow->dsession->state & DNS_GOT_RESPONSE)
        return;
    flow->dsession->state |= DNS_GOT_RESPONSE;

    flow->dsession->id            = id;
    flow->dsession->response_type = response_type;
    flow->dsession->ttl           = ttl;

    if (!flow->dsession->host)
    {
        if ((host != NULL) && (host_len > 0) && (host_offset > 0))
        {
            flow->dsession->host_len    = host_len;
            flow->dsession->host_offset = host_offset;
            flow->dsession->host        = dns_parse_host(host, host_len);
        }
    }
}

void AppIdResetDnsInfo(tAppIdData *flow)
{
    if (flow->dsession)
    {
        free(flow->dsession->host);
        memset(flow->dsession, 0, sizeof(*(flow->dsession)));
    }
}

void AppIdAddPayload(tAppIdData *flow, tAppId payload_id)
{
    if (appidStaticConfig->instance_id)
        checkSandboxDetection(payload_id);
    flow->payloadAppId = payload_id;
}

void AppIdAddMultiPayload(tAppIdData *flow, tAppId payload_id)
{
    if (appidStaticConfig->instance_id)
        checkSandboxDetection(payload_id);
    flow->payloadAppId = payload_id;

    if (flow->multiPayloadList && sfghash_find_node(flow->multiPayloadList, (const void*)&payload_id))
        return;

    if (!flow->multiPayloadList)
        flow->multiPayloadList = sfghash_new(4, sizeof(tAppId), 0, NULL);

    sfghash_add(flow->multiPayloadList, (const void *)&payload_id, (void *)NEW_PAYLOAD_STATE);

    if (app_id_debug_session_flag)
    {
        SFGHASH_NODE *n;
        int buf_index = 0;
        int size = 0;
        tAppId cur;
        char b[MULTI_BUF_SIZE];

        for (n = sfghash_findfirst(flow->multiPayloadList);
             n != 0 && size != -1;)
        {
            cur = *((tAppId*)n->key);
            size = sprintf(b+buf_index, "%d ", cur);
            buf_index+=size;
            n = sfghash_findnext(flow->multiPayloadList);
        }

        _dpd.logMsg("AppIdDbg %s service %d; adding payload %d to multipayload on packet %d.\n Mulipayload includes: %s\n", app_id_debug_session, flow->serviceAppId, payload_id, flow->session_packet_count, b);
    }
}

tAppId getOpenAppId(void *ssnptr)
{
    tAppIdData *session;
    tAppId payloadAppId = APP_ID_NONE;
    if (ssnptr && (session = getAppIdData(ssnptr)))
    {
        payloadAppId = session->payloadAppId;
    }

    return payloadAppId;
}

/**
 * @returns 1 if some appid is found, 0 otherwise.
 */
int sslAppGroupIdLookup(void *ssnptr, const char * serverName, const char * commonName,
        tAppId *serviceAppId, tAppId *clientAppId, tAppId *payloadAppId)
{
    tAppIdData *session;
    *serviceAppId = *clientAppId = *payloadAppId = APP_ID_NONE;

    if (commonName)
    {
        ssl_scan_cname((const uint8_t *)commonName, strlen(commonName), clientAppId, payloadAppId, &pAppidActiveConfig->serviceSslConfig);
    }
    if (serverName)
    {
        ssl_scan_hostname((const uint8_t *)serverName, strlen(serverName), clientAppId, payloadAppId, &pAppidActiveConfig->serviceSslConfig);
    }

    if (ssnptr && (session = getAppIdData(ssnptr)))

    {
        *serviceAppId = pickServiceAppId(session);
        if(*clientAppId == APP_ID_NONE) {
            *clientAppId = pickClientAppId(session);
        }
        if(*payloadAppId == APP_ID_NONE) {
            *payloadAppId = pickPayloadId(session);
        }

    }
    if(*serviceAppId != APP_ID_NONE ||
            *clientAppId != APP_ID_NONE ||
            *payloadAppId != APP_ID_NONE)
    {
        return 1;
    }
    return 0;
}

/*
 * In TLS 1.3, certificates come encrypted, hence SSL module does the
 * certificate decryption and provides the SNI/first-SAN/CN/ON to AppId module.
 * This API does the following:
 *  1. AppIds detection corresponding to the SNI/first-SAN/CN/ON
 *  2. Store the AppIds and SNI/first-SAN/CN/ON in AppID session struct.
 *  3. To make sure the SSL provided SNI/first-SAN/CN/ON are not further overwritten
 *      by NAVL, appropriate flags are set.
 * Note:
 * Valid SNI:              SNI->first_SAN->CN->OU
 * No SNI/Mismatched SNI:  first_SAN->CN->OU
 */
void setTlsHost(void *ssnptr, const char *serverName, const char *commonName,
                const char *orgName, const char *subjectAltName, bool isSniMismatch,
                tAppId *serviceAppId, tAppId *clientAppId, tAppId *payloadAppId)
{
    tAppIdData *session;
    *serviceAppId = *clientAppId = *payloadAppId = APP_ID_NONE;

    if (app_id_debug_session_flag)
        _dpd.logMsg("Received serverName=%s, commonName=%s, orgName=%s, subjectAltName=%s, isSniMismatch=%s, from SSL\n",
                serverName, commonName, orgName, subjectAltName, isSniMismatch?"true":"false");

    if (ssnptr && (session = getAppIdData(ssnptr)))
    {
        if (!session->tsession)
            session->tsession = _dpd.snortAlloc(1, sizeof(*session->tsession),
                    PP_APP_ID, PP_MEM_CATEGORY_SESSION);

        session->scan_flags |= SCAN_SSL_HOST_FLAG;
        session->scan_flags |= SCAN_SSL_CERTIFICATE_FLAG;
        session->scan_flags |= SCAN_CERTVIZ_ENABLED_FLAG;
        if (isSniMismatch)
            session->scan_flags |= SCAN_SPOOFED_SNI_FLAG;

        if (serverName && (*serverName != '\0') && !isSniMismatch)
        {
            if (session->tsession->tls_host)
                free(session->tsession->tls_host);
            session->tsession->tls_host = strdup(serverName);
            session->tsession->tls_host_strlen = strlen(serverName);
        }

        if (subjectAltName && (*subjectAltName != '\0'))
        {
            if (session->tsession->tls_first_san)
                free(session->tsession->tls_first_san);
            session->tsession->tls_first_san = strdup(subjectAltName);
            session->tsession->tls_first_san_strlen = strlen(subjectAltName);
        }

        if (commonName && (*commonName != '\0'))
        {
            if (session->tsession->tls_cname)
                free(session->tsession->tls_cname);
            session->tsession->tls_cname = strdup(commonName);
            session->tsession->tls_cname_strlen = strlen(commonName);
        }

        if (orgName && (*orgName != '\0'))
        {
            if (session->tsession->tls_orgUnit)
                free(session->tsession->tls_orgUnit);
            session->tsession->tls_orgUnit = strdup(orgName);
            session->tsession->tls_orgUnit_strlen = strlen(orgName);
        }

        (void)scanSslParamsLookupAppId(session, (const char*)session->tsession->tls_host,
                isSniMismatch, (const char*)session->tsession->tls_first_san,
                (const char*)session->tsession->tls_cname,
                (const char*)session->tsession->tls_orgUnit, clientAppId, payloadAppId);
    
        *serviceAppId = pickServiceAppId(session);
        if (APP_ID_NONE == *clientAppId)
            *clientAppId = pickClientAppId(session);

        if (APP_ID_NONE == *payloadAppId)
            *payloadAppId = pickPayloadId(session);

        session->serviceAppId = *serviceAppId;
        session->clientAppId  = *clientAppId;
        session->payloadAppId = *payloadAppId;

        if (app_id_debug_session_flag)
            _dpd.logMsg("serviceAppId %d, clientAppId %d, payloadAppId %d\n",
                    session->serviceAppId, session->clientAppId, session->payloadAppId);
    }
}

void httpHeaderCallback (SFSnortPacket *p, HttpParsedHeaders *const headers)
{
    tAppIdData *session;
    APPID_SESSION_DIRECTION direction;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (thirdparty_appid_module)
        return;
    if (!p || !(session = getAppIdData(p->stream_session)))
        return;

    direction = (_dpd.sessionAPI->get_packet_direction(p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;

#ifdef DEBUG_APP_ID_SESSIONS
    {
            char src_ip[INET6_ADDRSTRLEN];
            char dst_ip[INET6_ADDRSTRLEN];
            sfaddr_t *ip;

            src_ip[0] = 0;
            ip = GET_SRC_IP(p);
            inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), src_ip, sizeof(src_ip));
            dst_ip[0] = 0;
            ip = GET_DST_IP(p);
            inet_ntop(sfaddr_family(ip), (void *)sfaddr_get_ptr(ip), dst_ip, sizeof(dst_ip));
            fprintf(SF_DEBUG_FILE, "AppId Http Callback Session %s-%u -> %s-%u %d\n", src_ip,
                    (unsigned)p->src_port, dst_ip, (unsigned)p->dst_port, IsTCP(p) ? IPPROTO_TCP:IPPROTO_UDP);
    }
#endif

    if (!session->hsession)
    {
        if (!(session->hsession = _dpd.snortAlloc(1, sizeof(*session->hsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
            DynamicPreprocessorFatalMessage("Could not allocate httpSession data");
    }

    if (direction == APP_ID_FROM_INITIATOR)
    {
        if (headers->host.start)
        {
            free(session->hsession->host);
            session->hsession->host = strndup((char *)headers->host.start, headers->host.len);
            session->hsession->host_buflen =  headers->host.len;
            session->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;

            if (headers->url.start)
            {
                free(session->hsession->url);
                session->hsession->url = malloc(sizeof(HTTP_PREFIX) + headers->host.len + headers->url.len);
                if (session->hsession->url)
                {
                    strcpy(session->hsession->url, HTTP_PREFIX);
                    strncat(session->hsession->url, (char *)headers->host.start, headers->host.len);
                    strncat(session->hsession->url, (char *)headers->url.start, headers->url.len);
                    session->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
                }
                else
                {
                     DynamicPreprocessorFatalMessage("httpHeaderCallback: "
                             "Failed to allocate memory for URL in APP_ID session header\n");
                }
            }
        }
        if (headers->userAgent.start)
        {
            free(session->hsession->useragent);
            session->hsession->useragent  = strndup((char *)headers->userAgent.start, headers->userAgent.len);
            session->hsession->useragent_buflen = headers->userAgent.len;
            session->scan_flags |= SCAN_HTTP_USER_AGENT_FLAG;
        }
        if (headers->referer.start)
        {
            free(session->hsession->referer);
            session->hsession->referer  = strndup((char *)headers->referer.start, headers->referer.len);
            session->hsession->referer_buflen = headers->referer.len;

        }
        if (headers->via.start)
        {
            free(session->hsession->via);
            session->hsession->via  = strndup((char *)headers->via.start, headers->via.len);
            session->scan_flags |= SCAN_HTTP_VIA_FLAG;
        }

    }
    else
    {
        if (headers->via.start)
        {
            free(session->hsession->via);
            session->hsession->via  = strndup((char *)headers->via.start, headers->via.len);
            session->scan_flags |= SCAN_HTTP_VIA_FLAG;
        }
        if (headers->contentType.start)
        {
            free(session->hsession->content_type);
            session->hsession->content_type  = strndup((char *)headers->contentType.start, headers->contentType.len);
            session->hsession->content_type_buflen = headers->contentType.len;
        }
        if (headers->responseCode.start)
        {
            long responseCodeNum;
            responseCodeNum = strtoul((char *)headers->responseCode.start, NULL, 10);
            if (responseCodeNum > 0 && responseCodeNum < 700)
            {
                free(session->hsession->response_code);
                session->hsession->response_code  = strndup((char *)headers->responseCode.start, headers->responseCode.len);
                session->hsession->response_code_buflen = headers->responseCode.len;
            }
        }
    }
    processHTTPPacket(p, session, direction, headers, pConfig);

    setAppIdFlag(session, APPID_SESSION_SERVICE_DETECTED | APPID_SESSION_HTTP_SESSION);

    _dpd.streamAPI->set_application_id(p->stream_session, pickServiceAppId(session), pickClientAppId(session), pickPayloadId(session), pickMiscAppId(session));
}

static inline void ExamineRtmpMetadata(SFSnortPacket *p, APPID_SESSION_DIRECTION direction, tAppIdData *appIdSession)
{
    tAppId serviceAppId = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    tAppId referredPayloadAppId = 0;
    char *version = NULL;
    httpSession *hsession;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (!appIdSession->hsession)
    {
        if (!(appIdSession->hsession = _dpd.snortAlloc(1, sizeof(*appIdSession->hsession), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
            DynamicPreprocessorFatalMessage("Could not allocate httpSession data");
    }

    hsession = appIdSession->hsession;

    if (hsession->url)
    {
        if (((getAppIdFromUrl(NULL, hsession->url, &version,
                            hsession->referer, &clientAppId, &serviceAppId,
                            &payloadAppId, &referredPayloadAppId, 1, &pConfig->detectorHttpConfig)) ||
                    (getAppIdFromUrl(NULL, hsession->url, &version,
                                     hsession->referer, &clientAppId, &serviceAppId,
                                     &payloadAppId, &referredPayloadAppId, 0, &pConfig->detectorHttpConfig))) == 1)

        {
            /* do not overwrite a previously-set client or service */
            if (appIdSession->clientAppId <= APP_ID_NONE)
                setClientAppIdData(p, direction, appIdSession, clientAppId, NULL);
            if (appIdSession->serviceAppId <= APP_ID_NONE)
                setServiceAppIdData(p, direction, appIdSession, serviceAppId, NULL, NULL);

            /* DO overwrite a previously-set payload */
            setPayloadAppIdData(p, direction, appIdSession, payloadAppId, NULL);
            setReferredPayloadAppIdData(appIdSession, referredPayloadAppId);
        }
    }
}

void checkSandboxDetection(tAppId appId)
{
    AppInfoTableEntry *entry;
    tAppIdConfig *pConfig = appIdActiveConfigGet();

    if (appidStaticConfig->instance_id && pConfig)
    {
        entry = appInfoEntryGet(appId, pConfig);
        if (entry && entry->flags & APPINFO_FLAG_ACTIVE)
        {
            fprintf(SF_DEBUG_FILE, "add service\n");
            fprintf(SF_DEBUG_FILE, "Detected AppId %d\n", entry->appId);
        }
    }
}
