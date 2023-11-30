/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Alexander Sarmanow <asarmanow@gmail.com>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include "genl_json.h"
#include "main.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <getopt.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "functions.h"
#include "batadv_packet.h"
#include "batman_adv.h"
#include "netlink.h"

struct nla_policy_json {
	const char *name;
	void (*cb)(struct nlattr *attrs[], int idx);
};

static void sanitize_string(const char *str)
{
	while (*str) {
		if (*str == '"' || *str == '\\') {
			putchar('\\');
			putchar(*str);
		} else if (!isprint(*str)) {
			printf("\\x%02x", *str);
		} else {
			putchar(*str);
		}
		str++;
	}
}

static void nljson_print_str(struct nlattr *attrs[], int idx)
{
	const char *value;

	value = nla_get_string(attrs[idx]);

	putchar('"');
	sanitize_string(value);
	putchar('"');
}

static void nljson_print_flag(struct nlattr *attrs[] __maybe_unused,
			      int idx __maybe_unused)
{
	printf("true");
}

static void nljson_print_bool(struct nlattr *attrs[], int idx)
{
	printf("%s", nla_get_u8(attrs[idx]) ? "true" : "false");
}

static void nljson_print_uint8(struct nlattr *attrs[], int idx)
{
	printf("%"PRIu8, nla_get_u8(attrs[idx]));
}

static void nljson_print_uint16(struct nlattr *attrs[], int idx)
{
	printf("%"PRIu16, nla_get_u16(attrs[idx]));
}

static void nljson_print_uint32(struct nlattr *attrs[], int idx)
{
	printf("%"PRIu32, nla_get_u32(attrs[idx]));
}

static void nljson_print_uint64(struct nlattr *attrs[], int idx)
{
	printf("%"PRIu64, nla_get_u64(attrs[idx]));
}

static void nljson_print_vlanid(struct nlattr *attrs[], int idx)
{
	uint16_t vid = nla_get_u16(attrs[idx]);

	printf("%d", BATADV_PRINT_VID(vid));
}

static void nljson_print_mac(struct nlattr *attrs[], int idx)
{
	uint8_t *value = nla_data(attrs[idx]);

	printf("\"%02x:%02x:%02x:%02x:%02x:%02x\"",
	       value[0], value[1], value[2], value[3], value[4], value[5]);
}

static void nljson_print_ttflags(struct nlattr *attrs[], int idx)
{
	uint32_t val = nla_get_u32(attrs[idx]);

	putchar('{');
	printf("\"del\": %s,",
	       val & BATADV_TT_CLIENT_DEL ? "true" : "false");
	printf("\"roam\": %s,",
	       val & BATADV_TT_CLIENT_ROAM ? "true" : "false");
	printf("\"wifi\": %s,",
	       val & BATADV_TT_CLIENT_WIFI ? "true" : "false");
	printf("\"isolated\": %s,",
	       val & BATADV_TT_CLIENT_ISOLA ? "true" : "false");
	printf("\"nopurge\": %s,",
	       val & BATADV_TT_CLIENT_NOPURGE ? "true" : "false");
	printf("\"new\": %s,",
	       val & BATADV_TT_CLIENT_NEW ? "true" : "false");
	printf("\"pending\": %s,",
	       val & BATADV_TT_CLIENT_PENDING ? "true" : "false");
	printf("\"temp\": %s,",
	       val & BATADV_TT_CLIENT_TEMP ? "true" : "false");
	printf("\"raw\": %"PRIu32, val);
	putchar('}');
}

static void nljson_print_ipv4(struct nlattr *attrs[], int idx)
{
	uint32_t val = nla_get_u32(attrs[idx]);
	struct in_addr in_addr;
	char *addr;

	in_addr.s_addr = val;
	addr = inet_ntoa(in_addr);

	putchar('"');
	sanitize_string(addr);
	putchar('"');
}

