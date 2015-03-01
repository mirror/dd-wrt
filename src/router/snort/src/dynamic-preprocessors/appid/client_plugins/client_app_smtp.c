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
#include "client_app_smtp.h"

typedef enum
{
    SMTP_STATE_HELO,
    SMTP_STATE_MAIL_FROM,
    SMTP_STATE_RCPT_TO,
    SMTP_STATE_DATA,
    SMTP_STATE_MESSAGE,
    SMTP_STATE_CONNECTION_ERROR
} SMTPState;

#define MAX_VERSION_SIZE    64
typedef struct
{
    SMTPState state;
    uint8_t version[MAX_VERSION_SIZE];
} ClientSMTPData;

typedef struct _SMTP_CLIENT_APP_CONFIG
{
    int enabled;
} SMTP_CLIENT_APP_CONFIG;

static SMTP_CLIENT_APP_CONFIG smtp_config;

static CLIENT_APP_RETCODE smtp_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE smtp_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData);

RNAClientAppModule smtp_client_mod =
{
    .name = "SMTP",
    .proto = IPPROTO_TCP,
    .init = &smtp_init,
    .validate = &smtp_validate,
    .minimum_matches = 1
};

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

#define HELO "HELO "
#define EHLO "EHLO "
#define MAILFROM "MAIL FROM:"
#define RCPTTO "RCPT TO:"
#define DATA "DATA"
#define RSET "RSET"

#define MICROSOFT "Microsoft "
#define OUTLOOK "Outlook"
#define EXPRESS "Express "
#define IMO "IMO, "

static const uint8_t APP_SMTP_OUTLOOK[] = "Microsoft Outlook";
static const uint8_t APP_SMTP_OUTLOOK_EXPRESS[] = "Microsoft Outlook Express ";
static const uint8_t APP_SMTP_IMO[] = "IMO, ";
static const uint8_t APP_SMTP_EVOLUTION[] = "Ximian Evolution ";
static const uint8_t APP_SMTP_LOTUSNOTES[] =  "Lotus Notes ";
static const uint8_t APP_SMTP_APPLEMAIL[] =  "Apple Mail (";
static const uint8_t APP_SMTP_EUDORA[] =  "QUALCOMM Windows Eudora Version ";
static const uint8_t APP_SMTP_EUDORAPRO[] =  "Windows Eudora Pro Version ";
static const uint8_t APP_SMTP_AOL[] =  "AOL ";
static const uint8_t APP_SMTP_MUTT[] =  "Mutt/";
static const uint8_t APP_SMTP_KMAIL[] =  "KMail/";
static const uint8_t APP_SMTP_MTHUNDERBIRD[] =  "Mozilla Thunderbird ";
static const uint8_t APP_SMTP_THUNDERBIRD[] =  "Thunderbird ";
static const uint8_t APP_SMTP_MOZILLA[] = "Mozilla";
static const uint8_t APP_SMTP_THUNDERBIRD_SHORT[] = "Thunderbird/";

static Client_App_Pattern patterns[] =
{
    {(uint8_t *)HELO, sizeof(HELO)-1, 0, APP_ID_SMTP},
    {(uint8_t *)EHLO, sizeof(EHLO)-1, 0, APP_ID_SMTP},
	{APP_SMTP_OUTLOOK,         sizeof(APP_SMTP_OUTLOOK)-1,        -1, APP_ID_OUTLOOK},
	{APP_SMTP_OUTLOOK_EXPRESS, sizeof(APP_SMTP_OUTLOOK_EXPRESS)-1,-1, APP_ID_OUTLOOK_EXPRESS},
	{APP_SMTP_IMO,             sizeof(APP_SMTP_IMO)-1,            -1, APP_ID_SMTP_IMO},
	{APP_SMTP_EVOLUTION,       sizeof(APP_SMTP_EVOLUTION)-1,      -1, APP_ID_EVOLUTION},
	{APP_SMTP_LOTUSNOTES,      sizeof(APP_SMTP_LOTUSNOTES)-1,     -1, APP_ID_LOTUS_NOTES},
	{APP_SMTP_APPLEMAIL,       sizeof(APP_SMTP_APPLEMAIL)-1,      -1, APP_ID_APPLE_EMAIL},
	{APP_SMTP_EUDORA,          sizeof(APP_SMTP_EUDORA)-1,         -1, APP_ID_EUDORA},
	{APP_SMTP_EUDORAPRO,       sizeof(APP_SMTP_EUDORAPRO)-1,      -1, APP_ID_EUDORA_PRO},
	{APP_SMTP_AOL,             sizeof(APP_SMTP_AOL)-1,            -1, APP_ID_AOL_EMAIL},
	{APP_SMTP_MUTT,            sizeof(APP_SMTP_MUTT)-1,           -1, APP_ID_MUTT},
	{APP_SMTP_KMAIL,           sizeof(APP_SMTP_KMAIL)-1,          -1, APP_ID_KMAIL},
	{APP_SMTP_MTHUNDERBIRD,    sizeof(APP_SMTP_MTHUNDERBIRD)-1,   -1, APP_ID_THUNDERBIRD},
	{APP_SMTP_THUNDERBIRD,     sizeof(APP_SMTP_THUNDERBIRD)-1,    -1, APP_ID_THUNDERBIRD},
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_THUNDERBIRD, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_OUTLOOK, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_KMAIL, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_EUDORA_PRO, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_EVOLUTION, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_SMTP_IMO, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_EUDORA, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_LOTUS_NOTES, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_APPLE_EMAIL, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_AOL_EMAIL, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_MUTT, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_SMTP, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_OUTLOOK_EXPRESS, APPINFO_FLAG_CLIENT_ADDITIONAL}
};

