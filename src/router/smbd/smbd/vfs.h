/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __KSMBD_VFS_H__
#define __KSMBD_VFS_H__

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <uapi/linux/xattr.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
static inline bool d_is_symlink(const struct dentry *dentry) {
	return dentry->d_inode->i_op->follow_link;
}
#endif
/* CREATION TIME XATTR PREFIX */
#define CREATION_TIME_PREFIX		"creation.time."
#define CREATION_TIME_PREFIX_LEN	(sizeof(CREATION_TIME_PREFIX) - 1)
#define CREATIOM_TIME_LEN		(sizeof(__u64))
#define XATTR_NAME_CREATION_TIME	\
				(XATTR_USER_PREFIX CREATION_TIME_PREFIX)
#define XATTR_NAME_CREATION_TIME_LEN	(sizeof(XATTR_NAME_CREATION_TIME) - 1)

/* STREAM XATTR PREFIX */
#define STREAM_PREFIX			"stream."
#define STREAM_PREFIX_LEN		(sizeof(STREAM_PREFIX) - 1)
#define XATTR_NAME_STREAM		(XATTR_USER_PREFIX STREAM_PREFIX)
#define XATTR_NAME_STREAM_LEN		(sizeof(XATTR_NAME_STREAM) - 1)

/* FILE ATTRIBUITE XATTR PREFIX */
#define FILE_ATTRIBUTE_PREFIX		"file.attribute."
#define FILE_ATTRIBUTE_PREFIX_LEN	(sizeof(FILE_ATTRIBUTE_PREFIX) - 1)
#define FILE_ATTRIBUTE_LEN		(sizeof(__u32))
#define XATTR_NAME_FILE_ATTRIBUTE	\
		(XATTR_USER_PREFIX FILE_ATTRIBUTE_PREFIX)
#define XATTR_NAME_FILE_ATTRIBUTE_LEN	\
		(sizeof(XATTR_USER_PREFIX FILE_ATTRIBUTE_PREFIX) - 1)

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

struct ksmbd_dir_info {
	const char	*name;
	char		*wptr;
	char		*rptr;
	int		name_len;
	int		out_buf_len;
	int		num_entry;
	int		data_count;
	int		last_entry_offset;
	bool		hide_dot_file;
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
	unsigned int	optimal_io_size;
};

int ksmbd_vfs_inode_permission(struct dentry *dentry, int acc_mode,
		bool delete);
int ksmbd_vfs_create(struct ksmbd_work *work, const char *name, umode_t mode);
int ksmbd_vfs_mkdir(struct ksmbd_work *work, const char *name, umode_t mode);
int ksmbd_vfs_read(struct ksmbd_work *work, struct ksmbd_file *fp,
		 size_t count, loff_t *pos);
int ksmbd_vfs_write(struct ksmbd_work *work, struct ksmbd_file *fp,
	char *buf, size_t count, loff_t *pos, bool sync, ssize_t *written);
int ksmbd_vfs_getattr(struct ksmbd_work *work, uint64_t fid,
		struct kstat *stat);
int ksmbd_vfs_setattr(struct ksmbd_work *work, const char *name,
		uint64_t fid, struct iattr *attrs);
int ksmbd_vfs_fsync(struct ksmbd_work *work, uint64_t fid, uint64_t p_id);
int ksmbd_vfs_remove_file(struct ksmbd_work *work, char *name);
int ksmbd_vfs_link(struct ksmbd_work *work,
		const char *oldname, const char *newname);
int ksmbd_vfs_symlink(struct ksmbd_work *work,
		const char *name, const char *symname);
int ksmbd_vfs_readlink(struct path *path, char *buf, int lenp);

int ksmbd_vfs_fp_rename(struct ksmbd_work *work, struct ksmbd_file *fp,
		char *newname);
int ksmbd_vfs_rename_slowpath(struct ksmbd_work *work,
		char *oldname, char *newname);

