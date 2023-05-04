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

#ifdef HAVE_CONFIG_H
#include "config.h"     /* for WORDS_BIGENDIAN */
#endif
#include "client_app_api.h"
#include "client_app_rtp.h"

typedef enum
{
    RTP_STATE_CONNECTION,
    RTP_STATE_CONTINUE
} RTPState;

#define MAX_REMOTE_SIZE    128
#define NUMBER_OF_PACKETS  3
#define MAX_SSRC_SWITCHES  2

typedef struct
{
    RTPState state;
    uint16_t seq;
    uint8_t count;
    uint32_t timestamp;
    uint32_t ssrc;
    uint8_t numSsrcSwitches;
} ClientRTPDirData;

typedef struct _CLIENT_RTP_DATA
{
    ClientRTPDirData initiatorData;
    ClientRTPDirData responderData;
} ClientRTPData;

typedef struct _RTP_CLIENT_APP_CONFIG
{
    int enabled;
} RTP_CLIENT_APP_CONFIG;

static RTP_CLIENT_APP_CONFIG rtp_config;


static CLIENT_APP_RETCODE rtp_init(const InitClientAppAPI * const init_api, SF_LIST *config);
STATIC CLIENT_APP_RETCODE rtp_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig);

SF_SO_PUBLIC tRNAClientAppModule rtp_client_mod =
{
    .name = "RTP",
    .proto = IPPROTO_UDP,
    .init = &rtp_init,
    .validate = &rtp_validate,
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
    {(const uint8_t *)"\x000\x000", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x001", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x002", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x003", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x004", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x005", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x006", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x007", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x008", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x009", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x00a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x00b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x00c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x00d", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x00e", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x00f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x010", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x011", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x012", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x013", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x019", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x01a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x01b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x01c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x01f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x020", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x021", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x022", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x080", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x081", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x082", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x083", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x084", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x085", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x086", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x087", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x088", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x089", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x08a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x08b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x08c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x08d", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x08e", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x08f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x090", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x091", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x092", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x093", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x099", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x09a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x09b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x09c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x09f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x0a0", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x0a1", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x000\x0a2", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x000", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x001", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x002", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x003", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x004", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x005", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x006", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x007", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x008", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x009", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x00a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x00b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x00c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x00d", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x00e", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x00f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x010", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x011", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x012", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x013", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x019", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x01a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x01b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x01c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x01f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x020", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x021", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x022", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x080", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x081", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x082", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x083", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x084", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x085", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x086", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x087", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x088", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x089", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x08a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x08b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x08c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x08d", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x08e", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x08f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x090", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x091", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x092", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x093", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x099", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x09a", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x09b", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x09c", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x09f", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x0a0", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x0a1", 2, 0, APP_ID_RTP},
    {(const uint8_t *)"\x080\x0a2", 2, 0, APP_ID_RTP},
};

static tAppRegistryEntry appIdRegistry[] = {{APP_ID_RTP, 0}};

static CLIENT_APP_RETCODE rtp_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    rtp_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                rtp_config.enabled = atoi(item->value);
            }
        }
    }

    if (rtp_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&rtp_validate, IPPROTO_UDP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

    unsigned j;
    for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
        init_api->RegisterAppId(&rtp_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
    }

    return CLIENT_APP_SUCCESS;
}

static inline void rtpInitDirData(ClientRTPDirData* dirData, ClientRTPMsg* hdr)
{
    dirData->seq = ntohs(hdr->seq);
    dirData->timestamp = ntohl(hdr->timestamp);
    dirData->ssrc = ntohl(hdr->ssrc);
    dirData->count = 1;
}

static inline CLIENT_APP_RETCODE rtpValidateDirData(ClientRTPDirData* dirData, ClientRTPMsg* hdr)
{
    if ((ntohs(hdr->seq) != ++dirData->seq) ||
        (ntohl(hdr->timestamp) < dirData->timestamp))
        return CLIENT_APP_EINVALID;

    if (ntohl(hdr->ssrc) != dirData->ssrc)
    {
        if (++dirData->numSsrcSwitches > MAX_SSRC_SWITCHES)
            return CLIENT_APP_EINVALID;

        rtpInitDirData(dirData, hdr);
        return CLIENT_APP_INPROCESS;
    }

    dirData->timestamp = ntohl(hdr->timestamp);
    if (++dirData->count < NUMBER_OF_PACKETS)
        return CLIENT_APP_INPROCESS;

    return CLIENT_APP_SUCCESS;
}

STATIC CLIENT_APP_RETCODE rtp_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
    ClientRTPData *fd;
    ClientRTPMsg *hdr;
    RTPState *state;

    if (!size)
        return CLIENT_APP_INPROCESS;
    if (size < sizeof(ClientRTPMsg))
        return CLIENT_APP_EINVALID;
    hdr = (ClientRTPMsg *)data;
    if (hdr->vers > 2 || hdr->payloadtype > 34)
        return CLIENT_APP_EINVALID;

    fd = rtp_client_mod.api->data_get(flowp, rtp_client_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (rtp_client_mod.api->data_add(flowp, fd, rtp_client_mod.flow_data_index, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->initiatorData.state = RTP_STATE_CONNECTION;
        fd->responderData.state = RTP_STATE_CONNECTION;
    }

    state = (dir == APP_ID_FROM_INITIATOR ? &fd->initiatorData.state : &fd->responderData.state); 

    switch (*state)
    {
    CLIENT_APP_RETCODE retVal;

    case RTP_STATE_CONNECTION:
        if (dir == APP_ID_FROM_INITIATOR) rtpInitDirData(&fd->initiatorData, hdr);
        else rtpInitDirData(&fd->responderData, hdr);

        *state = RTP_STATE_CONTINUE;
        return CLIENT_APP_INPROCESS;

    case RTP_STATE_CONTINUE:
        if (dir == APP_ID_FROM_INITIATOR) retVal = rtpValidateDirData(&fd->initiatorData, hdr);
        else retVal = rtpValidateDirData(&fd->responderData, hdr);

        if (retVal != CLIENT_APP_SUCCESS) return retVal;
        break;

    default:
        return CLIENT_APP_INPROCESS;
    }

    rtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_RTP, APP_ID_RTP, NULL);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

