/*
 * pop_util.c
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
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * This file contains POP helper functions.
 *
 * Entry point functions:
 *
 *    safe_strchr()
 *    safe_strstr()
 *    copy_to_space()
 *    safe_sscanf()
 *
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_debug.h"
#include "snort_bounds.h"

#include "snort_pop.h"
#include "pop_util.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_packet.h"
#include "Unified2_common.h"

extern POP *pop_ssn;

void POP_GetEOL(const uint8_t *ptr, const uint8_t *end,
                 const uint8_t **eol, const uint8_t **eolm)
{
    const uint8_t *tmp_eol;
    const uint8_t *tmp_eolm;

    /* XXX maybe should fatal error here since none of these
     * pointers should be NULL */
    if (ptr == NULL || end == NULL || eol == NULL || eolm == NULL)
        return;

    tmp_eol = (uint8_t *)memchr(ptr, '\n', end - ptr);
    if (tmp_eol == NULL)
    {
        tmp_eol = end;
        tmp_eolm = end;
    }
    else
    {
        /* end of line marker (eolm) should point to marker and
         * end of line (eol) should point to end of marker */
        if ((tmp_eol > ptr) && (*(tmp_eol - 1) == '\r'))
        {
            tmp_eolm = tmp_eol - 1;
        }
        else
        {
            tmp_eolm = tmp_eol;
        }

        /* move past newline */
        tmp_eol++;
    }

    *eol = tmp_eol;
    *eolm = tmp_eolm;
}


#ifdef DEBUG_MSGS
char pop_print_buffer[65537];

const char * POP_PrintBuffer(SFSnortPacket *p)
{
    const uint8_t *ptr = NULL;
    int len = 0;
    int iorig, inew;

    ptr = p->payload;
    len = p->payload_size;

    for (iorig = 0, inew = 0; iorig < len; iorig++, inew++)
    {
        if ((isascii((int)ptr[iorig]) && isprint((int)ptr[iorig])) || (ptr[iorig] == '\n'))
        {
            pop_print_buffer[inew] = ptr[iorig];
        }
        else if (ptr[iorig] == '\r' &&
                 ((iorig + 1) < len) && (ptr[iorig + 1] == '\n'))
        {
            iorig++;
            pop_print_buffer[inew] = '\n';
        }
        else if (isspace((int)ptr[iorig]))
        {
            pop_print_buffer[inew] = ' ';
        }
        else
        {
            pop_print_buffer[inew] = '.';
        }
    }

    pop_print_buffer[inew] = '\0';

    return &pop_print_buffer[0];
}
#endif

