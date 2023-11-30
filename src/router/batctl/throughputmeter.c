// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli <a@unstable.cc>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include "main.h"

#include <netinet/ether.h>
#include <netinet/in.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "bat-hosts.h"
#include "batadv_packet.h"
#include "batman_adv.h"
#include "functions.h"
#include "genl.h"
#include "netlink.h"

static struct ether_addr *dst_mac;
static struct state *tp_state;

struct tp_result {
	int error;
	uint32_t cookie;
	uint8_t return_value;
	uint8_t found:1;
	uint32_t test_time;
	uint64_t total_bytes;
};

struct tp_cookie {
	int error;
	uint8_t found:1;
	uint32_t cookie;
};

static int tpmeter_nl_print_error(struct sockaddr_nl *nla __maybe_unused,
				  struct nlmsgerr *nlerr,
				  void *arg)
{
	struct tp_result *result = arg;

	if (nlerr->error != -EOPNOTSUPP)
		fprintf(stderr, "Error received: %s\n",
			strerror(-nlerr->error));

	result->error = nlerr->error;

	return NL_STOP;
}

static int tp_meter_result_callback(struct nl_msg *msg, void *arg)
{
	struct tp_result *result = arg;
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs[NUM_BATADV_ATTR];
	struct genlmsghdr *ghdr;
	uint32_t cookie;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		result->error = -EINVAL;
		return NL_STOP;
	}

	ghdr = nlmsg_data(nlh);
	if (ghdr->cmd != BATADV_CMD_TP_METER)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		result->error = -EINVAL;
		return NL_STOP;
	}

	if (!attrs[BATADV_ATTR_TPMETER_COOKIE]) {
		result->error = -EINVAL;
		return NL_STOP;
	}

	if (!attrs[BATADV_ATTR_TPMETER_RESULT])
		return NL_OK;

	cookie = nla_get_u32(attrs[BATADV_ATTR_TPMETER_COOKIE]);
	if (cookie != result->cookie)
		return NL_OK;

	result->found = true;

	result->return_value = nla_get_u8(attrs[BATADV_ATTR_TPMETER_RESULT]);

	if (attrs[BATADV_ATTR_TPMETER_TEST_TIME])
		result->test_time = nla_get_u32(attrs[BATADV_ATTR_TPMETER_TEST_TIME]);

	if (attrs[BATADV_ATTR_TPMETER_BYTES])
		result->total_bytes = nla_get_u64(attrs[BATADV_ATTR_TPMETER_BYTES]);

	return NL_OK;
}

static int tp_meter_cookie_callback(struct nl_msg *msg, void *arg)
{
	struct tp_cookie *cookie = arg;
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs[NUM_BATADV_ATTR];
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		cookie->error = -EINVAL;
		return NL_STOP;
	}

	ghdr = nlmsg_data(nlh);
	if (ghdr->cmd != BATADV_CMD_TP_METER) {
		cookie->error = -EINVAL;
		return NL_STOP;
	}

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		cookie->error = -EINVAL;
		return NL_STOP;
	}

	if (!attrs[BATADV_ATTR_TPMETER_COOKIE]) {
		cookie->error = -EINVAL;
		return NL_STOP;
	}

	cookie->cookie = nla_get_u32(attrs[BATADV_ATTR_TPMETER_COOKIE]);
	cookie->found = true;

	return NL_OK;
}


static int tp_meter_start(struct state *state, struct ether_addr *dst_mac,
			  uint32_t time, struct tp_cookie *cookie)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int err = 0;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, tp_meter_cookie_callback,
		  cookie);
	nl_cb_err(cb, NL_CB_CUSTOM, tpmeter_nl_print_error, cookie);

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family, 0,
		    0, BATADV_CMD_TP_METER, 1);

	nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, state->mesh_ifindex);
	nla_put(msg, BATADV_ATTR_ORIG_ADDRESS, ETH_ALEN, dst_mac);
	nla_put_u32(msg, BATADV_ATTR_TPMETER_TEST_TIME, time);

	nl_send_auto_complete(state->sock, msg);
	nlmsg_free(msg);

	nl_recvmsgs(state->sock, cb);

	nl_cb_put(cb);

	if (cookie->error < 0)
		err = cookie->error;
	else if (!cookie->found)
		err= -EINVAL;

	return err;
}

