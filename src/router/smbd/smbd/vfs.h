/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@kernel.org>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __KSMBD_VFS_H__
#define __KSMBD_VFS_H__

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <uapi/linux/xattr.h>
#include <linux/posix_acl.h>
#include <linux/unicode.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
static inline bool d_is_symlink(const struct dentry *dentry) {
	return dentry->d_inode->i_op->follow_link;
}
#endif
#include "smbacl.h"

/* STREAM XATTR PREFIX */
#define STREAM_PREFIX			"DosStream."
#define STREAM_PREFIX_LEN		(sizeof(STREAM_PREFIX) - 1)
#define XATTR_NAME_STREAM		(XATTR_USER_PREFIX STREAM_PREFIX)
#define XATTR_NAME_STREAM_LEN		(sizeof(XATTR_NAME_STREAM) - 1)

enum {
	XATTR_DOSINFO_ATTRIB		= 0x00000001,
	XATTR_DOSINFO_EA_SIZE		= 0x00000002,
	XATTR_DOSINFO_SIZE		= 0x00000004,
	XATTR_DOSINFO_ALLOC_SIZE	= 0x00000008,
	XATTR_DOSINFO_CREATE_TIME	= 0x00000010,
	XATTR_DOSINFO_CHANGE_TIME	= 0x00000020,
	XATTR_DOSINFO_ITIME		= 0x00000040
};

struct xattr_dos_attrib {
	__u16	version;
	__u32	flags;
	__u32	attr;
	__u32	ea_size;
	__u64	size;
	__u64	alloc_size;
	__u64	create_time;
	__u64	change_time;
	__u64	itime;
};

/* DOS ATTRIBUITE XATTR PREFIX */
#define DOS_ATTRIBUTE_PREFIX		"DOSATTRIB"
#define DOS_ATTRIBUTE_PREFIX_LEN	(sizeof(DOS_ATTRIBUTE_PREFIX) - 1)
#define XATTR_NAME_DOS_ATTRIBUTE	\
		(XATTR_USER_PREFIX DOS_ATTRIBUTE_PREFIX)
#define XATTR_NAME_DOS_ATTRIBUTE_LEN	\
		(sizeof(XATTR_USER_PREFIX DOS_ATTRIBUTE_PREFIX) - 1)

#define XATTR_SD_HASH_TYPE_SHA256	0x1
#define XATTR_SD_HASH_SIZE		64

#define SMB_ACL_READ			4
#define SMB_ACL_WRITE			2
#define SMB_ACL_EXECUTE			1

enum {
	SMB_ACL_TAG_INVALID = 0,
	SMB_ACL_USER,
	SMB_ACL_USER_OBJ,
	SMB_ACL_GROUP,
	SMB_ACL_GROUP_OBJ,
	SMB_ACL_OTHER,
	SMB_ACL_MASK
};

struct xattr_acl_entry {
	int type;
	uid_t uid;
	gid_t gid;
	mode_t perm;
};

struct xattr_smb_acl {
	int count;
	int next;
	struct xattr_acl_entry entries[0];
};

struct xattr_ntacl {
	__u16	version;
	void	*sd_buf;
	__u32	sd_size;
	__u16	hash_type;
	__u8	desc[10];
	__u16	desc_len;
	__u64	current_time;
	__u8	hash[XATTR_SD_HASH_SIZE];
	__u8	posix_acl_hash[XATTR_SD_HASH_SIZE];
};

/* SECURITY DESCRIPTOR XATTR PREFIX */
#define SD_PREFIX			"NTACL"
#define SD_PREFIX_LEN	(sizeof(SD_PREFIX) - 1)
#define XATTR_NAME_SD	\
		(XATTR_SECURITY_PREFIX SD_PREFIX)
#define XATTR_NAME_SD_LEN	\
		(sizeof(XATTR_SECURITY_PREFIX SD_PREFIX) - 1)

/*
 * Enumeration for stream type.
 */
