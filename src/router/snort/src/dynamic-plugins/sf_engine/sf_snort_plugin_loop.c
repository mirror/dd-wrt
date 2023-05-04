/*
 *  sf_snort_plugin_loop.c
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steve Sturges
 *         Andy Mullican
 *
 * Date: 5/2005
 *
 *
 * Loop Option operations for dynamic rule engine
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sfghash.h"

#include "sf_dynamic_engine.h"
#include "sf_snort_detection_engine.h"

/* From sf_snort_plugin_api.c -- not exported from shared lib,
 * but available to other code within the shared lib.
 */
extern int RegisterOneRule(struct _SnortConfig *sc, Rule *rule, int registerRule);
extern int ruleMatchInternal(SFSnortPacket *p, Rule* rule, uint32_t optIndex, const uint8_t **cursor);

/* Initialize a byteExtract structure. */
int ByteExtractInitialize(Rule *rule, ByteExtract *extractData)
{
    int ret = 0;
    void *memoryLocation;
    if (rule->ruleData == NULL)
    {
        /* Initialize the hash table */

        /* XXX: 3 rows ought to suffice for now... */
        /* 3 rows,
         * 0 bytes key size (ie, its a string),
         * user provided keys,
         * free func -- data is pointer to int
         */
        rule->ruleData = (void *)sfghash_new(3, 0, 1, free);
    }

    memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, extractData->refId);

    if (memoryLocation)
    {
        /* Cannot re-use refId */
        DynamicEngineFatalMessage("Cannot re-use ByteExtract location '%s' for rule [%d:%d]\n",
                                  extractData->refId, rule->info.genID, rule->info.sigID);
        //return -1;
    }

    memoryLocation = calloc(sizeof(uint32_t), 1);
    if (memoryLocation == NULL)
    {
        DynamicEngineFatalMessage("Failed to allocate memory\n");
    }

    ret = sfghash_add((SFGHASH*)rule->ruleData, extractData->refId, memoryLocation);
    if (ret != SFGHASH_OK)
    {
        free(memoryLocation);

        /* Some error, couldn't allocate hash entry */
        return -2;
    }

    extractData->memoryLocation = memoryLocation;

    return 0;
}

int DynamicElementInitialize(Rule *rule, DynamicElement *element)
{
    void *memoryLocation;

    if (!rule->ruleData)
    {
        DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                  element->refId, rule->info.genID, rule->info.sigID);
    }

    switch (element->dynamicType)
    {
    case DYNAMIC_TYPE_INT_REF:
        memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, element->refId);
        if (memoryLocation)
        {
            element->data.dynamicInt = memoryLocation;
        }
        else
        {
            element->data.dynamicInt = NULL;
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                      element->refId, rule->info.genID, rule->info.sigID);
            //return -1;
        }
        break;
    case DYNAMIC_TYPE_INT_STATIC:
    default:
        /* nothing to do, its static */
        break;
    }

    return 0;
}

int LoopInfoInitialize(struct _SnortConfig *sc, Rule *rule, LoopInfo *loopInfo)
{
    int ret;

    /* Initialize the dynamic start, end, increment fields */
    ret = DynamicElementInitialize(rule, loopInfo->start);
    if (ret)
    {
        return ret;
    }

    ret = DynamicElementInitialize(rule, loopInfo->end);
    if (ret)
    {
        return ret;
    }

    ret = DynamicElementInitialize(rule, loopInfo->increment);
    if (ret)
    {
        return ret;
    }

    /* Do all of the initialization for the subrule */
    ret = RegisterOneRule(sc, loopInfo->subRule, DONT_REGISTER_RULE);
    if (ret)
    {
        return ret;
    }

    /* This should always be relative. */
    loopInfo->cursorAdjust->flags |= CONTENT_RELATIVE;

    /* Anything else? */
    return 0;
}


/*
 *  Get buffer size remaining
 *
 *          p: packet data structure, same as the one found in snort.
 *      flags: defines what kind of content buffer to look at
 *     cursor: current position within buffer
 *
 * Returns:
 *    > 0 : size of buffer remaining
 *    = 0 : no buffer remaining
 *    < 0 : error
 *
 */
int getSizeRemaining(void *p, uint32_t flags, const uint8_t *cursor)
{
    const uint8_t *start;
    const uint8_t *end;
    SFSnortPacket *sp = (SFSnortPacket *) p;
    int ret;
    int size;

    ret = getBuffer((void *)sp, (int)flags, (const uint8_t **)&start, (const uint8_t **)&end);

    if ( ret < 0 )
        return 0;

    if ( cursor != NULL )
    {
        size = end - cursor;
    }
    else
    {
        size = end - start;
    }

    if ( size < 0 )
        return -1;

    return size;
}

