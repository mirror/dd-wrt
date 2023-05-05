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
#include <commonAppMatcher.h>

static const char SSH_CLIENT_BANNER[] = "SSH-";
#define SSH_CLIENT_BANNER_LEN (sizeof(SSH_CLIENT_BANNER)-1)
#define SSH_CLIENT_BANNER_MAXPOS (sizeof(SSH_CLIENT_BANNER)-2)

static const char DROPBEAR_BANNER[] = "dropbear";
#define DROPBEAR_BANNER_MAXPOS (sizeof(DROPBEAR_BANNER)-2)

static const char LSH_BANNER[] = "lsh";
#define LSH_BANNER_MAXPOS (sizeof(LSH_BANNER)-2)

static const char OPENSSH_BANNER[] = "OpenSSH";
#define OPENSSH_BANNER_MAXPOS (sizeof(OPENSSH_BANNER)-2)

static const char PUTTY_BANNER[] = "PuTTY";
#define PUTTY_BANNER_MAXPOS (sizeof(PUTTY_BANNER)-2)

#define SSH_MSG_KEYXINIT            20
#define SSH_MSG_IGNORE              2
#define SSH_MSG_SESSION_KEY         3
#define SSH_MAX_BANNER_LEN          255
#define SSH2                        2
#define SSH1                        1

typedef enum
{
    SSH_CLIENT_STATE_BANNER = 0,
    SSH_CLIENT_STATE_ID_PROTO_VERSION,
    SSH_CLIENT_STATE_LOOKING_FOR_DASH,
    SSH_CLIENT_STATE_ID_CLIENT,
    SSH_CLIENT_STATE_CHECK_OPENSSH,
    SSH_CLIENT_STATE_CHECK_PUTTY,
    SSH_CLIENT_STATE_CHECK_LSH,
    SSH_CLIENT_STATE_CHECK_DROPBEAR,
    SSH_CLIENT_STATE_ID_SOFTWARE_VERSION,
    SSH_CLIENT_STATE_ID_REST_OF_LINE,
    SSH_CLIENT_STATE_KEY
} SSHClientState;

typedef enum
{
    SSH2_HEADER_BEGIN,
    SSH2_HEADER_PLEN,
    SSH2_HEADER_CODE,
    SSH2_IGNORE,
    SSH2_PADDING,
    SSH2_KEYX_HEADER_FINISH,
    SSH2_FIELD_LEN_BEGIN,
    SSH2_FIELD_DATA_BEGIN,
    SSH2_PAYLOAD_BEGIN
} SSH2HeaderState;

typedef enum
{
    SSH1_HEADER_BEGIN,
    SSH1_HEADER_PLEN,
    SSH1_HEADER_FIND_CODE,
    SSH1_HEADER_CODE,
    SSH1_SESSION_KEY
} SSH1HeaderState;

typedef struct _CLIENT_SSH_DATA
{
    SSHClientState state;
    SSH2HeaderState hstate;
    SSH1HeaderState oldhstate;
    unsigned len;
    unsigned pos;
    unsigned field;
    unsigned field_len;
    unsigned read_data;
    union
    {
        uint32_t len;
        uint8_t raw_len[4];
    } l;
    unsigned ssh_version;
    uint8_t version[SSH_MAX_BANNER_LEN];
    uint8_t plen;
    uint8_t code;
    uint32_t client_id;
} ClientSSHData;

#pragma pack(1)

typedef struct _CLIENT_SSH_KEY_STRING
{
    uint32_t len;
    uint8_t data;
} ClientSSHKeyString;

typedef struct _CLIENT_SSH_MSG
{
    uint32_t len;
    uint8_t plen;
    uint8_t code;
} ClientSSHMsg;

typedef struct _CLIENT_SSH2_KEY_EXCHANGE
{
    ClientSSHMsg msg;
    uint8_t cookie[16];
} ClientSSH2KeyExchange;

typedef struct _CLIENT_SSH1_KEY_EXCHANGE_V1
{
    uint32_t len;
    uint8_t code;
} ClientSSH1KeyExchangeV1;

typedef struct _CLIENT_SSH_KEY_EXCHANGE_FINAL
{
    uint8_t kex_pkt;
    uint32_t future;
} ClientSSHKeyExchangeFinal;

#pragma pack()

typedef struct _SSH_CLIENT_CONFIG
{
    int enabled;
} SSH_CLIENT_CONFIG;

static SSH_CLIENT_CONFIG ssh_client_config;

static CLIENT_APP_RETCODE ssh_client_init(const InitClientAppAPI * const init_api,
                                          SF_LIST *config);
