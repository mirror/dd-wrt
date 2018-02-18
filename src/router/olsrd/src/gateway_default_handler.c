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
#ifdef __linux__

#include "gateway_default_handler.h"

#include "gateway_costs.h"
#include "defs.h"
#include "gateway.h"
#include "lq_plugin.h"

static uint32_t gw_def_nodecount;
static uint32_t gw_def_stablecount;
static bool gw_def_choose_new_ipv4_gw;
static bool gw_def_choose_new_ipv6_gw;
static struct timer_entry *gw_def_timer;

/* forward declarations */
static void gw_default_init(void);
static void gw_default_cleanup(void);
static void gw_default_startup_handler(void);
static int64_t gw_default_getcosts(struct gateway_entry *gw);
static void gw_default_choosegw_handler(bool ipv4, bool ipv6);
static void gw_default_update_handler(struct gateway_entry *);
static void gw_default_delete_handler(struct gateway_entry *);

/**
 * Callback list for the gateway (default) handler
 */
struct olsr_gw_handler gw_def_handler = {
    &gw_default_init,
    &gw_default_cleanup,
    &gw_default_startup_handler,
    &gw_default_getcosts,
    &gw_default_choosegw_handler,
    &gw_default_update_handler,
    &gw_default_delete_handler
};

/*
 * Helper functions
 */

/**
 * Calculate the threshold path cost.
 *
 * @param path_cost the path cost
 * @return the threshold path cost
 */
static INLINE int64_t gw_default_calc_threshold(int64_t path_cost) {
  if (olsr_cnf->smart_gw_thresh == 0) {
    return path_cost;
  }

  return ((path_cost * olsr_cnf->smart_gw_thresh) + 50) / 100;
}

/**
 * Look through the gateway list and select the best gateway
 * depending on the costs
 */
static void gw_default_choose_gateway(void) {
  int64_t cost_ipv4_threshold = INT64_MAX;
  int64_t cost_ipv6_threshold = INT64_MAX;
  bool cost_ipv4_threshold_valid = false;
  bool cost_ipv6_threshold_valid = false;
  struct gateway_entry *chosen_gw_ipv4 = NULL;
  struct gateway_entry *chosen_gw_ipv6 = NULL;
  struct gateway_entry *gw;
  bool dual = false;

  if (olsr_cnf->smart_gw_thresh) {
    /* determine the path cost thresholds */

    struct gateway_entry * current_gw = olsr_get_inet_gateway(false);
    if (current_gw) {
      cost_ipv4_threshold = gw_default_calc_threshold(current_gw->path_cost);
      cost_ipv4_threshold_valid = true;
    }

    current_gw = olsr_get_inet_gateway(true);
    if (current_gw) {
      cost_ipv6_threshold = gw_default_calc_threshold(current_gw->path_cost);
      cost_ipv6_threshold_valid = true;
    }
  }

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    int64_t gw_cost = gw->path_cost;

    if (gw_cost == INT64_MAX) {
      /* never select a node with infinite costs */
      continue;
    }

    if (gw_def_choose_new_ipv4_gw) {
      bool gw_eligible_v4 = isGwSelectable(gw, false) ;
      if (gw_eligible_v4 && gw_cost < (chosen_gw_ipv4 ? chosen_gw_ipv4->path_cost : INT64_MAX)
          && (!cost_ipv4_threshold_valid || (gw_cost < cost_ipv4_threshold))) {
        chosen_gw_ipv4 = gw;
      }
    }

    if (gw_def_choose_new_ipv6_gw) {
      bool gw_eligible_v6 = isGwSelectable(gw, true);
      if (gw_eligible_v6 && gw_cost < (chosen_gw_ipv6 ? chosen_gw_ipv6->path_cost : INT64_MAX)
          && (!cost_ipv6_threshold_valid || (gw_cost < cost_ipv6_threshold))) {
        chosen_gw_ipv6 = gw;
      }
    }
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)

  /* determine if we should keep looking for IPv4 and/or IPv6 gateways */
  gw_def_choose_new_ipv4_gw = gw_def_choose_new_ipv4_gw && (chosen_gw_ipv4 == NULL);
  gw_def_choose_new_ipv6_gw = gw_def_choose_new_ipv6_gw && (chosen_gw_ipv6 == NULL);

  /* determine if we are dealing with a dual stack gateway */
  dual = chosen_gw_ipv4 && (chosen_gw_ipv4 == chosen_gw_ipv6);

  if (chosen_gw_ipv4) {
    /* we are dealing with an IPv4 or dual stack gateway */
    olsr_set_inet_gateway(chosen_gw_ipv4, true, dual);
  }
  if (chosen_gw_ipv6 && !dual) {
    /* we are dealing with an IPv6-only gateway */
    olsr_set_inet_gateway(chosen_gw_ipv6, false, true);
  }

  if ((olsr_cnf->smart_gw_thresh == 0) && !gw_def_choose_new_ipv4_gw && !gw_def_choose_new_ipv6_gw) {
    /* stop looking for a better gateway */
    olsr_stop_timer(gw_def_timer);
    gw_def_timer = NULL;
  }
}

/**
 * Timer callback for lazy gateway selection
 *
 * @param unused unused
 */
