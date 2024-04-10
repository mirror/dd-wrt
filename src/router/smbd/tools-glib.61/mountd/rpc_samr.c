// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 *   Author(s): Namjae Jeon (linkinjeon@kernel.org)
 */

#include <memory.h>
#include <endian.h>
#include <glib.h>
#include <errno.h>
#include <linux/ksmbd_server.h>

#include <management/user.h>
#include <rpc.h>
#include <rpc_samr.h>
#include <smbacl.h>
#include <tools.h>

#define SAMR_OPNUM_CONNECT5		64
#define SAMR_OPNUM_ENUM_DOMAIN		6
#define SAMR_OPNUM_LOOKUP_DOMAIN	5
#define SAMR_OPNUM_OPEN_DOMAIN		7
#define SAMR_OPNUM_LOOKUP_NAMES		17
#define SAMR_OPNUM_OPEN_USER		34
#define SAMR_OPNUM_QUERY_USER_INFO	36
#define SAMR_OPNUM_QUERY_SECURITY	3
#define SAMR_OPNUM_GET_GROUP_FOR_USER	39
#define SAMR_OPNUM_GET_ALIAS_MEMBERSHIP	16
#define SAMR_OPNUM_CLOSE		1

static GHashTable	*ch_table;
static GRWLock		ch_table_lock;
static GPtrArray	*domain_entries;
static gchar		*domain_name;
static int		num_domain_entries;

static void samr_ch_free(struct connect_handle *ch)
{
	g_rw_lock_writer_lock(&ch_table_lock);
	g_hash_table_remove(ch_table, &(ch->handle));
	g_rw_lock_writer_unlock(&ch_table_lock);

	g_free(ch);
}

static struct connect_handle *samr_ch_lookup(unsigned char *handle)
{
	struct connect_handle *ch;

	g_rw_lock_reader_lock(&ch_table_lock);
	ch = g_hash_table_lookup(ch_table, handle);
	g_rw_lock_reader_unlock(&ch_table_lock);

	return ch;
}

static struct connect_handle *samr_ch_alloc(unsigned int id)
{
	struct connect_handle *ch;
	int ret;

	ch = g_try_malloc0(sizeof(struct connect_handle));
	if (!ch)
		return NULL;

	id++;
	memcpy(ch->handle, &id, sizeof(unsigned int));
	ch->refcount++;
	g_rw_lock_writer_lock(&ch_table_lock);
	ret = g_hash_table_insert(ch_table, &(ch->handle), ch);
	g_rw_lock_writer_unlock(&ch_table_lock);

	if (!ret) {
		samr_ch_free(ch);
		ch = NULL;
	}

	return ch;
}

static int samr_connect5_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct ndr_uniq_char_ptr server_name;

	if (ndr_read_uniq_vstring_ptr(dce, &server_name))
		return KSMBD_RPC_EINVALID_PARAMETER;
	ndr_free_uniq_vstring_ptr(&server_name);

	// Access mask
	if (ndr_read_int32(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// level in
	if (ndr_read_int32(dce, &dce->sm_req.level))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// Info in
	if (ndr_read_int32(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	if (ndr_read_int32(dce, &dce->sm_req.client_version))
		return KSMBD_RPC_EINVALID_PARAMETER;
	return 0;
}

static int samr_connect5_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	if (ndr_write_union_int32(dce, dce->sm_req.level)) //level out
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, dce->sm_req.client_version)) //client version
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0)) //reserved
		return KSMBD_RPC_EBAD_DATA;

	ch = samr_ch_alloc(pipe->id);
	if (!ch)
		return KSMBD_RPC_ENOMEM;

	/* write connect handle */
	if (ndr_write_bytes(dce, ch->handle, HANDLE_SIZE))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_enum_domain_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

