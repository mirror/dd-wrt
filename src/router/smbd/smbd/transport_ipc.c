// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/jhash.h>
#include <linux/slab.h>
#include <linux/rwsem.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/hashtable.h>
#include <net/net_namespace.h>
#include <net/genetlink.h>
#include <linux/socket.h>
#include <linux/workqueue.h>

#include "transport_ipc.h"
#include "buffer_pool.h"
#include "server.h"
#include "smb_common.h"
#include "vfs_cache.h"

#include "mgmt/user_config.h"
#include "mgmt/share_config.h"
#include "mgmt/user_session.h"
#include "mgmt/tree_connect.h"
#include "mgmt/smbd_ida.h"
#include "connection.h"
#include "transport_tcp.h"

/* @FIXME fix this code */
extern int get_protocol_idx(char *str);

#define IPC_WAIT_TIMEOUT	(2 * HZ)

#define IPC_MSG_HASH_BITS	3
static DEFINE_HASHTABLE(ipc_msg_table, IPC_MSG_HASH_BITS);
static DECLARE_RWSEM(ipc_msg_table_lock);
static DEFINE_MUTEX(startup_lock);

static struct smbd_ida *ida;

static unsigned int smbd_tools_pid;

#define SMBD_IPC_MSG_HANDLE(m)	(*(unsigned int *)m)

static bool smbd_ipc_validate_version(struct genl_info *m)
{
	if (m->genlhdr->version != SMBD_GENL_VERSION) {
		smbd_err("%s. smbd: %d, kernel module: %d. %s.\n",
			  "Daemon and kernel module version mismatch",
			  m->genlhdr->version,
			  SMBD_GENL_VERSION,
			  "User-space smbd should terminate");
		return false;
	}
	return true;
}

struct smbd_ipc_msg {
	unsigned int		type;
	unsigned int		sz;
	unsigned char		____payload[0];
};

#define SMBD_IPC_MSG_PAYLOAD(m)					\
	(void *)(((struct smbd_ipc_msg *)(m))->____payload)

struct ipc_msg_table_entry {
	unsigned int		handle;
	unsigned int		type;
	wait_queue_head_t	wait;
	struct hlist_node	ipc_table_hlist;

	void			*response;
};

static struct delayed_work ipc_timer_work;

static int handle_startup_event(struct sk_buff *skb, struct genl_info *info);
static int handle_unsupported_event(struct sk_buff *skb,
				    struct genl_info *info);
static int handle_generic_event(struct sk_buff *skb, struct genl_info *info);
static int smbd_ipc_heartbeat_request(void);

static const struct nla_policy smbd_nl_policy[SMBD_EVENT_MAX] = {
	[SMBD_EVENT_UNSPEC] = {
		.len = 0,
	},
	[SMBD_EVENT_HEARTBEAT_REQUEST] = {
		.len = sizeof(struct smbd_heartbeat),
	},
	[SMBD_EVENT_STARTING_UP] = {
		.len = sizeof(struct smbd_startup_request),
	},
	[SMBD_EVENT_SHUTTING_DOWN] = {
		.len = sizeof(struct smbd_shutdown_request),
	},
	[SMBD_EVENT_LOGIN_REQUEST] = {
		.len = sizeof(struct smbd_login_request),
	},
	[SMBD_EVENT_LOGIN_RESPONSE] = {
		.len = sizeof(struct smbd_login_response),
	},
	[SMBD_EVENT_SHARE_CONFIG_REQUEST] = {
		.len = sizeof(struct smbd_share_config_request),
	},
	[SMBD_EVENT_SHARE_CONFIG_RESPONSE] = {
		.len = sizeof(struct smbd_share_config_response),
	},
	[SMBD_EVENT_TREE_CONNECT_REQUEST] = {
		.len = sizeof(struct smbd_tree_connect_request),
	},
	[SMBD_EVENT_TREE_CONNECT_RESPONSE] = {
		.len = sizeof(struct smbd_tree_connect_response),
	},
	[SMBD_EVENT_TREE_DISCONNECT_REQUEST] = {
		.len = sizeof(struct smbd_tree_disconnect_request),
	},
	[SMBD_EVENT_LOGOUT_REQUEST] = {
		.len = sizeof(struct smbd_logout_request),
	},
	[SMBD_EVENT_RPC_REQUEST] = {
	},
	[SMBD_EVENT_RPC_RESPONSE] = {
	},
};

