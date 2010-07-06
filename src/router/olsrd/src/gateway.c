/*
 * gateway.c
 *
 *  Created on: 05.01.2010
 *      Author: henning
 */

#include "common/avl.h"
#include "defs.h"
#include "ipcalc.h"
#include "olsr.h"
#include "olsr_cfg.h"
#include "olsr_cookie.h"
#include "scheduler.h"
#include "kernel_routes.h"
#include "kernel_tunnel.h"
#include "net_os.h"
#include "duplicate_set.h"
#include "log.h"
#include "gateway_default_handler.h"
#include "gateway.h"

#include <assert.h>
#include <net/if.h>

#ifdef LINUX_NETLINK_ROUTING
struct avl_tree gateway_tree;

static struct olsr_cookie_info *gw_mem_cookie = NULL;
static uint8_t smart_gateway_netmask[sizeof(union olsr_ip_addr)];
static struct gateway_entry *current_ipv4_gw, *current_ipv6_gw;
static struct olsr_gw_handler *gw_handler;

static struct olsr_iptunnel_entry *v4gw_tunnel, *v6gw_tunnel;
static bool v4gw_choosen_external, v6gw_choosen_external;

/**
 * Reconstructs an uplink/downlink speed value from the encoded
 * 1 byte transport value (3 bit mantissa, 5 bit exponent)
 * @param value
 * @return
 */
static uint32_t
deserialize_gw_speed(uint8_t value) {
  uint32_t speed, exp;

  speed = (value >> 3)+1;
  exp = value & 7;
  while (exp-- > 0) {
    speed *= 10;
  }
  return speed;
}

/**
 * Convert an uplink/downlink speed into an exponential encoded
 * transport value (1 byte, 3 bit mantissa and 5 bit exponent)
 * @param value uplink/downlink speed in kbit/s
 * @return encoded byte value
 */
static uint8_t
serialize_gw_speed(uint32_t speed) {
  uint8_t exp = 0;

  if (speed == 0 || speed > 320000000) {
    return 0;
  }

  while (speed > 32 || (speed % 10) == 0) {
    speed /= 10;
    exp ++;
  }
  return ((speed-1) << 3) | exp;
}

/**
 * Callback for tunnel interface monitoring which will set the route into the tunnel
 * when the interface comes up again.
 * @param if_index
 * @param ifh
 * @param flag
 */
static void smartgw_tunnel_monitor (int if_index,
    struct interface *ifh __attribute__ ((unused)), enum olsr_ifchg_flag flag) {
  if (current_ipv4_gw != NULL && if_index == v4gw_tunnel->if_index && flag == IFCHG_IF_ADD) {
    /* v4 tunnel up again, set route */
    olsr_os_inetgw_tunnel_route(v4gw_tunnel->if_index, true, true);

    /* and ip */
    olsr_os_ifip(v4gw_tunnel->if_index, &olsr_cnf->main_addr, true);
  }
  if (current_ipv6_gw != NULL && if_index == v6gw_tunnel->if_index && flag == IFCHG_IF_ADD) {
    /* v6 status changed, set route */
    olsr_os_inetgw_tunnel_route(v6gw_tunnel->if_index, false, true);

    /* and ip */
    olsr_os_ifip(v6gw_tunnel->if_index, &olsr_cnf->main_addr, true);
  }
}

/**
 * Initialize gateway system
 */
