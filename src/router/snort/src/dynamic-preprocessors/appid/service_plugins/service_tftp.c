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

#include "appIdApi.h"
#include "appInfoTable.h"
#include "flow.h"
#include "service_api.h"

#define TFTP_PORT   69

#define TFTP_COUNT_THRESHOLD 1
#define TFTP_MAX_PACKET_SIZE 512

typedef enum
{
    TFTP_STATE_CONNECTION,
    TFTP_STATE_TRANSFER,
    TFTP_STATE_ACK,
    TFTP_STATE_DATA,
    TFTP_STATE_ERROR
} TFTPState;

typedef struct _SERVICE_TFTP_DATA
{
    TFTPState state;
    unsigned count;
    int last;
    uint16_t block;
} ServiceTFTPData;

#pragma pack(1)

typedef struct _SERVICE_TFTP_HEADER
{
    uint16_t opcode;
    union
    {
        uint16_t block;
        uint16_t errorcode;
    } d;
} ServiceTFTPHeader;

#pragma pack()

static int tftp_init(const InitServiceAPI * const api);
static int tftp_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &tftp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "tftp",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&tftp_validate, 69, IPPROTO_UDP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule tftp_service_mod =
{
    "TFTP",
    &tftp_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_TFTP, APPINFO_FLAG_SERVICE_ADDITIONAL}};

static int16_t app_id;

static int tftp_init(const InitServiceAPI * const init_api)
{
    unsigned i;
#ifdef TARGET_BASED
    app_id = init_api->dpd->addProtocolReference("tftp");

    for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
        init_api->RegisterAppId(&tftp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
    }
#endif

    return 0;
}

static int tftp_verify_header(const uint8_t *data, uint16_t size,
                              uint16_t *block)
{
    const ServiceTFTPHeader *hdr;

    if (size < sizeof(ServiceTFTPHeader)) return -1;
    hdr = (ServiceTFTPHeader *)data;
    switch (ntohs(hdr->opcode))
    {
    case 3:
        if (size > sizeof(ServiceTFTPHeader) + TFTP_MAX_PACKET_SIZE)
            return -1;
        *block = ntohs(hdr->d.block);
        return TFTP_STATE_DATA;
    case 4:
        if (size != sizeof(ServiceTFTPHeader)) return -1;
        *block = ntohs(hdr->d.block);
        return TFTP_STATE_ACK;
    case 5:
        if (ntohs(hdr->d.errorcode) > 7) return -1;
        if (size <= sizeof(ServiceTFTPHeader)) return -1;
        if (data[size-1] != 0) return -1;
        return TFTP_STATE_ERROR;
    default:
        return -1;
    }
}

