/******************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2009-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/

#ifndef __APPID_API_H__
#define __APPID_API_H__

#include "stdint.h"
#include "stdbool.h"
#include "ipv6_port.h"
#include "sfghash.h"

struct AppIdData;

typedef int32_t tAppId;

#define APPID_SESSION_RESPONDER_MONITORED   (1ULL << 0)
#define APPID_SESSION_INITIATOR_MONITORED   (1ULL << 1)
#define APPID_SESSION_SPECIAL_MONITORED     (1ULL << 2)
#define APPID_SESSION_INITIATOR_SEEN        (1ULL << 3)
#define APPID_SESSION_RESPONDER_SEEN        (1ULL << 4)
#define APPID_SESSION_DISCOVER_USER         (1ULL << 5)
#define APPID_SESSION_HAS_DHCP_FP           (1ULL << 6)
#define APPID_SESSION_HAS_DHCP_INFO         (1ULL << 7)
#define APPID_SESSION_HAS_SMB_INFO          (1ULL << 8)
#define APPID_SESSION_MID                   (1ULL << 9)
#define APPID_SESSION_OOO                   (1ULL << 10)
#define APPID_SESSION_SYN_RST               (1ULL << 11)

    /**Service missed the first UDP packet in a flow. This causes detectors to see traffic in reverse direction.
     * Detectors should set this flag by verifying that packet from initiator is indeed a packet from responder.
     * Setting this flag without this check will cause RNA to not try other detectors in some cases (see bug 77551).*/
#define APPID_SESSION_UDP_REVERSED          (1ULL << 12)
#define APPID_SESSION_HTTP_SESSION          (1ULL << 13)

    /**Service protocol was detected */
#define APPID_SESSION_SERVICE_DETECTED      (1ULL << 14)

    /**Finsihed with client app detection */
#define APPID_SESSION_CLIENT_DETECTED       (1ULL << 15)
    /**Flow is a data connection not a service */
#define APPID_SESSION_NOT_A_SERVICE         (1ULL << 16)

#define APPID_SESSION_DECRYPTED             (1ULL << 17)
#define APPID_SESSION_SERVICE_DELETED       (1ULL << 18)

    //The following attributes are references only with appId
    /**Continue calling the routine after the service has been identified. */
#define APPID_SESSION_CONTINUE              (1ULL << 19)
    /**Call service detection even if the host does not exist */
#define APPID_SESSION_IGNORE_HOST           (1ULL << 20)
    /**Service protocol had incompatible client data */
#define APPID_SESSION_INCOMPATIBLE          (1ULL << 21)
    /**we are ready to see out of network Server packets */
#define APPID_SESSION_CLIENT_GETS_SERVER_PACKETS    (1ULL << 22)

#define APPID_SESSION_DISCOVER_APP          (1ULL << 23)

#define APPID_SESSION_PORT_SERVICE_DONE     (1ULL << 24)
#define APPID_SESSION_ADDITIONAL_PACKET     (1ULL << 25)
#define APPID_SESSION_RESPONDER_CHECKED     (1ULL << 26)
#define APPID_SESSION_INITIATOR_CHECKED     (1ULL << 27)
#define APPID_SESSION_SSL_SESSION           (1ULL << 28)
#define APPID_SESSION_LOGIN_SUCCEEDED       (1ULL << 29)

#define APPID_SESSION_SPDY_SESSION          (1ULL << 30)
#define APPID_SESSION_ENCRYPTED             (1ULL << 31)

#define APPID_SESSION_APP_REINSPECT         (1ULL << 32)
#define APPID_SESSION_RESPONSE_CODE_CHECKED (1ULL << 33)
#define APPID_SESSION_REXEC_STDERR          (1ULL << 34)
#define APPID_SESSION_CHP_INSPECTING        (1ULL << 35)
#define APPID_SESSION_STICKY_SERVICE        (1ULL << 36)
#define APPID_SESSION_APP_REINSPECT_SSL     (1ULL << 37)

#define APPID_SESSION_NO_TPI                (1ULL << 38)
#define APPID_SESSION_IGNORE_FLOW           (1ULL << 39)
#define APPID_SESSION_IGNORE_FLOW_LOGGED    (1ULL << 40)

#define APPID_SESSION_EXPECTED_EVALUATE     (1ULL << 41)
#define APPID_SESSION_HOST_CACHE_MATCHED    (1ULL << 42)
#define APPID_SESSION_OOO_CHECK_TP          (1ULL << 43)

