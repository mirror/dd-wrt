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
#include "gateway_list.h"
#include "gateway.h"
#include "egressTypes.h"
#include "egressFile.h"

#include <assert.h>
#include <linux/rtnetlink.h>

/*
 * Defines for the multi-gateway script
 */

#define SCRIPT_MODE_GENERIC   "generic"
#define SCRIPT_MODE_OLSRIF    "olsrif"
#define SCRIPT_MODE_SGWSRVTUN "sgwsrvtun"
#define SCRIPT_MODE_EGRESSIF  "egressif"
#define SCRIPT_MODE_SGWTUN    "sgwtun"

/* ipv4 prefix 0.0.0.0/0 */
static struct olsr_ip_prefix ipv4_slash_0_route;

/* ipv4 prefixes 0.0.0.0/1 and 128.0.0.0/1 */
static struct olsr_ip_prefix ipv4_slash_1_routes[2];

/** the gateway tree */
struct avl_tree gateway_tree;

/** gateway cookie */
static struct olsr_cookie_info *gateway_entry_mem_cookie = NULL;

/** gateway container cookie */
static struct olsr_cookie_info *gw_container_entry_mem_cookie = NULL;

/** the gateway netmask for the HNA */
static uint8_t smart_gateway_netmask[sizeof(union olsr_ip_addr)];

/** the gateway handler/plugin */
static struct olsr_gw_handler *gw_handler;

/** the IPv4 gateway list */
struct gw_list gw_list_ipv4;

/** the IPv6 gateway list */
struct gw_list gw_list_ipv6;

/** the current IPv4 gateway */
static struct gw_container_entry *current_ipv4_gw;

/** the current IPv6 gateway */
static struct gw_container_entry *current_ipv6_gw;

/** interface names for smart gateway tunnel interfaces, IPv4 */
struct interfaceName * sgwTunnel4InterfaceNames;

/** interface names for smart gateway tunnel interfaces, IPv6 */
struct interfaceName * sgwTunnel6InterfaceNames;

/** the timer for proactive takedown */
static struct timer_entry *gw_takedown_timer;

struct BestOverallLink {
  bool valid;
  bool isOlsr;
  union {
    struct sgw_egress_if * egress;
    struct gateway_entry * olsr;
  } link;
  int olsrTunnelIfIndex;
};

static struct sgw_egress_if * bestEgressLinkPrevious = NULL;
static struct sgw_egress_if * bestEgressLink = NULL;

struct sgw_route_info bestEgressLinkPreviousRoute;
struct sgw_route_info bestEgressLinkRoute;

struct sgw_route_info bestOverallLinkPreviousRoutes[2];
struct sgw_route_info bestOverallLinkRoutes[2];

static struct BestOverallLink bestOverallLinkPrevious;
static struct BestOverallLink bestOverallLink;

/*
 * Forward Declarations
 */

static void olsr_delete_gateway_tree_entry(struct gateway_entry * gw, uint8_t prefixlen, bool immediate);

/*
 * Helper Functions
 */

/**
 * @return the gateway 'server' tunnel name to use
 */
static inline const char * server_tunnel_name(void) {
  return (olsr_cnf->ip_version == AF_INET ? TUNNEL_ENDPOINT_IF : TUNNEL_ENDPOINT_IF6);
}

/**
 * Convert the netmask of the HNA (in the form of an IP address) to a HNA
 * pointer.
 *
 * @param mask the netmask of the HNA (in the form of an IP address)
 * @param prefixlen the prefix length
 * @return a pointer to the HNA
 */
static inline uint8_t * hna_mask_to_hna_pointer(union olsr_ip_addr *mask, int prefixlen) {
  return (((uint8_t *)mask) + ((prefixlen+7)/8));
}

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

  if (value == UINT8_MAX) {
    /* maximum value: also return maximum value */
    return MAX_SMARTGW_SPEED;
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

  if (speed >= MAX_SMARTGW_SPEED) {
    return UINT8_MAX;
  }

  while ((speed > 32 || (speed % 10) == 0) && exp < 7) {
    speed /= 10;
    exp++;
  }
  return ((speed - 1) << 3) | exp;
}

/**
 * Find an interfaceName struct corresponding to a certain gateway
 * (when gw != NULL) or to an empty interfaceName struct (when gw == NULL).
 *
 * @param gw the gateway to find (when not NULL), or the empty struct to find (when NULL)
 * @return a pointer to the struct, or NULL when not found
 */
static struct interfaceName * find_interfaceName(struct gateway_entry *gw) {
  struct interfaceName * sgwTunnelInterfaceNames;
  uint8_t i = 0;

  if (!multi_gateway_mode()) {
    return NULL;
  }

  assert(sgwTunnel4InterfaceNames);
  assert(sgwTunnel6InterfaceNames);

  sgwTunnelInterfaceNames = (olsr_cnf->ip_version == AF_INET) ? sgwTunnel4InterfaceNames : sgwTunnel6InterfaceNames;
  while (i < olsr_cnf->smart_gw_use_count) {
    struct interfaceName * ifn = &sgwTunnelInterfaceNames[i];
    if (ifn->gw == gw) {
      return ifn;
    }
    i++;
  }

  return NULL;
}

/**
 * Get an unused olsr ipip tunnel name for a certain gateway and store it in name.
 *
 * @param gw pointer to the gateway
 * @param name pointer to output buffer (length IFNAMSIZ)
 * @param interfaceName a pointer to the location where to store a pointer to the interfaceName struct
 */
static void get_unused_iptunnel_name(struct gateway_entry *gw, char * name, struct interfaceName ** interfaceName) {
  static uint32_t counter = 0;

  assert(gw);
  assert(name);
  assert(interfaceName);

  memset(name, 0, IFNAMSIZ);

  if (multi_gateway_mode()) {
    struct interfaceName * ifn = find_interfaceName(NULL);

    if (ifn) {
      strncpy(&name[0], &ifn->name[0], sizeof(ifn->name));
      *interfaceName = ifn;
      ifn->gw = gw;
      return;
    }

    /* do not return, fall-through to classic naming as fallback */
  }

  snprintf(name, IFNAMSIZ, "tnl_%08x", (olsr_cnf->ip_version == AF_INET) ? gw->originator.v4.s_addr : ++counter);
  *interfaceName = NULL;
}

/**
 * Set an olsr ipip tunnel name that is used by a certain gateway as unused
 *
 * @param gw pointer to the gateway
 */
static void set_unused_iptunnel_name(struct gateway_entry *gw) {
  struct interfaceName * ifn;

  if (!multi_gateway_mode()) {
    return;
  }

  assert(gw);

  ifn = find_interfaceName(gw);
  if (ifn) {
    ifn->gw = NULL;
    return;
  }
}

/**
 * Run the multi-gateway script/
 *
 * @param mode the mode (see SCRIPT_MODE_* defines)
 * @param addMode true to add policy routing, false to remove it
 * @param ifname the interface name (optional)
 * @param tableNr the routing table number (optional)
 * @param ruleNr the IP rule number/priority (optional)
 * @param bypassRuleNr the bypass IP rule number/priority (optional)
 * @return true when successful
 */
static bool multiGwRunScript(const char * mode, bool addMode, const char * ifName, uint32_t tableNr, uint32_t ruleNr, uint32_t bypassRuleNr) {
  struct autobuf buf;
  int r;

  assert(!strcmp(mode, SCRIPT_MODE_GENERIC) //
      || !strcmp(mode, SCRIPT_MODE_OLSRIF)//
      || !strcmp(mode, SCRIPT_MODE_SGWSRVTUN)//
      || !strcmp(mode, SCRIPT_MODE_EGRESSIF)//
      || !strcmp(mode, SCRIPT_MODE_SGWTUN)//
      );

  assert(strcmp(mode, SCRIPT_MODE_GENERIC) //
      || (!strcmp(mode, SCRIPT_MODE_GENERIC) && !ifName && !tableNr && !ruleNr && !bypassRuleNr));

  assert(strcmp(mode, SCRIPT_MODE_OLSRIF) //
      || (!strcmp(mode, SCRIPT_MODE_OLSRIF) && ifName && !tableNr && !ruleNr && bypassRuleNr));

  assert(strcmp(mode, SCRIPT_MODE_SGWSRVTUN) //
      || (!strcmp(mode, SCRIPT_MODE_SGWSRVTUN) && ifName && tableNr&& ruleNr && !bypassRuleNr));

  assert(strcmp(mode, SCRIPT_MODE_EGRESSIF) //
      || (!strcmp(mode, SCRIPT_MODE_EGRESSIF) && ifName && tableNr && ruleNr && bypassRuleNr));

  assert(strcmp(mode, SCRIPT_MODE_SGWTUN) //
      || (!strcmp(mode, SCRIPT_MODE_SGWTUN) && ifName && tableNr && ruleNr && !bypassRuleNr));

  abuf_init(&buf, 1024);

  abuf_appendf(&buf, "\"%s\"", olsr_cnf->smart_gw_policyrouting_script);

  abuf_appendf(&buf, " \"%s\"", (olsr_cnf->ip_version == AF_INET) ? "ipv4" : "ipv6");

  abuf_appendf(&buf, " \"%s\"", mode);

  abuf_appendf(&buf, " \"%s\"", addMode ? "add" : "del");

  if (ifName) {
    abuf_appendf(&buf, " \"%s\"", ifName);
  }

  if (tableNr) {
    abuf_appendf(&buf, " \"%u\"", tableNr);
  }

  if (ruleNr) {
    abuf_appendf(&buf, " \"%u\"", ruleNr);
  }

  if (bypassRuleNr) {
    abuf_appendf(&buf, " \"%u\"", bypassRuleNr);
  }

  r = system(buf.buf);

  abuf_free(&buf);

  return (r == 0);
}

