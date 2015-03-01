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
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "flow.h"
#include "service_api.h"

#define SNMP_PORT   161

#define SNMP_VERSION_1  0
#define SNMP_VERSION_2c 1
#define SNMP_VERSION_2u 2
#define SNMP_VERSION_3  3

typedef enum
{
    SNMP_STATE_CONNECTION,
    SNMP_STATE_RESPONSE,
    SNMP_STATE_REQUEST,
    SNMP_STATE_R_RESPONSE,
    SNMP_STATE_R_REQUEST,
    SNMP_STATE_ERROR
} SNMPState;

typedef struct _SERVICE_SNMP_DATA
{
    SNMPState state;
} ServiceSNMPData;

typedef enum
{
    SNMP_PDU_GET_REQUEST,
    SNMP_PDU_GET_NEXT_REQUEST,
    SNMP_PDU_GET_RESPONSE,
    SNMP_PDU_SET_REQUEST,
    SNMP_PDU_TRAP,
    SNMP_PDU_GET_BULK_REQUEST,
    SNMP_PDU_INFORM_REQUEST,
    SNMP_PDU_TRAPV2
} SNMPPDUType;

#pragma pack(1)

typedef struct _SERVICE_SNMP_HEADER
{
    uint16_t opcode;
    union
    {
        uint16_t block;
        uint16_t errorcode;
    } d;
} ServiceSNMPHeader;

#pragma pack()

static int snmp_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(snmp_validate);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &snmp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "snmp",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&snmp_validate, SNMP_PORT, IPPROTO_TCP},
    {&snmp_validate, SNMP_PORT, IPPROTO_UDP},
    {&snmp_validate, 162, IPPROTO_UDP},
    {NULL, 0, 0}
};

RNAServiceValidationModule snmp_service_mod =
{
    "SNMP",
    &snmp_init,
    pp
};

static uint8_t SNMP_PATTERN_2[] = {0x02, 0x01, 0x00, 0x04};
static uint8_t SNMP_PATTERN_3[] = {0x02, 0x01, 0x01, 0x04};
static uint8_t SNMP_PATTERN_4[] = {0x02, 0x01, 0x03, 0x30};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_SNMP, APPINFO_FLAG_SERVICE_UDP_REVERSED}
};

static int16_t app_id = 0;

static int snmp_init(const InitServiceAPI * const init_api)
{
    app_id = init_api->dpd->addProtocolReference("snmp");

    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_2, sizeof(SNMP_PATTERN_2), 2, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_3, sizeof(SNMP_PATTERN_3), 2, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_4, sizeof(SNMP_PATTERN_4), 2, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_2, sizeof(SNMP_PATTERN_2), 3, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_3, sizeof(SNMP_PATTERN_3), 3, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_4, sizeof(SNMP_PATTERN_4), 3, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_2, sizeof(SNMP_PATTERN_2), 4, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_3, sizeof(SNMP_PATTERN_3), 4, "snmp");
    init_api->RegisterPattern(&snmp_validate, IPPROTO_UDP, SNMP_PATTERN_4, sizeof(SNMP_PATTERN_4), 4, "snmp");
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&snmp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, NULL);
	}

    return 0;
}

static int snmp_ans1_length(const uint8_t * * const data,
                            const uint8_t * const end,
                            uint32_t * const length)
{
    *length = 0;
    if (**data == 0x80) return -1;
    if (**data < 0x80)
    {
        *length = (uint32_t)**data;
        (*data)++;
    }
    else
    {
        int cnt = (**data) & 0x7F;
        (*data)++;
        for (; *data<end && cnt; cnt--, (*data)++)
        {
            *length <<= 8;
            *length |= **data;
        }
        if (cnt) return -1;
    }
    return 0;
}

