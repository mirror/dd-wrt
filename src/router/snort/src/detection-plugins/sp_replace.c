/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "sf_types.h"
#include "snort_bounds.h"
#include "snort_debug.h"
#include "decode.h"
#include "parser.h"
#include "sp_replace.h"
#include "snort.h"
#include "sfdaq.h"

#define MAX_PATTERN_SIZE 2048
extern int lastType;

static PatternMatchData* Replace_Parse(char*, OptTreeNode*);

void PayloadReplaceInit(struct _SnortConfig *sc, char *data, OptTreeNode * otn, int protocol)
{
    static int warned = 0;
    PatternMatchData *idx;

    if( !ScIpsInlineModeNewConf(sc) )
        return;

    if ( !DAQ_CanReplace() )
    {
        if ( !warned )
        {
            LogMessage("WARNING: payload replacements disabled because DAQ "
                " can't replace packets.\n");
            warned = 1;
        }
        return;
    }
    if ( lastType ==  PLUGIN_PATTERN_MATCH_URI )
    {
        FatalError("%s(%d) => \"replace\" option is not supported "
                "with uricontent, nor in conjunction with http_uri, "
                "http_header, http_method http_cookie,"
                "http_raw_uri, http_raw_header, or "
                "http_raw_cookie modifiers.\n",
                file_name, file_line);
    }
    idx = (PatternMatchData *) otn->ds_list[PLUGIN_PATTERN_MATCH];

    if(idx == NULL)
    {
        FatalError("%s(%d) => Please place \"content\" rules "
                   "before depth, nocase, replace or offset modifiers.\n",
                   file_name, file_line);
    }

    Replace_Parse(data, otn);

}

