/*
 *  $Id: icmp_mask.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  icmp.c - Build an ICMP_HOSTMASK packet at the link layer
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
    char errbuf[256];
    char *device = NULL;
    struct libnet_link_int *l;

    printf("link layer ICMP packet building/writing test\n");

    src_ip  = 0;
    dst_ip  = 0;
    while ((c = getopt(argc, argv, "i:d:s:")) != EOF)
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
            case 'i':
                device = optarg;
                break;
            case 's':
                if (!(src_ip = libnet_name_resolve(optarg, 1)))
                {
                    fprintf(stderr, "Bad source IP address: %s\n", optarg);
                    exit(1);
                }
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (!src_ip || !dst_ip || !device)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }   

    if ((l = libnet_open_link_interface(device, errbuf)) == NULL)
    {
        fprintf(stderr, "libnet_open_link_interface: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }
    c = send_icmp(l, device, src_ip, dst_ip);

    return (c == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}


int
send_icmp(struct libnet_link_int *l, u_char *device, u_long src_ip, u_long
            dst_ip)
{
    int n;
    u_char *buf;

    if (libnet_init_packet(LIBNET_ICMP_MASK_H + LIBNET_IP_H + LIBNET_ETH_H,
                &buf) == -1)
    {
        perror("no packet memory");
        exit(EXIT_FAILURE);
    }

    /*
     *  Ethernet header
     */
    libnet_build_ethernet(enet_dst, enet_src, ETHERTYPE_IP, NULL, 0, buf);

    libnet_build_ip(LIBNET_ICMP_MASK_H,
        0,                      /* IP tos */
        242,                    /* IP ID */
        0,                      /* Frag */
        64,                     /* TTL */
        IPPROTO_ICMP,           /* Transport protocol */
        src_ip,                 /* Source IP */
        dst_ip,                 /* Destination IP */
        NULL,                   /* Pointer to payload (none) */
        0,
        buf + LIBNET_ETH_H);    /* Packet header memory */

    libnet_build_icmp_mask(ICMP_MASKREPLY,  /* type */
		    0,                      /* code */ 
		    242,                    /* id */ 
		    0,                      /* seq */ 
		    0xffffffff,             /* mask */
		    NULL,                   /* payload */ 
		    0,                      /* payload_s */ 
		    buf + LIBNET_ETH_H + LIBNET_IP_H);

    libnet_do_checksum(buf + LIBNET_ETH_H, IPPROTO_IP, LIBNET_IP_H);
    libnet_do_checksum(buf + LIBNET_ETH_H, IPPROTO_ICMP, LIBNET_ICMP_MASK_H);

    printf("Packet as it will appear on the wire (give or take some byte ordering):");
    libnet_hex_dump(buf, LIBNET_ETH_H + LIBNET_IP_H + LIBNET_ICMP_MASK_H, 0,
                stdout);
    printf("\n");

    n = libnet_write_link_layer(l, device, buf, LIBNET_ETH_H + LIBNET_IP_H
                + LIBNET_ICMP_MASK_H);
    if (n != LIBNET_ETH_H + LIBNET_IP_H + LIBNET_ICMP_MASK_H)
    {
        fprintf(stderr, "Oopz.  Only wrote %d bytes\n", n);
    }
    else
    {
        printf("Wrote %d byte ICMP packet through linktype %d\n", n, l->linktype);
    }
    libnet_destroy_packet(&buf);
    return (n);
}


void
usage(u_char *name)
{
    fprintf(stderr, "usage: %s -i interface -s source_ip -d destination_ip\n", name);
}

/* EOF */
