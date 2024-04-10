// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdint.h>
#include <inttypes.h>

#include "linux/ksmbd_server.h"
#include "management/share.h"
#include "management/user.h"
#include "config_parser.h"
#include "tools.h"

#define KSMBD_USER_STATE_FREEING	1

static GHashTable	*users_table;
static GRWLock		users_table_lock;

static void kill_ksmbd_user(struct ksmbd_user *user)
{
	pr_debug("Kill user `%s' [0x%" PRIXPTR "]\n",
		 user->name,
		 (uintptr_t)user);

	g_free(user->name);
	g_free(user->pass_b64);
	g_free(user->pass);
	g_rw_lock_clear(&user->update_lock);
	g_free(user);
}

int usm_remove_user(struct ksmbd_user *user)
{
	int ret = 0;

	if (user->state != KSMBD_USER_STATE_FREEING) {
		g_rw_lock_writer_lock(&users_table_lock);
		if (!g_hash_table_remove(users_table, user->name))
			ret = -EINVAL;
		g_rw_lock_writer_unlock(&users_table_lock);
	}
	if (!ret)
		kill_ksmbd_user(user);
	return ret;
}

struct ksmbd_user *get_ksmbd_user(struct ksmbd_user *user)
{
	g_rw_lock_writer_lock(&user->update_lock);
	if (user->ref_count != 0) {
		user->ref_count++;
		g_rw_lock_writer_unlock(&user->update_lock);
	} else {
		g_rw_lock_writer_unlock(&user->update_lock);
		user = NULL;
	}
	return user;
}

void put_ksmbd_user(struct ksmbd_user *user)
{
	int drop;

	if (!user)
		return;

	g_rw_lock_writer_lock(&user->update_lock);
	user->ref_count--;
	drop = !user->ref_count;
	g_rw_lock_writer_unlock(&user->update_lock);

	if (!drop)
		return;

	usm_remove_user(user);
}

void usm_remove_all_users(void)
{
	struct ksmbd_user *user;
	GHashTableIter iter;

	g_rw_lock_writer_lock(&users_table_lock);
	ghash_for_each_remove(user, users_table, iter) {
		user->state = KSMBD_USER_STATE_FREEING;
		put_ksmbd_user(user);
	}
	g_rw_lock_writer_unlock(&users_table_lock);
}

static struct ksmbd_user *new_ksmbd_user(char *name, char *pwd)
{
	struct ksmbd_user *user;
	struct passwd *e;
	size_t pass_sz;

	user = g_try_malloc0(sizeof(struct ksmbd_user));
	if (!user)
		return NULL;

	g_rw_lock_init(&user->update_lock);
	user->name = name;
	user->pass_b64 = pwd;
	user->ref_count = 1;

	e = getpwnam(name);
	if (!e) {
		user->uid = KSMBD_SHARE_INVALID_UID;
		user->gid = KSMBD_SHARE_INVALID_GID;
	} else {
		user->uid = e->pw_uid;
		user->gid = e->pw_gid;
	}

	user->pass = base64_decode(user->pass_b64, &pass_sz);
	user->pass_sz = (int)pass_sz;
	return user;
}

static void usm_clear_users(void)
{
	struct ksmbd_user *user;
	GHashTableIter iter;

	ghash_for_each(user, users_table, iter)
		kill_ksmbd_user(user);
}

void usm_destroy(void)
{
	if (users_table) {
		usm_clear_users();
		g_hash_table_destroy(users_table);
		users_table = NULL;
	}
}

void usm_init(void)
{
	if (!users_table)
		users_table = g_hash_table_new(g_str_hash, g_str_equal);
}

static struct ksmbd_user *__usm_lookup_user(char *name)
{
	return g_hash_table_lookup(users_table, name);
}

struct ksmbd_user *usm_lookup_user(char *name)
{
	struct ksmbd_user *user, *ret;

	if (!name)
		return NULL;

	g_rw_lock_reader_lock(&users_table_lock);
	user = __usm_lookup_user(name);
	if (user) {
		ret = get_ksmbd_user(user);
		if (!ret)
			user = NULL;
	}
	g_rw_lock_reader_unlock(&users_table_lock);
	return user;
}

int usm_user_name(char *name, char *p)
{
	int is_name;

	is_name = p > name;
	if (!is_name) {
		pr_debug("User name is missing\n");
		goto out;
	}
	is_name = p - name < KSMBD_REQ_MAX_ACCOUNT_NAME_SZ;
	if (!is_name) {
		pr_debug("User name exceeds %d bytes\n",
			 KSMBD_REQ_MAX_ACCOUNT_NAME_SZ - 1);
		goto out;
	}
	is_name = g_utf8_validate(name, p - name, NULL);
	if (!is_name) {
		pr_debug("User name is not UTF-8\n");
		goto out;
	}
	for (; name < p; name++) {
		is_name = cp_printable(name) && *name != ':';
		if (!is_name) {
			pr_debug("User name contains `%c' [0x%.2X]\n",
				 *name,
				 (unsigned char)*name);
			goto out;
		}
	}
out:
	return is_name;
}

