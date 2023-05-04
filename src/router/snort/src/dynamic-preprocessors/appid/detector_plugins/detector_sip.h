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


#ifndef __DETECTOR_SIP_H__
#define __DETECTOR_SIP_H__

#include "client_app_api.h"
#include "service_api.h"
#include "sf_multi_mpse.h"
#include "sf_mlmp.h"

struct RNAServiceValidationModule;

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

typedef struct
{
    void                    *sipUaMatcher;
    tDetectorAppSipPattern  *appSipUaList;
    void                    *sipServerMatcher;
    tDetectorAppSipPattern  *appSipServerList;
} tDetectorSipConfig;

extern struct RNAClientAppModule sip_udp_client_mod;
extern struct RNAClientAppModule sip_tcp_client_mod;
extern struct RNAServiceValidationModule sip_service_mod;

#endif  /* __DETECTOR_SIP_H__ */

