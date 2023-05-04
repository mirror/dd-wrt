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
#include "appIdApi.h"
#include "appInfoTable.h"
//#include "flow.h"
#include "detector_api.h"

//#define  UNIT_TESTING

#ifdef UNIT_TESTING
#include "fw_appid.h"
#endif

typedef enum
{
    SMTP_CLIENT_STATE_NONE,
    SMTP_CLIENT_STATE_HELO,
    SMTP_CLIENT_STATE_MAIL_FROM,
    SMTP_CLIENT_STATE_RCPT_TO,
    SMTP_CLIENT_STATE_DATA,
    SMTP_CLIENT_STATE_MESSAGE,
    SMTP_CLIENT_STATE_GET_PRODUCT_VERSION,
    SMTP_CLIENT_STATE_SKIP_LINE,
    SMTP_CLIENT_STATE_SKIP_SPACE,
    SMTP_CLIENT_STATE_SKIP_EOL,
    SMTP_CLIENT_STATE_CONNECTION_ERROR,
    SMTP_CLIENT_STATE_STARTTLS,
    SMTP_CLIENT_STATE_LOGIN_USER,
    SMTP_CLIENT_STATE_LOGIN_PASSWORD
} SMTPClientState;

#define MAX_HEADER_LINE_SIZE 1024

#ifdef UNIT_TESTING
char *stateName [] =
{
    "SMTP_CLIENT_STATE_NONE",
    "SMTP_CLIENT_STATE_HELO",
    "SMTP_CLIENT_STATE_MAIL_FROM",
    "SMTP_CLIENT_STATE_RCPT_TO",
    "SMTP_CLIENT_STATE_DATA",
    "SMTP_CLIENT_STATE_MESSAGE",
    "SMTP_CLIENT_STATE_GET_PRODUCT_VERSION",
    "SMTP_CLIENT_STATE_SKIP_LINE",
    "SMTP_CLIENT_STATE_CONNECTION_ERROR",
    "SMTP_CLIENT_STATE_STARTTLS"
};
#endif

/* flag values for ClientSMTPData */
#define CLIENT_FLAG_STARTTLS_SUCCESS    0x01

#define MAX_VERSION_SIZE    64
#define SSL_WAIT_PACKETS    8  // This many un-decrypted packets without a HELO and we quit.
typedef struct
{
    int flags;
    SMTPClientState state;
    SMTPClientState nextstate;
    uint8_t version[MAX_VERSION_SIZE];
    unsigned pos;
    uint8_t     *headerline;
    int decryption_countdown;
} ClientSMTPData;

typedef struct _SMTP_CLIENT_APP_CONFIG
{
    int enabled;
} SMTP_CLIENT_APP_CONFIG;

static SMTP_CLIENT_APP_CONFIG smtp_config;

static CLIENT_APP_RETCODE smtp_ca_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE smtp_ca_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt,
                                        struct _Detector *userData, const struct appIdConfig_ *pConfig);

tRNAClientAppModule smtp_client_mod =
{
    .name = "SMTP",
    .proto = IPPROTO_TCP,
    .init = &smtp_ca_init,
    .validate = &smtp_ca_validate,
    .minimum_matches = 1
};

typedef struct {
    const u_int8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

#define HELO "HELO"
#define EHLO "EHLO"
#define MAILFROM "MAIL FROM:"
#define RCPTTO "RCPT TO:"
#define DATA "DATA"
#define RSET "RSET"
#define AUTH "AUTH "
#define AUTH_PLAIN "AUTH PLAIN"
#define AUTH_LOGIN "AUTH LOGIN"
#define STARTTLS "STARTTLS"

#define STARTTLS_COMMAND_SUCCESS "220 "

#define MICROSOFT "Microsoft"
#define OUTLOOK "Outlook"
#define EXPRESS "Express "
#define IMO "IMO, "
#define MAC "Mac"

#define XMAILER "X-Mailer: "
#define USERAGENT "User-Agent: "

static const uint8_t APP_SMTP_OUTLOOK[] = "Microsoft Outlook";
static const uint8_t APP_SMTP_OUTLOOK_MAC[] = "Microsoft-MacOutlook";
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
	{APP_SMTP_OUTLOOK_MAC,     sizeof(APP_SMTP_OUTLOOK_MAC)-1,    -1, APP_ID_OUTLOOK},
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

static tAppRegistryEntry clientAppIdRegistry[] =
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
    {APP_ID_OUTLOOK_EXPRESS, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_SMTPS, APPINFO_FLAG_CLIENT_ADDITIONAL}
};

