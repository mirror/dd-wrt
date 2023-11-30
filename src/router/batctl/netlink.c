// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>, Andrew Lunn <andrew@lunn.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include "netlink.h"
#include "main.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bat-hosts.h"
#include "batadv_packet.h"
#include "batman_adv.h"
#include "netlink.h"
#include "functions.h"
#include "main.h"

/* WARNING: attributes must also be added to batadv_genl_json */
struct nla_policy batadv_netlink_policy[NUM_BATADV_ATTR] = {
	[BATADV_ATTR_VERSION] = {
		.type = NLA_STRING,
	},
	[BATADV_ATTR_ALGO_NAME] = {
		.type = NLA_STRING,
	},
	[BATADV_ATTR_MESH_IFINDEX] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_MESH_IFNAME] = {
		.type = NLA_STRING,
		.maxlen = IFNAMSIZ,
	},
	[BATADV_ATTR_MESH_ADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_HARD_IFINDEX] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_HARD_IFNAME] = {
		.type = NLA_STRING,
		.maxlen = IFNAMSIZ,
	},
	[BATADV_ATTR_HARD_ADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_ORIG_ADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_TPMETER_RESULT] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_TPMETER_TEST_TIME] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_TPMETER_BYTES] = {
		.type = NLA_U64,
	},
	[BATADV_ATTR_TPMETER_COOKIE] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_PAD] = {
		.type = NLA_UNSPEC,
	},
	[BATADV_ATTR_ACTIVE] = {
		.type = NLA_FLAG,
	},
	[BATADV_ATTR_TT_ADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_TT_TTVN] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_TT_LAST_TTVN] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_TT_CRC32] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_TT_VID] = {
		.type = NLA_U16,
	},
	[BATADV_ATTR_TT_FLAGS] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_FLAG_BEST] = {
		.type = NLA_FLAG,
	},
	[BATADV_ATTR_LAST_SEEN_MSECS] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_NEIGH_ADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_TQ] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_THROUGHPUT] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_BANDWIDTH_UP] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_BANDWIDTH_DOWN] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_ROUTER] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_BLA_OWN] = {
		.type = NLA_FLAG,
	},
	[BATADV_ATTR_BLA_ADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_BLA_VID] = {
		.type = NLA_U16,
	},
	[BATADV_ATTR_BLA_BACKBONE] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_BLA_CRC] = {
		.type = NLA_U16,
	},
	[BATADV_ATTR_DAT_CACHE_IP4ADDRESS] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_DAT_CACHE_HWADDRESS] = {
		.type = NLA_UNSPEC,
		.minlen = ETH_ALEN,
		.maxlen = ETH_ALEN,
	},
	[BATADV_ATTR_DAT_CACHE_VID] = {
		.type = NLA_U16,
	},
	[BATADV_ATTR_MCAST_FLAGS] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_MCAST_FLAGS_PRIV] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_VLANID] = {
		.type = NLA_U16,
	},
	[BATADV_ATTR_AGGREGATED_OGMS_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_AP_ISOLATION_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_ISOLATION_MARK] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_ISOLATION_MASK] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_BONDING_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_BRIDGE_LOOP_AVOIDANCE_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_DISTRIBUTED_ARP_TABLE_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_FRAGMENTATION_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_GW_BANDWIDTH_DOWN] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_GW_BANDWIDTH_UP] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_GW_MODE] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_GW_SEL_CLASS] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_HOP_PENALTY] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_LOG_LEVEL] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_NETWORK_CODING_ENABLED] = {
		.type = NLA_U8,
	},
	[BATADV_ATTR_ORIG_INTERVAL] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_ELP_INTERVAL] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_THROUGHPUT_OVERRIDE] = {
		.type = NLA_U32,
	},
	[BATADV_ATTR_MULTICAST_FANOUT] = {
		.type = NLA_U32,
	},
};

