// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 *   Author(s): Namjae Jeon (linkinjeon@kernel.org)
 */

#include <memory.h>
#include <endian.h>
#include <glib.h>
#include <pwd.h>
#include <errno.h>
#include <linux/ksmbd_server.h>

#include <management/user.h>
#include <rpc.h>
#include <rpc_lsarpc.h>
#include <smbacl.h>
#include <tools.h>

#define LSARPC_OPNUM_DS_ROLE_GET_PRIMARY_DOMAIN_INFO	0
#define LSARPC_OPNUM_OPEN_POLICY2			44
#define LSARPC_OPNUM_QUERY_INFO_POLICY			7
#define LSARPC_OPNUM_LOOKUP_SID2			57
#define LSARPC_OPNUM_LOOKUP_NAMES3			68
#define LSARPC_OPNUM_CLOSE				0

#define DS_ROLE_STANDALONE_SERVER	2
#define DS_ROLE_BASIC_INFORMATION	1

#define LSA_POLICY_INFO_ACCOUNT_DOMAIN	5

static GHashTable	*ph_table;
static GRWLock		ph_table_lock;
static gchar		*domain_name;

static void lsarpc_ph_free(struct policy_handle *ph)
{
	g_rw_lock_writer_lock(&ph_table_lock);
	g_hash_table_remove(ph_table, &(ph->handle));
	g_rw_lock_writer_unlock(&ph_table_lock);

	g_free(ph);
}

static struct policy_handle *lsarpc_ph_lookup(unsigned char *handle)
{
	struct policy_handle *ph;

	g_rw_lock_reader_lock(&ph_table_lock);
	ph = g_hash_table_lookup(ph_table, handle);
	g_rw_lock_reader_unlock(&ph_table_lock);

	return ph;
}

static struct policy_handle *lsarpc_ph_alloc(unsigned int id)
{
	struct policy_handle *ph;
	int ret;

	ph = g_try_malloc0(sizeof(struct policy_handle));
	if (!ph)
		return NULL;

	id++;
	memcpy(ph->handle, &id, sizeof(unsigned int));
	g_rw_lock_writer_lock(&ph_table_lock);
	ret = g_hash_table_insert(ph_table, &(ph->handle), ph);
	g_rw_lock_writer_unlock(&ph_table_lock);

	if (!ret) {
		lsarpc_ph_free(ph);
		ph = NULL;
	}

	return ph;
}

static int lsa_domain_account_rep(struct ksmbd_dcerpc *dce, char *domain_name)
{
	int len, ret;

	len = strlen(domain_name);
	ret = ndr_write_int16(dce, len*2); // length
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, (len+1)*2); // size
	if (ret)
		return ret;

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); /* ref pointer for domain name*/
	if (ret)
		return ret;

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); /* ref pointer for sid*/

	return ret;
}

static int lsa_domain_account_data(struct ksmbd_dcerpc *dce, char *domain_name,
		struct smb_sid *sid)
{
	int ret;

	ret = ndr_write_lsa_string(dce, domain_name); // domain string
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, sid->num_subauth); // count
	if (ret)
		return ret;

	ret = smb_write_sid(dce, sid); // sid

	return ret;
}

static int lsarpc_get_primary_domain_info_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	__u16 val;

	if (ndr_read_int16(dce, &val))
		return KSMBD_RPC_EINVALID_PARAMETER;

	dce->lr_req.level = val;

	return KSMBD_RPC_OK;
}

