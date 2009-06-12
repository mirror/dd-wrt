//==========================================================================
//
//      src/ifaddrs.c
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

//
// Adapted from KAME getifaddrs.c, if_nametoindex.c, if_indextoname.c
//

/*	$KAME: getifaddrs.c,v 1.9 2001/08/20 02:31:20 itojun Exp $	*/
/*	$KAME: if_nametoindex.c,v 1.6 2000/11/24 08:18:54 itojun Exp $	*/
/*	$KAME: if_indextoname.c,v 1.7 2000/11/08 03:09:30 itojun Exp $	*/

/*
 * Copyright (c) 1995, 1999
 *	Berkeley Software Design, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY Berkeley Software Design, Inc. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Berkeley Software Design, Inc. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	BSDI getifaddrs.c,v 2.12 2000/02/23 14:51:59 dab Exp
 */

#include <cyg/infra/diag.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#undef _KERNEL
#undef __INSIDE_NET
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#ifndef CYGPKG_NET_OPENBSD_STACK
#include <net/if_var.h>
#endif
#include <errno.h>
#include <netinet/in.h>
#include <net/netdb.h>
#include <ifaddrs.h>
#include <netinet/in_var.h>


#if !defined(AF_LINK)
#define	SA_LEN(sa)	sizeof(struct sockaddr)
#endif

#if !defined(SA_LEN)
#define	SA_LEN(sa)	(sa)->sa_len
#endif

#define	SALIGN	(sizeof(long) - 1)
#define	SA_RLEN(sa)	((sa)->sa_len ? (((sa)->sa_len + SALIGN) & ~SALIGN) : (SALIGN + 1))

#ifndef	ALIGNBYTES
/*
 * On systems with a routing socket, ALIGNBYTES should match the value
 * that the kernel uses when building the messages.
 */
#define	ALIGNBYTES	XXX
#endif
#ifndef	ALIGN
#define	ALIGN(p)	(((u_long)(p) + ALIGNBYTES) &~ ALIGNBYTES)
#endif

int
getifaddrs(struct ifaddrs **pif)
{
    int icnt = 1;  // Interface count
    int dcnt = 0;  // Data [length] count
    int ncnt = 0;  // Length of interface names
    char *buf;
#define	IF_WORK_SPACE_SZ	1024
    int i, sock;
#ifdef CYGPKG_NET_INET6
    int sock6;
    struct in6_ifreq ifrq6;
#endif
    struct ifconf ifc;
    struct ifreq *ifr, *lifr;
    struct ifreq ifrq;
    char *data, *names;
    struct ifaddrs *ifa, *ift;

    buf = malloc(IF_WORK_SPACE_SZ);
    if (buf == NULL)
        return (-1);
    ifc.ifc_buf = buf;
    ifc.ifc_len = IF_WORK_SPACE_SZ;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        free(buf);
        return (-1);
    }
    i =  ioctl(sock, SIOCGIFCONF, (char *)&ifc);

    if (i < 0) {
        close(sock); 
        free(buf);
        return (-1);
    }

    ifr = ifc.ifc_req;
    lifr = (struct ifreq *)&ifc.ifc_buf[ifc.ifc_len];

    while (ifr < lifr) {
        struct sockaddr *sa;

        sa = &ifr->ifr_addr;
        ++icnt;
        dcnt += SA_RLEN(sa) * 3;  /* addr, mask, brdcst */
        ncnt += sizeof(ifr->ifr_name) + 1;
		
        if (SA_LEN(sa) < sizeof(*sa))
            ifr = (struct ifreq *)(((char *)sa) + sizeof(*sa));
        else
            ifr = (struct ifreq *)(((char *)sa) + SA_LEN(sa));
    }

    if (icnt + dcnt + ncnt == 1) {
        // Nothing found
        *pif = NULL;
        free(buf);
        close(sock);
        return (0);
    }
    data = malloc(sizeof(struct ifaddrs) * icnt + dcnt + ncnt);
    if (data == NULL) {
        free(buf);
        close(sock);
        return(-1);
    }

    ifa = (struct ifaddrs *)(void *)data;
    data += sizeof(struct ifaddrs) * icnt;
    names = data + dcnt;

    memset(ifa, 0, sizeof(struct ifaddrs) * icnt);
    ift = ifa;

    ifr = ifc.ifc_req;

