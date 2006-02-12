/*
 *  $Id: get_mac.c,v 1.1 2004/04/27 01:32:51 dyang Exp $
 *
 *  libnet
 *  get_mac.c - get the MAC address of a remote host
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
#include <libnet.h>
#include <pcap.h>

#define IP_UCHAR_COMP(x, y) \
    (x[0] == y[0] && x[1] == y[1] && x[2] == y[2] && x[3] == y[3])


u_char enet_dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int libnet_do_arp(struct libnet_link_int *, u_char *, struct ether_addr *,
            u_long);

int
main(int argc, char *argv[])
{
    int i, c;
    char errbuf[256];
    char *device = NULL;
    struct libnet_link_int *l;
    struct ether_addr e;
    u_long ip;

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
    if (argc > optind)
    {
        if ((ip = libnet_name_resolve(argv[optind], 1)) == -1)
        {
            fprintf(stderr, "Cannot resolve IP address\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "IP address to ARP for?\n");
        exit(EXIT_FAILURE);
    }

    l = libnet_open_link_interface(device, errbuf);
    if (!l)
    {
        fprintf(stderr, "libnet_open_link_interface: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    c = libnet_do_arp(l, device, &e, ip);
    if (c != -1)
    {
        for (i = 0; i < 6; i++)
        {
            printf("%x", e.ether_addr_octet[i]);
            if (i != 5)
            {
                printf(":");
            }
        }
        printf("\n");
    }
    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


int
libnet_do_arp(struct libnet_link_int *l, u_char *device, struct ether_addr *e,
        u_long ip)
{
    int n, i;
    u_char *buf, errbuf[256], *packet, *ip_p;
    struct libnet_ethernet_hdr *p;
    struct libnet_arp_hdr *a;
    u_long local_ip;
    pcap_t *pd;
    struct pcap_pkthdr pc_hdr;

    /*
     *  Initialize a packet.
     */
    if (libnet_init_packet(LIBNET_ARP_H + LIBNET_ETH_H, &buf) == -1)
    {
        perror("libnet_init_packet memory:");
        exit(EXIT_FAILURE);
    }

    /*
     *  Get the ethernet address of the device.
     */
    e = libnet_get_hwaddr(l, device, errbuf);
    if (e == NULL)
    {
        fprintf(stderr, "libnet_get_hwaddr: %s\n", errbuf);
        return (-1);
    }

    /*
     *  Get the IP address of the device.
     */
    local_ip = htonl(libnet_get_ipaddr(l, device, errbuf));
    if (!local_ip)
    {
        fprintf(stderr, "libnet_get_ipaddr: %s\n", errbuf);
        return (-1);
    }

    /*
     *  Open the pcap device.
     */
    pd = pcap_open_live(device, LIBNET_ARP_H + LIBNET_ETH_H, 1, 500, errbuf);
    if (pd == NULL)
    {
        fprintf(stderr, "pcap_open_live: %s\n", errbuf);
        return (-1);
    }

    /*
     *  Ethernet header
     */
    libnet_build_ethernet(
            enet_dst,               /* broadcast ethernet address */
            e->ether_addr_octet,    /* source ethernet address */
            ETHERTYPE_ARP,          /* this frame holds an ARP packet */
            NULL,                   /* payload */
            0,                      /* payload size */
            buf);                   /* packet header memory */

    /*
     *  ARP header
     */
    libnet_build_arp(
            ARPHRD_ETHER,           /* hardware address type */
            ETHERTYPE_IP,           /* protocol address type */
            ETHER_ADDR_LEN,         /* hardware address length */
            4,                      /* protocol address length */
            ARPOP_REQUEST,          /* packet type - ARP request */
            e->ether_addr_octet,    /* source (local) ethernet address */
            (u_char *)&local_ip,    /* source (local) IP address */
            enet_dst,               /* target's ethernet address (broadcast) */
            (u_char *)&ip,          /* target's IP address */
            NULL,                   /* payload */
            0,                      /* payload size */
            buf + LIBNET_ETH_H);    /* packet header memory */

    n = libnet_write_link_layer(l, device, buf, LIBNET_ARP_H + LIBNET_ETH_H);
    if (n == -1)
    {
        fprintf(stderr, "libnet_write_link_layer choked\n");
        return (-1);
    }
    printf("Sent a %d byte ARP request looking for the MAC of %s.\n",
        n, libnet_host_lookup(ip, 0));
    printf("Waiting for a response...\n");

    for (;(packet = ((u_char *)pcap_next(pd, &pc_hdr)));)
    {
        p = (struct libnet_ethernet_hdr *)(packet);
        if (ntohs(p->ether_type) == ETHERTYPE_ARP)
        {
            a = (struct libnet_arp_hdr *)(packet + LIBNET_ETH_H);
            if (ntohs(a->ar_op) == ARPOP_REPLY)
            {
                ip_p = (u_char *)&ip;
                if (IP_UCHAR_COMP(a->ar_spa, ip_p))
                {
                    printf("Target hardware address: ");
                    for (i = 0; i < 6; i++)
                    {
                        printf("%x", a->ar_sha[i]);
                        if (i != 5)
                        {
                            printf(":");
                        }
                    }
                    printf("\n");
                }
            }
        }
    }

    libnet_destroy_packet(&buf);
    return (n);
}

/* EOF */