static CLIENT_APP_RETCODE smtp_ca_init(const InitClientAppAPI * const init_api, SF_LIST *config)
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
            init_api->RegisterPattern(&smtp_ca_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index, init_api->pAppidConfig);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(clientAppIdRegistry)/sizeof(*clientAppIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",clientAppIdRegistry[j].appId);
		init_api->RegisterAppId(&smtp_ca_validate, clientAppIdRegistry[j].appId, clientAppIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return CLIENT_APP_SUCCESS;
}

#define SMTP_PORT   25
#define SMTP_CLOSING_CONN "closing connection\x0d\x0a"

typedef enum
{
    SMTP_SERVICE_STATE_NONE,
    SMTP_SERVICE_STATE_CONNECTION,
    SMTP_SERVICE_STATE_HELO,
    SMTP_SERVICE_STATE_BAD_CLIENT,
    SMTP_SERVICE_STATE_TRANSFER,
    SMTP_SERVICE_STATE_CONNECTION_ERROR,
    SMTP_SERVICE_STATE_STARTTLS,
    SMTP_SERVICE_STATE_SSL_HANDSHAKE
} SMTPServiceState;

typedef struct _SERVICE_SMTP_DATA
{
    SMTPServiceState state;
    int code;
    int multiline;
    int set_flags;
} ServiceSMTPData;

#pragma pack(1)

typedef struct _SERVICE_SMTP_CODE
{
    uint8_t code[3];
    uint8_t sp;
} ServiceSMTPCode;

#pragma pack()

static int smtp_svc_init(const InitServiceAPI * const init_api);
static int smtp_svc_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &smtp_svc_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "smtp",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&smtp_svc_validate, SMTP_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule smtp_service_mod =
{
    .name = "SMTP",
    .init = &smtp_svc_init,
    .pp = pp,
};

#define SMTP_PATTERN1 "220 "
#define SMTP_PATTERN2 "220-"
#define SMTP_PATTERN3 "SMTP"
#define SMTP_PATTERN4 "smtp"

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_SMTP,  APPINFO_FLAG_SERVICE_ADDITIONAL},
    {APP_ID_SMTPS, APPINFO_FLAG_SERVICE_ADDITIONAL}
};

typedef struct _SMTP_DETECTOR_DATA
{
    ClientSMTPData client;
    ServiceSMTPData server;
    int need_continue;
} SMTPDetectorData;

SF_SO_PUBLIC RNADetectorValidationModule smtp_detector_mod =
{
    .service = &smtp_service_mod,
    .client = &smtp_client_mod,
};

