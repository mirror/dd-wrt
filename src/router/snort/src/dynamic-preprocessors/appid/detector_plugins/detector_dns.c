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

#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>

#include "appInfoTable.h"
#include "service_base.h"
#include "detector_dns.h"
#include "service_api.h"
#include "str_search.h"
#include "client_app_base.h"
#include "client_app_api.h"
#include "dns_defs.h"

typedef enum
{
    DNS_STATE_QUERY,
    DNS_STATE_RESPONSE
} DNSState;

typedef struct _SERVICE_DNS_DATA
{
    DNSState state;
    uint16_t id;
} ServiceDNSData;

typedef struct _MatchedDNSPatterns {
    DNSHostPattern *mpattern;
    int index;
    struct _MatchedDNSPatterns *next;
} MatchedDNSPatterns;

static int dns_service_init(const InitServiceAPI * const init_api);
static int dns_udp_validate(ServiceValidationArgs* args);
static int dns_tcp_validate(ServiceValidationArgs* args);

static tRNAServiceElement udp_svc_element =
{
    .next = NULL,
    .validate = &dns_udp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "dns",
    .ref_count = 1,
    .current_ref_count = 1,
};
static tRNAServiceElement tcp_svc_element =
{
    .next = NULL,
    .validate = &dns_tcp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "tcp dns",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&dns_tcp_validate, 53, IPPROTO_TCP},
    {&dns_udp_validate, 53, IPPROTO_UDP},
    {&dns_udp_validate, 53, IPPROTO_UDP, 1},
    {&dns_tcp_validate, 5300, IPPROTO_TCP},
    {&dns_udp_validate, 5300, IPPROTO_UDP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule dns_service_mod =
{
    "DNS",
    &dns_service_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_DNS, APPINFO_FLAG_SERVICE_UDP_REVERSED | APPINFO_FLAG_SERVICE_ADDITIONAL}
};

static CLIENT_APP_RETCODE dns_udp_client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE dns_tcp_client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE dns_udp_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const tAppIdConfig *pConfig);
static CLIENT_APP_RETCODE dns_tcp_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const tAppIdConfig *pConfig);

SF_SO_PUBLIC tRNAClientAppModule dns_udp_client_mod =
{
    .name = "DNS",
    .proto = IPPROTO_UDP,
    .init = &dns_udp_client_init,
    .validate = &dns_udp_client_validate,
    .minimum_matches = 1
};

SF_SO_PUBLIC tRNAClientAppModule dns_tcp_client_mod =
{
    .name = "DNS",
    .proto = IPPROTO_TCP,
    .init = &dns_tcp_client_init,
    .validate = &dns_tcp_client_validate,
    .minimum_matches = 1
};

static CLIENT_APP_RETCODE dns_udp_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE dns_tcp_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE dns_udp_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const tAppIdConfig *pConfig)
{
    return CLIENT_APP_INPROCESS;
}

static CLIENT_APP_RETCODE dns_tcp_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const tAppIdConfig *pConfig)
{
    return CLIENT_APP_INPROCESS;
}

static int dns_host_pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    MatchedDNSPatterns *cm;
    MatchedDNSPatterns **matches = (MatchedDNSPatterns **)data;
    DNSHostPattern *target = (DNSHostPattern *)id;

    if (!(cm = (MatchedDNSPatterns *)malloc(sizeof(MatchedDNSPatterns))))
        return 1;

    cm->mpattern = target;
    cm->index = index;
    cm->next = *matches;
    *matches = cm;

    return 0;
}