/*
 *  Get maximum loop iterations possible
 *
 *          p: packet data structure, same as the one found in snort.
 *       loop: structure that defines buffer via flags, and has cursor increment
 *     cursor: current position within buffer
 *
 * Returns:
 *    >= 0 : calculated max possible loop count
 *     < 0 : error
 *
 * Notes:
 *    This function is a sanity check on a loop count.  It presumes the caller is looking
 *    through a content buffer, cursor_increment hops at a time.  It calculates how many whole
 *    hops (plus a last partial hop) are possible given the remaining buffer size.  Passing in
 *    a cursor of NULL means look at the whole buffer.
 *
 */
int32_t getLoopLimit(void *p, LoopInfo *loop, const uint8_t *cursor)
{
    int32_t loop_max;
    int size;

    size = getSizeRemaining(p, loop->cursorAdjust->flags, cursor);

    if ( size < 0 )
        return -1;

    /* Calculate how many whole hops are within buffer */
    loop_max = size/(loop->cursorAdjust->offset);

    /* Add one for partial hop remaining */
    if ( size%(loop->cursorAdjust->offset) != 0 )
        loop_max++;

    /* Sanity check; limit size to 65535 */
    return loop_max & 0xFFFF;
}

int checkLoopEnd(uint32_t op, int32_t index, int32_t end)
{
    switch (op)
    {
        case CHECK_EQ:
            if (index == end)
                return 1;
            break;
        case CHECK_NEQ:
            if (index != end)
                return 1;
            break;
        case CHECK_LT:
            if (index < end)
                return 1;
            break;
        case CHECK_GT:
            if (index > end)
                return 1;
            break;
        case CHECK_LTE:
            if (index <= end)
                return 1;
            break;
        case CHECK_GTE:
            if (index >= end)
                return 1;
            break;
        case CHECK_AND:
        case CHECK_ATLEASTONE:
            if ((index & end) != 0)
                return 1;
            break;
        case CHECK_XOR:
            if ((index ^ end) != 0)
                return 1;
            break;
        case CHECK_ALL:
            if ((index & end) == index)
                return 1;
            break;
        case CHECK_NONE:
            if ((index & end) == 0)
                return 1;
            break;
    }

    return 0;
}

/* Function to evaluate a loop (ie, a series of nested options) */
ENGINE_LINKAGE int loopEval(void *p, LoopInfo *loop, const uint8_t **cursor)
{
    const uint8_t *startingCursor;
    const uint8_t *tmpCursor;
    int32_t i;
    int32_t startValue;
    int32_t endValue;
    int32_t incrementValue;
    int maxIterations, iterationCount = 0;
    int ret = RULE_NOMATCH;

    if (!cursor || !*cursor)
        return RULE_NOMATCH;

    /* Protect ourselves... */
    if (!loop->initialized)
        return RULE_NOMATCH;

    tmpCursor = startingCursor = *cursor;

    if (loop->start->dynamicType == DYNAMIC_TYPE_INT_STATIC)
        startValue = loop->start->data.staticInt;
    else
        startValue = *(loop->start->data.dynamicInt);

    if (loop->end->dynamicType == DYNAMIC_TYPE_INT_STATIC)
        endValue = loop->end->data.staticInt;
    else
        endValue = *(loop->end->data.dynamicInt);

    if (loop->increment->dynamicType == DYNAMIC_TYPE_INT_STATIC)
        incrementValue = loop->increment->data.staticInt;
    else
        incrementValue = *(loop->increment->data.dynamicInt);

    /* determine max iterations - based on current cursor position,
     * relative to the appropriate buffer */
    maxIterations = getLoopLimit(p, loop, startingCursor);

    for (i=startValue;
         checkLoopEnd(loop->op, i, endValue) && (iterationCount < maxIterations);
         i+= incrementValue)
    {
        /* Evaluate the options from the sub rule */
        ret = ruleMatchInternal(p, loop->subRule, 0, &tmpCursor);
        if (ret > RULE_NOMATCH)
        {
            *cursor = tmpCursor;
            return ret;
        }

        /* Adjust the starting cursor as specified... */
        tmpCursor = startingCursor;
        ret = setCursor(p, loop->cursorAdjust, &tmpCursor);
        if (ret != RULE_MATCH)
        {
            /* ie, cursor went out of bounds */
            return ret;
        }

        /* And save off the starting cursor */
        startingCursor = tmpCursor;

        iterationCount++;
    }

    return RULE_NOMATCH;
}

