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
#include "str_search.h"
#include "appIdApi.h"
#include "appInfoTable.h"
#include "detector_api.h"
#include "appIdConfig.h"

/*#define DEBUG_IMAP_DETECTOR  1 */

#define IMAP_USER_NAME_MAX_LEN  32
#define IMAP_TAG_MAX_LEN         6

#define OK_LOGIN " LOGIN Ok."
#define NO_LOGIN " Login failed."

typedef struct _CLIENT_APP_CONFIG
{
    int enabled;
} CLIENT_APP_CONFIG;

typedef enum
{
    IMAP_CLIENT_STATE_NON_AUTH,         // IMAP - Non-Authenticated state
    IMAP_CLIENT_STATE_AUTH,             // IMAP - Authenticated state
    IMAP_CLIENT_STATE_AUTHENTICATE_CMD, // IMAP - authentication-in-progress state
    IMAP_CLIENT_STATE_STARTTLS_CMD,     // IMAP - authentication-in-progress state (probable IMAPS)
} IMAPClientState;

typedef struct _CLIENT_APP_DATA
{
    IMAPClientState state;
    unsigned count;
    int detected;
    int got_user;
    int auth;
    int set_flags;
    char username[IMAP_USER_NAME_MAX_LEN+1];
    char imapCmdTag[IMAP_TAG_MAX_LEN+1];
} ClientAppData;

#define MIN_CMDS    3

static CLIENT_APP_CONFIG ca_config;


static CLIENT_APP_RETCODE init(const InitClientAppAPI * const init_api, SF_LIST *config);
static void clean(const CleanClientAppAPI * const clean_api);
static CLIENT_APP_RETCODE validate(const uint8_t *data, uint16_t size, const int dir,
                                   tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                   const tAppIdConfig *pConfig);

static tRNAClientAppModule client_app_mod =
{
    .name = "IMAP",
    .proto = IPPROTO_TCP,
    .init = &init,
    .clean = &clean,
    .validate = &validate,
    .minimum_matches = 1,
    .provides_user = 1,
};

typedef struct {
    const uint8_t *pattern;
    unsigned length;
    int eoc;
} Client_App_Pattern;

static const uint8_t CAPA[] = "CAPABILITY\x00d\x00a";
static const uint8_t CAPA2[] = "CAPABILITY\x00a";
static const uint8_t NOOP[] = "NOOP\x00d\x00a";
static const uint8_t NOOP2[] = "NOOP\x00a";
static const uint8_t LOGOUT[] = "LOGOUT\x00d\x00a";
static const uint8_t LOGOUT2[] = "LOGOUT\x00a";
static const uint8_t AUTHENTICATE[] = "AUTHENTICATE ";
static const uint8_t LOGIN[] = "LOGIN ";
static const uint8_t SELECT[] = "SELECT ";
/*static const uint8_t EXAMINE[] = "EXAMINE "; */
static const uint8_t CREATE[] = "CREATE ";
static const uint8_t DELETE[] = "DELETE ";
static const uint8_t RENAME[] = "RENAME ";
static const uint8_t SUBSCRIBE[] = "SUBSCRIBE ";
static const uint8_t UNSUBSCRIBE[] = "UNSUBSCRIBE ";
static const uint8_t LISTC[] = "LIST ";
static const uint8_t LSUB[] = "LSUB ";
static const uint8_t APPEND[] = "APPEND ";
static const uint8_t CHECK[] = "CHECK\x00d\x00a";
static const uint8_t CHECK2[] = "CHECK\x00a";
static const uint8_t CLOSE[] = "CLOSE\x00d\x00a";
static const uint8_t CLOSE2[] = "CLOSE\x00a";
static const uint8_t EXPUNGE[] = "EXPUNGE\x00d\x00a";
static const uint8_t EXPUNGE2[] = "EXPUNGE\x00a";
static const uint8_t SEARCH[] = "SEARCH ";
static const uint8_t FETCH[] = "FETCH ";
static const uint8_t PARTIAL[] = "PARTIAL ";
static const uint8_t STORE[] = "STORE ";
static const uint8_t COPY[] = "COPY ";
static const uint8_t UID[] = "UID ";
static const uint8_t STARTTLS[] = "STARTTLS\x00d\x00a";
static const uint8_t STARTTLS2[] = "STARTTLS\x00a";