int
olsr_init_gateways(void) {
  gw_mem_cookie = olsr_alloc_cookie("Gateway cookie", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(gw_mem_cookie, sizeof(struct gateway_entry));

  avl_init(&gateway_tree, avl_comp_default);
  current_ipv4_gw = NULL;
  current_ipv6_gw = NULL;

  v4gw_tunnel = NULL;
  v6gw_tunnel = NULL;

  refresh_smartgw_netmask();

  if (olsr_os_init_iptunnel()) {
    return 1;
  }

  olsr_add_ifchange_handler(smartgw_tunnel_monitor);

  /*
   * initialize default gateway handler,
   * can be overwritten with olsr_set_inetgw_handler
   */
  olsr_gw_default_init();
  return 0;
}

void refresh_smartgw_netmask(void) {
  uint8_t *ip;
  memset(&smart_gateway_netmask, 0, sizeof(smart_gateway_netmask));

  if (olsr_cnf->smart_gw_active) {
    union olsr_ip_addr gw_net;
    memset(&gw_net, 0, sizeof(gw_net));

    ip = (uint8_t *) &smart_gateway_netmask;

    if (olsr_cnf->smart_gw_uplink > 0 || olsr_cnf->smart_gw_downlink > 0) {
      ip[GW_HNA_FLAGS] |= GW_HNA_FLAG_LINKSPEED;
      ip[GW_HNA_DOWNLINK] = serialize_gw_speed(olsr_cnf->smart_gw_downlink);
      ip[GW_HNA_UPLINK] = serialize_gw_speed(olsr_cnf->smart_gw_uplink);
    }
    if (olsr_cnf->ip_version == AF_INET6 && olsr_cnf->smart_gw_prefix.prefix_len > 0) {
      ip[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV6PREFIX;
      ip[GW_HNA_V6PREFIXLEN] = olsr_cnf->smart_gw_prefix.prefix_len;
      memcpy(&ip[GW_HNA_V6PREFIX], &olsr_cnf->smart_gw_prefix.prefix, 8);
    }
  }
}

/**
 * Cleanup gateway tunnel system
 */
void olsr_cleanup_gateways(void) {
  if (current_ipv4_gw) {
    olsr_os_del_ipip_tunnel(v4gw_tunnel);
  }
  if (current_ipv6_gw) {
    olsr_os_del_ipip_tunnel(v6gw_tunnel);
  }

  olsr_remove_ifchange_handler(smartgw_tunnel_monitor);
  olsr_os_cleanup_iptunnel();
}

/**
 * Triggers the first lookup of an gateway. This lookup might
 * take some time to wait for routing/gateway information.
 */
void
olsr_trigger_inetgw_startup(void) {
  gw_handler->handle_startup();
}

/**
 * Triggers an instant gateway selection based on the current data
 * @param ipv4 trigger a ipv4 gateway lookup
 * @param ipv6 trigger a ipv6 gateway lookup
 * @return 0 if successful, -1 otherwise
 */
int
olsr_trigger_inetgw_selection(bool ipv4, bool ipv6) {
  gw_handler->select_gateway(ipv4, ipv6);
  return ((ipv4 && current_ipv4_gw == NULL) || (ipv6 && current_ipv6_gw == NULL)) ? -1 : 0;
}

/**
 * Triggers a check if the one of the gateways have been lost
 * through ETX = infinity
 */
void olsr_trigger_gatewayloss_check(void) {
  struct tc_entry *tc;
  bool ipv4 = false, ipv6 = false;
  if (current_ipv4_gw) {
    tc = olsr_lookup_tc_entry(&current_ipv4_gw->originator);
    if (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN) {
      ipv4 = true;
    }
  }
  if (current_ipv6_gw) {
    tc = olsr_lookup_tc_entry(&current_ipv6_gw->originator);
    if (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN) {
      ipv6 = true;
    }
  }
  if (ipv4 || ipv6) {
    olsr_trigger_inetgw_selection(ipv4, ipv6);
  }
}
/**
 * Set a new gateway handler. Do only call this once during startup from
 * a plugin to overwrite the default handler.
 * @param h pointer to gateway handler struct
 */
void
olsr_set_inetgw_handler(struct olsr_gw_handler *h) {
  gw_handler = h;
}

/**
 * Sets a new internet gateway.
 * An external set command might trigger an internal one if
 * address is not a legal gateway.
 *
 * @param originator ip address of the node with the new gateway
 * @param ipv4 set ipv4 gateway
 * @param ipv6 set ipv6 gateway
 * @param external true if change was triggered directly by an user,
 *   false if triggered by automatic lookup.
 * @return true if an error happened, false otherwise
 */
bool
olsr_set_inet_gateway(union olsr_ip_addr *originator, bool ipv4, bool ipv6, bool external) {
  struct gateway_entry *entry, *oldV4, *oldV6;
  struct olsr_iptunnel_entry *tunnelV4, *tunnelV6;

  oldV4 = current_ipv4_gw;
  oldV6 = current_ipv6_gw;
  tunnelV4 = v4gw_tunnel;
  tunnelV6 = v6gw_tunnel;

  ipv4 = ipv4 && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit);
  ipv6 = ipv6 && (olsr_cnf->ip_version == AF_INET6);

  if (ipv4) {
    current_ipv4_gw = NULL;
  }
  if (ipv6) {
    current_ipv6_gw = NULL;
  }

  entry = olsr_find_gateway_entry(originator);
  if (entry != NULL) {
    if (ipv4 && entry != current_ipv4_gw && entry->ipv4
        && (!entry->ipv4nat || olsr_cnf->smart_gw_allow_nat)) {
      /* valid ipv4 gateway */
      current_ipv4_gw = entry;
    }
    if (ipv6 && entry != current_ipv6_gw && entry->ipv6) {
      /* valid ipv6 gateway */
      current_ipv6_gw = entry;
    }
  }

  /* handle IPv4 */
  if (oldV4 != current_ipv4_gw) {
    if ((v4gw_tunnel = olsr_os_add_ipip_tunnel(&current_ipv4_gw->originator, true)) != NULL) {
      olsr_os_inetgw_tunnel_route(v4gw_tunnel->if_index, true, true);
      v4gw_choosen_external = external;
    }
    else {
      // TODO: what to do now ? Choose another one ? Fire up a timer ?
      current_ipv4_gw = NULL;
    }
    if (oldV4 != NULL) {
      olsr_os_del_ipip_tunnel(tunnelV4);
    }
  }
  /* handle IPv6 */
  if (oldV6 != current_ipv6_gw) {
    if ((v6gw_tunnel = olsr_os_add_ipip_tunnel(&current_ipv6_gw->originator, false)) != NULL) {
      olsr_os_inetgw_tunnel_route(v6gw_tunnel->if_index, false, true);
      v6gw_choosen_external = external;
    }
    else {
      // TODO: what to do now ? Choose another one ? Fire up a timer ?
      current_ipv6_gw = NULL;
    }
    if (oldV6 != NULL) {
      olsr_os_del_ipip_tunnel(tunnelV6);
    }
  }
  return (ipv4 && current_ipv4_gw == NULL) || (ipv6 && current_ipv6_gw == NULL);
}

/**
 * returns the gateway_entry of the current ipv4 internet gw.
 * @return pointer to gateway_entry or NULL if not set
 */
struct gateway_entry *olsr_get_ipv4_inet_gateway(bool *ext) {
  if (ext) {
    *ext = v4gw_choosen_external;
  }
  return current_ipv4_gw;
}

/**
 * returns the gateway_entry of the current ipv6 internet gw.
 * @return pointer to gateway_entry or NULL if not set
 */
struct gateway_entry *olsr_get_ipv6_inet_gateway(bool *ext) {
  if (ext) {
    *ext = v6gw_choosen_external;
  }
  return current_ipv6_gw;
}

/**
 * @param originator
 * @return gateway_entry for corresponding router
 */
struct gateway_entry *
olsr_find_gateway_entry(union olsr_ip_addr *originator) {
  struct avl_node *node = avl_find(&gateway_tree, originator);

  return node == NULL ? NULL : node2gateway(node);
}

/**
 * update a gateway_entry based on data received from a HNA
 * @param originator ip of the source of the HNA
 * @param mask netmask of the HNA
 * @param prefixlen of the HNA
 */
void
olsr_update_gateway_entry(union olsr_ip_addr *originator, union olsr_ip_addr *mask, int prefixlen, uint16_t seqno) {
  struct gateway_entry *gw;
  uint8_t *ptr;

  ptr = ((uint8_t *)mask) + ((prefixlen+7)/8);

  gw = olsr_find_gateway_entry(originator);
  if (!gw) {
    struct ipaddr_str buf;
    gw = olsr_cookie_malloc(gw_mem_cookie);

    gw->originator = *originator;
    gw->node.key = &gw->originator;

    avl_insert(&gateway_tree, &gw->node, AVL_DUP_NO);
  }
  else if (olsr_seqno_diff(seqno, gw->seqno) <= 0) {
    /* ignore older HNAs */
    return;
  }

  /* keep new HNA seqno */
  gw->seqno = seqno;

  if ((ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_LINKSPEED) != 0) {
    gw->uplink = deserialize_gw_speed(ptr[GW_HNA_UPLINK]);
    gw->downlink = deserialize_gw_speed(ptr[GW_HNA_DOWNLINK]);
  }
  else {
    gw->uplink = 1;
    gw->downlink = 1;
  }

  gw->ipv4 = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV4) != 0;
  gw->ipv4nat = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV4_NAT) != 0;

  if (olsr_cnf->ip_version == AF_INET6) {
    gw->ipv6 = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV6) != 0;

    /* do not reset prefixlength for ::ffff:0:0 HNAs */
    if (prefixlen == ipv6_internet_route.prefix_len) {
      memset(&gw->external_prefix, 0, sizeof(gw->external_prefix));

      if ((ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV6PREFIX) != 0
          && memcmp(mask->v6.s6_addr, &ipv6_internet_route.prefix, olsr_cnf->ipsize) == 0) {
        /* this is the right prefix (2000::/3), so we can copy the prefix */
        gw->external_prefix.prefix_len = ptr[GW_HNA_V6PREFIXLEN];
        memcpy(&gw->external_prefix.prefix, &ptr[GW_HNA_V6PREFIX], 8);
      }
    }
  }

  /* stop cleanup timer if necessary */
  if (gw->cleanup_timer) {
    olsr_stop_timer(gw->cleanup_timer);
    gw->cleanup_timer = NULL;
  }

  /* call update handler */
  gw_handler->handle_update_gw(gw);
}

