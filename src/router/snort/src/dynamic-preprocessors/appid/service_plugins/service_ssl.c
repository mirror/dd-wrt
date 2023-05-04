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
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <openssl/x509.h>

#include "flow.h"
#include "service_base.h"
#include "service_ssl.h"
#include "fw_appid.h"
#include "serviceConfig.h"
#include "thirdparty_appid_utils.h"

#define SSL_PORT    443

typedef enum
{
    SSL_CHANGE_CIPHER = 20,
    SSL_ALERT = 21,
    SSL_HANDSHAKE = 22,
    SSL_APPLICATION_DATA = 23
} SSLContentType;

#define SSL_CLIENT_HELLO 1
#define SSL_SERVER_HELLO 2
#define SSL_CERTIFICATE 11
#define SSL_SERVER_KEY_XCHG 12
#define SSL_SERVER_CERT_REQ 13
#define SSL_SERVER_HELLO_DONE 14
#define SSL_CERTIFICATE_STATUS 22
#define SSL2_SERVER_HELLO 4
#define PCT_SERVER_HELLO 2

#define FIELD_SEPARATOR "/"
#define COMMON_NAME_STR "/CN="
#define ORG_NAME_STR    "/O="

/* Extension types. */
#define SSL_EXT_SERVER_NAME 0

typedef struct _MatchedSSLPatterns {
    SSLCertPattern *mpattern;
    int index;
    struct _MatchedSSLPatterns *next;
} MatchedSSLPatterns;

typedef enum
{
    SSL_STATE_INITIATE,      /* Client initiates. */
    SSL_STATE_CONNECTION,    /* Server responds... */
    SSL_STATE_HEADER
} SSLState;

typedef struct _SERVICE_SSL_DATA
{
    SSLState state;
    int pos;
    int length;
    int tot_length;
    /* From client: */
    char *host_name;
    int   host_name_strlen;
    /* While collecting certificates: */
    int certs_len;          /* (Total) length of certificate(s). */
    uint8_t *certs_data;    /* Certificate(s) data (each proceeded by length (3 bytes)). */
    int in_certs;           /* Currently collecting certificates? */
    int certs_curr_len;     /* Current amount of collected certificate data. */
    /* Data collected from certificates afterwards: */
    char *common_name;
    int   common_name_strlen;
    int   org_name_strlen;
    char *org_name;
} ServiceSSLData;

typedef struct _SERVICE_SSL_CERTIFICATE
{
    X509    *cert;
    uint8_t *common_name_ptr;
    int      common_name_len;
    uint8_t *org_name_ptr;
    int      org_name_len;
    struct _SERVICE_SSL_CERTIFICATE *next;
} ServiceSSLCertificate;

#pragma pack(1)

typedef struct _SERVICE_SSL_V3_HEADER    /* Actually a TLS Record. */
{
    uint8_t type;
    uint16_t version;
    uint16_t len;
} ServiceSSLV3Hdr;

typedef struct _SERVICE_SSL_V3_RECORD    /* Actually a Handshake. */
{
    uint8_t type;
    uint8_t length_msb;
    uint16_t length;
    uint16_t version;
    struct {
        uint32_t time;
        uint8_t data[28];
    } random;
} ServiceSSLV3Record;

typedef struct _SERVICE_SSL_V3_CERTS_RECORD    /* Actually a Certificate(s) Handshake. */
{
    uint8_t type;
    uint8_t length_msb;
    uint16_t length;
    uint8_t certs_len[3];    /* 3-byte length, network byte order. */
    /* Certificate(s) follow.
     * For each:
     *  - Length: 3 bytes
     *  - Data  : "Length" bytes */
} ServiceSSLV3CertsRecord;

typedef struct _SERVICE_SSL_V3_EXTENSION_SERVER_NAME
{
    uint16_t type;
    uint16_t length;
    uint16_t list_length;
    uint8_t  string_length_msb;
    uint16_t string_length;
    /* String follows. */
} ServiceSSLV3ExtensionServerName;

typedef struct _SERVICE_SSL_PCT_HEADER
{
    uint8_t len;
    uint8_t len2;
    uint8_t type;
    uint8_t pad;
    uint16_t version;
    uint8_t restart;
    uint8_t auth;
    uint32_t cipher;
    uint16_t hash;
    uint16_t cert;
    uint16_t exch;
    uint8_t id[32];
    uint16_t cert_len;
    uint16_t c_cert_len;
    uint16_t c_sig_len;
    uint16_t resp_len;
} ServiceSSLPCTHdr;

