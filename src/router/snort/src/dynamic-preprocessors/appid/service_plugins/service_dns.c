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


#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "service_api.h"

#define MAX_OPCODE 5
#define INVALID_OPCODE 3

#define MAX_RCODE 10

#define DNS_LENGTH_FLAGS    0xC0

#pragma pack(1)

typedef struct _DNS_HEADER {
    uint16_t id;
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t  QR:1,
              Opcode:4,
              AA:1,
              TC:1,
              RD:1;
    uint8_t  RA:1,
              Z:1,
              AD:1,
              CD:1,
              RCODE:4;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t  RD:1,
              TC:1,
              AA:1,
              Opcode:4,
              QR:1;
    uint8_t  RCODE:4,
              CD:1,
              AD:1,
              Z:1,
              RA:1;
#else
#error "Please fix <endian.h>"
#endif
    uint16_t QDCount;
    uint16_t ANCount;
    uint16_t NSCount;
    uint16_t ARCount;
} DNSHeader;

typedef struct _DNS_TCP_HEADER {
    uint16_t length;
} DNSTCPHeader;

typedef struct _DNS_LABEL {
    uint8_t len;
    uint8_t name;
} DNSLabel;

typedef struct _DNS_LABEL_POINTER {
    uint16_t position;
    uint8_t data;
} DNSLabelPtr;

typedef struct _DNS_QUERY_FIXED {
    uint16_t QType;
    uint16_t QClass;
} DNSQueryFixed;

typedef struct _DNS_ANSWER_DATA {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t r_len;
} DNSAnswerData;

typedef struct _DNS_LABEL_BITFIELD {
    uint8_t id;
    uint8_t len;
    uint8_t data;
} DNSLabelBitfield;

#pragma pack()

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

static int dns_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(dns_tcp_validate);
MakeRNAServiceValidationPrototype(dns_validate);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &dns_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "dns",
    .ref_count = 1,
};
static RNAServiceElement tcp_svc_element =
{
    .next = NULL,
    .validate = &dns_tcp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "tcp dns",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&dns_tcp_validate, 53, IPPROTO_TCP},
    {&dns_validate, 53, IPPROTO_UDP},
    {&dns_validate, 53, IPPROTO_UDP, 1},
    {&dns_tcp_validate, 5300, IPPROTO_TCP},
    {&dns_validate, 5300, IPPROTO_UDP},
    {NULL, 0, 0}
};

RNAServiceValidationModule dns_service_mod =
{
    "DNS",
    &dns_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_DNS, APPINFO_FLAG_SERVICE_UDP_REVERSED}
};

static int dns_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&dns_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, NULL);
	}

    return 0;
}

static int dns_validate_label(const uint8_t *data, uint16_t *offset, uint16_t size)
{
    const DNSLabel *lbl;
    const DNSLabelPtr *lbl_ptr;
    const DNSLabelBitfield *lbl_bit;
    uint16_t tmp;

    for (;;)
    {
        if ((size <= *offset) || (size-(*offset)) < (int)offsetof(DNSLabel, name))
            return SERVICE_NOMATCH;
        lbl = (DNSLabel *)(data + (*offset));
        switch (lbl->len & DNS_LENGTH_FLAGS)
        {
        case 0xC0:
            lbl_ptr = (DNSLabelPtr *)lbl;
            *offset += offsetof(DNSLabelPtr, data);
            if (*offset >= size) return SERVICE_NOMATCH;
            tmp = (uint16_t)(ntohs(lbl_ptr->position) & 0x3FFF);
            if (tmp > size - offsetof(DNSLabel, name))
                return SERVICE_NOMATCH;
            return SERVICE_SUCCESS;
        case 0x00:
            *offset += offsetof(DNSLabel, name);
            if (!lbl->len) return SERVICE_SUCCESS;
            *offset += lbl->len;
            break;
        case 0x40:
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
            return SERVICE_NOMATCH;
        }
    }
}

static int dns_validate_query(const uint8_t *data, uint16_t *offset, uint16_t size)
{
    int ret;

    ret = dns_validate_label(data, offset, size);
    if (ret == SERVICE_SUCCESS) *offset += sizeof(DNSQueryFixed);
    return ret;
}

static int dns_validate_answer(const uint8_t *data, uint16_t *offset, uint16_t size)
{
    int ret;

    ret = dns_validate_label(data, offset, size);
    if (ret == SERVICE_SUCCESS)
    {
        DNSAnswerData *ad = (DNSAnswerData *)(data + (*offset));
        *offset += sizeof(DNSAnswerData);
        if (*offset > size) return SERVICE_NOMATCH;
        *offset += ntohs(ad->r_len);
        if (*offset > size) return SERVICE_NOMATCH;
    }
    return ret;
}