typedef enum
{
    /* order MUST correspond to that in the array, patterns[], below */
    PATTERN_LOGIN,
    PATTERN_AUTHENTICATE,
    PATTERN_STARTTLS,
    PATTERN_STARTTLS2,
    PATTERN_IMAP_OTHER // always last
} Client_App_Pattern_Index;

static Client_App_Pattern patterns[] =
{
    {LOGIN, sizeof(LOGIN)-1, 0},
    {AUTHENTICATE, sizeof(AUTHENTICATE)-1, 0},
    {STARTTLS, sizeof(STARTTLS)-1, 1},
    {STARTTLS2, sizeof(STARTTLS2)-1, 1},
    /* These are represented by index >= PATTERN_IMAP_OTHER */
    {CAPA, sizeof(CAPA)-1, 1},
    {CAPA2, sizeof(CAPA2)-1, 1},
    {NOOP, sizeof(NOOP)-1, 1},
    {NOOP2, sizeof(NOOP2)-1, 1},
    {LOGOUT, sizeof(LOGOUT)-1, 1},
    {LOGOUT2, sizeof(LOGOUT2)-1, 1},
    {SELECT, sizeof(SELECT)-1, 0},
    {CREATE, sizeof(CREATE)-1, 0},
    {DELETE, sizeof(DELETE)-1, 0},
    {RENAME, sizeof(RENAME)-1, 0},
    {SUBSCRIBE, sizeof(SUBSCRIBE)-1, 0},
    {UNSUBSCRIBE, sizeof(UNSUBSCRIBE)-1, 0},
    {LISTC, sizeof(LISTC)-1, 0},
    {LSUB, sizeof(LSUB)-1, 0},
    {APPEND, sizeof(APPEND)-1, 0},
    {CHECK, sizeof(CHECK)-1, 1},
    {CHECK2, sizeof(CHECK2)-1, 1},
    {CLOSE, sizeof(CLOSE)-1, 1},
    {CLOSE2, sizeof(CLOSE2)-1, 1},
    {EXPUNGE, sizeof(EXPUNGE)-1, 1},
    {EXPUNGE2, sizeof(EXPUNGE2)-1, 1},
    {SEARCH, sizeof(SEARCH)-1, 0},
    {FETCH, sizeof(FETCH)-1, 0},
    {PARTIAL, sizeof(PARTIAL)-1, 0},
    {STORE, sizeof(STORE)-1, 0},
    {COPY, sizeof(COPY)-1, 0},
    {UID, sizeof(UID)-1, 0},
};

static size_t longest_pattern;

#define IMAP_PORT   143

#define IMAP_COUNT_THRESHOLD 1

#define OK "OK"
#define BAD "BAD"
#define NO "NO"

#define IMAP_FLAG_ALNUM         0x01
#define IMAP_FLAG_FIRST_PACKET  0x02
#define IMAP_FLAG_RESULT_OK     0x04
#define IMAP_FLAG_RESULT_NO     0x08
#define IMAP_FLAG_RESULT_BAD    0x10
#define IMAP_FLAG_RESULT_ALL    (IMAP_FLAG_RESULT_OK | IMAP_FLAG_RESULT_NO | IMAP_FLAG_RESULT_BAD)

#define IMAP_MAX_BANNER 192

typedef enum
{
    IMAP_STATE_BEGIN,
    IMAP_STATE_BANNER_SPACE,
    IMAP_STATE_BANNER_OK,
    IMAP_STATE_BANNER_WHITE_SPACE,
    IMAP_STATE_BANNER,
    IMAP_STATE_MID_LINE,
    IMAP_STATE_MID_ALNUM,
    IMAP_STATE_ALNUM_CODE,
    IMAP_STATE_ALNUM_CODE_TERM,
    IMAP_STATE_MID_OK,
    IMAP_STATE_MID_NO,
    IMAP_STATE_MID_BAD,
    IMAP_STATE_MID_TERM,
    IMAP_STATE_MID_OK_LOGIN,
    IMAP_STATE_MID_NO_LOGIN,
    IMAP_STATE_ALNUM_TAG
} IMAPState;

