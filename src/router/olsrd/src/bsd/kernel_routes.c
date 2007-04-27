/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: kernel_routes.c,v 1.9 2007/02/14 09:09:16 kattemat Exp $
 */


#include "kernel_routes.h"
#include "olsr.h"
#include "defs.h"

#include <net/if_dl.h>
#include <ifaddrs.h>

static unsigned int seq = 0;

static int add_del_route(struct rt_entry *dest, int add)
{
  struct rt_msghdr *rtm;
  unsigned char buff[512];
  unsigned char *walker;
  struct sockaddr_in sin;
  struct sockaddr_dl *sdl;
  struct ifaddrs *addrs;
  struct ifaddrs *awalker;
  int step, step2;
  int len;
  char Str1[16], Str2[16], Str3[16];
  int flags;

  inet_ntop(AF_INET, &dest->rt_dst.v4, Str1, 16);
  inet_ntop(AF_INET, &dest->rt_mask.v4, Str2, 16);
  inet_ntop(AF_INET, &dest->rt_router.v4, Str3, 16);

  OLSR_PRINTF(1, "%s IPv4 route to %s/%s via %s.\n",
    (add != 0) ? "Adding" : "Removing", Str1, Str2, Str3)

  memset(buff, 0, sizeof (buff));
  memset(&sin, 0, sizeof (sin));

  sin.sin_len = sizeof (sin);
  sin.sin_family = AF_INET;

  step = 1 + ((sizeof (struct sockaddr_in) - 1) | 3);
  step2 = 1 + ((sizeof (struct sockaddr_dl) - 1) | 3);

  rtm = (struct rt_msghdr *)buff;

  flags = dest->rt_flags;

  // the host is directly reachable, so use cloning and a /32 net
  // routing table entry

  if ((flags & RTF_GATEWAY) == 0)
  {
    flags |= RTF_CLONING;
    flags &= ~RTF_HOST;
  }

  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = (add != 0) ? RTM_ADD : RTM_DELETE;
  rtm->rtm_index = 0;
  rtm->rtm_flags = flags;
  rtm->rtm_addrs = RTA_DST | RTA_NETMASK | RTA_GATEWAY;
  rtm->rtm_seq = ++seq;

  walker = buff + sizeof (struct rt_msghdr);

  sin.sin_addr.s_addr = dest->rt_dst.v4;

  memcpy(walker, &sin, sizeof (sin));
  walker += step;

  if ((flags & RTF_GATEWAY) != 0)
  {
    sin.sin_addr.s_addr = dest->rt_router.v4;

    memcpy(walker, &sin, sizeof (sin));
    walker += step;
  }

  // the host is directly reachable, so add the output interface's
  // MAC address

  else
  {
    if (getifaddrs(&addrs))
    {
      fprintf(stderr, "getifaddrs() failed\n");
      return -1;
    }

    for (awalker = addrs; awalker != NULL; awalker = awalker->ifa_next)
      if (awalker->ifa_addr->sa_family == AF_LINK &&
          strcmp(awalker->ifa_name, dest->rt_if->int_name) == 0)
        break;

    if (awalker == NULL)
    {
      fprintf(stderr, "interface %s not found\n", dest->rt_if->int_name);
      freeifaddrs(addrs);
      return -1;
    }

    sdl = (struct sockaddr_dl *)awalker->ifa_addr;

    memcpy(walker, sdl, sdl->sdl_len);
    walker += step2;

    freeifaddrs(addrs);
  }

  sin.sin_addr.s_addr = dest->rt_mask.v4;

  memcpy(walker, &sin, sizeof (sin));
  walker += step;

  rtm->rtm_msglen = (unsigned short)(walker - buff);

  len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);

  if (len < rtm->rtm_msglen)
    fprintf(stderr, "cannot write to routing socket: %s\n", strerror(errno));

  return 0;
}

int olsr_ioctl_add_route(struct rt_entry *dest)
{
  return add_del_route(dest, 1);
}

