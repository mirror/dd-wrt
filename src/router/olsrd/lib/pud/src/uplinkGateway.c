#include "uplinkGateway.h"

/* Plugin includes */

/* OLSRD includes */
#include "gateway.h"
#include "tc_set.h"
#include "lq_plugin.h"

/* System includes */

/**
 * Determine the speed on which a gateway is chosen
 * @param gw the gateway entry
 * @return the speed
 */
static inline unsigned long long gw_speed(struct gateway_entry *gw) {
	return (gw->uplink + gw->downlink);
}

/**
 * Determine the best gateway for uplink: this is the cluster leader.
 *
 * Loop over all gateways to find the best one and return it.
 * When no best gateway is found then we return ourselves so that the behaviour
 * degrades gracefully.
 *
 * A gateway is better when the sum of its uplink and downlink are greater than
 * the previous best gateway. In case of a tie, the lowest IP address wins.
 *
 * This code is copied from lib/txtinfo/src/olsrd_txtinfo.c, function ipc_print_gateway.
 * It adjusted for best gateway selection but otherwise kept the same as much
 * as possible.
 *
 * @param bestGateway
 * a pointer to the variable in which to store the best gateway
 */
void getBestUplinkGateway(union olsr_ip_addr * bestGateway) {
	struct gateway_entry *gw_best = NULL;
	unsigned long long gw_best_value = 0;
	struct gateway_entry *gw;

	OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
		bool eval4 = false;
		bool eval6 = false;

		struct tc_entry * tc = olsr_lookup_tc_entry(&gw->originator);
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

		if (gw == olsr_get_inet_gateway(false)) {
			eval4 = true;
		} else if (gw->ipv4
				&& (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit)
				&& (olsr_cnf->smart_gw_allow_nat || !gw->ipv4nat)) {
			eval4 = true;
		}

		if (gw == olsr_get_inet_gateway(true)) {
			eval6 = true;
		} else if (gw->ipv6 && olsr_cnf->ip_version == AF_INET6) {
			eval6 = true;
		}

		if (eval4 || eval6) {
			unsigned long long gw_value = gw_speed(gw);
			if (!gw_best || (gw_value > gw_best_value)) {
				gw_best = gw;
				gw_best_value = gw_value;
			} else if (gw_value == gw_best_value) {
				bool gwHaslowerIpAddress = false;
				if (eval4) {
					gwHaslowerIpAddress = (ip4cmp(&gw->originator.v4,
							&gw_best->originator.v4) < 0);
				} else /* eval6 */{
					gwHaslowerIpAddress = (ip6cmp(&gw->originator.v6,
							&gw_best->originator.v6) < 0);
				}
				if (gwHaslowerIpAddress) {
					gw_best = gw;
					gw_best_value = gw_value;
				}
			}
		}
	} OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)

	if (!gw_best) {
		/* degrade gracefully */
		*bestGateway = olsr_cnf->main_addr;
		return;
	}

	*bestGateway = gw_best->originator;
}
