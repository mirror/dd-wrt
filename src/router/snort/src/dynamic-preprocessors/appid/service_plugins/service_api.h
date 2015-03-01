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


#ifndef __SERVICE_API_H__
#define __SERVICE_API_H__

#include <stdbool.h>

#include "sf_dynamic_preprocessor.h"
#include "service_util.h"
#include "commonAppMatcher.h"
#include "flow.h"

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

typedef enum {
    SERVICE_HOST_INFO_NETBIOS_NAME = 1
} SERVICE_HOST_INFO_CODE;

typedef struct _RNAServiceSubtype
{
    struct _RNAServiceSubtype *next;
    const char *service;
    const char *vendor;
    const char *version;
} RNAServiceSubtype;


typedef int (*RNAServiceValidationFCN)(const uint8_t *, uint16_t, const int,
                                       tAppIdData *, const SFSnortPacket *, struct _Detector *);
#define MakeRNAServiceValidationPrototype(name) static int name(const uint8_t *data, uint16_t size, const int dir, \
                                                                FLOW *flowp, const SFSnortPacket *pkt, \
                                                                struct _Detector *userdata)

struct _INIT_SERVICE_API;
typedef int (*RNAServiceValidationInitFCN)(const struct _INIT_SERVICE_API * const);
typedef void (*RNAServiceValidationCleanFCN)(void);

struct _RNA_SERVICE_VALIDATION_PP;
struct _RNA_SERVICE_VALIDATION_MODULE;

typedef struct _INIT_SERVICE_API
{
    void (*RegisterPattern)(RNAServiceValidationFCN fcn, uint8_t proto,
                            const uint8_t *pattern, unsigned size, int position,
                            const char *name);
    int (*AddPort)(struct _RNA_SERVICE_VALIDATION_PP *pp, struct _RNA_SERVICE_VALIDATION_MODULE *svm);
    void (*RemovePorts)(RNAServiceValidationFCN validate);
    void (*RegisterPatternUser)(RNAServiceValidationFCN fcn, uint8_t proto,
                                const uint8_t *pattern, unsigned size, int position,
                                const char *name);
    void (*RegisterAppId)(RNAServiceValidationFCN fcn, tAppId appId, uint32_t additionalInfo, struct _Detector *userdata);
    int debug;
    uint32_t instance_id;
    tAppId *service_instance;
    const char *csd_path;
    DynamicPreprocessorData *dpd;
} InitServiceAPI;

typedef struct _RNA_SERVICE_PERF
{
    /*time to validate */
    uint64_t totalValidateTime;
} RNAServicePerf;


typedef struct _RNA_SERVICE_ELEMENT
{
    struct _RNA_SERVICE_ELEMENT *next;
    RNAServiceValidationFCN validate;
    /**pointer to user data. Value of userdata pointer and validate pointer forms key for comparison.
     */
    struct _Detector *userdata;

    /**type of detector - pattern based, Sourcefire (validator) or User (Validator). */
    unsigned detectorType;

    /**Number of resources registered */
    unsigned ref_count;

    int provides_user;

    const char *name;
} RNAServiceElement;

typedef void *(*ServiceFlowdataGet)(FLOW *);
typedef int (*ServiceFlowdataAdd)(FLOW *, void *, AppIdFreeFCN);
typedef int (*ServiceFlowdataAddId)(FLOW *, uint16_t, const RNAServiceElement * const);
typedef int (*ServiceFlowdataAddDHCP)(FLOW *, unsigned, const uint8_t *, unsigned, const uint8_t *, const uint8_t *);
typedef FLOW *(*ServiceCreateNewFlow)( const SFSnortPacket *, snort_ip *, uint16_t,
                                       snort_ip *, uint16_t, uint8_t, int16_t);
typedef void (*ServiceDhcpNewLease)(FLOW *flow, const uint8_t *mac, uint32_t ip, int32_t zone,
                                      uint32_t subnetmask, uint32_t leaseSecs, uint32_t router);
typedef void (*ServiceAnalyzeFP)(FLOW *, unsigned, unsigned, uint32_t);

typedef int (*AddService)(FLOW *flow, const SFSnortPacket *pkt, int dir,
                          const RNAServiceElement *svc_element,
                          tAppId service, const char *vendor, const char *version,
                          const RNAServiceSubtype *subtype);
typedef int (*AddServiceConsumeSubtype)(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                        const RNAServiceElement *svc_element,
                                        tAppId service, const char *vendor, const char *version,
                                        RNAServiceSubtype *subtype);
typedef int (*ServiceInProcess)(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                const RNAServiceElement *svc_element);
typedef int (*FailService)(FLOW *flow, const SFSnortPacket *pkt, int dir,
                           const RNAServiceElement *svc_element);
typedef int (*IncompatibleData)(FLOW *flow, const SFSnortPacket *pkt, int dir,
                                const RNAServiceElement *svc_element);
typedef void (*AddHostInfo)(FLOW *flow, SERVICE_HOST_INFO_CODE code, const void *info);
typedef void (*AddPayload)(FLOW *, tAppId);
typedef void (*AddUser)(FLOW *, const char *, tAppId, int);
typedef void (*AddMisc)(FLOW *, tAppId);

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
    AddUser add_user;
    AddServiceConsumeSubtype add_service_consume_subtype;
    AddMisc add_misc;
} ServiceApi;

typedef struct _RNA_FLOW_STATE
{
    struct _RNA_FLOW_STATE *next;
    const RNAServiceElement *svc;
    uint16_t port;
} RNAFlowState;

typedef struct _RNA_SERVICE_VALIDATION_PP
{
    RNAServiceValidationFCN validate;
    uint16_t port;
    uint8_t proto;
    uint8_t reversed_validation;
} RNAServiceValidationPort;

typedef struct _RNA_SERVICE_VALIDATION_MODULE
{
    const char * name;
    RNAServiceValidationInitFCN init;
    RNAServiceValidationPort *pp;
    const ServiceApi *api;
    int is_custom;
    struct _RNA_SERVICE_VALIDATION_MODULE *next;
    int provides_user;
    RNAServiceValidationCleanFCN clean;
} RNAServiceValidationModule;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LETOHS(p)   (*((uint16_t *)(p)))
#define LETOHL(p)   (*((uint32_t *)(p)))
#else
#include <byteswap.h>
#define LETOHS(p)   bswap_16(*((uint16_t *)(p)))
#define LETOHL(p)   bswap_32(*((uint32_t *)(p)))
#endif

#endif /* __SERVICE_API_H__ */

