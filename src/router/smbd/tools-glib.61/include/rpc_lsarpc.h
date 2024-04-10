/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 *   Author(s): Namjae Jeon (linkinjeon@kernel.org)
 */

#ifndef __KSMBD_RPC_LSARPC_H__
#define __KSMBD_RPC_LSARPC_H__

#include <smbacl.h>

#define HANDLE_SIZE	20
#define DOMAIN_STR_SIZE	257

struct ksmbd_rpc_command;
struct ksmbd_rpc_pipe;

struct policy_handle {
	unsigned char handle[HANDLE_SIZE];
	struct ksmbd_user *user;
};

struct lsarpc_names_info {
	unsigned int index;
	int type;
	char domain_str[DOMAIN_STR_SIZE];
	struct smb_sid sid;
	struct ksmbd_user *user;
};

int rpc_lsarpc_read_request(struct ksmbd_rpc_pipe *pipe,
			  struct ksmbd_rpc_command *resp,
			  int max_resp_sz);

int rpc_lsarpc_write_request(struct ksmbd_rpc_pipe *pipe);
void rpc_lsarpc_init(void);
void rpc_lsarpc_destroy(void);

#endif /* __KSMBD_RPC_LSARPC_H__ */
