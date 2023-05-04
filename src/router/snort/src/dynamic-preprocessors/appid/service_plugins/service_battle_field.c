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
    CONN_STATE_HELLO_DETECTED,
    CONN_STATE_SERVICE_DETECTED,
    CONN_STATE_MESSAGE_DETECTED,
    CONN_STATE_MAX
} CONNECTION_STATES;

#define MAX_PACKET_INSPECTION_COUNT      10

typedef struct _SERVICE_DATA
{
    uint32_t state;
    uint32_t messageId;
    uint32_t packetCount;
} tServiceData;

static int battle_field_init(const InitServiceAPI * const init_api);
static int battle_field_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &battle_field_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "battle_field",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&battle_field_validate, 4711,  IPPROTO_TCP},
    {&battle_field_validate, 16567, IPPROTO_UDP},
    {&battle_field_validate, 27900, IPPROTO_UDP},
    {&battle_field_validate, 27900, IPPROTO_TCP},
    {&battle_field_validate, 29900, IPPROTO_UDP},
    {&battle_field_validate, 29900, IPPROTO_TCP},
    {&battle_field_validate, 27901, IPPROTO_TCP},
    {&battle_field_validate, 28910, IPPROTO_UDP},
    {NULL, 0, 0}
};


#define PATTERN_HELLO "battlefield2\x00"
#define PATTERN_2     "\xfe\xfd"
#define PATTERN_3     "\x11\x20\x00\x01\x00\x00\x50\xb9\x10\x11"
#define PATTERN_4     "\x11\x20\x00\x01\x00\x00\x30\xb9\x10\x11"
#define PATTERN_5     "\x11\x20\x00\x01\x00\x00\xa0\x98\x00\x11"
#define PATTERN_6     "\xfe\xfd\x09\x00\x00\x00\x00"


tRNAServiceValidationModule battlefield_service_mod =
{
    "BattleField",
    &battle_field_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_BATTLEFIELD, 0}};

static int battle_field_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&battle_field_validate, IPPROTO_TCP, (uint8_t *)PATTERN_HELLO, sizeof(PATTERN_HELLO)-1,  5, "battle_field", init_api->pAppidConfig);
    init_api->RegisterPattern(&battle_field_validate, IPPROTO_TCP, (uint8_t *)PATTERN_2, sizeof(PATTERN_2)-1,  0, "battle_field", init_api->pAppidConfig);
    init_api->RegisterPattern(&battle_field_validate, IPPROTO_TCP, (uint8_t *)PATTERN_3, sizeof(PATTERN_3)-1,  0, "battle_field", init_api->pAppidConfig);
    init_api->RegisterPattern(&battle_field_validate, IPPROTO_TCP, (uint8_t *)PATTERN_4, sizeof(PATTERN_4)-1,  0, "battle_field", init_api->pAppidConfig);
    init_api->RegisterPattern(&battle_field_validate, IPPROTO_TCP, (uint8_t *)PATTERN_5, sizeof(PATTERN_5)-1,  0, "battle_field", init_api->pAppidConfig);
    init_api->RegisterPattern(&battle_field_validate, IPPROTO_TCP, (uint8_t *)PATTERN_6, sizeof(PATTERN_6)-1,  0, "battle_field", init_api->pAppidConfig);

	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&battle_field_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}


static int battle_field_validate(ServiceValidationArgs* args)
{
    tServiceData *fd;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    SFSnortPacket *pkt = args->pkt; 
    uint16_t size = args->size;

    if (!size)
    {
        goto inprocess_nofd;
    }

    fd = battlefield_service_mod.api->data_get(flowp, battlefield_service_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return SERVICE_ENOMEM;
        if (battlefield_service_mod.api->data_add(flowp, fd, battlefield_service_mod.flow_data_index, &free))
        {
            free(fd);
            return SERVICE_ENOMEM;
        }
    }

    switch (fd->state)
    {
        case CONN_STATE_INIT:
            if ((pkt->src_port >= 27000 || pkt->dst_port >= 27000) && size >= 4)
            {
                if (data[0] == 0xfe && data[1] == 0xfd)
                {
                    fd->messageId = (data[2]<<8) | data[3];
                    fd->state = CONN_STATE_MESSAGE_DETECTED;
                    goto inprocess;
                }
            }

            if (size == 18 &&  memcmp(data+5, PATTERN_HELLO, sizeof(PATTERN_HELLO)-1) == 0) 
            {
                fd->state = CONN_STATE_HELLO_DETECTED;
                goto inprocess;
            }
            break;

        case CONN_STATE_MESSAGE_DETECTED:
            if (size > 8)
            {
                if ((uint32_t)(data[0]<<8 | data[1]) == fd->messageId)
                {
                    goto success;
                }

                if (data[0] == 0xfe && data[1] == 0xfd)
                {
                    fd->messageId = (data[2]<<8) | data[3];
                    goto inprocess;
                }
            }

            fd->state = CONN_STATE_INIT;
            goto inprocess;
            break;

        case CONN_STATE_HELLO_DETECTED:
            if ((size == 7) && (memcmp(data, PATTERN_6, sizeof(PATTERN_6)-1) == 0)) 
            {
                goto success;
            }

            if ((size > 10)
                && ((memcmp(data, PATTERN_3, sizeof(PATTERN_3)-1) == 0)
                     || (memcmp(data, PATTERN_4, sizeof(PATTERN_4)-1) == 0)
                     || (memcmp(data, PATTERN_5, sizeof(PATTERN_5)-1) == 0))) 
            {
                goto success;

            }
            break;
        case CONN_STATE_SERVICE_DETECTED:
            goto success;
    }

    
    battlefield_service_mod.api->fail_service(flowp, pkt, args->dir, &svc_element,
                                              battlefield_service_mod.flow_data_index,
                                              args->pConfig, NULL);
    return SERVICE_NOMATCH;

inprocess:
    fd->packetCount++;
    if (fd->packetCount >= MAX_PACKET_INSPECTION_COUNT)
        goto fail;
inprocess_nofd:
    battlefield_service_mod.api->service_inprocess(flowp, pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    if (args->dir != APP_ID_FROM_RESPONDER)
    {
        fd->state = CONN_STATE_SERVICE_DETECTED;
        goto inprocess;
    }

    battlefield_service_mod.api->add_service(flowp, pkt, args->dir, &svc_element,
                                      APP_ID_BATTLEFIELD, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    battlefield_service_mod.api->fail_service(flowp, pkt, args->dir, &svc_element,
                                              battlefield_service_mod.flow_data_index,
                                              args->pConfig, NULL);
    return SERVICE_NOMATCH;

}