typedef struct _SERVICE_IMAP_DATA
{
    IMAPState state;
    unsigned pos;
    unsigned flags;
    unsigned count;
    unsigned parens;
    char     tagValue[IMAP_TAG_MAX_LEN+1];
#ifdef DEBUG_IMAP_DETECTOR
    IMAPState last_state;
#endif
} ServiceIMAPData;

static int imap_init(const InitServiceAPI * const init_api);
static int imap_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &imap_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "imap",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&imap_validate, IMAP_PORT, IPPROTO_TCP},
    {&imap_validate, 220, IPPROTO_TCP},
    {NULL, 0, 0}
};

static tRNAServiceValidationModule service_mod =
{
    .name = "IMAP",
    .init = &imap_init,
    .pp = pp,
    .provides_user = 1,
};

#define IMAP_PATTERN "* OK"

typedef struct _DETECTOR_DATA
{
    ClientAppData client;
    ServiceIMAPData server;
    int need_continue;
} DetectorData;

SF_SO_PUBLIC RNADetectorValidationModule imap_detector_mod =
{
    .service = &service_mod,
    .client = &client_app_mod,
};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_IMAP, APPINFO_FLAG_CLIENT_USER},
    {APP_ID_IMAPS, APPINFO_FLAG_CLIENT_USER}
};

static CLIENT_APP_RETCODE init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;
    void *cmd_matcher;

    if (!(cmd_matcher = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return CLIENT_APP_EINVALID;

    for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
    {
        _dpd.searchAPI->search_instance_add_ex(cmd_matcher,
                (char *)patterns[i].pattern,
                patterns[i].length,
                &patterns[i],
                STR_SEARCH_CASE_INSENSITIVE);
        if (patterns[i].length > longest_pattern)
            longest_pattern = patterns[i].length;
    }
    _dpd.searchAPI->search_instance_prep(cmd_matcher);

    AppIdAddGenericConfigItem(init_api->pAppidConfig, client_app_mod.name, cmd_matcher);

    ca_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
                item;
                item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                ca_config.enabled = atoi(item->value);
            }
        }
    }

    if (ca_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering pattern: %s\n",(const char *)patterns[i].pattern);
            init_api->RegisterPatternNoCase(&validate, IPPROTO_TCP, patterns[i].pattern,
                    patterns[i].length, -1, init_api->pAppidConfig);
        }

    }


    unsigned j;
    for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
    {
        _dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
        init_api->RegisterAppId(&validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
    }

    return CLIENT_APP_SUCCESS;
}

