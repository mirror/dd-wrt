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


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <glob.h>
#include <str_search.h>

#include "appInfoTable.h"
#include "flow.h"
#include "service_api.h"
#include "client_app_base.h"
#include "detector_pattern.h"
#include "fw_appid.h"
#include "limits.h"

static int service_validate(ServiceValidationArgs* args);
static int csdPatternTreeSearch(const uint8_t *data, uint16_t size, int protocol, SFSnortPacket *pkt,
                                const tRNAServiceElement** serviceData, bool isClient,
                                const tAppIdConfig *pConfig);
static int pattern_service_init(const InitServiceAPI * const initServiceApi);
static void pattern_service_clean(const CleanServiceAPI * const clean_api);
static CLIENT_APP_RETCODE client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE client_init_tcp(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const tAppIdConfig *pConfig);
static void client_clean(const CleanClientAppAPI * const clean_api);
static const InitServiceAPI *initServiceApi;
static const InitClientAppAPI *initClientApi;

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &service_validate,
    .detectorType = DETECTOR_TYPE_PATTERN,
    .name = "pattern",
    .ref_count = 1,
    .current_ref_count = 1
};

tRNAServiceValidationModule pattern_service_mod =
{
    name: "pattern",
    init: &pattern_service_init,
    pp: NULL,
    clean: &pattern_service_clean
};

//client side
tRNAClientAppModule pattern_udp_client_mod =
{
    .name = "pattern",
    .proto = IPPROTO_UDP,
    .init = &client_init,
    .clean = &client_clean,
    .validate = &client_validate,
};
tRNAClientAppModule pattern_tcp_client_mod =
{
    .name = "pattern",
    .proto = IPPROTO_TCP,
    .init = &client_init_tcp,
    .validate = &client_validate,
};

#define PATTERN_TCP_PROTO   1
#define PATTERN_UDP_PROTO   2
#define PATTERN_BOTH_PROTOS (PATTERN_TCP_PROTO | PATTERN_UDP_PROTO)


static RNAServiceValidationPort pp =
{
    validate: &service_validate
};


static void FreePattern(Pattern *pattern)
{
    if (pattern)
    {
        if (pattern->data)
            free(pattern->data);
        free(pattern);
    }
}

static void FreePatternService(PatternService *ps)
{
    Pattern *pattern;
    Port *port;

    if (ps)
    {
        while ((pattern = ps->pattern))
        {
            ps->pattern = pattern->next;
            FreePattern(pattern);
        }
        while ((port = ps->port))
        {
            ps->port = port->next;
            free(port);
        }
        free(ps);
    }
}
static void read_patterns(tPortPatternNode *portPatternList, PatternService **serviceList)
{
    PatternService *ps = NULL;
    Pattern *pattern;
    Port *port;
    tPortPatternNode *pNode;
    char *lastName = NULL;
    short lastPort = 0;
    uint8_t lastProto = 0;
    bool newPs;

    for (pNode = portPatternList;
            pNode;
            pNode = pNode->next)
    {
        newPs = false;
        if (!ps || !lastName || strcmp(lastName, pNode->detectorName) || lastProto != pNode->protocol)
        {
            if ((ps = calloc(1, sizeof(*ps))) == NULL)
            {
                _dpd.errMsg( "Failed to allocate a pattern");
                return;
            }

            lastName = pNode->detectorName;
            lastProto = pNode->protocol;
            newPs = true;

            ps->id = pNode->appId;
            ps->proto = pNode->protocol;
            ps->next = *serviceList;
            *serviceList = ps;
        }

        if (pNode->port && (newPs || lastPort != pNode->port))
        {
            if ((port = calloc(1, sizeof(*port))) == NULL)
            {
                _dpd.errMsg( "Failed to allocate a port struct");
                return;
            }
            port->port = pNode->port;
            port->next = ps->port;
            lastPort = pNode->port;
            ps->port = port;
        }
        if ((pattern = calloc(1, sizeof(*pattern))) == NULL)
        {
            _dpd.errMsg( "Failed to allocate a pattern struct");
            return;
        }
        if ((pattern->data = malloc(pNode->length)) == NULL)
        {
            FreePattern(pattern);
            _dpd.errMsg( "Failed to allocate a %u byte pattern in pattern detector '%s'",
                    (unsigned)pNode->length, pNode->detectorName);
            return;
        }
        memcpy(pattern->data, pNode->pattern, pNode->length);
        pattern->length = pNode->length;
        if (pattern->length > ps->longest)
            ps->longest = pattern->length;
        pattern->ps = ps;
        pattern->offset = pNode->offset;
        pattern->next = ps->pattern;
        ps->pattern = pattern;

        appInfoSetActive(ps->id, true);
    }
}

