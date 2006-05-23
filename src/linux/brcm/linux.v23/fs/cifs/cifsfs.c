/*
 *   fs/cifs/cifsfs.c
 *
 *   Copyright (C) International Business Machines  Corp., 2002,2004
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Common Internet FileSystem (CIFS) client
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Note that BB means BUGBUG (ie something to fix eventually) */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/list.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,15))
#include <linux/seq_file.h>
#endif
#include <linux/vfs.h>
#include "cifsfs.h"
#include "cifspdu.h"
#define DECLARE_GLOBALS_HERE
#include "cifsglob.h"
#include "cifsproto.h"
#include "cifs_debug.h"
#include "cifs_fs_sb.h"
#include <linux/mm.h>
#define CIFS_MAGIC_NUMBER 0xFF534D42	/* the first four bytes of SMB PDUs */
/* BB when mempool_resize is added back in, we will resize pool on new mount */
#define CIFS_MIN_RCV_POOL 11 /* enough for progress to five servers */

#ifdef CONFIG_CIFS_QUOTA
static struct quotactl_ops cifs_quotactl_ops;
#endif

int cifsFYI = 0;
int cifsERROR = 1;
int traceSMB = 0;
unsigned int oplockEnabled = 1;
unsigned int quotaEnabled = 0;
unsigned int linuxExtEnabled = 1;
unsigned int lookupCacheEnabled = 1;
unsigned int multiuser_mount = 0;
unsigned int extended_security = 0;
unsigned int ntlmv2_support = 0;
unsigned int sign_CIFS_PDUs = 1;
unsigned int CIFSMaximumBufferSize = CIFS_MAX_MSGSIZE;
struct task_struct * oplockThread = NULL;

extern int cifs_mount(struct super_block *, struct cifs_sb_info *, char *,
			const char *);
extern int cifs_umount(struct super_block *, struct cifs_sb_info *);
void cifs_proc_init(void);
void cifs_proc_clean(void);

static DECLARE_COMPLETION(cifs_oplock_exited);


struct super_block *
cifs_read_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct cifs_sb_info *cifs_sb;
	int rc = 0;

	sb->s_flags |= MS_NODIRATIME; /* and probably even noatime */
	cifs_sb = CIFS_SB(sb);
	if(cifs_sb == NULL)
		return 0;
	else
		memset(cifs_sb,0,sizeof(struct cifs_sb_info));
	

	rc = cifs_mount(sb, cifs_sb, data, NULL);

	if (rc) {
		if (!silent)
			cERROR(1,
			       ("cifs_mount failed w/return code = %d", rc));
		goto out_mount_failed;
	}

	sb->s_magic = CIFS_MAGIC_NUMBER;
	sb->s_op = &cifs_super_ops;
/*	if(cifs_sb->tcon->ses->server->maxBuf > MAX_CIFS_HDR_SIZE + 512)
	    sb->s_blocksize = cifs_sb->tcon->ses->server->maxBuf - MAX_CIFS_HDR_SIZE; */
#ifdef CONFIG_CIFS_QUOTA
	sb->s_qcop = &cifs_quotactl_ops;
#endif
	sb->s_blocksize = CIFS_MAX_MSGSIZE;
	sb->s_blocksize_bits = 14;	/* default 2**14 = CIFS_MAX_MSGSIZE */
	inode = iget(sb, ROOT_I);

	if (!inode) {
		goto out_no_root;
	}

	sb->s_root = d_alloc_root(inode);

	if (!sb->s_root) {
		goto out_no_root;
	}

	return sb;

out_no_root:
	cERROR(1, ("cifs_read_super: get root inode failed"));
	if (inode)
		iput(inode);

out_mount_failed:
	if(cifs_sb->local_nls)
		unload_nls(cifs_sb->local_nls);
	sb->s_dev = 0;
	return 0;
}