static CLIENT_APP_RETCODE ssh_client_validate(const uint8_t *data, uint16_t size,
                                              const int dir, tAppIdData *flowp,
                                              SFSnortPacket *pkt, struct _Detector *userData,
                                              const struct appIdConfig_ *pConfig);

SF_SO_PUBLIC tRNAClientAppModule ssh_client_mod =
{
    .name = "SSH",
    .proto = IPPROTO_TCP,
    .init = &ssh_client_init,
    .validate = &ssh_client_validate,
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
    {(const uint8_t *)SSH_CLIENT_BANNER, sizeof(SSH_CLIENT_BANNER)-1, 0, APP_ID_SSH},
    {(const uint8_t *)OPENSSH_BANNER,    sizeof(OPENSSH_BANNER)-1,    0, APP_ID_OPENSSH},
    {(const uint8_t *)PUTTY_BANNER,      sizeof(PUTTY_BANNER)-1,      0, APP_ID_PUTTY},
    {(const uint8_t *)LSH_BANNER,        sizeof(LSH_BANNER)-1,        0, APP_ID_LSH},
    {(const uint8_t *)DROPBEAR_BANNER,   sizeof(DROPBEAR_BANNER)-1,   0, APP_ID_DROPBEAR},
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_DROPBEAR, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_SSH, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_LSH, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_PUTTY, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_OPENSSH, APPINFO_FLAG_CLIENT_ADDITIONAL}
};