int netlink_create(struct state *state)
{
	int ret;

	state->sock = NULL;
	state->cb = NULL;
	state->batadv_family = 0;

	state->sock = nl_socket_alloc();
	if (!state->sock)
		return -ENOMEM;

	ret = genl_connect(state->sock);
	if (ret < 0)
		goto err_free_sock;

	state->batadv_family = genl_ctrl_resolve(state->sock, BATADV_NL_NAME);
	if (state->batadv_family < 0) {
		ret = -EOPNOTSUPP;
		goto err_free_sock;
	}

	state->cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!state->cb) {
		ret = -ENOMEM;
		goto err_free_family;
	}

	return 0;

err_free_family:
	state->batadv_family = 0;

err_free_sock:
	nl_socket_free(state->sock);
	state->sock = NULL;

	return ret;
}

void netlink_destroy(struct state *state)
{
	if (state->cb) {
		nl_cb_put(state->cb);
		state->cb = NULL;
	}

	if (state->sock) {
		nl_socket_free(state->sock);
		state->sock = NULL;
	}
}

int last_err;
char algo_name_buf[256] = "";
int64_t mcast_flags = -EOPNOTSUPP;
int64_t mcast_flags_priv = -EOPNOTSUPP;

int missing_mandatory_attrs(struct nlattr *attrs[], const int mandatory[],
			    int num)
{
	int i;

	for (i = 0; i < num; i++)
		if (!attrs[mandatory[i]])
			return -EINVAL;

	return 0;
}

int netlink_print_error(struct sockaddr_nl *nla __maybe_unused,
			struct nlmsgerr *nlerr,	void *arg __maybe_unused)
{
	if (nlerr->error != -EOPNOTSUPP)
		fprintf(stderr, "Error received: %s\n",
			strerror(-nlerr->error));

	last_err = nlerr->error;

	return NL_STOP;
}

int netlink_stop_callback(struct nl_msg *msg, void *arg __maybe_unused)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	int *error = nlmsg_data(nlh);

	if (*error)
		fprintf(stderr, "Error received: %s\n", strerror(-*error));

	return NL_STOP;
}

static const int info_mandatory[] = {
	BATADV_ATTR_MESH_IFINDEX,
	BATADV_ATTR_MESH_IFNAME,
};

static const int info_hard_mandatory[] = {
	BATADV_ATTR_VERSION,
	BATADV_ATTR_ALGO_NAME,
	BATADV_ATTR_HARD_IFNAME,
	BATADV_ATTR_HARD_ADDRESS,
};

