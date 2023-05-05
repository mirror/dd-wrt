/*
 *  sf_snort_plugin_pcre.c
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
 * PCRE operations for dynamic rule engine
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre.h"
#include "sf_types.h"
#include "snort_debug.h"
#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_engine.h"
#include "sf_snort_detection_engine.h"

/* Need access to the snort-isms that were passed to the engine */
extern int checkCursorSimple(const uint8_t *cursor, int flags, const uint8_t *start, const uint8_t *end, int offset);
extern int checkCursorInternal(void *p, int flags, int offset, const uint8_t *cursor);
static int pcreMatchInternal(void *, PCREInfo*, const uint8_t **);

int PCRESetup(struct _SnortConfig *sc, Rule *rule, PCREInfo *pcreInfo)
{
    const char *error;
    int erroffset;

    pcreInfo->compiled_expr = (void *)_ded.pcreCompile(pcreInfo->expr,
                                                    pcreInfo->compile_flags,
                                                    &error,
                                                    &erroffset,
                                                    NULL);

    if (!pcreInfo->compiled_expr)
    {
        /* error doing compilation. */
        _ded.errMsg("Failed to compile PCRE in dynamic rule [%d:%d]\n",
            rule->info.genID, rule->info.sigID);
        return -1;
    }
    else
    {
        pcreInfo->compiled_extra = (void *)_ded.pcreStudy(sc, pcreInfo->compiled_expr, pcreInfo->compile_flags, &error);
    }

    if (error)
    {
        /* error doing study. */
        _ded.errMsg("Failed to study PCRE in dynamic rule [%d:%d]\n",
            rule->info.genID, rule->info.sigID);
        return -1;
    }

    _ded.pcreCapture(sc, pcreInfo->compiled_expr, pcreInfo->compiled_extra);


    return 0;
}

/**
 *  * Wrapper for pcre_exec to expose ovector.
 *   */
ENGINE_LINKAGE int pcreExecWrapper(const PCREInfo *pcre_info, const char *buf, int len, int start_offset,
                                    int options, int *ovector, int ovecsize)
{
    int result;
    int matched;

    if(pcre_info == NULL
        || buf == NULL
        || len <= 0
        || start_offset < 0
        || start_offset >= len
        || ovector == NULL)
    {
        return 0;
    }

    result = _ded.pcreExec(pcre_info->compiled_expr,    /* result of pcre_compile() */
                     pcre_info->compiled_extra,   /* result of pcre_study()   */
                     buf,                         /* the subject string */
                     len,                         /* the length of the subject string */
                     start_offset,                /* start at offset 0 in the subject */
                     options,                     /* options(handled at compile time */
                     ovector,                     /* vector for substring information */
                     ovecsize);    /* number of elements in the vector */

    if(result >= 0)
    {
        matched = 1;
    }
    else if(result == PCRE_ERROR_NOMATCH)
    {
        matched = 0;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "pcre_exec error : %d \n", result););
        return 0;
    }

    return matched;
}

/*
 * we need to specify the vector length for our pcre_exec call.  we only care
 * about the first vector, which if the match is successful will include the
 * offset to the end of the full pattern match.  If we decide to store other
 * matches, make *SURE* that this is a multiple of 3 as pcre requires it.
 */
#define SNORT_PCRE_OVECTOR_SIZE 3

/**
 * Perform a search of the PCRE data.
 *
 * @param pcre_data structure that options and patterns are passed in
 * @param buf buffer to search
 * @param len size of buffer
 * @param start_offset initial offset into the buffer
 * @param found_offset pointer to an integer so that we know where the search ended
 *
 * *found_offset will be set to -1 when the find is unsucessful OR the routine is inverted
 *
 * @return 1 when we find the string, 0 when we don't (unless we've been passed a flag to invert)
 */
