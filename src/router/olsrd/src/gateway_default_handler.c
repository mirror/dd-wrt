/*
 * gateway_default_handler.c
 *
 *  Created on: Jan 29, 2010
 *      Author: rogge
 */
#ifdef linux

#include "gateway_default_handler.h"

#include "defs.h"
#include "gateway.h"
#include "lq_plugin.h"

static uint32_t gw_def_nodecount;
static uint32_t gw_def_stablecount;
static bool gw_def_finished_ipv4;
static bool gw_def_finished_ipv6;
static struct timer_entry *gw_def_timer;

/* forward declarations */
static void gw_default_startup_handler(void);
static void gw_default_choosegw_handler(bool ipv4, bool ipv6);
static void gw_default_update_handler(struct gateway_entry *);
static void gw_default_delete_handler(struct gateway_entry *);

/**
 * Callback list for the gateway (default) handler
 */
static struct olsr_gw_handler gw_def_handler = {
  &gw_default_startup_handler,
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
static inline uint64_t gw_default_calc_threshold(uint64_t path_cost) {
  uint64_t path_cost_times_threshold;

  if (olsr_cnf->smart_gw_thresh == 0) {
    path_cost_times_threshold = path_cost;
  } else {
    path_cost_times_threshold = (path_cost * (uint64_t)olsr_cnf->smart_gw_thresh + (uint64_t)50) / (uint64_t)100;
  }

  return path_cost_times_threshold;
}

/**
 * Weigh the path costs and the gateway bandwidth.
 *
 * If the ETX divider is zero, then no weighing is performed and only the path
 * costs are considered (classic behaviour).
 *
 * If either of the uplink or downlink bandwidths is zero, then UINT64_MAX is
 * returned.
 *
 * @param path_cost the (ETX) path cost to the gateway
 * @param exitUk the gateway exit link uplink bandwidth (in kbps)
 * @param exitDk the gateway exit link downlink bandwidth (in kbps)
 * @return the weighed path cost
 */
static inline uint64_t gw_default_weigh_costs(uint64_t path_cost, uint32_t exitUk, uint32_t exitDk) {
	uint8_t WexitU = olsr_cnf->smart_gw_weight_exitlink_up;
	uint8_t WexitD = olsr_cnf->smart_gw_weight_exitlink_down;
	uint8_t Wetx = olsr_cnf->smart_gw_weight_etx;
	uint8_t Detx = olsr_cnf->smart_gw_divider_etx;
	uint64_t costU;
	uint64_t costD;
	uint64_t costE;

	if (!Detx) {
		/* only consider path costs (classic behaviour) */
		return path_cost;
	}

	if (!exitUk || !exitDk) {
		/* zero bandwidth */
		return UINT64_MAX;
	}

	/*
	 * Weighing of the path costs:
	 *
	 * exitUm = the gateway exit link uplink   bandwidth, in Mbps
	 * exitDm = the gateway exit link downlink bandwidth, in Mbps
	 * WexitU = the gateway exit link uplink   bandwidth weight   (configured)
	 * WexitD = the gateway exit link downlink bandwidth weight   (configured)
	 * Wetx   = the ETX path cost weight                          (configured)
	 * Detx   = the ETX path cost divider                         (configured)
	 *
	 *                      WexitU   WexitD   Wetx
	 * path_cost_weighed =  ------ + ------ + ---- * path_cost
	 *                      exitUm   exitDm   Detx
	 *
	 * Since the gateway exit link bandwidths are in Kbps, the following formula
	 * is used to convert them to the desired Mbps:
	 *
	 *       bwK
	 * bwM = ----       bwK = bandwidth in Kbps
	 *       1000       bwM = bandwidth in Mbps
	 *
	 * exitUm = the gateway exit link uplink   bandwidth, in Kbps
	 * exitDm = the gateway exit link downlink bandwidth, in Kbps
	 *
	 *                      1000 * WexitU   1000 * WexitD   Wetx
	 * path_cost_weighed =  ------------- + ------------- + ---- * path_cost
	 *                          exitUk          exitDk      Detx
	 *
	 *
	 * Analysis of the required bit width of the result:
	 *
	 * exitUk    = 29 bits = [1,   320,000,000]
	 * exitDk    = 29 bits = [1,   320,000,000]
	 * WexitU    =  8 bits = [1,           255]
	 * WexitD    =  8 bits = [1,           255]
	 * Wetx      =  8 bits = [1,           255]
	 * Detx      =  8 bits = [1,           255]
	 * path_cost = 32 bits = [1, 4,294,967,295]
	 *
	 *                          1000 * 255   1000 * 255   255
	 * path_cost_weighed(max) = ---------- + ---------- + --- * 4,294,967,295
	 *                               1             1       1
	 *
	 * path_cost_weighed(max) = 0x3E418    + 0x3E418    + 0xFEFFFFFF01
	 * path_cost_weighed(max) = 0xFF0007C731
	 *
	 * Because we can multiply 0xFF0007C731 by 2^24 without overflowing a
	 * 64 bits number, we do this to increase accuracy.
	 */

	costU = (((uint64_t)(1000 * WexitU   )) << 24) / exitUk;
	costD = (((uint64_t)(1000 * WexitD   )) << 24) / exitDk;
	costE = (((uint64_t)(Wetx * path_cost)) << 24) / Detx;

	return (costU + costD + costE);
}

/**
 * Look through the gateway list and select the best gateway
 * depending on the distance to this router
 */
static void gw_default_choose_gateway(void) {
  uint64_t cost_ipv4_threshold = UINT64_MAX;
  uint64_t cost_ipv6_threshold = UINT64_MAX;
  bool eval_cost_ipv4_threshold = false;
  bool eval_cost_ipv6_threshold = false;
  struct gateway_entry *inet_ipv4 = NULL;
  struct gateway_entry *inet_ipv6 = NULL;
  uint64_t cost_ipv4 = UINT64_MAX;
  uint64_t cost_ipv6 = UINT64_MAX;
  struct gateway_entry *gw;
  struct tc_entry *tc;
  bool dual;

  if (olsr_cnf->smart_gw_thresh) {
    /* determine the path cost thresholds */

    gw = olsr_get_ipv4_inet_gateway(NULL);
    if (gw) {
      tc = olsr_lookup_tc_entry(&gw->originator);
      if (tc) {
        uint64_t cost = gw_default_weigh_costs(tc->path_cost, gw->uplink, gw->downlink);
        cost_ipv4_threshold = gw_default_calc_threshold(cost);
        eval_cost_ipv4_threshold = true;
      }
    }
    gw = olsr_get_ipv6_inet_gateway(NULL);
    if (gw) {
      tc = olsr_lookup_tc_entry(&gw->originator);
      if (tc) {
        uint64_t cost = gw_default_weigh_costs(tc->path_cost, gw->uplink, gw->downlink);
        cost_ipv6_threshold = gw_default_calc_threshold(cost);
        eval_cost_ipv6_threshold = true;
      }
    }
  }

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    uint64_t path_cost;
    tc = olsr_lookup_tc_entry(&gw->originator);

    if (!tc) {
	  /* gateways should not exist without tc entry */
      continue;
    }

    if (tc->path_cost == ROUTE_COST_BROKEN) {
      /* do not consider nodes with an infinite ETX */
      continue;
    }

    if (!gw->uplink || !gw->downlink) {
      /* do not consider nodes without bandwidth or with a uni-directional link */
      continue;
    }

    /* determine the path cost */
    path_cost = gw_default_weigh_costs(tc->path_cost, gw->uplink, gw->downlink);

    if (!gw_def_finished_ipv4 && gw->ipv4 && gw->ipv4nat == olsr_cnf->smart_gw_allow_nat && path_cost < cost_ipv4 &&
        (!eval_cost_ipv4_threshold || (path_cost < cost_ipv4_threshold))) {
      inet_ipv4 = gw;
      cost_ipv4 = path_cost;
    }
    if (!gw_def_finished_ipv6 && gw->ipv6 && path_cost < cost_ipv6 &&
        (!eval_cost_ipv6_threshold || (path_cost < cost_ipv6_threshold))) {
      inet_ipv6 = gw;
      cost_ipv6 = path_cost;
    }
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)