static void nljson_print_mcastflags(struct nlattr *attrs[], int idx)
{
	uint32_t val = nla_get_u32(attrs[idx]);

	putchar('{');
	printf("\"all_unsnoopables\": %s,",
	       val & BATADV_MCAST_WANT_ALL_UNSNOOPABLES ? "true" : "false");
	printf("\"want_all_ipv4\": %s,",
	       val & BATADV_MCAST_WANT_ALL_IPV4 ? "true" : "false");
	printf("\"want_all_ipv6\": %s,",
	       val & BATADV_MCAST_WANT_ALL_IPV6 ? "true" : "false");
	printf("\"want_no_rtr_ipv4\": %s,",
	       val & BATADV_MCAST_WANT_NO_RTR4 ? "true" : "false");
	printf("\"want_no_rtr_ipv6\": %s,",
	       val & BATADV_MCAST_WANT_NO_RTR6 ? "true" : "false");
	printf("\"raw\": %"PRIu32, val);
	putchar('}');
}

static void nljson_print_mcastflags_priv(struct nlattr *attrs[], int idx)
{
	uint32_t val = nla_get_u32(attrs[idx]);

	putchar('{');
	printf("\"bridged\": %s,",
	       val & BATADV_MCAST_FLAGS_BRIDGED ? "true" : "false");
	printf("\"querier_ipv4_exists\": %s,",
	       val & BATADV_MCAST_FLAGS_QUERIER_IPV4_EXISTS ? "true" : "false");
	printf("\"querier_ipv6_exists\": %s,",
	       val & BATADV_MCAST_FLAGS_QUERIER_IPV6_EXISTS ? "true" : "false");
	printf("\"querier_ipv4_shadowing\": %s,",
	       val & BATADV_MCAST_FLAGS_QUERIER_IPV4_SHADOWING ? "true" : "false");
	printf("\"querier_ipv6_shadowing\": %s,",
	       val & BATADV_MCAST_FLAGS_QUERIER_IPV6_SHADOWING ? "true" : "false");
	printf("\"raw\": %"PRIu32, val);
	putchar('}');
}

static void nljson_print_gwmode(struct nlattr *attrs[], int idx)
{
	uint8_t val = nla_get_u8(attrs[idx]);

	switch (val) {
	case BATADV_GW_MODE_OFF:
		printf("\"off\"");
		break;
	case BATADV_GW_MODE_CLIENT:
		printf("\"client\"");
		break;
	case BATADV_GW_MODE_SERVER:
		printf("\"server\"");
		break;
	default:
		printf("\"unknown\"");
		break;
	}
}

static void nljson_print_loglevel(struct nlattr *attrs[], int idx)
{
	uint32_t val = nla_get_u32(attrs[idx]);

	putchar('{');
	printf("\"batman\": %s,",
	       val & BIT(0) ? "true" : "false");
	printf("\"routes\": %s,",
	       val & BIT(1) ? "true" : "false");
	printf("\"tt\": %s,",
	       val & BIT(2) ? "true" : "false");
	printf("\"bla\": %s,",
	       val & BIT(3) ? "true" : "false");
	printf("\"dat\": %s,",
	       val & BIT(4) ? "true" : "false");
	printf("\"nc\": %s,",
	       val & BIT(5) ? "true" : "false");
	printf("\"mcast\": %s,",
	       val & BIT(6) ? "true" : "false");
	printf("\"tp\": %s,",
	       val & BIT(7) ? "true" : "false");
	printf("\"raw\": %"PRIu32, val);
	putchar('}');
}

