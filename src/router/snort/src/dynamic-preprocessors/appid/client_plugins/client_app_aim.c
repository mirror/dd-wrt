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
#include <stdint.h>

#include "appInfoTable.h"
#include "client_app_api.h"
#include "client_app_aim.h"

#pragma pack(1)

typedef struct _FLAP_FNAC_SIGNON
{
    uint16_t len;
} FLAPFNACSignOn;

typedef struct _FLAP_FNAC
{
    uint16_t family;
    uint16_t subtype;
    uint16_t flags;
    uint32_t id;
} FLAPFNAC;

typedef struct _FLAP_TLV
{
    uint16_t subtype;
    uint16_t len;
} FLAPTLV;

typedef struct _FLAP_HEADER
{
    uint8_t start;
    uint8_t channel;
    uint16_t seq;
    uint16_t len;
} FLAPHeader;

#pragma pack()

typedef struct _AIM_CLIENT_APP_CONFIG
{
    int enabled;
} AIM_CLIENT_APP_CONFIG;

static AIM_CLIENT_APP_CONFIG aim_config;

#define MAX_VERSION_SIZE    64

static CLIENT_APP_RETCODE aim_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE aim_validate(const uint8_t *data, uint16_t size, const int dir,
                                       tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                       const struct appIdConfig_ *pConfig);

tRNAClientAppModule aim_client_mod =
{
    .name = "AIM",
    .proto = IPPROTO_TCP,
    .init = &aim_init,
    .validate = &aim_validate,
    .minimum_matches = 2,
    .provides_user = 1,
};

typedef struct
{
    const uint8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static const uint8_t NEW_CONNECTION[] = "\x02a\x001";
static const uint8_t AIM_PROTOCOL_VERSION[] = "\x000\x004\x000\x000\x000\x001";
static const uint8_t OLDER_AOL[] = "AOL Instant Messenger";
static const uint8_t AOL[] = "imApp";
static const uint8_t NETSCAPE_AOL[] = "Netscape 2000 an approved user of AOL Instant Messenger";

static Client_App_Pattern patterns[] =
{
    {NEW_CONNECTION, sizeof(NEW_CONNECTION)-1, 0},
    {AIM_PROTOCOL_VERSION, sizeof(AIM_PROTOCOL_VERSION)-1, 4},
    {OLDER_AOL, sizeof(OLDER_AOL)-1, -1, APP_ID_AOL_INSTANT_MESSENGER},
    {AOL, sizeof(AOL)-1, -1, APP_ID_AOL_INSTANT_MESSENGER},
    {NETSCAPE_AOL, sizeof(NETSCAPE_AOL), -1, APP_ID_AOL_NETSCAPE},
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_AOL_NETSCAPE, APPINFO_FLAG_CLIENT_ADDITIONAL | APPINFO_FLAG_CLIENT_USER},
    {APP_ID_AOL_INSTANT_MESSENGER, APPINFO_FLAG_CLIENT_ADDITIONAL | APPINFO_FLAG_CLIENT_USER},
};

