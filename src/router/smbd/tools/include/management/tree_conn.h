/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __MANAGEMENT_TREE_CONN_H__
#define __MANAGEMENT_TREE_CONN_H__

struct usmbd_share;

struct usmbd_tree_conn {
	unsigned long long	id;

	struct usmbd_share	*share;
	unsigned int		flags;
};

static inline void set_conn_flag(struct usmbd_tree_conn *conn, int bit)
{
	conn->flags |= bit;
}

static inline void clear_conn_flag(struct usmbd_tree_conn *conn, int bit)
{
	conn->flags &= ~bit;
}

static inline int test_conn_flag(struct usmbd_tree_conn *conn, int bit)
{
	return conn->flags & bit;
}

void tcm_tree_conn_free(struct usmbd_tree_conn *conn);

struct usmbd_tree_connect_request;
struct usmbd_tree_connect_response;

int tcm_handle_tree_connect(struct usmbd_tree_connect_request *req,
			    struct usmbd_tree_connect_response *resp);

int tcm_handle_tree_disconnect(unsigned long long sess_id,
			       unsigned long long tree_conn_id);
#endif /* __MANAGEMENT_TREE_CONN_H__ */
