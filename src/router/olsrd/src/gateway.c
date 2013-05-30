/*
 * gateway.c
 *
 *  Created on: 05.01.2010
 *      Author: henning
 */

#ifdef __linux__

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

/** the gateway tree */
struct avl_tree gateway_tree;

/** gateway cookie */
static struct olsr_cookie_info *gateway_entry_mem_cookie = NULL;

/** the gateway netmask for the HNA */
static uint8_t smart_gateway_netmask[sizeof(union olsr_ip_addr)];

/** the gateway handler/plugin */
static struct olsr_gw_handler *gw_handler;

/** the current IPv4 gateway */
static struct gateway_entry *current_ipv4_gw;

/** the tunnel of the current IPv4  gateway */
static struct olsr_iptunnel_entry *v4gw_tunnel;

/** the current IPv6 gateway */
static struct gateway_entry *current_ipv6_gw;

/** the tunnel of the current IPv6  gateway */
static struct olsr_iptunnel_entry *v6gw_tunnel;

/*
 * Forward Declarations
 */

static void olsr_delete_gateway_tree_entry(struct gateway_entry * gw, uint8_t prefixlen, bool immediate);

/*
 * Helper Functions
 */

#define TUNNEL_NAME (olsr_cnf->ip_version == AF_INET ? TUNNEL_ENDPOINT_IF : TUNNEL_ENDPOINT_IF6)

#define OLSR_IP_ADDR_2_HNA_PTR(mask, prefixlen) (((uint8_t *)mask) + ((prefixlen+7)/8))

/**
 * Convert an encoded 1 byte transport value (5 bits mantissa, 3 bits exponent)
 * to an uplink/downlink speed value
 *
 * @param value the encoded 1 byte transport value
 * @return the uplink/downlink speed value (in kbit/s)
 */
static uint32_t deserialize_gw_speed(uint8_t value) {
  uint32_t speed;
  uint32_t exp;

  if (!value) {
    /* 0 and 1 alias onto 0 during serialisation. We take 0 here to mean 0 and
     * not 1 (since a bandwidth of 1 is no bandwidth at all really) */
    return 0;
  }

  speed = (value >> 3) + 1;
  exp = value & 7;

  while (exp-- > 0) {
    speed *= 10;
  }
  return speed;
}

/**
 * Convert an uplink/downlink speed value into an encoded 1 byte transport
 * value (5 bits mantissa, 3 bits exponent)
 *
 * @param speed the uplink/downlink speed value (in kbit/s)
 * @return value the encoded 1 byte transport value
 */
static uint8_t serialize_gw_speed(uint32_t speed) {
  uint8_t exp = 0;

  if (speed == 0) {
    return 0;
  }

  if (speed > 320000000) {
    return 0xff;
  }

  while ((speed > 32 || (speed % 10) == 0) && exp < 7) {
    speed /= 10;
    exp++;
  }
  return ((speed - 1) << 3) | exp;
}

/*
 * Callback Functions
 */

/**
 * Callback for tunnel interface monitoring which will set the route into the tunnel
 * when the interface comes up again.
 *
 * @param if_index the interface index
 * @param ifh the interface
 * @param flag interface change flags
 */
static void smartgw_tunnel_monitor(int if_index __attribute__ ((unused)),
    struct interface *ifh __attribute__ ((unused)), enum olsr_ifchg_flag flag __attribute__ ((unused))) {
  return;
}

/**
 * Timer callback to remove and cleanup a gateway entry
 *
 * @param ptr
 */
static void cleanup_gateway_handler(void *ptr) {
  struct gateway_entry *gw = ptr;

  if (gw->ipv4 || gw->ipv6) {
    /* do not clean it up when it is in use */
    return;
  }

  /* remove gateway entry */
  avl_delete(&gateway_tree, &gw->node);
  olsr_cookie_free(gateway_entry_mem_cookie, gw);
}

/*
 * Main Interface
 */

/**
 * Initialize gateway system
 */
