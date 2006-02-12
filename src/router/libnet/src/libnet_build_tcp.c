/*
 *  $Id: libnet_build_tcp.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_build_tcp.c - TCP packet assembler
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
libnet_build_tcp(u_short sp, u_short dp, u_long seq, u_long ack, u_char control,
            u_short win, u_short urg, const u_char *payload, int payload_s,
            u_char *buf)
{
    struct libnet_tcp_hdr tcp_hdr;

    if (!buf)
    {
        return (-1);
    }

    tcp_hdr.th_sport   = htons(sp);    /* source port */
    tcp_hdr.th_dport   = htons(dp);    /* destination port */
    tcp_hdr.th_seq     = htonl(seq);   /* sequence number */
    tcp_hdr.th_ack     = htonl(ack);   /* acknowledgement number */
    tcp_hdr.th_flags   = control;      /* control flags */
    tcp_hdr.th_x2      = 0;            /* UNUSED */
    tcp_hdr.th_off     = 5;            /* 20 byte header */
    tcp_hdr.th_win     = htons(win);   /* window size */
    tcp_hdr.th_sum     = 0;            /* checksum done in userland */ 
    tcp_hdr.th_urp     = urg;          /* urgent pointer */

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + TCP_H + payload to be greater
         *  than the allocated heap memory.
         */
        memcpy(buf + LIBNET_TCP_H, payload, payload_s);
    }
    memcpy((u_char *)buf, (u_char *)&tcp_hdr, sizeof(tcp_hdr));
    return (1);
}

/* EOF */
