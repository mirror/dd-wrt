/*
 *  $Id: arp.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  arp.c - Build an ARP packet
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
    char errbuf[256];
    char *device = NULL;
    struct libnet_link_int *l;

    while ((c = getopt(argc, argv, "i:")) != EOF)
    {
        switch (c)
        {
            case 'i':
                device = optarg;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (!device)
    {
        fprintf(stderr, "Specify a device\n");
        exit(EXIT_FAILURE);
    }

    l = libnet_open_link_interface(device, errbuf);
    if (!l)
    {
        fprintf(stderr, "libnet_open_link_interface: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }
    c = send_arp(l, device);

    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


int
send_arp(struct libnet_link_int *l, u_char *device)
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
    libnet_build_ethernet(enet_dst, enet_src, ETHERTYPE_ARP, NULL, 0, buf);

    /*
     *  ARP header
     */
    libnet_build_arp(ARPHRD_ETHER,
        ETHERTYPE_IP,
        6,
        4,
        ARPOP_REQUEST,
        enet_src,
        ip_src,
        enet_dst,
        ip_dst,
        NULL,
        0,
        buf + LIBNET_ETH_H);

    n = libnet_write_link_layer(l, device, buf, LIBNET_ARP_H + LIBNET_ETH_H);

    printf("Wrote %d byte ARP packet through linktype %d\n", n, l->linktype);

    libnet_destroy_packet(&buf);
    return (n);
}

/* EOF */
