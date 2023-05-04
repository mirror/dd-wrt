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

#ifndef SPP_S7COMM_H
#define SPP_S7COMM_H

#include "sf_types.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

#define MAX_PORTS 65536

/* Default S7commplus port */
#define S7COMMPLUS_PORT 102

/* Convert port value into an index for the s7comm_config->ports array */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

/* S7commplus preprocessor configuration */
typedef struct _s7commplus_config
{
	uint8_t ports[MAX_PORTS/8];

	int ref_count;
} s7commplus_config_t;

/* S7commplus session data */
typedef struct _s7commplus_session_data
{
	uint8_t s7commplus_proto_id;
	uint8_t s7commplus_proto_version;
	uint16_t s7commplus_data_len;
	uint8_t s7commplus_opcode;
	uint16_t s7commplus_function, s7commplus_reserved_1, s7commplus_reserved_2;

	tSfPolicyId policy_id;
	tSfPolicyUserContextId context_id;
} s7commplus_session_data_t;

#define S7COMMPLUS_PORTS_KEYWORD    "ports"
#define S7COMMPLUS_MEMCAP_KEYWORD   "memcap"

#define TPKT_MIN_HDR_LEN 7     /* length field in TPKT header for S7commplus */
#define TPKT_MIN_DATA_HDR_LEN 11     /* length field in TPKT header for S7commplus */
#define INTEGRITY_PART_LEN 33 /* length of Integrity part in V3 Header packets */
#define S7COMMPLUS_MIN_HDR_LEN 4

#define S7COMMPLUS_PROTOCOL_ID                  0x72

#endif /* SPP_S7COMM_H */