static int tftp_validate(ServiceValidationArgs* args)
{
    ServiceTFTPData *td;
    ServiceTFTPData *tmp_td;
    int mode;
    uint16_t block = 0;
    uint16_t tmp;
    tAppIdData *pf;
    sfaddr_t *sip;
    sfaddr_t *dip;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    SFSnortPacket *pkt = args->pkt;
    const int dir = args->dir;
    uint16_t size = args->size;
    char* app_id_debug_session = args->app_id_debug_session;

    if (!size)
        goto inprocess;

    td = tftp_service_mod.api->data_get(flowp, tftp_service_mod.flow_data_index);
    if (!td)
    {
        td = calloc(1, sizeof(*td));
        if (!td)
            return SERVICE_ENOMEM;
        if (tftp_service_mod.api->data_add(flowp, td, tftp_service_mod.flow_data_index, &free))
        {
            free(td);
            return SERVICE_ENOMEM;
        }
        td->state = TFTP_STATE_CONNECTION;
    }
    if (args->app_id_debug_session_flag)
        _dpd.logMsg("AppIdDbg %s tftp state %d\n", app_id_debug_session, td->state);

    if (td->state == TFTP_STATE_CONNECTION && dir == APP_ID_FROM_RESPONDER)
        goto fail;
    if ((td->state == TFTP_STATE_TRANSFER || td->state == TFTP_STATE_DATA) &&
        dir == APP_ID_FROM_INITIATOR)
    {
        goto inprocess;
    }
    switch (td->state)
    {
    case TFTP_STATE_CONNECTION:
        if (size < 6) goto bail;
        tmp = ntohs(*((uint16_t *)data));
        if (tmp != 0x0001 && tmp != 0x0002) goto bail;
        data += sizeof(uint16_t);
        size -= sizeof(uint16_t);
        if (!(*data)) goto bail;
        for (; *data && size; data++, size--)
        {
            if (!isprint(*data)) goto bail;
        }
        if (!size) goto bail;
        size--;
        data++;
        if (!size || !(*data)) goto bail;
        if (data[size-1]) goto bail;
        if (strcasecmp((char *)data, "netascii") && strcasecmp((char *)data, "octet"))
            goto bail;

        tmp_td = calloc(1, sizeof(ServiceTFTPData));
        if (tmp_td == NULL) return SERVICE_ENOMEM;
        tmp_td->state = TFTP_STATE_TRANSFER;

        dip = GET_DST_IP(pkt);
        sip = GET_SRC_IP(pkt);
        pf = tftp_service_mod.api->flow_new(flowp, pkt, dip, 0, sip, pkt->src_port, flowp->proto, app_id, APPID_EARLY_SESSION_FLAG_FW_RULE);
        if (pf)
        {
            if (tftp_service_mod.api->data_add(pf, tmp_td, tftp_service_mod.flow_data_index, &free))
            {
                free(tmp_td);
                return SERVICE_ENOMEM;
            }
            if (tftp_service_mod.api->data_add_id(pf, pkt->dst_port, &svc_element))
            {
                setAppIdFlag(pf, APPID_SESSION_SERVICE_DETECTED);
                clearAppIdFlag(pf, APPID_SESSION_CONTINUE);
                tmp_td->state = TFTP_STATE_ERROR;
                return SERVICE_ENOMEM;
            }
            PopulateExpectedFlow(flowp, pf, APPID_SESSION_EXPECTED_EVALUATE, APP_ID_FROM_RESPONDER);
            sfaddr_copy_to_raw(&pf->common.initiator_ip, sip);
            pf->rnaServiceState = RNA_STATE_STATEFUL;
            pf->scan_flags |= SCAN_HOST_PORT_FLAG;
        }
        else
        {
            free(tmp_td);
            goto inprocess;   /* Assume that the flow already exists
                                 as in a retransmit situation */
        }
        break;
    case TFTP_STATE_TRANSFER:
        if ((mode=tftp_verify_header(data, size, &block)) < 0)
        {
            if (args->app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s tftp failed to verify\n", app_id_debug_session);
            goto fail;
        }
        if (args->app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s tftp mode %d and block %u\n", app_id_debug_session,
                        mode, (unsigned)block);
        if (mode == TFTP_STATE_ACK)
        {
            if (block != 0)
            {
                td->state = TFTP_STATE_ERROR;
                goto fail;
            }
            td->last = 0;
            td->block = 0;
            td->state = TFTP_STATE_ACK;
        }
        else if (mode == TFTP_STATE_DATA)
        {
            if (block != 1)
            {
                td->state = TFTP_STATE_ERROR;
                goto fail;
            }
            td->block = 1;
            td->state = TFTP_STATE_DATA;
        }
        else if (mode == TFTP_STATE_ERROR) break;
        else
        {
            td->state = TFTP_STATE_ERROR;
            goto fail;
        }
        break;
    case TFTP_STATE_ACK:
        if ((mode=tftp_verify_header(data, size, &block)) < 0)
        {
            if (dir == APP_ID_FROM_RESPONDER) goto fail;
            else
            {
                if (args->app_id_debug_session_flag)
                    _dpd.logMsg("AppIdDbg %s tftp failed to verify\n", app_id_debug_session);
                goto bail;
            }
        }
        if (args->app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s tftp mode %d\n", app_id_debug_session, mode);
        if (mode == TFTP_STATE_ERROR)
        {
            td->state = TFTP_STATE_TRANSFER;
            break;
        }
        if (dir == APP_ID_FROM_INITIATOR && mode != TFTP_STATE_DATA)
        {
            if (args->app_id_debug_session_flag)
                _dpd.logMsg("AppIdDbg %s tftp bad mode\n", app_id_debug_session);
            goto bail;
        }
        if (dir == APP_ID_FROM_RESPONDER && mode != TFTP_STATE_ACK)
            goto fail;
        if (dir == APP_ID_FROM_INITIATOR)
        {
            if (size < sizeof(ServiceTFTPHeader) + TFTP_MAX_PACKET_SIZE)
                td->last = 1;
            break;
        }
        if (block == (uint16_t)(td->block + 1))
            td->block++;
        else if (block != td->block)
            goto fail;
        td->count++;
        if (td->count >= TFTP_COUNT_THRESHOLD)
            goto success;
        if (td->last)
            td->state = TFTP_STATE_TRANSFER;
        break;
    case TFTP_STATE_DATA:
        if ((mode=tftp_verify_header(data, size, &block)) < 0)
            goto fail;
        if (mode == TFTP_STATE_ERROR) td->state = TFTP_STATE_TRANSFER;
        else if (mode != TFTP_STATE_DATA) goto fail;
        if (block == (uint16_t)(td->block + 1))
            td->block++;
        else if (block != td->block)
            goto fail;
        td->count++;
        if (td->count >= TFTP_COUNT_THRESHOLD)
            goto success;
        if (size < sizeof(ServiceTFTPHeader) + TFTP_MAX_PACKET_SIZE)
            td->state = TFTP_STATE_TRANSFER;
        break;
    case TFTP_STATE_ERROR:
    default:
        goto fail;
    }

inprocess:
    tftp_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    if (args->app_id_debug_session_flag)
        _dpd.logMsg("AppIdDbg %s tftp success\n", app_id_debug_session);
    tftp_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                      APP_ID_TFTP, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

bail:
    tftp_service_mod.api->incompatible_data(flowp, pkt, dir, &svc_element,
                                            tftp_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOT_COMPATIBLE;

fail:
    tftp_service_mod.api->fail_service(flowp, pkt, dir, &svc_element,
                                       tftp_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

