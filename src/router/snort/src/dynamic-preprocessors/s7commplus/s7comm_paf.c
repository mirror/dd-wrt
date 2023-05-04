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

#include "spp_s7comm.h"
#include "s7comm_decode.h"
#include "s7comm_paf.h"
#include "sf_dynamic_preprocessor.h"

int S7commplusPafRegisterPort (struct _SnortConfig *sc, uint16_t port, tSfPolicyId policy_id)
{
	if (!_dpd.isPafEnabled())
		return 0;

	_dpd.streamAPI->register_paf_port(sc, policy_id, port, 0, (PAF_Callback)S7commplusPaf, true);
	_dpd.streamAPI->register_paf_port(sc, policy_id, port, 1, (PAF_Callback)S7commplusPaf, true);

	return 0;
}

#ifdef TARGET_BASED
int S7commplusAddServiceToPaf (struct _SnortConfig *sc, uint16_t service, tSfPolicyId policy_id)
{
	if (!_dpd.isPafEnabled())
		return 0;

	_dpd.streamAPI->register_paf_service(sc, policy_id, service, 0, (PAF_Callback)S7commplusPaf, true);
	_dpd.streamAPI->register_paf_service(sc, policy_id, service, 1, (PAF_Callback)S7commplusPaf, true);

	return 0;
}
#endif

/* Function: S7commplusPaf()

Purpose: S7commplus/TCP PAF callback.
Statefully inspects S7commplus traffic from the start of a session,
Reads up until the length octet is found, then sets a flush point.

Arguments:
void * - stream5 session pointer
void ** - S7commplus state tracking structure
const uint8_t * - payload data to inspect
uint32_t - length of payload data
uint32_t - flags to check whether client or server
uint32_t * - pointer to set flush point

Returns:
PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 */

PAF_Status S7commplusPaf(void *ssn, void **user, const uint8_t *data,
		uint32_t len, uint32_t flags, uint32_t *fp, uint32_t *fp_eoh)
{
	s7commplus_paf_data_t *pafdata = *(s7commplus_paf_data_t **)user;
	uint32_t bytes_processed = 0;

	/* Allocate state object if it doesn't exist yet. */
	if (pafdata == NULL)
	{
		pafdata = calloc(1, sizeof(s7commplus_paf_data_t));
		if (pafdata == NULL)
			return PAF_ABORT;

		*user = pafdata;
	}

	/* Process this packet 1 byte at a time */
	while (bytes_processed < len)
	{
		switch (pafdata->state)
		{
			/* Skip the Transaction & Protocol IDs */
			case S7COMMPLUS_PAF_STATE__TPKT_VER:
			case S7COMMPLUS_PAF_STATE__TPKT_RESERVED:
			case S7COMMPLUS_PAF_STATE__COPT_LEN:
			case S7COMMPLUS_PAF_STATE__COPT_PDU_TYPE:
				pafdata->state++;
				break;

			case S7COMMPLUS_PAF_STATE__TPKT_LEN_1:
				pafdata->tpkt_length |= ( *(data + bytes_processed) << 8 );
				pafdata->state++;
				break;

			case S7COMMPLUS_PAF_STATE__TPKT_LEN_2:
				pafdata->tpkt_length |= *(data + bytes_processed);
				pafdata->state++;
				break;

			case S7COMMPLUS_PAF_STATE__SET_FLUSH:
				if ((pafdata->tpkt_length < TPKT_MIN_HDR_LEN))
				{
					_dpd.alertAdd(GENERATOR_SPP_S7COMMPLUS, S7COMMPLUS_BAD_LENGTH, 1, 0, 3,
							S7COMMPLUS_BAD_LENGTH_STR, 0);
				}
				/* flush point at the end of payload */
				*fp = pafdata->tpkt_length;
				pafdata->state = S7COMMPLUS_PAF_STATE__TPKT_VER;
				pafdata->tpkt_length = 0;
				return PAF_FLUSH;
		}

		bytes_processed++;
	}

	return PAF_SEARCH;
}

/* Take a S7commplus config + Snort policy, iterate through ports, register PAF callback */
void S7commplusAddPortsToPaf(struct _SnortConfig *sc, s7commplus_config_t *config, tSfPolicyId policy_id)
{
	unsigned int i;

	for (i = 0; i < MAX_PORTS; i++)
	{
		if (config->ports[PORT_INDEX(i)] & CONV_PORT(i))
		{
			S7commplusPafRegisterPort(sc, (uint16_t) i, policy_id);
		}
	}
}