/**
 * Setup generic multi-gateway iptables and ip rules
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesGeneric(bool add) {
  return multiGwRunScript(SCRIPT_MODE_GENERIC, add, NULL, 0, 0, 0);
}

/**
 * Setup multi-gateway iptables and ip rules for all OLSR interfaces.
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesOlsrInterfaces(bool add) {
  bool ok = true;
  struct olsr_if * ifn;
  unsigned int i = 0;

  for (ifn = olsr_cnf->interfaces; ifn; ifn = ifn->next, i++) {
    if (!multiGwRunScript( //
        SCRIPT_MODE_OLSRIF,//
        add, //
        ifn->name, //
        0, //
        0, //
        olsr_cnf->smart_gw_offset_rules + olsr_cnf->smart_gw_egress_interfaces_count + i //
            )) {
      ok = false;
      if (add) {
        return ok;
      }
    }
  }

  return ok;
}

/**
 * Setup multi-gateway iptables and ip rules for the smart gateway server tunnel.
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesSgwServerTunnel(bool add) {
  return multiGwRunScript( //
      SCRIPT_MODE_SGWSRVTUN,//
      add, //
      server_tunnel_name(), //
      olsr_cnf->smart_gw_offset_tables, //
      olsr_cnf->smart_gw_offset_rules + olsr_cnf->smart_gw_egress_interfaces_count + getNrOfOlsrInterfaces(olsr_cnf), //
      0 //
      );
}

/**
 * Setup multi-gateway iptables and ip rules for all egress interfaces.
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesEgressInterfaces(bool add) {
  bool ok = true;

  struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
  while (egress_if) {
    if (!multiGwRunScript(SCRIPT_MODE_EGRESSIF, add, egress_if->name, egress_if->tableNr, egress_if->ruleNr, egress_if->bypassRuleNr)) {
      ok = false;
      if (add) {
        return ok;
      }
    }

    egress_if = egress_if->next;
  }

  return ok;
}

/**
 * Setup multi-gateway iptables and ip rules for the smart gateway client tunnels.
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesSgwTunnels(bool add) {
  bool ok = true;
  unsigned int i = 0;

  while (i < olsr_cnf->smart_gw_use_count) {
    struct interfaceName * ifn = (olsr_cnf->ip_version == AF_INET) ? &sgwTunnel4InterfaceNames[i] : &sgwTunnel6InterfaceNames[i];
    if (!multiGwRunScript(SCRIPT_MODE_SGWTUN, add, ifn->name, ifn->tableNr, ifn->ruleNr, ifn->bypassRuleNr)) {
      ok = false;
      if (add) {
        return ok;
      }
    }

    i++;
  }

  return ok;
}

/**
 * Process interface up/down events for non-olsr interfaces, which are egress
 * interfaces
 *
 * @param if_index the index of the interface
 * @param flag the up/down event
 */
static void doEgressInterface(int if_index, enum olsr_ifchg_flag flag) {
  switch (flag) {
    case IFCHG_IF_ADD: {
      char ifname[IF_NAMESIZE];
      struct sgw_egress_if * egress_if;

      /*
       * we need to get the name of the interface first because the interface
       * might be hot-plugged _after_ olsrd has started
       */
      if (!if_indextoname(if_index, ifname)) {
        /* not a known OS interface */
        return;
      }

      egress_if = findEgressInterface(ifname);
      if (!egress_if) {
        /* not a known egress interface */
        return;
      }

      egress_if->if_index = if_index;

      if (egress_if->upCurrent) {
        /* interface is already up: no change */
        return;
      }

      egress_if->upPrevious = egress_if->upCurrent;
      egress_if->upCurrent = true;
      egress_if->upChanged = true;

      egress_if->bwCostsChanged = egressBwCalculateCosts(&egress_if->bwCurrent, egress_if->upCurrent);
    }
      break;

    case IFCHG_IF_REMOVE: {
      /*
       * we need to find the egress interface by if_index because we might
       * be too late; the kernel could already have removed the interface
       * in which case we'd get a NULL ifname here if we'd try to call
       * if_indextoname
       */
      struct sgw_egress_if * egress_if = findEgressInterfaceByIndex(if_index);
      if (!egress_if) {
        /* not a known egress interface */
        return;
      }

      if (!egress_if->upCurrent) {
        /* interface is already down: no change */
        return;
      }

      egress_if->upPrevious = egress_if->upCurrent;
      egress_if->upCurrent = false;
      egress_if->upChanged = true;

      egress_if->bwCostsChanged = egressBwCalculateCosts(&egress_if->bwCurrent, egress_if->upCurrent);
    }
      break;

    case IFCHG_IF_UPDATE:
    default:
      return;
  }

  doRoutesMultiGw(true, false, GW_MULTI_CHANGE_PHASE_RUNTIME);
}

/*
 * Callback Functions
 */

/**
 * Callback for tunnel interface monitoring which will set the route into the tunnel
 * when the interface comes up again.
 *
 * @param if_index the interface index
 * @param ifh the interface (NULL when not an olsr interface)
 * @param flag interface change flags
 */