int samr_ndr_write_domain_array(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int i, ret = 0;

	for (i = 0; i < num_domain_entries; i++) {
		char *entry;
		size_t name_len;

		ret = ndr_write_int32(dce, i);
		if (ret)
			return ret;
		entry = g_ptr_array_index(domain_entries, i);
		name_len = strlen(entry);
		ret = ndr_write_int16(dce, name_len*2);
		if (ret)
			return ret;

		ret = ndr_write_int16(dce, name_len*2);
		if (ret)
			return ret;

		/* ref pointer for name entry */
		dce->num_pointers++;
		ret = ndr_write_int32(dce, dce->num_pointers);
		if (ret)
			return ret;
	}

	for (i = 0; i < num_domain_entries; i++) {
		char *entry;

		entry = g_ptr_array_index(domain_entries, i);
		ret = ndr_write_string(dce, entry);
		if (ret)
			return ret;
	}

	return ret;
}

static int samr_enum_domain_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;
	/* Resume Handle */
	if (ndr_write_int32(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, num_domain_entries)) // Sam entry count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;

	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, num_domain_entries)) // Sam max entry count
		return KSMBD_RPC_EBAD_DATA;

	if (samr_ndr_write_domain_array(pipe))
		return KSMBD_RPC_EBAD_DATA;

	/* [out] DWORD* Num Entries */
	if (ndr_write_int32(dce, num_domain_entries))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_lookup_domain_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// name len
	if (ndr_read_int16(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// name size
	if (ndr_read_int16(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// domain name
	if (ndr_read_uniq_vstring_ptr(dce, &dce->sm_req.name))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_lookup_domain_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;
	struct smb_sid sid = {0};
	int i;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 4))
		return KSMBD_RPC_EBAD_DATA;

	for (i = 0; i < num_domain_entries; i++) {
		char *entry;

		entry = g_ptr_array_index(domain_entries, i);
		if (!strcmp(STR_VAL(dce->sm_req.name), entry)) {
			smb_init_domain_sid(&sid);
			if (smb_write_sid(dce, &sid))
				return KSMBD_RPC_EBAD_DATA;
		}
	}
	ndr_free_uniq_vstring_ptr(&dce->sm_req.name);

	return KSMBD_RPC_OK;
}

static int samr_open_domain_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_open_domain_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;
	ch->refcount++;
	if (ndr_write_bytes(dce, ch->handle, HANDLE_SIZE))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_lookup_names_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int user_num;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	if (ndr_read_int32(dce, &user_num))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// max count
	if (ndr_read_int32(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// offset
	if (ndr_read_int32(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// actual count
	if (ndr_read_int32(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// name len
	if (ndr_read_int16(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// name size
	if (ndr_read_int16(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;

	// names
	if (ndr_read_uniq_vstring_ptr(dce, &dce->sm_req.name))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_lookup_names_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;

	ch->user = usm_lookup_user(STR_VAL(dce->sm_req.name));
	if (!ch->user)
		return KSMBD_RPC_EACCESS_DENIED;

	if (ndr_write_int32(dce, 1)) // count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1)) // count
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, ch->user->uid)) // RID
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1))
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_open_user_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	if (ndr_read_int32(dce, NULL))
		return KSMBD_RPC_EINVALID_PARAMETER;
	// RID
	if (ndr_read_int32(dce, &dce->sm_req.rid))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_open_user_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;
	ch->refcount++;

	if (!ch->user)
		return KSMBD_RPC_EBAD_FID;

	if (dce->sm_req.rid != ch->user->uid)
		return KSMBD_RPC_EBAD_FID;

	if (ndr_write_bytes(dce, ch->handle, HANDLE_SIZE))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_query_user_info_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_query_user_info_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;
	char *home_dir;
	g_autofree char *profile_path = NULL;
	char hostname[NAME_MAX];
	int home_dir_len, i, ret;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;

	if (gethostname(hostname, NAME_MAX))
		return KSMBD_RPC_ENOMEM;

	home_dir_len = 2 + strlen(hostname) + 1 + strlen(ch->user->name) + 1;

	home_dir = g_try_malloc0(home_dir_len);
	if (!home_dir)
		return KSMBD_RPC_ENOMEM;

	/* Make Home dir string */
	strcpy(home_dir, "\\\\");
	strcat(home_dir, hostname);
	strcat(home_dir, "\\");
	strcat(home_dir, ch->user->name);

	profile_path = g_try_malloc0(home_dir_len + strlen("profile"));
	if (!profile_path) {
		g_free(home_dir);
		return KSMBD_RPC_ENOMEM;
	}

	/* Make Profile path string */
	strcat(profile_path, "\\\\");
	strcat(profile_path, hostname);
	strcat(profile_path, "\\");
	strcat(profile_path, ch->user->name);
	strcat(profile_path, "\\");
	strcat(profile_path, "profile");

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); // ref pointer
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, 0x15); // info
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, 0);
	if (ret)
		goto out;

	/*
	 * Last Logon/Logoff/Password change, Acct Expiry,
	 * Allow Passworkd Change, Force Password Change.
	 */
	for (i = 0; i < 6; i++) {
		ret = ndr_write_int64(dce, 0);
		if (ret)
			goto out;
	}

	ret = ndr_write_int16(dce, strlen(ch->user->name)*2); // account name length
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(ch->user->name)*2);
	if (ret)
		goto out;

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); // ref pointer
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(ch->user->name)*2); // full name length
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(ch->user->name)*2);
	if (ret)
		goto out;


	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); // ref pointer
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(home_dir)*2); // home directory length
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(home_dir)*2);
	if (ret)
		goto out;

	/* Home Drive, Logon Script */
	for (i = 0; i < 2; i++) {
		dce->num_pointers++;
		ret = ndr_write_int32(dce, dce->num_pointers); // ref pointer
		if (ret)
			goto out;

		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;
	}

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); // ref pointer
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(profile_path)*2); //profile path length
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, strlen(profile_path)*2);
	if (ret)
		goto out;


	/* Description, Workstations, Comments, Parameters */
	for (i = 0; i < 4; i++) {
		dce->num_pointers++;
		ret = ndr_write_int32(dce, dce->num_pointers); // ref pointer
		if (ret)
			goto out;

		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;
	}

	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers);
	if (ret)
		goto out;

	/* Lm, Nt, Password and Private*/
	for (i = 0; i < 3; i++) {
		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;
	}

	ret = ndr_write_int32(dce, 0); // buf count
	if (ret)
		goto out;

	/* Pointer to Buffer */
	ret = ndr_write_int32(dce, 0);
	if (ret)
		goto out;

	ret = ndr_write_int32(dce, ch->user->uid); // rid
	if (ret)
		goto out;

	ret = ndr_write_int32(dce, 513); // primary gid
	if (ret)
		goto out;

	ret = ndr_write_int32(dce, 0x00000010); // Acct Flags : Acb Normal
	if (ret)
		goto out;

	ret = ndr_write_int32(dce, 0x00FFFFFF); // Fields Present
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, 168); // logon hours
	if (ret)
		goto out;

	ret = ndr_write_int16(dce, 0);
	if (ret)
		goto out;

	/* Pointers to Bits */
	dce->num_pointers++;
	ret = ndr_write_int32(dce, dce->num_pointers); //ref pointer
	if (ret)
		goto out;

	/* Bad Password/Logon Count/Country Code/Code Page */
	for (i = 0; i < 4; i++) {
		ret = ndr_write_int16(dce, 0);
		if (ret)
			goto out;
	}


	/* Lm/Nt Password Set, Password Expired/etc */
	ret = ndr_write_int8(dce, 0);
	if (ret)
		goto out;

	ret = ndr_write_int8(dce, 0);
	if (ret)
		goto out;

	ret = ndr_write_int8(dce, 0);
	if (ret)
		goto out;

	ret = ndr_write_int8(dce, 0);
	if (ret)
		goto out;


	ret = ndr_write_string(dce, ch->user->name);
	if (ret)
		goto out;

	ret = ndr_write_string(dce, ch->user->name);
	if (ret)
		goto out;

	ret = ndr_write_string(dce, home_dir);
	if (ret)
		goto out;


	/* Home Drive, Logon Script */
	for (i = 0; i < 2; i++) {
		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;
	}

	ret = ndr_write_string(dce, profile_path);
	if (ret)
		goto out;

	/* Description, Workstations, Comments, Parameters */
	for (i = 0; i < 4; i++) {
		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;

		ret = ndr_write_int32(dce, 0);
		if (ret)
			goto out;
	}

	/* Logon Hours */
	ret = ndr_write_int32(dce, 1260);
	if (ret)
		goto out;

	ret = ndr_write_int32(dce, 0);
	if (ret)
		goto out;

	ret = ndr_write_int32(dce, 21);
	if (ret)
		goto out;

	for (i = 0; i < 21; i++) {
		ret = ndr_write_int8(dce, 0xff);
		if (ret)
			break;
	}

