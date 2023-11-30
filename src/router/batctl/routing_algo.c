// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <netinet/if_ether.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "batadv_packet.h"
#include "batman_adv.h"
#include "debug.h"
#include "functions.h"
#include "main.h"
#include "netlink.h"
#include "sys.h"

#define SYS_SELECTED_RA_PATH	"/sys/module/batman_adv/parameters/routing_algo"

static void ra_mode_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] routing_algo [algorithm]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, " \t -h print this help\n");
}

static const int routing_algos_mandatory[] = {
	BATADV_ATTR_ALGO_NAME,
};

static int routing_algos_callback(struct nl_msg *msg, void *arg __maybe_unused)
{
	struct nlattr *attrs[BATADV_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr;
	const char *algo_name;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != BATADV_CMD_GET_ROUTING_ALGOS)
		return NL_OK;

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	if (missing_mandatory_attrs(attrs, routing_algos_mandatory,
				    ARRAY_SIZE(routing_algos_mandatory))) {
		fputs("Missing attributes from kernel\n", stderr);
		exit(1);
	}

	algo_name = nla_get_string(attrs[BATADV_ATTR_ALGO_NAME]);

	printf(" * %s\n", algo_name);

	return NL_OK;
}

static int print_routing_algos(struct state *state)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	struct print_opts opts = {
		.callback = routing_algos_callback,
	};

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family, 0,
		    NLM_F_DUMP, BATADV_CMD_GET_ROUTING_ALGOS, 1);
	nl_send_auto_complete(state->sock, msg);
	nlmsg_free(msg);

	opts.remaining_header = strdup("Available routing algorithms:\n");

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		return -ENOMEM;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, netlink_print_common_cb,
		  &opts);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, netlink_stop_callback, NULL);
	nl_cb_err(cb, NL_CB_CUSTOM, netlink_print_error, NULL);

	nl_recvmsgs(state->sock, cb);

	if (!last_err)
		netlink_print_remaining_header(&opts);

	return last_err;
}

static int write_default_ra(const char *full_path, const char *arg1)
{
	ssize_t write_len;
	int fd = -1;

	fd = open(full_path, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Error - can't open file '%s': %s\n", full_path,
			strerror(errno));

		return EXIT_FAILURE;
	}

	write_len = write(fd, arg1, strlen(arg1) + 1);
	close(fd);
	if (write_len < 0) {
		fprintf(stderr, "Error - can't write to file '%s': %s\n",
			full_path, strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static struct nla_policy link_policy[IFLA_MAX + 1] = {
	[IFLA_IFNAME] = { .type = NLA_STRING, .maxlen = IFNAMSIZ },
};

struct print_ra_interfaces_rtnl_arg {
	uint8_t header_shown:1;
	struct state *state;
};

static int print_ra_interfaces_rtnl_parse(struct nl_msg *msg, void *arg)
{
	struct print_ra_interfaces_rtnl_arg *print_arg = arg;
	struct nlattr *attrs[IFLA_MAX + 1];
	char algoname[256];
	struct ifinfomsg *ifm;
	char *mesh_iface;
	int ret;

	ifm = nlmsg_data(nlmsg_hdr(msg));
	ret = nlmsg_parse(nlmsg_hdr(msg), sizeof(*ifm), attrs, IFLA_MAX,
			  link_policy);
	if (ret < 0)
		goto err;

	if (!attrs[IFLA_IFNAME])
		goto err;

	mesh_iface = nla_get_string(attrs[IFLA_IFNAME]);

	ret = get_algoname_netlink(print_arg->state, ifm->ifi_index, algoname,
				   sizeof(algoname));
	if (ret < 0)
		goto err;

	if(!print_arg->header_shown) {
		print_arg->header_shown = true;
		printf("Active routing protocol configuration:\n");
	}

	printf(" * %s: %s\n", mesh_iface, algoname);

err:
	return NL_OK;
}

static int print_ra_interfaces(struct state *state)
{
	struct print_ra_interfaces_rtnl_arg print_arg = {
		.state = state,
	};

	struct ifinfomsg rt_hdr = {
		.ifi_family = IFLA_UNSPEC,
	};
	struct nlattr *linkinfo;
	struct nl_sock *sock;
	struct nl_msg *msg;
	struct nl_cb *cb;
	int err = 0;
	int ret;

	sock = nl_socket_alloc();
	if (!sock)
		return -ENOMEM;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_sock;
	}

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		err = -ENOMEM;
		goto err_free_sock;
	}

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_ra_interfaces_rtnl_parse,
		 &print_arg);

	msg = nlmsg_alloc_simple(RTM_GETLINK, NLM_F_REQUEST | NLM_F_DUMP);
	if (!msg) {
		err = -ENOMEM;
		goto err_free_cb;
	}

	ret = nlmsg_append(msg, &rt_hdr, sizeof(rt_hdr), NLMSG_ALIGNTO);
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	linkinfo = nla_nest_start(msg, IFLA_LINKINFO);
	if (!linkinfo) {
		err = -ENOMEM;
		goto err_free_msg;
	}

	ret = nla_put_string(msg, IFLA_INFO_KIND, "batadv");
	if (ret < 0) {
		err = -ENOMEM;
		goto err_free_msg;
	}
	nla_nest_end(msg, linkinfo);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto err_free_msg;

	nl_recvmsgs(sock, cb);

	if (print_arg.header_shown)
		printf("\n");

err_free_msg:
	nlmsg_free(msg);
err_free_cb:
	nl_cb_put(cb);
err_free_sock:
	nl_socket_free(sock);

	return err;
}

static int routing_algo(struct state *state, int argc, char **argv)
{
	int optchar;
	int res = EXIT_FAILURE;
	int ret;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			ra_mode_usage();
			return EXIT_SUCCESS;
		default:
			ra_mode_usage();
			return EXIT_FAILURE;
		}
	}

	check_root_or_die("batctl routing_algo");

	if (argc == 2)
		return write_default_ra(SYS_SELECTED_RA_PATH, argv[1]);

	/* duplicated code here from the main() because interface doesn't always
	 * need COMMAND_FLAG_MESH_IFACE and COMMAND_FLAG_NETLINK
	 */
	ret = netlink_create(state);
	if (ret < 0)
		return EXIT_FAILURE;

	print_ra_interfaces(state);

	res = read_file(SYS_SELECTED_RA_PATH, USE_READ_BUFF);
	if (res != EXIT_SUCCESS)
		goto err_free_netlink;

	printf("Selected routing algorithm (used when next batX interface is created):\n");
	printf(" => %s\n", line_ptr);
	free(line_ptr);
	line_ptr = NULL;

	print_routing_algos(state);
	res = EXIT_SUCCESS;

err_free_netlink:
	netlink_destroy(state);

	return res;
}

COMMAND(SUBCOMMAND, routing_algo, "ra", 0, NULL,
	"[mode]            \tdisplay or modify the routing algorithm");
