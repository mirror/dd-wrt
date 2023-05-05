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


#ifndef __SERVICE_API_H__
#define __SERVICE_API_H__

#include <stdbool.h>

#ifdef HAVE_CONFIG_H
#include "config.h"     /* for WORDS_BIGENDIAN */
#endif
#include "sf_dynamic_preprocessor.h"
#include "appIdApi.h"
#include "service_util.h"
#include "commonAppMatcher.h"
#include "flow.h"


// Forward declaration
struct appIdConfig_;
struct _Detector;

typedef enum {
    SERVICE_SUCCESS = 0,
    SERVICE_INPROCESS = 10,
    SERVICE_NEED_REASSEMBLY = 11,
    SERVICE_NOT_COMPATIBLE = 12,
    SERVICE_INVALID_CLIENT = 13,
    SERVICE_REVERSED = 14,
    SERVICE_NOMATCH = 100,
    SERVICE_ENULL = -10,
    SERVICE_EINVALID = -11,
    SERVICE_ENOMEM = -12
} SERVICE_RETCODE;

typedef struct _ServiceValidationArgs
{
    const uint8_t *data;
    uint16_t size;
    int dir;
    tAppIdData *flowp;
    SFSnortPacket *pkt;
    struct _Detector *userdata;
    const struct appIdConfig_ *pConfig;
    bool app_id_debug_session_flag;
    char *app_id_debug_session;
} ServiceValidationArgs;
typedef int (*RNAServiceValidationFCN)(ServiceValidationArgs*);
typedef int (*RNAServiceCallbackFCN)(const uint8_t *, uint16_t, const int, tAppIdData *session,
                                     const SFSnortPacket *pkt, struct _Detector *userData,
                                     const struct appIdConfig_ *pConfig);
#define MakeRNAServiceValidationPrototype(name) static int name(ServiceValidationArgs* args)

struct _INIT_SERVICE_API;

typedef struct
{
    struct appIdConfig_ *pAppidConfig;  ///< AppId context for which this API should be used
} CleanServiceAPI;

typedef int (*RNAServiceValidationInitFCN)(const struct _INIT_SERVICE_API * const);
typedef void (*RNAServiceValidationCleanFCN)(const CleanServiceAPI *const);

struct _RNA_SERVICE_VALIDATION_PP;
struct RNAServiceValidationModule;

typedef struct _INIT_SERVICE_API
{
    void (*RegisterPattern)(RNAServiceValidationFCN fcn, uint8_t proto,
                            const uint8_t *pattern, unsigned size, int position,
                            const char *name, struct appIdConfig_ *pConfig);
    int (*AddPort)(struct _RNA_SERVICE_VALIDATION_PP *pp, struct RNAServiceValidationModule *svm, struct appIdConfig_ *pConfig);
    void (*RemovePorts)(RNAServiceValidationFCN validate, struct appIdConfig_ *pConfig);
    void (*RegisterPatternUser)(RNAServiceValidationFCN fcn, uint8_t proto,
                                const uint8_t *pattern, unsigned size, int position,
                                const char *name, struct appIdConfig_ *pConfig);
    void (*RegisterAppId)(RNAServiceValidationFCN fcn, tAppId appId, uint32_t additionalInfo, struct appIdConfig_ *pConfig);
    void (*RegisterDetectorCallback)(RNAServiceCallbackFCN fcn, tAppId appId, struct _Detector *userdata, struct appIdConfig_ *pConfig);
    int debug;
    uint32_t instance_id;
    DynamicPreprocessorData *dpd;
    struct appIdConfig_ *pAppidConfig;  ///< AppId context for which this API should be used
} InitServiceAPI;

typedef struct _RNA_SERVICE_PERF
{
    /*time to validate */
    uint64_t totalValidateTime;
} RNAServicePerf;


struct RNAServiceElement
{
    struct RNAServiceElement *next;
    RNAServiceValidationFCN validate;
    RNAServiceCallbackFCN detectorCallback;
    bool detectorContext;
    /**pointer to user data. Value of userdata pointer and validate pointer forms key for comparison.
     */
    struct _Detector *userdata;

    /**type of detector - pattern based, Sourcefire (validator) or User (Validator). */
    unsigned detectorType;

    /**Number of resources registered */
    unsigned ref_count;
    unsigned current_ref_count;

    int provides_user;

    const char *name;
};
typedef struct RNAServiceElement tRNAServiceElement;

typedef void *(*ServiceFlowdataGet)(tAppIdData *, unsigned);
typedef int (*ServiceFlowdataAdd)(tAppIdData *, void *, unsigned, AppIdFreeFCN);
typedef int (*ServiceFlowdataAddId)(tAppIdData *, uint16_t, const tRNAServiceElement * const);
typedef int (*ServiceFlowdataAddDHCP)(tAppIdData *, unsigned, const uint8_t *, unsigned, const uint8_t *, const uint8_t *);
#define APPID_EARLY_SESSION_FLAG_FW_RULE    1
typedef tAppIdData *(*ServiceCreateNewFlow)( tAppIdData *flowp, SFSnortPacket *, sfaddr_t *, uint16_t,
                                       sfaddr_t *, uint16_t, uint8_t, int16_t, int flags);
