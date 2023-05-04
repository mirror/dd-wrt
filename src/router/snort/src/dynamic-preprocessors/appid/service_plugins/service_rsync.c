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

#define RSYNC_PORT  873

#define RSYNC_BANNER "@RSYNCD: "

typedef enum
{
    RSYNC_STATE_BANNER,
    RSYNC_STATE_MOTD,
    RSYNC_STATE_DONE
} RSYNCState;

typedef struct _SERVICE_RSYNC_DATA
{
    RSYNCState state;
} ServiceRSYNCData;

static int rsync_init(const InitServiceAPI * const init_api);
static int rsync_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &rsync_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "rsync",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&rsync_validate, RSYNC_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule rsync_service_mod =
{
    "RSYNC",
    &rsync_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_RSYNC, APPINFO_FLAG_SERVICE_ADDITIONAL}};

static int rsync_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&rsync_validate, IPPROTO_TCP, (uint8_t *)RSYNC_BANNER, sizeof(RSYNC_BANNER)-1, 0, "rsync", init_api->pAppidConfig);
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&rsync_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int rsync_validate(ServiceValidationArgs* args)
{
    ServiceRSYNCData *rd;
    int i;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

    rd = rsync_service_mod.api->data_get(flowp, rsync_service_mod.flow_data_index);
    if (!rd)
    {
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (rsync_service_mod.api->data_add(flowp, rd, rsync_service_mod.flow_data_index, &free))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = RSYNC_STATE_BANNER;
    }

    switch (rd->state)
    {
    case RSYNC_STATE_BANNER:
        if (size < sizeof(RSYNC_BANNER)-1) goto fail;
        if (data[size-1] != 0x0A) goto fail;
        if (strncmp((char *)data, RSYNC_BANNER, sizeof(RSYNC_BANNER)-1))
            goto fail;
        data += sizeof(RSYNC_BANNER) - 1;
        size -= sizeof(RSYNC_BANNER) - 1;
        for (i=0; i<size-1; i++)
            if (!isdigit(data[i]) && data[i] != '.') goto fail;
        rd->state = RSYNC_STATE_MOTD;
        break;
    case RSYNC_STATE_MOTD:
        if (data[size-1] != 0x0A) goto fail;
        for (i=0; i<size-1; i++)
            if (!isprint(data[i]) && !isspace(data[i])) goto fail;
        rd->state = RSYNC_STATE_DONE;
        goto success;
    default:
        goto fail;
    }

inprocess:
    rsync_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    rsync_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                       APP_ID_RSYNC, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    rsync_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                        rsync_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;
}