typedef struct _SERVICE_SSL_V2_HEADER
{
    uint8_t len;
    uint8_t len2;
    uint8_t type;
    uint8_t id;
    uint8_t cert;
    uint16_t version;
    uint16_t cert_len;
    uint16_t cipher_len;
    uint16_t conn_len;
} ServiceSSLV2Hdr;

#pragma pack()

/* Convert 3-byte lengths in TLS headers to integers. */
#define ntoh3(msb_ptr) ((uint32_t)(   (uint32_t)(((uint8_t*)msb_ptr)[0] << 16)    \
                                    + (uint32_t)(((uint8_t*)msb_ptr)[1] <<  8)    \
                                    + (uint32_t)(((uint8_t*)msb_ptr)[2]      ) ))

static int ssl_cert_pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    MatchedSSLPatterns *cm;
    MatchedSSLPatterns **matches = (MatchedSSLPatterns **)data;
    SSLCertPattern *target = (SSLCertPattern *)id;

    if (!(cm = (MatchedSSLPatterns *)malloc(sizeof(MatchedSSLPatterns))))
        return 1;

    cm->mpattern = target;
    cm->index = index;
    cm->next = *matches;
    *matches = cm;

    return 0;
}

static int ssl_detector_create_matcher(void **matcher, DetectorSSLCertPattern *list)
{
    size_t *patternIndex;
    size_t size = 0;
    DetectorSSLCertPattern *element = NULL;

    if (*matcher)
        _dpd.searchAPI->search_instance_free(*matcher);

    if (!(*matcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return 0;

    patternIndex = &size;

    /* Add patterns from Lua API */
    for(element = list; element; element = element->next)
    {
        _dpd.searchAPI->search_instance_add_ex(*matcher,
                (char *)element->dpattern->pattern,
                element->dpattern->pattern_size,
                element->dpattern,
                STR_SEARCH_CASE_INSENSITIVE);
        (*patternIndex)++;
    }

    _dpd.searchAPI->search_instance_prep(*matcher);

    return 1;
}
int ssl_detector_process_patterns(tServiceSslConfig *pSslConfig)
{
    int retVal = 1;
    if (!ssl_detector_create_matcher(&pSslConfig->ssl_host_matcher, pSslConfig->DetectorSSLCertPatternList))
        retVal = 0;
    if (!ssl_detector_create_matcher(&pSslConfig->ssl_cname_matcher, pSslConfig->DetectorSSLCnamePatternList))
        retVal = 0;
    return retVal;
}

static int ssl_init(const InitServiceAPI * const api);
static int ssl_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &ssl_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "ssl",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&ssl_validate, 261, IPPROTO_TCP},
    {&ssl_validate, 261, IPPROTO_UDP},
    {&ssl_validate, 443, IPPROTO_TCP},
    {&ssl_validate, 443, IPPROTO_UDP},
    {&ssl_validate, 448, IPPROTO_TCP},
    {&ssl_validate, 448, IPPROTO_UDP},
    {&ssl_validate, 465, IPPROTO_TCP},
    {&ssl_validate, 563, IPPROTO_TCP},
    {&ssl_validate, 563, IPPROTO_UDP},
    {&ssl_validate, 585, IPPROTO_TCP},
    {&ssl_validate, 585, IPPROTO_UDP},
    {&ssl_validate, 614, IPPROTO_TCP},
    {&ssl_validate, 636, IPPROTO_TCP},
    {&ssl_validate, 636, IPPROTO_UDP},
    {&ssl_validate, 853, IPPROTO_TCP},
    {&ssl_validate, 989, IPPROTO_TCP},
    {&ssl_validate, 990, IPPROTO_TCP},
    {&ssl_validate, 992, IPPROTO_TCP},
    {&ssl_validate, 992, IPPROTO_UDP},
    {&ssl_validate, 993, IPPROTO_TCP},
    {&ssl_validate, 993, IPPROTO_UDP},
    {&ssl_validate, 994, IPPROTO_TCP},
    {&ssl_validate, 994, IPPROTO_UDP},
    {&ssl_validate, 995, IPPROTO_TCP},
    {&ssl_validate, 995, IPPROTO_UDP},
    {&ssl_validate, 3269, IPPROTO_TCP},
    {&ssl_validate, 8305, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule ssl_service_mod =
{
    "ssl",
    &ssl_init,
    pp
};

static uint8_t SSL_PATTERN_PCT[] = {0x02, 0x00, 0x80, 0x01};
static uint8_t SSL_PATTERN3_0[] = {0x16, 0x03, 0x00};
static uint8_t SSL_PATTERN3_1[] = {0x16, 0x03, 0x01};
static uint8_t SSL_PATTERN3_2[] = {0x16, 0x03, 0x02};
static uint8_t SSL_PATTERN3_3[] = {0x16, 0x03, 0x03};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_SSL, APPINFO_FLAG_SERVICE_ADDITIONAL}
};

