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
 * Rule options for S7commplus preprocessor.
 *
 */

#ifndef S7COMM_ROPTIONS_H
#define S7COMM_ROPTIONS_H

#include <stdint.h>

#define S7COMMPLUS_OPCODE_NAME  "s7commplus_opcode"
#define S7COMMPLUS_FUNC_NAME    "s7commplus_func"
#define S7COMMPLUS_CONTENT_NAME "s7commplus_content"

/* Data types */
typedef enum _s7commplus_option_type_t
{
	S7COMMPLUS_OPCODE = 0,
	S7COMMPLUS_FUNC,
	S7COMMPLUS_CONTENT
} s7commplus_option_type_t;

typedef struct _s7commplus_option_data_t
{
	s7commplus_option_type_t type;
	uint16_t arg;
} s7commplus_option_data_t;

typedef struct _s7commplus_opcode_map_t
{
	char *name;
	uint8_t opcode;
} s7commplus_opcode_map_t;

typedef struct _s7commplus_func_map_t
{
	char *name;
	uint16_t func;
} s7commplus_func_map_t;

int S7commplusOpcodeInit(struct _SnortConfig *sc, char *name, char *params, void **data);
int S7commplusFuncInit(struct _SnortConfig *sc, char *name, char *params, void **data);
int S7commplusContentInit(struct _SnortConfig *sc, char *name, char *params, void **data);

int S7commplusRuleEval(void *raw_packet, const uint8_t **cursor, void *data);

#endif /* S7COMM_ROPTIONS_H */
