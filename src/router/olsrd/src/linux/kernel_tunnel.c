/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
 * Copyright (c) 2007, Sven-Ola for the policy routing stuff
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

#include "kernel_tunnel.h"
#include "kernel_routes.h"
#include "log.h"
#include "olsr_types.h"
#include "net_os.h"
#include "olsr_cookie.h"
#include "ipcalc.h"

#include <assert.h>

//ipip includes
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#include <linux/ip6_tunnel.h>
#endif

//ifup includes
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>

static const char DEV_IPV4_TUNNEL[IFNAMSIZ] = TUNNEL_ENDPOINT_IF;
static const char DEV_IPV6_TUNNEL[IFNAMSIZ] = TUNNEL_ENDPOINT_IF6;

static bool store_iptunnel_state;
static struct olsr_cookie_info *tunnel_cookie;
static struct avl_tree tunnel_tree;

int olsr_os_init_iptunnel(void) {
  const char *dev = olsr_cnf->ip_version == AF_INET ? DEV_IPV4_TUNNEL : DEV_IPV6_TUNNEL;

  tunnel_cookie = olsr_alloc_cookie("iptunnel", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(tunnel_cookie, sizeof(struct olsr_iptunnel_entry));
  avl_init(&tunnel_tree, avl_comp_default);

  store_iptunnel_state = olsr_if_isup(dev);
  if (store_iptunnel_state) {
    return 0;
  }
  if (olsr_if_set_state(dev, true)) {
    return -1;
  }

  return olsr_os_ifip(if_nametoindex(dev), &olsr_cnf->main_addr, true);
}

void olsr_os_cleanup_iptunnel(void) {
  while (tunnel_tree.count > 0) {
    struct olsr_iptunnel_entry *t;

    /* kill tunnel */
    t = (struct olsr_iptunnel_entry *)avl_walk_first(&tunnel_tree);
    t->usage = 1;

    olsr_os_del_ipip_tunnel(t);
  }
  if (!store_iptunnel_state) {
    olsr_if_set_state(olsr_cnf->ip_version == AF_INET ? DEV_IPV4_TUNNEL : DEV_IPV6_TUNNEL, false);
  }

  olsr_free_cookie(tunnel_cookie);
}

/**
 * creates an ipip tunnel (for ipv4)
 * @param name interface name
 * @param target pointer to tunnel target IP, NULL if tunnel should be removed
 * @return 0 if an error happened,
 *   if_index for successful created tunnel, 1 for successful deleted tunnel
 */
static int os_ip4_tunnel(const char *name, in_addr_t *target)
{
  struct ifreq ifr;
  int err;
  struct ip_tunnel_parm p;

  /* only IPIP tunnel if OLSR runs with IPv4 */
  assert (olsr_cnf->ip_version == AF_INET);
  memset(&p, 0, sizeof(p));
  p.iph.version = 4;
  p.iph.ihl = 5;
  p.iph.ttl = 64;
  p.iph.protocol = IPPROTO_IPIP;
  if (target) {
    p.iph.daddr = *target;
  }
  strncpy(p.name, name, IFNAMSIZ);

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, target != NULL ? DEV_IPV4_TUNNEL : name, IFNAMSIZ);
  ifr.ifr_ifru.ifru_data = (void *) &p;

  if ((err = ioctl(olsr_cnf->ioctl_s, target != NULL ? SIOCADDTUNNEL : SIOCDELTUNNEL, &ifr))) {
    char buffer[INET6_ADDRSTRLEN];

    olsr_syslog(OLSR_LOG_ERR, "Cannot %s a tunnel %s to %s: %s (%d)\n",
        target != NULL ? "add" : "remove", name,
        target != NULL ? inet_ntop(olsr_cnf->ip_version, target, buffer, sizeof(buffer)) : "-",
        strerror(errno), errno);
    return 0;
  }
  return target != NULL ? if_nametoindex(name) : 1;
}

