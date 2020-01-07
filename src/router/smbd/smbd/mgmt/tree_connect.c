// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/list.h>
#include <linux/slab.h>

#include "../smbd_server.h" /* FIXME */
#include "../buffer_pool.h"
#include "../transport_ipc.h"
#include "../connection.h"

#include "tree_connect.h"
#include "user_config.h"
#include "share_config.h"
#include "user_session.h"

struct smbd_tree_conn_status
smbd_tree_conn_connect(struct smbd_session *sess, char *share_name)
{
	struct smbd_tree_conn_status status = {-EINVAL, NULL};
	struct smbd_tree_connect_response *resp = NULL;
	struct smbd_share_config *sc;
	struct smbd_tree_connect *tree_conn = NULL;
	struct sockaddr *peer_addr;

	sc = smbd_share_config_get(share_name);
	if (!sc)
		return status;

	tree_conn = smbd_alloc(sizeof(struct smbd_tree_connect));
	if (!tree_conn) {
		status.ret = -ENOMEM;
		goto out_error;
	}

	tree_conn->id = smbd_acquire_tree_conn_id(sess);
	if (tree_conn->id < 0) {
		status.ret = -EINVAL;
		goto out_error;
	}

	peer_addr = SMBD_TCP_PEER_SOCKADDR(sess->conn);
	resp = smbd_ipc_tree_connect_request(sess,
					      sc,
					      tree_conn,
					      peer_addr);
	if (!resp) {
		status.ret = -EINVAL;
		goto out_error;
	}

	status.ret = resp->status;
	if (status.ret != SMBD_TREE_CONN_STATUS_OK)
		goto out_error;

	tree_conn->flags = resp->connection_flags;
	tree_conn->user = sess->user;
	tree_conn->share_conf = sc;
	status.tree_conn = tree_conn;

	list_add(&tree_conn->list, &sess->tree_conn_list);

	smbd_free(resp);
	return status;

out_error:
	if (tree_conn)
		smbd_release_tree_conn_id(sess, tree_conn->id);
	smbd_share_config_put(sc);
	smbd_free(tree_conn);
	smbd_free(resp);
	return status;
}

int smbd_tree_conn_disconnect(struct smbd_session *sess,
			       struct smbd_tree_connect *tree_conn)
{
	int ret;

	ret = smbd_ipc_tree_disconnect_request(sess->id, tree_conn->id);
	smbd_release_tree_conn_id(sess, tree_conn->id);
	list_del(&tree_conn->list);
	smbd_share_config_put(tree_conn->share_conf);
	smbd_free(tree_conn);
	return ret;
}

struct smbd_tree_connect *smbd_tree_conn_lookup(struct smbd_session *sess,
						  unsigned int id)
{
	struct smbd_tree_connect *tree_conn;
	struct list_head *tmp;

	list_for_each(tmp, &sess->tree_conn_list) {
		tree_conn = list_entry(tmp, struct smbd_tree_connect, list);
		if (tree_conn->id == id)
			return tree_conn;
	}
	return NULL;
}

struct smbd_share_config *smbd_tree_conn_share(struct smbd_session *sess,
						 unsigned int id)
{
	struct smbd_tree_connect *tc;

	tc = smbd_tree_conn_lookup(sess, id);
	if (tc)
		return tc->share_conf;
	return NULL;
}

int smbd_tree_conn_session_logoff(struct smbd_session *sess)
{
	int ret = 0;

	while (!list_empty(&sess->tree_conn_list)) {
		struct smbd_tree_connect *tc;

		tc = list_entry(sess->tree_conn_list.next,
				struct smbd_tree_connect,
				list);
		ret |= smbd_tree_conn_disconnect(sess, tc);
	}

	return ret;
}
