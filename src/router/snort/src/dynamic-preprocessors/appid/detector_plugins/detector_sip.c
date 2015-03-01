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
#include "client_app_base.h"
#include "sf_multi_mpse.h"
#include "sf_mlmp.h"
#include "flow.h"
#include "fw_appid.h"

#include "client_app_api.h"
#include "service_api.h"
#include "sip_common.h"

static const char SIP_REGISTER_BANNER[] = "REGISTER ";
static const char SIP_INVITE_BANNER[] = "INVITE ";
static const char SIP_CANCEL_BANNER[] = "CANCEL ";
static const char SIP_ACK_BANNER[] = "ACK ";
static const char SIP_BYE_BANNER[] = "BYE ";
static const char SIP_OPTIONS_BANNER[] = "OPTIONS ";
static const char SIP_BANNER[] = "SIP/2.0 ";
static const char SIP_BANNER_END[] = "SIP/2.0\x00d\x00a";
#define SIP_REGISTER_BANNER_LEN (sizeof(SIP_REGISTER_BANNER)-1)
#define SIP_INVITE_BANNER_LEN (sizeof(SIP_INVITE_BANNER)-1)
#define SIP_CANCEL_BANNER_LEN (sizeof(SIP_CANCEL_BANNER)-1)
#define SIP_ACK_BANNER_LEN (sizeof(SIP_ACK_BANNER)-1)
#define SIP_BYE_BANNER_LEN (sizeof(SIP_BYE_BANNER)-1)
#define SIP_OPTIONS_BANNER_LEN (sizeof(SIP_OPTIONS_BANNER)-1)
#define SIP_BANNER_LEN (sizeof(SIP_BANNER)-1)
#define SIP_BANNER_END_LEN (sizeof(SIP_BANNER_END)-1)
#define SIP_BANNER_LEN    (sizeof(SIP_BANNER)-1)

#define USER_STRING "from: "
#define MAX_USER_POS ((int)sizeof(USER_STRING) - 2)

static const char svc_name[] = "sip";
#define SIP_PORT    5060

#define MAX_ADDRESS_SIZE    16
#define MAX_CALLID_SIZE        64
#define MAX_VENDOR_SIZE        64
#define MAX_PORT_SIZE        6

typedef enum
{
    SIP_STATE_INIT=0,
    SIP_STATE_REGISTER,
    SIP_STATE_CALL
} SIPState;

#define SIP_STATUS_OK  200

#define SIP_MAX_INFO_SIZE    63

typedef enum 
{
    SIP_FLAG_SERVER_CHECKED = (1<< 0)
} tSIP_FLAGS;

typedef struct _CLIENT_SIP_DATA
{
    SIPState state;
    uint32_t flags;
    char     *userName;
    char     *clientUserAgent;
    char     *from;
} ClientSIPData; 

typedef struct _SIP_CLIENT_APP_CONFIG
{
    int enabled;
} SIP_CLIENT_APP_CONFIG;

static SIP_CLIENT_APP_CONFIG sip_config;


static CLIENT_APP_RETCODE sip_client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static void sip_clean(const CleanClientAppAPI * const clean_api);
static CLIENT_APP_RETCODE sip_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData);
static CLIENT_APP_RETCODE sip_tcp_client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static CLIENT_APP_RETCODE sip_tcp_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData);
typedef struct
{
    tAppId clientAppId;
    char* clientVersion;
} tSipUaUserData;

typedef struct _tDetectorAppSipPattern
{
    tMlpPattern pattern;

    tSipUaUserData userData;

    struct _tDetectorAppSipPattern *next;

} tDetectorAppSipPattern;


static int sipAppGetClientApp(void *patternMatcher, char *pattern, uint32_t patternLen, tAppId *clientAppId, char **clientVersion);
static void sipUaClean(void);
static tDetectorAppSipPattern *appSipUaList;
static void *sipUaMatcher;

static void sipServerClean(void);
static tDetectorAppSipPattern *appSipServerList;
static void *sipServerMatcher;


