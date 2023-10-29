/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2018 Avast software
 */

#include "nl-default.h"

#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/ct.h>

struct private_nl_object
{
	int			ce_refcnt;
	struct nl_object_ops *	ce_ops;
	struct nl_cache *	ce_cache;
	struct nl_list_head	ce_list;
	int			ce_msgtype;
	int			ce_flags;
	uint64_t		ce_mask;
};

static void nf_conntrack_parse_callback(struct nl_object *obj, void *opaque)
{
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_DETAILS,
	};

	nl_object_dump(obj, &params);
}

static int nf_conntrack_event_callback(struct nl_msg *msg, void *opaque)
{
	int err;
	struct nlmsghdr *hdr = nlmsg_hdr(msg);

	enum cntl_msg_types type = (enum cntl_msg_types) NFNL_MSG_TYPE(hdr->nlmsg_type);

	int flags = hdr->nlmsg_flags;

	if (type == IPCTNL_MSG_CT_DELETE) {
		printf("DELETE ");
	} else if (type == IPCTNL_MSG_CT_NEW) {
		if (flags & (NLM_F_CREATE|NLM_F_EXCL)) {
			printf("NEW ");
		} else {
			printf("UPDATE ");
		}
	} else {
		printf("UNKNOWN ");
	}

	if ((err = nl_msg_parse(msg, &nf_conntrack_parse_callback, opaque)) < 0) {
		nl_cli_fatal(err, "nl_msg_parse: %s", nl_geterror(err));
	}
	/* Continue with next event */
	return NL_OK;
}

int main(int argc, char *argv[])
{
	struct nl_sock *socket;
	int err;

	socket = nl_cli_alloc_socket();
	if (socket == NULL) {
		nl_cli_fatal(ENOBUFS, "Unable to allocate netlink socket");
	}

	/*
	 * Disable sequence number checking.
	 * This is required to allow messages to be processed which were not requested by
	 * a preceding request message, e.g. netlink events.
	 */
	nl_socket_disable_seq_check(socket);

	/* subscribe conntrack events */
	nl_join_groups(socket, NF_NETLINK_CONNTRACK_NEW |
												 NF_NETLINK_CONNTRACK_UPDATE |
												 NF_NETLINK_CONNTRACK_DESTROY |
												 NF_NETLINK_CONNTRACK_EXP_NEW |
												 NF_NETLINK_CONNTRACK_EXP_UPDATE |
												 NF_NETLINK_CONNTRACK_EXP_DESTROY);

	nl_cli_connect(socket, NETLINK_NETFILTER);

	nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, &nf_conntrack_event_callback, 0);

	while (1) {

		errno = 0;
		if ((err = nl_recvmsgs_default(socket)) < 0) {
			switch (errno) {
				case 	ENOBUFS:
					// just print warning
					fprintf(stderr, "Lost events because of ENOBUFS\n");
					break;
				case EAGAIN:
				case EINTR:
					// continue reading
					break;
				default:
					nl_cli_fatal(err, "Failed to receive: %s", nl_geterror(err));
			}
		}
	}
}