#ifdef CYGPKG_NET_INET6
    if ((sock6 = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
      free(buf);
      free(data);
      close(sock);
      return (-1);
    }
#endif

    while (ifr < lifr) {
       struct sockaddr * sa;

        ift->ifa_name = names;
        names[sizeof(ifr->ifr_name)] = 0;
        strncpy(names, ifr->ifr_name, sizeof(ifr->ifr_name));
        while (*names++) ;

        ift->ifa_addr = (struct sockaddr *)data;
        sa = &ifr->ifr_addr;
        memcpy(data, sa, SA_LEN(sa));
        data += SA_RLEN(sa);

        if ((sa->sa_family == AF_INET) || (sa->sa_family == AF_INET6)) {
          struct sockaddr *sa_netmask = NULL;
          struct sockaddr *sa_broadcast = NULL;
          struct sockaddr *sa_dst = NULL;
          
          memset(&ifrq,0,sizeof(ifrq));
          strcpy(ifrq.ifr_name,ifr->ifr_name);
          ioctl( sock, SIOCGIFFLAGS, &ifrq );

          ift->ifa_flags = ifrq.ifr_flags;

          memcpy(&ifrq.ifr_addr, ift->ifa_addr,sizeof(struct sockaddr));
          if (sa->sa_family == AF_INET) {
            ioctl(sock, SIOCGIFNETMASK, &ifrq); 
            sa_netmask = &ifrq.ifr_addr;
          }
#ifdef CYGPKG_NET_INET6
          if (sa->sa_family == AF_INET6) {
            memset(&ifrq6,0,sizeof(ifrq));
            strcpy(ifrq6.ifr_name,ifr->ifr_name);
            memcpy(&ifrq6.ifr_addr, ift->ifa_addr,sizeof(struct sockaddr));
          
            ioctl(sock6, SIOCGIFNETMASK_IN6, &ifrq6);
            sa_netmask = (struct sockaddr *)&ifrq6.ifr_addr;
          }
#endif
          ift->ifa_netmask = (struct sockaddr *)data;
          memcpy(data, sa_netmask, SA_LEN(sa_netmask));
          data += SA_RLEN(sa_netmask);

          memcpy(&ifrq.ifr_addr, ift->ifa_addr,sizeof(struct sockaddr));
          if ((sa->sa_family == AF_INET) && (ift->ifa_flags & IFF_BROADCAST)) {
            if (ioctl(sock, SIOCGIFBRDADDR, &ifrq) == 0) {
              sa_broadcast = &ifrq.ifr_addr;
              ift->ifa_broadaddr = (struct sockaddr *)data;
              memcpy(data, sa_broadcast, SA_LEN(sa_broadcast));
              data += SA_RLEN(sa_broadcast);
            }
          }

          memcpy(&ifrq.ifr_addr, ift->ifa_addr,sizeof(struct sockaddr));
          if ((sa->sa_family == AF_INET) && 
              (ift->ifa_flags & IFF_POINTOPOINT)) {
            if (ioctl(sock, SIOCGIFDSTADDR, &ifrq) == 0) {
              sa_dst = &ifrq.ifr_addr;
              ift->ifa_dstaddr = (struct sockaddr *)data;
              memcpy(data, sa_dst, SA_LEN(sa_dst));
              data += SA_RLEN(sa_dst);
            }
          }
        }
        
        if (SA_LEN(sa) < sizeof(*sa))
            ifr = (struct ifreq *)(((char *)sa) + sizeof(*sa));
        else
            ifr = (struct ifreq *)(((char *)sa) + SA_LEN(sa));
        ift = (ift->ifa_next = ift + 1);
    }
    free(buf);

    if (--ift >= ifa) {
        ift->ifa_next = NULL;
        *pif = ifa;
    } else {
        *pif = NULL;
        free(ifa);
    }
#ifdef CYGPKG_NET_INET6
    close(sock6);
#endif
    close(sock);
    return (0);
}

