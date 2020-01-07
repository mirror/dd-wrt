// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/slab.h>

#include "user_config.h"
#include "../buffer_pool.h"
#include "../transport_ipc.h"
#include "../smbd_server.h" /* FIXME */

struct smbd_user *smbd_alloc_user(const char *account)
{
	struct smbd_login_response *resp;
	struct smbd_user *user = NULL;

	resp = smbd_ipc_login_request(account);
	if (!resp)
		return NULL;

	if (!(resp->status & SMBD_USER_FLAG_OK))
		goto out;

	user = smbd_alloc(sizeof(struct smbd_user));
	if (!user)
		goto out;

	user->name = kstrdup(resp->account, GFP_KERNEL);
	user->flags = resp->status;
	user->gid = resp->gid;
	user->uid = resp->uid;
	user->passkey_sz = resp->hash_sz;
	user->passkey = smbd_alloc(resp->hash_sz);
	if (user->passkey)
		memcpy(user->passkey, resp->hash, resp->hash_sz);

	if (!user->name || !user->passkey) {
		kfree(user->name);
		smbd_free(user->passkey);
		smbd_free(user);
		user = NULL;
	}
out:
	smbd_free(resp);
	return user;
}

void smbd_free_user(struct smbd_user *user)
{
	smbd_ipc_logout_request(user->name);
	kfree(user->name);
	smbd_free(user->passkey);
	smbd_free(user);
}

int smbd_anonymous_user(struct smbd_user *user)
{
	if (user->name[0] == '\0')
		return 1;
	return 0;
}
