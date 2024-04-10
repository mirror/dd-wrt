// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "linux/ksmbd_server.h"
#include "management/tree_conn.h"
#include "management/session.h"
#include "management/share.h"
#include "management/user.h"
#include "tools.h"

static struct ksmbd_tree_conn *new_ksmbd_tree_conn(void)
{
	struct ksmbd_tree_conn *conn;

	conn = g_try_malloc0(sizeof(struct ksmbd_tree_conn));
	if (!conn)
		return NULL;

	conn->id = 0;
	return conn;
}

void tcm_tree_conn_free(struct ksmbd_tree_conn *conn)
{
	shm_close_connection(conn->share);
	put_ksmbd_share(conn->share);
	g_free(conn);
}

int tcm_handle_tree_connect(struct ksmbd_tree_connect_request *req,
			    struct ksmbd_tree_connect_response *resp)
{
	struct ksmbd_user *user = NULL;
	struct ksmbd_share *share = NULL;
	struct ksmbd_tree_conn *conn = new_ksmbd_tree_conn();
	int ret;

	if (!conn) {
		resp->status = KSMBD_TREE_CONN_STATUS_NOMEM;
		return -ENOMEM;
	}

	if (sm_check_sessions_capacity(req->session_id)) {
		resp->status = KSMBD_TREE_CONN_STATUS_TOO_MANY_SESSIONS;
		pr_debug("treecon: Too many active sessions\n");
		goto out_error;
	}

	if (global_conf.map_to_guest == KSMBD_CONF_MAP_TO_GUEST_NEVER) {
		if (req->account_flags & KSMBD_USER_FLAG_BAD_PASSWORD) {
			resp->status = KSMBD_TREE_CONN_STATUS_INVALID_USER;
			pr_debug("treecon: Bad user password\n");
			goto out_error;
		}
	}

	share = shm_lookup_share(req->share);
	if (!share) {
		resp->status = KSMBD_TREE_CONN_STATUS_NO_SHARE;
		pr_err("treecon: Unknown net share: %s\n", req->share);
		goto out_error;
	}

