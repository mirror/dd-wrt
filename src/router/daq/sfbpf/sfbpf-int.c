/*
 * Copyright (c) 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998
 *	The Regents of the University of California.  All rights reserved.
 *
 * Some portions Copyright (C) 2010-2013 Sourcefire, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * This file mostly consists of code extraced from libpcap as required by the
 * BPF library.  It originates from multiple files in the original libpcap
 * distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#ifndef WIN32
#include <netdb.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfbpf-int.h"
#include "namedb.h"
#include "gencode.h"

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const u_char charmap[] = {
    (u_char) '\000', (u_char) '\001', (u_char) '\002', (u_char) '\003',
    (u_char) '\004', (u_char) '\005', (u_char) '\006', (u_char) '\007',
    (u_char) '\010', (u_char) '\011', (u_char) '\012', (u_char) '\013',
    (u_char) '\014', (u_char) '\015', (u_char) '\016', (u_char) '\017',
    (u_char) '\020', (u_char) '\021', (u_char) '\022', (u_char) '\023',
    (u_char) '\024', (u_char) '\025', (u_char) '\026', (u_char) '\027',
    (u_char) '\030', (u_char) '\031', (u_char) '\032', (u_char) '\033',
    (u_char) '\034', (u_char) '\035', (u_char) '\036', (u_char) '\037',
    (u_char) '\040', (u_char) '\041', (u_char) '\042', (u_char) '\043',
    (u_char) '\044', (u_char) '\045', (u_char) '\046', (u_char) '\047',
    (u_char) '\050', (u_char) '\051', (u_char) '\052', (u_char) '\053',
    (u_char) '\054', (u_char) '\055', (u_char) '\056', (u_char) '\057',
    (u_char) '\060', (u_char) '\061', (u_char) '\062', (u_char) '\063',
    (u_char) '\064', (u_char) '\065', (u_char) '\066', (u_char) '\067',
    (u_char) '\070', (u_char) '\071', (u_char) '\072', (u_char) '\073',
    (u_char) '\074', (u_char) '\075', (u_char) '\076', (u_char) '\077',
    (u_char) '\100', (u_char) '\141', (u_char) '\142', (u_char) '\143',
    (u_char) '\144', (u_char) '\145', (u_char) '\146', (u_char) '\147',
    (u_char) '\150', (u_char) '\151', (u_char) '\152', (u_char) '\153',
    (u_char) '\154', (u_char) '\155', (u_char) '\156', (u_char) '\157',
    (u_char) '\160', (u_char) '\161', (u_char) '\162', (u_char) '\163',
    (u_char) '\164', (u_char) '\165', (u_char) '\166', (u_char) '\167',
    (u_char) '\170', (u_char) '\171', (u_char) '\172', (u_char) '\133',
    (u_char) '\134', (u_char) '\135', (u_char) '\136', (u_char) '\137',
    (u_char) '\140', (u_char) '\141', (u_char) '\142', (u_char) '\143',
    (u_char) '\144', (u_char) '\145', (u_char) '\146', (u_char) '\147',
    (u_char) '\150', (u_char) '\151', (u_char) '\152', (u_char) '\153',
    (u_char) '\154', (u_char) '\155', (u_char) '\156', (u_char) '\157',
    (u_char) '\160', (u_char) '\161', (u_char) '\162', (u_char) '\163',
    (u_char) '\164', (u_char) '\165', (u_char) '\166', (u_char) '\167',
    (u_char) '\170', (u_char) '\171', (u_char) '\172', (u_char) '\173',
    (u_char) '\174', (u_char) '\175', (u_char) '\176', (u_char) '\177',
    (u_char) '\200', (u_char) '\201', (u_char) '\202', (u_char) '\203',
    (u_char) '\204', (u_char) '\205', (u_char) '\206', (u_char) '\207',
    (u_char) '\210', (u_char) '\211', (u_char) '\212', (u_char) '\213',
    (u_char) '\214', (u_char) '\215', (u_char) '\216', (u_char) '\217',
    (u_char) '\220', (u_char) '\221', (u_char) '\222', (u_char) '\223',
    (u_char) '\224', (u_char) '\225', (u_char) '\226', (u_char) '\227',
    (u_char) '\230', (u_char) '\231', (u_char) '\232', (u_char) '\233',
    (u_char) '\234', (u_char) '\235', (u_char) '\236', (u_char) '\237',
    (u_char) '\240', (u_char) '\241', (u_char) '\242', (u_char) '\243',
    (u_char) '\244', (u_char) '\245', (u_char) '\246', (u_char) '\247',
    (u_char) '\250', (u_char) '\251', (u_char) '\252', (u_char) '\253',
    (u_char) '\254', (u_char) '\255', (u_char) '\256', (u_char) '\257',
    (u_char) '\260', (u_char) '\261', (u_char) '\262', (u_char) '\263',
    (u_char) '\264', (u_char) '\265', (u_char) '\266', (u_char) '\267',
    (u_char) '\270', (u_char) '\271', (u_char) '\272', (u_char) '\273',
    (u_char) '\274', (u_char) '\275', (u_char) '\276', (u_char) '\277',
    (u_char) '\300', (u_char) '\341', (u_char) '\342', (u_char) '\343',
    (u_char) '\344', (u_char) '\345', (u_char) '\346', (u_char) '\347',
    (u_char) '\350', (u_char) '\351', (u_char) '\352', (u_char) '\353',
    (u_char) '\354', (u_char) '\355', (u_char) '\356', (u_char) '\357',
    (u_char) '\360', (u_char) '\361', (u_char) '\362', (u_char) '\363',
    (u_char) '\364', (u_char) '\365', (u_char) '\366', (u_char) '\367',
    (u_char) '\370', (u_char) '\371', (u_char) '\372', (u_char) '\333',
    (u_char) '\334', (u_char) '\335', (u_char) '\336', (u_char) '\337',
    (u_char) '\340', (u_char) '\341', (u_char) '\342', (u_char) '\343',
    (u_char) '\344', (u_char) '\345', (u_char) '\346', (u_char) '\347',
    (u_char) '\350', (u_char) '\351', (u_char) '\352', (u_char) '\353',
    (u_char) '\354', (u_char) '\355', (u_char) '\356', (u_char) '\357',
    (u_char) '\360', (u_char) '\361', (u_char) '\362', (u_char) '\363',
    (u_char) '\364', (u_char) '\365', (u_char) '\366', (u_char) '\367',
    (u_char) '\370', (u_char) '\371', (u_char) '\372', (u_char) '\373',
    (u_char) '\374', (u_char) '\375', (u_char) '\376', (u_char) '\377',
};

int sfbpf_strcasecmp(const char *s1, const char *s2)
{
    register const u_char *cm = charmap, *us1 = (const u_char *) s1, *us2 = (const u_char *) s2;

    while (cm[*us1] == cm[*us2++])
        if (*us1++ == '\0')
            return (0);
    return (cm[*us1] - cm[*--us2]);
}

/* Hex digit to integer. */
static inline int xdtoi(c)
     register int c;
{
    if (isdigit(c))
        return c - '0';
    else if (islower(c))
        return c - 'a' + 10;
    else
        return c - 'A' + 10;
}

