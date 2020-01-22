// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <memory.h>
#include <glib.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/handlers.h>
#include <linux/genetlink.h>
#include <netlink/genl/mngt.h>

#include <linux/usmbd_server.h>

#include <usmbdtools.h>
#include <ipc.h>
#include <worker.h>
#include <config_parser.h>

static struct nl_sock *sk;

struct usmbd_ipc_msg *ipc_msg_alloc(size_t sz)
{
	struct usmbd_ipc_msg *msg;
	size_t msg_sz = sz + sizeof(struct usmbd_ipc_msg) + 1;

	if (msg_sz > USMBD_IPC_MAX_MESSAGE_SIZE)
		pr_err("IPC message is too large: %zu\n", msg_sz);

	msg = calloc(1, msg_sz);
	if (msg)
		msg->sz = sz;
	return msg;
}

void ipc_msg_free(struct usmbd_ipc_msg *msg)
{
	free(msg);
}

static int generic_event(int type, void *payload, size_t sz)
{
	struct usmbd_ipc_msg *event;

	event = ipc_msg_alloc(sz);
	if (!event)
		return -ENOMEM;

	event->type = type;
	event->sz = sz;

	memcpy(USMBD_IPC_MSG_PAYLOAD(event),
	       payload,
	       sz);
	wp_ipc_msg_push(event);
	return 0;
}

static int handle_generic_event(struct nl_cache_ops *unused,
				struct genl_cmd *cmd,
				struct genl_info *info,
				void *arg)
{
	if (!info->attrs[cmd->c_id])
		return NL_SKIP;

	return generic_event(cmd->c_id,
			    nla_data(info->attrs[cmd->c_id]),
			    nla_len(info->attrs[cmd->c_id]));
}

static int nlink_msg_cb(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = genlmsg_hdr(nlmsg_hdr(msg));

	if (gnlh->version != USMBD_GENL_VERSION) {
		pr_err("IPC message version mistamtch: %d\n", gnlh->version);
		return NL_SKIP;
	}

#if TRACING_DUMP_NL_MSG
	nl_msg_dump(msg, stdout);
#endif

	return genl_handle_msg(msg, NULL);
}

static int handle_unsupported_event(struct nl_cache_ops *unused,
				    struct genl_cmd *cmd,
				    struct genl_info *info,
				    void *arg)
{
	pr_err("Unsupported IPC event %d, ignore.\n", cmd->c_id);
	return NL_SKIP;
}

static int ifc_list_size(void)
{
	int len = 0;
	int i;

	for (i = 0; global_conf.interfaces[i] != NULL; i++) {
		char *ifc = global_conf.interfaces[i];

		ifc = cp_ltrim(ifc);
		if (!ifc)
			continue;

		len += strlen(ifc) + 1;
	}
	return len;
}