out:
	g_free(home_dir);
	return ret ? KSMBD_RPC_EBAD_DATA: KSMBD_RPC_OK;
}

static int samr_query_security_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_query_security_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;
	int sec_desc_len, curr_offset, payload_offset;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;

	if (!ch->user)
		return KSMBD_RPC_EBAD_FID;

	curr_offset = dce->offset;
	dce->offset += 16;
	if (build_sec_desc(dce, &sec_desc_len, ch->user->uid))
		return KSMBD_RPC_EBAD_DATA;

	payload_offset = dce->offset;

	dce->offset = curr_offset;
	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;
	if (ndr_write_int32(dce, sec_desc_len))
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, sec_desc_len))
		return KSMBD_RPC_EBAD_DATA;

	dce->offset = payload_offset;

	return KSMBD_RPC_OK;
}

static int samr_get_group_for_user_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EINVALID_PARAMETER;

	return KSMBD_RPC_OK;
}

static int samr_get_group_for_user_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1)) // count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1)) // max count
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 513)) // group rid
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0x00000007)) // attributes
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_get_alias_membership_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EACCESS_DENIED;

	return KSMBD_RPC_OK;
}

static int samr_get_alias_membership_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;

	if (ndr_write_int32(dce, 0)) // count
		return KSMBD_RPC_EBAD_DATA;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers)) // ref pointer
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0)) // max count
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_close_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	if (ndr_read_bytes(dce, dce->sm_req.handle, HANDLE_SIZE))
		return KSMBD_RPC_EACCESS_DENIED;

	return KSMBD_RPC_OK;
}