/**
 * creates an ipip tunnel (for ipv6)
 * @param name interface name
 * @param target pointer to tunnel target IP, NULL if tunnel should be removed
 * @return 0 if an error happened,
 *   if_index for successful created tunnel, 1 for successful deleted tunnel
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
static int os_ip6_tunnel(const char *name, struct in6_addr *target)
{
  struct ifreq ifr;
  int err;
  struct ip6_tnl_parm p;

  /* only IP6 tunnel if OLSR runs with IPv6 */
  assert (olsr_cnf->ip_version == AF_INET6);
  memset(&p, 0, sizeof(p));
  p.proto = 0; /* any protocol */
  if (target) {
    p.raddr = *target;
  }
  strncpy(p.name, name, IFNAMSIZ);

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, target != NULL ? DEV_IPV6_TUNNEL : name, IFNAMSIZ);
  ifr.ifr_ifru.ifru_data = (void *) &p;

  if ((err = ioctl(olsr_cnf->ioctl_s, target != NULL ? SIOCADDTUNNEL : SIOCDELTUNNEL, &ifr))) {
    char buffer[INET6_ADDRSTRLEN];

    olsr_syslog(OLSR_LOG_ERR, "Cannot %s a tunnel %s to %s: %s (%d)\n",
        target != NULL ? "add" : "remove", name,
        target != NULL ? inet_ntop(olsr_cnf->ip_version, target, buffer, sizeof(buffer)) : "-",
        strerror(errno), errno);
    return 0;
  }
  return target != NULL ? if_nametoindex(name) : 1;
}
#endif

/**
 * Dummy for generating an interface name for an olsr ipip tunnel
 * @param target IP destination of the tunnel
 * @param name pointer to output buffer (length IFNAMSIZ)
 */
static void generate_iptunnel_name(union olsr_ip_addr *target, char *name) {
  static char PREFIX[] = "tnl_";
  static uint32_t counter = 0;

  snprintf(name, IFNAMSIZ, "%s%08x", PREFIX,
      olsr_cnf->ip_version == AF_INET ? target->v4.s_addr : ++counter);
}

/**
 * demands an ipip tunnel to a certain target. If no tunnel exists it will be created
 * @param target ip address of the target
 * @param transportV4 true if IPv4 traffic is used, false for IPv6 traffic
 * @return NULL if an error happened, pointer to olsr_iptunnel_entry otherwise
 */
struct olsr_iptunnel_entry *olsr_os_add_ipip_tunnel(union olsr_ip_addr *target, bool transportV4 __attribute__ ((unused))) {
  struct olsr_iptunnel_entry *t;

  assert(olsr_cnf->ip_version == AF_INET6 || transportV4);

  t = (struct olsr_iptunnel_entry *)avl_find(&tunnel_tree, target);
  if (t == NULL) {
    char name[IFNAMSIZ];
    int if_idx;
    struct ipaddr_str buf;

    memset(name, 0, sizeof(name));
    generate_iptunnel_name(target, name);

    if (olsr_cnf->ip_version == AF_INET) {
      if_idx = os_ip4_tunnel(name, &target->v4.s_addr);
    }
    else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
      if_idx = os_ip6_tunnel(name, &target->v6);
#else
      if_idx = 0;
#endif
    }

    if (if_idx == 0) {
      // cannot create tunnel
      olsr_syslog(OLSR_LOG_ERR, "Cannot create tunnel %s\n", name);
      return NULL;
    }

    if (olsr_if_set_state(name, true)) {
      if (olsr_cnf->ip_version == AF_INET) {
        os_ip4_tunnel(name, NULL);
      }
      else {
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
        os_ip6_tunnel(name, NULL);
  #endif
      }
      return NULL;
    }

    /* set originator IP for tunnel */
    olsr_os_ifip(if_idx, &olsr_cnf->main_addr, true);

    t = olsr_cookie_malloc(tunnel_cookie);
    memcpy(&t->target, target, sizeof(*target));
    t->node.key = &t->target;

    strncpy(t->if_name, name, IFNAMSIZ);
    t->if_index = if_idx;

    avl_insert(&tunnel_tree, &t->node, AVL_DUP_NO);
  }

  t->usage++;
  return t;
}

/**
 * Release an olsr ipip tunnel. Tunnel will be deleted
 * if this was the last user
 * @param t pointer to olsr_iptunnel_entry
 */
static void internal_olsr_os_del_ipip_tunnel(struct olsr_iptunnel_entry *t, bool cleanup) {
  if (!cleanup) {
    if (t->usage == 0) {
      return;
    }
    t->usage--;

    if (t->usage > 0) {
      return;
    }
  }

  olsr_if_set_state(t->if_name, false);
  if (olsr_cnf->ip_version == AF_INET) {
    os_ip4_tunnel(t->if_name, NULL);
  }
  else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
    os_ip6_tunnel(t->if_name, NULL);
#endif
  }

  avl_delete(&tunnel_tree, &t->node);
  if (!cleanup) {
    olsr_cookie_free(tunnel_cookie, t);
  }
}

void olsr_os_del_ipip_tunnel(struct olsr_iptunnel_entry *t) {
  internal_olsr_os_del_ipip_tunnel(t, false);
}
