// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich <sw@simonwunderlich.de>
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

static const int bla_backbone_mandatory[] = {
	BATADV_ATTR_BLA_VID,
	BATADV_ATTR_BLA_BACKBONE,
	BATADV_ATTR_BLA_CRC,
	BATADV_ATTR_LAST_SEEN_MSECS,
};

static int bla_backbone_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	int last_seen_msecs, last_seen_secs;
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	uint16_t backbone_crc;
	uint8_t *backbone;
	uint16_t vid;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_BLA_BACKBONE)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, bla_backbone_mandatory,
				    ARRAY_SIZE(bla_backbone_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	/* don't show own backbones */
	if (attrs[BATADV_ATTR_BLA_OWN])
		return NL_OK;

	vid = nla_get_u16(attrs[BATADV_ATTR_BLA_VID]);
	backbone = nla_data(attrs[BATADV_ATTR_BLA_BACKBONE]);
	backbone_crc = nla_get_u16(attrs[BATADV_ATTR_BLA_CRC]);

	last_seen_msecs = nla_get_u32(attrs[BATADV_ATTR_LAST_SEEN_MSECS]);
	last_seen_secs = last_seen_msecs / 1000;
	last_seen_msecs = last_seen_msecs % 1000;

	bat_host = bat_hosts_find_by_mac((char *)backbone);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       backbone[0], backbone[1], backbone[2],
		       backbone[3], backbone[4], backbone[5]);
	else
		printf("%17s ", bat_host->name);

	printf("on %5d %4i.%03is (0x%04x)\n",
	       BATADV_PRINT_VID(vid), last_seen_secs, last_seen_msecs,
	       backbone_crc);

	return NL_OK;
}

static int netlink_print_bla_backbone(struct state *state, char *orig_iface,
				      int read_opts, float orig_timeout,
				      float watch_interval)
{
	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval,
				    "Originator           VID   last seen (CRC   )\n",
				    BATADV_CMD_GET_BLA_BACKBONE,
				    bla_backbone_callback);
}

static struct debug_table_data batctl_debug_table_backbonetable = {
	.netlink_fn = netlink_print_bla_backbone,
};

COMMAND_NAMED(DEBUGTABLE, backbonetable, "bbt", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_backbonetable, "");