static int info_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct print_opts *opts = arg;
	const uint8_t *primary_mac;
	struct genlmsghdr *ghdr;
	const uint8_t *mesh_mac;
	const char *primary_if;
	const char *mesh_name;
	const char *version;
	char *extra_info = NULL;
	uint8_t ttvn = 0;
	uint16_t bla_group_id = 0;
	const char *algo_name;
	const char *extra_header;
	int ret;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_MESH_INFO)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, info_mandatory,
				    ARRAY_SIZE(info_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	mesh_name = nla_get_string(attrs[BATADV_ATTR_MESH_IFNAME]);
	mesh_mac = nla_data(attrs[BATADV_ATTR_MESH_ADDRESS]);

	if (attrs[BATADV_ATTR_HARD_IFNAME]) {
		if (missing_mandatory_attrs(attrs, info_hard_mandatory,
					    ARRAY_SIZE(info_hard_mandatory))) {
			fputs("Missing attributes from kernel\n",
			      stderr);
			exit(1);
		}

		version = nla_get_string(attrs[BATADV_ATTR_VERSION]);
		algo_name = nla_get_string(attrs[BATADV_ATTR_ALGO_NAME]);
		primary_if = nla_get_string(attrs[BATADV_ATTR_HARD_IFNAME]);
		primary_mac = nla_data(attrs[BATADV_ATTR_HARD_ADDRESS]);

		snprintf(algo_name_buf, sizeof(algo_name_buf), "%s", algo_name);

		if (attrs[BATADV_ATTR_TT_TTVN])
			ttvn = nla_get_u8(attrs[BATADV_ATTR_TT_TTVN]);

		if (attrs[BATADV_ATTR_BLA_CRC])
			bla_group_id = nla_get_u16(attrs[BATADV_ATTR_BLA_CRC]);

		if (attrs[BATADV_ATTR_MCAST_FLAGS])
			mcast_flags = nla_get_u32(attrs[BATADV_ATTR_MCAST_FLAGS]);
		else
			mcast_flags = -EOPNOTSUPP;

		if (attrs[BATADV_ATTR_MCAST_FLAGS_PRIV])
			mcast_flags_priv = nla_get_u32(attrs[BATADV_ATTR_MCAST_FLAGS_PRIV]);
		else
			mcast_flags_priv = -EOPNOTSUPP;

		switch (opts->nl_cmd) {
		case BATADV_CMD_GET_TRANSTABLE_LOCAL:
			ret = asprintf(&extra_info, ", TTVN: %u", ttvn);
			if (ret < 0)
				extra_info = NULL;
			break;
		case BATADV_CMD_GET_BLA_BACKBONE:
		case BATADV_CMD_GET_BLA_CLAIM:
			ret = asprintf(&extra_info, ", group id: 0x%04x",
				       bla_group_id);
			if (ret < 0)
				extra_info = NULL;
			break;
		default:
			extra_info = strdup("");
			break;
		}

		if (opts->static_header)
			extra_header = opts->static_header;
		else
			extra_header = "";

		ret = asprintf(&opts->remaining_header,
			       "[B.A.T.M.A.N. adv %s, MainIF/MAC: %s/%02x:%02x:%02x:%02x:%02x:%02x (%s/%02x:%02x:%02x:%02x:%02x:%02x %s)%s]\n%s",
			       version, primary_if,
			       primary_mac[0], primary_mac[1], primary_mac[2],
			       primary_mac[3], primary_mac[4], primary_mac[5],
			       mesh_name,
			       mesh_mac[0], mesh_mac[1], mesh_mac[2],
			       mesh_mac[3], mesh_mac[4], mesh_mac[5],
			       algo_name, extra_info, extra_header);
		if (ret < 0)
			opts->remaining_header = NULL;

		if (extra_info)
			free(extra_info);
	} else {
		ret = asprintf(&opts->remaining_header,
			       "BATMAN mesh %s disabled\n", mesh_name);
		if (ret < 0)
			opts->remaining_header = NULL;
	}

	return NL_OK;
}

char *netlink_get_info(struct state *state, uint8_t nl_cmd, const char *header)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	struct print_opts opts = {
		.read_opt = 0,
		.nl_cmd = nl_cmd,
		.remaining_header = NULL,
		.static_header = header,
	};
	int ret;

	msg = nlmsg_alloc();
	if (!msg)
		return NULL;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family, 0, 0,
		    BATADV_CMD_GET_MESH_INFO, 1);

	nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, state->mesh_ifindex);

	nl_send_auto_complete(state->sock, msg);

	nlmsg_free(msg);

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		return NULL;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, info_callback, &opts);
	nl_cb_err(cb, NL_CB_CUSTOM, netlink_print_error, NULL);

	ret = nl_recvmsgs(state->sock, cb);
	if (ret < 0)
		return opts.remaining_header;

	nl_wait_for_ack(state->sock);

	return opts.remaining_header;
}

void netlink_print_remaining_header(struct print_opts *opts)
{
	if (!opts->remaining_header)
		return;

	fputs(opts->remaining_header, stdout);
	free(opts->remaining_header);
	opts->remaining_header = NULL;
}

int netlink_print_common_cb(struct nl_msg *msg, void *arg)
{
	struct print_opts *opts = arg;

	netlink_print_remaining_header(opts);

	return opts->callback(msg, arg);
}

