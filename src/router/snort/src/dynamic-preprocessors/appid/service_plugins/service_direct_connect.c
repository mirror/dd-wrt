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


typedef enum {
    CONN_STATE_INIT,
    CONN_STATE_1,
    CONN_STATE_2,
    CONN_STATE_SERVICE_DETECTED,
    CONN_STATE_MAX
} CONNECTION_STATES;

#define MAX_PACKET_INSPECTION_COUNT      10

typedef struct _SERVICE_DATA
{
    uint32_t state;
    uint32_t packetCount;
} tServiceData;

static int direct_connect_init(const InitServiceAPI * const init_api);
static int direct_connect_validate(ServiceValidationArgs* args);
static int validateDirectConnectTcp(const uint8_t *data, uint16_t size, const int dir, 
        tAppIdData *flowp, const SFSnortPacket *pkt, tServiceData *serviceData,
        const struct appIdConfig_ *pConfig);
static int validateDirectConnectUdp(const uint8_t *data, uint16_t size, const int dir, 
        tAppIdData *flowp, const SFSnortPacket *pkt, tServiceData *serviceData,
        const struct appIdConfig_ *pConfig);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &direct_connect_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "direct_connect",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&direct_connect_validate, 411, IPPROTO_TCP},
    {&direct_connect_validate, 411, IPPROTO_UDP},
    {&direct_connect_validate, 412, IPPROTO_TCP},
    {&direct_connect_validate, 412, IPPROTO_UDP},
    {&direct_connect_validate, 413, IPPROTO_TCP},
    {&direct_connect_validate, 413, IPPROTO_UDP},
    {&direct_connect_validate, 414, IPPROTO_TCP},
    {&direct_connect_validate, 414, IPPROTO_UDP},
    {NULL, 0, 0}
};

#define PATTERN1     "$Lock "
#define PATTERN2     "$MyNick "
#define PATTERN3     "HSUP ADBAS0"
#define PATTERN4     "HSUP ADBASE"
#define PATTERN5     "CSUP ADBAS0"
#define PATTERN6     "CSUP ADBASE"
#define PATTERN7     "$SR "


tRNAServiceValidationModule directconnect_service_mod =
{
    "DirectConnect",
    &direct_connect_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_DIRECT_CONNECT, 0}};

static int direct_connect_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_TCP, (uint8_t *)PATTERN1, sizeof(PATTERN1)-1,  0, "direct_connect", init_api->pAppidConfig);
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_TCP, (uint8_t *)PATTERN2, sizeof(PATTERN2)-1,  0, "direct_connect", init_api->pAppidConfig);
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_TCP, (uint8_t *)PATTERN3, sizeof(PATTERN3)-1,  0, "direct_connect", init_api->pAppidConfig);
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_TCP, (uint8_t *)PATTERN4, sizeof(PATTERN4)-1,  0, "direct_connect", init_api->pAppidConfig);
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_TCP, (uint8_t *)PATTERN5, sizeof(PATTERN5)-1,  0, "direct_connect", init_api->pAppidConfig);
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_TCP, (uint8_t *)PATTERN6, sizeof(PATTERN6)-1,  0, "direct_connect", init_api->pAppidConfig);
    init_api->RegisterPattern(&direct_connect_validate, IPPROTO_UDP, (uint8_t *)PATTERN7, sizeof(PATTERN7)-1,  0, "direct_connect", init_api->pAppidConfig);

	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&direct_connect_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}


static int direct_connect_validate(ServiceValidationArgs* args)
{
    tServiceData *fd;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (!size)
    {
        directconnect_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
        return SERVICE_INPROCESS;
    }

    fd = directconnect_service_mod.api->data_get(flowp, directconnect_service_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return SERVICE_ENOMEM;
        if (directconnect_service_mod.api->data_add(flowp, fd, directconnect_service_mod.flow_data_index, &free))
        {
            free(fd);
            return SERVICE_ENOMEM;
        }
    }

    if (flowp->proto == IPPROTO_TCP)
        return validateDirectConnectTcp(data, size, args->dir, flowp, args->pkt, fd, args->pConfig);
    else 
        return validateDirectConnectUdp(data, size, args->dir, flowp, args->pkt, fd, args->pConfig);
}

