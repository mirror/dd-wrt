// SPDX-License-Identifier: LGPL-2.1+
/*
 *   Copyright (c) International Business Machines  Corp., 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *   Author(s): Namjae Jeon (linkinjeon@kernel.org)
 */

#include <smbacl.h>
#include <tools.h>
#include <glib.h>
#include <rpc_lsarpc.h>

static const struct smb_sid sid_domain = {1, 1, {0, 0, 0, 0, 0, 5},
	{21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

/* security id for everyone/world system group */
static const struct smb_sid sid_everyone = {
	1, 1, {0, 0, 0, 0, 0, 1}, {0} };

/* S-1-22-1 Unmapped Unix users */
static const struct smb_sid sid_unix_users = {1, 1, {0, 0, 0, 0, 0, 22},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

/* S-1-22-2 Unmapped Unix groups */
static const struct smb_sid sid_unix_groups = { 1, 1, {0, 0, 0, 0, 0, 22},
	{2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

/* security id for local group */
static const struct smb_sid sid_local_group = {
	1, 1, {0, 0, 0, 0, 0, 5}, {32} };

int smb_read_sid(struct ksmbd_dcerpc *dce, struct smb_sid *sid)
{
	int i;

	if (ndr_read_int8(dce, &sid->revision))
		return -EINVAL;
	if (ndr_read_int8(dce, &sid->num_subauth))
		return -EINVAL;
	if (!sid->num_subauth || sid->num_subauth >= SID_MAX_SUB_AUTHORITIES)
		return -EINVAL;
	for (i = 0; i < NUM_AUTHS; ++i)
		if (ndr_read_int8(dce, &sid->authority[i]))
			return -EINVAL;
	for (i = 0; i < sid->num_subauth; ++i)
		if (ndr_read_int32(dce, &sid->sub_auth[i]))
			return -EINVAL;
	return 0;
}

int smb_write_sid(struct ksmbd_dcerpc *dce, const struct smb_sid *src)
{
	int i;

	if (ndr_write_int8(dce, src->revision))
		return -ENOMEM;

	if (ndr_write_int8(dce, src->num_subauth))
		return -ENOMEM;

	for (i = 0; i < NUM_AUTHS; ++i) {
		if (ndr_write_int8(dce, src->authority[i]))
			return -ENOMEM;
	}
	for (i = 0; i < src->num_subauth; ++i) {
		if (ndr_write_int32(dce, src->sub_auth[i]))
			return -ENOMEM;
	}

	return 0;
}

void smb_copy_sid(struct smb_sid *dst, const struct smb_sid *src)
{
	int i;

	dst->revision = src->revision;
	dst->num_subauth = src->num_subauth;
	for (i = 0; i < NUM_AUTHS; ++i)
		dst->authority[i] = src->authority[i];
	for (i = 0; i < dst->num_subauth; ++i)
		dst->sub_auth[i] = src->sub_auth[i];
}

void smb_init_domain_sid(struct smb_sid *sid)
{
	int i;

	memset(sid, 0, sizeof(struct smb_sid));
	sid->revision = 1;
	sid->num_subauth = 4;
	sid->authority[5] = 5;
	sid->sub_auth[0] = 21;
	for (i = 0; i < 3; i++)
		sid->sub_auth[i+1] = global_conf.gen_subauth[i];
}

int smb_compare_sids(const struct smb_sid *ctsid, const struct smb_sid *cwsid)
{
	int i;
	int num_subauth, num_sat, num_saw;

	if ((!ctsid) || (!cwsid))
		return 1;

	/* compare the revision */
	if (ctsid->revision != cwsid->revision) {
		if (ctsid->revision > cwsid->revision)
			return 1;
		return -1;
	}

	/* compare all of the six auth values */
	for (i = 0; i < NUM_AUTHS; ++i) {
		if (ctsid->authority[i] != cwsid->authority[i]) {
			if (ctsid->authority[i] > cwsid->authority[i])
				return 1;
			return -1;
		}
	}

	/* compare all of the subauth values if any */
	num_sat = ctsid->num_subauth;
	num_saw = cwsid->num_subauth;
	num_subauth = num_sat < num_saw ? num_sat : num_saw;
	if (num_subauth) {
		for (i = 0; i < num_subauth; ++i) {
			if (ctsid->sub_auth[i] != cwsid->sub_auth[i]) {
				if (ctsid->sub_auth[i] >
					cwsid->sub_auth[i])
					return 1;
				return -1;
			}
		}
	}

	return 0; /* sids compare/match */
}

static int smb_sid_to_string(char *domain, size_t domain_len,
			     struct smb_sid *sid)
{
	int i, len;

	len = snprintf(domain, domain_len, "S-%i-%i", (int)sid->revision,
		       (int)sid->authority[5]);

	if (len < 0 || len > domain_len)
		return -ENOMEM;

	for (i = 0; i < sid->num_subauth; i++) {
		len += snprintf(domain + len, domain_len - len, "-%u",
				sid->sub_auth[i]);
		if (len < 0 || len > domain_len)
			return -ENOMEM;
	}

	return len;
}

int set_domain_name(struct smb_sid *sid, char *domain, size_t domain_len,
		    int *type)
{
	int ret = 0;
	char domain_string[DOMAIN_STR_SIZE] = {0};
	g_autofree char *domain_name = NULL;

	if (!smb_compare_sids(sid, &sid_domain) &&
	    !memcmp(&sid->sub_auth[1], global_conf.gen_subauth,
		    sizeof(__u32) * 3)) {
		if (gethostname(domain_string, DOMAIN_STR_SIZE))
			return -ENOMEM;

		domain_name = g_ascii_strup(domain_string, -1);
		if (!domain_name)
			return -ENOMEM;

		ret = snprintf(domain, domain_len, "%s", domain_name);
		if (ret < 0 || ret >= domain_len)
			return -ENOMEM;

		*type = SID_TYPE_USER;
	} else if (!smb_compare_sids(sid, &sid_unix_users)) {
		ret = snprintf(domain, domain_len, "Unix User");
		if (ret < 0 || ret >= domain_len)
			return -ENOMEM;

		*type = SID_TYPE_USER;
	} else if (!smb_compare_sids(sid, &sid_unix_groups)) {
		ret = snprintf(domain, domain_len, "Unix Group");
		if (ret < 0 || ret >= domain_len)
			return -ENOMEM;

		*type = SID_TYPE_GROUP;
	} else {
		ret = smb_sid_to_string(domain_string, sizeof(domain_string),
					sid);
		if (ret < 0)
			return ret;

		if (ret > domain_len)
			return -ENOMEM;

		domain_name = g_ascii_strup(domain_string, -1);
		if (!domain_name)
			return -ENOMEM;

		ret = snprintf(domain, domain_len, "%s", domain_name);
		if (ret < 0 || ret >= domain_len)
			return -ENOMEM;

		*type = SID_TYPE_UNKNOWN;
		ret = -ENOENT;
	}
	return ret;
}

static int smb_set_ace(struct ksmbd_dcerpc *dce, int access_req, int rid,
		const struct smb_sid *rsid)
{
	int size;
	struct smb_sid sid = {0};

	memcpy(&sid, rsid, sizeof(struct smb_sid));
	// ace type
	if (ndr_write_int8(dce, ACCESS_ALLOWED))
		return -ENOMEM;

	// ace flags
	if (ndr_write_int8(dce, 0))
		return -ENOMEM;

	size = 1 + 1 + 2 + 4 + 1 + 1 + 6 + (sid.num_subauth * 4);
	if (rid)
		size += 4;

	// ace size
	if (ndr_write_int16(dce, size))
		return -ENOMEM;

	// ace access required
	if (ndr_write_int32(dce, access_req))
		return -ENOMEM;

	if (rid)
		sid.sub_auth[sid.num_subauth++] = rid;

	if (smb_write_sid(dce, &sid))
		return -ENOMEM;

	return size;
}

static int set_dacl(struct ksmbd_dcerpc *dce, int rid)
{
	int size = 0, i, ret;
	struct smb_sid owner_domain;

	/* Other */
	ret = smb_set_ace(dce, 0x0002035b, 0, &sid_everyone);
	if (ret < 0)
		return ret;
	size += ret;

	/* Local Group Administrators */
	ret = smb_set_ace(dce, 0x000f07ff, 544, &sid_local_group);
	if (ret < 0)
		return ret;
	size += ret;

	/* Local Group Account Operators */
	ret = smb_set_ace(dce, 0x000f07ff, 548, &sid_local_group);
	if (ret < 0)
		return ret;
	size += ret;

	/* Owner RID */
	memcpy(&owner_domain, &sid_domain, sizeof(struct smb_sid));
	for (i = 0; i < 3; ++i) {
		owner_domain.sub_auth[i + 1] = global_conf.gen_subauth[i];
		owner_domain.num_subauth++;
	}

	ret = smb_set_ace(dce, 0x00020044, rid, &owner_domain);
	if (ret < 0)
		return ret;
	size += ret;

	return size;
}

int build_sec_desc(struct ksmbd_dcerpc *dce, __u32 *secdesclen, int rid)
{
	int l_offset, acl_size_offset;
	int acl_size;

	/* NT Security Descriptor : Revision */
	if (ndr_write_int16(dce, 1))
		return -ENOMEM;

	/* ACL Type */
	if (ndr_write_int16(dce, SELF_RELATIVE | DACL_PRESENT))
		return -ENOMEM;

	/* Offset to owner SID */
	if (ndr_write_int32(dce, 0))
		return -ENOMEM;

	/* Offset to group SID */
	if (ndr_write_int32(dce, 0))
		return -ENOMEM;

	/* Offset to SACL */
	if (ndr_write_int32(dce, 0))
		return -ENOMEM;

	/* Offset to DACL */
	if (ndr_write_int32(dce, sizeof(struct smb_ntsd)))
		return -ENOMEM;

	/* DACL Revision */
	if (ndr_write_int16(dce, 2))
		return -ENOMEM;

	acl_size_offset = dce->offset;
	dce->offset += 2;

	/* Number of ACEs */
	if (ndr_write_int32(dce, 4))
		return -ENOMEM;

	acl_size = set_dacl(dce, rid) + sizeof(struct smb_acl);
	if (acl_size < 0)
		return -ENOMEM;
	/* ACL Size */
	l_offset = dce->offset;
	dce->offset = acl_size_offset;
	if (ndr_write_int16(dce, acl_size))
		return -ENOMEM;
	dce->offset = l_offset;

	*secdesclen = sizeof(struct smb_ntsd) + acl_size;
	return 0;
}