int netlink_print_common(struct state *state, char *orig_iface, int read_opt,
			 float orig_timeout, float watch_interval,
			 const char *header, uint8_t nl_cmd,
			 nl_recvmsg_msg_cb_t callback)
{
	struct print_opts opts = {
		.read_opt = read_opt,
		.orig_timeout = orig_timeout,
		.watch_interval = watch_interval,
		.remaining_header = NULL,
		.callback = callback,
	};
	int hardifindex = 0;
	struct nl_msg *msg;

	if (!state->sock) {
		last_err = -EOPNOTSUPP;
		return last_err;
	}

	if (orig_iface) {
		hardifindex = if_nametoindex(orig_iface);
		if (!hardifindex) {
			fprintf(stderr, "Interface %s is unknown\n",
				orig_iface);
			last_err = -ENODEV;
			return last_err;
		}
	}

	bat_hosts_init(read_opt);

	nl_cb_set(state->cb, NL_CB_VALID, NL_CB_CUSTOM, netlink_print_common_cb, &opts);
	nl_cb_set(state->cb, NL_CB_FINISH, NL_CB_CUSTOM, netlink_stop_callback, NULL);
	nl_cb_err(state->cb, NL_CB_CUSTOM, netlink_print_error, NULL);

	do {
		if (read_opt & CLR_CONT_READ)
			/* clear screen, set cursor back to 0,0 */
			printf("\033[2J\033[0;0f");

		if (!(read_opt & SKIP_HEADER))
			opts.remaining_header = netlink_get_info(state,
								 nl_cmd,
								 header);

		msg = nlmsg_alloc();
		if (!msg)
			continue;

		genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family,
			    0, NLM_F_DUMP, nl_cmd, 1);

		nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, state->mesh_ifindex);
		if (hardifindex)
			nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX,
				    hardifindex);

		nl_send_auto_complete(state->sock, msg);

		nlmsg_free(msg);

		last_err = 0;
		nl_recvmsgs(state->sock, state->cb);

		/* the header should still be printed when no entry was received */
		if (!last_err)
			netlink_print_remaining_header(&opts);

		if (!last_err && read_opt & (CONT_READ|CLR_CONT_READ))
			usleep(1000000 * watch_interval);

	} while (!last_err && read_opt & (CONT_READ|CLR_CONT_READ));

	bat_hosts_free();

	return last_err;
}

static int nlquery_error_cb(struct sockaddr_nl *nla __maybe_unused,
			    struct nlmsgerr *nlerr, void *arg)
{
	struct nlquery_opts *query_opts = arg;

	query_opts->err = nlerr->error;

	return NL_STOP;
}

static int nlquery_stop_cb(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlquery_opts *query_opts = arg;
	int *error = nlmsg_data(nlh);

	if (*error)
		query_opts->err = *error;

	return NL_STOP;
}

int netlink_query_common(struct state *state,
			 unsigned int mesh_ifindex, uint8_t nl_cmd,
			 nl_recvmsg_msg_cb_t callback,
			 nl_recvmsg_msg_cb_t attribute_cb,
			 int flags, struct nlquery_opts *query_opts)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int ret;

	query_opts->err = 0;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		return -ENOMEM;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, callback, query_opts);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, nlquery_stop_cb, query_opts);
	nl_cb_err(cb, NL_CB_CUSTOM, nlquery_error_cb, query_opts);

	msg = nlmsg_alloc();
	if (!msg) {
		query_opts->err = -ENOMEM;
		goto err_free_cb;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family, 0,
		    flags, nl_cmd, 1);

	nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, mesh_ifindex);

	if (attribute_cb) {
		ret = attribute_cb(msg, state);
		if (ret < 0) {
			nlmsg_free(msg);
			goto err_free_cb;
		}
	}

	nl_send_auto_complete(state->sock, msg);
	nlmsg_free(msg);

	ret = nl_recvmsgs(state->sock, cb);
	if (ret < 0) {
		query_opts->err = ret;
		goto err_free_cb;
	}

	if (!(flags & NLM_F_DUMP))
		nl_wait_for_ack(state->sock);

err_free_cb:
	nl_cb_put(cb);

	return query_opts->err;
}

static const int translate_mac_netlink_mandatory[] = {
	BATADV_ATTR_TT_ADDRESS,
	BATADV_ATTR_ORIG_ADDRESS,
};

struct translate_mac_netlink_opts {
	struct ether_addr mac;
	uint8_t found:1;
	struct nlquery_opts query_opts;
};

