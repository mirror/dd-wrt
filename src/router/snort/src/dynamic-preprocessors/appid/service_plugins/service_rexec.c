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

#define REXEC_PORT  512
#define REXEC_MAX_PORT_PACKET 6

typedef enum
{
    REXEC_STATE_PORT,
    REXEC_STATE_SERVER_CONNECT,
    REXEC_STATE_USERNAME,
    REXEC_STATE_PASSWORD,
    REXEC_STATE_COMMAND,
    REXEC_STATE_REPLY,
    REXEC_STATE_DONE,
    REXEC_STATE_STDERR_CONNECT_SYN,
    REXEC_STATE_STDERR_CONNECT_SYN_ACK
} REXECState;


typedef struct _SERVICE_REXEC_DATA
{
    REXECState state;
    struct _SERVICE_REXEC_DATA *parent;
    struct _SERVICE_REXEC_DATA *child;
} ServiceREXECData;

static int rexec_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(rexec_validate);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &rexec_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "rexec",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&rexec_validate, REXEC_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

RNAServiceValidationModule rexec_service_mod =
{
    "REXEC",
    &rexec_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_EXEC, 0}};

static int16_t app_id = 0;

static int rexec_init(const InitServiceAPI * const init_api)
{
    unsigned i;

    app_id = init_api->dpd->addProtocolReference("rexec");

    for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
        init_api->RegisterAppId(&rexec_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, NULL);
    }

    return 0;
}

static void rexec_free_state(void *data)
{
    ServiceREXECData *rd = (ServiceREXECData *)data;

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

MakeRNAServiceValidationPrototype(rexec_validate)
{
    ServiceREXECData *rd;
    ServiceREXECData *tmp_rd;
    int i;
    uint32_t port;
    FLOW *pf;

    rd = rexec_service_mod.api->data_get(flowp);
    if (!rd)
    {
        if (!size)
            goto inprocess;
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (rexec_service_mod.api->data_add(flowp, rd, &rexec_free_state))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = REXEC_STATE_PORT;
    }

    switch (rd->state)
    {
    case REXEC_STATE_PORT:
        if (dir != APP_ID_FROM_INITIATOR) goto bail;
        if (size > REXEC_MAX_PORT_PACKET) goto bail;
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

            dip = GET_DST_IP(pkt);
            sip = GET_SRC_IP(pkt);
            pf = rexec_service_mod.api->flow_new(pkt, dip, 0, sip, (uint16_t)port, IPPROTO_TCP, app_id);
            if (pf)
            {
                tmp_rd = calloc(1, sizeof(ServiceREXECData));
                if (tmp_rd == NULL)
                    return SERVICE_ENOMEM;
                tmp_rd->state = REXEC_STATE_STDERR_CONNECT_SYN;
                tmp_rd->parent = rd;

                if (rexec_service_mod.api->data_add(pf, tmp_rd, &rexec_free_state))
                {
                    free(tmp_rd);
                    return SERVICE_ENOMEM;
                }
                flow_mark(pf, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
                pf->rnaClientState = RNA_STATE_FINISHED;
                if (rexec_service_mod.api->data_add_id(pf, (uint16_t)port, &svc_element))
                {
                    pf->rnaServiceState = RNA_STATE_FINISHED;
                    tmp_rd->state = REXEC_STATE_DONE;
                    tmp_rd->parent = NULL;
                    return SERVICE_ENULL;
                }
                flow_mark(pf, FLOW_CONTINUE | FLOW_REXEC_STDERR);
                rd->child = tmp_rd;
                rd->state = REXEC_STATE_SERVER_CONNECT;
            }
            else
                rd->state = REXEC_STATE_USERNAME;

        }
        else rd->state = REXEC_STATE_USERNAME;
        break;
    case REXEC_STATE_SERVER_CONNECT:
        if (!size) break;
        /* The only valid way out of this state is for the child flow to change it. */
        goto fail;
    case REXEC_STATE_USERNAME:
        if (!size) break;
        if (dir != APP_ID_FROM_INITIATOR) goto bail;
        for (i=0; i<size && data[i]; i++)
            if (!isprint(data[i]) || isspace(data[i])) goto bail;
        rd->state = REXEC_STATE_PASSWORD;
        if (i >= size) goto bail;
        i++;
        data += i;
        size -= i;
        /* Fall through */
    case REXEC_STATE_PASSWORD:
        if (!size) break;
        if (dir != APP_ID_FROM_INITIATOR) goto bail;
        for (i=0; i<size && data[i]; i++)
            if (!isprint(data[i])) goto bail;
        rd->state = REXEC_STATE_COMMAND;
        if (i >= size) goto bail;
        i++;
        data += i;
        size -= i;
        /* Fall through */
    case REXEC_STATE_COMMAND:
        if (!size) break;
        if (dir != APP_ID_FROM_INITIATOR) goto bail;
        for (i=0; i<size && data[i]; i++)
            if (!isprint(data[i])) goto bail;
        rd->state = REXEC_STATE_COMMAND;
        if (i >= size) goto bail;
        i++;
        data += i;
        size -= i;
        if (!size)
        {
            rd->state = REXEC_STATE_REPLY;
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
        rd->state = REXEC_STATE_REPLY;
        break;
    case REXEC_STATE_REPLY:
        if (!size) goto inprocess;
        if (dir != APP_ID_FROM_RESPONDER) goto fail;
        if (size != 1) goto fail;
        goto success;
        break;
    case REXEC_STATE_STDERR_CONNECT_SYN:
        rd->state = REXEC_STATE_STDERR_CONNECT_SYN_ACK;
        break;
    case REXEC_STATE_STDERR_CONNECT_SYN_ACK:
        if (rd->parent && rd->parent->state == REXEC_STATE_SERVER_CONNECT)
        {
            rd->parent->state = REXEC_STATE_USERNAME;
            flow_clear(flowp, FLOW_REXEC_STDERR);
        }
        goto bail;
    default:
        goto bail;
    }

inprocess:
    if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
    {
        rexec_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
    }
    return SERVICE_INPROCESS;

success:
    if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
    {
        rexec_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                           APP_ID_EXEC, NULL, NULL, NULL);
    }
    return SERVICE_SUCCESS;

bail:
    if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
    {
        rexec_service_mod.api->incompatible_data(flowp, pkt, dir, &svc_element);
    }
    flow_clear(flowp, FLOW_CONTINUE);
    return SERVICE_NOT_COMPATIBLE;

fail:
    if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
    {
        rexec_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
    }
    flow_clear(flowp, FLOW_CONTINUE);
    return SERVICE_NOMATCH;
}

