
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
 */


#include "kernel_routes.h"
#include "olsr.h"
#include "defs.h"
#include "process_routes.h"
#include "net_olsr.h"
#include "ipcalc.h"

#include <net/if_dl.h>
#include <ifaddrs.h>

#ifdef _WRS_KERNEL
#include <wrn/coreip/net/route.h>
#include <m2Lib.h>
#define OLSR_PID taskIdSelf ()
#else
#define OLSR_PID getpid ()
#endif

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
  struct rt_msghdr *rtm;	       /* message to configure a route */
                                       /* contains data to be written to the
                                          routing socket */
  unsigned char buff[512];
  unsigned char *walker;	       /* points within the buffer */
  struct sockaddr_in sin;	       /* internet style sockaddr */
  struct sockaddr_dl *sdl;	       /* link level sockaddr */
  struct ifaddrs *addrs;
  struct ifaddrs *awalker;
  const struct rt_nexthop *nexthop;
  union olsr_ip_addr mask;	       /* netmask as ip address */
  int sin_size, sdl_size;              /* size of addresses - e.g. destination
                                          (payload of the message) */
  int len;                             /* message size written to routing socket */

  if (add) {
    OLSR_PRINTF(2, "KERN: Adding %s\n", olsr_rtp_to_string(rt->rt_best));
  } else {
    OLSR_PRINTF(2, "KERN: Deleting %s\n", olsr_rt_to_string(rt));
  }

  memset(buff, 0, sizeof(buff));
  memset(&sin, 0, sizeof(sin));

  sin.sin_len = sizeof(sin);
  sin.sin_family = AF_INET;

  sin_size = 1 + ((sizeof(struct sockaddr_in) - 1) | 3);
  sdl_size = 1 + ((sizeof(struct sockaddr_dl) - 1) | 3);

  /**********************************************************************
   *                  FILL THE ROUTING MESSAGE HEADER
   **********************************************************************/

  /* position header to the beginning of the buffer */
  rtm = (struct rt_msghdr *)buff;

  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = add ? RTM_ADD : RTM_DELETE;
  rtm->rtm_index = 0;		/* is ignored in outgoing messages */
  /* RTF_UP [and RTF_HOST and/or RTF_GATEWAY] */
  rtm->rtm_flags = olsr_rt_flags(rt);
  rtm->rtm_pid = OLSR_PID;
  rtm->rtm_seq = ++seq;

  /* walk to the end of the header */
  walker = buff + sizeof(struct rt_msghdr);

  /**********************************************************************
   *                  SET  DESTINATION OF THE ROUTE
   **********************************************************************/

  rtm->rtm_addrs = RTA_DST;	/* part of the header */

  sin.sin_addr = rt->rt_dst.prefix.v4;
  OLSR_PRINTF(8, "\t- Destination of the route: %s\n", inet_ntoa(sin.sin_addr));

  /* change proto or tos here */
#ifdef CODE_IS_FIXED_ON_FBSD
  OLSR_PRINTF(8, "\t- Setting Protocol: 0\n");
  ((struct sockaddr_rt *)(&sin))->srt_proto = 0;
  OLSR_PRINTF(8, "\t- Setting TOS: 0\n");
  ((struct sockaddr_rt *)(&sin))->srt_tos = 0;
