// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andrew Lunn <andrew@lunn.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <net/if.h>
#include <netinet/if_ether.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "batadv_packet.h"
#include "batman_adv.h"
#include "bat-hosts.h"
#include "debug.h"
#include "functions.h"
#include "main.h"
#include "netlink.h"

static const int neighbors_mandatory[] = {
	BATADV_ATTR_NEIGH_ADDRESS,
	BATADV_ATTR_HARD_IFINDEX,
	BATADV_ATTR_LAST_SEEN_MSECS,
};

static int neighbors_callback(struct nl_msg *msg, void *arg)
{
	unsigned throughput_mbits, throughput_kbits;
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	int last_seen_msecs, last_seen_secs;
	struct print_opts *opts = arg;
	char ifname_buf[IF_NAMESIZE];
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	uint32_t ifindex;
	uint8_t *neigh;
	char *ifname;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_NEIGHBORS)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, neighbors_mandatory,
				    ARRAY_SIZE(neighbors_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	neigh = nla_data(attrs[BATADV_ATTR_NEIGH_ADDRESS]);
	bat_host = bat_hosts_find_by_mac((char *)neigh);

	if (attrs[BATADV_ATTR_HARD_IFNAME]) {
		ifname = nla_get_string(attrs[BATADV_ATTR_HARD_IFNAME]);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		ifindex = nla_get_u32(attrs[BATADV_ATTR_HARD_IFINDEX]);
		if (!if_indextoname(ifindex, ifname_buf))
			ifname_buf[0] = '\0';

		ifname = ifname_buf;
	}

	last_seen_msecs = nla_get_u32(attrs[BATADV_ATTR_LAST_SEEN_MSECS]);
	last_seen_secs = last_seen_msecs / 1000;
	last_seen_msecs = last_seen_msecs % 1000;

	if (attrs[BATADV_ATTR_THROUGHPUT]) {
		throughput_kbits = nla_get_u32(attrs[BATADV_ATTR_THROUGHPUT]);
		throughput_mbits = throughput_kbits / 1000;
		throughput_kbits = throughput_kbits % 1000;

		if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
			printf("%02x:%02x:%02x:%02x:%02x:%02x ",
			       neigh[0], neigh[1], neigh[2],
			       neigh[3], neigh[4], neigh[5]);
		else
			printf("%17s ", bat_host->name);

		printf("%4i.%03is (%9u.%1u) [%10s]\n",
		       last_seen_secs, last_seen_msecs,
		       throughput_mbits, throughput_kbits / 100,
		       ifname);
	} else {
		printf("   %10s	  ", ifname);

		if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
			printf("%02x:%02x:%02x:%02x:%02x:%02x ",
			       neigh[0], neigh[1], neigh[2],
			       neigh[3], neigh[4], neigh[5]);
		else
			printf("%17s ", bat_host->name);

		printf("%4i.%03is\n", last_seen_secs, last_seen_msecs);
	}

	return NL_OK;
}

static int netlink_print_neighbors(struct state *state, char *orig_iface,
				   int read_opts, float orig_timeout,
				   float watch_interval)
{
	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval,
				    "IF             Neighbor              last-seen\n",
				    BATADV_CMD_GET_NEIGHBORS,
				    neighbors_callback);
}

static struct debug_table_data batctl_debug_table_neighbors = {
	.netlink_fn = netlink_print_neighbors,
};

COMMAND_NAMED(DEBUGTABLE, neighbors, "n", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_neighbors, "");