static int ipc_usmbd_starting_up(void)
{
	struct usmbd_startup_request *ev;
	struct usmbd_ipc_msg *msg;
	int ifc_list_sz = 0;
	int ret;

	if (global_conf.bind_interfaces_only && global_conf.interfaces)
		ifc_list_sz += ifc_list_size();

	msg = ipc_msg_alloc(sizeof(*ev) + ifc_list_sz);
	if (!msg)
		return -ENOMEM;

	ev = USMBD_IPC_MSG_PAYLOAD(msg);
	msg->type = USMBD_EVENT_STARTING_UP;

	ev->flags = global_conf.flags;
	ev->signing = global_conf.server_signing;
	ev->tcp_port = global_conf.tcp_port;
	ev->ipc_timeout = global_conf.ipc_timeout;
	ev->deadtime = global_conf.deadtime;
	ev->file_max = global_conf.file_max;
	ev->smb2_max_read = global_conf.smb2_max_read;
	ev->smb2_max_write = global_conf.smb2_max_write;
	ev->smb2_max_trans = global_conf.smb2_max_trans;

	if (global_conf.server_min_protocol) {
		strncpy(ev->min_prot,
			global_conf.server_min_protocol,
			sizeof(ev->min_prot) - 1);
	}
	if (global_conf.server_max_protocol) {
		strncpy(ev->max_prot,
			global_conf.server_max_protocol,
			sizeof(ev->max_prot) - 1);
	}
	if (global_conf.netbios_name) {
		strncpy(ev->netbios_name,
			global_conf.netbios_name,
			sizeof(ev->netbios_name) - 1);
	}
	if (global_conf.server_string) {
		strncpy(ev->server_string,
			global_conf.server_string,
			sizeof(ev->server_string) - 1);
	}
	if (global_conf.work_group) {
		strncpy(ev->work_group,
			global_conf.work_group,
			sizeof(ev->work_group) - 1);
	}

	if (ifc_list_sz) {
		int i;
		int sz = 0;
		char *config_payload = USMBD_STARTUP_CONFIG_INTERFACES(ev);

		ev->ifc_list_sz = ifc_list_sz;

		for (i = 0; global_conf.interfaces[i] != NULL; i++) {
			char *ifc = global_conf.interfaces[i];

			ifc = cp_ltrim(ifc);
			if (!ifc)
				continue;

			strcpy(config_payload + sz, ifc);
			sz += strlen(ifc) + 1;
		}

		global_conf.bind_interfaces_only = 0;
		cp_group_kv_list_free(global_conf.interfaces);
	}

	ret = ipc_msg_send(msg);
	ipc_msg_free(msg);
	return ret;
}

static int ipc_usmbd_shutting_down(void)
{
	return 0;
}

int ipc_process_event(void)
{
	int ret = 0;

	ret = nl_recvmsgs_default(sk);
	if (ret < 0) {
		pr_err("Recv() error %s [%d]\n", nl_geterror(ret), ret);
		return -USMBD_STATUS_IPC_FATAL_ERROR;
	}
	return ret;
}

static struct nla_policy usmbd_nl_policy[USMBD_EVENT_MAX] = {
	[USMBD_EVENT_UNSPEC] = {
		.minlen = 0,
	},

	[USMBD_EVENT_HEARTBEAT_REQUEST] = {
		.minlen = sizeof(struct usmbd_heartbeat),
	},

	[USMBD_EVENT_STARTING_UP] = {
		.minlen = sizeof(struct usmbd_startup_request),
	},

	[USMBD_EVENT_SHUTTING_DOWN] = {
		.minlen = sizeof(struct usmbd_startup_request),
	},

	[USMBD_EVENT_LOGIN_REQUEST] = {
		.minlen = sizeof(struct usmbd_login_request),
	},

	[USMBD_EVENT_LOGIN_RESPONSE] = {
		.minlen = sizeof(struct usmbd_login_response),
	},

	[USMBD_EVENT_SHARE_CONFIG_REQUEST] = {
		.minlen = sizeof(struct usmbd_share_config_request),
	},

	[USMBD_EVENT_SHARE_CONFIG_RESPONSE] = {
		.minlen = sizeof(struct usmbd_share_config_response),
	},

	[USMBD_EVENT_TREE_CONNECT_REQUEST] = {
		.minlen = sizeof(struct usmbd_tree_connect_request),
	},

	[USMBD_EVENT_TREE_CONNECT_RESPONSE] = {
		.minlen = sizeof(struct usmbd_tree_connect_response),
	},

	[USMBD_EVENT_TREE_DISCONNECT_REQUEST] = {
		.minlen = sizeof(struct usmbd_tree_disconnect_request),
	},

	[USMBD_EVENT_LOGOUT_REQUEST] = {
		.minlen = sizeof(struct usmbd_logout_request),
	},

	[USMBD_EVENT_RPC_REQUEST] = {
		.minlen = sizeof(struct usmbd_rpc_command),
	},

	[USMBD_EVENT_RPC_RESPONSE] = {
		.minlen = sizeof(struct usmbd_rpc_command),
	},
};

