/*
 *  $Id: libnet_build_udp.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_build_udp.c - UDP packet assembler
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
libnet_build_udp(u_short sp, u_short dp, const u_char *payload, int payload_s,
            u_char *buf)
{
    struct libnet_udp_hdr udp_hdr;

    if (!buf)
    {
        return (-1);
    }

    udp_hdr.uh_sport   = htons(sp);                /* source port */
    udp_hdr.uh_dport   = htons(dp);                /* destination port */
    udp_hdr.uh_ulen    = htons(LIBNET_UDP_H + payload_s);   /* total length */
    udp_hdr.uh_sum     = 0;                        /* checksum */

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + UDP_H + payload to be greater
         *  than the allocated heap memory.
         */
        memcpy(buf + LIBNET_UDP_H, payload, payload_s);
    } 
    memcpy(buf, &udp_hdr, sizeof(udp_hdr));
    return (1);
}


/* EOF */
