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
 * Rule options for S7commplus preprocessor
 *
 */

#include <string.h>

#include "sf_types.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_preprocessor.h"
#include "spp_s7comm.h"
#include "s7comm_decode.h"
#include "s7comm_roptions.h"
#include "s7comm_paf.h"

static s7commplus_opcode_map_t s7commplus_opcode_map[] =
{
	{"request",      0x31},
	{"response",     0x32},
	{"notification", 0x33},
	{"response2",    0x02}
};

/* Mapping of name -> function code for 's7commplus_function' option. */
static s7commplus_func_map_t s7commplus_func_map[] =
{
	{"explore",          0x04BB},
	{"createobject",     0x04CA},
	{"deleteobject",     0x04D4},
	{"setvariable",      0x04F2},
	{"getlink",          0x0524},
	{"setmultivar",      0x0542},
	{"getmultivar",      0x054C},
	{"beginsequence",    0x0556},
	{"endsequence",      0x0560},
	{"invoke",           0x056B},
	{"getvarsubstr",     0x0586}
};

int S7commplusOpcodeInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
	char *endptr;
	s7commplus_option_data_t *s7commplus_data;
	unsigned int opcode = 0;

	if (name == NULL || data == NULL)
		return 0;

	if (strcmp(name, S7COMMPLUS_OPCODE_NAME) != 0)
		return 0;

	if (params == NULL)
	{
		DynamicPreprocessorFatalMessage("%s(%d): No argument given for s7commplus_opcode. "
				"s7commplus_opcode requires a number between 0 and 0xFF.\n",
				*_dpd.config_file, *_dpd.config_line);
	}

	s7commplus_data = (s7commplus_option_data_t *)calloc(1, sizeof(s7commplus_option_data_t));
	if (s7commplus_data == NULL)
	{
		DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
				"s7commplus_option_data_t data structure.\n", __FILE__, __LINE__);
	}

	/* Parsing time */
	if (isdigit(params[0]))
	{
		/* argument given as integer (hexidecimal) */
		opcode = _dpd.SnortStrtoul(params, &endptr, 16);
		if ((opcode > 255) || (*endptr != '\0'))
		{
			DynamicPreprocessorFatalMessage("%s(%d): s7commplus_opcode requires a "
					"number between 0 and 0xFF.\n", *_dpd.config_file, *_dpd.config_line);
		}
	} else {
		/* Check the argument against the map */
		size_t i;
		int parse_success = 0;
		for (i = 0; i < (sizeof(s7commplus_opcode_map) / sizeof(s7commplus_opcode_map_t)); i++)
		{
			if (strcmp(params, s7commplus_opcode_map[i].name) == 0)
			{
				parse_success = 1;
				opcode = s7commplus_opcode_map[i].opcode;
				break;
			}
		}

		if (!parse_success)
		{
			DynamicPreprocessorFatalMessage("%s(%d): s7commplus_opcode requires a "
					"number between 0 and 0xFF, or a valid opcode name.\n",
					*_dpd.config_file, *_dpd.config_line);
		}
	}

	s7commplus_data->type = S7COMMPLUS_OPCODE;
	s7commplus_data->arg = (uint8_t) opcode;

	*data = (void *)s7commplus_data;

	return 1;
}

