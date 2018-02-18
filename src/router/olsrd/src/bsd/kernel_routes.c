/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 */

#include "kernel_routes.h"
#include "olsr.h"
#include "defs.h"
#include "process_routes.h"
#include "net_olsr.h"
#include "ipcalc.h"

#include <errno.h>
#include <unistd.h>
#include <net/if_dl.h>

#ifdef _WRS_KERNEL
#include <net/ifaddrs.h>
#include <wrn/coreip/net/route.h>
#include <m2Lib.h>
#define OLSR_PID taskIdSelf ()
#else /* _WRS_KERNEL */
#include <ifaddrs.h>
#define OLSR_PID getpid ()
#endif /* _WRS_KERNEL */

static unsigned int seq = 0;

/*
 * Sends an add or delete message via the routing socket.
 * The message consists of:
 *  - a header i.e. struct rt_msghdr
 *  - 0-8 socket address structures
 */
static int
add_del_route(const struct rt_entry *rt, int add)
{
  struct rt_msghdr *rtm;               /* message to send to the routing socket */
  unsigned char buff[512];
  unsigned char *walker;               /* points within the buffer */
  struct sockaddr_in sin4;             /* internet style sockaddr */
  struct sockaddr_dl *sdl;             /* link level sockaddr */
  struct ifaddrs *addrs;
  struct ifaddrs *awalker;
  const struct rt_nexthop *nexthop;
  union olsr_ip_addr mask;             /* netmask as ip address */
  int sin_size, sdl_size;              /* payload of the message */
  int len;                             /* message size written to routing socket */

  if (add) {
    OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));
  } else {
    OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));
  }

  memset(buff, 0, sizeof(buff));
  memset(&sin4, 0, sizeof(sin4));

  sin4.sin_len = sizeof(sin4);
  sin4.sin_family = AF_INET;

  sin_size = 1 + ((sizeof(struct sockaddr_in) - 1) | (sizeof(long) - 1));
  sdl_size = 1 + ((sizeof(struct sockaddr_dl) - 1) | (sizeof(long) - 1));

  /**********************************************************************
   *                  FILL THE ROUTING MESSAGE HEADER
   **********************************************************************/

  /* position header to the beginning of the buffer */
  rtm = (struct rt_msghdr *)buff;

  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = add ? RTM_ADD : RTM_DELETE;
  rtm->rtm_index = 0;           /* is ignored in outgoing messages */
  rtm->rtm_flags = olsr_rt_flags(rt, add);
  rtm->rtm_pid = OLSR_PID;
  rtm->rtm_seq = ++seq;

  /* walk to the end of the header */
  walker = buff + sizeof(struct rt_msghdr);

  /**********************************************************************
   *                  SET  DESTINATION OF THE ROUTE
   **********************************************************************/

#ifdef _WRS_KERNEL
  /*
   * vxWorks: change proto or tos
   */
  OLSR_PRINTF(8, "\t- Setting Protocol: 0\n");
  ((struct sockaddr_rt *)(&sin4))->srt_proto = 0;
  OLSR_PRINTF(8, "\t- Setting TOS: 0\n");
  ((struct sockaddr_rt *)(&sin4))->srt_tos = 0;
#endif /* _WRS_KERNEL */

  sin4.sin_addr = rt->rt_dst.prefix.v4;
  memcpy(walker, &sin4, sizeof(sin4));
  walker += sin_size;
  rtm->rtm_addrs = RTA_DST;

  /**********************************************************************
   *                  SET GATEWAY OF THE ROUTE
   **********************************************************************/

#ifdef _WRS_KERNEL
  /*
   * vxWorks: Route with no gateway is deleted
   */
  if (add) {
#endif /* _WRS_KERNEL */
    nexthop = olsr_get_nh(rt);
    if (0 != (rtm->rtm_flags & RTF_GATEWAY)) {
      sin4.sin_addr = nexthop->gateway.v4;
      memcpy(walker, &sin4, sizeof(sin4));
      walker += sin_size;
      rtm->rtm_addrs |= RTA_GATEWAY;
    }
    else {
      /*
       * Host is directly reachable, so add
       * the output interface MAC address.
       */
      if (getifaddrs(&addrs)) {
        fprintf(stderr, "\ngetifaddrs() failed\n");
        return -1;
      }

      for (awalker = addrs; awalker != NULL; awalker = awalker->ifa_next)
        if (awalker->ifa_addr->sa_family == AF_LINK && strcmp(awalker->ifa_name, if_ifwithindex_name(nexthop->iif_index)) == 0)
          break;

      if (awalker == NULL) {
        fprintf(stderr, "\nInterface %s not found\n", if_ifwithindex_name(nexthop->iif_index));
        freeifaddrs(addrs);
        return -1;
      }

      /* sdl is "struct sockaddr_dl" */
      sdl = (struct sockaddr_dl *)awalker->ifa_addr;
      memcpy(walker, sdl, sdl->sdl_len);
      walker += sdl_size;
      rtm->rtm_addrs |= RTA_GATEWAY;
#ifdef RTF_CLONING
      rtm->rtm_flags |= RTF_CLONING;
#endif /* RTF_CLONING */
#ifndef _WRS_KERNEL
      rtm->rtm_flags &= ~RTF_HOST;
#endif /* _WRS_KERNEL */
      freeifaddrs(addrs);
    }
#ifdef _WRS_KERNEL
  }