  /* determine if we found an IPv4 and IPv6 gateway */
  gw_def_finished_ipv4 |= (inet_ipv4 != NULL);
  gw_def_finished_ipv6 |= (inet_ipv6 != NULL);

  /* determine if we are dealing with a dual stack gateway */
  dual = (inet_ipv4 == inet_ipv6) && (inet_ipv4 != NULL);

  if (inet_ipv4) {
	/* we are dealing with an IPv4 or dual stack gateway */
    olsr_set_inet_gateway(&inet_ipv4->originator, true, dual, false);
  }
  if (inet_ipv6 && !dual) {
    /* we are dealing with an IPv6-only gateway */
    olsr_set_inet_gateway(&inet_ipv6->originator, false, true, false);
  }

  if ((olsr_cnf->smart_gw_thresh == 0) && gw_def_finished_ipv4 && gw_def_finished_ipv6) {
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
  if (((tc_tree.count * 10) <= (gw_def_nodecount * 11)) ||
      ((tc_tree.count * 10) >= (gw_def_nodecount *  9))) {
    gw_def_nodecount = tc_tree.count;
  }

  if (tc_tree.count == gw_def_nodecount) {
    /* the number of gateway nodes is 'stable' */
    gw_def_stablecount++;
  }
  else {
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
static void olsr_gw_default_lookup_gateway(bool ipv4, bool ipv6) {
  if (ipv4) {
    /* get a new IPv4 gateway if we use OLSRv4 or NIIT */
    gw_def_finished_ipv4 = !(olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit);
  }
  if (ipv6) {
    /* get a new IPv6 gateway if we use OLSRv6 */
    gw_def_finished_ipv6 = !(olsr_cnf->ip_version == AF_INET6);
  }

  if (!(gw_def_finished_ipv4 && gw_def_finished_ipv6)) {
    gw_default_choose_gateway();
  }
}

/*
 * Exported functions
 */

/**
 * initialization of default gateway handler
 */
void olsr_gw_default_init(void) {
  /* initialize values */
  gw_def_nodecount = 0;
  gw_def_stablecount = 0;
  gw_def_finished_ipv4 = false;
  gw_def_finished_ipv6 = false;
  gw_def_timer = NULL;

  /* setup default handler */
  olsr_set_inetgw_handler(&gw_def_handler);
}

/*
 * Handler functions
 */

/**
 * Handle gateway startup
 */
static void gw_default_startup_handler(void) {
  /* reset node count */
  gw_def_nodecount = tc_tree.count;
  gw_def_stablecount = 0;

  /* get a new IPv4 gateway if we use OLSRv4 or NIIT */
  gw_def_finished_ipv4 = !(olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit);

  /* get a new IPv6 gateway if we use OLSRv6 */
  gw_def_finished_ipv6 = !(olsr_cnf->ip_version == AF_INET6);

  /* keep in mind we might be a gateway ourself */
  gw_def_finished_ipv4 |= olsr_cnf->has_ipv4_gateway;
  gw_def_finished_ipv6 |= olsr_cnf->has_ipv6_gateway;

  /* (re)start gateway lazy selection timer */
  olsr_set_timer(&gw_def_timer, olsr_cnf->smart_gw_period, 0, true, &gw_default_timer, NULL, 0);
}

/**
 * Choose a new gateway
 *
 * @param ipv4 lookup new v4 gateway
 * @param ipv6 lookup new v6 gateway
 */
static void gw_default_choosegw_handler(bool ipv4, bool ipv6) {
  olsr_gw_default_lookup_gateway(ipv4, ipv6);

  if (!(gw_def_finished_ipv4 && gw_def_finished_ipv6)) {
    gw_default_startup_handler();
  }
}

/**
 * Update a gateway entry
 *
 * @param gw the gateway entry
 */
static void gw_default_update_handler(struct gateway_entry *gw) {
  bool v4changed = gw && (gw == olsr_get_ipv4_inet_gateway(NULL))
      && (!gw->ipv4 || (gw->ipv4nat && !olsr_cnf->smart_gw_allow_nat));
  bool v6changed = gw && (gw == olsr_get_ipv6_inet_gateway(NULL)) && !gw->ipv6;

  if (v4changed || v6changed) {
    olsr_gw_default_lookup_gateway(v4changed, v6changed);
  }
}

/**
 * Remove a gateway entry
 *
 * @param gw the gateway entry
 */
static void gw_default_delete_handler(struct gateway_entry *gw) {
  bool isv4 = gw && (gw == olsr_get_ipv4_inet_gateway(NULL));
  bool isv6 = gw && (gw == olsr_get_ipv6_inet_gateway(NULL));

  if (isv4 || isv6) {
    olsr_gw_default_lookup_gateway(isv4, isv6);
  }
}
#endif /* linux */
