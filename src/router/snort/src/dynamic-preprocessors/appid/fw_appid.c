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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <syslog.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#endif
#include <pthread.h>
#include "fw_appid.h"
#include "profiler.h"
#include "client_app_base.h"
#include "httpCommon.h"
#include "luaDetectorApi.h"
#include "http_url_patterns.h"
#include "fw_appid.h"
#include "detector_http.h"
#include "service_ssl.h"
#include "flow.h"
#include "common_util.h"
#include "spp_appid.h"
#include "hostPortAppCache.h"
#include "appInfoTable.h"
#include "appIdStats.h"
#include "sf_mlmp.h"
#include "ip_funcs.h"

/*#define DEBUG_APP_ID_SESSIONS   1 */
/*#define DEBUG_FW_APPID  1 */
/*#define DEBUG_FW_APPID_PORT 80 */

#define MAX_ATTR_LEN           1024
#define GENERIC_APP_OFFSET 2000000000
#define HTTP_PREFIX "http://"

#define APP_MAPPING_FILE "appMapping.data"

#ifdef RNA_DEBUG_PE
static const char *MODULE_NAME = "fw_appid";
#endif


static volatile int app_id_debug_flag;
static FWDebugSessionConstraints app_id_debug_info;
char app_id_debug_session[FW_DEBUG_SESSION_ID_SIZE];
bool app_id_debug_session_flag;
char SSLBitList[BITNSLOTS(SF_APPID_MAX)];
char referredAppIdBitList[BITNSLOTS(SF_APPID_MAX)];
char SSLSquelchBitList[BITNSLOTS(SF_APPID_MAX)];

#ifdef PERF_PROFILING
PreprocStats httpPerfStats;
PreprocStats clientMatchPerfStats;
PreprocStats serviceMatchPerfStats;
#endif

#define HTTP_PATTERN_MAX_LEN    1024
#define PORT_MAX 65535

unsigned dhcp_fp_table_size = 0;
unsigned long app_id_raw_packet_count = 0;
unsigned long app_id_processed_packet_count = 0;
unsigned long app_id_ignored_packet_count = 0;
static tAppIdData *app_id_free_list;

static void appSharedDataFree(tAppIdData * sharedData)
{
    sharedData->next = app_id_free_list;
    app_id_free_list = sharedData;
}

static void appTlsSessionDataFree (tlsSession *tsession)
{
    if (tsession == NULL) return;

    if (tsession->tls_host)
        free(tsession->tls_host);
    if (tsession->tls_cname)
        free(tsession->tls_cname);
    if (tsession->tls_orgUnit)
        free(tsession->tls_orgUnit);
    free(tsession);
}

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
        /*check daq flag */
        appIdStatsUpdate(sharedData);

        if (sharedData->ssn)
            FailInProcessService(sharedData);
        AppIdFlowdataFree(sharedData);

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
        free(sharedData->username);
        free(sharedData->netbiosDomain);
        free(sharedData->host);
        free(sharedData->url);
        free(sharedData->via);
        free(sharedData->useragent);
        free(sharedData->response_code);
        free(sharedData->payloadVersion);
        free(sharedData->referer);
        appTlsSessionDataFree(sharedData->tsession);
        sharedData->tsession = NULL;
        appSharedDataFree(sharedData);
    }
}

tAppIdData* appSharedDataAlloc(uint8_t proto, snort_ip *ip)
{
    static uint32_t gFlowId;
    tAppIdData *data;

    if (app_id_free_list)
    {
        data = app_id_free_list;
        app_id_free_list = data->next;
        memset(data, 0, sizeof(*data));
    }
    else if (!(data = calloc(1, sizeof(*data))))
        DynamicPreprocessorFatalMessage("Could not allocate tAppIdData data");

    data->flowId = ++gFlowId;
    data->common.fsf_type.flow_type = FLOW_TYPE_NORMAL;
    data->proto = proto;
    data->common.initiator_ip = *ip;
    data->session_packet_count = 0;
    return data;
}

static tAppIdData* appSharedCreateData(const SFSnortPacket *p, uint8_t proto, int direction)
{
#ifdef DEBUG_FW_APPID
    static unsigned long packet_count;
#endif
    tAppIdData *data;
    snort_ip *ip;

    ip = (direction == APP_ID_FROM_INITIATOR) ? GET_SRC_IP(p) : GET_DST_IP(p);
    data = appSharedDataAlloc(proto, ip);

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

void fwAppIdFini(void)
{
#ifdef RNA_FULL_CLEANUP
    tAppIdData *app_id;

    while ((app_id = app_id_free_list))
    {
        app_id_free_list = app_id->next;
        free(app_id);
    }

    AppIdFlowdataFini();
#endif

    appInfoTableFini();

#if 0
    if (sipUaMatcher)
    {
        mlmpDestroy(sipUaMatcher);
        sipUaMatcher = NULL;
        for (node = appSipUaList; node; node = appSipUaList)
        {
            appSipUaList = node->next;
            free((void*)node->pattern.pattern);
            free(node->userData.clientVersion);
            free(node);
        }
    }
#endif
}

#if 0
static inline int PENetworkMatch(snort_ip *pktAddr, PortExclusion *pe)
{
    if( pktAddr->family == pe->family)
    {
        if (IS_IP4(pktAddr))
        {
            return ((pktAddr->ip32[0] & pe->netmask.s6_addr32[0]) == pe->ip.s6_addr32[0]);
        }
        else
        {
            return (((pktAddr->ip32[0] & pe->netmask.s6_addr32[0]) == pe->ip.s6_addr32[0])
                    && ((pktAddr->ip32[1] & pe->netmask.s6_addr32[1]) == pe->ip.s6_addr32[1])
                    && ((pktAddr->ip32[2] & pe->netmask.s6_addr32[2]) == pe->ip.s6_addr32[2])
                    && ((pktAddr->ip32[3] & pe->netmask.s6_addr32[3]) == pe->ip.s6_addr32[3]));
        }

    }
    return 0;
}
#endif

static inline bool fwAppIdDebugCheck(void *lwssn, tAppIdData *session, volatile int debug_flag,
                                     FWDebugSessionConstraints *info, char *debug_session, int direction)
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
            const uint32_t *sip;
            const uint32_t *dip;
            uint16_t sport;
            uint16_t dport;
            char sipstr[INET6_ADDRSTRLEN];
            char dipstr[INET6_ADDRSTRLEN];
            if (!key->ip_l[1] && !key->ip_l[2] && !key->ip_l[3] && !key->ip_h[1] && !key->ip_h[2] && !key->ip_h[3])
                af = AF_INET;
            else
                af = AF_INET6;
            if (session && session->common.fsf_type.flow_type != FLOW_TYPE_IGNORE)
            {
                if (session->common.initiator_port)
                {
                    if (session->common.initiator_port == key->port_l)
                    {
                        sip = key->ip_l;
                        dip = key->ip_h;
                        sport = key->port_l;
                        dport = key->port_h;
                    }
                    else
                    {
                        sip = key->ip_h;
                        dip = key->ip_l;
                        sport = key->port_h;
                        dport = key->port_l;
                    }
                }
                else if (memcmp(&session->common.initiator_ip.ip.u6_addr8[0], key->ip_l, 16) == 0)
                {
                    sip = key->ip_l;
                    dip = key->ip_h;
                    sport = key->port_l;
                    dport = key->port_h;
                }
                else
                {
                    sip = key->ip_h;
                    dip = key->ip_l;
                    sport = key->port_h;
                    dport = key->port_l;
                }
            }
            else
            {
                sip = key->ip_l;
                dip = key->ip_h;
                sport = key->port_l;
                dport = key->port_h;
            }
            sipstr[0] = 0;
            inet_ntop(af, sip, sipstr, sizeof(sipstr));
            dipstr[0] = 0;
            inet_ntop(af, dip, dipstr, sizeof(dipstr));
            snprintf(debug_session, FW_DEBUG_SESSION_ID_SIZE, "%s-%u and %s-%u %u%s",
                     sipstr, (unsigned)sport, dipstr, (unsigned)dport, (unsigned)key->protocol,
                     (direction == APP_ID_FROM_INITIATOR) ? "":" R");
            return true;
        }
    }
    return false;
}

