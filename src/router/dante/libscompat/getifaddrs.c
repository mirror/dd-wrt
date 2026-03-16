/* $Id: getifaddrs.c,v 1.38.10.2 2020/11/11 16:11:57 karls Exp $ */

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#include "osdep.h"

/*
 * Copyright (c) 2012, 2013, 2019
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

#if PRERELEASE
#define IFCONF_STARTENT 4
#else
#define IFCONF_STARTENT 10
#endif /* PRERELEASE */
#define IFCONF_MAXENT 1000

struct ifawrap {
   struct ifaddrs *ifaddrs;
   struct ifaddrs *prev;
};

static struct sockaddr *
getifval(int s, int flag, struct ifreq *ifreq, struct sockaddr *addr,
         uint8_t addrlen);
/*
 * lookup and verify address via specified flag using ioctl().
 */

int
lifconf_add(struct ifawrap *ifawrap);
/*
 * add IPv6 addresses via SunOS lifconf interface.
 */


static int
ifaddrs_add(struct ifawrap *ifawrap,
            char *name, unsigned int flags,
            struct sockaddr *addr, struct sockaddr *netmask,
            struct sockaddr *dstaddr, struct sockaddr *data, size_t addrlen);
/*
 * add ifaddrs interface values to ifawrap structure.
 */


int
getifaddrs(struct ifaddrs **ifap)
{
   struct sockaddr_storage i_addr, i_netmask, i_broaddst;
   struct sockaddr *p_addr, *p_netmask, *p_broaddst;
   struct ifawrap ifawrap;
   struct ifconf ifconf;
   struct ifreq *ifreq;
   struct ifreq ifreq2;
   size_t addrlen;
   unsigned int flags;
   int addrskip;
   int prevlen;
   int badname;
   char *nbuf;
   char *buf;
   char *p;
   int cnt;
   int len;
   int i;
   int s;

   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      return -1;

   bzero(&ifconf, sizeof(ifconf));
   /* attempt to get required size (ifc_len set to zero) */
   if (ioctl(s, SIOCGIFCONF, &ifconf) != -1 && ifconf.ifc_len != 0) {
      len = ifconf.ifc_len;
      if ((buf = malloc(len)) == NULL) {
         close(s);
         return -1;
      }
      ifconf.ifc_len = len;
      ifconf.ifc_buf = buf;
      if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
         free(buf);
         close(s);
         return -1;
      }
   } else {
      buf = NULL;
      prevlen = 0;
      cnt = IFCONF_STARTENT;
      /* try with increasing size until sure all entries obtained */
      for (;;) {
         len = sizeof(struct ifreq) * cnt;
         if ((nbuf = realloc(buf, len)) == NULL) {
            free(buf);
            close(s);
            return -1;
         }
         buf = nbuf;

         ifconf.ifc_len = len;
         ifconf.ifc_buf = buf;

         if (ioctl(s, SIOCGIFCONF, &ifconf) == -1 && errno != EINVAL) {
            free(buf);
            close(s);
            return -1;
         }

         /* end upon having two ioctl() calls of same size */
         if (ifconf.ifc_len > 0 && ifconf.ifc_len == prevlen)
            break;

         prevlen = ifconf.ifc_len;
         cnt *= 2;

#if 0
         if (cnt >= IFCONF_MAXENT) {
            /* too many entries */
            free(buf);
            close(s);
            return -1;
         }
#endif
      }
   }

   ifawrap.ifaddrs = NULL;
   p = ifconf.ifc_buf;
   while (p < ifconf.ifc_buf + ifconf.ifc_len) {
      ifreq = (struct ifreq *)p;

      p_addr = p_netmask = p_broaddst = NULL;
      bzero(&i_addr, sizeof(i_addr));
      bzero(&i_netmask, sizeof(i_netmask));
      bzero(&i_broaddst, sizeof(i_broaddst));

      /* invalid name likely indication of a problem */
      badname = 0;
      if (ifreq->ifr_name[0] == '\0')
         badname = 1;
      else
         for (i = 0; (size_t)i < sizeof(ifreq->ifr_name); i++) {
            if (ifreq->ifr_name[i] == '\0')
               break;
            if (!isgraph(ifreq->ifr_name[i])) {
               badname = 1;
               break;
            }
         }
      if (badname) {
         free(buf);
         close(s);
         errno = EFAULT;
         return -1;
      }

#if HAVE_SOCKADDR_SA_LEN

      addrlen = MAX(sizeof(struct sockaddr), ifreq->ifr_addr.sa_len);

#else /*  !HAVE_SOCKADDR_SA_LEN */

      switch (ifreq->ifr_addr.sa_family) {
         case AF_INET6:
            addrlen = sizeof(struct sockaddr_in6);
            break;

#ifdef AF_LINK
         case AF_LINK:
            addrlen = sizeof(struct sockaddr_dl);
            break;
#endif /* AF_LINK */

         case AF_INET: /*FALLTHROUGH*/
         default:
            addrlen = sizeof(struct sockaddr);
            break;
      }
#endif /* !HAVE_SOCKADDR_SA_LEN */
      addrskip = ROUNDUP(addrlen, sizeof(uint32_t));

      p += MAX(sizeof(ifreq->ifr_name) + addrskip, sizeof(struct ifreq));
      /* skip everything but inet/inet6/af_link */
      switch (ifreq->ifr_addr.sa_family) {
      case AF_INET:/*FALLTHROUGH*/
      case AF_INET6:/*FALLTHROUGH*/
#ifdef AF_LINK
      case AF_LINK:
#endif /* AF_LINK */
         break;
      default:
         continue;
      }

      /* skip unless interface is up */
      ifreq2 = *ifreq;
      if (ioctl(s, SIOCGIFFLAGS, &ifreq2) == -1) {
         free(buf);
         close(s);
         return -1;
      }
      flags = ifreq2.ifr_flags;
      if ((flags & IFF_UP) == 0) {
         continue;
      }

      bzero(&i_addr, sizeof(i_addr));
      memcpy(&i_addr, &ifreq->ifr_addr,
             MIN(sizeof(ifreq->ifr_addr), sizeof(i_addr)));
      p_addr = (struct sockaddr *)&i_addr;

#ifdef SIOCGIFNETMASK
      p_netmask = getifval(s, SIOCGIFNETMASK, ifreq,
                           (struct sockaddr *)&i_netmask, addrlen);
#endif /* SIOCGIFNETMASK */

#ifdef SIOCGIFBRDADDR
      if (flags & IFF_BROADCAST)
         p_broaddst = getifval(s, SIOCGIFBRDADDR, ifreq,
                               (struct sockaddr *)&i_broaddst, addrlen);
#endif /* SIOCGIFBRDADDR */

#ifdef SIOCGIFDSTADDR
      if (p_broaddst == NULL && flags & IFF_POINTOPOINT)
         p_broaddst = getifval(s, SIOCGIFDSTADDR, ifreq,
                               (struct sockaddr *)&i_broaddst, addrlen);
#endif /* SIOCGIFDSTADDR */

      /*XXX data */
      if (ifaddrs_add(&ifawrap, ifreq->ifr_name, flags, p_addr, p_netmask,
                      p_broaddst, NULL, addrlen) == -1) {
         if (ifawrap.ifaddrs != NULL) {
            freeifaddrs(ifawrap.ifaddrs);
            free(buf);
            close(s);
            return -1;
         }
      }
   }

