/*
 *  $Id: libnet-example-2.c,v 1.1 2004/04/27 01:31:22 dyang Exp $
 *
 *  libnet example code
 *  example 2:  link-layer api / ICMP hostmask packet
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

#include "../include/libnet.h"

void usage(char *);

u_char enet_src[6] = {0x0d, 0x0e, 0x0a, 0x0d, 0x00, 0x00};
u_char enet_dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int
main(int argc, char *argv[])
{
    int packet_size,                    /* size of our packet */
        c;                              /* misc */
    u_long src_ip, dst_ip;              /* source ip, dest ip */
    u_char *packet;                     /* pointer to our packet buffer */
    char err_buf[LIBNET_ERRBUF_SIZE];   /* error buffer */
    u_char *device;                     /* pointer to the device to use */
    struct libnet_link_int *network;    /* pointer to link interface struct */

    printf("libnet example code:\tmodule 2\n\n");
    printf("packet injection interface:\tlink layer\n");
    printf("packet type:\t\t\tICMP net mask [no payload]\n");

    device = NULL;
    src_ip  = 0;
    dst_ip  = 0;

    while ((c = getopt(argc, argv, "i:d:s:")) != EOF)
    {
        switch (c)
        {
            case 'd':
                if (!(dst_ip = libnet_name_resolve(optarg, LIBNET_RESOLVE)))
                {
                    libnet_error(LIBNET_ERR_FATAL, "Bad destination IP address: %s\n", optarg);

                }
                break;
            case 'i':
                device = optarg;
                break;
            case 's':
                if (!(src_ip = libnet_name_resolve(optarg, LIBNET_RESOLVE)))
                {
                    libnet_error(LIBNET_ERR_FATAL, "Bad source IP address: %s\n", optarg);
                }
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (!src_ip || !dst_ip)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }


    /*
     *  Step 1: Network Initialization (interchangable with step 2).
     */
    if (device == NULL)
    {
        struct sockaddr_in sin;
        /*
         *  Try to locate a device.
         */
        if (libnet_select_device(&sin, &device, err_buf) == -1)
        {
            libnet_error(LIBNET_ERR_FATAL, "libnet_select_device failed: %s\n", err_buf);
        }
        printf("device:\t\t\t\t%s\n", device);
    }
    if ((network = libnet_open_link_interface(device, err_buf)) == NULL)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_open_link_interface: %s\n", err_buf);
    }


    /*
     *  We're going to build an ICMP packet with no payload using the
     *  link-layer API, so this time we need memory for a ethernet header
     *  as well as memory for the ICMP and IP headers.
     */
    packet_size = LIBNET_IP_H + LIBNET_ETH_H + LIBNET_ICMP_MASK_H;


    /*
     *  Step 2: Memory Initialization (interchangable with step 1).
     */
    if (libnet_init_packet(packet_size, &packet) == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_init_packet failed\n");
    }


    /*
     *  Step 3: Packet construction (ethernet header).
     */
    libnet_build_ethernet(enet_dst,
            enet_src,
            ETHERTYPE_IP,
            NULL,
            0,
            packet);

    /*
     *  Step 3: Packet construction (IP header).
     */
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
            packet + LIBNET_ETH_H); /* Packet header memory */


    /*
     *  Step 3: Packet construction (ICMP header).
     */
    libnet_build_icmp_mask(ICMP_MASKREPLY,  /* type */
            0,                      /* code */ 
            242,                    /* id */ 
            0,                      /* seq */ 
            0xffffffff,             /* mask */
            NULL,                   /* payload */ 
            0,                      /* payload_s */ 
            packet + LIBNET_ETH_H + LIBNET_IP_H);

    /*
     *  Step 4: Packet checksums (ICMP header *AND* IP header).
     */
    if (libnet_do_checksum(packet + LIBNET_ETH_H, IPPROTO_ICMP, LIBNET_ICMP_MASK_H) == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_do_checksum failed\n");
    }
    if (libnet_do_checksum(packet + LIBNET_ETH_H, IPPROTO_IP, LIBNET_IP_H) == -1)
    {
        libnet_error(LIBNET_ERR_FATAL, "libnet_do_checksum failed\n");
    }


    /*
     *  Step 5: Packet injection.
     */
    c = libnet_write_link_layer(network, device, packet, packet_size);
    if (c < packet_size)
    {
        libnet_error(LIBNET_ERR_WARNING, "libnet_write_link_layer only wrote %d bytes\n", c);
    }
    else
    {
        printf("construction and injection completed, wrote all %d bytes\n", c);
    }


    /*
     *  Shut down the interface.
     */
    if (libnet_close_link_interface(network) == -1)
    {   
        libnet_error(LIBNET_ERR_WARNING, "libnet_close_link_interface couldn't close the interface");
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
    fprintf(stderr, "usage: %s [-i interface] -s s_ip -d d_ip\n", name);
}

/* EOF */
