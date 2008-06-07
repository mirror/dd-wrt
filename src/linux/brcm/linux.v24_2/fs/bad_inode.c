/*
 *  linux/fs/bad_inode.c
 *
 *  Copyright (C) 1997, Stephen Tweedie
 *
 *  Provide stub functions for unreadable inodes
 */

#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/poll.h>

static loff_t bad_file_llseek(struct file *file, loff_t offset, int origin)
{
	return -EIO;
}

static ssize_t bad_file_read(struct file *filp, char __user *buf,
			size_t size, loff_t *ppos)
{
        return -EIO;
}

static ssize_t bad_file_write(struct file *filp, const char __user *buf,
			size_t siz, loff_t *ppos)
{
        return -EIO;
}

static int bad_file_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	return -EIO;
}

static unsigned int bad_file_poll(struct file *filp, poll_table *wait)
{
	return POLLERR;
}

static int bad_file_ioctl (struct inode *inode, struct file *filp,
			unsigned int cmd, unsigned long arg)
{
	return -EIO;
}

static int bad_file_mmap(struct file *file, struct vm_area_struct *vma)
{
	return -EIO;
}

static int bad_file_open(struct inode *inode, struct file *filp)
{
	return -EIO;
}

static int bad_file_flush(struct file *file)
{
	return -EIO;
}

static int bad_file_release(struct inode *inode, struct file *filp)
{
	return -EIO;
}

static int bad_file_fsync(struct file *file, struct dentry *dentry,
			int datasync)
{
	return -EIO;
}

static int bad_file_fasync(int fd, struct file *filp, int on)
{
	return -EIO;
}

static int bad_file_lock(struct file *file, int cmd, struct file_lock *fl)
{
	return -EIO;
}

/*
 * The follow_link operation is special: it must behave as a no-op
 * so that a bad root inode can at least be unmounted. To do this
 * we must dput() the base and return the dentry with a dget().
 */
static int bad_follow_link(struct dentry *dent, struct nameidata *nd)
{
	return vfs_follow_link(nd, ERR_PTR(-EIO));
}

static struct file_operations bad_file_ops =
{
	llseek:		bad_file_llseek,
	read:		bad_file_read,
	write:		bad_file_write,
	readdir:	bad_file_readdir,
	poll:		bad_file_poll,
	ioctl:		bad_file_ioctl,
	mmap:		bad_file_mmap,
	open:		bad_file_open,
	flush:		bad_file_flush,
	release:	bad_file_release,
	fsync:		bad_file_fsync,
	fasync:		bad_file_fasync,
	lock:		bad_file_lock,
};

static int bad_inode_create (struct inode *dir, struct dentry *dentry,
		int mode)
{
	return -EIO;
}
  
static struct dentry *bad_inode_lookup(struct inode *dir,
			struct dentry *dentry)
{
	return ERR_PTR(-EIO);
}

static int bad_inode_link (struct dentry *old_dentry, struct inode *dir,
		struct dentry *dentry)
{
	return -EIO;
}

static int bad_inode_unlink(struct inode *dir, struct dentry *dentry)
{
	return -EIO;
}

static int bad_inode_symlink (struct inode *dir, struct dentry *dentry,
		const char *symname)
{
	return -EIO;
}

static int bad_inode_mkdir(struct inode *dir, struct dentry *dentry,
			int mode)
{
	return -EIO;
}

static int bad_inode_rmdir (struct inode *dir, struct dentry *dentry)
{
	return -EIO;
}

static int bad_inode_mknod (struct inode *dir, struct dentry *dentry,
			int mode, int rdev)
{
	return -EIO;
}

static int bad_inode_rename (struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry)
{
	return -EIO;
}

static int bad_inode_readlink(struct dentry *dentry, char __user *buffer,
		int buflen)
{
	return -EIO;
}

static int bad_inode_permission(struct inode *inode, int mask)
{
	return -EIO;
}

static int bad_inode_revalidate(struct dentry *dentry)
{
	return -EIO;
}

struct inode_operations bad_inode_ops =
{
	create:		bad_inode_create,
	lookup:		bad_inode_lookup,
	link:		bad_inode_link,
	unlink:		bad_inode_unlink,
	symlink:	bad_inode_symlink,
	mkdir:		bad_inode_mkdir,
	rmdir:		bad_inode_rmdir,
	mknod:		bad_inode_mknod,
	rename:		bad_inode_rename,
	readlink:	bad_inode_readlink,
	follow_link:	bad_follow_link,
	/* truncate returns void */
	permission:	bad_inode_permission,
	revalidate:	bad_inode_revalidate,
};


/*
 * When a filesystem is unable to read an inode due to an I/O error in
 * its read_inode() function, it can call make_bad_inode() to return a
 * set of stubs which will return EIO errors as required. 
 *
 * We only need to do limited initialisation: all other fields are
 * preinitialised to zero automatically.
 */
 
/**
 *	make_bad_inode - mark an inode bad due to an I/O error
 *	@inode: Inode to mark bad
 *
 *	When an inode cannot be read due to a media or remote network
 *	failure this function makes the inode "bad" and causes I/O operations
 *	on it to fail from this point on.
 */
 
void make_bad_inode(struct inode * inode) 
{
	inode->i_mode = S_IFREG;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	inode->i_op = &bad_inode_ops;	
	inode->i_fop = &bad_file_ops;	
}

/*
 * This tests whether an inode has been flagged as bad. The test uses
 * &bad_inode_ops to cover the case of invalidated inodes as well as
 * those created by make_bad_inode() above.
 */
 
/**
 *	is_bad_inode - is an inode errored
 *	@inode: inode to test
 *
 *	Returns true if the inode in question has been marked as bad.
 */
 
int is_bad_inode(struct inode * inode) 
{
	return (inode->i_op == &bad_inode_ops);	
}
