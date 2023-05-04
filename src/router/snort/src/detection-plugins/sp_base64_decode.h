/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2003-2013 Sourcefire, Inc.
** 
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


#ifndef __SP_BASE64_DECODE_H__
#define __SP_BASE64_DECODE_H__

#include "sp_pattern_match.h"

#define BASE64DECODE_RELATIVE_FLAG 0x01

typedef struct _Base64DecodeData
{
    uint32_t bytes_to_decode;
    uint32_t offset;
    uint8_t  flags;
    HTTP_BUFFER buffer_type;
}Base64DecodeData;

int Base64DecodeCompare(void *, void *);
uint32_t Base64DecodeHash(void *);
void SetupBase64Decode(void);

#endif  
