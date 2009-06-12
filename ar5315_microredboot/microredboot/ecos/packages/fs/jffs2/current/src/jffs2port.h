#ifndef __LINUX_JFFS2PORT_H__
#define __LINUX_JFFS2PORT_H__

/* $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/fs/jffs2/current/src/jffs2port.h#3 $ */

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
//#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>
//#include <pkgconf/fs_ram.h>

#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <stdlib.h>
#include <string.h>

#include <cyg/fileio/fileio.h>

#include <cyg/hal/drv_api.h>
#include <cyg/infra/diag.h>

#include <cyg/io/flash.h>

#include <pkgconf/fs_jffs2.h>
#include <linux/types.h>
#include <linux/list.h>
#include <asm/bug.h>
// Linux types
#define printf diag_printf

// Structures used by VFS

typedef unsigned short kdev_t;

struct qstr {
	const unsigned char * name;
	unsigned int len;
	unsigned int hash;
};

#define DNAME_INLINE_LEN 16

struct dentry {
	//	atomic_t d_count;
	unsigned int d_flags;
	struct inode  * d_inode;	/* Where the name belongs to - NULL is negative */
	struct dentry * d_parent;	/* parent directory */
	struct list_head d_hash;	/* lookup hash list */
	struct list_head d_child;	/* child of parent list */
	struct list_head d_subdirs;	/* our children */
	struct list_head d_alias;	/* inode alias list */
	struct qstr d_name;
	struct dentry_operations  *d_op;
	struct super_block * d_sb;	/* The root of the dentry tree */
	unsigned char d_iname[DNAME_INLINE_LEN]; /* small names */
};

struct file {
	struct dentry		*f_dentry;
	unsigned int 		f_flags;
	mode_t			    f_mode;
	loff_t			    f_pos;
	unsigned long 		f_reada, f_ramax, f_raend, f_ralen, f_rawin;
};

#define ATTR_MODE	1
#define ATTR_UID	2
#define ATTR_GID	4
#define ATTR_SIZE	8
#define ATTR_ATIME	16
#define ATTR_MTIME	32
#define ATTR_CTIME	64
#define ATTR_ATIME_SET	128
#define ATTR_MTIME_SET	256
#define ATTR_FORCE	512	/* Not a change, but a change it */
#define ATTR_ATTR_FLAG	1024

typedef unsigned short umode_t;

struct iattr {
        unsigned int ia_valid;
	umode_t		ia_mode;
	uid_t		ia_uid;
	gid_t		ia_gid;
	loff_t		ia_size;
	time_t		ia_atime;
	time_t		ia_mtime;
	time_t		ia_ctime;
};

struct page {
	unsigned long index;
	void *virtual;
};

struct nameidata {
	struct dentry *dentry;
	struct qstr last;
	unsigned int flags;
	int last_type;
};

struct file_operations {
	//struct module *owner;
	//loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char *, size_t, loff_t *);
	//ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
	int (*readdir) (struct file *, char *, int);
	//unsigned int (*poll) (struct file *, struct poll_table_struct *);
	int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
	//int (*mmap) (struct file *, struct vm_area_struct *);
	//int (*open) (struct inode *, struct file *);
	//int (*flush) (struct file *);
	//int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, struct dentry *, int datasync);
	//int (*fasync) (int, struct file *, int);
	//int (*lock) (struct file *, int, struct file_lock *);
	//ssize_t (*readv) (struct file *, const struct iovec *, unsigned long, loff_t *);
	//ssize_t (*writev) (struct file *, const struct iovec *, unsigned long, loff_t *);
};

