// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */
#include <memory.h>
#include <glib.h>
#include <errno.h>
#include <linux/ksmbd_server.h>

#include <tools.h>
#include <worker.h>
#include <ipc.h>
#include <rpc.h>

#include <management/user.h>
#include <management/share.h>
#include <management/tree_conn.h>
#include <management/spnego.h>

#define MAX_WORKER_THREADS	4
static GThreadPool *pool;

#define VALID_IPC_MSG(m, t)					\
	({							\
		int ret = 1;					\
		if (((m)->sz != sizeof(t))) {			\
			pr_err("Bad message: %s\n", __func__);	\
			ret = 0;				\
		}						\
		ret;						\
	})

static int login_request(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_login_request *req;
	struct ksmbd_login_response *resp;
	struct ksmbd_ipc_msg *resp_msg;

	resp_msg = ipc_msg_alloc(sizeof(*resp));
	if (!resp_msg)
		goto out;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);
	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);

	resp->status = KSMBD_USER_FLAG_INVALID;
	if (VALID_IPC_MSG(msg, struct ksmbd_login_request))
		usm_handle_login_request(req, resp);

	resp_msg->type = KSMBD_EVENT_LOGIN_RESPONSE;
	resp->handle = req->handle;

	ipc_msg_send(resp_msg);
out:
	ipc_msg_free(resp_msg);
	return 0;
}

static int login_response_payload_sz(char *account)
{
	int payload_sz;
	struct ksmbd_user *user;

	if (account[0] == '\0')
		return 0;

	user = usm_lookup_user(account);
	if (!user)
		return 0;

	payload_sz = sizeof(gid_t) * user->ngroups;

	put_ksmbd_user(user);
	return payload_sz;
}

static int login_request_ext(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_login_request *req;
	struct ksmbd_login_response_ext *resp;
	struct ksmbd_ipc_msg *resp_msg;
	int payload_sz;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);

	payload_sz = login_response_payload_sz(req->account);
	resp_msg = ipc_msg_alloc(sizeof(*resp) + payload_sz);
	if (!resp_msg)
		goto out;

	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);

	if (VALID_IPC_MSG(msg, struct ksmbd_login_request))
		usm_handle_login_request_ext(req, resp);

	resp_msg->type = KSMBD_EVENT_LOGIN_RESPONSE_EXT;
	resp->handle = req->handle;

	ipc_msg_send(resp_msg);
out:
	ipc_msg_free(resp_msg);
	return 0;
}

static int spnego_authen_request(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_spnego_authen_request *req;
	struct ksmbd_spnego_authen_response *resp;
	struct ksmbd_ipc_msg *resp_msg = NULL;
	struct ksmbd_spnego_auth_out auth_out;
	struct ksmbd_login_request login_req;
	int retval = 0;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);
	resp_msg = ipc_msg_alloc(sizeof(*resp));
	if (!resp_msg)
		return -ENOMEM;
	resp_msg->type = KSMBD_EVENT_SPNEGO_AUTHEN_RESPONSE;
	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);
	resp->handle = req->handle;
	resp->login_response.status = KSMBD_USER_FLAG_INVALID;

	if (msg->sz <= sizeof(struct ksmbd_spnego_authen_request)) {
		retval = -EINVAL;
		goto out;
	}

	/* Authentication */
	if (spnego_handle_authen_request(req, &auth_out) != 0) {
		retval = -EPERM;
		goto out;
	}

	ipc_msg_free(resp_msg);
	resp_msg = ipc_msg_alloc(sizeof(*resp) +
			auth_out.key_len + auth_out.blob_len);
	if (!resp_msg) {
		retval = -ENOMEM;
		goto out_free_auth;
	}

	resp_msg->type = KSMBD_EVENT_SPNEGO_AUTHEN_RESPONSE;
	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);
	resp->handle = req->handle;
	resp->login_response.status = KSMBD_USER_FLAG_INVALID;

	/* login */
	login_req.handle = req->handle;
	strncpy(login_req.account, auth_out.user_name,
			sizeof(login_req.account));
	usm_handle_login_request(&login_req, &resp->login_response);
	if (!(resp->login_response.status & KSMBD_USER_FLAG_OK)) {
		pr_info("Unable to login user `%s'\n", login_req.account);
		goto out_free_auth;
	}

	resp->session_key_len = auth_out.key_len;
	memcpy(resp->payload, auth_out.sess_key, auth_out.key_len);
	resp->spnego_blob_len = auth_out.blob_len;
	memcpy(resp->payload + auth_out.key_len, auth_out.spnego_blob,
			auth_out.blob_len);
