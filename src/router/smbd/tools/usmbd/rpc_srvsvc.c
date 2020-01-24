// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <memory.h>
#include <endian.h>
#include <errno.h>
#include <linux/usmbd_server.h>

#include <management/share.h>

#include <rpc.h>
#include <rpc_srvsvc.h>
#include <usmbdtools.h>

#define SHARE_TYPE_TEMP			0x40000000
#define SHARE_TYPE_HIDDEN		0x80000000

#define SHARE_TYPE_DISKTREE		0
#define SHARE_TYPE_DISKTREE_TEMP	(SHARE_TYPE_DISKTREE|SHARE_TYPE_TEMP)
#define SHARE_TYPE_DISKTREE_HIDDEN	(SHARE_TYPE_DISKTREE|SHARE_TYPE_HIDDEN)
#define SHARE_TYPE_PRINTQ		1
#define SHARE_TYPE_PRINTQ_TEMP		(SHARE_TYPE_PRINTQ|SHARE_TYPE_TEMP)
#define SHARE_TYPE_PRINTQ_HIDDEN	(SHARE_TYPE_PRINTQ|SHARE_TYPE_HIDDEN)
#define SHARE_TYPE_DEVICE		2
#define SHARE_TYPE_DEVICE_TEMP		(SHARE_TYPE_DEVICE|SHARE_TYPE_TEMP)
#define SHARE_TYPE_DEVICE_HIDDEN	(SHARE_TYPE_DEVICE|SHARE_TYPE_HIDDEN)
#define SHARE_TYPE_IPC			3
#define SHARE_TYPE_IPC_TEMP		(SHARE_TYPE_IPC|SHARE_TYPE_TEMP)
#define SHARE_TYPE_IPC_HIDDEN		(SHARE_TYPE_IPC|SHARE_TYPE_HIDDEN)

#define SRVSVC_OPNUM_SHARE_ENUM_ALL	15
#define SRVSVC_OPNUM_GET_SHARE_INFO	16

static int __share_type(struct usmbd_share *share)
{
	if (test_share_flag(share, USMBD_SHARE_FLAG_PIPE))
		return SHARE_TYPE_IPC;
	if (!strncasecmp(share->name, "IPC", strlen("IPC")))
		return SHARE_TYPE_IPC;
	return SHARE_TYPE_DISKTREE;
}

static int __share_entry_size_ctr0(struct usmbd_dcerpc *dce, void *entry)
{
	struct usmbd_share *share = entry;

	return strlen(share->name) * 2 + 4 * sizeof(__u32);
}

static int __share_entry_size_ctr1(struct usmbd_dcerpc *dce, void *entry)
{
	struct usmbd_share *share = entry;
	int sz = 0;

	sz = strlen(share->name) * 2;
	if (share->comment)
		sz += strlen(share->comment) * 2;

	sz += 9 * sizeof(__u32);
	return sz;
}

/*
 * Embedded Reference Pointers
 *
 * An embedded reference pointer is represented in two parts, a 4 octet
 * value in place and a possibly deferred representation of the referent.
 */
static int __share_entry_rep_ctr0(struct usmbd_dcerpc *dce, void *entry)
{
	dce->num_pointers++;
	return ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
}

static int __share_entry_rep_ctr1(struct usmbd_dcerpc *dce, void *entry)
{
	struct usmbd_share *share = entry;
	int ret;

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
	ret |= ndr_write_int32(dce, __share_type(share));
	dce->num_pointers++;
	ret |= ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
	return ret;
}

static int __share_entry_data_ctr0(struct usmbd_dcerpc *dce, void *entry)
{
	struct usmbd_share *share = entry;

	return ndr_write_vstring(dce, share->name);
}

static int __share_entry_data_ctr1(struct usmbd_dcerpc *dce, void *entry)
{
	struct usmbd_share *share = entry;
	int ret;

	ret = ndr_write_vstring(dce, share->name);
	ret |= ndr_write_vstring(dce, share->comment);
	return ret;
}

static int __share_entry_null_rep_ctr0(struct usmbd_dcerpc *dce,
				       void *entry)
{
	return ndr_write_int32(dce, 0); /* ref pointer */
}

static int __share_entry_null_rep_ctr1(struct usmbd_dcerpc *dce,
				       void *entry)
{
	int ret;

	ret = ndr_write_int32(dce, 0); /* ref pointer */
	ret |= ndr_write_int32(dce, 0);
	ret |= ndr_write_int32(dce, 0); /* ref pointer */
	return ret;
}

static int __share_entry_processed(struct usmbd_rpc_pipe *pipe, int i)
{
	struct usmbd_share *share;

	share = list_get(&pipe->entries,  i);

	list_remove_dec(&pipe->entries, i);
	pipe->num_entries--;
	pipe->num_processed++;
	put_usmbd_share(share);

	return 0;
}