static void smartgw_tunnel_monitor(int if_index, struct interface_olsr *ifh, enum olsr_ifchg_flag flag) {
  if (!ifh && multi_gateway_mode()) {
    /* non-olsr interface in multi-sgw mode */
    doEgressInterface(if_index, flag);
  }

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

/**
 * Remove a gateway from a gateway list.
 *
 * @param gw_list a pointer to the gateway list
 * @param ipv4 true when dealing with an IPv4 gateway / gateway list
 * @param gw a pointer to the gateway to remove from the list
 */
static void removeGatewayFromList(struct gw_list * gw_list, bool ipv4, struct gw_container_entry * gw) {
  if (gw->tunnel) {
    struct interfaceName * ifn = find_interfaceName(gw->gw);
    if (ifn) {
      olsr_os_inetgw_tunnel_route(gw->tunnel->if_index, ipv4, false, ifn->tableNr);
    }
    olsr_os_del_ipip_tunnel(gw->tunnel);
    set_unused_iptunnel_name(gw->gw);
    gw->tunnel = NULL;
  }
  gw->gw = NULL;
  olsr_cookie_free(gw_container_entry_mem_cookie, olsr_gw_list_remove(gw_list, gw));
}

/**
 * Remove expensive gateways from the gateway list.
 * It uses the smart_gw_takedown_percentage configuration parameter
 *
 * @param gw_list a pointer to the gateway list
 * @param ipv4 true when dealing with an IPv4 gateway / gateway list
 * @param current_gw the current gateway
 */
static void takeDownExpensiveGateways(struct gw_list * gw_list, bool ipv4, struct gw_container_entry * current_gw) {
  int64_t current_gw_cost_boundary;

  /*
   * exit immediately when takedown is disabled, there is no current gateway, or
   * when there is only a single gateway
   */
  if ((olsr_cnf->smart_gw_takedown_percentage == 0) || (current_gw == NULL ) || (gw_list->count <= 1)) {
    return;
  }

  /* get the cost boundary */
  current_gw_cost_boundary = current_gw->gw->path_cost;
  if (olsr_cnf->smart_gw_takedown_percentage < 100) {
    if (current_gw_cost_boundary <= (INT64_MAX / 100)) {
      current_gw_cost_boundary =  ((current_gw_cost_boundary * 100) / olsr_cnf->smart_gw_takedown_percentage);
    } else {
      /* perform scaling because otherwise the percentage calculation can overflow */
      current_gw_cost_boundary = (((current_gw_cost_boundary      ) / olsr_cnf->smart_gw_takedown_percentage) * 100) + 99;
    }
  }

  /* loop while we still have gateways */
  while (gw_list->count > 1) {
    /* get the worst gateway */
    struct gw_container_entry * worst_gw = olsr_gw_list_get_worst_entry(gw_list);

    /* exit when it's the current gateway */
    if (worst_gw == current_gw) {
      return;
    }

    /*
     * exit when it (and further ones; the list is sorted on costs) has lower
     * costs than the boundary costs
     */
    if (worst_gw->gw->path_cost < current_gw_cost_boundary) {
      return;
    }

    /* it's too expensive: take it down */
    removeGatewayFromList(gw_list, ipv4, worst_gw);
  }
}

/**
 * Timer callback for proactive gateway takedown
 *
 * @param unused unused
 */
static void gw_takedown_timer_callback(void *unused __attribute__ ((unused))) {
  takeDownExpensiveGateways(&gw_list_ipv4, true, current_ipv4_gw);
  takeDownExpensiveGateways(&gw_list_ipv6, false, current_ipv6_gw);
}

/*
 * Main Interface
 */

/**
 * Initialize gateway system
 */
int olsr_init_gateways(void) {
  int retries = 5;

  memset(&bestEgressLinkPreviousRoute, 0, sizeof(bestEgressLinkPreviousRoute));
  memset(&bestEgressLinkRoute, 0, sizeof(bestEgressLinkRoute));
  memset(bestOverallLinkPreviousRoutes, 0, sizeof(bestOverallLinkPreviousRoutes));
  memset(bestOverallLinkRoutes, 0, sizeof(bestOverallLinkRoutes));

  /* ipv4 prefix 0.0.0.0/0 */
  memset(&ipv4_slash_0_route, 0, sizeof(ipv4_slash_0_route));

  /* ipv4 prefixes 0.0.0.0/1 and 128.0.0.0/1 */
  memset(&ipv4_slash_1_routes, 0, sizeof(ipv4_slash_1_routes));
  ipv4_slash_1_routes[0].prefix.v4.s_addr = htonl(0x00000000);
  ipv4_slash_1_routes[0].prefix_len = 1;
  ipv4_slash_1_routes[1].prefix.v4.s_addr = htonl(0x80000000);
  ipv4_slash_1_routes[1].prefix_len = 1;

  gateway_entry_mem_cookie = olsr_alloc_cookie("gateway_entry_mem_cookie", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(gateway_entry_mem_cookie, sizeof(struct gateway_entry));

  gw_container_entry_mem_cookie = olsr_alloc_cookie("gw_container_entry_mem_cookie", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(gw_container_entry_mem_cookie, sizeof(struct gw_container_entry));

  avl_init(&gateway_tree, avl_comp_default);

  olsr_gw_list_init(&gw_list_ipv4, olsr_cnf->smart_gw_use_count);
  olsr_gw_list_init(&gw_list_ipv6, olsr_cnf->smart_gw_use_count);

  sgwTunnel4InterfaceNames = NULL;
  sgwTunnel6InterfaceNames = NULL;
  memset(&bestOverallLinkPrevious, 0, sizeof(bestOverallLinkPrevious));
  memset(&bestOverallLink, 0, sizeof(bestOverallLink));

  if (multi_gateway_mode()) {
    uint8_t i;
    struct sgw_egress_if * egressif;
    unsigned int nrOlsrIfs = getNrOfOlsrInterfaces(olsr_cnf);

    /* Initialise the egress interfaces */
    /* setup the egress interface name/mark pairs */
    i = 0;
    egressif = olsr_cnf->smart_gw_egress_interfaces;
    while (egressif) {
      egressif->if_index = if_nametoindex(egressif->name);

      egressif->tableNr = olsr_cnf->smart_gw_offset_tables + 1 + i;
      egressif->ruleNr = olsr_cnf->smart_gw_offset_rules + olsr_cnf->smart_gw_egress_interfaces_count + nrOlsrIfs + 1 + i;
      egressif->bypassRuleNr = olsr_cnf->smart_gw_offset_rules + i;

      egressif->upPrevious = egressif->upCurrent = olsr_if_isup(egressif->name);
      egressif->upChanged = (egressif->upPrevious != egressif->upCurrent);

      egressBwClear(&egressif->bwPrevious, egressif->upPrevious);
      egressBwClear(&egressif->bwCurrent, egressif->upCurrent);
      egressif->bwCostsChanged = egressBwCostsChanged(egressif);
      egressif->bwNetworkChanged = egressBwNetworkChanged(egressif);
      egressif->bwGatewayChanged = egressBwGatewayChanged(egressif);
      egressif->bwChanged = egressBwChanged(egressif);

      egressif->inEgressFile = false;

      egressif = egressif->next;
      i++;
    }
    assert(i == olsr_cnf->smart_gw_egress_interfaces_count);

    /* setup the SGW tunnel name/mark pairs */
    sgwTunnel4InterfaceNames = olsr_malloc(sizeof(struct interfaceName) * olsr_cnf->smart_gw_use_count, "sgwTunnel4InterfaceNames");
    sgwTunnel6InterfaceNames = olsr_malloc(sizeof(struct interfaceName) * olsr_cnf->smart_gw_use_count, "sgwTunnel6InterfaceNames");
    for (i = 0; i < olsr_cnf->smart_gw_use_count; i++) {
      struct interfaceName * ifn = &sgwTunnel4InterfaceNames[i];
      uint32_t tableNr = olsr_cnf->smart_gw_offset_tables + 1 + olsr_cnf->smart_gw_egress_interfaces_count + i;
      uint32_t ruleNr = olsr_cnf->smart_gw_offset_rules + olsr_cnf->smart_gw_egress_interfaces_count + nrOlsrIfs + 1 + olsr_cnf->smart_gw_egress_interfaces_count + i;

      ifn->gw = NULL;
      ifn->tableNr = tableNr;
      ifn->ruleNr = ruleNr;
      ifn->bypassRuleNr = 0;
      snprintf(&ifn->name[0], sizeof(ifn->name), "tnl_4%03u", ifn->tableNr);

      ifn = &sgwTunnel6InterfaceNames[i];
      ifn->gw = NULL;
      ifn->tableNr = tableNr;
      ifn->ruleNr = ruleNr;
      ifn->bypassRuleNr = 0;
      snprintf(&ifn->name[0], sizeof(ifn->name), "tnl_6%03u", ifn->tableNr);
    }
  }

  current_ipv4_gw = NULL;
  current_ipv6_gw = NULL;

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
    if (!olsr_os_init_iptunnel(server_tunnel_name())) {
      retries = 5;
      break;
    }

    olsr_printf(0, "Could not initialise the IPIP server tunnel, retrying %d more times\n", retries);
  }
  if (retries <= 0) {
    return 1;
  }

  return 0;
}

/**
 * Startup gateway system
 */
int olsr_startup_gateways(void) {
  bool ok = true;

  if (!multi_gateway_mode()) {
    olsr_add_ifchange_handler(smartgw_tunnel_monitor);
    return 0;
  }

  ok = ok && multiGwRulesGeneric(true);
  ok = ok && multiGwRulesSgwServerTunnel(true);
  ok = ok && multiGwRulesOlsrInterfaces(true);
  ok = ok && multiGwRulesEgressInterfaces(true);
  ok = ok && multiGwRulesSgwTunnels(true);
  if (!ok) {
    olsr_printf(0, "Could not setup multi-gateway iptables and ip rules\n");
    olsr_shutdown_gateways();
    return 1;
  }

  startEgressFile();
  doRoutesMultiGw(true, false, GW_MULTI_CHANGE_PHASE_STARTUP);

  olsr_add_ifchange_handler(smartgw_tunnel_monitor);

  /* Check egress interfaces up status to compensate for a race: the interfaces
   * can change status between initialising their data structures and
   * registering the tunnel monitor */
  {
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      bool upCurrent = olsr_if_isup(egress_if->name);

      if (upCurrent != egress_if->upCurrent) {
        int idx = upCurrent ? (int) if_nametoindex(egress_if->name) : egress_if->if_index;
        enum olsr_ifchg_flag flag = upCurrent ? IFCHG_IF_ADD : IFCHG_IF_REMOVE;
        smartgw_tunnel_monitor(idx, NULL, flag);
      }

      egress_if = egress_if->next;
    }
  }

  if (olsr_cnf->smart_gw_takedown_percentage > 0) {
    /* start gateway takedown timer */
    olsr_set_timer(&gw_takedown_timer, olsr_cnf->smart_gw_period, 0, true, &gw_takedown_timer_callback, NULL, 0);
  }

  return 0;
}

/**
 * Shutdown gateway tunnel system
 */
void olsr_shutdown_gateways(void) {
  if (!multi_gateway_mode()) {
    olsr_remove_ifchange_handler(smartgw_tunnel_monitor);
    return;
  }

  if (olsr_cnf->smart_gw_takedown_percentage > 0) {
    /* stop gateway takedown timer */
    olsr_stop_timer(gw_takedown_timer);
    gw_takedown_timer = NULL;
  }

  olsr_remove_ifchange_handler(smartgw_tunnel_monitor);

  stopEgressFile();
  {
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      egress_if->upPrevious = egress_if->upCurrent;
      egress_if->upCurrent = false;
      egress_if->upChanged = (egress_if->upPrevious != egress_if->upCurrent);

      egress_if->bwPrevious = egress_if->bwCurrent;
      egressBwClear(&egress_if->bwCurrent, egress_if->upCurrent);
      egress_if->bwCostsChanged = egressBwCostsChanged(egress_if);
      egress_if->bwNetworkChanged = egressBwNetworkChanged(egress_if);
      egress_if->bwGatewayChanged = egressBwGatewayChanged(egress_if);
      egress_if->bwChanged = egressBwChanged(egress_if);

      egress_if->inEgressFile = false;

      egress_if = egress_if->next;
    }
  }
  doRoutesMultiGw(true, false, GW_MULTI_CHANGE_PHASE_SHUTDOWN);

  (void)multiGwRulesSgwTunnels(false);
  (void)multiGwRulesEgressInterfaces(false);
  (void)multiGwRulesOlsrInterfaces(false);
  (void)multiGwRulesSgwServerTunnel(false);
  (void)multiGwRulesGeneric(false);
}