static int no_seq_check(struct nl_msg *msg __maybe_unused,
			void *arg __maybe_unused)
{
	return NL_OK;
}

static int tp_recv_result(struct nl_sock *sock, struct tp_result *result)
{
	int err = 0;
	struct nl_cb *cb;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, tp_meter_result_callback,
		  result);
	nl_cb_err(cb, NL_CB_CUSTOM, tpmeter_nl_print_error, result);

	while (result->error == 0 && !result->found)
		nl_recvmsgs(sock, cb);

	nl_cb_put(cb);

	if (result->error < 0)
		err = result->error;
	else if (!result->found)
		err= -EINVAL;

	return err;
}

static int tp_meter_stop(struct state *state, struct ether_addr *dst_mac)
{
	struct nl_msg *msg;

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family, 0,
		    0, BATADV_CMD_TP_METER_CANCEL, 1);

	nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, state->mesh_ifindex);
	nla_put(msg, BATADV_ATTR_ORIG_ADDRESS, ETH_ALEN, dst_mac);

	nl_send_auto_complete(state->sock, msg);
	nlmsg_free(msg);

	return 0;
}

static struct nl_sock *tp_prepare_listening_sock(void)
{
	struct nl_sock *sock;
	int family;
	int ret;
	int mcid;

	sock = nl_socket_alloc();
	if (!sock)
		return NULL;

	ret = genl_connect(sock);
	if (ret < 0) {
		fprintf(stderr, "Failed to connect to generic netlink: %d\n",
			ret);
		goto err;
	}

	family = genl_ctrl_resolve(sock, BATADV_NL_NAME);
	if (family < 0) {
		fprintf(stderr, "Failed to resolve batman-adv netlink: %d\n",
			family);
		goto err;
	}

	mcid = nl_get_multicast_id(sock, BATADV_NL_NAME,
				   BATADV_NL_MCAST_GROUP_TPMETER);
	if (mcid < 0) {
		fprintf(stderr, "Failed to resolve batman-adv tpmeter multicast group: %d\n",
			mcid);
		goto err;
	}

	ret = nl_socket_add_membership(sock, mcid);
	if (ret) {
		fprintf(stderr, "Failed to join batman-adv tpmeter multicast group: %d\n",
			ret);
		goto err;
	}

	return sock;

err:
	nl_socket_free(sock);

	return NULL;
}

void tp_sig_handler(int sig)
{
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		fflush(stdout);
		tp_meter_stop(tp_state, dst_mac);
		break;
	default:
		break;
	}
}

static void tp_meter_usage(void)
{
	fprintf(stderr, "Usage: batctl tp [parameters] <MAC>\n");
	fprintf(stderr, "Parameters:\n");
	fprintf(stderr, "\t -t <time> test length in milliseconds\n");
	fprintf(stderr, "\t -n don't convert addresses to bat-host names\n");
}

