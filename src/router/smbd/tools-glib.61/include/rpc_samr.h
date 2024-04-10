/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 *   Author(s): Namjae Jeon (linkinjeon@kernel.org)
 */

#ifndef __KSMBD_RPC_SAMR_H__
#define __KSMBD_RPC_SAMR_H__

#include <smbacl.h>

#define HANDLE_SIZE	20

struct ksmbd_rpc_command;
struct ksmbd_rpc_pipe;

struct connect_handle {
	unsigned char handle[HANDLE_SIZE];
	unsigned int refcount;
	struct ksmbd_user *user;
};

int rpc_samr_read_request(struct ksmbd_rpc_pipe *pipe,
			  struct ksmbd_rpc_command *resp,
			  int max_resp_sz);

int rpc_samr_write_request(struct ksmbd_rpc_pipe *pipe);

void rpc_samr_init(void);
void rpc_samr_destroy(void);
#endif /* __KSMBD_RPC_SAMR_H__ */
