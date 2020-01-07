// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __MANAGEMENT_USER_H__
#define __MANAGEMENT_USER_H__

#include <sys/types.h>
#include <pwd.h>
#include <glib.h>

struct smbd_user {
	char		*name;
	char		*pass_b64;

	char		*pass;
	int		pass_sz;

	uid_t		uid;
	gid_t		gid;

	int		ref_count;
	int 		flags;
	GRWLock		update_lock;
};

static inline void set_user_flag(struct smbd_user *user, int bit)
{
	user->flags |= bit;
}

static inline int test_user_flag(struct smbd_user *user, int bit)
{
	return user->flags & bit;
}

struct smbd_user *get_smbd_user(struct smbd_user *user);
void put_smbd_user(struct smbd_user *user);

struct smbd_user *usm_lookup_user(char *name);

int usm_update_user_password(struct smbd_user *user, char *pass);

int usm_add_new_user(char *name, char *pwd);
int usm_add_update_user_from_pwdentry(char *data);

void usm_destroy(void);
int usm_init(void);

typedef void (*walk_users)(gpointer key,
			   gpointer value,
			   gpointer user_data);
void for_each_smbd_user(walk_users cb, gpointer user_data);

struct smbd_login_request;
struct smbd_login_response;

int usm_handle_login_request(struct smbd_login_request *req,
			     struct smbd_login_response *resp);

#endif /* __MANAGEMENT_USER_H__ */
