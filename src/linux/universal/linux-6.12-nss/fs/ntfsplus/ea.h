/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _LINUX_NTFS_EA_H
#define _LINUX_NTFS_EA_H

#define NTFS_EA_UID	BIT(1)
#define NTFS_EA_GID	BIT(2)
#define NTFS_EA_MODE	BIT(3)

extern const struct xattr_handler *const ntfs_xattr_handlers[];

int ntfs_ea_set_wsl_not_symlink(struct ntfs_inode *ni, mode_t mode, dev_t dev);
int ntfs_ea_get_wsl_inode(struct inode *inode, dev_t *rdevp, unsigned int flags,
			  bool *has_lxmod);
int ntfs_ea_set_wsl_inode(struct inode *inode, dev_t rdev, __le16 *ea_size,
		unsigned int flags);
ssize_t ntfs_listxattr(struct dentry *dentry, char *buffer, size_t size);

#ifdef CONFIG_NTFS_FS_POSIX_ACL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
struct posix_acl *ntfs_get_acl(struct mnt_idmap *idmap, struct dentry *dentry,
			       int type);
#else
struct posix_acl *ntfs_get_acl(struct inode *inode, int type, bool rcu);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
int ntfs_set_acl(struct mnt_idmap *idmap, struct dentry *dentry,
		 struct posix_acl *acl, int type);
int ntfs_init_acl(struct mnt_idmap *idmap, struct inode *inode,
		  struct inode *dir);
#else
int ntfs_set_acl(struct user_namespace *mnt_userns, struct inode *inode,
		 struct posix_acl *acl, int type);
int ntfs_init_acl(struct user_namespace *mnt_userns, struct inode *inode,
		  struct inode *dir);
#endif
#else
#define ntfs_get_acl NULL
#define ntfs_set_acl NULL
#endif

#endif /* _LINUX_NTFS_EA_H */
