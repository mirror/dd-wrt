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

#define NNTP_PORT   119

#define NNTP_COUNT_THRESHOLD 2

typedef enum
{
    NNTP_STATE_CONNECTION,
    NNTP_STATE_TRANSFER,
    NNTP_STATE_DATA,
    NNTP_STATE_CONNECTION_ERROR
} NNTPState;

#define NNTP_CR_RECEIVED    0x0001
#define NNTP_MID_LINE       0x0002
#define NNTP_MID_TERM       0x0004

typedef struct _SERVICE_NNTP_DATA
{
    NNTPState state;
    uint32_t flags;
    unsigned count;
} ServiceNNTPData;

#pragma pack(1)

typedef struct _SERVICE_NNTP_CODE
{
    uint8_t code[3];
    uint8_t sp;
} ServiceNNTPCode;

#pragma pack()

static int nntp_init(const InitServiceAPI * const init_api);
static int nntp_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &nntp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "nntp",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&nntp_validate, NNTP_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule nntp_service_mod =
{
    "NNTP",
    &nntp_init,
    pp
};


#define NNTP_PATTERN1 "200 "
#define NNTP_PATTERN2 "201 "

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_NNTP, 0}};

static int nntp_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&nntp_validate, IPPROTO_TCP, (uint8_t *)NNTP_PATTERN1, sizeof(NNTP_PATTERN1)-1, 0, "nntp", init_api->pAppidConfig);
    init_api->RegisterPattern(&nntp_validate, IPPROTO_TCP, (uint8_t *)NNTP_PATTERN2, sizeof(NNTP_PATTERN2)-1, 0, "nntp", init_api->pAppidConfig);

	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&nntp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int nntp_validate_reply(const uint8_t *data, uint16_t *offset,
                               uint16_t size)
{
    const ServiceNNTPCode *code_hdr;
    int code;

    /* Trim any blank lines (be a little tolerant) */
    for (; *offset<size; (*offset)++)
    {
        if (data[*offset] != 0x0D && data[*offset] != 0x0A) break;
    }

    if (size - *offset < (int)sizeof(ServiceNNTPCode))
    {
        for (; *offset<size; (*offset)++)
        {
            if (!isspace(data[*offset])) return -1;
        }
        return 0;
    }

    code_hdr = (ServiceNNTPCode *)(data + *offset);

    if (code_hdr->sp != ' ') return -1;

    if (code_hdr->code[0] < '1' || code_hdr->code[0] > '5') return -1;
    code = (code_hdr->code[0] - '0') * 100;

    if (code_hdr->code[1] < '0' ||
        (code_hdr->code[1] > '5' && code_hdr->code[1] < '8') ||
        code_hdr->code[1] > '9')
    {
        return -1;
    }
    code += (code_hdr->code[1] - '0') * 10;

    if (!isdigit(code_hdr->code[2])) return -1;
    code += code_hdr->code[2] - '0';

    /* We have a valid code, now we need to see if the rest of the line
        is okay */

    *offset += sizeof(ServiceNNTPCode);
    for (; *offset < size; (*offset)++)
    {
        if (data[*offset] == 0x0D)
        {
            (*offset)++;
            if (*offset >= size) return -1;
            if (data[*offset] != 0x0A) return -1;
        }
        if (data[*offset] == 0x0A)
        {
            (*offset)++;
            return code;
        }
        else if (!isprint(data[*offset])) return -1;
    }


    return 0;
}