static void cleanup_gateway_handler(void *ptr) {
  struct gateway_entry *gw = ptr;

  if (gw->ipv4 || gw->ipv6) {
    return;
  }

  /* remove gateway entry */
  avl_delete(&gateway_tree, &gw->node);
  olsr_cookie_free(gw_mem_cookie, gw);
}

/**
 * Delete a gateway based on the originator IP and the prefixlength of a HNA.
 * Should only be called if prefix is a smart_gw prefix or if node is removed
 * from TC set.
 * @param originator
 * @param prefixlen
 */
void
olsr_delete_gateway_entry(union olsr_ip_addr *originator, uint8_t prefixlen) {
  struct gateway_entry *gw;
  bool change = false;
  gw = olsr_find_gateway_entry(originator);
  if (gw && (gw->cleanup_timer == NULL || gw->ipv4 || gw->ipv6)) {
    if (olsr_cnf->ip_version == AF_INET && prefixlen == 0) {
      change = gw->ipv4;
      gw->ipv4 = false;
      gw->ipv4nat = false;
    }
    else if (olsr_cnf->ip_version == AF_INET6 && prefixlen == ipv6_internet_route.prefix_len) {
      change = gw->ipv6;
      gw->ipv6 = false;
    }
    else if (olsr_cnf->ip_version == AF_INET6 && prefixlen == ipv6_mappedv4_route.prefix_len) {
      change = gw->ipv4;
      gw->ipv4 = false;
      gw->ipv4nat = false;
    }

    if (prefixlen == FORCE_DELETE_GW_ENTRY || !(gw->ipv4 || gw->ipv6)) {
      /* prevent this gateway from being choosen as the new gateway */
      gw->ipv4 = false;
      gw->ipv4nat = false;
      gw->ipv6 = false;

      /* handle gateway loss */
      gw_handler->handle_delete_gw(gw);

      /* cleanup gateway if necessary */
      if (current_ipv4_gw == gw) {
        olsr_os_inetgw_tunnel_route(v4gw_tunnel->if_index, true, false);
        olsr_os_del_ipip_tunnel(v4gw_tunnel);

        current_ipv4_gw = NULL;
        v4gw_tunnel = NULL;
      }
      if (current_ipv6_gw == gw) {
        olsr_os_inetgw_tunnel_route(v6gw_tunnel->if_index, false, false);
        olsr_os_del_ipip_tunnel(v6gw_tunnel);

        current_ipv6_gw = NULL;
        v6gw_tunnel = NULL;
      }

      /* remove gateway entry */
      olsr_set_timer(&gw->cleanup_timer, GW_CLEANUP_INTERVAL, 0, false, cleanup_gateway_handler, gw, NULL);
    }
    else if (change) {
      gw_handler->handle_update_gw(gw);
    }
  }
}