static void
cifs_put_super(struct super_block *sb)
{
	int rc = 0;
	struct cifs_sb_info *cifs_sb;

	cFYI(1, ("In cifs_put_super"));
	cifs_sb = CIFS_SB(sb);
	if(cifs_sb == NULL) {
		cFYI(1,("Empty cifs superblock info passed to unmount"));
		return;
	}
	rc = cifs_umount(sb, cifs_sb); 
	if (rc) {
		cERROR(1, ("cifs_umount failed with return code %d", rc));
	}
	unload_nls(cifs_sb->local_nls);
	return;
}

static int
cifs_statfs(struct super_block *sb, struct statfs *buf)
{
	int xid, rc;
	struct cifs_sb_info *cifs_sb;
	struct cifsTconInfo *pTcon;

	xid = GetXid();

	cifs_sb = CIFS_SB(sb);
	pTcon = cifs_sb->tcon;

	buf->f_type = CIFS_MAGIC_NUMBER;

	/* instead could get the real value via SMB_QUERY_FS_ATTRIBUTE_INFO */
	buf->f_namelen = PATH_MAX;	/* PATH_MAX may be too long - it would presumably
					   be length of total path, note that some servers may be 
					   able to support more than this, but best to be safe
					   since Win2k and others can not handle very long filenames */
	buf->f_files = 0;	/* undefined */
	buf->f_ffree = 0;	/* unlimited */

	rc = CIFSSMBQFSInfo(xid, pTcon, buf, cifs_sb->local_nls);

	/*     
	   int f_type;
	   __fsid_t f_fsid;
	   int f_namelen;  */
	/* BB get from info put in tcon struct at mount time with call to QFSAttrInfo */
	FreeXid(xid);
	return 0;		/* always return success? what if volume is no longer available? */
}

static int cifs_permission(struct inode * inode, int mask)
{
	/* the server does permission checks, we do not need to do it here */
	return 0;
}

kmem_cache_t *cifs_req_cachep;
kmem_cache_t *cifs_mid_cachep;
kmem_cache_t *cifs_oplock_cachep;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0))
static struct inode *
cifs_alloc_inode(struct super_block *sb)
{
        struct cifsInodeInfo *cifs_inode;
        cifs_inode =
            (struct cifsInodeInfo *) kmem_cache_alloc(cifs_inode_cachep,
                                                      SLAB_KERNEL);
        if (!cifs_inode)
                return NULL;
        cifs_inode->cifsAttrs = 0x20;   /* default */
        atomic_set(&cifs_inode->inUse, 0);
        cifs_inode->time = 0;
        /* Until the file is open and we have gotten oplock
        info back from the server, can not assume caching of
        file data or metadata */
        cifs_inode->clientCanCacheRead = FALSE;
        cifs_inode->clientCanCacheAll = FALSE;
        INIT_LIST_HEAD(&cifs_inode->openFileList);
        return &cifs_inode->vfs_inode;
}

static void
cifs_destroy_inode(struct inode *inode)
{
	kmem_cache_free(cifs_inode_cachep, CIFS_I(inode));
}
#endif

/*
 * cifs_show_options() is for displaying mount options in /proc/mounts.
 * Not all settable options are displayed but most of the important
 * ones are.
 */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,15))
static int
cifs_show_options(struct seq_file *s, struct vfsmount *m)
{
	struct cifs_sb_info *cifs_sb;

	cifs_sb = CIFS_SB(m->mnt_sb);

	if (cifs_sb) {
		if (cifs_sb->tcon) {
			seq_printf(s, ",unc=%s", cifs_sb->tcon->treeName);
			if ((cifs_sb->tcon->ses) && (cifs_sb->tcon->ses->userName))
				seq_printf(s, ",username=%s",
					   cifs_sb->tcon->ses->userName);
			if(cifs_sb->tcon->ses->domainName)
				seq_printf(s, ",domain=%s",
					cifs_sb->tcon->ses->domainName);
		}
		seq_printf(s, ",rsize=%d",cifs_sb->rsize);
		seq_printf(s, ",wsize=%d",cifs_sb->wsize);
	}
	return 0;
}
#endif

