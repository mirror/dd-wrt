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
 * Tables for DNP3 function & indicator definitions
 *
 */

#ifndef DNP3_MAP__H
#define DNP3_MAP__H

#include <stdint.h>

/* Check if "code" is in the function map.
 *
 * Returns: 1 on success, 0 on failure.
 */
int DNP3FuncIsDefined(uint16_t code);

/* Return the DNP3 function code corresponding to "name".
 *
 * Returns: integer
 *          -1 on failure
 */
int DNP3FuncStrToCode(char *name);

/* Return the DNP3 indication code corresponding to "name".
 *
 * Returns: integer
 *          -1 on failure
 */
int DNP3IndStrToCode(char *name);

#endif
