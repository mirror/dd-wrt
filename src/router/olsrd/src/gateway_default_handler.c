/*
 * gateway_default_handler.c
 *
 *  Created on: Jan 29, 2010
 *      Author: rogge
 */

#include "defs.h"
#include "gateway.h"
#include "gateway_default_handler.h"
#include "scheduler.h"
#include "tc_set.h"
#include "log.h"
#include "lq_plugin.h"

#include "assert.h"

#ifdef LINUX_NETLINK_ROUTING
static uint32_t gw_def_nodecount, gw_def_stablecount;
static bool gw_def_finished_ipv4, gw_def_finished_ipv6;

static struct timer_entry *gw_def_timer;

static void gw_default_startup_handler(void);
static void gw_default_choosegw_handler(bool ipv4, bool ipv6);
static void gw_default_update_handler(struct gateway_entry *);
static void gw_default_delete_handler(struct gateway_entry *);

static struct olsr_gw_handler gw_def_handler = {
  &gw_default_startup_handler,
  &gw_default_choosegw_handler,
  &gw_default_update_handler,
  &gw_default_delete_handler
};

/**
 * Look through the gateway list and select the best gateway
 * depending on the distance to this router
 */
static void gw_default_choose_gateway(void) {
  struct tc_entry *tc;
  struct gateway_entry *inet_ipv4, *inet_ipv6;
  olsr_linkcost cost_ipv4, cost_ipv6;
  struct gateway_entry *gw;
  bool dual;

  cost_ipv4 = ROUTE_COST_BROKEN;
  cost_ipv6 = ROUTE_COST_BROKEN;

  inet_ipv4 = NULL;
  inet_ipv6 = NULL;

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    /* gateways should not exist without tc entry */
    if ((tc = olsr_lookup_tc_entry(&gw->originator)) == NULL) {
      continue;
    }

    if (!gw_def_finished_ipv4 && gw->ipv4 && gw->ipv4nat == olsr_cnf->smart_gw_allow_nat && tc->path_cost < cost_ipv4) {
      inet_ipv4 = gw;
      cost_ipv4 = tc->path_cost;
    }
    if (!gw_def_finished_ipv6 && gw->ipv6 && tc->path_cost < cost_ipv6) {
      inet_ipv6 = gw;
      cost_ipv6 = tc->path_cost;
    }
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)

  /* found an IPv4 gateway ? */
  gw_def_finished_ipv4 |= inet_ipv4 != NULL;
  gw_def_finished_ipv6 |= inet_ipv6 != NULL;
  dual = inet_ipv4 == inet_ipv6;
  if (inet_ipv4) {
    olsr_set_inet_gateway(&inet_ipv4->originator, true, dual, false);
  }
  if (inet_ipv6 && !dual) {
    olsr_set_inet_gateway(&inet_ipv6->originator, false, true, false);
  }

  /* finished ? */
  if (gw_def_finished_ipv4 && gw_def_finished_ipv6) {
    olsr_stop_timer(gw_def_timer);
    gw_def_timer = NULL;
  }
}

/* timer for laze gateway selection */
static void gw_default_timer(void *unused __attribute__ ((unused))) {
  /* accept a 10% increase without trigger a stablecount reset */
  if (tc_tree.count * 10 <= gw_def_nodecount * 11) {
    gw_def_nodecount = tc_tree.count;
  }
  if (tc_tree.count <= gw_def_nodecount) {
    gw_def_stablecount++;
  }
  else {
    gw_def_nodecount = tc_tree.count;
    gw_def_stablecount = 0;
  }

  if (gw_def_stablecount >= GW_DEFAULT_STABLE_COUNT) {
    gw_default_choose_gateway();
  }
}

/* gateway handler callbacks */
static void gw_default_startup_handler(void) {
  /* reset node count */
  gw_def_nodecount = tc_tree.count;
  gw_def_stablecount = 0;

  /* get new ipv4 GW if we use OLSRv4 or NIIT */
  gw_def_finished_ipv4 = olsr_cnf->ip_version == AF_INET6 && !olsr_cnf->use_niit;

  /* get new ipv6 GW if we use OLSRv6 */
  gw_def_finished_ipv6 = olsr_cnf->ip_version == AF_INET;

  /* keep in mind we might be a gateway ourself */
  gw_def_finished_ipv4 |= olsr_cnf->has_ipv4_gateway;
  gw_def_finished_ipv6 |= olsr_cnf->has_ipv6_gateway;

  /* start gateway selection timer */
  olsr_set_timer(&gw_def_timer, GW_DEFAULT_TIMER_INTERVAL, 0, true, &gw_default_timer, NULL, 0);
}

static void gw_default_update_handler(struct gateway_entry *gw) {
  bool v4changed, v6changed;

  v4changed = (gw == olsr_get_ipv4_inet_gateway(NULL))
      && (!gw->ipv4 || (gw->ipv4nat && !olsr_cnf->smart_gw_allow_nat));
  v6changed = (gw == olsr_get_ipv6_inet_gateway(NULL)) && !gw->ipv6;

  if (v4changed || v6changed) {
    olsr_gw_default_lookup_gateway(v4changed, v6changed);
  }
}

static void gw_default_delete_handler(struct gateway_entry *gw) {
  bool isv4, isv6;

  isv4 = gw == olsr_get_ipv4_inet_gateway(NULL);
  isv6 = gw == olsr_get_ipv6_inet_gateway(NULL);

  if (gw != NULL && (isv4 || isv6)) {
    olsr_gw_default_lookup_gateway(isv4, isv6);
  }
}

static void gw_default_choosegw_handler(bool ipv4, bool ipv6) {
  olsr_gw_default_lookup_gateway(ipv4, ipv6);

  if (!(gw_def_finished_ipv4 && gw_def_finished_ipv6)) {
    gw_default_startup_handler();
  }
}

/**
 * initialization of default gateway handler
 */
void olsr_gw_default_init(void) {
  /* initialize values */
  gw_def_timer = NULL;
  gw_def_finished_ipv4 = false;
  gw_def_finished_ipv6 = false;
  gw_def_nodecount = 0;
  gw_def_stablecount = 0;
  gw_def_timer = NULL;

  /* setup default handler */
  olsr_set_inetgw_handler(&gw_def_handler);
}

/**
 * Lookup a new gateway based on distance metric
 *
 * @param ipv4 lookup new v4 gateway
 * @param ipv6 lookup new v6 gateway
 */
void olsr_gw_default_lookup_gateway(bool ipv4, bool ipv6) {
  if (ipv4) {
    /* get new ipv4 GW if we use OLSRv4 or NIIT */
    gw_def_finished_ipv4 = olsr_cnf->ip_version == AF_INET6 && !olsr_cnf->use_niit;
  }
  if (ipv6) {
    /* get new ipv6 GW if we use OLSRv6 */
    gw_def_finished_ipv6 = olsr_cnf->ip_version == AF_INET;
  }

  if (!(gw_def_finished_ipv4 && gw_def_finished_ipv6)) {
    gw_default_choose_gateway();
  }
}
#endif
