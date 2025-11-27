/* SPDX-License-Identifier: GPL-2.0-or-later */

#define NTFS_EA_UID	BIT(1)
#define NTFS_EA_GID	BIT(2)
#define NTFS_EA_MODE	BIT(3)

extern const struct xattr_handler *const ntfsp_xattr_handlers[];

int ntfs_ea_set_wsl_not_symlink(struct ntfs_inode *ni, mode_t mode, dev_t dev);
int ntfs_ea_get_wsl_inode(struct inode *inode, dev_t *rdevp, unsigned int flags);
int ntfs_ea_set_wsl_inode(struct inode *inode, dev_t rdev, __le16 *ea_size,
		unsigned int flags);
ssize_t ntfsp_listxattr(struct dentry *dentry, char *buffer, size_t size);

#ifdef CONFIG_NTFSPLUS_FS_POSIX_ACL
struct posix_acl *ntfsp_get_acl(struct mnt_idmap *idmap, struct dentry *dentry,
			       int type);
int ntfsp_set_acl(struct mnt_idmap *idmap, struct dentry *dentry,
		 struct posix_acl *acl, int type);
int ntfsp_init_acl(struct mnt_idmap *idmap, struct inode *inode,
		  struct inode *dir);
#else
#define ntfsp_get_acl NULL
#define ntfsp_set_acl NULL
#endif