#endif

  memcpy(walker, &sin, sizeof(sin));
  walker += sin_size;

  /**********************************************************************
   *                  SET GATEWAY OF THE ROUTE
   **********************************************************************/

  if (add || (rtm->rtm_addrs & RTF_GATEWAY)) {
    rtm->rtm_addrs |= RTA_GATEWAY;	/* part of the header */
    nexthop = olsr_get_nh(rt);

    if ((rtm->rtm_flags & RTF_GATEWAY)) {	/* GATEWAY */
      sin.sin_addr = nexthop->gateway.v4;

      memcpy(walker, &sin, sizeof(sin));
      walker += sin_size;

      OLSR_PRINTF(8, "\t- Gateway of the route: %s\n", inet_ntoa(sin.sin_addr));
    }
    /* NO GATEWAY - destination is directly reachable */
    else {
      rtm->rtm_flags |= RTF_CLONING;	/* part of the header! */

      /*
       * Host is directly reachable, so add the output interface MAC address.
       */
      if (getifaddrs(&addrs)) {
	fprintf(stderr, "\ngetifaddrs() failed\n");
	return -1;
      }

      for (awalker = addrs; awalker != NULL; awalker = awalker->ifa_next)
	if (awalker->ifa_addr->sa_family == AF_LINK &&
	    strcmp(awalker->ifa_name,
		   if_ifwithindex_name(nexthop->iif_index)) == 0)
	  break;

      if (awalker == NULL) {
	fprintf(stderr, "\nInterface %s not found\n",
		if_ifwithindex_name(nexthop->iif_index));
	freeifaddrs(addrs);
	return -1;
      }

      /* sdl is "struct sockaddr_dl" */
      sdl = (struct sockaddr_dl *)awalker->ifa_addr;
#ifdef DEBUG
      OLSR_PRINTF(8,"\t- Link layer address of the non gateway route: %s\n",
                  LLADDR(sdl));
#endif

      memcpy(walker, sdl, sdl->sdl_len);
      walker += sdl_size;

      freeifaddrs(addrs);
    }
  } else {
    /* Route with no gateway is deleted */
  }

  /**********************************************************************
   *                         SET  NETMASK
   **********************************************************************/

  if ((rtm->rtm_flags & RTF_HOST)) {
    OLSR_PRINTF(8, "\t- No netmask needed for a host route.\n");
  } else {			/* NO! hoste route */

    rtm->rtm_addrs |= RTA_NETMASK; /* part of the header */

    if (!olsr_prefix_to_netmask(&mask, rt->rt_dst.prefix_len)) {
      return -1;
    }
    sin.sin_addr = mask.v4;

    memcpy(walker, &sin, sizeof(sin));
    walker += sin_size;

    OLSR_PRINTF(8, "\t- Netmask of the route: %s\n", inet_ntoa(sin.sin_addr));
  }

  /**********************************************************************
   *           WRITE CONFIGURATION MESSAGE TO THE ROUTING SOCKET
   **********************************************************************/

  rtm->rtm_msglen = (unsigned short)(walker - buff);

  len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
  OLSR_PRINTF(8, "\nWrote %d bytes to rts socket (FD=%d)\n", len,
	      olsr_cnf->rts);

  if (0 != rtm->rtm_errno || len < rtm->rtm_msglen) {
    fprintf(stderr,
	    "\nCannot write to routing socket: (rtm_errno= 0x%x) (last error message: %s)\n",
	    rtm->rtm_errno, strerror(errno));
  }

  OLSR_PRINTF(8,
	      "\nWriting the following information to routing socket (message header):"
	      "\n\trtm_msglen: %u" "\n\trtm_version: %u" "\n\trtm_type: %u"
	      "\n\trtm_index: %u" "\n\trtm_flags: 0x%x" "\n\trtm_addrs: %u"
	      "\n\trtm_pid: 0x%x" "\n\trtm_seq: %u" "\n\trtm_errno: 0x%x"
	      "\n\trtm_use %u" "\n\trtm_inits: %u\n",
	      (unsigned int)rtm->rtm_msglen, (unsigned int)rtm->rtm_version,
	      (unsigned int)rtm->rtm_type, (unsigned int)rtm->rtm_index,
	      (unsigned int)rtm->rtm_flags, (unsigned int)rtm->rtm_addrs,
	      (unsigned int)rtm->rtm_pid, (unsigned int)rtm->rtm_seq,
	      (unsigned int)rtm->rtm_errno, (unsigned int)rtm->rtm_use,
	      (unsigned int)rtm->rtm_inits);

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
  int step, step_dl;
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

  step = 1 + ((sizeof(struct sockaddr_in6) - 1) | 3);
  step_dl = 1 + ((sizeof(struct sockaddr_dl) - 1) | 3);

  rtm = (struct rt_msghdr *)buff;
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_type = (add != 0) ? RTM_ADD : RTM_DELETE;
  rtm->rtm_index = 0;
  rtm->rtm_flags = olsr_rt_flags(rt);
  rtm->rtm_addrs = RTA_DST | RTA_GATEWAY;
  rtm->rtm_seq = ++seq;

  walker = buff + sizeof(struct rt_msghdr);

  memcpy(&sin6.sin6_addr.s6_addr, &rt->rt_dst.prefix.v6,
	 sizeof(struct in6_addr));

  memcpy(walker, &sin6, sizeof(sin6));
  walker += step;

  nexthop = olsr_get_nh(rt);
  if ((rtm->rtm_flags & RTF_GATEWAY) != 0) {
    memcpy(&sin6.sin6_addr.s6_addr, &nexthop->gateway.v6,
	   sizeof(struct in6_addr));

    memset(&sin6.sin6_addr.s6_addr, 0, 8);
    sin6.sin6_addr.s6_addr[0] = 0xfe;
    sin6.sin6_addr.s6_addr[1] = 0x80;
    sin6.sin6_scope_id = nexthop->iif_index;
#ifdef __KAME__
    *(u_int16_t *) & sin6.sin6_addr.s6_addr[2] = htons(sin6.sin6_scope_id);
    sin6.sin6_scope_id = 0;
#endif
    memcpy(walker, &sin6, sizeof(sin6));
    walker += step;
  }

  /* the host is directly reachable, so add the output interface's address */

  else {
    memcpy(&sin6.sin6_addr.s6_addr, &rt->rt_dst.prefix.v6,
	   sizeof(struct in6_addr));
    memset(&sin6.sin6_addr.s6_addr, 0, 8);
    sin6.sin6_addr.s6_addr[0] = 0xfe;
    sin6.sin6_addr.s6_addr[1] = 0x80;
    sin6.sin6_scope_id = nexthop->iif_index;
#ifdef __KAME__
    *(u_int16_t *) & sin6.sin6_addr.s6_addr[2] = htons(sin6.sin6_scope_id);
    sin6.sin6_scope_id = 0;
#endif

    memcpy(walker, &sin6, sizeof(sin6));
    walker += step;
    rtm->rtm_flags |= RTF_GATEWAY;
  }

  if ((rtm->rtm_flags & RTF_HOST) == 0) {
    olsr_prefix_to_netmask((union olsr_ip_addr *)&sin6.sin6_addr,
			   rt->rt_dst.prefix_len);
    memcpy(walker, &sin6, sizeof(sin6));
    walker += step;
    rtm->rtm_addrs |= RTA_NETMASK;
  }

  rtm->rtm_msglen = (unsigned short)(walker - buff);

  len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
  if (len < 0 && !(errno == EEXIST || errno == ESRCH))
    fprintf(stderr, "cannot write to routing socket: %s\n", strerror(errno));

  /* If we get an EEXIST error while adding, delete and retry. */
  if (len < 0 && errno == EEXIST && rtm->rtm_type == RTM_ADD) {
    struct rt_msghdr *drtm;
    unsigned char dbuff[512];

    memset(dbuff, 0, sizeof(dbuff));
    drtm = (struct rt_msghdr *)dbuff;
    drtm->rtm_version = RTM_VERSION;
    drtm->rtm_type = RTM_DELETE;
    drtm->rtm_addrs = RTA_DST;
    drtm->rtm_index = 0;
    drtm->rtm_flags = olsr_rt_flags(rt);
    drtm->rtm_seq = ++seq;

    walker = dbuff + sizeof(struct rt_msghdr);
    memcpy(&sin6.sin6_addr.s6_addr, &rt->rt_dst.prefix.v6,
	   sizeof(struct in6_addr));
    memcpy(walker, &sin6, sizeof(sin6));
    walker += step;
    drtm->rtm_msglen = (unsigned short)(walker - dbuff);
    len = write(olsr_cnf->rts, dbuff, drtm->rtm_msglen);
    if (len < 0)
      fprintf(stderr, "cannot delete route: %s\n", strerror(errno));
    rtm->rtm_seq = ++seq;
    len = write(olsr_cnf->rts, buff, rtm->rtm_msglen);
    if (len < 0)
      fprintf(stderr, "still cannot add route: %s\n", strerror(errno));
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
 * End:
 */
