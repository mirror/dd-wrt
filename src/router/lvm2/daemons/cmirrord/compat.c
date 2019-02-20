/*
 * Copyright (C) 2010 Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */
#include "logging.h"
#include "cluster.h"
#include "compat.h"
#include "lib/mm/xlate.h"

#include <errno.h>

/*
 * Older versions of the log daemon communicate with different
 * versions of the inter-machine communication structure, which
 * varies in size and fields.  The older versions append the
 * standard upstream version of the structure to every request.
 * COMPAT_OFFSET is where the upstream structure starts.
 */
#define COMPAT_OFFSET 256

static void v5_data_endian_switch(struct clog_request *rq, int to_network __attribute__((unused)))
{
	int i, end;
	int64_t *pi64;
	uint64_t *pu64;
	uint32_t rq_type = rq->u_rq.request_type & ~DM_ULOG_RESPONSE;

	if (rq->u_rq.request_type & DM_ULOG_RESPONSE) {
		switch (rq_type) {
		case DM_ULOG_CTR:
		case DM_ULOG_DTR:
			LOG_ERROR("Invalid response type in endian switch");
			exit(EXIT_FAILURE);

		case DM_ULOG_PRESUSPEND:
		case DM_ULOG_POSTSUSPEND:
		case DM_ULOG_RESUME:
		case DM_ULOG_FLUSH:
		case DM_ULOG_MARK_REGION:
		case DM_ULOG_CLEAR_REGION:
		case DM_ULOG_SET_REGION_SYNC:
		case DM_ULOG_CHECKPOINT_READY:
		case DM_ULOG_MEMBER_JOIN:
		case DM_ULOG_STATUS_INFO:
		case DM_ULOG_STATUS_TABLE:
			/* No outbound data */
			break;

		case DM_ULOG_GET_REGION_SIZE:
		case DM_ULOG_GET_SYNC_COUNT:
			pu64 = (uint64_t *)rq->u_rq.data;
			*pu64 = xlate64(*pu64);
			break;
		case DM_ULOG_IS_CLEAN:
		case DM_ULOG_IN_SYNC:
			pi64 = (int64_t *)rq->u_rq.data;
			*pi64 = xlate64(*pi64);
			break;
		case DM_ULOG_GET_RESYNC_WORK:
		case DM_ULOG_IS_REMOTE_RECOVERING:
			pi64 = (int64_t *)rq->u_rq.data;
			pu64 = ((uint64_t *)rq->u_rq.data) + 1;
			*pi64 = xlate64(*pi64);
			*pu64 = xlate64(*pu64);
			break;
		default:
			LOG_ERROR("Unknown request type, %u", rq_type);
			return;
		}
	} else {
		switch (rq_type) {
		case DM_ULOG_CTR:
		case DM_ULOG_DTR:
			LOG_ERROR("Invalid request type in endian switch");
			exit(EXIT_FAILURE);

		case DM_ULOG_PRESUSPEND:
		case DM_ULOG_POSTSUSPEND:
		case DM_ULOG_RESUME:
		case DM_ULOG_GET_REGION_SIZE:
		case DM_ULOG_FLUSH:
		case DM_ULOG_GET_RESYNC_WORK:
		case DM_ULOG_GET_SYNC_COUNT:
		case DM_ULOG_STATUS_INFO:
		case DM_ULOG_STATUS_TABLE:
		case DM_ULOG_CHECKPOINT_READY:
		case DM_ULOG_MEMBER_JOIN:
			/* No incoming data */
			break;
		case DM_ULOG_IS_CLEAN:
		case DM_ULOG_IN_SYNC:
		case DM_ULOG_IS_REMOTE_RECOVERING:
			pu64 = (uint64_t *)rq->u_rq.data;
			*pu64 = xlate64(*pu64);
			break;
		case DM_ULOG_MARK_REGION:
		case DM_ULOG_CLEAR_REGION:
			end = rq->u_rq.data_size/sizeof(uint64_t);

			pu64 = (uint64_t *)rq->u_rq.data;
			for (i = 0; i < end; i++)
				pu64[i] = xlate64(pu64[i]);
			break;
		case DM_ULOG_SET_REGION_SYNC:
			pu64 = (uint64_t *)rq->u_rq.data;
			pi64 = ((int64_t *)rq->u_rq.data) + 1;
			*pu64 = xlate64(*pu64);
			*pi64 = xlate64(*pi64);
			break;
		default:
			LOG_ERROR("Unknown request type, %u", rq_type);
			exit(EXIT_FAILURE);
		}
	}
}

static int v5_endian_to_network(struct clog_request *rq)
{
	int size;
	struct dm_ulog_request *u_rq = &rq->u_rq;

	size = sizeof(*rq) + u_rq->data_size;

	u_rq->error = xlate32(u_rq->error);
	u_rq->seq = xlate32(u_rq->seq);

	rq->originator = xlate32(rq->originator);

	v5_data_endian_switch(rq, 1);

	u_rq->request_type = xlate32(u_rq->request_type);
	u_rq->data_size = xlate32(u_rq->data_size);

	return size;
}

int clog_request_to_network(struct clog_request *rq)
{
	int r;

	/* FIXME: Remove this safety check */
	if (rq->u.version[0] != xlate64(rq->u.version[1])) {
		LOG_ERROR("Programmer error:  version[0] must be LE");
		exit(EXIT_FAILURE);
	}

	/*
	 * Are we already running in the endian mode we send
	 * over the wire?
	 */
	if (rq->u.version[0] == rq->u.version[1])
		return 0;

	r = v5_endian_to_network(rq);
	if (r < 0)
		return r;
	return 0;
}

static int v5_endian_from_network(struct clog_request *rq)
{
	int size;
	struct dm_ulog_request *u_rq = &rq->u_rq;

	u_rq->error = xlate32(u_rq->error);
	u_rq->seq = xlate32(u_rq->seq);
	u_rq->request_type = xlate32(u_rq->request_type);
	u_rq->data_size = xlate32(u_rq->data_size);

	rq->originator = xlate32(rq->originator);

	size = sizeof(*rq) + u_rq->data_size;

	v5_data_endian_switch(rq, 0);

	return size;
}

int clog_request_from_network(void *data, size_t data_len)
{
	uint64_t *vp = data;
	uint64_t version = xlate64(vp[0]);
	struct clog_request *rq = data;

	switch (version) {
	case 5: /* Upstream */
		if (version == vp[0])
			return 0;
		break;
	case 4: /* RHEL 5.[45] */
	case 3: /* RHEL 5.3 */
	case 2: /* RHEL 5.2 */
		/* FIXME: still need to account for payload */
		if (data_len < (COMPAT_OFFSET + sizeof(*rq)))
			return -ENOSPC;

		rq = (struct clog_request *)((char *)data + COMPAT_OFFSET);
		break;
	default:
		LOG_ERROR("Unable to process cluster message: "
			  "Incompatible version");
		return -EINVAL;
	}

	v5_endian_from_network(rq);
	return 0;
}
