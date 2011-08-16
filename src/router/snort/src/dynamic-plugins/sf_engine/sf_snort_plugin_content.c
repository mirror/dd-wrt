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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
#include "ctype.h"

#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_engine.h"

#include "bmh.h"

extern DynamicEngineData _ded; /* sf_detection_engine.c */
extern int checkCursorInternal(void *p, int flags, int offset, const u_int8_t *cursor);

static const u_int8_t *_buffer_end = NULL;
static const u_int8_t *_alt_buffer_end = NULL;
static const u_int8_t *_uri_buffer_end = NULL;

void ContentSetup(void)
{
    _buffer_end = NULL;
    _alt_buffer_end = NULL;
    _uri_buffer_end = NULL;
}

/*
 *  Initialize Boyer-Moore-Horspool data for single pattern comparisons
 *
 *  returns: 0  -> success
 *           !0 -> error,failed
 */
int BoyerContentSetup(Rule *rule, ContentInfo *content)
{
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

    return 0;
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
ENGINE_LINKAGE int contentMatch(void *p, ContentInfo* content, const u_int8_t **cursor)
{
    const u_int8_t * q = NULL;
    const u_int8_t * buffer_start;
    const u_int8_t * buffer_end = NULL;
    u_int  buffer_len;
    int    length;
    int    i;
    char   relative = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    /* This content is only used for fast pattern matching and
     * should not be evaluated */
    if (content->flags & CONTENT_FAST_PATTERN_ONLY)
        return CONTENT_MATCH;

    if (content->flags & CONTENT_RELATIVE)
    {
        if( !cursor || !(*cursor) )
        {
            return CONTENT_NOMATCH;
        } 
        relative = 1;
    }

    if (content->flags & URI_CONTENT_BUFS)
    {
        for (i=0; i<sp->num_uris; i++)
        {
            switch (i)
            {
                case HTTP_BUFFER_URI:
                    if (!(content->flags & CONTENT_BUF_URI))
                        continue; /* Go to next, not looking at URI buffer */
                    break;
                case HTTP_BUFFER_HEADER:
                    if (!(content->flags & CONTENT_BUF_HEADER))
                        continue; /* Go to next, not looking at HEADER buffer */
                    break;
                case HTTP_BUFFER_CLIENT_BODY:
                    if (!(content->flags & CONTENT_BUF_POST))
                        continue; /* Go to next, not looking at POST buffer */
                    break;
                case HTTP_BUFFER_METHOD:
                    if (!(content->flags & CONTENT_BUF_METHOD))
                        continue; /* Go to next, not looking at METHOD buffer */
                    break;
                case HTTP_BUFFER_COOKIE:
                    if (!(content->flags & CONTENT_BUF_COOKIE))
                        continue; /* Go to next, not looking at COOKIE buffer */
                    break;
                case HTTP_BUFFER_RAW_URI:
                    if (!(content->flags & CONTENT_BUF_RAW_URI))
                        continue; /* Go to next, not looking at RAW URI buffer */
                    break;
                case HTTP_BUFFER_RAW_HEADER:
                    if (!(content->flags & CONTENT_BUF_RAW_HEADER))
                        continue; /* Go to next, not looking at RAW HEADER buffer */
                    break;
                case HTTP_BUFFER_RAW_COOKIE:
                    if (!(content->flags & CONTENT_BUF_RAW_COOKIE))
                        continue; /* Go to next, not looking at RAW COOKIE buffer */
                    break;
                case HTTP_BUFFER_STAT_CODE:
                    if (!(content->flags & CONTENT_BUF_STAT_CODE))
                        continue; /* Go to next, not looking at STAT CODE buffer */
                    break;
                case HTTP_BUFFER_STAT_MSG:
                    if (!(content->flags & CONTENT_BUF_STAT_MSG))
                        continue; /* Go to next, not looking at STAT MSG buffer */
                    break;
                default:
                    /* Uh, what buffer is this? */
                    return CONTENT_NOMATCH;
            }

            if (!_ded.uriBuffers[i]->uriBuffer || (_ded.uriBuffers[i]->uriLength == 0))
                continue;

            if (relative)
            {
                if (checkCursorInternal(p, content->flags, content->offset, *cursor) <= 0)
                {
                    /* Okay, cursor is NOT within this buffer... */
                    continue;
                }
                buffer_start = *cursor + content->offset;
            }
            else
            {
                buffer_start = _ded.uriBuffers[i]->uriBuffer + content->offset;
            }

            buffer_end = _ded.uriBuffers[i]->uriBuffer + _ded.uriBuffers[i]->uriLength;

            length = buffer_len = buffer_end - buffer_start;

            if (length <= 0)
            {
                continue;
            }
            
            /* Don't bother looking deeper than depth */
            if ( content->depth != 0 && content->depth < buffer_len )
            {
                buffer_len = content->depth;
            }

            q = hbm_match((HBM_STRUCT*)content->boyer_ptr,buffer_start,buffer_len);

            if (q)
            {
                if (content->flags & CONTENT_END_BUFFER)
                {
                    _uri_buffer_end = q;
                }
                if (cursor)
                {
                    *cursor = q + content->patternByteFormLength;
                }
                return CONTENT_MATCH;
            }
        }

        return CONTENT_NOMATCH;
    }

    if (relative)
    {
        if (checkCursorInternal(p, content->flags, content->offset, *cursor) <= 0)
        {
            return CONTENT_NOMATCH;
        }

        if ((content->flags & CONTENT_BUF_NORMALIZED) && (sp->flags & FLAG_ALT_DECODE))
        {
            if (_alt_buffer_end)
            {
                buffer_end = _alt_buffer_end;
            }
            else
            {         
                buffer_end = _ded.altBuffer->data + _ded.altBuffer->len;
            }
        }
        else
        {
            if(sp->normalized_payload_size)
            {
                buffer_end = sp->payload + sp->normalized_payload_size;
            }
            else if (_buffer_end)
            {
                buffer_end = _buffer_end;
            }
            else
            {
                buffer_end = sp->payload + sp->payload_size;
            }
        }
        buffer_start = *cursor + content->offset;
    }
    else
    {
        if ((content->flags & CONTENT_BUF_NORMALIZED) && (sp->flags & FLAG_ALT_DECODE))
        {
            buffer_start = _ded.altBuffer->data + content->offset;
            if (_alt_buffer_end)
            {
                buffer_end = _alt_buffer_end;
            }
            else
            {
                buffer_end = _ded.altBuffer->data + _ded.altBuffer->len;
            }
        }
        else
        {
            buffer_start = sp->payload + content->offset;
            if(sp->normalized_payload_size)
            {
                buffer_end = sp->payload + sp->normalized_payload_size;
            }
            else if (_buffer_end)
            {
                buffer_end = _buffer_end;
            }
            else
            {
                buffer_end = sp->payload + sp->payload_size;
            }
        }
    }
    length = buffer_len = buffer_end - buffer_start;

    if (length <= 0)
    {
        return CONTENT_NOMATCH;
    }

    /* Don't bother looking deeper than depth */
    if ( content->depth != 0 && content->depth < buffer_len )
    {
        buffer_len = content->depth;
    }

    q = hbm_match((HBM_STRUCT*)content->boyer_ptr,buffer_start,buffer_len);

    if (q)
    {
        if (content->flags & CONTENT_END_BUFFER)
        {
            if ((content->flags & CONTENT_BUF_NORMALIZED) && (sp->flags & FLAG_ALT_DECODE))
            {
                _alt_buffer_end = q;
            }
            else
            {
                _buffer_end = q;
            }
        }
        if (cursor)
        {
            *cursor = q + content->patternByteFormLength;
        }
        return CONTENT_MATCH;
    }

    return CONTENT_NOMATCH;
}
