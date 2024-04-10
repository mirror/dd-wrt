/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __KSMBD_SHARE_ADMIN_H__
#define __KSMBD_SHARE_ADMIN_H__

typedef int command_fn(char *smbconf, char *name, char **options);

command_fn command_add_share, command_update_share, command_delete_share;

#endif /* __KSMBD_SHARE_ADMIN_H__ */
