/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __TREE_CONNECT_MANAGEMENT_H__
#define __TREE_CONNECT_MANAGEMENT_H__

#include <linux/hashtable.h>

#include "../smbd_server.h" /* FIXME */

struct smbd_share_config;
struct smbd_user;

struct smbd_tree_connect {
	int				id;

	unsigned int			flags;
	struct smbd_share_config	*share_conf;
	struct smbd_user		*user;

	struct list_head		list;

	int				maximal_access;
	bool				posix_extensions;
};

struct smbd_tree_conn_status {
	unsigned int			ret;
	struct smbd_tree_connect	*tree_conn;
};

static inline int test_tree_conn_flag(struct smbd_tree_connect *tree_conn,
				      int flag)
{
	return tree_conn->flags & flag;
}

struct smbd_session;

struct smbd_tree_conn_status
smbd_tree_conn_connect(struct smbd_session *sess, char *share_name);

int smbd_tree_conn_disconnect(struct smbd_session *sess,
			       struct smbd_tree_connect *tree_conn);

struct smbd_tree_connect *smbd_tree_conn_lookup(struct smbd_session *sess,
						  unsigned int id);

struct smbd_share_config *smbd_tree_conn_share(struct smbd_session *sess,
						 unsigned int id);

int smbd_tree_conn_session_logoff(struct smbd_session *sess);

#endif /* __TREE_CONNECT_MANAGEMENT_H__ */