static struct genl_ops smbd_genl_ops[] = {
	{
		.cmd	= SMBD_EVENT_UNSPEC,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_HEARTBEAT_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_STARTING_UP,
		.doit	= handle_startup_event,
	},
	{
		.cmd	= SMBD_EVENT_SHUTTING_DOWN,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_LOGIN_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_LOGIN_RESPONSE,
		.doit	= handle_generic_event,
	},
	{
		.cmd	= SMBD_EVENT_SHARE_CONFIG_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_SHARE_CONFIG_RESPONSE,
		.doit	= handle_generic_event,
	},
	{
		.cmd	= SMBD_EVENT_TREE_CONNECT_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_TREE_CONNECT_RESPONSE,
		.doit	= handle_generic_event,
	},
	{
		.cmd	= SMBD_EVENT_TREE_DISCONNECT_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_LOGOUT_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_RPC_REQUEST,
		.doit	= handle_unsupported_event,
	},
	{
		.cmd	= SMBD_EVENT_RPC_RESPONSE,
		.doit	= handle_generic_event,
	},
};

static struct genl_family smbd_genl_family = {
	.name		= SMBD_GENL_NAME,
	.version	= SMBD_GENL_VERSION,
	.hdrsize	= 0,
	.maxattr	= SMBD_EVENT_MAX,
	.netnsok	= true,
	.module		= THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	.ops		= smbd_genl_ops,
	.n_ops		= ARRAY_SIZE(smbd_genl_ops),
#endif
};

static void smbd_nl_init_fixup(void)
{
	int i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0)
	for (i = 0; i < ARRAY_SIZE(smbd_genl_ops); i++)
		smbd_genl_ops[i].validate = GENL_DONT_VALIDATE_STRICT |
						GENL_DONT_VALIDATE_DUMP;

	smbd_genl_family.policy = smbd_nl_policy;
#else
	for (i = 0; i < ARRAY_SIZE(smbd_genl_ops); i++)
		smbd_genl_ops[i].policy = smbd_nl_policy;
#endif
}

static int rpc_context_flags(struct smbd_session *sess)
{
	if (user_guest(sess->user))
		return SMBD_RPC_RESTRICTED_CONTEXT;
	return 0;
}

static void ipc_update_last_active(void)
{
	if (server_conf.ipc_timeout)
		server_conf.ipc_last_active = jiffies;
}

static struct smbd_ipc_msg *ipc_msg_alloc(size_t sz)
{
	struct smbd_ipc_msg *msg;
	size_t msg_sz = sz + sizeof(struct smbd_ipc_msg);

	msg = smbd_alloc(msg_sz);
	if (msg)
		msg->sz = sz;
	return msg;
}

static void ipc_msg_free(struct smbd_ipc_msg *msg)
{
	smbd_free(msg);
}

static void ipc_msg_handle_free(int handle)
{
	if (handle >= 0)
		smbd_release_id(ida, handle);
}

static int handle_response(int type, void *payload, size_t sz)
{
	int handle = SMBD_IPC_MSG_HANDLE(payload);
	struct ipc_msg_table_entry *entry;
	int ret = 0;

	ipc_update_last_active();
	down_read(&ipc_msg_table_lock);
	hash_for_each_possible(ipc_msg_table, entry, ipc_table_hlist, handle) {
		if (handle != entry->handle)
			continue;

		entry->response = NULL;
		/*
		 * Response message type value should be equal to
		 * request message type + 1.
		 */
		if (entry->type + 1 != type) {
			smbd_err("Waiting for IPC type %d, got %d. Ignore.\n",
				entry->type + 1, type);
		}

		entry->response = smbd_alloc(sz);
		if (!entry->response) {
			ret = -ENOMEM;
			break;
		}

		memcpy(entry->response, payload, sz);
		wake_up_interruptible(&entry->wait);
		ret = 0;
		break;
	}
	up_read(&ipc_msg_table_lock);

	return ret;
}

