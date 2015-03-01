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


#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "flow.h"
#include "service_api.h"
#include "dcerpc.h"

#define DCERPC_THRESHOLD    3

#define min(x,y) ((x)<(y) ? (x):(y))

typedef struct _SERVICE_DCERPC_DATA
{
    unsigned count;
} ServiceDCERPCData;

static int dcerpc_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(dcerpc_tcp_validate);
MakeRNAServiceValidationPrototype(dcerpc_udp_validate);

static RNAServiceElement tcp_svc_element =
{
    .next = NULL,
    .validate = &dcerpc_tcp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "dcerpc",
    .ref_count = 1,
};
static RNAServiceElement udp_svc_element =
{
    .next = NULL,
    .validate = &dcerpc_udp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "udp dcerpc",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&dcerpc_tcp_validate, 135, IPPROTO_TCP},
    {&dcerpc_udp_validate, 135, IPPROTO_UDP},
    {NULL, 0, 0}
};

RNAServiceValidationModule dcerpc_service_mod =
{
    "DCERPC",
    &dcerpc_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_DCE_RPC, 0}};

static int dcerpc_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&dcerpc_udp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, NULL);
	}

    return 0;
}

MakeRNAServiceValidationPrototype(dcerpc_tcp_validate)
{
    ServiceDCERPCData *dd;
    int retval = SERVICE_INPROCESS;
    int length;

    if (dir != APP_ID_FROM_RESPONDER)
        goto inprocess;
    if (!size)
        goto inprocess;

    dd = dcerpc_service_mod.api->data_get(flowp);
    if (!dd)
    {
        dd = calloc(1, sizeof(*dd));
        if (!dd)
            return SERVICE_ENOMEM;
        if (dcerpc_service_mod.api->data_add(flowp, dd, &free))
        {
            free(dd);
            return SERVICE_ENOMEM;
        }
    }

    while (size)
    {
        length = dcerpc_validate(data, size);
        if (length < 0) goto fail;
        dd->count++;
        if (dd->count >= DCERPC_THRESHOLD)
            retval = SERVICE_SUCCESS;
        data += length;
        size -= length;
    }
    if (retval == SERVICE_SUCCESS)
    {
        dcerpc_service_mod.api->add_service(flowp, pkt, dir, &tcp_svc_element,
                                            APP_ID_DCE_RPC, NULL, NULL, NULL);
        return SERVICE_SUCCESS;
    }

inprocess:
    dcerpc_service_mod.api->service_inprocess(flowp, pkt, dir, &tcp_svc_element);
    return SERVICE_INPROCESS;

fail:
    dcerpc_service_mod.api->fail_service(flowp, pkt, dir, &tcp_svc_element);
    return SERVICE_NOMATCH;
}

MakeRNAServiceValidationPrototype(dcerpc_udp_validate)
{
    ServiceDCERPCData *dd;
    int retval = SERVICE_NOMATCH;
    int length;

    if (dir != APP_ID_FROM_RESPONDER)
        goto inprocess;
    if (!size)
        goto inprocess;

    dd = dcerpc_service_mod.api->data_get(flowp);
    if (!dd)
    {
        dd = calloc(1, sizeof(*dd));
        if (!dd)
            return SERVICE_ENOMEM;
        if (dcerpc_service_mod.api->data_add(flowp, dd, &free))
        {
            free(dd);
            return SERVICE_ENOMEM;
        }
    }

    while (size)
    {
        length = dcerpc_validate(data, size);
        if (length < 0) goto fail;
        dd->count++;
        if (dd->count >= DCERPC_THRESHOLD)
            retval = SERVICE_SUCCESS;
        data += length;
        size -= length;
    }
    if (retval == SERVICE_SUCCESS)
    {
        dcerpc_service_mod.api->add_service(flowp, pkt, dir, &udp_svc_element,
                                            APP_ID_DCE_RPC, NULL, NULL, NULL);
        return SERVICE_SUCCESS;
    }

inprocess:
    dcerpc_service_mod.api->service_inprocess(flowp, pkt, dir, &udp_svc_element);
    return SERVICE_INPROCESS;

fail:
    dcerpc_service_mod.api->fail_service(flowp, pkt, dir, &udp_svc_element);
    return SERVICE_NOMATCH;
}

