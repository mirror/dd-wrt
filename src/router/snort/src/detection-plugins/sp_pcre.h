/*
** Copyright (C) 2003 Brian Caswell <bmc@snort.org>
** Copyright (C) 2003 Michael J. Pomraning <mjp@securepipe.com>
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2003-2013 Sourcefire, Inc.
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

/*  I N C L U D E S
**********************************************************/

/*  D E F I N E S
************************************************************/
#ifndef __SNORT_PCRE_H__
#define __SNORT_PCRE_H__

// low nibble must be same as HTTP_BUFFER_*
// see detection_util.h for enum
#define SNORT_PCRE_HTTP_URI         0x00001 // check URI buffers
#define SNORT_PCRE_HTTP_HEADER      0x00002 // Check HTTP header buffer
#define SNORT_PCRE_HTTP_BODY        0x00003 // Check HTTP body buffer
#define SNORT_PCRE_HTTP_METHOD      0x00004 // Check HTTP method buffer
#define SNORT_PCRE_HTTP_COOKIE      0x00005 // Check HTTP cookie buffer
#define SNORT_PCRE_HTTP_STAT_CODE   0x00006
#define SNORT_PCRE_HTTP_STAT_MSG    0x00007
#define SNORT_PCRE_HTTP_RAW_URI     0x00008
#define SNORT_PCRE_HTTP_RAW_HEADER  0x00009
#define SNORT_PCRE_HTTP_RAW_COOKIE  0x0000A
#define SNORT_PCRE_HTTP_BUFS        0x0000F
#define SNORT_PCRE_RELATIVE         0x00010 // relative to the end of the last match
#define SNORT_PCRE_INVERT           0x00020 // invert detect
#define SNORT_PCRE_RAWBYTES         0x00040 // Don't use decoded buffer (if available)
#define SNORT_PCRE_ANCHORED         0x00080
#define SNORT_OVERRIDE_MATCH_LIMIT  0x00100 // Override default limits on match & match recursion

void SetupPcre(void);

#include <pcre.h>
typedef struct _PcreData
{
    pcre *re;           /* compiled regex */
    pcre_extra *pe;     /* studied regex foo */
    int options;        /* sp_pcre specfic options (relative & inverse) */
    char *expression;
    uint32_t search_offset;
} PcreData;

void PcreCapture(struct _SnortConfig *sc, const void *code, const void *extra);
void PcreFree(void *d);
uint32_t PcreHash(void *d);
int PcreCompare(void *l, void *r);
void PcreDuplicatePcreData(void *src, PcreData *pcre_dup);
int PcreAdjustRelativeOffsets(PcreData *pcre, uint32_t search_offset);
void PcreCheckAnchored(PcreData *);

#endif /* __SNORT_PCRE_H__ */