RNAClientAppModule sip_udp_client_mod =
{
    .name = "SIP",
    .proto = IPPROTO_UDP,
    .init = &sip_client_init,
    .clean = &sip_clean,
    .validate = &sip_client_validate,
    .minimum_matches = 2,
    .provides_user = 1,
};
RNAClientAppModule sip_tcp_client_mod =
{
    .name = "SIP",
    .proto = IPPROTO_TCP,
    .init = &sip_tcp_client_init,
    .validate = &sip_tcp_client_validate,
    .minimum_matches = 2,
    .provides_user = 1,
};

typedef struct {
    const uint8_t *pattern;
    unsigned length;
    int index;
    unsigned appId;
} Client_App_Pattern;

static Client_App_Pattern patterns[] =
{
    {(const uint8_t *)SIP_REGISTER_BANNER, sizeof(SIP_REGISTER_BANNER)-1, 0, APP_ID_SIP},
    {(const uint8_t *)SIP_INVITE_BANNER, sizeof(SIP_INVITE_BANNER)-1,     0, APP_ID_SIP},
    {(const uint8_t *)SIP_CANCEL_BANNER, sizeof(SIP_CANCEL_BANNER)-1,     0, APP_ID_SIP},
    {(const uint8_t *)SIP_ACK_BANNER, sizeof(SIP_ACK_BANNER)-1,           0, APP_ID_SIP},
    {(const uint8_t *)SIP_BYE_BANNER, sizeof(SIP_BYE_BANNER)-1,           0, APP_ID_SIP},
    {(const uint8_t *)SIP_OPTIONS_BANNER, sizeof(SIP_OPTIONS_BANNER)-1,   0, APP_ID_SIP},
    {(const uint8_t *)SIP_BANNER, sizeof(SIP_BANNER)-1,                   0, APP_ID_SIP},
    {(const uint8_t *)SIP_BANNER_END, sizeof(SIP_BANNER_END)-1,          -1, APP_ID_SIP},
};

static tAppRegistryEntry appIdClientRegistry[] =
{
    {APP_ID_SIP, APPINFO_FLAG_CLIENT_ADDITIONAL|APPINFO_FLAG_CLIENT_USER},

};

static tAppRegistryEntry appIdServiceRegistry[] =
{
    {APP_ID_SIP, APPINFO_FLAG_SERVICE_ADDITIONAL|APPINFO_FLAG_CLIENT_USER},
    {APP_ID_RTP, APPINFO_FLAG_SERVICE_ADDITIONAL}

};

//service side
typedef struct _SERVICE_SIP_DATA
{
    uint8_t serverPkt;
    char     vendor[MAX_VENDOR_SIZE];
} ServiceSIPData;

static int sip_service_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(sip_service_validate);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &sip_service_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "sip",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&sip_service_validate, SIP_PORT, IPPROTO_TCP},
    {&sip_service_validate, SIP_PORT, IPPROTO_UDP},
    {NULL, 0, 0}
};

SO_PUBLIC RNAServiceValidationModule sip_service_mod =
{
    svc_name,
    &sip_service_init,
    pp
};

static CLIENT_APP_RETCODE sip_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;


    /*configuration is read by sip_tcp_init(), which is called first */

    if (sip_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&sip_client_validate, IPPROTO_UDP, patterns[i].pattern, patterns[i].length, patterns[i].index);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdClientRegistry)/sizeof(*appIdClientRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdClientRegistry[j].appId);
		init_api->RegisterAppId(&sip_client_validate, appIdClientRegistry[j].appId, appIdClientRegistry[j].additionalInfo, NULL);
	}

    if (sipUaMatcher)
    {
        sipUaClean();
    }
    if (sipServerMatcher)
    {
        sipServerClean();
    }
    return CLIENT_APP_SUCCESS;
}

static void sip_clean(const CleanClientAppAPI * const clean_api)
{
    if (sipUaMatcher)
    {
        sipUaClean();
    }
    if (sipServerMatcher)
    {
        sipServerClean();
    }
}