/**
 * Cleanup gateway tunnel system
 */
void olsr_cleanup_gateways(void) {
  struct gateway_entry * tree_gw;
  struct gw_container_entry * gw;

  /* remove all gateways in the gateway tree that are not the active gateway */
  OLSR_FOR_ALL_GATEWAY_ENTRIES(tree_gw) {
    if ((tree_gw != olsr_get_inet_gateway(false)) && (tree_gw != olsr_get_inet_gateway(true))) {
      olsr_delete_gateway_tree_entry(tree_gw, FORCE_DELETE_GW_ENTRY, true);
    }
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(tree_gw)

  /* remove all active IPv4 gateways (should be at most 1 now) */
  OLSR_FOR_ALL_GWS(&gw_list_ipv4.head, gw) {
    if (gw && gw->gw) {
      olsr_delete_gateway_entry(&gw->gw->originator, FORCE_DELETE_GW_ENTRY, true);
    }
  }
  OLSR_FOR_ALL_GWS_END(gw);

  /* remove all active IPv6 gateways (should be at most 1 now) */
  OLSR_FOR_ALL_GWS(&gw_list_ipv6.head, gw) {
    if (gw && gw->gw) {
      olsr_delete_gateway_entry(&gw->gw->originator, FORCE_DELETE_GW_ENTRY, true);
    }
  }
  OLSR_FOR_ALL_GWS_END(gw);

  /* there should be no more gateways */
  assert(!avl_walk_first(&gateway_tree));
  assert(!current_ipv4_gw);
  assert(!current_ipv6_gw);

  olsr_os_cleanup_iptunnel(server_tunnel_name());

  assert(gw_handler);
  gw_handler->cleanup();
  gw_handler = NULL;

  if (sgwTunnel4InterfaceNames) {
    free(sgwTunnel4InterfaceNames);
    sgwTunnel4InterfaceNames = NULL;
  }
  if (sgwTunnel6InterfaceNames) {
    free(sgwTunnel6InterfaceNames);
    sgwTunnel6InterfaceNames = NULL;
  }

  olsr_gw_list_cleanup(&gw_list_ipv6);
  olsr_gw_list_cleanup(&gw_list_ipv4);
  olsr_free_cookie(gw_container_entry_mem_cookie);
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
  uint8_t *ptr = hna_mask_to_hna_pointer(mask, prefixlen);

  /* copy the current settings for uplink/downlink into the mask */
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

  /* clear the mask */
  memset(&smart_gateway_netmask, 0, sizeof(smart_gateway_netmask));

  if (olsr_cnf->smart_gw_active) {
    ip = (uint8_t *) &smart_gateway_netmask;

    ip[GW_HNA_FLAGS] |= GW_HNA_FLAG_LINKSPEED;
    ip[GW_HNA_DOWNLINK] = serialize_gw_speed(olsr_cnf->smart_gw_downlink);
    ip[GW_HNA_UPLINK] = serialize_gw_speed(olsr_cnf->smart_gw_uplink);

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

  ptr = hna_mask_to_hna_pointer(mask, prefix->prefix_len);
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
  struct gw_container_entry * new_gw_in_list;
  uint8_t *ptr;
  int64_t prev_path_cost = 0;
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

  ptr = hna_mask_to_hna_pointer(mask, prefixlen);
  if ((ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_LINKSPEED) != 0) {
    gw->uplink = deserialize_gw_speed(ptr[GW_HNA_UPLINK]);
    gw->downlink = deserialize_gw_speed(ptr[GW_HNA_DOWNLINK]);
  } else {
    gw->uplink = 0;
    gw->downlink = 0;
  }

  gw->ipv4 = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV4) != 0;
  gw->ipv4nat = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV4_NAT) != 0;
  gw->ipv6 = false;

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

  assert(gw_handler);
  prev_path_cost = gw->path_cost;
  gw->path_cost = gw_handler->getcosts(gw);

  if (prev_path_cost != gw->path_cost) {
    /* re-sort the gateway list when costs have changed and when it is an active gateway */
    new_gw_in_list = olsr_gw_list_find(&gw_list_ipv4, gw);
    if (new_gw_in_list) {
      new_gw_in_list = olsr_gw_list_update(&gw_list_ipv4, new_gw_in_list);
      assert(new_gw_in_list);

      if (multi_gateway_mode() && new_gw_in_list->tunnel) {
        /* the active gateway has changed its costs: re-evaluate egress routes */
        doRoutesMultiGw(false, true, GW_MULTI_CHANGE_PHASE_RUNTIME);
      }
    }

    new_gw_in_list = olsr_gw_list_find(&gw_list_ipv6, gw);
    if (new_gw_in_list) {
      new_gw_in_list = olsr_gw_list_update(&gw_list_ipv6, new_gw_in_list);
      assert(new_gw_in_list);

      if (multi_gateway_mode() && new_gw_in_list->tunnel) {
        /* the active gateway has changed its costs: re-evaluate egress routes */
        doRoutesMultiGw(false, true, GW_MULTI_CHANGE_PHASE_RUNTIME);
      }
    }
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
    /* the gw  is not scheduled for deletion */

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
      struct gw_container_entry * gw_in_list;

      /* prevent this gateway from being chosen as the new gateway */
      gw->ipv4 = false;
      gw->ipv4nat = false;
      gw->ipv6 = false;

      /* handle gateway loss */
      assert(gw_handler);
      gw_handler->delete(gw);

      /* cleanup gateway if necessary */
      gw_in_list = olsr_gw_list_find(&gw_list_ipv4, gw);
      if (gw_in_list) {
        if (current_ipv4_gw && current_ipv4_gw->gw == gw) {
          olsr_os_inetgw_tunnel_route(current_ipv4_gw->tunnel->if_index, true, false, olsr_cnf->rt_table_tunnel);
          current_ipv4_gw = NULL;
        }

        if (gw_in_list->tunnel) {
          struct interfaceName * ifn = find_interfaceName(gw_in_list->gw);
          if (ifn) {
            olsr_os_inetgw_tunnel_route(gw_in_list->tunnel->if_index, true, false, ifn->tableNr);
          }
          olsr_os_del_ipip_tunnel(gw_in_list->tunnel);
          set_unused_iptunnel_name(gw_in_list->gw);
          gw_in_list->tunnel = NULL;
        }

        gw_in_list->gw = NULL;
        gw_in_list = olsr_gw_list_remove(&gw_list_ipv4, gw_in_list);
        olsr_cookie_free(gw_container_entry_mem_cookie, gw_in_list);
      }

      gw_in_list = olsr_gw_list_find(&gw_list_ipv6, gw);
      if (gw_in_list) {
        if (current_ipv6_gw && current_ipv6_gw->gw == gw) {
          olsr_os_inetgw_tunnel_route(current_ipv6_gw->tunnel->if_index, false, false, olsr_cnf->rt_table_tunnel);
          current_ipv6_gw = NULL;
        }

        if (gw_in_list->tunnel) {
          struct interfaceName * ifn = find_interfaceName(gw_in_list->gw);
          if (ifn) {
            olsr_os_inetgw_tunnel_route(gw_in_list->tunnel->if_index, false, false, ifn->tableNr);
          }
          olsr_os_del_ipip_tunnel(gw_in_list->tunnel);
          set_unused_iptunnel_name(gw_in_list->gw);
          gw_in_list->tunnel = NULL;
        }

        gw_in_list->gw = NULL;
        gw_in_list = olsr_gw_list_remove(&gw_list_ipv6, gw_in_list);
        olsr_cookie_free(gw_container_entry_mem_cookie, gw_in_list);
      }

      if (!immediate) {
        /* remove gateway entry on a delayed schedule */
        olsr_set_timer(&gw->cleanup_timer, GW_CLEANUP_INTERVAL, 0, false, cleanup_gateway_handler, gw, NULL);
      } else {
        cleanup_gateway_handler(gw);
      }

      /* when the current gateway was deleted, then immediately choose a new gateway */
      if (!current_ipv4_gw || !current_ipv6_gw) {
        assert(gw_handler);
        gw_handler->choose(!current_ipv4_gw, !current_ipv6_gw);
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

  if (current_ipv4_gw && current_ipv4_gw->gw) {
    struct tc_entry *tc = olsr_lookup_tc_entry(&current_ipv4_gw->gw->originator);
    ipv4 = (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN);
  }
  if (current_ipv6_gw && current_ipv6_gw->gw) {
    struct tc_entry *tc = olsr_lookup_tc_entry(&current_ipv6_gw->gw->originator);
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
 * @param the chosen gateway
 * @param ipv4 set ipv4 gateway
 * @param ipv6 set ipv6 gateway
 * @return true if an error happened, false otherwise
 */
bool olsr_set_inet_gateway(struct gateway_entry * chosen_gw, bool ipv4, bool ipv6) {
  struct gateway_entry *new_gw;

  ipv4 = ipv4 && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit);
  ipv6 = ipv6 && (olsr_cnf->ip_version == AF_INET6);
  if (!ipv4 && !ipv6) {
    return true;
  }

  new_gw = node2gateway(avl_find(&gateway_tree, &chosen_gw->originator));
  if (!new_gw) {
    /* the originator is not in the gateway tree, we can't set it as gateway */
    return true;
  }

  /* handle IPv4 */
  if (ipv4 &&
      new_gw->ipv4 &&
      (!new_gw->ipv4nat || olsr_cnf->smart_gw_allow_nat) &&
      (!current_ipv4_gw || current_ipv4_gw->gw != new_gw)) {
    /* new gw is different than the current gw */

    struct gw_container_entry * new_gw_in_list = olsr_gw_list_find(&gw_list_ipv4, new_gw);
    if (new_gw_in_list) {
      /* new gw is already in the gw list */
      assert(new_gw_in_list->tunnel);
      olsr_os_inetgw_tunnel_route(new_gw_in_list->tunnel->if_index, true, true, olsr_cnf->rt_table_tunnel);
      current_ipv4_gw = new_gw_in_list;

      if (multi_gateway_mode()) {
        doRoutesMultiGw(false, true, GW_MULTI_CHANGE_PHASE_RUNTIME);
      }
    } else {
      /* new gw is not yet in the gw list */
      char name[IFNAMSIZ];
      struct olsr_iptunnel_entry *new_v4gw_tunnel;
      struct interfaceName * interfaceName;

      if (olsr_gw_list_full(&gw_list_ipv4)) {
        /* the list is full: remove the worst active gateway */
        struct gw_container_entry* worst = olsr_gw_list_get_worst_entry(&gw_list_ipv4);
        assert(worst);

        removeGatewayFromList(&gw_list_ipv4, true, worst);
      }

      get_unused_iptunnel_name(new_gw, name, &interfaceName);
      new_v4gw_tunnel = olsr_os_add_ipip_tunnel(&new_gw->originator, true, name);
      if (new_v4gw_tunnel) {
        if (interfaceName) {
          olsr_os_inetgw_tunnel_route(new_v4gw_tunnel->if_index, true, true, interfaceName->tableNr);
        }
        olsr_os_inetgw_tunnel_route(new_v4gw_tunnel->if_index, true, true, olsr_cnf->rt_table_tunnel);

        new_gw_in_list = olsr_cookie_malloc(gw_container_entry_mem_cookie);
        new_gw_in_list->gw = new_gw;
        new_gw_in_list->tunnel = new_v4gw_tunnel;
        current_ipv4_gw = olsr_gw_list_add(&gw_list_ipv4, new_gw_in_list);

        if (multi_gateway_mode()) {
          doRoutesMultiGw(false, true, GW_MULTI_CHANGE_PHASE_RUNTIME);
        }
      } else {
        /* adding the tunnel failed, we try again in the next cycle */
        set_unused_iptunnel_name(new_gw);
        ipv4 = false;
      }
    }
  }

  /* handle IPv6 */
  if (ipv6 &&
      new_gw->ipv6 &&
      (!current_ipv6_gw || current_ipv6_gw->gw != new_gw)) {
    /* new gw is different than the current gw */

  	struct gw_container_entry * new_gw_in_list = olsr_gw_list_find(&gw_list_ipv6, new_gw);
    if (new_gw_in_list) {
      /* new gw is already in the gw list */
      assert(new_gw_in_list->tunnel);
      olsr_os_inetgw_tunnel_route(new_gw_in_list->tunnel->if_index, true, true, olsr_cnf->rt_table_tunnel);
      current_ipv6_gw = new_gw_in_list;

      if (multi_gateway_mode()) {
        doRoutesMultiGw(false, true, GW_MULTI_CHANGE_PHASE_RUNTIME);
      }
    } else {
      /* new gw is not yet in the gw list */
      char name[IFNAMSIZ];
      struct olsr_iptunnel_entry *new_v6gw_tunnel;
      struct interfaceName * interfaceName;

      if (olsr_gw_list_full(&gw_list_ipv6)) {
        /* the list is full: remove the worst active gateway */
        struct gw_container_entry* worst = olsr_gw_list_get_worst_entry(&gw_list_ipv6);
        assert(worst);

        removeGatewayFromList(&gw_list_ipv6, false, worst);
      }

      get_unused_iptunnel_name(new_gw, name, &interfaceName);
      new_v6gw_tunnel = olsr_os_add_ipip_tunnel(&new_gw->originator, false, name);
      if (new_v6gw_tunnel) {
        if (interfaceName) {
          olsr_os_inetgw_tunnel_route(new_v6gw_tunnel->if_index, false, true, interfaceName->tableNr);
        }
        olsr_os_inetgw_tunnel_route(new_v6gw_tunnel->if_index, false, true, olsr_cnf->rt_table_tunnel);

        new_gw_in_list = olsr_cookie_malloc(gw_container_entry_mem_cookie);
        new_gw_in_list->gw = new_gw;
        new_gw_in_list->tunnel = new_v6gw_tunnel;
        current_ipv6_gw = olsr_gw_list_add(&gw_list_ipv6, new_gw_in_list);

        if (multi_gateway_mode()) {
          doRoutesMultiGw(false, true, GW_MULTI_CHANGE_PHASE_RUNTIME);
        }
      } else {
        /* adding the tunnel failed, we try again in the next cycle */
        set_unused_iptunnel_name(new_gw);
        ipv6 = false;
      }
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
		return current_ipv6_gw ? current_ipv6_gw->gw : NULL;
	}

	return current_ipv4_gw ? current_ipv4_gw->gw : NULL;
}

/*
 * Process Egress Changes
 */

#define MSGW_ROUTE_ADD_ALLOWED(phase)   ((phase == GW_MULTI_CHANGE_PHASE_STARTUP ) || (phase == GW_MULTI_CHANGE_PHASE_RUNTIME ))
#define MSGW_ROUTE_ADD_FORCED(phase)    ( phase == GW_MULTI_CHANGE_PHASE_STARTUP )
#define MSGW_ROUTE_DEL_ALLOWED(phase)   ((phase == GW_MULTI_CHANGE_PHASE_RUNTIME ) || (phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN))
#define MSGW_ROUTE_DEL_FORCED(phase)    ( phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN)
#define MSGW_ROUTE_FORCED(phase)        ((phase == GW_MULTI_CHANGE_PHASE_STARTUP ) || (phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN))

/**
 * Determine best egress link.
 * The list of egress interface is ordered on priority (the declaration order),
 * so the function will - for multiple egress links with the same costs - set the
 * best egress interface to the first declared one of those.
 * When there is no best egress interface (that is up) then the function will
 * set the best egress interface to NULL.
 *
 * @param phase the phase of the change (startup/runtime/shutdown)
 * @return true when the best egress link changed or when any of its relevant
 * parameters has changed
 */
static bool determineBestEgressLink(enum sgw_multi_change_phase phase) {
  bool changed = false;
  struct sgw_egress_if * bestEgress = olsr_cnf->smart_gw_egress_interfaces;

  if (phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN) {
    bestEgress = NULL;
  } else {
    struct sgw_egress_if * egress_if = bestEgress;
    if (egress_if) {
      egress_if = egress_if->next;
    }
    while (egress_if) {
      if (egress_if->upCurrent && (egress_if->bwCurrent.costs < bestEgress->bwCurrent.costs)) {
        bestEgress = egress_if;
      }

      egress_if = egress_if->next;
    }

    if (bestEgress && (!bestEgress->upCurrent || (bestEgress->bwCurrent.costs == INT64_MAX))) {
      bestEgress = NULL;
    }
  }

  bestEgressLinkPrevious = bestEgressLink;
  bestEgressLink = bestEgress;

  changed = (bestEgressLinkPrevious != bestEgressLink) || //
      (bestEgressLink && (bestEgressLink->upChanged || bestEgressLink->bwChanged));

  if (changed || MSGW_ROUTE_FORCED(phase)) {
    if (!bestEgressLink || !bestEgressLink->upCurrent) {
      olsr_cnf->smart_gw_uplink = 0;
      olsr_cnf->smart_gw_downlink = 0;
    } else {
      olsr_cnf->smart_gw_uplink = bestEgressLink->bwCurrent.egressUk;
      olsr_cnf->smart_gw_downlink = bestEgressLink->bwCurrent.egressDk;
    }
    refresh_smartgw_netmask();
  }

  return changed;
}

/**
 * Determine best overall link (choose egress interface over olsrd).
 *
 * When there is no best overall link, the best overall link will be set to a
 * NULL egress interface.
 *
 * @param phase the phase of the change (startup/runtime/shutdown)
 * @return true when the best egress link changed or when any of its relevant
 * parameters has changed
 */
static bool determineBestOverallLink(enum sgw_multi_change_phase phase) {
  struct gw_container_entry * gwContainer = (olsr_cnf->ip_version == AF_INET) ? current_ipv4_gw : current_ipv6_gw;
  struct gateway_entry * olsrGw = !gwContainer ? NULL : gwContainer->gw;

  int64_t egressCosts = !bestEgressLink ? INT64_MAX : bestEgressLink->bwCurrent.costs;
  int64_t olsrCosts = !olsrGw ? INT64_MAX : olsrGw->path_cost;
  int64_t bestOverallCosts = MIN(egressCosts, olsrCosts);

  bestOverallLinkPrevious = bestOverallLink;
  if ((bestOverallCosts == INT64_MAX) || (phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN)) {
    bestOverallLink.valid = false;
    bestOverallLink.isOlsr = false;
    bestOverallLink.link.egress = NULL;
    bestOverallLink.olsrTunnelIfIndex = 0;
  } else if (egressCosts <= olsrCosts) {
    bestOverallLink.valid = bestEgressLink;
    bestOverallLink.isOlsr = false;
    bestOverallLink.link.egress = bestEgressLink;
    bestOverallLink.olsrTunnelIfIndex = 0;
  } else {
    struct olsr_iptunnel_entry * tunnel = !gwContainer ? NULL : gwContainer->tunnel;

    bestOverallLink.valid = olsrGw;
    bestOverallLink.isOlsr = true;
    bestOverallLink.link.olsr = olsrGw;
    bestOverallLink.olsrTunnelIfIndex = !tunnel ? 0 : tunnel->if_index;
  }

  return memcmp(&bestOverallLink, &bestOverallLinkPrevious, sizeof(bestOverallLink));
}

/**
 * Program a route (add or remove) through netlink
 *
 * @param add true to add the route, false to remove it
 * @param route the route
 * @param linkName the human readable id of the route, used in error reports in
 * the syslog
 */
static void programRoute(bool add, struct sgw_route_info * route, const char * linkName) {
  if (!route || !route->active) {
    return;
  }

  if (olsr_new_netlink_route( //
      route->route.family, //
      route->route.rttable, //
      route->route.flags, //
      route->route.scope, //
      route->route.if_index, //
      route->route.metric, //
      route->route.protocol, //
      !route->route.srcSet ? NULL : &route->route.srcStore, //
      !route->route.gwSet ? NULL : &route->route.gwStore, //
      !route->route.dstSet ? NULL : &route->route.dstStore, //
      add, //
      route->route.del_similar, //
      route->route.blackhole //
      )) {
    olsr_syslog(OLSR_LOG_ERR, "Could not %s a route for the %s %s", !add ? "remove" : "add", !add ? "previous" : "current", linkName);
    route->active = false;
  } else {
    route->active = add;
  }
}

/**
 * Determine the best overall egress/olsr interface routes.
 *
 * These are a set of 2 /1 routes to override any default gateway
 * routes that are setup through other means such as a DHCP client.
 *
 * @param routes a pointer to the array of 2 /1 routes where to store the
 * newly determined routes
 */
static void determineBestOverallLinkRoutes(struct sgw_route_info * routes) {
  int ifIndex = 0;
  union olsr_ip_addr * gw = NULL;
  int i;
  if (!bestOverallLink.valid) {
    /* there is no current best overall link */
  } else if (!bestOverallLink.isOlsr) {
    /* current best overall link is an egress interface */
    struct sgw_egress_if * egress = bestOverallLink.link.egress;
    if (egress) {
      ifIndex = egress->if_index;
      if (egress->bwCurrent.gatewaySet) {
        gw = &egress->bwCurrent.gateway;
      }
    }
  } else {
    /* current best overall link is an olsr tunnel interface */
    struct gw_container_entry * gwContainer = current_ipv4_gw;
    struct olsr_iptunnel_entry * tunnel = !gwContainer ? NULL : gwContainer->tunnel;

    if (tunnel) {
      ifIndex = tunnel->if_index;
    }
  }

  if (!ifIndex) {
    for (i = 0; i < 2; i++) {
      routes[i].active = false;
    }
    return;
  }

  for (i = 0; i < 2; i++) {
    memset(&routes[i], 0, sizeof(routes[i]));
    routes[i].active = true;
    routes[i].route.family = AF_INET;
    routes[i].route.rttable = olsr_cnf->rt_table;
    routes[i].route.flags = 0;
    routes[i].route.scope = !gw ? RT_SCOPE_LINK : RT_SCOPE_UNIVERSE;
    routes[i].route.if_index = ifIndex;
    routes[i].route.metric = 0;
    routes[i].route.protocol = RTPROT_STATIC;
    routes[i].route.srcSet = false;
    routes[i].route.gwSet = false;
    if (gw) {
      routes[i].route.gwSet = true;
      routes[i].route.gwStore = *gw;
    }
    routes[i].route.dstSet = true;
    routes[i].route.dstStore = ipv4_slash_1_routes[i];
    routes[i].route.del_similar = false;
    routes[i].route.blackhole = false;
  }
}

/**
 * Setup default gateway override routes: a set of 2 /1 routes for the best
 * overall link
 *
 * @param phase the phase of the change (startup/runtime/shutdown)
 */
static void setupDefaultGatewayOverrideRoutes(enum sgw_multi_change_phase phase) {
  bool force = MSGW_ROUTE_FORCED(phase);
  int i;

  if (!bestOverallLinkPrevious.valid && !bestOverallLink.valid && !force) {
    return;
  }

  memcpy(&bestOverallLinkPreviousRoutes, &bestOverallLinkRoutes, sizeof(bestOverallLinkPreviousRoutes));

  determineBestOverallLinkRoutes(bestOverallLinkRoutes);

  for (i = 0; i < 2; i++) {
    bool routeChanged = //
        (bestOverallLinkPreviousRoutes[i].active != bestOverallLinkRoutes[i].active) || //
        memcmp(&bestOverallLinkPreviousRoutes[i].route, &bestOverallLinkRoutes[i].route, sizeof(bestOverallLinkRoutes[i].route));

    if ((routeChanged || MSGW_ROUTE_DEL_FORCED(phase)) && MSGW_ROUTE_DEL_ALLOWED(phase)) {
      programRoute(false, &bestOverallLinkPreviousRoutes[i], "overall best gateway");
    }

    if ((routeChanged || MSGW_ROUTE_ADD_FORCED(phase)) && MSGW_ROUTE_ADD_ALLOWED(phase)) {
      programRoute(true, &bestOverallLinkRoutes[i], "overall best gateway");
    }
  }
}

/**
 * Determine the best egress interface route.
 *
 * @param route a pointer to the an route where to store the
 * newly determined route
 * @param networkRoute true when the route is a network route (not an internet
 * route)
 * @param if_index the index of the interface that the route is for
 * @param gw the gateway for the route
 * @param dst the destination for the route
 * @param table the routing table for the route
 */
static void determineEgressLinkRoute( //
    struct sgw_route_info * route, //
    bool networkRoute, //
    int if_index, //
    union olsr_ip_addr * gw, //
    struct olsr_ip_prefix * dst, //
    int table) {
  // default route
  // ----: ip route replace|delete blackhole default                          table 90: !egress_if               (1)
  // ppp0: ip route replace|delete           default                 dev ppp0 table 90:  egress_if && dst && !gw (2)
  // eth1: ip route replace|delete           default via 192.168.0.1 dev eth1 table 90:  egress_if && dst &&  gw (3)

  // network route
  // eth1: ip route replace|delete to 192.168.0.0/24                 dev eth1 table 90:  egress_if && dst && !gw (2*)

  memset(route, 0, sizeof(*route));
  if (if_index <= 0) { /* 1 */
    route->active = true;
    route->route.family = AF_INET;
    route->route.rttable = table;
    route->route.flags = RTNH_F_ONLINK;
    route->route.scope = RT_SCOPE_UNIVERSE;
    route->route.if_index = 0;
    route->route.metric = 0;
    route->route.protocol = RTPROT_STATIC;
    route->route.srcSet = false;
    route->route.gwSet = false;
    route->route.dstSet = false;
    if (dst) {
      route->route.dstSet = true;
      route->route.dstStore = *dst;
    }
    route->route.del_similar = false;
    route->route.blackhole = true;
  } else if (dst && !gw) { /* 2 */
    route->active = true;
    route->route.family = AF_INET;
    route->route.rttable = table;
    route->route.flags = !networkRoute ? RTNH_F_ONLINK /* 2 */ : 0 /* 2* */;
    route->route.scope = RT_SCOPE_LINK;
    route->route.if_index = if_index;
    route->route.metric = 0;
    route->route.protocol = RTPROT_STATIC;
    route->route.srcSet = false;
    route->route.gwSet = false;
    route->route.dstSet = true;
    route->route.dstStore = *dst;
    route->route.del_similar = false;
    route->route.blackhole = false;
  } else if (dst && gw) { /* 3 */
    route->active = true;
    route->route.family = AF_INET;
    route->route.rttable = table;
    route->route.flags = 0;
    route->route.scope = RT_SCOPE_UNIVERSE;
    route->route.if_index = if_index;
    route->route.metric = 0;
    route->route.protocol = RTPROT_STATIC;
    route->route.srcSet = false;
    route->route.gwSet = true;
    route->route.gwStore = *gw;
    route->route.dstSet = true;
    route->route.dstStore = *dst;
    route->route.del_similar = false;
    route->route.blackhole = false;
  } else {
    /* no destination set */
    route->active = false;
    olsr_syslog(OLSR_LOG_ERR, "No route destination specified in %s", __FUNCTION__);
    return;
  }
}

/**
 * Setup default route for the best egress interface
 *
 * When there is no best egress link, then a blackhole route is setup to prevent
 * looping smart gateway tunnel traffic.
 *
 * @param phase the phase of the change (startup/runtime/shutdown)
 */
static void configureBestEgressLinkRoute(enum sgw_multi_change_phase phase) {
  bool force = MSGW_ROUTE_FORCED(phase);

  /*
   * bestEgressLinkPrevious  bestEgressLink  Action
   *                   NULL            NULL  -
   *                   NULL               x  add new route
   *                      x            NULL  remove old route
   *                      x               x  remove old route && add new routes
   */

  if (!bestEgressLinkPrevious && !bestEgressLink && !force) {
    return;
  }

  memcpy(&bestEgressLinkPreviousRoute, &bestEgressLinkRoute, sizeof(bestEgressLinkPreviousRoute));

  determineEgressLinkRoute( //
      &bestEgressLinkRoute, // route
      false, // networkRoute
      !bestEgressLink ? 0 : bestEgressLink->if_index, // if_index
      (!bestEgressLink || !bestEgressLink->bwCurrent.gatewaySet) ? NULL : &bestEgressLink->bwCurrent.gateway, // gw
      &ipv4_slash_0_route, // dst
      olsr_cnf->smart_gw_offset_tables // table
      );

  {
    bool routeChanged = //
        (bestEgressLinkPreviousRoute.active != bestEgressLinkRoute.active) || //
        memcmp(&bestEgressLinkPreviousRoute.route, &bestEgressLinkRoute.route, sizeof(bestEgressLinkRoute.route));

    if ((routeChanged || MSGW_ROUTE_DEL_FORCED(phase)) && MSGW_ROUTE_DEL_ALLOWED(phase)) {
      programRoute(false, &bestEgressLinkPreviousRoute, "best egress link");
    }

    if ((routeChanged || MSGW_ROUTE_ADD_FORCED(phase)) && MSGW_ROUTE_ADD_ALLOWED(phase)) {
      programRoute(true, &bestEgressLinkRoute, "best egress link");
    }
  }
}

/**
 * Setup network (when relevant) and default routes for the every egress
 * interface
 *
 * @param phase the phase of the change (startup/runtime/shutdown)
 */
static void configureEgressLinkRoutes(enum sgw_multi_change_phase phase) {
  bool force = MSGW_ROUTE_FORCED(phase);

  /* egress interfaces */
  struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
  while (egress_if) {
    if (!egress_if->bwNetworkChanged && !egress_if->bwGatewayChanged && !egress_if->upChanged && !force) {
      /* no relevant change */
      goto next;
    }

    /* network route */
    if (egress_if->bwNetworkChanged || force) {
      bool routeChanged;

      struct sgw_route_info networkRoutePrevious = egress_if->networkRouteCurrent;

      if (!egress_if->bwCurrent.networkSet || !egress_if->upCurrent || (egress_if->bwCurrent.costs == INT64_MAX)) {
        memset(&egress_if->networkRouteCurrent, 0, sizeof(egress_if->networkRouteCurrent));
        egress_if->networkRouteCurrent.active = false;
      } else {
        determineEgressLinkRoute( //
            &egress_if->networkRouteCurrent, // route
            true,// networkRoute
            egress_if->if_index, // if_index
            NULL, // gw
            &egress_if->bwCurrent.network, // dst
            egress_if->tableNr // table
            );
      }

      routeChanged = //
          (networkRoutePrevious.active != egress_if->networkRouteCurrent.active) || //
          memcmp(&networkRoutePrevious.route, &egress_if->networkRouteCurrent.route, sizeof(egress_if->networkRouteCurrent.route));

      if ((routeChanged || MSGW_ROUTE_DEL_FORCED(phase)) && MSGW_ROUTE_DEL_ALLOWED(phase)) {
        programRoute(false, &networkRoutePrevious, egress_if->name);
      }

      if ((routeChanged || MSGW_ROUTE_ADD_FORCED(phase)) && MSGW_ROUTE_ADD_ALLOWED(phase)) {
        programRoute(true, &egress_if->networkRouteCurrent, egress_if->name);
      }
    }

    /* default route */
    if (egress_if->bwGatewayChanged || force) {
      bool routeChanged;

      struct sgw_route_info egressRoutePrevious = egress_if->egressRouteCurrent;

      if (!egress_if->upCurrent || (egress_if->bwCurrent.costs == INT64_MAX)) {
        memset(&egress_if->egressRouteCurrent, 0, sizeof(egress_if->egressRouteCurrent));
        egress_if->egressRouteCurrent.active = false;
      } else {
        determineEgressLinkRoute( //
            &egress_if->egressRouteCurrent, // route
            false,// networkRoute
            egress_if->if_index, // if_index
            !egress_if->bwCurrent.gatewaySet ? NULL : &egress_if->bwCurrent.gateway, // gw
            &ipv4_slash_0_route, // dst
            egress_if->tableNr // table
            );
      }

      routeChanged = //
          (egressRoutePrevious.active != egress_if->egressRouteCurrent.active) || //
          memcmp(&egressRoutePrevious, &egress_if->egressRouteCurrent, sizeof(egress_if->egressRouteCurrent));

       if ((routeChanged || MSGW_ROUTE_DEL_FORCED(phase)) && MSGW_ROUTE_DEL_ALLOWED(phase)) {
         programRoute(false, &egressRoutePrevious, egress_if->name);
       }

       if ((routeChanged || MSGW_ROUTE_ADD_FORCED(phase)) && MSGW_ROUTE_ADD_ALLOWED(phase)) {
         programRoute(true, &egress_if->egressRouteCurrent, egress_if->name);
       }
    }

    next: egress_if = egress_if->next;
  }
}

/*
 * Multi-Smart-Gateway Status Overview
 */

#define IPNONE    ((olsr_cnf->ip_version == AF_INET) ? "0.0.0.0"     : "::")
#define MASKNONE  ((olsr_cnf->ip_version == AF_INET) ? "0.0.0.0/0"   : "::/0")
#define IPLOCAL   ((olsr_cnf->ip_version == AF_INET) ? "127.0.0.1"   : "::1")
#define MASKLOCAL ((olsr_cnf->ip_version == AF_INET) ? "127.0.0.0/8" : "::1/128")

/**
 * Print a timestamp to a file
 *
 * @param f the file
 */
static void printDate(FILE * f) {
  time_t timer;
  struct tm* tm_info;
  char buffer[64];

  time(&timer);
  tm_info = localtime(&timer);

  strftime(buffer, sizeof(buffer), "%B %d, %Y at %H:%M:%S", tm_info);
  fprintf(f, "%s", buffer);
}

/**
 * Write multi-smart-gateway status file
 *
 * <pre>
 * # multi-smart-gateway status overview, generated on October 10, 2014 at 08:27:15
 *
 * #Originator Prefix      Uplink Downlink PathCost   Type   Interface Gateway     Cost
 *  127.0.0.1  127.0.0.0/8 0      0        4294967295 egress ppp0      0.0.0.0     9223372036854775807
 *  127.0.0.1  127.0.0.0/8 0      0        4294967295 egress eth1      192.168.0.1 9223372036854775807
 * *0.0.0.0    0.0.0.0/0   290    1500     1024       olsr   tnl_4094  0.0.0.0     2182002287
 * </pre>
 *
 * @param phase the phase of the change (startup/runtime/shutdown)
 */
static void writeProgramStatusFile(enum sgw_multi_change_phase phase) {
  /*                                # Origi Prefx Upln Dwnl PathC Type Intfc Gtway Cost */
  static const char * fmt_header = "%s%-15s %-18s %-9s %-9s %-10s %-6s %-16s %-15s %s\n";
  static const char * fmt_values = "%s%-15s %-18s %-9u %-9u %-10u %-6s %-16s %-15s %lld\n";

  char * fileName = olsr_cnf->smart_gw_status_file;
  FILE * fp = NULL;

  if (!fileName || (fileName[0] == '\0')) {
    return;
  }

  if (phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN) {
    remove(fileName);
    return;
  }

  fp = fopen(fileName, "w");
  if (!fp) {
    olsr_syslog(OLSR_LOG_ERR, "Could not write to %s", fileName);
    return;
  }

  fprintf(fp, "# OLSRd Multi-Smart-Gateway Status Overview\n");
  fprintf(fp, "# Generated on ");
  printDate(fp);
  fprintf(fp, "\n\n");

  /* header */
  fprintf(fp, fmt_header, "#", "Originator", "Prefix", "Uplink", "Downlink", "PathCost", "Type", "Interface", "Gateway", "Cost");

  /* egress interfaces */
  {
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      struct ipaddr_str gwStr;
      const char * gw = !egress_if->bwCurrent.gatewaySet ? IPNONE : olsr_ip_to_string(&gwStr, &egress_if->bwCurrent.gateway);
      bool selected = bestOverallLink.valid && !bestOverallLink.isOlsr && (bestOverallLink.link.egress == egress_if);

      fprintf(fp, fmt_values, //
          selected ? "*" : " ", //selected
          IPLOCAL, // Originator
          MASKLOCAL, // Prefix
          egress_if->bwCurrent.egressUk, // Uplink
          egress_if->bwCurrent.egressDk, // Downlink
          egress_if->bwCurrent.path_cost, // PathCost
          "egress", // Type
          egress_if->name, // Interface
          gw, // Gateway
          egress_if->bwCurrent.costs // Cost
          );

      egress_if = egress_if->next;
    }
  }

  /* olsr */
  {
    struct gateway_entry * current_gw = olsr_get_inet_gateway((olsr_cnf->ip_version == AF_INET) ? false : true);
    struct interfaceName * sgwTunnelInterfaceNames = (olsr_cnf->ip_version == AF_INET) ? sgwTunnel4InterfaceNames : sgwTunnel6InterfaceNames;

    int i = 0;
    for (i = 0; i < olsr_cnf->smart_gw_use_count; i++) {
      struct interfaceName * node = &sgwTunnelInterfaceNames[i];
      struct gateway_entry * gw = node->gw;
      struct tc_entry* tc = !gw ? NULL : olsr_lookup_tc_entry(&gw->originator);

      struct ipaddr_str originatorStr;
      const char * originator = !gw ? IPNONE : olsr_ip_to_string(&originatorStr, &gw->originator);
      struct ipaddr_str prefixIpStr;
      const char * prefixIPStr = !gw ? IPNONE : olsr_ip_to_string(&prefixIpStr, &gw->external_prefix.prefix);
      uint8_t prefix_len = !gw ? 0 : gw->external_prefix.prefix_len;
      struct ipaddr_str tunnelGwStr;
      const char * tunnelGw = !gw ? IPNONE : olsr_ip_to_string(&tunnelGwStr, &gw->originator);
      bool selected = bestOverallLink.valid && bestOverallLink.isOlsr && current_gw && (current_gw == gw);

      char prefix[strlen(prefixIPStr) + 1 + 3 + 1];
      snprintf(prefix, sizeof(prefix), "%s/%d", prefixIPStr, prefix_len);

      fprintf(fp, fmt_values, //
          selected ? "*" : " ", // selected
          originator, // Originator
          !gw ? MASKNONE : prefix, // Prefix IP
          !gw ? 0 : gw->uplink, // Uplink
          !gw ? 0 : gw->downlink, // Downlink
          (!gw || !tc) ? ROUTE_COST_BROKEN : tc->path_cost, // PathCost
          "olsr", // Type
          node->name, // Interface
          tunnelGw, // Gateway
          !gw ? INT64_MAX : gw->path_cost // Cost
      );
    }
  }

  fclose(fp);
}

/**
 * Report a new gateway with its most relevant parameters in the syslog
 */
static void reportNewGateway(void) {
  if (!bestOverallLink.valid) {
    /* best overall link is invalid (none) */
    olsr_syslog(OLSR_LOG_INFO, "New gateway selected: none");
    return;
  }

  if (!bestOverallLink.isOlsr) {
    /* best overall link is an egress interface */
    struct ipaddr_str gwStr;
    const char * gw = !bestOverallLink.link.egress->bwCurrent.gatewaySet ? //
        NULL : //
        olsr_ip_to_string(&gwStr, &bestOverallLink.link.egress->bwCurrent.gateway);

    olsr_syslog(OLSR_LOG_INFO, "New gateway selected: %s %s%s%swith uplink/downlink/pathcost = %u/%u/%u", //
        bestOverallLink.link.egress->name, //
        !gw ? "" : "via ", //
        !gw ? "" : gwStr.buf, //
        !gw ? "" : " ", //
        bestOverallLink.link.egress->bwCurrent.egressUk, //
        bestOverallLink.link.egress->bwCurrent.egressDk, //
        bestOverallLink.link.egress->bwCurrent.path_cost);
    return;
  }

  /* best overall link is an olsr (tunnel) interface */
  {
    struct tc_entry* tc = olsr_lookup_tc_entry(&bestOverallLink.link.olsr->originator);

    char ifNameBuf[IFNAMSIZ];
    const char * ifName = if_indextoname(bestOverallLink.olsrTunnelIfIndex, ifNameBuf);

    struct ipaddr_str gwStr;
    const char * gw = olsr_ip_to_string(&gwStr, &bestOverallLink.link.olsr->originator);

    olsr_syslog(OLSR_LOG_INFO, "New gateway selected: %s %s%s%swith uplink/downlink/pathcost = %u/%u/%u", //
        !ifName ? "none" : ifName, //
        !gw ? "" : "via ", //
        !gw ? "" : gwStr.buf, //
        !gw ? "" : " ", //
        bestOverallLink.link.olsr->uplink, //
        bestOverallLink.link.olsr->downlink, //
        !tc ? ROUTE_COST_BROKEN : tc->path_cost);
  }
}

/**
 * Process changes that are relevant to egress interface: changes to the
 * egress interfaces themselves and to the smart gateway that is chosen by olsrd
 *
 * @param egressChanged true when an egress interface changed
 * @param olsrChanged true when the smart gateway changed
 * @param phase the phase of the change (startup/runtime/shutdown)
 */
void doRoutesMultiGw(bool egressChanged, bool olsrChanged, enum sgw_multi_change_phase phase) {
  bool bestEgressChanged = false;
  bool bestOverallChanged = false;
  bool force = MSGW_ROUTE_FORCED(phase);

  assert( //
      (phase == GW_MULTI_CHANGE_PHASE_STARTUP) || //
      (phase == GW_MULTI_CHANGE_PHASE_RUNTIME) || //
      (phase == GW_MULTI_CHANGE_PHASE_SHUTDOWN));

  if (!egressChanged && !olsrChanged && !force) {
    goto out;
  }

  assert(multi_gateway_mode());

  if (egressChanged || force) {
    bestEgressChanged = determineBestEgressLink(phase);
    configureEgressLinkRoutes(phase);
  }

  if (olsrChanged || bestEgressChanged || force) {
    bestOverallChanged = determineBestOverallLink(phase);
  }

  if (bestOverallChanged || force) {
    setupDefaultGatewayOverrideRoutes(phase);
  }

  if (bestEgressChanged || force) {
    configureBestEgressLinkRoute(phase);
  }

  if (bestOverallChanged || force) {
    reportNewGateway();
  }

  writeProgramStatusFile(phase);

  out: if (egressChanged) {
    /* clear the 'changed' flags of egress interfaces */
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      egress_if->upChanged = false;

      egress_if->bwCostsChanged = false;
      egress_if->bwNetworkChanged = false;
      egress_if->bwGatewayChanged = false;
      egress_if->bwChanged = false;

      egress_if = egress_if->next;
    }
  }
}

#endif /* __linux__ */
