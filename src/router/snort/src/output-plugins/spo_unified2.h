/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */

#ifndef __SPO_UNIFIED2_H__
#define __SPO_UNIFIED2_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef WIN32
#include <netinet/in.h>
#endif
#include "decode.h" /* for struct in6_addr -- maybe move to sf_types.h? */
#include "sf_types.h"

typedef struct _PESessionEndRecord
{
    uint32_t    sensorId;
    uint8_t     ingressZone[16];
    uint8_t     egressZone[16];
    uint8_t     ingressIntf[16];
    uint8_t     egressIntf[16];
    uint8_t     initiatorIp[16];
    uint8_t     responderIp[16];
    uint8_t     policyRevision[16];
    uint32_t    policyengine_ruleId;
    uint32_t    policyengine_ruleAction;
    uint16_t    initiatorPort;
    uint16_t    responderPort;
    uint16_t    tcpFlags;
    uint8_t     protocol;
    uint8_t     padding;
    uint8_t     netflowSource[16];
    uint32_t    firstPktsecond;
    uint32_t    lastPktsecond;
    uint64_t    initiatorPkts;
    uint64_t    responderPkts;
    uint64_t    initiatorBytes;
    uint64_t    responderBytes;
    uint32_t    appProtoId;
    uint32_t    webAppId;
    uint32_t    userId;
    uint32_t    urlCategory;
    uint32_t    urlReputation;
    uint32_t    clientId;

} PESessionRecord;

void Unified2Setup(void);

#endif  /* __SPO_UNIFIED_H__ */
