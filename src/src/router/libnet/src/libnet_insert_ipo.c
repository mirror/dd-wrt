/*
 *  $Id: libnet_insert_ipo.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_insert_ipo.c - inserts IP options into a prebuilt IP packet
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

int
libnet_insert_ipo(struct ipoption *opt, u_char opt_len, u_char *buf)
{
    struct libnet_ip_hdr *ip_hdr;
    u_char *p;
    u_short s, j;
    u_char i;

    if (!buf)
    {
        return (-1);
    }

    ip_hdr = (struct libnet_ip_hdr *)(buf);
    s = UNFIX(ip_hdr->ip_len);

    if ((s + opt_len) > IP_MAXPACKET)
    {
        /*
         *  Nope.  Too big.
         */
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING,
            "insert_ipo: options list would result in too large of a packet\n");
#endif
        return (-1);
    }

    /*
     *  Do we have more then just an IP header?
     */
    if (s > LIBNET_IP_H)
    {
        /*
         *  Move over whatever's in the way.
         */
        memmove((u_char *)ip_hdr + LIBNET_IP_H + opt_len, (u_char *)ip_hdr
                + LIBNET_IP_H, opt_len);
    }

    /*
     *  Copy over option list.  We rely on the programmer having been
     *  smart enough to allocate enough heap memory here.  Uh oh.
     */
    p = (u_char *)ip_hdr + LIBNET_IP_H;
    memcpy(p, opt->ipopt_list, opt_len);

    /*
     *  Count up number of 32-bit words in options list, padding if
     *  neccessary.
     */
    for (i = 0, j = 0; i < opt_len; i++) (i % 4) ? j : j++;

    ip_hdr->ip_hl   += j;
    ip_hdr->ip_len  = FIX(opt_len + s);

    return (1);
}

/* EOF */
