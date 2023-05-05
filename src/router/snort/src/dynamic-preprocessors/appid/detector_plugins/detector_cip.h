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

/**
 * @author RA/Cisco
 */

#ifndef DETECTOR_CIP_H
#define DETECTOR_CIP_H

#include <stdbool.h>     // For bool in stream_api.h
#include <stdint.h>      // For C integer types
#include <sf_types.h>    // For tAppId

#include "appIdConfig.h" // For appidStaticConfig
#include "stream_api.h"  // For ServiceEventType
#include "fw_appid.h"    // For AppIdAddMultiPayload
#include "client_app_api.h" // For RNAClientAppModule
#include "service_api.h"    // For RNAServiceValidationModule

// ENIP Command Data
typedef struct _EnipCommandData
{
    tAppId app_id;

    uint16_t command_id;
} EnipCommandData;

typedef struct _EnipCommandList
{
    EnipCommandData data;

    struct _EnipCommandList *next;
} EnipCommandList;

// CIP Path Data
typedef struct _CipPathData
{
    tAppId app_id;

    uint32_t class_id;
    uint8_t service_id;
} CipPathData;

typedef struct _CipPathList
{
    CipPathData data;

    struct _CipPathList *next;
} CipPathList;

// CIP Set Attribute Data
typedef struct _CipSetAttributeData
{
    tAppId app_id;

    uint32_t class_id;
    bool is_class_instance;
    uint32_t attribute_id;
} CipSetAttributeData;

typedef struct _CipSetAttributeList
{
    CipSetAttributeData data;

    struct _CipSetAttributeList *next;
} CipSetAttributeList;

// CIP Connection Class Data
typedef struct _CipConnectionClassData
{
    tAppId app_id;

    uint32_t class_id;
} CipConnectionClassData;

typedef struct _CipConnectionClassList
{
    CipConnectionClassData data;

    struct _CipConnectionClassList *next;
} CipConnectionClassList;

// CIP Service Data
typedef struct _CipServiceData
{
    tAppId app_id;

    uint8_t service_id;
} CipServiceData;

typedef struct _CipServiceList
{
    CipServiceData data;

    struct _CipServiceList *next;
} CipServiceList;

// CIP registration data struct
typedef struct _CipPatternLists
{
    EnipCommandList *enip_command_list;
    CipPathList *path_list;
    CipSetAttributeList *set_attribute_list;
    CipConnectionClassList *connection_list;
    CipServiceList *symbol_list;
    CipServiceList *service_list;
} CipPatternLists;

// CIP Registration
int CipAddEnipCommand(tAppId app_id, uint16_t command_id);
int CipAddPath(tAppId app_id,
    uint32_t class_id,
    uint8_t service_id);
int CipAddSetAttribute(tAppId app_id,
    uint32_t class_id,
    bool is_class_instance,
    uint32_t attribute_id);
int CipAddConnectionClass(tAppId app_id, uint32_t class_id);
int CipAddExtendedSymbolService(tAppId app_id, uint8_t service_id);

int CipAddService(tAppId app_id, uint8_t service_id);

// CIP Event Handling
void CipSessionSnortCallback(void *ssnptr, ServiceEventType eventType, void *data);

void CipClean(void);

extern struct RNAClientAppModule cip_client_mod;
extern struct RNAClientAppModule enip_client_mod;
extern struct RNAServiceValidationModule cip_service_mod;
extern struct RNAServiceValidationModule enip_service_mod;

#endif  // DETECTOR_CIP_H