static CLIENT_APP_RETCODE sip_tcp_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned i;
    RNAClientAppModuleConfigItem *item;

    sip_config.enabled = 1;

    if (config)
    {
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(config);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(config))
        {
            _dpd.debugMsg(DEBUG_LOG,"Processing %s: %s\n",item->name, item->value);
            if (strcasecmp(item->name, "enabled") == 0)
            {
                sip_config.enabled = atoi(item->value);
            }
        }
    }

    if (sip_config.enabled)
    {
        for (i=0; i < sizeof(patterns)/sizeof(*patterns); i++)
        {
            _dpd.debugMsg(DEBUG_LOG,"registering patterns: %s: %d\n",(const char *)patterns[i].pattern, patterns[i].index);
            init_api->RegisterPattern(&sip_tcp_client_validate, IPPROTO_TCP, patterns[i].pattern, patterns[i].length, patterns[i].index);
        }
    }

	unsigned j;
	for (j=0; j < sizeof(appIdClientRegistry)/sizeof(*appIdClientRegistry); j++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdClientRegistry[j].appId);
		init_api->RegisterAppId(&sip_tcp_client_validate, appIdClientRegistry[j].appId, appIdClientRegistry[j].additionalInfo, NULL);
	}

    return CLIENT_APP_SUCCESS;
}

static void clientDataFree(void *data)
{
    ClientSIPData *fd = data;
    free(fd->from);
    free(fd->clientUserAgent);
    free(fd->userName);
    free (fd);
}

#define SIP_USRNAME_BEGIN_MARKER "<sip:"
static CLIENT_APP_RETCODE sip_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData)
{
    ClientSIPData *fd;

    fd = sip_udp_client_mod.api->data_get(flowp);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return CLIENT_APP_ENOMEM;
        if (sip_udp_client_mod.api->data_add(flowp, fd, &clientDataFree))
        {
            free(fd);
            return CLIENT_APP_ENOMEM;
        }
        flow_mark(flowp, FLOW_CLIENT_GETS_SERVER_PACKETS);
    }

    return CLIENT_APP_INPROCESS;
}

static CLIENT_APP_RETCODE sip_tcp_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                        FLOW *flowp, const SFSnortPacket *pkt, struct _Detector *userData)
{
    return sip_client_validate(data, size, dir, flowp, pkt, userData);
}


static int sipAppAddPattern(
        tDetectorAppSipPattern **patternList,
        tAppId clientAppId, 
        const char* clientVersion,
        const char* serverPattern
        )
{
    /* Allocate memory for data structures */
    tDetectorAppSipPattern *pattern = malloc(sizeof(tDetectorAppSipPattern));
    if (!pattern)
    {
        return -1;
    }

    pattern->userData.clientAppId = clientAppId;
    pattern->userData.clientVersion = strdup(clientVersion);
    if (!pattern->userData.clientVersion)
    {
        _dpd.errMsg("failed to allocate client version");
        free(pattern);
        return -1;
    }
    pattern->pattern.pattern = (uint8_t *)strdup(serverPattern);
    if (!pattern->pattern.pattern)
    {
        _dpd.errMsg("failed to allocate patterns");
        free(pattern->userData.clientVersion);
        free(pattern);
        return -1;
    }
    pattern->pattern.patternSize = (int) strlen(serverPattern);

    pattern->next = *patternList;
    *patternList = pattern;

    return 0;
}

int sipUaPatternAdd(
        tAppId clientAppId, 
        const char* clientVersion,
        const char* pattern
        )
{
    return sipAppAddPattern( &appSipUaList, clientAppId, clientVersion, pattern);
}

int sipServerPatternAdd(
        tAppId clientAppId, 
        const char* clientVersion,
        const char* pattern
        )
{
    return sipAppAddPattern( &appSipServerList, clientAppId, clientVersion, pattern);
}