static CLIENT_APP_RETCODE aim_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    aim_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                aim_config.enabled = atoi(item->value);
            }
        }
    }

    if (aim_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering pattern length %u at %d\n",patterns[i].length, patterns[i].index);
            init_api->RegisterPattern(&aim_validate, IPPROTO_TCP, patterns[i].pattern,
                                      patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&aim_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static CLIENT_APP_RETCODE aim_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{

    const uint8_t *end;
    const uint8_t *frame_end;
    uint16_t len;
    FLAPHeader *fh;
    FLAPFNAC *fnac;
    FLAPTLV *tlv;
    uint16_t tlvtype;
    uint16_t tlvlen;
    int check_user_name;

    if (dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    end = data + size;

    while (data < end)
    {
        if (size < sizeof(*fh))
            goto bail;

        fh = (FLAPHeader *)data;
        data += sizeof(*fh);

        if (fh->start != 0x2a || fh->channel < 1 || fh->channel > 5)
            goto bail;

        len = ntohs(fh->len);

        if (len > (end - data))
            goto bail;

        check_user_name = 0;
        if (fh->channel == 0x02)
        {
            if (len < sizeof(*fnac))
                goto bail;
            fnac = (FLAPFNAC *)data;
            if (fnac->family == htons(0x0017) && fnac->subtype == htons(0x0006))
                check_user_name = 1;
            data += sizeof(*fnac);
            len -= sizeof(*fnac);
        }
        else if (fh->channel == 0x01)
        {
            if (len < 4 || memcmp(data, &AIM_PROTOCOL_VERSION[2], 4) != 0)
                goto bail;
            len -= 4;
            data += 4;
        }
        if (len)
        {
            int got_id = 0;
            uint32_t product_id = APP_ID_AOL_INSTANT_MESSENGER;
            uint16_t major = 0;
            uint16_t minor = 0;
            uint16_t lesser = 0;
            char username[256];

            frame_end = data + len;
            while (data < frame_end)
            {
                if ((size_t)(frame_end - data) < sizeof(*tlv))
                    goto bail;
                tlv = (FLAPTLV *)data;
                data += sizeof(*tlv);
                tlvtype = ntohs(tlv->subtype);
                tlvlen = ntohs(tlv->len);
                if (frame_end - data < tlvlen)
                    goto bail;
                switch (tlvtype)
                {
                    case 0x0001:
                        if (check_user_name)
                        {
                            char *u;
                            const char *u_end;
                            const char *p;
                            const char *p_end;
                            p = (const char *)data;
                            p_end = p + tlvlen;
                            u = username;
                            u_end = u + sizeof(username) - 1;
                            for (; p < p_end; p++)
                            {
                                if (isalnum(*p) || *p == '.' || *p == '@' || *p == '-' || *p == '_')
                                {
                                    if (u < u_end)
                                    {
                                        *u = *p;
                                        u++;
                                    }
                                }
                                else
                                {
                                    u = username;
                                    break;
                                }
                            }
                            if (u != username)
                            {
                                *u = 0;
                                aim_client_mod.api->add_user(flowp, username, APP_ID_AOL_INSTANT_MESSENGER, 1);
                            }
                        }
                        break;
                    case 0x0003:
                        got_id = 1;
                        if (tlvlen >= sizeof(AOL)-1 && memcmp(data, AOL, sizeof(AOL)-1) == 0)
                        {
                            product_id = APP_ID_AOL_INSTANT_MESSENGER;
                        }
                        else if (tlvlen >= sizeof(NETSCAPE_AOL)-1 && memcmp(data, NETSCAPE_AOL, sizeof(NETSCAPE_AOL)-1) == 0)
                        {
                            product_id = APP_ID_AOL_INSTANT_MESSENGER;
                        }
                        else if (tlvlen >= sizeof(OLDER_AOL)-1 && memcmp(data, OLDER_AOL, sizeof(OLDER_AOL)-1) == 0 )
                        {
                            product_id = APP_ID_AOL_INSTANT_MESSENGER;
                        }
                        break;
                    case 0x0017:
                        got_id = 1;
                        major = ntohs(*(uint16_t *)data);
                        break;
                    case 0x0018:
                        got_id = 1;
                        minor = ntohs(*(uint16_t *)data);
                        break;
                    case 0x0019:
                        got_id = 1;
                        lesser = ntohs(*(uint16_t *)data);
                        break;
                    default:
                        break;
                }
                data += tlvlen;
            }
            if (got_id)
            {
                char version[MAX_VERSION_SIZE];

                snprintf(version, sizeof(version), "%d.%d.%d", major, minor, lesser);
                aim_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_AOL_INSTANT_MESSENGER, product_id, version);
            }
        }
    }
    return CLIENT_APP_INPROCESS;

bail:
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