static int samr_close_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	struct connect_handle *ch;

	ch = samr_ch_lookup(dce->sm_req.handle);
	if (!ch)
		return KSMBD_RPC_EBAD_FID;
	if (ch->refcount > 1)
		ch->refcount--;
	else
		samr_ch_free(ch);

	/* write connect handle */
	if (ndr_write_int64(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int64(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

static int samr_invoke(struct ksmbd_rpc_pipe *pipe)
{
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;

	switch (pipe->dce->req_hdr.opnum) {
	case SAMR_OPNUM_CONNECT5:
		ret = samr_connect5_invoke(pipe);
		break;
	case SAMR_OPNUM_ENUM_DOMAIN:
		ret = samr_enum_domain_invoke(pipe);
		break;
	case SAMR_OPNUM_LOOKUP_DOMAIN:
		ret = samr_lookup_domain_invoke(pipe);
		break;
	case SAMR_OPNUM_OPEN_DOMAIN:
		ret = samr_open_domain_invoke(pipe);
		break;
	case SAMR_OPNUM_LOOKUP_NAMES:
		ret = samr_lookup_names_invoke(pipe);
		break;
	case SAMR_OPNUM_OPEN_USER:
		ret = samr_open_user_invoke(pipe);
		break;
	case SAMR_OPNUM_QUERY_USER_INFO:
		ret = samr_query_user_info_invoke(pipe);
		break;
	case SAMR_OPNUM_QUERY_SECURITY:
		ret = samr_query_security_invoke(pipe);
		break;
	case SAMR_OPNUM_GET_GROUP_FOR_USER:
		ret = samr_get_group_for_user_invoke(pipe);
		break;
	case SAMR_OPNUM_GET_ALIAS_MEMBERSHIP:
		ret = samr_get_alias_membership_invoke(pipe);
		break;
	case SAMR_OPNUM_CLOSE:
		ret = samr_close_invoke(pipe);
		break;
	default:
		pr_err("SAMR: unsupported INVOKE method %d\n",
		       pipe->dce->req_hdr.opnum);
		break;
	}

	return ret;
}

static int samr_return(struct ksmbd_rpc_pipe *pipe,
			 struct ksmbd_rpc_command *resp,
			 int max_resp_sz)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int status;

	/*
	 * Reserve space for response NDR header. We don't know yet if
	 * the payload buffer is big enough. This will determine if we
	 * can set DCERPC_PFC_FIRST_FRAG|DCERPC_PFC_LAST_FRAG or if we
	 * will have a multi-part response.
	 */
	dce->offset = sizeof(struct dcerpc_header);
	dce->offset += sizeof(struct dcerpc_response_header);

	switch (dce->req_hdr.opnum) {
	case SAMR_OPNUM_CONNECT5:
		status = samr_connect5_return(pipe);
		break;
	case SAMR_OPNUM_ENUM_DOMAIN:
		status = samr_enum_domain_return(pipe);
		break;
	case SAMR_OPNUM_LOOKUP_DOMAIN:
		status = samr_lookup_domain_return(pipe);
		break;
	case SAMR_OPNUM_OPEN_DOMAIN:
		status = samr_open_domain_return(pipe);
		break;
	case SAMR_OPNUM_LOOKUP_NAMES:
		status = samr_lookup_names_return(pipe);
		break;
	case SAMR_OPNUM_OPEN_USER:
		status = samr_open_user_return(pipe);
		break;
	case SAMR_OPNUM_QUERY_USER_INFO:
		status = samr_query_user_info_return(pipe);
		break;
	case SAMR_OPNUM_QUERY_SECURITY:
		status = samr_query_security_return(pipe);
		break;
	case SAMR_OPNUM_GET_GROUP_FOR_USER:
		status = samr_get_group_for_user_return(pipe);
		break;
	case SAMR_OPNUM_GET_ALIAS_MEMBERSHIP:
		status = samr_get_alias_membership_return(pipe);
		break;
	case SAMR_OPNUM_CLOSE:
		status = samr_close_return(pipe);
		break;
	default:
		pr_err("SAMR: unsupported RETURN method %d\n",
			dce->req_hdr.opnum);
		status = KSMBD_RPC_EBAD_FUNC;
		break;
	}

	if (rpc_restricted_context(dce->rpc_req))
		status = KSMBD_RPC_EACCESS_DENIED;

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

int rpc_samr_read_request(struct ksmbd_rpc_pipe *pipe,
			    struct ksmbd_rpc_command *resp,
			    int max_resp_sz)
{
	return samr_return(pipe, resp, max_resp_sz);
}

int rpc_samr_write_request(struct ksmbd_rpc_pipe *pipe)
{
	return samr_invoke(pipe);
}

static void rpc_samr_add_domain_entry(char *name)
{
	g_ptr_array_add(domain_entries, g_strdup(name));
	num_domain_entries++;
}

void rpc_samr_init(void)
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

	if (!domain_entries) {
		domain_entries = g_ptr_array_new_with_free_func(g_free);
		rpc_samr_add_domain_entry(domain_name);
		rpc_samr_add_domain_entry("Builtin");
	}

	if (!ch_table)
		ch_table = g_hash_table_new(g_str_hash, g_str_equal);
}

static void samr_ch_clear_table(void)
{
	struct connect_handle *ch;
	GHashTableIter iter;

	g_rw_lock_writer_lock(&ch_table_lock);
	ghash_for_each(ch, ch_table, iter)
		g_free(ch);
	g_rw_lock_writer_unlock(&ch_table_lock);
}

void rpc_samr_destroy(void)
{
	if (ch_table) {
		samr_ch_clear_table();
		g_hash_table_destroy(ch_table);
		ch_table = NULL;
	}

	if (domain_entries) {
		g_ptr_array_free(domain_entries, 1);
		domain_entries = NULL;
	}

	num_domain_entries = 0;

	g_free(domain_name);
	domain_name = NULL;
}
