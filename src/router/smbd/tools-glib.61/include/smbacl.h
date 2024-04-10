/* SPDX-License-Identifier: LGPL-2.1+ */
/*
 *   Copyright (c) International Business Machines  Corp., 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *   Author(s): Namjae Jeon (linkinjeon@kernel.org)
 */

#ifndef __KSMBD_SMBACL_H__
#define __KSMBD_SMBACL_H__

#include <linux/types.h>
#include <glib.h>
#include <rpc.h>

#define NUM_AUTHS (6)	/* number of authority fields */
#define SID_MAX_SUB_AUTHORITIES (15) /* max number of sub authority fields */

#define ACCESS_ALLOWED	0
#define ACCESS_DENIED	1

/* Control flags for Security Descriptor */
#define OWNER_DEFAULTED		0x0001
#define GROUP_DEFAULTED		0x0002
#define DACL_PRESENT		0x0004
#define DACL_DEFAULTED		0x0008
#define SACL_PRESENT		0x0010
#define SACL_DEFAULTED		0x0020
#define DACL_TRUSTED		0x0040
#define SERVER_SECURITY		0x0080
#define DACL_AUTO_INHERIT_REQ	0x0100
#define SACL_AUTO_INHERIT_REQ	0x0200
#define DACL_AUTO_INHERITED	0x0400
#define SACL_AUTO_INHERITED	0x0800
#define DACL_PROTECTED		0x1000
#define SACL_PROTECTED		0x2000
#define RM_CONTROL_VALID	0x4000
#define SELF_RELATIVE		0x8000

#define SID_TYPE_USER		1
#define SID_TYPE_GROUP		2
#define SID_TYPE_UNKNOWN	8

struct smb_ntsd {
	__u16 revision; /* revision level */
	__u16 type;
	__u32 osidoffset;
	__u32 gsidoffset;
	__u32 sacloffset;
	__u32 dacloffset;
};

struct smb_sid {
	__u8 revision; /* revision level */
	__u8 num_subauth;
	__u8 authority[NUM_AUTHS];
	__u32 sub_auth[SID_MAX_SUB_AUTHORITIES]; /* sub_auth[num_subauth] */
};

struct smb_acl {
	__u16 revision; /* revision level */
	__u16 size;
	__u32 num_aces;
};

struct smb_ace {
	__u8 type;
	__u8 flags;
	__u16 size;
	__u32 access_req;
	struct smb_sid sid; /* ie UUID of user or group who gets these perms */
};

void smb_init_domain_sid(struct smb_sid *sid);
int smb_read_sid(struct ksmbd_dcerpc *dce, struct smb_sid *sid);
int smb_write_sid(struct ksmbd_dcerpc *dce, const struct smb_sid *src);
void smb_copy_sid(struct smb_sid *dst, const struct smb_sid *src);
int smb_compare_sids(const struct smb_sid *ctsid, const struct smb_sid *cwsid);
int build_sec_desc(struct ksmbd_dcerpc *dce, __u32 *secdesclen, int rid);
int set_domain_name(struct smb_sid *sid, char *domain, size_t domain_len, int *type);
#endif /* __KSMBD_SMBACL_H__ */
