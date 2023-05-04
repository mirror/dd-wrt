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

static const char BIT_BANNER[] = "\023BitTorrent protocol";

#define BIT_BANNER_LEN (sizeof(BIT_BANNER)-1)
#define RES_LEN 8
#define SHA_LEN 20
#define MAX_STR_LEN 20
#define PEER_ID_LEN 20
#define MAX_VER_LEN 4
#define LAST_BANNER_OFFSET	(BIT_BANNER_LEN+RES_LEN+SHA_LEN+PEER_ID_LEN - 1)


typedef enum
{
    BIT_STATE_BANNER = 0,
    BIT_STATE_BANNER_DC,
    BIT_STATE_MESSAGE_LEN,
    BIT_STATE_MESSAGE_DATA
} BITState;

typedef struct _CLIENT_BIT_DATA
{
    BITState state;
    unsigned stringlen;
    unsigned pos;
    union
    {
        uint32_t len;
        uint8_t raw_len[4];
    }l;
} ClientBITData;

#pragma pack(1)
typedef struct _CLIENT_BIT_MSG
{
    uint32_t len;
    uint8_t code;
} ClientBITMsg;
#pragma pack()

typedef struct _BIT_CLIENT_APP_CONFIG
{
    int enabled;
} BIT_CLIENT_APP_CONFIG;

static BIT_CLIENT_APP_CONFIG bit_config;


static CLIENT_APP_RETCODE bit_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE bit_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

SF_SO_PUBLIC tRNAClientAppModule bit_client_mod =
{
    .name = "BIT",
    .proto = IPPROTO_TCP,
    .init = &bit_init,
    .validate = &bit_validate,
    .minimum_matches = 1
};

typedef struct
{
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static Client_App_Pattern patterns[] =
{
  {(const uint8_t *)BIT_BANNER, sizeof(BIT_BANNER)-1, 0, APP_ID_BITTORRENT},
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_BITTORRENT, 0}};

static CLIENT_APP_RETCODE bit_init(const InitClientAppAPI * const init_api, SF_LIST *config)
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
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&bit_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&bit_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE bit_validate(const uint8_t *data, uint16_t size, const int dir,
                                       tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                       const struct appIdConfig_ *pConfig)
{
    ClientBITData *fd;
    uint16_t offset;

    if (dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    fd = bit_client_mod.api->data_get(flowp, bit_client_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (bit_client_mod.api->data_add(flowp, fd, bit_client_mod.flow_data_index, &free))
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
            if(data[offset] != BIT_BANNER[fd->pos])
                return CLIENT_APP_EINVALID;
        if(fd->pos == BIT_BANNER_LEN-1)
                fd->state = BIT_STATE_BANNER_DC;
            fd->pos++;
            break;
        case BIT_STATE_BANNER_DC:
        if(fd->pos == LAST_BANNER_OFFSET)
            {
                fd->pos = 0;
                fd->state = BIT_STATE_MESSAGE_LEN;
                break;
            }
            fd->pos++;
            break;
        case BIT_STATE_MESSAGE_LEN:
            fd->l.raw_len[fd->pos] = data[offset];
            fd->pos++;
            if(fd->pos >= offsetof(ClientBITMsg , code))
            {
                fd->stringlen = ntohl(fd->l.len) ;
                fd->state = BIT_STATE_MESSAGE_DATA;
                if(!fd->stringlen)
                {
                    if(offset == size-1)
                        goto done;
                    return CLIENT_APP_EINVALID;
                }
                fd->pos = 0;
            }
            break;

        case BIT_STATE_MESSAGE_DATA:
            fd->pos++;
            if(fd->pos == fd->stringlen)
                goto done;
            break;
        default:
            goto inprocess;
        }
    offset++;
    }
inprocess:
    return CLIENT_APP_INPROCESS;

done:
    bit_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_BITTORRENT, APP_ID_BITTORRENT, NULL);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