#define APPID_SESSION_HTTP_TUNNEL           (1ULL << 44)
#define APPID_SESSION_HTTP_CONNECT          (1ULL << 45)

#define APPID_SESSION_IGNORE_ID_FLAGS       (APPID_SESSION_IGNORE_FLOW | \
                                             APPID_SESSION_NOT_A_SERVICE | \
                                             APPID_SESSION_NO_TPI | \
                                             APPID_SESSION_SERVICE_DETECTED | \
                                             APPID_SESSION_PORT_SERVICE_DONE)

typedef enum
{
    APPID_FLOW_TYPE_IGNORE,
    APPID_FLOW_TYPE_NORMAL,
    APPID_FLOW_TYPE_TMP
} APPID_FLOW_TYPE;

typedef struct _RNAServiceSubtype
{
    struct _RNAServiceSubtype *next;
    const char *service;
    const char *vendor;
    const char *version;
} RNAServiceSubtype;

#define DHCP_OP55_MAX_SIZE  64
#define DHCP_OP60_MAX_SIZE  64

typedef struct _DHCP_FP_DATA
{
    struct _DHCP_FP_DATA *next;
    unsigned op55_len;
    unsigned op60_len;
    uint8_t op55[DHCP_OP55_MAX_SIZE];
    uint8_t op60[DHCP_OP60_MAX_SIZE];
    uint8_t mac[6];
} DhcpFPData;

typedef struct _DHCPInfo
{
    struct _DHCPInfo *next;
    uint32_t ipAddr;
    uint8_t  macAddr[6];
    uint32_t subnetmask;
    uint32_t leaseSecs;
    uint32_t router;
} DHCPInfo;

typedef struct _FpSMBData
{
    struct _FpSMBData *next;
    unsigned major;
    unsigned minor;
    uint32_t flags;
} FpSMBData;

//maximum number of appIds replicated for a flow/session
#define APPID_HA_SESSION_APP_NUM_MAX 8
#define APPID_HA_FLAGS_APP (1<<0)
#define APPID_HA_FLAGS_TP_DONE (1<<1)
#define APPID_HA_FLAGS_SVC_DONE (1<<2)
#define APPID_HA_FLAGS_HTTP (1<<3)

typedef struct _AppIdSessionHA
{
    uint16_t flags;
    tAppId appId[APPID_HA_SESSION_APP_NUM_MAX];
} AppIdSessionHA;

typedef enum
{
    NOT_A_SEARCH_ENGINE,
    SUPPORTED_SEARCH_ENGINE,
    UNSUPPORTED_SEARCH_ENGINE,
    SEARCH_SUPPORT_TYPE_UNKNOWN,
} SEARCH_SUPPORT_TYPE;

typedef enum
{
    REQ_AGENT_FID       = 0,
    REQ_HOST_FID        = 1,
    REQ_REFERER_FID     = 2,
    REQ_URI_FID         = 3,
    REQ_COOKIE_FID      = 4,
    REQ_BODY_FID        = 5,
    RSP_CONTENT_TYPE_FID = 6,
    RSP_LOCATION_FID    = 7,
    RSP_BODY_FID        = 8,
    HTTP_FIELD_MAX      = RSP_BODY_FID
} HTTP_FIELD_ID;

/*******************************************************************************
 * AppId API
 ******************************************************************************/
struct AppIdApi
{
    const char * (*getApplicationName)(int32_t appId);
    tAppId (*getApplicationId)(const char *appName);

    tAppId (*getServiceAppId)(struct AppIdData *session);
    tAppId (*getPortServiceAppId)(struct AppIdData *session);
    tAppId (*getOnlyServiceAppId)(struct AppIdData *session);
    tAppId (*getMiscAppId)(struct AppIdData *session);
    tAppId (*getClientAppId)(struct AppIdData *session);
    tAppId (*getPayloadAppId)(struct AppIdData *session);
    tAppId (*getReferredAppId)(struct AppIdData *session);
    tAppId (*getFwServiceAppId)(struct AppIdData *session);
    tAppId (*getFwMiscAppId)(struct AppIdData *session);
    tAppId (*getFwClientAppId)(struct AppIdData *session);
    tAppId (*getFwPayloadAppId)(struct AppIdData *session);
    tAppId (*getFwReferredAppId)(struct AppIdData *session);
    SFGHASH*(*getFwMultiPayloadList)(struct AppIdData *session);

    bool (*isSessionSslDecrypted)(struct AppIdData *session);
    bool (*isAppIdInspectingSession)(struct AppIdData *session);
    bool (*isAppIdAvailable)(struct AppIdData *session);

