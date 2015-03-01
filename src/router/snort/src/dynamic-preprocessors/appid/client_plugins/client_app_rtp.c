/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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

typedef enum
{
    RTP_STATE_CONNECTION,
    RTP_STATE_CONTINUE
} RTPState;

#define MAX_REMOTE_SIZE    128
#define NUMBER_OF_PACKETS  3

typedef struct _CLIENT_RTP_DATA
{
    RTPState state;
    uint8_t  pos;
    uint16_t init_seq;
    uint16_t resp_seq;
    uint8_t init_count;
    uint8_t resp_count;
    uint32_t init_timestamp;
    uint32_t resp_timestamp;
    uint32_t init_ssrc;
    uint32_t resp_ssrc;
} ClientRTPData;

#pragma pack(1)
typedef struct _CLIENT_RTP_MSG
{
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t vers:2,
             padding:1,
             extension:1,
             count:4;
    uint8_t marker:1,
             payloadtype:7;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t count:4,
             extension:1,
             padding:1,
             vers:2;
    uint8_t payloadtype:7,
             marker:1;
#else
#error "Please fix <endian.h>"
#endif
    uint16_t seq;
    uint32_t timestamp;
    uint32_t ssrc;
} ClientRTPMsg;
#pragma pack()

typedef struct _RTP_CLIENT_APP_CONFIG
{
    int enabled;
} RTP_CLIENT_APP_CONFIG;

static RTP_CLIENT_APP_CONFIG rtp_config;


static CLIENT_APP_RETCODE rtp_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE rtp_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData);

SO_PUBLIC RNAClientAppModule rtp_client_mod =
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
            init_api->RegisterPattern(&rtp_validate, IPPROTO_UDP, patterns[i].pattern, patterns[i].length, patterns[i].index);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&rtp_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, NULL);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE rtp_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData)
{
    ClientRTPData *fd;
    ClientRTPMsg *hdr;

    if (!size)
        return CLIENT_APP_INPROCESS;

    fd = rtp_client_mod.api->data_get(flowp);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (rtp_client_mod.api->data_add(flowp, fd, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->state = RTP_STATE_CONNECTION;
    }

    switch (fd->state)
    {
    case RTP_STATE_CONNECTION:
        if (size < sizeof(ClientRTPMsg))
            return CLIENT_APP_EINVALID;
        hdr = (ClientRTPMsg *)data;
        if (hdr->vers > 2 || hdr->payloadtype > 34)
            return CLIENT_APP_EINVALID;
        if (dir == APP_ID_FROM_INITIATOR)
        {
            fd->init_seq = ntohs(hdr->seq);
            fd->init_timestamp = ntohl(hdr->timestamp);
            fd->init_ssrc = ntohl(hdr->ssrc);
            fd->init_count++;
        }
        else
        {
            fd->resp_seq = ntohs(hdr->seq);
            fd->resp_timestamp = ntohl(hdr->timestamp);
            fd->resp_ssrc = ntohl(hdr->ssrc);
            fd->resp_count++;
        }
        fd->state = RTP_STATE_CONTINUE;
        return CLIENT_APP_INPROCESS;

    case RTP_STATE_CONTINUE:
        if (size < sizeof(ClientRTPMsg))
            return CLIENT_APP_EINVALID;
        hdr = (ClientRTPMsg *)data;
        if (hdr->vers > 2)
            return CLIENT_APP_EINVALID;
        if (hdr->payloadtype > 34)
            return CLIENT_APP_EINVALID;
        if (dir == APP_ID_FROM_INITIATOR)
        {
            if ((ntohs(hdr->seq) != ++fd->init_seq) ||
                (ntohl(hdr->ssrc) != fd->init_ssrc) ||
                (ntohl(hdr->timestamp) < fd->init_timestamp))
                return CLIENT_APP_EINVALID;
            fd->init_timestamp = ntohl(hdr->timestamp);
            if (++fd->init_count < NUMBER_OF_PACKETS)
                return CLIENT_APP_INPROCESS;
        }
        else
        {
            if ((ntohs(hdr->seq) != ++fd->resp_seq) ||
                (ntohl(hdr->ssrc) != fd->resp_ssrc) ||
                (ntohl(hdr->timestamp) < fd->resp_timestamp))
                return CLIENT_APP_EINVALID;
            fd->resp_timestamp = ntohl(hdr->timestamp);
            if (++fd->resp_count < NUMBER_OF_PACKETS)
                return CLIENT_APP_INPROCESS;
        }
        break;

    default:
        return CLIENT_APP_INPROCESS;
    }

    rtp_client_mod.api->add_app(flowp, APP_ID_RTP, APP_ID_RTP, NULL);
    flow_mark(flowp, FLOW_CLIENTAPPDETECTED);
    return CLIENT_APP_SUCCESS;
}

