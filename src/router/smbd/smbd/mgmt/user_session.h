/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __USER_SESSION_MANAGEMENT_H__
#define __USER_SESSION_MANAGEMENT_H__

#include <linux/hashtable.h>

#include "../smb_common.h"
#include "../ntlmssp.h"

#ifdef CONFIG_SMB_INSECURE_SERVER
#define CIFDS_SESSION_FLAG_SMB1		BIT(0)
#endif
#define CIFDS_SESSION_FLAG_SMB2		BIT(1)

#define PREAUTH_HASHVALUE_SIZE		64

struct ksmbd_file_table;

struct channel {
	__u8			smb3signingkey[SMB3_SIGN_KEY_SIZE];
	struct ksmbd_conn	*conn;
	struct list_head	chann_list;
};

struct preauth_session {
	__u8			Preauth_HashValue[PREAUTH_HASHVALUE_SIZE];
	u64			id;
	struct list_head	preauth_entry;
};

struct ksmbd_session {
	u64				id;

	struct ksmbd_user		*user;
	struct ksmbd_conn		*conn;
	unsigned int			sequence_number;
	unsigned int			flags;

	bool				sign;
	bool				enc;
	bool				is_anonymous;

	int				state;
	__u8				*Preauth_HashValue;

	char				sess_key[CIFS_KEY_SIZE];

	struct hlist_node		hlist;
	rwlock_t			chann_lock;
	struct list_head		ksmbd_chann_list;
	struct list_head		tree_conn_list;
	struct ida			tree_conn_ida;
	struct list_head		rpc_handle_list;

	__u8				smb3encryptionkey[SMB3_ENC_DEC_KEY_SIZE];
	__u8				smb3decryptionkey[SMB3_ENC_DEC_KEY_SIZE];
	__u8				smb3signingkey[SMB3_SIGN_KEY_SIZE];

	struct list_head		sessions_entry;
	struct ksmbd_file_table		file_table;
	atomic_t			refcnt;
	unsigned long last_active;
	rwlock_t tree_conns_lock;
};

static inline int test_session_flag(struct ksmbd_session *sess, int bit)
{
	return sess->flags & bit;
}

static inline void set_session_flag(struct ksmbd_session *sess, int bit)
{
	sess->flags |= bit;
}

static inline void clear_session_flag(struct ksmbd_session *sess, int bit)
{
	sess->flags &= ~bit;
}

#ifdef CONFIG_SMB_INSECURE_SERVER
static struct ksmbd_session *ksmbd_smb1_session_create(void);
#endif
static struct ksmbd_session *ksmbd_smb2_session_create(void);

static void ksmbd_session_destroy(struct ksmbd_session *sess);

static struct ksmbd_session *ksmbd_session_lookup_slowpath(unsigned long long id);
static struct ksmbd_session *ksmbd_session_lookup(struct ksmbd_conn *conn,
					   unsigned long long id);
static void ksmbd_session_register(struct ksmbd_conn *conn,
			    struct ksmbd_session *sess);
static void ksmbd_sessions_deregister(struct ksmbd_conn *conn);
static struct ksmbd_session *ksmbd_session_lookup_all(struct ksmbd_conn *conn,
					       unsigned long long id);
static struct preauth_session *ksmbd_preauth_session_alloc(struct ksmbd_conn *conn,
						    u64 sess_id);
static struct preauth_session *ksmbd_preauth_session_lookup(struct ksmbd_conn *conn,
						     unsigned long long id);

static int ksmbd_acquire_tree_conn_id(struct ksmbd_session *sess);
static void ksmbd_release_tree_conn_id(struct ksmbd_session *sess, int id);

static int ksmbd_session_rpc_open(struct ksmbd_session *sess, char *rpc_name);
static void ksmbd_session_rpc_close(struct ksmbd_session *sess, int id);
static int ksmbd_session_rpc_method(struct ksmbd_session *sess, int id);
static int get_session(struct ksmbd_session *sess);
static void put_session(struct ksmbd_session *sess);
#endif /* __USER_SESSION_MANAGEMENT_H__ */