static void __enum_all_shares(void *value, unsigned long long id, void *user_data)
{
	struct usmbd_rpc_pipe *pipe = (struct usmbd_rpc_pipe *)user_data;
	struct usmbd_share *share = (struct usmbd_share *)value;

	if (!get_usmbd_share(share))
		return;

	if (!test_share_flag(share, USMBD_SHARE_FLAG_BROWSEABLE)) {
		put_usmbd_share(share);
		return;
	}

	if (!test_share_flag(share, USMBD_SHARE_FLAG_AVAILABLE)) {
		put_usmbd_share(share);
		return;
	}
	list_append(&pipe->entries, share);
	pipe->num_entries++;
}

static int srvsvc_share_enum_all_invoke(struct usmbd_rpc_pipe *pipe)
{
	for_each_usmbd_share(__enum_all_shares, pipe);
	pipe->entry_processed = __share_entry_processed;
	return 0;
}

static int srvsvc_share_get_info_invoke(struct usmbd_rpc_pipe *pipe,
					struct srvsvc_share_info_request *hdr)
{
	struct usmbd_share *share;
	int ret;

	share = shm_lookup_share(STR_VAL(hdr->share_name));
	if (!share)
		return 0;

	if (!test_share_flag(share, USMBD_SHARE_FLAG_AVAILABLE)) {
		put_usmbd_share(share);
		return 0;
	}

	ret = shm_lookup_hosts_map(share,
				   USMBD_SHARE_HOSTS_ALLOW_MAP,
				   STR_VAL(hdr->server_name));
	if (ret == -ENOENT) {
		put_usmbd_share(share);
		return 0;
	}

	if (ret != 0) {
		ret = shm_lookup_hosts_map(share,
					   USMBD_SHARE_HOSTS_DENY_MAP,
					   STR_VAL(hdr->server_name));
		if (ret == 0) {
			put_usmbd_share(share);
			return 0;
		}
	}

	list_append(&pipe->entries, share);
	pipe->num_entries++;
	pipe->entry_processed = __share_entry_processed;
	return 0;
}

static int srvsvc_share_enum_all_return(struct usmbd_rpc_pipe *pipe)
{
	struct usmbd_dcerpc *dce = pipe->dce;
	int status = USMBD_RPC_OK;

	ndr_write_union_int32(dce, dce->si_req.level);

	status = ndr_write_array_of_structs(pipe);
	/*
	 * [out] DWORD* TotalEntries
	 * [out, unique] DWORD* ResumeHandle
	 */
	ndr_write_int32(dce, pipe->num_processed);

	if (status == USMBD_RPC_EMORE_DATA) {
		dce->num_pointers++;
		ndr_write_int32(dce, dce->num_pointers);
		ndr_write_int32(dce, 0x01);
		/* Have pending data, set RETURN_READY again */
		dce->flags |= USMBD_DCERPC_RETURN_READY;
	} else {
		dce->num_pointers++;
		ndr_write_int32(dce, dce->num_pointers);
		ndr_write_int32(dce, 0);
	}
	return status;
}

static int srvsvc_share_get_info_return(struct usmbd_rpc_pipe *pipe)
{
	struct usmbd_dcerpc *dce = pipe->dce;

	ndr_write_union_int32(dce, dce->si_req.level);
	if (pipe->num_entries)
		return __ndr_write_array_of_structs(pipe, pipe->num_entries);

	/*
	 * No data. Either we didn't find the requested net share,
	 * or we didn't even lookup it due to restricted context
	 * rule. In any case, return a zero representation of the
	 * corresponding share_info level.
	 */
	if (dce->si_req.level == 0) {
		dce->entry_rep = __share_entry_null_rep_ctr0;
	} else if (dce->si_req.level == 1) {
		dce->entry_rep = __share_entry_null_rep_ctr1;
	} else {
		pr_err("Unsupported share info level (read): %d\n",
			dce->si_req.level);
		dce->entry_rep = NULL;
		return USMBD_RPC_EINVALID_LEVEL;
	}

	dce->entry_rep(dce, NULL);

	if (!rpc_restricted_context(dce->rpc_req))
		return USMBD_RPC_EINVALID_PARAMETER;
	return USMBD_RPC_EACCESS_DENIED;
}

static int srvsvc_parse_share_info_req(struct usmbd_dcerpc *dce,
				       struct srvsvc_share_info_request *hdr)
{
	ndr_read_uniq_vsting_ptr(dce, &hdr->server_name);

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_SHARE_ENUM_ALL) {
		int ptr;

		/* Read union switch selector */
		hdr->level = ndr_read_union_int32(dce);
		if (hdr->level == -EINVAL)
			return -EINVAL;
		ndr_read_int32(dce); // read container pointer ref id
		ndr_read_int32(dce); // read container array size
		ptr = ndr_read_int32(dce); // read container array pointer
					   // it should be null
		if (ptr != 0x00) {
			pr_err("SRVSVC: container array pointer is %x\n",
				ptr);
			return -EINVAL;
		}
		hdr->max_size = ndr_read_int32(dce);
		ndr_read_uniq_ptr(dce, &hdr->payload_handle);
		return 0;
	}

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO) {
		ndr_read_vstring_ptr(dce, &hdr->share_name);
		hdr->level = ndr_read_int32(dce);
		return 0;
	}

	return -ENOTSUP;
}

