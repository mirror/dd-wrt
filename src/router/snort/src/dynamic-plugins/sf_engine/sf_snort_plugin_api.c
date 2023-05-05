/*
 * sf_snort_plugin_api.c
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
 *         Andy  Mullican
 *
 * Date: 5/2005
 *
 * Dyanmic Rule Engine
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_engine.h"
#include "sf_snort_detection_engine.h"
#include "sfghash.h"
#define BLEN 65535

const uint8_t base64decodebuf[BLEN];
uint32_t base64decodesize;

static int processFlowbitsInternal(void *, FlowBitsInfo *);
static int checkFlowInternal(void *, FlowFlags *);
static int detectAsn1Internal(void *, Asn1Context*, const uint8_t *);
static int fileDataInternal(void *, CursorInfo*, const uint8_t **);
static int base64DataInternal(void *, CursorInfo*, const uint8_t **);

int CursorInfoInitialize(Rule *rule, CursorInfo *cursor)
{
    void *memoryLocation;

    /* Initialize byte_extract pointers */
    if (cursor->offset_refId)
    {
        if (!rule->ruleData)
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       cursor->offset_refId, rule->info.genID, rule->info.sigID);
        }

        memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, cursor->offset_refId);
        if (memoryLocation)
        {
            cursor->offset_location = memoryLocation;
        }
        else
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       cursor->offset_refId, rule->info.genID, rule->info.sigID);
        }
    }

    return 0;
}

/*
 *  Get the start and end of the buffer, as divined by the packet flags.
 *  This function is useful so that we don't have to figure out which buffer
 *  we are looking at twice, if we want to check two offsets one after the other.
 *
 *  return 1 if successful
 *  return < 0 if unsuccessful
 */