out_free_auth:
	g_free(auth_out.spnego_blob);
	g_free(auth_out.sess_key);
	g_free(auth_out.user_name);
out:
	if (resp_msg) {
		ipc_msg_send(resp_msg);
		ipc_msg_free(resp_msg);
	}
	return retval;
}

static int tree_connect_request(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_tree_connect_request *req;
	struct ksmbd_tree_connect_response *resp;
	struct ksmbd_ipc_msg *resp_msg;

	resp_msg = ipc_msg_alloc(sizeof(*resp));
	if (!resp_msg)
		goto out;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);
	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);

	resp->status = KSMBD_TREE_CONN_STATUS_ERROR;
	resp->connection_flags = 0;

	if (VALID_IPC_MSG(msg, struct ksmbd_tree_connect_request))
		tcm_handle_tree_connect(req, resp);

	resp_msg->type = KSMBD_EVENT_TREE_CONNECT_RESPONSE;
	resp->handle = req->handle;

	ipc_msg_send(resp_msg);
out:
	ipc_msg_free(resp_msg);
	return 0;
}

static int share_config_request(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_share_config_request *req;
	struct ksmbd_share_config_response *resp;
	struct ksmbd_share *share = NULL;
	struct ksmbd_ipc_msg *resp_msg;
	int payload_sz = 0;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);
	if (VALID_IPC_MSG(msg, struct ksmbd_share_config_request)) {
		share = shm_lookup_share(req->share_name);
		if (share)
			payload_sz = shm_share_config_payload_size(share);
	}

	resp_msg = ipc_msg_alloc(sizeof(*resp) + payload_sz);
	if (!resp_msg)
		goto out;

	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);
	resp->payload_sz = payload_sz;
	shm_handle_share_config_request(share, resp);
	resp_msg->type = KSMBD_EVENT_SHARE_CONFIG_RESPONSE;
	resp->handle = req->handle;

	ipc_msg_send(resp_msg);
out:
	put_ksmbd_share(share);
	ipc_msg_free(resp_msg);
	return 0;
}

static int tree_disconnect_request(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_tree_disconnect_request *req;

	if (!VALID_IPC_MSG(msg, struct ksmbd_tree_disconnect_request))
		return -EINVAL;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);
	tcm_handle_tree_disconnect(req->session_id, req->connect_id);

	return 0;
}

static int logout_request(struct ksmbd_ipc_msg *msg)
{
	if (!VALID_IPC_MSG(msg, struct ksmbd_logout_request))
		return -EINVAL;

	return usm_handle_logout_request(KSMBD_IPC_MSG_PAYLOAD(msg));
}

static int heartbeat_request(struct ksmbd_ipc_msg *msg)
{
	if (!VALID_IPC_MSG(msg, struct ksmbd_heartbeat))
		return -EINVAL;

	pr_debug("HEARTBEAT frame from the server\n");
	return 0;
}

