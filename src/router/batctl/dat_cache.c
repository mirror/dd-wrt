// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>

#include "batadv_packet.h"
#include "batman_adv.h"
#include "bat-hosts.h"
#include "debug.h"
#include "functions.h"
#include "main.h"
#include "netlink.h"

static const int dat_cache_mandatory[] = {
	BATADV_ATTR_DAT_CACHE_IP4ADDRESS,
	BATADV_ATTR_DAT_CACHE_HWADDRESS,
	BATADV_ATTR_DAT_CACHE_VID,
	BATADV_ATTR_LAST_SEEN_MSECS,
};

static int dat_cache_callback(struct nl_msg *msg, void *arg)
{
	int last_seen_msecs, last_seen_secs, last_seen_mins;
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	struct in_addr in_addr;
	uint8_t *hwaddr;
	int16_t vid;
	char *addr;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_DAT_CACHE)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, dat_cache_mandatory,
				    ARRAY_SIZE(dat_cache_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	in_addr.s_addr = nla_get_u32(attrs[BATADV_ATTR_DAT_CACHE_IP4ADDRESS]);
	addr = inet_ntoa(in_addr);
	hwaddr = nla_data(attrs[BATADV_ATTR_DAT_CACHE_HWADDRESS]);
	vid = nla_get_u16(attrs[BATADV_ATTR_DAT_CACHE_VID]);

	last_seen_msecs = nla_get_u32(attrs[BATADV_ATTR_LAST_SEEN_MSECS]);
	last_seen_mins = last_seen_msecs / 60000;
	last_seen_msecs = last_seen_msecs % 60000;
	last_seen_secs = last_seen_msecs / 1000;

	if (opts->read_opt & MULTICAST_ONLY && !(addr[0] & 0x01))
		return NL_OK;

	if (opts->read_opt & UNICAST_ONLY && (addr[0] & 0x01))
		return NL_OK;

	printf(" * %15s ", addr);

	bat_host = bat_hosts_find_by_mac((char *)hwaddr);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       hwaddr[0], hwaddr[1], hwaddr[2],
		       hwaddr[3], hwaddr[4], hwaddr[5]);
	else
		printf("%17s ", bat_host->name);

	printf("%4i %6i:%02i\n",
	       BATADV_PRINT_VID(vid), last_seen_mins, last_seen_secs);

	return NL_OK;
}

static int netlink_print_dat_cache(struct state *state, char *orig_iface,
				   int read_opts, float orig_timeout,
				   float watch_interval)
{
	char *header;
	int ret;

	ret = asprintf(&header, "Distributed ARP Table (%s):\n%s\n",
		       state->mesh_iface,
		       "          IPv4             MAC        VID   last-seen");

	if (ret < 0)
		return ret;

	ret = netlink_print_common(state, orig_iface, read_opts,
				   orig_timeout, watch_interval, header,
				   BATADV_CMD_GET_DAT_CACHE,
				   dat_cache_callback);

	free(header);
	return ret;
}

static struct debug_table_data batctl_debug_table_dat_cache = {
	.netlink_fn = netlink_print_dat_cache,
};

COMMAND_NAMED(DEBUGTABLE, dat_cache, "dc", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_dat_cache, "");