int ksmbd_vfs_truncate(struct ksmbd_work *work, const char *name,
	struct ksmbd_file *fp, loff_t size);

struct srv_copychunk;
int ksmbd_vfs_copy_file_ranges(struct ksmbd_work *work,
				struct ksmbd_file *src_fp,
				struct ksmbd_file *dst_fp,
				struct srv_copychunk *chunks,
				unsigned int chunk_count,
				unsigned int *chunk_count_written,
				unsigned int *chunk_size_written,
				loff_t  *total_size_written);

struct ksmbd_file *ksmbd_vfs_dentry_open(struct ksmbd_work *work,
					 const struct path *path,
					 int flags,
					 __le32 option,
					 int fexist);

ssize_t ksmbd_vfs_listxattr(struct dentry *dentry, char **list);
ssize_t ksmbd_vfs_getxattr(struct dentry *dentry,
			   char *xattr_name,
			   char **xattr_buf);

ssize_t ksmbd_vfs_casexattr_len(struct dentry *dentry,
				char *attr_name,
				int attr_name_len);

int ksmbd_vfs_setxattr(struct dentry *dentry,
		       const char *attr_name,
		       const void *attr_value,
		       size_t attr_size,
		       int flags);

int ksmbd_vfs_fsetxattr(struct ksmbd_work *work,
			const char *filename,
			const char *attr_name,
			const void *attr_value,
			size_t attr_size,
			int flags);

int ksmbd_vfs_xattr_stream_name(char *stream_name,
				char **xattr_stream_name,
				size_t *xattr_stream_name_size);

int ksmbd_vfs_truncate_xattr(struct dentry *dentry, int wo_streams);
int ksmbd_vfs_remove_xattr(struct dentry *dentry, char *attr_name);
void ksmbd_vfs_xattr_free(char *xattr);

int ksmbd_vfs_kern_path(char *name, unsigned int flags, struct path *path,
		bool caseless);
int ksmbd_vfs_empty_dir(struct ksmbd_file *fp);
void ksmbd_vfs_set_fadvise(struct file *filp, __le32 option);
int ksmbd_vfs_lock(struct file *filp, int cmd, struct file_lock *flock);
int ksmbd_vfs_readdir(struct file *file, struct ksmbd_readdir_data *rdata);
int ksmbd_vfs_alloc_size(struct ksmbd_work *work,
			 struct ksmbd_file *fp,
			 loff_t len);
int ksmbd_vfs_zero_data(struct ksmbd_work *work,
			 struct ksmbd_file *fp,
			 loff_t off,
			 loff_t len);

struct file_allocated_range_buffer;
int ksmbd_vfs_fiemap(struct ksmbd_file *fp, u64 start, u64 length,
			struct file_allocated_range_buffer *ranges,
			int in_count, int *out_count);
int ksmbd_vfs_unlink(struct dentry *dir, struct dentry *dentry);
unsigned short ksmbd_vfs_logical_sector_size(struct inode *inode);
void ksmbd_vfs_smb2_sector_size(struct inode *inode,
				struct ksmbd_fs_sector_size *fs_ss);
int ksmbd_vfs_readdir_name(struct ksmbd_work *work,
			   struct ksmbd_kstat *ksmbd_kstat,
			   const char *de_name,
			   int de_name_len,
			   const char *dir_path);
void *ksmbd_vfs_init_kstat(char **p, struct ksmbd_kstat *ksmbd_kstat);

int ksmbd_vfs_fill_dentry_attrs(struct ksmbd_work *work,
				struct dentry *dentry,
				struct ksmbd_kstat *ksmbd_kstat);

int ksmbd_vfs_posix_lock_wait(struct file_lock *flock);
int ksmbd_vfs_posix_lock_wait_timeout(struct file_lock *flock, long timeout);
void ksmbd_vfs_posix_lock_unblock(struct file_lock *flock);

#endif /* __KSMBD_VFS_H__ */