void
freeifaddrs(struct ifaddrs *ifp)
{
    free(ifp);
}

void
_show_all_interfaces(void)
{
    struct ifaddrs *iflist, *ifp;
    char addr[64];
    int indx;

    if (getifaddrs(&iflist) < 0) {
        diag_printf("Can't get interface information!!\n");
        return;
    }
    ifp = iflist;
    while (ifp != (struct ifaddrs *)NULL) {
        if (ifp->ifa_addr->sa_family != AF_LINK) {
            getnameinfo (ifp->ifa_addr, ifp->ifa_addr->sa_len, addr, 
                         sizeof(addr), 0, 0, NI_NUMERICHOST);
            diag_printf("%p - %s - %s\n", ifp, ifp->ifa_name, addr);
        }
        ifp = ifp->ifa_next;
    }
    indx = if_nametoindex(iflist->ifa_name);
    diag_printf("indx(%s) = %d\n", iflist->ifa_name, indx);
    if (indx > 0) {
        if (if_indextoname(indx, addr)) {
            diag_printf("index(%s) = %d/%s\n", iflist->ifa_name, indx, addr);
        } else {
            diag_printf("index(%s) = %d: %s\n", iflist->ifa_name, indx, strerror(errno));
        }
    } else {
        diag_printf("index(%s): %s\n", iflist->ifa_name, strerror(errno));
    }
    freeifaddrs(iflist);
}

/*
 * From RFC 2553:
 *
 * 4.1 Name-to-Index
 *
 *
 *    The first function maps an interface name into its corresponding
 *    index.
 *
 *       #include <net/if.h>
 *
 *       unsigned int  if_nametoindex(const char *ifname);
 *
 *    If the specified interface name does not exist, the return value is
 *    0, and errno is set to ENXIO.  If there was a system error (such as
 *    running out of memory), the return value is 0 and errno is set to the
 *    proper value (e.g., ENOMEM).
 */

unsigned int
if_nametoindex(const char *ifname)
{
    struct ifaddrs *ifaddrs, *ifa;
    unsigned int ni;

    if (getifaddrs(&ifaddrs) < 0)
        return(0);

    ni = 0;

    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr &&
            ifa->ifa_addr->sa_family == AF_LINK &&
            strcmp(ifa->ifa_name, ifname) == 0) {
            ni = ((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index;
            break;
        }
    }

    freeifaddrs(ifaddrs);
    if (!ni)
        errno = ENXIO;
    return(ni);
}

/*
 * From RFC 2533:
 *
 * The second function maps an interface index into its corresponding
 * name.
 *
 *    #include <net/if.h>
 *
 *    char  *if_indextoname(unsigned int ifindex, char *ifname);
 *
 * The ifname argument must point to a buffer of at least IF_NAMESIZE
 * bytes into which the interface name corresponding to the specified
 * index is returned.  (IF_NAMESIZE is also defined in <net/if.h> and
 * its value includes a terminating null byte at the end of the
 * interface name.) This pointer is also the return value of the
 * function.  If there is no interface corresponding to the specified
 * index, NULL is returned, and errno is set to ENXIO, if there was a
 * system error (such as running out of memory), if_indextoname returns
 * NULL and errno would be set to the proper value (e.g., ENOMEM).
 */

char *
if_indextoname(unsigned int ifindex, char *ifname)
{
    struct ifaddrs *ifaddrs, *ifa;
    int error = 0;

    if (getifaddrs(&ifaddrs) < 0)
        return(NULL);	/* getifaddrs properly set errno */

    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr &&
            ifa->ifa_addr->sa_family == AF_LINK &&
            ifindex == ((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index)
            break;
    }

    if (ifa == NULL) {
        error = ENXIO;
        ifname = NULL;
    }
    else
        strncpy(ifname, ifa->ifa_name, IFNAMSIZ);

    freeifaddrs(ifaddrs);

    errno = error;
    return(ifname);
}