#ifdef CONFIG_CIFS_QUOTA
int cifs_xquota_set(struct super_block * sb, int quota_type, qid_t qid,
		struct fs_disk_quota * pdquota)
{
	int xid;
	int rc = 0;
	struct cifs_sb_info *cifs_sb = CIFS_SB(sb);
	struct cifsTconInfo *pTcon;
	
	if(cifs_sb)
		pTcon = cifs_sb->tcon;
	else
		return -EIO;


	xid = GetXid();
	if(pTcon) {
		cFYI(1,("set type: 0x%x id: %d",quota_type,qid));		
	} else {
		return -EIO;
	}

	FreeXid(xid);
	return rc;
}

int cifs_xquota_get(struct super_block * sb, int quota_type, qid_t qid,
                struct fs_disk_quota * pdquota)
{
	int xid;
	int rc = 0;
	struct cifs_sb_info *cifs_sb = CIFS_SB(sb);
	struct cifsTconInfo *pTcon;

	if(cifs_sb)
		pTcon = cifs_sb->tcon;
	else
		return -EIO;

	xid = GetXid();
	if(pTcon) {
                cFYI(1,("set type: 0x%x id: %d",quota_type,qid));
	} else {
		rc = -EIO;
	}

	FreeXid(xid);
	return rc;
}

int cifs_xstate_set(struct super_block * sb, unsigned int flags, int operation)
{
	int xid; 
	int rc = 0;
	struct cifs_sb_info *cifs_sb = CIFS_SB(sb);
	struct cifsTconInfo *pTcon;

	if(cifs_sb)
		pTcon = cifs_sb->tcon;
	else
		return -EIO;

	xid = GetXid();
	if(pTcon) {
                cFYI(1,("flags: 0x%x operation: 0x%x",flags,operation));
	} else {
		rc = -EIO;
	}

	FreeXid(xid);
	return rc;
}

int cifs_xstate_get(struct super_block * sb, struct fs_quota_stat *qstats)
{
	int xid;
	int rc = 0;
	struct cifs_sb_info *cifs_sb = CIFS_SB(sb);
	struct cifsTconInfo *pTcon;

	if(cifs_sb) {
		pTcon = cifs_sb->tcon;
	} else {
		return -EIO;
	}
	xid = GetXid();
	if(pTcon) {
		cFYI(1,("pqstats %p",qstats));		
	} else {
		rc = -EIO;
	}

	FreeXid(xid);
	return rc;
}

static struct quotactl_ops cifs_quotactl_ops = {
	.set_xquota	= cifs_xquota_set,
	.get_xquota	= cifs_xquota_set,
	.set_xstate	= cifs_xstate_set,
	.get_xstate	= cifs_xstate_get,
};
#endif

static int cifs_remount(struct super_block *sb, int *flags, char *data)
{
	*flags |= MS_NODIRATIME;
	return 0;
}

struct super_operations cifs_super_ops = {
	.read_inode = cifs_read_inode,
	.put_super = cifs_put_super,
	.statfs = cifs_statfs,
/*	.alloc_inode = cifs_alloc_inode,
	.destroy_inode = cifs_destroy_inode, */
/*	.drop_inode	    = generic_delete_inode, 
	.delete_inode	= cifs_delete_inode,  *//* Do not need the above two functions     
   unless later we add lazy close of inodes or unless the kernel forgets to call
   us with the same number of releases (closes) as opens */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,15))
	.show_options = cifs_show_options,
#endif
/*    .umount_begin   = cifs_umount_begin, *//* consider adding in the future */
};

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0))
static struct super_block *
cifs_get_sb(struct file_system_type *fs_type,
	    int flags, const char *dev_name, void *data)
{
	int rc;
	struct super_block *sb = sget(fs_type, NULL, set_anon_super, NULL);

	cFYI(1, ("Devname: %s flags: %d ", dev_name, flags));

	if (IS_ERR(sb))
		return sb;

	sb->s_flags = flags;

	rc = cifs_read_super(sb, data, dev_name, flags & MS_VERBOSE ? 1 : 0);
	if (rc) {
		up_write(&sb->s_umount);
		deactivate_super(sb);
		return ERR_PTR(rc);
	}
	sb->s_flags |= MS_ACTIVE;
	return sb;
}
#endif