static int ipc_server_config_on_startup(struct smbd_startup_request *req)
{
	int ret;

	smbd_set_fd_limit(req->file_max);
	server_conf.flags = req->flags;
	server_conf.signing = req->signing;
	server_conf.tcp_port = req->tcp_port;
	server_conf.ipc_timeout = req->ipc_timeout * HZ;
	server_conf.deadtime = req->deadtime * SMB_ECHO_INTERVAL;

#ifdef CONFIG_SMB_INSECURE_SERVER
	server_conf.flags &= ~SMBD_GLOBAL_FLAG_CACHE_TBUF;
#endif

	if (req->smb2_max_read)
		init_smb2_max_read_size(req->smb2_max_read);
	if (req->smb2_max_write)
		init_smb2_max_write_size(req->smb2_max_write);
	if (req->smb2_max_trans)
		init_smb2_max_trans_size(req->smb2_max_trans);

	ret = smbd_set_netbios_name(req->netbios_name);
	ret |= smbd_set_server_string(req->server_string);
	ret |= smbd_set_work_group(req->work_group);
	ret |= smbd_tcp_set_interfaces(SMBD_STARTUP_CONFIG_INTERFACES(req),
					req->ifc_list_sz);
	if (ret) {
		smbd_err("Server configuration error: %s %s %s\n",
				req->netbios_name,
				req->server_string,
				req->work_group);
		return ret;
	}

	if (req->min_prot[0]) {
		ret = smbd_lookup_protocol_idx(req->min_prot);
		if (ret >= 0)
			server_conf.min_protocol = ret;
	}
	if (req->max_prot[0]) {
		ret = smbd_lookup_protocol_idx(req->max_prot);
		if (ret >= 0)
			server_conf.max_protocol = ret;
	}

	if (server_conf.ipc_timeout)
		schedule_delayed_work(&ipc_timer_work, server_conf.ipc_timeout);
	return 0;
}

static int handle_startup_event(struct sk_buff *skb, struct genl_info *info)
{
	int ret = 0;

	if (!smbd_ipc_validate_version(info))
		return -EINVAL;

	if (!info->attrs[SMBD_EVENT_STARTING_UP])
		return -EINVAL;

	mutex_lock(&startup_lock);
	if (!smbd_server_configurable()) {
		mutex_unlock(&startup_lock);
		smbd_err("Server reset is in progress, can't start daemon\n");
		return -EINVAL;
	}

	if (smbd_tools_pid) {
		if (smbd_ipc_heartbeat_request() == 0) {
			ret = -EINVAL;
			goto out;
		}

		smbd_err("Reconnect to a new user space daemon\n");
	} else {
		struct smbd_startup_request *req;

		req = nla_data(info->attrs[info->genlhdr->cmd]);
		ret = ipc_server_config_on_startup(req);
		if (ret)
			goto out;
		server_queue_ctrl_init_work();
	}

	smbd_tools_pid = info->snd_portid;
	ipc_update_last_active();

out:
	mutex_unlock(&startup_lock);
	return ret;
}

static int handle_unsupported_event(struct sk_buff *skb,
				    struct genl_info *info)
{
	smbd_err("Unknown IPC event: %d, ignore.\n", info->genlhdr->cmd);
	return -EINVAL;
}

static int handle_generic_event(struct sk_buff *skb, struct genl_info *info)
{
	void *payload;
	int sz;
	int type = info->genlhdr->cmd;

	if (type >= SMBD_EVENT_MAX) {
		WARN_ON(1);
		return -EINVAL;
	}

	if (!smbd_ipc_validate_version(info))
		return -EINVAL;

	if (!info->attrs[type])
		return -EINVAL;

	payload = nla_data(info->attrs[info->genlhdr->cmd]);
	sz = nla_len(info->attrs[info->genlhdr->cmd]);
	return handle_response(type, payload, sz);
}