enum {
	DATA_STREAM	= 1,	/* type $DATA */
	DIR_STREAM		/* type $INDEX_ALLOCATION */
};

/* CreateOptions */
/* Flag is set, it must not be a file , valid for directory only */
#define FILE_DIRECTORY_FILE_LE			cpu_to_le32(0x00000001)
#define FILE_WRITE_THROUGH_LE			cpu_to_le32(0x00000002)
#define FILE_SEQUENTIAL_ONLY_LE			cpu_to_le32(0x00000004)

/* Should not buffer on server*/
#define FILE_NO_INTERMEDIATE_BUFFERING_LE	cpu_to_le32(0x00000008)
/* MBZ */
#define FILE_SYNCHRONOUS_IO_ALERT_LE		cpu_to_le32(0x00000010)
/* MBZ */
#define FILE_SYNCHRONOUS_IO_NONALERT_LE		cpu_to_le32(0x00000020)

/* Flaf must not be set for directory */
#define FILE_NON_DIRECTORY_FILE_LE		cpu_to_le32(0x00000040)

/* Should be zero */
#define CREATE_TREE_CONNECTION			cpu_to_le32(0x00000080)
#define FILE_COMPLETE_IF_OPLOCKED_LE		cpu_to_le32(0x00000100)
#define FILE_NO_EA_KNOWLEDGE_LE			cpu_to_le32(0x00000200)
#define FILE_OPEN_REMOTE_INSTANCE		cpu_to_le32(0x00000400)

/**
 * Doc says this is obsolete "open for recovery" flag should be zero
 * in any case.
 */
#define CREATE_OPEN_FOR_RECOVERY		cpu_to_le32(0x00000400)
#define FILE_RANDOM_ACCESS_LE			cpu_to_le32(0x00000800)
#define FILE_DELETE_ON_CLOSE_LE			cpu_to_le32(0x00001000)
#define FILE_OPEN_BY_FILE_ID_LE			cpu_to_le32(0x00002000)
#define FILE_OPEN_FOR_BACKUP_INTENT_LE		cpu_to_le32(0x00004000)
#define FILE_NO_COMPRESSION_LE			cpu_to_le32(0x00008000)

/* Should be zero*/
#define FILE_OPEN_REQUIRING_OPLOCK		cpu_to_le32(0x00010000)
#define FILE_DISALLOW_EXCLUSIVE			cpu_to_le32(0x00020000)
#define FILE_RESERVE_OPFILTER_LE		cpu_to_le32(0x00100000)
#define FILE_OPEN_REPARSE_POINT_LE		cpu_to_le32(0x00200000)
#define FILE_OPEN_NO_RECALL_LE			cpu_to_le32(0x00400000)

/* Should be zero */
#define FILE_OPEN_FOR_FREE_SPACE_QUERY_LE	cpu_to_le32(0x00800000)
#define CREATE_OPTIONS_MASK			cpu_to_le32(0x00FFFFFF)
#define CREATE_OPTION_READONLY			0x10000000
/* system. NB not sent over wire */
#define CREATE_OPTION_SPECIAL			0x20000000

struct ksmbd_work;
struct ksmbd_file;
struct ksmbd_conn;

struct ksmbd_dir_info {
	const char	*name;
#ifdef CONFIG_SMB_INSECURE_SERVER
	char		*smb1_name;
#endif
	char		*wptr;
	char		*rptr;
	int		name_len;
	int		out_buf_len;
	int		num_entry;
	int		data_count;
	int		last_entry_offset;
	bool		hide_dot_file;
	int		flags;
	int		last_entry_off_align;
};

struct ksmbd_readdir_data {
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 11, 0)
	struct dir_context	ctx;
#else
	filldir_t filldir;
#endif
	union {
		void		*private;
		char		*dirent;
	};

	unsigned int		used;
	unsigned int		dirent_count;
	unsigned int		file_attr;
	struct unicode_map	*um;
};

