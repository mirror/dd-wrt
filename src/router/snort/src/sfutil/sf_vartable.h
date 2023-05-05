/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 1998-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 * Adam Keeton
 * sf_vartable.h
 * 11/17/06
 *
 * Library for implementing a variable table.  
 * All API calls have the prefix "sfvt".
*/

#ifndef SF_VARTABLE_H
#define SF_VARTABLE_H

#include "ipv6_port.h"
#include "sf_ipvar.h"

/* Allocates new variable table */
vartable_t * sfvt_alloc_table(void);
void sfvt_free_table(vartable_t *table);

/* Adds the variable described by "str" to the table "table" */
SFIP_RET sfvt_add_str(vartable_t *table, char *str, sfip_var_t **);
SFIP_RET sfvt_define(vartable_t *table, char *name, char *value);

/* Adds the variable described by "str" to the variable "dst",
 * using the vartable for looking variables used within "str" */
SFIP_RET sfvt_add_to_var(vartable_t *table, sfip_var_t *dst, char *src);

/* Looks up a variable from the table using the name as the key */
sfip_var_t *sfvt_lookup_var(vartable_t *table, char *name);

/* Prints a table's contents */
void sfvt_print(FILE *f, vartable_t *table);

#endif
