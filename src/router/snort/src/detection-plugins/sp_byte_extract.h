/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2010-2013 Sourcefire, Inc.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __SP_BYTE_EXTRACT_H__
#define __SP_BYTE_EXTRACT_H__

#include "decode.h"
#include "plugbase.h"

#define BYTE_EXTRACT_SUCCESS 1
#define BYTE_EXTRACT_FAILURE -1

#define NUM_BYTE_EXTRACT_VARS 2
#define BYTE_EXTRACT_NO_VAR -1
#define BYTE_EXTRACT_INVALID_ERR_FMT "Rule option %s uses an undefined byte_extract variable name (%s)." //format: rule name, variable name
#define MAX_BYTES_TO_GRAB 4
#define MAX_BYTES_TO_EXTRACT 10

#define MIN_BYTE_EXTRACT_OFFSET -65535
#define MAX_BYTE_EXTRACT_OFFSET 65535
#define MIN_BYTE_EXTRACT_MULTIPLIER 1
#define MAX_BYTE_EXTRACT_MULTIPLIER 65535

typedef struct _ByteExtractData
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
    uint32_t bitmask_val;
} ByteExtractData;

void SetupByteExtract(void);
uint32_t ByteExtractHash(void *d);
int ByteExtractCompare(void *l, void *r);
int DetectByteExtract(void *, Packet *);
void ByteExtractFree(void *d);

void isvalidstr(char *str,char *feature);
int8_t GetVarByName(char *name);
void ClearVarNames(OptFpList *fpl);
int8_t AddVarNameToList(ByteExtractData *data);

int GetByteExtractValue(uint32_t *dst, int8_t var_number);
int SetByteExtractValue(uint32_t value, int8_t var_number);

uint32_t getNumberTailingZerosInBitmask(uint32_t);
int numBytesInBitmask(uint32_t );
void RuleOptionBitmaskParse(uint32_t* , char *, uint32_t ,char* );

#endif /* __SP_BYTE_EXTRACT_H__ */
