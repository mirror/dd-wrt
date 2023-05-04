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

// Description: Common types for passing data between CIP preprocessor and CIP appid detector.

#ifndef CIP_COMMON_H
#define CIP_COMMON_H

#include <stdbool.h>  // For bool
#include <stdint.h>   // For C integer types

typedef enum _CipDataType
{
    CIP_DATA_TYPE_PATH_CLASS = 0,
    CIP_DATA_TYPE_PATH_EXT_SYMBOL,
    CIP_DATA_TYPE_SET_ATTRIBUTE,
    CIP_DATA_TYPE_CONNECTION,
    CIP_DATA_TYPE_IMPLICIT,
    CIP_DATA_TYPE_OTHER,
    CIP_DATA_TYPE_ENIP_COMMAND,
    CIP_DATA_TYPE_MALFORMED
} CipDataType;

typedef struct _CipEventData
{
    // Specify the type of CIP data.
    CipDataType type;

    // Used for:
    //  CIP_DATA_TYPE_ENIP_COMMAND
    uint16_t enip_command_id;

    // Used for:
    //  CIP_DATA_TYPE_PATH_CLASS
    //  CIP_DATA_TYPE_PATH_EXT_SYMBOL
    //  CIP_DATA_TYPE_SET_ATTRIBUTE
    uint8_t service_id;

    // Used for:
    //  CIP_DATA_TYPE_PATH_CLASS: This represents the Request Path Class.
    //  CIP_DATA_TYPE_SET_ATTRIBUTE: This represents the Request Path Class.
    //  CIP_DATA_TYPE_CONNECTION: This represents the Connection Path Class.
    //  CIP_DATA_TYPE_IMPLICIT: This represents the Connection Path Class from
    //      the original connection request, for this connection.
    uint32_t class_id;

    // Used for:
    //  CIP_DATA_TYPE_SET_ATTRIBUTE: This represents the Request Path Instance.
    uint32_t instance_id;

    // Used for:
    //  CIP_DATA_TYPE_SET_ATTRIBUTE: This represents the Request Path Attribute.
    uint32_t attribute_id;

    // Pointer to SFSnortPacket packet.
    const void* snort_packet;
} CipEventData;

typedef enum _CipEventType
{
    CIP_EVENT_TYPE_CIP_DATA
} CipEventType;

#endif  // CIP_COMMON_H
