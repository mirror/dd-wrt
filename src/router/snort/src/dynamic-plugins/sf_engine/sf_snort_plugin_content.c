/*
 *  sf_snort_plugin_content.c
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
 * Author: Marc Norton
 *         Steve Sturges
 *         Andy Mullican
 *
 * Date: 5/2005
 *
 *
 * Content operations for dynamic rule engine
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ctype.h"

#include "sf_sechash.h"
#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_engine.h"
#include "sfghash.h"
#include "sf_snort_detection_engine.h"

#include "bmh.h"

extern int checkCursorInternal(void *p, int flags, int offset, const uint8_t *cursor);
static int contentMatchInternal(void *, ContentInfo*, const uint8_t **);
static int contentMatchCommon(ContentInfo*, const uint8_t *, int, const uint8_t **);
static int protectedContentMatchInternal(void *, ProtectedContentInfo*, const uint8_t **);
static int protectedContentMatchCommon(ProtectedContentInfo*, const uint8_t *, int, const uint8_t **);

static const uint8_t *_buffer_end = NULL;
static const uint8_t *_alt_buffer_end = NULL;
static const uint8_t *_uri_buffer_end = NULL;
static const uint8_t *_alt_detect_end = NULL;

void ContentSetup(void)
{
    _buffer_end = NULL;
    _alt_buffer_end = NULL;
    _uri_buffer_end = NULL;
    _alt_detect_end = NULL;
}

/*
 *  Initialize Boyer-Moore-Horspool data for single pattern comparisons
 *
 *  returns: 0  -> success
 *           !0 -> error,failed
 */
int BoyerContentSetup(Rule *rule, ContentInfo *content)
{
    void *memoryLocation;

    /* XXX: need to precompile the B-M stuff */

    if( !content->patternByteForm || !content->patternByteFormLength )
        return 0;

    content->boyer_ptr = hbm_prep(content->patternByteForm,
        content->patternByteFormLength,
        content->flags & CONTENT_NOCASE);

    if( !content->boyer_ptr )
    {
        /* error doing compilation. */
        _ded.errMsg("Failed to setup pattern match for dynamic rule [%d:%d]\n",
            rule->info.genID, rule->info.sigID);
        return -1;
    }

    /* Initialize byte_extract pointers */
    if (content->offset_refId)
    {
        if (!rule->ruleData)
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       content->offset_refId, rule->info.genID, rule->info.sigID);
        }

        memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, content->offset_refId);
        if (memoryLocation)
        {
            content->offset_location = memoryLocation;
        }
        else
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       content->offset_refId, rule->info.genID, rule->info.sigID);
        }
    }

    if (content->depth_refId)
    {
        if (!rule->ruleData)
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       content->depth_refId, rule->info.genID, rule->info.sigID);
        }

        memoryLocation = sfghash_find((SFGHASH*)rule->ruleData, content->depth_refId);
        if (memoryLocation)
        {
            content->depth_location = memoryLocation;
        }
        else
        {
            DynamicEngineFatalMessage("ByteExtract variable '%s' in rule [%d:%d] is used before it is defined.\n",
                                       content->depth_refId, rule->info.genID, rule->info.sigID);
        }
    }

    return 0;
}

ENGINE_LINKAGE int contentMatch(void *p, ContentInfo* content, const uint8_t **cursor)
{
    int ret = contentMatchInternal(p, content, cursor);
    if (ret < 0)
        return CONTENT_NOMATCH;
    if (content->flags & NOT_FLAG)
        return invertMatchResult(ret);
    return ret;
}