static int snmp_verify_packet(const uint8_t * * const data,
                              const uint8_t * const end, uint8_t * const pdu)
{
    uint32_t overall_length;
    uint32_t community_length;
    uint32_t global_length;
    uint32_t length;
    uint8_t version;
    uint8_t cls;
    const uint8_t *p;

    if (**data != 0x30) return -1;
    (*data)++;
    if (*data >= end) return -1;
    if (snmp_ans1_length(data, end, &overall_length)) return -1;
    if (overall_length < 3 || (int)overall_length > end-(*data)) return -1;
    if (**data != 0x02) return -1;
    (*data)++;
    if (**data != 0x01) return -1;
    (*data)++;
    version = **data;
    (*data)++;
    overall_length -= 3;
    if (!overall_length) return -1;
    switch (version)
    {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
        if (**data != 0x04) return -1;
        (*data)++;
        overall_length--;
        if (!overall_length) return -1;
        p = *data;
        if (snmp_ans1_length(data, *data+overall_length, &community_length))
            return -1;
        overall_length -= *data - p;
        if (overall_length < community_length)
            return -1;
        for (;
             community_length;
             (*data)++, community_length--, overall_length--)
        {
            if (!isprint(**data)) return -1;
        }
        break;
    case SNMP_VERSION_2u:
        if (**data != 0x04) return -1;
        (*data)++;
        overall_length--;
        if (!overall_length) return -1;
        p = *data;
        if (snmp_ans1_length(data, *data+overall_length, &community_length))
            return -1;
        overall_length -= *data - p;
        if (!community_length || overall_length < community_length)
            return -1;
        if (**data != 1) return -1;
        *data += community_length;
        overall_length -= community_length;
        break;
    case SNMP_VERSION_3:
        /* Global header */
        if (**data != 0x30) return -1;
        (*data)++;
        overall_length--;
        if (!overall_length) return -1;
        p = *data;
        if (snmp_ans1_length(data, *data+overall_length, &global_length))
            return -1;
        overall_length -= *data - p;
        if (global_length < 2 || overall_length < global_length) return -1;

        /* Message id */
        if (**data != 0x02) return -1;
        (*data)++;
        global_length--;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+global_length, &length)) return -1;
        global_length -= *data - p;
        overall_length -= *data - p;
        if (global_length < length || length > sizeof(uint32_t)) return -1;
        *data += length;
        global_length -= length;
        overall_length -= length;

        /* Max message size */
        if (global_length < 2) return -1;
        if (**data != 0x02) return -1;
        (*data)++;
        global_length--;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+global_length, &length)) return -1;
        global_length -= *data - p;
        overall_length -= *data - p;
        if (global_length < length || length > sizeof(uint32_t)) return -1;
        *data += length;
        global_length -= length;
        overall_length -= length;

        /* Message flags */
        if (global_length < 2) return -1;
        if (**data != 0x04) return -1;
        (*data)++;
        global_length--;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+global_length, &length))
            return -1;
        global_length -= *data - p;
        overall_length -= *data - p;
        if (length != 1 || global_length < length) return -1;
        (*data)++;
        global_length--;
        overall_length--;

        /* Security model */
        if (global_length < 2) return -1;
        if (**data != 0x02) return -1;
        (*data)++;
        global_length--;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+global_length, &length))
            return -1;
        global_length -= *data - p;
        overall_length -= *data - p;
        if (global_length < length || length > sizeof(uint32_t)) return -1;
        *data += length;
        global_length -= length;
        overall_length -= length;

        /* Security Parameters */
        if (overall_length < 2) return -1;
        if (**data != 0x04) return -1;
        (*data)++;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+overall_length, &global_length))
            return -1;
        overall_length -= *data - p;
        if (overall_length < global_length) return -1;
        *data += global_length;
        overall_length -= global_length;

        /* Message */
        if (overall_length < 2) return -1;
        if (**data != 0x30) return -1;
        (*data)++;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+overall_length, &global_length))
            return -1;
        overall_length -= *data - p;
        if (overall_length < global_length) return -1;

        /* Context Engine ID */
        if (global_length < 2) return -1;
        if (**data != 0x04) return -1;
        (*data)++;
        global_length--;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+global_length, &length))
            return -1;
        global_length -= *data - p;
        overall_length -= *data - p;
        if (global_length < length) return -1;
        *data += length;
        global_length -= length;
        overall_length -= length;

        /* Context Name */
        if (global_length < 2) return -1;
        if (**data != 0x04) return -1;
        (*data)++;
        global_length--;
        overall_length --;
        p = *data;
        if (snmp_ans1_length(data, *data+global_length, &length))
            return -1;
        global_length -= *data - p;
        overall_length -= *data - p;
        if (global_length < length) return -1;
        *data += length;
        global_length -= length;
        overall_length -= length;
        break;
    default:
        return -1;
    }
    if (!overall_length) return -1;
    cls = (**data) & 0xC0;
    if (cls != 0x80 && cls != 0x40) return -1;
    *pdu = (**data) & 0x1F;
    return 0;
}

