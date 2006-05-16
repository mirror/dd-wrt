/*
 *  $Id: silvertongue.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  silvertongue.c - ICMP_REDIRECT Packet assembly tester
 *
 *  Written by Nicholas Brawn with some modifications by Mike D. Schiffman.
 *  Copyright (c) 1999 Nicholas Brawn <nick@feralmonkey.org>
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
#include "../../include/config.h"
#endif
#include "../libnet_test.h"

int
main(int argc, char **argv)
{
    int sockfd, c;
    u_char *buf;
    u_long src, dst, gateway;

    if (argc < 4)
    {
        fprintf(stderr,
                "usage: %s <old_router> <target> <new_gateway>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!(src = libnet_name_resolve(argv[1], 1)))
    {
        perror("Error resolving source host");
        exit(EXIT_FAILURE);
    }

    if (!(dst = libnet_name_resolve(argv[2], 1)))
    {
        perror("Error resolving destination host");
        exit(EXIT_FAILURE);
    }

    if (!(gateway = libnet_name_resolve(argv[3], 1)))
    {
        perror("Error resolving gateway host");
        exit(EXIT_FAILURE);
    }

    if (libnet_init_packet(IP_MAXPACKET, &buf) == -1)
    {
        perror("Couldn't allocate memory for header");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = libnet_open_raw_sock(IPPROTO_RAW)) == -1)
    {
        perror("Couldn't open raw socket");
        exit(EXIT_FAILURE);
    }

    libnet_build_ip(LIBNET_ICMP_REDIRECT_H + LIBNET_IP_H,
                IPTOS_LOWDELAY | IPTOS_THROUGHPUT,
                242,
                0,
                48,
                IPPROTO_ICMP,
                src,
                dst,
                NULL,
                0,
                buf);

    libnet_build_icmp_redirect(
                ICMP_REDIRECT,
                ICMP_UNREACH_HOST,
                gateway,
                0,                                  /* just an ip header */
                IPTOS_LOWDELAY | IPTOS_THROUGHPUT,  /* IP tos */
                424,                                /* IP ID */
                0,                                  /* Frag stuff */
                64,                                 /* TTL */
                IPPROTO_ICMP,                       /* Transport protocol */
                dst,                                /* Source IP */
                src,                                /* Destination IP */
                NULL,                               /* pointer to payload */
                0,                                  /* size of payload */
                buf + LIBNET_IP_H);                 /* packet header memory */



    libnet_do_checksum(buf, IPPROTO_ICMP, LIBNET_ICMP_REDIRECT_H +
                LIBNET_IP_H);
 
    c = libnet_write_ip(sockfd, buf, LIBNET_ICMP_REDIRECT_H + 2 *
                LIBNET_IP_H);
    if (c != LIBNET_ICMP_REDIRECT_H + 2 * LIBNET_IP_H)
    {
        fprintf(stderr, "Error writing to socket, only wrote %d bytes\n", c);
        exit(EXIT_FAILURE);
    }
    printf("Completed, wrote %d bytes\n", c);
    libnet_destroy_packet(&buf);

    exit(EXIT_SUCCESS);
}
