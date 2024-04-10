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

#include <linux/ksmbd_server.h>

#include <tools.h>
#include <ipc.h>
#include <worker.h>
#include <config_parser.h>
#include <management/user.h>
#include <management/share.h>

static struct nl_sock *sk;

struct ksmbd_ipc_msg *ipc_msg_alloc(size_t sz)
{
	struct ksmbd_ipc_msg *msg;
	size_t msg_sz = sz + sizeof(struct ksmbd_ipc_msg) + 1;

	if (msg_sz > KSMBD_IPC_MAX_MESSAGE_SIZE) {
		pr_err("IPC message is too large: %zu\n", msg_sz);
		return NULL;
	}

	msg = g_try_malloc0(msg_sz);
	if (msg)
		msg->sz = sz;
	return msg;
}

void ipc_msg_free(struct ksmbd_ipc_msg *msg)
{
	g_free(msg);
}

static int generic_event(int type, void *payload, size_t sz)
{
	struct ksmbd_ipc_msg *event;

	event = ipc_msg_alloc(sz);
	if (!event)
		return -ENOMEM;

	event->type = type;
	event->sz = sz;

	memcpy(KSMBD_IPC_MSG_PAYLOAD(event),
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

	if (gnlh->version != KSMBD_GENL_VERSION) {
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
	pr_err("Unsupported IPC event %d, ignore\n", cmd->c_id);
	return NL_SKIP;
}

static int ifc_list_size(void)
{
	char **pp = global_conf.interfaces;
	int sz = 0;

	for (; *pp; pp++) {
		char *p = *pp;

		if (*p == 0x00)
			continue;
		sz += strlen(p) + 1;
	}
	return sz;
}

static int ipc_ksmbd_starting_up(void)
{
	struct ksmbd_startup_request *ev;
	struct ksmbd_ipc_msg *msg;
	int ifc_list_sz = 0;
	int ret;

	if (global_conf.bind_interfaces_only && global_conf.interfaces)
		ifc_list_sz += ifc_list_size();

	msg = ipc_msg_alloc(sizeof(*ev) + ifc_list_sz);
	if (!msg)
		return -ENOMEM;

	ev = KSMBD_IPC_MSG_PAYLOAD(msg);
	msg->type = KSMBD_EVENT_STARTING_UP;

	ev->flags = global_conf.flags;
	ev->signing = global_conf.server_signing;
	ev->tcp_port = global_conf.tcp_port;
	ev->ipc_timeout = global_conf.ipc_timeout;
	ev->deadtime = global_conf.deadtime;
	ev->file_max = global_conf.file_max;
	ev->smb2_max_read = global_conf.smb2_max_read;
	ev->smb2_max_write = global_conf.smb2_max_write;
	ev->smb2_max_trans = global_conf.smb2_max_trans;
	ev->smbd_max_io_size = global_conf.smbd_max_io_size;
	ev->max_connections = global_conf.max_connections;
	ev->share_fake_fscaps = global_conf.share_fake_fscaps;
	memcpy(ev->sub_auth, global_conf.gen_subauth, sizeof(ev->sub_auth));
	ev->smb2_max_credits = global_conf.smb2_max_credits;

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
		char *config_payload = KSMBD_STARTUP_CONFIG_INTERFACES(ev);
		char **pp = global_conf.interfaces;
		int sz = 0;

		ev->ifc_list_sz = ifc_list_sz;

		for (; *pp; pp++) {
			char *p = *pp;

			if (*p == 0x00)
				continue;
			strcpy(config_payload + sz, p);
			sz += strlen(p) + 1;
		}

		global_conf.bind_interfaces_only = 0;
		cp_group_kv_list_free(global_conf.interfaces);
	}

	ret = ipc_msg_send(msg);
	ipc_msg_free(msg);
	return ret;
}

int ipc_process_event(void)
{
	fd_set rfds;
	int sk_fd, ret;

	FD_ZERO(&rfds);

	sk_fd = nl_socket_get_fd(sk);
	FD_SET(sk_fd, &rfds);

	if (select(sk_fd + 1, &rfds, NULL, NULL, NULL) < 0) {
		if (errno != EINTR) {
			ret = -errno;
			pr_err("Can't wait on netlink socket: %m\n");
			return ret;
		}
		ret = 0;
		return ret;
	}

	ret = nl_recvmsgs_default(sk);
	if (ret < 0)
		pr_err("nl_recv() failed, check dmesg, error: %s\n",
		       nl_geterror(ret));
	return ret;
}

static struct nla_policy ksmbd_nl_policy[KSMBD_EVENT_MAX] = {
	[KSMBD_EVENT_UNSPEC] = {
		.minlen = 0,
	},

	[KSMBD_EVENT_HEARTBEAT_REQUEST] = {
		.minlen = sizeof(struct ksmbd_heartbeat),
	},

	[KSMBD_EVENT_STARTING_UP] = {
		.minlen = sizeof(struct ksmbd_startup_request),
	},

	[KSMBD_EVENT_SHUTTING_DOWN] = {
		.minlen = sizeof(struct ksmbd_shutdown_request),
	},

	[KSMBD_EVENT_LOGIN_REQUEST] = {
		.minlen = sizeof(struct ksmbd_login_request),
	},

	[KSMBD_EVENT_LOGIN_RESPONSE] = {
		.minlen = sizeof(struct ksmbd_login_response),
	},

	[KSMBD_EVENT_SHARE_CONFIG_REQUEST] = {
		.minlen = sizeof(struct ksmbd_share_config_request),
	},

	[KSMBD_EVENT_SHARE_CONFIG_RESPONSE] = {
		.minlen = sizeof(struct ksmbd_share_config_response),
	},

	[KSMBD_EVENT_TREE_CONNECT_REQUEST] = {
		.minlen = sizeof(struct ksmbd_tree_connect_request),
	},

	[KSMBD_EVENT_TREE_CONNECT_RESPONSE] = {
		.minlen = sizeof(struct ksmbd_tree_connect_response),
	},

	[KSMBD_EVENT_TREE_DISCONNECT_REQUEST] = {
		.minlen = sizeof(struct ksmbd_tree_disconnect_request),
	},

	[KSMBD_EVENT_LOGOUT_REQUEST] = {
		.minlen = sizeof(struct ksmbd_logout_request),
	},

	[KSMBD_EVENT_RPC_REQUEST] = {
		.minlen = sizeof(struct ksmbd_rpc_command),
	},

	[KSMBD_EVENT_RPC_RESPONSE] = {
		.minlen = sizeof(struct ksmbd_rpc_command),
	},

	[KSMBD_EVENT_SPNEGO_AUTHEN_REQUEST] = {
		.minlen = sizeof(struct ksmbd_spnego_authen_request),
	},

	[KSMBD_EVENT_SPNEGO_AUTHEN_RESPONSE] = {
		.minlen = sizeof(struct ksmbd_spnego_authen_response),
	},
};

static struct genl_cmd ksmbd_genl_cmds[] = {
	{
		.c_id		= KSMBD_EVENT_UNSPEC,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_HEARTBEAT_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_STARTING_UP,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_SHUTTING_DOWN,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_LOGIN_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_LOGIN_RESPONSE,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_SHARE_CONFIG_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_SHARE_CONFIG_RESPONSE,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_TREE_CONNECT_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_TREE_CONNECT_RESPONSE,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_TREE_DISCONNECT_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_LOGOUT_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_RPC_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_RPC_RESPONSE,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_SPNEGO_AUTHEN_REQUEST,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_generic_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
	{
		.c_id		= KSMBD_EVENT_SPNEGO_AUTHEN_RESPONSE,
		.c_attr_policy	= ksmbd_nl_policy,
		.c_msg_parser	= &handle_unsupported_event,
		.c_maxattr	= KSMBD_EVENT_MAX,
	},
};

static struct genl_ops ksmbd_family_ops = {
	.o_name = KSMBD_GENL_NAME,
	.o_cmds = ksmbd_genl_cmds,
	.o_ncmds = ARRAY_SIZE(ksmbd_genl_cmds),
};

int ipc_msg_send(struct ksmbd_ipc_msg *msg)
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
	hdr = genlmsg_put(nlmsg, getpid(), 0, ksmbd_family_ops.o_id,
			  0, 0, msg->type, KSMBD_GENL_VERSION);
	if (!hdr) {
		pr_err("genlmsg_put() has failed, aborting IPC send()\n");
		goto out_error;
	}

	/* Use msg->type as attribute TYPE */
	ret = nla_put(nlmsg, msg->type, msg->sz, KSMBD_IPC_MSG_PAYLOAD(msg));
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
	switch (genl_register_family(&ksmbd_family_ops)) {
	case -NLE_EXIST:
	case 0:
		genl_unregister_family(&ksmbd_family_ops);
	}

	nl_socket_free(sk);
	sk = NULL;
}

void ipc_init(void)
{
	int ret;

	if (sk)
		return;

	sk = nl_socket_alloc();
	if (!sk) {
		pr_err("Can't allocate netlink socket\n");
		abort();
	}

	nl_socket_disable_seq_check(sk);
	nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, nlink_msg_cb, NULL);

	if (nl_connect(sk, NETLINK_GENERIC)) {
		pr_err("Can't connect to generic netlink\n");
		abort();
	}

	ret = nl_socket_set_buffer_size(sk, KSMBD_IPC_SO_RCVBUF_SIZE, 0);
	if (ret) {
		pr_err("Can't set netlink socket buffer size: %s\n",
		       nl_geterror(ret));
		abort();
	}

	if (genl_register_family(&ksmbd_family_ops)) {
		pr_err("Can't register netlink family\n");
		abort();
	}

	ret = genl_ops_resolve(sk, &ksmbd_family_ops);
	if (ret) {
		pr_err("Can't resolve netlink family\n");
		abort();
	}

	if (ipc_ksmbd_starting_up()) {
		pr_err("Can't send startup event\n");
		abort();
	}

	ksmbd_health_status = KSMBD_HEALTH_RUNNING;
}