int usm_add_new_user(char *name, char *pwd)
{
	int ret = 0;
	struct ksmbd_user *user = new_ksmbd_user(name, pwd);

	if (!user) {
		g_free(name);
		g_free(pwd);
		return -ENOMEM;
	}

	g_rw_lock_writer_lock(&users_table_lock);
	if (__usm_lookup_user(name)) {
		g_rw_lock_writer_unlock(&users_table_lock);
		pr_debug("Clashed new user `%s' [0x%" PRIXPTR "]\n",
			 name,
			 (uintptr_t)user);
		kill_ksmbd_user(user);
		return 0;
	}

	pr_debug("New user `%s' [0x%" PRIXPTR "]\n",
		 user->name,
		 (uintptr_t)user);
	if (!g_hash_table_insert(users_table, user->name, user)) {
		kill_ksmbd_user(user);
		ret = -EINVAL;
	}
	g_rw_lock_writer_unlock(&users_table_lock);
	return ret;
}

int usm_add_guest_account(char *name)
{
	int ret;
	struct ksmbd_user *user;

	ret = usm_add_new_user(g_strdup(name), g_strdup("NULL"));
	if (ret)
		return ret;

	user = usm_lookup_user(name);
	if (!user) {
		ret = -EINVAL;
	} else {
		set_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT);
		put_ksmbd_user(user);
	}
	return ret;
}

void usm_iter_users(user_cb cb, void *data)
{
	struct ksmbd_user *user;
	GHashTableIter iter;

	g_rw_lock_reader_lock(&users_table_lock);
	ghash_for_each(user, users_table, iter)
		cb(user, data);
	g_rw_lock_reader_unlock(&users_table_lock);
}

void usm_update_user_password(struct ksmbd_user *user, char *pwd)
{
	size_t pass_sz;
	char *pass_b64 = g_strdup(pwd);
	char *pass = base64_decode(pass_b64, &pass_sz);

	pr_debug("Update user password: %s\n", user->name);
	g_rw_lock_writer_lock(&user->update_lock);
	g_free(user->pass_b64);
	g_free(user->pass);
	user->pass_b64 = pass_b64;
	user->pass = pass;
	user->pass_sz = (int)pass_sz;
	g_rw_lock_writer_unlock(&user->update_lock);
}

static int usm_copy_user_passhash(struct ksmbd_user *user,
				  char *pass,
				  size_t sz)
{
	int ret = -ENOSPC;

	if (test_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT))
		return 0;

	g_rw_lock_reader_lock(&user->update_lock);
	if (sz >= user->pass_sz) {
		memcpy(pass, user->pass, user->pass_sz);
		ret = user->pass_sz;
	}
	g_rw_lock_reader_unlock(&user->update_lock);

	return ret;
}

static int usm_copy_user_account(struct ksmbd_user *user,
				 char *account,
				 size_t sz)
{
	int account_sz;

	if (test_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT))
		return 0;

	account_sz = strlen(user->name);
	if (sz >= account_sz) {
		memcpy(account, user->name, account_sz);
		return 0;
	}
	pr_err("Cannot copy user data, buffer overrun\n");
	return -ENOSPC;
}

static void __handle_login_request(struct ksmbd_login_response *resp,
				   struct ksmbd_user *user)
{
	int hash_sz;

	resp->gid = user->gid;
	resp->uid = user->uid;
	resp->status = user->flags;
	resp->status |= KSMBD_USER_FLAG_OK;

	hash_sz = usm_copy_user_passhash(user,
					 resp->hash,
					 sizeof(resp->hash));
	if (hash_sz < 0) {
		resp->status = KSMBD_USER_FLAG_INVALID;
	} else {
		resp->hash_sz = (unsigned short)hash_sz;
		if (usm_copy_user_account(user,
					  resp->account,
					  sizeof(resp->account)))
			resp->status = KSMBD_USER_FLAG_INVALID;
	}
}

int usm_handle_login_request(struct ksmbd_login_request *req,
			     struct ksmbd_login_response *resp)
{
	struct ksmbd_user *user = NULL;
	int null_session = 0;

	if (req->account[0] == '\0')
		null_session = 1;

	if (!null_session)
		user = usm_lookup_user(req->account);
	if (user) {
		__handle_login_request(resp, user);
		put_ksmbd_user(user);
		return 0;
	}

	resp->status = KSMBD_USER_FLAG_BAD_USER;
	if (!null_session &&
		global_conf.map_to_guest == KSMBD_CONF_MAP_TO_GUEST_NEVER)
		return 0;

	if (null_session ||
		global_conf.map_to_guest == KSMBD_CONF_MAP_TO_GUEST_BAD_USER)
		user = usm_lookup_user(global_conf.guest_account);

	if (!user)
		return 0;

	__handle_login_request(resp, user);
	put_ksmbd_user(user);
	return 0;
}

int usm_handle_logout_request(struct ksmbd_logout_request *req)
{
	struct ksmbd_user *user;

	user = usm_lookup_user(req->account);
	if (!user)
		return -ENOENT;

	if (req->account_flags & KSMBD_USER_FLAG_BAD_PASSWORD) {
		if (user->failed_login_count < 10)
			user->failed_login_count++;
		else
			user->flags |= KSMBD_USER_FLAG_DELAY_SESSION;
	} else {
		user->failed_login_count = 0;
		user->flags &= ~KSMBD_USER_FLAG_DELAY_SESSION;
	}

	put_ksmbd_user(user);
	return 0;
}
