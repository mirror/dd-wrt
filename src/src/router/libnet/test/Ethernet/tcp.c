/*
 *  $Id: tcp.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  tcp.c - Build a TCP packet at the link layer
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *                           route|daemon9 <route@infonexus.com>
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
main(int argc, char *argv[])
{
    int  c;
    u_long src_ip, dst_ip;
    u_short src_prt, dst_prt;
    u_char *cp;
    char errbuf[256];
    char *device = NULL;
    struct libnet_link_int *l;

    printf("link layer TCP packet building/writing test\n");

    src_ip  = 0;
    dst_ip  = 0;
    src_prt = 0;
    dst_prt = 0;
    while ((c = getopt(argc, argv, "i:d:s:")) != EOF)
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
                if (!(dst_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad destination IP address: %s\n", optarg);
                    exit(1);
                }
                break;
            case 's':
                if (!(cp = strrchr(optarg, '.')))
                {
                    usage(argv[0]);
                }
                *cp++ = 0;
                src_prt = (u_short)atoi(cp);
                if (!(src_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad source IP address: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'i':
                device = optarg;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (!src_ip || !src_prt || !dst_ip || !dst_prt || !device)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((l = libnet_open_link_interface(device, errbuf)) == NULL)
    {
        fprintf(stderr, "libnet_open_link_interface: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }
    c = send_tcp(l, device, src_ip, src_prt, dst_ip, dst_prt);

    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


int
send_tcp(struct libnet_link_int *l, u_char *device, u_long src_ip, u_short
        src_prt, u_long dst_ip, u_short dst_prt)
{
    int n;
    u_char *buf;

    if (libnet_init_packet(LIBNET_TCP_H + LIBNET_IP_H + LIBNET_ETH_H,
                &buf) == -1)
    {
        perror("no packet memory");
        exit(EXIT_FAILURE);
    }

    /*
     *  Ethernet header
     */
    libnet_build_ethernet(enet_dst, enet_src, ETHERTYPE_IP, NULL, 0, buf);

    libnet_build_ip(LIBNET_TCP_H,
        0,
        242,
        0,
        64,
        IPPROTO_TCP,
        src_ip,
        dst_ip,
        NULL,
        0,
        buf + LIBNET_ETH_H);

    libnet_build_tcp(src_prt,
        dst_prt,
        111111,
        999999,
        TH_SYN,
        32767,
        0,
        NULL,
        0,
        buf + LIBNET_IP_H + LIBNET_ETH_H);

    libnet_do_checksum(buf + LIBNET_ETH_H, IPPROTO_IP, LIBNET_IP_H);
    libnet_do_checksum(buf + LIBNET_ETH_H, IPPROTO_TCP, LIBNET_TCP_H);

    n = libnet_write_link_layer(l, device, buf, LIBNET_ETH_H + LIBNET_IP_H
                + LIBNET_TCP_H);
    if (n != LIBNET_ETH_H + LIBNET_IP_H + LIBNET_TCP_H)
    {
        fprintf(stderr, "Oopz.  Only wrote %d bytes\n", n);
    }
    else
    {
        printf("Wrote %d byte TCP packet through linktype %d\n", n, l->linktype);
    }
    libnet_destroy_packet(&buf);
    return (n);
}

void
usage(u_char *name)
{
    fprintf(stderr,
        "usage: %s -i interface -s source_ip.source_port"
        " -d destination_ip.destination_port\n",
        name);
}

/* EOF */
