/*
 *  $Id: libnet_link_pf.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_pf.c - pf routines
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Copyright (c) 1990, 1991, 1992, 1993, 1994, 1995, 1996
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
 *
 * packet filter subroutines for tcpdump
 *	Extraction/creation by Jeffrey Mogul, DECWRL
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/low_libnet.h"

#include "../include/gnuc.h"
#ifdef HAVE_OS_PROTO_H
#include "../include/os-proto.h"
#endif

struct libnet_link_int *
libnet_open_link_interface(char *device, char *ebuf)
{
    register struct libnet_link_int *l;
    short enmode;
    int backlog = -1;   /* request the most */
    struct enfilter Filter;
    struct endevp devparams;

    l = (struct libnet_link_int *)malloc(sizeof(*l));
    if (l == NULL)
    {
        sprintf(ebuf, "libnet_open_link_int: %s", ll_strerror(errno));
        return (0);
    }
    memset(l, 0, sizeof(*l));
    l->fd = pfopen(device, O_RDWR);
    if (l->fd < 0)
    {
        sprintf(ebuf, "pf open: %s: %s\n\your system may not be properly configured; see \"man packetfilter(4)\"\n",
            device, ll_strerror(errno));
        goto bad;
    }

    enmode = ENTSTAMP|ENBATCH|ENNONEXCL;
    if (ioctl(l->fd, EIOCMBIS, (caddr_t)&enmode) < 0)
    {
        sprintf(ebuf, "EIOCMBIS: %s", ll_strerror(errno));
        goto bad;
    }
#ifdef	ENCOPYALL
    /* Try to set COPYALL mode so that we see packets to ourself */
    enmode = ENCOPYALL;
    ioctl(l->fd, EIOCMBIS, (caddr_t)&enmode);   /* OK if this fails */
#endif
	/* set the backlog */
    if (ioctl(l->fd, EIOCSETW, (caddr_t)&backlog) < 0)
    {
        sprintf(ebuf, "EIOCSETW: %s", ll_strerror(errno));
        goto bad;
    }
    /*
     *  discover interface type
     */
    if (ioctl(l->fd, EIOCDEVP, (caddr_t)&devparams) < 0)
    {
        sprintf(ebuf, "EIOCDEVP: %s", ll_strerror(errno));
        goto bad;
    }

    /* HACK: to compile prior to Ultrix 4.2 */
#ifndef	ENDT_FDDI
#define	ENDT_FDDI   4
#endif
    switch (devparams.end_dev_type)
    {
        case ENDT_10MB:
            l->linktype = DLT_EN10MB;
            break;
        case ENDT_FDDI:
            l->linktype = DLT_FDDI;
            break;
        default:
            /*
             * XXX
             * Currently, the Ultrix packet filter supports only
             * Ethernet and FDDI.  Eventually, support for SLIP and PPP
             * (and possibly others: T1?) should be added.
             */
            l->linktype = DLT_EN10MB;
            break;
	}
    /*
     *  accept all packets
     */
    bzero((char *)&Filter, sizeof(Filter));
    Filter.enf_Priority = 37;	/* anything > 2 */
    Filter.enf_FilterLen = 0;	/* means "always true" */
    if (ioctl(l->fd, EIOCSETF, (caddr_t)&Filter) < 0)
    {
        sprintf(ebuf, "EIOCSETF: %s", ll_strerror(errno));
        goto bad;
    }

    return (l);
bad:
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
            const u_char *buf, int len)
{
    int c;

    c = write(l->fd, buf, len);
    if (c != len)
    {
#if (__DEBUG)
        fprintf(stderr, "libnet_write_link_layer: %d bytes written (%s)\n", c,
            strerror(errno));
#endif
    }
    return (c);
}