/**
 * Checks if a prefix/netmask combination is a smart gateway
 * @param prefix
 * @param mask
 * @return true if is a valid smart gateway HNA, false otherwise
 */
bool
olsr_is_smart_gateway(struct olsr_ip_prefix *prefix, union olsr_ip_addr *mask) {
  uint8_t *ptr;

  if (!is_prefix_inetgw(prefix)) {
    return false;
  }

  ptr = ((uint8_t *)mask) + ((prefix->prefix_len+7)/8);
  return ptr[GW_HNA_PAD] == 0 && ptr[GW_HNA_FLAGS] != 0;
}

/**
 * Apply the smart gateway modifications to an outgoing HNA
 * @param mask pointer to netmask of the HNA
 * @param prefixlen of the HNA
 */
void
olsr_modifiy_inetgw_netmask(union olsr_ip_addr *mask, int prefixlen) {
  uint8_t *ptr = ((uint8_t *)mask) + ((prefixlen+7)/8);

  memcpy(ptr, &smart_gateway_netmask, sizeof(smart_gateway_netmask) - prefixlen/8);
  if (olsr_cnf->has_ipv4_gateway) {
    ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV4;

    if (olsr_cnf->smart_gw_uplink_nat) {
      ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV4_NAT;
    }
  }
  if (olsr_cnf->has_ipv6_gateway) {
    ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV6;
  }
  if (!olsr_cnf->has_ipv6_gateway || prefixlen != ipv6_internet_route.prefix_len){
    ptr[GW_HNA_FLAGS] &= ~GW_HNA_FLAG_IPV6PREFIX;
  }
}