static void ruleEngineDebugParse(const char *desc, const uint8_t *data, uint32_t length,
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
            if (info->sip.s6_addr32[0] || info->sip.s6_addr32[1] || info->sip.s6_addr32[2] || info->sip.s6_addr32[3])
                info->sip_flag = 1;
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
            if (info->dip.s6_addr32[0] || info->dip.s6_addr32[1] || info->dip.s6_addr32[2] || info->dip.s6_addr32[3])
                info->dip_flag = 1;
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
        int af;
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        if (!info->sip.s6_addr32[1] && !info->sip.s6_addr32[2] && !info->sip.s6_addr32[3] &&
            !info->dip.s6_addr32[1] && !info->dip.s6_addr32[2] && !info->dip.s6_addr32[3] &&
            (info->sip.s6_addr32[0] || info->dip.s6_addr32[0]))
        {
            af = AF_INET;
        }
        else
            af = AF_INET6;
        sipstr[0] = 0;
        inet_ntop(af, &info->sip, sipstr, sizeof(sipstr));
        dipstr[0] = 0;
        inet_ntop(af, &info->dip, dipstr, sizeof(dipstr));
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
    ruleEngineDebugParse("appId", data, length, &app_id_debug_flag, &app_id_debug_info);
    return 0;
}


static void setServiceAppIdData(tAppIdData *session, tAppId serviceAppId, char *vendor, char *version)
{
    if (serviceAppId <= APP_ID_NONE || serviceAppId == APP_ID_HTTP)
        return;

    if (session->serviceAppId != serviceAppId)
    {
        session->serviceAppId = serviceAppId;

        /* Clear out previous values of vendor & version */
        if (session->serviceVendor)
            free(session->serviceVendor);
        if (session->serviceVersion)
            free(session->serviceVersion);

        if (vendor && vendor[0])
        {
            session->serviceVendor = strdup(vendor);
            if (!session->serviceVendor)
                _dpd.errMsg("failed to allocate service vendor name");
        }
        else
            session->serviceVendor = NULL;

        if (version && version[0])
        {
            session->serviceVersion = strdup(version);
            if (!session->serviceVersion)
                _dpd.errMsg("failed to allocate service version");
        }
        else
            session->serviceVersion = NULL;
    }
    else
    {
        if ((vendor && vendor[0]) || (version && version[0]))
        {
            /* Clear previous values */
            if (session->serviceVendor)
                free(session->serviceVendor);
            if (session->serviceVersion)
                free(session->serviceVersion);

            /* set vendor */
            if (vendor && vendor[0])
            {
                session->serviceVendor = strdup(vendor);
                if (!session->serviceVendor)
                    _dpd.errMsg("failed to allocate service vendor name");
            }
            else
                session->serviceVendor = NULL;

            /* set version */
            if (version && version[0])
            {
                session->serviceVersion = strdup(version);
                if (!session->serviceVersion)
                    _dpd.errMsg("failed to allocate service version");
            }
            else
                session->serviceVersion = NULL;
        }
    }
}

static void setClientAppIdData(tAppIdData *session, tAppId clientAppId, char *version)
{
    if (clientAppId <= APP_ID_NONE || clientAppId == APP_ID_HTTP)
        return;

    if (session->clientAppId != clientAppId)
    {
        session->clientAppId = clientAppId;

        if (session->clientVersion)
            free(session->clientVersion);

        if (version && version[0])
        {
            session->clientVersion = strdup(version);
            if (!session->clientVersion)
                _dpd.errMsg("failed to allocate client version");
        }
        else
            session->clientVersion = NULL;
    }
    else if (version && version[0])
    {
        if (session->clientVersion)
            free(session->clientVersion);
        session->clientVersion = strdup(version);
        if (!session->clientVersion)
            _dpd.errMsg("failed to allocate client version");
    }
}

static void setReferredPayloadAppIdData(tAppIdData *session, tAppId referredPayloadAppId)
{
    if (referredPayloadAppId <= APP_ID_NONE)
        return;

    if (session->referredPayloadAppId != referredPayloadAppId)
    {
        session->referredPayloadAppId = referredPayloadAppId;
    }
}

