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

#include "flow.h"
#include "service_api.h"

#define FLAP_PORT   5190

typedef enum
{
    FLAP_STATE_ACK,
    FLAP_STATE_COOKIE
} FLAPState;

#define FNAC_SIGNON 0x0017
#define FNAC_GENERIC 0x0001
#define FNAC_SUB_SIGNON_REPLY 0x0007
#define FNAC_SUB_SERVER_READY 0x0003

typedef struct _SERVICE_FLAP_DATA
{
    FLAPState state;
} ServiceFLAPData;

#pragma pack(1)

typedef struct _FLAP_FNAC_SIGNON
{
    uint16_t len;
} FLAPFNACSignOn;

typedef struct _FLAP_FNAC
{
    uint16_t family;
    uint16_t subtype;
    uint16_t flags;
    uint32_t id;
} FLAPFNAC;

typedef struct _FLAP_TLV
{
    uint16_t subtype;
    uint16_t len;
} FLAPTLV;

typedef struct _FLAP_HEADER
{
    uint8_t start;
    uint8_t type;
    uint16_t seq;
    uint16_t len;
} FLAPHeader;

#pragma pack()

static int flap_init(const InitServiceAPI * const init_api);
static int flap_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &flap_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "flap",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&flap_validate, 5190, IPPROTO_TCP},
    {&flap_validate, 9898, IPPROTO_TCP},
    {&flap_validate, 4443, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule flap_service_mod =
{
    "FLAP",
    &flap_init,
    pp
};


static uint8_t FLAP_PATTERN[] = {0x2A, 0x01};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_AOL_INSTANT_MESSENGER, 0}};

static int flap_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&flap_validate, IPPROTO_TCP, FLAP_PATTERN, sizeof(FLAP_PATTERN), 0, "flap", init_api->pAppidConfig);
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&flap_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int flap_validate(ServiceValidationArgs* args)
{
    ServiceFLAPData *sf;
    const uint8_t *data = args->data;
    const FLAPHeader *hdr = (const FLAPHeader *)args->data;
    const FLAPFNAC *ff;
    const FLAPTLV *tlv;
    tAppIdData *flowp = args->flowp;
    uint16_t size = args->size;
    uint16_t len;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    sf = flap_service_mod.api->data_get(flowp, flap_service_mod.flow_data_index);
    if (!sf)
    {
        sf = calloc(1, sizeof(*sf));
        if (!sf)
            return SERVICE_ENOMEM;
        if (flap_service_mod.api->data_add(flowp, sf, flap_service_mod.flow_data_index, &free))
        {
            free(sf);
            return SERVICE_ENOMEM;
        }
        sf->state = FLAP_STATE_ACK;
    }

    switch (sf->state)
    {
    case FLAP_STATE_ACK:
        sf->state = FLAP_STATE_COOKIE;
        if (size < sizeof(FLAPHeader)) goto fail;
        if (hdr->start != 0x2A) goto fail;
        if (hdr->type != 0x01) goto fail;
        if (ntohs(hdr->len) != 4) goto fail;
        if (size - sizeof(FLAPHeader) != 4) goto fail;
        if (ntohl(*((uint32_t *)(data + sizeof(FLAPHeader)))) != 0x00000001)
            goto fail;
        goto inprocess;
    case FLAP_STATE_COOKIE:
        if (size < sizeof(FLAPHeader) + sizeof(FLAPFNAC))
            goto fail;
        if (hdr->start != 0x2A) goto fail;
        if ((uint16_t)ntohs(hdr->len) != (uint16_t)(size - sizeof(FLAPHeader)))
            goto fail;
        if (hdr->type == 0x02)
        {
            ff = (FLAPFNAC *)(data + sizeof(FLAPHeader));
            if (ntohs(ff->family) == FNAC_SIGNON)
            {
                FLAPFNACSignOn *ffs = (FLAPFNACSignOn *)((uint8_t *)ff + sizeof(FLAPFNAC));

                if (ntohs(ff->subtype) != FNAC_SUB_SIGNON_REPLY)
                    goto fail;
                if ((uint16_t)ntohs(ffs->len) != (uint16_t)(size -
                        (sizeof(FLAPHeader) +
                         sizeof(FLAPFNAC) +
                         sizeof(FLAPFNACSignOn))))
                    goto fail;
            }
            else if (ntohs(ff->family) == FNAC_GENERIC)
            {
                if (ntohs(ff->subtype) != FNAC_SUB_SERVER_READY)
                    goto fail;
            }
            else goto fail;
            goto success;
        }
        if (hdr->type == 0x04)
        {
            data += sizeof(FLAPHeader);
            size -= sizeof(FLAPHeader);
            while (size >= sizeof(FLAPTLV))
            {
                tlv = (FLAPTLV *)data;
                data += sizeof(FLAPTLV);
                size -= sizeof(FLAPTLV);
                len = ntohs(tlv->len);
                if (size < len) goto fail;
                size -= len;
                data += len;
            }
            if (size) goto fail;
            goto success;
        }
        goto fail;
    }

fail:
    flap_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                       flap_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;

success:
    flap_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                      APP_ID_AOL_INSTANT_MESSENGER, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

inprocess:
    flap_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;
}

