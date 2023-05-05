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

static const char VNC_BANNER[] = "RFB ";
static const char VNC_BANNER2[] = ".";

#define VNC_BANNER_LEN (sizeof(VNC_BANNER)-1)


typedef enum
{
    VNC_STATE_BANNER = 0,
    VNC_STATE_VERSION
} VNCState;

#define MAX_VERSION_SIZE	8
typedef struct _CLIENT_VNC_DATA
{
    VNCState state;
    unsigned pos;
    uint8_t version[MAX_VERSION_SIZE];
} ClientVNCData;

typedef struct _VNC_CLIENT_APP_CONFIG
{
    int enabled;
} VNC_CLIENT_APP_CONFIG;

static VNC_CLIENT_APP_CONFIG vnc_config;


static CLIENT_APP_RETCODE vnc_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE vnc_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

SF_SO_PUBLIC tRNAClientAppModule vnc_client_mod =
{
    .name = "RFB",
    .proto = IPPROTO_TCP,
    .init = &vnc_init,
    .validate = &vnc_validate,
    .minimum_matches = 2
};

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static Client_App_Pattern patterns[] =
{
    {(const uint8_t *)VNC_BANNER,  sizeof(VNC_BANNER)-1,  0, APP_ID_VNC},
    {(const uint8_t *)VNC_BANNER2, sizeof(VNC_BANNER2)-1, 7, APP_ID_VNC},
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_VNC, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_VNC_RFB, APPINFO_FLAG_CLIENT_ADDITIONAL}
};

static CLIENT_APP_RETCODE vnc_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    vnc_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                vnc_config.enabled = atoi(item->value);
            }
        }
    }

    if (vnc_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&vnc_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&vnc_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE vnc_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
    ClientVNCData *fd;
    uint16_t offset;

    if (dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    fd = vnc_client_mod.api->data_get(flowp, vnc_client_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (vnc_client_mod.api->data_add(flowp, fd, vnc_client_mod.flow_data_index, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->state = VNC_STATE_BANNER;
    }

    offset = 0;
    while(offset < size)
    {
        switch (fd->state)
        {
        case VNC_STATE_BANNER:
            if(data[offset] != VNC_BANNER[fd->pos])
                return CLIENT_APP_EINVALID;
            if(fd->pos >= VNC_BANNER_LEN-1)
            {
                fd->state = VNC_STATE_VERSION;
                fd->pos = 0;
		break;
            }
            fd->pos++;
            break;
        case VNC_STATE_VERSION:
	    if((isdigit(data[offset]) || data[offset] == '.' || data[offset] == '\n') && fd->pos < MAX_VERSION_SIZE)
            {
	        fd->version[fd->pos] = data[offset];
	        if(data[offset] == '\n' && fd->pos == 7)
                {
	            fd->version[fd->pos] = 0;
	            goto done;
                }
            }
	    else
                return CLIENT_APP_EINVALID;
            fd->pos++;
            break;
        default:
	    goto inprocess;
        }
	offset++;
    }
inprocess:
    return CLIENT_APP_INPROCESS;

done:
    vnc_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_VNC_RFB, APP_ID_VNC, (const char *) fd->version);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