static void setPayloadAppIdData(tAppIdData *session, tAppId payloadAppId, char *version)
{
    if (payloadAppId <= APP_ID_NONE)
        return;

    if (session->payloadAppId != payloadAppId)
    {
        session->payloadAppId = payloadAppId;

        if (session->payloadVersion)
            free(session->payloadVersion);

        if (version && version[0])
        {
            session->payloadVersion = strdup(version);
            if (!session->payloadVersion)
                _dpd.errMsg("failed to allocate payload version");
        }
        else
            session->payloadVersion = NULL;
    }
    else if (version && version[0])
    {
        if (session->payloadVersion)
            free(session->payloadVersion);
        session->payloadVersion = strdup(version);
        if (!session->payloadVersion)
            _dpd.errMsg("failed to allocate payload version");
    }
}

static void clearSessionAppIdData(tAppIdData *session)
{
    session->payloadAppId = APP_ID_UNKNOWN;
    session->serviceAppId = APP_ID_UNKNOWN;
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
    if (session->tsession)
    {
        appTlsSessionDataFree(session->tsession);
        session->tsession = NULL;
    }
}

static int processHTTPPacket(SFSnortPacket *p, tAppIdData *session, int direction, HttpParsedHeaders *const headers)
{
#define RESPONSE_CODE_LENGTH 3
    int start, end;
    int size;
    char vendor[MAX_VERSION_SIZE];
    char version[MAX_VERSION_SIZE];
    tAppId serviceAppId = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    tAppId referredPayloadAppId = 0;
    char *response_code = session->response_code;
    char *host;
    char *url;
    char *useragent;
    char *referer;
    char *via;
    PROFILE_VARS;

    PREPROC_PROFILE_START(httpPerfStats);

    if (direction == APP_ID_FROM_RESPONDER && !flow_checkflag(session, FLOW_RESPONSE_CODE_CHECKED))
    {
        if (response_code)
        {
            flow_mark(session, FLOW_RESPONSE_CODE_CHECKED);
            if (strlen(response_code) != RESPONSE_CODE_LENGTH)
            {
                /* received bad response code. Stop processing this session */
                clearSessionAppIdData(session);
                if (app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s bad http resopnse code\n", app_id_debug_session);
                PREPROC_PROFILE_END(httpPerfStats);
                return 0;
            }
        }
#if RESPONSE_CODE_PACKET_THRESHHOLD
        else if (++(session->response_code_packets) == RESPONSE_CODE_PACKET_THRESHHOLD)
        {
            flow_mark(session, FLOW_RESPONSE_CODE_CHECKED);
            /* didn't receive response code in first X packets. Stop processing this session */
            clearSessionAppIdData(session);
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s no response code received\n", app_id_debug_session);
            PREPROC_PROFILE_END(httpPerfStats);
            return 0;
        }
#endif
    }

    host = session->host;
    url = session->url;
    useragent = session->useragent;
    referer = session->referer;
    via = session->via;

    if (!flow_checkflag(session, FLOW_APP_REINSPECT))
    {
        if (session->serviceAppId == APP_ID_NONE)
            session->serviceAppId = APP_ID_HTTP;

        /* Scan Server Header for Vendor & Version */
        if (headers->server.start)
        {
            if (session->serviceAppId == APP_ID_NONE || session->serviceAppId == APP_ID_HTTP)
            {
                RNAServiceSubtype *subtype = NULL;
                RNAServiceSubtype **tmpSubtype;

                version[0] = 0;
                vendor[0] = 0;
                getServerVendorVersion(headers->server.start, headers->server.len, version, vendor, &subtype);
                if (vendor[0] || version[0])
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
                    if (vendor[0])
                    {
                        session->serviceVendor = strdup(vendor);
                        if (!session->serviceVendor)
                            _dpd.errMsg("failed to allocate service vendor name");
                    }
                    if (version[0])
                    {
                        session->serviceVersion = strdup(version);
                        if (!session->serviceVersion)
                            _dpd.errMsg("failed to allocate service version");
                    }
                }
                if (subtype)
                {
                    for (tmpSubtype = &session->subtype; *tmpSubtype; tmpSubtype = &(*tmpSubtype)->next);

                    *tmpSubtype = subtype;
                }
            }
        }

        /* set webdav_found for following cases */
        {
            HeaderMatchedPatterns hmp;
            memset(&hmp, 0, sizeof(hmp));
            if ((getHTTPHeaderLocation(headers->method.start, headers->method.len, HTTP_ID_COPY, &start, &end, &hmp) == 1)
                    || (hmp.headers[HTTP_ID_MOVE].start > 0) || (hmp.headers[HTTP_ID_LOCK].start > 0)
                    || (hmp.headers[HTTP_ID_UNLOCK].start > 0) || (hmp.headers[HTTP_ID_MKCOL].start > 0) || (hmp.headers[HTTP_ID_PROPPATCH].start > 0)
                    || (hmp.headers[HTTP_ID_PROPFIND].start > 0))
            {
                if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
                    _dpd.logMsg("AppIdDbg %s payload is webdav\n", app_id_debug_session);
                setPayloadAppIdData(session, APP_ID_WEBDAV, NULL);
            }
        }

        /* Scan for Skype & Bittorrent in URL */
        if ((session->scan_flags & SCAN_HTTP_HOST_URL_FLAG) && url && (size = strlen(url)) > 0)
        {
            version[0] = 0;
            if (ScanURLForClientApp((const u_int8_t *)url, size, &clientAppId, &serviceAppId, version) > 0)
            {
                if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                    _dpd.logMsg("AppIdDbg %s URL is client %d\n", app_id_debug_session, clientAppId);
                setClientAppIdData(session, clientAppId, version);
                if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                    _dpd.logMsg("AppIdDbg %s URL is service %d\n", app_id_debug_session, serviceAppId);
                setServiceAppIdData(session, serviceAppId, NULL, NULL);
            }
        }

        /* Scan User-Agent for Browser types or Skype */
        if ((session->scan_flags & SCAN_HTTP_USER_AGENT_FLAG) && useragent && (size = strlen(useragent)) > 0)
        {
            version[0] = 0;
            identifyUserAgent((uint8_t *)useragent, size, &serviceAppId, &clientAppId, version);
            if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                _dpd.logMsg("AppIdDbg %s User Agent is service %d\n", app_id_debug_session, serviceAppId);
            setServiceAppIdData(session, serviceAppId, NULL, NULL);
            if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                _dpd.logMsg("AppIdDbg %s User Agent is client %d\n", app_id_debug_session, clientAppId);
            setClientAppIdData(session, clientAppId, version);
            session->scan_flags &= ~SCAN_HTTP_USER_AGENT_FLAG;
        }

        /* Scan Via Header for squid */
        if (!(session->payloadAppId) && (session->scan_flags & SCAN_HTTP_VIA_FLAG) && via && (size = strlen(via)) > 0)
        {
            version[0] = 0;
            payloadAppId = getAppidByViaPattern((uint8_t *)via, size, version);
            if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
                _dpd.logMsg("AppIdDbg %s VIA is payload %d\n", app_id_debug_session, payloadAppId);
            setPayloadAppIdData(session, payloadAppId, NULL);
            session->scan_flags &= ~SCAN_HTTP_VIA_FLAG;
        }
    }

    /* Scan X-Working-With HTTP header */
    if (headers->xWorkingWith.start)
    {
        tAppId appId;

        appId = scan_header_x_working_with(headers->xWorkingWith.start, headers->xWorkingWith.len, version);
        if (appId)
        {
            if (direction == APP_ID_FROM_INITIATOR)
            {
                if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                    _dpd.logMsg("AppIdDbg %s X is client %d\n", app_id_debug_session, appId);
                setClientAppIdData(session, appId, version);
            }
            else
            {
                if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                    _dpd.logMsg("AppIdDbg %s X is service %d\n", app_id_debug_session, appId);
                setServiceAppIdData(session, appId, NULL, version);
            }
        }
    }

    /* Scan Content-Type Header for multimedia types and scan contents */
    if (!(session->payloadAppId) && (headers->xWorkingWith.start) && headers->contentType.start)
    {
        payloadAppId = getAppidByContentType(headers->contentType.start, headers->contentType.len);
        if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
            _dpd.logMsg("AppIdDbg %s Content-Type is payload %d\n", app_id_debug_session, payloadAppId);
        setPayloadAppIdData(session, payloadAppId, NULL);
    }

    version[0] = 0;
    if ((session->scan_flags & SCAN_HTTP_HOST_URL_FLAG) &&
        getAppIdFromUrl(host, url, version, sizeof(version), referer, &clientAppId, &serviceAppId, &payloadAppId, &referredPayloadAppId, 0) == 1)
    {
        /* do not overwrite a previously-set client or service */
        if (session->clientAppId <= APP_ID_NONE)
        {
            if (app_id_debug_session_flag && clientAppId > APP_ID_NONE && clientAppId != APP_ID_HTTP && session->clientAppId != clientAppId)
                _dpd.logMsg("AppIdDbg %s URL is client %d\n", app_id_debug_session, clientAppId);
            setClientAppIdData(session, clientAppId, NULL);
        }
        if (session->serviceAppId <= APP_ID_NONE)
        {
            if (app_id_debug_session_flag && serviceAppId > APP_ID_NONE && serviceAppId != APP_ID_HTTP && session->serviceAppId != serviceAppId)
                _dpd.logMsg("AppIdDbg %s URL is service %d\n", app_id_debug_session, serviceAppId);
            setServiceAppIdData(session, serviceAppId, NULL, NULL);
        }

        /* DO overwrite a previously-set payload */
        if (app_id_debug_session_flag && payloadAppId > APP_ID_NONE && session->payloadAppId != payloadAppId)
            _dpd.logMsg("AppIdDbg %s URL is payload %d\n", app_id_debug_session, payloadAppId);
        setPayloadAppIdData(session, payloadAppId, version);
        setReferredPayloadAppIdData(session, referredPayloadAppId);
    }

    if (session->clientAppId == APP_ID_APPLE_CORE_MEDIA)
    {
        if (session->payloadAppId > APP_ID_NONE)
        {   
            session->miscAppId = session->clientAppId;
            session->clientAppId = session->payloadAppId + GENERIC_APP_OFFSET;
        }   
    }
    session->scan_flags &= ~SCAN_HTTP_HOST_URL_FLAG;

    PREPROC_PROFILE_END(httpPerfStats);
    return 0;
}

