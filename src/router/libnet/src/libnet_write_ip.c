/*
 *  $Id: libnet_write_ip.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_write_ip.c - writes a prebuilt IP packet to the network
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
libnet_write_ip(int sock, u_char *buf, int len)
{
    int c;
    struct sockaddr_in sin;
    struct libnet_ip_hdr  *ip_hdr;

    ip_hdr = (struct libnet_ip_hdr *)buf;

#if (LIBNET_BSD_BYTE_SWAP)
    /*
     *  For link access, we don't need to worry about the inconsistencies of
     *  certain BSD kernels.  However, raw socket nuances abound.  Certain
     *  BSD implmentations require the ip_len and ip_off fields to be in host
     *  byte order.  It's MUCH easier to change it here than inside the bpf
     *  writing routine.
     */
    ip_hdr->ip_len = FIX(ip_hdr->ip_len);
    ip_hdr->ip_off = FIX(ip_hdr->ip_off);
#endif

    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family  = AF_INET;
    sin.sin_addr.s_addr = ip_hdr->ip_dst.s_addr;

    c = sendto(sock, buf, len, 0, (struct sockaddr *)&sin,
        sizeof(struct sockaddr));

#if (LIBNET_BSD_BYTE_SWAP)
    ip_hdr->ip_len = UNFIX(ip_hdr->ip_len);
    ip_hdr->ip_off = UNFIX(ip_hdr->ip_off);
#endif
             
    if (c != len)
    {
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING, "write_ip: %d bytes written (%s)\n",
                c, strerror(errno));
#endif
    }
    return (c);
}

/* EOF */