#ifdef SIOCGLIFCONF
   lifconf_add(&ifawrap);
#endif /* SIOCGLIFCONF */

   free(buf);
   close(s);
   *ifap = ifawrap.ifaddrs;
   return 0;
}

static struct sockaddr *
getifval(int s, int flag, struct ifreq *ifreq, struct sockaddr *addr,
         uint8_t addrlen)
{
   char hbuf[NI_MAXHOST];
   struct ifreq ifreq2;

   ifreq2 = *ifreq;
   if (ioctl(s, flag, &ifreq2) == -1)
      return NULL;

   /* family/len might not be correctly set, copy original value */
   ifreq2.ifr_addr.sa_family = ifreq->ifr_addr.sa_family;
#if HAVE_SOCKADDR_SA_LEN
   ifreq2.ifr_addr.sa_len = addrlen;
#endif /* HAVE_SOCKADDR_SA_LEN */

   /* XXX verify address correctness for now */
   if ((ifreq2.ifr_addr.sa_family == AF_INET
   ||  ifreq2.ifr_addr.sa_family == AF_INET6)
      && (getnameinfo((struct sockaddr *)&ifreq2.ifr_addr, addrlen,
                          hbuf, sizeof(hbuf),
                          NULL,
                          0,
                          NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
      return NULL;
   }

   memcpy(addr, &ifreq2.ifr_addr, addrlen);
   return addr;
}

static int
ifaddrs_add(struct ifawrap *ifawrap, char *name, unsigned int flags,
            struct sockaddr *addr, struct sockaddr *netmask,
            struct sockaddr *dstaddr, struct sockaddr *data, size_t addrlen)
{
   size_t nameoff, addroff, maskoff, dstoff;
#if 0
   size_t dataoff;
#endif
   struct ifaddrs *new;
   size_t addrskip;
   size_t namelen;
   size_t nsize;
   char *p;

   namelen = strlen(name) + 1;

   addrskip = ROUNDUP(addrlen, sizeof(uint32_t));

   nsize = 0;
   nsize += ROUNDUP(sizeof(struct ifaddrs), sizeof(uint32_t));
   nameoff = nsize;

   nsize += ROUNDUP(namelen, sizeof(uint32_t));

   nsize += ROUNDUP(sizeof(flags), sizeof(uint32_t));
   addroff = nsize;

   if (addr != NULL)
      nsize += addrskip;
   maskoff = nsize;

   if (netmask != NULL)
      nsize += addrskip;
   dstoff = nsize;

   if (dstaddr != NULL)
      nsize += addrskip;
#if 0
   dataoff = nsize;
#endif

   if (data != NULL) /*XXX*/
      nsize += addrskip; /*XXX*/

   if ((new = malloc(nsize)) == NULL)
      return -1; /* let caller free already allocated data */
   if (ifawrap->ifaddrs == NULL)
      ifawrap->ifaddrs = new;
   else
      ifawrap->prev->ifa_next = new;
   ifawrap->prev = new;

   new->ifa_next = NULL;

   p = (char *)new + nameoff;
   strncpy(p, name, namelen - 1);
   p[namelen - 1] = '\0';
   new->ifa_name = p;

   new->ifa_flags = flags;

   if (addr != NULL) {
      p = (char *)new + addroff;
      memcpy(p, addr, addrlen);
      new->ifa_addr = (struct sockaddr *)p;
   } else
      new->ifa_addr = NULL;

   if (netmask != NULL) {
      p = (char *)new + maskoff;
      memcpy(p, netmask, addrlen);
      new->ifa_netmask = (struct sockaddr *)p;
   } else
      new->ifa_netmask = NULL;

   if (dstaddr != NULL) {
      p = (char *)new + dstoff;
      memcpy(p, dstaddr, addrlen);
      new->ifa_dstaddr = (struct sockaddr *)p;
   } else
      new->ifa_dstaddr = NULL;

#if 0 /*XXX*/
   if (data != NULL) {
      p = (char *)new + dataoff;
      memcpy(p, data, addrlen);
      new->ifa_data = (struct sockaddr *)p;
   } else
#endif
      new->ifa_data = NULL;

   return 0;
}

void
freeifaddrs(struct ifaddrs *ifap)
{
   struct ifaddrs *c, *n;

   c = ifap;
   while (c != NULL) {
      n = c->ifa_next;
      free(c);
      c = n;
   }
}

#ifdef SIOCGLIFCONF

/* Solaris lif variant for ipv6 */

/* XXX can likely be simplified/merged better with above code */

static struct sockaddr *
getifval2(int s, int flag, struct lifreq *lifreq, struct sockaddr *addr,
         uint8_t addrlen);

int
lifconf_add(struct ifawrap *ifawrap)
{
   struct sockaddr_storage i_addr, i_netmask, i_broaddst;
   struct sockaddr *p_addr, *p_netmask, *p_broaddst;
   struct lifnum lifnum;
   struct lifconf lifconf;
   struct lifreq *lifreq;
   struct lifreq lifreq2;
   unsigned int flags;
   size_t addrlen;
   int addrskip;
   int badname;
   char *buf;
   char *p;
   int i;
   int s;

   /* need AF_INET6 or attempt to obtain netmask will fail (Solaris 10) */
   if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) == -1)
      return -1;

   bzero(&lifnum, sizeof(lifnum));
   lifnum.lifn_family = AF_UNSPEC;
   if ((i = ioctl(s, SIOCGLIFNUM, &lifnum)) == -1) {
      close(s);
      return -1;
   }

   if (lifnum.lifn_count == 0) {
      close(s);
      return -1;
   }

   bzero(&lifconf, sizeof(lifconf));
   lifconf.lifc_len = lifnum.lifn_count * sizeof(struct lifreq);

   /*XXX pad count? possible race as new interfaces might be added */

   if ((buf = malloc(lifconf.lifc_len)) == NULL) {
      close(s);
      return -1;
   }
   lifconf.lifc_buf = buf;

   if (ioctl(s, SIOCGLIFCONF, &lifconf) == -1) {
      free(buf);
      close(s);
      return -1;
   }

   p = lifconf.lifc_buf;
   while (p < lifconf.lifc_buf + lifconf.lifc_len) {
      lifreq = (struct lifreq *)p;

      p_addr = p_netmask = p_broaddst = NULL;
      bzero(&i_addr, sizeof(i_addr));
      bzero(&i_netmask, sizeof(i_netmask));
      bzero(&i_broaddst, sizeof(i_broaddst));

      /* invalid name likely indication of a problem */
      badname = 0;
      if (lifreq->lifr_name[0] == '\0')
         badname = 1;
      else
         for (i = 0; (size_t)i < sizeof(lifreq->lifr_name); i++) {
            if (lifreq->lifr_name[i] == '\0')
               break;
            if (!isgraph(lifreq->lifr_name[i])) {
               badname = 1;
               break;
            }
         }
      if (badname) {
         free(buf);
         close(s);
         errno = EFAULT;
         return -1;
      }

#if HAVE_SOCKADDR_SA_LEN

      addrlen = MAX(sizeof(struct sockaddr), lifreq->lifr_addr.sa_len);

#else /*  !HAVE_SOCKADDR_SA_LEN */

      switch (lifreq->lifr_addr.ss_family) {
         case AF_INET6:
            addrlen = sizeof(struct sockaddr_in6);
            break;

#ifdef AF_LINK
         case AF_LINK:
            addrlen = sizeof(struct sockaddr_dl);
            break;
#endif /* AF_LINK */

         case AF_INET: /*FALLTHROUGH*/
         default:
            addrlen = sizeof(struct sockaddr);
            break;
      }
#endif /* !HAVE_SOCKADDR_SA_LEN */
      addrskip = ROUNDUP(addrlen, sizeof(uint32_t));

      p += MAX(sizeof(lifreq->lifr_name) + addrskip, sizeof(struct lifreq));
      /* skip everything but inet6 */

      /* get IPv6 addresses only */
      switch (lifreq->lifr_addr.ss_family) {
      case AF_INET6:
         break;
      default:
         continue;
      }

      /* skip unless interface is up */
      lifreq2 = *lifreq;
      if (ioctl(s, SIOCGLIFFLAGS, &lifreq2) == -1) {
         free(buf);
         close(s);
         return -1;
      }
      flags = lifreq2.lifr_flags;
#if 0 /* XXX appears to skip aliases configured via ifconfig */
      if ((flags & IFF_UP) == 0) {
         continue;
      }
#endif

      bzero(&i_addr, sizeof(i_addr));
      memcpy(&i_addr, &lifreq->lifr_addr,
             MIN(sizeof(lifreq->lifr_addr), sizeof(i_addr)));
      p_addr = (struct sockaddr *)&i_addr;

#ifdef SIOCGLIFNETMASK
      p_netmask = getifval2(s, SIOCGLIFNETMASK, lifreq,
                           (struct sockaddr *)&i_netmask, addrlen);
#endif /* SIOCGLIFNETMASK */

#ifdef SIOCGLIFBRDADDR
      if (flags & IFF_BROADCAST)
         p_broaddst = getifval2(s, SIOCGLIFBRDADDR, lifreq,
                               (struct sockaddr *)&i_broaddst, addrlen);
#endif /* SIOCGLIFBRDADDR */

#ifdef SIOCGLIFDSTADDR
      if (p_broaddst == NULL && flags & IFF_POINTOPOINT)
         p_broaddst = getifval2(s, SIOCGLIFDSTADDR, lifreq,
                               (struct sockaddr *)&i_broaddst, addrlen);
#endif /* SIOCGLIFDSTADDR */

      /*XXX data */
      if (ifaddrs_add(ifawrap, lifreq->lifr_name, flags, p_addr, p_netmask,
                      p_broaddst, NULL, addrlen) == -1) {
         if (ifawrap->ifaddrs != NULL) {
            freeifaddrs(ifawrap->ifaddrs);
            free(buf);
            close(s);
            return -1;
         }
      }
   }

   free(buf);
   close(s);
   return 0;
}

/* XXX should ideally be merged with getifval() */
static struct sockaddr *
getifval2(int s, int flag, struct lifreq *lifreq, struct sockaddr *addr,
         uint8_t addrlen)
{
   char hbuf[NI_MAXHOST];
   struct lifreq lifreq2;

   lifreq2 = *lifreq;
   if (ioctl(s, flag, &lifreq2) == -1)
      return NULL;

   /* family/len might not be correctly set, copy original value */
//   lifreq2.lifr_addr.ss_family = lifreq->lifr_addr.ss_family;
#if HAVE_SOCKADDR_SA_LEN
   lifreq2.lifr_addr.ss_len = addrlen;
#endif /* HAVE_SOCKADDR_SA_LEN */

   /* XXX verify address correctness for now */
   if ((lifreq2.lifr_addr.ss_family == AF_INET
   ||  lifreq2.lifr_addr.ss_family == AF_INET6)
      && (getnameinfo((struct sockaddr *)&lifreq2.lifr_addr, addrlen,
                          hbuf, sizeof(hbuf),
                          NULL,
                          0,
                          NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
      return NULL;
   }

   memcpy(addr, &lifreq2.lifr_addr, addrlen);
   return addr;
}

#endif /* SIOCGLIFCONF */