static int validateDirectConnectTcp(const uint8_t *data, uint16_t size, const int dir, 
        tAppIdData *flowp, const SFSnortPacket *pkt, tServiceData *serviceData,
        const struct appIdConfig_ *pConfig)
{
    
    switch (serviceData->state)
    {
        case CONN_STATE_INIT:
            if (size > 6 
                    && data[size-2] == '|'
                    && data[size-1] == '$')
            {
                if (memcmp(data, PATTERN1, sizeof(PATTERN1)-1) == 0) 
                {
                    printf("maybe first directconnect to hub  detected\n");
                    serviceData->state = CONN_STATE_1;
                    goto inprocess;
                }

                if (memcmp(data, PATTERN2, sizeof(PATTERN2)-1) == 0) 
                {
                    printf("maybe first dc connect between peers  detected\n");
                    serviceData->state = CONN_STATE_2;
                    goto inprocess;
                }
            }

            if (size >= 11) 
            {
                if (memcmp(data, PATTERN3, sizeof(PATTERN3)-1) == 0 
                        || memcmp(data, PATTERN4, sizeof(PATTERN4)-1) == 0 
                        || memcmp(data, PATTERN5, sizeof(PATTERN5)-1) == 0 
                        || memcmp(data, PATTERN6, sizeof(PATTERN6)-1) == 0) 
                {
                    goto success;
                }
            }
            break;

        case CONN_STATE_1:
            printf ("ValidateDirectConnectTcp(): state 1 size %d\n", size);
            if (size >= 11) 
            {
                if (memcmp(data, PATTERN3, sizeof(PATTERN3)-1) == 0 
                        || memcmp(data, PATTERN4, sizeof(PATTERN4)-1) == 0 
                        || memcmp(data, PATTERN5, sizeof(PATTERN5)-1) == 0 
                        || memcmp(data, PATTERN6, sizeof(PATTERN6)-1) == 0) 
                {
                    printf("found directconnect HSUP ADBAS E in second packet\n");
                    goto success;
                }
            }

            if (size > 6) 
            {
                if ((data[0] == '$' || data[0] == '<')
                        && data[size-2] == '|'
                        && data[size-1] == '$')
                {
                    goto success;
                }
                else
                {
                    goto inprocess;
                }
            }
            break;

        case CONN_STATE_2: 
            if (size > 6) 
            {
                if (data[0] == '$' && data[size-2] == '|' && data[size-1] == '$')
                {
                    goto success;
                }
                else
                {
                    goto inprocess;
                }
            }
            break;

        case CONN_STATE_SERVICE_DETECTED: 
            goto success;
    }

inprocess:
    serviceData->packetCount++;
    if (serviceData->packetCount >= MAX_PACKET_INSPECTION_COUNT)
        goto fail;

    directconnect_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    if (dir != APP_ID_FROM_RESPONDER)
    {
        serviceData->state = CONN_STATE_SERVICE_DETECTED;
        goto inprocess;
    }

    directconnect_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                      APP_ID_DIRECT_CONNECT, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    directconnect_service_mod.api->fail_service(flowp, pkt, dir, &svc_element, directconnect_service_mod.flow_data_index, pConfig, NULL);
    return SERVICE_NOMATCH;
}

static int validateDirectConnectUdp(const uint8_t *data, uint16_t size, const int dir, 
        tAppIdData *flowp, const SFSnortPacket *pkt, tServiceData *serviceData,
        const struct appIdConfig_ *pConfig)
{
    if (dir == APP_ID_FROM_RESPONDER && serviceData->state == CONN_STATE_SERVICE_DETECTED) 
    {
        goto reportSuccess;

    }

    if (size > 58) 
    {
        if (memcmp(data, PATTERN7, sizeof(PATTERN7)-1) == 0
                && data[size-3] == ')'
                && data[size-2] == '|'
                && data[size-1] == '$')
        {
            goto success;
        }
        serviceData->state +=  1;

        if (serviceData->state != CONN_STATE_SERVICE_DETECTED) 
            goto inprocess;
        else
            goto fail;

    }

inprocess:
    serviceData->packetCount++;
    if (serviceData->packetCount >= MAX_PACKET_INSPECTION_COUNT)
        goto fail;

    directconnect_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    if (dir != APP_ID_FROM_RESPONDER)
    {
        serviceData->state = CONN_STATE_SERVICE_DETECTED;
        goto inprocess;
    }

reportSuccess:
    directconnect_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                      APP_ID_DIRECT_CONNECT, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    directconnect_service_mod.api->fail_service(flowp, pkt, dir, &svc_element, directconnect_service_mod.flow_data_index, pConfig, NULL);
    return SERVICE_NOMATCH;

}

