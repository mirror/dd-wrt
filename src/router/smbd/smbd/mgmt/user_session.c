// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/list.h>
#include <linux/slab.h>
#include <linux/rwsem.h>

#include "smbd_ida.h"
#include "user_session.h"
#include "user_config.h"
#include "tree_connect.h"
#include "../transport_ipc.h"
#include "../connection.h"
#include "../buffer_pool.h"
#include "../smbd_server.h" /* FIXME */
#include "../vfs_cache.h"

static struct smbd_ida *session_ida;

#define SESSION_HASH_BITS		3
static DEFINE_HASHTABLE(sessions_table, SESSION_HASH_BITS);
static DECLARE_RWSEM(sessions_table_lock);

struct smbd_session_rpc {
	int			id;
	unsigned int		method;
	struct list_head	list;
};

static void free_channel_list(struct smbd_session *sess)
{
	struct channel *chann;
	struct list_head *tmp, *t;

	list_for_each_safe(tmp, t, &sess->smbd_chann_list) {
		chann = list_entry(tmp, struct channel, chann_list);
		if (chann) {
			list_del(&chann->chann_list);
			kfree(chann);
		}
	}
}

static void __session_rpc_close(struct smbd_session *sess,
				struct smbd_session_rpc *entry)
{
	struct smbd_rpc_command *resp;

	resp = smbd_rpc_close(sess, entry->id);
	if (!resp)
		pr_err("Unable to close RPC pipe %d\n", entry->id);

	smbd_free(resp);
	smbd_rpc_id_free(entry->id);
	smbd_free(entry);
}

static void smbd_session_rpc_clear_list(struct smbd_session *sess)
{
	struct smbd_session_rpc *entry;

	while (!list_empty(&sess->rpc_handle_list)) {
		entry = list_entry(sess->rpc_handle_list.next,
				   struct smbd_session_rpc,
				   list);

		list_del(&entry->list);
		__session_rpc_close(sess, entry);
	}
}

static int __rpc_method(char *rpc_name)
{
	if (!strcmp(rpc_name, "\\srvsvc") || !strcmp(rpc_name, "srvsvc"))
		return SMBD_RPC_SRVSVC_METHOD_INVOKE;

	if (!strcmp(rpc_name, "\\wkssvc") || !strcmp(rpc_name, "wkssvc"))
		return SMBD_RPC_WKSSVC_METHOD_INVOKE;

	if (!strcmp(rpc_name, "LANMAN") || !strcmp(rpc_name, "lanman"))
		return SMBD_RPC_RAP_METHOD;

	smbd_err("Unsupported RPC: %s\n", rpc_name);
	return 0;
}

int smbd_session_rpc_open(struct smbd_session *sess, char *rpc_name)
{
	struct smbd_session_rpc *entry;
	struct smbd_rpc_command *resp;
	int method;

	method = __rpc_method(rpc_name);
	if (!method)
		return -EINVAL;

	entry = smbd_alloc(sizeof(struct smbd_session_rpc));
	if (!entry)
		return -EINVAL;

	list_add(&entry->list, &sess->rpc_handle_list);
	entry->method = method;
	entry->id = smbd_ipc_id_alloc();
	if (entry->id < 0)
		goto error;

	resp = smbd_rpc_open(sess, entry->id);
	if (!resp)
		goto error;

	smbd_free(resp);
	return entry->id;
error:
	list_del(&entry->list);
	smbd_free(entry);
	return -EINVAL;
}

void smbd_session_rpc_close(struct smbd_session *sess, int id)
{
	struct smbd_session_rpc *entry;

	list_for_each_entry(entry, &sess->rpc_handle_list, list) {
		if (entry->id == id) {
			list_del(&entry->list);
			__session_rpc_close(sess, entry);
			break;
		}
	}
}

int smbd_session_rpc_method(struct smbd_session *sess, int id)
{
	struct smbd_session_rpc *entry;

	list_for_each_entry(entry, &sess->rpc_handle_list, list) {
		if (entry->id == id)
			return entry->method;
	}
	return 0;
}

void smbd_session_destroy(struct smbd_session *sess)
{
	if (!sess)
		return;

	if (sess->user)
		smbd_free_user(sess->user);

	smbd_destroy_file_table(&sess->file_table);
	smbd_session_rpc_clear_list(sess);
	free_channel_list(sess);
	kfree(sess->Preauth_HashValue);
	smbd_release_id(session_ida, sess->id);

	list_del(&sess->sessions_entry);
	down_write(&sessions_table_lock);
	hash_del(&sess->hlist);
	up_write(&sessions_table_lock);

	smbd_ida_free(sess->tree_conn_ida);
	smbd_free(sess);
}

