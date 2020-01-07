// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2016 Namjae Jeon <namjae.jeon@protocolfreedom.org>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/mutex.h>
#include <linux/freezer.h>
#include <linux/module.h>

#include "server.h"
#include "buffer_pool.h"
#include "smb_common.h"
#ifdef CONFIG_SMB_INSECURE_SERVER
#include "smb1pdu.h"
#endif
#include "mgmt/smbd_ida.h"
#include "connection.h"
#include "transport_tcp.h"
#include "transport_rdma.h"

static DEFINE_MUTEX(init_lock);

static struct smbd_conn_ops default_conn_ops;

static LIST_HEAD(conn_list);
static DEFINE_RWLOCK(conn_list_lock);

/**
 * smbd_conn_free() - free resources of the connection instance
 *
 * @conn:	connection instance to be cleand up
 *
 * During the thread termination, the corresponding conn instance
 * resources(sock/memory) are released and finally the conn object is freed.
 */
void smbd_conn_free(struct smbd_conn *conn)
{
	write_lock(&conn_list_lock);
	list_del(&conn->conns_list);
	write_unlock(&conn_list_lock);

	smbd_free_request(conn->request_buf);
	smbd_ida_free(conn->async_ida);
	kfree(conn->preauth_info);
	kfree(conn);
}

/**
 * smbd_conn_alloc() - initialize a new connection instance
 *
 * Return:	smbd_conn struct on success, otherwise NULL
 */
struct smbd_conn *smbd_conn_alloc(void)
{
	struct smbd_conn *conn;

	conn = kzalloc(sizeof(struct smbd_conn), GFP_KERNEL);
	if (!conn)
		return NULL;

	conn->need_neg = true;
	conn->status = SMBD_SESS_NEW;
	conn->local_nls = load_nls("utf8");
	if (!conn->local_nls)
		conn->local_nls = load_nls_default();
	atomic_set(&conn->req_running, 0);
	atomic_set(&conn->r_count, 0);
	init_waitqueue_head(&conn->req_running_q);
	INIT_LIST_HEAD(&conn->conns_list);
	INIT_LIST_HEAD(&conn->sessions);
	INIT_LIST_HEAD(&conn->requests);
	INIT_LIST_HEAD(&conn->async_requests);
	spin_lock_init(&conn->request_lock);
	spin_lock_init(&conn->credits_lock);
	conn->async_ida = smbd_ida_alloc();

	write_lock(&conn_list_lock);
	list_add(&conn->conns_list, &conn_list);
	write_unlock(&conn_list_lock);
	return conn;
}

bool smbd_conn_lookup_dialect(struct smbd_conn *c)
{
	struct smbd_conn *t;
	bool ret = false;

	read_lock(&conn_list_lock);
	list_for_each_entry(t, &conn_list, conns_list) {
		if (memcmp(t->ClientGUID, c->ClientGUID, SMB2_CLIENT_GUID_SIZE))
			continue;

		ret = true;
		break;
	}
	read_unlock(&conn_list_lock);
	return ret;
}

void smbd_conn_enqueue_request(struct smbd_work *work)
{
	struct smbd_conn *conn = work->conn;
	struct list_head *requests_queue = NULL;
#ifdef CONFIG_SMB_INSECURE_SERVER
	struct smb2_hdr *hdr = REQUEST_BUF(work);

	if (hdr->ProtocolId == SMB2_PROTO_NUMBER) {
		if (conn->ops->get_cmd_val(work) != SMB2_CANCEL_HE) {
			requests_queue = &conn->requests;
			work->syncronous = true;
		}
	} else {
		if (conn->ops->get_cmd_val(work) != SMB_COM_NT_CANCEL)
			requests_queue = &conn->requests;
	}
#else
	if (conn->ops->get_cmd_val(work) != SMB2_CANCEL_HE) {
		requests_queue = &conn->requests;
		work->syncronous = true;
	}
#endif

	if (requests_queue) {
		atomic_inc(&conn->req_running);
		spin_lock(&conn->request_lock);
		list_add_tail(&work->request_entry, requests_queue);
		spin_unlock(&conn->request_lock);
	}
}

