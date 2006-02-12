/*
 *  $Id: libnet_build_icmp.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_build_icmp.c - ICMP packet assembler
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
libnet_build_icmp_echo(u_char type, u_char code, u_short id, u_short seq,
            const u_char *payload, int payload_s, u_char *buf)
{
    struct libnet_icmp_hdr icmp_hdr;

    if (!buf)
    {
        return (-1);
    }

    icmp_hdr.icmp_type = type;         /* packet type */
    icmp_hdr.icmp_code = code;         /* packet code */
    icmp_hdr.icmp_id   = htons(id);    /* packet id */
    icmp_hdr.icmp_seq  = htons(seq);   /* packet seq */

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + LIBNET_ICMP_ECHO_H payload
         *  to be greater than the allocated heap memory.
         */
        memcpy(buf + LIBNET_ICMP_ECHO_H, payload, payload_s);
    }
    memcpy(buf, &icmp_hdr, LIBNET_ICMP_ECHO_H);
    return (1);
}


int
libnet_build_icmp_mask(u_char type, u_char code, u_short id, u_short seq,
            u_long mask, const u_char *payload, int payload_s, u_char *buf)
{
    struct libnet_icmp_hdr icmp_hdr;

    if (!buf)
    {
        return (-1);
    }

    icmp_hdr.icmp_type = type;         /* packet type */
    icmp_hdr.icmp_code = code;         /* packet code */
    icmp_hdr.icmp_id   = htons(id);    /* packet id */
    icmp_hdr.icmp_seq  = htons(seq);   /* packet seq */
    icmp_hdr.icmp_mask = htonl(mask);  /* address mask */

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + ICMP_H payload to begreater than
         *  the allocated heap memory.
         */
        memcpy(buf + LIBNET_ICMP_MASK_H, payload, payload_s);
    }
    memcpy(buf, &icmp_hdr, LIBNET_ICMP_MASK_H);
    return (1);
}


int
libnet_build_icmp_unreach(u_char type, u_char code, u_short orig_len,
        u_char orig_tos, u_short orig_id, u_short orig_frag, u_char orig_ttl,
        u_char orig_prot, u_long orig_src, u_long orig_dst, const u_char
        *orig_payload, int payload_s, u_char *buf)
{
    struct libnet_icmp_hdr icmp_hdr;

    if (!buf)
    {
        return (-1);
    }

    icmp_hdr.icmp_type = type;          /* packet type */
    icmp_hdr.icmp_code = code;          /* packet code */
    icmp_hdr.icmp_id   = 0;             /* must be 0 */
    icmp_hdr.icmp_seq  = 0;             /* must be 0 */

    /*
     *  How convenient!  We can use our build_ip function to tack on the
     *  original header!
     */
    libnet_build_ip(0, orig_tos, orig_id, orig_frag, orig_ttl, orig_prot,
            orig_src, orig_dst, orig_payload, payload_s, buf +
            LIBNET_ICMP_UNREACH_H);

    memcpy(buf, &icmp_hdr, LIBNET_ICMP_UNREACH_H);
    return (1);
}


int
libnet_build_icmp_timeexceed(u_char type, u_char code, u_short orig_len,
        u_char orig_tos, u_short orig_id, u_short orig_frag, u_char orig_ttl,
        u_char orig_prot, u_long orig_src, u_long orig_dst,
        const u_char *orig_payload, int payload_s, u_char *buf)
{
    struct libnet_icmp_hdr icmp_hdr;

    if (!buf)
    {
        return (-1);
    }

    icmp_hdr.icmp_type = type;          /* packet type */
    icmp_hdr.icmp_code = code;          /* packet code */
    icmp_hdr.icmp_id   = 0;             /* must be 0 */
    icmp_hdr.icmp_seq  = 0;             /* must be 0 */

    /*
     *  How convenient!  We can use our build_ip function to tack on the
     *  original header!
     */
    libnet_build_ip(0, orig_tos, orig_id, orig_frag, orig_ttl, orig_prot,
            orig_src, orig_dst, orig_payload, payload_s, buf +
            LIBNET_ICMP_TIMXCEED_H);

    memcpy(buf, &icmp_hdr, LIBNET_ICMP_TIMXCEED_H);
    return (1);
}


int
libnet_build_icmp_timestamp(u_char type, u_char code, u_short id, u_short seq,
        n_time otime, n_time rtime, n_time ttime, const u_char *payload,
        int payload_s, u_char *buf)
{
    struct libnet_icmp_hdr icmp_hdr;

    if (!buf)
    {
        return (-1);
    }

    icmp_hdr.icmp_type   = type;            /* packet type */
    icmp_hdr.icmp_code   = code;            /* packet code */
    icmp_hdr.icmp_id     = htons(id);       /* packet id */
    icmp_hdr.icmp_seq    = htons(seq);      /* packet seq */
    icmp_hdr.icmp_otime  = htonl(otime);    /* original timestamp */
    icmp_hdr.icmp_rtime  = htonl(rtime);    /* receive timestamp */
    icmp_hdr.icmp_ttime  = htonl(ttime);    /* transmit timestamp */

    if (payload && payload_s)
    {
        /*
         *  Unchecked runtime error for buf + ICMP_TS_H payload to be greater
         *  than the allocated heap memory.
         */
        memcpy(buf + LIBNET_ICMP_TS_H, payload, payload_s);
    }
    memcpy(buf, &icmp_hdr, LIBNET_ICMP_TS_H);
    return (1);
}


int
libnet_build_icmp_redirect(u_char type, u_char code, u_long gateway,
            u_short orig_len, u_char orig_tos, u_short orig_id,
            u_short orig_frag, u_char orig_ttl, u_char orig_prot,
            u_long orig_src, u_long orig_dst, const u_char *orig_payload,
            int payload_s, u_char *buf)
{
    struct libnet_icmp_hdr icmp_hdr;

    if (!buf)
    {
        return (-1);
    }

    icmp_hdr.icmp_type      = type;             /* packet type */
    icmp_hdr.icmp_code      = code;             /* packet code */
    icmp_hdr.hun.gateway    = htonl(gateway);   /* gateway address */

    /*
     *  How convenient!  We can use our build_ip function to tack on the
     *  original header!
     */
    libnet_build_ip(0, orig_tos, orig_id, orig_frag, orig_ttl, orig_prot,
            orig_src, orig_dst, orig_payload, payload_s, buf +
            LIBNET_ICMP_REDIRECT_H);

    memcpy(buf, &icmp_hdr, LIBNET_ICMP_REDIRECT_H);
    return (1);
}

/* EOF */