int olsr_init_gateways(void) {
  int retries = 5;

  gateway_entry_mem_cookie = olsr_alloc_cookie("gateway_entry_mem_cookie", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(gateway_entry_mem_cookie, sizeof(struct gateway_entry));

  avl_init(&gateway_tree, avl_comp_default);

  current_ipv4_gw = NULL;
  v4gw_tunnel = NULL;

  current_ipv6_gw = NULL;
  v6gw_tunnel = NULL;

  gw_handler = NULL;

  refresh_smartgw_netmask();

  /* initialize default gateway handler */
  gw_handler = &gw_def_handler;
  gw_handler->init();

  /*
   * There appears to be a kernel bug in some kernels (at least in the 3.0
   * Debian Squeeze kernel, but not in the Fedora 17 kernels) around
   * initialising the IPIP server tunnel (loading the IPIP module), so we retry
   * a few times before giving up
   */
  while (retries-- > 0) {
    if (!olsr_os_init_iptunnel(TUNNEL_NAME)) {
      retries = 5;
      break;
    }

    olsr_printf(0, "Could not initialise the IPIP server tunnel, retrying %d more times\n", retries);
  }
  if (retries <= 0) {
    return 1;
  }

  olsr_add_ifchange_handler(smartgw_tunnel_monitor);

  return 0;
}

/**
 * Cleanup gateway tunnel system
 */
void olsr_cleanup_gateways(void) {
  struct avl_node * avlnode = NULL;

  olsr_remove_ifchange_handler(smartgw_tunnel_monitor);

  olsr_os_cleanup_iptunnel(TUNNEL_NAME);

  /* remove all gateways in the gateway tree that are not the active gateway */
  while ((avlnode = avl_walk_first(&gateway_tree))) {
    struct gateway_entry* tree_gw = node2gateway(avlnode);
    if ((tree_gw != olsr_get_inet_gateway(false)) && (tree_gw != olsr_get_inet_gateway(true))) {
      olsr_delete_gateway_tree_entry(tree_gw, FORCE_DELETE_GW_ENTRY, true);
    }
  }

  /* remove the active IPv4 gateway */
  olsr_delete_gateway_tree_entry(olsr_get_inet_gateway(false), FORCE_DELETE_GW_ENTRY, true);

  /* remove the active IPv6 gateway */
  olsr_delete_gateway_tree_entry(olsr_get_inet_gateway(true), FORCE_DELETE_GW_ENTRY, true);

  /* there should be no more gateways */
  assert(!avl_walk_first(&gateway_tree));

  assert(gw_handler);
  gw_handler->cleanup();
  gw_handler = NULL;

  olsr_free_cookie(gateway_entry_mem_cookie);
}

/**
 * Triggers the first lookup of a gateway.
 */
void olsr_trigger_inetgw_startup(void) {
  assert(gw_handler);
  gw_handler->startup();
}

/**
 * Print debug information about gateway entries
 */
#ifndef NODEBUG
void olsr_print_gateway_entries(void) {
  struct ipaddr_str buf;
  struct gateway_entry *gw;
  const int addrsize = olsr_cnf->ip_version == AF_INET ? (INET_ADDRSTRLEN - 1) : (INET6_ADDRSTRLEN - 1);

  OLSR_PRINTF(0, "\n--- %s ---------------------------------------------------- GATEWAYS\n\n", olsr_wallclock_string());
  OLSR_PRINTF(0, "%-*s %-6s %-9s %-9s %s\n",
      addrsize, "IP address", "Type", "Uplink", "Downlink", olsr_cnf->ip_version == AF_INET ? "" : "External Prefix");

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    OLSR_PRINTF(0, "%-*s %s%c%s%c%c %-9u %-9u %s\n",
        addrsize,
        olsr_ip_to_string(&buf, &gw->originator),
        gw->ipv4nat ? "" : "   ",
        gw->ipv4 ? '4' : ' ',
        gw->ipv4nat ? "(N)" : "",
        (gw->ipv4 && gw->ipv6) ? ',' : ' ',
        gw->ipv6 ? '6' : ' ',
        gw->uplink,
        gw->downlink,
        gw->external_prefix.prefix_len == 0 ? "" : olsr_ip_prefix_to_string(&gw->external_prefix));
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)
}
#endif /* NODEBUG */

/*
 * Tx Path Interface
 */

/**
 * Apply the smart gateway modifications to an outgoing HNA
 *
 * @param mask pointer to netmask of the HNA
 * @param prefixlen of the HNA
 */
void olsr_modifiy_inetgw_netmask(union olsr_ip_addr *mask, int prefixlen) {
  uint8_t *ptr = OLSR_IP_ADDR_2_HNA_PTR(mask, prefixlen);

  memcpy(ptr, &smart_gateway_netmask, sizeof(smart_gateway_netmask) - prefixlen / 8);
  if (olsr_cnf->has_ipv4_gateway) {
    ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV4;

    if (olsr_cnf->smart_gw_uplink_nat) {
      ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV4_NAT;
    }
  }
  if (olsr_cnf->has_ipv6_gateway) {
    ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV6;
  }
  if (!olsr_cnf->has_ipv6_gateway || prefixlen != ipv6_internet_route.prefix_len) {
    ptr[GW_HNA_FLAGS] &= ~GW_HNA_FLAG_IPV6PREFIX;
  }
}

/*
 * SgwDynSpeed Plugin Interface
 */

/**
 * Setup the gateway netmask
 */
