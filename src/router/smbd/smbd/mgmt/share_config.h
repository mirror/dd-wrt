/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SHARE_CONFIG_MANAGEMENT_H__
#define __SHARE_CONFIG_MANAGEMENT_H__

#include <linux/workqueue.h>
#include <linux/hashtable.h>
#include <linux/path.h>

#include "../glob.h"  /* FIXME */

struct smbd_share_config {
	char			*name;
	char			*path;

	unsigned int		path_sz;
	unsigned int		flags;
	struct list_head	veto_list;

	struct path		vfs_path;

	atomic_t		refcount;
	struct hlist_node	hlist;
	struct work_struct	free_work;
	unsigned short		create_mask;
	unsigned short		directory_mask;
	unsigned short		force_create_mode;
	unsigned short		force_directory_mode;
	unsigned short		force_uid;
	unsigned short		force_gid;
};

static inline int share_config_create_mode(struct smbd_share_config *share,
	umode_t posix_mode)
{
	if (!share->force_create_mode) {
		if (!posix_mode)
			return share->create_mask;
		else
			return posix_mode & share->create_mask;
	}
	return share->force_create_mode & share->create_mask;
}

static inline int share_config_directory_mode(struct smbd_share_config *share,
	umode_t posix_mode)
{
	if (!share->force_directory_mode) {
		if (!posix_mode)
			return share->directory_mask;
		else
			return posix_mode & share->directory_mask;
	}

	return share->force_directory_mode & share->directory_mask;
}

static inline int test_share_config_flag(struct smbd_share_config *share,
					 int flag)
{
	return share->flags & flag;
}

extern void __smbd_share_config_put(struct smbd_share_config *share);

static inline void smbd_share_config_put(struct smbd_share_config *share)
{
	if (!atomic_dec_and_test(&share->refcount))
		return;
	__smbd_share_config_put(share);
}

struct smbd_share_config *smbd_share_config_get(char *name);
bool smbd_share_veto_filename(struct smbd_share_config *share,
			       const char *filename);
void smbd_share_configs_cleanup(void);

#endif /* __SHARE_CONFIG_MANAGEMENT_H__ */
