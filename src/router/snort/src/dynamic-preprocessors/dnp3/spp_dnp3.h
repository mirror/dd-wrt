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
 * Dynamic preprocessor for the DNP3 protocol
 *
 */

#ifndef SPP_DNP3_H
#define SPP_DNP3_H

#include "config.h"
#include "sf_types.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

/* GIDs, SIDs, Messages */
#define GENERATOR_SPP_DNP3  145

#define DNP3_BAD_CRC                    1
#define DNP3_DROPPED_FRAME              2
#define DNP3_DROPPED_SEGMENT            3
#define DNP3_REASSEMBLY_BUFFER_CLEARED  4
#define DNP3_RESERVED_ADDRESS           5
#define DNP3_RESERVED_FUNCTION          6

#define DNP3_BAD_CRC_STR  "(spp_dnp3): DNP3 Link-Layer Frame contains bad CRC."
#define DNP3_DROPPED_FRAME_STR "(spp_dnp3): DNP3 Link-Layer Frame was dropped."
#define DNP3_DROPPED_SEGMENT_STR "(spp_dnp3): DNP3 Transport-Layer Segment was dropped during reassembly."
#define DNP3_REASSEMBLY_BUFFER_CLEARED_STR "(spp_dnp3): DNP3 Reassembly Buffer was cleared without reassembling a complete message."
#define DNP3_RESERVED_ADDRESS_STR "(spp_dnp3): DNP3 Link-Layer Frame uses a reserved address."
#define DNP3_RESERVED_FUNCTION_STR "(spp_dnp3): DNP3 Application-Layer Fragment uses a reserved function code."

#define MAX_PORTS 65536

/* Default DNP3 port */
#define DNP3_PORT 20000

/* Memcap limits. */
#define MIN_DNP3_MEMCAP 4144
#define MAX_DNP3_MEMCAP (100 * 1024 * 1024)

/* Convert port value into an index for the dnp3_config->ports array */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

/* Packet directions */
#define DNP3_CLIENT 0
#define DNP3_SERVER 1

/* Session data flags */
#define DNP3_FUNC_RULE_FIRED    0x0001
#define DNP3_OBJ_RULE_FIRED     0x0002
#define DNP3_IND_RULE_FIRED     0x0004
#define DNP3_DATA_RULE_FIRED    0x0008

/* DNP3 minimum length: start (2 octets) + len (1 octet) */
#define DNP3_MIN_LEN 3
#define DNP3_LEN_OFFSET 2

/* Length of the rest of a DNP3 link-layer header: ctrl + src + dest */
#define DNP3_HEADER_REMAINDER_LEN 5

/* Reassembly data types moved here to avoid circular dependency
   with dnp3_sesion_data_t */
#define DNP3_BUFFER_SIZE 2048
typedef enum _dnp3_reassembly_state_t
{
    DNP3_REASSEMBLY_STATE__IDLE = 0,
    DNP3_REASSEMBLY_STATE__ASSEMBLY,
    DNP3_REASSEMBLY_STATE__DONE
} dnp3_reassembly_state_t;

typedef struct _dnp3_reassembly_data_t
{
    char buffer[DNP3_BUFFER_SIZE];
    uint16_t buflen;
    dnp3_reassembly_state_t state;
    uint8_t last_seq;
} dnp3_reassembly_data_t;


/* DNP3 preprocessor configuration */
typedef struct _dnp3_config
{
    uint32_t memcap;
    uint8_t  ports[MAX_PORTS/8];
    uint8_t  check_crc;
    int disabled;

    int ref_count;
} dnp3_config_t;

/* DNP3 session data */
typedef struct _dnp3_session_data
{
    /* Fields for rule option matching. */
    uint8_t direction;
    uint8_t func;
    uint8_t obj_group;
    uint8_t obj_var;
    uint16_t indications;
    uint16_t flags;

    /* Reassembly stuff */
    dnp3_reassembly_data_t client_rdata;
    dnp3_reassembly_data_t server_rdata;

    tSfPolicyId policy_id;
    tSfPolicyUserContextId context_id;
} dnp3_session_data_t;


/* DNP3 header structures */
typedef struct _dnp3_link_header_t
{
    uint16_t start;
    uint8_t len;
    uint8_t ctrl;
    uint16_t dest;
    uint16_t src;
} dnp3_link_header_t;

#define DNP3_TRANSPORT_FIN(x) (x & 0x80)
#define DNP3_TRANSPORT_FIR(x) (x & 0x40)
#define DNP3_TRANSPORT_SEQ(x) (x & 0x3F)
#define DNP3_MAX_TRANSPORT_LEN 250
typedef struct _dnp3_transport_header_t
{
    uint8_t control;
} dnp3_transport_header_t;


/* Yep, the locations of FIR and FIN are switched at this layer... */
#define DNP3_APP_FIR(x) (x & 0x80)
#define DNP3_APP_FIN(x) (x & 0x40)
#define DNP3_APP_SEQ(x) (x & 0x0F)
typedef struct _dnp3_app_request_header_t
{
    uint8_t control;
    uint8_t function;
} dnp3_app_request_header_t;

typedef struct _dnp3_app_response_header_t
{
    uint8_t control;
    uint8_t function;
    uint16_t indications;
} dnp3_app_response_header_t;

#define DNP3_CHECK_CRC_KEYWORD  "check_crc"
#define DNP3_PORTS_KEYWORD      "ports"
#define DNP3_MEMCAP_KEYWORD     "memcap"
#define DNP3_DISABLED_KEYWORD   "disabled"

#define DNP3_OK 1
#define DNP3_FAIL (-1)

#ifdef WORDS_BIGENDIAN
#define DNP3_MIN_RESERVED_ADDR 0xF0FF
#define DNP3_MAX_RESERVED_ADDR 0xFBFF
#define DNP3_START_BYTES       0x0564
#else
#define DNP3_MIN_RESERVED_ADDR 0xFFF0
#define DNP3_MAX_RESERVED_ADDR 0xFFFB
#define DNP3_START_BYTES       0x6405
#endif

#define DNP3_START_BYTE_1   0x05
#define DNP3_START_BYTE_2   0x64

#define DNP3_CHUNK_SIZE     16
#define DNP3_CRC_SIZE        2

#endif /* SPP_DNP3_H */
