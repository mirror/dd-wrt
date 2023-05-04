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

#ifndef CLIENT_APP_CONFIG_H_
#define CLIENT_APP_CONFIG_H_

/****************************** INCLUDES **************************************/

#include "client_app_api.h"

/********************************* TYPES **************************************/

struct RNAClientAppModule;
struct RNAClientAppRecord;

typedef struct ClientPatternData
{
    struct ClientPatternData   *next;
    int                         position;
    const struct RNAClientAppModule  *ca;
} tClientPatternData;
//typedef struct ClientPatternData tClientPatternData;

typedef struct ClientAppConfig
{
    struct RNAClientAppRecord_  *tcp_client_app_list;   ///< List of all TCP client apps (C and Lua)
    struct RNAClientAppRecord_  *udp_client_app_list;   ///< List of all UDP client apps (C and Lua)
    int                 enabled;
    SF_LIST             module_configs;
    tClientPatternData  *pattern_data_list;
    void                *tcp_patterns;
    int                 tcp_pattern_count;
    void                *udp_patterns;
    int                 udp_pattern_count;
} tClientAppConfig;

#endif // CLIENT_APP_CONFIG_H_