static int smtp_svc_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&smtp_svc_validate, IPPROTO_TCP, (uint8_t *)SMTP_PATTERN1, sizeof(SMTP_PATTERN1)-1, 0, "smtp", init_api->pAppidConfig);
    init_api->RegisterPattern(&smtp_svc_validate, IPPROTO_TCP, (uint8_t *)SMTP_PATTERN2, sizeof(SMTP_PATTERN2)-1, 0, "smtp", init_api->pAppidConfig);
    init_api->RegisterPattern(&smtp_svc_validate, IPPROTO_TCP, (uint8_t *)SMTP_PATTERN3, sizeof(SMTP_PATTERN3)-1, -1, "smtp", init_api->pAppidConfig);
    init_api->RegisterPattern(&smtp_svc_validate, IPPROTO_TCP, (uint8_t *)SMTP_PATTERN4, sizeof(SMTP_PATTERN4)-1, -1, "smtp", init_api->pAppidConfig);

	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&smtp_svc_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int ExtractVersion(ClientSMTPData * const fd, const uint8_t *product,
                          const uint8_t *data, tAppIdData *flowp, SFSnortPacket *pkt, const int dir, const tAppIdConfig *pConfig)
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

        if (*p == '-' || isspace(*p)) p++;
        else return 1;

        if (data-p >= (int)sizeof(MAC) && memcmp(p, MAC, sizeof(MAC)-1) == 0)
            p += sizeof(MAC) - 1;

        if (data-p >= (int)sizeof(OUTLOOK) && memcmp(p, OUTLOOK, sizeof(OUTLOOK)-1) == 0)
        {
            p += sizeof(OUTLOOK) - 1;

            if (*p == ',' || *p == '/' || isspace(*p))
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
                    smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_OUTLOOK_EXPRESS, (char *)fd->version);
                    return 0;
                }
                else if (data-p >= (int)sizeof(IMO) && memcmp(p, IMO, sizeof(IMO)-1) == 0)
                {
                    p += sizeof(IMO) - 1;
                    if (p >= data || isspace(*p)) return 1;
                    for (v=fd->version; v<v_end && p < data; v++,p++)
                    {
                        *v = *p;
                    }
                    *v = 0;
                    smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_SMTP_IMO, (char *)fd->version);
                    return 0;
                }
                else
                {
                    if (p >= data || isspace(*p)) return 1;
                    for (v=fd->version; v<v_end && p < data; v++,p++)
                    {
                        *v = *p;
                    }
                    *v = 0;
                    smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_OUTLOOK, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_EVOLUTION, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_LOTUS_NOTES, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_APPLE_EMAIL, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_EUDORA, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_EUDORA_PRO, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_AOL_EMAIL, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_MUTT, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_SMTP/*KMAIL_ID*/, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_THUNDERBIRD, (char *)fd->version);
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
        smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_THUNDERBIRD, (char *)fd->version);
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
                    smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_THUNDERBIRD, (char *)fd->version);
                    return 0;
                }
            }
        }
    }

    return 1;
}
static void smtp_free_state(void *data)
{
    SMTPDetectorData *dd = (SMTPDetectorData *)data;
    ClientSMTPData *cd;

    if (dd)
    {
        cd = &dd->client;
        if (cd->headerline)
            free(cd->headerline);
        free(dd);
    }
}
static inline SMTPDetectorData *smtp_get_SMTPDetectorData(tAppIdData *flowp)
{
    SMTPDetectorData *dd = smtp_detector_mod.api->data_get(flowp, smtp_detector_mod.flow_data_index);
    if (dd)
        return dd;

    if ((dd =  calloc(1, sizeof(*dd))) == NULL)
        return NULL;
    if (smtp_detector_mod.api->data_add(flowp, dd, smtp_detector_mod.flow_data_index, &smtp_free_state))
    {
        free(dd);
        return NULL;
    }
    dd->server.state = SMTP_SERVICE_STATE_CONNECTION;
    dd->client.state = SMTP_CLIENT_STATE_HELO;
    dd->need_continue = 1;
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
    return dd;
}

// #define UNIT_TEST_SKIP
static CLIENT_APP_RETCODE smtp_ca_validate(const uint8_t *data, uint16_t size, const int dir,
                                        tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                        const struct appIdConfig_ *pConfig)
{
    SMTPDetectorData *dd;
    ClientSMTPData *fd;
    const uint8_t *end;
    unsigned len;
    int line_break = 0;
    SMTPServiceState serviceState;
#ifdef UNIT_TESTING
    SMTPClientState currState = SMTP_CLIENT_STATE_NONE;
#endif

#ifdef APP_ID_USES_REASSEMBLED
    smtp_detector_mod.streamAPI->response_flush_stream(pkt);
#endif
    if ((dd = smtp_get_SMTPDetectorData(flowp)) == NULL)
        return CLIENT_APP_ENOMEM;

    if (dir != APP_ID_FROM_INITIATOR)
        return CLIENT_APP_INPROCESS;

    fd = &dd->client;

    if (getAppIdFlag(flowp, APPID_SESSION_ENCRYPTED | APPID_SESSION_DECRYPTED) == APPID_SESSION_ENCRYPTED)
    {
        if ((fd->flags & CLIENT_FLAG_STARTTLS_SUCCESS))
        {
            fd->decryption_countdown--;
            if (!fd->decryption_countdown)
#ifdef UNIT_TEST_SKIP
            if (flowp->session_packet_count == 0)
#endif
            {
                /* Because we can't see any further info without decryption we settle for
                   plain APP_ID_SMTPS instead of perhaps finding data that would make calling
                   ExtractVersion() worthwhile, So set the appid and call it good. */
                smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTPS, APP_ID_SMTPS, NULL);
                goto done;
            }
        }
        return CLIENT_APP_INPROCESS;
    }


    for (end = data + size; data < end; data++)
    {
#ifdef UNIT_TESTING
        if (app_id_debug_session_flag && currState != fd->state)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_APPID, "AppIdDbg %s SMTP client state %s\n", app_id_debug_session, stateName[fd->state]););
            currState = fd->state;
        }
