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

static const char TIMBUKTU_BANNER[] = "\000\001";

#define TIMBUKTU_BANNER_LEN (sizeof(TIMBUKTU_BANNER)-1)
#define MAX_ANY_SIZE	2


typedef enum
{
    TIMBUKTU_STATE_BANNER = 0,
    TIMBUKTU_STATE_ANY_MESSAGE_LEN,
    TIMBUKTU_STATE_MESSAGE_LEN,
    TIMBUKTU_STATE_MESSAGE_DATA
} TIMBUKTUState;

typedef struct _CLIENT_TIMBUKTU_DATA
{
    TIMBUKTUState state;
    uint16_t stringlen;
    unsigned pos;
    union
    {
        uint16_t len;
	uint8_t raw_len[2];
    }l;
} ClientTIMBUKTUData;

#pragma pack(1)
typedef struct _CLIENT_TIMBUKTU_MSG
{
    uint16_t len;
    uint8_t message;
} ClientTIMBUKTUMsg;
#pragma pack()

typedef struct _TIMBUKTU_CLIENT_APP_CONFIG
{
    int enabled;
} TIMBUKTU_CLIENT_APP_CONFIG;

static TIMBUKTU_CLIENT_APP_CONFIG timbuktu_config;


static CLIENT_APP_RETCODE timbuktu_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE timbuktu_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

SF_SO_PUBLIC tRNAClientAppModule timbuktu_client_mod =
{
    .name = "TIMBUKTU",
    .proto = IPPROTO_TCP,
    .init = &timbuktu_init,
    .validate = &timbuktu_validate,
    .minimum_matches = 1
};

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static Client_App_Pattern patterns[] =
{
    {(const uint8_t *)TIMBUKTU_BANNER, sizeof(TIMBUKTU_BANNER)-1, 0, APP_ID_TIMBUKTU},
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_TIMBUKTU, 0}};

static CLIENT_APP_RETCODE timbuktu_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    timbuktu_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                timbuktu_config.enabled = atoi(item->value);
            }
        }
    }

    if (timbuktu_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&timbuktu_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&timbuktu_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE timbuktu_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
    ClientTIMBUKTUData *fd;
    uint16_t offset;

    if (dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    fd = timbuktu_client_mod.api->data_get(flowp, timbuktu_client_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (timbuktu_client_mod.api->data_add(flowp, fd, timbuktu_client_mod.flow_data_index, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->state = TIMBUKTU_STATE_BANNER;
    }

    offset = 0;
    while(offset < size)
    {
        switch (fd->state)
        {
        case TIMBUKTU_STATE_BANNER:
            if(data[offset] != TIMBUKTU_BANNER[fd->pos])
                return CLIENT_APP_EINVALID;
            if(fd->pos >= TIMBUKTU_BANNER_LEN-1)
            {
                fd->pos = 0;
                fd->state = TIMBUKTU_STATE_ANY_MESSAGE_LEN;
	        break;
            }
            fd->pos++;
            break;
	    /* cheeck any 2 bytes fisrt */
        case TIMBUKTU_STATE_ANY_MESSAGE_LEN:
            fd->pos++;
            if(fd->pos >= MAX_ANY_SIZE)
	    {
                fd->pos = 0;
                fd->state = TIMBUKTU_STATE_MESSAGE_LEN;
                break;
	    }
            break;
        case TIMBUKTU_STATE_MESSAGE_LEN:
            if(fd->pos < offsetof(ClientTIMBUKTUMsg, message))
	    {
                fd->l.raw_len[fd->pos] = data[offset];
	    }
            fd->pos++;
            if(fd->pos >= offsetof(ClientTIMBUKTUMsg, message))
            {
                fd->stringlen = ntohs(fd->l.len);
		if(!fd->stringlen)
		{
                    if(offset == size - 1)
                        goto done;
                    return CLIENT_APP_EINVALID;
                }
		else if((fd->stringlen + TIMBUKTU_BANNER_LEN + MAX_ANY_SIZE + offsetof(ClientTIMBUKTUMsg, message)) > size)
                    return CLIENT_APP_EINVALID;
                fd->state = TIMBUKTU_STATE_MESSAGE_DATA;
                fd->pos = 0;
            }
            break;
        case TIMBUKTU_STATE_MESSAGE_DATA:
            fd->pos++;
            if(fd->pos == fd->stringlen)
            {
                if(offset == size - 1)
                    goto done;
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
    timbuktu_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_TIMBUKTU, APP_ID_TIMBUKTU, NULL);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