static CLIENT_APP_RETCODE smtp_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    smtp_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                smtp_config.enabled = atoi(item->value);
            }
        }
    }

    if (smtp_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            init_api->RegisterPattern(&smtp_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&smtp_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, NULL);
	}

    return CLIENT_APP_SUCCESS;
}

static int ExtractVersion(ClientSMTPData * const fd, const uint8_t *product,
                          const uint8_t *data, FLOW *flowp, const SFSnortPacket *pkt)
{
    const u_int8_t *p;
    u_int8_t *v;
    u_int8_t *v_end;
    unsigned len;
    unsigned sublen;

    v_end = fd->version;
    v_end += MAX_VERSION_SIZE - 1;
    len = data - product;
    if (len >= sizeof(MICROSOFT) && memcmp(product, MICROSOFT, sizeof(MICROSOFT)-1) == 0)
    {
        p = product + sizeof(MICROSOFT) - 1;

        if (data-p >= (int)sizeof(OUTLOOK) && memcmp(p, OUTLOOK, sizeof(OUTLOOK)-1) == 0)
        {
            p += sizeof(OUTLOOK) - 1;
            if (p >= data) return 1;
            if (*p == ',')
            {
                p++;
                if (p >= data || *p != ' ') return 1;
                p++;
                if (p >= data || isspace(*p)) return 1;
                for (v=fd->version; v<v_end && p < data; v++,p++)
                {
                    *v = *p;
                }
                *v = 0;
                smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_OUTLOOK, (char *)fd->version);
                return 0;
            }
            else if (*p == ' ')
            {
                p++;
                if (data-p >= (int)sizeof(EXPRESS) && memcmp(p, EXPRESS, sizeof(EXPRESS)-1) == 0)
                {
                    p += sizeof(EXPRESS) - 1;
                    if (p >= data || isspace(*p)) return 1;
                    for (v=fd->version; v<v_end && p < data; v++,p++)
                    {
                        *v = *p;
                    }
                    *v = 0;
                    smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_OUTLOOK_EXPRESS, (char *)fd->version);
                    return 0;
                }
                else if (data-p >= (int)sizeof(IMO) && memcmp(p, IMO, sizeof(IMO)-1) == 0)
                {
                    p += sizeof(IMO) - 1;
                    if (p >= data) return 1;
                    for (v=fd->version; v<v_end && p < data; v++,p++)
                    {
                        *v = *p;
                    }
                    *v = 0;
                    smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_OUTLOOK, (char *)fd->version);
                    return 0;
                }
            }
        }
    }
    else if (len >= sizeof(APP_SMTP_EVOLUTION) && memcmp(product, APP_SMTP_EVOLUTION, sizeof(APP_SMTP_EVOLUTION)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_EVOLUTION) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_EVOLUTION, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_LOTUSNOTES) && memcmp(product, APP_SMTP_LOTUSNOTES, sizeof(APP_SMTP_LOTUSNOTES)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_LOTUSNOTES) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_LOTUS_NOTES, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_APPLEMAIL) && memcmp(product, APP_SMTP_APPLEMAIL, sizeof(APP_SMTP_APPLEMAIL)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_APPLEMAIL) - 1;
        if (p >= data || *(data - 1) != ')' || *p == ')' || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data-1; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_APPLE_EMAIL, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_EUDORA) && memcmp(product, APP_SMTP_EUDORA, sizeof(APP_SMTP_EUDORA)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_EUDORA) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_EUDORA, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_EUDORAPRO) && memcmp(product, APP_SMTP_EUDORAPRO, sizeof(APP_SMTP_EUDORAPRO)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_EUDORAPRO) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_EUDORA_PRO, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_AOL) && memcmp(product, APP_SMTP_AOL, sizeof(APP_SMTP_AOL)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_AOL) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_AOL_EMAIL, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_MUTT) && memcmp(product, APP_SMTP_MUTT, sizeof(APP_SMTP_MUTT)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_MUTT) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_MUTT, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_KMAIL) && memcmp(product, APP_SMTP_KMAIL, sizeof(APP_SMTP_KMAIL)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_KMAIL) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_SMTP/*KMAIL_ID*/, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_THUNDERBIRD) && memcmp(product, APP_SMTP_THUNDERBIRD, sizeof(APP_SMTP_THUNDERBIRD)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_THUNDERBIRD) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_THUNDERBIRD, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_MTHUNDERBIRD) && memcmp(product, APP_SMTP_MTHUNDERBIRD, sizeof(APP_SMTP_MTHUNDERBIRD)-1) == 0)
    {
        p = product + sizeof(APP_SMTP_MTHUNDERBIRD) - 1;
        if (p >= data || isspace(*p)) return 1;
        for (v=fd->version; v<v_end && p < data; v++,p++)
        {
            *v = *p;
        }
        *v = 0;
        smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_THUNDERBIRD, (char *)fd->version);
        return 0;
    }
    else if (len >= sizeof(APP_SMTP_MOZILLA) && memcmp(product, APP_SMTP_MOZILLA, sizeof(APP_SMTP_MOZILLA)-1) == 0)
    {
        for (p = product + sizeof(APP_SMTP_MOZILLA) - 1; p < data; p++)
        {
            if (*p == 'T')
            { 
                sublen = data - p;
                if (sublen >= sizeof(APP_SMTP_THUNDERBIRD_SHORT) && memcmp(p, APP_SMTP_THUNDERBIRD_SHORT, sizeof(APP_SMTP_THUNDERBIRD_SHORT)-1) == 0)
                {
                    p = p + sizeof(APP_SMTP_THUNDERBIRD_SHORT) - 1;
                    for (v=fd->version; v<v_end && p < data; p++)
                    {
                        if (*p == 0x0A || *p == 0x0D || !isprint(*p)) break;
                        *v = *p;
                        v++;
                    }
                    *v = 0;
                    smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_THUNDERBIRD, (char *)fd->version);
                    return 0;
                }  
            }                         
        }
    } 

    return 1;
}

