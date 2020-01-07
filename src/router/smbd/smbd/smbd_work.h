/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_WORK_H__
#define __SMBD_WORK_H__

#include <linux/ctype.h>
#include <linux/workqueue.h>

struct smbd_conn;
struct smbd_session;
struct smbd_tree_connect;

enum {
	SMBD_WORK_ACTIVE = 0,
	SMBD_WORK_CANCELLED,
	SMBD_WORK_CLOSED,
};

/* one of these for every pending CIFS request at the connection */
struct smbd_work {
	/* Server corresponding to this mid */
	struct smbd_conn               *conn;
	struct smbd_session            *sess;
	struct smbd_tree_connect       *tcon;

	/* Pointer to received SMB header */
	char                            *request_buf;
	/* Response buffer */
	char                            *response_buf;

	/* Read data buffer */
	char                            *aux_payload_buf;

	/* Next cmd hdr in compound req buf*/
	int                             next_smb2_rcv_hdr_off;
	/* Next cmd hdr in compound rsp buf*/
	int                             next_smb2_rsp_hdr_off;

	/*
	 * Current Local FID assigned compound response if SMB2 CREATE
	 * command is present in compound request
	 */
	unsigned int                    compound_fid;
	unsigned int                    compound_pfid;
	unsigned int                    compound_sid;

	/* response smb header size */
	unsigned int                    resp_hdr_sz;
	unsigned int                    response_sz;
	/* Read data count */
	unsigned int                    aux_payload_sz;

	void				*tr_buf;

	unsigned char			state;
	/* Multiple responses for one request e.g. SMB ECHO */
	bool                            multiRsp:1;
	/* No response for cancelled request */
	bool                            send_no_response:1;
	/* Request is encrypted */
	bool                            encrypted:1;
	/* Is this SYNC or ASYNC smbd_work */
	bool                            syncronous:1;
	bool                            need_invalidate_rkey:1;

	unsigned int                    remote_key;
	/* cancel works */
	int                             async_id;
	void                            **cancel_argv;
	void                            (*cancel_fn)(void **argv);

	struct work_struct              work;
	/* List head at conn->requests */
	struct list_head                request_entry;
	/* List head at conn->async_requests */
	struct list_head                async_request_entry;
	struct list_head                fp_entry;
	struct list_head                interim_entry;
};

#define WORK_CANCELLED(w)	((w)->state == SMBD_WORK_CANCELLED)
#define WORK_CLOSED(w)		((w)->state == SMBD_WORK_CLOSED)
#define WORK_ACTIVE(w)		((w)->state == SMBD_WORK_ACTIVE)

#define RESPONSE_BUF(w)		((void *)(w)->response_buf)
#define REQUEST_BUF(w)		((void *)(w)->request_buf)

#define RESPONSE_BUF_NEXT(w)	\
	((void *)((w)->response_buf + (w)->next_smb2_rsp_hdr_off))
#define REQUEST_BUF_NEXT(w)	\
	((void *)((w)->request_buf + (w)->next_smb2_rcv_hdr_off))

#define RESPONSE_SZ(w)		((w)->response_sz)

#define INIT_AUX_PAYLOAD(w)	((w)->aux_payload_buf = NULL)
#define HAS_AUX_PAYLOAD(w)	((w)->aux_payload_sz != 0)
#define AUX_PAYLOAD(w)		(void *)((w)->aux_payload_buf)
#define AUX_PAYLOAD_SIZE(w)	((w)->aux_payload_sz)
#define RESP_HDR_SIZE(w)	((w)->resp_hdr_sz)

#define HAS_TRANSFORM_BUF(w)	((w)->tr_buf != NULL)
#define TRANSFORM_BUF(w)	(void *)((w)->tr_buf)

struct smbd_work *smbd_alloc_work_struct(void);
void smbd_free_work_struct(struct smbd_work *work);

void smbd_work_pool_destroy(void);
int smbd_work_pool_init(void);

int smbd_workqueue_init(void);
void smbd_workqueue_destroy(void);
bool smbd_queue_work(struct smbd_work *work);

#endif /* __SMBD_WORK_H__ */
