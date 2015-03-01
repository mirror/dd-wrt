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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steve Sturges
 *         Andy Mullican
 *
 * Date: 5/2005
 *
 *
 * Byte operations for dynamic rule engine
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sfghash.h"
#include "sf_snort_detection_engine.h"

extern int checkCursorSimple(const uint8_t *cursor, int flags, const uint8_t *start, const uint8_t *end, int offset);
extern int setCursorInternal(void *p, int flags, int offset, const uint8_t **cursor);
static int byteTestInternal(void *, ByteData *, const uint8_t *);
static int byteJumpInternal(void *, ByteData *, const uint8_t **);

#define BYTE_STRING_LEN     11


int ByteDataInitialize(Rule *rule, ByteData *byte)
{
    void *memoryLocation;

    /* Initialize byte_extract pointers */
    if (byte->offset_refId)
    {
        if (!rule->ruleData)
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       byte->offset_refId, rule->info.genID, rule->info.sigID);
        }

        memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, byte->offset_refId);
        if (memoryLocation)
        {
            byte->offset_location = memoryLocation;
        }
        else
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       byte->offset_refId, rule->info.genID, rule->info.sigID);
        }
    }

    if (byte->value_refId)
    {
        if (!rule->ruleData)
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       byte->value_refId, rule->info.genID, rule->info.sigID);
        }

        memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, byte->value_refId);
        if (memoryLocation)
        {
            byte->value_location = memoryLocation;
        }
        else
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       byte->value_refId, rule->info.genID, rule->info.sigID);
        }
    }

    return 0;
}

/*
 * extract byte value from data
 *
 * Return 1 if successfully extract value.
 * Return < 0 if fail to extract value.
 */
int extractValueInternal(void *p, ByteData *byteData, uint32_t *value, const uint8_t *cursor)
{
    char byteArray[BYTE_STRING_LEN];
    uint32_t i;
    char *endPtr;
    uint32_t extracted = 0;
    int base = 10;
    const uint8_t *start;
    const uint8_t *end;
    int ret;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = getBuffer(sp, byteData->flags, &start, &end);

    if ( ret < 0 )
    {
        return ret;
    }

    /* Check for byte_extract variables and use them if present. */
    if (byteData->offset_location)
    {
        byteData->offset = *byteData->offset_location;
    }
    if (byteData->value_location)
    {
        byteData->value = *byteData->value_location;
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
        const uint8_t *space_ptr = cursor + byteData->offset;

        if (byteData->bytes < 1 || byteData->bytes > (BYTE_STRING_LEN - 1))
        {
            /* Log Error message */
            return -2;
        }

        // Only positive numbers should be processed and strtoul will
        // eat up white space and process '-' and '+' so move past
        // white space and check for a negative sign.
        while ((space_ptr < (cursor + byteData->offset + byteData->bytes))
                && isspace((int)*space_ptr))
            space_ptr++;

        // If all spaces or a negative sign is found, return error.
        if ((space_ptr == (cursor + byteData->offset + byteData->bytes))
                || (*space_ptr == '-'))
            return -2;

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
ENGINE_LINKAGE int extractValue(void *p, ByteExtract *byteExtract, const uint8_t *cursor)
{
    ByteData byteData;
    int ret;
    uint32_t extracted = 0;
    uint32_t *location = (uint32_t *)byteExtract->memoryLocation;

    byteData.bytes = byteExtract->bytes;
    byteData.flags = byteExtract->flags;
    byteData.multiplier = byteExtract->multiplier;
    byteData.offset = byteExtract->offset;

    /* The following fields are not used, but must be zeroed out. */
    byteData.op = 0;
    byteData.value = 0;
    byteData.offset_refId = 0;
    byteData.value_refId = 0;
    byteData.offset_location = 0;
    byteData.value_location = 0;

    ret = extractValueInternal(p, &byteData, &extracted, cursor);
    if (byteExtract->flags & NOT_FLAG)
        ret = invertMatchResult(ret);
    if (ret > 0)
    {
        if ((byteExtract->align == 2) || (byteExtract->align == 4))
        {
            extracted = extracted + byteExtract->align - (extracted % byteExtract->align);
        }
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
ENGINE_LINKAGE int checkValue(void *p, ByteData *byteData, uint32_t value, const uint8_t *cursor)
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

ENGINE_LINKAGE int byteTest(void *p, ByteData *byteData, const uint8_t *cursor)
{
    if (byteData->flags & NOT_FLAG)
        return invertMatchResult(byteTestInternal(p, byteData, cursor));
    return byteTestInternal(p, byteData, cursor);
}

/*
 * Check byteData->value against extracted value from data
 *
 * Return 1 if check is true (e.g. value > byteData.value)
 * Return 0 if check is not true.
 */
static int byteTestInternal(void *p, ByteData *byteData, const uint8_t *cursor)
{
    int       ret;
    uint32_t value;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = extractValueInternal(sp, byteData, &value, cursor);

    if ( ret < 0 )
        return 0;

    ret = checkValue(sp, byteData, value, cursor);

    return ret;
}

ENGINE_LINKAGE int byteJump(void *p, ByteData *byteData, const uint8_t **cursor)
{
    if (byteData->flags & NOT_FLAG)
        return invertMatchResult(byteJumpInternal(p, byteData, cursor));
    return byteJumpInternal(p, byteData, cursor);
}

/*
 * Jump extracted value from data
 *
 * Return 1 if cursor in bounds
 * Return 0 if cursor out of bounds
 * Return < 0 if error
 */
static int byteJumpInternal(void *p, ByteData *byteData, const uint8_t **cursor)
{
    int       ret;
    uint32_t readValue;
    uint32_t jumpValue;
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