static int srvsvc_share_info_invoke(struct usmbd_rpc_pipe *pipe)
{
	struct usmbd_dcerpc *dce = pipe->dce;
	int ret = USMBD_RPC_ENOTIMPLEMENTED;

	if (srvsvc_parse_share_info_req(dce, &dce->si_req))
		return USMBD_RPC_EBAD_DATA;

	pipe->entry_processed = __share_entry_processed;

	if (rpc_restricted_context(dce->rpc_req))
		return 0;

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO)
		ret = srvsvc_share_get_info_invoke(pipe, &dce->si_req);
	if (dce->req_hdr.opnum == SRVSVC_OPNUM_SHARE_ENUM_ALL)
		ret = srvsvc_share_enum_all_invoke(pipe);
	return ret;
}

static int srvsvc_clear_headers(struct usmbd_rpc_pipe *pipe,
				int status)
{
	if (status == USMBD_RPC_EMORE_DATA)
		return 0;

	ndr_free_uniq_vsting_ptr(&pipe->dce->si_req.server_name);
	if (pipe->dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO) {
		ndr_free_vstring_ptr(&pipe->dce->si_req.share_name);
	}

	return 0;
}

static int srvsvc_share_info_return(struct usmbd_rpc_pipe *pipe)
{
	struct usmbd_dcerpc *dce = pipe->dce;
	int ret = USMBD_RPC_OK, status;

	/*
	 * Reserve space for response NDR header. We don't know yet if
	 * the payload buffer is big enough. This will determine if we
	 * can set DCERPC_PFC_FIRST_FRAG|DCERPC_PFC_LAST_FRAG or if we
	 * will have a multi-part response.
	 */
	dce->offset = sizeof(struct dcerpc_header);
	dce->offset += sizeof(struct dcerpc_response_header);
	pipe->num_processed = 0;

	if (dce->si_req.level == 0) {
		dce->entry_size = __share_entry_size_ctr0;
		dce->entry_rep = __share_entry_rep_ctr0;
		dce->entry_data = __share_entry_data_ctr0;
	} else if (dce->si_req.level == 1) {
		dce->entry_size = __share_entry_size_ctr1;
		dce->entry_rep = __share_entry_rep_ctr1;
		dce->entry_data = __share_entry_data_ctr1;
	} else {
		pr_err("Unsupported share info level (write): %d\n",
			dce->si_req.level);
		rpc_pipe_reset(pipe);
	}

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO) {
		status = srvsvc_share_get_info_return(pipe);
	}
	if (dce->req_hdr.opnum == SRVSVC_OPNUM_SHARE_ENUM_ALL) {
		status = srvsvc_share_enum_all_return(pipe);
	}

	if (rpc_restricted_context(dce->rpc_req))
		status = USMBD_RPC_EACCESS_DENIED;

	//srvsvc_clear_headers(pipe, status);

	/*
	 * [out] DWORD Return value/code
	 */
	if (ret != USMBD_RPC_OK)
		status = ret;

	ndr_write_int32(dce, status);
	dcerpc_write_headers(dce, status);

	dce->rpc_resp->payload_sz = dce->offset;
	return ret;
}

static int srvsvc_invoke(struct usmbd_rpc_pipe *pipe)
{
	int ret = USMBD_RPC_ENOTIMPLEMENTED;

	switch (pipe->dce->req_hdr.opnum) {
	case SRVSVC_OPNUM_SHARE_ENUM_ALL:
	case SRVSVC_OPNUM_GET_SHARE_INFO:
		ret = srvsvc_share_info_invoke(pipe);
		break;
	default:
		pr_err("SRVSVC: unsupported INVOKE method %d\n",
		       pipe->dce->req_hdr.opnum);
		break;
	}

	return ret;
}

static int srvsvc_return(struct usmbd_rpc_pipe *pipe,
			 struct usmbd_rpc_command *resp,
			 int max_resp_sz)
{
	struct usmbd_dcerpc *dce = pipe->dce;
	int ret;

	switch (dce->req_hdr.opnum) {
	case SRVSVC_OPNUM_SHARE_ENUM_ALL:
		if (dce->si_req.max_size < (unsigned int)max_resp_sz)
			max_resp_sz = dce->si_req.max_size;
		/* Fall through */
	case SRVSVC_OPNUM_GET_SHARE_INFO:
		dcerpc_set_ext_payload(dce, resp->payload, max_resp_sz);

		ret = srvsvc_share_info_return(pipe);
		break;
	default:
		pr_err("SRVSVC: unsupported RETURN method %d\n",
			dce->req_hdr.opnum);
		ret = USMBD_RPC_EBAD_FUNC;
		break;
	}
	return ret;
}

int rpc_srvsvc_read_request(struct usmbd_rpc_pipe *pipe,
			    struct usmbd_rpc_command *resp,
			    int max_resp_sz)
{
	return srvsvc_return(pipe, resp, max_resp_sz);
}

int rpc_srvsvc_write_request(struct usmbd_rpc_pipe *pipe)
{
	return srvsvc_invoke(pipe);
}