int olsr_ioctl_del_route(struct rt_entry *dest)
{
  return add_del_route(dest, 0);
}

static int add_del_route6(struct rt_entry *dest, int add)
{
  struct rt_msghdr *rtm;
  unsigned char buff[512];
  unsigned char *walker;
  struct sockaddr_in6 sin6;
  struct sockaddr_dl sdl;
  int step, step_dl;
  int len;
  char Str1[40], Str2[40];

  inet_ntop(AF_INET6, &dest->rt_dst.v6, Str1, 40);
  inet_ntop(AF_INET6, &dest->rt_router.v6, Str2, 40);

  OLSR_PRINTF(1, "%s IPv6 route to %s/%d via %s.\n",
    (add != 0) ? "Adding" : "Removing", Str1, dest->rt_mask.v6, Str2)

  memset(buff, 0, sizeof (buff));
  memset(&sin6, 0, sizeof (sin6));
  memset(&sdl, 0, sizeof (sdl));

  sin6.sin6_len = sizeof (sin6);
  sin6.sin6_family = AF_INET6;
  sdl.sdl_len = sizeof (sdl);
  sdl.sdl_family = AF_LINK;

  step = 1 + ((sizeof (struct sockaddr_in6) - 1) | 3);
  step_dl = 1 + ((sizeof (struct sockaddr_dl) - 1) | 3);

  rtm = (struct rt_msghdr *)buff;
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = (add != 0) ? RTM_ADD : RTM_DELETE;
  rtm->rtm_index = 0;
  rtm->rtm_flags = dest->rt_flags;
  rtm->rtm_addrs = RTA_DST | RTA_GATEWAY;
  rtm->rtm_seq = ++seq;

  walker = buff + sizeof (struct rt_msghdr);

  memcpy(&sin6.sin6_addr.s6_addr, &dest->rt_dst.v6, sizeof(struct in6_addr));

  memcpy(walker, &sin6, sizeof (sin6));
  walker += step;

  if ((rtm->rtm_flags & RTF_GATEWAY) != 0)
  {
    memcpy(&sin6.sin6_addr.s6_addr, &dest->rt_router.v6, sizeof(struct in6_addr));

    memcpy(walker, &sin6, sizeof (sin6));
    walker += step;
  }

  // the host is directly reachable, so add the output interface's address

  else
  {
    memcpy(&sin6.sin6_addr.s6_addr, &dest->rt_if->int6_addr.sin6_addr.s6_addr,
      sizeof(struct in6_addr));

    memcpy(walker, &sin6, sizeof (sin6));
    walker += step;
    rtm->rtm_flags |= RTF_LLINFO;
  }

  if ((rtm->rtm_flags & RTF_HOST) == 0)
  {
    olsr_prefix_to_netmask((union olsr_ip_addr *)&sin6.sin6_addr, dest->rt_mask.v6);
    memcpy(walker, &sin6, sizeof (sin6));
    walker += step;
    rtm->rtm_addrs |= RTA_NETMASK;
  }

  if ((rtm->rtm_flags & RTF_GATEWAY) != 0)
  {
    strcpy(&sdl.sdl_data[0], dest->rt_if->int_name);
    sdl.sdl_nlen = (u_char)strlen(dest->rt_if->int_name);
    memcpy(walker, &sdl, sizeof (sdl));
    walker += step_dl;
    rtm->rtm_addrs |= RTA_IFP;
  }

  rtm->rtm_msglen = (unsigned short)(walker - buff);

  len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);

  if (len < rtm->rtm_msglen)
    fprintf(stderr, "cannot write to routing socket: %s\n", strerror(errno));

  return 0;
}

int olsr_ioctl_add_route6(struct rt_entry *dest)
{
  return add_del_route6(dest, 1);
}

int olsr_ioctl_del_route6(struct rt_entry *dest)
{
  return add_del_route6(dest, 0);
}