static int dns_host_detector_create_matcher(void **matcher, DetectorDNSHostPattern *list)
{
    DetectorDNSHostPattern *element = NULL;

    if (*matcher)
        _dpd.searchAPI->search_instance_free(*matcher);

    if (!(*matcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return 0;

    /* Add patterns from Lua API */
    for(element = list; element; element = element->next)
    {
        _dpd.searchAPI->search_instance_add_ex(*matcher,
                           (char *)element->dpattern->pattern,
                           element->dpattern->pattern_size,
                           element->dpattern,
                           STR_SEARCH_CASE_INSENSITIVE);
    }

    _dpd.searchAPI->search_instance_prep(*matcher);

    return 1;
}

int dns_host_detector_process_patterns(tServiceDnsConfig *pDnsConfig)
{
    int retVal = 1;
    if (!dns_host_detector_create_matcher(&pDnsConfig->dns_host_host_matcher, pDnsConfig->DetectorDNSHostPatternList))
        retVal = 0;
    return retVal;
}

static int dns_service_init(const InitServiceAPI * const init_api)
{
    unsigned i;
    for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
        init_api->RegisterAppId(&dns_udp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
    }

    return 0;
}

static int dns_validate_label(const uint8_t *data, uint16_t *offset, uint16_t size, uint8_t *len, unsigned *len_valid)
{
    const DNSLabel *lbl;
    const DNSLabelPtr *lbl_ptr;
    const DNSLabelBitfield *lbl_bit;
    uint16_t tmp;

    *len = 0;
    *len_valid = 1;
    for (;;)
    {
        if ((size <= *offset) || (size-(*offset)) < (int)offsetof(DNSLabel, name))
            return SERVICE_NOMATCH;
        lbl = (DNSLabel *)(data + (*offset));
        switch (lbl->len & DNS_LENGTH_FLAGS)
        {
        case 0xC0:
            *len_valid = 0;
            lbl_ptr = (DNSLabelPtr *)lbl;
            *offset += offsetof(DNSLabelPtr, data);
            if (*offset > size) return SERVICE_NOMATCH;
            tmp = (uint16_t)(ntohs(lbl_ptr->position) & 0x3FFF);
            if (tmp > size - offsetof(DNSLabel, name))
                return SERVICE_NOMATCH;
            return SERVICE_SUCCESS;
        case 0x00:
            *offset += offsetof(DNSLabel, name);
            if (!lbl->len)
            {
                if (*len > 0)
                    (*len)--;    // take off the extra '.' at the end
                return SERVICE_SUCCESS;
            }
            *offset += lbl->len;
            *len += lbl->len + 1;    // add 1 for '.'
            break;
        case 0x40:
            *len_valid = 0;
            if (lbl->len != 0x41) return SERVICE_NOMATCH;
            *offset += offsetof(DNSLabelBitfield, data);
            if (*offset >= size) return SERVICE_NOMATCH;
            lbl_bit = (DNSLabelBitfield *)lbl;
            if (lbl_bit->len)
            {
                *offset += ((lbl_bit->len - 1) / 8) + 1;
            }
            else
            {
                *offset += 32;
            }
            break;
        default:
            *len_valid = 0;
            return SERVICE_NOMATCH;
        }
    }
}

static int dns_validate_query(const uint8_t *data, uint16_t *offset, uint16_t size,
                              uint16_t id, unsigned host_reporting, tAppIdData *flowp)
{
    int ret;
    const uint8_t *host;
    uint8_t host_len;
    unsigned host_len_valid;
    uint16_t host_offset;
    DNSQueryFixed *query;
    uint16_t record_type;
    bool root_query = false;

    host = data + *offset;
    host_offset = *offset;
    ret = dns_validate_label(data, offset, size, &host_len, &host_len_valid);
    if (ret == SERVICE_SUCCESS)
    {
        query = (DNSQueryFixed*)(data + *offset);
        *offset += sizeof(DNSQueryFixed);
        if (host_reporting)
        {
            record_type = ntohs(query->QType);
            if (!host_len && host_len_valid)
                root_query = true;
            else if ((host_len == 0) || (!host_len_valid))
            {
                host        = NULL;
                host_len    = 0;
                host_offset = 0;
            }
            switch (record_type)
            {
            case PATTERN_A_REC:
            case PATTERN_AAAA_REC:
            case PATTERN_CNAME_REC:
            case PATTERN_SRV_REC:
            case PATTERN_TXT_REC:
            case PATTERN_MX_REC:
            case PATTERN_SOA_REC:
            case PATTERN_NS_REC:
            case PATTERN_ANY_REC:
                dns_service_mod.api->add_dns_query_info(flowp, id, host, host_len, host_offset, record_type, *offset, root_query);
                break;
            case PATTERN_PTR_REC:
                dns_service_mod.api->add_dns_query_info(flowp, id, NULL, 0, 0, record_type, *offset, false);
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

static int dns_validate_answer(const uint8_t *data, uint16_t *offset, uint16_t size,
                               uint16_t id, uint8_t rcode, unsigned host_reporting, tAppIdData *flowp)
{
    int ret;
    const uint8_t *host;
    uint8_t host_len;
    unsigned host_len_valid;
    uint16_t host_offset;
    uint16_t record_type;
    uint32_t ttl;
    uint16_t r_data_offset;

    ret = dns_validate_label(data, offset, size, &host_len, &host_len_valid);
    if (ret == SERVICE_SUCCESS)
    {
        DNSAnswerData *ad = (DNSAnswerData *)(data + (*offset));
        *offset += sizeof(DNSAnswerData);
        if (*offset > size) return SERVICE_NOMATCH;
        r_data_offset = *offset;
        *offset += ntohs(ad->r_len);
        if (*offset > size) return SERVICE_NOMATCH;
        if (host_reporting)
        {
            record_type = ntohs(ad->type);
            ttl         = ntohl(ad->ttl);
            switch (record_type)
            {
            case PATTERN_A_REC:
            case PATTERN_AAAA_REC:
            case PATTERN_CNAME_REC:
            case PATTERN_SRV_REC:
            case PATTERN_TXT_REC:
            case PATTERN_MX_REC:
            case PATTERN_SOA_REC:
            case PATTERN_NS_REC:
	    // case PATTERN_ANY_REC:  // commented out by design
                dns_service_mod.api->add_dns_response_info(flowp, id, NULL, 0, 0, rcode, ttl);
                break;
            case PATTERN_PTR_REC:
                host = data + r_data_offset;
                host_offset = r_data_offset;
                ret = dns_validate_label(data, &r_data_offset, size, &host_len, &host_len_valid);
                if ((host_len == 0) || (!host_len_valid))
                {
                    host        = NULL;
                    host_len    = 0;
                    host_offset = 0;
                }
                dns_service_mod.api->add_dns_response_info(flowp, id, host, host_len, host_offset, rcode, ttl);
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

static int dns_validate_header(const int dir, DNSHeader *hdr,
                               unsigned host_reporting, tAppIdData *flowp)
{
    if (hdr->Opcode > MAX_OPCODE || hdr->Opcode == INVALID_OPCODE)
    {
        return SERVICE_NOMATCH;
    }
    if (hdr->Z)
    {
        return SERVICE_NOMATCH;
    }
    if (hdr->RCODE > MAX_RCODE)
    {
        return SERVICE_NOMATCH;
    }
    if (!hdr->QR)
    {
        // Query.
        if (host_reporting)
            dns_service_mod.api->reset_dns_info(flowp);
        return dir == APP_ID_FROM_INITIATOR ? SERVICE_SUCCESS:SERVICE_REVERSED;
    }

    // Response.
    return dir == APP_ID_FROM_INITIATOR ? SERVICE_REVERSED:SERVICE_SUCCESS;
}

static int validate_packet(const uint8_t *data, uint16_t size, const int dir,
                           unsigned host_reporting, tAppIdData *flowp)
{
    uint16_t i;
    uint16_t count;
    const DNSHeader *hdr = (const DNSHeader *)data;
    uint16_t offset;

    if (hdr->TC && size == 512) return SERVICE_SUCCESS;

    offset = sizeof(DNSHeader);

    if (hdr->QDCount)
    {
        count = ntohs(hdr->QDCount);
        for (i=0; i<count; i++)
        {
            if (dns_validate_query(data, &offset, size, ntohs(hdr->id), host_reporting, flowp) != SERVICE_SUCCESS)
            {
                return SERVICE_NOMATCH;
            }
        }
    }

    if (hdr->ANCount)
    {
        count = ntohs(hdr->ANCount);
        for (i=0; i<count; i++)
        {
            if (dns_validate_answer(data, &offset, size, ntohs(hdr->id), hdr->RCODE, host_reporting, flowp) != SERVICE_SUCCESS)
            {
                return SERVICE_NOMATCH;
            }
        }
    }

    if (hdr->NSCount)
    {
        count = ntohs(hdr->NSCount);
        for (i=0; i<count; i++)
        {
            if (dns_validate_answer(data, &offset, size, ntohs(hdr->id), hdr->RCODE, host_reporting, flowp) != SERVICE_SUCCESS)
            {
                return SERVICE_NOMATCH;
            }
        }
    }

    if (hdr->ARCount)
    {
        count = ntohs(hdr->ARCount);
        for (i=0; i<count; i++)
        {
            if (dns_validate_answer(data, &offset, size, ntohs(hdr->id), hdr->RCODE, host_reporting, flowp) != SERVICE_SUCCESS)
            {
                return SERVICE_NOMATCH;
            }
        }
    }

    if (hdr->QR && (hdr->RCODE != 0))    // error response
        dns_service_mod.api->add_dns_response_info(flowp, ntohs(hdr->id), NULL, 0, 0, hdr->RCODE, 0);

    return SERVICE_SUCCESS;
}

static int dns_udp_validate(ServiceValidationArgs* args)
{
    int rval;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        return SERVICE_INPROCESS;

    if (size < sizeof(DNSHeader))
    {
        rval = (dir == APP_ID_FROM_INITIATOR) ? SERVICE_INVALID_CLIENT:SERVICE_NOMATCH;
        goto udp_done;
    }
    if ((rval = dns_validate_header(dir, (DNSHeader *)data, appidStaticConfig->dns_host_reporting, flowp)) != SERVICE_SUCCESS)
    {
        if (rval == SERVICE_REVERSED)
        {
            if (dir == APP_ID_FROM_RESPONDER)
            {
                if (getAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED))
                {
                    // To get here, we missed the initial query, got a
                    // response, and now we've got another query.
                    rval = validate_packet(data, size, dir, appidStaticConfig->dns_host_reporting, flowp);
                    if (rval == SERVICE_SUCCESS)
                        goto success;
                }
                goto invalid;
            }
            else
            {
                // To get here, we missed the initial query, but now we've got
                // a response.
                rval = validate_packet(data, size, dir, appidStaticConfig->dns_host_reporting, flowp);
                if (rval == SERVICE_SUCCESS)
                {
                    setAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED);
                    goto success;
                }
                goto nomatch;
            }
        }
        rval = (dir == APP_ID_FROM_INITIATOR) ? SERVICE_INVALID_CLIENT:SERVICE_NOMATCH;
        goto udp_done;
    }

    rval = validate_packet(data, size, dir, appidStaticConfig->dns_host_reporting, flowp);

udp_done:
    switch (rval)
    {
    case SERVICE_SUCCESS:
success:
        // We will declare DNS as soon as we have seen a good query (we do not
        // wait until we get a reply).
        setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        dns_service_mod.api->add_service(flowp, args->pkt, dir, &udp_svc_element,
                                         APP_ID_DNS, NULL, NULL, NULL, NULL);
        return SERVICE_SUCCESS;
    case SERVICE_INVALID_CLIENT:
invalid:
        dns_service_mod.api->incompatible_data(flowp, args->pkt, dir, &udp_svc_element,
                                               dns_service_mod.flow_data_index,
                                               args->pConfig, NULL);
        return SERVICE_NOT_COMPATIBLE;
    case SERVICE_NOMATCH:
nomatch:
        dns_service_mod.api->fail_service(flowp, args->pkt, dir, &udp_svc_element,
                                          dns_service_mod.flow_data_index,
                                          args->pConfig, NULL);
        return SERVICE_NOMATCH;
    case SERVICE_INPROCESS:    // inprocess:
        dns_service_mod.api->service_inprocess(flowp, args->pkt, dir, &udp_svc_element, NULL);
        return SERVICE_INPROCESS;
    default:
        return rval;
    }
}

static int dns_tcp_validate(ServiceValidationArgs* args)
{
    ServiceDNSData *dd;
    const DNSTCPHeader *hdr;
    uint16_t tmp;
    int rval;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (size < sizeof(DNSTCPHeader))
    {
        if (dir == APP_ID_FROM_INITIATOR)
            goto not_compatible;
        else goto fail;
    }
    hdr = (DNSTCPHeader *)data;
    data += sizeof(DNSTCPHeader);
    size -= sizeof(DNSTCPHeader);
    tmp = ntohs(hdr->length);
    if (tmp < sizeof(DNSHeader) || dns_validate_header(dir, (DNSHeader *)data, appidStaticConfig->dns_host_reporting, flowp))
    {
        if (dir == APP_ID_FROM_INITIATOR)
            goto not_compatible;
        else
            goto fail;
    }

    if (tmp > size)
        goto not_compatible;
    rval = validate_packet(data, size, dir, appidStaticConfig->dns_host_reporting, flowp);
    if (rval != SERVICE_SUCCESS)
        goto tcp_done;

    dd = dns_service_mod.api->data_get(flowp, dns_service_mod.flow_data_index);
    if (!dd)
    {
        dd = calloc(1, sizeof(*dd));
        if (!dd)
            return SERVICE_ENOMEM;
        if (dns_service_mod.api->data_add(flowp, dd, dns_service_mod.flow_data_index, &free))
        {
            free(dd);
            return SERVICE_ENOMEM;
        }
        dd->state = DNS_STATE_QUERY;
    }

    if (dd->state == DNS_STATE_QUERY)
    {
        if (dir != APP_ID_FROM_INITIATOR)
            goto fail;
        dd->id = ((DNSHeader *)data)->id;
        dd->state = DNS_STATE_RESPONSE;
    }
    else
    {
        if (dir != APP_ID_FROM_RESPONDER || dd->id != ((DNSHeader *)data)->id)
            goto fail;
    }

tcp_done:
    switch (rval)
    {
    case SERVICE_SUCCESS:
        goto success;
    case SERVICE_INVALID_CLIENT:
        goto not_compatible;
    case SERVICE_NOMATCH:
        goto fail;
    case SERVICE_INPROCESS:
        goto inprocess;
    default:
        return rval;
    }

success:
    // We will declare DNS as soon as we have seen a good query (we do not
    // wait until we get a reply).
    setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
    dns_service_mod.api->add_service(flowp, args->pkt, dir, &tcp_svc_element,
                                     APP_ID_DNS, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

not_compatible:
    dns_service_mod.api->incompatible_data(flowp, args->pkt, dir, &tcp_svc_element,
                                           dns_service_mod.flow_data_index,
                                           args->pConfig, NULL);
    return SERVICE_NOT_COMPATIBLE;

fail:
    dns_service_mod.api->fail_service(flowp, args->pkt, dir, &tcp_svc_element,
                                      dns_service_mod.flow_data_index,
                                      args->pConfig, NULL);
    return SERVICE_NOMATCH;

inprocess:
    dns_service_mod.api->service_inprocess(flowp, args->pkt, dir, &tcp_svc_element, NULL);
    return SERVICE_INPROCESS;
}

static int dns_host_scan_patterns(void * matcher, const u_int8_t *pattern, size_t size, tAppId *clientAppId, tAppId *payloadId)
{
    MatchedDNSPatterns *mp = NULL;
    MatchedDNSPatterns *tmpMp;
    DNSHostPattern *best_match;

    if (!matcher) return 0;

    _dpd.searchAPI->search_instance_find_all(matcher,
               (char *)pattern,
               size, 0,
               dns_host_pattern_match,
               (void *)(&mp));

    if (!mp) return 0;

    best_match = mp->mpattern;
    tmpMp = mp->next;
    free(mp);

    while ((mp = tmpMp))
    {
        tmpMp = mp->next;
        if (mp->mpattern->pattern_size > best_match->pattern_size)
        {
            best_match = mp->mpattern;
        }
        free(mp);
    }

    switch (best_match->type)
    {
    // type 0 means WEB APP
    case 0:
        *clientAppId = APP_ID_DNS;
        *payloadId = best_match->appId;
        break;
    // type 1 means CLIENT
    case 1:
        *clientAppId = best_match->appId;
        *payloadId = 0;
        break;
    default:
        return 0;
    }

    return 1;
}

int dns_host_scan_hostname(const u_int8_t *pattern, size_t size, tAppId *clientAppId, tAppId *payloadId, const tServiceDnsConfig* pDnsConfig)
{
        return dns_host_scan_patterns(pDnsConfig->dns_host_host_matcher , pattern, size, clientAppId, payloadId);
}

void service_dns_host_clean(tServiceDnsConfig* pDnsConfig)
{
    if (pDnsConfig->dns_host_host_matcher )
    {
        _dpd.searchAPI->search_instance_free(pDnsConfig->dns_host_host_matcher );
        pDnsConfig->dns_host_host_matcher = NULL;
    }
}

static int dns_add_pattern(DetectorDNSHostPattern **list, uint8_t *pattern_str, size_t pattern_size, uint8_t type, tAppId app_id)
{
    DetectorDNSHostPattern *new_dns_host_pattern;

    new_dns_host_pattern = calloc(1, sizeof(DetectorDNSHostPattern));
    if (!new_dns_host_pattern)
    {
        return 0;
    }
    new_dns_host_pattern->dpattern = calloc(1, sizeof(DNSHostPattern));
    if (!new_dns_host_pattern->dpattern)
    {
        free(new_dns_host_pattern);
        return 0;
    }

    new_dns_host_pattern->dpattern->type = type;
    new_dns_host_pattern->dpattern->appId = app_id;
    new_dns_host_pattern->dpattern->pattern = pattern_str;
    new_dns_host_pattern->dpattern->pattern_size = pattern_size;

    new_dns_host_pattern->next = *list;
    *list = new_dns_host_pattern;

    return 1;
}

int dns_add_host_pattern(uint8_t *pattern_str, size_t pattern_size, uint8_t type, tAppId app_id, tServiceDnsConfig *pDnsConfig)
{
    return dns_add_pattern(&pDnsConfig->DetectorDNSHostPatternList, pattern_str, pattern_size, type, app_id);
}

static void dns_patterns_free(DetectorDNSHostPattern **list)
{
    DetectorDNSHostPattern *tmp_pattern;

    while ((tmp_pattern = *list))
    {
        *list = tmp_pattern->next;
        if (tmp_pattern->dpattern)
        {
            if (tmp_pattern->dpattern->pattern)
                free(tmp_pattern->dpattern->pattern);
            free (tmp_pattern->dpattern);
        }
        free(tmp_pattern);
    }
}

void dns_detector_free_patterns(tServiceDnsConfig *pDnsConfig)
{
    dns_patterns_free(&pDnsConfig->DetectorDNSHostPatternList);
}

char *dns_parse_host(const uint8_t *host, uint8_t host_len)
{
    char          *str;
    const uint8_t *src;
    char          *dst;
    uint8_t        len;
    uint32_t       dstLen = 0;

    str = malloc(host_len + 1);    // plus '\0' at end
    if (str != NULL)
    {
        src = host;
        dst = str;
        while (*src != 0)
        {
            len = *src;
            src++;
            if ((dstLen + len) <= host_len)
                memcpy(dst, src, len);
            else
            {
                // Malformed DNS host, return
                free(str);
                return NULL;
            }
            src += len;
            dst += len;
            *dst = '.';
            dstLen += len + 1;
            dst++;
        }
        str[host_len] = '\0';    // NULL term
    }
    return str;
}
