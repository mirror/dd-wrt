/*
** Copyright (C) 2003 Brian Caswell <bmc@snort.org>
** Copyright (C) 2003 Michael J. Pomraning <mjp@securepipe.com>
** Copyright (C) 2003-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*  I N C L U D E S
**********************************************************/

/*  D E F I N E S
************************************************************/
#ifndef __SNORT_PCRE_H__
#define __SNORT_PCRE_H__

#define SNORT_PCRE_RELATIVE     0x01  /* relative to the end of the last match */
#define SNORT_PCRE_INVERT       0x02  /* invert detect */
#define SNORT_PCRE_HTTP_URI     0x04  /* check URI buffers */
#define SNORT_PCRE_RAWBYTES     0x08  /* Don't use decoded buffer (if available) */
#define SNORT_PCRE_HTTP_BODY    0x10 /* Check HTTP body buffer */
#define SNORT_OVERRIDE_MATCH_LIMIT  0x20 /* Override default limits on match & match recursion */
#define SNORT_PCRE_HTTP_HEADER  0x40 /* Check HTTP header buffer */
#define SNORT_PCRE_HTTP_METHOD 0x80 /* Check HTTP method buffer */
#define SNORT_PCRE_HTTP_COOKIE 0x100 /* Check HTTP cookie buffer */
#define SNORT_PCRE_ANCHORED 0x200
#define SNORT_PCRE_HTTP_RAW_URI 0x400
#define SNORT_PCRE_HTTP_RAW_HEADER 0x800
#define SNORT_PCRE_HTTP_RAW_COOKIE 0x1000
#define SNORT_PCRE_HTTP_STAT_CODE 0x2000
#define SNORT_PCRE_HTTP_STAT_MSG 0x4000

#define SNORT_PCRE_URI_BUFS (SNORT_PCRE_HTTP_URI | SNORT_PCRE_HTTP_BODY | SNORT_PCRE_HTTP_HEADER | SNORT_PCRE_HTTP_METHOD | SNORT_PCRE_HTTP_COOKIE | \
                SNORT_PCRE_HTTP_RAW_URI | SNORT_PCRE_HTTP_RAW_HEADER | SNORT_PCRE_HTTP_RAW_COOKIE | SNORT_PCRE_HTTP_STAT_CODE | SNORT_PCRE_HTTP_STAT_MSG)

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

void PcreFree(void *d);
uint32_t PcreHash(void *d);
int PcreCompare(void *l, void *r);
void PcreDuplicatePcreData(void *src, PcreData *pcre_dup);
int PcreAdjustRelativeOffsets(PcreData *pcre, uint32_t search_offset);
void PcreCheckAnchored(PcreData *);

#endif /* __SNORT_PCRE_H__ */
