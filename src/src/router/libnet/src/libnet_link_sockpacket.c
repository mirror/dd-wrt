/*
 *  $Id: libnet_link_sockpacket.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_sockpacket.c - linux sockpacket routines
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Copyright (c) 1996, 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include <sys/time.h>

#include <net/if.h>
#if (__GLIBC__)
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#else
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif

#if (HAVE_PF_PACKET)
#include <linux/if_packet.h>
#ifndef SOL_PACKET
#define SOL_PACKET 263
#endif  /* SOL_PACKET */
#endif  /* HAVE_PF_PACKET */

#include "../include/bpf.h"
#include "../include/libnet.h"

#include "../include/gnuc.h"
#ifdef HAVE_OS_PROTO_H
#include "../include/os-proto.h"
#endif

struct libnet_link_int *
libnet_open_link_interface(char *device, char *ebuf)
{
    register struct libnet_link_int *l;
    struct ifreq ifr;
#if (HAVE_PF_PACKET)
    struct packet_mreq mr;
    struct sockaddr_ll sa;
#endif

    l = (struct libnet_link_int *)malloc(sizeof (*l));
    if (l == NULL)
    {
        sprintf(ebuf, "malloc: %s", ll_strerror(errno));
        return (NULL);
    }
    memset(l, 0, sizeof (*l));

#if (HAVE_PF_PACKET)
    l->fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
#else
    l->fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL));
#endif
    if (l->fd == -1)
    {
        sprintf(ebuf, "socket: %s", ll_strerror(errno));
        goto bad;
    }

#if (HAVE_PF_PACKET)
    strcpy(ifr.ifr_name, device);
    if (ioctl(l->fd, SIOCGIFINDEX, &ifr) < 0)
    {
        sprintf(ebuf, "ioctl: SIOCGIFINDEX: %s", ll_strerror(errno));
        goto bad;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_protocol = htons(ETH_P_ALL);

    memset(&mr, 0, sizeof (mr));
    mr.mr_ifindex = ifr.ifr_ifindex;
    mr.mr_type = PACKET_MR_ALLMULTI;

    if (setsockopt(l->fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (char *)&mr,
        sizeof (mr)) < 0)
    {
        sprintf(ebuf, "setsockopt %s: %s", device, ll_strerror(errno));
        goto bad;
    }
#endif

    memset(&ifr, 0, sizeof (ifr));
    strncpy(ifr.ifr_name, device, sizeof (ifr.ifr_name));
    if (ioctl(l->fd, SIOCGIFHWADDR, &ifr) < 0 )
    {
        sprintf(ebuf, "SIOCGIFHWADDR: %s", ll_strerror(errno));
        goto bad;
    }

    switch (ifr.ifr_hwaddr.sa_family)
    {
        case ARPHRD_ETHER:
        case ARPHRD_METRICOM:
            l->linktype = DLT_EN10MB;
            l->linkoffset = 0xe;
            break;
        case ARPHRD_SLIP:
        case ARPHRD_CSLIP:
        case ARPHRD_SLIP6:
        case ARPHRD_CSLIP6:
        case ARPHRD_PPP:
            l->linktype = DLT_RAW;
            break;
        default:
            sprintf(ebuf, "unknown physical layer type 0x%x",
                ifr.ifr_hwaddr.sa_family);
        goto bad;
    }
    return (l);

bad:
    if (l->fd >= 0)
    {
        close(l->fd);
    }
    free(l);
    return (NULL);
}


int
libnet_close_link_interface(struct libnet_link_int *l)
{
    return (close(l->fd));
}


int
libnet_write_link_layer(struct libnet_link_int *l, const u_char *device,
            u_char *buf, int len)
{
    int c;
    struct sockaddr sa;

    memset(&sa, 0, sizeof (sa));
    strncpy(sa.sa_data, device, sizeof (sa.sa_data));

    c = sendto(l->fd, buf, len, 0, &sa, sizeof (sa));
    if (c != len)
    {
#if (__DEBUG)
        libnet_error(LIBNET_ERR_WARNING,
            "write_link_layer: %d bytes written (%s)\n", c,
            strerror(errno));
#endif
    }
    return (c);
}


struct ether_addr *
libnet_get_hwaddr(struct libnet_link_int *l, const u_char *device, char *ebuf)
{
    int fd;
    struct ifreq ifr;
    struct ether_addr *eap;
    /*
     *  XXX - non-re-entrant!
     */
    static struct ether_addr ea;

    /*
     *  Create dummy socket to perform an ioctl upon.
     */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        sprintf(ebuf, "get_hwaddr: %s", strerror(errno));
        return (NULL);
    }

    memset(&ifr, 0, sizeof(ifr));
    eap = &ea;
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFHWADDR, (char *)&ifr) < 0)
    {
        close(fd);
        sprintf(ebuf, "get_hwaddr: %s", strerror(errno));
        return (NULL);
    }
    memcpy(eap, &ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
    close(fd);
    return (eap);
}


/* EOF */