static int lsarpc_get_primary_domain_info_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int i;

	if (dce->lr_req.level != DS_ROLE_BASIC_INFORMATION)
		return KSMBD_RPC_EBAD_FUNC;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int16(dce, 1)) // count
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int16(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int16(dce, DS_ROLE_STANDALONE_SERVER)) // role
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int16(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0)) // flags
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0)) // NULL pointer : Pointer to Dns Domain
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0)) // NULL pointer : Pointer to Forest
		return KSMBD_RPC_EBAD_DATA;


	/* NULL Domain guid */
	for (i = 0; i < 16; i++) {
		if (ndr_write_int8(dce, 0))
			return KSMBD_RPC_EBAD_DATA;
	}

	if (ndr_write_vstring(dce, domain_name)) // domain string
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int lsarpc_open_policy2_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct policy_handle *ph;

	ph = lsarpc_ph_alloc(pipe->id);
	if (!ph)
		return KSMBD_RPC_ENOMEM;

	/* write connect handle */
	if (ndr_write_bytes(dce, ph->handle, HANDLE_SIZE))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int lsarpc_query_info_policy_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	__u16 val;

	if (ndr_read_bytes(dce, dce->lr_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// level
	if (ndr_read_int16(dce, &val))
		return KSMBD_RPC_EINVALID_PARAMETER;

	dce->lr_req.level = val;

	return KSMBD_RPC_OK;
}

static int lsarpc_query_info_policy_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct smb_sid sid;
	struct policy_handle *ph;

	ph = lsarpc_ph_lookup(dce->lr_req.handle);
	if (!ph)
		return KSMBD_RPC_EBAD_FID;

	if (dce->lr_req.level != LSA_POLICY_INFO_ACCOUNT_DOMAIN)
		return KSMBD_RPC_EBAD_FUNC;

	dce->num_pointers++;
	// ref pointer
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;

	// level
	if (ndr_write_int16(dce, LSA_POLICY_INFO_ACCOUNT_DOMAIN))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int16(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	/* Domain, Sid ref pointer */
	if (lsa_domain_account_rep(dce, domain_name))
		return KSMBD_RPC_EBAD_DATA;

	/* Pointer to domain, Sid */
	smb_init_domain_sid(&sid);
	if (lsa_domain_account_data(dce, domain_name, &sid))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int __lsarpc_entry_processed(struct ksmbd_rpc_pipe *pipe, int i)
{
	struct lsarpc_names_info *ni;

	ni = g_ptr_array_remove_index(pipe->entries, i);
	g_free(ni);
	return 0;
}

static int lsarpc_lookup_sid2_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct lsarpc_names_info *ni = NULL;
	unsigned int num_sid, i;

	if (ndr_read_bytes(dce, dce->lr_req.handle, HANDLE_SIZE))
		goto fail;

	if (ndr_read_int32(dce, &num_sid))
		goto fail;
	// ref pointer
	if (ndr_read_int32(dce, NULL))
		goto fail;
	// max count
	if (ndr_read_int32(dce, NULL))
		goto fail;

	for (i = 0; i < num_sid; i++)
		if (ndr_read_int32(dce, NULL)) // ref pointer
			goto fail;

	for (i = 0; i < num_sid; i++) {
		struct passwd *passwd;
		int rid;

		ni = g_try_malloc0(sizeof(struct lsarpc_names_info));
		if (!ni)
			break;

		// max count
		if (ndr_read_int32(dce, NULL))
			goto fail;
		// sid
		if (smb_read_sid(dce, &ni->sid))
			goto fail;
		ni->sid.num_subauth--;
		rid = ni->sid.sub_auth[ni->sid.num_subauth];
		passwd = getpwuid(rid);
		if (passwd)
			ni->user = usm_lookup_user(passwd->pw_name);

		ni->index = i + 1;
		if (set_domain_name(&ni->sid, ni->domain_str,
				    sizeof(ni->domain_str), &ni->type))
			ni->index = -1;

		g_ptr_array_add(pipe->entries, ni);
		pipe->num_entries++;
	}

	pipe->entry_processed = __lsarpc_entry_processed;
	return KSMBD_RPC_OK;
fail:
	g_free(ni);
	return KSMBD_RPC_EINVALID_PARAMETER;
}

static int lsarpc_lookup_sid2_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct policy_handle *ph;
	int i, rc = KSMBD_RPC_OK;

	ph = lsarpc_ph_lookup(dce->lr_req.handle);
	if (!ph)
		return KSMBD_RPC_EBAD_FID;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, pipe->num_entries)) // count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 32)) // max size
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, pipe->num_entries)) // max count
		return KSMBD_RPC_EBAD_DATA;

	for (i = 0; i < pipe->num_entries; i++) {
		struct lsarpc_names_info *ni;

		ni = g_ptr_array_index(pipe->entries, i);
		if (ni->type == -1)
			rc = KSMBD_RPC_SOME_NOT_MAPPED;
		if (lsa_domain_account_rep(dce, ni->domain_str))
			return KSMBD_RPC_EBAD_DATA;
	}

	for (i = 0; i < pipe->num_entries; i++) {
		struct lsarpc_names_info *ni;

		ni = g_ptr_array_index(pipe->entries, i);
		if (lsa_domain_account_data(dce, ni->domain_str, &ni->sid))
			return KSMBD_RPC_EBAD_DATA;
	}

	/* Pointer to Names */
	if (ndr_write_int32(dce, pipe->num_entries)) // count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++; // ref pointer
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, pipe->num_entries)) // max count
		return KSMBD_RPC_EBAD_DATA;

	for (i = 0; i < pipe->num_entries; i++) {
		struct lsarpc_names_info *ni;
		size_t len;

		ni = g_ptr_array_index(pipe->entries, i);
		if (ndr_write_int16(dce, ni->type)) // sid type
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int16(dce, 0))
			return KSMBD_RPC_EBAD_DATA;

		if (ni->user)
			len = strlen(ni->user->name);
		else
			len = strlen("None");
		if (ndr_write_int16(dce, len*2)) // length
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int16(dce, len*2)) // size
			return KSMBD_RPC_EBAD_DATA;

		dce->num_pointers++; // ref pointer
		if (ndr_write_int32(dce, dce->num_pointers))
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int32(dce, ni->index)) // sid index
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int32(dce, 0)) // unknown
			return KSMBD_RPC_EBAD_DATA;
	}

	for (i = 0; i < pipe->num_entries; i++) {
		struct lsarpc_names_info *ni;
		char *username = "None";

		ni = g_ptr_array_index(pipe->entries, i);
		if (ni->user)
			username = ni->user->name;
		if (ndr_write_string(dce, username)) // username
			return KSMBD_RPC_EBAD_DATA;

	}

	if (ndr_write_int32(dce, pipe->num_entries)) // count
		return KSMBD_RPC_EBAD_DATA;

	if (pipe->entry_processed) {
		for (i = 0; i < pipe->num_entries; i++)
			pipe->entry_processed(pipe, 0);
		pipe->num_entries = 0;
	}

	return rc;
}