#include "http_url_patterns.h"
#define PATTERN_PART_MAX 10
int sipUaFinalize(void)
{
    static tMlmpPattern patterns[PATTERN_PART_MAX];
    int num_patterns;
    tDetectorAppSipPattern *patternNode;

    sipUaMatcher = mlmpCreate();
    if (!sipUaMatcher)
        return -1;

    sipServerMatcher = mlmpCreate();
    if (!sipServerMatcher)
    {
        mlmpDestroy(sipUaMatcher);
        sipUaMatcher = NULL;
        return -1;
    }

    for (patternNode = appSipUaList; patternNode; patternNode = patternNode->next)
    {
        num_patterns = parseMultipleHTTPPatterns((const char *)patternNode->pattern.pattern, patterns,  PATTERN_PART_MAX, 0);
        patterns[num_patterns].pattern = NULL;

        mlmpAddPattern(sipUaMatcher, patterns, patternNode);
    }

    for (patternNode = appSipServerList; patternNode; patternNode = patternNode->next)
    {
        num_patterns = parseMultipleHTTPPatterns((const char *)patternNode->pattern.pattern, patterns,  PATTERN_PART_MAX, 0);
        patterns[num_patterns].pattern = NULL;

        mlmpAddPattern(sipServerMatcher, patterns, patternNode);
    }

    mlmpProcessPatterns(sipUaMatcher);
    mlmpProcessPatterns(sipServerMatcher);
    return 0;
}

static void sipUaClean(void)
{
    tDetectorAppSipPattern *node;

    if (sipUaMatcher)
    {
        mlmpDestroy(sipUaMatcher);
        sipUaMatcher = NULL;
    }

    for (node = appSipUaList; node; node = appSipUaList)
    {
        appSipUaList = node->next;
        free((void*)node->pattern.pattern);
        free(node->userData.clientVersion);
        free(node);
    }
}

static void sipServerClean(void)
{
    tDetectorAppSipPattern *node;

    if (sipServerMatcher)
    {
        mlmpDestroy(sipServerMatcher);
        sipServerMatcher = NULL;
    }

    for (node = appSipServerList; node; node = appSipServerList)
    {
        appSipServerList = node->next;
        free((void*)node->pattern.pattern);
        free(node->userData.clientVersion);
        free(node);
    }
}

static int sipAppGetClientApp(
        void *patternMatcher, 
        char *pattern, 
        uint32_t patternLen, 
        tAppId *clientAppId, 
        char **clientVersion)
{
    tMlmpPattern patterns[3];
    tDetectorAppSipPattern *data;

    if (!pattern)
        return 0;

    patterns[0].pattern = (uint8_t *)pattern;
    patterns[0].patternSize = patternLen;
    patterns[1].pattern = NULL;

    data = (tDetectorAppSipPattern *)mlmpMatchPatternUrl(patternMatcher, patterns);

    if (data == NULL)
        return 0;

    *clientAppId = data->userData.clientAppId;
    *clientVersion = data->userData.clientVersion;

    return 1;
}

#define SIP_FAILURE -1
#define SIP_SUCCESS  0


#define SIP_KEYWORD          "SIP/"
#define SIP_KEYWORD_LEN      4
#define SIP_VERSION_NUM_LEN  3  /*2.0 or 1.0 or 1.1*/
#define SIP_VERSION_LEN      SIP_KEYWORD_LEN + SIP_VERSION_NUM_LEN
#define SIP_MIN_MSG_LEN      SIP_VERSION_LEN

#define MAX_STAT_CODE      999
#define MIN_STAT_CODE      100