/* WARNING: attributes must also be added to batadv_netlink_policy */
static const struct nla_policy_json batadv_genl_json[NUM_BATADV_ATTR] = {
	[BATADV_ATTR_VERSION] = {
		.name = "version",
		.cb = nljson_print_str,
	},
	[BATADV_ATTR_ALGO_NAME] = {
		.name = "algo_name",
		.cb = nljson_print_str,
	},
	[BATADV_ATTR_MESH_IFINDEX] = {
		.name = "mesh_ifindex",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_MESH_IFNAME] = {
		.name = "mesh_ifname",
		.cb = nljson_print_str,
	},
	[BATADV_ATTR_MESH_ADDRESS] = {
		.name = "mesh_address",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_HARD_IFINDEX] = {
		.name = "hard_ifindex",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_HARD_IFNAME] = {
		.name = "hard_ifname",
		.cb = nljson_print_str,
	},
	[BATADV_ATTR_HARD_ADDRESS] = {
		.name = "hard_address",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_ORIG_ADDRESS] = {
		.name = "orig_address",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_TPMETER_RESULT] = {
		.name = "tpmeter_result",
		.cb = nljson_print_uint8,
	},
	[BATADV_ATTR_TPMETER_TEST_TIME] = {
		.name = "tpmeter_test_time",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_TPMETER_BYTES] = {
		.name = "tpmeter_bytes",
		.cb = nljson_print_uint64,
	},
	[BATADV_ATTR_TPMETER_COOKIE] = {
		.name = "tpmeter_cookie",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_ACTIVE] = {
		.name = "active",
		.cb = nljson_print_flag,
	},
	[BATADV_ATTR_TT_ADDRESS] = {
		.name = "tt_address",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_TT_TTVN] = {
		.name = "tt_ttvn",
		.cb = nljson_print_uint8,
	},
	[BATADV_ATTR_TT_LAST_TTVN] = {
		.name = "tt_last_ttvn",
		.cb = nljson_print_uint8,
	},
	[BATADV_ATTR_TT_CRC32] = {
		.name = "tt_crc32",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_TT_VID] = {
		.name = "tt_vid",
		.cb = nljson_print_vlanid,
	},
	[BATADV_ATTR_TT_FLAGS] = {
		.name = "tt_flags",
		.cb = nljson_print_ttflags,
	},
	[BATADV_ATTR_FLAG_BEST] = {
		.name = "best",
		.cb = nljson_print_flag,
	},
	[BATADV_ATTR_LAST_SEEN_MSECS] = {
		.name = "last_seen_msecs",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_NEIGH_ADDRESS] = {
		.name = "neigh_address",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_TQ] = {
		.name = "tq",
		.cb = nljson_print_uint8,
	},
	[BATADV_ATTR_THROUGHPUT] = {
		.name = "throughput",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_BANDWIDTH_UP] = {
		.name = "bandwidth_up",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_BANDWIDTH_DOWN] = {
		.name = "bandwidth_down",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_ROUTER] = {
		.name = "router",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_BLA_OWN] = {
		.name = "bla_own",
		.cb = nljson_print_flag,
	},
	[BATADV_ATTR_BLA_ADDRESS] = {
		.name = "bla_address",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_BLA_VID] = {
		.name = "bla_vid",
		.cb = nljson_print_vlanid,
	},
	[BATADV_ATTR_BLA_BACKBONE] = {
		.name = "bla_backbone",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_BLA_CRC] = {
		.name = "bla_crc",
		.cb = nljson_print_uint16,
	},
	[BATADV_ATTR_DAT_CACHE_IP4ADDRESS] = {
		.name = "dat_cache_ip4address",
		.cb = nljson_print_ipv4,
	},
	[BATADV_ATTR_DAT_CACHE_HWADDRESS] = {
		.name = "dat_cache_hwaddress",
		.cb = nljson_print_mac,
	},
	[BATADV_ATTR_DAT_CACHE_VID] = {
		.name = "dat_cache_vid",
		.cb = nljson_print_vlanid,
	},
	[BATADV_ATTR_MCAST_FLAGS] = {
		.name = "mcast_flags",
		.cb = nljson_print_mcastflags,
	},
	[BATADV_ATTR_MCAST_FLAGS_PRIV] = {
		.name = "mcast_flags_priv",
		.cb = nljson_print_mcastflags_priv,
	},
	[BATADV_ATTR_VLANID] = {
		.name = "vlanid",
		.cb = nljson_print_uint16,
	},
	[BATADV_ATTR_AGGREGATED_OGMS_ENABLED] = {
		.name = "aggregated_ogms_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_AP_ISOLATION_ENABLED] = {
		.name = "ap_isolation_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_ISOLATION_MARK] = {
		.name = "isolation_mark",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_ISOLATION_MASK] = {
		.name = "isolation_mask",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_BONDING_ENABLED] = {
		.name = "bonding_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_BRIDGE_LOOP_AVOIDANCE_ENABLED] = {
		.name = "bridge_loop_avoidance_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_DISTRIBUTED_ARP_TABLE_ENABLED] = {
		.name = "distributed_arp_table_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_FRAGMENTATION_ENABLED] = {
		.name = "fragmentation_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_GW_BANDWIDTH_DOWN] = {
		.name = "gw_bandwidth_down",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_GW_BANDWIDTH_UP] = {
		.name = "gw_bandwidth_up",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_GW_MODE] = {
		.name = "gw_mode",
		.cb = nljson_print_gwmode,
	},
	[BATADV_ATTR_GW_SEL_CLASS] = {
		.name = "gw_sel_class",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_HOP_PENALTY] = {
		.name = "hop_penalty",
		.cb = nljson_print_uint8,
	},
	[BATADV_ATTR_LOG_LEVEL] = {
		.name = "log_level",
		.cb = nljson_print_loglevel,
	},
	[BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED] = {
		.name = "multicast_forceflood_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_NETWORK_CODING_ENABLED] = {
		.name = "network_coding_enabled",
		.cb = nljson_print_bool,
	},
	[BATADV_ATTR_ORIG_INTERVAL] = {
		.name = "orig_interval",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_ELP_INTERVAL] = {
		.name = "elp_interval",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_THROUGHPUT_OVERRIDE] = {
		.name = "throughput_override",
		.cb = nljson_print_uint32,
	},
	[BATADV_ATTR_MULTICAST_FANOUT] = {
		.name = "multicast_fanout",
		.cb = nljson_print_uint32,
	},
};