static int ipc_msg_send(struct smbd_ipc_msg *msg)
{
	struct genlmsghdr *nlh;
	struct sk_buff *skb;
	int ret = -EINVAL;

	if (!smbd_tools_pid)
		return ret;

	skb = genlmsg_new(msg->sz, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	nlh = genlmsg_put(skb, 0, 0, &smbd_genl_family, 0, msg->type);
	if (!nlh)
		goto out;

	ret = nla_put(skb, msg->type, msg->sz, SMBD_IPC_MSG_PAYLOAD(msg));
	if (ret) {
		genlmsg_cancel(skb, nlh);
		goto out;
	}

	genlmsg_end(skb, nlh);
	ret = genlmsg_unicast(&init_net, skb, smbd_tools_pid);
	if (!ret)
		ipc_update_last_active();
	return ret;

out:
	nlmsg_free(skb);
	return ret;
}

static void *ipc_msg_send_request(struct smbd_ipc_msg *msg,
				  unsigned int handle)
{
	struct ipc_msg_table_entry entry;
	int ret;

	if ((int)handle < 0)
		return NULL;

	entry.type = msg->type;
	entry.response = NULL;
	init_waitqueue_head(&entry.wait);

	down_write(&ipc_msg_table_lock);
	entry.handle = handle;
	hash_add(ipc_msg_table, &entry.ipc_table_hlist, entry.handle);
	up_write(&ipc_msg_table_lock);

	ret = ipc_msg_send(msg);
	if (ret)
		goto out;

	ret = wait_event_interruptible_timeout(entry.wait,
					       entry.response != NULL,
					       IPC_WAIT_TIMEOUT);
out:
	down_write(&ipc_msg_table_lock);
	hash_del(&entry.ipc_table_hlist);
	up_write(&ipc_msg_table_lock);
	return entry.response;
}

static int smbd_ipc_heartbeat_request(void)
{
	struct smbd_ipc_msg *msg;
	int ret;

	msg = ipc_msg_alloc(sizeof(struct smbd_heartbeat));
	if (!msg)
		return -EINVAL;

	msg->type = SMBD_EVENT_HEARTBEAT_REQUEST;
	ret = ipc_msg_send(msg);
	ipc_msg_free(msg);
	return ret;
}

struct smbd_login_response *smbd_ipc_login_request(const char *account)
{
	struct smbd_ipc_msg *msg;
	struct smbd_login_request *req;
	struct smbd_login_response *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_login_request));
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_LOGIN_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = smbd_acquire_id(ida);
	memcpy(req->account, account, sizeof(req->account) - 1);

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_handle_free(req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_tree_connect_response *
smbd_ipc_tree_connect_request(struct smbd_session *sess,
			       struct smbd_share_config *share,
			       struct smbd_tree_connect *tree_conn,
			       struct sockaddr *peer_addr)
{
	struct smbd_ipc_msg *msg;
	struct smbd_tree_connect_request *req;
	struct smbd_tree_connect_response *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_tree_connect_request));
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_TREE_CONNECT_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);

	req->handle = smbd_acquire_id(ida);
	req->account_flags = sess->user->flags;
	req->session_id = sess->id;
	req->connect_id = tree_conn->id;
	memcpy(req->account, user_name(sess->user), sizeof(req->account) - 1);
	memcpy(req->share, share->name, sizeof(req->share) - 1);
	snprintf(req->peer_addr, sizeof(req->peer_addr), "%pIS", peer_addr);

	if (peer_addr->sa_family == AF_INET6)
		req->flags |= SMBD_TREE_CONN_FLAG_REQUEST_IPV6;
	if (test_session_flag(sess, CIFDS_SESSION_FLAG_SMB2))
		req->flags |= SMBD_TREE_CONN_FLAG_REQUEST_SMB2;

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_handle_free(req->handle);
	ipc_msg_free(msg);
	return resp;
}

