// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include "smbd_ida.h"

struct smbd_ida *smbd_ida_alloc(void)
{
	struct smbd_ida *ida;

	ida = kmalloc(sizeof(struct smbd_ida), GFP_KERNEL);
	if (!ida)
		return NULL;

	ida_init(&ida->map);
	return ida;
}

void smbd_ida_free(struct smbd_ida *ida)
{
	if (!ida)
		return;

	ida_destroy(&ida->map);
	kfree(ida);
}

static inline int __acquire_id(struct smbd_ida *ida, int from, int to)
{
	return ida_simple_get(&ida->map, from, to, GFP_KERNEL);
}

int smbd_acquire_smb1_tid(struct smbd_ida *ida)
{
	return __acquire_id(ida, 0, 0xFFFF);
}

int smbd_acquire_smb2_tid(struct smbd_ida *ida)
{
	int id;

	do {
		id = __acquire_id(ida, 0, 0);
	} while (id == 0xFFFF);

	return id;
}

int smbd_acquire_smb1_uid(struct smbd_ida *ida)
{
	return __acquire_id(ida, 1, 0xFFFE);
}

int smbd_acquire_smb2_uid(struct smbd_ida *ida)
{
	int id;

	do {
		id = __acquire_id(ida, 1, 0);
	} while (id == 0xFFFE);

	return id;
}

int smbd_acquire_async_msg_id(struct smbd_ida *ida)
{
	return __acquire_id(ida, 1, 0);
}

int smbd_acquire_id(struct smbd_ida *ida)
{
	return __acquire_id(ida, 0, 0);
}

void smbd_release_id(struct smbd_ida *ida, int id)
{
	ida_simple_remove(&ida->map, id);
}
