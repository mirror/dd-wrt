// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <memory.h>
#include <endian.h>
#include <glib.h>
#include <errno.h>
#include <linux/ksmbd_server.h>

#include <management/share.h>

#include <rpc.h>
#include <rpc_srvsvc.h>
#include <tools.h>

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

static int __share_type(struct ksmbd_share *share)
{
	if (test_share_flag(share, KSMBD_SHARE_FLAG_PIPE))
		return SHARE_TYPE_IPC;
	if (!g_ascii_strncasecmp(share->name, "IPC", strlen("IPC")))
		return SHARE_TYPE_IPC;
	return SHARE_TYPE_DISKTREE;
}

static int __share_entry_size_ctr0(struct ksmbd_dcerpc *dce, gpointer entry)
{
	struct ksmbd_share *share = entry;

	return strlen(share->name) * 2 + 4 * sizeof(__u32);
}

static int __share_entry_size_ctr1(struct ksmbd_dcerpc *dce, gpointer entry)
{
	struct ksmbd_share *share = entry;
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
static int __share_entry_rep_ctr0(struct ksmbd_dcerpc *dce, gpointer entry)
{
	dce->num_pointers++;
	return ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
}

static int __share_entry_rep_ctr1(struct ksmbd_dcerpc *dce, gpointer entry)
{
	struct ksmbd_share *share = entry;
	int ret;

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, __share_type(share));
	if (ret)
		return ret;

	dce->num_pointers++;
	return ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
}

static int __share_entry_data_ctr0(struct ksmbd_dcerpc *dce, gpointer entry)
{
	struct ksmbd_share *share = entry;

	return ndr_write_vstring(dce, share->name);
}

static int __share_entry_data_ctr1(struct ksmbd_dcerpc *dce, gpointer entry)
{
	struct ksmbd_share *share = entry;
	int ret;

	ret = ndr_write_vstring(dce, share->name);
	if (ret)
		return ret;

	return ndr_write_vstring(dce, share->comment);
}

static int __share_entry_null_rep_ctr0(struct ksmbd_dcerpc *dce,
				       gpointer entry)
{
	return ndr_write_int32(dce, 0); /* ref pointer */
}

static int __share_entry_null_rep_ctr1(struct ksmbd_dcerpc *dce,
				       gpointer entry)
{
	int ret;

	ret = ndr_write_int32(dce, 0); /* ref pointer */
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, 0);
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, 0); /* ref pointer */

	return ret;
}

static int __share_entry_processed(struct ksmbd_rpc_pipe *pipe, int i)
{
	struct ksmbd_share *share;

	share = g_ptr_array_remove_index(pipe->entries, i);
	pipe->num_entries--;
	pipe->num_processed++;
	put_ksmbd_share(share);

	return 0;
}

static void __enum_all_shares(struct ksmbd_share *share,
			      struct ksmbd_rpc_pipe *pipe)
{
	if (!get_ksmbd_share(share))
		return;

	if (!test_share_flag(share, KSMBD_SHARE_FLAG_BROWSEABLE)) {
		put_ksmbd_share(share);
		return;
	}

	if (!test_share_flag(share, KSMBD_SHARE_FLAG_AVAILABLE)) {
		put_ksmbd_share(share);
		return;
	}

	g_ptr_array_add(pipe->entries, share);
	pipe->num_entries++;
}

static int srvsvc_share_enum_all_invoke(struct ksmbd_rpc_pipe *pipe)
{
	shm_iter_shares((share_cb)__enum_all_shares, pipe);
	pipe->entry_processed = __share_entry_processed;
	return 0;
}

static int srvsvc_share_get_info_invoke(struct ksmbd_rpc_pipe *pipe,
					struct srvsvc_share_info_request *hdr)
{
	struct ksmbd_share *share;
	int ret;

	share = shm_lookup_share(STR_VAL(hdr->share_name));
	if (!share)
		return 0;

	if (!test_share_flag(share, KSMBD_SHARE_FLAG_AVAILABLE)) {
		put_ksmbd_share(share);
		return 0;
	}

	ret = shm_lookup_hosts_map(share,
				   KSMBD_SHARE_HOSTS_ALLOW_MAP,
				   STR_VAL(hdr->server_name));
	if (ret == -ENOENT) {
		put_ksmbd_share(share);
		return 0;
	}
	if (ret) {
		ret = shm_lookup_hosts_map(share,
					   KSMBD_SHARE_HOSTS_DENY_MAP,
					   STR_VAL(hdr->server_name));
		if (!ret) {
			put_ksmbd_share(share);
			return 0;
		}
	}

	g_ptr_array_add(pipe->entries, share);
	pipe->num_entries++;
	pipe->entry_processed = __share_entry_processed;
	return 0;
}