void refresh_smartgw_netmask(void) {
  uint8_t *ip;
  memset(&smart_gateway_netmask, 0, sizeof(smart_gateway_netmask));

  if (olsr_cnf->smart_gw_active) {
    ip = (uint8_t *) &smart_gateway_netmask;

    if (olsr_cnf->smart_gw_uplink > 0 && olsr_cnf->smart_gw_downlink > 0) {
      /* the link is bi-directional with a non-zero bandwidth */
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

/*
 * TC/SPF/HNA Interface
 */

/**
 * Checks if a HNA prefix/netmask combination is a smart gateway
 *
 * @param prefix
 * @param mask
 * @return true if is a valid smart gateway HNA, false otherwise
 */
bool olsr_is_smart_gateway(struct olsr_ip_prefix *prefix, union olsr_ip_addr *mask) {
  uint8_t *ptr;

  if (!is_prefix_inetgw(prefix)) {
    return false;
  }

  ptr = OLSR_IP_ADDR_2_HNA_PTR(mask, prefix->prefix_len);
  return ptr[GW_HNA_PAD] == 0 && ptr[GW_HNA_FLAGS] != 0;
}

/**
 * Update a gateway_entry based on a HNA
 *
 * @param originator ip of the source of the HNA
 * @param mask netmask of the HNA
 * @param prefixlen of the HNA
 * @param seqno the sequence number of the HNA
 */
void olsr_update_gateway_entry(union olsr_ip_addr *originator, union olsr_ip_addr *mask, int prefixlen, uint16_t seqno) {
  uint8_t *ptr;
  struct gateway_entry *gw = node2gateway(avl_find(&gateway_tree, originator));

  if (!gw) {
    gw = olsr_cookie_malloc(gateway_entry_mem_cookie);
    gw->originator = *originator;
    gw->node.key = &gw->originator;

    avl_insert(&gateway_tree, &gw->node, AVL_DUP_NO);
  } else if (olsr_seqno_diff(seqno, gw->seqno) <= 0) {
    /* ignore older HNAs */
    return;
  }

  /* keep new HNA seqno */
  gw->seqno = seqno;

  ptr = OLSR_IP_ADDR_2_HNA_PTR(mask, prefixlen);
  if ((ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_LINKSPEED) != 0) {
    gw->uplink = deserialize_gw_speed(ptr[GW_HNA_UPLINK]);
    gw->downlink = deserialize_gw_speed(ptr[GW_HNA_DOWNLINK]);
  } else {
    gw->uplink = 0;
    gw->downlink = 0;
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
  assert(gw_handler);
  gw_handler->update(gw);
}

/**
 * Delete a gateway based on the originator IP and the prefixlength of a HNA.
 * Should only be called if prefix is a smart_gw prefix or if node is removed
 * from TC set.
 *
 * @param originator
 * @param prefixlen
 * @param immediate when set to true then the gateway is removed from the
 * gateway tree immediately, else it is removed on a delayed schedule.
 */
void olsr_delete_gateway_entry(union olsr_ip_addr *originator, uint8_t prefixlen, bool immediate) {
  olsr_delete_gateway_tree_entry(node2gateway(avl_find(&gateway_tree, originator)), prefixlen, immediate);
}

/**
 * Delete a gateway entry .
 *
 * @param gw a gateway entry from the gateway tree
 * @param prefixlen
 * @param immediate when set to true then the gateway is removed from the
 * gateway tree immediately, else it is removed on a delayed schedule.
 */
static void olsr_delete_gateway_tree_entry(struct gateway_entry * gw, uint8_t prefixlen, bool immediate) {
  bool change = false;

  if (!gw) {
    return;
  }

  if (immediate && gw->cleanup_timer) {
    /* stop timer if we have to remove immediately */
    olsr_stop_timer(gw->cleanup_timer);
    gw->cleanup_timer = NULL;
  }

  if (gw->cleanup_timer == NULL || gw->ipv4 || gw->ipv6) {
    /* found a gw and it wasn't deleted yet */

    if (olsr_cnf->ip_version == AF_INET && prefixlen == 0) {
      change = gw->ipv4;
      gw->ipv4 = false;
      gw->ipv4nat = false;
    } else if (olsr_cnf->ip_version == AF_INET6 && prefixlen == ipv6_internet_route.prefix_len) {
      change = gw->ipv6;
      gw->ipv6 = false;
    } else if (olsr_cnf->ip_version == AF_INET6 && prefixlen == ipv6_mappedv4_route.prefix_len) {
      change = gw->ipv4;
      gw->ipv4 = false;
      gw->ipv4nat = false;
    }

    if (prefixlen == FORCE_DELETE_GW_ENTRY || !(gw->ipv4 || gw->ipv6)) {
      /* prevent this gateway from being chosen as the new gateway */
      gw->ipv4 = false;
      gw->ipv4nat = false;
      gw->ipv6 = false;

      /* handle gateway loss */
      assert(gw_handler);
      gw_handler->delete(gw);

      /* cleanup gateway if necessary */
      if (current_ipv4_gw == gw) {
        if (v4gw_tunnel) {
          olsr_os_inetgw_tunnel_route(v4gw_tunnel->if_index, true, false);
          olsr_os_del_ipip_tunnel(v4gw_tunnel);
          v4gw_tunnel = NULL;
        }

        current_ipv4_gw = NULL;
      }
      if (current_ipv6_gw == gw) {
        if (v6gw_tunnel) {
          olsr_os_inetgw_tunnel_route(v6gw_tunnel->if_index, false, false);
          olsr_os_del_ipip_tunnel(v6gw_tunnel);
          v6gw_tunnel = NULL;
        }

        current_ipv6_gw = NULL;
      }

      if (!immediate) {
        /* remove gateway entry on a delayed schedule */
        olsr_set_timer(&gw->cleanup_timer, GW_CLEANUP_INTERVAL, 0, false, cleanup_gateway_handler, gw, NULL);
      } else {
        cleanup_gateway_handler(gw);
      }
    } else if (change) {
      assert(gw_handler);
      gw_handler->update(gw);
    }
  }
}

/**
 * Triggers a check if the one of the gateways have been lost or has an
 * ETX = infinity
 */
void olsr_trigger_gatewayloss_check(void) {
  bool ipv4 = false;
  bool ipv6 = false;

  if (current_ipv4_gw) {
    struct tc_entry *tc = olsr_lookup_tc_entry(&current_ipv4_gw->originator);
    ipv4 = (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN);
  }
  if (current_ipv6_gw) {
    struct tc_entry *tc = olsr_lookup_tc_entry(&current_ipv6_gw->originator);
    ipv6 = (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN);
  }

  if (ipv4 || ipv6) {
    assert(gw_handler);
    gw_handler->choose(ipv4, ipv6);
  }
}

/*
 * Gateway Plugin Functions
 */

/**
 * Sets a new internet gateway.
 *
 * @param originator ip address of the node with the new gateway
 * @param ipv4 set ipv4 gateway
 * @param ipv6 set ipv6 gateway
 * @return true if an error happened, false otherwise
 */
bool olsr_set_inet_gateway(union olsr_ip_addr *originator, bool ipv4, bool ipv6) {
  struct gateway_entry *new_gw;

  ipv4 = ipv4 && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit);
  ipv6 = ipv6 && (olsr_cnf->ip_version == AF_INET6);
  if (!ipv4 && !ipv6) {
    return true;
  }

  new_gw = node2gateway(avl_find(&gateway_tree, originator));
  if (!new_gw) {
    /* the originator is not in the gateway tree, we can't set it as gateway */
    return true;
  }

  /* handle IPv4 */
  if (ipv4 && new_gw->ipv4 && (!new_gw->ipv4nat || olsr_cnf->smart_gw_allow_nat) && current_ipv4_gw != new_gw) {
    struct olsr_iptunnel_entry *new_v4gw_tunnel = olsr_os_add_ipip_tunnel(&new_gw->originator, true);
    if (new_v4gw_tunnel) {
      olsr_os_inetgw_tunnel_route(new_v4gw_tunnel->if_index, true, true);
      if (v4gw_tunnel) {
        olsr_os_del_ipip_tunnel(v4gw_tunnel);
        v4gw_tunnel = NULL;
      }
      current_ipv4_gw = new_gw;
      v4gw_tunnel = new_v4gw_tunnel;
    } else {
      /* adding the tunnel failed, we try again in the next cycle */
      ipv4 = false;
    }
  }

  /* handle IPv6 */
  if (ipv6 && new_gw->ipv6 && current_ipv6_gw != new_gw) {
    struct olsr_iptunnel_entry *new_v6gw_tunnel = olsr_os_add_ipip_tunnel(&new_gw->originator, false);
		if (new_v6gw_tunnel) {
			olsr_os_inetgw_tunnel_route(new_v6gw_tunnel->if_index, false, true);
			if (v6gw_tunnel) {
				olsr_os_del_ipip_tunnel(v6gw_tunnel);
				v6gw_tunnel = NULL;
			}
			current_ipv6_gw = new_gw;
			v6gw_tunnel = new_v6gw_tunnel;
		} else {
			/* adding the tunnel failed, we try again in the next cycle */
			ipv6 = false;
		}
  }

  return !ipv4 && !ipv6;
}

/**
 * @param ipv6 if set to true then the IPv6 gateway is returned, otherwise the IPv4
 * gateway is returned
 * @return a pointer to the gateway_entry of the current ipv4 internet gw or
 * NULL if not set.
 */
struct gateway_entry *olsr_get_inet_gateway(bool ipv6) {
	if (ipv6) {
		return current_ipv6_gw;
	}

	return current_ipv4_gw;
}

#endif /* __linux__ */
