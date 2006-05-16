/*
 *  $Id: libnet-example-1.c,v 1.1 2004/04/27 01:31:22 dyang Exp $
 *
 *  libnet example code
 *  example 1:  raw socket api / TCP packet
 *
 *  Copyright (c) 1999 Mike D. Schiffman <mike@infonexus.com>
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

#include "../include/libnet.h"

void usage(char *);

int
main(int argc, char **argv)
{
    int network, packet_size, c;
    u_long src_ip, dst_ip;
    u_short src_prt, dst_prt;
    u_char *cp, *packet;

    printf("libnet example code:\tmodule 1\n\n");
    printf("packet injection interface:\traw socket\n");
    printf("packet type:\t\t\tTCP [no payload]\n");

    src_ip  = 0;
    dst_ip  = 0;
    src_prt = 0;
    dst_prt = 0;

    while((c = getopt(argc, argv, "d:s:")) != EOF)
    {      
        switch (c)
        {
            /*
             *  We expect the input to be of the form `ip.ip.ip.ip.port`.  We
             *  point cp to the last dot of the IP address/port string and
             *  then seperate them with a NULL byte.  The optarg now points to
             *  just the IP address, and cp points to the port.
             */
            case 'd':
                if (!(cp = strrchr(optarg, '.')))
                {
                    usage(argv[0]);
                }
                *cp++ = 0;
                dst_prt = (u_short)atoi(cp);
                if (!(dst_ip = libnet_name_resolve(optarg, LIBNET_RESOLVE)))
                {
                    libnet_error(LIBNET_ERR_FATAL, "Bad destination IP address: %s\n", optarg);
                }
                break;
            case 's':
                if (!(cp = strrchr(optarg, '.')))
                {
                    usage(argv[0]);
                }
                *cp++ = 0;
                src_prt = (u_short)atoi(cp);
                if (!(src_ip = libnet_name_resolve(optarg, LIBNET_RESOLVE)))
                {
                    libnet_error(LIBNET_ERR_FATAL, "Bad source IP address: %s\n", optarg);
                }
                break;
        }
    }
    if (!src_ip || !src_prt || !dst_ip || !dst_prt)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
     *  We're just going to build a TCP packet with no payload using the
     *  raw sockets API, so we only need memory for a TCP header and an IP
     *  header.
     */
    packet_size = LIBNET_IP_H + LIBNET_TCP_H;

    /*
     *  Step 1: Memory initialization (interchangable with step 2).
     */
    libnet_init_packet(packet_size, &packet);
    if (packet == NULL)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_init_packet failed\n");
    }

    /*
     *  Step 2: Network initialization (interchangable with step 1).
     */
    network = libnet_open_raw_sock(IPPROTO_RAW);
    if (network == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "Can't open network.\n");
    }
    

    /*
     *  Step 3: Packet construction (IP header).
     */
    libnet_build_ip(LIBNET_TCP_H,   /* size of the packet sans IP header */
            IPTOS_LOWDELAY,         /* IP tos */
            242,                    /* IP ID */
            0,                      /* frag stuff */
            48,                     /* TTL */
            IPPROTO_TCP,            /* transport protocol */
            src_ip,                 /* source IP */
            dst_ip,                 /* destination IP */
            NULL,                   /* payload (none) */
            0,                      /* payload length */
            packet);                /* packet header memory */


    /*
     *  Step 3: Packet construction (TCP header).
     */
    libnet_build_tcp(src_prt,       /* source TCP port */
            dst_prt,                /* destination TCP port */
            0xa1d95,                /* sequence number */
            0x53,                   /* acknowledgement number */
            TH_SYN,                 /* control flags */
            1024,                   /* window size */
            0,                      /* urgent pointer */
            NULL,                   /* payload (none) */
            0,                      /* payload length */
            packet + LIBNET_IP_H);  /* packet header memory */


    /*
     *  Step 4: Packet checksums (TCP header only).
     */
    if (libnet_do_checksum(packet, IPPROTO_TCP, LIBNET_TCP_H) == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_do_checksum failed\n");
    }


    /*
     *  Step 5: Packet injection.
     */
    c = libnet_write_ip(network, packet, packet_size);
    if (c < packet_size)
    {
        libnet_error(LIBNET_ERR_WARNING, "libnet_write_ip only wrote %d bytes\n", c);
    }
    else
    {
        printf("construction and injection completed, wrote all %d bytes\n", c);
    }

    /*
     *  Shut down the interface.
     */
    if (libnet_close_raw_sock(network) == -1)
    {
        libnet_error(LIBNET_ERR_WARNING, "libnet_close_raw_sock couldn't close the interface");
    }


    /*
     *  Free packet memory.
     */
    libnet_destroy_packet(&packet);

    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


void
usage(char *name)
{
    fprintf(stderr, "usage: %s -s s_ip.s_port -d d_ip.d_port\n", name);
}


/* EOF */
