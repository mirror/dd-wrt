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


#ifndef __COMMON_APP_MATCHER_H__
#define __COMMON_APP_MATCHER_H__

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>

#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "sflsq.h"
#include "service_state.h"
#include "flow.h"
#include "appId.h"

typedef struct _appRegistryEntry
{
    tAppId appId;
    uint32_t  additionalInfo;
} tAppRegistryEntry;

#define APP_ID_MAX_DIRS 16
#define MAX_ZONES   1024
#define APP_ID_PORT_ARRAY_SIZE  65536

typedef struct _port_ex
{
    int family;
    struct in6_addr ip;
    struct in6_addr netmask;
} PortExclusion;

typedef struct APP_ID_CONFIG
{
    unsigned mdns_user_reporting;
    unsigned referred_appId_disabled;
    unsigned rtmp_max_packets;
    const char *appid_directory;  
    tAppId tcp_port_only[65536];      /* Service IDs for port-only TCP services */
    tAppId udp_port_only[65536];      /* Service IDs for port-only UDP services */
    tAppId ip_protocol[255];          /* Service IDs for non-TCP / UDP protocol services */

    SF_LIST client_app_args;            /* List of Client App arguments */
} tAppIdConfig;

extern unsigned appIdPolicyId;
extern tAppIdConfig appIdConfig;

int appMatcherIsAppDetected(void *appSet, tAppId app);
int AppIdCommonInit(unsigned long memcap);
int AppIdCommonFini(void);
int AppIdCommonReload(void);

void *AppIDFlowdataGet(FLOW *flowp, unsigned id);

#endif  /* __COMMON_APP_MATCHER_H__ */

