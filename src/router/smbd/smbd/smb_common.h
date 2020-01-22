/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMB_COMMON_H__
#define __SMB_COMMON_H__

#include <linux/kernel.h>

#include "glob.h"
#include "nterr.h"
#include "smb2pdu.h"

/* ksmbd's Specific ERRNO */
#define ESHARE			50000

#define SMB1_PROT		0
#define SMB2_PROT		1
#define SMB21_PROT		2
/* multi-protocol negotiate request */
#define SMB2X_PROT		3
#define SMB30_PROT		4
#define SMB302_PROT		5
#define SMB311_PROT		6
#define BAD_PROT		0xFFFF

#define SMB1_VERSION_STRING	"1.0"
#define SMB20_VERSION_STRING	"2.0"
#define SMB21_VERSION_STRING	"2.1"
#define SMB30_VERSION_STRING	"3.0"
#define SMB302_VERSION_STRING	"3.02"
#define SMB311_VERSION_STRING	"3.1.1"

/* Dialects */
#define SMB10_PROT_ID		0x00
#define SMB20_PROT_ID		0x0202
#define SMB21_PROT_ID		0x0210
/* multi-protocol negotiate request */
#define SMB2X_PROT_ID		0x02FF
#define SMB30_PROT_ID		0x0300
#define SMB302_PROT_ID		0x0302
#define SMB311_PROT_ID		0x0311
#define BAD_PROT_ID		0xFFFF

#define SMB_ECHO_INTERVAL	(60*HZ)

#define CIFS_DEFAULT_IOSIZE	(64 * 1024)

extern struct list_head global_lock_list;

#define IS_SMB2(x)		((x)->vals->protocol_id != SMB10_PROT_ID)

#define HEADER_SIZE(conn)		((conn)->vals->header_size)
#define HEADER_SIZE_NO_BUF_LEN(conn)	((conn)->vals->header_size - 4)
#define MAX_HEADER_SIZE(conn)		((conn)->vals->max_header_size)

/* RFC 1002 session packet types */
#define RFC1002_SESSION_MESSAGE			0x00
#define RFC1002_SESSION_REQUEST			0x81
#define RFC1002_POSITIVE_SESSION_RESPONSE	0x82
#define RFC1002_NEGATIVE_SESSION_RESPONSE	0x83
#define RFC1002_RETARGET_SESSION_RESPONSE	0x84
#define RFC1002_SESSION_KEEP_ALIVE		0x85

/* Responses when opening a file. */
#define F_SUPERSEDED	0
#define F_OPENED	1
#define F_CREATED	2
#define F_OVERWRITTEN	3

/*
 * File Attribute flags
 */
#define ATTR_READONLY			0x0001
#define ATTR_HIDDEN			0x0002
#define ATTR_SYSTEM			0x0004
#define ATTR_VOLUME			0x0008
#define ATTR_DIRECTORY			0x0010
#define ATTR_ARCHIVE			0x0020
#define ATTR_DEVICE			0x0040
#define ATTR_NORMAL			0x0080
#define ATTR_TEMPORARY			0x0100
#define ATTR_SPARSE			0x0200
#define ATTR_REPARSE			0x0400
#define ATTR_COMPRESSED			0x0800
#define ATTR_OFFLINE			0x1000
#define ATTR_NOT_CONTENT_INDEXED	0x2000
#define ATTR_ENCRYPTED			0x4000
#define ATTR_POSIX_SEMANTICS		0x01000000
#define ATTR_BACKUP_SEMANTICS		0x02000000
#define ATTR_DELETE_ON_CLOSE		0x04000000
#define ATTR_SEQUENTIAL_SCAN		0x08000000
#define ATTR_RANDOM_ACCESS		0x10000000
#define ATTR_NO_BUFFERING		0x20000000
#define ATTR_WRITE_THROUGH		0x80000000