static int pcre_test(const PCREInfo *pcre_info,
                       const char *buf,
                       int len,
                       int start_offset,
                       int *found_offset)
{
    int matched;
    int result;

    int *ovector;
    int ovector_size;

    _ded.pcreOvectorInfo(&ovector, &ovector_size);

    if(pcre_info == NULL
       || buf == NULL
       || len <= 0
       || start_offset < 0
       || start_offset >= len
       || found_offset == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Returning 0 because we didn't have the required parameters!\n"););
        return 0;
    }

    *found_offset = -1;

    result = _ded.pcreExec(pcre_info->compiled_expr,/* result of pcre_compile() */
                       pcre_info->compiled_extra,   /* result of pcre_study()   */
                       buf,                         /* the subject string */
                       len,                         /* the length of the subject string */
                       start_offset,                /* start at offset 0 in the subject */
                       0,                           /* options(handled at compile time */
                       ovector,                     /* vector for substring information */
                       ovector_size);               /* number of elements in the vector */

    if(result >= 0)
    {
        matched = 1;
    }
    else if(result == PCRE_ERROR_NOMATCH)
    {
        matched = 0;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "pcre_exec error : %d \n", result););
        return 0;
    }

    if (found_offset)
    {
        *found_offset = ovector[1];
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Setting buffer and found_offset: %p %d\n",
                                buf, found_offset););
    }

    return matched;
}

ENGINE_LINKAGE int pcreMatch(void *p, PCREInfo* pcre_info, const uint8_t **cursor)
{
    if (pcre_info->flags & NOT_FLAG)
        return invertMatchResult(pcreMatchInternal(p, pcre_info, cursor));
    return pcreMatchInternal(p, pcre_info, cursor);
}

static int pcreMatchInternal(void *p, PCREInfo* pcre_info, const uint8_t **cursor)
{
    const uint8_t *buffer_start;
    int buffer_len;
    int pcre_offset;
    int pcre_found;
    int relative = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;
    unsigned hb_type, len;

    /* Input validation */
    if (!p || !pcre_info)
    {
        return RULE_NOMATCH;
    }

    /* Input validation for cursor & relative flag */
    if (pcre_info->flags & CONTENT_RELATIVE)
    {
        if (!cursor || !*cursor)
        {
            return RULE_NOMATCH;
        }
        relative = 1;
    }

    hb_type = HTTP_CONTENT(pcre_info->flags);
    buffer_start = _ded.getHttpBuffer(hb_type, &len);

    if ( buffer_start )
    {
        buffer_len = len;
        if (relative)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,"PCRE unsupported configuration : both relative & uri options specified\n"););
            return RULE_NOMATCH;
        }
    }
    else
    {

        if ((pcre_info->flags & CONTENT_BUF_NORMALIZED) && _ded.Is_DetectFlag(SF_FLAG_DETECT_ALL))
        {
            if(_ded.Is_DetectFlag(SF_FLAG_ALT_DETECT))
            {
                buffer_start = _ded.altDetect->data;
                buffer_len = _ded.altDetect->len;
            }
            else
            {
                buffer_start = _ded.altBuffer->data;
                buffer_len = _ded.altBuffer->len;
            }
        }
        else
        {
            buffer_start = sp->payload;

            if(sp->normalized_payload_size)
                buffer_len = sp->normalized_payload_size;
            else
                buffer_len = sp->payload_size;
        }

        if (!buffer_start || !buffer_len)
            return RULE_NOMATCH;
        if (relative)
        {
            if ( checkCursorSimple(*cursor, pcre_info->flags, buffer_start, buffer_start+buffer_len,
                pcre_info->offset) == CURSOR_OUT_OF_BOUNDS )
                return RULE_NOMATCH;

            buffer_len = (buffer_start + buffer_len) - *cursor;
            buffer_start = *cursor;
        }
    }


    pcre_found = pcre_test(pcre_info, (const char *)buffer_start, buffer_len, pcre_info->offset, &pcre_offset);

    if (pcre_found)
    {
        if (cursor)
        {
            *cursor = buffer_start + pcre_offset;
        }
    }

    if (pcre_found)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

