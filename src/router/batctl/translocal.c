// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andrew Lunn <andrew@lunn.ch>
 * Sven Eckelmann <sven@narfation.org>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

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

static const int translocal_mandatory[] = {
	BATADV_ATTR_TT_ADDRESS,
	BATADV_ATTR_TT_VID,
	BATADV_ATTR_TT_CRC32,
	BATADV_ATTR_TT_FLAGS,
};

static int translocal_callback(struct nl_msg *msg, void *arg)
{
	int last_seen_msecs = 0, last_seen_secs = 0;
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	char r, p, n, x, w, i;
	uint8_t *addr;
	int16_t vid;
	uint32_t crc32;
	uint32_t flags;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_TRANSTABLE_LOCAL)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, translocal_mandatory,
				    ARRAY_SIZE(translocal_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	addr = nla_data(attrs[BATADV_ATTR_TT_ADDRESS]);
	vid = nla_get_u16(attrs[BATADV_ATTR_TT_VID]);
	crc32 = nla_get_u32(attrs[BATADV_ATTR_TT_CRC32]);
	flags = nla_get_u32(attrs[BATADV_ATTR_TT_FLAGS]);
	last_seen_msecs = 0, last_seen_secs = 0;

	if (opts->read_opt & MULTICAST_ONLY && !(addr[0] & 0x01))
		return NL_OK;

	if (opts->read_opt & UNICAST_ONLY && (addr[0] & 0x01))
		return NL_OK;

	r = '.', p = '.', n = '.', x = '.', w = '.', i = '.';
	if (flags & BATADV_TT_CLIENT_ROAM)
		r = 'R';
	if (flags & BATADV_TT_CLIENT_NEW)
		n = 'N';
	if (flags & BATADV_TT_CLIENT_PENDING)
		x = 'X';
	if (flags & BATADV_TT_CLIENT_WIFI)
		w = 'W';
	if (flags & BATADV_TT_CLIENT_ISOLA)
		i = 'I';
	if (flags & BATADV_TT_CLIENT_NOPURGE)
		p = 'P';

	if (attrs[BATADV_ATTR_LAST_SEEN_MSECS]) {
		last_seen_msecs = nla_get_u32(
			attrs[BATADV_ATTR_LAST_SEEN_MSECS]);
		last_seen_secs = last_seen_msecs / 1000;
		last_seen_msecs = last_seen_msecs % 1000;
	}

	bat_host = bat_hosts_find_by_mac((char *)addr);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       addr[0], addr[1], addr[2],
		       addr[3], addr[4], addr[5]);
	else
		printf("%17s ", bat_host->name);

	printf("%4i [%c%c%c%c%c%c] %3u.%03u   (0x%.8x)\n",
	       BATADV_PRINT_VID(vid), r, p, n, x, w, i,
	       last_seen_secs, last_seen_msecs,
	       crc32);

	return NL_OK;
}

static int netlink_print_translocal(struct state *state, char *orig_iface,
				    int read_opts, float orig_timeout,
				    float watch_interval)
{
	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval,
				    "Client             VID Flags    Last seen (CRC       )\n",
				    BATADV_CMD_GET_TRANSTABLE_LOCAL,
				    translocal_callback);
}

static struct debug_table_data batctl_debug_table_translocal = {
	.netlink_fn = netlink_print_translocal,
	.option_unicast_only = 1,
	.option_multicast_only = 1,
};

COMMAND_NAMED(DEBUGTABLE, translocal, "tl", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_translocal, "");