static int ssl_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&ssl_validate, IPPROTO_TCP, SSL_PATTERN_PCT, sizeof(SSL_PATTERN_PCT), 2, "ssl", init_api->pAppidConfig);
    init_api->RegisterPattern(&ssl_validate, IPPROTO_TCP, SSL_PATTERN3_0, sizeof(SSL_PATTERN3_0), 0, "ssl", init_api->pAppidConfig);
    init_api->RegisterPattern(&ssl_validate, IPPROTO_TCP, SSL_PATTERN3_1, sizeof(SSL_PATTERN3_1), 0, "ssl", init_api->pAppidConfig);
    init_api->RegisterPattern(&ssl_validate, IPPROTO_TCP, SSL_PATTERN3_2, sizeof(SSL_PATTERN3_2), 0, "ssl", init_api->pAppidConfig);
    init_api->RegisterPattern(&ssl_validate, IPPROTO_TCP, SSL_PATTERN3_3, sizeof(SSL_PATTERN3_3), 0, "ssl", init_api->pAppidConfig);
    unsigned i;
    for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
        init_api->RegisterAppId(&ssl_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
    }

    return 0;
}

void ssl_free(void *ss)    /* AppIdFreeFCN */
{
    ServiceSSLData *ss_tmp = (ServiceSSLData*)ss;
    free(ss_tmp->certs_data);
    free(ss_tmp->host_name);
    free(ss_tmp->common_name);
    free(ss_tmp->org_name);
    free(ss_tmp);
}

void parse_client_initiation(const uint8_t *data, uint16_t size, ServiceSSLData *ss)
{
    const ServiceSSLV3Hdr *hdr3;
    const ServiceSSLV3Record *rec;
    int length;
    uint16_t ver;

    /* Sanity check header stuff. */
    if (size < sizeof(ServiceSSLV3Hdr)) return;
    hdr3 = (ServiceSSLV3Hdr *)data;
    ver = ntohs(hdr3->version);
    if (hdr3->type != SSL_HANDSHAKE ||
        (ver != 0x0300 &&
         ver != 0x0301 &&
         ver != 0x0302 &&
         ver != 0x0303))
    {
        return;
    }
    data += sizeof(ServiceSSLV3Hdr);
    size -= sizeof(ServiceSSLV3Hdr);

    if (size < sizeof(ServiceSSLV3Record)) return;
    rec = (ServiceSSLV3Record *)data;
    ver = ntohs(rec->version);
    if (rec->type != SSL_CLIENT_HELLO ||
        (ver != 0x0300 &&
         ver != 0x0301 &&
         ver != 0x0302 &&
         ver != 0x0303) ||
        rec->length_msb)
    {
        return;
    }
    length = ntohs(rec->length) + offsetof(ServiceSSLV3Record, version);
    if (size < length) return;
    data += sizeof(ServiceSSLV3Record);
    size -= sizeof(ServiceSSLV3Record);

    /* Session ID (1-byte length). */
    if (size < 1) return;
    length = *((uint8_t*)data);
    data += length + 1;
    if (size < (length + 1)) return;
    size -= length + 1;

    /* Cipher Suites (2-byte length). */
    if (size < 2) return;
    length = ntohs(*((uint16_t*)data));
    data += length + 2;
    if (size < (length + 2)) return;
    size -= length + 2;

    /* Compression Methods (1-byte length). */
    if (size < 1) return;
    length = *((uint8_t*)data);
    data += length + 1;
    if (size < (length + 1)) return;
    size -= length + 1;

    /* Extensions (2-byte length) */
    if (size < 2) return;
    length = ntohs(*((uint16_t*)data));
    data += 2;
    size -= 2;
    if (size < length) return;

    // We need at least type (2 bytes) and length (2 bytes) fields in the extension
    while (length >= 4)
    {
        ServiceSSLV3ExtensionServerName *ext = (ServiceSSLV3ExtensionServerName*)data;
        if (ntohs(ext->type) == SSL_EXT_SERVER_NAME)
        {
            /* Found server host name. */
            if (length < sizeof(ServiceSSLV3ExtensionServerName)) return;

            int len = ntohs(ext->string_length);
            if ((length - sizeof(ServiceSSLV3ExtensionServerName)) < len) return;

            const uint8_t *str =   data
                                 + offsetof(ServiceSSLV3ExtensionServerName, string_length)
                                 + sizeof(ext->string_length);
            ss->host_name = malloc(len + 1);    /* Plus NULL term. */
            if (!ss->host_name)
            {
                _dpd.errMsg("parse_client_initiation: "
                        "Could not allocate memory for host name in ServiceSSLData\n");
                return;
            }
            else
            {
                memcpy(ss->host_name, str, len);
                ss->host_name[len] = '\0';
                ss->host_name_strlen = len;
            }
            return;
        }
        data   += ntohs(ext->length) + offsetof(ServiceSSLV3ExtensionServerName, list_length);
        length -= ntohs(ext->length) + offsetof(ServiceSSLV3ExtensionServerName, list_length);
    }
}

