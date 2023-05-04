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

#include "appInfoTable.h"
#include "flow.h"
#include "service_api.h"

#define RADIUS_CODE_ACCESS_REQUEST       1
#define RADIUS_CODE_ACCESS_ACCEPT        2
#define RADIUS_CODE_ACCESS_REJECT        3
#define RADIUS_CODE_ACCOUNTING_REQUEST   4
#define RADIUS_CODE_ACCOUNTING_RESPONSE  5
#define RADIUS_CODE_ACCESS_CHALLENGE    11

typedef enum
{
    RADIUS_STATE_REQUEST,
    RADIUS_STATE_RESPONSE
} RADIUSState;

typedef struct _SERVICE_RADIUS_DATA
{
    RADIUSState state;
    uint8_t id;
} ServiceRADIUSData;

#pragma pack(1)

typedef struct _RADIUS_HEADER {
    uint8_t code;
    uint8_t id;
    uint16_t length;
    uint8_t auth[16];
} RADIUSHeader;

#pragma pack()

static int radius_init(const InitServiceAPI * const init_api);
static int radius_validate(ServiceValidationArgs* args);
static int radius_validate_accounting(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &radius_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "radius",
    .ref_count = 1,
    .current_ref_count = 1,
};
static tRNAServiceElement acct_svc_element =
{
    .next = NULL,
    .validate = &radius_validate_accounting,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "radacct",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&radius_validate, 1812, IPPROTO_UDP},
    {&radius_validate, 1812, IPPROTO_UDP, 1},
    {&radius_validate_accounting, 1813, IPPROTO_UDP},
    {&radius_validate_accounting, 1813, IPPROTO_UDP, 1},
    {NULL, 0, 0}
};

tRNAServiceValidationModule radius_service_mod =
{
    "RADIUS",
    &radius_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_RADIUS_ACCT, APPINFO_FLAG_SERVICE_UDP_REVERSED},
    {APP_ID_RADIUS, APPINFO_FLAG_SERVICE_UDP_REVERSED}
};

static int radius_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&radius_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int radius_validate(ServiceValidationArgs* args)
{
    ServiceRADIUSData *rd;
    const RADIUSHeader *hdr = (const RADIUSHeader *)args->data;
    uint16_t len;
    int new_dir;
    tAppIdData *flowp = args->flowp;
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (size < sizeof(RADIUSHeader))
        goto fail;

    rd = radius_service_mod.api->data_get(flowp, radius_service_mod.flow_data_index);
    if (!rd)
    {
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (radius_service_mod.api->data_add(flowp, rd, radius_service_mod.flow_data_index, &free))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = RADIUS_STATE_REQUEST;
    }

    new_dir = dir;
    if (rd->state == RADIUS_STATE_REQUEST)
    {
        if (hdr->code == RADIUS_CODE_ACCESS_ACCEPT ||
            hdr->code == RADIUS_CODE_ACCESS_REJECT ||
            hdr->code == RADIUS_CODE_ACCESS_CHALLENGE)
        {
            setAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED);
            rd->state = RADIUS_STATE_RESPONSE;
            new_dir = APP_ID_FROM_RESPONDER;
        }
    }
    else if (getAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED))
    {
        new_dir = (dir == APP_ID_FROM_RESPONDER) ? APP_ID_FROM_INITIATOR:APP_ID_FROM_RESPONDER;
    }

    switch (rd->state)
    {
    case RADIUS_STATE_REQUEST:
        if (new_dir != APP_ID_FROM_INITIATOR) goto inprocess;
        if (hdr->code != RADIUS_CODE_ACCESS_REQUEST)
        {
            goto not_compatible;
        }
        len = ntohs(hdr->length);
        if (len > size)
        {
            goto not_compatible;
        }
        /* Must contain a username attribute */
        if (len < sizeof(RADIUSHeader)+3)
        {
            goto not_compatible;
        }
        rd->id = hdr->id;
        rd->state = RADIUS_STATE_RESPONSE;
        break;
    case RADIUS_STATE_RESPONSE:
        if (new_dir != APP_ID_FROM_RESPONDER) goto inprocess;
        if (hdr->code != RADIUS_CODE_ACCESS_ACCEPT &&
            hdr->code != RADIUS_CODE_ACCESS_REJECT &&
            hdr->code != RADIUS_CODE_ACCESS_CHALLENGE)
        {
            goto fail;
        }
        len = ntohs(hdr->length);
        if (len > size) goto fail;
        /* Must contain a username attribute */
        if (len < sizeof(RADIUSHeader)) goto fail;
        if (hdr->id != rd->id)
        {
            rd->state = RADIUS_STATE_REQUEST;
            goto inprocess;
        }
        goto success;
    default:
        goto fail;
    }
