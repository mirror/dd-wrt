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


#ifndef _FLOW_H
#define _FLOW_H

#include <stdint.h>
#include <time.h>

#include "sf_snort_packet.h"
#include "flow_error.h"
#include "appId.h"
#include "service_state.h"

#define SF_DEBUG_FILE   stdout

typedef enum
{
    APPINFO_FLAG_SERVICE_ADDITIONAL = 0x1,
    APPINFO_FLAG_SERVICE_UDP_REVERSED = 0x2,
    APPINFO_FLAG_CLIENT_ADDITIONAL = 0x4,
    APPINFO_FLAG_CLIENT_USER = 0x8,
    APPINFO_FLAG_ACTIVE = 0x10
} tAppInfoFlags;

#define FLOW_DATA_ID_SVC_FLOWSTATE      0
#define FLOW_DATA_CLIENT_APP_MODSTATE   1
#define FLOW_DATA_DHCP_FP_DATA          2
#define FLOW_DATA_SMB_DATA              4
#define FLOW_DATA_DHCP_INFO             5
#define FLOW_DATA_DETECTOR_MODSTATE_BIT 0x80000000

typedef enum {
    APP_ID_FROM_INITIATOR,
    APP_ID_FROM_RESPONDER,
    APP_ID_FLOW_DIRECTION_MAX /* Maximum value of a direction (must be last in the list */
} FLOW_DIRECTION;

/* flow flags */
typedef enum {
    /**Service protocol was detected */
    FLOW_SERVICEDETECTED = 0x00000008,
    /**Continue calling the routine after the service has been identified. */
    FLOW_CONTINUE = 0x40000000,
    FLOW_HAS_DHCP_FP = 0x20000000,
    /**Flow is a data connection not a service */
    FLOW_NOT_A_SERVICE = 0x10000000,
    FLOW_HAS_DHCP_INFO = 0x08000000,
    /**Finsihed with client app detection */
    FLOW_CLIENTAPPDETECTED = 0x04000000,
    /**Call service detection even if the host does not exist */
    FLOW_IGNORE_HOST = 0x02000000,
    /**Service protocol had incompatible client data */
    FLOW_INCOMPATIBLE = 0x01000000,
    /**Service missed the first UDP packet in a flow. This causes detectors to see traffic in reverse direction.
     * Detectors should set this flag by verifying that packet from initiator is indeed a packet from responder.
     * Setting this flag without this check will cause RNA to not try other detectors in some cases (see bug 77551).*/
    FLOW_UDP_REVERSED = 0x00800000,
    /**we are ready to see out of network Server packets */
    FLOW_CLIENT_GETS_SERVER_PACKETS = 0x00400000,

    FLOW_RESPONDER_SEEN = 0x00200000,
    FLOW_INITIATOR_SEEN = 0x00100000,

    FLOW_DISCOVER_APP = 0x00010000,

    FLOW_MID = 0x00008000,
    FLOW_SERVICEDELETED = 0x00004000,
    FLOW_HAS_SMB_INFO = 0x00002000,
    FLOW_PORT_SERVICE_DONE = 0x00001000,
    FLOW_RESPONSE_CODE_CHECKED = 0x00000800,
    FLOW_ADDITIONAL_PACKET = 0x00000400,
    FLOW_HTTP_SESSION = 0x00000080,
    FLOW_SSL_SESSION = 0x00000040,
    FLOW_LOGIN_SUCCEEDED = 0x00000020,
    FLOW_APP_REINSPECT = 0x00000010,
    FLOW_OOO = 0x00000004,
    FLOW_SYN_RST = 0x00000002,
    FLOW_REXEC_STDERR = 0x00000001
} tFlowFlags;

struct _RNA_SERVICE_ELEMENT;
struct _RNA_CLIENT_APP_MODULE;

typedef enum
{
    RNA_STATE_NONE = 0,
    RNA_STATE_DIRECT,
    RNA_STATE_STATEFUL,
    RNA_STATE_FINISHED
} RNA_INSPECTION_STATE;

typedef void (*AppIdFreeFCN)(void *);

#define FINGERPRINT_UDP_FLAGS_XENIX 0x00000800
#define FINGERPRINT_UDP_FLAGS_NT    0x00001000
#define FINGERPRINT_UDP_FLAGS_MASK  (FINGERPRINT_UDP_FLAGS_XENIX | FINGERPRINT_UDP_FLAGS_NT)

typedef struct _FpSMBData
{
    struct _FpSMBData *next;
    unsigned major;
    unsigned minor;
    uint32_t flags;
} FpSMBData;

typedef struct _DHCPInfo
{
    struct _DHCPInfo *next;
    uint32_t ipAddr;
    uint8_t  macAddr[6];
    uint32_t subnetmask;
    uint32_t leaseSecs;
    uint32_t router;
} DHCPInfo;

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

typedef struct _AppIdFlowData
{
    struct _AppIdFlowData *next;
    unsigned fd_id;
    void *fd_data;
    AppIdFreeFCN fd_free;
} AppIdFlowData;