#define ATTR_READONLY_LE		cpu_to_le32(ATTR_READONLY)
#define ATTR_HIDDEN_LE			cpu_to_le32(ATTR_HIDDEN)
#define ATTR_SYSTEM_LE			cpu_to_le32(ATTR_SYSTEM)
#define ATTR_DIRECTORY_LE		cpu_to_le32(ATTR_DIRECTORY)
#define ATTR_ARCHIVE_LE			cpu_to_le32(ATTR_ARCHIVE)
#define ATTR_NORMAL_LE			cpu_to_le32(ATTR_NORMAL)
#define ATTR_TEMPORARY_LE		cpu_to_le32(ATTR_TEMPORARY)
#define ATTR_SPARSE_FILE_LE		cpu_to_le32(ATTR_SPARSE)
#define ATTR_REPARSE_POINT_LE		cpu_to_le32(ATTR_REPARSE)
#define ATTR_COMPRESSED_LE		cpu_to_le32(ATTR_COMPRESSED)
#define ATTR_OFFLINE_LE			cpu_to_le32(ATTR_OFFLINE)
#define ATTR_NOT_CONTENT_INDEXED_LE	cpu_to_le32(ATTR_NOT_CONTENT_INDEXED)
#define ATTR_ENCRYPTED_LE		cpu_to_le32(ATTR_ENCRYPTED)
#define ATTR_INTEGRITY_STREAML_LE	cpu_to_le32(0x00008000)
#define ATTR_NO_SCRUB_DATA_LE		cpu_to_le32(0x00020000)
#define ATTR_MASK_LE			cpu_to_le32(0x00007FB7)

#define SMB1_PROTO_NUMBER		cpu_to_le32(0x424d53ff)

#define SMB1_CLIENT_GUID_SIZE		(16)
struct smb_hdr {
	__be32 smb_buf_length;
	__u8 Protocol[4];
	__u8 Command;
	union {
		struct {
			__u8 ErrorClass;
			__u8 Reserved;
			__le16 Error;
		} __packed DosError;
		__le32 CifsError;
	} __packed Status;
	__u8 Flags;
	__le16 Flags2;          /* note: le */
	__le16 PidHigh;
	union {
		struct {
			__le32 SequenceNumber;  /* le */
			__u32 Reserved; /* zero */
		} __packed Sequence;
		__u8 SecuritySignature[8];      /* le */
	} __packed Signature;
	__u8 pad[2];
	__le16 Tid;
	__le16 Pid;
	__le16 Uid;
	__le16 Mid;
	__u8 WordCount;
} __packed;

struct smb_negotiate_req {
	struct smb_hdr hdr;     /* wct = 0 */
	__le16 ByteCount;
	unsigned char DialectsArray[1];
} __packed;

struct smb_negotiate_rsp {
	struct smb_hdr hdr;     /* wct = 17 */
	__le16 DialectIndex; /* 0xFFFF = no dialect acceptable */
	__u8 SecurityMode;
	__le16 MaxMpxCount;
	__le16 MaxNumberVcs;
	__le32 MaxBufferSize;
	__le32 MaxRawSize;
	__le32 SessionKey;
	__le32 Capabilities;    /* see below */
	__le32 SystemTimeLow;
	__le32 SystemTimeHigh;
	__le16 ServerTimeZone;
	__u8 EncryptionKeyLength;
	__u16 ByteCount;
	union {
		unsigned char EncryptionKey[8]; /* cap extended security off */
		/* followed by Domain name - if extended security is off */
		/* followed by 16 bytes of server GUID */
		/* then security blob if cap_extended_security negotiated */
		struct {
			unsigned char GUID[SMB1_CLIENT_GUID_SIZE];
			unsigned char SecurityBlob[1];
		} __packed extended_response;
	} __packed u;
} __packed;

struct filesystem_attribute_info {
	__le32 Attributes;
	__le32 MaxPathNameComponentLength;
	__le32 FileSystemNameLen;
	__le16 FileSystemName[1]; /* do not have to save this - get subset? */
} __packed;

