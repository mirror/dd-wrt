/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Dynamic preprocessor for the Modbus protocol
 *
 */

#ifndef MODBUS_DECODE_H
#define MODBUS_DECODE_H

#include <stdint.h>

#include "spp_modbus.h"
#include "sf_snort_plugin_api.h"

/* Need 8 bytes for MBAP Header + Function Code */
#define MODBUS_MIN_LEN 8

/* GIDs, SIDs, and Strings */
#define GENERATOR_SPP_MODBUS 144

#define MODBUS_BAD_LENGTH 1
#define MODBUS_BAD_PROTO_ID 2
#define MODBUS_RESERVED_FUNCTION 3

#define MODBUS_BAD_LENGTH_STR "(spp_modbus): Length in Modbus MBAP header does not match the length needed for the given Modbus function."
#define MODBUS_BAD_PROTO_ID_STR "(spp_modbus): Modbus protocol ID is non-zero."
#define MODBUS_RESERVED_FUNCTION_STR "(spp_modbus): Reserved Modbus function code in use."

int ModbusDecode(modbus_config_t *config, SFSnortPacket *packet);

#endif /* MODBUS_DECODE_H */