static PatternMatchData * Replace_Parse(char *rule, OptTreeNode * otn)
{
    char tmp_buf[MAX_PATTERN_SIZE];

    /* got enough ptrs for you? */
    char *start_ptr;
    char *end_ptr;
    char *idx;
    const char *dummy_idx;
    const char *dummy_end;
    char hex_buf[3];
    u_int dummy_size = 0;
    int size;
    int hexmode = 0;
    int hexsize = 0;
    int pending = 0;
    int cnt = 0;
    int literal = 0;
    PatternMatchData *ds_idx;
    int ret;

    if ( !rule )
    {
        FatalError("%s(%d) => missing argument to 'replace' option\n",
            file_name, file_line);
    }
    /* clear out the temp buffer */
    memset(tmp_buf, 0, MAX_PATTERN_SIZE);

    while(isspace((int)*rule))
        rule++;

    /* find the start of the data */
    start_ptr = strchr(rule, '"');

    if(start_ptr == NULL)
    {
        FatalError("%s(%d) => Replace data needs to be "
                   "enclosed in quotation marks (\")!\n",
                   file_name, file_line);
    }

    /* move the start up from the beggining quotes */
    start_ptr++;

    /* find the end of the data */
    end_ptr = strrchr(start_ptr, '"');

    if(end_ptr == NULL)
    {
        FatalError("%s(%d) => Replace data needs to be enclosed "
                   "in quotation marks (\")!\n", file_name, file_line);
    }

    /* set the end to be NULL */
    *end_ptr = '\0';

    /* how big is it?? */
    size = end_ptr - start_ptr;

    /* uh, this shouldn't happen */
    if(size <= 0)
    {
        FatalError("%s(%d) => Replace data has bad pattern length!\n",
                   file_name, file_line);
    }
    /* set all the pointers to the appropriate places... */
    idx = start_ptr;

    /* set the indexes into the temp buffer */
    dummy_idx = tmp_buf;
    dummy_end = (dummy_idx + size);

    /* why is this buffer so small? */
    memset(hex_buf, '0', 2);
    hex_buf[2] = '\0';


    /* BEGIN BAD JUJU..... */
    while(idx < end_ptr)
    {
        if (dummy_size >= MAX_PATTERN_SIZE-1)
        {
            /* Have more data to parse and pattern is about to go beyond end of buffer */
            FatalError("%s(%d) => Replace buffer overflow, make a "
                    "smaller pattern please! (Max size = %d)\n",
                    file_name, file_line, MAX_PATTERN_SIZE-1);
        }

        DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "processing char: %c\n", *idx););

        switch(*idx)
        {
            case '|':

                DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Got bar... "););

                if(!literal)
                {

                    DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                        "not in literal mode... "););

                    if(!hexmode)
                    {
                        DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                        "Entering hexmode\n"););

                        hexmode = 1;
                    }
                    else
                    {

                        DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                        "Exiting hexmode\n"););

                        hexmode = 0;
                        pending = 0;
                    }

                    if(hexmode)
                        hexsize = 0;
                }
                else
                {

                    DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                        "literal set, Clearing\n"););

                    literal = 0;
                    tmp_buf[dummy_size] = start_ptr[cnt];
                    dummy_size++;
                }

                break;

            case '\\':

                DEBUG_WRAP(DebugMessage(DEBUG_PARSER, "Got literal char... "););

                if(!literal)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                        "Setting literal\n"););

                    literal = 1;
                }
                else
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                        "Clearing literal\n"););

                    tmp_buf[dummy_size] = start_ptr[cnt];
                    literal = 0;
                    dummy_size++;
                }
                break;

            default:
                if(hexmode)
                {
                    if(isxdigit((int) *idx))
                    {
                        hexsize++;

                        if(!pending)
                        {
                            hex_buf[0] = *idx;
                            pending++;
                        }
                        else
                        {
                            hex_buf[1] = *idx;
                            pending--;

                            if(dummy_idx < dummy_end)
                            {
                                tmp_buf[dummy_size] = (u_char)
                                    strtol(hex_buf, (char **) NULL, 16)&0xFF;

                                dummy_size++;
                                memset(hex_buf, '0', 2);
                                hex_buf[2] = '\0';
                            }
                            else
                            {
                                FatalError("%s(%d) => Replace buffer overflow, make a "
                                           "smaller pattern please! (Max size = %d)\n",
                                           file_name, file_line, MAX_PATTERN_SIZE-1);
                            }
                        }
                    }
                    else
                    {
                        if(*idx != ' ')
                        {
                            FatalError("%s(%d) => Replace found \"%c\"(0x%X) in "
                                       "your binary buffer.  Valid hex values only "
                                       "please! (0x0 -0xF) Position: %d\n",
                                       file_name, file_line, (char) *idx, (char) *idx, cnt);
                        }
                    }
                }
                else
                {
                    if(*idx >= 0x1F && *idx <= 0x7e)
                    {
                        if(dummy_idx < dummy_end)
                        {
                            tmp_buf[dummy_size] = start_ptr[cnt];
                            dummy_size++;
                        }
                        else
                        {
                            FatalError("%s(%d) => Replace buffer overflow!\n",
                                       file_name, file_line);
                        }

                        if(literal)
                        {
                            literal = 0;
                        }
                    }
                    else
                    {
                        if(literal)
                        {
                            tmp_buf[dummy_size] = start_ptr[cnt];
                            dummy_size++;

                            DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                            "Clearing literal\n"););

                            literal = 0;
                        }
                        else
                        {
                            FatalError("%s(%d) => Replace found character value out of "
                                       "range, only hex characters allowed in binary "
                                       "content buffers\n",
                                       file_name, file_line);
                        }
                    }
                }

                break;

        } /* end switch */

        dummy_idx++;
        idx++;
        cnt++;
    }
    /* ...END BAD JUJU */

    /* error pruning */

    if (literal) {
        FatalError("%s(%d) => Replace backslash escape is not completed\n",
            file_name, file_line);
    }
    if (hexmode) {
        FatalError("%s(%d) => Replace hexmode is not completed\n",
            file_name, file_line);
    }
    ds_idx = (PatternMatchData *) otn->ds_list[PLUGIN_PATTERN_MATCH];

    while(ds_idx->next != NULL)
        ds_idx = ds_idx->next;

    if((ds_idx->replace_buf = (char *) calloc(dummy_size+1,
                                                  sizeof(char))) == NULL)
    {
        FatalError("%s(%d) => Replace pattern_buf malloc failed!\n",
            file_name, file_line);
    }

    ret = SafeMemcpy(ds_idx->replace_buf, tmp_buf, dummy_size,
                     ds_idx->replace_buf, (ds_idx->replace_buf+dummy_size));

    if (ret == SAFEMEM_ERROR)
    {
        FatalError("%s(%d) => Replace SafeMemcpy failed\n", file_name, file_line);
    }

    ds_idx->replace_size = dummy_size;

    DEBUG_WRAP(DebugMessage(DEBUG_PARSER,
                "ds_idx (%p) replace_size(%d) replace_buf(%s)\n", ds_idx,
                ds_idx->replace_size, ds_idx->replace_buf););

    return ds_idx;
}

typedef struct {
    const char* data;
    int size;
    int depth;
} Replacement;

#define MAX_REPLACEMENTS 32
static Replacement rpl[MAX_REPLACEMENTS];
static int num_rpl = 0;

void Replace_ResetQueue(void)
{
    num_rpl = 0;
}

void Replace_QueueChange(PatternMatchData* pmd)
{
    Replacement* r;

    if ( num_rpl == MAX_REPLACEMENTS )
        return;

    r = rpl + num_rpl++;

    r->data = pmd->replace_buf;
    r->size = pmd->replace_size;
    r->depth = pmd->replace_depth;
}

static inline void Replace_ApplyChange(Packet *p, Replacement* r)
{
    int err;
    int rsize;

    if( (p->data + r->depth + r->size) >= (p->data + p->dsize))
        rsize = (p->dsize - r->depth);
    else
        rsize = r->size;

    err = SafeMemcpy(
        (void *)(p->data + r->depth), r->data,
        rsize, p->data, (p->data + p->dsize) );

    if ( err == SAFEMEM_ERROR )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                "Replace_Apply() => SafeMemcpy() failed\n"););
        return;
    }
}

void Replace_ModifyPacket(Packet *p)
{
    int n;

    if ( num_rpl == 0 )
        return;

    for ( n = 0; n < num_rpl; n++ )
    {
        Replace_ApplyChange(p, rpl+n);
    }
    p->packet_flags |= PKT_MODIFIED;
    num_rpl = 0;
}

