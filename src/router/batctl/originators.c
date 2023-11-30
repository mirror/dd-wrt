// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andrew Lunn <andrew@lunn.ch>
 * Sven Eckelmann <sven@narfation.org>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "batadv_packet.h"
#include "batman_adv.h"
#include "bat-hosts.h"
#include "debug.h"
#include "functions.h"
#include "main.h"
#include "netlink.h"

static const int originators_mandatory[] = {
	BATADV_ATTR_ORIG_ADDRESS,
	BATADV_ATTR_NEIGH_ADDRESS,
	BATADV_ATTR_HARD_IFINDEX,
	BATADV_ATTR_LAST_SEEN_MSECS,
};

static int originators_callback(struct nl_msg *msg, void *arg)
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
	float last_seen;
	uint8_t *neigh;
	uint8_t *orig;
	char *ifname;
	char c = ' ';
	uint8_t tq;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_ORIGINATORS)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, originators_mandatory,
				       ARRAY_SIZE(originators_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	orig = nla_data(attrs[BATADV_ATTR_ORIG_ADDRESS]);
	neigh = nla_data(attrs[BATADV_ATTR_NEIGH_ADDRESS]);

	if (attrs[BATADV_ATTR_HARD_IFNAME]) {
		ifname = nla_get_string(attrs[BATADV_ATTR_HARD_IFNAME]);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		ifindex = nla_get_u32(attrs[BATADV_ATTR_HARD_IFINDEX]);
		if (!if_indextoname(ifindex, ifname_buf))
			ifname_buf[0] = '\0';

		ifname = ifname_buf;
	}

	if (attrs[BATADV_ATTR_FLAG_BEST])
		c = '*';

	last_seen_msecs = nla_get_u32(attrs[BATADV_ATTR_LAST_SEEN_MSECS]);
	last_seen = (float)last_seen_msecs / 1000.0;
	last_seen_secs = last_seen_msecs / 1000;
	last_seen_msecs = last_seen_msecs % 1000;

	/* skip timed out originators */
	if (opts->read_opt & NO_OLD_ORIGS)
		if (last_seen > opts->orig_timeout)
			return NL_OK;

	if (attrs[BATADV_ATTR_THROUGHPUT]) {
		throughput_kbits = nla_get_u32(attrs[BATADV_ATTR_THROUGHPUT]);
		throughput_mbits = throughput_kbits / 1000;
		throughput_kbits = throughput_kbits % 1000;

		if (!(opts->read_opt & USE_BAT_HOSTS)) {
			printf(" %c %02x:%02x:%02x:%02x:%02x:%02x %4i.%03is (%9u.%1u) %02x:%02x:%02x:%02x:%02x:%02x [%10s]\n",
			       c,
			       orig[0], orig[1], orig[2],
			       orig[3], orig[4], orig[5],
			       last_seen_secs, last_seen_msecs,
			       throughput_mbits, throughput_kbits / 100,
			       neigh[0], neigh[1], neigh[2],
			       neigh[3], neigh[4], neigh[5],
			       ifname);
		} else {
			bat_host = bat_hosts_find_by_mac((char *)orig);
			if (bat_host)
				printf(" %c %17s ", c, bat_host->name);
			else
				printf(" %c %02x:%02x:%02x:%02x:%02x:%02x ",
				       c,
				       orig[0], orig[1], orig[2],
				       orig[3], orig[4], orig[5]);
			printf("%4i.%03is (%9u.%1u) ",
			       last_seen_secs, last_seen_msecs,
			       throughput_mbits, throughput_kbits / 100);
			bat_host = bat_hosts_find_by_mac((char *)neigh);
			if (bat_host)
				printf(" %c %17s ", c, bat_host->name);
			else
				printf(" %02x:%02x:%02x:%02x:%02x:%02x ",
				       neigh[0], neigh[1], neigh[2],
				       neigh[3], neigh[4], neigh[5]);
			printf("[%10s]\n", ifname);
		}
	}
	if (attrs[BATADV_ATTR_TQ]) {
		tq = nla_get_u8(attrs[BATADV_ATTR_TQ]);

		if (!(opts->read_opt & USE_BAT_HOSTS)) {
			printf(" %c %02x:%02x:%02x:%02x:%02x:%02x %4i.%03is   (%3i) %02x:%02x:%02x:%02x:%02x:%02x [%10s]\n",
			       c,
			       orig[0], orig[1], orig[2],
			       orig[3], orig[4], orig[5],
			       last_seen_secs, last_seen_msecs, tq,
			       neigh[0], neigh[1], neigh[2],
			       neigh[3], neigh[4], neigh[5],
			       ifname);
		} else {
			bat_host = bat_hosts_find_by_mac((char *)orig);
			if (bat_host)
				printf(" %c %17s ", c, bat_host->name);
			else
				printf(" %c %02x:%02x:%02x:%02x:%02x:%02x ",
				       c,
				       orig[0], orig[1], orig[2],
				       orig[3], orig[4], orig[5]);
			printf("%4i.%03is   (%3i) ",
			       last_seen_secs, last_seen_msecs, tq);
			bat_host = bat_hosts_find_by_mac((char *)neigh);
			if (bat_host)
				printf("%17s ", bat_host->name);
			else
				printf("%02x:%02x:%02x:%02x:%02x:%02x ",
				       neigh[0], neigh[1], neigh[2],
				       neigh[3], neigh[4], neigh[5]);
			printf("[%10s]\n", ifname);
		}
	}

	return NL_OK;
}

static int netlink_print_originators(struct state *state, char *orig_iface,
				     int read_opts, float orig_timeout,
				     float watch_interval)
{
	char *header = NULL;
	char *info_header;

	/* only parse routing algorithm name */
	last_err = -EINVAL;
	info_header = netlink_get_info(state, BATADV_CMD_GET_ORIGINATORS, NULL);
	free(info_header);

	if (strlen(algo_name_buf) == 0)
		return last_err;

	if (!strcmp("BATMAN_IV", algo_name_buf))
		header = "   Originator        last-seen (#/255) Nexthop           [outgoingIF]\n";
	if (!strcmp("BATMAN_V", algo_name_buf))
		header = "   Originator        last-seen ( throughput)  Nexthop           [outgoingIF]\n";

	if (!header)
		return -EINVAL;

	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval, header,
				    BATADV_CMD_GET_ORIGINATORS,
				    originators_callback);
}

static struct debug_table_data batctl_debug_table_originators = {
	.netlink_fn = netlink_print_originators,
	.option_timeout_interval = 1,
	.option_orig_iface = 1,
};

COMMAND_NAMED(DEBUGTABLE, originators, "o", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_originators, "");
