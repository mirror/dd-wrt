/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */

#ifndef __SP_BYTE_JUMP_H__
#define __SP_BYTE_JUMP_H__

#include "decode.h"
#include "plugbase.h"

typedef struct _ByteJumpData
{
    uint32_t bytes_to_grab; /* number of bytes to compare */
    int32_t offset;
    uint8_t relative_flag;
    uint8_t data_string_convert_flag;
    uint8_t from_beginning_flag;
    uint8_t align_flag;
    int8_t endianess;
    uint32_t base;
    uint32_t multiplier;
    int32_t post_offset;
    int8_t offset_var;
    RuleOptByteOrderFunc byte_order_func;
} ByteJumpData;

void SetupByteJump(void);
uint32_t ByteJumpHash(void *d);
int ByteJumpCompare(void *l, void *r);
int ByteJump(void *, Packet *);

#endif  /* __SP_BYTE_JUMP_H__ */
