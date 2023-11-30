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

static const int transglobal_mandatory[] = {
	BATADV_ATTR_TT_ADDRESS,
	BATADV_ATTR_ORIG_ADDRESS,
	BATADV_ATTR_TT_VID,
	BATADV_ATTR_TT_TTVN,
	BATADV_ATTR_TT_LAST_TTVN,
	BATADV_ATTR_TT_CRC32,
	BATADV_ATTR_TT_FLAGS,
};

static int transglobal_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	char c, r, w, i, t;
	uint8_t last_ttvn;
	uint32_t crc32;
	uint32_t flags;
	uint8_t *addr;
	uint8_t *orig;
	uint8_t ttvn;
	int16_t vid;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_TRANSTABLE_GLOBAL)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, transglobal_mandatory,
				    ARRAY_SIZE(transglobal_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	addr = nla_data(attrs[BATADV_ATTR_TT_ADDRESS]);
	orig = nla_data(attrs[BATADV_ATTR_ORIG_ADDRESS]);
	vid = nla_get_u16(attrs[BATADV_ATTR_TT_VID]);
	ttvn = nla_get_u8(attrs[BATADV_ATTR_TT_TTVN]);
	last_ttvn = nla_get_u8(attrs[BATADV_ATTR_TT_LAST_TTVN]);
	crc32 = nla_get_u32(attrs[BATADV_ATTR_TT_CRC32]);
	flags = nla_get_u32(attrs[BATADV_ATTR_TT_FLAGS]);

	if (opts->read_opt & MULTICAST_ONLY && !(addr[0] & 0x01))
		return NL_OK;

	if (opts->read_opt & UNICAST_ONLY && (addr[0] & 0x01))
		return NL_OK;

	c = ' ', r = '.', w = '.', i = '.', t = '.';
	if (attrs[BATADV_ATTR_FLAG_BEST])
		c = '*';
	if (flags & BATADV_TT_CLIENT_ROAM)
		r = 'R';
	if (flags & BATADV_TT_CLIENT_WIFI)
		w = 'W';
	if (flags & BATADV_TT_CLIENT_ISOLA)
		i = 'I';
	if (flags & BATADV_TT_CLIENT_TEMP)
		t = 'T';

	printf(" %c ", c);

	bat_host = bat_hosts_find_by_mac((char *)addr);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       addr[0], addr[1], addr[2],
		       addr[3], addr[4], addr[5]);
	else
		printf("%17s ", bat_host->name);

	printf("%4i [%c%c%c%c] (%3u) ",
	       BATADV_PRINT_VID(vid), r, w, i, t, ttvn);

	bat_host = bat_hosts_find_by_mac((char *)orig);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       orig[0], orig[1], orig[2],
		       orig[3], orig[4], orig[5]);
	else
		printf("%17s ", bat_host->name);

	printf("(%3u) (0x%.8x)\n",
	       last_ttvn, crc32);

	return NL_OK;
}

static int netlink_print_transglobal(struct state *state, char *orig_iface,
				     int read_opts, float orig_timeout,
				     float watch_interval)
{
	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval,
				    "   Client             VID Flags Last ttvn     Via        ttvn  (CRC       )\n",
				    BATADV_CMD_GET_TRANSTABLE_GLOBAL,
				    transglobal_callback);
}

static struct debug_table_data batctl_debug_table_transglobal = {
	.netlink_fn = netlink_print_transglobal,
	.option_unicast_only = 1,
	.option_multicast_only = 1,
};

COMMAND_NAMED(DEBUGTABLE, transglobal, "tg", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_transglobal, "");