static ssize_t
cifs_read_wrapper(struct file * file, char *read_data, size_t read_size,
          loff_t * poffset)
{
	if(file == NULL)
		return -EIO;
	else if(file->f_dentry == NULL)
		return -EIO;
	else if(file->f_dentry->d_inode == NULL)
		return -EIO;

	if(CIFS_I(file->f_dentry->d_inode)->clientCanCacheRead) {
		return generic_file_read(file,read_data,read_size,poffset);
	} else {
		/* BB do we need to lock inode from here until after invalidate? */
/*		if(file->f_dentry->d_inode->i_mapping) {
			filemap_fdatawrite(file->f_dentry->d_inode->i_mapping);
			filemap_fdatawait(file->f_dentry->d_inode->i_mapping);
		}*/
/*		cifs_revalidate(file->f_dentry);*/ /* BB fixme */

		/* BB we should make timer configurable - perhaps 
		   by simply calling cifs_revalidate here */
		/* invalidate_remote_inode(file->f_dentry->d_inode);*/
		return generic_file_read(file,read_data,read_size,poffset);
	}
}

static ssize_t
cifs_write_wrapper(struct file * file, const char *write_data,
           size_t write_size, loff_t * poffset) 
{
	ssize_t written;

	if(file == NULL)
		return -EIO;
	else if(file->f_dentry == NULL)
		return -EIO;
	else if(file->f_dentry->d_inode == NULL)
		return -EIO;

	/* check whether we can cache writes locally */
	written = generic_file_write(file,write_data,write_size,poffset);
	if(!CIFS_I(file->f_dentry->d_inode)->clientCanCacheAll)  {
		if(file->f_dentry->d_inode->i_mapping) {
			filemap_fdatasync(file->f_dentry->d_inode->i_mapping);
		}
	}
	return written;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0))
static struct file_system_type cifs_fs_type = {
	.owner = THIS_MODULE,
	.name = "cifs",
	.get_sb = cifs_get_sb,
	.kill_sb = kill_anon_super,
	/*  .fs_flags */
};
#endif

static DECLARE_FSTYPE(cifs_fs_type, "cifs", cifs_read_super,0);


struct inode_operations cifs_dir_inode_ops = {
	.create = cifs_create,
	.lookup = cifs_lookup,
	.unlink = cifs_unlink,
	.link = cifs_hardlink,
	.mkdir = cifs_mkdir,
	.rmdir = cifs_rmdir,
	.rename = cifs_rename,
	.permission = cifs_permission,
	.revalidate = cifs_revalidate,  
	.setattr = cifs_setattr,
	.symlink = cifs_symlink,
	.mknod   = cifs_mknod,
};

struct inode_operations cifs_file_inode_ops = {
	.revalidate = cifs_revalidate,
	.setattr = cifs_setattr,
/*	.getattr = cifs_getattr,*/
	.rename = cifs_rename,
	.permission = cifs_permission,
#ifdef CONFIG_CIFS_XATTR
	.setxattr = cifs_setxattr,
	.getxattr = cifs_getxattr,
	.listxattr = cifs_listxattr,
	.removexattr = cifs_removexattr,
#endif 
};

struct inode_operations cifs_symlink_inode_ops = {
	.readlink = cifs_readlink,
	.follow_link = cifs_follow_link,
	.permission = cifs_permission,
	/* BB add the following two eventually */
	/* revalidate: cifs_revalidate,
	   setattr:    cifs_notify_change, *//* BB do we need notify change */
#ifdef CONFIG_CIFS_XATTR
	.setxattr = cifs_setxattr,
	.getxattr = cifs_getxattr,
	.listxattr = cifs_listxattr,
	.removexattr = cifs_removexattr,
#endif 
};

struct file_operations cifs_file_ops = {
	.read = cifs_read_wrapper,
	.write = cifs_write_wrapper, 
	.open = cifs_open,
	.release = cifs_close,
	.lock = cifs_lock,
	.fsync = cifs_fsync,
	.flush = cifs_flush,
	.mmap  = cifs_file_mmap,
/*	.sendfile = generic_file_sendfile,*/
#ifdef CONFIG_CIFS_FCNTL
	.fcntl = cifs_fcntl,
#endif
};

