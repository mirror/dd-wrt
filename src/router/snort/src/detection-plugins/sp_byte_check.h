/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

/* $Id$ */

#ifndef __SP_BYTE_CHECK_H__
#define __SP_BYTE_CHECK_H__

#include "sf_engine/sf_snort_plugin_api.h"
#include "decode.h"
#include "plugbase.h"

#define BT_LESS_THAN            CHECK_LT
#define BT_EQUALS               CHECK_EQ
#define BT_GREATER_THAN         CHECK_GT
#define BT_AND                  CHECK_AND
#define BT_XOR                  CHECK_XOR
#define BT_GREATER_THAN_EQUAL   CHECK_GTE
#define BT_LESS_THAN_EQUAL      CHECK_LTE
#define BT_CHECK_ALL            CHECK_ALL
#define BT_CHECK_ATLEASTONE     CHECK_ATLEASTONE
#define BT_CHECK_NONE           CHECK_NONE

#define BIG    0
#define LITTLE 1

#define BYTE_TEST_INVALID_ERR_FMT "Rule option %s uses an undefined byte_extract/byte_math variable name (%s)." //format: rule name, variable name

typedef struct _ByteTestData
{
    uint32_t bytes_to_compare; /* number of bytes to compare */
    uint32_t cmp_value;
    uint32_t operator;
    int32_t offset;
    uint8_t not_flag;
    uint8_t relative_flag;
    uint8_t data_string_convert_flag;
    int8_t endianess;
    uint32_t base;
    int8_t cmp_value_var;
    int8_t offset_var;
    RuleOptByteOrderFunc byte_order_func;
    uint32_t bitmask_val;
} ByteTestData;

void SetupByteTest(void);
uint32_t ByteTestHash(void *d);
int ByteTestCompare(void *l, void *r);
int ByteTest(void *, Packet *);
#endif  /* __SP_BYTE_CHECK_H__ */
