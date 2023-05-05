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
 * Rule options for the DNP3 preprocessor
 *
 */

#ifndef DNP3_ROPTIONS__H
#define DNP3_ROPTIONS__H

#include <stdint.h>

/* option names */
#define DNP3_FUNC_NAME "dnp3_func"
#define DNP3_OBJ_NAME "dnp3_obj"
#define DNP3_IND_NAME "dnp3_ind"
#define DNP3_DATA_NAME "dnp3_data"

/* Rule registration functions */
int DNP3FuncInit(struct _SnortConfig *sc, char *name, char *params, void **data);
int DNP3ObjInit(struct _SnortConfig *sc, char *name, char *params, void **data);
int DNP3IndInit(struct _SnortConfig *sc, char *name, char *params, void **data);
int DNP3DataInit(struct _SnortConfig *sc, char *name, char *params, void **data);

/* Rule evaluation functions */
int DNP3FuncEval(void *raw_packet, const uint8_t **cursor, void *data);
int DNP3ObjEval(void *raw_packet, const uint8_t **cursor, void *data);
int DNP3IndEval(void *raw_packet, const uint8_t **cursor, void *data);
int DNP3DataEval(void *raw_packet, const uint8_t **cursor, void *data);

#endif /* DNP3_ROPTIONS__H */