static int srvsvc_share_enum_all_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int status = KSMBD_RPC_OK;

	if (ndr_write_union_int32(dce, dce->si_req.level))
		return KSMBD_RPC_EBAD_DATA;

	status = ndr_write_array_of_structs(pipe);
	if (status == KSMBD_RPC_EBAD_DATA)
		return status;
	/*
	 * [out] DWORD* TotalEntries
	 * [out, unique] DWORD* ResumeHandle
	 */
	if (ndr_write_int32(dce, pipe->num_processed))
		return KSMBD_RPC_EBAD_DATA;

	if (status == KSMBD_RPC_EMORE_DATA) {
		dce->num_pointers++;
		if (ndr_write_int32(dce, dce->num_pointers))
			return KSMBD_RPC_EBAD_DATA;
		if (ndr_write_int32(dce, 0x01))
			return KSMBD_RPC_EBAD_DATA;
		/* Have pending data, set RETURN_READY again */
		dce->flags |= KSMBD_DCERPC_RETURN_READY;
	} else {
		dce->num_pointers++;
		if (ndr_write_int32(dce, dce->num_pointers))
			return KSMBD_RPC_EBAD_DATA;
		if (ndr_write_int32(dce, 0))
			return KSMBD_RPC_EBAD_DATA;
	}

	return status;
}

static int srvsvc_share_get_info_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_write_union_int32(dce, dce->si_req.level))
		return KSMBD_RPC_EBAD_DATA;

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
		return KSMBD_RPC_EINVALID_LEVEL;
	}

	if (dce->entry_rep(dce, NULL))
		return KSMBD_RPC_EBAD_DATA;

	if (!rpc_restricted_context(dce->rpc_req))
		return KSMBD_RPC_EINVALID_PARAMETER;
	return KSMBD_RPC_EACCESS_DENIED;
}

static int srvsvc_parse_share_info_req(struct ksmbd_dcerpc *dce,
				       struct srvsvc_share_info_request *hdr)
{
	if (ndr_read_uniq_vstring_ptr(dce, &hdr->server_name))
		return -EINVAL;

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_SHARE_ENUM_ALL) {
		int ptr;
		__u32 val;

		/* Read union switch selector */
		if (ndr_read_union_int32(dce, &val))
			return -EINVAL;
		hdr->level = val;
		// read container pointer ref id
		if (ndr_read_int32(dce, NULL))
			return -EINVAL;
		// read container array size
		if (ndr_read_int32(dce, NULL))
			return -EINVAL;
		// read container array pointer
		if (ndr_read_int32(dce, &ptr))
			return -EINVAL;
		// it should be null
		if (ptr != 0x00) {
			pr_err("SRVSVC: container array pointer is %x\n",
				ptr);
			return -EINVAL;
		}
		if (ndr_read_int32(dce, &val))
			return -EINVAL;
		hdr->max_size = val;
		if (ndr_read_uniq_ptr(dce, &hdr->payload_handle))
			return -EINVAL;
		return 0;
	}

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO) {
		if (ndr_read_vstring_ptr(dce, &hdr->share_name))
			return -EINVAL;
		if (ndr_read_int32(dce, &hdr->level))
			return -EINVAL;
		return 0;
	}

	return -ENOTSUP;
}

static int srvsvc_share_info_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;

	if (srvsvc_parse_share_info_req(dce, &dce->si_req))
		return KSMBD_RPC_EBAD_DATA;

	pipe->entry_processed = __share_entry_processed;

	if (rpc_restricted_context(dce->rpc_req))
		return 0;

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO)
		ret = srvsvc_share_get_info_invoke(pipe, &dce->si_req);
	if (dce->req_hdr.opnum == SRVSVC_OPNUM_SHARE_ENUM_ALL)
		ret = srvsvc_share_enum_all_invoke(pipe);
	return ret;
}

static int srvsvc_clear_headers(struct ksmbd_rpc_pipe *pipe,
				int status)
{
	if (status == KSMBD_RPC_EMORE_DATA)
		return 0;

	ndr_free_uniq_vstring_ptr(&pipe->dce->si_req.server_name);
	if (pipe->dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO)
		ndr_free_vstring_ptr(&pipe->dce->si_req.share_name);

	return 0;
}

static int srvsvc_share_info_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int status = KSMBD_RPC_ENOTIMPLEMENTED;

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

	if (dce->req_hdr.opnum == SRVSVC_OPNUM_GET_SHARE_INFO)
		status = srvsvc_share_get_info_return(pipe);
	if (dce->req_hdr.opnum == SRVSVC_OPNUM_SHARE_ENUM_ALL)
		status = srvsvc_share_enum_all_return(pipe);

	if (rpc_restricted_context(dce->rpc_req))
		status = KSMBD_RPC_EACCESS_DENIED;

	srvsvc_clear_headers(pipe, status);

	/*
	 * [out] DWORD Return value/code
	 */
	if (ndr_write_int32(dce, status))
		return KSMBD_RPC_EBAD_DATA;

	if (dcerpc_write_headers(dce, status))
		return KSMBD_RPC_EBAD_DATA;

	dce->rpc_resp->payload_sz = dce->offset;

	return KSMBD_RPC_OK;
}

static int srvsvc_invoke(struct ksmbd_rpc_pipe *pipe)
{
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;

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

static int srvsvc_return(struct ksmbd_rpc_pipe *pipe,
			 struct ksmbd_rpc_command *resp,
			 int max_resp_sz)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
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
		ret = KSMBD_RPC_EBAD_FUNC;
		break;
	}
	return ret;
}

int rpc_srvsvc_read_request(struct ksmbd_rpc_pipe *pipe,
			    struct ksmbd_rpc_command *resp,
			    int max_resp_sz)
{
	return srvsvc_return(pipe, resp, max_resp_sz);
}

int rpc_srvsvc_write_request(struct ksmbd_rpc_pipe *pipe)
{
	return srvsvc_invoke(pipe);
}
