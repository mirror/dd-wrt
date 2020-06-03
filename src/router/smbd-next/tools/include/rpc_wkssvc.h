/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __KSMBD_RPC_WKSSVC_H__
#define __KSMBD_RPC_WKSSVC_H__

struct ksmbd_rpc_command;
struct ksmbd_rpc_pipe;

int rpc_wkssvc_read_request(struct ksmbd_rpc_pipe *pipe,
			    struct ksmbd_rpc_command *resp,
			    int max_resp_sz);

int rpc_wkssvc_write_request(struct ksmbd_rpc_pipe *pipe);

#endif /* __KSMBD_RPC_WKSSVC_H__ */
