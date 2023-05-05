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
 * Protocol-Aware Flushing (PAF) code for the S7commplus preprocessor.
 *
 */

#ifndef S7COMM_PAF__H
#define S7COMM_PAF__H

#include "spp_s7comm.h"
#include "stream_api.h"

typedef enum _s7commplus_paf_state
{
	S7COMMPLUS_PAF_STATE__TPKT_VER = 0,
	S7COMMPLUS_PAF_STATE__TPKT_RESERVED,
	S7COMMPLUS_PAF_STATE__TPKT_LEN_1,
	S7COMMPLUS_PAF_STATE__TPKT_LEN_2,
	S7COMMPLUS_PAF_STATE__COPT_LEN,
	S7COMMPLUS_PAF_STATE__COPT_PDU_TYPE,
	S7COMMPLUS_PAF_STATE__SET_FLUSH
} s7commplus_paf_state_t;

typedef struct _s7commplus_paf_data
{
	s7commplus_paf_state_t state;
	uint16_t tpkt_length;
} s7commplus_paf_data_t;

void S7commplusAddPortsToPaf(struct _SnortConfig *sc, s7commplus_config_t *config, tSfPolicyId policy_id);
int S7commplusPafRegisterPort(struct _SnortConfig *sc, uint16_t port, tSfPolicyId policy_id);
int S7commplusAddServiceToPaf(struct _SnortConfig *sc, uint16_t service, tSfPolicyId policy_id);
PAF_Status S7commplusPaf(void *ssn, void **user, const uint8_t *data,
		uint32_t len, uint32_t flags, uint32_t *fp, uint32_t *fp_eoh);
static inline bool S7commplusIsPafActive(const SFSnortPacket *p)
{
	bool to_server = (p->flags & FLAG_FROM_CLIENT)? true:false;
	if ((p->stream_session_ptr)
			&& _dpd.streamAPI->is_paf_active(p->stream_session_ptr, to_server))
		return true;

	return false;
}
#endif /* S7COMMPLUS_PAF__H */
