/*
** Copyright (C) 2010-2011 Sourcefire, Inc.
** Author: Ryan Jordan <ryan.jordan@sourcefire.com>
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __SP_BYTE_EXTRACT_H__
#define __SP_BYTE_EXTRACT_H__

#include "decode.h"
#include "plugbase.h"

#define BYTE_EXTRACT_SUCCESS 1
#define BYTE_EXTRACT_FAILURE -1

#define NUM_BYTE_EXTRACT_VARS 2
#define BYTE_EXTRACT_NO_VAR -1
#define BYTE_EXTRACT_INVALID_ERR_STR "Rule option uses an undefined byte_extract variable name."
#define MAX_BYTES_TO_GRAB 4

#define MIN_BYTE_EXTRACT_OFFSET -65535
#define MAX_BYTE_EXTRACT_OFFSET 65535
#define MIN_BYTE_EXTRACT_MULTIPLIER 1
#define MAX_BYTE_EXTRACT_MULTIPLIER 65535

typedef struct _ByteExractData
{
    uint32_t bytes_to_grab;
    int32_t offset;
    uint8_t relative_flag;
    uint8_t data_string_convert_flag;
    uint8_t align;
    int8_t endianess;
    uint32_t base;
    uint32_t multiplier;
    int8_t var_number;
    char *name;
    RuleOptByteOrderFunc byte_order_func;
} ByteExtractData;

void SetupByteExtract(void);
uint32_t ByteExtractHash(void *d);
int ByteExtractCompare(void *l, void *r);
int DetectByteExtract(void *, Packet *);
void ByteExtractFree(void *d);

int8_t GetVarByName(char *name);

int GetByteExtractValue(uint32_t *dst, int8_t var_number);
int SetByteExtractValue(uint32_t value, int8_t var_number);

#endif /* __SP_BYTE_EXTRACT_H__ */