struct filesystem_device_info {
	__le32 DeviceType;
	__le32 DeviceCharacteristics;
} __packed; /* device info level 0x104 */

struct filesystem_vol_info {
	__le64 VolumeCreationTime;
	__le32 SerialNumber;
	__le32 VolumeLabelSize;
	__le16 Reserved;
	__le16 VolumeLabel[1];
} __packed;

struct filesystem_info {
	__le64 TotalAllocationUnits;
	__le64 FreeAllocationUnits;
	__le32 SectorsPerAllocationUnit;
	__le32 BytesPerSector;
} __packed;     /* size info, level 0x103 */

#define EXTENDED_INFO_MAGIC 0x43667364	/* Cfsd */
#define STRING_LENGTH 28

struct fs_extended_info {
	__le32 magic;
	__le32 version;
	__le32 release;
	__u64 rel_date;
	char    version_string[STRING_LENGTH];
} __packed;

struct object_id_info {
	char objid[16];
	struct fs_extended_info extended_info;
} __packed;

struct file_directory_info {
	__le32 NextEntryOffset;
	__u32 FileIndex;
	__le64 CreationTime;
	__le64 LastAccessTime;
	__le64 LastWriteTime;
	__le64 ChangeTime;
	__le64 EndOfFile;
	__le64 AllocationSize;
	__le32 ExtFileAttributes;
	__le32 FileNameLength;
	char FileName[1];
} __packed;   /* level 0x101 FF resp data */

struct file_names_info {
	__le32 NextEntryOffset;
	__u32 FileIndex;
	__le32 FileNameLength;
	char FileName[1];
} __packed;   /* level 0xc FF resp data */

struct file_full_directory_info {
	__le32 NextEntryOffset;
	__u32 FileIndex;
	__le64 CreationTime;
	__le64 LastAccessTime;
	__le64 LastWriteTime;
	__le64 ChangeTime;
	__le64 EndOfFile;
	__le64 AllocationSize;
	__le32 ExtFileAttributes;
	__le32 FileNameLength;
	__le32 EaSize;
	char FileName[1];
} __packed; /* level 0x102 FF resp */

struct file_both_directory_info {
	__le32 NextEntryOffset;
	__u32 FileIndex;
	__le64 CreationTime;
	__le64 LastAccessTime;
	__le64 LastWriteTime;
	__le64 ChangeTime;
	__le64 EndOfFile;
	__le64 AllocationSize;
	__le32 ExtFileAttributes;
	__le32 FileNameLength;
	__le32 EaSize; /* length of the xattrs */
	__u8   ShortNameLength;
	__u8   Reserved;
	__u8   ShortName[24];
	char FileName[1];
} __packed; /* level 0x104 FFrsp data */

struct file_id_both_directory_info {
	__le32 NextEntryOffset;
	__u32 FileIndex;
	__le64 CreationTime;
	__le64 LastAccessTime;
	__le64 LastWriteTime;
	__le64 ChangeTime;
	__le64 EndOfFile;
	__le64 AllocationSize;
	__le32 ExtFileAttributes;
	__le32 FileNameLength;
	__le32 EaSize; /* length of the xattrs */
	__u8   ShortNameLength;
	__u8   Reserved;
	__u8   ShortName[24];
	__le16 Reserved2;
	__le64 UniqueId;
	char FileName[1];
} __packed;

struct file_id_full_dir_info {
	__le32 NextEntryOffset;
	__u32 FileIndex;
	__le64 CreationTime;
	__le64 LastAccessTime;
	__le64 LastWriteTime;
	__le64 ChangeTime;
	__le64 EndOfFile;
	__le64 AllocationSize;
	__le32 ExtFileAttributes;
	__le32 FileNameLength;
	__le32 EaSize; /* EA size */
	__le32 Reserved;
	__le64 UniqueId; /* inode num - le since Samba puts ino in low 32 bit*/
	char FileName[1];
} __packed; /* level 0x105 FF rsp data */

