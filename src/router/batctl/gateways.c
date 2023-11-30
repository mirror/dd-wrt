// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Linus Lüssing <linus.luessing@c0d3.blue>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "batadv_packet.h"
#include "batman_adv.h"
#include "bat-hosts.h"
#include "debug.h"
#include "functions.h"
#include "main.h"
#include "netlink.h"

static const int gateways_mandatory[] = {
	BATADV_ATTR_ORIG_ADDRESS,
	BATADV_ATTR_ROUTER,
	BATADV_ATTR_HARD_IFNAME,
	BATADV_ATTR_BANDWIDTH_DOWN,
	BATADV_ATTR_BANDWIDTH_UP,
};

static int gateways_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	const char *primary_if;
	uint32_t bandwidth_down;
	uint32_t bandwidth_up;
	uint32_t throughput;
	uint8_t *router;
	uint8_t *orig;
	char c = ' ';
	uint8_t tq;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_GATEWAYS)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, gateways_mandatory,
				    ARRAY_SIZE(gateways_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	if (attrs[BATADV_ATTR_FLAG_BEST])
		c = '*';

	orig = nla_data(attrs[BATADV_ATTR_ORIG_ADDRESS]);
	router = nla_data(attrs[BATADV_ATTR_ROUTER]);
	primary_if = nla_get_string(attrs[BATADV_ATTR_HARD_IFNAME]);
	bandwidth_down = nla_get_u32(attrs[BATADV_ATTR_BANDWIDTH_DOWN]);
	bandwidth_up = nla_get_u32(attrs[BATADV_ATTR_BANDWIDTH_UP]);

	printf("%c ", c);

	bat_host = bat_hosts_find_by_mac((char *)orig);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       orig[0], orig[1], orig[2],
		       orig[3], orig[4], orig[5]);
	else
		printf("%17s ", bat_host->name);

	if (attrs[BATADV_ATTR_THROUGHPUT]) {
		throughput = nla_get_u32(attrs[BATADV_ATTR_THROUGHPUT]);
		printf("(%9u.%1u) ", throughput / 10, throughput % 10);
	} else if (attrs[BATADV_ATTR_TQ]) {
		tq = nla_get_u8(attrs[BATADV_ATTR_TQ]);
		printf("(%3i) ", tq);
	}

	bat_host = bat_hosts_find_by_mac((char *)router);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       router[0], router[1], router[2],
		       router[3], router[4], router[5]);
	else
		printf("%17s ", bat_host->name);

	printf("[%10s]: %u.%u/%u.%u MBit\n",
	       primary_if, bandwidth_down / 10, bandwidth_down % 10,
	       bandwidth_up / 10, bandwidth_up % 10);

	return NL_OK;
}

static int netlink_print_gateways(struct state *state, char *orig_iface,
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
		header = "  Router            ( TQ) Next Hop          [outgoingIf]  Bandwidth\n";
	if (!strcmp("BATMAN_V", algo_name_buf))
		header = "  Router            ( throughput) Next Hop          [outgoingIf]  Bandwidth\n";

	if (!header)
		return -EINVAL;

	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval,
				    header,
				    BATADV_CMD_GET_GATEWAYS,
				    gateways_callback);
}

static struct debug_table_data batctl_debug_table_gateways = {
	.netlink_fn = netlink_print_gateways,
};

COMMAND_NAMED(DEBUGTABLE, gateways, "gwl", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_gateways, "");
