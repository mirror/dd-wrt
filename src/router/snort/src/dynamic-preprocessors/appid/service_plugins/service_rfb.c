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

#define RFB_BANNER_SIZE 12

#define RFB_BANNER "RFB "

static int rfb_init(const InitServiceAPI * const init_api);
static int rfb_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &rfb_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "rfb",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&rfb_validate, 5900, IPPROTO_TCP},
    {&rfb_validate, 5901, IPPROTO_TCP},
    {&rfb_validate, 5902, IPPROTO_TCP},
    {&rfb_validate, 5903, IPPROTO_TCP},
    {&rfb_validate, 5904, IPPROTO_TCP},
    {&rfb_validate, 5905, IPPROTO_TCP},
    {&rfb_validate, 5906, IPPROTO_TCP},
    {&rfb_validate, 5907, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule rfb_service_mod =
{
    "RFB",
    &rfb_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_VNC, APPINFO_FLAG_SERVICE_ADDITIONAL},
    {APP_ID_VNC_RFB, APPINFO_FLAG_SERVICE_ADDITIONAL}
};

static int rfb_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&rfb_validate, IPPROTO_TCP, (uint8_t *)RFB_BANNER, sizeof(RFB_BANNER)-1, 0, "rfb", init_api->pAppidConfig);
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&rfb_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int rfb_validate(ServiceValidationArgs* args)
{
    char version[RFB_BANNER_SIZE-4];
    unsigned i;
    char *v;
    const unsigned char *p;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (!size) goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER) goto inprocess;

    if (size != RFB_BANNER_SIZE) goto fail;
    if (strncmp(RFB_BANNER, (char *)data, sizeof(RFB_BANNER)-1)) goto fail;
    if (data[7] != '.' || data[RFB_BANNER_SIZE-1] != 0x0A) goto fail;
    if (!isdigit(data[4]) || !isdigit(data[5]) || !isdigit(data[6]) ||
        !isdigit(data[8]) || !isdigit(data[9]) || !isdigit(data[10]))
    {
        goto fail;
    }
    v = version;
    p = &data[4];
    for (i=4; i<RFB_BANNER_SIZE-1; i++)
    {
        *v = *p;
        v++;
        p++;
    }
    *v = 0;
    rfb_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                     APP_ID_VNC_RFB, NULL, version, NULL, NULL);
    return SERVICE_SUCCESS;

inprocess:
    rfb_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

fail:
    rfb_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                      rfb_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