struct inode_operations {
	int (*create) (struct inode *,struct dentry *,int);
	struct dentry * (*lookup) (struct inode *,struct dentry *);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*unlink) (struct inode *,struct dentry *);
	int (*symlink) (struct inode *,struct dentry *,const char *);
	int (*mkdir) (struct inode *,struct dentry *,int);
	int (*rmdir) (struct inode *,struct dentry *);
	int (*mknod) (struct inode *,struct dentry *,int,int);
	int (*rename) (struct inode *, struct dentry *,
			struct inode *, struct dentry *);
	int (*readlink) (struct dentry *, char *,int);
	int (*follow_link) (struct dentry *, struct nameidata *);
	//void (*truncate) (struct inode *);
	int (*permission) (struct inode *, int);
	//int (*revalidate) (struct dentry *);
	int (*setattr) (struct dentry *, struct iattr *);
	//int (*getattr) (struct dentry *, struct iattr *);
};


struct iovec {
        void *iov_base;
        ssize_t iov_len; 
};


// called by JFFS2

#define to_kdev_t(rdev) (rdev)
#define MAJOR(rdev) (rdev)>>8
#define MINOR(rdev) (rdev)

#define page_address(page)	((page)->virtual)

static __inline__ void * kmap(struct page * page) {
	return page_address(page);
}

#define kunmap(page) do { } while (0)

//struct page * read_cache_page(cyg_uint32 start, void * f, struct inode * i);
struct page *read_cache_page(unsigned long index, int (*filler)(void *,struct page*), void *data);
void page_cache_release(struct page * pg);

struct inode * new_inode(struct super_block *sb);
struct inode * iget(struct super_block *sb, cyg_uint32 ino);
void iput(struct inode * i);
void make_bad_inode(struct inode * inode);
int is_bad_inode(struct inode * inode);

#define insert_inode_hash(inode) do { } while (0)

#define d_alloc_root(root_inode) root_inode

#define flush_dcache_page(page) do { } while (0)

struct jffs2_sb_info;
struct jffs2_eraseblock;

cyg_bool jffs2_flash_read(struct jffs2_sb_info *c, cyg_uint32 read_buffer_offset, const size_t size, size_t * return_size, char * write_buffer);
cyg_bool jffs2_flash_write(struct jffs2_sb_info *c, cyg_uint32 write_buffer_offset, const size_t size, size_t * return_size, char * read_buffer);
int jffs2_flash_direct_writev(struct jffs2_sb_info *c, const struct iovec *vecs, unsigned long count, loff_t to, size_t *retlen);
cyg_bool jffs2_flash_erase(struct jffs2_sb_info *c, struct jffs2_eraseblock *jeb);


// calls to JFFS2

// dir-ecos.c
struct inode *jffs2_lookup(struct inode *dir_i, struct qstr *name);
int jffs2_readdir (struct inode *d_inode, unsigned long f_pos, char *nbuf, int nlen);
int jffs2_create(struct inode *dir_i, struct qstr *d_name, int mode, struct inode **new_i);
int jffs2_mkdir (struct inode *dir_i, struct qstr *d_name, int mode, struct inode **new_i);
int jffs2_link (struct inode *old_d_inode, struct inode *dir_i, struct qstr *d_name);
int jffs2_unlink(struct inode *dir_i, struct inode *d_inode, struct qstr *d_name);
int jffs2_rmdir (struct inode *dir_i, struct inode *d_inode, struct qstr *d_name);
int jffs2_rename (struct inode *old_dir_i, struct inode *d_inode, struct qstr *old_d_name,
                        struct inode *new_dir_i, struct qstr *new_d_name);

#define init_name_hash()		0
static inline unsigned long partial_name_hash(unsigned long c, unsigned long prevhash)
{
	prevhash = (prevhash << 4) | (prevhash >> (8*sizeof(unsigned long)-4));
	return prevhash ^ c;
}

static inline unsigned long end_name_hash(unsigned long hash)
{
	if (sizeof(hash) > sizeof(unsigned int))
		hash += hash >> 4*sizeof(hash);
	return (unsigned int) hash;
}

static inline unsigned int full_name_hash(const unsigned char * name, unsigned int len) {

	unsigned long hash = init_name_hash();
	while (len--)
		hash = partial_name_hash(*name++, hash);
	return end_name_hash(hash);
}

#endif /* __LINUX_JFFS2PORT_H__ */










