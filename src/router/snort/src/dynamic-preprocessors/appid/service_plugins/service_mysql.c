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

#pragma pack(1)

typedef struct _SERVICE_MYSQL_HEADER
{
    union
    {
        uint32_t len;
        struct
        {
            uint8_t len[3];
            uint8_t packet;
        } p;
    } l;
    uint8_t proto;
} ServiceMYSQLHdr;

#pragma pack()

static int svc_mysql_init(const InitServiceAPI * const init_api);
static int svc_mysql_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &svc_mysql_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "mysql",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&svc_mysql_validate, 3306, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule mysql_service_mod =
{
    "MYSQL",
    &svc_mysql_init,
    pp
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_MYSQL, APPINFO_FLAG_SERVICE_ADDITIONAL}
};

static int svc_mysql_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&svc_mysql_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int svc_mysql_validate(ServiceValidationArgs* args)
{
    const uint8_t *data = args->data;
    const ServiceMYSQLHdr *hdr = (const ServiceMYSQLHdr *)data;
    uint32_t len;
    const uint8_t *end;
    const uint8_t *p = NULL;
    tAppIdData *flowp = args->flowp;
    uint16_t size = args->size;

    if (!size) goto inprocess;
    if (args->dir != APP_ID_FROM_RESPONDER) goto inprocess;
    if (size < sizeof(ServiceMYSQLHdr)) goto fail;

    len = hdr->l.p.len[0];
    len |= hdr->l.p.len[1] << 8;
    len |= hdr->l.p.len[2] << 16;
    len += 4;
    if (len > size) goto fail;
    if (hdr->l.p.packet) goto fail;
    if (hdr->proto != 0x0A) goto fail;

    end = data + len;
    data += sizeof(ServiceMYSQLHdr);
    p = data;
    for (; data<end && *data; data++)
    {
        if (!isprint(*data)) goto fail;
    }
    if (data >= end) goto fail;
    if (data == p) p = NULL;
    data += 5;
    if (data >= end) goto fail;
    for (; data<end && *data; data++)
    {
        if (!isprint(*data)) goto fail;
    }
    data += 6;
    if (data >= end) goto fail;
    mysql_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                       APP_ID_MYSQL, NULL, (char *)p, NULL, NULL);
    return SERVICE_SUCCESS;

inprocess:
    mysql_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

fail:
    mysql_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                        mysql_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;

}

