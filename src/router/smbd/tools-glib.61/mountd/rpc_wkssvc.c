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
#include <rpc_wkssvc.h>
#include <tools.h>

#define WKSSVC_NETWKSTA_GET_INFO	(0)

#define WKSSVC_PLATFORM_ID_DOS		300
#define WKSSVC_PLATFORM_ID_OS2		400
#define WKSSVC_PLATFORM_ID_NT		500
#define WKSSVC_PLATFORM_ID_OSF		600
#define WKSSVC_PLATFORM_ID_VMS		700

#define WKSSVC_VERSION_MAJOR		0x2
#define WKSSVC_VERSION_MINOR		0x1

static int wkssvc_clear_headers(struct ksmbd_rpc_pipe *pipe,
				int status)
{
	ndr_free_uniq_vstring_ptr(&pipe->dce->wi_req.server_name);
	return 0;
}

static int __netwksta_entry_rep_ctr100(struct ksmbd_dcerpc *dce,
				       gpointer entry)
{
	int ret = 0;

	/* srvsvc_PlatformId */
	ret = ndr_write_int32(dce, WKSSVC_PLATFORM_ID_NT);
	if (ret)
		return ret;

	/* server_name */
	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
	if (ret)
		return ret;

	/* domain_name */
	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); /* ref pointer */
	if (ret)
		return ret;

	/* version_major */
	ret = ndr_write_int32(dce, WKSSVC_VERSION_MAJOR);
	if (ret)
		return ret;

	/* version_minor */
	ret = ndr_write_int32(dce, WKSSVC_VERSION_MINOR);

	return ret;
}

static int __netwksta_entry_data_ctr100(struct ksmbd_dcerpc *dce,
					gpointer entry)
{
	int ret = 0;

	/*
	 * Umm... Hmm... Huh...
	 */
	ret = ndr_write_vstring(dce, STR_VAL(dce->wi_req.server_name));
	if (ret)
		return ret;

	return ndr_write_vstring(dce, global_conf.work_group);
}

static int wkssvc_netwksta_get_info_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_write_union_int32(dce, dce->wi_req.level))
		return KSMBD_RPC_EBAD_DATA;

	if (dce->wi_req.level != 100) {
		pr_err("Unsupported wksta info level (read): %d\n",
			dce->wi_req.level);
		dce->entry_rep = NULL;
		return KSMBD_RPC_EINVALID_LEVEL;
	}

	if (dce->entry_rep(dce, NULL))
		return KSMBD_RPC_EBAD_DATA;

	if (dce->entry_data(dce, NULL))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int wkssvc_netwksta_info_return(struct ksmbd_rpc_pipe *pipe)
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

	if (dce->wi_req.level == 100) {
		dce->entry_rep = __netwksta_entry_rep_ctr100;
		dce->entry_data = __netwksta_entry_data_ctr100;
	} else {
		pr_err("Unsupported wksta info level (write): %d\n",
			dce->wi_req.level);
		rpc_pipe_reset(pipe);
	}

	if (dce->req_hdr.opnum == WKSSVC_NETWKSTA_GET_INFO)
		status = wkssvc_netwksta_get_info_return(pipe);

	if (rpc_restricted_context(dce->rpc_req))
		status = KSMBD_RPC_EACCESS_DENIED;

	wkssvc_clear_headers(pipe, status);

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

static int
wkssvc_netwksta_get_info_invoke(struct ksmbd_rpc_pipe *pipe,
				struct wkssvc_netwksta_info_request *hdr)
{
	return KSMBD_RPC_OK;
}

static int
wkssvc_parse_netwksta_info_req(struct ksmbd_dcerpc *dce,
			       struct wkssvc_netwksta_info_request *hdr)
{
	int val;

	if (ndr_read_uniq_vstring_ptr(dce, &hdr->server_name))
		return -EINVAL;
	if (ndr_read_int32(dce, &val))
		return -EINVAL;
	hdr->level = val;
	return 0;
}

static int wkssvc_netwksta_info_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;

	if (wkssvc_parse_netwksta_info_req(dce, &dce->wi_req))
		return KSMBD_RPC_EBAD_DATA;

	if (rpc_restricted_context(dce->rpc_req))
		return KSMBD_RPC_OK;

	if (dce->req_hdr.opnum == WKSSVC_NETWKSTA_GET_INFO)
		ret = wkssvc_netwksta_get_info_invoke(pipe, &dce->wi_req);
	return ret;
}

static int wkssvc_invoke(struct ksmbd_rpc_pipe *pipe)
{
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;

	switch (pipe->dce->req_hdr.opnum) {
	case WKSSVC_NETWKSTA_GET_INFO:
		ret = wkssvc_netwksta_info_invoke(pipe);
		break;
	default:
		pr_debug("WKSSVC: unsupported INVOKE method %d\n",
		       pipe->dce->req_hdr.opnum);
		break;
	}

	return ret;
}

static int wkssvc_return(struct ksmbd_rpc_pipe *pipe,
			 struct ksmbd_rpc_command *resp,
			 int max_resp_sz)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int ret;

	switch (dce->req_hdr.opnum) {
	case WKSSVC_NETWKSTA_GET_INFO:
		dcerpc_set_ext_payload(dce, resp->payload, max_resp_sz);

		ret = wkssvc_netwksta_info_return(pipe);
		break;
	default:
		pr_err("WKSSVC: unsupported RETURN method %d\n",
			dce->req_hdr.opnum);
		ret = KSMBD_RPC_EBAD_FUNC;
		break;
	}
	return ret;
}

int rpc_wkssvc_read_request(struct ksmbd_rpc_pipe *pipe,
			    struct ksmbd_rpc_command *resp,
			    int max_resp_sz)
{
	return wkssvc_return(pipe, resp, max_resp_sz);
}

int rpc_wkssvc_write_request(struct ksmbd_rpc_pipe *pipe)
{
	return wkssvc_invoke(pipe);
}
