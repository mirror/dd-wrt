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


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <glob.h>
#include <str_search.h>

#include "flow.h"
#include "service_api.h"
#include "service_pattern.h"
#include "fw_appid.h"
#include "limits.h"

static int pattern_init(const InitServiceAPI * const init_api);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &pattern_validate,
    .detectorType = DETECTOR_TYPE_PATTERN,
    .name = "pattern",
    .ref_count = 1,
};

RNAServiceValidationModule pattern_service_mod =
{
    name: "pattern",
    init: &pattern_init,
    pp: NULL,
    is_custom: 1,
};

#define PATTERN_TCP_PROTO   1
#define PATTERN_UDP_PROTO   2
#define PATTERN_BOTH_PROTOS (PATTERN_TCP_PROTO | PATTERN_UDP_PROTO)

struct _PATTERN_SERVICE;
typedef struct _PATTERN
{
    struct _PATTERN *next;
    unsigned length;
    int offset;
    uint8_t *data;
    struct _PATTERN_SERVICE *ps;
} Pattern;

typedef struct _PORT
{
    struct _PORT *next;
    uint16_t port;
} Port;

/**list for pattern services. Each pattern service is unique for a given uuid. */
typedef struct _PATTERN_SERVICE
{
    struct _PATTERN_SERVICE *next;
    tAppId id;
    Pattern *pattern;
    Port *port;
    unsigned proto;
    unsigned count;
    unsigned longest;
} PatternService;

static PatternService *config;
static RNAServiceValidationPort pp =
{
    validate: &pattern_validate
};

static void *tcpPortPatternTree[65536];
static void *udpPortPatternTree[65536];
static void *tcp_patterns;
static unsigned tcp_count;
static void *udp_patterns;
static unsigned udp_count;

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