/**Register ports for detectors which have a pattern associated with it.
 */
static void install_ports(PatternService *serviceList, const InitServiceAPI * const initServiceApi)
{
    PatternService *ps;
    Port *port;

    for (ps = serviceList; ps; ps = ps->next)
    {
        if (!ps->port)
            continue;

        for (port = ps->port; port; port = port->next)
        {
            pp.port = port->port;
            pp.proto = ps->proto;
            if (initServiceApi->AddPort(&pp, &pattern_service_mod, initServiceApi->pAppidConfig))
                _dpd.errMsg("Failed to add port - %d:%u:%d\n",ps->id, (unsigned)pp.port, pp.proto);
            else
                _dpd.debugMsg(DEBUG_LOG,"Installed ports - %d:%u:%d\n",ps->id, (unsigned)pp.port, pp.proto);
        }
    }
}

static void RegisterPattern(void **patterns, Pattern *pattern)
{
    if (!*patterns)
    {
        *patterns = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF);
        if (!*patterns)
        {
            _dpd.errMsg( "Error initializing the pattern table");
            return;
        }
    }

    _dpd.searchAPI->search_instance_add_ex(*patterns, (char *)pattern->data, pattern->length, pattern, STR_SEARCH_CASE_SENSITIVE);
}

/**Creates unique subset of services registered on  ports, and then creates pattern trees.
 */
static void createServicePatternTrees(
        tAppIdConfig *pConfig
        )
{
    PatternService *ps;
    Pattern *pattern;
    Port *port;
    unsigned i;

    for (ps = pConfig->servicePortPattern->servicePortPattern; ps; ps = ps->next)
    {
        for (port = ps->port; port; port = port->next)
        {
            for (pattern = ps->pattern; pattern; pattern = pattern->next)
            {
                if (ps->proto == IPPROTO_TCP)
                    RegisterPattern(&pConfig->servicePortPattern->tcpPortPatternTree[port->port], pattern);
                else
                    RegisterPattern(&pConfig->servicePortPattern->udpPortPatternTree[port->port], pattern);
            }
        }
    }
    for (i = 0; i < 65536; i++)
    {
        if (pConfig->servicePortPattern->tcpPortPatternTree[i])
        {
            for (ps = pConfig->servicePortPattern->servicePortPattern; ps; ps = ps->next)
            {
                if (ps->port || (ps->proto != IPPROTO_TCP))
                    continue;

                for (pattern = ps->pattern; pattern; pattern = pattern->next)
                    RegisterPattern(&pConfig->servicePortPattern->tcpPortPatternTree[i], pattern);

            }

            _dpd.searchAPI->search_instance_prep(pConfig->servicePortPattern->tcpPortPatternTree[i]);
        }
        if (pConfig->servicePortPattern->udpPortPatternTree[i])
        {
            for (ps = pConfig->servicePortPattern->servicePortPattern; ps; ps = ps->next)
            {
                if (ps->port || (ps->proto != IPPROTO_UDP))
                    continue;

                for (pattern = ps->pattern; pattern; pattern = pattern->next)
                    RegisterPattern(&pConfig->servicePortPattern->udpPortPatternTree[i], pattern);

            }

            _dpd.searchAPI->search_instance_prep(pConfig->servicePortPattern->udpPortPatternTree[i]);
        }
    }

}

static void createClientPatternTrees(
        tAppIdConfig *pConfig
        )
{
    PatternService *ps;
    Pattern *pattern;

    for (ps = pConfig->clientPortPattern->servicePortPattern; ps; ps = ps->next)
    {
        for (pattern = ps->pattern; pattern; pattern = pattern->next)
        {
            if (ps->proto == IPPROTO_TCP)
                RegisterPattern(&pConfig->clientPortPattern->tcp_patterns, pattern);
            else
                RegisterPattern(&pConfig->clientPortPattern->udp_patterns, pattern);
        }
    }
}