int parse_certificates(ServiceSSLData *ss)
{
    int success = 0;
    if (ss->certs_data && ss->certs_len)
    {
        char *common_name = 0;
        char *org_name = 0;

        uint8_t *data = ss->certs_data;
        int len = ss->certs_len;
        int common_name_tot_len = 0;
        int org_name_tot_len = 0;

        success = 1;
        while (len > 0 && !(common_name && org_name))
        {
            X509 *cert = NULL;
            char *cert_name = NULL;
            char *start = NULL;
            char *end = NULL;
            int length = 0;

            /* Get each certificate. */
            int cert_len = ntoh3(data);
            data += 3;
            len -= 3;
            if (len < cert_len) 
            {
                success = 0;
                break;
            }

            cert = d2i_X509(NULL, (const unsigned char **)&data, cert_len);
            len -= cert_len;    /* Above call increments data pointer already. */
            if (!cert)
            {
                success = 0;
                break;
            }

            /* look for common name or org name if we haven't seen either */
            if (!common_name || !org_name)
            {
                if ((cert_name = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0)))
                {
                    if (!common_name)
                    {
                        if ((start = strstr(cert_name, COMMON_NAME_STR)))
                        {
                            start += strlen(COMMON_NAME_STR);
                            end = strstr(start, FIELD_SEPARATOR);
                            if (end) *end = 0;
                            length = strlen(start);
                            if (length>2 && *start=='*' && *(start+1)=='.') 
                            {
                                start += 2; // remove leading .*
                                length -= 2;
                            }
                            common_name = strndup(start, length);
                            common_name_tot_len += length;
                            start = NULL;
                        }
                    }
                    if (!org_name)
                    {
                        if ((start = strstr(cert_name, ORG_NAME_STR)))
                        {
                            start += strlen(ORG_NAME_STR);
                            end = strstr(start, FIELD_SEPARATOR);
                            if (end) *end = 0;
                            length = strlen(start);
                            if (length>2 && *start=='*' && *(start+1)=='.')
                            {
                                start += 2; // remove leading .*
                                length -= 2;
                            }
                            org_name = strndup(start, length);
                            org_name_tot_len += length;
                        }
                    }
                    free(cert_name);
                    cert_name = NULL;
                }
            }
        X509_free(cert);
        }

        if (common_name)
        {
            ss->common_name = common_name;
            ss->common_name_strlen = common_name_tot_len;
        }

        if (org_name)
        {
            ss->org_name = org_name;
            ss->org_name_strlen = org_name_tot_len;
        }

        /* No longer need entire certificates.  We have what we came for. */
        free(ss->certs_data);
        ss->certs_data = NULL;
        ss->certs_len = 0;
    }
    return success;  // could be 1 even though common_name or org_name == 0
}