#if 0
static  inline void stopRnaServiceInspection(SFSnortPacket *p, tAppIdData* session, int direction)
{
    snort_ip *ip;
    if (direction == APP_ID_FROM_INITIATOR)
    {
        ip = GET_DST_IP(p);
        session->service_ip = *ip;
        session->service_port = p->dst_port;
    }
    else
    {
        ip = GET_SRC_IP(p);
        session->service_ip = *ip;
        session->service_port = p->src_port;
    }
    session->rnaServiceState = RNA_STATE_FINISHED;
    flow_mark(session, FLOW_SERVICEDETECTED);
    flow_clear(session, FLOW_CONTINUE);
}
#endif

void httpHeaderCallback (SFSnortPacket *p, HttpParsedHeaders *const headers)
{
    tAppIdData *session;
    int direction;

    direction = (_dpd.sessionAPI->get_packet_direction(p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;

#ifdef DEBUG_APP_ID_SESSIONS
    {
            char src_ip[INET6_ADDRSTRLEN];
            char dst_ip[INET6_ADDRSTRLEN];

            src_ip[0] = 0;
            ip = GET_SRC_IP(p);
            inet_ntop(ip->family, (void *)ip->ip32, src_ip, sizeof(src_ip));
            dst_ip[0] = 0;
            ip = GET_DST_IP(p);
            inet_ntop(ip->family, (void *)ip->ip32, dst_ip, sizeof(dst_ip));
            fprintf(SF_DEBUG_FILE, "AppId Http Callback Session %s-%u -> %s-%u %d\n", src_ip,
                    (unsigned)p->src_port, dst_ip, (unsigned)p->dst_port, IsTCP(p) ? IPPROTO_TCP:IPPROTO_UDP);
    }
#endif
    session = appSharedGetData(p);
    if (!session)
    {
        _dpd.errMsg("Missing session\n");
        return;
    }

    if (direction == APP_ID_FROM_INITIATOR)
    {
        if (headers->host.start)
        {
            free(session->host);
            session->host = strndup((char *)headers->host.start, headers->host.len);
            session->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;

            if (headers->url.start)
            {
                free(session->url);
                session->url = malloc(sizeof(HTTP_PREFIX) + headers->host.len + headers->url.len);
                if (session->url)
                {
                    strcpy(session->url, HTTP_PREFIX);
                    strncat(session->url, (char *)headers->host.start, headers->host.len);
                    strncat(session->url, (char *)headers->url.start, headers->url.len); 
                    session->scan_flags |= SCAN_HTTP_HOST_URL_FLAG;
                }
            }
        }
        if (headers->userAgent.start)
        {
            free(session->useragent);
            session->useragent  = strndup((char *)headers->userAgent.start, headers->userAgent.len);
            session->scan_flags |= SCAN_HTTP_USER_AGENT_FLAG;
        }
        if (headers->referer.start)
        {
            free(session->referer);
            session->referer  = strndup((char *)headers->referer.start, headers->referer.len);

        }
        if (headers->via.start)
        {
            free(session->via);
            session->via  = strndup((char *)headers->via.start, headers->via.len);
            session->scan_flags |= SCAN_HTTP_VIA_FLAG;
        }

    }
    else 
    {
        if (headers->via.start)
        {
            free(session->via);
            session->via  = strndup((char *)headers->via.start, headers->via.len);
            session->scan_flags |= SCAN_HTTP_VIA_FLAG;
        }
        if (headers->responseCode.start)
        {
            long responseCodeNum;
            responseCodeNum = strtoul((char *)headers->responseCode.start, NULL, 10);
            if (responseCodeNum > 0 && responseCodeNum < 700)
            {
                free(session->response_code);
                session->response_code  = strndup((char *)headers->responseCode.start, headers->responseCode.len);
            }
        }
    }
    processHTTPPacket(p, session, direction, headers);

    flow_mark(session, FLOW_SERVICEDETECTED);
    flow_mark(session, FLOW_HTTP_SESSION);

    _dpd.streamAPI->set_application_id(p->stream_session, appGetSnortIdFromAppId(pickServiceAppId(session)), appGetSnortIdFromAppId(pickClientAppId(session)), 
                appGetSnortIdFromAppId(pickPayloadId(session)), appGetSnortIdFromAppId(pickMiscAppId(session)));
}

static void ExamineSslMetadata(SFSnortPacket *p, tAppIdData *session)
{
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    size_t size;
    int ret;

    if ((session->scan_flags & SCAN_SSL_HOST_FLAG) && session->tsession->tls_host)
    {
        size = session->tsession->tls_host_strlen;
        if ((ret = ssl_scan_hostname((const u_int8_t *)session->tsession->tls_host, size, &clientAppId, &payloadAppId)))
        {
            setClientAppIdData(session, clientAppId, NULL);
            setPayloadAppIdData(session, payloadAppId, NULL);
            setSSLSquelch(p, ret, (ret == 1 ? payloadAppId : clientAppId));
        }
        session->scan_flags &= ~SCAN_SSL_HOST_FLAG;
        // ret = 0;
    }
    if (session->tsession->tls_cname)
    {
        size = session->tsession->tls_cname_strlen;
        if ((ret = ssl_scan_cname((const u_int8_t *)session->tsession->tls_cname, size, &clientAppId, &payloadAppId)))
        {
            setClientAppIdData(session, clientAppId, NULL);
            setPayloadAppIdData(session, payloadAppId, NULL);
            setSSLSquelch(p, ret, (ret == 1 ? payloadAppId : clientAppId));
        }
        free(session->tsession->tls_cname);
        session->tsession->tls_cname = NULL;
        // ret = 0;
    }
    if (session->tsession->tls_orgUnit)
    {
        size = session->tsession->tls_orgUnit_strlen;
        if ((ret = ssl_scan_cname((const u_int8_t *)session->tsession->tls_orgUnit, size, &clientAppId, &payloadAppId)))
        {
            setClientAppIdData(session, clientAppId, NULL);
            setPayloadAppIdData(session, payloadAppId, NULL);
            setSSLSquelch(p, ret, (ret == 1 ? payloadAppId : clientAppId));
        }
        free(session->tsession->tls_orgUnit);
        session->tsession->tls_orgUnit = NULL;
        // ret = 0;
    }
}

void ExamineRtmpMetadata(tAppIdData *appIdSession)
{
    tAppId serviceAppId = 0;
    tAppId clientAppId = 0;
    tAppId payloadAppId = 0;
    tAppId referredPayloadAppId = 0;
    char version[MAX_VERSION_SIZE];

    if (appIdSession->url)
    {
        version[0] = 0;
        if (((getAppIdFromUrl(NULL, appIdSession->url, version, sizeof(version),
                            appIdSession->referer, &clientAppId, &serviceAppId,
                            &payloadAppId, &referredPayloadAppId, 1)) ||
                    (getAppIdFromUrl(NULL, appIdSession->url, version, sizeof(version),
                                     appIdSession->referer, &clientAppId, &serviceAppId,
                                     &payloadAppId, &referredPayloadAppId, 0))) == 1)

        {
            /* do not overwrite a previously-set client or service */
            if (appIdSession->clientAppId <= APP_ID_NONE)
                setClientAppIdData(appIdSession, clientAppId, NULL);
            if (appIdSession->serviceAppId <= APP_ID_NONE)
                setServiceAppIdData(appIdSession, serviceAppId, NULL, NULL);

            /* DO overwrite a previously-set payload */
            setPayloadAppIdData(appIdSession, payloadAppId, NULL);
            setReferredPayloadAppIdData(appIdSession, referredPayloadAppId);
        }
    }
}

void fwAppIdSearch(SFSnortPacket *p)
{
    tAppIdData *session;
    uint8_t protocol;
    /*tAppId clientAppId = 0; */
    /*tAppId payloadAppId = 0; */
    int direction;
    snort_ip *ip;
    uint16_t port;
    /*size_t size; */
#ifdef TARGET_BASED
    AppInfoTableEntry *entry;
#endif

    app_id_raw_packet_count++;

    if (!p->stream_session||
#ifdef APP_ID_USES_REASSEMBLED
        (p->flags & FLAG_STREAM_INSERT)
#else
        (p->flags & FLAG_REBUILT_STREAM)
#endif
       )
    {
        app_id_ignored_packet_count++;
        return;
    }

    SetPacketRealTime(p->pkt_header->ts.tv_sec);

    session = appSharedGetData(p);
    if (session)
    {
#ifdef DEBUG_APP_ID_SESSIONS
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
        if (session->service_port == DEBUG_FW_APPID_PORT)
#endif
        {
            char src_ip[INET6_ADDRSTRLEN];
            char dst_ip[INET6_ADDRSTRLEN];

            src_ip[0] = 0;
            ip = GET_SRC_IP(p);
            inet_ntop(ip->family, (void *)ip->ip32, src_ip, sizeof(src_ip));
            dst_ip[0] = 0;
            ip = GET_DST_IP(p);
            inet_ntop(ip->family, (void *)ip->ip32, dst_ip, sizeof(dst_ip));
            fprintf(SF_DEBUG_FILE, "AppId Session %p %p for %s-%u -> %s-%u %d\n", session, session->ssn, src_ip,
                    (unsigned)p->src_port, dst_ip, (unsigned)p->dst_port, IsTCP(p) ? IPPROTO_TCP:IPPROTO_UDP);
        }
#endif
        if (session->common.fsf_type.flow_type == FLOW_TYPE_IGNORE)
            return;
        if (session->common.fsf_type.flow_type == FLOW_TYPE_NORMAL)
        {
            protocol = session->proto;
            session->ssn = p->stream_session;
        }
        else if (IsTCP(p))
            protocol = IPPROTO_TCP;
        else
            protocol = IPPROTO_UDP;
        ip = GET_SRC_IP(p);
        if (session->common.initiator_port)
            direction = (session->common.initiator_port == p->src_port) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
        else
            direction = (sfip_fast_equals_raw(ip, &session->common.initiator_ip)) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
    }
    else
    {
        if (IsTCP(p))
            protocol = IPPROTO_TCP;
        else if (IsUDP(p))
            protocol = IPPROTO_UDP;
        else if (p->ip4h)
            protocol = p->ip4h->ip_proto;
        else
            return;
        direction = (_dpd.sessionAPI->get_packet_direction(p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
    }

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
#endif
    {
        char sipstr[INET6_ADDRSTRLEN];
        char dipstr[INET6_ADDRSTRLEN];

        sipstr[0] = 0;
        ip = GET_SRC_IP(p);
        inet_ntop(ip->family, (void *)ip->ip32, sipstr, sizeof(sipstr));
        dipstr[0] = 0;
        ip = GET_DST_IP(p);
        inet_ntop(ip->family, (void *)ip->ip32, dipstr, sizeof(dipstr));
        fprintf(SF_DEBUG_FILE, "%s-%u -> %s-%u %u\n", sipstr, (unsigned)p->src_port, dipstr, (unsigned)p->dst_port, (unsigned)protocol);
        /*DumpHex(SF_DEBUG_FILE, p->payload, p->payload_size); */
    }
#endif

    app_id_debug_session_flag = fwAppIdDebugCheck(p->stream_session, session, app_id_debug_flag,
                                                  &app_id_debug_info, app_id_debug_session, direction);

    if (!session || session->common.fsf_type.flow_type == FLOW_TYPE_TMP)
    {
        /* This call will free the existing temporary session, if there is one */
        session = appSharedCreateData(p, protocol, direction);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s new session\n", app_id_debug_session);
    }

    app_id_processed_packet_count++;
    session->session_packet_count++;
    if (direction == APP_ID_FROM_INITIATOR)
        session->stats.initiatorBytes += p->pkt_header->pktlen;
    else
        session->stats.responderBytes += p->pkt_header->pktlen;

    session->common.policyId = appIdPolicyId;

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
    {
#endif
        fprintf(SF_DEBUG_FILE, "%u -> %u %u Begin %d %u - %d %d %d %u %08X %u\n",
                (unsigned)p->src_port, (unsigned)p->dst_port, (unsigned)protocol, direction,
                (unsigned)p->payload_size, session->serviceAppId, session->clientAppId, session->payloadAppId,
                session->rnaServiceState, p->flags, session->nsession->session_state);
        /*DumpHex(SF_DEBUG_FILE, p->payload, p->payload_size); */
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    }
#endif
#endif

    if (p->flags & FLAG_STREAM_ORDER_BAD)
        flow_mark(session, FLOW_OOO);
    else if (p->tcp_header)
    {
        if ((p->tcp_header->flags & TCPHEADER_RST) && session->previous_tcp_flags == TCPHEADER_SYN)
        {
            AppIdServiceIDState *id_state;

            flow_mark(session, FLOW_SYN_RST);
            if (session->service_ip.family)
            {
                ip = &session->service_ip;
                port = session->service_port;
            }
            else
            {
                ip = GET_SRC_IP(p);
                port = p->src_port;
            }
            id_state = AppIdGetServiceIDState(ip, IPPROTO_TCP, port);
            if (id_state)
            {
                if (!id_state->reset_time)
                    id_state->reset_time = GetPacketRealTime;
                else if ((GetPacketRealTime - id_state->reset_time) >= 60)
                {
                    AppIdRemoveServiceIDState(ip, IPPROTO_TCP, port);
                    flow_mark(session, FLOW_SERVICEDELETED);
                }
            }
        }
        session->previous_tcp_flags = p->tcp_header->flags;
    }


    /*HostPort based AppId.  */
    if (!(session->scan_flags & SCAN_HOST_PORT_FLAG)) 
    {
        tAppId tmpAppId;
        int16_t snortId;

        session->scan_flags |= SCAN_HOST_PORT_FLAG;
        if (direction == APP_ID_FROM_INITIATOR)
        {
            ip = GET_DST_IP(p);
            port = p->dst_port;
        }
        else
        {
            ip = GET_SRC_IP(p);
            port = p->src_port;
        }
    
        if ((tmpAppId = hostPortAppCacheFind(ip, port, protocol)) > APP_ID_NONE)
        {
            session->serviceAppId = tmpAppId;
            if ((entry = getAppInfoEntry(tmpAppId)) && (snortId = entry->snortId) && snortId != session->snortId)
            {
                _dpd.sessionAPI->set_application_protocol_id(p->stream_session, snortId);
                session->snortId = snortId;
            }
            session->rnaServiceState = RNA_STATE_FINISHED;
            session->rnaClientState = RNA_STATE_FINISHED;
            flow_mark(session, FLOW_SERVICEDETECTED);
        }
    }

    /*restart app inspection */
    if (isAppDetectionDone(session)
        && flow_checkflag(session, FLOW_HTTP_SESSION) && p->payload_size
            && (direction == APP_ID_FROM_INITIATOR))
    {
        flow_mark(session, FLOW_APP_REINSPECT);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s reinspecting http\n", app_id_debug_session);
        if (session->url)
        {
            free(session->url);
            session->url = NULL;
        }
    }

    if (session->payloadAppId != APP_ID_SFTP 
        && session->serviceAppId == APP_ID_SSH
        && session->session_packet_count >= MIN_SFTP_PACKET_COUNT && session->session_packet_count < MAX_SFTP_PACKET_COUNT)
    {   
        if (GET_IPH_TOS(p) == 8)
        {   
            session->payloadAppId = APP_ID_SFTP;
            if (app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s payload is SFTP\n", app_id_debug_session);
        }
    }

    if (direction == APP_ID_FROM_RESPONDER && !flow_checkflag(session, FLOW_PORT_SERVICE_DONE|FLOW_SYN_RST))
    {
        flow_mark(session, FLOW_PORT_SERVICE_DONE);
        session->portServiceAppId = getPortServiceId(protocol, p->src_port);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s port service %d\n", app_id_debug_session, session->portServiceAppId);
    }

    /* exceptions for rexec and any other service detector that needs to see SYN and SYN/ACK */
    if (flow_checkflag(session, FLOW_REXEC_STDERR))
    {
        AppIdDiscoverService(p, direction, session);
    }

    /*service */
#ifndef APP_ID_USES_REASSEMBLED
    if (protocol != IPPROTO_TCP || (p->flags & FLAG_STREAM_ORDER_OK))
#endif
    {
        if (session->rnaServiceState != RNA_STATE_FINISHED)
        {
            /*uint32_t prevRnaServiceState; */
            PROFILE_VARS;
            PREPROC_PROFILE_START(serviceMatchPerfStats);

            /*prevRnaServiceState = session->rnaServiceState; */

            /*decision to directly call validator or go through elaborate service_state tracking */
            /*is made once at the beginning of sesssion. */
            if (session->rnaServiceState == RNA_STATE_NONE && p->payload_size)
            {
                if (_dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
                {
                    /* Unless it could be ftp control */
                    if (protocol == IPPROTO_TCP && (p->src_port == 21 || p->dst_port == 21) &&
                        !(p->tcp_header->flags & (TCPHEADER_FIN | TCPHEADER_RST)))
                    {
                        flow_mark(session, FLOW_CLIENTAPPDETECTED | FLOW_NOT_A_SERVICE | FLOW_SERVICEDETECTED);
                        if (!AddFTPServiceState(session))
                        {
                            flow_mark(session, FLOW_CONTINUE);
                            if (p->dst_port != 21)
                                flow_mark(session, FLOW_RESPONDER_SEEN);
                        }
                        session->rnaServiceState = RNA_STATE_STATEFUL;
                    }
                    else
                    {
                        flow_mark(session, FLOW_MID | FLOW_SERVICEDETECTED);
                        session->rnaServiceState = RNA_STATE_FINISHED;
                    }
                }
                else
                    session->rnaServiceState = RNA_STATE_STATEFUL;
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
                AppIdDiscoverService(p, direction, session);
                /*to stop executing validator after service has been detected by RNA. */
                if (flow_checkflag(session, FLOW_SERVICEDETECTED) && !flow_checkflag(session, FLOW_CONTINUE))
                    session->rnaServiceState = RNA_STATE_FINISHED;

                /* SSL */
                if (flow_checkflag(session, FLOW_SSL_SESSION))
                {
                    if (!(session->scan_flags & SCAN_SSL_HOST_FLAG))
                    {
                        setSSLSquelch(p, 1, session->serviceAppId);
                    }
                    if (session->tsession)
                    {
                        ExamineSslMetadata(p, session);
                    }
                }

                /* RTMP */
                if (session->serviceAppId == APP_ID_RTMP)
                    ExamineRtmpMetadata(session);

#ifdef TARGET_BASED
                if (flow_checkflag(session, FLOW_SERVICEDETECTED) &&
                    !flow_checkflag(session, FLOW_NOT_A_SERVICE | FLOW_IGNORE_HOST))
                {
                    tAppId svcId;
                    int16_t snortId;
                    if ((svcId = session->serviceAppId) > APP_ID_NONE && svcId < SF_APPID_MAX &&
                        (entry = getAppInfoEntry(svcId)) && (snortId = entry->snortId) && snortId != session->snortId)
                    {
                        _dpd.sessionAPI->set_application_protocol_id(p->stream_session, snortId);
                        session->snortId = snortId;
                    }
                }
#endif
            }
            PREPROC_PROFILE_END(serviceMatchPerfStats);
        }

        if (session->rnaClientState != RNA_STATE_FINISHED)
        {
            PROFILE_VARS;
            PREPROC_PROFILE_START(clientMatchPerfStats);
            /*uint32_t prevRnaClientState = session->rnaClientState; */

            /*decision to directly call validator or go through elaborate service_state tracking */
            /*is made once at the beginning of sesssion. */
            if (session->rnaClientState == RNA_STATE_NONE && p->payload_size && direction == APP_ID_FROM_INITIATOR)
            {
                if (_dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
                    session->rnaClientState = RNA_STATE_FINISHED;
                else if (flow_checkflag(session, FLOW_HTTP_SESSION))
                    session->rnaClientState = RNA_STATE_FINISHED;
                else
                    session->rnaClientState = RNA_STATE_STATEFUL;
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
                    if (!flow_checkflag(session, FLOW_CLIENTAPPDETECTED))
                    {
                        ret = session->clientData->validate(p->payload, p->payload_size, direction,
                                                            session, p, session->clientData->userData);
                        if (app_id_debug_session_flag)
                            _dpd.logMsg("AppIdDbg %s %s client detector returned %d\n", app_id_debug_session,
                                        session->clientData->name ? session->clientData->name:"UNKNOWN", ret);
                    }
                }
                else if (session->rnaServiceState != RNA_STATE_STATEFUL &&
                         flow_checkflag(session, FLOW_CLIENT_GETS_SERVER_PACKETS))
                {
                    ret = session->clientData->validate(p->payload, p->payload_size, direction,
                                                        session, p, session->clientData->userData);
                    if (app_id_debug_session_flag)
                        _dpd.logMsg("AppIdDbg %s %s client detector returned %d\n", app_id_debug_session,
                                    session->clientData->name ? session->clientData->name:"UNKNOWN", ret);
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
                AppIdDiscoverClientApp(p, direction, session);
            }
            PREPROC_PROFILE_END(clientMatchPerfStats);
        }

        flow_mark(session, FLOW_ADDITIONAL_PACKET);

    }
    else
    {
#ifdef DEBUG_FW_APPID
        fprintf(SF_DEBUG_FILE, "Packet not okay\n");
#endif
        if (app_id_debug_session_flag && p->payload_size)
            _dpd.logMsg("AppIdDbg %s packet out-of-order\n", app_id_debug_session);
    }

#ifdef DEBUG_FW_APPID
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    if (p->dst_port == DEBUG_FW_APPID_PORT || p->src_port == DEBUG_FW_APPID_PORT)
    {
#endif
        fprintf(SF_DEBUG_FILE, "%u -> %u %u End %d %u - %d %d %d %u\n", (unsigned)p->src_port, (unsigned)p->dst_port,
                (unsigned)protocol, direction, (unsigned)p->payload_size,
                session->serviceAppId, session->clientAppId, session->payloadAppId,
                session->rnaServiceState);
        /*DumpHex(SF_DEBUG_FILE, p->payload, p->payload_size); */
#if defined(DEBUG_FW_APPID_PORT) && DEBUG_FW_APPID_PORT
    }
#endif
#endif
    {
    
        _dpd.streamAPI->set_application_id(p->stream_session, appGetSnortIdFromAppId(pickServiceAppId(session)), appGetSnortIdFromAppId(pickClientAppId(session)), 
                appGetSnortIdFromAppId(pickPayloadId(session)), appGetSnortIdFromAppId(pickMiscAppId(session)));
    }
}

void appSetServiceValidator(RNAServiceValidationFCN fcn, tAppId appId, unsigned extractsInfo, struct _Detector *data)
{
    AppInfoTableEntry *entry;

    if ((entry = getAppInfoEntry(appId)))
    {

        entry->flags |= APPINFO_FLAG_ACTIVE;

        extractsInfo &= (APPINFO_FLAG_SERVICE_ADDITIONAL | APPINFO_FLAG_SERVICE_UDP_REVERSED);
        if (!extractsInfo)
        {
            _dpd.debugMsg(DEBUG_LOG,"Ignoring direct service without info for %p %p with AppId %d\n",fcn, data, appId);
            return;
        }

        entry->flags |= extractsInfo;
    }
    else
    {
        _dpd.errMsg("Invalid direct service AppId, %d, for %p %p\n",appId, fcn, data);
    }
}

void appSetClientValidator(RNAClientAppFCN fcn, tAppId appId, unsigned extractsInfo, struct _Detector *data)
{
    AppInfoTableEntry* entry;

    if ((entry = getAppInfoEntry(appId)))
    {
        entry->flags |= APPINFO_FLAG_ACTIVE;
        extractsInfo &= (APPINFO_FLAG_CLIENT_ADDITIONAL | APPINFO_FLAG_CLIENT_USER);
        if (!extractsInfo)
        {
            _dpd.debugMsg(DEBUG_LOG,"Ignoring direct client application without info for %p %p with AppId %d\n",fcn, data, appId);
            return;
        }

        entry->flags |= extractsInfo;
    }
    else
    {
        _dpd.errMsg("Invalid direct client application AppId, %d, for %p %p\n",appId, fcn, data);
        return;
    }
}

void AppIdAddUser(FLOW *flowp, const char *username, tAppId appId, int success)
{
    return;
}

void AppIdAddPayload(FLOW *flow, tAppId payload_id)
{
    flow->payloadAppId = payload_id;
}

#ifdef FW_TRACKER_DEBUG
void logAppIdInfo(SFSnortPacket *p, char *message, tAppId id)
{
    if (!p || !message || id <= APP_ID_NONE)
        return;

    char source_addr[INET6_ADDRSTRLEN];
    char dest_addr[INET6_ADDRSTRLEN];
    snort_ip *ip;

    source_addr[0] = 0;
    ip = GET_SRC_IP(p);
    inet_ntop(ip->family, (void *)ip->ip32, source_addr, sizeof(source_addr));
    dest_addr[0] = 0;
    ip = GET_DST_IP(p);
    inet_ntop(ip->family, (void *)ip->ip32, dest_addr, sizeof(dest_addr));

    if ((_dpd.streamAPI->get_packet_direction(p) & FLAG_FROM_CLIENT))
        _dpd.logMsg( "%s:%s:%u:%u:%d:%s:%s",
                         source_addr,
                         dest_addr,
                         (unsigned)p->src_port,
                         (unsigned)p->dst_port,
                         id,
                         (IsTCP(p) ? "TCP" : "UDP"),
                         message);
    else
        _dpd.logMsg( "%s:%s:%u:%u:%d:%s:%s",
                         dest_addr,
                         source_addr,
                         (unsigned)p->dst_port,
                         (unsigned)p->src_port,
                         id,
                         (IsTCP(p) ? "TCP" : "UDP"),
                         message);
}
#endif

tAppId getOpenAppId(void *ssnptr)
{
    tAppIdData *session;
    tAppId *payloadAppId = APP_ID_NONE;
    if (ssnptr && (session = _dpd.sessionAPI->get_application_data(ssnptr, PP_APP_ID)))
    {
        payloadAppId = session->payloadAppId;
    }

    return payloadAppId;
}