/* ksmbd kstat wrapper to get valid create time when reading dir entry */
struct ksmbd_kstat {
	struct kstat		*kstat;
	unsigned long long	create_time;
	__le32			file_attributes;
};

struct ksmbd_fs_sector_size {
	unsigned short	logical_sector_size;
	unsigned int	physical_sector_size;
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
static inline struct user_namespace *mnt_user_ns(const struct vfsmount *mnt)
{
	return &init_user_ns;
}

static inline struct user_namespace *file_mnt_user_ns(struct file *file)
{
	return &init_user_ns;
}
#endif

static int ksmbd_vfs_inode_permission(struct dentry *dentry, int acc_mode,
		bool delete);
static int ksmbd_vfs_query_maximal_access(struct user_namespace *user_ns,
				   struct dentry *dentry, __le32 *daccess);
static int ksmbd_vfs_create(struct ksmbd_work *work, const char *name, umode_t mode);
static int ksmbd_vfs_mkdir(struct ksmbd_work *work, const char *name, umode_t mode);
static int ksmbd_vfs_read(struct ksmbd_work *work, struct ksmbd_file *fp, size_t count,
		   loff_t *pos, char *rbuf);
static int ksmbd_vfs_write(struct ksmbd_work *work, struct ksmbd_file *fp,
		char *buf, size_t count, loff_t *pos, bool sync,
		ssize_t *written);
static int ksmbd_vfs_fsync(struct ksmbd_work *work, u64 fid, u64 p_id);
static int ksmbd_vfs_remove_file(struct ksmbd_work *work, char *name);
static int ksmbd_vfs_link(struct ksmbd_work *work,
		const char *oldname, const char *newname);
static int ksmbd_vfs_getattr(struct path *path, struct kstat *stat);
#ifdef CONFIG_SMB_INSECURE_SERVER
static int ksmbd_vfs_setattr(struct ksmbd_work *work, const char *name,
		u64 fid, struct iattr *attrs);
static int ksmbd_vfs_symlink(struct ksmbd_work *work,
		const char *name, const char *symname);
static int ksmbd_vfs_readlink(struct path *path, char *buf, int lenp);
static int ksmbd_vfs_readdir_name(struct ksmbd_work *work,
			   struct user_namespace *user_ns,
			   struct ksmbd_kstat *ksmbd_kstat,
			   const char *de_name, int de_name_len,
			   const char *dir_path);
#endif
static int ksmbd_vfs_fp_rename(struct ksmbd_work *work, struct ksmbd_file *fp,
		char *newname);
static int ksmbd_vfs_truncate(struct ksmbd_work *work,
		struct ksmbd_file *fp, loff_t size);
struct srv_copychunk;
static int ksmbd_vfs_copy_file_ranges(struct ksmbd_work *work,
		struct ksmbd_file *src_fp, struct ksmbd_file *dst_fp,
		struct srv_copychunk *chunks, unsigned int chunk_count,
		unsigned int *chunk_count_written,
		unsigned int *chunk_size_written, loff_t  *total_size_written);
static struct ksmbd_file *ksmbd_vfs_dentry_open(struct ksmbd_work *work,
		const struct path *path, int flags, __le32 option, int fexist);
static ssize_t ksmbd_vfs_listxattr(struct dentry *dentry, char **list);
static ssize_t ksmbd_vfs_getxattr(struct user_namespace *user_ns,
			   struct dentry *dentry,
			   char *xattr_name,
		char **xattr_buf);
static ssize_t ksmbd_vfs_casexattr_len(struct user_namespace *user_ns,
				struct dentry *dentry, char *attr_name,
		int attr_name_len);
static int ksmbd_vfs_setxattr(struct user_namespace *user_ns,
		       struct dentry *dentry, const char *attr_name,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
		       void *attr_value, size_t attr_size, int flags);
#else
		const void *attr_value, size_t attr_size, int flags);
