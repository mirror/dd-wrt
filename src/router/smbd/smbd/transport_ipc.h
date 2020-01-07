/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_TRANSPORT_IPC_H__
#define __SMBD_TRANSPORT_IPC_H__

#include <linux/wait.h>
#include "smbd_server.h"  /* FIXME */

#define SMBD_IPC_MAX_PAYLOAD	4096

struct smbd_login_response *
smbd_ipc_login_request(const char *account);

struct smbd_session;
struct smbd_share_config;
struct smbd_tree_connect;
struct sockaddr;

struct smbd_tree_connect_response *
smbd_ipc_tree_connect_request(struct smbd_session *sess,
			       struct smbd_share_config *share,
			       struct smbd_tree_connect *tree_conn,
			       struct sockaddr *peer_addr);

int smbd_ipc_tree_disconnect_request(unsigned long long session_id,
				      unsigned long long connect_id);
int smbd_ipc_logout_request(const char *account);

struct smbd_share_config_response *
smbd_ipc_share_config_request(const char *name);

int smbd_ipc_id_alloc(void);
void smbd_rpc_id_free(int handle);

struct smbd_rpc_command *smbd_rpc_open(struct smbd_session *sess,
					 int handle);
struct smbd_rpc_command *smbd_rpc_close(struct smbd_session *sess,
					  int handle);

struct smbd_rpc_command *smbd_rpc_write(struct smbd_session *sess,
					  int handle,
					  void *payload,
					  size_t payload_sz);
struct smbd_rpc_command *smbd_rpc_read(struct smbd_session *sess,
					 int handle);
struct smbd_rpc_command *smbd_rpc_ioctl(struct smbd_session *sess,
					  int handle,
					  void *payload,
					  size_t payload_sz);
struct smbd_rpc_command *smbd_rpc_rap(struct smbd_session *sess,
					  void *payload,
					  size_t payload_sz);

void smbd_ipc_release(void);
void smbd_ipc_soft_reset(void);
int smbd_ipc_init(void);
#endif /* __SMBD_TRANSPORT_IPC_H__ */