static int lsarpc_lookup_names3_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct lsarpc_names_info *ni = NULL;
	struct ndr_uniq_char_ptr username;
	int num_names, i;

	if (ndr_read_bytes(dce, dce->lr_req.handle, HANDLE_SIZE))
		goto fail;

	// num names
	if (ndr_read_int32(dce, &num_names))
		goto fail;
	// max count
	if (ndr_read_int32(dce, NULL))
		goto fail;

	for (i = 0; i < num_names; i++) {
		char *name = NULL;

		ni = g_try_malloc(sizeof(struct lsarpc_names_info));
		if (!ni)
			break;
		// length
		if (ndr_read_int16(dce, NULL))
			goto fail;
		// size
		if (ndr_read_int16(dce, NULL))
			goto fail;
		if (ndr_read_uniq_vstring_ptr(dce, &username))
			goto fail;
		if (STR_VAL(username) && strstr(STR_VAL(username), "\\")) {
			strtok(STR_VAL(username), "\\");
			name = strtok(NULL, "\\");
		}

		ni->user = usm_lookup_user(name);
		ndr_free_uniq_vstring_ptr(&username);
		if (!ni->user) {
			g_free(ni);
			break;
		}
		g_ptr_array_add(pipe->entries, ni);
		pipe->num_entries++;
		smb_init_domain_sid(&ni->sid);
	}
	pipe->entry_processed = __lsarpc_entry_processed;

	return KSMBD_RPC_OK;

fail:
	g_free(ni);
	return KSMBD_RPC_EINVALID_PARAMETER;
}

static int lsarpc_lookup_names3_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct policy_handle *ph;
	struct smb_sid sid;
	int i;

	ph = lsarpc_ph_lookup(dce->lr_req.handle);
	if (!ph)
		return KSMBD_RPC_EBAD_FID;

	/* Domain list */
	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1)) // domain count
		return KSMBD_RPC_EBAD_DATA;
	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;
	if (ndr_write_int32(dce, 32)) // max size
		return KSMBD_RPC_EBAD_DATA;
	if (ndr_write_int32(dce, 1)) // max count
		return KSMBD_RPC_EBAD_DATA;

	if (lsa_domain_account_rep(dce, domain_name))
		return KSMBD_RPC_EBAD_DATA;

	smb_init_domain_sid(&sid);
	if (lsa_domain_account_data(dce, domain_name, &sid))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, pipe->num_entries)) // count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // sid ref id
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, pipe->num_entries)) // count
		return KSMBD_RPC_EBAD_DATA;

	for (i = 0; i < pipe->num_entries; i++) {
		if (ndr_write_int16(dce, SID_TYPE_USER)) // sid type
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int16(dce, 0))
			return KSMBD_RPC_EBAD_DATA;

		dce->num_pointers++;
		if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int32(dce, i)) // sid index
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int32(dce, 0))
			return KSMBD_RPC_EBAD_DATA;
	}

	for (i = 0; i < pipe->num_entries; i++) {
		struct lsarpc_names_info *ni;

		ni = g_ptr_array_index(pipe->entries, i);
		ni->sid.sub_auth[ni->sid.num_subauth++] = ni->user->uid;

		if (ndr_write_int32(dce, ni->sid.num_subauth)) // sid auth count
			return KSMBD_RPC_EBAD_DATA;

		if (smb_write_sid(dce, &ni->sid)) // sid
			return KSMBD_RPC_EBAD_DATA;
	}

	if (ndr_write_int32(dce, pipe->num_entries))
		return KSMBD_RPC_EBAD_DATA;

	if (pipe->entry_processed) {
		for (i = 0; i < pipe->num_entries; i++)
			pipe->entry_processed(pipe, 0);
		pipe->num_entries = 0;
	}

	return KSMBD_RPC_OK;
}

