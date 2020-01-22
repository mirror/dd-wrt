/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __USMBD_WORKER__H__
#define __USMBD_WORKER__H__

struct usmbd_ipc_msg;

int wp_ipc_msg_push(struct usmbd_ipc_msg *msg);
void wp_destroy(void);
int wp_init(void);

#endif /* __USMBD_WORKER_H__ */
