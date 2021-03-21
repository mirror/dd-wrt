/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __KSMBD_SHARE_ADMIN_H__
#define __KSMBD_SHARE_ADMIN_H__

int command_add_share(char *smbconf, char *name, char *opts);
int command_update_share(char *smbconf, char *name, char *opts);
int command_del_share(char *smbconf, char *name);

#endif /* __KSMBD_SHARE_ADMIN_H__ */
