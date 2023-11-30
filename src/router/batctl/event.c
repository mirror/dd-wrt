// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Sven Eckelmann <sven@narfation.org>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <getopt.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include "batadv_packet.h"
#include "batman_adv.h"
#include "bat-hosts.h"
#include "debug.h"
#include "functions.h"
#include "genl.h"
#include "main.h"
#include "netlink.h"

enum event_time_mode {
	EVENT_TIME_NO,
	EVENT_TIME_LOCAL,
	EVENT_TIME_RELATIVE,
};

struct event_args {
	enum event_time_mode mode;
	struct timeval tv;
};

static const char *u8_to_boolstr(struct nlattr *attrs)
{
	if (nla_get_u8(attrs))
		return "true";
	else
		return "false";
}

static void event_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] event [parameters]\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -t print local timestamp\n");
	fprintf(stderr, " \t -r print relative timestamp\n");
}

static int event_prepare(struct state *state)
{
	int ret;
	int mcid;

	if (!state->sock)
		return -EOPNOTSUPP;

	mcid = nl_get_multicast_id(state->sock, BATADV_NL_NAME,
				   BATADV_NL_MCAST_GROUP_TPMETER);
	if (mcid < 0) {
		fprintf(stderr, "Failed to resolve batadv tp_meter multicast group: %d\n",
			mcid);
		/* ignore error for now */
		goto skip_tp_meter;
	}

	ret = nl_socket_add_membership(state->sock, mcid);
	if (ret) {
		fprintf(stderr, "Failed to join batadv tp_meter multicast group: %d\n",
			ret);
		/* ignore error for now */
		goto skip_tp_meter;
	}

skip_tp_meter:

	mcid = nl_get_multicast_id(state->sock, BATADV_NL_NAME,
				   BATADV_NL_MCAST_GROUP_CONFIG);
	if (mcid < 0) {
		fprintf(stderr, "Failed to resolve batadv config multicast group: %d\n",
			mcid);
		/* ignore error for now */
		goto skip_config;
	}

	ret = nl_socket_add_membership(state->sock, mcid);
	if (ret) {
		fprintf(stderr, "Failed to join batadv config multicast group: %d\n",
			ret);
		/* ignore error for now */
		goto skip_config;
	}

skip_config:

	return 0;
}

static int no_seq_check(struct nl_msg *msg __maybe_unused,
			void *arg __maybe_unused)
{
	return NL_OK;
}

static const int tp_meter_mandatory[] = {
	BATADV_ATTR_TPMETER_COOKIE,
	BATADV_ATTR_TPMETER_RESULT,
};

static void event_parse_tp_meter(struct nlattr **attrs)
{
	const char *result_str;
	uint32_t cookie;
	uint8_t result;

	/* ignore entry when attributes are missing */
	if (missing_mandatory_attrs(attrs, tp_meter_mandatory,
				    ARRAY_SIZE(tp_meter_mandatory)))
		return;

	cookie = nla_get_u32(attrs[BATADV_ATTR_TPMETER_COOKIE]);
	result = nla_get_u8(attrs[BATADV_ATTR_TPMETER_RESULT]);

	switch (result) {
	case BATADV_TP_REASON_DST_UNREACHABLE:
		result_str = "Destination unreachable";
		break;
	case BATADV_TP_REASON_RESEND_LIMIT:
		result_str = "The number of retry for the same window exceeds the limit, test aborted";
		break;
	case BATADV_TP_REASON_ALREADY_ONGOING:
		result_str = "Cannot run two test towards the same node";
		break;
	case BATADV_TP_REASON_MEMORY_ERROR:
		result_str = "Kernel cannot allocate memory, aborted";
		break;
	case BATADV_TP_REASON_TOO_MANY:
		result_str = "Too many ongoing sessions";
		break;
	case BATADV_TP_REASON_CANCEL:
		result_str = "CANCEL received: test aborted";
		break;
	case BATADV_TP_REASON_COMPLETE:
		result_str = "complete";
		break;
	default:
		result_str = "unknown";
		break;
	}

	printf("tp_meter 0x%08x: %s\n", cookie, result_str);
}

