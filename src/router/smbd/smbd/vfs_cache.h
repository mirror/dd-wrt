/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 */

#ifndef __VFS_CACHE_H__
#define __VFS_CACHE_H__

#include <linux/version.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/rwsem.h>
#include <linux/spinlock.h>
#include <linux/idr.h>
#include <linux/workqueue.h>

#include "vfs.h"

/* Windows style file permissions for extended response */
#define	FILE_GENERIC_ALL	0x1F01FF
#define	FILE_GENERIC_READ	0x120089
#define	FILE_GENERIC_WRITE	0x120116
#define	FILE_GENERIC_EXECUTE	0X1200a0

#define KSMBD_START_FID		0
#define KSMBD_NO_FID		(INT_MAX)
#define SMB2_NO_FID		(0xFFFFFFFFFFFFFFFFULL)

#define FP_FILENAME(fp)		((fp)->filp->f_path.dentry->d_name.name)
#define PARENT_INODE(fp)	d_inode((fp)->filp->f_path.dentry->d_parent)

struct ksmbd_conn;
struct ksmbd_session;

struct ksmbd_lock {
	struct file_lock *fl;
	struct list_head clist;
	struct list_head flist;
	struct list_head llist;
	unsigned int flags;
	int cmd;
	int zero_len;
	unsigned long long start;
	unsigned long long end;
};

struct stream {
	char *name;
	ssize_t size;
};

struct ksmbd_inode {
	rwlock_t			m_lock;
	atomic_t			m_count;
	atomic_t			op_count;
	/* opinfo count for streams */
	atomic_t			sop_count;
	struct inode			*m_inode;
	unsigned int			m_flags;
	struct hlist_node		m_hash;
	struct list_head		m_fp_list;
	struct list_head		m_op_list;
	struct oplock_info		*m_opinfo;
	__le32				m_fattr;
};

enum {
	FP_NEW = 0,
	FP_INITED,
	FP_CLOSED
};

struct ksmbd_file {
	struct file			*filp;
	char				*filename;
	u64				persistent_id;
	u64				volatile_id;

	spinlock_t			f_lock;

	struct ksmbd_inode		*f_ci;
	struct ksmbd_inode		*f_parent_ci;
	struct oplock_info __rcu	*f_opinfo;
	struct ksmbd_conn		*conn;
	struct ksmbd_tree_connect	*tcon;

	atomic_t			refcount;
	__le32				daccess;
	__le32				saccess;
	__le32				coption;
	__le32				cdoption;
	__u64				create_time;
	__u64				itime;

	bool				is_nt_open;
	bool				attrib_only;

	char				client_guid[16];
	char				create_guid[16];
	char				app_instance_id[16];

	struct stream			stream;
	struct list_head		node;
	struct list_head		blocked_works;
	struct list_head		lock_list;

	int				durable_timeout;

#ifdef CONFIG_SMB_INSECURE_SERVER
	/* for SMB1 */
	int				pid;

	/* conflict lock fail count for SMB1 */
	unsigned int			cflock_cnt;
	/* last lock failure start offset for SMB1 */
	unsigned long long		llock_fstart;

	int				dirent_offset;
#endif
	/* if ls is happening on directory, below is valid*/
	struct ksmbd_readdir_data	readdir_data;
	int				dot_dotdot[2];
	unsigned int			f_state;
};

/*
 * Starting from 4.16 ->actor is not const anymore. The const prevents
 * the structure from being used as part of a kmalloc'd object as it
 * makes the compiler require that the actor member be set at object
 * initialisation time (or not at all).
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 11, 0)
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 16, 0)
static inline void set_ctx_actor(struct dir_context *ctx,
				 filldir_t actor)
{
	struct dir_context c = {
		.actor	= actor,
		.pos	= ctx->pos,
	};
	memcpy(ctx, &c, sizeof(struct dir_context));
}
#else
static inline void set_ctx_actor(struct dir_context *ctx,
				 filldir_t actor)
{
	ctx->actor = actor;
}
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static inline struct inode *d_inode(const struct dentry *dentry)
{
	return dentry->d_inode;
}
#endif
#define KSMBD_NR_OPEN_DEFAULT BITS_PER_LONG

struct ksmbd_file_table {
	rwlock_t		lock;
	struct idr		*idr;
};

static inline bool has_file_id(u64 id)
{
	return id < KSMBD_NO_FID;
}

static inline bool ksmbd_stream_fd(struct ksmbd_file *fp)
{
	return fp->stream.name != NULL;
}

static int ksmbd_init_file_table(struct ksmbd_file_table *ft);
static void ksmbd_destroy_file_table(struct ksmbd_file_table *ft);

static int ksmbd_close_fd(struct ksmbd_work *work, u64 id);
static struct ksmbd_file *ksmbd_lookup_fd_fast(struct ksmbd_work *work, u64 id);
static struct ksmbd_file *ksmbd_lookup_foreign_fd(struct ksmbd_work *work, u64 id);
static struct ksmbd_file *ksmbd_lookup_fd_slow(struct ksmbd_work *work, u64 id,
					u64 pid);

static void ksmbd_fd_put(struct ksmbd_work *work, struct ksmbd_file *fp);

static struct ksmbd_file *ksmbd_lookup_durable_fd(unsigned long long id);
static struct ksmbd_file *ksmbd_lookup_fd_cguid(char *cguid);
#ifdef CONFIG_SMB_INSECURE_SERVER
static struct ksmbd_file *ksmbd_lookup_fd_filename(struct ksmbd_work *work,
					    char *filename);
#endif
static struct ksmbd_file *ksmbd_lookup_fd_inode(struct inode *inode);

static unsigned int ksmbd_open_durable_fd(struct ksmbd_file *fp);

static struct ksmbd_file *ksmbd_open_fd(struct ksmbd_work *work,
				 struct file *filp);

static void ksmbd_close_tree_conn_fds(struct ksmbd_work *work);
static void ksmbd_close_session_fds(struct ksmbd_work *work);

static int ksmbd_close_inode_fds(struct ksmbd_work *work, struct inode *inode);

static int ksmbd_init_global_file_table(void);
static void ksmbd_free_global_file_table(void);

static int ksmbd_file_table_flush(struct ksmbd_work *work);

static void ksmbd_set_fd_limit(unsigned long limit);
static void ksmbd_update_fstate(struct ksmbd_file_table *ft, struct ksmbd_file *fp,
			 unsigned int state);


/*
 * INODE hash
 */

static int __init ksmbd_inode_hash_init(void);
static void ksmbd_release_inode_hash(void);

enum KSMBD_INODE_STATUS {
	KSMBD_INODE_STATUS_OK,
	KSMBD_INODE_STATUS_UNKNOWN,
	KSMBD_INODE_STATUS_PENDING_DELETE,
};

static int ksmbd_query_inode_status(struct inode *inode);
static bool ksmbd_inode_pending_delete(struct ksmbd_file *fp);
static void ksmbd_set_inode_pending_delete(struct ksmbd_file *fp);
static void ksmbd_clear_inode_pending_delete(struct ksmbd_file *fp);
static void ksmbd_fd_set_delete_on_close(struct ksmbd_file *fp,
				  int file_info);
#endif /* __VFS_CACHE_H__ */