#endif
static int ksmbd_vfs_fsetxattr(struct ksmbd_work *work, const char *filename,
		const char *attr_name, const void *attr_value, size_t attr_size,
		int flags);
static int ksmbd_vfs_xattr_stream_name(char *stream_name, char **xattr_stream_name,
		size_t *xattr_stream_name_size, int s_type);
static int ksmbd_vfs_truncate_xattr(struct dentry *dentry, int wo_streams);
static int ksmbd_vfs_remove_xattr(struct user_namespace *user_ns,
			   struct dentry *dentry, char *attr_name);
static void ksmbd_vfs_xattr_free(char *xattr);
static int ksmbd_vfs_kern_path(struct ksmbd_work *work,
			char *name, unsigned int flags, struct path *path,
		bool caseless);
static struct dentry *ksmbd_vfs_kern_path_create(struct ksmbd_work *work,
					  const char *name,
					  unsigned int flags,
					  struct path *path);
static int ksmbd_vfs_empty_dir(struct ksmbd_file *fp);
static void ksmbd_vfs_set_fadvise(struct file *filp, __le32 option);
static int ksmbd_vfs_zero_data(struct ksmbd_work *work, struct ksmbd_file *fp,
		loff_t off, loff_t len);
struct file_allocated_range_buffer;
static int ksmbd_vfs_fqar_lseek(struct ksmbd_file *fp, loff_t start, loff_t length,
		struct file_allocated_range_buffer *ranges,
		unsigned int in_count, unsigned int *out_count);
static int ksmbd_vfs_unlink(struct user_namespace *user_ns,
		     struct dentry *dir, struct dentry *dentry);
static void *ksmbd_vfs_init_kstat(char **p, struct ksmbd_kstat *ksmbd_kstat);
static int ksmbd_vfs_fill_dentry_attrs(struct ksmbd_work *work,
				struct user_namespace *user_ns,
				struct dentry *dentry,
		struct ksmbd_kstat *ksmbd_kstat);
static void ksmbd_vfs_posix_lock_wait(struct file_lock *flock);
static int ksmbd_vfs_posix_lock_wait_timeout(struct file_lock *flock, long timeout);
static void ksmbd_vfs_posix_lock_unblock(struct file_lock *flock);
static int ksmbd_vfs_remove_acl_xattrs(struct user_namespace *user_ns,
				struct dentry *dentry);
static int ksmbd_vfs_remove_sd_xattrs(struct user_namespace *user_ns,
			       struct dentry *dentry);
static int ksmbd_vfs_set_sd_xattr(struct ksmbd_conn *conn,
			   struct user_namespace *user_ns,
			   struct dentry *dentry,
		struct smb_ntsd *pntsd, int len);
static int ksmbd_vfs_get_sd_xattr(struct ksmbd_conn *conn,
			   struct user_namespace *user_ns,
			   struct dentry *dentry,
		struct smb_ntsd **pntsd);
static int ksmbd_vfs_set_dos_attrib_xattr(struct user_namespace *user_ns,
				   struct dentry *dentry,
		struct xattr_dos_attrib *da);
static int ksmbd_vfs_get_dos_attrib_xattr(struct user_namespace *user_ns,
				   struct dentry *dentry,
		struct xattr_dos_attrib *da);
static struct posix_acl *ksmbd_vfs_posix_acl_alloc(int count, gfp_t flags);
static struct posix_acl *ksmbd_vfs_get_acl(struct inode *inode, int type);
static int ksmbd_vfs_set_posix_acl(struct inode *inode, int type,
		struct posix_acl *acl);
static int ksmbd_vfs_set_init_posix_acl(struct user_namespace *user_ns,
				 struct dentry *dentry);
static int ksmbd_vfs_inherit_posix_acl(struct user_namespace *user_ns,
				struct dentry *dentry,
		struct inode *parent_inode);
static void ksmbd_vfs_sector_size(struct inode *inode,
			   struct ksmbd_fs_sector_size *fs_ss);
#endif /* __KSMBD_VFS_H__ */
