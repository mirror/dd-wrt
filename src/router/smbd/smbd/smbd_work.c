// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 */

#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "server.h"
#include "connection.h"
#include "smbd_work.h"
#include "buffer_pool.h"
#include "mgmt/smbd_ida.h"

/* @FIXME */
#include "smbd_server.h"

static struct kmem_cache *work_cache;
static struct workqueue_struct *smbd_wq;

struct smbd_work *smbd_alloc_work_struct(void)
{
	struct smbd_work *work = kmem_cache_zalloc(work_cache, GFP_KERNEL);

	if (work) {
		INIT_LIST_HEAD(&work->request_entry);
		INIT_LIST_HEAD(&work->async_request_entry);
		INIT_LIST_HEAD(&work->fp_entry);
		INIT_LIST_HEAD(&work->interim_entry);
	}
	return work;
}

void smbd_free_work_struct(struct smbd_work *work)
{
	if (server_conf.flags & SMBD_GLOBAL_FLAG_CACHE_TBUF)
		smbd_release_buffer(RESPONSE_BUF(work));
	else
		smbd_free_response(RESPONSE_BUF(work));

	if (server_conf.flags & SMBD_GLOBAL_FLAG_CACHE_RBUF)
		smbd_release_buffer(AUX_PAYLOAD(work));
	else
		smbd_free_response(AUX_PAYLOAD(work));

	smbd_free_response(TRANSFORM_BUF(work));
	smbd_free_request(REQUEST_BUF(work));
	if (work->async_id)
		smbd_release_id(work->conn->async_ida, work->async_id);
	kmem_cache_free(work_cache, work);
}

void smbd_work_pool_destroy(void)
{
	kmem_cache_destroy(work_cache);
}

int smbd_work_pool_init(void)
{
	work_cache = kmem_cache_create("smbd_work_cache",
					sizeof(struct smbd_work), 0,
					SLAB_HWCACHE_ALIGN, NULL);
	if (!work_cache)
		return -EINVAL;
	return 0;
}

int smbd_workqueue_init(void)
{
	smbd_wq = alloc_workqueue("ksmbd-io", 0, 0);
	if (!smbd_wq)
		return -EINVAL;
	return 0;
}

void smbd_workqueue_destroy(void)
{
	flush_workqueue(smbd_wq);
	destroy_workqueue(smbd_wq);
	smbd_wq = NULL;
}

bool smbd_queue_work(struct smbd_work *work)
{
	return queue_work(smbd_wq, &work->work);
}
