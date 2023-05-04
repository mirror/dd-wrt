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

#include "client_app_api.h"
#include "client_app_ym.h"
#include "appInfoTable.h"

typedef struct _YM_CLIENT_APP_CONFIG
{
    int enabled;
} YM_CLIENT_APP_CONFIG;

static YM_CLIENT_APP_CONFIG ym_config;

#define MAX_VERSION_SIZE    64

static CLIENT_APP_RETCODE ym_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE ym_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

tRNAClientAppModule ym_client_mod =
{
    .name = "YM",
    .proto = IPPROTO_TCP,
    .init = &ym_init,
    .validate = &ym_validate,
    .minimum_matches = 1
};

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static const uint8_t APP_YMSG[] = "YMSG";

static Client_App_Pattern patterns[] =
{
    {APP_YMSG, sizeof(APP_YMSG)-1, 0, APP_ID_YAHOO_MSG},
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_YAHOO, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_YAHOO_MSG, APPINFO_FLAG_CLIENT_ADDITIONAL}
};

static CLIENT_APP_RETCODE ym_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

	ym_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                ym_config.enabled = atoi(item->value);
            }
        }
    }

    if (ym_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
			_dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char*)patterns[i].pattern,
                    patterns[i].index);
            init_api->RegisterPattern(&ym_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&ym_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static const uint8_t * skip_separator(const uint8_t *data, const uint8_t *end)
{
	while( data + 1 < end  )
	{
		if( data[0] == 0xc0 && data[1] == 0x80 )
			break;

		data++;
	}

	data += 2;

	return data;
}

static CLIENT_APP_RETCODE ym_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
#define HEADERSIZE 20
#define VERSIONID "135"
#define SEPARATOR 0xc080

	const uint8_t *end;
	uint16_t len;
	uint8_t version[MAX_VERSION_SIZE];
	uint8_t *v;
    uint8_t *v_end;
    uint32_t product_id;

	product_id = APP_ID_YAHOO;
	memset(&version,0,sizeof(version));

	_dpd.debugMsg(DEBUG_LOG,"Found yahoo! client: %zu\n",sizeof(VERSIONID));

	if (!data || !ym_client_mod.api || !flowp || !pkt) return CLIENT_APP_ENULL;

	if (dir != APP_ID_FROM_INITIATOR) return CLIENT_APP_INPROCESS;

	/* Validate the packet using the length field, otherwise abort. */
	if( size < 10 )
		return CLIENT_APP_ENULL;

	len = *((uint16_t*) (data + 8));
	len = ntohs(len);

	if( len != (size - HEADERSIZE) )
		return CLIENT_APP_ENULL;

	end = data + size;

	if( size >= HEADERSIZE )
	{
		data += HEADERSIZE;
	}

	while( data < end )
	{
		if( end-data >= (int)sizeof(VERSIONID) && memcmp(data, VERSIONID, sizeof(VERSIONID)-1) == 0 )
		{
			data += sizeof(VERSIONID)-1;

			if( data + 2 >= end )  /* Skip the separator */
				goto done;
			else
				data += 2;

			product_id = APP_ID_YAHOO;

			v = version;

			v_end = v + (MAX_VERSION_SIZE - 1);

			/* Get the version */
			while( data + 1 < end && v < v_end )
			{
				if( data[0] == 0xc0 && data[1] == 0x80 )
				break;

				*v = *data;
				v++;
				data++;
			}

			goto done;
		}

		data = skip_separator(data,end); /*skip to the command end separator */
		data = skip_separator(data,end); /* skip to the command data end separator */
	}

	return CLIENT_APP_INPROCESS;

done:
    ym_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_YAHOO, product_id, (char *)version);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

