/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_CONNECTION_H__
#define __SMBD_CONNECTION_H__

#include <linux/list.h>
#include <linux/ip.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/inet_connection_sock.h>
#include <net/request_sock.h>
#include <linux/kthread.h>
#include <linux/nls.h>

#include "smb_common.h"
#include "smbd_work.h"

#define SMBD_SOCKET_BACKLOG		16

/*
 * WARNING
 *
 * This is nothing but a HACK. Session status should move to channel
 * or to session. As of now we have 1 tcp_conn : 1 smbd_session, but
 * we need to change it to 1 tcp_conn : N smbd_sessions.
 */
enum {
	SMBD_SESS_NEW = 0,
	SMBD_SESS_GOOD,
	SMBD_SESS_EXITING,
	SMBD_SESS_NEED_RECONNECT,
	SMBD_SESS_NEED_NEGOTIATE
};

struct smbd_stats {
	atomic_t			open_files_count;
	atomic64_t			request_served;
};

struct smbd_transport;

struct smbd_conn {
	struct smb_version_values	*vals;
	struct smb_version_ops		*ops;
	struct smb_version_cmds		*cmds;
	unsigned int			max_cmds;
	struct mutex			srv_mutex;
	int				status;
	unsigned int			cli_cap;
	char				*request_buf;
	struct smbd_transport		*transport;
	struct nls_table		*local_nls;
	struct list_head		conns_list;
	/* smb session 1 per user */
	struct list_head		sessions;
	unsigned long			last_active;
	/* How many request are running currently */
	atomic_t			req_running;
	/* References which are made for this Server object*/
	atomic_t			r_count;
	unsigned short			total_credits;
	unsigned short			max_credits;
	spinlock_t			credits_lock;
	wait_queue_head_t		req_running_q;
	/* Lock to protect requests list*/
	spinlock_t			request_lock;
	struct list_head		requests;
	struct list_head		async_requests;
	int				connection_type;
	struct smbd_stats		stats;
	char				ClientGUID[SMB2_CLIENT_GUID_SIZE];
	union {
		/* pending trans request table */
		struct trans_state	*recent_trans;
		/* Used by ntlmssp */
		char			*ntlmssp_cryptkey;
	};

	struct preauth_integrity_info	*preauth_info;

	bool				need_neg;
	/* Supports NTLMSSP */
	bool				sec_ntlmssp;
	/* Supports U2U Kerberos */
	bool				sec_kerberosu2u;
	/* Supports plain Kerberos */
	bool				sec_kerberos;
	/* Supports legacy MS Kerberos */
	bool				sec_mskerberos;
	bool				sign;
	bool				use_spnego:1;
	__u16				cli_sec_mode;
	__u16				srv_sec_mode;
	/* dialect index that server chose */
	__u16				dialect;

	char				*mechToken;

	struct smbd_conn_ops	*conn_ops;

	/* Preauth Session Table */
	struct list_head		preauth_sess_table;

	struct sockaddr_storage		peer_addr;

	/* Identifier for async message */
	struct smbd_ida		*async_ida;

	__le16				cipher_type;
	__le16				compress_algorithm;
	bool				posix_ext_supported;
};

struct smbd_conn_ops {
	int	(*process_fn)(struct smbd_conn *conn);
	int	(*terminate_fn)(struct smbd_conn *conn);
};

struct smbd_transport_ops {
	int (*prepare)(struct smbd_transport *t);
	void (*disconnect)(struct smbd_transport *t);
	int (*read)(struct smbd_transport *t,
			char *buf, unsigned int size);
	int (*writev)(struct smbd_transport *t,
			struct kvec *iovs, int niov, int size,
			bool need_invalidate_rkey, unsigned int remote_key);
	int (*rdma_read)(struct smbd_transport *t,
				void *buf, unsigned int len, u32 remote_key,
				u64 remote_offset, u32 remote_len);
	int (*rdma_write)(struct smbd_transport *t,
				void *buf, unsigned int len, u32 remote_key,
				u64 remote_offset, u32 remote_len);
};

struct smbd_transport {
	struct smbd_conn		*conn;
	struct smbd_transport_ops	*ops;
	struct task_struct		*handler;
};

#define SMBD_TCP_RECV_TIMEOUT	(7 * HZ)
#define SMBD_TCP_SEND_TIMEOUT	(5 * HZ)
#define SMBD_TCP_PEER_SOCKADDR(c)	((struct sockaddr *)&((c)->peer_addr))

bool smbd_conn_alive(struct smbd_conn *conn);
void smbd_conn_wait_idle(struct smbd_conn *conn);

struct smbd_conn *smbd_conn_alloc(void);
void smbd_conn_free(struct smbd_conn *conn);
bool smbd_conn_lookup_dialect(struct smbd_conn *c);
int smbd_conn_write(struct smbd_work *work);
int smbd_conn_rdma_read(struct smbd_conn *conn,
				void *buf, unsigned int buflen,
				u32 remote_key, u64 remote_offset,
				u32 remote_len);
int smbd_conn_rdma_write(struct smbd_conn *conn,
				void *buf, unsigned int buflen,
				u32 remote_key, u64 remote_offset,
				u32 remote_len);

void smbd_conn_enqueue_request(struct smbd_work *work);
int smbd_conn_try_dequeue_request(struct smbd_work *work);
void smbd_conn_init_server_callbacks(struct smbd_conn_ops *ops);

int smbd_conn_handler_loop(void *p);

int smbd_conn_transport_init(void);
void smbd_conn_transport_destroy(void);

/*
 * WARNING
 *
 * This is a hack. We will move status to a proper place once we land
 * a multi-sessions support.
 */
static inline bool smbd_conn_good(struct smbd_work *work)
{
	return work->conn->status == SMBD_SESS_GOOD;
}

static inline bool smbd_conn_need_negotiate(struct smbd_work *work)
{
	return work->conn->status == SMBD_SESS_NEED_NEGOTIATE;
}

static inline bool smbd_conn_need_reconnect(struct smbd_work *work)
{
	return work->conn->status == SMBD_SESS_NEED_RECONNECT;
}

static inline bool smbd_conn_exiting(struct smbd_work *work)
{
	return work->conn->status == SMBD_SESS_EXITING;
}

static inline void smbd_conn_set_good(struct smbd_work *work)
{
	work->conn->status = SMBD_SESS_GOOD;
}

static inline void smbd_conn_set_need_negotiate(struct smbd_work *work)
{
	work->conn->status = SMBD_SESS_NEED_NEGOTIATE;
}

static inline void smbd_conn_set_need_reconnect(struct smbd_work *work)
{
	work->conn->status = SMBD_SESS_NEED_RECONNECT;
}

static inline void smbd_conn_set_exiting(struct smbd_work *work)
{
	work->conn->status = SMBD_SESS_EXITING;
}
#endif /* __CONNECTION_H__ */