void registerServicePatterns(
        tAppIdConfig *pConfig
        )
{
    PatternService *ps;
    Pattern *pattern;

    /**Register patterns with no associated ports, to RNA and local
     * pattern tree. Register patterns with ports with local pattern
     * tree only.
     */
    for (ps = pConfig->servicePortPattern->servicePortPattern; ps; ps = ps->next)
    {
        if (!ps->port)
        {
            for (pattern = ps->pattern; pattern; pattern = pattern->next)
            {
                if (pattern->data && pattern->length)
                {
                    if (ps->proto == IPPROTO_TCP)
                    {
                        _dpd.debugMsg(DEBUG_LOG,"Adding pattern with length %u\n",pattern->length);
                        initServiceApi->RegisterPattern(&service_validate, IPPROTO_TCP, pattern->data, pattern->length,
                                                  pattern->offset, "pattern", initServiceApi->pAppidConfig);
                        RegisterPattern(&pConfig->servicePortPattern->tcp_patterns, pattern);
                    }
                    else
                    {
                        _dpd.debugMsg(DEBUG_LOG,"Adding pattern with length %u\n",pattern->length);
                        initServiceApi->RegisterPattern(&service_validate, IPPROTO_UDP, pattern->data, pattern->length,
                                                  pattern->offset, "pattern", initServiceApi->pAppidConfig);
                        RegisterPattern(&pConfig->servicePortPattern->udp_patterns, pattern);
                    }
                }
            }
        }
        else
        {
            for (pattern = ps->pattern; pattern; pattern = pattern->next)
                ps->count++;
        }
    }
    if (pConfig->servicePortPattern->tcp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(pConfig->servicePortPattern->tcp_patterns);
    }
    if (pConfig->servicePortPattern->udp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(pConfig->servicePortPattern->udp_patterns);
    }

}
void registerClientPatterns(
        tAppIdConfig *pConfig
        )
{
    PatternService *ps;
    Pattern *pattern;

    /**Register patterns with no associated ports, to RNA and local
     * pattern tree. Register patterns with ports with local pattern
     * tree only.
     */
    for (ps = pConfig->clientPortPattern->servicePortPattern; ps; ps = ps->next)
    {
        for (pattern = ps->pattern; pattern; pattern = pattern->next)
        {
            if (pattern->data && pattern->length)
            {
                if (ps->proto == IPPROTO_TCP)
                {
                    _dpd.debugMsg(DEBUG_LOG,"Adding pattern with length %u\n",pattern->length);
                    initClientApi->RegisterPattern(&client_validate, IPPROTO_TCP, pattern->data, pattern->length,
                            pattern->offset, initClientApi->pAppidConfig);
                    RegisterPattern(&pConfig->clientPortPattern->tcp_patterns, pattern);
                }
                else
                {
                    _dpd.debugMsg(DEBUG_LOG,"Adding pattern with length %u\n",pattern->length);
                    initClientApi->RegisterPattern(&client_validate, IPPROTO_UDP, pattern->data, pattern->length,
                            pattern->offset, initClientApi->pAppidConfig);
                    RegisterPattern(&pConfig->clientPortPattern->udp_patterns, pattern);
                }
            }
            ps->count++;
        }
    }
    if (pConfig->clientPortPattern->tcp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(pConfig->clientPortPattern->tcp_patterns);
    }
    if (pConfig->clientPortPattern->udp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(pConfig->clientPortPattern->udp_patterns);
    }
}
void dumpPatterns(char *name, PatternService *pList)
{
    PatternService *ps;
    Pattern *pattern;

    /**Register patterns with no associated ports, to RNA and local
     * pattern tree. Register patterns with ports with local pattern
     * tree only.
     */

    _dpd.debugMsg(DEBUG_LOG,"Adding pattern for \"%s\"\n",name);
    for (ps = pList; ps; ps = ps->next)
    {
        for (pattern = ps->pattern; pattern; pattern = pattern->next)
        {
            _dpd.debugMsg(DEBUG_LOG,"\t%s, %d\n",pattern->data, pattern->length);
            if (pattern->data && pattern->length)
            {
                _dpd.debugMsg(DEBUG_LOG,"\t\t%s, %d\n",pattern->data, pattern->length);
            }
        }
    }
}

int portPatternFinalize(tAppIdConfig *pConfig)
{
    if (pConfig->clientPortPattern)
    {
        read_patterns(pConfig->clientPortPattern->luaInjectedPatterns, &pConfig->clientPortPattern->servicePortPattern);
        createClientPatternTrees(pConfig);
        registerClientPatterns(pConfig);
        dumpPatterns("Client", pConfig->clientPortPattern->servicePortPattern);
    }
    if (pConfig->servicePortPattern)
    {
        read_patterns(pConfig->servicePortPattern->luaInjectedPatterns, &pConfig->servicePortPattern->servicePortPattern);
        install_ports(pConfig->servicePortPattern->servicePortPattern, initServiceApi);
        createServicePatternTrees(pConfig);
        registerServicePatterns(pConfig);
        dumpPatterns("Server", pConfig->servicePortPattern->servicePortPattern);
    }

    return 0;
}

