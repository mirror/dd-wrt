/*
    routing.c, manipulating routing table for PPTP Client
    Copyright (C) 2006  James Cameron <quozl@us.netrek.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA

*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "routing.h"
#include "config.h"

#if defined (__SVR4) && defined (__sun) /* Solaris */
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include "util.h"
/* PF_ROUTE socket*/
int rts; 
/* Destination and gateway addresses */
struct sockaddr_in rdst, rgw;
/* Request sequence */
int rseq;
int dorouting;
#else /* Solaris */
/* route to the server */
char *route;
#endif /* Solaris */

/*

Design discussion.

The primary task of this module is to add a host route to the PPTP
server so that the kernel continues to deliver PPTP control and data
connection packets to the server despite the new PPP interface that is
created.  The flag --nohostroute is to disable this.

A secondary task may be to implement all-to-tunnel routing if the
appropriate flag is specified on the command line.  The flag
--route-all is to implement this (not yet implemented).

Caveat.

It is not possible from the "ip route" command to determine if a host
route already exists, so it isn't practical to put the routing table
back exactly as it was.

We have a choice of either leaving our route lying around, or
destroying a route that the user had pre-arranged.  Both are
unfortunate.  The flag --remove-host-route is to remove the route
regardless (not yet implemented).

*/

void routing_init(char *ip) {
#if defined (__SVR4) && defined (__sun) /* Solaris */
  rdst.sin_family = AF_INET;
  if ( ! inet_pton(AF_INET, ip, &rdst.sin_addr) ) {
    log("Cannot convert address: %s", strerror(errno));
    return;
  }

  if ( (rts = socket(PF_ROUTE, SOCK_RAW, AF_INET )) < 0 ) {
    log("Cannot open routing socket: %s", strerror(errno));
    return;
  }

  struct rt_msg rtm = {
    .hdr.rtm_msglen = sizeof(struct rt_msg),
    .hdr.rtm_version = RTM_VERSION,
    .hdr.rtm_type = RTM_GET,
    .hdr.rtm_addrs = RTA_DST,
    .hdr.rtm_pid = getpid(),
    .hdr.rtm_seq = ++rseq,
    .addrs[RTAX_DST] = rdst
  };

  if ( write(rts, &rtm, rtm.hdr.rtm_msglen) != rtm.hdr.rtm_msglen ) {
    log("Error writing to routing socket: %s", strerror(errno));
    close(rts);
    return;
  }

  while ( read(rts, &rtm, sizeof(struct rt_msg)) > 0 )
    if ( rtm.hdr.rtm_pid == getpid() && rtm.hdr.rtm_seq == rseq) {
      /* Check if host route already present */
      if ( ( rtm.hdr.rtm_flags & RTF_HOST ) != RTF_HOST ) {
        rgw = rtm.addrs[RTAX_GATEWAY];
        dorouting = 1;
      }
      break;
    }
#endif /* Solaris */ 
#if defined(__linux)
  char buf[256];
  FILE *p;

  snprintf(buf, 255, "%s route get %s", IP_BINARY, ip);
  p = popen(buf, "r");
  fgets(buf, 255, p);
  /* TODO: check for failure of fgets */
  route = strdup(buf);
  pclose(p);
  /* TODO: check for failure of command */
#endif /* __linux__ */
}

void routing_start(void) {
#if defined (__SVR4) && defined (__sun) /* Solaris */
  if ( ! dorouting )
     return;

  struct rt_msg rtm = {
    .hdr.rtm_msglen = sizeof(struct rt_msg),
    .hdr.rtm_version = RTM_VERSION,
    .hdr.rtm_type = RTM_ADD,
    .hdr.rtm_flags = RTF_HOST | RTF_GATEWAY | RTF_STATIC,
    .hdr.rtm_addrs = RTA_DST | RTA_GATEWAY,
    .hdr.rtm_pid = getpid(),
    .hdr.rtm_seq = ++rseq,
    .addrs[RTAX_DST] = rdst,
    .addrs[RTAX_GATEWAY] = rgw
  };

  if ( write(rts, &rtm, rtm.hdr.rtm_msglen) != rtm.hdr.rtm_msglen ) {
    log("Error adding route: %s", strerror(errno));
  }
#endif
#if defined(__linux__)
  char buf[256];
  FILE *p;

  snprintf(buf, 255, "%s route replace %s", IP_BINARY, route);
  p = popen(buf, "r");
  pclose(p);
#endif /* __linux__ */
}

void routing_end(void) {
#if defined (__SVR4) && defined (__sun) /* Solaris */
  if ( ! dorouting)
    return;

  struct rt_msg rtm = {
    .hdr.rtm_msglen = sizeof(struct rt_msg),
    .hdr.rtm_version = RTM_VERSION,
    .hdr.rtm_type = RTM_DELETE,
    .hdr.rtm_flags = RTF_HOST | RTF_GATEWAY | RTF_STATIC,
    .hdr.rtm_addrs = RTA_DST | RTA_GATEWAY,
    .hdr.rtm_pid = getpid(),
    .hdr.rtm_seq = ++rseq,
    .addrs[RTAX_DST] = rdst,
    .addrs[RTAX_GATEWAY] = rgw
  };

  if ( write(rts, &rtm, rtm.hdr.rtm_msglen) != rtm.hdr.rtm_msglen ) {
    log("Error deleting route: %s", strerror(errno));
  }
#endif /* Solaris */
#if defined(__linux__)
  char buf[256];
  FILE *p;

  snprintf(buf, 255, "%s route delete %s", IP_BINARY, route);
  p = popen(buf, "r");
  pclose(p);
#endif /* __linux__ */
}