int S7commplusFuncInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
	char *endptr;
	s7commplus_option_data_t *s7commplus_data;
	unsigned int func = 0;

	if (name == NULL || data == NULL)
		return 0;

	if (strcmp(name, S7COMMPLUS_FUNC_NAME) != 0)
		return 0;

	if (params == NULL)
	{
		DynamicPreprocessorFatalMessage("%s(%d): No argument given for s7commplus_function. "
				"s7commplus_function requires a number between 0 and 0xFFFF.\n",
				*_dpd.config_file, *_dpd.config_line);
	}

	s7commplus_data = (s7commplus_option_data_t *)calloc(1, sizeof(s7commplus_option_data_t));
	if (s7commplus_data == NULL)
	{
		DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
				"s7commplus_option_data_t data structure.\n", __FILE__, __LINE__);
	}

	/* Parsing time */
	if (isdigit(params[0]))
	{
		/* argument given as integer (hexidecimal) */
		func = _dpd.SnortStrtoul(params, &endptr, 16);
		if ((func > 0xFFFF) || (*endptr != '\0'))
		{
			DynamicPreprocessorFatalMessage("%s(%d): s7commplus_function requires a "
					"number between 0 and 0xFFFF.\n", *_dpd.config_file, *_dpd.config_line);
		}
	} else {
		/* Check the argument against the map */
		size_t i;
		int parse_success = 0;
		for (i = 0; i < (sizeof(s7commplus_func_map) / sizeof(s7commplus_func_map_t)); i++)
		{
			if (strcmp(params, s7commplus_func_map[i].name) == 0)
			{
				parse_success = 1;
				func = s7commplus_func_map[i].func;
				break;
			}
		}

		if (!parse_success)
		{
			DynamicPreprocessorFatalMessage("%s(%d): s7commplus_function requires a "
					"number between 0 and 0xFFFF, or a valid function name.\n",
					*_dpd.config_file, *_dpd.config_line);
		}
	}

	s7commplus_data->type = S7COMMPLUS_FUNC;
	s7commplus_data->arg = (uint16_t) func;

	*data = (void *)s7commplus_data;

	return 1;    
}

int S7commplusContentInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
	s7commplus_option_data_t *s7commplus_data;

	if (strcmp(name, S7COMMPLUS_CONTENT_NAME) != 0)
		return 0;

	/* Nothing to parse. */
	if (params)
	{
		DynamicPreprocessorFatalMessage("%s(%d): s7commplus_content does not take "
				"any arguments.\n", *_dpd.config_file, *_dpd.config_line);
	}

	s7commplus_data = (s7commplus_option_data_t *)calloc(1, sizeof(s7commplus_option_data_t));
	if (s7commplus_data == NULL)
	{
		DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
				"s7commplus_option_data_t data structure.\n", __FILE__, __LINE__);
	}

	s7commplus_data->type = S7COMMPLUS_CONTENT;
	s7commplus_data->arg = 0;

	*data = (void *)s7commplus_data;
	return 1;
}

/* S7commplus rule evaluation callback. */
int S7commplusRuleEval(void *raw_packet, const uint8_t **cursor, void *data)
{
	SFSnortPacket *packet = (SFSnortPacket *)raw_packet;
	s7commplus_option_data_t *rule_data = (s7commplus_option_data_t *)data;
	s7commplus_session_data_t *session_data;

	/*
	 * The preprocessor only evaluates PAF-flushed PDUs. If the rule options
	 * don't check for this, they'll fire on stale session data when the
	 * original packet goes through before flushing.
	 */
	if (!PacketHasFullPDU(packet) && S7commplusIsPafActive(packet))
		return RULE_NOMATCH;

	session_data = (s7commplus_session_data_t *)
		_dpd.sessionAPI->get_application_data(packet->stream_session, PP_S7COMMPLUS);

	if ((packet->payload_size == 0 ) || (session_data == NULL))
	{
		return RULE_NOMATCH;
	}

	switch (rule_data->type)
	{
		case S7COMMPLUS_OPCODE:
			if (session_data->s7commplus_proto_id != S7COMMPLUS_PROTOCOL_ID)
				return RULE_NOMATCH;
			if (session_data->s7commplus_opcode == rule_data->arg)
				return RULE_MATCH;
			break;

		case S7COMMPLUS_FUNC:
			if (session_data->s7commplus_proto_id != S7COMMPLUS_PROTOCOL_ID)
				return RULE_NOMATCH;
			if (session_data->s7commplus_function == rule_data->arg)
				return RULE_MATCH;
			break;

		case S7COMMPLUS_CONTENT:
			if (session_data->s7commplus_proto_id != S7COMMPLUS_PROTOCOL_ID)
				return RULE_NOMATCH;
			if (packet->payload_size < TPKT_MIN_HDR_LEN + S7COMMPLUS_MIN_HDR_LEN)
				return RULE_NOMATCH;

			/* S7commplus data */
			*cursor = (const uint8_t *) (packet->payload + TPKT_MIN_HDR_LEN + S7COMMPLUS_MIN_HDR_LEN);
			_dpd.SetAltDetect((uint8_t *)*cursor, (uint16_t)(packet->payload_size - TPKT_MIN_HDR_LEN - S7COMMPLUS_MIN_HDR_LEN));

			return RULE_MATCH;
	}

	return RULE_NOMATCH;
}