static int translate_mac_netlink_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlquery_opts *query_opts = arg;
	struct translate_mac_netlink_opts *opts;
	struct genlmsghdr *ghdr;
	uint8_t *addr;
	uint8_t *orig;

	opts = container_of(query_opts, struct translate_mac_netlink_opts,
			    query_opts);

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_TRANSTABLE_GLOBAL)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (missing_mandatory_attrs(attrs, translate_mac_netlink_mandatory,
				    ARRAY_SIZE(translate_mac_netlink_mandatory)))
		return NL_OK;

	addr = nla_data(attrs[BATADV_ATTR_TT_ADDRESS]);
	orig = nla_data(attrs[BATADV_ATTR_ORIG_ADDRESS]);

	if (!attrs[BATADV_ATTR_FLAG_BEST])
		return NL_OK;

	if (memcmp(&opts->mac, addr, ETH_ALEN) != 0)
		return NL_OK;

	memcpy(&opts->mac, orig, ETH_ALEN);
	opts->found = true;
	opts->query_opts.err = 0;

	return NL_OK;
}

int translate_mac_netlink(struct state *state, const struct ether_addr *mac,
			  struct ether_addr *mac_out)
{
	struct translate_mac_netlink_opts opts = {
		.found = false,
		.query_opts = {
			.err = 0,
		},
	};
	int ret;

	memcpy(&opts.mac, mac, ETH_ALEN);

	ret = netlink_query_common(state, state->mesh_ifindex,
				   BATADV_CMD_GET_TRANSTABLE_GLOBAL,
			           translate_mac_netlink_cb, NULL, NLM_F_DUMP,
				   &opts.query_opts);
	if (ret < 0)
		return ret;

	if (!opts.found)
		return -ENOENT;

	memcpy(mac_out, &opts.mac, ETH_ALEN);

	return 0;
}

static const int get_nexthop_netlink_mandatory[] = {
	BATADV_ATTR_ORIG_ADDRESS,
	BATADV_ATTR_NEIGH_ADDRESS,
	BATADV_ATTR_HARD_IFINDEX,
};

struct get_nexthop_netlink_opts {
	struct ether_addr mac;
	uint8_t *nexthop;
	char *ifname;
	uint8_t found:1;
	struct nlquery_opts query_opts;
};

static int get_nexthop_netlink_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlquery_opts *query_opts = arg;
	struct get_nexthop_netlink_opts *opts;
	struct genlmsghdr *ghdr;
	const uint8_t *orig;
	const uint8_t *neigh;
	uint32_t index;
	const char *ifname;

	opts = container_of(query_opts, struct get_nexthop_netlink_opts,
			    query_opts);

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_ORIGINATORS)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (missing_mandatory_attrs(attrs, get_nexthop_netlink_mandatory,
				    ARRAY_SIZE(get_nexthop_netlink_mandatory)))
		return NL_OK;

	orig = nla_data(attrs[BATADV_ATTR_ORIG_ADDRESS]);
	neigh = nla_data(attrs[BATADV_ATTR_NEIGH_ADDRESS]);
	index = nla_get_u32(attrs[BATADV_ATTR_HARD_IFINDEX]);

	if (!attrs[BATADV_ATTR_FLAG_BEST])
		return NL_OK;

	if (memcmp(&opts->mac, orig, ETH_ALEN) != 0)
		return NL_OK;

	/* save result */
	memcpy(opts->nexthop, neigh, ETH_ALEN);

	if (attrs[BATADV_ATTR_HARD_IFNAME]) {
		ifname = nla_get_string(attrs[BATADV_ATTR_HARD_IFNAME]);
		strncpy(opts->ifname, ifname, IFNAMSIZ);
	} else {
		/* compatibility for Linux < 5.14/batman-adv < 2021.2 */
		ifname = if_indextoname(index, opts->ifname);
		if (!ifname)
			return NL_OK;
	}

	opts->found = true;
	opts->query_opts.err = 0;

	return NL_OK;
}

