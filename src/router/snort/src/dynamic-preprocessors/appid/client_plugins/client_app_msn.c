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
#include "client_app_api.h"
#include "client_app_msn.h"

typedef struct _MSN_CLIENT_APP_CONFIG
{
    int enabled;
} MSN_CLIENT_APP_CONFIG;

static MSN_CLIENT_APP_CONFIG msn_config;

#define MAX_VERSION_SIZE    64

static CLIENT_APP_RETCODE msn_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE msn_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

tRNAClientAppModule msn_client_mod =
{
    .name = "MSN",
    .proto = IPPROTO_TCP,
    .init = &msn_init,
    .validate = &msn_validate,
    .minimum_matches = 2
};

typedef struct {
    const uint8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static const uint8_t VER[] = "VER ";
static const uint8_t CVRMAIN[] = "CVR0\x00d\x00a";
static const uint8_t CVR[] = "CVR";
static const uint8_t MSNMSGR[] = "MSNMSGR";
static const uint8_t MACMSGS[] = "macmsgs";
static const uint8_t MSMSGS[] = "MSMSGS";

static Client_App_Pattern patterns[] =
{
	{VER,     sizeof(VER)-1,      0, APP_ID_MSN},
	{CVRMAIN, sizeof(CVRMAIN)-1, -1, APP_ID_MSN},
	{MSNMSGR, sizeof(MSNMSGR)-1, -1, APP_ID_MSN_MESSENGER},
	{MACMSGS, sizeof(MACMSGS)-1, -1, APP_ID_MSN_MESSENGER},
	{MSMSGS,  sizeof(MSMSGS)-1,  -1, APP_ID_MICROSOFT_WINDOWS_MESSENGER}
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_MICROSOFT_WINDOWS_MESSENGER, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_MSN_MESSENGER, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_MSN, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_MSNP, APPINFO_FLAG_CLIENT_ADDITIONAL}
};

static CLIENT_APP_RETCODE msn_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

	msn_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                msn_config.enabled = atoi(item->value);
            }
        }
    }

    if (msn_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
			_dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern,
                    patterns[i].index);
            init_api->RegisterPattern(&msn_validate, IPPROTO_TCP, patterns[i].pattern,
                                      patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&msn_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE msn_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
	const u_int8_t *end;
	u_int8_t version[MAX_VERSION_SIZE];
	u_int8_t *v;
    u_int8_t *v_end;
    uint32_t product_id;

	product_id = APP_ID_MSN_MESSENGER;
	memset(&version,0,sizeof(version));

	if (!data || !msn_client_mod.api || !flowp || !pkt) return CLIENT_APP_ENULL;

	if (dir != APP_ID_FROM_INITIATOR) return CLIENT_APP_INPROCESS;

	if (size >= sizeof(CVR) && memcmp(data, CVR, sizeof(CVR)-1) == 0)
	{
		int space_count = 0;

		end = data + size;

		while( data < end && space_count < 6 ) /* Skip to the product and version strings */
		{
			if( *data == ' ' )
				space_count++;

			data++;
		}

		/* Get the product */
		if( end-data >= (int)sizeof(MSNMSGR) && memcmp(data, MSNMSGR, sizeof(MSNMSGR)-1) == 0 )
		{
			product_id = APP_ID_MSN_MESSENGER;
			data += sizeof(MSNMSGR) - 1;

			data++; /* skip the space */
		}
		else if( end-data >= (int)sizeof(MACMSGS) && memcmp(data, MACMSGS, sizeof(MACMSGS)-1) == 0 )
		{
			product_id = APP_ID_MSN_MESSENGER;
			data += sizeof(MACMSGS) - 1;

			data++; /* skip the space */
		}
                else if( end-data >= (int)sizeof(MSMSGS) && memcmp(data, MSMSGS, sizeof(MSMSGS)-1) == 0 )
                {
                    product_id = APP_ID_MICROSOFT_WINDOWS_MESSENGER;
                    data += sizeof(MSMSGS) - 1;

                    data++; /* skip the space */
                }
		else /* advance past the unknown product name */
		{
			while( data < end && *data != ' ')
				data++;

			data++; /* skip the space */
		}

		v = version;

		v_end = v + (MAX_VERSION_SIZE - 1);

		/* Get the version */
		while( data < end && *data != ' ' && v < v_end )
		{
			*v = *data;
			v++;
			data++;
		}

		goto done;
	}

	return CLIENT_APP_INPROCESS;

done:
    msn_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_MSN_MESSENGER, product_id, (char *)version);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