static inline int skip_space(f)
     FILE *f;
{
    int c;

    do
    {
        c = getc(f);
    } while (isspace(c) && c != '\n');

    return c;
}

static inline int skip_line(f)
     FILE *f;
{
    int c;

    do
        c = getc(f);
    while (c != '\n' && c != EOF);

    return c;
}

struct pcap_etherent *pcap_next_etherent(FILE * fp)
{
    register int c, d, i;
    char *bp;
    static struct pcap_etherent e;

    memset((char *) &e, 0, sizeof(e));
    do
    {
        /* Find addr */
        c = skip_space(fp);
        if (c == '\n')
            continue;

        /* If this is a comment, or first thing on line
           cannot be etehrnet address, skip the line. */
        if (!isxdigit(c))
        {
            c = skip_line(fp);
            continue;
        }

        /* must be the start of an address */
        for (i = 0; i < 6; i += 1)
        {
            d = xdtoi(c);
            c = getc(fp);
            if (isxdigit(c))
            {
                d <<= 4;
                d |= xdtoi(c);
                c = getc(fp);
            }
            e.addr[i] = d;
            if (c != ':')
                break;
            c = getc(fp);
        }
        if (c == EOF)
            break;

        /* Must be whitespace */
        if (!isspace(c))
        {
            c = skip_line(fp);
            continue;
        }
        c = skip_space(fp);

        /* hit end of line... */
        if (c == '\n')
            continue;

        if (c == '#')
        {
            c = skip_line(fp);
            continue;
        }

        /* pick up name */
        bp = e.name;
        /* Use 'd' to prevent buffer overflow. */
        d = sizeof(e.name) - 1;
        do
        {
            *bp++ = c;
            c = getc(fp);
        } while (!isspace(c) && c != EOF && --d > 0);
        *bp = '\0';

        /* Eat trailing junk */
        if (c != '\n')
            (void) skip_line(fp);

        return &e;

    } while (c != EOF);

    return (NULL);
}
