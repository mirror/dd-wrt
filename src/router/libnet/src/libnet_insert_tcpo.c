/*
 *  $Id: libnet_insert_tcpo.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_insert_tcpo.c - inserts TCP options into a prebuilt IP packet
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
libnet_insert_tcpo(struct tcpoption *opt, u_char opt_len, u_char *buf)
{
    struct libnet_ip_hdr  *ip_hdr;
    struct libnet_tcp_hdr *tcp_hdr;
    u_char *p;
    u_short s, j;
    u_char i, opt_padded_len;
    u_char opt_padded_buf[MAX_IPOPTLEN];

    if (!buf)
    {
        return (-1);
    }

    ip_hdr = (struct libnet_ip_hdr *)(buf);

    if (ip_hdr->ip_p != IPPROTO_TCP)
    {
        /*
         *  Hey retard, where's the TCP header?
         */
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING,
            "insert_tcpo: tried to insert TCP options into a NON-TCP packet!\n");
#endif
        return (-1);
    }

    if (opt_len > MAX_IPOPTLEN)
    {
        /*
         *  Hey retard, TCP options can only be 20 octets long!
         */
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING,
            "insert_tcpo: options list too long!\n");
#endif
        return (-1);
    }

     s = ntohs(ip_hdr->ip_len);

    /*
     *  Figure out length of options with any necessary padding.
     */
    if (opt_len % 4)
    {
        opt_padded_len = opt_len + 4 - (opt_len % 4);
    }
    else
    {
        opt_padded_len = opt_len;
    }

    if ((s + opt_padded_len) > IP_MAXPACKET)
    {
        /*
         *  Nope.  Too big.
         */
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING,
            "insert_tcpo: options list would result in too large of a packet\n");
#endif
        return (-1);
    }


    /*
     *  Fixed pointer math -- now uses ip_hl instead of IP_H to calculate
     *  offset.
     */
    tcp_hdr = (struct libnet_tcp_hdr *)(buf + (ip_hdr->ip_hl << 2));

    /*
     *  Do we have more then just an TCP header?  (We are ignoring the IP
     *  header at this point).
     */
    if (s > (ip_hdr->ip_hl << 2) + LIBNET_TCP_H)
    {
        /*
         *  Move over whatever's in the way.
         */
        memmove((u_char *)tcp_hdr + LIBNET_TCP_H + opt_padded_len,
                (u_char *)tcp_hdr + LIBNET_TCP_H,
                (s - (ip_hdr->ip_hl << 2) - LIBNET_TCP_H));
    }

    /*
     *  Copy over option list.  We rely on the programmer having been
     *  smart enough to allocate enough heap memory here.  Uh oh.
     */
    p = (u_char *)tcp_hdr + LIBNET_TCP_H;
    memcpy(opt_padded_buf, (u_char *)opt->tcpopt_list, opt_len);
    memcpy((u_char *)p, opt_padded_buf, opt_padded_len);

    /*
     *  Count up number of 32-bit words in options list, padding if
     *  neccessary.
     */
    for (i = 0, j = 0; i < opt_padded_len; i++) (i % 4) ? j : j++;

    tcp_hdr->th_off += j;
    tcp_hdr->th_sum = 0;
    ip_hdr->ip_len  = htons(opt_padded_len + s); 

    return (1);
}

/* EOF */
