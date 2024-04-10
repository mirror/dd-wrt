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
#include "management/session.h"
#include "management/tree_conn.h"
#include "management/user.h"
#include "tools.h"

static GHashTable	*sessions_table;
static GRWLock		sessions_table_lock;

static void kill_ksmbd_session(struct ksmbd_session *sess)
{
	g_list_free_full(sess->tree_conns, (GDestroyNotify)tcm_tree_conn_free);
	g_rw_lock_clear(&sess->update_lock);
	g_free(sess);
}

static struct ksmbd_session *new_ksmbd_session(unsigned long long id,
					       struct ksmbd_user *user)
{
	struct ksmbd_session *sess;

	sess = g_try_malloc0(sizeof(struct ksmbd_session));
	if (!sess)
		return NULL;

	g_rw_lock_init(&sess->update_lock);
	sess->ref_counter = 1;
	sess->id = id;
	sess->user = user;
	return sess;
}

static void sm_clear_sessions(void)
{
	struct ksmbd_session *sess;
	GHashTableIter iter;

	ghash_for_each(sess, sessions_table, iter)
		kill_ksmbd_session(sess);
}

static int __sm_remove_session(struct ksmbd_session *sess)
{
	int ret = -EINVAL;

	g_rw_lock_writer_lock(&sessions_table_lock);
	if (g_hash_table_remove(sessions_table, &sess->id))
		ret = 0;
	g_rw_lock_writer_unlock(&sessions_table_lock);

	if (!ret)
		kill_ksmbd_session(sess);
	return ret;
}

static struct ksmbd_session *__get_session(struct ksmbd_session *sess)
{
	struct ksmbd_session *ret = NULL;

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

static void __put_session(struct ksmbd_session *sess)
{
	int drop = 0;

	g_rw_lock_writer_lock(&sess->update_lock);
	sess->ref_counter--;
	drop = !sess->ref_counter;
	g_rw_lock_writer_unlock(&sess->update_lock);

	if (drop)
		__sm_remove_session(sess);
}

static struct ksmbd_session *__sm_lookup_session(unsigned long long id)
{
	return g_hash_table_lookup(sessions_table, &id);
}

static struct ksmbd_session *sm_lookup_session(unsigned long long id)
{
	struct ksmbd_session *sess;

	g_rw_lock_reader_lock(&sessions_table_lock);
	sess = __sm_lookup_session(id);
	if (sess)
		sess = __get_session(sess);
	g_rw_lock_reader_unlock(&sessions_table_lock);
	return sess;
}

int sm_handle_tree_connect(unsigned long long id,
			   struct ksmbd_user *user,
			   struct ksmbd_tree_conn *tree_conn)
{
	struct ksmbd_session *sess, *lookup;

retry:
	sess = sm_lookup_session(id);
	if (!sess) {
		sess = new_ksmbd_session(id, user);
		if (!sess)
			return -EINVAL;

		g_rw_lock_writer_lock(&sessions_table_lock);
		lookup = __sm_lookup_session(id);
		if (lookup)
			lookup = __get_session(lookup);
		if (lookup) {
			kill_ksmbd_session(sess);
			sess = lookup;
		}
		if (!g_hash_table_insert(sessions_table, &(sess->id), sess)) {
			kill_ksmbd_session(sess);
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
	struct ksmbd_session *sess;

	sess = sm_lookup_session(id);
	if (sess) {
		__put_session(sess);
		return ret;
	}

	if (g_atomic_int_add(&global_conf.sessions_cap, -1) < 1) {
		ret = -EINVAL;
		g_atomic_int_inc(&global_conf.sessions_cap);
	}
	return ret;
}

static int lookup_tree_conn(const struct ksmbd_tree_conn *tree_conn,
			    const struct ksmbd_tree_conn *dummy)
{
	return !(tree_conn->id == dummy->id);
}

int sm_handle_tree_disconnect(unsigned long long sess_id,
			      unsigned long long tree_conn_id)
{
	struct ksmbd_tree_conn dummy;
	struct ksmbd_session *sess;
	GList *tc_list;

	sess = sm_lookup_session(sess_id);
	if (!sess)
		return 0;

	g_atomic_int_inc(&global_conf.sessions_cap);
	g_rw_lock_writer_lock(&sess->update_lock);
	dummy.id = tree_conn_id;
	tc_list = g_list_find_custom(sess->tree_conns,
				     &dummy,
				     (GCompareFunc)lookup_tree_conn);
	if (tc_list) {
		struct ksmbd_tree_conn *tree_conn;

		tree_conn = (struct ksmbd_tree_conn *)tc_list->data;
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
		sessions_table = NULL;
	}
}

void sm_init(void)
{
	if (!sessions_table)
		sessions_table = g_hash_table_new(g_int64_hash, g_int64_equal);
}
