/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 */

#ifndef __VFS_CACHE_H__
#define __VFS_CACHE_H__

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

#define SMBD_START_FID		0
#define SMBD_NO_FID		(UINT_MAX)
#define SMB2_NO_FID		(0xFFFFFFFFFFFFFFFFULL)

#define FP_FILENAME(fp)		fp->filp->f_path.dentry->d_name.name
#define FP_INODE(fp)		fp->filp->f_path.dentry->d_inode
#define PARENT_INODE(fp)	fp->filp->f_path.dentry->d_parent->d_inode

#define ATTR_FP(fp) (fp->attrib_only && \
		(fp->cdoption != FILE_OVERWRITE_IF_LE && \
		fp->cdoption != FILE_OVERWRITE_LE && \
		fp->cdoption != FILE_SUPERSEDE_LE))

struct smbd_conn;
struct smbd_session;

struct smbd_lock {
	struct file_lock *fl;
	struct list_head glist;
	struct list_head llist;
	unsigned int flags;
	unsigned int cmd;
	int zero_len;
	unsigned long long start;
	unsigned long long end;
};

struct stream {
	char *name;
	int type;
	ssize_t size;
};

struct smbd_inode {
	rwlock_t			m_lock;
	atomic_t			m_count;
	atomic_t			op_count;
	struct inode			*m_inode;
	unsigned int			m_flags;
	struct hlist_node		m_hash;
	struct list_head		m_fp_list;
	struct list_head		m_op_list;
	struct oplock_info		*m_opinfo;
	__le32				m_fattr;
};

struct smbd_file {
	struct file			*filp;
	char				*filename;
	unsigned int			persistent_id;
	unsigned int			volatile_id;

	spinlock_t			f_lock;

	struct smbd_inode		*f_ci;
	struct smbd_inode		*f_parent_ci;
	struct oplock_info __rcu	*f_opinfo;
	struct smbd_conn		*conn;
	struct smbd_tree_connect	*tcon;

	atomic_t			refcount;
	__le32				daccess;
	__le32				saccess;
	__le32				coption;
	__le32				cdoption;
	__u64				create_time;

	bool				is_durable;
	bool				is_resilient;
	bool				is_persistent;
	bool				is_nt_open;
	bool				delete_on_close;
	bool				attrib_only;

	char				client_guid[16];
	char				create_guid[16];
	char				app_instance_id[16];

	struct stream			stream;
	struct list_head		node;
	struct list_head		blocked_works;

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
	struct smbd_readdir_data	readdir_data;
	int				dot_dotdot[2];
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
#define SMBD_NR_OPEN_DEFAULT BITS_PER_LONG

struct smbd_file_table {
	rwlock_t		lock;
	struct idr		*idr;
};

static inline bool HAS_FILE_ID(unsigned long long req)
{
	unsigned int id = (unsigned int)req;

	return id < SMBD_NO_FID;
}

static inline bool smbd_stream_fd(struct smbd_file *fp)
{
	return fp->stream.name != NULL;
}

int smbd_init_file_table(struct smbd_file_table *ft);
void smbd_destroy_file_table(struct smbd_file_table *ft);

int smbd_close_fd(struct smbd_work *work, unsigned int id);

struct smbd_file *smbd_lookup_fd_fast(struct smbd_work *work,
					unsigned int id);
struct smbd_file *smbd_lookup_foreign_fd(struct smbd_work *work,
					   unsigned int id);
struct smbd_file *smbd_lookup_fd_slow(struct smbd_work *work,
					unsigned int id,
					unsigned int pid);

void smbd_fd_put(struct smbd_work *work, struct smbd_file *fp);

int smbd_close_fd_app_id(struct smbd_work *work, char *app_id);
struct smbd_file *smbd_lookup_durable_fd(unsigned long long id);
struct smbd_file *smbd_lookup_fd_cguid(char *cguid);
struct smbd_file *smbd_lookup_fd_filename(struct smbd_work *work,
					    char *filename);
struct smbd_file *smbd_lookup_fd_inode(struct inode *inode);

unsigned int smbd_open_durable_fd(struct smbd_file *fp);

struct smbd_file *smbd_open_fd(struct smbd_work *work,
				 struct file *filp);

void smbd_close_tree_conn_fds(struct smbd_work *work);
void smbd_close_session_fds(struct smbd_work *work);

int smbd_close_inode_fds(struct smbd_work *work, struct inode *inode);

int smbd_reopen_durable_fd(struct smbd_work *work,
			    struct smbd_file *fp);

int smbd_init_global_file_table(void);
void smbd_free_global_file_table(void);

int smbd_file_table_flush(struct smbd_work *work);

void smbd_set_fd_limit(unsigned long limit);


/*
 * INODE hash
 */

int __init smbd_inode_hash_init(void);
void __exit smbd_release_inode_hash(void);

enum SMBD_INODE_STATUS {
	SMBD_INODE_STATUS_OK,
	SMBD_INODE_STATUS_UNKNOWN,
	SMBD_INODE_STATUS_PENDING_DELETE,
};

int smbd_query_inode_status(struct inode *inode);

bool smbd_inode_pending_delete(struct smbd_file *fp);
void smbd_set_inode_pending_delete(struct smbd_file *fp);
void smbd_clear_inode_pending_delete(struct smbd_file *fp);

void smbd_fd_set_delete_on_close(struct smbd_file *fp,
				  int file_info);
#endif /* __VFS_CACHE_H__ */
