// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Linus Lüssing <linus.luessing@c0d3.blue>
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

static const int bla_claim_mandatory[] = {
	BATADV_ATTR_BLA_ADDRESS,
	BATADV_ATTR_BLA_VID,
	BATADV_ATTR_BLA_BACKBONE,
	BATADV_ATTR_BLA_CRC,
};

static int bla_claim_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	struct bat_host *bat_host;
	struct genlmsghdr *ghdr;
	uint16_t backbone_crc;
	uint8_t *backbone;
	uint8_t *client;
	uint16_t vid;
	char c = ' ';

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_BLA_CLAIM)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, bla_claim_mandatory,
				       ARRAY_SIZE(bla_claim_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	if (attrs[BATADV_ATTR_BLA_OWN])
		c = '*';

	client = nla_data(attrs[BATADV_ATTR_BLA_ADDRESS]);
	vid = nla_get_u16(attrs[BATADV_ATTR_BLA_VID]);
	backbone = nla_data(attrs[BATADV_ATTR_BLA_BACKBONE]);
	backbone_crc = nla_get_u16(attrs[BATADV_ATTR_BLA_CRC]);

	bat_host = bat_hosts_find_by_mac((char *)client);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       client[0], client[1], client[2],
		       client[3], client[4], client[5]);
	else
		printf("%17s ", bat_host->name);

	printf("on %5d by ", BATADV_PRINT_VID(vid));

	bat_host = bat_hosts_find_by_mac((char *)backbone);
	if (!(opts->read_opt & USE_BAT_HOSTS) || !bat_host)
		printf("%02x:%02x:%02x:%02x:%02x:%02x ",
		       backbone[0], backbone[1], backbone[2],
		       backbone[3], backbone[4], backbone[5]);
	else
		printf("%17s ", bat_host->name);

	printf("[%c] (0x%04x)\n", c, backbone_crc);

	return NL_OK;
}

static int netlink_print_bla_claim(struct state *state, char *orig_iface,
				   int read_opts, float orig_timeout,
				   float watch_interval)
{
	return netlink_print_common(state, orig_iface, read_opts,
				    orig_timeout, watch_interval,
				    "Client               VID      Originator        [o] (CRC   )\n",
				    BATADV_CMD_GET_BLA_CLAIM,
				    bla_claim_callback);
}

static struct debug_table_data batctl_debug_table_claimtable = {
	.netlink_fn = netlink_print_bla_claim,
};

COMMAND_NAMED(DEBUGTABLE, claimtable, "cl", handle_debug_table,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_debug_table_claimtable, "");