static inline int CheckTag(const uint8_t *tag, const uint8_t *data)
{
#define XMAILER "X-Mailer"
#define USERAGENT "User-Agent"

    unsigned len;

    len = data - tag;

    if (len >= sizeof(XMAILER)-1 && memcmp(tag, XMAILER, sizeof(XMAILER)-1) == 0)
        return 0;
    if (len >= sizeof(USERAGENT)-1 && memcmp(tag, USERAGENT, sizeof(USERAGENT)-1) == 0)
        return 0;
    return 1;
}

static CLIENT_APP_RETCODE smtp_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData)
{
#define FIND_NEWLINE() \
    { \
        while (data < end) \
        { \
            if (*data == 0x0A) break; \
            else if (*data == 0x0D) \
            { \
                data++; \
                if (data >= end || *data != 0x0A) goto done; \
                break; \
            } \
            else if (!isprint(*data)) goto done; \
            data++; \
        } \
        if (data >= end) goto done; \
        data++; \
    }

    ClientSMTPData *fd;
    const uint8_t *end;
    const uint8_t *tag;
    const uint8_t *product;
    int rval;
    unsigned len;

    if (dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    fd = smtp_client_mod.api->data_get(flowp);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (smtp_client_mod.api->data_add(flowp, fd, &free))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        fd->state = SMTP_STATE_HELO;
    }

    end = data + size;

    while (data < end)
    {
        switch (fd->state)
        {
        case SMTP_STATE_HELO:
            len = end - data;
            if (len >= sizeof(HELO) && memcmp(HELO, data, sizeof(HELO)-1) == 0)
            {
                data += sizeof(HELO) - 1;
            }
            else if (len >= sizeof(EHLO) && memcmp(EHLO, data, sizeof(EHLO)-1) == 0)
            {
                data += sizeof(EHLO) - 1;
            }
            else goto done;
            FIND_NEWLINE();
            fd->state = SMTP_STATE_MAIL_FROM;
            break;
        case SMTP_STATE_MAIL_FROM:
            if (end-data > (int)sizeof(MAILFROM) && memcmp(MAILFROM, data, sizeof(MAILFROM)-1) == 0)
            {
                data += sizeof(MAILFROM) - 1;
                FIND_NEWLINE();
                fd->state = SMTP_STATE_RCPT_TO;
            }
            else if (end-data > (int)sizeof(RSET) && memcmp(RSET, data, sizeof(RSET)-1) == 0)
            {
                data += sizeof(RSET) - 1;
                if (*data != 0x0A)
                {
                    if (*data != 0x0D) goto done;
                    data++;
                    if (data >= end || *data != 0x0A) goto done;
                }
                data++;
            }
            else goto done;
            break;
        case SMTP_STATE_RCPT_TO:
            if (end-data < (int)sizeof(RCPTTO) || memcmp(RCPTTO, data, sizeof(RCPTTO)-1) != 0) goto done;
            data += sizeof(RCPTTO) - 1;
            FIND_NEWLINE();
            fd->state = SMTP_STATE_DATA;
            break;
        case SMTP_STATE_DATA:
            if (end-data > (int)sizeof(DATA) && memcmp(DATA, data, sizeof(DATA)-1) == 0)
            {
                data += sizeof(DATA) - 1;
                if (*data != 0x0A)
                {
                    if (*data != 0x0D) goto done;
                    data++;
                    if (data >= end || *data != 0x0A) goto done;
                }
                data++;
                fd->state = SMTP_STATE_MESSAGE;
            }
            else
            {
                if (end-data < (int)sizeof(RCPTTO) || memcmp(RCPTTO, data, sizeof(RCPTTO)-1) != 0) goto done;
                data += sizeof(RCPTTO) - 1;
                FIND_NEWLINE();
            }
            break;
        case SMTP_STATE_MESSAGE:
            if (*data == '.')
            {
                len = end - data;
                if ((len >= 1 && data[1] == 0x0A) ||
                    (len >= 2 && data[1] == 0x0D && data[2] == 0x0A))
                {
                    smtp_client_mod.api->add_app(flowp, APP_ID_SMTP, APP_ID_SMTP, NULL);
                    goto done;
                }
            }
            tag = data;
            while (data < end)
            {
                if (*data == ':')
                {
                    /* Must end with ': ' and have more on the line */
                    if (end-data < 3 || data[1] != ' ' || data[2] == 0x0D || data[2] == 0x0A || !isprint(data[2])) goto done;
                    break;
                }
                else if (*data == 0x0A) break;
                else if (*data == 0x0D)
                {
                    data++;
                    if (data >= end || *data != 0x0A) goto done;
                    break;
                }
                else if (!isprint(*data) && *data != 0x09) goto done;
                data++;
            }
            if (data >= end) goto done;
            if (*data == ':')
            {
                rval = CheckTag(tag, data);
                data += 2;
                if (rval == 0)
                {
                    while (data < end)
                    {
                        if (*data != ' ') break;
                        data++;
                    }
                    if (data >= end) goto done;
                    product = data;
                    while (data < end)
                    {
                        if (*data == 0x0A) break;
                        else if (*data == 0x0D)
                        {
                            if (end - data < 2 || data[1] != 0x0A) goto done;
                            break;
                        }
                        else if (!isprint(*data)) goto done;
                        data++;
                    }
                    if (data >= end) goto done;
                    if (ExtractVersion(fd, product, data, flowp, pkt) == 0) goto done;
                }
                else FIND_NEWLINE();
            }
            else data++;
            break;
        default:
            goto done;
        }
    }
    return CLIENT_APP_INPROCESS;

done:
    flow_mark(flowp, FLOW_CLIENTAPPDETECTED);
    return CLIENT_APP_SUCCESS;
}

