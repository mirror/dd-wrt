/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __KSMBD_TRANSPORT_IPC_H__
#define __KSMBD_TRANSPORT_IPC_H__

#include <linux/wait.h>

#define KSMBD_IPC_MAX_PAYLOAD	4096

static struct ksmbd_login_response *
ksmbd_ipc_login_request(const char *account);

struct ksmbd_session;
struct ksmbd_share_config;
struct ksmbd_tree_connect;
struct sockaddr;

static struct ksmbd_tree_connect_response *
ksmbd_ipc_tree_connect_request(struct ksmbd_session *sess,
			       struct ksmbd_share_config *share,
			       struct ksmbd_tree_connect *tree_conn,
			       struct sockaddr *peer_addr);
static int ksmbd_ipc_tree_disconnect_request(unsigned long long session_id,
				      unsigned long long connect_id);
static int ksmbd_ipc_logout_request(const char *account, int flags);
static struct ksmbd_share_config_response *
ksmbd_ipc_share_config_request(const char *name);
static struct ksmbd_spnego_authen_response *
ksmbd_ipc_spnego_authen_request(const char *spnego_blob, int blob_len);
static int ksmbd_ipc_id_alloc(void);
static void ksmbd_rpc_id_free(int handle);
static struct ksmbd_rpc_command *ksmbd_rpc_open(struct ksmbd_session *sess, int handle);
static struct ksmbd_rpc_command *ksmbd_rpc_close(struct ksmbd_session *sess, int handle);
static struct ksmbd_rpc_command *ksmbd_rpc_write(struct ksmbd_session *sess, int handle,
					  void *payload, size_t payload_sz);
static struct ksmbd_rpc_command *ksmbd_rpc_read(struct ksmbd_session *sess, int handle);
static struct ksmbd_rpc_command *ksmbd_rpc_ioctl(struct ksmbd_session *sess, int handle,
					  void *payload, size_t payload_sz);
static struct ksmbd_rpc_command *ksmbd_rpc_rap(struct ksmbd_session *sess, void *payload,
					size_t payload_sz);
static void ksmbd_ipc_release(void);
static void ksmbd_ipc_soft_reset(void);
static int ksmbd_ipc_init(void);
#endif /* __KSMBD_TRANSPORT_IPC_H__ */
