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
 * Copyright (C) 2020-2022 Cisco and/or its affiliates. All rights reserved.
 *
 * Authors: Jeffrey Gu <jgu@cisco.com>, Pradeep Damodharan <prdamodh@cisco.com>
 *
 * Dynamic preprocessor for the S7commplus protocol
 *
 */

#ifndef S7COMM_DECODE_H
#define S7COMM_DECODE_H

#include <stdint.h>
#include <string.h>						/* For memset */

#include "spp_s7comm.h"
#include "sf_snort_plugin_api.h"

/* GIDs, SIDs, and Strings */
#define GENERATOR_SPP_S7COMMPLUS 149   /* matches generators.h */

/* S7Commplus defines */
#define S7COMMPLUS_PROTOCOL_ID 0x72
#define S7COMMPLUS_PDUTYPE_CONNECT                 0x01
#define S7COMMPLUS_PDUTYPE_DATA                    0x02
#define S7COMMPLUS_PDUTYPE_DATAFW1_5               0x03
#define S7COMMPLUS_PDUTYPE_KEEPALIVE               0xFF

#define COTP_HDR_LEN_FOR_S7COMMPLUS 2
#define COTP_HDR_PDU_TYPE_DATA  0xF0

#define S7COMMPLUS_BAD_LENGTH 1
#define S7COMMPLUS_BAD_PROTO_ID 2
#define S7COMMPLUS_RESERVED_FUNCTION 3

#define S7COMMPLUS_BAD_LENGTH_STR "(spp_s7commplus): Length in S7commplus header does not match the length needed for the given S7comm function."
#define S7COMMPLUS_BAD_PROTO_ID_STR "(spp_s7commplus): S7commplus protocol ID is non-zero."
#define S7COMMPLUS_RESERVED_FUNCTION_STR "(spp_s7commplus): Reserved S7commplus function code in use."

int S7commplusDecode(s7commplus_config_t *config, SFSnortPacket *packet);

#endif /* S7COMM_DECODE_H */
