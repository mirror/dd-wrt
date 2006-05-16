/*
 *  $Id: icmp_echo.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  icmp_echo.c - ICMP_ECHO Packet assembly tester
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
#include "../../include/config.h"
#endif
#include "../libnet_test.h"

int
main(int argc, char **argv)
{
    int sock, n, c, p_num;
    struct libnet_arena arena, *arena_p;
    u_char *packets[10];
    u_long src_ip, dst_ip;
    
    printf("ICMP_ECHO / Arena allocator test\n");

    src_ip = 0;
    dst_ip = 0;
    while((c = getopt(argc, argv, "d:s:")) != EOF)
    {
        switch (c)
        {
            case 'd':
                if (!(dst_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad destination IP address: %s\n", optarg);
                    exit(1);
                }
                break;
            case 's':
                if (!(src_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad source IP address: %s\n", optarg);
                    exit(1);
                }
                break;
        }
    }
    if (!src_ip || !dst_ip)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    arena_p = &arena;
    p_num = 10;
    if (libnet_init_packet_arena(&arena_p, p_num, LIBNET_ICMP_ECHO_H +
                LIBNET_IP_H) == -1)
    {
        fprintf(stderr, "libnet_init_packet_arena failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Allocated an arena of %ld bytes..\n",
            LIBNET_GET_ARENA_SIZE(arena));
    }

    /*
     *  Open our raw IP socket and set IP_HDRINCL.
     */
    sock = libnet_open_raw_sock(IPPROTO_RAW);
    if (sock == -1)
    {
        perror("No socket");
        exit(EXIT_FAILURE);
    }
    
    for (n = 0; n < p_num; n++)
    {
        printf("%ld bytes remaining in arena\n",
            LIBNET_GET_ARENA_REMAINING_BYTES(arena));
        packets[n] = libnet_next_packet_from_arena(&arena_p,
            LIBNET_ICMP_ECHO_H + LIBNET_IP_H);
        if (!packets[n])
        {
            fprintf(stderr, "Arena is empty\n");
            continue;
        }

        /*
         *  Build the IP header (shown exploded for commenting).
         */
        libnet_build_ip(LIBNET_ICMP_ECHO_H,     /* Size of the payload */
                IPTOS_LOWDELAY | IPTOS_THROUGHPUT, /* IP tos */
                242,                            /* IP ID */
                0,                              /* Frag stuff */
                48,                             /* TTL */
                IPPROTO_ICMP,                   /* Transport protocol */
                src_ip,                         /* Source IP */
                dst_ip,                         /* Destination IP */
                NULL,                           /* Pointer to payload (none) */
                0,
                packets[n]);                    /* Packet header memory */

        /*
         *  Build the ICMP header.
         */
        libnet_build_icmp_echo(ICMP_ECHO,       /* type */
                0,                              /* code */
                242,                            /* id */
                1,                              /* seq */
                NULL,	                        /* pointer to payload */
                0,                              /* size of payload */
                packets[n] + LIBNET_IP_H);      /* packet header memory */

        if (libnet_do_checksum(packets[n], IPPROTO_ICMP, LIBNET_ICMP_ECHO_H)
                    == -1)
        {
            fprintf(stderr, "Can't do checksum!\n");
        }

        /*
         *  Write the packet to the network.
         */
        c = libnet_write_ip(sock, packets[n], LIBNET_ICMP_ECHO_H + LIBNET_IP_H);
        if (c < LIBNET_ICMP_ECHO_H + LIBNET_IP_H)
        {
            fprintf(stderr, "write_ip\n");
        }
        printf("Completed %d of %d, wrote %d bytes\n", n + 1, p_num, c);
    }

    libnet_destroy_packet_arena(&arena_p);
    /* Blah. */
    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


void
usage(u_char *name)
{
    fprintf(stderr, "usage: %s -s source_ip -d destination_ip\n ", name);
}

/* EOF */