#endif /* _WRS_KERNEL */

  /**********************************************************************
   *                         SET  NETMASK
   **********************************************************************/

  if (0 == (rtm->rtm_flags & RTF_HOST)) {
    olsr_prefix_to_netmask(&mask, rt->rt_dst.prefix_len);
    sin4.sin_addr = mask.v4;
    memcpy(walker, &sin4, sizeof(sin4));
    walker += sin_size;
    rtm->rtm_addrs |= RTA_NETMASK;
  }

  /**********************************************************************
   *           WRITE CONFIGURATION MESSAGE TO THE ROUTING SOCKET
   **********************************************************************/

  rtm->rtm_msglen = (unsigned short)(walker - buff);
  len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
  if (len < 0 && !(errno == EEXIST || errno == ESRCH)) {
    fprintf(stderr, "cannot write to routing socket: %s\n", strerror(errno));
  }

  /*
   * If we get an EEXIST error while adding, delete and retry.
   */
  if (len < 0 && errno == EEXIST && rtm->rtm_type == RTM_ADD) {
    struct rt_msghdr *drtm;
    unsigned char dbuff[512];

    memset(dbuff, 0, sizeof(dbuff));
    drtm = (struct rt_msghdr *)dbuff;
    drtm->rtm_version = RTM_VERSION;
    drtm->rtm_type = RTM_DELETE;
    drtm->rtm_index = 0;
    drtm->rtm_flags = olsr_rt_flags(rt, add);
    drtm->rtm_seq = ++seq;

    walker = dbuff + sizeof(struct rt_msghdr);
    sin4.sin_addr = rt->rt_dst.prefix.v4;
    memcpy(walker, &sin4, sizeof(sin4));
    walker += sin_size;
    drtm->rtm_addrs = RTA_DST;
    drtm->rtm_msglen = (unsigned short)(walker - dbuff);
    len = write(olsr_cnf->rts, dbuff, drtm->rtm_msglen);
    if (len < 0) {
      fprintf(stderr, "cannot delete route: %s\n", strerror(errno));
    }
    rtm->rtm_seq = ++seq;
    len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
    if (len < 0) {
      fprintf(stderr, "still cannot add route: %s\n", strerror(errno));
    }
  }
  return 0;
}

int
olsr_ioctl_add_route(const struct rt_entry *rt)
{
  return add_del_route(rt, 1);
}

int
olsr_ioctl_del_route(const struct rt_entry *rt)
{
  return add_del_route(rt, 0);
}

