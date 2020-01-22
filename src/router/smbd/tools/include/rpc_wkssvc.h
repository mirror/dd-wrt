/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __USMBD_RPC_WKSSVC_H__
#define __USMBD_RPC_WKSSVC_H__

struct usmbd_rpc_command;
struct usmbd_rpc_pipe;

int rpc_wkssvc_read_request(struct usmbd_rpc_pipe *pipe,
			    struct usmbd_rpc_command *resp,
			    int max_resp_sz);

int rpc_wkssvc_write_request(struct usmbd_rpc_pipe *pipe);

#endif /* __USMBD_RPC_WKSSVC_H__ */