static int dns_validate_header(const int dir, DNSHeader *hdr)
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
        return dir == APP_ID_FROM_INITIATOR ? SERVICE_SUCCESS:SERVICE_REVERSED;
    }

    return dir == APP_ID_FROM_INITIATOR ? SERVICE_REVERSED:SERVICE_SUCCESS;
}

static int validate_packet(const uint8_t *data, uint16_t size, const int dir)
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
            if (dns_validate_query(data, &offset, size) != SERVICE_SUCCESS)
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
            if (dns_validate_answer(data, &offset, size) != SERVICE_SUCCESS)
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
            if (dns_validate_answer(data, &offset, size) != SERVICE_SUCCESS)
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
            if (dns_validate_answer(data, &offset, size) != SERVICE_SUCCESS)
            {
                return SERVICE_NOMATCH;
            }
        }
    }
    return SERVICE_SUCCESS;
}

MakeRNAServiceValidationPrototype(dns_validate)
{
    int rval;

    if (!size)
        return SERVICE_INPROCESS;

    if (size < sizeof(DNSHeader))
    {
        rval = (dir == APP_ID_FROM_INITIATOR) ? SERVICE_INVALID_CLIENT:SERVICE_NOMATCH;
        goto done;
    }
    if ((rval = dns_validate_header(dir, (DNSHeader *)data)) != SERVICE_SUCCESS)
    {
        if (rval == SERVICE_REVERSED)
        {
            if (dir == APP_ID_FROM_RESPONDER)
            {
                if (flow_checkflag(flowp, FLOW_UDP_REVERSED))
                    goto success;
                goto nomatch;
            }
            else
            {
                rval = validate_packet(data, size, dir);
                if (rval == SERVICE_SUCCESS)
                {
                    flow_mark(flowp, FLOW_UDP_REVERSED);
                    goto inprocess;
                }
                goto invalid;
            }
        }
        rval = (dir == APP_ID_FROM_INITIATOR) ? SERVICE_INVALID_CLIENT:SERVICE_NOMATCH;
        goto done;
    }

    if (dir == APP_ID_FROM_INITIATOR)
        goto inprocess;

    rval = validate_packet(data, size, dir);
done:
    switch (rval)
    {
    case SERVICE_SUCCESS:
success:
        dns_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                         APP_ID_DNS, NULL, NULL, NULL);
        return SERVICE_SUCCESS;
    case SERVICE_INVALID_CLIENT:
invalid:
        dns_service_mod.api->incompatible_data(flowp, pkt, dir, &svc_element);
        return SERVICE_NOT_COMPATIBLE;
    case SERVICE_NOMATCH:
nomatch:
        dns_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
        return SERVICE_NOMATCH;
    case SERVICE_INPROCESS:
inprocess:
        dns_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
        return SERVICE_INPROCESS;
    default:
        return rval;
    }
}

MakeRNAServiceValidationPrototype(dns_tcp_validate)
{
    ServiceDNSData *dd;
    const DNSTCPHeader *hdr;
    uint16_t tmp;
    int rval;

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
    if (tmp < sizeof(DNSHeader) || dns_validate_header(dir, (DNSHeader *)data))
    {
        if (dir == APP_ID_FROM_INITIATOR)
            goto not_compatible;
        else
            goto fail;
    }

    dd = dns_service_mod.api->data_get(flowp);
    if (!dd)
    {
        dd = calloc(1, sizeof(*dd));
        if (!dd)
            return SERVICE_ENOMEM;
        if (dns_service_mod.api->data_add(flowp, dd, &free))
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
        goto inprocess;
    }
    else
    {
        if (dir != APP_ID_FROM_RESPONDER || dd->id != ((DNSHeader *)data)->id)
            goto fail;
    }
    if (tmp > size)
        goto not_compatible;

    rval = validate_packet(data, size, dir);
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
    dns_service_mod.api->add_service(flowp, pkt, dir, &tcp_svc_element,
                                     APP_ID_DNS, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

not_compatible:
    dns_service_mod.api->incompatible_data(flowp, pkt, dir, &tcp_svc_element);
    return SERVICE_NOT_COMPATIBLE;

fail:
    dns_service_mod.api->fail_service(flowp, pkt, dir, &tcp_svc_element);
    return SERVICE_NOMATCH;

inprocess:
    dns_service_mod.api->service_inprocess(flowp, pkt, dir, &tcp_svc_element);
    return SERVICE_INPROCESS;
}

