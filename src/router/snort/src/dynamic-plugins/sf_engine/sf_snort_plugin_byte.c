/*
 * sf_snort_plugin_byte.c
 *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
 *
 * Author: Steve Sturges
 *         Andy Mullican
 *
 * Date: 5/2005
 *
 *
 * Byte operations for dynamic rule engine
 */
#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"

extern int checkCursorSimple(const u_int8_t *cursor, int flags, const u_int8_t *start, const u_int8_t *end, int offset);
extern int setCursorInternal(void *p, int flags, int offset, const u_int8_t **cursor);

#define BYTE_STRING_LEN     11

/*
 * extract byte value from data
 *
 * Return 1 if successfully extract value.
 * Return < 0 if fail to extract value.
 */
int extractValueInternal(void *p, ByteData *byteData, u_int32_t *value, const u_int8_t *cursor)
{
    char byteArray[BYTE_STRING_LEN];
    u_int32_t i;
    char *endPtr;
    u_int32_t extracted = 0;
    int base = 10;
    const u_int8_t *start;
    const u_int8_t *end;
    int ret;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = getBuffer(sp, byteData->flags, &start, &end);

    if ( ret < 0 )
    {
        return ret;
    }    

    /* Check the start location */
    if (checkCursorSimple(cursor, byteData->flags, start, end, byteData->offset) <= 0)
        return -1;

    /* and the end location */
    if (checkCursorSimple(cursor, byteData->flags, start, end, byteData->offset + byteData->bytes - 1) <= 0)
        return -1;

    /* Extract can be from beginning of buffer, or relative to cursor */
    if ( cursor == NULL || !(byteData->flags & CONTENT_RELATIVE) )
    {
        cursor = start;
    }
    
    if (byteData->flags & EXTRACT_AS_BYTE)
    {        
        if ( byteData->bytes != 1 && byteData->bytes != 2 && byteData->bytes != 4 )
        {
            return -5;  /* We only support 1, 2, or 4 bytes */
        }

        if (byteData->bytes < 1 || byteData->bytes > 4)
            return -2;

        if ( byteData->flags & BYTE_BIG_ENDIAN )
        {
            for (i = byteData->bytes; i > 0; i--)
            {
                extracted |= *(cursor + byteData->offset + byteData->bytes - i) << 8*(i-1);
            }
        }
        else
        {            
            for (i = 0; i < byteData->bytes; i++)
            {
                extracted |= *(cursor + byteData->offset + i) << 8*i;
            }
        }
        
        *value = extracted;
        return 1;
    }
    else if (byteData->flags & EXTRACT_AS_STRING)
    {
        if (byteData->bytes < 1 || byteData->bytes > (BYTE_STRING_LEN - 1))
        {
            /* Log Error message */
            return -2;
        }

        if (byteData->flags & EXTRACT_AS_DEC)
            base = 10;
        else if (byteData->flags & EXTRACT_AS_HEX)
            base = 16;
        else if (byteData->flags & EXTRACT_AS_OCT)
            base = 8;
        else if (byteData->flags & EXTRACT_AS_BIN)
            base = 2;

        for (i=0;i<byteData->bytes;i++)
        {
            byteArray[i] = *(cursor + byteData->offset + i);
        }
        byteArray[i] = '\0';
        
        extracted = strtoul(byteArray, &endPtr, base);

        if (endPtr == &byteArray[0])
            return -3;  /* Nothing to convert */

        *value = extracted;
        return 1;
    }
    return -4;
}

/*
 * Extract value, store in byteExtract->memoryLocation
 *
 * Return 1 if success
 * Return 0 if can't extract.
 */
ENGINE_LINKAGE int extractValue(void *p, ByteExtract *byteExtract, const u_int8_t *cursor)
{
    ByteData byteData;
    int ret;
    u_int32_t extracted = 0;
    u_int32_t *location = (u_int32_t *)byteExtract->memoryLocation;

    byteData.bytes = byteExtract->bytes;
    byteData.flags = byteExtract->flags;
    byteData.multiplier = byteExtract->multiplier;
    byteData.offset = byteExtract->offset;
    byteData.op = 0; /* Not used */
    byteData.value = 0;  /* Not used */

    ret = extractValueInternal(p, &byteData, &extracted, cursor);
    if (ret > 0)
    {
        *location = extracted;
    }

    return ret;
}

/*
 * Check byteData->value against value
 *
 * Return 1 if check is true (e.g. value > byteData.value)
 * Return 0 if check is not true.
 */
ENGINE_LINKAGE int checkValue(void *p, ByteData *byteData, u_int32_t value, const u_int8_t *cursor)
{
    switch (byteData->op)
    {
        case CHECK_EQ:
            if (value == byteData->value)
                return 1;
            break;
        case CHECK_NEQ:
            if (value != byteData->value)
                return 1;
            break;
        case CHECK_LT:
            if (value < byteData->value)
                return 1;
            break;
        case CHECK_GT:
            if (value > byteData->value)
                return 1;
            break;
        case CHECK_LTE:
            if (value <= byteData->value)
                return 1;
            break;
        case CHECK_GTE:
            if (value >= byteData->value)
                return 1;
            break;
        case CHECK_AND:
        case CHECK_ATLEASTONE:
            if ((value & byteData->value) != 0)
                return 1;
            break;
        case CHECK_XOR:
            if ((value ^ byteData->value) != 0)
                return 1;
            break;
        case CHECK_ALL:
            if ((value & byteData->value) == value)
                return 1;
            break;
        case CHECK_NONE:
            if ((value & byteData->value) == 0)
                return 1;
            break;
    }

    return 0;
}

/*
 * Check byteData->value against extracted value from data
 *
 * Return 1 if check is true (e.g. value > byteData.value)
 * Return 0 if check is not true.
 */
ENGINE_LINKAGE int byteTest(void *p, ByteData *byteData, const u_int8_t *cursor)
{
    int       ret;
    u_int32_t value;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = extractValueInternal(sp, byteData, &value, cursor);

    if ( ret < 0 )
        return 0;

    ret = checkValue(sp, byteData, value, cursor);
    
    return ret;
}

/*
 * Jump extracted value from data
 *
 * Return 1 if cursor in bounds
 * Return 0 if cursor out of bounds
 * Return < 0 if error
 */
ENGINE_LINKAGE int byteJump(void *p, ByteData *byteData, const u_int8_t **cursor)
{
    int       ret;
    u_int32_t readValue;
    u_int32_t jumpValue;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = extractValueInternal(sp, byteData, &readValue, *cursor);

    if ( ret < 0 )
        return ret;

    if (byteData->multiplier)
        jumpValue = readValue * byteData->multiplier;
    else
        jumpValue = readValue;

    if (byteData->flags & JUMP_ALIGN)
    {
        if ((jumpValue % 4) != 0)
        {
            jumpValue += (4 - (jumpValue % 4));
        }
    }

    if (!(byteData->flags & JUMP_FROM_BEGINNING))
    {
        jumpValue += byteData->bytes + byteData->offset;
    }

    jumpValue += byteData->post_offset;

    ret = setCursorInternal(sp, byteData->flags, jumpValue, cursor);
    
    return ret;
}