void netlink_print_json_entries(struct nlattr *attrs[], struct json_opts *json_opts)
{
	bool first_valid_attr = true;
	int i;

	if (!json_opts->is_first)
		putchar(',');
	else
		json_opts->is_first = false;

	putchar('{');
	for (i = 0; i < BATADV_ATTR_MAX + 1; i++) {
		if (!attrs[i])
			continue;

		if (!batadv_genl_json[i].cb)
			continue;

		if (!first_valid_attr)
			putchar(',');
		else
			first_valid_attr = false;

		putchar('"');
		sanitize_string(batadv_genl_json[i].name);
		putchar('"');
		putchar(':');
		batadv_genl_json[i].cb(attrs, i);
	}

	putchar('}');
}

static void json_query_usage(struct state *state)
{
	fprintf(stderr, "Usage: batctl [options] %s|%s [parameters]\n",
		state->cmd->name, state->cmd->abbr);
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
}

static int netlink_print_query_json_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlquery_opts *query_opts = arg;
	struct json_opts *json_opts;
	struct genlmsghdr *ghdr;

	json_opts = container_of(query_opts, struct json_opts, query_opts);

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	netlink_print_json_entries(attrs, json_opts);

	return NL_OK;
}

static int netlink_print_query_json_attributes(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;

	switch (state->selector) {
	case SP_NONE_OR_MESHIF:
	case SP_MESHIF:
		break;
	case SP_VLAN:
		nla_put_u16(msg, BATADV_ATTR_VLANID, state->vid);
		break;
	case SP_HARDIF:
		nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX, state->hif);
		break;
	}

	return 0;
}

static int netlink_print_query_json(struct state *state,
				    struct json_query_data *json_query)
{
	int ret;
	struct json_opts json_opts = {
		.is_first = true,
		.query_opts = {
			.err = 0,
		},
	};

	if (json_query->nlm_flags & NLM_F_DUMP)
		putchar('[');

	ret = netlink_query_common(state, state->mesh_ifindex,
				   json_query->cmd,
				   netlink_print_query_json_cb,
				   netlink_print_query_json_attributes,
				   json_query->nlm_flags,
				   &json_opts.query_opts);

	if (json_query->nlm_flags & NLM_F_DUMP)
		puts("]");
	else
		putchar('\n');

	return ret;
}

int handle_json_query(struct state *state, int argc, char **argv)
{
	struct json_query_data *json_query = state->cmd->arg;
	int optchar;
	int err;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			json_query_usage(state);
			return EXIT_SUCCESS;
		}
	}

	check_root_or_die("batctl");

	err = netlink_print_query_json(state, json_query);

	return err;
}