static int throughputmeter(struct state *state, int argc, char **argv)
{
	struct bat_host *bat_host;
	uint64_t throughput;
	char *dst_string;
	int ret = EXIT_FAILURE;
	int found_args = 1, read_opt = USE_BAT_HOSTS;
	uint32_t time = 0;
	int optchar;
	struct nl_sock *listen_sock = NULL;
	struct tp_result result = {
		.error = 0,
		.return_value = 0,
		.test_time = 0,
		.total_bytes =  0,
		.found = false,
	};
	struct tp_cookie cookie = {
		.error = 0,
		.cookie = 0,
		.found = false,
	};

	while ((optchar = getopt(argc, argv, "t:n")) != -1) {
		switch (optchar) {
		case 't':
			found_args += 2;
			time = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			found_args += 1;
			break;
		default:
			tp_meter_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc <= found_args) {
		tp_meter_usage();
		return EXIT_FAILURE;
	}

	check_root_or_die("batctl throughputmeter");

	dst_string = argv[found_args];
	bat_hosts_init(read_opt);
	bat_host = bat_hosts_find_by_name(dst_string);

	if (bat_host)
		dst_mac = &bat_host->mac_addr;

	if (!dst_mac) {
		dst_mac = ether_aton(dst_string);

		if (!dst_mac) {
			printf("Error - the tp meter destination is not a mac address or bat-host name: %s\n",
			       dst_string);
			goto out;
		}
	}


	if (bat_host && (read_opt & USE_BAT_HOSTS))
		dst_string = bat_host->name;
	else
		dst_string = ether_ntoa_long(dst_mac);

	/* for sighandler */
	tp_state = state;
	signal(SIGINT, tp_sig_handler);
	signal(SIGTERM, tp_sig_handler);

	listen_sock = tp_prepare_listening_sock();
	if (!listen_sock)
		goto out;

	ret = tp_meter_start(state, dst_mac, time, &cookie);
	if (ret < 0) {
		printf("Failed to send tp_meter request to kernel: %d\n", ret);
		goto out;
	}

	result.cookie = cookie.cookie;
	ret = tp_recv_result(listen_sock, &result);
	if (ret < 0) {
		printf("Failed to recv tp_meter result from kernel: %d\n", ret);
		goto out;
	}

	ret = EXIT_FAILURE;
	switch (result.return_value) {
	case BATADV_TP_REASON_DST_UNREACHABLE:
		fprintf(stderr, "Destination unreachable\n");
		break;
	case BATADV_TP_REASON_RESEND_LIMIT:
		fprintf(stderr,
			"The number of retry for the same window exceeds the limit, test aborted\n");
		break;
	case BATADV_TP_REASON_ALREADY_ONGOING:
		fprintf(stderr,
			"Cannot run two test towards the same node\n");
		break;
	case BATADV_TP_REASON_MEMORY_ERROR:
		fprintf(stderr,
			"Kernel cannot allocate memory, aborted\n");
		break;
	case BATADV_TP_REASON_TOO_MANY:
		fprintf(stderr, "Too many ongoing sessions\n");
		break;
	case BATADV_TP_REASON_CANCEL:
		printf("CANCEL received: test aborted\n");
		/* fall through */
	case BATADV_TP_REASON_COMPLETE:
		/* print the partial result */
		if (result.test_time > 0) {
			throughput = result.total_bytes * 1000;
			throughput /= result.test_time;
		} else {
			throughput = UINT64_MAX;
		}

		printf("Test duration %ums.\n", result.test_time);
		printf("Sent %" PRIu64 " Bytes.\n", result.total_bytes);
		printf("Throughput: ");
		if (throughput == UINT64_MAX)
			printf("inf\n");
		else if (throughput > (1UL<<30))
			printf("%.2f GB/s (%2.f Gbps)\n",
				(float)throughput / (1<<30),
				(float)throughput * 8 / 1000000000);
		else if (throughput > (1UL<<20))
			printf("%.2f MB/s (%.2f Mbps)\n",
				(float)throughput / (1<<20),
				(float)throughput * 8 / 1000000);
		else if (throughput > (1UL<<10))
			printf("%.2f KB/s (%.2f Kbps)\n",
				(float)throughput / (1<<10),
				(float)throughput * 8 / 1000);
		else
			printf("%" PRIu64 " Bytes/s (%" PRIu64 " Bps)\n",
			       throughput, throughput * 8);

		ret = 0;
		break;
	default:
		printf("Unrecognized return value %d\n", result.return_value);
	}

out:
	nl_socket_free(listen_sock);
	bat_hosts_free();
	return ret;
}

COMMAND(SUBCOMMAND_MIF, throughputmeter, "tp",
	COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK, NULL,
	"<destination>     \tstart a throughput measurement");