static struct genl_cmd usmbd_genl_cmds[] = {
	{
		.c_id		= USMBD_EVENT_UNSPEC,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_HEARTBEAT_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_STARTING_UP,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_SHUTTING_DOWN,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_LOGIN_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_LOGIN_RESPONSE,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_SHARE_CONFIG_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_SHARE_CONFIG_RESPONSE,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_TREE_CONNECT_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_TREE_CONNECT_RESPONSE,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_TREE_DISCONNECT_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_LOGOUT_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_RPC_REQUEST,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
	{
		.c_id		= USMBD_EVENT_RPC_RESPONSE,
		.c_attr_policy	= usmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= USMBD_EVENT_MAX,
	},
};

static struct genl_ops usmbd_family_ops = {
	.o_name = USMBD_GENL_NAME,
	.o_cmds = usmbd_genl_cmds,
	.o_ncmds = ARRAY_SIZE(usmbd_genl_cmds),
};

int ipc_msg_send(struct usmbd_ipc_msg *msg)
{
	struct nl_msg *nlmsg;
	struct nlmsghdr *hdr;
	int ret = -EINVAL;

	nlmsg = nlmsg_alloc();
	if (!nlmsg) {
		ret = -ENOMEM;
		goto out_error;
	}

	nlmsg_set_proto(nlmsg, NETLINK_GENERIC);
	hdr = genlmsg_put(nlmsg, getpid(), 0, usmbd_family_ops.o_id,
			  0, 0, msg->type, USMBD_GENL_VERSION);
	if (!hdr) {
		pr_err("genlmsg_put() has failed, aborting IPC send()\n");
		goto out_error;
	}

	/* Use msg->type as attribute TYPE */
	ret = nla_put(nlmsg, msg->type, msg->sz, USMBD_IPC_MSG_PAYLOAD(msg));
	if (ret) {
		pr_err("nla_put() has failed, aborting IPC send()\n");
		goto out_error;
	}

#if TRACING_DUMP_NL_MSG
	nl_msg_dump(nlmsg, stdout);
#endif

	nl_complete_msg(sk, nlmsg);
	ret = nl_send_auto(sk, nlmsg);
	if (ret > 0)
		ret = 0;
	else
		pr_err("nl_send_auto() has failed: %d\n", ret);

out_error:
	if (nlmsg)
		nlmsg_free(nlmsg);
	return ret;
}

void ipc_destroy(void)
{
	if (usmbd_health_status & USMBD_HEALTH_RUNNING) {
		ipc_usmbd_shutting_down();
		genl_unregister_family(&usmbd_family_ops);
	}

	nl_socket_free(sk);
	sk = NULL;
}

int ipc_init(void)
{
	int ret;

	sk = nl_socket_alloc();
	if (!sk) {
		pr_err("Cannot allocate netlink socket\n");
		goto out_error;
	}

	nl_socket_disable_seq_check(sk);
	if (nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM,
				nlink_msg_cb, NULL))
		goto out_error;

	if (nl_connect(sk, NETLINK_GENERIC)) {
		pr_err("Cannot connect to generic netlink.\n");
		goto out_error;
	}

	if (genl_register_family(&usmbd_family_ops)) {
		pr_err("Cannot register netlink family\n");
		goto out_error;
	}

	do {
		/*
		 * Chances are we can start before usmbd kernel module is up
		 * and running. So just wait for the kusmbd to register the
		 * netlink family and accept our connection.
		 */
		ret = genl_ops_resolve(sk, &usmbd_family_ops);
		if (ret) {
			pr_err("Cannot resolve netlink family\n");
			sleep(5);
		}
	} while (ret);

	if (ipc_usmbd_starting_up()) {
		pr_err("Unable to send startup event\n");
		return -EINVAL;
	}

	usmbd_health_status = USMBD_HEALTH_RUNNING;
	return 0;

out_error:
	ipc_destroy();
	return -EINVAL;
}