ENGINE_LINKAGE int getBuffer(void *packet, int flags, const uint8_t **start, const uint8_t **end)
{
    SFSnortPacket *p = (SFSnortPacket *)packet;
    unsigned hb_type;

    if ((flags & CONTENT_BUF_NORMALIZED) && (_ded.Is_DetectFlag(SF_FLAG_DETECT_ALL)))
    {
        if(_ded.Is_DetectFlag(SF_FLAG_ALT_DETECT))
        {
            *start = _ded.altDetect->data;
            *end = *start + _ded.altDetect->len;
        }
        else if(_ded.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        {
            *start = _ded.altBuffer->data;
            *end = *start + _ded.altBuffer->len;
        }
    }
    else if ((flags & CONTENT_BUF_RAW) || (flags & CONTENT_BUF_NORMALIZED))
    {
        *start = p->payload;

        if(p->normalized_payload_size)
            *end = *start + p->normalized_payload_size;
        else
            *end = *start + p->payload_size;
    }
    else if ( (hb_type = HTTP_CONTENT(flags)) )
    {
        if (p->flags & FLAG_HTTP_DECODE)
        {
            unsigned len;
            *start = _ded.getHttpBuffer(hb_type, &len);

            if ( !*start )
                return CONTENT_TYPE_MISMATCH;

            *end = *start + len;
        }
        else
        {
            return CONTENT_TYPE_MISMATCH;
        }
    }
    else
    {
        return CONTENT_TYPE_MISSING;
    }
    return CURSOR_IN_BOUNDS;
}

int checkCursorSimple(const uint8_t *cursor, int flags, const uint8_t *start, const uint8_t *end, int offset)
{
    if ( cursor == NULL || !(flags & CONTENT_RELATIVE) )
        cursor = start;

    if ((cursor + offset < end) && (cursor + offset >= start))
        return CURSOR_IN_BOUNDS;

    return CURSOR_OUT_OF_BOUNDS;
}

int checkCursorSimple_end(const uint8_t *cursor, int flags, const uint8_t *start, const uint8_t *end, int offset)
{
    if (cursor == NULL)
         cursor = end;

    if ((cursor + offset <= end) && (cursor + offset >= start))
        return CURSOR_IN_BOUNDS;

    return CURSOR_OUT_OF_BOUNDS;
}


/* Returns one if cursor is within the buffer */
int checkCursorInternal(void *p, int flags, int offset, const uint8_t *cursor)
{
    const uint8_t *start;
    const uint8_t *end;
    int ret;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = getBuffer(sp, flags, &start, &end);

    if ( ret < 0 )
    {
        return ret;
    }

    return checkCursorSimple(cursor, flags, start, end, offset);
}

int setCursorInternal(void *p, int flags, int offset, const uint8_t **cursor)
{
    const uint8_t *start;
    const uint8_t *end;
    int       ret;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    if(!cursor)
    {
        return RULE_NOMATCH;
    }

    ret = getBuffer(sp, flags, &start, &end);

    if ( ret < 0 )
    {
        return ret;
    }

    if ( flags & JUMP_FROM_BEGINNING )
    {
        ret = checkCursorSimple(start, flags, start, end, offset);
    }
    else if ( flags & JUMP_FROM_END )
    {
        ret = checkCursorSimple_end(end, flags, start, end, offset);
    }
    else
    {
        ret = checkCursorSimple(*cursor, flags, start, end, offset);
    }

    if ( ret <= 0 )
    {
        return ret;
    }

    if ( flags & JUMP_FROM_BEGINNING )
    {
        *cursor = start + offset;
    }
    else if ( flags & JUMP_FROM_END )
    {
        *cursor = end + offset;
    }
    else if ( flags & CONTENT_RELATIVE )
    {
        *cursor += offset;
    }
    else
    {
        *cursor = start + offset;
    }

    return CURSOR_IN_BOUNDS;
}

        /*    API FUNCTIONS    */

/*
 *  Check cursor function
 *
 *          p: packet data structure, same as the one found in snort.
 * cursorInfo: data defined in the detection plugin for this rule cursor option
 *     cursor: current position within buffer
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *    < 0 : error
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    CURSOR_IN_BOUNDS     -  if content specifier is found within buffer
 *    CURSOR_OUT_OF_BOUNDS -  if content specifier is not found within buffer
 *
 * Notes:
 *   Since we are checking the cursor position within a buffer, relativity is assumed.
 *     To check absolute position within a buffer, a NULL pointer can be passed in.
 *     In this case, offset will be checked from the start of the given buffer.
 *
 *   Currently support:
 *    options:
 *      offset
 *    buffers:
 *      normalized(alt-decode)
 *      raw
 *      uri
 *
 */
ENGINE_LINKAGE int checkCursor(void *p, CursorInfo* cursorInfo, const uint8_t *cursor)
{
    /* Get byte_extracted offset if present */
    if (cursorInfo->offset_location)
        cursorInfo->offset = *cursorInfo->offset_location;
    if (cursorInfo->flags & NOT_FLAG)
        return invertMatchResult(checkCursorInternal(p, cursorInfo->flags, cursorInfo->offset, cursor));
    return checkCursorInternal(p, cursorInfo->flags, cursorInfo->offset, cursor);
}

/*
 *  Set cursor function
 *
 *          p: packet data structure, same as the one found in snort.
 * cursorInfo: data defined in the detection plugin for this rule cursor option
 *     cursor: updated to point to offset bytes after the buffer start
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *    < 0 : error
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    CURSOR_IN_BOUNDS     -  if content specifier is found within buffer
 *    CURSOR_OUT_OF_BOUNDS -  if content specifier is not found within buffer
 *
 * Notes:
 *
 *   Currently support:
 *    options:
 *      offset
 *    buffers:
 *      normalized(alt-decode)
 *      raw
 *      uri
 *
 */
ENGINE_LINKAGE int setCursor(void *p, CursorInfo* cursorInfo, const uint8_t **cursor)
{
    if (cursorInfo->flags & NOT_FLAG)
        return invertMatchResult(setCursorInternal(p, cursorInfo->flags, cursorInfo->offset, cursor));
    return setCursorInternal(p, cursorInfo->flags, cursorInfo->offset, cursor);
}

ENGINE_LINKAGE void setTempCursor(const uint8_t **temp_cursor, const uint8_t **cursor)
{
    *temp_cursor = *cursor;
}

ENGINE_LINKAGE void revertTempCursor(const uint8_t **temp_cursor, const uint8_t **cursor)
{
    *cursor = *temp_cursor;
}

ENGINE_LINKAGE int checkFlow(void *p, FlowFlags *flowFlags)
{
    if (flowFlags->flags & NOT_FLAG)
        return invertMatchResult(checkFlowInternal(p, flowFlags));
    return checkFlowInternal(p, flowFlags);
}

/*
 *  Check flow function
 *
 *          p: packet data structure, same as the one found in snort.
 *  flowFlags: data defined in the detection plugin for this rule option
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    RULE_MATCH   -  if packet flow matches rule
 *    RULE_NOMATCH -  if packet flow does not match rule
 *
 */
static int checkFlowInternal(void *p, FlowFlags *flowFlags)
{
    SFSnortPacket *sp = (SFSnortPacket *) p;

#define FLOW_DIRECTION_FLAGS (FLOW_ESTABLISHED | FLOW_TO_CLIENT | FLOW_TO_SERVER)

    /* Check that the flags set in the flow structure are the same ones
     * set in the packet -- covers established, and to/from client/server */
    if ((sp->flags & (flowFlags->flags & FLOW_DIRECTION_FLAGS)) != (flowFlags->flags & FLOW_DIRECTION_FLAGS))
        return RULE_NOMATCH;

    /* check if this rule only applies to reassembled */
    if (flowFlags->flags & FLOW_ONLY_REASSEMBLED)
    {
        if ( !(PacketHasPAFPayload(sp)) )
            return RULE_NOMATCH;
    }
    /* check if this rule only applies to non-reassembled */
    if ((flowFlags->flags & FLOW_IGNORE_REASSEMBLED) &&
        (sp->flags & FLAG_REBUILT_STREAM))
        return RULE_NOMATCH;

    return RULE_MATCH;
}

ENGINE_LINKAGE int processFlowbits(void *p, FlowBitsInfo *flowBits)
{
    if (flowBits->flags & NOT_FLAG)
        return invertMatchResult(processFlowbitsInternal(p, flowBits));
    return processFlowbitsInternal(p, flowBits);
}

/*
 *  Process flowbits function
 *
 *          p: packet data structure, same as the one found in snort.
 *   flowBits: data defined in the detection plugin for this rule option
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    RULE_MATCH   -  if flowbit operation succeeded
 *    RULE_NOMATCH -  if flowbit operation failed
 *
 */
static int processFlowbitsInternal(void *p, FlowBitsInfo *flowBits)
{
    /* flowbitCheck returns non-zero if the flow bit operation succeeded. */
    if (_ded.flowbitCheck(p, flowBits) == RULE_MATCH)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

ENGINE_LINKAGE int detectAsn1(void *p, Asn1Context* asn1, const uint8_t *cursor)
{
    if (asn1->flags & NOT_FLAG)
        return invertMatchResult(detectAsn1Internal(p, asn1, cursor));
    return detectAsn1Internal(p, asn1, cursor);
}

/*
 *  Detect ASN1 function
 *
 *          p: packet data structure, same as the one found in snort.
 *       asn1: data defined in the detection plugin for this rule option
 *     cursor: current position within buffer
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    RULE_MATCH   -  if asn1 specifier is found within buffer
 *    RULE_NOMATCH -  if asn1 specifier is not found within buffer
 *
 */
static int detectAsn1Internal(void *p, Asn1Context* asn1, const uint8_t *cursor)
{
    /* asn1Detect returns non-zero if the options matched. */
    if (_ded.asn1Detect(p, (void *) asn1, cursor))
        return RULE_MATCH;

    return RULE_NOMATCH;
}

ENGINE_LINKAGE int fileData(void *p, CursorInfo* cursorInfo, const uint8_t **cursor)
{
    if (cursorInfo->flags & NOT_FLAG)
        return invertMatchResult(fileDataInternal(p, cursorInfo, cursor));
    return fileDataInternal(p, cursorInfo, cursor);
}

static int fileDataInternal(void *p, CursorInfo* cursorInfo, const uint8_t **cursor)
{
    int retVal = RULE_NOMATCH;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    if((sp->payload_size == 0) || (_ded.fileDataBuf->data == NULL) || (_ded.fileDataBuf->len == 0) )
    {
        return RULE_NOMATCH;
    }
    _ded.SetAltDetect((uint8_t*)_ded.fileDataBuf->data, _ded.fileDataBuf->len);
    retVal = setCursorInternal(p, cursorInfo->flags, cursorInfo->offset, cursor);

    if( retVal > RULE_NOMATCH)
        return RULE_MATCH;

    _ded.DetectFlag_Disable(SF_FLAG_ALT_DETECT);

    return retVal;
}

ENGINE_LINKAGE int pktData(void *p, CursorInfo* cursorInfo, const uint8_t **cursor)
{
    _ded.DetectFlag_Disable(SF_FLAG_ALT_DETECT);
    cursorInfo->flags |= JUMP_FROM_BEGINNING;
    return setCursor(p, cursorInfo, cursor);
}

ENGINE_LINKAGE int base64Data(void *p, CursorInfo* cursorInfo, const uint8_t **cursor)
{
    if (cursorInfo->flags & NOT_FLAG)
        return invertMatchResult(base64DataInternal(p, cursorInfo, cursor));
    return base64DataInternal(p, cursorInfo, cursor);
}

static int base64DataInternal(void *p, CursorInfo* cursorInfo, const uint8_t **cursor)
{
    int retVal = RULE_NOMATCH;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    if((sp->payload_size == 0) || !base64decodesize )
        return retVal;

    _ded.SetAltDetect((uint8_t *)base64decodebuf, (uint16_t)base64decodesize);
    retVal = setCursorInternal(p, cursorInfo->flags, cursorInfo->offset, cursor);

    if( retVal > RULE_NOMATCH)
        return RULE_MATCH;

    _ded.DetectFlag_Disable(SF_FLAG_ALT_DETECT);
    return retVal;
}

ENGINE_LINKAGE int base64Decode(void *p, base64DecodeData *data, const uint8_t *cursor)
{
    const uint8_t *start;
    const uint8_t *end;
    int       ret;
    const uint8_t base64_encodebuf[BLEN];
    uint32_t base64_encodesize = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    ret = getBuffer(sp, CONTENT_BUF_RAW, &start, &end);

    if ( ret < 0 )
    {
        return ret;
    }


    if(data->relative )
    {
        if(cursor)
        {
            start = cursor;
        }
    }
    start = start + data->offset;

    if( start >= end )
        return RULE_NOMATCH;

    if(_ded.sfUnfold(start, end-start, (uint8_t *)base64_encodebuf, sizeof(base64_encodebuf), &base64_encodesize) != 0)
    {
        return RULE_NOMATCH;
    }

   if (data->bytes && (base64_encodesize > data->bytes))
   {
       base64_encodesize = data->bytes;
   }

    if(_ded.sfbase64decode((uint8_t *)base64_encodebuf, base64_encodesize, (uint8_t *)base64decodebuf, BLEN, &base64decodesize) != 0)
    {
        return RULE_NOMATCH;
    }


    return RULE_MATCH;
}

ENGINE_LINKAGE int isDetectFlag(SFDetectFlagType df)
{
    return _ded.Is_DetectFlag(df);
}

ENGINE_LINKAGE void detectFlagDisable(SFDetectFlagType df)
{
    _ded.DetectFlag_Disable(df);
}

ENGINE_LINKAGE int getAltDetect(uint8_t **bufPtr, uint16_t *altLenPtr)
{
    return _ded.GetAltDetect(bufPtr, altLenPtr);
}

ENGINE_LINKAGE void setAltDetect(uint8_t *buf, uint16_t altLen)
{
    _ded.SetAltDetect(buf, altLen);
}

/*
 *  Store Rule Specific session data
 *
 *          p: packet data structure, same as the one found in snort.
 *          rule_data: data to store in the session
 *
 * Returns:
 *    nothing
 *
 */
ENGINE_LINKAGE int storeRuleData(void *p, const RuleInformation *info,
                                 void *rule_data, void *rule_compression_data)
{
    if ( _ded.setRuleData(p, info, rule_data, rule_compression_data) != 0 )
        return RULE_NOMATCH;

    return RULE_MATCH;
}

/*
 *  Retrieve Rule Specific session data
 *
 *          p: packet data structure, same as the one found in snort.
 *
 * Returns:
 *    pointer to rule specific session data, NULL if none available
 *
 */
ENGINE_LINKAGE void getRuleData(void *p, const RuleInformation *info,
                                void **rule_data, void **rule_compression_data)
{
    _ded.getRuleData(p, info, rule_data, rule_compression_data);
}

ENGINE_LINKAGE void * allocRuleData(size_t size)
{
    return _ded.allocRuleData(size);
}

ENGINE_LINKAGE void freeRuleData(void *data)
{
    _ded.freeRuleData(data);
}

/*
 *  Preprocessor Defined Detection
 *
 *          p: packet data structure, same as the one found in snort.
 * preprocOpt: data defined in the detection plugin for this rule preprocessor specific option
 *     cursor: current position within buffer
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    RULE_MATCH   -  if preprocessor indicates match
 *    RULE_NOMATCH -  if preprocessor indicates no match
 *
 */
ENGINE_LINKAGE int preprocOptionEval(void *p, PreprocessorOption *preprocOpt, const uint8_t **cursor)
{
    PreprocOptionEval evalFunc = (PreprocOptionEval)preprocOpt->optionEval;
    if (preprocOpt->flags & NOT_FLAG)
        return invertMatchResult(evalFunc(p, cursor, preprocOpt->dataPtr));
    return evalFunc(p, cursor, preprocOpt->dataPtr);
}

int isRelativeOption(RuleOption *option)
{
    int thisType = option->optionType;
    int relative = 0;

    switch (thisType)
    {
    case OPTION_TYPE_CONTENT:
        relative = option->option_u.content->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_PCRE:
        relative = option->option_u.pcre->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_FLOWBIT:
        /* Never relative */
        break;
    case OPTION_TYPE_BYTE_TEST:
        relative = option->option_u.byte->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_BYTE_MATH:
        relative = option->option_u.byte->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_BYTE_JUMP:
        relative = option->option_u.byte->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_BYTE_EXTRACT:
        relative = option->option_u.byteExtract->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_FLOWFLAGS:
        /* Never relative */
        break;
    case OPTION_TYPE_ASN1:
        relative = option->option_u.asn1->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_CURSOR:
        relative = option->option_u.cursor->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_LOOP:
        /* A Loop is relative, only in that the cursor adjust for
         * the loop is always relative.
         */
        relative = option->option_u.loop->cursorAdjust->flags & CONTENT_RELATIVE;
        break;
    case OPTION_TYPE_BASE64_DECODE:
        relative = option->option_u.bData->relative;
        break;
    case OPTION_TYPE_HDR_CHECK:
    case OPTION_TYPE_FILE_DATA:
    case OPTION_TYPE_PKT_DATA:
    case OPTION_TYPE_BASE64_DATA:
    case OPTION_TYPE_PREPROCESSOR:
        /* Never relative */
        break;
    }

    return relative;
}

/*
 *  ruleMatch
 *
 *          p: packet data structure, same as the one found in snort.
 *    options: NULL terminated list of rule options
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    RULE_MATCH   -  if asn1 specifier is found within buffer
 *    RULE_NOMATCH -  if asn1 specifier is not found within buffer
 *
 */
int ruleMatchInternal(SFSnortPacket *p, Rule* rule, uint32_t optIndex, const uint8_t **cursor)
{
    const uint8_t *thisCursor = NULL, *startCursor = NULL;
    const uint8_t *tmpCursor = NULL;
    int retVal = RULE_NOMATCH;
    uint32_t notFlag = 0;
    int thisType;
    ContentInfo *thisContentInfo = NULL;
    int startAdjust = 0;
    uint32_t origFlags = 0;
    int32_t origOffset = 0;
    uint32_t origDepth = 0;
    int continueLoop = 1;
    PCREInfo *thisPCREInfo = NULL;

    if (cursor)
        startCursor = thisCursor = *cursor;

    if (optIndex >= rule->numOptions || !rule->options[optIndex] )
        return RULE_NOMATCH;

    thisType = rule->options[optIndex]->optionType;

    /* Do some saving off of some info for recursion purposes */
    switch (thisType)
    {
        case OPTION_TYPE_CONTENT:
            thisContentInfo = rule->options[optIndex]->option_u.content;
            origFlags = thisContentInfo->flags;
            origDepth = thisContentInfo->depth;
            origOffset = thisContentInfo->offset;
            break;
        case OPTION_TYPE_PCRE:
            thisPCREInfo = rule->options[optIndex]->option_u.pcre;
            origFlags = thisPCREInfo->flags;
            origOffset = thisPCREInfo->offset;
            break;
        default:
            /* Other checks should not need to check again like
             * PCRE & Content do.
             */
            break;
    }

    do
    {
        switch (thisType)
        {
        case OPTION_TYPE_CONTENT:
            retVal = contentMatch(p, rule->options[optIndex]->option_u.content, &thisCursor);
            notFlag = rule->options[optIndex]->option_u.content->flags & NOT_FLAG;
            break;
        case OPTION_TYPE_PROTECTED_CONTENT:
            retVal = protectedContentMatch(p, rule->options[optIndex]->option_u.protectedContent, &thisCursor);
            notFlag = rule->options[optIndex]->option_u.protectedContent->flags & NOT_FLAG;
            break;
        case OPTION_TYPE_PCRE:
            retVal = pcreMatch(p, rule->options[optIndex]->option_u.pcre, &thisCursor);
            notFlag = rule->options[optIndex]->option_u.pcre->flags & NOT_FLAG;
            break;
        case OPTION_TYPE_FLOWBIT:
            retVal = processFlowbits(p, rule->options[optIndex]->option_u.flowBit);
            break;
        case OPTION_TYPE_BYTE_TEST:
            retVal = byteTest(p, rule->options[optIndex]->option_u.byte, thisCursor);
            break;
        case OPTION_TYPE_BYTE_MATH:
            retVal = byteMath(p, rule->options[optIndex]->option_u.byte, thisCursor);
            break;
        case OPTION_TYPE_BYTE_JUMP:
            retVal = byteJump(p, rule->options[optIndex]->option_u.byte, &thisCursor);
            break;
        case OPTION_TYPE_FLOWFLAGS:
            retVal = checkFlow(p, rule->options[optIndex]->option_u.flowFlags);
            break;
        case OPTION_TYPE_ASN1:
            retVal = detectAsn1(p, rule->options[optIndex]->option_u.asn1, thisCursor);
            break;
        case OPTION_TYPE_CURSOR:
            retVal = checkCursor(p, rule->options[optIndex]->option_u.cursor, thisCursor);
            break;
        case OPTION_TYPE_SET_CURSOR:
            retVal = setCursor(p, rule->options[optIndex]->option_u.cursor, &thisCursor);
            break;
        case OPTION_TYPE_FILE_DATA:
            retVal = fileData(p, rule->options[optIndex]->option_u.cursor, &thisCursor);
            break;
        case OPTION_TYPE_PKT_DATA:
            retVal = pktData(p, rule->options[optIndex]->option_u.cursor, &thisCursor);
            break;
        case OPTION_TYPE_BASE64_DATA:
            retVal = base64Data(p, rule->options[optIndex]->option_u.cursor, &thisCursor);
            break;
        case OPTION_TYPE_BASE64_DECODE:
            retVal = base64Decode(p, rule->options[optIndex]->option_u.bData, thisCursor);
            break;
        case OPTION_TYPE_HDR_CHECK:
            retVal = checkHdrOpt(p, rule->options[optIndex]->option_u.hdrData);
            break;
        case OPTION_TYPE_BYTE_EXTRACT:
            retVal = extractValue(p, rule->options[optIndex]->option_u.byteExtract, thisCursor);
            break;
        case OPTION_TYPE_LOOP:
            retVal = loopEval(p, rule->options[optIndex]->option_u.loop, &thisCursor);
            notFlag = rule->options[optIndex]->option_u.loop->flags & NOT_FLAG;
            if (notFlag)
                retVal = invertMatchResult(retVal);
            break;
        case OPTION_TYPE_PREPROCESSOR:
            retVal = preprocOptionEval(p, rule->options[optIndex]->option_u.preprocOpt, &thisCursor);
            break;
        }

        if (retVal > RULE_NOMATCH)
        {
            /* This one matched.  Depending on type, check the next one
             * either in a loop, or not, saving cursor temporarily.
             */
            if (optIndex < rule->numOptions -1) /* hehe, optIndex is 0 based */
            {
                int nestedRetVal;
                /* Here's where it gets tricky... */
                if (thisType == OPTION_TYPE_CONTENT)
                {
                    /* If this is a content option, we've found a match.
                     * Save off the end-point of the current match.
                     * Less the length of current match plus 1.
                     *
                     * This gives us the starting point to check for this
                     * content again if subsequent options fail.  That starting
                     * point is the byte AFTER the beginning of the current
                     * match.
                     */
                    if ((origFlags & CONTENT_RELATIVE) && startCursor)
                    {
                        /* relative content.
                         * need to adjust offset/depth as well
                         */
                        tmpCursor = thisCursor -
                            thisContentInfo->patternByteFormLength + thisContentInfo->incrementLength;

                        /* Start Adjust is the difference between the old
                         * starting point and the 'next' starting point. */
                        startAdjust = tmpCursor - startCursor;
                    }
                    else
                    {
                        /* non-relative content */
                        tmpCursor = thisCursor -
                            thisContentInfo->patternByteFormLength + thisContentInfo->incrementLength;
                    }
                }
                else if (thisType == OPTION_TYPE_PCRE)
                {
                    /* Start next search at end of current pattern */
                    /* XXX: Could miss something here if part of pattern
                     * repeats but not easy to tell with PCRE.
                     */
                    tmpCursor = thisCursor;

                    /* Start Adjust is the difference between the old
                     * starting point and the 'next' starting point. */
                    startAdjust = tmpCursor - startCursor;
                }

                nestedRetVal = ruleMatchInternal(p, rule, optIndex+1, &thisCursor);
                if (nestedRetVal == RULE_MATCH)
                {
                    /* Cool, everyone after us matched, we're done with a match */
                    if (cursor)
                        *cursor = thisCursor;
                    break;
                }

                /* If Content or PCRE, look farther into the packet
                 * for another match. */
                if (((thisType == OPTION_TYPE_CONTENT) ||
                     (thisType == OPTION_TYPE_PCRE))
                     && !notFlag)
                {
                    /* Only try to find this content again if it is a
                     * positive match.
                     */

                    /* And only if the next option is relative */
                    if (!isRelativeOption(rule->options[optIndex+1]))
                    {
                        /* Match failed, next option is not relative.
                         * We're done. */
                        retVal = nestedRetVal;
                        break;
                    }

                    switch (thisType)
                    {
                    case OPTION_TYPE_CONTENT:
                        if (origFlags & CONTENT_RELATIVE)
                        {
                            if ((int32_t)(origDepth - (startAdjust - origOffset)) < (int32_t)thisContentInfo->patternByteFormLength)
                            {
                                /* Adjusted depth would be less than the content we're searching for?
                                 * we're done. */
                                retVal = nestedRetVal;
                                continueLoop = 0;
                            }
                            else
                            {
                                /* For contents that were already relative, adjust the offset & depth fields
                                 * from their original values.  Makes it easy to determine when we'll be out
                                 * of the original bounds, relative to the original cursor. */
                                thisContentInfo->offset = origOffset + startAdjust;
                                thisContentInfo->depth = origDepth - startAdjust;

                                /* And use the original cursor that was passed in */
                                thisCursor = startCursor;
                            }
                        }
                        else
                        {
                            thisContentInfo->flags |= CONTENT_RELATIVE;
                            /* For contents that were not already relative, we simply use the adjusted
                             * cursor.  Set thisCursor to tmpCursor as calculated above */
                            thisCursor = tmpCursor;
                        }
                        break;
                    case OPTION_TYPE_PCRE:

                        if (origFlags & CONTENT_RELATIVE)
                        {
                            thisCursor = startCursor;

                            thisPCREInfo->offset = origOffset + startAdjust;
                        }
                        else
                        {
                            thisPCREInfo->flags |= CONTENT_RELATIVE;
                            /* For PCREs that were not already relative, we use the cursor
                             * that was returned at the end of the pattern to start searching
                             * again. */
                            thisCursor = tmpCursor;
                        }
                        break;
                    }
                    continue;
                }


                /* Only need to search again when this is a
                 * content option.  If its not, we're done. */
                if (nestedRetVal <= RULE_NOMATCH)
                {
                    /* Handle the case when an error is propigated
                     * via nestedRetVal.
                     */
                    retVal = RULE_NOMATCH;
                }
                else if (nestedRetVal > RULE_MATCH)
                {
                    /* Propigate "no alert" values back up the chain */
                    retVal = nestedRetVal;
                }
                break;
            }
            else
            {
                /* Cool, nobody after us, we're done with a match */
                if (cursor)
                    *cursor = thisCursor;
                break;
            }
        }
        else
        {
            /* No match, get outta dodge */
            break;
        }
    } while (continueLoop);
    /* Keep looping until we break or serialized content checks returns no match. */

    /* Reset the flags for this content in case we added the
     * relative flag above.
     */
    if (thisContentInfo)
    {
        thisContentInfo->flags = origFlags;
        thisContentInfo->offset = origOffset;
        thisContentInfo->depth = origDepth;
    }
    if (thisPCREInfo)
    {
        thisPCREInfo->flags = origFlags;
        thisPCREInfo->offset = origOffset;
    }

    return retVal;
}

int ruleMatch(void *p, Rule* rule)
{
    int retVal;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    /* Uh, stupid rule... Just checking IPs & Ports apparently */
    if (rule->numOptions == 0)
        return RULE_MATCH;

    retVal = ruleMatchInternal(sp, rule, 0, NULL);

    /* Special case for rules that just set a flowbit, but
     * don't alert.
     */

    if (rule->noAlert)
        return RULE_NOMATCH;

    return retVal;
}