static int imap_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPatternUser(&imap_validate, IPPROTO_TCP, (uint8_t *)IMAP_PATTERN, sizeof(IMAP_PATTERN)-1, 0, "imap", init_api->pAppidConfig);

	unsigned j;
	for (j=0; j < sizeof(appIdRegistry)/sizeof(*appIdRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[j].appId);
		init_api->RegisterAppId(&imap_validate, appIdRegistry[j].appId, appIdRegistry[j].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static void clean(const CleanClientAppAPI * const clean_api)
{
    void    *cmd_matcher = AppIdFindGenericConfigItem(clean_api->pAppidConfig, client_app_mod.name);
    if (cmd_matcher)
    {
        _dpd.searchAPI->search_instance_free(cmd_matcher);
        cmd_matcher = NULL;
    }

    AppIdRemoveGenericConfigItem(clean_api->pAppidConfig, client_app_mod.name);
}

static int pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    Client_App_Pattern **pcmd = (Client_App_Pattern **)data;

    if (index)
        return 0;
    pcmd = (Client_App_Pattern **)data;
    *pcmd = id;
    return 1;
}

inline static int isImapTagChar(uint8_t tag)
{
    /* Per RFC 3501 */
    /* tag char's cannot consist of ", %, { */
    if ((tag == 0x7B) || (tag == 0x22) || (tag == 0x25))
        return 0;

    /* Alpha Numeric's */
    if (isalnum(tag) /* valid tag chars: 0-9, A-Z, a-z */
            || (tag >=0x2C && tag <=0x2F) /* valid tag chars: , - . / */
            || (tag >=0x5D && tag <= 0x60) /* valid tag chars: ] ^ _ ` */
            || (tag >= 0x21 && tag <= 0x27) /* valid tag chars: ! # $ & , */
            /* 0x22 " and 0x25 % invalid as above */
            || (tag >= 0x3a && tag <= 0x40) /*valid tag chars: : ; < = > ? @ */
            || (tag == 0x5b) /*valid tag chars: [ */
            || (tag >= 0x7c && tag <= 0x7e) /* valid tag chars: | } ~ */
       )
        return 1;

    return 0;
}

static int imap_server_validate(SFSnortPacket *pkt, const int dir, const tAppIdConfig *pConfig, DetectorData *dd, const uint8_t *data, uint16_t size, tAppIdData *flowp)
{
    const uint8_t *end = data + size;
    ServiceIMAPData *id = &dd->server;

    id->flags &= ~ IMAP_FLAG_RESULT_ALL; // when we are done these flags will tell us OK vs. NO vs. BAD
    for (; data < end; data++)
    {
#ifdef DEBUG_IMAP_DETECTOR
        if (id->state != id->last_state)
        {
            _dpd.debugMsg(DEBUG_LOG,"%p State %d\n",flowp, id->state);
            id->last_state = id->state;
        }
#endif
        switch (id->state)
        {
        case IMAP_STATE_BEGIN:
            if (id->flags & IMAP_FLAG_FIRST_PACKET)
            {
                id->flags &= ~IMAP_FLAG_FIRST_PACKET;
                if (*data == '*')
                {
                    id->state = IMAP_STATE_BANNER_SPACE;
                    break;
                }
            }
            if (*data == '+' || *data == '*')
            {
                id->state = IMAP_STATE_MID_LINE;
                id->flags &= ~IMAP_FLAG_ALNUM;
            }
            else if (isImapTagChar(*data))
            {
                id->flags |= IMAP_FLAG_ALNUM;
                id->tagValue[0] = *data;
                id->pos = 1;
                id->state = IMAP_STATE_ALNUM_TAG;
            }
            else
                return -1;
            break;
        case IMAP_STATE_BANNER_SPACE:
            if (*data == ' ')
            {
                id->state = IMAP_STATE_BANNER_OK;
                id->pos = 0;
            }
            else
                id->state = IMAP_STATE_MID_LINE;
            break;
        case IMAP_STATE_BANNER_OK:
            if (*data == OK[id->pos])
            {
                id->pos++;
                if (id->pos >= sizeof(OK)-1)
                    id->state = IMAP_STATE_BANNER_WHITE_SPACE;
            }
            else
                id->state = IMAP_STATE_MID_LINE;
            break;
        case IMAP_STATE_BANNER_WHITE_SPACE:
            if (*data==' ' || *data=='\t')
                break;
            else if (*data == 0x0D)
                id->state = IMAP_STATE_MID_TERM;
            else if (*data == 0x0A)
                id->state = IMAP_STATE_BEGIN;
            else if (!isprint(*data))
                return -1;
            else
                id->state = IMAP_STATE_BANNER;
            break;
        case IMAP_STATE_BANNER:
            if (*data == 0x0D)
                id->state = IMAP_STATE_MID_TERM;
            else if (*data == 0x0A)
                id->state = IMAP_STATE_BEGIN;
            else if (!isprint(*data))
                return -1;
            break;
        case IMAP_STATE_MID_LINE:
            if (*data == 0x0D)
            {
                if (!id->parens)
                    id->state = IMAP_STATE_MID_TERM;
            }
            else if (*data == 0x0A)
            {
                if (!id->parens)
                {
                    id->state = IMAP_STATE_BEGIN;
                    if (id->flags & IMAP_FLAG_ALNUM)
                        id->count++;
                }
            }
            else if (*data == '(')
                id->parens++;
            else if (*data == ')')
            {
                if (id->parens)
                    id->parens--;
            }
            else if (!isprint(*data) && *data != 0x09)
                return -1;
            break;
        case IMAP_STATE_MID_TERM:
            if (*data == 0x0A)
            {
                id->state = IMAP_STATE_BEGIN;
                if (id->flags & IMAP_FLAG_ALNUM)
                    id->count++;
            }
            else
                return -1;
            break;
        case IMAP_STATE_MID_ALNUM:
            if (*data == ' ')
                id->state = IMAP_STATE_ALNUM_CODE;
            else
                return -1;
            break;
        case IMAP_STATE_ALNUM_TAG:
            if ((id->pos < (sizeof(id->tagValue)-1))
                && (isImapTagChar(*data)))
            {
                id->tagValue[id->pos++] = *data;
            }
            else
            {
                id->tagValue[id->pos] = '\0';
                id->state = IMAP_STATE_ALNUM_CODE;
            }
            break;

        case IMAP_STATE_ALNUM_CODE:
            if (*data == OK[0])
            {
                id->state = IMAP_STATE_MID_OK;
                id->pos = 1;
            }
            else if (*data == NO[0])
            {
                id->state = IMAP_STATE_MID_NO;
                id->pos = 1;
            }
            else if (*data == BAD[0])
            {
                id->state = IMAP_STATE_MID_BAD;
                id->pos = 1;
            }
            else
                return -1;
            break;
        case IMAP_STATE_MID_OK:
            if (*data == OK[id->pos])
            {
                id->pos++;
                if (id->pos >= sizeof(OK)-1)
                {
                    id->pos = 0;
                    id->state = IMAP_STATE_MID_OK_LOGIN;
                    if (!strcasecmp(id->tagValue, dd->client.imapCmdTag))
                    {
                        dd->client.imapCmdTag[0] = '\0';
                        id->flags |= IMAP_FLAG_RESULT_OK;
                    }
                }
            }
            else
                return -1;
            break;

        case IMAP_STATE_MID_OK_LOGIN:
            /*add user successful */
            if ((id->flags & IMAP_FLAG_RESULT_OK) && dd->client.username[0])
            {
                service_mod.api->add_user(flowp, dd->client.username, APP_ID_IMAP, 1);  // use of LOGIN cmd implies no IMAPS
            }
            id->state = IMAP_STATE_MID_LINE;
            break;
        case IMAP_STATE_MID_NO:
            if (*data == NO[id->pos])
            {
                id->pos++;
                if (id->pos >= sizeof(NO)-1)
                {
                    id->pos = 0;
                    id->state = IMAP_STATE_MID_NO_LOGIN;
                    if (!strcasecmp(id->tagValue, dd->client.imapCmdTag))
                    {
                        dd->client.imapCmdTag[0] = '\0';
                        id->flags |= IMAP_FLAG_RESULT_NO;
                    }
                }
            }
            else
                return -1;
            break;
        case IMAP_STATE_MID_NO_LOGIN:
            if (*data == NO_LOGIN[id->pos])
            {
                id->pos++;
                if (id->pos >= sizeof(NO_LOGIN)-1)
                {
                    id->state = IMAP_STATE_ALNUM_CODE_TERM;
                    /*add user login failed */
                    if ((id->flags & IMAP_FLAG_RESULT_NO) && dd->client.username[0])
                    {
                        service_mod.api->add_user(flowp, dd->client.username, APP_ID_IMAP, 0); // use of LOGIN cmd implies no IMAPS
                    }
                }
            }
            else
                id->state = IMAP_STATE_MID_LINE;
            break;

        case IMAP_STATE_MID_BAD:
            if (*data == BAD[id->pos])
            {
                id->pos++;
                if (id->pos >= sizeof(BAD)-1)
                {
                    id->state = IMAP_STATE_ALNUM_CODE_TERM;
                    if (!strcasecmp(id->tagValue, dd->client.imapCmdTag))
                    {
                        dd->client.imapCmdTag[0] = '\0';
                        id->flags |= IMAP_FLAG_RESULT_BAD;
                    }
                }
            }
            else
                return -1;
            break;
        case IMAP_STATE_ALNUM_CODE_TERM:
            if (*data == 0x0D)
                id->state = IMAP_STATE_MID_TERM;
            else if (*data == 0x0A)
            {
                id->state = IMAP_STATE_BEGIN;
                id->count++;
            }
            else if (*data == ' ')
                id->state = IMAP_STATE_MID_LINE;
            else
                return -1;
            break;
        }
    }
    if (dd->client.state == IMAP_CLIENT_STATE_STARTTLS_CMD)
    {
        if (id->flags & IMAP_FLAG_RESULT_OK)
        {
            clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
            /* we are potentially overriding any APP_ID_IMAP assessment that was made earlier. */
            client_app_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_IMAPS, APP_ID_IMAPS, NULL); // sets APPID_SESSION_CLIENT_DETECTED
        }
        else
        {
            /* We failed to transition to IMAPS - fall back to normal IMAP state, Non-Authenticated */
            dd->client.state = IMAP_CLIENT_STATE_NON_AUTH;
        }
    }
    else if (dd->client.state == IMAP_CLIENT_STATE_AUTHENTICATE_CMD)
    {
        dd->client.auth = 0; // stop discarding intervening command packets (part of the authenticate)
        /* Change state as appropriate */
        dd->client.state = (id->flags & IMAP_FLAG_RESULT_OK) ?
            IMAP_CLIENT_STATE_AUTH :
            IMAP_CLIENT_STATE_NON_AUTH;
    }
    return 0;
}