#endif
        len = end - data;
        switch (fd->state)
        {
        case SMTP_CLIENT_STATE_HELO:
            if (len >= (sizeof(HELO)-1) && strncasecmp((const char *)data, HELO, sizeof(HELO)-1) == 0)
            {
                data += (sizeof(HELO)-1)-1;
                fd->nextstate = SMTP_CLIENT_STATE_MAIL_FROM;
                fd->state = SMTP_CLIENT_STATE_SKIP_SPACE;
                fd->flags &= ~ CLIENT_FLAG_STARTTLS_SUCCESS;
            }
            else if (len >= (sizeof(EHLO)-1) && strncasecmp((const char *)data, EHLO, sizeof(EHLO)-1) == 0)
            {
                data += (sizeof(EHLO)-1)-1;
                fd->nextstate = SMTP_CLIENT_STATE_MAIL_FROM;
                fd->state = SMTP_CLIENT_STATE_SKIP_SPACE;
                fd->flags &= ~ CLIENT_FLAG_STARTTLS_SUCCESS;
            }
            else goto done;
            break;

        case SMTP_CLIENT_STATE_MAIL_FROM:
            serviceState = dd->server.state;
            if (len >= (sizeof(MAILFROM)-1) && strncasecmp((const char *)data, MAILFROM, sizeof(MAILFROM)-1) == 0)
            {
                data += (sizeof(MAILFROM)-1)-1;
                fd->nextstate = SMTP_CLIENT_STATE_RCPT_TO;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (len >= (sizeof(RSET)-1) && strncasecmp((const char *)data, RSET, sizeof(RSET)-1) == 0)
            {
                data += (sizeof(RSET)-1)-1;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (len >= (sizeof(AUTH_PLAIN)-1) && strncasecmp((const char *)data, AUTH_PLAIN, sizeof(AUTH_PLAIN)-1) == 0)
            {
                data += (sizeof(AUTH_PLAIN)-1)-1;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (len >= (sizeof(AUTH_LOGIN)-1) && strncasecmp((const char *)data, AUTH_LOGIN, sizeof(AUTH_LOGIN)-1) == 0)
            {
                data += (sizeof(AUTH_LOGIN)-1)-1;
                fd->nextstate = SMTP_CLIENT_STATE_LOGIN_USER;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (len >= (sizeof(AUTH)-1) && strncasecmp((const char *)data, AUTH, sizeof(AUTH)-1) == 0)
            {
                data += (sizeof(AUTH)-1)-1;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (len >= (sizeof(STARTTLS)-1) && strncasecmp((const char *)data, STARTTLS, sizeof(STARTTLS)-1) == 0)
            {
                data += (sizeof(STARTTLS)-1)-1;
                serviceState = dd->server.state = SMTP_SERVICE_STATE_STARTTLS;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            /* check for state reversion */
            else if (len >= (sizeof(HELO)-1) && strncasecmp((const char *)data, HELO, sizeof(HELO)-1) == 0)
            {
                data += (sizeof(HELO)-1)-1;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
                dd->server.state = SMTP_SERVICE_STATE_HELO; // make sure that service side expects the 250
            }
            else if (len >= (sizeof(EHLO)-1) && strncasecmp((const char *)data, EHLO, sizeof(EHLO)-1) == 0)
            {
                data += (sizeof(EHLO)-1)-1;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
                dd->server.state = SMTP_SERVICE_STATE_HELO; // make sure that service side expects the 250
            }
            else goto done;
            if (serviceState == SMTP_SERVICE_STATE_TRANSFER)
            {
                setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
                smtp_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                                  APP_ID_SMTP, NULL, NULL, NULL, NULL);
            }
            break;
        case SMTP_CLIENT_STATE_LOGIN_USER:
            {
                fd->nextstate = SMTP_CLIENT_STATE_LOGIN_PASSWORD;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            break;
        case SMTP_CLIENT_STATE_LOGIN_PASSWORD:
            {
                fd->nextstate = SMTP_CLIENT_STATE_MAIL_FROM;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            break;
        case SMTP_CLIENT_STATE_RCPT_TO:
            if (len >= (sizeof(RCPTTO)-1) && strncasecmp((const char *)data, RCPTTO, sizeof(RCPTTO)-1) == 0)
            {
                data += (sizeof(RCPTTO)-1)-1;
                fd->nextstate = SMTP_CLIENT_STATE_DATA;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else
                goto done;
            break;

        case SMTP_CLIENT_STATE_DATA:
            if (len >= (sizeof(DATA)-1) && strncasecmp((const char *)data, DATA, sizeof(DATA)-1) == 0)
            {
                data += (sizeof(DATA)-1)-1;
                fd->nextstate = SMTP_CLIENT_STATE_MESSAGE;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (len >= (sizeof(RCPTTO)-1) && strncasecmp((const char *)data, RCPTTO, sizeof(RCPTTO)-1) == 0)
            {
                data += (sizeof(RCPTTO)-1)-1;
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            break;
        case SMTP_CLIENT_STATE_MESSAGE:
            if (*data == '.')
            {
                if (len == 0 ||
                    (len >= 1 && data[1] == '\n') ||
                    (len >= 2 && data[1] == '\r' && data[2] == '\n'))
                {
                    smtp_client_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_SMTP, APP_ID_SMTP, NULL);
                    goto done;
                }
            }
            else if (len >= (sizeof(XMAILER)-1) && strncasecmp((const char *)data, XMAILER, sizeof(XMAILER)-1) == 0)
            {
                data += (sizeof(XMAILER)-1)-1;
                fd->state = SMTP_CLIENT_STATE_GET_PRODUCT_VERSION;
            }
            else if (len >= (sizeof(USERAGENT)-1) && strncasecmp((const char *)data, USERAGENT, sizeof(USERAGENT)-1) == 0)
            {
                data += (sizeof(USERAGENT)-1)-1;
                fd->state = SMTP_CLIENT_STATE_GET_PRODUCT_VERSION;
            }
            else if (!isprint(*data) && *data != '\t')
                goto done;
            else
            {
                fd->nextstate = fd->state;
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            break;

        case SMTP_CLIENT_STATE_GET_PRODUCT_VERSION:
            if (!fd->headerline)
            {
                if (!(fd->headerline = malloc(MAX_HEADER_LINE_SIZE)))
                    goto done;
            }
            while((data < end) && (fd->pos < (MAX_HEADER_LINE_SIZE-1)))
            {
                if ((*data == ' ') || (*data == '\t'))
                {
                    line_break = 0;
                    fd->headerline[fd->pos++] = *data;
                }
                else if((*data == '\n') && (line_break != 1))
                {
                    /* Can't have multiple LFs in a row, but if we get one it
                     * needs to be followed by at least one space */
                    line_break = 1;
                }
                else if(*data == '\r')
                {
                    // CR needs to be followed by LF and can't start a line
                    line_break = 2;
                }
                else if (!isprint(*data))
                {
                    free(fd->headerline);
                    fd->headerline = NULL;
                    fd->pos = 0;
                    goto done;
                }
                else if(!line_break)
                {
                    fd->headerline[fd->pos++] = *data;
                }
                else
                {
                     // We have reached the end of the header
                     break;
                }
                data++;
            }
            data--;
            if (line_break || fd->pos >= (MAX_HEADER_LINE_SIZE-1))
            {
                ExtractVersion(fd, fd->headerline, fd->headerline + fd->pos, flowp, pkt, dir, pConfig);
                free(fd->headerline);
                fd->headerline = NULL;
                fd->pos = 0;
                goto done;
            }
            break;

        case SMTP_CLIENT_STATE_SKIP_SPACE:
            if (*data == ' ')
            {
                fd->state = SMTP_CLIENT_STATE_SKIP_LINE;
            }
            else if (*data == '\n')
            {
                fd->pos = 0;
                fd->state = fd->nextstate;
                fd->nextstate = SMTP_CLIENT_STATE_NONE;
            }
            else if (*data == '\r')
                fd->state = SMTP_CLIENT_STATE_SKIP_EOL;
            else
                goto done;
            break;

        case SMTP_CLIENT_STATE_SKIP_EOL:
            if (*data == '\n')
            {
                fd->pos = 0;
                fd->state = fd->nextstate;
                fd->nextstate = SMTP_CLIENT_STATE_NONE;
            }
            else
                goto done;
            break;

        case SMTP_CLIENT_STATE_SKIP_LINE:
            if (*data == '\n')
            {
                fd->pos = 0;
                fd->state = fd->nextstate;
                fd->nextstate = SMTP_CLIENT_STATE_NONE;
            }
            else if (!(*data == '\r' || isprint(*data)))
                goto done;
            break;

        default:
            goto done;
        }
    }
    return CLIENT_APP_INPROCESS;

done:
    dd->need_continue = 0;
    if (getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE | APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
    else
        clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    return CLIENT_APP_SUCCESS;
}

static inline int smtp_validate_reply(const uint8_t *data, uint16_t *offset,
                               uint16_t size, int *multi, int *code)
{
    const ServiceSMTPCode *code_hdr;
    int tmp;

    /* Trim any blank lines (be a little tolerant) */
    for (; *offset<size; (*offset)++)
    {
        if (data[*offset] != 0x0D && data[*offset] != 0x0A) break;
    }

    if (size - *offset < (int)sizeof(ServiceSMTPCode))
    {
        for (; *offset<size; (*offset)++)
        {
            if (!isspace(data[*offset])) return -1;
        }
        return 0;
    }

    code_hdr = (ServiceSMTPCode *)(data + *offset);

    if (code_hdr->code[0] < '1' || code_hdr->code[0] > '5') return -1;
    tmp = (code_hdr->code[0] - '0') * 100;

    if (code_hdr->code[1] < '0' || code_hdr->code[1] > '5') return -1;
    tmp += (code_hdr->code[1] - '0') * 10;

    if (!isdigit(code_hdr->code[2])) return -1;
    tmp += code_hdr->code[2] - '0';

    if (*multi && tmp != *code) return -1;
    *code = tmp;
    if (code_hdr->sp == '-') *multi = 1;
    else if (code_hdr->sp == ' ') *multi = 0;
    else return -1;

    /* We have a valid code, now we need to see if the rest of the line
        is okay */

    *offset += sizeof(ServiceSMTPCode);
    for (; *offset < size; (*offset)++)
    {
        if (data[*offset] == 0x0D)
        {
            (*offset)++;
            if (*offset >= size) return -1;
            if (data[*offset] != 0x0A) return -1;
        }
        if (data[*offset] == 0x0A)
        {
            if (*multi)
            {
                if ((*offset + 1) >= size) return 0;

                if (size - (*offset + 1) < (int)sizeof(ServiceSMTPCode)) return -1;

                code_hdr = (ServiceSMTPCode *)(data + *offset + 1);

                if (code_hdr->code[0] < '1' || code_hdr->code[0] > '5')
                    return -1;
                tmp = (code_hdr->code[0] - '0') * 100;

                if (code_hdr->code[1] < '1' || code_hdr->code[1] > '5')
                    return -1;
                tmp += (code_hdr->code[1] - '0') * 10;

                if (!isdigit(code_hdr->code[2])) return -1;
                tmp += code_hdr->code[2] - '0';

                if (tmp != *code) return -1;

                if (code_hdr->sp == ' ') *multi = 0;
                else if (code_hdr->sp != '-') return -1;

                *offset += sizeof(ServiceSMTPCode);
            }
            else
            {
                (*offset)++;
                return *code;
            }
        }
        else if (!isprint(data[*offset])) return -1;
    }


    return 0;
}

static int smtp_svc_validate(ServiceValidationArgs* args)
{
    SMTPDetectorData *dd;
    ServiceSMTPData *fd;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;
    uint16_t offset;

#ifdef APP_ID_USES_REASSEMBLED
    pop3_detector_mod.streamAPI->response_flush_stream(pkt);
#endif

    if ((dd = smtp_get_SMTPDetectorData(flowp)) == NULL)
        return SERVICE_ENOMEM;

    if (!size)
        goto inprocess;

    if (getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
    {
        if (!dd->need_continue)
            clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_SUCCESS; // client made the decision so we are totally done
    }

    fd = &dd->server;

    if (args->dir != APP_ID_FROM_RESPONDER)
    {
        if (SMTP_SERVICE_STATE_HELO == fd->state)
        {
            if (!((size >= (sizeof(HELO)-1) && strncasecmp((const char *)data, HELO, sizeof(HELO)-1) == 0) ||
                  (size >= (sizeof(EHLO)-1) && strncasecmp((const char *)data, EHLO, sizeof(EHLO)-1) == 0)))
            {
                fd->state = SMTP_SERVICE_STATE_BAD_CLIENT;
            }
        }
        goto inprocess;
    }

    offset = 0;
    while (offset < size)
    {
        if (smtp_validate_reply(data, &offset, size, &fd->multiline, &fd->code) < 0)
        {
            if (!(dd->client.flags & CLIENT_FLAG_STARTTLS_SUCCESS))
                goto fail;
            goto inprocess;
        }
        if (!fd->code) goto inprocess;
        switch (fd->state)
        {
        case SMTP_SERVICE_STATE_CONNECTION:
            switch (fd->code)
            {
            case 220:
                fd->state = SMTP_SERVICE_STATE_HELO;
                break;
            case 421:
                if (service_strstr(data, size, (const uint8_t *)SMTP_CLOSING_CONN, sizeof(SMTP_CLOSING_CONN)-1))
                    goto success;
                goto fail;
            case 554:
                goto success;
            default:
                goto fail;
            }
            break;
        case SMTP_SERVICE_STATE_HELO:
            switch (fd->code)
            {
            case 250:
                fd->state = SMTP_SERVICE_STATE_TRANSFER;
                break;
            case 220:
            case 500:
            case 501:
            case 502:
            case 504:
                break;
            case 421:
            case 553:
                fd->state = SMTP_SERVICE_STATE_CONNECTION_ERROR;
                break;
            default:
                goto fail;
            }
            break;
        case SMTP_SERVICE_STATE_STARTTLS:
            // success or fail, return client to connection-complete state.
            dd->client.state = SMTP_CLIENT_STATE_HELO;
            fd->state = SMTP_SERVICE_STATE_HELO;
            if (fd->code == 220)
            {
                dd->client.flags |= CLIENT_FLAG_STARTTLS_SUCCESS;
                if (_dpd.isSSLPolicyEnabled(NULL))
                    dd->client.decryption_countdown = SSL_WAIT_PACKETS; // max wait if decryption fails (e.g., cert error)
                else
                    dd->client.decryption_countdown = 1; // no wait for decryption
                smtp_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                                  APP_ID_SMTPS, NULL, NULL, NULL, NULL);
                if (dd->need_continue > 0)
                    setAppIdFlag(flowp, APPID_SESSION_ENCRYPTED | APPID_SESSION_STICKY_SERVICE | APPID_SESSION_CONTINUE);
                else
                    setAppIdFlag(flowp, APPID_SESSION_ENCRYPTED | APPID_SESSION_STICKY_SERVICE);
                return SERVICE_SUCCESS;
            }
            /* STARTTLS failed. Fall through and call this SMTP */
        case SMTP_SERVICE_STATE_TRANSFER:
            goto success;
        case SMTP_SERVICE_STATE_BAD_CLIENT:
            switch (fd->code)
            {
            case 500:
            case 501:
            case 502:
            case 550:
                goto not_compatible;
            }
        case SMTP_SERVICE_STATE_CONNECTION_ERROR:
        default:
            goto fail;
        }
    }

inprocess:
    smtp_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;

success:
    if (dd->need_continue > 0)
        setAppIdFlag(flowp, APPID_SESSION_CONTINUE);

    smtp_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                      APP_ID_SMTP, NULL, NULL, NULL, NULL);
    return SERVICE_SUCCESS;

fail:
    smtp_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                       smtp_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOMATCH;

not_compatible:
    smtp_service_mod.api->incompatible_data(flowp, args->pkt, args->dir, &svc_element,
                                            smtp_service_mod.flow_data_index, args->pConfig, NULL);
    return SERVICE_NOT_COMPATIBLE;
}
