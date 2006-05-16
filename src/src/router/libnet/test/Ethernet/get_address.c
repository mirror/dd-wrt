/*
 *  $Id: get_address.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  libnet
 *  get_address.c - Retrieve the MAC and IP address of an interface
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
    struct ether_addr *e;
    u_long i;

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

    printf("Interface %s\n", device);
    e = libnet_get_hwaddr(l, device,  errbuf);
    if (!e)
    {
        fprintf(stderr, "Can't get hardware address: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("MAC address: ");
        for (c = 0; c < 6; c++)
        {
            printf("%x", e->ether_addr_octet[c]);
            if (c != 5)
            {
                printf(":");
            }
        }
        printf("\n");
    }

    i = libnet_get_ipaddr(l, device, errbuf);
    if (!i)
    {
        fprintf(stderr, "Can't get ip address: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("IP  address: ");
        printf("%s\n", libnet_host_lookup(ntohl(i), 0));
    }
    exit(EXIT_SUCCESS);
}

/* EOF */