static int rpc_request(struct ksmbd_ipc_msg *msg)
{
	struct ksmbd_rpc_command *req;
	struct ksmbd_rpc_command *resp;
	struct ksmbd_ipc_msg *resp_msg = NULL;
	int ret = -ENOTSUP;

	if (msg->sz < sizeof(struct ksmbd_rpc_command))
		goto out;

	req = KSMBD_IPC_MSG_PAYLOAD(msg);
	if (req->flags & KSMBD_RPC_METHOD_RETURN)
		resp_msg = ipc_msg_alloc(KSMBD_IPC_MAX_MESSAGE_SIZE -
				sizeof(struct ksmbd_rpc_command));
	else
		resp_msg = ipc_msg_alloc(sizeof(struct ksmbd_rpc_command));
	if (!resp_msg)
		goto out;

	resp = KSMBD_IPC_MSG_PAYLOAD(resp_msg);

	if ((req->flags & KSMBD_RPC_RAP_METHOD) == KSMBD_RPC_RAP_METHOD) {
		pr_err("RAP command is not supported yet %x\n", req->flags);
		ret = KSMBD_RPC_ENOTIMPLEMENTED;
	} else if (req->flags & KSMBD_RPC_OPEN_METHOD) {
		ret = rpc_open_request(req, resp);
	} else if (req->flags & KSMBD_RPC_CLOSE_METHOD) {
		ret = rpc_close_request(req, resp);
	} else if (req->flags & KSMBD_RPC_IOCTL_METHOD) {
		ret = rpc_ioctl_request(req, resp, resp_msg->sz);
	} else if (req->flags & KSMBD_RPC_WRITE_METHOD) {
		ret = rpc_write_request(req, resp);
	} else if (req->flags & KSMBD_RPC_READ_METHOD) {
		ret = rpc_read_request(req, resp, resp_msg->sz);
	} else {
		pr_err("Unknown RPC method: %x\n", req->flags);
		ret = KSMBD_RPC_ENOTIMPLEMENTED;
	}

	resp_msg->type = KSMBD_EVENT_RPC_RESPONSE;
	resp->handle = req->handle;
	resp->flags = ret;
	resp_msg->sz = sizeof(struct ksmbd_rpc_command) + resp->payload_sz;

	ipc_msg_send(resp_msg);
out:
	ipc_msg_free(resp_msg);
	return 0;
}

static void worker_pool_fn(gpointer event, gpointer user_data)
{
	struct ksmbd_ipc_msg *msg = (struct ksmbd_ipc_msg *)event;

	switch (msg->type) {
	case KSMBD_EVENT_LOGIN_REQUEST:
		login_request(msg);
		break;

	case KSMBD_EVENT_TREE_CONNECT_REQUEST:
		tree_connect_request(msg);
		break;

	case KSMBD_EVENT_TREE_DISCONNECT_REQUEST:
		tree_disconnect_request(msg);
		break;

	case KSMBD_EVENT_LOGOUT_REQUEST:
		logout_request(msg);
		break;

	case KSMBD_EVENT_SHARE_CONFIG_REQUEST:
		share_config_request(msg);
		break;

	case KSMBD_EVENT_RPC_REQUEST:
		rpc_request(msg);
		break;

	case KSMBD_EVENT_HEARTBEAT_REQUEST:
		heartbeat_request(msg);
		break;

	case KSMBD_EVENT_SPNEGO_AUTHEN_REQUEST:
		spnego_authen_request(msg);
		break;

	case KSMBD_EVENT_LOGIN_REQUEST_EXT:
		login_request_ext(msg);
		break;

	default:
		pr_err("Unknown IPC message type: %d\n", msg->type);
		break;
	}

	ipc_msg_free(msg);
}

int wp_ipc_msg_push(struct ksmbd_ipc_msg *msg)
{
	return g_thread_pool_push(pool, msg, NULL);
}

void wp_destroy(void)
{
	if (pool) {
		g_thread_pool_free(pool, 1, 1);
		pool = NULL;
	}
}

void wp_init(void)
{
	if (!pool)
		pool = g_thread_pool_new(
			worker_pool_fn,
			NULL,
			MAX_WORKER_THREADS,
			0,
			NULL);
}