struct smb_version_values {
	char		*version_string;
	__u16		protocol_id;
	__le16		lock_cmd;
	__u32		capabilities;
	__u32		max_read_size;
	__u32		max_write_size;
	__u32		max_trans_size;
	__u32		large_lock_type;
	__u32		exclusive_lock_type;
	__u32		shared_lock_type;
	__u32		unlock_lock_type;
	size_t		header_size;
	size_t		max_header_size;
	size_t		read_rsp_size;
	unsigned int	cap_unix;
	unsigned int	cap_nt_find;
	unsigned int	cap_large_files;
	__u16		signing_enabled;
	__u16		signing_required;
	size_t		create_lease_size;
	size_t		create_durable_size;
	size_t		create_durable_v2_size;
	size_t		create_mxac_size;
	size_t		create_disk_id_size;
};

struct smb_version_ops {
	int (*get_cmd_val)(struct ksmbd_work *swork);
	int (*init_rsp_hdr)(struct ksmbd_work *swork);
	void (*set_rsp_status)(struct ksmbd_work *swork, __le32 err);
	int (*allocate_rsp_buf)(struct ksmbd_work *work);
	int (*check_user_session)(struct ksmbd_work *work);
	int (*get_ksmbd_tcon)(struct ksmbd_work *work);
	int (*is_sign_req)(struct ksmbd_work *work, unsigned int command);
	int (*check_sign_req)(struct ksmbd_work *work);
	void (*set_sign_rsp)(struct ksmbd_work *work);
	int (*generate_signingkey)(struct ksmbd_session *sess);
	int (*generate_encryptionkey)(struct ksmbd_session *sess);
	int (*is_transform_hdr)(void *buf);
	int (*decrypt_req)(struct ksmbd_work *work);
	int (*encrypt_resp)(struct ksmbd_work *work);
};

struct smb_version_cmds {
	int (*proc)(struct ksmbd_work *swork);
};



int ksmbd_min_protocol(void);
int ksmbd_max_protocol(void);

int ksmbd_lookup_protocol_idx(char *str);

int ksmbd_verify_smb_message(struct ksmbd_work *work);
bool ksmbd_smb_request(struct ksmbd_conn *conn);

int ksmbd_lookup_dialect_by_id(__le16 *cli_dialects, __le16 dialects_count);

int ksmbd_negotiate_smb_dialect(void *buf);
int ksmbd_init_smb_server(struct ksmbd_work *work);

bool ksmbd_pdu_size_has_room(unsigned int pdu);

struct ksmbd_kstat;
int ksmbd_populate_dot_dotdot_entries(struct ksmbd_conn *conn,
				      int info_level,
				      struct ksmbd_file *dir,
				      struct ksmbd_dir_info *d_info,
				      char *search_pattern,
				      int (*fn)(struct ksmbd_conn *,
						int,
						struct ksmbd_dir_info *,
						struct ksmbd_kstat *));

int ksmbd_extract_shortname(struct ksmbd_conn *conn,
			    const char *longname,
			    char *shortname);

void ksmbd_init_smb2_server_common(struct ksmbd_conn *conn);
int ksmbd_smb_negotiate_common(struct ksmbd_work *work, unsigned int command);

int ksmbd_smb_check_shared_mode(struct file *filp, struct ksmbd_file *curr_fp);

unsigned int ksmbd_small_buffer_size(void);
unsigned int ksmbd_server_side_copy_max_chunk_count(void);
unsigned int ksmbd_server_side_copy_max_chunk_size(void);
unsigned int ksmbd_server_side_copy_max_total_size(void);
bool is_asterisk(char *p);

static inline unsigned int get_rfc1002_len(void *buf)
{
	return be32_to_cpu(*((__be32 *)buf)) & 0xffffff;
}

static inline void inc_rfc1001_len(void *buf, int count)
{
	be32_add_cpu((__be32 *)buf, count);
}
#endif /* __SMB_COMMON_H__ */
