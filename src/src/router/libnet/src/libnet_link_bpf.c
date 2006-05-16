/*
 *  $Id: libnet_link_bpf.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_bpf.c - low-level bpf routines
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Copyright (c) 1993, 1994, 1995, 1996, 1998
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

#include <sys/param.h>  /* optionally get BSD define */
#include <sys/timeb.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif 
#include "../include/libnet.h"
#include <sys/sysctl.h>
#include <net/route.h>
#include <net/if_dl.h>
#include "../include/gnuc.h"
#include "../include/bpf.h"

#ifdef HAVE_OS_PROTO_H
#include "../include/os-proto.h"
#endif

int
libnet_bpf_open(char *errbuf)
{
    int i, fd;
    char device[sizeof "/dev/bpf000"];

    /*
     *  Go through all the minors and find one that isn't in use.
     */
    for (i = 0;;i++)
    {
        sprintf(device, "/dev/bpf%d", i);

        fd = open(device, O_RDWR);
        if (fd == -1 && errno == EBUSY)
        {
            /*
             *  Device is busy.
             */
            continue;
        }
        else
        {
            /*
             *  Either we've got a valid file descriptor, or errno is not
             *  EBUSY meaning we've probably run out of devices.
             */
            break;
        }
    }

    if (fd == -1)
    {
        sprintf(errbuf, "%s: %s", device, ll_strerror(errno));
    }
    return (fd);
}


struct libnet_link_int *
libnet_open_link_interface(char *device, char *ebuf)
{
    struct ifreq ifr;
    struct bpf_version bv;
    u_int v;
    struct libnet_link_int *l;

    l = (struct libnet_link_int *)malloc(sizeof(*l));
    if (!l)
    {
        sprintf(ebuf, "malloc: %s", ll_strerror(errno));
#if (__DEBUG)
        libnet_error(LN_ERR_CRITICAL, "bpf libnet_open_link_interface: malloc %s",
                ll_strerror(errno));
#endif
        return (NULL);
    }

    memset(l, 0, sizeof(*l));

    l->fd = libnet_bpf_open(ebuf);
    if (l->fd == -1)
    {
        goto bad;
    }

    /*
     *  Get bpf version.
     */
    if (ioctl(l->fd, BIOCVERSION, (caddr_t)&bv) < 0)
    {
        sprintf(ebuf, "BIOCVERSION: %s", ll_strerror(errno));
        goto bad;
    }

    if (bv.bv_major != BPF_MAJOR_VERSION || bv.bv_minor < BPF_MINOR_VERSION)
    {
        sprintf(ebuf, "kernel bpf filter out of date");
        goto bad;
    }

    /*
     *  Attach network interface to bpf device.
     */
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
    if (ioctl(l->fd, BIOCSETIF, (caddr_t)&ifr) == -1)
    {
        sprintf(ebuf, "%s: %s", device, ll_strerror(errno));
        goto bad;
    }

    /*
     *  Get the data link layer type.
     */
    if (ioctl(l->fd, BIOCGDLT, (caddr_t)&v) == -1)
    {
        sprintf(ebuf, "BIOCGDLT: %s", ll_strerror(errno));
        goto bad;
    }

    /*
     *  Assign link type and offset.
     */
    switch (v)
    {
        case DLT_SLIP:
            l->linkoffset = 0x10;
            break;
        case DLT_RAW:
            l->linkoffset = 0x0;
            break;
        case DLT_PPP:
            l->linkoffset = 0x04;
            break;
        case DLT_EN10MB:
        default:
            l->linkoffset = 0xe;     /* default to ethernet */
            break;
    }
#if _BSDI_VERSION - 0 >= 199510
    switch (v)
    {
        case DLT_SLIP:
            v = DLT_SLIP_BSDOS;
            l->linkoffset = 0x10;
            break;
        case DLT_PPP:
            v = DLT_PPP_BSDOS;
            l->linkoffset = 0x04;
            break;
    }
#endif
    l->linktype = v;

    return (l);

bad:
    close(l->fd);      /* this can fail ok */
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

    c = write(l->fd, buf, len);
    if (c != len)
    {
#if (__DEBUG)
        libnet_error(LN_ERR_WARNING,
                "write_link_layer: %d bytes written (%s)\n", c,
                strerror(errno));
#endif
    }
    return (c);
}


struct ether_addr *
libnet_get_hwaddr(struct libnet_link_int *l, const u_char *device, char *ebuf)
{
    int mib[6];
    size_t len;
    char *buf, *next, *end;
    struct if_msghdr *ifm;
    struct sockaddr_dl *sdl;
    struct ether_addr *ea = NULL;

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    mib[5] = 0;

    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
    {
        return (NULL);
    }

    if (!(buf = (char *)malloc(len)))
    {
        return NULL;
    }
   if (sysctl(mib, 6, buf, &len, NULL, 0) < 0)
   {
        free(buf);
        return (NULL);
    }
    end = buf + len;

    for (next = buf ; next < end ; next += ifm->ifm_msglen)
    {
        ifm = (struct if_msghdr *)next;
        if (ifm->ifm_type == RTM_IFINFO)
        {
            sdl = (struct sockaddr_dl *)(ifm + 1);
            if (strncmp(&sdl->sdl_data[0], device, sdl->sdl_nlen) == 0)
            {
                if (!(ea = malloc(sizeof(struct ether_addr))))
                {
                    return (NULL);
                }
                memcpy(ea->ether_addr_octet, LLADDR(sdl), ETHER_ADDR_LEN);
                break;
            }
        }
    }
    free(buf);
    return (ea);
}


/* EOF */
