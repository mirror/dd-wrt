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

#include <management/user.h>
#include <usmbdtools.h>

static GHashTable	*users_table;
static GRWLock		users_table_lock;

static void kill_usmbd_user(struct usmbd_user *user)
{
	pr_debug("Kill user %s\n", user->name);

	free(user->name);
	free(user->pass_b64);
	free(user->pass);
	g_rw_lock_clear(&user->update_lock);
	free(user);
}

static int __usm_remove_user(struct usmbd_user *user)
{
	int ret = -EINVAL;

	g_rw_lock_writer_lock(&users_table_lock);
	if (g_hash_table_remove(users_table, user->name))
		ret = 0;
	g_rw_lock_writer_unlock(&users_table_lock);

	if (!ret)
		kill_usmbd_user(user);
	return ret;
}

struct usmbd_user *get_usmbd_user(struct usmbd_user *user)
{
	g_rw_lock_writer_lock(&user->update_lock);
	if (user->ref_count != 0)
		user->ref_count++;
	else
		user = NULL;
	g_rw_lock_writer_unlock(&user->update_lock);
	return user;
}

void put_usmbd_user(struct usmbd_user *user)
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

	__usm_remove_user(user);
}

static struct usmbd_user *new_usmbd_user(char *name, char *pwd)
{
	struct usmbd_user *user;
	struct passwd *passwd;
	size_t pass_sz;

	user = calloc(1, sizeof(struct usmbd_user));
	if (!user)
		return NULL;

	g_rw_lock_clear(&user->update_lock);
	user->name = name;
	user->pass_b64 = pwd;
	user->ref_count = 1;
	user->gid = 9999;
	user->uid = 9999;
	passwd = getpwnam(name);
	if (passwd) {
		user->uid = passwd->pw_uid;
		user->gid = passwd->pw_gid;
	}

	user->pass = base64_decode(user->pass_b64, &pass_sz);
	user->pass_sz = (int)pass_sz;
	return user;
}

static void free_hash_entry(gpointer k, gpointer u, gpointer user_data)
{
	kill_usmbd_user(u);
}

static void usm_clear_users(void)
{
	g_hash_table_foreach(users_table, free_hash_entry, NULL);
}

void usm_destroy(void)
{
	if (users_table) {
		usm_clear_users();
		g_hash_table_destroy(users_table);
	}
	g_rw_lock_clear(&users_table_lock);
}

int usm_init(void)
{
	users_table = g_hash_table_new(g_str_hash, g_str_equal);
	if (!users_table)
		return -ENOMEM;
	g_rw_lock_init(&users_table_lock);
	return 0;
}

static struct usmbd_user *__usm_lookup_user(char *name)
{
	return g_hash_table_lookup(users_table, name);
}

struct usmbd_user *usm_lookup_user(char *name)
{
	struct usmbd_user *user, *ret;

	if (!name)
		return NULL;

	g_rw_lock_reader_lock(&users_table_lock);
	user = __usm_lookup_user(name);
	if (user) {
		ret = get_usmbd_user(user);
		if (!ret)
			user = NULL;
	}
	g_rw_lock_reader_unlock(&users_table_lock);
	return user;
}

int usm_add_new_user(char *name, char *pwd)
{
	int ret = 0;
	struct usmbd_user *user = new_usmbd_user(name, pwd);

	if (!user) {
		free(name);
		free(pwd);
		return -ENOMEM;
	}

	g_rw_lock_writer_lock(&users_table_lock);
	if (__usm_lookup_user(name)) {
		g_rw_lock_writer_unlock(&users_table_lock);
		pr_info("User already exists %s\n", name);
		kill_usmbd_user(user);
		return 0;
	}

	if (!g_hash_table_insert(users_table, user->name, user)) {
		kill_usmbd_user(user);
		ret = -EINVAL;
	}
	g_rw_lock_writer_unlock(&users_table_lock);
	return ret;
}