int smbd_ipc_tree_disconnect_request(unsigned long long session_id,
				      unsigned long long connect_id)
{
	struct smbd_ipc_msg *msg;
	struct smbd_tree_disconnect_request *req;
	int ret;

	msg = ipc_msg_alloc(sizeof(struct smbd_tree_disconnect_request));
	if (!msg)
		return -ENOMEM;

	msg->type = SMBD_EVENT_TREE_DISCONNECT_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->session_id = session_id;
	req->connect_id = connect_id;

	ret = ipc_msg_send(msg);
	ipc_msg_free(msg);
	return ret;
}

int smbd_ipc_logout_request(const char *account)
{
	struct smbd_ipc_msg *msg;
	struct smbd_logout_request *req;
	int ret;

	msg = ipc_msg_alloc(sizeof(struct smbd_logout_request));
	if (!msg)
		return -ENOMEM;

	msg->type = SMBD_EVENT_LOGOUT_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	memcpy(req->account, account, SMBD_REQ_MAX_ACCOUNT_NAME_SZ - 1);

	ret = ipc_msg_send(msg);
	ipc_msg_free(msg);
	return ret;
}

struct smbd_share_config_response *
smbd_ipc_share_config_request(const char *name)
{
	struct smbd_ipc_msg *msg;
	struct smbd_share_config_request *req;
	struct smbd_share_config_response *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_share_config_request));
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_SHARE_CONFIG_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = smbd_acquire_id(ida);
	memcpy(req->share_name, name, sizeof(req->share_name) - 1);

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_handle_free(req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_rpc_command *smbd_rpc_open(struct smbd_session *sess,
					 int handle)
{
	struct smbd_ipc_msg *msg;
	struct smbd_rpc_command *req;
	struct smbd_rpc_command *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_rpc_command));
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_RPC_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = handle;
	req->flags = smbd_session_rpc_method(sess, handle);
	req->flags |= SMBD_RPC_OPEN_METHOD;
	req->payload_sz = 0;

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_rpc_command *smbd_rpc_close(struct smbd_session *sess,
					  int handle)
{
	struct smbd_ipc_msg *msg;
	struct smbd_rpc_command *req;
	struct smbd_rpc_command *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_rpc_command));
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_RPC_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = handle;
	req->flags = smbd_session_rpc_method(sess, handle);
	req->flags |= SMBD_RPC_CLOSE_METHOD;
	req->payload_sz = 0;

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_rpc_command *smbd_rpc_write(struct smbd_session *sess,
					  int handle,
					  void *payload,
					  size_t payload_sz)
{
	struct smbd_ipc_msg *msg;
	struct smbd_rpc_command *req;
	struct smbd_rpc_command *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_rpc_command) + payload_sz + 1);
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_RPC_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = handle;
	req->flags = smbd_session_rpc_method(sess, handle);
	req->flags |= rpc_context_flags(sess);
	req->flags |= SMBD_RPC_WRITE_METHOD;
	req->payload_sz = payload_sz;
	memcpy(req->payload, payload, payload_sz);

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_rpc_command *smbd_rpc_read(struct smbd_session *sess,
					 int handle)
{
	struct smbd_ipc_msg *msg;
	struct smbd_rpc_command *req;
	struct smbd_rpc_command *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_rpc_command));
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_RPC_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = handle;
	req->flags = smbd_session_rpc_method(sess, handle);
	req->flags |= rpc_context_flags(sess);
	req->flags |= SMBD_RPC_READ_METHOD;
	req->payload_sz = 0;

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_rpc_command *smbd_rpc_ioctl(struct smbd_session *sess,
					  int handle,
					  void *payload,
					  size_t payload_sz)
{
	struct smbd_ipc_msg *msg;
	struct smbd_rpc_command *req;
	struct smbd_rpc_command *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_rpc_command) + payload_sz + 1);
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_RPC_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = handle;
	req->flags = smbd_session_rpc_method(sess, handle);
	req->flags |= rpc_context_flags(sess);
	req->flags |= SMBD_RPC_IOCTL_METHOD;
	req->payload_sz = payload_sz;
	memcpy(req->payload, payload, payload_sz);

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_free(msg);
	return resp;
}