static CLIENT_APP_RETCODE ssh_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    ssh_client_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                ssh_client_config.enabled = atoi(item->value);
            }
        }
    }

    if (ssh_client_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG, "registering patterns: %s: %d",
                    (const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&ssh_client_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }


	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&ssh_client_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

static inline CLIENT_APP_RETCODE ssh_client_validate_keyx(uint16_t offset, const uint8_t *data,
                                                    uint16_t size, ClientSSHData *fd)
{
    const ClientSSHMsg *ckx;
    const ClientSSHKeyString *cks;
    const ClientSSH2KeyExchange *ckex;

    while (offset < size)
    {
        switch (fd->hstate)
        {
        case SSH2_HEADER_BEGIN:
            fd->l.raw_len[fd->pos] = data[offset];
            fd->pos++;
            if(fd->pos == sizeof(ckx->len))
            {
                fd->len = ntohl(fd->l.len) ;
                fd->hstate = SSH2_HEADER_PLEN;
            }
            break;
        case SSH2_HEADER_PLEN:
            fd->plen = data[offset];
            fd->hstate = SSH2_HEADER_CODE;
            fd->pos++;
            break;
        case SSH2_HEADER_CODE:
            fd->code = data[offset];
            if (fd->code == SSH_MSG_KEYXINIT)
            {
                fd->pos = 0;
                fd->hstate = SSH2_KEYX_HEADER_FINISH;
                fd->read_data = fd->plen + sizeof(ckex->cookie) + sizeof(ckx->len);
            }
            else if (fd->code == SSH_MSG_IGNORE)
            {
                fd->pos = sizeof(ckx->len) + 2;
                fd->hstate = SSH2_IGNORE;
            }
            else
                return CLIENT_APP_EINVALID;
            fd->len = ntohl(fd->l.len) + sizeof(ckx->len);
            if (fd->len > 35000)
                return CLIENT_APP_EINVALID;
            break;
        case SSH2_IGNORE:
            fd->pos++;
            if (fd->pos >= fd->len)
            {
                fd->hstate = SSH2_HEADER_BEGIN;
                fd->pos = 0;
            }
            break;
        case SSH2_KEYX_HEADER_FINISH:
            fd->pos++;
            if (fd->pos >= sizeof(ckex->cookie))
            {
                fd->hstate = SSH2_FIELD_LEN_BEGIN;
                fd->pos = 0;
            }
            break;
        case SSH2_FIELD_LEN_BEGIN:
            fd->l.raw_len[fd->pos] = data[offset];
            fd->pos++;
            if (fd->pos >= sizeof(cks->len))
            {
                fd->pos = 0;
                fd->field_len = ntohl(fd->l.len);
                fd->read_data += fd->field_len + sizeof(cks->len);
                if (fd->read_data > fd->len)
                    return CLIENT_APP_EINVALID;
                if (fd->field_len)
                    fd->hstate = SSH2_FIELD_DATA_BEGIN;
                else
                {
                    fd->field++;
                    if (fd->field >= 10)
                        fd->hstate = SSH2_PAYLOAD_BEGIN;
                }
            }
            break;
        case SSH2_FIELD_DATA_BEGIN:
            fd->pos++;
            if (fd->pos >= fd->field_len)
            {
                fd->field++;
                if (fd->field >= 10)
                    fd->hstate = SSH2_PAYLOAD_BEGIN;
                else
                    fd->hstate = SSH2_FIELD_LEN_BEGIN;
                fd->pos = 0;
            }
            break;
        case SSH2_PAYLOAD_BEGIN:
            if (fd->pos >= offsetof(ClientSSHKeyExchangeFinal, future))
            {
                fd->l.raw_len[fd->pos - offsetof(ClientSSHKeyExchangeFinal, future)] = data[offset];
            }
            fd->pos++;
            if (fd->pos >= sizeof(ClientSSHKeyExchangeFinal))
            {
                if (fd->l.len != 0)
                    return CLIENT_APP_EINVALID;
                fd->hstate = SSH2_PADDING;
                fd->pos = 0;
            }
            break;
        case SSH2_PADDING:
            fd->pos++;
            if (fd->pos >= fd->plen)
            {
                offset++;
                if (offset == size)
                    return CLIENT_APP_SUCCESS;
                return CLIENT_APP_EINVALID;
            }
            break;
        }
        offset++;
    }
    return CLIENT_APP_INPROCESS;
}

static inline CLIENT_APP_RETCODE ssh_client_validate_pubkey(uint16_t offset, const uint8_t *data,
                                                    uint16_t size, ClientSSHData *fd)
{
    const ClientSSHMsg *ckx;

    while (offset < size)
    {
        switch (fd->oldhstate)
        {
        case SSH1_HEADER_BEGIN:
            fd->l.raw_len[fd->pos] = data[offset];
            fd->pos++;
            if(fd->pos == sizeof(ckx->len))
            {
                fd->len = ntohl(fd->l.len) ;
                fd->oldhstate = SSH1_HEADER_PLEN;
            }
            break;
        case SSH1_HEADER_PLEN:
            if(size > (fd->len + sizeof(ckx->len)))
                fd->plen = size - (fd->len + sizeof(ckx->len)) ;
            else
                fd->plen = 0;
            fd->oldhstate = SSH1_HEADER_FIND_CODE;
        case SSH1_HEADER_FIND_CODE:
            if (fd->pos == fd->plen + sizeof(ckx->len))
            {
                fd->oldhstate = SSH1_HEADER_CODE;
                fd->code = data[offset];
            }
            fd->pos++;
            break;
        case SSH1_HEADER_CODE:
            if (fd->code == SSH_MSG_SESSION_KEY)
            {
                fd->oldhstate = SSH1_SESSION_KEY;
                fd->pos++;
            }
            else
                return CLIENT_APP_EINVALID;
            fd->len = fd->len + fd->plen + sizeof(ckx->len);
            if (fd->len > 35000)
                return CLIENT_APP_EINVALID;
            break;
        case SSH1_SESSION_KEY:
            fd->pos++;
            if (fd->pos >= fd->len)
            {
                offset++;
                if(offset == size)
                    return CLIENT_APP_SUCCESS;
                return CLIENT_APP_EINVALID;
            }
            break;
        }
        offset++;
    }
    return CLIENT_APP_INPROCESS;
}

static inline CLIENT_APP_RETCODE ssh_client_sm(const uint8_t *data, uint16_t size, ClientSSHData *fd)
{
    uint16_t offset = 0;
    uint8_t d;

    while(offset < size)
    {
        d = data[offset];
        switch (fd->state)
        {

        case SSH_CLIENT_STATE_BANNER:
            if (d != SSH_CLIENT_BANNER[fd->pos])
                return CLIENT_APP_EINVALID;
            if (fd->pos >= SSH_CLIENT_BANNER_MAXPOS)
                fd->state = SSH_CLIENT_STATE_ID_PROTO_VERSION;
            else
                fd->pos++;
            break;

        case SSH_CLIENT_STATE_ID_PROTO_VERSION:
            if (d == '1')
                fd->ssh_version = SSH1;
            else if (d == '2')
                fd->ssh_version = SSH2;
            else
                return CLIENT_APP_EINVALID;
            fd->state = SSH_CLIENT_STATE_LOOKING_FOR_DASH;
            break;

        case SSH_CLIENT_STATE_LOOKING_FOR_DASH:
            if (d == '-')
            {
                fd->state = SSH_CLIENT_STATE_ID_CLIENT;
                break;
            }
            break;

        case SSH_CLIENT_STATE_ID_CLIENT:
            switch (d)
            {
            case 'O':
                fd->state = SSH_CLIENT_STATE_CHECK_OPENSSH;
                break;
            case 'P':
                fd->state = SSH_CLIENT_STATE_CHECK_PUTTY;
                break;
            case 'l':
                fd->state = SSH_CLIENT_STATE_CHECK_LSH;
                break;
            case 'd':
                fd->state = SSH_CLIENT_STATE_CHECK_DROPBEAR;
                break;
            default:
                fd->state = SSH_CLIENT_STATE_ID_REST_OF_LINE;
                fd->client_id = APP_ID_SSH;
            }
            /*the next thing we want to see is the SECOND character... */
            fd->pos = 1;
            break;

        case SSH_CLIENT_STATE_CHECK_OPENSSH:
            if (d != OPENSSH_BANNER[fd->pos])
            {
                fd->client_id = APP_ID_SSH;
                fd->state = SSH_CLIENT_STATE_ID_REST_OF_LINE;
            }
            else if (fd->pos >= OPENSSH_BANNER_MAXPOS)
            {
                fd->client_id = APP_ID_OPENSSH;
                fd->state = SSH_CLIENT_STATE_ID_SOFTWARE_VERSION;
                fd->pos = 0;
            }
            else fd->pos++;
            break;

        case SSH_CLIENT_STATE_CHECK_PUTTY:
            if (d != PUTTY_BANNER[fd->pos])
            {
                fd->client_id = APP_ID_SSH;
                fd->state = SSH_CLIENT_STATE_ID_REST_OF_LINE;
            }
            else if (fd->pos >= PUTTY_BANNER_MAXPOS)
            {
                fd->client_id = APP_ID_PUTTY;
                fd->state = SSH_CLIENT_STATE_ID_SOFTWARE_VERSION;
                fd->pos = 0;
            }
            else fd->pos++;
            break;

        case SSH_CLIENT_STATE_CHECK_LSH:
            if (d != LSH_BANNER[fd->pos])
            {
                fd->client_id = APP_ID_SSH;
                fd->state = SSH_CLIENT_STATE_ID_REST_OF_LINE;
            }
            else if (fd->pos >= LSH_BANNER_MAXPOS)
            {
                fd->client_id = APP_ID_LSH;
                fd->state = SSH_CLIENT_STATE_ID_SOFTWARE_VERSION;
                fd->pos = 0;
            }
            else fd->pos++;
            break;

        case SSH_CLIENT_STATE_CHECK_DROPBEAR:
            if (d != DROPBEAR_BANNER[fd->pos])
            {
                fd->client_id = APP_ID_SSH;
                fd->state = SSH_CLIENT_STATE_ID_REST_OF_LINE;
            }
            else if (fd->pos >= DROPBEAR_BANNER_MAXPOS)
            {
                fd->client_id = APP_ID_DROPBEAR;
                fd->state = SSH_CLIENT_STATE_ID_SOFTWARE_VERSION;
                fd->pos = 0;
            }
            else fd->pos++;
            break;

        case SSH_CLIENT_STATE_ID_SOFTWARE_VERSION:
            if (d == '\n')
            {
                fd->version[fd->pos] = 0;
                fd->pos = 0;
                fd->state = SSH_CLIENT_STATE_KEY;
                break;
            }
            if (d == ' ')
            {
                fd->version[fd->pos] = 0;
                fd->state = SSH_CLIENT_STATE_ID_REST_OF_LINE;
                break;
            }
            if (fd->pos < SSH_MAX_BANNER_LEN - 1 && d != '\r' && d != '-' && d != '_')
            {
                fd->version[fd->pos++] = d;
            }
            break;

        case SSH_CLIENT_STATE_ID_REST_OF_LINE:
            if (d == '\n')
            {
                fd->pos = 0;
                fd->state = SSH_CLIENT_STATE_KEY;
                break;
            }
            break;

        case SSH_CLIENT_STATE_KEY:
            switch (fd->ssh_version)
            {
            case SSH2:
                return ssh_client_validate_keyx(offset, data, size, fd);
            case SSH1:
                return ssh_client_validate_pubkey(offset, data, size, fd);
            default:
                return CLIENT_APP_EINVALID;
            }

        default:
            return CLIENT_APP_EINVALID;
        }
        offset++;
    }
    return CLIENT_APP_INPROCESS;
}

static CLIENT_APP_RETCODE ssh_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
    ClientSSHData *fd;
    CLIENT_APP_RETCODE sm_ret;

    if (!size || dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    fd = ssh_client_mod.api->data_get(flowp, ssh_client_mod.flow_data_index);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (ssh_client_mod.api->data_add(flowp, fd, ssh_client_mod.flow_data_index, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->state = SSH_CLIENT_STATE_BANNER;
        fd->hstate = SSH2_HEADER_BEGIN;
        fd->oldhstate = SSH1_HEADER_BEGIN;
    }

    sm_ret = ssh_client_sm(data, size, fd);
    if (sm_ret != CLIENT_APP_SUCCESS)
        return sm_ret;

    ssh_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SSH, fd->client_id, (const char *)fd->version);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}
