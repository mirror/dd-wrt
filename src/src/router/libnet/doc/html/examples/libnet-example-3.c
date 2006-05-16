/*
 *  $Id: libnet-example-3.c,v 1.1 2004/04/27 01:30:28 dyang Exp $
 *
 *  libnet example code
 *  example 2:  raw socket api / ICMP_ECHO packet(s) using an arena
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

#include <libnet.h>

void usage(char *);


int
main(int argc, char **argv)
{
    int network, n, c, number_of_packets, packet_size;
    struct libnet_arena arena, *arena_p;
    u_char *packets[10];
    u_long src_ip, dst_ip;
    
    printf("libnet example code:\tmodule 3\n\n");
    printf("packet injection interface:\tlink layer\n");
    printf("packet type:\t\t\tICMP_ECHO [no payload] using an arena\n");

    src_ip = 0;
    dst_ip = 0;
    while((c = getopt(argc, argv, "d:s:")) != EOF)
    {
        switch (c)
        {
            case 'd':
                if (!(dst_ip = libnet_name_resolve(optarg, LIBNET_RESOLVE)))
                {
                    libnet_error(LIBNET_ERR_FATAL, "Bad destination IP address: %s\n", optarg);
                }
                break;
            case 's':
                if (!(src_ip = libnet_name_resolve(optarg, LIBNET_RESOLVE)))
                {
                    libnet_error(LIBNET_ERR_FATAL, "Bad source IP address: %s\n", optarg);
                }
                break;
        }
    }
    if (!src_ip || !dst_ip)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
     *  We're just going to build an ICMP packet with no payload using the
     *  raw sockets API, so we only need memory for a ICMP header and an IP
     *  header.
     */
    packet_size = LIBNET_IP_H + LIBNET_ICMP_ECHO_H;

    /*
     *  Let's just build say, 10 packets.
     */
    number_of_packets = 10;

    arena_p = &arena;
    if (libnet_init_packet_arena(&arena_p, number_of_packets, packet_size) == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_init_packet_arena failed\n");
    }
    else
    {
        printf("Allocated an arena of %ld bytes..\n", LIBNET_GET_ARENA_SIZE(arena));
    }

    network = libnet_open_raw_sock(IPPROTO_RAW);
    if (network == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "Can't open the network.\n");
    }
    
    for (n = 0; n < number_of_packets; n++)
    {
        printf("%ld bytes remaining in arena\n", LIBNET_GET_ARENA_REMAINING_BYTES(arena));
        packets[n] = libnet_next_packet_from_arena(&arena_p, packet_size);
        if (!packets[n])
        {
            libnet_error(LIBNET_ERR_WARNING, "Arena is empty\n");
            continue;
        }

        libnet_build_ip(ICMP_ECHO_H,          	    /* Size of the payload */
                IPTOS_LOWDELAY | IPTOS_THROUGHPUT,  /* IP tos */
                242,                                /* IP ID */
                0,                                  /* frag stuff */
                48,                                 /* TTL */
                IPPROTO_ICMP,                       /* transport protocol */
                src_ip,                             /* source IP */
                dst_ip,                             /* destination IP */
                NULL,                               /* pointer to payload */
                0,                                  /* payload length */
                packets[n]);                        /* packet header memory */

        libnet_build_icmp_echo(ICMP_ECHO,           /* type */
                0,                                  /* code */
                242,                                /* id */
                5,                                  /* seq */
                NULL,	                            /* pointer to payload */
                0,                                  /* payload length */
                packets[n] + LIBNET_IP_H);          /* packet header memory */

        if (libnet_do_checksum(packets[n], IPPROTO_ICMP, LIBNET_ICMP_ECHO_H) == -1)
        {
            libnet_error(LIBNET_ERR_FATAL, "libnet_do_checksum failed\n");
        }

        c = libnet_write_ip(network, packets[n], packet_size);
        if (c < packet_size)
        {
            libnet_error(LN_ERR_WARNING, "libnet_write_ip only wrote %d bytes\n", c);
        }
        else
        {
            printf("construction and injection of packet %d of %d completed, wrote all %d bytes\n",  n + 1, number_of_packets, c);
        }
    }

    libnet_destroy_packet_arena(&arena_p);
    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


void
usage(char *name)
{
    fprintf(stderr, "usage: %s -s source_ip -d destination_ip\n ", name);
}

/* EOF */
