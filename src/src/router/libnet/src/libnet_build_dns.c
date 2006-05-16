/*
 *  $Id: libnet_build_dns.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_build_dns.c - DNS packet assembler
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
libnet_build_dns(u_short id, u_short flags, u_short num_q, u_short num_anws_rr,
        u_short num_auth_rr, u_short num_addi_rr, const u_char *payload,
        int payload_s, u_char *buf)
{
    struct libnet_dns_hdr dns_hdr;

    if (!buf)
    {
        return (-1);
    }

    dns_hdr.id          = htons(id);
    dns_hdr.flags       = htons(flags);
    dns_hdr.num_q       = htons(num_q);
    dns_hdr.num_answ_rr = htons(num_anws_rr);
    dns_hdr.num_auth_rr = htons(num_auth_rr);
    dns_hdr.num_addi_rr = htons(num_addi_rr);

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + DNS_H + payload to be greater
         *  than the allocated heap memory.
         */
        memcpy(buf + LIBNET_DNS_H, payload, payload_s);
    } 
    memcpy(buf, &dns_hdr, sizeof(dns_hdr));
    return (1);
}


/* EOF */