struct smbd_rpc_command *smbd_rpc_rap(struct smbd_session *sess,
					void *payload,
					size_t payload_sz)
{
	struct smbd_ipc_msg *msg;
	struct smbd_rpc_command *req;
	struct smbd_rpc_command *resp;

	msg = ipc_msg_alloc(sizeof(struct smbd_rpc_command) + payload_sz + 1);
	if (!msg)
		return NULL;

	msg->type = SMBD_EVENT_RPC_REQUEST;
	req = SMBD_IPC_MSG_PAYLOAD(msg);
	req->handle = smbd_acquire_id(ida);
	req->flags = rpc_context_flags(sess);
	req->flags |= SMBD_RPC_RAP_METHOD;
	req->payload_sz = payload_sz;
	memcpy(req->payload, payload, payload_sz);

	resp = ipc_msg_send_request(msg, req->handle);
	ipc_msg_handle_free(req->handle);
	ipc_msg_free(msg);
	return resp;
}

static int __ipc_heartbeat(void)
{
	unsigned long delta;

	if (!smbd_server_running())
		return 0;

	if (time_after(jiffies, server_conf.ipc_last_active)) {
		delta = (jiffies - server_conf.ipc_last_active);
	} else {
		ipc_update_last_active();
		schedule_delayed_work(&ipc_timer_work,
				      server_conf.ipc_timeout);
		return 0;
	}

	if (delta < server_conf.ipc_timeout) {
		schedule_delayed_work(&ipc_timer_work,
				      server_conf.ipc_timeout - delta);
		return 0;
	}

	if (smbd_ipc_heartbeat_request() == 0) {
		schedule_delayed_work(&ipc_timer_work,
				      server_conf.ipc_timeout);
		return 0;
	}

	mutex_lock(&startup_lock);
	WRITE_ONCE(server_conf.state, SERVER_STATE_RESETTING);
	server_conf.ipc_last_active = 0;
	smbd_tools_pid = 0;
	smbd_err("No IPC daemon response for %lus\n", delta / HZ);
	mutex_unlock(&startup_lock);
	return -EINVAL;
}

static void ipc_timer_heartbeat(struct work_struct *w)
{
	if (__ipc_heartbeat())
		server_queue_ctrl_reset_work();
}

int smbd_ipc_id_alloc(void)
{
	return smbd_acquire_id(ida);
}

void smbd_rpc_id_free(int handle)
{
	smbd_release_id(ida, handle);
}

void smbd_ipc_release(void)
{
	cancel_delayed_work_sync(&ipc_timer_work);
	smbd_ida_free(ida);
	genl_unregister_family(&smbd_genl_family);
}

void smbd_ipc_soft_reset(void)
{
	mutex_lock(&startup_lock);
	smbd_tools_pid = 0;
	cancel_delayed_work_sync(&ipc_timer_work);
	mutex_unlock(&startup_lock);
}

int smbd_ipc_init(void)
{
	int ret;
	int i;

	smbd_nl_init_fixup();
	INIT_DELAYED_WORK(&ipc_timer_work, ipc_timer_heartbeat);

	ret = genl_register_family(&smbd_genl_family);
	if (ret) {
		smbd_err("Failed to register SMBD netlink interface %d\n",
				ret);
		return ret;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
	for (i = 0; i < ARRAY_SIZE(smbd_genl_ops); i++) {
		genl_register_ops(&smbd_genl_family,
				&smbd_genl_ops[i]);
	}
#endif
	ida = smbd_ida_alloc();
	if (!ida)
		return -ENOMEM;
	return 0;
}