static int pattern_service_init(const InitServiceAPI * const init_api)
{
    initServiceApi = init_api;

    _dpd.debugMsg(DEBUG_LOG,"Initializing with instance %u\n",initServiceApi->instance_id);

    return 0;
}

static void pattern_service_clean(const CleanServiceAPI * const clean_api)
{
    PatternService *ps;
    tAppIdConfig *pConfig = clean_api->pAppidConfig;

    if (pConfig->servicePortPattern && pConfig->servicePortPattern->servicePortPattern)
    {
        unsigned i;

        if (pConfig->servicePortPattern->tcp_patterns)
            _dpd.searchAPI->search_instance_free(pConfig->servicePortPattern->tcp_patterns);
        pConfig->servicePortPattern->tcp_patterns = NULL;
        if (pConfig->servicePortPattern->udp_patterns)
            _dpd.searchAPI->search_instance_free(pConfig->servicePortPattern->udp_patterns);
        pConfig->servicePortPattern->udp_patterns = NULL;
        for (i = 0; i < 65536; i++)
        {
            if (pConfig->servicePortPattern->tcpPortPatternTree[i])
            {
                _dpd.searchAPI->search_instance_free(pConfig->servicePortPattern->tcpPortPatternTree[i]);
                pConfig->servicePortPattern->tcpPortPatternTree[i] = NULL;
            }
            if (pConfig->servicePortPattern->udpPortPatternTree[i])
            {
                _dpd.searchAPI->search_instance_free(pConfig->servicePortPattern->udpPortPatternTree[i]);
                pConfig->servicePortPattern->udpPortPatternTree[i] = NULL;
            }
        }
        while (pConfig->servicePortPattern->servicePortPattern)
        {
            ps = pConfig->servicePortPattern->servicePortPattern;
            pConfig->servicePortPattern->servicePortPattern = ps->next;
            FreePatternService(ps);
        }
    }
}

typedef struct _P_SERVICE_MATCH
{
    /**Matches are aggregated by PatternService first and then by patterns. next is used to walk matches by PatternService*/
    struct _P_SERVICE_MATCH *next;

    /**Walks matches by pattern within a PatternService. */
    struct _P_SERVICE_MATCH *ps_next;

    Pattern *data;
} PServiceMatch;

static PServiceMatch *free_servicematch_list;

static int pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    PServiceMatch **matches = (PServiceMatch **)data;
    Pattern *pd = (Pattern *)id;
    PServiceMatch *psm;
    PServiceMatch *sm;

    if (pd->offset >= 0 && pd->offset != index)
        return 0;

    /*find if previously this PS was matched. */
    for (psm=*matches; psm; psm=psm->next)
        if (psm->data->ps == pd->ps)
            break;

    if (psm)
    {
        /*walks patterns within a PatternService. */
        for (sm=psm; sm; sm=sm->ps_next)
            if (sm->data == pd)
                return 0;

        if (free_servicematch_list)
        {
            sm = free_servicematch_list;
            free_servicematch_list = sm->next;
            memset(sm, 0, sizeof(*sm));
        }
        else if ((sm=calloc(1, sizeof(*sm))) == NULL)
        {
            _dpd.errMsg( "Failed to allocate a service match");
            return 0;
        }
        sm->data = pd;
        sm->ps_next = psm->ps_next;
        psm->ps_next = sm;
        return 0;
    }
    else if (free_servicematch_list)
    {
        sm = free_servicematch_list;
        free_servicematch_list = sm->next;
        memset(sm, 0, sizeof(*sm));
    }
    else if ((sm = calloc(1, sizeof(*sm))) == NULL)
    {
        _dpd.errMsg( "Failed to allocate a service match");
        return 0;
    }
    sm->data = pd;
    sm->next = *matches;
    *matches = sm;
    return 0;
}

