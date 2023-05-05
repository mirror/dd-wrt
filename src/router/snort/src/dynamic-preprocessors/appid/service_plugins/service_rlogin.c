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

#define RLOGIN_PASSWORD "Password: "
typedef enum
{
    RLOGIN_STATE_HANDSHAKE,
    RLOGIN_STATE_PASSWORD,
    RLOGIN_STATE_CRLF,
    RLOGIN_STATE_DATA,
    RLOGIN_STATE_DONE
} RLOGINState;

typedef struct _SERVICE_RLOGIN_DATA
{
    RLOGINState state;
} ServiceRLOGINData;

static int rlogin_init(const InitServiceAPI * const init_api);
static int rlogin_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &rlogin_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "rlogin",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&rlogin_validate, 513, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule rlogin_service_mod =
{
    "RLOGIN",
    &rlogin_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_RLOGIN, 0}};

static int rlogin_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&rlogin_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int rlogin_validate(ServiceValidationArgs* args)
{
    ServiceRLOGINData *rd;
    tAppIdData *flowp = args->flowp;
    SFSnortPacket *pkt = args->pkt; 
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    rd = rlogin_service_mod.api->data_get(flowp, rlogin_service_mod.flow_data_index);
    if (!rd)
    {
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (rlogin_service_mod.api->data_add(flowp, rd, rlogin_service_mod.flow_data_index, &free))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = RLOGIN_STATE_HANDSHAKE;
    }

    switch (rd->state)
    {
    case RLOGIN_STATE_HANDSHAKE:
        if (size != 1) goto fail;
        if (*data) goto fail;
        rd->state = RLOGIN_STATE_PASSWORD;
        break;
    case RLOGIN_STATE_PASSWORD:
        if ((pkt->tcp_header->flags & TCPHEADER_URG) &&
            size >= ntohs(pkt->tcp_header->urgent_pointer))
        {
            if (size != 1) goto fail;
            if (*data != 0x80) goto fail;
            rd->state = RLOGIN_STATE_DATA;
        }
        else
        {
            if (size != sizeof(RLOGIN_PASSWORD)-1) goto fail;
            if (strncmp((char *)data, RLOGIN_PASSWORD, sizeof(RLOGIN_PASSWORD)-1))
                goto fail;
            rd->state = RLOGIN_STATE_CRLF;
        }
        break;
    case RLOGIN_STATE_CRLF:
        if (size != 2) goto fail;
        if (*data != 0x0A || *(data+1) != 0x0D) goto fail;
        rd->state = RLOGIN_STATE_DATA;
        break;
    case RLOGIN_STATE_DATA:
        rd->state = RLOGIN_STATE_DONE;
        goto success;
    default:
        goto fail;
    }

inprocess:
    rlogin_service_mod.api->service_inprocess(flowp, pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    rlogin_service_mod.api->add_service(flowp, pkt, args->dir, &svc_element,
                                        APP_ID_RLOGIN, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    rlogin_service_mod.api->fail_service(flowp, pkt, args->dir, &svc_element,
                                         rlogin_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