MakeRNAServiceValidationPrototype(snmp_validate)
{
    ServiceSNMPData *sd;
    ServiceSNMPData *tmp_sd;
    FLOW *pf;
    uint8_t pdu;
    snort_ip *sip;
    snort_ip *dip;

    if (!size)
        goto inprocess;

    sd = snmp_service_mod.api->data_get(flowp);
    if (!sd)
    {
        sd = calloc(1, sizeof(*sd));
        if (!sd)
            return SERVICE_ENOMEM;
        if (snmp_service_mod.api->data_add(flowp, sd, &free))
        {
            free(sd);
            return SERVICE_ENOMEM;
        }
        sd->state = SNMP_STATE_CONNECTION;
    }

    if (snmp_verify_packet(&data, data+size, &pdu))
    {
        if (flow_checkflag(flowp, FLOW_UDP_REVERSED))
        {
            if (dir == APP_ID_FROM_RESPONDER) goto bail;
            else goto fail;
        }
        else
        {
            if (dir == APP_ID_FROM_RESPONDER) goto fail;
            else goto bail;
        }
    }

    switch (sd->state)
    {
    case SNMP_STATE_CONNECTION:
        if (pdu != SNMP_PDU_GET_RESPONSE && dir == APP_ID_FROM_RESPONDER)
        {
            sd->state = SNMP_STATE_R_RESPONSE;
            flow_mark(flowp, FLOW_UDP_REVERSED);
            break;
        }
        if (pdu == SNMP_PDU_GET_RESPONSE && dir == APP_ID_FROM_INITIATOR)
        {
            sd->state = SNMP_STATE_R_REQUEST;
            flow_mark(flowp, FLOW_UDP_REVERSED);
            break;
        }

        if (dir == APP_ID_FROM_RESPONDER)
        {
            sd->state = SNMP_STATE_REQUEST;
            break;
        }

        if (pdu == SNMP_PDU_TRAP || pdu == SNMP_PDU_TRAPV2)
        {
            flow_mark(flowp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE);
            flow_clear(flowp, FLOW_CONTINUE);
            flowp->serviceAppId = APP_ID_SNMP;
            break;
        }
        sd->state = SNMP_STATE_RESPONSE;

        /*adding expected connection to account for future traps */
        dip = GET_DST_IP(pkt);
        sip = GET_SRC_IP(pkt);
        pf = snmp_service_mod.api->flow_new(pkt, sip, pkt->src_port, dip, 0, flowp->proto, app_id);
        if (pf)
        {
            tmp_sd = calloc(1, sizeof(ServiceSNMPData));
            if (tmp_sd == NULL) return SERVICE_ENOMEM;
            tmp_sd->state = SNMP_STATE_RESPONSE;

            if (snmp_service_mod.api->data_add(pf, tmp_sd, &free))
            {
                free(tmp_sd);
                return SERVICE_ENOMEM;
            }
            if (snmp_service_mod.api->data_add_id(pf, pkt->dst_port, &svc_element))
            {
                flow_mark(pf, FLOW_SERVICEDETECTED);
                flow_clear(pf, FLOW_CONTINUE);
                tmp_sd->state = SNMP_STATE_ERROR;
                return SERVICE_ENULL;
            }
        }
        break;
    case SNMP_STATE_RESPONSE:
        if (pdu == SNMP_PDU_GET_RESPONSE)
        {
            if (dir == APP_ID_FROM_RESPONDER) goto success;
            goto fail;
        }
        if (dir == APP_ID_FROM_RESPONDER) goto fail;
        break;
    case SNMP_STATE_REQUEST:
        if (pdu != SNMP_PDU_GET_RESPONSE)
        {
            if (dir == APP_ID_FROM_INITIATOR) goto success;
            goto fail;
        }
        if (dir == APP_ID_FROM_INITIATOR) goto fail;
        break;
    case SNMP_STATE_R_RESPONSE:
        if (pdu == SNMP_PDU_GET_RESPONSE)
        {
            if (dir == APP_ID_FROM_INITIATOR) goto success;
            goto fail;
        }
        if (dir == APP_ID_FROM_INITIATOR) goto fail;
        break;
    case SNMP_STATE_R_REQUEST:
        if (pdu != SNMP_PDU_GET_RESPONSE)
        {
            if (dir == APP_ID_FROM_RESPONDER) goto success;
            goto fail;
        }
        if (dir == APP_ID_FROM_RESPONDER) goto fail;
        break;
    default:
        if (dir == APP_ID_FROM_RESPONDER) goto fail;
        else goto bail;
    }

inprocess:
    snmp_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
    return SERVICE_INPROCESS;

success:
    snmp_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                      APP_ID_SNMP, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

bail:
    snmp_service_mod.api->incompatible_data(flowp, pkt, dir, &svc_element);
    return SERVICE_NOT_COMPATIBLE;

fail:
    snmp_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
    return SERVICE_NOMATCH;
}