static void event_parse_set_mesh(struct nlattr **attrs)
{
	static const int mesh_mandatory[] = {
		BATADV_ATTR_MESH_IFINDEX,
		BATADV_ATTR_ALGO_NAME,
	};
	char meshif_buf[IF_NAMESIZE];
	char *meshif_name;
	uint32_t mesh_ifindex;

	/* ignore entry when attributes are missing */
	if (missing_mandatory_attrs(attrs, mesh_mandatory,
				    ARRAY_SIZE(mesh_mandatory)))
		return;

	if (attrs[BATADV_ATTR_MESH_IFNAME]) {
		meshif_name = nla_get_string(attrs[BATADV_ATTR_MESH_IFNAME]);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		mesh_ifindex = nla_get_u32(attrs[BATADV_ATTR_MESH_IFINDEX]);
		meshif_name = if_indextoname(mesh_ifindex, meshif_buf);
		if (!meshif_name)
			return;
	}

	printf("%s: set mesh:\n", meshif_name);

	if (attrs[BATADV_ATTR_AGGREGATED_OGMS_ENABLED])
		printf("* aggregated_ogms %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_AGGREGATED_OGMS_ENABLED]));

	if (attrs[BATADV_ATTR_AP_ISOLATION_ENABLED])
		printf("* ap_isolation %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_AP_ISOLATION_ENABLED]));

	if (attrs[BATADV_ATTR_ISOLATION_MARK])
		printf("* isolation_mark 0x%08x\n",
		       nla_get_u32(attrs[BATADV_ATTR_ISOLATION_MARK]));

	if (attrs[BATADV_ATTR_ISOLATION_MASK])
		printf("* isolation_mask 0x%08x\n",
		       nla_get_u32(attrs[BATADV_ATTR_ISOLATION_MASK]));

	if (attrs[BATADV_ATTR_BONDING_ENABLED])
		printf("* bonding %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_BONDING_ENABLED]));

	if (attrs[BATADV_ATTR_BRIDGE_LOOP_AVOIDANCE_ENABLED])
		printf("* bridge_loop_avoidance %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_BRIDGE_LOOP_AVOIDANCE_ENABLED]));

	if (attrs[BATADV_ATTR_DISTRIBUTED_ARP_TABLE_ENABLED])
		printf("* distributed_arp_table %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_DISTRIBUTED_ARP_TABLE_ENABLED]));

	if (attrs[BATADV_ATTR_FRAGMENTATION_ENABLED])
		printf("* fragmentation %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_FRAGMENTATION_ENABLED]));

	if (attrs[BATADV_ATTR_GW_BANDWIDTH_DOWN]) {
		uint32_t val;

		val = nla_get_u32(attrs[BATADV_ATTR_GW_BANDWIDTH_DOWN]);
		printf("* gw_bandwidth_down %u.%01u MBit/s\n", val / 10,
		       val % 10);
	}

	if (attrs[BATADV_ATTR_GW_BANDWIDTH_UP]) {
		uint32_t val;

		val = nla_get_u32(attrs[BATADV_ATTR_GW_BANDWIDTH_UP]);
		printf("* gw_bandwidth_up %u.%01u MBit/s\n", val / 10,
		       val % 10);
	}

	if (attrs[BATADV_ATTR_GW_MODE]) {
		uint8_t val = nla_get_u8(attrs[BATADV_ATTR_GW_MODE]);
		const char *valstr;

		switch (val) {
		case BATADV_GW_MODE_OFF:
			valstr = "off";
			break;
		case BATADV_GW_MODE_CLIENT:
			valstr = "client";
			break;
		case BATADV_GW_MODE_SERVER:
			valstr = "server";
			break;
		default:
			valstr = "unknown";
			break;
		}

		printf("* gw_mode %s\n", valstr);
	}

	if (attrs[BATADV_ATTR_GW_SEL_CLASS]) {
		uint32_t val = nla_get_u32(attrs[BATADV_ATTR_GW_SEL_CLASS]);
		const char *algo = nla_data(attrs[BATADV_ATTR_ALGO_NAME]);

		if (strcmp(algo, "BATMAN_V") == 0)
			printf("* gw_sel_class %u.%01u MBit/s\n", val / 10,
			       val % 10);
		else
			printf("* gw_sel_class %u\n", val);
	}

	if (attrs[BATADV_ATTR_HOP_PENALTY])
		printf("* hop_penalty %u\n",
		       nla_get_u8(attrs[BATADV_ATTR_HOP_PENALTY]));

	if (attrs[BATADV_ATTR_LOG_LEVEL])
		printf("* log_level 0x%08x\n",
		       nla_get_u32(attrs[BATADV_ATTR_LOG_LEVEL]));

	if (attrs[BATADV_ATTR_MULTICAST_FANOUT])
		printf("* multicast_fanout %u\n",
		       nla_get_u32(attrs[BATADV_ATTR_MULTICAST_FANOUT]));

	if (attrs[BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED])
		printf("* multicast_forceflood %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED]));

	if (attrs[BATADV_ATTR_NETWORK_CODING_ENABLED])
		printf("* network_coding %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_NETWORK_CODING_ENABLED]));

	if (attrs[BATADV_ATTR_ORIG_INTERVAL])
		printf("* orig_interval %u ms\n",
		       nla_get_u32(attrs[BATADV_ATTR_ORIG_INTERVAL]));
}

static void event_parse_set_hardif(struct nlattr **attrs)
{
	static const int hardif_mandatory[] = {
		BATADV_ATTR_MESH_IFINDEX,
		BATADV_ATTR_HARD_IFINDEX,
	};
	char meshif_buf[IF_NAMESIZE];
	char hardif_buf[IF_NAMESIZE];
	char *meshif_name;
	char *hardif_name;
	uint32_t hardif_ifindex;
	uint32_t mesh_ifindex;

	/* ignore entry when attributes are missing */
	if (missing_mandatory_attrs(attrs, hardif_mandatory,
				    ARRAY_SIZE(hardif_mandatory)))
		return;

	if (attrs[BATADV_ATTR_MESH_IFNAME]) {
		meshif_name = nla_get_string(attrs[BATADV_ATTR_MESH_IFNAME]);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		mesh_ifindex = nla_get_u32(attrs[BATADV_ATTR_MESH_IFINDEX]);
		meshif_name = if_indextoname(mesh_ifindex, meshif_buf);
		if (!meshif_name)
			return;
	}

	if (attrs[BATADV_ATTR_MESH_IFNAME]) {
		hardif_name = nla_get_string(attrs[BATADV_ATTR_HARD_IFNAME]);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		hardif_ifindex = nla_get_u32(attrs[BATADV_ATTR_HARD_IFINDEX]);
		hardif_name = if_indextoname(hardif_ifindex, hardif_buf);
		if (!hardif_name)
			return;
	}

	printf("%s (%s): set hardif:\n", meshif_name, hardif_name);

	if (attrs[BATADV_ATTR_HOP_PENALTY])
		printf("* hop_penalty %u\n",
		       nla_get_u8(attrs[BATADV_ATTR_HOP_PENALTY]));

	if (attrs[BATADV_ATTR_ELP_INTERVAL])
		printf("* elp_interval %u ms\n",
		       nla_get_u32(attrs[BATADV_ATTR_ELP_INTERVAL]));

	if (attrs[BATADV_ATTR_THROUGHPUT_OVERRIDE]) {
		uint32_t val;

		val = nla_get_u32(attrs[BATADV_ATTR_THROUGHPUT_OVERRIDE]);
		printf("* throughput_override %u.%01u MBit/s\n", val / 10,
		       val % 10);
	}
}

static void event_parse_set_vlan(struct nlattr **attrs)
{
	static const int vlan_mandatory[] = {
		BATADV_ATTR_MESH_IFINDEX,
		BATADV_ATTR_VLANID,
	};
	char meshif_buf[IF_NAMESIZE];
	char *meshif_name;
	uint32_t mesh_ifindex;
	uint16_t vid;

	/* ignore entry when attributes are missing */
	if (missing_mandatory_attrs(attrs, vlan_mandatory,
				    ARRAY_SIZE(vlan_mandatory)))
		return;

	if (attrs[BATADV_ATTR_MESH_IFNAME]) {
		meshif_name = nla_get_string(attrs[BATADV_ATTR_MESH_IFNAME]);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		mesh_ifindex = nla_get_u32(attrs[BATADV_ATTR_MESH_IFINDEX]);
		meshif_name = if_indextoname(mesh_ifindex, meshif_buf);
		if (!meshif_name)
			return;
	}

	vid = nla_get_u16(attrs[BATADV_ATTR_VLANID]);

	printf("%s (vid %u): set vlan:\n", meshif_name, vid);

	if (attrs[BATADV_ATTR_AP_ISOLATION_ENABLED])
		printf("* ap_isolation %s\n",
		       u8_to_boolstr(attrs[BATADV_ATTR_AP_ISOLATION_ENABLED]));
}

static unsigned long long get_timestamp(struct event_args *event_args)
{
	unsigned long long prevtime = 0;
	unsigned long long now;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	now = 1000000ULL * tv.tv_sec + tv.tv_usec;

	if (event_args->mode == EVENT_TIME_RELATIVE) {
		prevtime = 1000000ULL * event_args->tv.tv_sec + event_args->tv.tv_usec;
		event_args->tv = tv;
	}

	return now - prevtime;
}

static int event_parse(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs[NUM_BATADV_ATTR];
	struct event_args *event_args = arg;
	unsigned long long timestamp;
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		return NL_OK;
	}

	if (event_args->mode != EVENT_TIME_NO) {
		timestamp = get_timestamp(event_args);
		printf("%llu.%06llu: ", timestamp / 1000000, timestamp % 1000000);
	}

	switch (ghdr->cmd) {
	case BATADV_CMD_TP_METER:
		event_parse_tp_meter(attrs);
		break;
	case BATADV_CMD_SET_MESH:
		event_parse_set_mesh(attrs);
		break;
	case BATADV_CMD_SET_HARDIF:
		event_parse_set_hardif(attrs);
		break;
	case BATADV_CMD_SET_VLAN:
		event_parse_set_vlan(attrs);
		break;
	default:
		printf("Received unknown event %u\n", ghdr->cmd);
		break;
	}

	return NL_OK;
}

static int event(struct state *state, int argc, char **argv)
{
	struct event_args event_args = {
		.mode = EVENT_TIME_NO,
	};
	int opt;
	int ret;

	while ((opt = getopt(argc, argv, "htr")) != -1) {
		switch (opt) {
		case 'h':
			event_usage();
			return EXIT_SUCCESS;
		case 't':
			event_args.mode = EVENT_TIME_LOCAL;
			break;
		case 'r':
			event_args.mode = EVENT_TIME_RELATIVE;
			break;
		default:
			event_usage();
			return  EXIT_FAILURE;
		}
	}

	ret = event_prepare(state);
	if (ret < 0) {
		fprintf(stderr, "Failed to prepare event netlink: %s (%d)\n",
			strerror(-ret), -ret);
		return 1;
	}

	if (event_args.mode == EVENT_TIME_RELATIVE)
		get_timestamp(&event_args);

	nl_cb_set(state->cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(state->cb, NL_CB_VALID, NL_CB_CUSTOM, event_parse, &event_args);

	while (1)
		nl_recvmsgs(state->sock, state->cb);

	return 0;
}

COMMAND(SUBCOMMAND, event, "e", COMMAND_FLAG_NETLINK, NULL,
	"                  \tdisplay events from batman-adv");