static int csdPatternTreeSearch(const uint8_t *data, uint16_t size, int protocol, SFSnortPacket *pkt,
                                const tRNAServiceElement** serviceData, bool isClient,
                                const tAppIdConfig *pConfig)
{
    void *patternTree = NULL;
    PatternService *ps;
    PServiceMatch *matches = NULL;
    PServiceMatch *sm;
    PServiceMatch *psm;
    Pattern *pattern;

    if (!data || !pkt || !size)
        return 0;

    *serviceData = NULL;

    if (!isClient)
    {
        if (protocol == IPPROTO_UDP)
            patternTree = pConfig->servicePortPattern->udpPortPatternTree[pkt->src_port];
        else
            patternTree = pConfig->servicePortPattern->tcpPortPatternTree[pkt->src_port];

    }

    if (!patternTree)
    {
        if (protocol == IPPROTO_UDP)
            patternTree = (isClient)? pConfig->clientPortPattern->udp_patterns: pConfig->servicePortPattern->udp_patterns;
        else
            patternTree = (isClient)? pConfig->clientPortPattern->tcp_patterns: pConfig->servicePortPattern->tcp_patterns;
    }

    if (patternTree)
    {
        _dpd.searchAPI->search_instance_find_all(patternTree,
                (char *)data,
                size, 0,
                pattern_match, (void*)&matches);
    }

    if (matches == NULL)
        return 0;

    /*match highest count and then longest pattern. */
    ps = NULL;
    for (sm = matches; sm; sm = sm->next)
    {
        /*walk all patterns in PatternService */
        for (pattern = sm->data->ps->pattern; pattern; pattern = pattern->next)
        {
            for (psm = sm; psm; psm = psm->ps_next)
                if (pattern == psm->data)
                    break;
            if (psm == NULL)
                break;
        }

        if (pattern == NULL)    /*all patterns in PatternService were matched */
        {
            if (ps)
            {
                if (sm->data->ps->count > ps->count)
                    ps = sm->data->ps;
                else if (sm->data->ps->count == ps->count && sm->data->ps->longest > ps->longest)
                    ps = sm->data->ps;
            }
            else ps = sm->data->ps;
        }
    }

    /*free match list */
    while (matches)
    {
        while (matches->ps_next)
        {
            sm = matches->ps_next;
            matches->ps_next = sm->ps_next;
            sm->next = free_servicematch_list;
            free_servicematch_list = sm;
        }
        sm = matches;
        matches = sm->next;
        sm->next = free_servicematch_list;
        free_servicematch_list = sm;
    }

    if (ps == NULL) return 0;
    *serviceData = &svc_element;
    return ps->id;
}

static int service_validate(ServiceValidationArgs* args)
{
    uint32_t id;
    const tRNAServiceElement *service = NULL;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    SFSnortPacket *pkt = args->pkt; 
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!data || !pattern_service_mod.api || !flowp || !pkt)
        return SERVICE_ENULL;
    if (!size)
        goto inprocess;
    if (dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    id = csdPatternTreeSearch(data, size, flowp->proto, pkt, &service, false, args->pConfig);
    if (!id) goto fail;

    pattern_service_mod.api->add_service(flowp, pkt, dir, &svc_element, id, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

inprocess:
    pattern_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

fail:
    pattern_service_mod.api->fail_service(flowp, pkt, dir, &svc_element,
                                          pattern_service_mod.flow_data_index,
                                          args->pConfig, NULL);
    return SERVICE_NOMATCH;
}


static CLIENT_APP_RETCODE client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    initClientApi = init_api;

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE client_init_tcp(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    return CLIENT_APP_SUCCESS;
}

static void client_clean(const CleanClientAppAPI * const clean_api)
{
    tAppIdConfig *pConfig = clean_api->pAppidConfig;

    if (pConfig->clientPortPattern && pConfig->clientPortPattern->servicePortPattern)
    {
        if (pConfig->clientPortPattern->tcp_patterns)
            _dpd.searchAPI->search_instance_free(pConfig->clientPortPattern->tcp_patterns);
        pConfig->clientPortPattern->tcp_patterns = NULL;

        if (pConfig->clientPortPattern->udp_patterns)
            _dpd.searchAPI->search_instance_free(pConfig->clientPortPattern->udp_patterns);
        pConfig->clientPortPattern->udp_patterns = NULL;
    }
}

static CLIENT_APP_RETCODE client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const tAppIdConfig *pConfig)
{
    tAppId id;
    const tRNAServiceElement *service = NULL;

    if (!data || !flowp || !pkt)
        return CLIENT_APP_ENULL;
    if (!size)
        goto inprocess;
    if (dir == APP_ID_FROM_RESPONDER)
        goto inprocess;

    id = csdPatternTreeSearch(data, size, flowp->proto, pkt, &service, true, (tAppIdConfig *)pConfig);
    if (!id) goto fail;

    pattern_tcp_client_mod.api->add_app(pkt, dir, pConfig, flowp, id, id, NULL);
    return CLIENT_APP_SUCCESS;

inprocess:
    return CLIENT_APP_INPROCESS;

fail:
    return CLIENT_APP_EINVALID;
}