static int
add_del_route6(const struct rt_entry *rt, int add)
{
  struct rt_msghdr *rtm;
  unsigned char buff[512];
  unsigned char *walker;
  struct sockaddr_in6 sin6;
  struct sockaddr_dl sdl;
  const struct rt_nexthop *nexthop;
  int sin_size, sdl_size;
  int len;

  if (add) {
    OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));
  } else {
    OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));
  }

  memset(buff, 0, sizeof(buff));
  memset(&sin6, 0, sizeof(sin6));
  memset(&sdl, 0, sizeof(sdl));

  sin6.sin6_len = sizeof(sin6);
  sin6.sin6_family = AF_INET6;
  sdl.sdl_len = sizeof(sdl);
  sdl.sdl_family = AF_LINK;

  sin_size = 1 + ((sizeof(struct sockaddr_in6) - 1) | (sizeof(long) - 1));
  sdl_size = 1 + ((sizeof(struct sockaddr_dl) - 1) | (sizeof(long) - 1));

  /**********************************************************************
   *                  FILL THE ROUTING MESSAGE HEADER
   **********************************************************************/

  /* position header to the beginning of the buffer */
  rtm = (struct rt_msghdr *)buff;
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = (add != 0) ? RTM_ADD : RTM_DELETE;
  rtm->rtm_index = 0;
  rtm->rtm_flags = olsr_rt_flags(rt, add);
  rtm->rtm_pid = OLSR_PID;
  rtm->rtm_seq = ++seq;

  /* walk to the end of the header */
  walker = buff + sizeof(struct rt_msghdr);

  /**********************************************************************
   *                  SET  DESTINATION OF THE ROUTE
   **********************************************************************/

  memcpy(&sin6.sin6_addr.s6_addr, &rt->rt_dst.prefix.v6, sizeof(struct in6_addr));
  memcpy(walker, &sin6, sizeof(sin6));
  walker += sin_size;
  rtm->rtm_addrs = RTA_DST;

  /**********************************************************************
   *                  SET GATEWAY OF THE ROUTE
   **********************************************************************/

  nexthop = olsr_get_nh(rt);
  if (0 != (rtm->rtm_flags & RTF_GATEWAY)) {
    memcpy(&sin6.sin6_addr.s6_addr, &nexthop->gateway.v6, sizeof(struct in6_addr));
    memset(&sin6.sin6_addr.s6_addr, 0, 8);
    sin6.sin6_addr.s6_addr[0] = 0xfe;
    sin6.sin6_addr.s6_addr[1] = 0x80;
    sin6.sin6_scope_id = nexthop->iif_index;
#ifdef __KAME__
    *(u_int16_t *) & sin6.sin6_addr.s6_addr[2] = htons(sin6.sin6_scope_id);
    sin6.sin6_scope_id = 0;
#endif /* __KAME__ */
    memcpy(walker, &sin6, sizeof(sin6));
    walker += sin_size;
    rtm->rtm_addrs |= RTA_GATEWAY;
  }
  else {
    /*
     * Host is directly reachable, so add
     * the output interface MAC address.
     */
    memcpy(&sin6.sin6_addr.s6_addr, &rt->rt_dst.prefix.v6, sizeof(struct in6_addr));
    memset(&sin6.sin6_addr.s6_addr, 0, 8);
    sin6.sin6_addr.s6_addr[0] = 0xfe;
    sin6.sin6_addr.s6_addr[1] = 0x80;
    sin6.sin6_scope_id = nexthop->iif_index;
#ifdef __KAME__
    *(u_int16_t *) & sin6.sin6_addr.s6_addr[2] = htons(sin6.sin6_scope_id);
    sin6.sin6_scope_id = 0;
#endif /* __KAME__ */
    memcpy(walker, &sin6, sizeof(sin6));
    walker += sin_size;
    rtm->rtm_addrs |= RTA_GATEWAY;
    rtm->rtm_flags |= RTF_GATEWAY;
  }

  /**********************************************************************
   *                         SET  NETMASK
   **********************************************************************/

  if (0 == (rtm->rtm_flags & RTF_HOST)) {
    olsr_prefix_to_netmask((union olsr_ip_addr *)&sin6.sin6_addr, rt->rt_dst.prefix_len);
    memcpy(walker, &sin6, sizeof(sin6));
    walker += sin_size;
    rtm->rtm_addrs |= RTA_NETMASK;
  }

  /**********************************************************************
   *           WRITE CONFIGURATION MESSAGE TO THE ROUTING SOCKET
   **********************************************************************/

  rtm->rtm_msglen = (unsigned short)(walker - buff);
  len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
  if (len < 0 && !(errno == EEXIST || errno == ESRCH)) {
    fprintf(stderr, "cannot write to routing socket: %s\n", strerror(errno));
  }

  /*
   * If we get an EEXIST error while adding, delete and retry.
   */
  if (len < 0 && errno == EEXIST && rtm->rtm_type == RTM_ADD) {
    struct rt_msghdr *drtm;
    unsigned char dbuff[512];

    memset(dbuff, 0, sizeof(dbuff));
    drtm = (struct rt_msghdr *)dbuff;
    drtm->rtm_version = RTM_VERSION;
    drtm->rtm_type = RTM_DELETE;
    drtm->rtm_index = 0;
    drtm->rtm_flags = olsr_rt_flags(rt, add);
    drtm->rtm_seq = ++seq;

    walker = dbuff + sizeof(struct rt_msghdr);
    memcpy(&sin6.sin6_addr.s6_addr, &rt->rt_dst.prefix.v6, sizeof(struct in6_addr));
    memcpy(walker, &sin6, sizeof(sin6));
    walker += sin_size;
    drtm->rtm_addrs = RTA_DST;
    drtm->rtm_msglen = (unsigned short)(walker - dbuff);
    len = write(olsr_cnf->rts, dbuff, drtm->rtm_msglen);
    if (len < 0) {
      fprintf(stderr, "cannot delete route: %s\n", strerror(errno));
    }
    rtm->rtm_seq = ++seq;
    len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
    if (len < 0) {
      fprintf(stderr, "still cannot add route: %s\n", strerror(errno));
    }
  }
  return 0;
}

int
olsr_ioctl_add_route6(const struct rt_entry *rt)
{
  return add_del_route6(rt, 1);
}

int
olsr_ioctl_del_route6(const struct rt_entry *rt)
{
  return add_del_route6(rt, 0);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