int smbd_conn_try_dequeue_request(struct smbd_work *work)
{
	struct smbd_conn *conn = work->conn;
	int ret = 1;

	if (list_empty(&work->request_entry) &&
		list_empty(&work->async_request_entry))
		return 0;

	atomic_dec(&conn->req_running);
	spin_lock(&conn->request_lock);
	if (!work->multiRsp) {
		list_del_init(&work->request_entry);
		if (work->syncronous == false)
			list_del_init(&work->async_request_entry);
		ret = 0;
	}
	spin_unlock(&conn->request_lock);

	wake_up_all(&conn->req_running_q);
	return ret;
}

static void smbd_conn_lock(struct smbd_conn *conn)
{
	mutex_lock(&conn->srv_mutex);
}

static void smbd_conn_unlock(struct smbd_conn *conn)
{
	mutex_unlock(&conn->srv_mutex);
}

void smbd_conn_wait_idle(struct smbd_conn *conn)
{
	wait_event(conn->req_running_q, atomic_read(&conn->req_running) < 2);
}

int smbd_conn_write(struct smbd_work *work)
{
	struct smbd_conn *conn = work->conn;
	struct smb_hdr *rsp_hdr = RESPONSE_BUF(work);
	size_t len = 0;
	int sent;
	struct kvec iov[3];
	int iov_idx = 0;

	smbd_conn_try_dequeue_request(work);
	if (!rsp_hdr) {
		smbd_err("NULL response header\n");
		return -EINVAL;
	}

	if (HAS_TRANSFORM_BUF(work)) {
		iov[iov_idx] = (struct kvec) { work->tr_buf,
				sizeof(struct smb2_transform_hdr) };
		len += iov[iov_idx++].iov_len;
	}

	if (HAS_AUX_PAYLOAD(work)) {
		iov[iov_idx] = (struct kvec) { rsp_hdr, RESP_HDR_SIZE(work) };
		len += iov[iov_idx++].iov_len;
		iov[iov_idx] = (struct kvec) { AUX_PAYLOAD(work),
			AUX_PAYLOAD_SIZE(work) };
		len += iov[iov_idx++].iov_len;
	} else {
		if (HAS_TRANSFORM_BUF(work))
			iov[iov_idx].iov_len = RESP_HDR_SIZE(work);
		else
			iov[iov_idx].iov_len = get_rfc1002_len(rsp_hdr) + 4;
		iov[iov_idx].iov_base = rsp_hdr;
		len += iov[iov_idx++].iov_len;
	}

	smbd_conn_lock(conn);
	sent = conn->transport->ops->writev(conn->transport, &iov[0],
					iov_idx, len,
					work->need_invalidate_rkey,
					work->remote_key);
	smbd_conn_unlock(conn);

	if (sent < 0) {
		smbd_err("Failed to send message: %d\n", sent);
		return sent;
	}

	return 0;
}

int smbd_conn_rdma_read(struct smbd_conn *conn,
				void *buf, unsigned int buflen,
				u32 remote_key, u64 remote_offset,
				u32 remote_len)
{
	int ret = -EINVAL;

	if (conn->transport->ops->rdma_read)
		ret = conn->transport->ops->rdma_read(conn->transport,
						buf, buflen,
						remote_key, remote_offset,
						remote_len);
	return ret;
}

int smbd_conn_rdma_write(struct smbd_conn *conn,
				void *buf, unsigned int buflen,
				u32 remote_key, u64 remote_offset,
				u32 remote_len)
{
	int ret = -EINVAL;

	if (conn->transport->ops->rdma_write)
		ret = conn->transport->ops->rdma_write(conn->transport,
						buf, buflen,
						remote_key, remote_offset,
						remote_len);
	return ret;
}

bool smbd_conn_alive(struct smbd_conn *conn)
{
	if (!smbd_server_running())
		return false;

	if (conn->status == SMBD_SESS_EXITING)
		return false;

	if (kthread_should_stop())
		return false;

	if (atomic_read(&conn->stats.open_files_count) > 0)
		return true;

	/*
	 * Stop current session if the time that get last request from client
	 * is bigger than deadtime user configured and openning file count is
	 * zero.
	 */
	if (server_conf.deadtime > 0 &&
		time_after(jiffies, conn->last_active + server_conf.deadtime)) {
		smbd_debug("No response from client in %lu minutes\n",
			server_conf.deadtime);
		return false;
	}
	return true;
}