static int nntp_validate_data(const uint8_t *data, uint16_t *offset,
                              uint16_t size, int *flags)
{
    if (*flags & NNTP_CR_RECEIVED)
    {
        if (data[*offset] != 0x0A) return -1;
        if (*flags & NNTP_MID_TERM)
        {
            *flags = 0;
            (*offset)++;
            return 1;
        }
        *flags &= ~NNTP_CR_RECEIVED;
        (*offset)++;
    }
    if (*flags & NNTP_MID_TERM)
    {
        if (*offset >= size) return 0;
        if (data[*offset] == 0x0D)
        {
            *flags |= NNTP_CR_RECEIVED;
            (*offset)++;
            if (*offset >= size) return 0;
            if (data[*offset] != 0x0A) return -1;
            *flags = 0;
            (*offset)++;
            return 1;
        }
        else if (data[*offset] == 0x0A)
        {
            *flags = 0;
            (*offset)++;
            return 1;
        }
        else if (data[*offset] != '.') return -1;
        *flags = NNTP_MID_LINE;
        (*offset)++;
    }
    for (; *offset < size; (*offset)++)
    {
        if (!(*flags & NNTP_MID_LINE))
        {
            if (data[*offset] == '.')
            {
                *flags |= NNTP_MID_TERM;
                (*offset)++;
                if (*offset >= size) return 0;
                if (data[*offset] == 0x0D)
                {
                    *flags |= NNTP_CR_RECEIVED;
                    (*offset)++;
                    if (*offset >= size) return 0;
                    if (data[*offset] != 0x0A) return -1;
                    *flags = 0;
                    (*offset)++;
                    return 1;
                }
                else if (data[*offset] == 0x0A)
                {
                    *flags = 0;
                    (*offset)++;
                    return 1;
                }
                else if (data[*offset] != '.') return -1;
                (*offset)++;
            }
        }
        *flags = NNTP_MID_LINE;
        for (; *offset < size; (*offset)++)
        {
            if (data[*offset] == 0x0D)
            {
                (*offset)++;
                if (*offset >= size)
                {
                    *flags |= NNTP_CR_RECEIVED;
                    return 0;
                }
                if (data[*offset] != 0x0A) return -1;
                *flags = 0;
                break;
            }
            if (data[*offset] == 0x0A)
            {
                *flags = 0;
                break;
            }
        }
    }
    return 0;
}

static int nntp_validate(ServiceValidationArgs* args)
{
    ServiceNNTPData *nd;
    uint16_t offset;
    int code;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    nd = nntp_service_mod.api->data_get(flowp, nntp_service_mod.flow_data_index);
    if (!nd)
    {
        nd = calloc(1, sizeof(*nd));
        if (!nd)
            return SERVICE_ENOMEM;
        if (nntp_service_mod.api->data_add(flowp, nd, nntp_service_mod.flow_data_index, &free))
        {
            free(nd);
            return SERVICE_ENOMEM;
        }
        nd->state = NNTP_STATE_CONNECTION;
    }

    offset = 0;
    while (offset < size)
    {
        if (nd->state == NNTP_STATE_DATA)
        {
            if ((code=nntp_validate_data(data, &offset, size, (int *)&nd->flags)) < 0)
                goto fail;
            if (!code) goto inprocess;
            nd->state = NNTP_STATE_TRANSFER;
        }
        if ((code=nntp_validate_reply(data, &offset, size)) < 0)
            goto fail;
        if (!code) goto inprocess;
        if (code == 400 || code == 502)
        {
            nd->state = NNTP_STATE_CONNECTION_ERROR;
        }
        else {

            switch (nd->state)
            {
            case NNTP_STATE_CONNECTION:
                switch (code)
                {
                case 201:
                case 200:
                    nd->state = NNTP_STATE_TRANSFER;
                    break;
                default:
                    goto fail;
                }
                break;
            case NNTP_STATE_TRANSFER:
                nd->count++;
                if (nd->count >= NNTP_COUNT_THRESHOLD)
                    goto success;
                switch (code)
                {
                case 100:
                case 215:
                case 220:
                case 221:
                case 222:
                case 224:
                case 230:
                case 231:
                    nd->state = NNTP_STATE_DATA;
                    break;
                }
                break;
            case NNTP_STATE_CONNECTION_ERROR:
            default:
                goto fail;
            }
        }
    }

inprocess:
    nntp_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    nntp_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                      APP_ID_NNTP, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    nntp_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                       nntp_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