int usm_add_update_user_from_pwdentry(char *data)
{
	struct usmbd_user *user;
	char *name;
	char *pwd;
	char *pos = strchr(data, ':');
	int ret;

	if (!pos) {
		pr_err("Invalid pwd entry %s\n", data);
		return -EINVAL;
	}

	*pos = 0x00;
	name = g_strdup(data);
	pwd = g_strdup(pos + 1);

	if (!name || !pwd) {
		free(name);
		free(pwd);
		return -ENOMEM;
	}

	user = usm_lookup_user(name);
	if (user) {
		ret = usm_update_user_password(user, pwd);
		put_usmbd_user(user);

		free(name);
		free(pwd);
		return ret;
	}
	return usm_add_new_user(name, pwd);
}

void for_each_usmbd_user(walk_users cb, gpointer user_data)
{
	g_rw_lock_reader_lock(&users_table_lock);
	g_hash_table_foreach(users_table, cb, user_data);
	g_rw_lock_reader_unlock(&users_table_lock);
}

int usm_update_user_password(struct usmbd_user *user, char *pswd)
{
	size_t pass_sz;
	char *pass_b64 = g_strdup(pswd);
	char *pass = base64_decode(pass_b64, &pass_sz);

	if (!pass_b64 || !pass) {
		free(pass_b64);
		free(pass);
		pr_err("Cannot allocate new user entry: out of memory\n");
		return -ENOMEM;
	}

	pr_debug("Update user password: %s\n", user->name);
	g_rw_lock_writer_lock(&user->update_lock);
	free(user->pass_b64);
	free(user->pass);
	user->pass_b64 = pass_b64;
	user->pass = pass;
	user->pass_sz = (int)pass_sz;
	g_rw_lock_writer_unlock(&user->update_lock);

	return 0;
}

static int usm_copy_user_passhash(struct usmbd_user *user,
				  char *pass,
				  size_t sz)
{
	int ret = -ENOSPC;

	if (test_user_flag(user, USMBD_USER_FLAG_GUEST_ACCOUNT))
		return 0;

	g_rw_lock_reader_lock(&user->update_lock);
	if (sz >= user->pass_sz) {
		memcpy(pass, user->pass, user->pass_sz);
		ret = user->pass_sz;
	}
	g_rw_lock_reader_unlock(&user->update_lock);

	return ret;
}

static int usm_copy_user_account(struct usmbd_user *user,
				 char *account,
				 size_t sz)
{
	int account_sz;

	if (test_user_flag(user, USMBD_USER_FLAG_GUEST_ACCOUNT))
		return 0;

	account_sz = strlen(user->name);
	if (sz >= account_sz) {
		memcpy(account, user->name, account_sz);
		return 0;
	}
	pr_err("Cannot copy user data, buffer overrun\n");
	return -ENOSPC;
}

static void __handle_login_request(struct usmbd_login_response *resp,
				   struct usmbd_user *user)
{
	int hash_sz;

	resp->gid = user->gid;
	resp->uid = user->uid;
	resp->status = user->flags;
	resp->status |= USMBD_USER_FLAG_OK;

	hash_sz = usm_copy_user_passhash(user,
					 resp->hash,
					 sizeof(resp->hash));
	if (hash_sz < 0) {
		resp->status = USMBD_USER_FLAG_INVALID;
	} else {
		resp->hash_sz = (unsigned short)hash_sz;
		if (usm_copy_user_account(user,
					  resp->account,
					  sizeof(resp->account)))
			resp->status = USMBD_USER_FLAG_INVALID;
	}
}

int usm_handle_login_request(struct usmbd_login_request *req,
			     struct usmbd_login_response *resp)
{
	struct usmbd_user *user = NULL;
	int null_session = 0;

	if (req->account[0] == '\0')
		null_session = 1;

	if (!null_session)
		user = usm_lookup_user(req->account);
	if (user) {
		__handle_login_request(resp, user);
		put_usmbd_user(user);
		return 0;
	}

	resp->status = USMBD_USER_FLAG_BAD_USER;
	if (!null_session &&
		global_conf.map_to_guest == USMBD_CONF_MAP_TO_GUEST_NEVER)
		return 0;

	if (null_session ||
		global_conf.map_to_guest == USMBD_CONF_MAP_TO_GUEST_BAD_USER)
		user = usm_lookup_user(global_conf.guest_account);

	if (!user)
		return 0;

	__handle_login_request(resp, user);
	put_usmbd_user(user);
	return 0;
}