static int ssl_validate(ServiceValidationArgs* args)
{
    ServiceSSLData *ss;
    const ServiceSSLPCTHdr *pct;
    const ServiceSSLV2Hdr *hdr2;
    const ServiceSSLV3Hdr *hdr3;
    const ServiceSSLV3Record *rec;
    const ServiceSSLV3CertsRecord *certs_rec;
    uint16_t ver;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;

    ss = ssl_service_mod.api->data_get(flowp, ssl_service_mod.flow_data_index);
    if (!ss)
    {
        ss = calloc(1, sizeof(*ss));
        if (!ss)
            return SERVICE_ENOMEM;
        if (ssl_service_mod.api->data_add(flowp, ss, ssl_service_mod.flow_data_index, &ssl_free))
        {
            free(ss);
            return SERVICE_ENOMEM;
        }
        ss->state = SSL_STATE_INITIATE;
    }
    /* Start off with a Client Hello from client to server. */
    if (ss->state == SSL_STATE_INITIATE)
    {
        ss->state = SSL_STATE_CONNECTION;

        if (!(flowp->scan_flags & SCAN_CERTVIZ_ENABLED_FLAG) && 
            dir == APP_ID_FROM_INITIATOR)
        {
            parse_client_initiation(data, size, ss);
            goto inprocess;
        }
    }

    if (dir != APP_ID_FROM_RESPONDER)
    {
            goto inprocess;
    }

    switch (ss->state)
    {
    case SSL_STATE_CONNECTION:
        pct = (ServiceSSLPCTHdr *)data;
        hdr2 = (ServiceSSLV2Hdr *)data;
        hdr3 = (ServiceSSLV3Hdr *)data;
        /* SSL PCT header? */
        if (size >= sizeof(ServiceSSLPCTHdr) && pct->len >= 0x80 &&
            pct->type == PCT_SERVER_HELLO && ntohs(pct->version) == 0x8001)
        {
            goto success;
        }
        /* SSL v2 header? */
        if (size >= sizeof(ServiceSSLV2Hdr) && hdr2->len >= 0x80 &&
            hdr2->type == SSL2_SERVER_HELLO && !(hdr2->cert & 0xFE))
        {
            uint16_t h2v = ntohs(hdr2->version);
            if ((h2v == 0x0002 || h2v == 0x0300 ||
                 h2v == 0x0301 || h2v == 0x0303) &&
                !(hdr2->cipher_len % 3))
            {
                goto success;
            }
        }
        /* it is probably an SSLv3, TLS 1.2, or TLS 1.3 header.
           First record must be a handshake (type 22). */
        if (size < sizeof(ServiceSSLV3Hdr) ||
            hdr3->type != SSL_HANDSHAKE ||
            (ntohs(hdr3->version) != 0x0300 &&
             ntohs(hdr3->version) != 0x0301 && 
             ntohs(hdr3->version) != 0x0302 && 
             ntohs(hdr3->version) != 0x0303)) 
        {
            goto fail;
        }
        data += sizeof(ServiceSSLV3Hdr);
        size -= sizeof(ServiceSSLV3Hdr);
        rec = (ServiceSSLV3Record *)data;
        if (size < sizeof(ServiceSSLV3Record) ||
            rec->type != SSL_SERVER_HELLO ||
            (ntohs(rec->version) != 0x0300 &&
             ntohs(rec->version) != 0x0301 && 
             ntohs(rec->version) != 0x0302 && 
             ntohs(rec->version) != 0x0303) ||
            rec->length_msb)
        {
            goto fail;
        }
        ss->tot_length = ntohs(hdr3->len);
        ss->length = ntohs(rec->length) +
                     offsetof(ServiceSSLV3Record, version);
        if (ss->tot_length < ss->length) goto fail;
        ss->tot_length -= ss->length;
        if (size < ss->length) goto fail;
        data += ss->length;
        size -= ss->length;
        ss->state = SSL_STATE_HEADER;
        ss->pos = 0;
        /* fall through */
    case SSL_STATE_HEADER:
        while (size > 0)
        {
            if (!ss->pos)
            {
                /* Need to move onto (and past) next header (i.e., record) if
                 * previous was completely consumed. */
                if (ss->tot_length == 0)
                {
                    hdr3 = (ServiceSSLV3Hdr *)data;
                    ver = ntohs(hdr3->version);
                    if (size < sizeof(ServiceSSLV3Hdr) ||
                        (hdr3->type != SSL_HANDSHAKE &&
                         hdr3->type != SSL_CHANGE_CIPHER &&
                         hdr3->type != SSL_APPLICATION_DATA) ||
                        (ver != 0x0300 &&
                         ver != 0x0301 &&
                         ver != 0x0302 &&
                         ver != 0x0303))
                    {
                        goto fail;
                    }
                    data += sizeof(ServiceSSLV3Hdr);
                    size -= sizeof(ServiceSSLV3Hdr);
                    ss->tot_length = ntohs(hdr3->len);
                    if (hdr3->type == SSL_CHANGE_CIPHER ||
                        hdr3->type == SSL_APPLICATION_DATA)
                    {
                        goto success;
                    }
                }

                rec = (ServiceSSLV3Record *)data;
                if ( rec->type != SSL_SERVER_HELLO_DONE  &&
                    ( size < offsetof(ServiceSSLV3Record, version) ||
                    rec->length_msb) )
                {
                    goto fail;
                }
                switch (rec->type)
                {
                case SSL_CERTIFICATE:
                    /* Start pulling out certificates. */
                    if (!ss->certs_data)
                    {
                        certs_rec = (ServiceSSLV3CertsRecord *)data;
                        ss->certs_len = ntoh3(certs_rec->certs_len);
                        ss->certs_data = malloc(ss->certs_len);
                        if (!ss->certs_data)
                            return SERVICE_ENOMEM;
                        if ((size - sizeof(ServiceSSLV3CertsRecord)) < ss->certs_len)
                        {
                            /* Will have to get more next time around. */
                            ss->in_certs       = 1;
                            ss->certs_curr_len = size - sizeof(ServiceSSLV3CertsRecord);    /* Skip over header to data. */
                            memcpy(ss->certs_data, data + sizeof(ServiceSSLV3CertsRecord), ss->certs_curr_len);
                        }
                        else
                        {
                            /* Can get it all this time. */
                            ss->in_certs       = 0;
                            ss->certs_curr_len = ss->certs_len;
                            memcpy(ss->certs_data, data + sizeof(ServiceSSLV3CertsRecord), ss->certs_curr_len);
                            break;
                        }
                    }
                    /* fall through */
                case SSL_CERTIFICATE_STATUS:
                case SSL_SERVER_KEY_XCHG:
                case SSL_SERVER_CERT_REQ:
                    ss->length = ntohs(rec->length) +
                                 offsetof(ServiceSSLV3Record, version);
                    if (ss->tot_length < ss->length) goto fail;
                    ss->tot_length -= ss->length;
                    if (size < ss->length)
                    {
                        ss->pos = size;
                        size = 0;
                    }
                    else
                    {
                        data += ss->length;
                        size -= ss->length;
                        ss->pos = 0;
                    }
                    break;
                case SSL_SERVER_HELLO_DONE:
                    if (size < offsetof(ServiceSSLV3Record, version))
                    {
                        goto success;
                    }
                    if (rec->length) goto fail;
                    if (ss->tot_length != offsetof(ServiceSSLV3Record, version))
                        goto fail;
                    goto success;
                default:
                    goto fail;
                }
            }
            else
            {
                /* See if there's more certificate data to grab. */
                if (ss->in_certs && ss->certs_data)
                {
                    if (size < (ss->certs_len - ss->certs_curr_len))
                    {
                        /* Will have to get more next time around. */
                        memcpy(ss->certs_data + ss->certs_curr_len, data, size);
                        ss->in_certs        = 1;
                        ss->certs_curr_len += size;
                    }
                    else
                    {
                        /* Can get it all this time. */
                        memcpy(ss->certs_data + ss->certs_curr_len, data, ss->certs_len - ss->certs_curr_len);
                        ss->in_certs       = 0;
                        ss->certs_curr_len = ss->certs_len;
                    }
                }

                if (size+ss->pos < ss->length)
                {
                    ss->pos += size;
                    size = 0;
                }
                else
                {
                    data += ss->length - ss->pos;
                    size -= ss->length - ss->pos;
                    ss->pos = 0;
                }
            }
        }
        break;
    default:
        goto fail;
    }

inprocess:
    ssl_service_mod.api->service_inprocess(flowp, args->pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

fail:
    free(ss->certs_data);
    free(ss->host_name);
    free(ss->common_name);
    free(ss->org_name);
    ss->certs_data = NULL;
    ss->host_name = ss->common_name = ss->org_name = NULL;
    ssl_service_mod.api->fail_service(flowp, args->pkt, dir, &svc_element,
                                      ssl_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;

success:
    if (ss->certs_data && ss->certs_len)
    {
        if (!(flowp->scan_flags & SCAN_CERTVIZ_ENABLED_FLAG) &&
            !thirdparty_appid_module && !parse_certificates(ss))
        {
            goto fail;
        }
    }
    setAppIdFlag(flowp, APPID_SESSION_SSL_SESSION);
    if (ss->host_name || ss->common_name || ss->org_name)
    {
        if (!flowp->tsession)
        {
            if (!(flowp->tsession = calloc(1, sizeof(*flowp->tsession))))
            {
                goto fail;
            }
        }

        /* TLS Host */
        if (ss->host_name)
        {
            if (flowp->tsession->tls_host)
                free(flowp->tsession->tls_host);
            flowp->tsession->tls_host = ss->host_name;
            flowp->tsession->tls_host_strlen = ss->host_name_strlen;
            flowp->scan_flags |= SCAN_SSL_HOST_FLAG;
        }
        else if (ss->common_name)    // use common name (from server) if we didn't see host name (from client)
        {
            char *common_name = strndup(ss->common_name, ss->common_name_strlen);
            if (common_name)
            {
                if (flowp->tsession->tls_host)
                    free(flowp->tsession->tls_host);
                flowp->tsession->tls_host = common_name;
                flowp->tsession->tls_host_strlen = ss->common_name_strlen;
                flowp->scan_flags |= SCAN_SSL_HOST_FLAG;
            }
        }

        /* TLS Common Name */
        if (ss->common_name)
        {
            if (flowp->tsession->tls_cname)
                free(flowp->tsession->tls_cname);
            flowp->tsession->tls_cname = ss->common_name;
            flowp->tsession->tls_cname_strlen = ss->common_name_strlen;
            flowp->scan_flags |= SCAN_SSL_CERTIFICATE_FLAG;
        }

        /* TLS Org Unit */
        if (ss->org_name)
        {
            if (flowp->tsession->tls_orgUnit)
                free(flowp->tsession->tls_orgUnit);
            flowp->tsession->tls_orgUnit = ss->org_name;
            flowp->tsession->tls_orgUnit_strlen = ss->org_name_strlen;
        }

        ss->host_name = ss->common_name = ss->org_name = NULL;
        flowp->tsession->tls_handshake_done = true;
    }
    ssl_service_mod.api->add_service(flowp, args->pkt, dir, &svc_element,
                                     getSslServiceAppId(args->pkt->src_port), NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;
}

tAppId getSslServiceAppId( short srcPort)
{
    switch (srcPort)
    {
    case 261:
        return APP_ID_NSIIOPS;
    case 443:
        return APP_ID_HTTPS;
    case 448:
        return APP_ID_DDM_SSL;
    case 465:
        return APP_ID_SMTPS;
    case 563:
        return APP_ID_NNTPS;
    case 585:  /*Currently 585 is de-registered at IANA but old implementation may still use it. */
    case 993:
        return APP_ID_IMAPS;
    case 614:
        return APP_ID_SSHELL;
    case 636:
        return APP_ID_LDAPS;
    case 853:
        return APP_ID_DNS_OVER_TLS;
    case 989:
        return APP_ID_FTPSDATA;
    case 990:
        return APP_ID_FTPS;
    case 992:
        return APP_ID_TELNETS;
    case 994:
        return APP_ID_IRCS;
    case 995:
        return APP_ID_POP3S;
    case 3269:
        return APP_ID_MSFT_GC_SSL;
    case 8305:
        return APP_ID_SF_APPLIANCE_MGMT;
    default:
        return APP_ID_SSL;
    }
}

bool isSslServiceAppId(tAppId appId)
{
    switch (appId)
    {
    case APP_ID_NSIIOPS:
    case APP_ID_HTTPS:
    case APP_ID_DDM_SSL:
    case APP_ID_SMTPS:
    case APP_ID_NNTPS:
    case APP_ID_IMAPS:
    case APP_ID_SSHELL:
    case APP_ID_LDAPS:
    case APP_ID_FTPSDATA:
    case APP_ID_FTPS:
    case APP_ID_TELNETS:
    case APP_ID_IRCS:
    case APP_ID_POP3S:
    case APP_ID_MSFT_GC_SSL:
    case APP_ID_SF_APPLIANCE_MGMT:
    case APP_ID_SSL:
        return true;
    }

    return false;
}

static int ssl_scan_patterns(void * matcher, const u_int8_t *pattern, size_t size, tAppId *clientAppId, tAppId *payloadId)
{
    MatchedSSLPatterns *mp = NULL;
    MatchedSSLPatterns *tmpMp;
    SSLCertPattern *best_match;

    if (!matcher) return 0;

    _dpd.searchAPI->search_instance_find_all(matcher,
               (char *)pattern,
               size, 0,
               ssl_cert_pattern_match, (void *)&mp);

    if (!mp) return 0;

    best_match = NULL;
    while (mp)
    {
        //only patterns that match start of payload, or patterns starting with '.' or patterns folowing '.' in payload
        //are considered a match.
        if (mp->index == 0 || *mp->mpattern->pattern == '.' || pattern[mp->index-1] == '.')
        {
            if (!best_match || mp->mpattern->pattern_size > best_match->pattern_size)
            {
                best_match = mp->mpattern;
            }
        } 
        tmpMp = mp;
        mp = mp->next;
        free (tmpMp);
    }
    if (!best_match) return 0;

    switch (best_match->type)
    {
    /* type 0 means WEB APP */
    case 0:
        *clientAppId = APP_ID_SSL_CLIENT;
        *payloadId = best_match->appId;
        break;
    /* type 1 means CLIENT */
    case 1:
        *clientAppId = best_match->appId;
        *payloadId = 0;
        break;
    default:
        return 0;
    }

    return 1;
}

int ssl_scan_hostname(const u_int8_t *pattern, size_t size, tAppId *clientAppId, tAppId *payloadId, tServiceSslConfig *pSslConfig)
{
    return ssl_scan_patterns(pSslConfig->ssl_host_matcher, pattern, size, clientAppId, payloadId);
}

int ssl_scan_cname(const u_int8_t *pattern, size_t size, tAppId *clientAppId, tAppId *payloadId, tServiceSslConfig *pSslConfig)
{
    return ssl_scan_patterns(pSslConfig->ssl_cname_matcher, pattern, size, clientAppId, payloadId);
}

void service_ssl_clean(tServiceSslConfig *pSslConfig)
{
    if (pSslConfig->ssl_host_matcher)
    {
        _dpd.searchAPI->search_instance_free(pSslConfig->ssl_host_matcher);
        pSslConfig->ssl_host_matcher = NULL;
    }
    if (pSslConfig->ssl_cname_matcher)
    {
        _dpd.searchAPI->search_instance_free(pSslConfig->ssl_cname_matcher);
        pSslConfig->ssl_cname_matcher = NULL;
    }
}

static int ssl_add_pattern(DetectorSSLCertPattern **list, uint8_t *pattern_str, size_t pattern_size, uint8_t type, tAppId app_id)
{
    DetectorSSLCertPattern *new_ssl_pattern;

    new_ssl_pattern = calloc(1, sizeof(DetectorSSLCertPattern));
    if (!new_ssl_pattern)
    {
        return 0;
    }
    new_ssl_pattern->dpattern = calloc(1, sizeof(SSLCertPattern));
    if (!new_ssl_pattern->dpattern)
    {
        free(new_ssl_pattern);
        return 0;
    }

    new_ssl_pattern->dpattern->type = type;
    new_ssl_pattern->dpattern->appId = app_id;
    new_ssl_pattern->dpattern->pattern = pattern_str;
    new_ssl_pattern->dpattern->pattern_size = pattern_size;

    new_ssl_pattern->next = *list;
    *list = new_ssl_pattern;

    return 1;
}
int ssl_add_cert_pattern(uint8_t *pattern_str, size_t pattern_size, uint8_t type, tAppId app_id, tServiceSslConfig *pSslConfig)
{
    return ssl_add_pattern(&pSslConfig->DetectorSSLCertPatternList, pattern_str, pattern_size, type, app_id);
}
int ssl_add_cname_pattern(uint8_t *pattern_str, size_t pattern_size, uint8_t type, tAppId app_id, tServiceSslConfig *pSslConfig)
{
    return ssl_add_pattern(&pSslConfig->DetectorSSLCnamePatternList, pattern_str, pattern_size, type, app_id);
}

static void ssl_patterns_free(DetectorSSLCertPattern **list)
{
    DetectorSSLCertPattern *tmp_pattern;

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

void ssl_detector_free_patterns(tServiceSslConfig *pSslConfig)
{
    ssl_patterns_free(&pSslConfig->DetectorSSLCertPatternList);
    ssl_patterns_free(&pSslConfig->DetectorSSLCnamePatternList);
}

int setSSLSquelch(SFSnortPacket *p, int type, tAppId appId)
{
    sfaddr_t *sip, *dip;
    tAppIdData *f;

    if (!appInfoEntryFlagGet(appId, APPINFO_FLAG_SSL_SQUELCH, appIdActiveConfigGet()))
        return 0;

    dip = GET_DST_IP(p);
    sip = GET_SRC_IP(p);

    if (!(f = AppIdEarlySessionCreate(NULL, p, sip, 0, dip, p->dst_port,
                                       IPPROTO_TCP, appId, 0)))
        return 0;

    switch (type)
    {
    case 1:
        f->payloadAppId = appId;
        break;
    case 2:
        f->clientAppId = appId;
        f->rnaClientState = RNA_STATE_FINISHED;
        break;
    default:
        return 0;
    }

    return 1;

}