    char* (*getUserName)(struct AppIdData *session, tAppId *service, bool *isLoginSuccessful);
    char* (*getClientVersion)(struct AppIdData *session);

    uint64_t (*getAppIdSessionAttribute)(struct AppIdData *session, uint64_t flag);

    APPID_FLOW_TYPE (*getFlowType)(struct AppIdData *session);
    void (*getServiceInfo)(struct AppIdData *session, char **serviceVendor, char **serviceVersion, RNAServiceSubtype **subtype);
    short (*getServicePort)(struct AppIdData *session);
    sfaddr_t* (*getServiceIp)(struct AppIdData *session);
    struct in6_addr* (*getInitiatorIp)(struct AppIdData *session);

    char* (*getHttpUserAgent)(struct AppIdData *session);
    char* (*getHttpHost)(struct AppIdData *session);
    char* (*getHttpUrl)(struct AppIdData *session);
    char* (*getHttpReferer)(struct AppIdData *session);
    char* (*getHttpNewUrl)(struct AppIdData *session);
    char* (*getHttpUri)(struct AppIdData *session);
    char* (*getHttpResponseCode)(struct AppIdData *session);
    char* (*getHttpCookie)(struct AppIdData *session);
    char* (*getHttpNewCookie)(struct AppIdData *session);
    char* (*getHttpContentType)(struct AppIdData *session);
    char* (*getHttpLocation)(struct AppIdData *session);
    char* (*getHttpBody)(struct AppIdData *session);
    char* (*getHttpReqBody)(struct AppIdData *session);
    uint16_t (*getHttpUriOffset)(struct AppIdData *session);
    uint16_t (*getHttpUriEndOffset)(struct AppIdData *session);
    uint16_t (*getHttpCookieOffset)(struct AppIdData *session);
    uint16_t (*getHttpCookieEndOffset)(struct AppIdData *session);
    SEARCH_SUPPORT_TYPE (*getHttpSearch)(struct AppIdData *session);
    sfaddr_t* (*getHttpXffAddr)(struct AppIdData *session);

    char* (*getTlsHost)(struct AppIdData *session);

    DhcpFPData* (*getDhcpFpData)(struct AppIdData *session);
    void (*freeDhcpFpData)(struct AppIdData *session, DhcpFPData *data);
    DHCPInfo* (*getDhcpInfo)(struct AppIdData *session);
    void (*freeDhcpInfo)(struct AppIdData *session, DHCPInfo *data);
    FpSMBData* (*getSmbFpData)(struct AppIdData *session);
    void (*freeSmbFpData)(struct AppIdData *session, FpSMBData *data);
    char* (*getNetbiosName)(struct AppIdData *session);
    uint32_t (*produceHAState)(void *lwssn, uint8_t *buf);
    uint32_t (*consumeHAState)(void *lwssn, const uint8_t *buf, uint8_t length, uint8_t proto, const struct in6_addr* ip, uint16_t initiatorPort);
    struct AppIdData * (*getAppIdData)(void *lwssn);
    int (*getAppIdSessionPacketCount)(struct AppIdData *appIdData);

    char* (*getDNSQuery)(struct AppIdData *appIdData, uint8_t *query_len, bool *got_response);
    uint16_t (*getDNSQueryoffset)(struct AppIdData *appIdData);
    uint16_t (*getDNSRecordType)(struct AppIdData *appIdData);
    uint8_t (*getDNSResponseType)(struct AppIdData *appIdData);
    uint32_t (*getDNSTTL)(struct AppIdData *appIdData);
    uint16_t (*getDNSOptionsOffset)(struct AppIdData *appIdData);
    char* (*getHttpNewField)(struct AppIdData *session, HTTP_FIELD_ID fieldId);
    void (*freeHttpNewField)(struct AppIdData *appIdData, HTTP_FIELD_ID fieldId);
    uint16_t (*getHttpFieldOffset)(struct AppIdData *session, HTTP_FIELD_ID fieldId);
    uint16_t (*getHttpFieldEndOffset)(struct AppIdData *session, HTTP_FIELD_ID fieldId);
    bool (*isHttpInspectionDone)(struct AppIdData *session);
    void (*dumpDebugHostInfo)(void);
};

/* For access when including header */
extern struct AppIdApi appIdApi;

//#define UNIT_TESTING // NOTE These testing #define's are used in service_base.c and fw_appid.c
//#define UNIT_TEST_FIRST_DECRYPTED_PACKET 12 // WARNING this assumes a single stream in a decrypted pcap

#endif  /* __APPID_API_H__ */