struct file_operations cifs_dir_ops = {
	.readdir = cifs_readdir,
	.release = cifs_closedir,
	.read    = generic_read_dir,
#ifdef CONFIG_CIFS_FCNTL
	.fcntl   = cifs_fcntl,
#endif
};
/*
static void
cifs_init_once(void *inode, kmem_cache_t * cachep, unsigned long flags)
{
	struct cifsInodeInfo *cifsi = (struct cifsInodeInfo *) inode;

	if ((flags & (SLAB_CTOR_VERIFY | SLAB_CTOR_CONSTRUCTOR)) ==
	    SLAB_CTOR_CONSTRUCTOR) {
		inode_init_once(&cifsi->vfs_inode);
		INIT_LIST_HEAD(&cifsi->lockList);
	}
}

static int
cifs_init_inodecache(void)
{
	cifs_inode_cachep = kmem_cache_create("cifs_inode_cache",
					      sizeof (struct cifsInodeInfo),
					      0, SLAB_HWCACHE_ALIGN,
					      cifs_init_once, NULL);
	if (cifs_inode_cachep == NULL)
		return -ENOMEM;
	

	return 0;
}

static void
cifs_destroy_inodecache(void)
{
	if (kmem_cache_destroy(cifs_inode_cachep))
		printk(KERN_WARNING "cifs_inode_cache: error freeing\n");
} */

static int
cifs_init_request_bufs(void)
{
	cifs_req_cachep = kmem_cache_create("cifs_request",
					    CIFS_MAX_MSGSIZE +
					    MAX_CIFS_HDR_SIZE, 0,
					    SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (cifs_req_cachep == NULL)
		return -ENOMEM;

	return 0;
}

static void
cifs_destroy_request_bufs(void)
{
	if (kmem_cache_destroy(cifs_req_cachep))
		printk(KERN_WARNING
		       "cifs_destroy_request_cache: error not all structures were freed\n");
}

static int
cifs_init_mids(void)
{
	cifs_mid_cachep = kmem_cache_create("cifs_mpx_ids",
				sizeof (struct mid_q_entry), 0,
				SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (cifs_mid_cachep == NULL)
		return -ENOMEM;
	cifs_oplock_cachep = kmem_cache_create("cifs_oplock_struct",
				sizeof (struct oplock_q_entry), 0,
				SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (cifs_oplock_cachep == NULL) {
		kmem_cache_destroy(cifs_mid_cachep);
		return -ENOMEM;
	}

	return 0;
}

static void
cifs_destroy_mids(void)
{
	if (kmem_cache_destroy(cifs_mid_cachep))
		printk(KERN_WARNING
		       "cifs_destroy_mids: error not all structures were freed\n");
	if (kmem_cache_destroy(cifs_oplock_cachep))
		printk(KERN_WARNING
		       "error not all oplock structures were freed\n");
}

static int cifs_oplock_thread(void * dummyarg)
{
	struct oplock_q_entry * oplock_item;
	struct cifsTconInfo *pTcon;
	struct inode * inode;
	__u16  netfid;
	int rc = 0;

	daemonize();
	sprintf(current->comm,"cifsoplockd");

	oplockThread = current;
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		
		schedule_timeout(1*HZ);  
		spin_lock(&GlobalMid_Lock);
		if(list_empty(&GlobalOplock_Q)) {
			spin_unlock(&GlobalMid_Lock);
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(39*HZ);
		} else {
			oplock_item = list_entry(GlobalOplock_Q.next, 
				struct oplock_q_entry, qhead);
			if(oplock_item) {
				cFYI(1,("found oplock item to write out")); 
				pTcon = oplock_item->tcon;
				inode = oplock_item->pinode;
				netfid = oplock_item->netfid;
				spin_unlock(&GlobalMid_Lock);
				DeleteOplockQEntry(oplock_item);
				/* can not grab inode sem here since it would
				deadlock when oplock received on delete 
				since vfs_unlink holds the i_sem across
				the call */
				/* down(&inode->i_sem);*/
				if (S_ISREG(inode->i_mode)) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,17))
					rc = filemap_fdatasync(inode->i_mapping);
					if(rc)
						CIFS_I(inode)->write_behind_rc = rc;
#else 
					filemap_fdatasync(inode->i_mapping);
#endif
					if(CIFS_I(inode)->clientCanCacheRead == 0) 
						invalidate_inode_pages(inode);
				} else
					rc = 0;
				/* releasing a stale oplock after recent reconnection 
				of smb session using a now incorrect file 
				handle is not a data integrity issue but do  
				not bother sending an oplock release if session 
				to server still is disconnected since oplock 
				already released by the server in that case */
				if(pTcon->tidStatus != CifsNeedReconnect) {
					rc = CIFSSMBLock(0, pTcon, 
						netfid,
						0 /* len */ , 0 /* offset */, 0, 
						0, LOCKING_ANDX_OPLOCK_RELEASE,
						0 /* wait flag */);
					cFYI(1,("Oplock release rc = %d ",rc));
				}
			} else
				spin_unlock(&GlobalMid_Lock);
		}
	} while(!signal_pending(current));
	complete_and_exit (&cifs_oplock_exited, 0);
}

