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

static const char UDP_BIT_QUERY[] = "d1:a";
static const char UDP_BIT_RESPONSE[] = "d1:r";
static const char UDP_BIT_ERROR[] = "d1:e";
static const char UDP_BIT_FIRST[] = "d1:";
static const char UDP_BIT_COMMON_END[] = "1:y1:";

#define UDP_BIT_FIRST_LEN (sizeof(UDP_BIT_FIRST)-1)
#define UDP_BIT_COMMON_END_LEN (sizeof(UDP_BIT_COMMON_END)-1)
#define UDP_BIT_END_LEN (UDP_BIT_COMMON_END_LEN+2)

typedef enum
{
    BIT_STATE_BANNER = 0,
    BIT_STATE_TYPES,
    BIT_STATE_DC,
    BIT_STATE_CHECK_END,
    BIT_STATE_CHECK_END_TYPES,
    BIT_STATE_CHECK_LAST
} BITState;

typedef enum
{
    BIT_TYPE_REQUEST = 1,
    BIT_TYPE_RESPONSE,
    BIT_TYPE_ERROR
} BITType;

typedef struct _CLIENT_BIT_DATA
{
    BITState state;
    BITType type;
    unsigned pos;
} ClientBITData;

typedef struct _BIT_CLIENT_APP_CONFIG
{
    int enabled;
} BIT_CLIENT_APP_CONFIG;

static BIT_CLIENT_APP_CONFIG bit_config;


static CLIENT_APP_RETCODE udp_bit_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE udp_bit_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

SF_SO_PUBLIC tRNAClientAppModule bit_tracker_client_mod =
{
    .name = "BIT-UDP",
    .proto = IPPROTO_UDP,
    .init = &udp_bit_init,
    .validate = &udp_bit_validate,
    .minimum_matches = 1
};

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static Client_App_Pattern udp_patterns[] =
{
  {(const uint8_t *)UDP_BIT_QUERY,    sizeof(UDP_BIT_QUERY),    0, APP_ID_BITTRACKER_CLIENT},
  {(const uint8_t *)UDP_BIT_RESPONSE, sizeof(UDP_BIT_RESPONSE), 0, APP_ID_BITTRACKER_CLIENT},
  {(const uint8_t *)UDP_BIT_ERROR,    sizeof(UDP_BIT_ERROR),    0, APP_ID_BITTRACKER_CLIENT},
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_BITTRACKER_CLIENT, 0}};

static CLIENT_APP_RETCODE udp_bit_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    bit_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                bit_config.enabled = atoi(item->value);
            }
        }
    }

    if (bit_config.enabled)
    {
        for (i=0; i < sizeof(udp_patterns)/sizeof(*udp_patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)udp_patterns[i].pattern, udp_patterns[i].index);
            init_api->RegisterPattern(&udp_bit_validate, IPPROTO_UDP, udp_patterns[i].pattern, udp_patterns[i].length, udp_patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&udp_bit_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE udp_bit_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
    ClientBITData *fd;
    uint16_t offset;

    if (size < (UDP_BIT_FIRST_LEN + UDP_BIT_END_LEN + 3))
        return CLIENT_APP_EINVALID;

    fd = bit_tracker_client_mod.api->data_get(flowp, bit_tracker_client_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (bit_tracker_client_mod.api->data_add(flowp, fd, bit_tracker_client_mod.flow_data_index, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->state = BIT_STATE_BANNER;
    }

    offset = 0;
    while(offset < size)
    {
        switch (fd->state)
        {
        case BIT_STATE_BANNER:
            if(data[offset] != UDP_BIT_FIRST[fd->pos])
                return CLIENT_APP_EINVALID;
	    if(fd->pos == UDP_BIT_FIRST_LEN-1)
                fd->state = BIT_STATE_TYPES;
            fd->pos++;
            break;
        case BIT_STATE_TYPES:
	    switch(data[offset])
            {
            case 'a':
                fd->type = BIT_TYPE_REQUEST;
                fd->state = BIT_STATE_DC;
                break;
            case 'r':
                fd->type = BIT_TYPE_RESPONSE;
                fd->state = BIT_STATE_DC;
                break;
            case 'e':
                fd->type = BIT_TYPE_ERROR;
                fd->state = BIT_STATE_DC;
                break;
	    default:
                return CLIENT_APP_EINVALID;
            }
            break;

        case BIT_STATE_DC:
            if(offset < (size - UDP_BIT_END_LEN))
                break;
	    else if(offset == (size - UDP_BIT_END_LEN) && data[offset] == UDP_BIT_COMMON_END[0])
            {
                fd->state = BIT_STATE_CHECK_END;
                fd->pos = 0;
            }
            else
                return CLIENT_APP_EINVALID;
            /*fall through */
        case BIT_STATE_CHECK_END:
            if(data[offset] != UDP_BIT_COMMON_END[fd->pos])
                return CLIENT_APP_EINVALID;
	    if(fd->pos == UDP_BIT_COMMON_END_LEN-1)
                fd->state = BIT_STATE_CHECK_END_TYPES;
            fd->pos++;
            break;

        case BIT_STATE_CHECK_END_TYPES:
	    switch(data[offset])
            {
            case 'q':
                if(fd->type != BIT_TYPE_REQUEST)
                    return CLIENT_APP_EINVALID;
                fd->state = BIT_STATE_CHECK_LAST;
                break;
            case 'r':
                if(fd->type != BIT_TYPE_RESPONSE)
                    return CLIENT_APP_EINVALID;
                fd->state = BIT_STATE_CHECK_LAST;
                break;
            case 'e':
                if(fd->type != BIT_TYPE_ERROR)
                    return CLIENT_APP_EINVALID;
                fd->state = BIT_STATE_CHECK_LAST;
                break;
	    default:
                return CLIENT_APP_EINVALID;
            }
            break;

        case BIT_STATE_CHECK_LAST:
	    switch(data[offset])
            {
            case 'e':
	        goto done;
	    default:
                return CLIENT_APP_EINVALID;
            }
            break;

        default:
	    goto inprocess;
        }
	offset++;
    }
inprocess:
    return CLIENT_APP_INPROCESS;

done:
    bit_tracker_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_BITTORRENT, APP_ID_BITTRACKER_CLIENT, NULL);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}