inprocess:
    radius_service_mod.api->service_inprocess(flowp, args->pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    radius_service_mod.api->add_service(flowp, args->pkt, dir, &svc_element,
                                        APP_ID_RADIUS, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

not_compatible:
    radius_service_mod.api->incompatible_data(flowp, args->pkt, dir, &svc_element,
                                              radius_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOT_COMPATIBLE;

fail:
    radius_service_mod.api->fail_service(flowp, args->pkt, dir, &svc_element,
                                         radius_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

static int radius_validate_accounting(ServiceValidationArgs* args)
{
    ServiceRADIUSData *rd;
    const RADIUSHeader *hdr = (const RADIUSHeader *)args->data;
    uint16_t len;
    int new_dir;
    tAppIdData *flowp = args->flowp;
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (size < sizeof(RADIUSHeader))
        goto fail;

    rd = radius_service_mod.api->data_get(flowp, radius_service_mod.flow_data_index);
    if (!rd)
    {
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (radius_service_mod.api->data_add(flowp, rd, radius_service_mod.flow_data_index, &free))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = RADIUS_STATE_REQUEST;
    }

    new_dir = dir;
    if (rd->state == RADIUS_STATE_REQUEST)
    {
        if (hdr->code == RADIUS_CODE_ACCOUNTING_RESPONSE)
        {
            setAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED);
            rd->state = RADIUS_STATE_RESPONSE;
            new_dir = APP_ID_FROM_RESPONDER;
        }
    }
    else if (getAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED))
    {
        new_dir = (dir == APP_ID_FROM_RESPONDER) ? APP_ID_FROM_INITIATOR:APP_ID_FROM_RESPONDER;
    }

    switch (rd->state)
    {
    case RADIUS_STATE_REQUEST:
        if (new_dir != APP_ID_FROM_INITIATOR) goto inprocess;
        if (hdr->code != RADIUS_CODE_ACCOUNTING_REQUEST)
        {
            goto not_compatible;
        }
        len = ntohs(hdr->length);
        if (len > size)
        {
            goto not_compatible;
        }
        /* Must contain a username attribute */
        if (len < sizeof(RADIUSHeader)+3)
        {
            goto not_compatible;
        }
        rd->id = hdr->id;
        rd->state = RADIUS_STATE_RESPONSE;
        break;
    case RADIUS_STATE_RESPONSE:
        if (new_dir != APP_ID_FROM_RESPONDER) goto inprocess;
        if (hdr->code != RADIUS_CODE_ACCOUNTING_RESPONSE)
            goto fail;
        len = ntohs(hdr->length);
        if (len > size) goto fail;
        /* Must contain a NAS-IP-Address or NAS-Identifier attribute */
        if (len < sizeof(RADIUSHeader)) goto fail;
        if (hdr->id != rd->id)
        {
            rd->state = RADIUS_STATE_REQUEST;
            goto inprocess;
        }
        goto success;
    default:
        goto fail;
    }
inprocess:
    radius_service_mod.api->service_inprocess(flowp, args->pkt, dir, &acct_svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    radius_service_mod.api->add_service(flowp, args->pkt, dir, &acct_svc_element,
                                        APP_ID_RADIUS_ACCT, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

not_compatible:
    radius_service_mod.api->incompatible_data(flowp, args->pkt, dir, &acct_svc_element,
                                              radius_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOT_COMPATIBLE;

fail:
    radius_service_mod.api->fail_service(flowp, args->pkt, dir, &acct_svc_element,
                                         radius_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

