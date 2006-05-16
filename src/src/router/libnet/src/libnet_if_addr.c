/*
 *  $Id: libnet_if_addr.c,v 1.1 2004/04/27 01:29:50 dyang Exp $
 *
 *  libnet
 *  libnet_if_addr.c - interface selection code
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  Originally pulled from traceroute sources.
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
#include "../include/config.h"
#endif
#include "../include/libnet.h"
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#define MAX_IPADDR 32

/*
 *  Return the interface list
 */
#if (!__solaris__)
int
libnet_ifaddrlist(register struct libnet_ifaddr_list **ipaddrp,
            register char *errbuf)
{
    register int fd, nipaddr;
#ifdef HAVE_SOCKADDR_SA_LEN
    register int n;
#endif
    register struct ifreq *ifrp, *ifend, *ifnext, *mp;
    register struct sockaddr_in *sin;
    register struct libnet_ifaddr_list *al;
    struct ifconf ifc;
    struct ifreq ibuf[MAX_IPADDR], ifr;
    char device[sizeof(ifr.ifr_name) + 1];
    static struct libnet_ifaddr_list ifaddrlist[MAX_IPADDR];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        sprintf(errbuf, "socket: %s", strerror(errno));
        return (-1);
    }
    ifc.ifc_len = sizeof(ibuf);
    ifc.ifc_buf = (caddr_t)ibuf;

    if (ioctl(fd,
            SIOCGIFCONF,
            (char *)&ifc) < 0 || ifc.ifc_len < sizeof(struct ifreq))
    {
        sprintf(errbuf, "SIOCGIFCONF: %s", strerror(errno));
        close(fd);
        return (-1);
    }
    ifrp = ibuf;
    ifend = (struct ifreq *)((char *)ibuf + ifc.ifc_len);

    al = ifaddrlist;
    mp = NULL;
    nipaddr = 0;
    for (; ifrp < ifend; ifrp = ifnext)
    {
#ifdef HAVE_SOCKADDR_SA_LEN
        n = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
        if (n < sizeof(*ifrp))
        {
            ifnext = ifrp + 1;
        }
        else
        {
            ifnext = (struct ifreq *)((char *)ifrp + n);
        }
        if (ifrp->ifr_addr.sa_family != AF_INET) continue;
#else
        ifnext = ifrp + 1;
#endif
        /*
         * Need a template to preserve address info that is
         * used below to locate the next entry.  (Otherwise,
         * SIOCGIFFLAGS stomps over it because the requests
         * are returned in a union.)
         */
        strncpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));
        if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) < 0)
        {
            if (errno == ENXIO) continue;
            sprintf(errbuf,
                    "SIOCGIFFLAGS: %.*s: %s",
                    (int)sizeof(ifr.ifr_name),
                     ifr.ifr_name,
                     strerror(errno));
            close(fd);
            return (-1);
        }

        /* Must be up and not the loopback */
        if ((ifr.ifr_flags & IFF_UP) == 0 || LIBNET_ISLOOPBACK(&ifr))
        {
            continue;
        }
        
        strncpy(device, ifr.ifr_name, sizeof(ifr.ifr_name));
        device[sizeof(device) - 1] = '\0';
        if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) < 0)
        {
            sprintf(errbuf, "SIOCGIFADDR: %s: %s", device, strerror(errno));
            close(fd);
            return (-1);
        }
    
        sin = (struct sockaddr_in *)&ifr.ifr_addr;
        al->addr = sin->sin_addr.s_addr;
        /*
         *  Replaced savestr() with strdup().  -- MDS
         */
        al->device = strdup(device);
        ++al;
        ++nipaddr;
    }
    close(fd);

    *ipaddrp = ifaddrlist;
    return (nipaddr);
}
#endif  /* !__solaris__ */

int
libnet_select_device(struct sockaddr_in *sin, u_char **device, u_char *errbuf)
{
    int c, i;
    char err_buf[BUFSIZ];
    struct libnet_ifaddr_list *address_list;

#if (__solaris__)
   /* 
    *  XXX - this is temporary and needs to be better documented.
    */
    *device = "le0";     
    return (1);
#else
    /*
     *  Number of interfaces.
     */
    c = libnet_ifaddrlist(&address_list, err_buf);
    if (c < 0)
    {
        sprintf(errbuf, "ifaddrlist : %s\n", err_buf);
        return (-1);
    }
    else if (c == 0)
    {
        sprintf(errbuf, "No network interfaces found.\n");
        return (-1);
    }
    if (*device)
    {
        for (i = c; i; --i, ++address_list)
        {
            if (!(strncmp(*device, address_list->device, strlen(address_list->device))))
            {
                break;
            }
        }
        if (i <= 0)
        {
            sprintf(errbuf, "Can't find interface %s\n", *device);
            return (-1);
        }
    }
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = address_list->addr;

    /*
     *  Do we need to assign a name to device?
     */
    if (!*device)
    {
        if (c > 1)
	{
#if (__DEBUG)
            fprintf(stdout,
                "Multiple interfaces found, using %s @ %s.\n",
                host_lookup(sin->sin_addr.s_addr, 0),
                address_list->device);
#endif
        }
        *device = address_list->device;
    }
    return (1);
}
#endif  /* __solaris__ */

/* EOF */
