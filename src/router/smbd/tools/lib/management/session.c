// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <linux/usmbd_server.h>

#include <management/session.h>
#include <management/tree_conn.h>
#include <management/user.h>
#include <usmbdtools.h>

static GHashTable	*sessions_table;
static GRWLock		sessions_table_lock;

static void __free_func(gpointer data, gpointer user_data)
{
	struct usmbd_tree_conn *tree_conn;

	tree_conn = (struct usmbd_tree_conn *)data;
	tcm_tree_conn_free(tree_conn);
}

static void kill_usmbd_session(struct usmbd_session *sess)
{
	g_list_foreach(sess->tree_conns, __free_func, NULL);
	g_list_free(sess->tree_conns);
	g_rw_lock_clear(&sess->update_lock);
	free(sess);
}

static struct usmbd_session *new_usmbd_session(unsigned long long id,
					       struct usmbd_user *user)
{
	struct usmbd_session *sess;

	sess = calloc(1, sizeof(struct usmbd_session));
	if (!sess)
		return NULL;

	g_rw_lock_init(&sess->update_lock);
	sess->ref_counter = 1;
	sess->id = id;
	sess->user = user;
	return sess;
}

static void free_hash_entry(gpointer k, gpointer s, gpointer user_data)
{
	kill_usmbd_session(s);
}

static void sm_clear_sessions(void)
{
	g_hash_table_foreach(sessions_table, free_hash_entry, NULL);
}

static int __sm_remove_session(struct usmbd_session *sess)
{
	int ret = -EINVAL;

	g_rw_lock_writer_lock(&sessions_table_lock);
	if (g_hash_table_remove(sessions_table, &sess->id))
		ret = 0;
	g_rw_lock_writer_unlock(&sessions_table_lock);

	if (!ret)
		kill_usmbd_session(sess);
	return ret;
}

static struct usmbd_session *__get_session(struct usmbd_session *sess)
{
	struct usmbd_session *ret = NULL;

	g_rw_lock_writer_lock(&sess->update_lock);
	if (sess->ref_counter != 0) {
		sess->ref_counter++;
		ret = sess;
	} else {
		ret = NULL;
	}
	g_rw_lock_writer_unlock(&sess->update_lock);
	return ret;
}

static void __put_session(struct usmbd_session *sess)
{
	int drop = 0;

	g_rw_lock_writer_lock(&sess->update_lock);
	sess->ref_counter--;
	drop = !sess->ref_counter;
	g_rw_lock_writer_unlock(&sess->update_lock);

	if (drop)
		__sm_remove_session(sess);
}

static struct usmbd_session *__sm_lookup_session(unsigned long long id)
{
	return g_hash_table_lookup(sessions_table, &id);
}

static struct usmbd_session *sm_lookup_session(unsigned long long id)
{
	struct usmbd_session *sess;

	g_rw_lock_reader_lock(&sessions_table_lock);
	sess = __sm_lookup_session(id);
	if (sess)
		sess = __get_session(sess);
	g_rw_lock_reader_unlock(&sessions_table_lock);
	return sess;
}

static int sm_insert_session(struct usmbd_session *sess)
{
	int ret;

	g_rw_lock_writer_lock(&sessions_table_lock);
	ret = g_hash_table_insert(sessions_table, &(sess->id), sess);
	g_rw_lock_writer_unlock(&sessions_table_lock);

	return ret;
}

int sm_handle_tree_connect(unsigned long long id,
			   struct usmbd_user *user,
			   struct usmbd_tree_conn *tree_conn)
{
	struct usmbd_session *sess, *lookup;

retry:
	sess = sm_lookup_session(id);
	if (!sess) {
		sess = new_usmbd_session(id, user);
		if (!sess)
			return -EINVAL;

		g_rw_lock_writer_lock(&sessions_table_lock);
		lookup = __sm_lookup_session(id);
		if (lookup)
			lookup = __get_session(lookup);
		if (lookup) {
			kill_usmbd_session(sess);
			sess = lookup;
		}
		if (!g_hash_table_insert(sessions_table, &(sess->id), sess)) {
			kill_usmbd_session(sess);
			sess = NULL;
		}
		g_rw_lock_writer_unlock(&sessions_table_lock);

		if (!sess)
			goto retry;
	}

	g_rw_lock_writer_lock(&sess->update_lock);
	sess->tree_conns = g_list_insert(sess->tree_conns, tree_conn, -1);
	g_rw_lock_writer_unlock(&sess->update_lock);
	return 0;
}

int sm_check_sessions_capacity(unsigned long long id)
{
	int ret = 0;

	if (sm_lookup_session(id))
		return ret;

	if (g_atomic_int_add(&global_conf.sessions_cap, -1) < 1) {
		ret = -EINVAL;
		g_atomic_int_inc(&global_conf.sessions_cap);
	}
	return ret;
}

static gint lookup_tree_conn(gconstpointer data, gconstpointer user_data)
{
	struct usmbd_tree_conn *tree_conn = (struct usmbd_tree_conn *)data;
	struct usmbd_tree_conn *dummy = (struct usmbd_tree_conn *)user_data;

	if (tree_conn->id == dummy->id)
		return 0;
	return 1;
}

int sm_handle_tree_disconnect(unsigned long long sess_id,
			      unsigned long long tree_conn_id)
{
	struct usmbd_tree_conn dummy;
	struct usmbd_session *sess;
	GList *tc_list;

	sess = sm_lookup_session(sess_id);
	if (!sess)
		return 0;

	g_atomic_int_inc(&global_conf.sessions_cap);
	g_rw_lock_writer_lock(&sess->update_lock);
	dummy.id = tree_conn_id;
	tc_list = g_list_find_custom(sess->tree_conns,
				     &dummy,
				     lookup_tree_conn);
	if (tc_list) {
		struct usmbd_tree_conn *tree_conn;

		tree_conn = (struct usmbd_tree_conn *)tc_list->data;
		sess->tree_conns = g_list_remove(sess->tree_conns, tree_conn);
		sess->ref_counter--;
		tcm_tree_conn_free(tree_conn);
	}
	g_rw_lock_writer_unlock(&sess->update_lock);

	__put_session(sess);
	return 0;
}

void sm_destroy(void)
{
	if (sessions_table) {
		sm_clear_sessions();
		g_hash_table_destroy(sessions_table);
	}
	g_rw_lock_clear(&sessions_table_lock);
}

int sm_init(void)
{
	sessions_table = g_hash_table_new(g_int64_hash, g_int64_equal);
	if (!sessions_table)
		return -ENOMEM;
	g_rw_lock_init(&sessions_table_lock);
	return 0;
}