static struct smbd_session *__session_lookup(unsigned long long id)
{
	struct smbd_session *sess;

	hash_for_each_possible(sessions_table, sess, hlist, id) {
		if (id == sess->id)
			return sess;
	}
	return NULL;
}

void smbd_session_register(struct smbd_conn *conn,
			    struct smbd_session *sess)
{
	sess->conn = conn;
	list_add(&sess->sessions_entry, &conn->sessions);
}

void smbd_sessions_deregister(struct smbd_conn *conn)
{
	struct smbd_session *sess;

	while (!list_empty(&conn->sessions)) {
		sess = list_entry(conn->sessions.next,
				  struct smbd_session,
				  sessions_entry);

		smbd_session_destroy(sess);
	}
}

bool smbd_session_id_match(struct smbd_session *sess, unsigned long long id)
{
	return sess->id == id;
}

struct smbd_session *smbd_session_lookup(struct smbd_conn *conn,
					   unsigned long long id)
{
	struct smbd_session *sess = NULL;

	list_for_each_entry(sess, &conn->sessions, sessions_entry) {
		if (smbd_session_id_match(sess, id))
			return sess;
	}
	return NULL;
}

struct smbd_session *smbd_session_lookup_slowpath(unsigned long long id)
{
	struct smbd_session *sess;

	down_read(&sessions_table_lock);
	sess = __session_lookup(id);
	up_read(&sessions_table_lock);

	return sess;
}

#ifdef CONFIG_SMB_INSECURE_SERVER
static int __init_smb1_session(struct smbd_session *sess)
{
	int id = smbd_acquire_smb1_uid(session_ida);

	if (id < 0)
		return -EINVAL;
	sess->id = id;
	return 0;
}
#endif

static int __init_smb2_session(struct smbd_session *sess)
{
	int id = smbd_acquire_smb2_uid(session_ida);

	if (id < 0)
		return -EINVAL;
	sess->id = id;
	return 0;
}

static struct smbd_session *__session_create(int protocol)
{
	struct smbd_session *sess;
	int ret;

	sess = smbd_alloc(sizeof(struct smbd_session));
	if (!sess)
		return NULL;

	if (smbd_init_file_table(&sess->file_table))
		goto error;

	set_session_flag(sess, protocol);
	INIT_LIST_HEAD(&sess->sessions_entry);
	INIT_LIST_HEAD(&sess->tree_conn_list);
	INIT_LIST_HEAD(&sess->smbd_chann_list);
	INIT_LIST_HEAD(&sess->rpc_handle_list);
	sess->sequence_number = 1;

	switch (protocol) {
#ifdef CONFIG_SMB_INSECURE_SERVER
	case CIFDS_SESSION_FLAG_SMB1:
		ret = __init_smb1_session(sess);
		break;
#endif
	case CIFDS_SESSION_FLAG_SMB2:
		ret = __init_smb2_session(sess);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret)
		goto error;

	sess->tree_conn_ida = smbd_ida_alloc();
	if (!sess->tree_conn_ida)
		goto error;

	down_read(&sessions_table_lock);
	hash_add(sessions_table, &sess->hlist, sess->id);
	up_read(&sessions_table_lock);
	return sess;

error:
	smbd_session_destroy(sess);
	return NULL;
}

struct smbd_session *smbd_smb1_session_create(void)
{
	return __session_create(CIFDS_SESSION_FLAG_SMB1);
}

struct smbd_session *smbd_smb2_session_create(void)
{
	return __session_create(CIFDS_SESSION_FLAG_SMB2);
}

int smbd_acquire_tree_conn_id(struct smbd_session *sess)
{
	int id = -EINVAL;

	if (test_session_flag(sess, CIFDS_SESSION_FLAG_SMB1))
		id = smbd_acquire_smb1_tid(sess->tree_conn_ida);
	if (test_session_flag(sess, CIFDS_SESSION_FLAG_SMB2))
		id = smbd_acquire_smb2_tid(sess->tree_conn_ida);

	return id;
}

void smbd_release_tree_conn_id(struct smbd_session *sess, int id)
{
	if (id >= 0)
		smbd_release_id(sess->tree_conn_ida, id);
}

int smbd_init_session_table(void)
{
	session_ida = smbd_ida_alloc();
	if (!session_ida)
		return -ENOMEM;
	return 0;
}

void smbd_free_session_table(void)
{
	smbd_ida_free(session_ida);
}