static void createRtpFlow(const SFSnortPacket *pkt, snort_ip *cliIp, uint16_t cliPort,
                          snort_ip *srvIp, uint16_t srvPort, uint8_t proto, int16_t app_id)
{
    FLOW *fp;

    fp = sip_service_mod.api->flow_new(pkt, cliIp, cliPort, srvIp, srvPort,
            proto, app_id);
    if(fp)
    {
        fp->serviceAppId = APP_ID_RTP;
        flow_mark(fp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
        //fp->nsession->session_state = NAVL_STATE_TERMINATED;
        fp->rnaServiceState = RNA_STATE_FINISHED;
        fp->rnaClientState = RNA_STATE_FINISHED;
    }
}

static int addFutureRtpFlows( const SipDialog *dialog, const SFSnortPacket *p)
{
	SIP_MediaData *mdataA,*mdataB;

	// check the first media session
	if (NULL == dialog->mediaSessions)
		return -1;
	// check the second media session
	if (NULL == dialog->mediaSessions->nextS)
		return -1;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Adding future media sessions ID: %u and %u\n",
			dialog->mediaSessions->sessionID, dialog->mediaSessions->nextS->sessionID););
	mdataA = dialog->mediaSessions->medias;
	mdataB = dialog->mediaSessions->nextS->medias;
	while((NULL != mdataA)&&(NULL != mdataB))
    {
    	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Adding future channels Source IP: %s Port: %u\n",
    			sfip_to_str(&mdataA->maddress), mdataA->mport););
    	DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Adding future channels Destine IP: %s Port: %u\n",
    			sfip_to_str(&mdataB->maddress), mdataB->mport););

    	createRtpFlow(p, &mdataA->maddress, mdataA->mport, &mdataB->maddress,
    	                            mdataB->mport, IPPROTO_UDP, APP_ID_RTP);
    	mdataA = mdataA->nextM;
    	mdataB = mdataB->nextM;
    }
	return 0;

}