static void gw_default_timer(void *unused __attribute__ ((unused))) {
  /* accept a 10% increase/decrease in the number of gateway nodes without triggering a stablecount reset */
  unsigned int tree100percent = tc_tree.count * 10;
  uint32_t nodecount090percent = gw_def_nodecount * 9;
  uint32_t nodecount110percent = gw_def_nodecount * 11;
  if ((tree100percent >= nodecount090percent) && (tree100percent <= nodecount110percent)) {
    gw_def_nodecount = tc_tree.count;
  }

  if (tc_tree.count == gw_def_nodecount) {
    /* the number of gateway nodes is 'stable' */
    gw_def_stablecount++;
  } else {
    /* there was a significant change in the number of gateway nodes */
    gw_def_nodecount = tc_tree.count;
    gw_def_stablecount = 0;
  }

  if (gw_def_stablecount >= olsr_cnf->smart_gw_stablecount) {
    /* the number of gateway nodes is stable enough, so we should select a new gateway now */
    gw_default_choose_gateway();
  }
}

/**
 * Lookup a new gateway
 *
 * @param ipv4 lookup new v4 gateway
 * @param ipv6 lookup new v6 gateway
 */
static void gw_default_lookup_gateway(bool ipv4, bool ipv6) {
  if (ipv4) {
    /* get a new IPv4 gateway if we use OLSRv4 or NIIT */
    gw_def_choose_new_ipv4_gw = (olsr_cnf->ip_version == AF_INET) || olsr_cnf->use_niit;
  }
  if (ipv6) {
    /* get a new IPv6 gateway if we use OLSRv6 */
    gw_def_choose_new_ipv6_gw = olsr_cnf->ip_version == AF_INET6;
  }

  if (gw_def_choose_new_ipv4_gw || gw_def_choose_new_ipv6_gw) {
    gw_default_choose_gateway();
  }
}

/*
 * Exported functions
 */

/*
 * Handler functions
 */

/**
 * initialization of default gateway handler
 */
static void gw_default_init(void) {
  /* initialize values */
  gw_def_nodecount = 0;
  gw_def_stablecount = 0;
  gw_def_choose_new_ipv4_gw = true;
  gw_def_choose_new_ipv6_gw = true;
  gw_def_timer = NULL;
}

/**
 * Cleanup default gateway handler
 */
static void gw_default_cleanup(void) {
}

/**
 * Handle gateway startup
 */
static void gw_default_startup_handler(void) {
  /* reset node count */
  gw_def_nodecount = tc_tree.count;
  gw_def_stablecount = 0;

  /* get a new IPv4 gateway if we use OLSRv4 or NIIT */
  gw_def_choose_new_ipv4_gw = (olsr_cnf->ip_version == AF_INET) || olsr_cnf->use_niit;

  /* get a new IPv6 gateway if we use OLSRv6 */
  gw_def_choose_new_ipv6_gw = olsr_cnf->ip_version == AF_INET6;

  /* keep in mind we might be a gateway ourself */
  gw_def_choose_new_ipv4_gw = gw_def_choose_new_ipv4_gw && !olsr_cnf->has_ipv4_gateway;
  gw_def_choose_new_ipv6_gw = gw_def_choose_new_ipv6_gw && !olsr_cnf->has_ipv6_gateway;

  /* (re)start gateway lazy selection timer */
  olsr_set_timer(&gw_def_timer, olsr_cnf->smart_gw_period, 0, true, &gw_default_timer, NULL, 0);
}

/**
 * Called when the costs of a gateway must be determined.
 *
 * @param gw the gateway
 * @return the costs, or INT64_MAX in case the gateway is null or has infinite costs
 */
static int64_t gw_default_getcosts(struct gateway_entry *gw) {
  struct tc_entry* tc;

  if (!gw) {
    return INT64_MAX;
  }

  tc = olsr_lookup_tc_entry(&gw->originator);

  if (!tc || (tc->path_cost >= ROUTE_COST_BROKEN) || (!gw->uplink || !gw->downlink)) {
    /* gateways should not exist without tc entry */
    /* do not consider nodes with an infinite ETX */
    /* do not consider nodes without bandwidth or with a uni-directional link */
    return INT64_MAX;
  }

  /* determine the path cost */
  return gw_costs_weigh(true, tc->path_cost, gw->uplink, gw->downlink);
}

/**
 * Choose a new gateway
 *
 * @param ipv4 lookup new v4 gateway
 * @param ipv6 lookup new v6 gateway
 */
static void gw_default_choosegw_handler(bool ipv4, bool ipv6) {
  gw_default_lookup_gateway(ipv4, ipv6);

  if (gw_def_choose_new_ipv4_gw || gw_def_choose_new_ipv6_gw) {
    gw_default_startup_handler();
  }
}

/**
 * Update a gateway entry
 *
 * @param gw the gateway entry
 */
static void gw_default_update_handler(struct gateway_entry *gw) {
  if (olsr_cnf->smart_gw_thresh == 0) {
    /* classical behaviour: stick with the chosen gateway unless it changes */
    bool v4changed = gw && (gw == olsr_get_inet_gateway(false))
        && (!gw->ipv4 || (gw->ipv4nat && !olsr_cnf->smart_gw_allow_nat));
    bool v6changed = gw && (gw == olsr_get_inet_gateway(true)) && !gw->ipv6;

    if (v4changed || v6changed) {
      gw_default_lookup_gateway(v4changed, v6changed);
    }
  } else {
    /* new behaviour: always pick a new gateway */
    gw_default_lookup_gateway(true, true);
  }
}

/**
 * Remove a gateway entry
 *
 * @param gw the gateway entry
 */
static void gw_default_delete_handler(struct gateway_entry *gw) {
  bool isv4 = gw && (gw == olsr_get_inet_gateway(false));
  bool isv6 = gw && (gw == olsr_get_inet_gateway(true));

  if (isv4 || isv6) {
    gw_default_lookup_gateway(isv4, isv6);
  }
}
#endif /* __linux__ */