static int __init
init_cifs(void)
{
	int rc = 0;
#if CONFIG_PROC_FS
	cifs_proc_init();
#endif
	INIT_LIST_HEAD(&GlobalServerList);	/* BB not implemented yet */
	INIT_LIST_HEAD(&GlobalSMBSessionList);
	INIT_LIST_HEAD(&GlobalTreeConnectionList);
	INIT_LIST_HEAD(&GlobalOplock_Q);
/*
 *  Initialize Global counters
 */
	atomic_set(&sesInfoAllocCount, 0);
	atomic_set(&tconInfoAllocCount, 0);
	atomic_set(&tcpSesReconnectCount, 0);
	atomic_set(&tconInfoReconnectCount, 0);

	atomic_set(&bufAllocCount, 0);
	atomic_set(&midCount, 0);
	GlobalCurrentXid = 0;
	GlobalTotalActiveXid = 0;
	GlobalMaxActiveXid = 0;
	GlobalSMBSeslock = RW_LOCK_UNLOCKED;
	GlobalMid_Lock = SPIN_LOCK_UNLOCKED;

/*	rc = cifs_init_inodecache();*/
	if (!rc) {
		rc = cifs_init_mids();
		if (!rc) {
			rc = cifs_init_request_bufs();
			if (!rc) {
				rc = register_filesystem(&cifs_fs_type);
				if (!rc) {                
					kernel_thread(cifs_oplock_thread, NULL, 
						CLONE_FS | CLONE_FILES | CLONE_VM);
					return rc; /* Success */
				} else
					cifs_destroy_request_bufs();
			}
			cifs_destroy_mids();
		}
/*		cifs_destroy_inodecache(); */
	}
#if CONFIG_PROC_FS
	cifs_proc_clean();
#endif
	return rc;
}

static void __exit
exit_cifs(void)
{
	cFYI(0, ("In unregister ie exit_cifs"));
#if CONFIG_PROC_FS
	cifs_proc_clean();
#endif
    	unregister_filesystem(&cifs_fs_type);
/*	cifs_destroy_inodecache();*/
	cifs_destroy_mids();
	cifs_destroy_request_bufs();
	if(oplockThread) {
		send_sig(SIGTERM, oplockThread, 1);
		wait_for_completion(&cifs_oplock_exited);
	}
}

MODULE_AUTHOR("Steve French <sfrench@us.ibm.com>");
MODULE_LICENSE("GPL");		/* combination of LGPL + GPL source behaves as GPL */
MODULE_DESCRIPTION
    ("VFS to access servers complying with the SNIA CIFS Specification e.g. Samba and Windows");
module_init(init_cifs)
module_exit(exit_cifs)