static CLIENT_APP_RETCODE validate(const uint8_t *data, uint16_t size, const int dir,
                                   tAppIdData *flowp, SFSnortPacket *pkt, struct _Detector *userData,
                                   const tAppIdConfig *pConfig)
{
    const uint8_t *s = data;
    const uint8_t *end = (data + size);
    unsigned length;
    Client_App_Pattern *cmd = NULL;
    DetectorData *dd;
    ClientAppData *fd;
    char tag[IMAP_TAG_MAX_LEN+1] = {0};
    void    *cmd_matcher = AppIdFindGenericConfigItem((tAppIdConfig *)pConfig, client_app_mod.name);

#ifdef APP_ID_USES_REASSEMBLED
    imap_detector_mod.streamAPI->response_flush_stream(pkt);
#endif

    if (!size)
        return CLIENT_APP_INPROCESS;

    dd = imap_detector_mod.api->data_get(flowp, imap_detector_mod.flow_data_index);
    if (!dd)
    {
        if ((dd =  calloc(1, sizeof(*dd))) == NULL)
            return CLIENT_APP_ENOMEM;
        if (imap_detector_mod.api->data_add(flowp, dd, imap_detector_mod.flow_data_index, &free))
        {
            free(dd);
            return CLIENT_APP_ENOMEM;
        }
        dd->server.flags = IMAP_FLAG_FIRST_PACKET;
        fd = &dd->client;
    }
    else
        fd = &dd->client;

    if (!fd->set_flags)
    {
        dd->need_continue = 1;
        fd->set_flags = 1;
        setAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
    }

    if (dir == APP_ID_FROM_RESPONDER)
    {
        if (imap_server_validate(pkt, dir, pConfig, dd, data, size, flowp))
            clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
        return CLIENT_APP_INPROCESS;
    }

    while ((length = (end - s)) > 0)
    {
        unsigned pattern_index;
        if (fd->auth)
        {
            /* authentication exchange in progress ignore all client-side
               packets until server-side OK/BAD/NO received */
            for (; (s < end) && *s != '\r' && *s != '\n'; s++);
            for (; (s < end) && (*s == '\r' || *s == '\n'); s++);
            continue;
        }

        {
            /*processing tags */
            char *p = tag;
            char *p_end = p + sizeof(tag) - 1;
            for (; (s < end) && isImapTagChar(*s); s++)
            {
                if (p < p_end)
                {
                    *p++ = *s;
                }
            }
            for (; (s < end) && !isspace(*s); s++);
            *p = '\0';
        }

        if (end == s || !isblank(*s))
        {
            dd->need_continue = 0;
            setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
            clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
            return CLIENT_APP_SUCCESS;
        }
        for (; (s < end) && isblank(*s); s++);

        /*s is now at command beginning */
        if ((length = (end - s)) <= 0)
        {
            dd->need_continue = 0;
            setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
            clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
            return CLIENT_APP_SUCCESS;
        }
        cmd = NULL;
        _dpd.searchAPI->search_instance_find_all(cmd_matcher,
                   (char *)s,
                   (length > longest_pattern ? longest_pattern:length), 0,
                   &pattern_match, (void*)&cmd);

        if (!cmd)
        {
            if ( (s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z') )
            {
                // Command was not in the recognized list. Keep searching.
                return CLIENT_APP_INPROCESS;
            }
            else
            {
                // IMAP commands are English words, or at least start with X.
                return CLIENT_APP_ENULL; // anything but CLIENT_APP_SUCCESS or CLIENT_APP_INPROCESS
            }
        }
        s += cmd->length;

        pattern_index = cmd - patterns; // diff of ptr into array and its base addr is the corresponding index.
        switch (fd->state)
        {
        case IMAP_CLIENT_STATE_AUTHENTICATE_CMD:
        case IMAP_CLIENT_STATE_STARTTLS_CMD:
            /* The command we received was rejected by the server side -
               fall back to normal IMAP Non-Authorized state */
            fd->state = IMAP_CLIENT_STATE_NON_AUTH;
            // fall through

        case IMAP_CLIENT_STATE_NON_AUTH:
            switch (pattern_index)
            {
            case PATTERN_LOGIN:
                strncpy(fd->imapCmdTag, tag, sizeof(fd->imapCmdTag));
                {
                    char *p = fd->username;
                    char *p_end = p + sizeof(fd->username) - 1;
                    int found_tick = 0;

                    if (*s == '"')
                    {
                        s++;
                        for (; s < end && p < p_end; s++)
                        {
                            if (*s == '"')
                            {
                                fd->count++;
                                if (fd->count == MIN_CMDS)
                                {
                                    client_app_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_IMAP, APP_ID_IMAP, NULL);
                                    fd->detected = 1;
                                    if (fd->got_user)
                                    {
                                        setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
                                        clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
                                    }
                                    fd->state = IMAP_CLIENT_STATE_AUTH;
                                }
                                *p = 0;
                                fd->got_user = 1;
                                break;
                            }
                            else if (isalnum(*s) || *s == '.' || *s == '@' || *s == '-' || *s == '_' || *s == '`' || *s == ' ')
                            {
                                *p = *s;
                                p++;
                            }
                            else
                                break;
                        }
                    }
                    else
                    {
                        for (; s < end && p < p_end; s++)
                        {
                            if (isalnum(*s) || *s == '.' || *s == '@' || *s == '-' || *s == '_')
                            {
                                if (!found_tick)
                                {
                                    *p = *s;
                                    p++;
                                }
                            }
                            else if (*s == '`')
                                found_tick = 1;
                            else if (*s == ' ')
                            {
                                fd->count++;
                                if (fd->count == MIN_CMDS)
                                {
                                    client_app_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_IMAP, APP_ID_IMAP, NULL);
                                    fd->detected = 1;
                                    if (fd->got_user)
                                    {
                                        setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
                                        clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
                                    }
                                }
                                *p = 0;
                                fd->got_user = 1;
                                break;
                            }
                            else
                                break;
                        }
                    }
                    for (; (s < end) && *s != '\r' && *s != '\n'; s++);
                    for (; (s < end) && (*s == '\r' || *s == '\n'); s++);
                }
                break;

            case PATTERN_STARTTLS:
            case PATTERN_STARTTLS2:
                strncpy(fd->imapCmdTag, tag, sizeof(fd->imapCmdTag));
                fd->state = IMAP_CLIENT_STATE_STARTTLS_CMD;
                for (; (s < end) && (*s == '\r' || *s == '\n'); s++); // all we need because cmd->eoc == 1
                /* No other commands will be coming until the result from this one. */
                break;

            case PATTERN_AUTHENTICATE:
                strncpy(fd->imapCmdTag, tag, sizeof(fd->imapCmdTag));
                fd->auth = 1; // gobble additional client packets until the server OK/BAD/NO response
                fd->state = IMAP_CLIENT_STATE_AUTHENTICATE_CMD;
                for (; (s < end) && *s != '\r' && *s != '\n'; s++);
                for (; (s < end) && (*s == '\r' || *s == '\n'); s++);
                break;

            default:
                {
                    fd->count++;
                    if (fd->count == MIN_CMDS)
                    {
                        client_app_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_IMAP, APP_ID_IMAP, NULL);
                        fd->detected = 1;
                        if (fd->got_user)
                        {
                            setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
                            clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
                        }
                    }
                    if (!cmd->eoc)
                        for (; (s < end) && *s != '\r' && *s != '\n'; s++);
                    for (; (s < end) && (*s == '\r' || *s == '\n'); s++);
                }
                break;
            }
            break;
        case IMAP_CLIENT_STATE_AUTH:
            {
                fd->count++;
                if (fd->count == MIN_CMDS)
                {
                    client_app_mod.api->add_app(pkt, dir, pConfig, flowp, APP_ID_IMAP, APP_ID_IMAP, NULL);
                    fd->detected = 1;
                    if (fd->got_user)
                    {
                        setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
                        clearAppIdFlag(flowp, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS);
                    }
                }
                if (!cmd->eoc)
                    for (; (s < end) && *s != '\r' && *s != '\n'; s++);
                for (; (s < end) && (*s == '\r' || *s == '\n'); s++);
            }
            break;
        } // end switch(fd->state)
    } // end 'while'
    return CLIENT_APP_INPROCESS;
}

