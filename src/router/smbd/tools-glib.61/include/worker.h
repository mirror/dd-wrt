/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __KSMBD_WORKER__H__
#define __KSMBD_WORKER__H__

struct ksmbd_ipc_msg;

int wp_ipc_msg_push(struct ksmbd_ipc_msg *msg);
void wp_destroy(void);
void wp_init(void);

#endif /* __KSMBD_WORKER_H__ */
