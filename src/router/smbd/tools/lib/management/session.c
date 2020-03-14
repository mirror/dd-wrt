// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <linux/ksmbd_server.h>
#include <pthread.h>
#include <management/session.h>
#include <management/tree_conn.h>
#include <management/user.h>
#include <ksmbdtools.h>

static struct LIST *sessions_table;
static pthread_rwlock_t sessions_table_lock;

#define pthread_rwlock_unlock(a)
#define pthread_rwlock_wrlock(a)
#define pthread_rwlock_rdlock(a)
#define pthread_rwlock_init(a,b)
#define pthread_rwlock_destroy(a)

static void __free_func(void *data, unsigned long long id, void *user_data)
{
	struct usmbd_tree_conn *tree_conn;

	tree_conn = (struct usmbd_tree_conn *)data;
	tcm_tree_conn_free(tree_conn);
}

static void kill_usmbd_session(struct usmbd_session *sess)
{
	list_foreach(&sess->tree_conns, __free_func, NULL);
	list_clear(&sess->tree_conns);
	pthread_rwlock_destroy(&sess->update_lock);
	free(sess);
}

static struct usmbd_session *new_usmbd_session(unsigned long long id,
					       struct usmbd_user *user)
{
	struct usmbd_session *sess;

	sess = calloc(1, sizeof(struct usmbd_session));
	if (!sess)
		return NULL;

	pthread_rwlock_init(&sess->update_lock, NULL);
	sess->ref_counter = 1;
	sess->id = id;
	sess->user = user;
	return sess;
}

static void free_hash_entry(void *s, unsigned long long id, void *user_data)
{
	kill_usmbd_session(s);
}

static void sm_clear_sessions(void)
{
	list_foreach(&sessions_table, free_hash_entry, NULL);
}

static int __sm_remove_session(struct usmbd_session *sess)
{
	int ret = -EINVAL;

	pthread_rwlock_wrlock(&sessions_table_lock);
	if (list_remove(&sessions_table, sess->id))
		ret = 0;
	pthread_rwlock_unlock(&sessions_table_lock);

	if (!ret)
		kill_usmbd_session(sess);
	return ret;
}

static struct usmbd_session *__get_session(struct usmbd_session *sess)
{
	struct usmbd_session *ret = NULL;

	pthread_rwlock_wrlock(&sess->update_lock);
	if (sess->ref_counter != 0) {
		sess->ref_counter++;
		ret = sess;
	} else {
		ret = NULL;
	}
	pthread_rwlock_unlock(&sess->update_lock);
	return ret;
}

static void __put_session(struct usmbd_session *sess)
{
	int drop = 0;

	pthread_rwlock_wrlock(&sess->update_lock);
	sess->ref_counter--;
	drop = !sess->ref_counter;
	pthread_rwlock_unlock(&sess->update_lock);

	if (drop)
		__sm_remove_session(sess);
}

static struct usmbd_session *__sm_lookup_session(unsigned long long id)
{
	return list_get(&sessions_table, id);
}

static struct usmbd_session *sm_lookup_session(unsigned long long id)
{
	struct usmbd_session *sess;

	pthread_rwlock_rdlock(&sessions_table_lock);
	sess = __sm_lookup_session(id);
	if (sess)
		sess = __get_session(sess);
	pthread_rwlock_unlock(&sessions_table_lock);
	return sess;
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

		pthread_rwlock_wrlock(&sessions_table_lock);
		lookup = __sm_lookup_session(id);
		if (lookup)
			lookup = __get_session(lookup);
		if (lookup) {
			kill_usmbd_session(sess);
			sess = lookup;
		}
		if (!list_add(&sessions_table, sess, sess->id)) {
			kill_usmbd_session(sess);
			sess = NULL;
		}
		pthread_rwlock_unlock(&sessions_table_lock);

		if (!sess)
			goto retry;
	}

	pthread_rwlock_wrlock(&sess->update_lock);
	list_add(&sess->tree_conns, tree_conn, -1);
	pthread_rwlock_unlock(&sess->update_lock);
	return 0;
}

int sm_check_sessions_capacity(unsigned long long id)
{
	int ret = 0;

	if (sm_lookup_session(id))
		return ret;

	if (atomic_int_add(&global_conf.sessions_cap, -1) < 1) {
		ret = -EINVAL;
		atomic_int_inc(&global_conf.sessions_cap);
	}
	return ret;
}

static int lookup_tree_conn(const void *data, const void *user_data)
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
	struct usmbd_session *sess;
	struct usmbd_tree_conn *tree_conn;

	sess = sm_lookup_session(sess_id);
	if (!sess)
		return 0;

	atomic_int_inc(&global_conf.sessions_cap);
	pthread_rwlock_wrlock(&sess->update_lock);
	tree_conn = list_get(&sess->tree_conns, tree_conn_id);
	if (tree_conn) {
		list_remove(&sess->tree_conns, tree_conn_id);
		sess->ref_counter--;
		tcm_tree_conn_free(tree_conn);
	}
	pthread_rwlock_unlock(&sess->update_lock);

	put_ksmbd_user(sess->user);
	__put_session(sess);
	return 0;
}

void sm_destroy(void)
{
	if (sessions_table) {
		sm_clear_sessions();
		list_clear(&sessions_table);
	}
	pthread_rwlock_destroy(&sessions_table_lock);
}

int sm_init(void)
{
	list_init(&sessions_table);
	if (!sessions_table)
		return -ENOMEM;
	pthread_rwlock_init(&sessions_table_lock, NULL);
	return 0;
}