static void SipSessionCbClientProcess (const SFSnortPacket *p, const SipHeaders *headers, const SipDialog *dialog, tAppIdData *flowp)
{
    ClientSIPData *fd;
    tAppId clientAppId = APP_ID_SIP;
    char   *clientVersion = NULL;
    int direction;

    fd = sip_udp_client_mod.api->data_get(flowp);
    if (!fd) return;

    direction = (_dpd.sessionAPI->get_packet_direction((SFSnortPacket *)p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;

    if (headers->methodFlag == SIP_METHOD_INVITE && direction == APP_ID_FROM_INITIATOR)
    {
        if (headers->from)
        {
            free(fd->from);
            fd->from = strndup(headers->from, headers->fromLen); 
        }

        if (headers->userName)
        {
            free(fd->userName);
            fd->userName = strndup(headers->userName, headers->userNameLen); 
        }
        if (headers->userAgent)
        {
            free(fd->clientUserAgent);
            fd->clientUserAgent = strndup(headers->userAgent, headers->userAgentLen); 
        }
    }

    if (!dialog || dialog->state != SIP_DLG_ESTABLISHED) 
       return; 

    if (fd->clientUserAgent)
    {
            if (sipAppGetClientApp(sipUaMatcher, fd->clientUserAgent, strlen(fd->clientUserAgent), &clientAppId, &clientVersion))
                goto success;
    }
    if ( fd->from && !(fd->flags & SIP_FLAG_SERVER_CHECKED))
    {
            fd->flags |= SIP_FLAG_SERVER_CHECKED;

            sipAppGetClientApp(sipServerMatcher, (char *)fd->from, strlen(fd->from), &clientAppId, &clientVersion);
    }

success:
    //client detection successful
    sip_udp_client_mod.api->add_app(flowp, APP_ID_SIP, clientAppId, clientVersion);

    if(fd->userName)
        sip_udp_client_mod.api->add_user(flowp, (char *)fd->userName, APP_ID_SIP, 1);

    flow_mark(flowp, FLOW_CLIENTAPPDETECTED);
}

static void SipSessionCbServiceProcess (const SFSnortPacket *p, const SipHeaders *headers, const SipDialog *dialog, tAppIdData *flowp)
{
    ServiceSIPData *ss;
    int direction;

    ss = sip_service_mod.api->data_get(flowp);
    if (!ss) return;

    ss->serverPkt = 0;

    direction = (_dpd.sessionAPI->get_packet_direction((SFSnortPacket *)p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;

    if (direction == APP_ID_FROM_RESPONDER)
    {
        if (headers->userAgent)
        {
            memcpy(ss->vendor, headers->userAgent, 
                    headers->userAgentLen > (MAX_VENDOR_SIZE - 1) ?  (MAX_VENDOR_SIZE - 1): headers->userAgentLen);
        }
        else if (headers->server)
        {
            memcpy(ss->vendor, headers->server, 
                    headers->serverLen > (MAX_VENDOR_SIZE - 1) ?  (MAX_VENDOR_SIZE - 1): headers->serverLen);
        }
    }

    if (!dialog) return;

    if (dialog->mediaUpdated)
        addFutureRtpFlows(dialog, p);

    if (dialog->state == SIP_DLG_ESTABLISHED)
    {
        if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
        {
            flow_mark(flowp, FLOW_CONTINUE);
            sip_service_mod.api->add_service(flowp, p, direction, &svc_element,
                    APP_ID_SIP, ss->vendor[0] ? ss->vendor:NULL,
                    NULL, NULL);
        }
    }
}

void SipSessionSnortCallback (void *ssnptr, ServiceEventType eventType, void *data)
{
    tAppIdData *flowp = NULL;
    SipEventData *eventData = (SipEventData *)data;

    const SFSnortPacket *p = eventData->packet;
    const SipHeaders *headers = eventData->headers;
    const SipDialog *dialog = eventData->dialog;

#ifdef DEBUG_APP_ID_SESSIONS
    {
        char src_ip[INET6_ADDRSTRLEN];
        char dst_ip[INET6_ADDRSTRLEN];

        src_ip[0] = 0;
        ip = GET_SRC_IP(p);
        inet_ntop(ip->family, (void *)ip->ip32, src_ip, sizeof(src_ip));
        dst_ip[0] = 0;
        ip = GET_DST_IP(p);
        inet_ntop(ip->family, (void *)ip->ip32, dst_ip, sizeof(dst_ip));
        fprintf(SF_DEBUG_FILE, "AppId Sip Snort Callback Session %s-%u -> %s-%u %d\n", src_ip,
                (unsigned)p->src_port, dst_ip, (unsigned)p->dst_port, IsTCP(p) ? IPPROTO_TCP:IPPROTO_UDP);
    }
#endif
    if (p->stream_session)
        flowp = _dpd.sessionAPI->get_application_data(p->stream_session, PP_APP_ID);

    if (!flowp)
    {
        _dpd.errMsg("Missing session\n");
        return;
    }

    SipSessionCbClientProcess(p, headers, dialog, flowp);
    SipSessionCbServiceProcess(p, headers, dialog, flowp);
}

static int sip_service_init(const InitServiceAPI * const init_api)
{
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_BANNER, SIP_BANNER_LEN, 0, svc_name);
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_INVITE_BANNER, SIP_INVITE_BANNER_LEN, 0, svc_name);
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_ACK_BANNER, SIP_ACK_BANNER_LEN, 0, svc_name);
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_REGISTER_BANNER, SIP_REGISTER_BANNER_LEN, 0, svc_name);
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_CANCEL_BANNER, SIP_CANCEL_BANNER_LEN, 0, svc_name);
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_BYE_BANNER, SIP_BYE_BANNER_LEN, 0, svc_name);
    init_api->RegisterPattern(&sip_service_validate, IPPROTO_UDP, (const uint8_t *) SIP_OPTIONS_BANNER, SIP_OPTIONS_BANNER_LEN, 0, svc_name);
	unsigned i;
	for (i=0; i < sizeof(appIdServiceRegistry)/sizeof(*appIdServiceRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdServiceRegistry[i].appId);
		init_api->RegisterAppId(&sip_service_validate, appIdServiceRegistry[i].appId, appIdServiceRegistry[i].additionalInfo, NULL);
	}

    return 0;
}

MakeRNAServiceValidationPrototype(sip_service_validate)
{
    ServiceSIPData *ss;

    ss = sip_service_mod.api->data_get(flowp);
    if (!ss)
    {
        ss = calloc(1, sizeof(*ss));
        if (!ss)
            return SERVICE_ENOMEM;
        if (sip_service_mod.api->data_add(flowp, ss, &free))
        {
            free(ss);
            return SERVICE_ENOMEM;
        }
    }

    if (size && dir == APP_ID_FROM_RESPONDER)
    {
        ss->serverPkt++;
    }

    if (ss->serverPkt > 10)
    {
        if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
        {
            sip_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
        }
        flow_clear(flowp, FLOW_CONTINUE);
        return SERVICE_NOMATCH;
    }

    if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
    {
        sip_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
    }

    return SERVICE_INPROCESS;
}