int get_nexthop_netlink(struct state *state, const struct ether_addr *mac,
			uint8_t *nexthop, char *ifname)
{
	struct get_nexthop_netlink_opts opts = {
		.nexthop = 0,
		.found = false,
		.query_opts = {
			.err = 0,
		},
	};
	int ret;

	memcpy(&opts.mac, mac, ETH_ALEN);
	opts.nexthop = nexthop;
	opts.ifname = ifname;

	ret = netlink_query_common(state, state->mesh_ifindex,
				   BATADV_CMD_GET_ORIGINATORS,
			           get_nexthop_netlink_cb, NULL, NLM_F_DUMP,
				   &opts.query_opts);
	if (ret < 0)
		return ret;

	if (!opts.found)
		return -ENOENT;

	return 0;
}

static const int get_primarymac_netlink_mandatory[] = {
	BATADV_ATTR_HARD_ADDRESS,
};

struct get_primarymac_netlink_opts {
	uint8_t *primarymac;
	uint8_t found:1;
	struct nlquery_opts query_opts;
};

static int get_primarymac_netlink_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlquery_opts *query_opts = arg;
	struct get_primarymac_netlink_opts *opts;
	struct genlmsghdr *ghdr;
	const uint8_t *primary_mac;

	opts = container_of(query_opts, struct get_primarymac_netlink_opts,
			    query_opts);

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_MESH_INFO)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (missing_mandatory_attrs(attrs, get_primarymac_netlink_mandatory,
				    ARRAY_SIZE(get_primarymac_netlink_mandatory)))
		return NL_OK;

	primary_mac = nla_data(attrs[BATADV_ATTR_HARD_ADDRESS]);

	/* save result */
	memcpy(opts->primarymac, primary_mac, ETH_ALEN);

	opts->found = true;
	opts->query_opts.err = 0;

	return NL_OK;
}

int get_primarymac_netlink(struct state *state, uint8_t *primarymac)
{
	struct get_primarymac_netlink_opts opts = {
		.primarymac = 0,
		.found = false,
		.query_opts = {
			.err = 0,
		},
	};
	int ret;

	opts.primarymac = primarymac;

	ret = netlink_query_common(state, state->mesh_ifindex,
				   BATADV_CMD_GET_MESH_INFO,
			           get_primarymac_netlink_cb, NULL, 0,
				   &opts.query_opts);
	if (ret < 0)
		return ret;

	if (!opts.found)
		return -ENOENT;

	return 0;
}

struct get_algoname_netlink_opts {
	char *algoname;
	size_t algoname_len;
	uint8_t found:1;
	struct nlquery_opts query_opts;
};

static int get_algoname_netlink_cb(struct nl_msg *msg, void *arg)
{
	static const int mandatory[] = {
		BATADV_ATTR_ALGO_NAME,
	};
	struct nlattr *attrs[BATADV_ATTR_MAX + 1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlquery_opts *query_opts = arg;
	struct get_algoname_netlink_opts *opts;
	struct genlmsghdr *ghdr;
	const char *algoname;

	opts = container_of(query_opts, struct get_algoname_netlink_opts,
			    query_opts);

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_MESH)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (missing_mandatory_attrs(attrs, mandatory, ARRAY_SIZE(mandatory)))
		return NL_OK;

	algoname = nla_data(attrs[BATADV_ATTR_ALGO_NAME]);

	/* save result */
	strncpy(opts->algoname, algoname, opts->algoname_len);
	if (opts->algoname_len > 0)
		opts->algoname[opts->algoname_len - 1] = '\0';

	opts->found = true;
	opts->query_opts.err = 0;

	return NL_OK;
}

int get_algoname_netlink(struct state *state, unsigned int mesh_ifindex,
			 char *algoname, size_t algoname_len)
{
	struct get_algoname_netlink_opts opts = {
		.algoname = algoname,
		.algoname_len = algoname_len,
		.found = false,
		.query_opts = {
			.err = 0,
		},
	};
	int ret;

	ret = netlink_query_common(state, mesh_ifindex, BATADV_CMD_GET_MESH,
			           get_algoname_netlink_cb, NULL, 0,
				   &opts.query_opts);
	if (ret < 0)
		return ret;

	if (!opts.found)
		return -EOPNOTSUPP;

	return 0;
}