static int imap_validate(ServiceValidationArgs* args)
{
    DetectorData *dd;
    ServiceIMAPData *id;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    uint16_t size = args->size;

    if (args->dir != APP_ID_FROM_RESPONDER)
        goto inprocess;

#ifdef APP_ID_USES_REASSEMBLED
    imap_detector_mod.streamAPI->response_flush_stream(pkt);
#endif

    if (!size)
        goto inprocess;

    dd = imap_detector_mod.api->data_get(flowp, imap_detector_mod.flow_data_index);
    if (!dd)
    {
        if ((dd = calloc(1, sizeof(*dd))) == NULL)
            return SERVICE_ENOMEM;
        if (imap_detector_mod.api->data_add(flowp, dd, imap_detector_mod.flow_data_index, &free))
        {
            free(dd);
            return SERVICE_ENOMEM;
        }
        id = &dd->server;
        id->state = IMAP_STATE_BEGIN;
        id->flags = IMAP_FLAG_FIRST_PACKET;
    }
    else
        id = &dd->server;

    if (dd->need_continue)
        setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
    else
    {
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        if (getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
            return SERVICE_SUCCESS;
    }

    if (!imap_server_validate(args->pkt, args->dir, args->pConfig, dd, data, size, flowp))
    {
        if ((id->flags & IMAP_FLAG_RESULT_OK) && dd->client.state == IMAP_CLIENT_STATE_STARTTLS_CMD)
        {
            /* IMAP server response to STARTTLS command from client was OK */
            service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                         APP_ID_IMAPS, NULL, NULL, NULL, NULL);
            return SERVICE_SUCCESS;
        }
        if (id->count >= IMAP_COUNT_THRESHOLD && !getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                         APP_ID_IMAP, NULL, NULL, NULL, NULL);
            return SERVICE_SUCCESS;
        }
    }
    else if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
    {
        service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                      service_mod.flow_data_index, args->pConfig, NULL);
        return SERVICE_NOMATCH;
    }
    else
    {
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_SUCCESS;
    }

inprocess:
    service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    return SERVICE_INPROCESS;
}

