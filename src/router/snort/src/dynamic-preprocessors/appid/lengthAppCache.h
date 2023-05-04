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

#ifndef _LENGTH_APP_CACHE_
#define _LENGTH_APP_CACHE_

#include "appId.h"
#include "appIdApi.h"

#define LENGTH_SEQUENCE_CNT_MAX (5)

#pragma pack(1)

// Forward declaration for AppId config. Cannot include appIdConfig.h because of
// circular dependency
struct appIdConfig_;

typedef struct _tLengthSequenceEntry {
    uint8_t  direction;    /* APP_ID_FROM_INITIATOR or APP_ID_FROM_RESPONDER */
    uint16_t length;       /* payload size (bytes) */
} tLengthSequenceEntry;

typedef struct _tLengthKey {
    uint8_t              proto;           /* IPPROTO_TCP or IPPROTO_UDP */
    uint8_t              sequence_cnt;    /* num valid entries in sequence */
    tLengthSequenceEntry sequence[LENGTH_SEQUENCE_CNT_MAX];
} tLengthKey;

#pragma pack()
void lengthAppCacheInit(struct appIdConfig_ *pConfig);
void lengthAppCacheFini(struct appIdConfig_ *pConfig);
tAppId lengthAppCacheFind(const tLengthKey *key, const struct appIdConfig_ *pConfig);
int lengthAppCacheAdd(const tLengthKey *key, tAppId val, struct appIdConfig_ *pConfig);

#endif