ENGINE_LINKAGE int protectedContentMatch(void *p, ProtectedContentInfo* content, const uint8_t **cursor)
{
    int ret = protectedContentMatchInternal(p, content, cursor);
    if (ret < 0)
        return CONTENT_NOMATCH;
    if (content->flags & NOT_FLAG)
        return invertMatchResult(ret);
    return ret;
}
/*
 *  Content Option processing function
 *
 *       p: packet data structure, same as the one found in snort.
 * content: data defined in the detection plugin for this rule content option
 *  cursor: updated to point the 1st byte after the match
 *
 * Returns:
 *    > 0 : match found
 *    = 0 : no match found
 *    < 0 : error
 *
 * Predefined constants:
 *    (see sf_snort_plugin_api.h for more values)
 *    CONTENT_MATCH   -  if content specifier is found within buffer
 *    CONTENT_NOMATCH -  if content specifier is not found within buffer
 *
 * Notes:
 *   For multiple URI buffers, we scan each buffer, if any one of them
 *   contains the content we return a match. This is essentially an OR
 *   operation.
 *
 *   Currently support:
 *    options:
 *      nocase
 *      offset
 *      depth
 *    buffers:
 *      normalized(alt-decode)
 *      raw
 *      uri
 *      post
 *
 */
static int contentMatchInternal(void *p, ContentInfo* content, const uint8_t **cursor)
{
    const uint8_t *start_ptr;
    const uint8_t *end_ptr;
    SFSnortPacket *sp = (SFSnortPacket *)p;
    unsigned hb_type;

    /* This content is only used for fast pattern matching and
     * should not be evaluated */
    if (content->flags & CONTENT_FAST_PATTERN_ONLY)
        return CONTENT_MATCH;

    /* Check for byte_extract variables and use them if present. */
    if (content->offset_location)
        content->offset = *content->offset_location;
    if (content->depth_location)
        content->depth = *content->depth_location;

    if ( (hb_type = HTTP_CONTENT(content->flags)) )
    {
        unsigned len;
        const uint8_t* buf = _ded.getHttpBuffer(hb_type, &len);

        if ( buf )
        {
            if (contentMatchCommon(content, buf, len, cursor) == CONTENT_MATCH)
                return CONTENT_MATCH;
        }
        return CONTENT_NOMATCH;
    }

    if ((content->flags & CONTENT_BUF_NORMALIZED) && _ded.Is_DetectFlag(SF_FLAG_DETECT_ALL))
    {
        if (_ded.Is_DetectFlag(SF_FLAG_ALT_DETECT))
        {
            start_ptr = _ded.altDetect->data;

            if (_alt_detect_end)
                end_ptr = _alt_detect_end;
            else
                end_ptr = _ded.altDetect->data + _ded.altDetect->len;
        }
        else if (_ded.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        {
            start_ptr = _ded.altBuffer->data;

            if (_alt_buffer_end)
                end_ptr = _alt_buffer_end;
            else
                end_ptr = _ded.altBuffer->data + _ded.altBuffer->len;
        }
        else
        {
            return CONTENT_CURSOR_ERROR;
        }
    }
    else
    {
        start_ptr = sp->payload;

        if (sp->normalized_payload_size)
            end_ptr = sp->payload + sp->normalized_payload_size;
        else if (_buffer_end)
            end_ptr = _buffer_end;
        else
            end_ptr = sp->payload + sp->payload_size;
    }

    return contentMatchCommon(content, start_ptr, end_ptr - start_ptr, cursor);
}

static int protectedContentMatchInternal(void *p, ProtectedContentInfo* content, const uint8_t **cursor)
{
    const uint8_t *start_ptr;
    const uint8_t *end_ptr;
    SFSnortPacket *sp = (SFSnortPacket *)p;
    unsigned hb_type;

    /* Check for byte_extract variables and use them if present. */
    if (content->offset_location)
        content->offset = *content->offset_location;

    if ( (hb_type = HTTP_CONTENT(content->flags)) )
    {
        unsigned len;
        const uint8_t* buf = _ded.getHttpBuffer(hb_type, &len);

        if ( buf )
        {
            if (protectedContentMatchCommon(content, buf, len, cursor) == CONTENT_MATCH)
                return CONTENT_MATCH;
        }
        return CONTENT_NOMATCH;
    }

    if ((content->flags & CONTENT_BUF_NORMALIZED) && _ded.Is_DetectFlag(SF_FLAG_DETECT_ALL))
    {
        if (_ded.Is_DetectFlag(SF_FLAG_ALT_DETECT))
        {
            start_ptr = _ded.altDetect->data;

            if (_alt_detect_end)
                end_ptr = _alt_detect_end;
            else
                end_ptr = _ded.altDetect->data + _ded.altDetect->len;
        }
        else if (_ded.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        {
            start_ptr = _ded.altBuffer->data;

            if (_alt_buffer_end)
                end_ptr = _alt_buffer_end;
            else
                end_ptr = _ded.altBuffer->data + _ded.altBuffer->len;
        }
        else
        {
            return CONTENT_CURSOR_ERROR;
        }
    }
    else
    {
        start_ptr = sp->payload;

        if (sp->normalized_payload_size)
            end_ptr = sp->payload + sp->normalized_payload_size;
        else if (_buffer_end)
            end_ptr = _buffer_end;
        else
            end_ptr = sp->payload + sp->payload_size;
    }

    return protectedContentMatchCommon(content, start_ptr, end_ptr - start_ptr, cursor);
}

static int contentMatchCommon(ContentInfo* content,
        const uint8_t *start_ptr, int dlen, const uint8_t **cursor)
{
    const uint8_t *q;
    const uint8_t *base_ptr;
    const uint8_t *end_ptr = start_ptr + dlen;
    int depth;
    char relative = (content->flags & CONTENT_RELATIVE) ? 1 : 0;

    if (relative)
    {
        // Sanity check to make sure the cursor isn't NULL and is within the
        // buffer we're searching.  It could be at the very end of the buffer
        // due to a previous match, but may have a negative offset here.
        if ((cursor == NULL) || (*cursor == NULL)
                || (*cursor < start_ptr) || (*cursor > end_ptr))
            return CONTENT_CURSOR_ERROR;

        base_ptr = *cursor;
        depth = dlen - (*cursor - start_ptr);
    }
    else
    {
        base_ptr = start_ptr;
        depth = dlen;
    }

    // Adjust base_ptr and depth based on offset/depth parameters.
    if (relative && ((content->offset != 0) || (content->depth != 0)))
    {
        if (content->offset != 0)
        {
            base_ptr += content->offset;
            depth -= content->offset;
        }

        // If the offset is negative and puts us before start_ptr
        // set base_ptr to start_ptr and adjust depth based on depth.
        if (base_ptr < start_ptr)
        {
            int delta = (int)content->depth - (start_ptr - base_ptr);
            base_ptr = start_ptr;
            depth = ((content->depth == 0) || (delta > dlen)) ? dlen : delta;
        }
        else if ((content->depth != 0) && ((int)content->depth < depth))
        {
            depth = (int)content->depth;
        }
    }
    else if ((content->offset != 0) || (content->depth != 0))
    {
        if (content->offset != 0)
        {
            base_ptr += content->offset;
            depth -= content->offset;
        }

        if ((content->depth != 0) && ((int)content->depth < depth))
            depth = content->depth;
    }

    // If the pattern size is greater than the amount of data we have to
    // search, there's no way we can match, so return error, however, return
    // CONTENT_NOMATCH here for the case where the match is inverted and there
    // is at least some data.
    if ((int)content->patternByteFormLength > depth)
    {
        if ((content->flags & NOT_FLAG) && (depth > 0))
            return CONTENT_NOMATCH;  // This will get inverted on return
        return CONTENT_CURSOR_ERROR;
    }

    q = hbm_match((HBM_STRUCT*)content->boyer_ptr, base_ptr, depth);

    if (q)
    {
        if (content->flags & CONTENT_END_BUFFER)
        {
            if ( HTTP_CONTENT(content->flags) )
                _uri_buffer_end = q;
            else if ((content->flags & CONTENT_BUF_NORMALIZED)
                    && _ded.Is_DetectFlag(SF_FLAG_ALT_DETECT) )
                _alt_detect_end = q;
            else if ((content->flags & CONTENT_BUF_NORMALIZED)
                    && _ded.Is_DetectFlag(SF_FLAG_ALT_DECODE) )
                _alt_buffer_end = q;
            else
                _buffer_end = q;
        }

        if (cursor)
            *cursor = q + content->patternByteFormLength;
        return CONTENT_MATCH;
    }

    return CONTENT_NOMATCH;
}

static int protectedContentMatchCommon(ProtectedContentInfo* content,
        const uint8_t *start_ptr, int dlen, const uint8_t **cursor)
{
    const uint8_t *base_ptr;
    const uint8_t *end_ptr = start_ptr + dlen;
    int depth, ret;
    char relative = (content->flags & CONTENT_RELATIVE) ? 1 : 0;

    if (relative)
    {
        // Sanity check to make sure the cursor isn't NULL and is within the
        // buffer we're searching.  It could be at the very end of the buffer
        // due to a previous match, but may have a negative offset here.
        if ((cursor == NULL) || (*cursor == NULL)
                || (*cursor < start_ptr) || (*cursor > end_ptr))
            return CONTENT_CURSOR_ERROR;

        base_ptr = *cursor;
        depth = dlen - (*cursor - start_ptr);
    }
    else
    {
        base_ptr = start_ptr;
        depth = dlen;
    }

    // Adjust base_ptr and depth based on offset/depth parameters.
    if (relative && ((content->offset != 0)))
    {
        base_ptr += content->offset;
        depth -= content->offset;

        // If the offset is negative and puts us before start_ptr
        // set base_ptr to start_ptr and adjust depth based on depth.
        if (base_ptr < start_ptr)
            return CONTENT_NOMATCH;
        else if((int)content->protected_length < depth)
            depth = (int)content->protected_length;
    }
    else if (content->offset != 0)
    {
        base_ptr += content->offset;
        depth -= content->offset;
    }

    // If the pattern size is greater than the amount of data we have to
    // search, there's no way we can match, so return error, however, return
    // CONTENT_NOMATCH here for the case where the match is inverted and there
    // is at least some data.
    if ((int)content->protected_length > depth)
    {
        if ((content->flags & NOT_FLAG) && (depth > 0))
            return CONTENT_NOMATCH;  // This will get inverted on return
        return CONTENT_CURSOR_ERROR;
    }

    switch( content->hash_type )
    {
        case PROTECTED_CONTENT_HASH_MD5:
        {
            ret = memcmp(MD5DIGEST(base_ptr, content->protected_length, NULL),
                         content->patternByteForm, MD5_HASH_SIZE);
            break;
        }
        case PROTECTED_CONTENT_HASH_SHA256:
        {
            ret = memcmp(SHA256DIGEST(base_ptr, content->protected_length, NULL),
                         content->patternByteForm, SHA256_HASH_SIZE);
            break;
        }
        case PROTECTED_CONTENT_HASH_SHA512:
        {
            ret = memcmp(SHA512DIGEST(base_ptr, content->protected_length, NULL),
                         content->patternByteForm, SHA512_HASH_SIZE);
            break;
        }
        default:
            return( CONTENT_HASH_ERROR );
    }

    if( ret == 0 )
    {
        if (content->flags & CONTENT_END_BUFFER)
        {
            if ( HTTP_CONTENT(content->flags) )
                _uri_buffer_end = base_ptr;
            else if ((content->flags & CONTENT_BUF_NORMALIZED)
                    && _ded.Is_DetectFlag(SF_FLAG_ALT_DETECT) )
                _alt_detect_end = base_ptr;
            else if ((content->flags & CONTENT_BUF_NORMALIZED)
                    && _ded.Is_DetectFlag(SF_FLAG_ALT_DECODE) )
                _alt_buffer_end = base_ptr;
            else
                _buffer_end = base_ptr;
        }

        if (cursor)
            *cursor = base_ptr + content->protected_length;

        return CONTENT_MATCH;
    }

    return CONTENT_NOMATCH;
}