/**
 * Print debug information about gateway entries
 */
void
olsr_print_gateway_entries(void) {
#ifndef NODEBUG
  struct ipaddr_str buf;
  struct gateway_entry *gw;
  const int addrsize = olsr_cnf->ip_version == AF_INET ? 15 : 39;

  OLSR_PRINTF(0, "\n--- %s ---------------------------------------------------- GATEWAYS\n\n",
      olsr_wallclock_string());
  OLSR_PRINTF(0, "%-*s %-6s %-9s %-9s %s\n", addrsize, "IP address", "Type", "Uplink", "Downlink",
      olsr_cnf->ip_version == AF_INET ? "" : "External Prefix");

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    OLSR_PRINTF(0, "%-*s %s%c%s%c%c %-9u %-9u %s\n", addrsize, olsr_ip_to_string(&buf, &gw->originator),
        gw->ipv4nat ? "" : "   ",
        gw->ipv4 ? '4' : ' ',
        gw->ipv4nat ? "(N)" : "",
        (gw->ipv4 && gw->ipv6) ? ',' : ' ',
        gw->ipv6 ? '6' : ' ',
        gw->uplink, gw->downlink,
        gw->external_prefix.prefix_len == 0 ? "" : olsr_ip_prefix_to_string(&gw->external_prefix));
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)
#endif
}

#endif /* !WIN32 */
