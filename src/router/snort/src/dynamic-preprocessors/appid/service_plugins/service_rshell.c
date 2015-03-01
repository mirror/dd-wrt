/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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

#define RSHELL_PORT  514
#define RSHELL_MAX_PORT_PACKET 6

typedef enum
{
    RSHELL_STATE_PORT,
    RSHELL_STATE_SERVER_CONNECT,
    RSHELL_STATE_USERNAME,
    RSHELL_STATE_USERNAME2,
    RSHELL_STATE_COMMAND,
    RSHELL_STATE_REPLY,
    RSHELL_STATE_DONE,
    RSHELL_STATE_STDERR_CONNECT_SYN,
    RSHELL_STATE_STDERR_CONNECT_SYN_ACK
} RSHELLState;

typedef struct _SERVICE_RSHELL_DATA
{
    RSHELLState state;
    struct _SERVICE_RSHELL_DATA *parent;
    struct _SERVICE_RSHELL_DATA *child;
} ServiceRSHELLData;

static int rshell_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(rshell_validate);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &rshell_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "rshell",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&rshell_validate, RSHELL_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

RNAServiceValidationModule rshell_service_mod =
{
    "RSHELL",
    &rshell_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_SHELL, 0}};

static int16_t app_id = 0;

static int rshell_init(const InitServiceAPI * const init_api)
{
	unsigned i;

    app_id = init_api->dpd->addProtocolReference("rsh-error");

	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&rshell_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, NULL);
	}

    return 0;
}

static void rshell_free_state(void *data)
{
    ServiceRSHELLData *rd = (ServiceRSHELLData *)data;

    if (rd)
    {
        if (rd->parent)
        {
            rd->parent->child = NULL;
            rd->parent->parent = NULL;
        }
        if (rd->child)
        {
            rd->child->parent = NULL;
            rd->child->child = NULL;
        }
        free(rd);
    }
}

MakeRNAServiceValidationPrototype(rshell_validate)
{
    ServiceRSHELLData *rd = NULL;
    ServiceRSHELLData *tmp_rd;
    int i;
    uint32_t port;
    FLOW *pf;

    rd = rshell_service_mod.api->data_get(flowp);
    if (!rd)
    {
        if (!size)
            goto inprocess;
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (rshell_service_mod.api->data_add(flowp, rd, &rshell_free_state))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = RSHELL_STATE_PORT;
    }

    switch (rd->state)
    {
    case RSHELL_STATE_PORT:
        if (dir != APP_ID_FROM_INITIATOR) goto fail;
        if (size > RSHELL_MAX_PORT_PACKET) goto bail;
        if (data[size-1]) goto bail;
        port = 0;
        for (i=0; i<size-1; i++)
        {
            if (!isdigit(data[i])) goto bail;
            port *= 10;
            port += data[i] - '0';
        }
        if (port > 65535) goto bail;
        if (port)
        {
            snort_ip *sip;
            snort_ip *dip;

            tmp_rd = calloc(1, sizeof(ServiceRSHELLData));
            if (tmp_rd == NULL)
                return SERVICE_ENOMEM;
            tmp_rd->state = RSHELL_STATE_STDERR_CONNECT_SYN;
            tmp_rd->parent = rd;
            dip = GET_DST_IP(pkt);
            sip = GET_SRC_IP(pkt);
            pf = rshell_service_mod.api->flow_new(pkt, dip, 0, sip, (uint16_t)port, IPPROTO_TCP, app_id);
            if (pf)
            {
                flow_mark(pf, FLOW_IGNORE_HOST | FLOW_NOT_A_SERVICE);
                pf->rnaClientState = RNA_STATE_FINISHED;
                if (rshell_service_mod.api->data_add(pf, tmp_rd, &rshell_free_state))
                {
                    pf->rnaServiceState = RNA_STATE_FINISHED;
                    free(tmp_rd);
                    return SERVICE_ENOMEM;
                }
                if (rshell_service_mod.api->data_add_id(pf, (uint16_t)port, &svc_element))
                {
                    pf->rnaServiceState = RNA_STATE_FINISHED;
                    tmp_rd->state = RSHELL_STATE_DONE;
                    tmp_rd->parent = NULL;
                    return SERVICE_ENOMEM;
                }
            }
            else
            {
                free(tmp_rd);
                return SERVICE_ENOMEM;
            }
            rd->child = tmp_rd;
            rd->state = RSHELL_STATE_SERVER_CONNECT;
        }
        else rd->state = RSHELL_STATE_USERNAME;
        break;
    case RSHELL_STATE_SERVER_CONNECT:
        if (!size) break;
        /* The only valid way out of this state is for the child flow to change it. */
        goto fail;
    case RSHELL_STATE_USERNAME:
        if (!size) break;
        if (dir != APP_ID_FROM_INITIATOR) goto fail;
        for (i=0; i<size && data[i]; i++)
            if (!isprint(data[i]) || isspace(data[i])) goto bail;
        rd->state = RSHELL_STATE_USERNAME2;
        if (i >= size) goto bail;
        i++;
        data += i;
        size -= i;
        /* Fall through */
    case RSHELL_STATE_USERNAME2:
        if (!size) break;
        if (dir != APP_ID_FROM_INITIATOR) goto fail;
        for (i=0; i<size && data[i]; i++)
            if (!isprint(data[i]) || isspace(data[i])) goto bail;
        rd->state = RSHELL_STATE_COMMAND;
        if (i >= size) goto bail;
        i++;
        data += i;
        size -= i;
        /* Fall through */
    case RSHELL_STATE_COMMAND:
        if (!size) break;
        if (dir != APP_ID_FROM_INITIATOR) goto fail;
        for (i=0; i<size && data[i]; i++)
            if (!isprint(data[i])) goto bail;
        rd->state = RSHELL_STATE_COMMAND;
        if (i >= size) goto bail;
        i++;
        data += i;
        size -= i;
        if (!size)
        {
            rd->state = RSHELL_STATE_REPLY;
            break;
        }
        if (data[size-1]) goto bail;
        /* stdin */
        for (i=0; i<size && data[i]; i++)
        {
            if (!isprint(data[i])) goto bail;
        }
        i++;
        if (i != size) goto bail;
        rd->state = RSHELL_STATE_REPLY;
        break;
    case RSHELL_STATE_REPLY:
        if (!size) goto inprocess;
        if (dir != APP_ID_FROM_RESPONDER) goto fail;
        if (size == 1) goto success;
        if (*data == 0x01)
        {
            data++;
            size--;
            for (i=0; i<size && data[i]; i++)
            {
                if (!isprint(data[i]) && data[i] != 0x0A && data[i] != 0x0D && data[i] != 0x09) goto fail;
            }
            goto success;
        }
        goto fail;
        break;
    case RSHELL_STATE_STDERR_CONNECT_SYN:
        rd->state = RSHELL_STATE_STDERR_CONNECT_SYN_ACK;
        break;
    case RSHELL_STATE_STDERR_CONNECT_SYN_ACK:
        if (rd->parent && rd->parent->state == RSHELL_STATE_SERVER_CONNECT)
            rd->parent->state = RSHELL_STATE_USERNAME;
        flow_mark(flowp, FLOW_SERVICEDETECTED);
        return SERVICE_SUCCESS;
    default:
        goto bail;
    }

inprocess:
    rshell_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
    return SERVICE_INPROCESS;

success:
    rshell_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                        APP_ID_SHELL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

bail:
    rshell_service_mod.api->incompatible_data(flowp, pkt, dir, &svc_element);
    return SERVICE_NOT_COMPATIBLE;

fail:
    rshell_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
    return SERVICE_NOMATCH;
}

