/* SPDX-License-Identifier: LGPL-2.1+ */
/*
 *   Copyright (c) International Business Machines  Corp., 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *   Modified by Namjae Jeon (linkinjeon@gmail.com)
 */

#ifndef _SMBACL_H
#define _SMBACL_H

#define NUM_AUTHS (6)	/* number of authority fields */
#define SID_MAX_SUB_AUTHORITIES (15) /* max number of sub authority fields */

#define ACCESS_ALLOWED	0
#define ACCESS_DENIED	1

#define SIDOWNER 1
#define SIDGROUP 2

/* Revision for ACLs */
#define SD_REVISION	1

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

/* ACE types - see MS-DTYP 2.4.4.1 */
#define ACCESS_ALLOWED_ACE_TYPE 0x00
#define ACCESS_DENIED_ACE_TYPE  0x01
#define SYSTEM_AUDIT_ACE_TYPE   0x02
#define SYSTEM_ALARM_ACE_TYPE   0x03
#define ACCESS_ALLOWED_COMPOUND_ACE_TYPE 0x04
#define ACCESS_ALLOWED_OBJECT_ACE_TYPE  0x05
#define ACCESS_DENIED_OBJECT_ACE_TYPE   0x06
#define SYSTEM_AUDIT_OBJECT_ACE_TYPE    0x07
#define SYSTEM_ALARM_OBJECT_ACE_TYPE    0x08
#define ACCESS_ALLOWED_CALLBACK_ACE_TYPE 0x09
#define ACCESS_DENIED_CALLBACK_ACE_TYPE 0x0A
#define ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE 0x0B
#define ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE  0x0C
#define SYSTEM_AUDIT_CALLBACK_ACE_TYPE  0x0D
#define SYSTEM_ALARM_CALLBACK_ACE_TYPE  0x0E /* Reserved */
#define SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE 0x0F
#define SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE 0x10 /* reserved */
#define SYSTEM_MANDATORY_LABEL_ACE_TYPE 0x11
#define SYSTEM_RESOURCE_ATTRIBUTE_ACE_TYPE 0x12
#define SYSTEM_SCOPED_POLICY_ID_ACE_TYPE 0x13

struct smb_ntsd {
	__le16 revision; /* revision level */
	__le16 type;
	__le32 osidoffset;
	__le32 gsidoffset;
	__le32 sacloffset;
	__le32 dacloffset;
} __packed;

struct smb_sid {
	__u8 revision; /* revision level */
	__u8 num_subauth;
	__u8 authority[NUM_AUTHS];
	__le32 sub_auth[SID_MAX_SUB_AUTHORITIES]; /* sub_auth[num_subauth] */
} __packed;

struct smb_acl {
	__le16 revision; /* revision level */
	__le16 size;
	__le32 num_aces;
} __packed;

struct smb_ace {
	__u8 type;
	__u8 flags;
	__le16 size;
	__le32 access_req;
	struct smb_sid sid; /* ie UUID of user or group who gets these perms */
} __packed;

struct smb_fattr {
	kuid_t	cf_uid;
	kgid_t	cf_gid;
	umode_t	cf_mode;
};

int parse_sec_desc(struct smb_ntsd *pntsd, int acl_len,
		struct smb_fattr *fattr);
int build_sec_desc(struct smb_ntsd *pntsd, __u32 *secdesclen, umode_t mode);

#endif /* _SMBACL_H */