typedef void (*ServiceDhcpNewLease)(tAppIdData *flow, const uint8_t *mac, uint32_t ip, int32_t zone,
                                      uint32_t subnetmask, uint32_t leaseSecs, uint32_t router);
typedef void (*ServiceAnalyzeFP)(tAppIdData *, unsigned, unsigned, uint32_t);

typedef int (*AddService)(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                          const tRNAServiceElement *svc_element,
                          tAppId service, const char *vendor, const char *version,
                          const RNAServiceSubtype *subtype, AppIdServiceIDState *id_state);
typedef int (*AddServiceConsumeSubtype)(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                                        const tRNAServiceElement *svc_element,
                                        tAppId service, const char *vendor, const char *version,
                                        RNAServiceSubtype *subtype, AppIdServiceIDState *id_state);
typedef int (*ServiceInProcess)(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                                const tRNAServiceElement *svc_element, AppIdServiceIDState *id_state);
typedef int (*FailService)(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                           const tRNAServiceElement *svc_element, unsigned flow_data_index, const struct appIdConfig_ *pConfig, AppIdServiceIDState *id_state);
typedef int (*IncompatibleData)(tAppIdData *flow, const SFSnortPacket *pkt, int dir,
                                const tRNAServiceElement *svc_element, unsigned flow_data_index, const struct appIdConfig_ *pConfig, AppIdServiceIDState *id_state);
typedef void (*AddHostInfo)(tAppIdData *flow, SERVICE_HOST_INFO_CODE code, const void *info);
typedef void (*AddPayload)(tAppIdData *, tAppId);
typedef void (*AddMultiPayload)(tAppIdData *, tAppId);
typedef void (*AddUser)(tAppIdData *, const char *, tAppId, int);
typedef void (*AddMisc)(tAppIdData *, tAppId);
typedef void (*AddDnsQueryInfo)(tAppIdData *flow,
                                uint16_t id,
                                const uint8_t *host, uint8_t host_len, uint16_t host_offset,
                                uint16_t record_type, uint16_t options_offset, bool root_query);
typedef void (*AddDnsResponseInfo)(tAppIdData *flow,
                                   uint16_t id,
                                   const uint8_t *host, uint8_t host_len, uint16_t host_offset,
                                   uint8_t response_type, uint32_t ttl);
typedef void (*ResetDnsInfo)(tAppIdData *flow);

typedef struct _SERVICE_API
{
    ServiceFlowdataGet data_get;
    ServiceFlowdataAdd data_add;
    ServiceCreateNewFlow flow_new;
    ServiceFlowdataAddId data_add_id;
    ServiceFlowdataAddDHCP data_add_dhcp;
    ServiceDhcpNewLease dhcpNewLease;
    ServiceAnalyzeFP analyzefp;
    AddService add_service;
    FailService fail_service;
    ServiceInProcess service_inprocess;
    IncompatibleData incompatible_data;
    AddHostInfo  add_host_info;
    AddPayload add_payload;
    AddMultiPayload add_multipayload;
    AddUser add_user;
    AddServiceConsumeSubtype add_service_consume_subtype;
    AddMisc add_misc;
    AddDnsQueryInfo add_dns_query_info;
    AddDnsResponseInfo add_dns_response_info;
    ResetDnsInfo reset_dns_info;
} ServiceApi;

typedef struct _RNA_tAppIdData_STATE
{
    struct _RNA_tAppIdData_STATE *next;
    const tRNAServiceElement *svc;
    uint16_t port;
} RNAFlowState;

typedef struct _RNA_SERVICE_VALIDATION_PP
{
    RNAServiceValidationFCN validate;
    uint16_t port;
    uint8_t proto;
    uint8_t reversed_validation;
} RNAServiceValidationPort;

struct RNAServiceValidationModule
{
    const char * name;
    RNAServiceValidationInitFCN init;
    RNAServiceValidationPort *pp;
    const ServiceApi *api;
    struct RNAServiceValidationModule *next;
    int provides_user;
    RNAServiceValidationCleanFCN clean;
    unsigned flow_data_index;
};

typedef struct RNAServiceValidationModule tRNAServiceValidationModule;

#if defined(WORDS_BIGENDIAN)
#define LETOHS(p)   BYTE_SWAP_16(*((uint16_t *)(p)))
#define LETOHL(p)   BYTE_SWAP_32(*((uint32_t *)(p)))
#else
#define LETOHS(p)   (*((uint16_t *)(p)))
#define LETOHL(p)   (*((uint32_t *)(p)))
#endif

#endif /* __SERVICE_API_H__ */

