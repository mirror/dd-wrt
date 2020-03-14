/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __MANAGEMENT_USER_H__
#define __MANAGEMENT_USER_H__

#include <sys/types.h>
#include <pwd.h>

struct ksmbd_user {
	char		*name;
	char		*pass_b64;

	char		*pass;
	int		pass_sz;

	uid_t		uid;
	gid_t		gid;

	int		ref_count;
	int		flags;
	int		state;
	pthread_rwlock_t	update_lock;
};

static inline void set_user_flag(struct ksmbd_user *user, int bit)
{
	user->flags |= bit;
}

static inline int test_user_flag(struct ksmbd_user *user, int bit)
{
	return user->flags & bit;
}

struct ksmbd_user *get_ksmbd_user(struct ksmbd_user *user);
void put_ksmbd_user(struct ksmbd_user *user);

struct ksmbd_user *usm_lookup_user(char *name);

int usm_update_user_password(struct ksmbd_user *user, char *pass);

int usm_add_new_user(char *name, char *pwd);
int usm_add_update_user_from_pwdentry(char *data);

void usm_remove_all_users(void);

void usm_destroy(void);
int usm_init(void);

typedef void (*walk_users)(void *value,
			   unsigned long long key,
			   void *user_data);
void foreach_ksmbd_user(walk_users cb, void *user_data);

struct ksmbd_login_request;
struct ksmbd_login_response;

int usm_handle_login_request(struct ksmbd_login_request *req,
			     struct ksmbd_login_response *resp);

#endif /* __MANAGEMENT_USER_H__ */
