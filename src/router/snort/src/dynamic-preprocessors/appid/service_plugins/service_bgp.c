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

#include "flow.h"
#include "service_api.h"
#include "service_bgp.h"

#define BGP_PORT    179

#define BGP_V1_TYPE_OPEN 1
#define BGP_V1_TYPE_OPEN_CONFIRM 5
#define BGP_TYPE_OPEN 1
#define BGP_TYPE_KEEPALIVE 4

#define BGP_OPEN_LINK_MAX 3

#define BGP_VERSION_MAX 4
#define BGP_VERSION_MIN 2

typedef enum
{
    BGP_STATE_CONNECTION,
    BGP_STATE_OPENSENT
} BGPState;

#pragma pack(1)

typedef struct _SERVICE_BGP_DATA
{
    BGPState state;
    int v1;
} ServiceBGPData;

typedef union _SERVICE_BGP_HEADER
{
    struct
    {
        uint16_t marker;
        uint16_t len;
        uint8_t version;
        uint8_t type;
        uint16_t hold;
    } v1;
    struct
    {
        uint32_t marker[4];
        uint16_t len;
        uint8_t type;
    } v;
} ServiceBGPHeader;

typedef struct _SERVICE_BGP_OPEN
{
    uint8_t version;
    uint16_t as;
    uint16_t holdtime;
} ServiceBGPOpen;

typedef struct _SERVICE_BGP_V1_OPEN
{
    uint16_t system;
    uint8_t link;
    uint8_t auth;
} ServiceBGPV1Open;

#pragma pack()

static int bgp_init(const InitServiceAPI * const init_api);
static int bgp_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &bgp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "bgp",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&bgp_validate, BGP_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule bgp_service_mod =
{
    "BGP",
    &bgp_init,
    pp
};

static uint8_t BGP_PATTERN[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_BGP, 0}};

static int bgp_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&bgp_validate, IPPROTO_TCP, BGP_PATTERN, sizeof(BGP_PATTERN), 0, "bgp", init_api->pAppidConfig);
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&bgp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int bgp_validate(ServiceValidationArgs* args)
{
    ServiceBGPData *bd;
    const ServiceBGPHeader *bh;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;
    uint16_t len;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    if (size < sizeof(ServiceBGPHeader))
        goto fail;

    bd = bgp_service_mod.api->data_get(flowp, bgp_service_mod.flow_data_index);
    if (!bd)
    {
        bd = calloc(1, sizeof(*bd));
        if (!bd)
            return SERVICE_ENOMEM;
        if (bgp_service_mod.api->data_add(flowp, bd, bgp_service_mod.flow_data_index, &free))
        {
            free(bd);
            return SERVICE_ENOMEM;
        }
        bd->state = BGP_STATE_CONNECTION;
    }

    bh = (const ServiceBGPHeader *)data;
    switch (bd->state)
    {
    case BGP_STATE_CONNECTION:
        if (size >= sizeof(bh->v1) + sizeof(ServiceBGPV1Open) &&
            bh->v1.marker == 0xFFFF &&
            bh->v1.version == 0x01 && bh->v1.type == BGP_V1_TYPE_OPEN)
        {
            ServiceBGPV1Open *open;

            len = ntohs(bh->v1.len);
            if (len > 1024) goto fail;
            open = (ServiceBGPV1Open *)(data + sizeof(bh->v1));
            if (open->link > BGP_OPEN_LINK_MAX) goto fail;
            bd->v1 = 1;
        }
        else if (size >= sizeof(bh->v) + sizeof(ServiceBGPOpen) &&
                 bh->v.marker[0] == 0xFFFFFFFF &&
                 bh->v.marker[1] == 0xFFFFFFFF &&
                 bh->v.marker[2] == 0xFFFFFFFF &&
                 bh->v.marker[3] == 0xFFFFFFFF &&
                 bh->v.type == BGP_TYPE_OPEN)
        {
            ServiceBGPOpen *open;

            len = ntohs(bh->v.len);
            if (len > 4096) goto fail;
            open = (ServiceBGPOpen *)(data + sizeof(bh->v));
            if (open->version > BGP_VERSION_MAX ||
                open->version < BGP_VERSION_MIN)
            {
                goto fail;
            }
            bd->v1 = 0;
        }
        else goto fail;
        bd->state = BGP_STATE_OPENSENT;
        break;
    case BGP_STATE_OPENSENT:
        if (bd->v1)
        {
           if (size >= sizeof(bh->v1) && bh->v1.marker == 0xFFFF &&
               bh->v1.version == 0x01 &&
               bh->v1.type == BGP_V1_TYPE_OPEN_CONFIRM)
           {
               len = ntohs(bh->v1.len);
               if (len != sizeof(bh->v1)) goto fail;
               goto success;
           }
        }
        else
        {
            if (size >= sizeof(bh->v) &&
                bh->v.type == BGP_TYPE_KEEPALIVE)
            {
                len = ntohs(bh->v.len);
                if (len != sizeof(bh->v)) goto fail;
                goto success;
            }
        }
    default:
        goto fail;
    }

inprocess:
    bgp_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

fail:
    bgp_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                      bgp_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;

success:
    bgp_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                     APP_ID_BGP, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;
}

