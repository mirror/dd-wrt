// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <linux/ksmbd_server.h>

#include <management/user.h>
#include <ksmbdtools.h>

static struct LIST *users_table;
#if 0
static pthread_rwlock_t users_table_lock;
#else
#define pthread_rwlock_unlock(a)
#define pthread_rwlock_wrlock(a)
#define pthread_rwlock_rdlock(a)
#define pthread_rwlock_init(a,b)
#define pthread_rwlock_destroy(a)
#endif
static void kill_usmbd_user(struct usmbd_user *user)
{
	pr_debug("Kill user %s\n", user->name);

	free(user->name);
	free(user->pass_b64);
	free(user->pass);
	pthread_rwlock_destroy(&user->update_lock);
	free(user);
}

static int __usm_remove_user(struct usmbd_user *user)
{
	int ret = -EINVAL;

	pthread_rwlock_wrlock(&users_table_lock);
	if (list_remove(&users_table, list_tokey(user->name)))
		ret = 0;
	pthread_rwlock_unlock(&users_table_lock);

	if (!ret)
		kill_usmbd_user(user);
	return ret;
}

struct usmbd_user *get_usmbd_user(struct usmbd_user *user)
{
	pthread_rwlock_wrlock(&user->update_lock);
	if (user->ref_count != 0)
		user->ref_count++;
	else
		user = NULL;
	pthread_rwlock_unlock(&user->update_lock);
	return user;
}

void put_usmbd_user(struct usmbd_user *user)
{
	int drop;

	if (!user)
		return;

	pthread_rwlock_wrlock(&user->update_lock);
	user->ref_count--;
	drop = !user->ref_count;
	pthread_rwlock_unlock(&user->update_lock);

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

	pthread_rwlock_destroy(&user->update_lock);
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

static void free_hash_entry(void *u, unsigned long long id, void *user_data)
{
	kill_usmbd_user(u);
}

static void usm_clear_users(void)
{
	list_foreach(&users_table, free_hash_entry, NULL);
}

void usm_destroy(void)
{
	if (users_table) {
		usm_clear_users();
		list_clear(&users_table);
	}
	pthread_rwlock_destroy(&users_table_lock);
}

int usm_init(void)
{
	list_init(&users_table);
	if (!users_table)
		return -ENOMEM;
	pthread_rwlock_init(&users_table_lock, NULL);
	return 0;
}

static struct usmbd_user *__usm_lookup_user(char *name)
{
	return list_get(&users_table, list_tokey(name));
}

struct usmbd_user *usm_lookup_user(char *name)
{
	struct usmbd_user *user, *ret;

	if (!name)
		return NULL;

	pthread_rwlock_rdlock(&users_table_lock);
	user = __usm_lookup_user(name);
	if (user) {
		ret = get_usmbd_user(user);
		if (!ret)
			user = NULL;
	}
	pthread_rwlock_unlock(&users_table_lock);
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

	pthread_rwlock_wrlock(&users_table_lock);
	if (__usm_lookup_user(name)) {
		pthread_rwlock_unlock(&users_table_lock);
		pr_info("User already exists %s\n", name);
		kill_usmbd_user(user);
		return 0;
	}
	if (!list_add_str(&users_table, user, user->name)) {
		kill_usmbd_user(user);
		ret = -EINVAL;
	}
	pthread_rwlock_unlock(&users_table_lock);
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
	name = strdup(data);
	pwd = strdup(pos + 1);

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

void foreach_usmbd_user(walk_users cb, void *user_data)
{
	pthread_rwlock_rdlock(&users_table_lock);
	list_foreach(&users_table, cb, user_data);
	pthread_rwlock_unlock(&users_table_lock);
}

int usm_update_user_password(struct usmbd_user *user, char *pswd)
{
	size_t pass_sz;
	char *pass_b64 = strdup(pswd);
	char *pass = base64_decode(pass_b64, &pass_sz);

	if (!pass_b64 || !pass) {
		free(pass_b64);
		free(pass);
		pr_err("Cannot allocate new user entry: out of memory\n");
		return -ENOMEM;
	}

	pr_debug("Update user password: %s\n", user->name);
	pthread_rwlock_wrlock(&user->update_lock);
	free(user->pass_b64);
	free(user->pass);
	user->pass_b64 = pass_b64;
	user->pass = pass;
	user->pass_sz = (int)pass_sz;
	pthread_rwlock_unlock(&user->update_lock);

	return 0;
}

static int usm_copy_user_passhash(struct usmbd_user *user,
				  char *pass,
				  size_t sz)
{
	int ret = -ENOSPC;

	if (test_user_flag(user, USMBD_USER_FLAG_GUEST_ACCOUNT))
		return 0;

	pthread_rwlock_rdlock(&user->update_lock);
	if (sz >= user->pass_sz) {
		memcpy(pass, user->pass, user->pass_sz);
		ret = user->pass_sz;
	}
	pthread_rwlock_unlock(&user->update_lock);

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
