/*
 *  $Id: libnet_hex_dump.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_hex_dump.c - architecture independent packet dumper
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"

/*
 *  Much of this routine ala tcpdump.
 */

void
libnet_hex_dump(u_char *buf, int len, int swap, FILE *stream)
{
    int i, s_cnt;
    u_short *p;

    p        = (u_short *)buf;
    s_cnt    = len / sizeof(u_short);
    i        = 0;

    fprintf(stream, "\t");
    while (--s_cnt >= 0)
    {
        if ((!(i % 8)))
        {
            fprintf(stream, "\n%02x\t", (i * 2));
        }
        fprintf(stream, "%04x ", swap ? ntohs(*(p++)) : *(p++));
        i++;
    }

    /*
     *  Mop up an odd byte.
     */
    if (len & 1)
    {
        if ((!(i % 8)))
        {
            fprintf(stream, "\n%02x\t", (i * 2));
        }
        fprintf(stream, "%02x ", *(u_char *)p);
    }
    fprintf(stream, "\n");
}

/* EOF */