static void read_patterns(const InitServiceAPI * const init_api)
{
    PatternService *ps;
    Pattern *pattern;
    Port *port;
    int rval;
    glob_t globs;
    char file_pattern[PATH_MAX];
    unsigned n;
    FILE *file;
    char line[4096];

    snprintf(file_pattern, sizeof(file_pattern), "%s/pattern/*.yaml", init_api->csd_path);

    memset(&globs, 0, sizeof(globs));
    rval = glob(file_pattern, 0, NULL, &globs);
    if (rval != 0 && rval != GLOB_NOMATCH)
    {
        _dpd.errMsg("Unable to read directory '%s'\n",file_pattern);
        return;
    }

    for (n = 0; n < globs.gl_pathc; n++)
    {
        if ((file = fopen(globs.gl_pathv[n], "r")) == NULL)
        {
            _dpd.errMsg("Unable to read pattern detector '%s'\n",globs.gl_pathv[n]);
            continue;
        }

        if ((ps = calloc(1, sizeof(*ps))) == NULL)
        {
            _dpd.errMsg( "Failed to allocate a pattern");
            goto next;
        }
        ps->id = APP_ID_NONE;

        while (fgets(line, sizeof(line), file))
        {
            char *key, *value, *p;
            size_t len;

            len = strlen(line);
            for (; len && (line[len - 1] == '\n' || line[len - 1] == '\r'); len--)
                line[len - 1] = 0;

            /* find key/value for lines of the format "key: value\n" */
            if ((value = strchr(line, ':')))
            {
                key = line;
                *value = '\0';
                value++;
                for (; *value && *value == ' '; value++);

                if (strcasecmp(key, "appId") == 0)
                {
                    ps->id = (tAppId)strtol(value, &p, 10);
                    if (!*value || *p)
                    {
                        _dpd.errMsg("Invalid app ID, '%s', in pattern detector '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                    if (ps->id <= APP_ID_NONE)
                    {
                        _dpd.errMsg("Invalid app ID, '%s', in pattern detector '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                }
                else if (strcasecmp(key, "protocol") == 0)
                {
                    if (strcasecmp(value, "tcp") == 0)
                        ps->proto = PATTERN_TCP_PROTO;
                    else if (strcasecmp(value, "udp") == 0)
                        ps->proto = PATTERN_UDP_PROTO;
                    else if (strcasecmp(value, "tcp/udp") == 0)
                        ps->proto = PATTERN_BOTH_PROTOS;
                    else
                    {
                        _dpd.errMsg("Invalid protocol, '%s', in pattern service '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                }
                else if (strcasecmp(key, "ports") == 0)
                {
                    char *context = NULL;
                    char *ptr;
                    unsigned long tmp;

                    for (ptr = strtok_r(value, ",", &context); ptr; ptr = strtok_r(NULL, ",", &context))
                    {
                        for (; *ptr && *ptr == ' '; ptr++);
                        len = strlen(ptr);
                        for (; len && ptr[len - 1] == ' '; len--)
                            ptr[len - 1] = 0;
                        tmp = strtoul(ptr, &p, 10);
                        if (!*ptr || *p || !tmp || tmp > 65535)
                        {
                            _dpd.errMsg("Invalid port, '%s', in pattern detector '%s'\n",ptr, globs.gl_pathv[n]);
                            goto next;
                        }
                        if ((port = calloc(1, sizeof(*port))) == NULL)
                        {
                            _dpd.errMsg( "Failed to allocate a port struct");
                            goto next;
                        }
                        port->port = (uint16_t)tmp;
                        port->next = ps->port;
                        ps->port = port;
                    }
                }
                else if (strcasecmp(key, "pattern") == 0)
                {
                    static const char PATTERN_HEX[] = "HEX";
                    static const char PATTERN_ASCII[] = "ASCII";
                    char *context = NULL;
                    char *ptr;
                    int decode;
                    int offset;

                    ptr = strtok_r(value, ",", &context);
                    if (!ptr)
                    {
                        _dpd.errMsg("Invalid pattern in pattern detector '%s'\n",globs.gl_pathv[n]);
                        goto next;
                    }
                    if (strncasecmp(ptr, PATTERN_HEX, sizeof(PATTERN_HEX) - 1) == 0)
                        decode = 1;
                    else if (strncasecmp(ptr, PATTERN_ASCII, sizeof(PATTERN_ASCII) - 1) == 0)
                        decode = 2;
                    else
                    {
                        _dpd.errMsg("Invalid pattern in pattern detector '%s'\n",globs.gl_pathv[n]);
                        goto next;
                    }
                    ptr = strtok_r(NULL, ",", &context);
                    if (!ptr)
                    {
                        _dpd.errMsg("Invalid pattern in pattern detector '%s'\n",globs.gl_pathv[n]);
                        goto next;
                    }
                    for (; *ptr && *ptr == ' '; ptr++);
                    len = strlen(ptr);
                    for (; len && ptr[len - 1] == ' '; len--)
                        ptr[len - 1] = 0;
                    offset = (int)strtol(ptr, &p, 10);
                    if (!*ptr || *p)
                    {
                        _dpd.errMsg("Invalid offset, '%s', in pattern detector '%s'\n",ptr, globs.gl_pathv[n]);
                        goto next;
                    }
                    ptr = strtok_r(NULL, ",", &context);
                    if (!ptr)
                    {
                        _dpd.errMsg("Invalid pattern, '%s', in pattern detector '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                    for (; *ptr && *ptr == ' '; ptr++);
                    len = strlen(ptr);
                    if (!len || (decode == 1 && (len & 1)))
                    {
                        _dpd.errMsg("Invalid pattern data length in pattern detector '%s'\n",globs.gl_pathv[n]);
                        goto next;
                    }
                    if ((pattern = calloc(1, sizeof(*pattern))) == NULL)
                    {
                        _dpd.errMsg( "Failed to allocate a pattern struct");
                        goto next;
                    }
                    if (decode == 1)
                        len >>= 1;
                    if ((pattern->data = malloc(len)) == NULL)
                    {
                        FreePattern(pattern);
                        _dpd.errMsg( "Failed to allocate a %u byte pattern in pattern detector '%s'",
                               (unsigned)len, globs.gl_pathv[n]);
                        goto next;
                    }
                    if (decode == 1)
                    {
                        unsigned i;
                        char str[5];
                        unsigned long tmp;

                        str[0] = '0';
                        str[1] = 'x';
                        str[4] = 0;
                        for (i = 0; i < len; i++)
                        {
                            str[2] = *ptr;
                            ptr++;
                            str[3] = *ptr;
                            ptr++;
                            tmp = strtoul(str, &p, 16);
                            if (*p || tmp > 255)
                            {
                                FreePattern(pattern);
                                _dpd.errMsg("Invalid pattern data in pattern detector '%s'\n",globs.gl_pathv[n]);
                                goto next;
                            }
                            pattern->data[i] = (uint8_t)tmp;
                        }
                    }
                    else
                        memcpy(pattern->data, ptr, len);

                    pattern->length = len;
                    if (pattern->length > ps->longest)
                        ps->longest = pattern->length;
                    pattern->ps = ps;
                    pattern->offset = offset;
                    pattern->next = ps->pattern;
                    ps->pattern = pattern;
                }
            }
            else
            {
                _dpd.errMsg("Invalid line in pattern service '%s'\n",globs.gl_pathv[n]);
                goto next;
            }
        }

        if (ps->id <= APP_ID_NONE || !ps->pattern || !ps->proto)
        {
            _dpd.errMsg("Missing parameter(s) in pattern service '%s'\n",globs.gl_pathv[n]);
            goto next;
        }

        ps->next = config;
        config = ps;
        if (init_api->instance_id)
            *init_api->service_instance = ps->id;
        ps = NULL;

next:;
        if (ps)
            FreePatternService(ps);
        fclose(file);
    }

    globfree(&globs);
}

/**Register ports for detectors which have a pattern associated with it.
 */
static void install_ports(const InitServiceAPI * const init_api)
{
    PatternService *ps;
    Port *port;

    for (ps = config; ps; ps = ps->next)
    {
        if (!ps->port)
            continue;

        for (port = ps->port; port; port = port->next)
        {
            pp.port = port->port;
            if (ps->proto & PATTERN_TCP_PROTO)
            {
                pp.proto = IPPROTO_TCP;
                if (init_api->AddPort(&pp, &pattern_service_mod))
                    _dpd.errMsg("Failed to add port - %d:%u:%d\n",ps->id, (unsigned)port->port, IPPROTO_TCP);
                else
                    _dpd.debugMsg(DEBUG_LOG,"Installed ports - %d:%u:%d\n",ps->id, (unsigned)port->port, IPPROTO_TCP);
            }
            if (ps->proto & PATTERN_UDP_PROTO)
            {
                pp.proto = IPPROTO_UDP;
                if (init_api->AddPort(&pp, &pattern_service_mod))
                    _dpd.errMsg("Failed to add port - %d:%u:%d\n",ps->id, (unsigned)port->port, IPPROTO_TCP);
                else
                    _dpd.debugMsg(DEBUG_LOG,"Installed ports - %d:%u:%d\n",ps->id, (unsigned)port->port, IPPROTO_TCP);
            }
        }
    }
}

static void RegisterPattern(PatternService *ps, void **patterns,
                            Pattern *pattern, unsigned *count)
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
    (*count)++;
}

/**Creates unique subset of services registered on  ports, and then creates pattern trees.
 */
static void createPatternTrees(
        const InitServiceAPI * const init_api
        )
{
    PatternService *ps;
    Pattern *pattern;
    Port *port;
    unsigned dummyCount = 0;
    unsigned i;

    for (ps = config; ps; ps = ps->next)
    {
        for (port = ps->port; port; port = port->next)
        {
            for (pattern = ps->pattern; pattern; pattern = pattern->next)
            {
                if (ps->proto & PATTERN_TCP_PROTO)
                    RegisterPattern(ps, &tcpPortPatternTree[port->port], pattern, &dummyCount);
                if (ps->proto & PATTERN_UDP_PROTO)
                    RegisterPattern(ps, &udpPortPatternTree[port->port], pattern, &dummyCount);
            }
        }
    }
    for (i = 0; i < 65536; i++)
    {
        if (tcpPortPatternTree[i])
        {
            for (ps = config; ps; ps = ps->next)
            {
                if (ps->port || !(ps->proto & PATTERN_TCP_PROTO))
                    continue;

                for (pattern = ps->pattern; pattern; pattern = pattern->next)
                    RegisterPattern(ps, &tcpPortPatternTree[i], pattern, &dummyCount);

            }

            _dpd.searchAPI->search_instance_prep(tcpPortPatternTree[i]);
        }
        if (udpPortPatternTree[i])
        {
            for (ps = config; ps; ps = ps->next)
            {
                if (ps->port || !(ps->proto & PATTERN_UDP_PROTO))
                    continue;

                for (pattern = ps->pattern; pattern; pattern = pattern->next)
                    RegisterPattern(ps, &udpPortPatternTree[i], pattern, &dummyCount);

            }

            _dpd.searchAPI->search_instance_prep(udpPortPatternTree[i]);
        }
    }
}

static int pattern_init(const InitServiceAPI * const init_api)
{
    PatternService *ps;
    Pattern *pattern;

    if (config)
    {
        unsigned i;

        if (tcp_patterns)
            _dpd.searchAPI->search_instance_free(tcp_patterns);
        tcp_patterns = NULL;
        tcp_count = 0;
        if (udp_patterns)
            _dpd.searchAPI->search_instance_free(udp_patterns);
        udp_patterns = NULL;
        udp_count = 0;
        for (i = 0; i < 65536; i++)
        {
            if (tcpPortPatternTree[i])
            {
                _dpd.searchAPI->search_instance_free(tcpPortPatternTree[i]);
                tcpPortPatternTree[i] = NULL;
            }
            if (udpPortPatternTree[i])
            {
                _dpd.searchAPI->search_instance_free(udpPortPatternTree[i]);
                udpPortPatternTree[i] = NULL;
            }
        }
        init_api->RemovePorts(&pattern_validate);
        while (config)
        {
            ps = config;
            config = ps->next;
            FreePatternService(ps);
        }
    }

    _dpd.debugMsg(DEBUG_LOG,"Initializing with instance %u\n",init_api->instance_id);
    read_patterns(init_api);

    install_ports(init_api);

    createPatternTrees(init_api);

    /**Register patterns with no associated ports, to RNA and local
     * pattern tree. Register patterns with ports with local pattern
     * tree only.
     */
    for (ps = config; ps; ps = ps->next)
    {
        if (!ps->port)
        {
            for (pattern = ps->pattern; pattern; pattern = pattern->next)
            {
                if (pattern->data && pattern->length)
                {
                    if (ps->proto & PATTERN_UDP_PROTO)
                    {
                        _dpd.debugMsg(DEBUG_LOG,"Adding pattern with length %u\n",pattern->length);
                        init_api->RegisterPattern(&pattern_validate, IPPROTO_UDP, pattern->data, pattern->length,
                                                  pattern->offset, "pattern");
                        RegisterPattern(ps, &udp_patterns, pattern, &udp_count);
                    }
                    if (ps->proto & PATTERN_TCP_PROTO)
                    {
                        _dpd.debugMsg(DEBUG_LOG,"Adding pattern with length %u\n",pattern->length);
                        init_api->RegisterPattern(&pattern_validate, IPPROTO_TCP, pattern->data, pattern->length,
                                                  pattern->offset, "pattern");
                        RegisterPattern(ps, &tcp_patterns, pattern, &tcp_count);
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
    if (tcp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(tcp_patterns);
    }
    if (udp_patterns)
    {
        _dpd.searchAPI->search_instance_prep(udp_patterns);
    }
    return 0;
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

int csdPatternTreeSearch(const uint8_t *data, uint16_t size, int protocol, const SFSnortPacket *pkt,
                         const RNAServiceElement** serviceData)
{
    void *patternTree;
    PatternService *ps;
    void *patterns;
    PServiceMatch *matches = NULL;
    PServiceMatch *sm;
    PServiceMatch *psm;
    Pattern *pattern;

    if (!data || !pkt || !size)
        return 0;

    *serviceData = NULL;

    if (protocol == IPPROTO_UDP)
        patternTree = udpPortPatternTree[pkt->src_port];
    else
        patternTree = tcpPortPatternTree[pkt->src_port];

    if (patternTree)
    {
        _dpd.searchAPI->search_instance_find_all(patternTree,
                   (char *)data,
                   size, 0,
                   pattern_match, (void*)&matches);
    }
    else
    {
        if (protocol == IPPROTO_UDP)
            patterns = udp_patterns;
        else
            patterns = tcp_patterns;
        if (patterns)
        {
            _dpd.searchAPI->search_instance_find_all(patterns,
                       (char *)data,
                       size, 0,
                       pattern_match, (void*)&matches);
        }
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

int pattern_validate(const uint8_t *data, uint16_t size, const int dir,
                     FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userdata)
{
    uint32_t id;
    const RNAServiceElement *service = NULL;

    if (!data || !pattern_service_mod.api || !flowp || !pkt)
        return SERVICE_ENULL;
    if (!size)
        goto inprocess;
    if (dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    id = csdPatternTreeSearch(data, size, flowp->proto, pkt, &service);
    if (!id) goto fail;

    pattern_service_mod.api->add_service(flowp, pkt, dir, &svc_element, id, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

inprocess:
    pattern_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
    return SERVICE_INPROCESS;

fail:
    pattern_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
    return SERVICE_NOMATCH;
}

