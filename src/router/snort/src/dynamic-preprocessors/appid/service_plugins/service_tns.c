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

#include "appInfoTable.h"
#include "flow.h"
#include "service_api.h"

static const char svc_name[] = "oracle";
static const uint8_t TNS_BANNER[]  = "\000\000";

#define TNS_BANNER_LEN    (sizeof(TNS_BANNER)-1)
#define TNS_PORT    1521

#define TNS_TYPE_CONNECT 1
#define TNS_TYPE_ACCEPT 2
#define TNS_TYPE_ACK 3
#define TNS_TYPE_REFUSE 4
#define TNS_TYPE_REDIRECT 5
#define TNS_TYPE_DATA 6
#define TNS_TYPE_NULL 7
#define TNS_TYPE_ABORT 9
#define TNS_TYPE_RESEND 11
#define TNS_TYPE_MARKER 12
#define TNS_TYPE_ATTENTION 13
#define TNS_TYPE_CONTROL 14
#define TNS_TYPE_MAX 19

typedef enum
{
    TNS_STATE_MESSAGE_LEN,
    TNS_STATE_MESSAGE_CHECKSUM,
    TNS_STATE_MESSAGE,
    TNS_STATE_MESSAGE_RES,
    TNS_STATE_MESSAGE_HD_CHECKSUM,
    TNS_STATE_MESSAGE_ACCEPT,
    TNS_STATE_MESSAGE_DATA
} TNSState;

#define ACCEPT_VERSION_OFFSET	8
#define MAX_VERSION_SIZE	12
typedef struct _SERVICE_TNS_DATA
{
    TNSState state;
    unsigned stringlen;
    unsigned pos;
    unsigned message;
    union
    {
        uint16_t len;
        uint8_t raw_len[2];
    }l;
    const char *version;
} ServiceTNSData;

#pragma pack(1)
typedef struct _SERVICE_TNS_MSG
{
    uint16_t len;
    uint16_t checksum;
    uint8_t msg;
    uint8_t res;
    uint16_t hdchecksum;
    uint8_t data;
} ServiceTNSMsg;
#pragma pack()

static int tns_init(const InitServiceAPI * const init_api);
static int tns_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &tns_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "tns",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&tns_validate, TNS_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

SF_SO_PUBLIC tRNAServiceValidationModule tns_service_mod =
{
    svc_name,
    &tns_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_ORACLE_TNS, APPINFO_FLAG_SERVICE_ADDITIONAL},
};

static int tns_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&tns_validate, IPPROTO_TCP, (const uint8_t *) TNS_BANNER, TNS_BANNER_LEN, 2, svc_name, init_api->pAppidConfig);
    unsigned i;
    for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
        init_api->RegisterAppId(&tns_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
    }

    return 0;
}

static int tns_validate(ServiceValidationArgs* args)
{
    ServiceTNSData *ss;
    uint16_t offset;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    ss = tns_service_mod.api->data_get(flowp, tns_service_mod.flow_data_index);
    if (!ss)
    {
        ss = calloc(1, sizeof(*ss));
        if (!ss)
            return SERVICE_ENOMEM;
        if (tns_service_mod.api->data_add(flowp, ss, tns_service_mod.flow_data_index, &free))
        {
            free(ss);
            return SERVICE_ENOMEM;
        }
        ss->state = TNS_STATE_MESSAGE_LEN;
    }

    offset = 0;
    while(offset < size)
    {
        switch (ss->state)
        {
        case TNS_STATE_MESSAGE_LEN:
            ss->l.raw_len[ss->pos++] = data[offset];
            if(ss->pos >= offsetof(ServiceTNSMsg , checksum))
            {
                ss->stringlen = ntohs(ss->l.len);
                if(ss->stringlen == 2)
		{
                    if(offset == (size - 1))
                        goto success;
		    goto fail;
		}
		else if(ss->stringlen < 2)
		    goto fail;
		else
		{
                    ss->state = TNS_STATE_MESSAGE_CHECKSUM;
		}
            }
            break;

        case TNS_STATE_MESSAGE_CHECKSUM:
            if(data[offset] != 0)
                goto fail;
            ss->pos++;
            if(ss->pos >= offsetof(ServiceTNSMsg , msg))
            {
                ss->state = TNS_STATE_MESSAGE;
            }
            break;

        case TNS_STATE_MESSAGE:
            ss->message = data[offset];
            if(ss->message < TNS_TYPE_CONNECT || ss->message > TNS_TYPE_MAX)
                goto fail;
            ss->pos++;
            ss->state = TNS_STATE_MESSAGE_RES;
            break;

        case TNS_STATE_MESSAGE_RES:
            ss->pos++;
            ss->state = TNS_STATE_MESSAGE_HD_CHECKSUM;
            break;

        case TNS_STATE_MESSAGE_HD_CHECKSUM:
            ss->pos++;
            if(ss->pos >= offsetof(ServiceTNSMsg , data))
	    {
                switch(ss->message)
		{
                case TNS_TYPE_ACCEPT:
                    ss->state = TNS_STATE_MESSAGE_ACCEPT;
                    break;
                case TNS_TYPE_ACK:
                case TNS_TYPE_REFUSE:
                case TNS_TYPE_REDIRECT:
                case TNS_TYPE_DATA:
                case TNS_TYPE_NULL:
                case TNS_TYPE_ABORT:
                case TNS_TYPE_MARKER:
                case TNS_TYPE_ATTENTION:
                case TNS_TYPE_CONTROL:
                    if(ss->pos == ss->stringlen)
                    {
                        if(offset == (size - 1))
                            goto success;
		        else
	                    goto fail;
                    }
                    ss->state = TNS_STATE_MESSAGE_DATA;
                    break;
                case TNS_TYPE_RESEND:
                    if(ss->pos == ss->stringlen)
                    {
                        if(offset == (size - 1))
                        {
                            ss->state = TNS_STATE_MESSAGE_LEN;
                            ss->pos = 0;
                            goto inprocess;
                        }
		        else
	                    goto fail;
                    }
                    break;
                case TNS_TYPE_CONNECT:
                default:
	            goto fail;
		}
	    }
            break;

        case TNS_STATE_MESSAGE_ACCEPT:
            ss->l.raw_len[ss->pos - ACCEPT_VERSION_OFFSET] = data[offset];
            ss->pos++;
            if(ss->pos >= (ACCEPT_VERSION_OFFSET + 2))
	    {
                switch(ntohs(ss->l.len))
		{
                case 0x136:
                    ss->version = "8";
                    break;
                case 0x137:
                    ss->version = "9i R1";
                    break;
                case 0x138:
                    ss->version = "9i R2";
                    break;
                case 0x139:
                    ss->version = "10g R1/R2";
                    break;
                case 0x13A:
                    ss->version = "11g R1";
                    break;
                default:
                    break;
		}
                ss->state = TNS_STATE_MESSAGE_DATA;
	    }
            break;
        case TNS_STATE_MESSAGE_DATA:
            ss->pos++;
            if(ss->pos == ss->stringlen)
            {
                if(offset == (size - 1))
                    goto success;
		else
	            goto fail;
            }
            break;
        default:
            goto fail;
        }
        offset++;
    }

inprocess:
    tns_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    tns_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element, APP_ID_ORACLE_TNS,
                                     NULL, ss->version ? ss->version:NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    tns_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                      tns_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

