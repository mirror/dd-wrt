// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdbool.h>
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

static const int mcast_flags_mandatory[] = {
	BATADV_ATTR_ORIG_ADDRESS,
};

static int mcast_flags_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	uint32_t flags;
	uint8_t *addr;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_MCAST_FLAGS)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, mcast_flags_mandatory,
				    ARRAY_SIZE(mcast_flags_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	addr = nla_data(attrs[BATADV_ATTR_ORIG_ADDRESS]);

	if (opts->read_opt & MULTICAST_ONLY && !(addr[0] & 0x01))
		return NL_OK;

	if (opts->read_opt & UNICAST_ONLY && (addr[0] & 0x01))
		return NL_OK;

	bat_host = bat_hosts_find_by_mac((char *)addr);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       addr[0], addr[1], addr[2],
		       addr[3], addr[4], addr[5]);
	else
		printf("%17s ", bat_host->name);

	if (attrs[BATADV_ATTR_MCAST_FLAGS]) {
		flags = nla_get_u32(attrs[BATADV_ATTR_MCAST_FLAGS]);

		printf("[%c%c%c%s%s]\n",
		       flags & BATADV_MCAST_WANT_ALL_UNSNOOPABLES ? 'U' : '.',
		       flags & BATADV_MCAST_WANT_ALL_IPV4 ? '4' : '.',
		       flags & BATADV_MCAST_WANT_ALL_IPV6 ? '6' : '.',
		       !(flags & BATADV_MCAST_WANT_NO_RTR4) ? "R4" : ". ",
		       !(flags & BATADV_MCAST_WANT_NO_RTR6) ? "R6" : ". ");
	} else {
		printf("-\n");
	}

	return NL_OK;
}

static int netlink_print_mcast_flags(struct state *state, char *orig_iface,
				     int read_opts, float orig_timeout,
				     float watch_interval)
{
	char querier4, querier6, shadowing4, shadowing6;
	char *info_header;
	char *header;
	bool bridged;
	int ret;

	/* only parse own multicast flags */
	info_header = netlink_get_info(state, BATADV_CMD_GET_MCAST_FLAGS, NULL);
	free(info_header);

	if (mcast_flags == -EOPNOTSUPP || mcast_flags_priv == -EOPNOTSUPP)
		return -EOPNOTSUPP;

	bridged = mcast_flags_priv & BATADV_MCAST_FLAGS_BRIDGED;

	if (bridged) {
                querier4 = (mcast_flags_priv & BATADV_MCAST_FLAGS_QUERIER_IPV4_EXISTS) ? '.' : '4';
                querier6 = (mcast_flags_priv & BATADV_MCAST_FLAGS_QUERIER_IPV6_EXISTS) ? '.' : '6';
                shadowing4 = (mcast_flags_priv & BATADV_MCAST_FLAGS_QUERIER_IPV4_SHADOWING) ? '4' : '.';
                shadowing6 = (mcast_flags_priv & BATADV_MCAST_FLAGS_QUERIER_IPV6_SHADOWING) ? '6' : '.';
        } else {
                querier4 = '?';
                querier6 = '?';
                shadowing4 = '?';
                shadowing6 = '?';
        }

	ret = asprintf(&header,
		"Multicast flags (own flags: [%c%c%c%s%s])\n"
		 "* Bridged [U]\t\t\t\t%c\n"
		 "* No IGMP/MLD Querier [4/6]:\t\t%c/%c\n"
		 "* Shadowing IGMP/MLD Querier [4/6]:\t%c/%c\n"
		 "-------------------------------------------\n"
		 "       %-10s %s\n",
		 (mcast_flags & BATADV_MCAST_WANT_ALL_UNSNOOPABLES) ? 'U' : '.',
		 (mcast_flags & BATADV_MCAST_WANT_ALL_IPV4) ? '4' : '.',
		 (mcast_flags & BATADV_MCAST_WANT_ALL_IPV6) ? '6' : '.',
	         !(mcast_flags & BATADV_MCAST_WANT_NO_RTR4) ? "R4" : ". ",
	         !(mcast_flags & BATADV_MCAST_WANT_NO_RTR6) ? "R6" : ". ",
		 bridged ? 'U' : '.',
		 querier4, querier6, shadowing4, shadowing6,
		 "Originator", "Flags");

	if (ret < 0)
		return ret;

	ret = netlink_print_common(state, orig_iface, read_opts,
				   orig_timeout, watch_interval, header,
				   BATADV_CMD_GET_MCAST_FLAGS,
				   mcast_flags_callback);

	free(header);
	return ret;
}

static struct debug_table_data batctl_debug_table_mcast_flags = {
	.netlink_fn = netlink_print_mcast_flags,
};

COMMAND_NAMED(DEBUGTABLE, mcast_flags, "mf", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_mcast_flags, "");