static int lsarpc_close_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->lr_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int lsarpc_close_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct policy_handle *ph;

	ph = lsarpc_ph_lookup(dce->lr_req.handle);
	if (!ph)
		return KSMBD_RPC_EBAD_FID;
	lsarpc_ph_free(ph);

	if (ndr_write_int64(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int64(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int lsarpc_invoke(struct ksmbd_rpc_pipe *pipe)
{
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;

	switch (pipe->dce->req_hdr.opnum) {
	case LSARPC_OPNUM_DS_ROLE_GET_PRIMARY_DOMAIN_INFO || LSARPC_OPNUM_CLOSE:
		if (pipe->dce->hdr.frag_length == 26)
			ret = lsarpc_get_primary_domain_info_invoke(pipe);
		else
			ret = lsarpc_close_invoke(pipe);
		break;
	case LSARPC_OPNUM_OPEN_POLICY2:
		ret = KSMBD_RPC_OK;
		break;
	case LSARPC_OPNUM_QUERY_INFO_POLICY:
		ret = lsarpc_query_info_policy_invoke(pipe);
		break;
	case LSARPC_OPNUM_LOOKUP_SID2:
		ret = lsarpc_lookup_sid2_invoke(pipe);
		break;
	case LSARPC_OPNUM_LOOKUP_NAMES3:
		ret = lsarpc_lookup_names3_invoke(pipe);
		break;
	default:
		pr_err("LSARPC: unsupported INVOKE method %d, alloc_hint : %d\n",
		       pipe->dce->req_hdr.opnum, pipe->dce->req_hdr.alloc_hint);
		break;
	}

	return ret;
}

static int lsarpc_return(struct ksmbd_rpc_pipe *pipe,
			 struct ksmbd_rpc_command *resp,
			 int max_resp_sz)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int status = KSMBD_RPC_ENOTIMPLEMENTED;

	dce->offset = sizeof(struct dcerpc_header);
	dce->offset += sizeof(struct dcerpc_response_header);

	switch (dce->req_hdr.opnum) {
	case LSARPC_OPNUM_DS_ROLE_GET_PRIMARY_DOMAIN_INFO || LSARPC_OPNUM_CLOSE:
		if (dce->hdr.frag_length == 26)
			status = lsarpc_get_primary_domain_info_return(pipe);
		else
			status = lsarpc_close_return(pipe);
		break;
	case LSARPC_OPNUM_OPEN_POLICY2:
		status = lsarpc_open_policy2_return(pipe);
		break;
	case LSARPC_OPNUM_QUERY_INFO_POLICY:
		status = lsarpc_query_info_policy_return(pipe);
		break;
	case LSARPC_OPNUM_LOOKUP_SID2:
		status = lsarpc_lookup_sid2_return(pipe);
		break;
	case LSARPC_OPNUM_LOOKUP_NAMES3:
		status = lsarpc_lookup_names3_return(pipe);
		break;
	default:
		pr_err("LSARPC: unsupported RETURN method %d\n",
			dce->req_hdr.opnum);
		status = KSMBD_RPC_EBAD_FUNC;
		break;
	}

	/*
	 * [out] DWORD Return value/code
	 */
	if (ndr_write_int32(dce, status))
		return KSMBD_RPC_EBAD_DATA;

	if (dcerpc_write_headers(dce, status))
		return KSMBD_RPC_EBAD_DATA;

	dce->rpc_resp->payload_sz = dce->offset;
	return status;
}

int rpc_lsarpc_read_request(struct ksmbd_rpc_pipe *pipe,
			    struct ksmbd_rpc_command *resp,
			    int max_resp_sz)
{
	return lsarpc_return(pipe, resp, max_resp_sz);
}

int rpc_lsarpc_write_request(struct ksmbd_rpc_pipe *pipe)
{
	return lsarpc_invoke(pipe);
}

static void lsarpc_ph_clear_table(void)
{
	struct policy_handle *ph;
	GHashTableIter iter;

	g_rw_lock_writer_lock(&ph_table_lock);
	ghash_for_each(ph, ph_table, iter)
		g_free(ph);
	g_rw_lock_writer_unlock(&ph_table_lock);
}

void rpc_lsarpc_init(void)
{
	if (!domain_name) {
		char hostname[NAME_MAX];

		/*
		 * ksmbd supports the standalone server and
		 * uses the hostname as the domain name.
		 */
		if (gethostname(hostname, NAME_MAX))
			abort();

		domain_name = g_ascii_strup(hostname, -1);
	}

	if (!ph_table)
		ph_table = g_hash_table_new(g_str_hash, g_str_equal);
}

void rpc_lsarpc_destroy(void)
{
	if (ph_table) {
		lsarpc_ph_clear_table();
		g_hash_table_destroy(ph_table);
		ph_table = NULL;
	}

	g_free(domain_name);
	domain_name = NULL;
}