typedef enum
{
    FLOW_TYPE_IGNORE,
    FLOW_TYPE_NORMAL,
    FLOW_TYPE_TMP
} FLOW_STRUCT_TYPE;

typedef struct _FLOW_STRUCT_FLAG
{
    FLOW_STRUCT_TYPE flow_type;
} FLOW_STRUCT_FLAG;

typedef struct _tCommonAppIdData
{
    FLOW_STRUCT_FLAG fsf_type;  /* This must be first. */
    unsigned policyId;
    unsigned flow_flags;
    snort_ip initiator_ip;
    uint16_t initiator_port;
} tCommonAppIdData;

typedef struct _tTmpAppIdData
{
    tCommonAppIdData common;

    struct _tTmpAppIdData *next;
} tTmpAppIdData;

#define SCAN_HTTP_VIA_FLAG          0x1
#define SCAN_HTTP_USER_AGENT_FLAG   0x2
#define SCAN_HTTP_HOST_URL_FLAG     0x4
#define SCAN_USER_AGENT_HOST_FLAG   0x8
#define SCAN_SSL_HOST_FLAG          0x10
#define SCAN_HOST_PORT_FLAG         0x20

#define RESPONSE_CODE_PACKET_THRESHHOLD 0

struct _RNAServiceSubtype;

typedef struct _tlsSession
{
    char *tls_host;
    int   tls_host_strlen;
    char *tls_cname;
    int   tls_cname_strlen;
    char *tls_orgUnit;
    int   tls_orgUnit_strlen;
} tlsSession;

typedef struct _tAppIdData
{
    tCommonAppIdData common;

    struct _tAppIdData *next;

    void *ssn;
    snort_ip service_ip;
    uint16_t service_port;
    uint8_t proto;
    uint8_t previous_tcp_flags;

    AppIdFlowData *flowData;

    /**AppId matching service side */
    tAppId serviceAppId;
    tAppId portServiceAppId;
    /**RNAServiceElement for identifying detector*/
    const struct _RNA_SERVICE_ELEMENT *serviceData;
    RNA_INSPECTION_STATE rnaServiceState;
    char *serviceVendor;
    char *serviceVersion;
    struct _RNAServiceSubtype *subtype;
    AppIdServiceIDState *id_state;
    char *netbios_name;

    /**AppId matching client side */
    tAppId clientAppId;
    tAppId clientServiceAppId;
    char *clientVersion;
    /**_RNA_CLIENT_APP_MODULE for identifying client detector*/
    const struct _RNA_CLIENT_APP_MODULE *clientData;
    RNA_INSPECTION_STATE rnaClientState;

    /**AppId matching payload*/
    tAppId payloadAppId;
    tAppId referredPayloadAppId;
    tAppId miscAppId;

    char *username;
    tAppId usernameService;

    char *netbiosDomain;

    uint32_t flowId;

    tlsSession  *tsession;

    char *host;
    char *url;
    char *via;
    char *useragent;
    char *response_code;
    char *referer;
    unsigned scan_flags;
#if RESPONSE_CODE_PACKET_THRESHHOLD
    unsigned response_code_packets;
#endif

    tAppId referredAppId;

    tAppId tmpAppId;
    char *payloadVersion;

    uint16_t session_packet_count;
    int16_t snortId;

    struct 
    {
        uint32_t    firstPktsecond;
        uint32_t    lastPktsecond;
        uint64_t    initiatorBytes;
        uint64_t    responderBytes;
    } stats;

} tAppIdData;

typedef tAppIdData FLOW;

/**
 * Mark a flow with a particular flag
 *
 * @param flow
 * @param flags
 */
static inline void flow_mark(tAppIdData *flow, unsigned flags)
{
    flow->common.flow_flags |= flags;
}

/**
 * Mark a flow with a particular flag
 *
 * @param flow
 * @param flags
 */
static inline void flow_clear(tAppIdData *flow, unsigned flags)
{
    flow->common.flow_flags &= ~flags;
}

/**
 * Check to see if a particular flag exists
 *
 * @param flow
 * @param flags
 */
static inline unsigned flow_checkflag(tAppIdData *flow, unsigned flags)
{
    return (flow->common.flow_flags & flags);
}

void AppIdFlowdataFree(tAppIdData *flowp);
void AppIdFlowdataFini(void);
void *AppIdFlowdataGet(tAppIdData *flowp, unsigned id);
int AppIdFlowdataAdd(tAppIdData *flowp, void *data, unsigned id, AppIdFreeFCN fcn);
void *AppIdFlowdataRemove(tAppIdData *flowp, unsigned id);
void AppIdFlowdataDelete(tAppIdData *flowp, unsigned id);
tAppIdData *AppIdEarlySessionCreate(const SFSnortPacket *ctrlPkt, snort_ip *cliIp, uint16_t cliPort,
                          snort_ip *srvIp, uint16_t srvPort, uint8_t proto, int16_t app_id);
struct _RNA_SERVICE_ELEMENT;
int AppIdFlowdataAddId(tAppIdData *flowp, uint16_t port, const struct _RNA_SERVICE_ELEMENT *svc_element);

#endif /* _FLOW_H */

