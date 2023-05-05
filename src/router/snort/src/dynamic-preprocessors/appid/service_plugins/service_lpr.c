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

#define LPR_COUNT_THRESHOLD 5

typedef enum
{
    LPR_STATE_COMMAND,
    LPR_STATE_RECEIVE,
    LPR_STATE_REPLY1,
    LPR_STATE_REPLY,
    LPR_STATE_IGNORE
} LPRState;

typedef enum
{
    LPR_CMD_PRINT = 1,
    LPR_CMD_RECEIVE,
    LPR_CMD_SHORT_STATE,
    LPR_CMD_LONG_STATE,
    LPR_CMD_REMOVE
} LPRCommand;

typedef enum
{
    LPR_SUBCMD_ABORT = 1,
    LPR_SUBCMD_CONTROL,
    LPR_SUBCMD_DATA
} LPRSubCommand;

typedef struct _SERVICE_LPR_DATA
{
    LPRState state;
    unsigned no_data_count;
    unsigned count;
} ServiceLPRData;

static int lpr_init(const InitServiceAPI * const init_api);
static int lpr_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &lpr_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "lpr",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&lpr_validate, 515, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule lpr_service_mod =
{
    "LPR",
    &lpr_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_PRINTSRV, 0}};

static int lpr_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&lpr_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int lpr_validate(ServiceValidationArgs* args)
{
    ServiceLPRData *ld;
    int i;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size) goto inprocess;

    ld = lpr_service_mod.api->data_get(flowp, lpr_service_mod.flow_data_index);
    if (!ld)
    {
        ld = calloc(1, sizeof(*ld));
        if (!ld)
            return SERVICE_ENOMEM;
        if (lpr_service_mod.api->data_add(flowp, ld, lpr_service_mod.flow_data_index, &free))
        {
            free(ld);
            return SERVICE_ENOMEM;
        }
        ld->state = LPR_STATE_COMMAND;
    }

    switch (ld->state)
    {
    case LPR_STATE_COMMAND:
        if (dir != APP_ID_FROM_INITIATOR)
            goto bail;
        if (size < 3) goto bail;
        switch (*data)
        {
        case LPR_CMD_RECEIVE:
            if (data[size-1] != 0x0A) goto bail;
            size--;
            for (i=1; i<size; i++)
                if (!isprint(data[i]) || isspace(data[i])) goto bail;
            ld->state = LPR_STATE_REPLY;
            break;
        case LPR_CMD_PRINT:
            ld->state = LPR_STATE_IGNORE;
            break;
        case LPR_CMD_SHORT_STATE:
            ld->state = LPR_STATE_IGNORE;
            break;
        case LPR_CMD_LONG_STATE:
            ld->state = LPR_STATE_IGNORE;
            break;
        case LPR_CMD_REMOVE:
            ld->state = LPR_STATE_IGNORE;
            break;
        default:
            goto bail;
        }
        break;
    case LPR_STATE_RECEIVE:
        if (dir != APP_ID_FROM_INITIATOR) goto inprocess;
        if (size < 2) goto bail;
        switch (*data)
        {
        case LPR_SUBCMD_ABORT:
            if (size != 2) goto bail;
            if (data[1] != 0x0A) goto bail;
            ld->state = LPR_STATE_REPLY;
            break;
        case LPR_SUBCMD_CONTROL:
        case LPR_SUBCMD_DATA:
            if (size < 5) goto bail;
            if (data[size-1] != 0x0A) goto bail;
            if (!isdigit(data[1])) goto bail;
            for (i=2; i<size; i++)
            {
                if (data[i] == 0x0A) goto bail;
                else if (isspace(data[i])) break;
                if (!isdigit(data[i])) goto bail;
            }
            i++;
            if (i >= size) goto bail;
            for (; i<size-1; i++)
                if (!isprint(data[i]) || isspace(data[i])) goto bail;
            ld->state = LPR_STATE_REPLY1;
            break;
        default:
            goto bail;
        }
        break;
    case LPR_STATE_REPLY1:
        if (dir != APP_ID_FROM_RESPONDER) goto inprocess;
        if (size != 1) goto fail;
        ld->count++;
        if (ld->count >= LPR_COUNT_THRESHOLD)
        {
            ld->state = LPR_STATE_IGNORE;
            goto success;
        }
        ld->state = LPR_STATE_REPLY;
        break;
    case LPR_STATE_REPLY:
        if (dir != APP_ID_FROM_RESPONDER) goto inprocess;
        if (size != 1) goto fail;
        ld->count++;
        if (ld->count >= LPR_COUNT_THRESHOLD)
        {
            ld->state = LPR_STATE_IGNORE;
            goto success;
        }
        ld->state = LPR_STATE_RECEIVE;
        break;
    case LPR_STATE_IGNORE:
        break;
    default:
        goto bail;
    }
inprocess:
    lpr_service_mod.api->service_inprocess(flowp, args->pkt, dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    lpr_service_mod.api->add_service(flowp, args->pkt, dir, &svc_element,
                                     APP_ID_PRINTSRV, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    lpr_service_mod.api->fail_service(flowp, args->pkt, dir, &svc_element,
                                      lpr_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;

bail:
    lpr_service_mod.api->incompatible_data(flowp, args->pkt, dir, &svc_element,
                                           lpr_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOT_COMPATIBLE;
}

