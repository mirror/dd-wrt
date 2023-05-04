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

static const char svc_name[] = "timbuktu";
static char TIMBUKTU_BANNER[]  = "\001\001";

#define TIMBUKTU_PORT    407

#define TIMBUKTU_BANNER_LEN (sizeof(TIMBUKTU_BANNER)-1)

typedef enum
{
    TIMBUKTU_STATE_BANNER,
    TIMBUKTU_STATE_MESSAGE_LEN,
    TIMBUKTU_STATE_MESSAGE_DATA
} TIMBUKTUState;


typedef struct _SERVICE_TIMBUKTU_DATA
{
    TIMBUKTUState state;
    unsigned stringlen;
    unsigned pos;
} ServiceTIMBUKTUData;

#pragma pack(1)
typedef struct _SERVICE_TIMBUKTU_MSG
{
    uint16_t any;
    uint8_t res;
    uint8_t len;
    uint8_t message;
} ServiceTIMBUKTUMsg;
#pragma pack()

static int timbuktu_init(const InitServiceAPI * const init_api);
static int timbuktu_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &timbuktu_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "timbuktu",
    .ref_count = 1,
    .current_ref_count = 1,
};
static RNAServiceValidationPort pp[] =
{
    {&timbuktu_validate, TIMBUKTU_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

SF_SO_PUBLIC tRNAServiceValidationModule timbuktu_service_mod =
{
    svc_name,
    &timbuktu_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_TIMBUKTU, 0}};

static int timbuktu_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&timbuktu_validate, IPPROTO_TCP, (const u_int8_t *) TIMBUKTU_BANNER, sizeof(TIMBUKTU_BANNER)-1, 0, svc_name, init_api->pAppidConfig);
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&timbuktu_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int timbuktu_validate(ServiceValidationArgs* args)
{
    ServiceTIMBUKTUData *ss;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;
    uint16_t offset=0;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    ss = timbuktu_service_mod.api->data_get(flowp, timbuktu_service_mod.flow_data_index);
    if (!ss)
    {
        ss = calloc(1, sizeof(*ss));
        if (!ss)
            return SERVICE_ENOMEM;
        if (timbuktu_service_mod.api->data_add(flowp, ss, timbuktu_service_mod.flow_data_index, &free))
        {
            free(ss);
            return SERVICE_ENOMEM;
        }
        ss->state = TIMBUKTU_STATE_BANNER;
    }

    offset = 0;
    while(offset < size)
    {
        switch (ss->state)
        {
        case TIMBUKTU_STATE_BANNER:
            if(data[offset] !=  TIMBUKTU_BANNER[ss->pos])
                goto fail;
	    if(ss->pos >= TIMBUKTU_BANNER_LEN-1)
            {
                ss->pos = 0;
                ss->state = TIMBUKTU_STATE_MESSAGE_LEN;
		break;
            }
            ss->pos++;
            break;
        case TIMBUKTU_STATE_MESSAGE_LEN:
            ss->pos++;
            if(ss->pos >= offsetof(ServiceTIMBUKTUMsg , message))
            {
                ss->stringlen = data[offset];
                ss->state = TIMBUKTU_STATE_MESSAGE_DATA;
                if(!ss->stringlen)
		{
                    if(offset == size-1)
                        goto success;
		    goto fail;
		}
                ss->pos = 0;
            }
            break;

        case TIMBUKTU_STATE_MESSAGE_DATA:
            ss->pos++;
            if(ss->pos == ss->stringlen)
            {
                if(offset == (size-1))
                    goto success;
		goto fail;
            }
            break;
        default:
             goto fail;
        }
        offset++;
    }

inprocess:
        timbuktu_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
        return SERVICE_INPROCESS;

success:
        timbuktu_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                              APP_ID_TIMBUKTU, NULL, NULL, NULL, NULL);
        return SERVICE_SUCCESS;

fail:
        timbuktu_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                               timbuktu_service_mod.flow_data_index,
                                               args->pConfig, NULL);
        return SERVICE_NOMATCH;

}

