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
 * Dynamic preprocessor for the S7comm protocol
 *
 */

/* 
 * This is the encapsulation of S7comm/S7comm-plus protocol: 
 *   Ethernet | IP | TCP (server port 102) | TPKT | COTP | S7comm or S7comm-plus
 */

#include "s7comm_decode.h"

/* TPKT header */
typedef struct _tpkt_header
{
	uint8_t version;
	uint8_t reserved;
	uint16_t length;
} tpkt_header_t;

/* COTP header */
typedef struct _cotp_header
{
	uint8_t length;
	uint8_t pdu_type;
	uint8_t tpdu_num;
} cotp_header_t;

/* S7commplus data structures */
typedef struct _s7commplus_header
{
	uint8_t proto_id;
	uint8_t proto_version;
	uint16_t data_len;
} s7commplus_header_t;

#pragma pack(1)
typedef struct _s7commplus_data_hdr
{
	uint8_t opcode;
	uint16_t reserved_1;
	uint16_t function;
	uint16_t reserved_2;
} s7commplus_data_hdr_t;
#pragma pack()

static int S7commplusProtocolDecode(s7commplus_session_data_t *session, SFSnortPacket *packet)
{
	const s7commplus_header_t* s7commplus_header;
	const s7commplus_data_hdr_t* s7commplus_data_header;
	int offset;

	offset = sizeof(tpkt_header_t) + sizeof(cotp_header_t);

	s7commplus_header = (const s7commplus_header_t*)(packet->payload + offset);
	/* Set the session data. Swap byte order for 16-bit fields. */
	session->s7commplus_proto_id = s7commplus_header->proto_id;
	session->s7commplus_proto_version= s7commplus_header->proto_version;
	session->s7commplus_data_len = ntohs(s7commplus_header->data_len);

	/* V1 or V2 header packets */
	if (s7commplus_header->proto_version <= 0x02)
		offset += sizeof(s7commplus_header_t);
	else 
	{
		/* 33 byte Integrity part for V3 header packets */
		offset += sizeof(s7commplus_header_t) + INTEGRITY_PART_LEN ;
	}
	s7commplus_data_header = (const s7commplus_data_hdr_t *)(packet->payload + offset);
	/* Set the session data. Swap byte order for 16-bit fields. */
	session->s7commplus_opcode = s7commplus_data_header->opcode;
	session->s7commplus_reserved_1 = ntohs(s7commplus_data_header->reserved_1);
	session->s7commplus_function = ntohs(s7commplus_data_header->function);
	session->s7commplus_reserved_2 = ntohs(s7commplus_data_header->reserved_2);

	return true;
}

int S7commplusDecode(s7commplus_config_t *config, SFSnortPacket *packet)
{
	s7commplus_session_data_t *session;
	const tpkt_header_t *tpkt_header;
	const cotp_header_t *cotp_header;
	const s7commplus_header_t *s7commplus_header;
	uint16_t tpkt_length;

	if (packet->payload_size < TPKT_MIN_HDR_LEN) {
		memset(&session, 0, sizeof(session));
		return false; 
	}

	session = (s7commplus_session_data_t *)
		_dpd.sessionAPI->get_application_data(packet->stream_session, PP_S7COMMPLUS);
	session->s7commplus_proto_id = 0;

	/* Lay the header struct over the payload */
	tpkt_header = (const tpkt_header_t *) packet->payload;
	cotp_header = (const cotp_header_t *) (packet->payload + sizeof(tpkt_header_t));
	tpkt_length = ntohs(tpkt_header->length);

	/* It might be COTP fragment data */
	if ((tpkt_length == TPKT_MIN_HDR_LEN) || (tpkt_length == TPKT_MIN_DATA_HDR_LEN))
	{
		memset(&session, 0, sizeof(session));
		return true;
	}

	/* It might be a TPKT/COTP packet for other purpose, e.g. connect */
	if (cotp_header->length != COTP_HDR_LEN_FOR_S7COMMPLUS ||
			cotp_header->pdu_type != COTP_HDR_PDU_TYPE_DATA)
	{
		memset(&session, 0, sizeof(session));
		return true;
	}

	s7commplus_header = (const s7commplus_header_t *)(packet->payload +
			sizeof(tpkt_header_t) + sizeof(cotp_header_t));

	if (s7commplus_header->proto_id == S7COMMPLUS_PROTOCOL_ID)
	{
		return (S7commplusProtocolDecode(session, packet));
	}
	else 
	{
		_dpd.alertAdd(GENERATOR_SPP_S7COMMPLUS, S7COMMPLUS_BAD_PROTO_ID, 1, 0, 3,
				S7COMMPLUS_BAD_PROTO_ID_STR, 0);
		return false;
	}
}