	if (test_share_flag(share, KSMBD_SHARE_FLAG_WRITEABLE))
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_WRITABLE);
	if (test_share_flag(share, KSMBD_SHARE_FLAG_READONLY))
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_READ_ONLY);
	if (test_share_flag(share, KSMBD_SHARE_FLAG_UPDATE))
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_UPDATE);

	if (shm_open_connection(share)) {
		resp->status = KSMBD_TREE_CONN_STATUS_TOO_MANY_CONNS;
		pr_debug("treecon: Too many connections to net share\n");
		goto out_error;
	}

	ret = shm_lookup_hosts_map(share,
				   KSMBD_SHARE_HOSTS_ALLOW_MAP,
				   req->peer_addr);
	if (ret == -ENOENT) {
		resp->status = KSMBD_TREE_CONN_STATUS_HOST_DENIED;
		pr_debug("treecon: Host denied: %s\n", req->peer_addr);
		goto out_error;
	}

	if (ret != 0) {
		ret = shm_lookup_hosts_map(share,
					   KSMBD_SHARE_HOSTS_DENY_MAP,
					   req->peer_addr);
		if (ret == 0) {
			resp->status = KSMBD_TREE_CONN_STATUS_HOST_DENIED;
			pr_err("treecon: Host denied: %s\n", req->peer_addr);
			goto out_error;
		}
	}

	if (global_conf.restrict_anon >= KSMBD_RESTRICT_ANON_TYPE_1) {
		int deny;

		deny = !test_share_flag(share, KSMBD_SHARE_FLAG_GUEST_OK);
		deny |= test_share_flag(share, KSMBD_SHARE_FLAG_PIPE);

		if (req->account_flags & KSMBD_USER_FLAG_GUEST_ACCOUNT &&
				deny) {
			pr_debug("treecon: Deny, restricted session\n");
			resp->status = KSMBD_TREE_CONN_STATUS_ERROR;
			goto out_error;
		}
	}

	if ((req->account_flags & KSMBD_USER_FLAG_GUEST_ACCOUNT) &&
	    !test_share_flag(share, KSMBD_SHARE_FLAG_PIPE) &&
	    !test_share_flag(share, KSMBD_SHARE_FLAG_GUEST_OK)) {
		pr_debug("treecon: Deny, guest not allowed\n");
		resp->status = KSMBD_TREE_CONN_STATUS_ERROR;
		goto out_error;
	}

	if ((req->account_flags & KSMBD_USER_FLAG_GUEST_ACCOUNT) &&
	     test_share_flag(share, KSMBD_SHARE_FLAG_GUEST_OK)) {
		pr_debug("treecon: Net share permits guest login\n");
		user = usm_lookup_user(share->guest_account);
		if (user) {
			set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_GUEST_ACCOUNT);
			goto bind;
		}

		user = usm_lookup_user(global_conf.guest_account);
		if (user) {
			set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_GUEST_ACCOUNT);
			goto bind;
		}
	}

	user = usm_lookup_user(req->account);
	if (!user) {
		resp->status = KSMBD_TREE_CONN_STATUS_NO_USER;
		pr_err("treecon: User `%s' not found\n", req->account);
		goto out_error;
	}

	user->failed_login_count = 0;
	user->flags &= ~KSMBD_USER_FLAG_DELAY_SESSION;

	if (test_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT))
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_GUEST_ACCOUNT);

	ret = shm_lookup_users_map(share,
				   KSMBD_SHARE_ADMIN_USERS_MAP,
				   req->account);
	if (ret == 0) {
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_ADMIN_ACCOUNT);
		goto bind;
	}

	ret = shm_lookup_users_map(share,
				   KSMBD_SHARE_INVALID_USERS_MAP,
				   req->account);
	if (ret == 0) {
		resp->status = KSMBD_TREE_CONN_STATUS_INVALID_USER;
		pr_err("treecon: User is on invalid users list\n");
		goto out_error;
	}

	ret = shm_lookup_users_map(share,
				   KSMBD_SHARE_READ_LIST_MAP,
				   req->account);
	if (ret == 0) {
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_READ_ONLY);
		clear_conn_flag(conn, KSMBD_TREE_CONN_FLAG_WRITABLE);
		goto bind;
	}

	ret = shm_lookup_users_map(share,
				   KSMBD_SHARE_WRITE_LIST_MAP,
				   req->account);
	if (ret == 0) {
		set_conn_flag(conn, KSMBD_TREE_CONN_FLAG_WRITABLE);
		goto bind;
	}

	ret = shm_lookup_users_map(share,
				   KSMBD_SHARE_VALID_USERS_MAP,
				   req->account);
	if (ret == 0)
		goto bind;
	if (ret == -ENOENT) {
		resp->status = KSMBD_TREE_CONN_STATUS_INVALID_USER;
		pr_err("treecon: User is not on valid users list\n");
		goto out_error;
	}

bind:
	conn->id = req->connect_id;
	conn->share = share;
	resp->status = KSMBD_TREE_CONN_STATUS_OK;
	resp->connection_flags = conn->flags;

	if (sm_handle_tree_connect(req->session_id, user, conn))
		pr_err("treecon: Unable to bind tree connection\n");

	g_rw_lock_writer_lock(&share->update_lock);
	clear_share_flag(share, KSMBD_SHARE_FLAG_UPDATE);
	g_rw_lock_writer_unlock(&share->update_lock);

	return 0;
out_error:
	tcm_tree_conn_free(conn);
	shm_close_connection(share);
	put_ksmbd_share(share);
	put_ksmbd_user(user);
	return -EINVAL;
}

int tcm_handle_tree_disconnect(unsigned long long sess_id,
			       unsigned long long tree_conn_id)
{
	sm_handle_tree_disconnect(sess_id, tree_conn_id);
	return 0;
}