/**
 * smbd_conn_handler_loop() - session thread to listen on new smb requests
 * @p:		connection instance
 *
 * One thread each per connection
 *
 * Return:	0 on success
 */
int smbd_conn_handler_loop(void *p)
{
	struct smbd_conn *conn = (struct smbd_conn *)p;
	struct smbd_transport *t = conn->transport;
	unsigned int pdu_size;
	char hdr_buf[4] = {0,};
	int size;

	mutex_init(&conn->srv_mutex);
	__module_get(THIS_MODULE);

	if (t->ops->prepare && t->ops->prepare(t))
		goto out;

	conn->last_active = jiffies;
	while (smbd_conn_alive(conn)) {
		if (try_to_freeze())
			continue;

		smbd_free_request(conn->request_buf);
		conn->request_buf = NULL;

		size = t->ops->read(t, hdr_buf, sizeof(hdr_buf));
		if (size != sizeof(hdr_buf))
			break;

		pdu_size = get_rfc1002_len(hdr_buf);
		smbd_debug("RFC1002 header %u bytes\n", pdu_size);

		/* make sure we have enough to get to SMB header end */
		if (!smbd_pdu_size_has_room(pdu_size)) {
			smbd_debug("SMB request too short (%u bytes)\n",
				    pdu_size);
			continue;
		}

		/* 4 for rfc1002 length field */
		size = pdu_size + 4;
		conn->request_buf = smbd_alloc_request(size);
		if (!conn->request_buf)
			continue;

		memcpy(conn->request_buf, hdr_buf, sizeof(hdr_buf));
		if (!smbd_smb_request(conn))
			break;

		/*
		 * We already read 4 bytes to find out PDU size, now
		 * read in PDU
		 */
		size = t->ops->read(t, conn->request_buf + 4, pdu_size);
		if (size < 0) {
			smbd_err("sock_read failed: %d\n", size);
			break;
		}

		if (size != pdu_size) {
			smbd_err("PDU error. Read: %d, Expected: %d\n",
				  size,
				  pdu_size);
			continue;
		}

		if (!default_conn_ops.process_fn) {
			smbd_err("No connection request callback\n");
			break;
		}

		if (default_conn_ops.process_fn(conn)) {
			smbd_err("Cannot handle request\n");
			break;
		}
	}

out:
	/* Wait till all reference dropped to the Server object*/
	while (atomic_read(&conn->r_count) > 0)
		schedule_timeout(HZ);

	unload_nls(conn->local_nls);
	if (default_conn_ops.terminate_fn)
		default_conn_ops.terminate_fn(conn);
	t->ops->disconnect(t);
	module_put(THIS_MODULE);
	return 0;
}

void smbd_conn_init_server_callbacks(struct smbd_conn_ops *ops)
{
	default_conn_ops.process_fn = ops->process_fn;
	default_conn_ops.terminate_fn = ops->terminate_fn;
}

int smbd_conn_transport_init(void)
{
	int ret;

	mutex_lock(&init_lock);
	ret = smbd_tcp_init();
	if (ret) {
		pr_err("Failed to init TCP subsystem: %d\n", ret);
		goto out;
	}

	ret = smbd_rdma_init();
	if (ret) {
		pr_err("Failed to init SMBD subsystem: %d\n", ret);
		goto out;
	}
out:
	mutex_unlock(&init_lock);
	return ret;
}

static void stop_sessions(void)
{
	struct smbd_conn *conn;

again:
	read_lock(&conn_list_lock);
	list_for_each_entry(conn, &conn_list, conns_list) {
		struct task_struct *task;

		task = conn->transport->handler;
		if (task)
			smbd_debug("Stop session handler %s/%d\n",
				  task->comm,
				  task_pid_nr(task));
		conn->status = SMBD_SESS_EXITING;
	}
	read_unlock(&conn_list_lock);

	if (!list_empty(&conn_list)) {
		schedule_timeout_interruptible(HZ/10); /* 100ms */
		goto again;
	}
}

void smbd_conn_transport_destroy(void)
{
	mutex_lock(&init_lock);
	smbd_tcp_destroy();
	smbd_rdma_destroy();
	stop_sessions();
	mutex_unlock(&init_lock);
}
