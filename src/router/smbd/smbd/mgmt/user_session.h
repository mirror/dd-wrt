/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __USER_SESSION_MANAGEMENT_H__
#define __USER_SESSION_MANAGEMENT_H__

#include <linux/hashtable.h>

#include "../smb_common.h"
#include "../ntlmssp.h"

#define CIFDS_SESSION_FLAG_SMB1		(1 << 0)
#define CIFDS_SESSION_FLAG_SMB2		(1 << 1)

#define PREAUTH_HASHVALUE_SIZE		64

struct smbd_ida;
struct smbd_file_table;

struct channel {
	__u8			smb3signingkey[SMB3_SIGN_KEY_SIZE];
	struct smbd_conn	*conn;
	struct list_head	chann_list;
};

struct preauth_session {
	__u8			Preauth_HashValue[PREAUTH_HASHVALUE_SIZE];
	uint64_t		sess_id;
	struct list_head	list_entry;
};

struct smbd_session {
	uint64_t			id;

	struct smbd_user		*user;
	struct smbd_conn		*conn;
	unsigned int			sequence_number;
	unsigned int			flags;

	bool				sign;
	bool				enc;
	bool				is_anonymous;

	int				state;
	__u8				*Preauth_HashValue;

	struct ntlmssp_auth		ntlmssp;
	char				sess_key[CIFS_KEY_SIZE];

	struct hlist_node		hlist;
	struct list_head		smbd_chann_list;
	struct list_head		tree_conn_list;
	struct smbd_ida		*tree_conn_ida;
	struct list_head		rpc_handle_list;

	__u8				smb3encryptionkey[SMB3_SIGN_KEY_SIZE];
	__u8				smb3decryptionkey[SMB3_SIGN_KEY_SIZE];
	__u8				smb3signingkey[SMB3_SIGN_KEY_SIZE];

	struct list_head		sessions_entry;
	struct smbd_file_table		file_table;
};

static inline int test_session_flag(struct smbd_session *sess, int bit)
{
	return sess->flags & bit;
}

static inline void set_session_flag(struct smbd_session *sess, int bit)
{
	sess->flags |= bit;
}

static inline void clear_session_flag(struct smbd_session *sess, int bit)
{
	sess->flags &= ~bit;
}

struct smbd_session *smbd_smb1_session_create(void);
struct smbd_session *smbd_smb2_session_create(void);

void smbd_session_destroy(struct smbd_session *sess);

bool smbd_session_id_match(struct smbd_session *sess, unsigned long long id);
struct smbd_session *smbd_session_lookup_slowpath(unsigned long long id);
struct smbd_session *smbd_session_lookup(struct smbd_conn *conn,
					   unsigned long long id);
void smbd_session_register(struct smbd_conn *conn,
			    struct smbd_session *sess);
void smbd_sessions_deregister(struct smbd_conn *conn);

int smbd_acquire_tree_conn_id(struct smbd_session *sess);
void smbd_release_tree_conn_id(struct smbd_session *sess, int id);

int smbd_session_rpc_open(struct smbd_session *sess, char *rpc_name);
void smbd_session_rpc_close(struct smbd_session *sess, int id);
int smbd_session_rpc_method(struct smbd_session *sess, int id);

int smbd_init_session_table(void);
void smbd_free_session_table(void);

#endif /* __USER_SESSION_MANAGEMENT_H__ */
