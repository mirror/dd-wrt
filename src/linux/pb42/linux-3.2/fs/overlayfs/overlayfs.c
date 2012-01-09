#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/file.h>
#include <linux/xattr.h>
#include <linux/security.h>
#include <linux/device_cgroup.h>
#include <linux/mount.h>
#include <linux/splice.h>
#include <linux/slab.h>
#include <linux/parser.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/rbtree.h>

MODULE_AUTHOR("Miklos Szeredi <miklos@szeredi.hu>");
MODULE_DESCRIPTION("Overlay filesystem");
MODULE_LICENSE("GPL");

#define OVL_COPY_UP_CHUNK_SIZE (1 << 20)

struct ovl_fs {
	struct vfsmount *upper_mnt;
	struct vfsmount *lower_mnt;
};

struct ovl_entry {
	struct dentry *__upperdentry;
	struct dentry *lowerdentry;
	union {
		struct {
			u64 version;
			bool opaque;
		};
		struct rcu_head rcu;
	};
};

static const char *ovl_whiteout_xattr = "trusted.overlay.whiteout";
static const char *ovl_opaque_xattr = "trusted.overlay.opaque";
static const char *ovl_whiteout_symlink = "(overlay-whiteout)";

enum ovl_path_type {
	OVL_PATH_UPPER,
	OVL_PATH_MERGE,
	OVL_PATH_LOWER,
};

static enum ovl_path_type ovl_path_type(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	if (oe->__upperdentry) {
		if (oe->lowerdentry && S_ISDIR(dentry->d_inode->i_mode))
			return OVL_PATH_MERGE;
		else
			return OVL_PATH_UPPER;
	} else {
		return OVL_PATH_LOWER;
	}
}

static struct dentry *ovl_upperdentry_dereference(struct ovl_entry *oe)
{
	struct dentry *upperdentry = ACCESS_ONCE(oe->__upperdentry);
	smp_read_barrier_depends();
	return upperdentry;
}

static void ovl_path_upper(struct dentry *dentry, struct path *path)
{
	struct ovl_fs *ofs = dentry->d_sb->s_fs_info;
	struct ovl_entry *oe = dentry->d_fsdata;

	path->mnt = ofs->upper_mnt;
	path->dentry = ovl_upperdentry_dereference(oe);
}

static void ovl_path_lower(struct dentry *dentry, struct path *path)
{
	struct ovl_fs *ofs = dentry->d_sb->s_fs_info;
	struct ovl_entry *oe = dentry->d_fsdata;

	path->mnt = ofs->lower_mnt;
	path->dentry = oe->lowerdentry;
}

static enum ovl_path_type ovl_path_real(struct dentry *dentry,
					struct path *path)
{

	enum ovl_path_type type = ovl_path_type(dentry);

	if (type == OVL_PATH_LOWER)
		ovl_path_lower(dentry, path);
	else
		ovl_path_upper(dentry, path);

	return type;
}

static struct dentry *ovl_dentry_upper(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	return ovl_upperdentry_dereference(oe);
}

static struct dentry *ovl_dentry_lower(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	return oe->lowerdentry;
}

static struct dentry *ovl_dentry_real(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;
	struct dentry *realdentry;

	realdentry = ovl_upperdentry_dereference(oe);
	if (!realdentry)
		realdentry = oe->lowerdentry;

	return realdentry;
}

static bool ovl_dentry_is_opaque(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;
	return oe->opaque;
}

static void ovl_dentry_set_opaque(struct dentry *dentry, bool opaque)
{
	struct ovl_entry *oe = dentry->d_fsdata;
	oe->opaque = opaque;
}

static void ovl_dentry_update(struct dentry *dentry, struct dentry *upperdentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	WARN_ON(!mutex_is_locked(&upperdentry->d_parent->d_inode->i_mutex));
	WARN_ON(oe->__upperdentry);
	smp_wmb();
	oe->__upperdentry = upperdentry;
}

static void ovl_dentry_version_inc(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	WARN_ON(!mutex_is_locked(&dentry->d_inode->i_mutex));
	oe->version++;
}

static u64 ovl_dentry_version_get(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	WARN_ON(!mutex_is_locked(&dentry->d_inode->i_mutex));
	return oe->version;
}

static bool ovl_is_whiteout(struct dentry *dentry)
{
	int res;
	char val;

	if (!dentry)
		return false;
	if (!dentry->d_inode)
		return false;
	if (!S_ISLNK(dentry->d_inode->i_mode))
		return false;

	res = vfs_getxattr(dentry, ovl_whiteout_xattr, &val, 1);
	if (res == 1 && val == 'y')
		return true;

	return false;
}

static bool ovl_is_opaquedir(struct dentry *dentry)
{
	int res;
	char val;

	if (!S_ISDIR(dentry->d_inode->i_mode))
		return false;

	res = vfs_getxattr(dentry, ovl_opaque_xattr, &val, 1);
	if (res == 1 && val == 'y')
		return true;

	return false;
}

struct ovl_cache_entry {
	const char *name;
	unsigned int len;
	unsigned int type;
	u64 ino;
	bool is_whiteout;
	struct list_head l_node;
	struct rb_node node;
};

struct ovl_readdir_data {
	struct rb_root *root;
	struct list_head *list;
	struct list_head *middle;
	struct dentry *dir;
	int count;
	int err;
};

struct ovl_dir_file {
	bool is_real;
	bool is_cached;
	struct list_head cursor;
	u64 cache_version;
	struct list_head cache;
	struct file *realfile;
};

static struct ovl_cache_entry *ovl_cache_entry_from_node(struct rb_node *n)
{
	return container_of(n, struct ovl_cache_entry, node);
}

static struct ovl_cache_entry *ovl_cache_entry_find(struct rb_root *root,
						    const char *name, int len)
{
	struct rb_node *node = root->rb_node;
	int cmp;

	while (node) {
		struct ovl_cache_entry *p = ovl_cache_entry_from_node(node);

		cmp = strncmp(name, p->name, len);
		if (cmp > 0)
			node = p->node.rb_right;
		else if (cmp < 0 || len < p->len)
			node = p->node.rb_left;
		else
			return p;
	}

	return NULL;
}

static struct ovl_cache_entry *ovl_cache_entry_new(const char *name, int len,
						   u64 ino, unsigned int d_type)
{
	struct ovl_cache_entry *p;

	p = kmalloc(sizeof(*p) + len + 1, GFP_KERNEL);
	if (p) {
		char *name_copy = (char *) (p + 1);
		memcpy(name_copy, name, len);
		name_copy[len] = '\0';
		p->name = name_copy;
		p->len = len;
		p->type = d_type;
		p->ino = ino;
		p->is_whiteout = false;
	}

	return p;
}

static int ovl_cache_entry_add_rb(struct ovl_readdir_data *rdd,
				  const char *name, int len, u64 ino,
				  unsigned int d_type)
{
	struct rb_node **newp = &rdd->root->rb_node;
	struct rb_node *parent = NULL;
	struct ovl_cache_entry *p;

	while (*newp) {
		int cmp;
		struct ovl_cache_entry *tmp;

		parent = *newp;
		tmp = ovl_cache_entry_from_node(*newp);
		cmp = strncmp(name, tmp->name, len);
		if (cmp > 0)
			newp = &tmp->node.rb_right;
		else if (cmp < 0 || len < tmp->len)
			newp = &tmp->node.rb_left;
		else
			return 0;
	}

	p = ovl_cache_entry_new(name, len, ino, d_type);
	if (p == NULL)
		return -ENOMEM;

	list_add_tail(&p->l_node, rdd->list);
	rb_link_node(&p->node, parent, newp);
	rb_insert_color(&p->node, rdd->root);

	return 0;
}

static int ovl_fill_lower(void *buf, const char *name, int namelen,
			    loff_t offset, u64 ino, unsigned int d_type)
{
	struct ovl_readdir_data *rdd = buf;
	struct ovl_cache_entry *p;

	rdd->count++;
	p = ovl_cache_entry_find(rdd->root, name, namelen);
	if (p) {
		list_move_tail(&p->l_node, rdd->middle);
	} else {
		p = ovl_cache_entry_new(name, namelen, ino, d_type);
		if (p == NULL)
			rdd->err = -ENOMEM;
		else
			list_add_tail(&p->l_node, rdd->middle);
	}

	return rdd->err;
}

static void ovl_cache_free(struct list_head *list)
{
	struct ovl_cache_entry *p;
	struct ovl_cache_entry *n;

	list_for_each_entry_safe(p, n, list, l_node)
		kfree(p);

	INIT_LIST_HEAD(list);
}

static int ovl_fill_upper(void *buf, const char *name, int namelen,
			  loff_t offset, u64 ino, unsigned int d_type)
{
	struct ovl_readdir_data *rdd = buf;

	rdd->count++;
	return ovl_cache_entry_add_rb(rdd, name, namelen, ino, d_type);
}

static int ovl_dir_read(struct path *realpath, struct ovl_readdir_data *rdd,
			  filldir_t filler)
{
	struct file *realfile;
	int err;

	realfile = vfs_open(realpath, O_RDONLY | O_DIRECTORY, current_cred());
	if (IS_ERR(realfile))
		return PTR_ERR(realfile);

	do {
		rdd->count = 0;
		rdd->err = 0;
		err = vfs_readdir(realfile, filler, rdd);
		if (err >= 0)
			err = rdd->err;
	} while (!err && rdd->count);
	fput(realfile);

	return 0;
}

static void ovl_dir_reset(struct file *file)
{
	struct ovl_dir_file *od = file->private_data;
	enum ovl_path_type type = ovl_path_type(file->f_path.dentry);

	if (ovl_dentry_version_get(file->f_path.dentry) != od->cache_version) {
		list_del_init(&od->cursor);
		ovl_cache_free(&od->cache);
		od->is_cached = false;
	}
	WARN_ON(!od->is_real && type != OVL_PATH_MERGE);
	if (od->is_real && type == OVL_PATH_MERGE) {
		fput(od->realfile);
		od->realfile = NULL;
		od->is_real = false;
	}
}

static int ovl_dir_mark_whiteouts(struct ovl_readdir_data *rdd)
{
	struct ovl_cache_entry *p;
	struct dentry *dentry;
	const struct cred *old_cred;
	struct cred *override_cred;

	override_cred = prepare_creds();
	if (!override_cred) {
		ovl_cache_free(rdd->list);
		return -ENOMEM;
	}

	/*
	 * CAP_SYS_ADMIN for getxattr
	 * CAP_DAC_OVERRIDE for lookup
	 */
	cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
	cap_raise(override_cred->cap_effective, CAP_DAC_OVERRIDE);
	old_cred = override_creds(override_cred);

	mutex_lock(&rdd->dir->d_inode->i_mutex);
	list_for_each_entry(p, rdd->list, l_node) {
		if (p->type != DT_LNK)
			continue;

		dentry = lookup_one_len(p->name, rdd->dir, p->len);
		if (IS_ERR(dentry))
			continue;

		p->is_whiteout = ovl_is_whiteout(dentry);
		dput(dentry);
	}
	mutex_unlock(&rdd->dir->d_inode->i_mutex);

	revert_creds(old_cred);
	put_cred(override_cred);

	return 0;
}

static int ovl_dir_read_merged(struct path *upperpath, struct path *lowerpath,
			       struct ovl_readdir_data *rdd)
{
	int err;
	struct rb_root root = RB_ROOT;
	struct list_head middle;

	rdd->root = &root;
	if (upperpath->dentry) {
		rdd->dir = upperpath->dentry;
		err = ovl_dir_read(upperpath, rdd, ovl_fill_upper);
		if (err)
			goto out;

		err = ovl_dir_mark_whiteouts(rdd);
		if (err)
			goto out;
	}
	/*
	 * Insert lowerpath entries before upperpath ones, this allows
	 * offsets to be reasonably constant
	 */
	list_add(&middle, rdd->list);
	rdd->middle = &middle;
	err = ovl_dir_read(lowerpath, rdd, ovl_fill_lower);
	list_del(&middle);
out:
	rdd->root = NULL;

	return err;
}

static void ovl_seek_cursor(struct ovl_dir_file *od, loff_t pos)
{
	struct list_head *l;
	loff_t off;

	l = od->cache.next;
	for (off = 0; off < pos; off++) {
		if (l == &od->cache)
			break;
		l = l->next;
	}
	list_move_tail(&od->cursor, l);
}

static int ovl_readdir(struct file *file, void *buf, filldir_t filler)
{
	struct ovl_dir_file *od = file->private_data;
	int res;

	if (!file->f_pos)
		ovl_dir_reset(file);

	if (od->is_real) {
		res = vfs_readdir(od->realfile, filler, buf);
		file->f_pos = od->realfile->f_pos;

		return res;
	}

	if (!od->is_cached) {
		struct path lowerpath;
		struct path upperpath;
		struct ovl_readdir_data rdd = { .list = &od->cache };

		ovl_path_lower(file->f_path.dentry, &lowerpath);
		ovl_path_upper(file->f_path.dentry, &upperpath);

		res = ovl_dir_read_merged(&upperpath, &lowerpath, &rdd);
		if (res) {
			ovl_cache_free(rdd.list);
			return res;
		}

		od->cache_version = ovl_dentry_version_get(file->f_path.dentry);
		od->is_cached = true;

		ovl_seek_cursor(od, file->f_pos);
	}

	while (od->cursor.next != &od->cache) {
		int over;
		loff_t off;
		struct ovl_cache_entry *p;

		p = list_entry(od->cursor.next, struct ovl_cache_entry, l_node);
		off = file->f_pos;
		file->f_pos++;
		list_move(&od->cursor, &p->l_node);

		if (p->is_whiteout)
			continue;

		over = filler(buf, p->name, p->len, off, p->ino, p->type);
		if (over)
			break;
	}

	return 0;
}

static loff_t ovl_dir_llseek(struct file *file, loff_t offset, int origin)
{
	loff_t res;
	struct ovl_dir_file *od = file->private_data;

	mutex_lock(&file->f_dentry->d_inode->i_mutex);
	if (!file->f_pos)
		ovl_dir_reset(file);

	if (od->is_real) {
		res = vfs_llseek(od->realfile, offset, origin);
		file->f_pos = od->realfile->f_pos;
	} else {
		res = -EINVAL;

		switch (origin) {
		case SEEK_CUR:
			offset += file->f_pos;
			break;
		case SEEK_SET:
			break;
		default:
			goto out_unlock;
		}
		if (offset < 0)
			goto out_unlock;

		if (offset != file->f_pos) {
			file->f_pos = offset;
			if (od->is_cached)
				ovl_seek_cursor(od, offset);
		}
		res = offset;
	}
out_unlock:
	mutex_unlock(&file->f_dentry->d_inode->i_mutex);

	return res;
}

static int ovl_dir_fsync(struct file *file, int datasync)
{
	struct ovl_dir_file *od = file->private_data;

	/* May need to reopen directory if it got copied up */
	if (!od->realfile) {
		struct path upperpath;

		ovl_path_upper(file->f_path.dentry, &upperpath);
		od->realfile = vfs_open(&upperpath, O_RDONLY, current_cred());
		if (IS_ERR(od->realfile))
			return PTR_ERR(od->realfile);
	}

	return vfs_fsync(od->realfile, datasync);
}

static int ovl_dir_release(struct inode *inode, struct file *file)
{
	struct ovl_dir_file *od = file->private_data;

	list_del(&od->cursor);
	ovl_cache_free(&od->cache);
	if (od->realfile)
		fput(od->realfile);
	kfree(od);

	return 0;
}

static int ovl_dir_open(struct inode *inode, struct file *file)
{
	struct path realpath;
	struct file *realfile;
	struct ovl_dir_file *od;
	enum ovl_path_type type;

	od = kzalloc(sizeof(struct ovl_dir_file), GFP_KERNEL);
	if (!od)
		return -ENOMEM;

	type = ovl_path_real(file->f_path.dentry, &realpath);
	realfile = vfs_open(&realpath, file->f_flags, current_cred());
	if (IS_ERR(realfile)) {
		kfree(od);
		return PTR_ERR(realfile);
	}
	INIT_LIST_HEAD(&od->cache);
	INIT_LIST_HEAD(&od->cursor);
	od->is_cached = false;
	od->realfile = realfile;
	od->is_real = (type != OVL_PATH_MERGE);
	file->private_data = od;

	return 0;
}

static const struct file_operations ovl_dir_operations = {
	.read		= generic_read_dir,
	.open		= ovl_dir_open,
	.readdir	= ovl_readdir,
	.llseek		= ovl_dir_llseek,
	.fsync		= ovl_dir_fsync,
	.release	= ovl_dir_release,
};

static const struct inode_operations ovl_dir_inode_operations;

static void ovl_entry_free(struct rcu_head *head)
{
	struct ovl_entry *oe = container_of(head, struct ovl_entry, rcu);
	kfree(oe);
}

static void ovl_dentry_release(struct dentry *dentry)
{
	struct ovl_entry *oe = dentry->d_fsdata;

	if (oe) {
		dput(oe->__upperdentry);
		dput(oe->lowerdentry);
		call_rcu(&oe->rcu, ovl_entry_free);
	}
}

static const struct dentry_operations ovl_dentry_operations = {
	.d_release = ovl_dentry_release,
};

static struct dentry *ovl_lookup_real(struct dentry *dir, struct qstr *name)
{
	struct dentry *dentry;

	mutex_lock(&dir->d_inode->i_mutex);
	dentry = lookup_one_len(name->name, dir, name->len);
	mutex_unlock(&dir->d_inode->i_mutex);

	if (IS_ERR(dentry)) {
		if (PTR_ERR(dentry) == -ENOENT)
			dentry = NULL;
	} else if (!dentry->d_inode) {
		dput(dentry);
		dentry = NULL;
	}
	return dentry;
}

static struct ovl_entry *ovl_alloc_entry(void)
{
	return kzalloc(sizeof(struct ovl_entry), GFP_KERNEL);
}

static struct inode *ovl_new_inode(struct super_block *sb, umode_t mode,
				   struct ovl_entry *oe);

static int ovl_whiteout(struct dentry *upperdir, struct dentry *dentry)
{
	int err;
	struct dentry *newdentry;
	const struct cred *old_cred;
	struct cred *override_cred;

	/* FIXME: recheck lower dentry to see if whiteout is really needed */

	err = -ENOMEM;
	override_cred = prepare_creds();
	if (!override_cred)
		goto out;

	/*
	 * CAP_SYS_ADMIN for setxattr
	 * CAP_DAC_OVERRIDE for symlink creation
	 */
	cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
	cap_raise(override_cred->cap_effective, CAP_DAC_OVERRIDE);
	override_cred->fsuid = 0;
	override_cred->fsgid = 0;
	old_cred = override_creds(override_cred);

	newdentry = lookup_one_len(dentry->d_name.name, upperdir,
				   dentry->d_name.len);
	err = PTR_ERR(newdentry);
	if (IS_ERR(newdentry))
		goto out_put_cred;

	/* Just been removed within the same locked region */
	WARN_ON(newdentry->d_inode);

	err = vfs_symlink(upperdir->d_inode, newdentry, ovl_whiteout_symlink);
	if (err)
		goto out_dput;

	ovl_dentry_version_inc(dentry->d_parent);

	err = vfs_setxattr(newdentry, ovl_whiteout_xattr, "y", 1, 0);
	if (err)
		vfs_unlink(upperdir->d_inode, newdentry);

out_dput:
	dput(newdentry);
out_put_cred:
	revert_creds(old_cred);
	put_cred(override_cred);
out:
	if (err) {
		/*
		 * There's no way to recover from failure to whiteout.
		 * What should we do?  Log a big fat error and... ?
		 */
		printk(KERN_ERR "overlayfs: ERROR - failed to whiteout '%s'\n",
		       dentry->d_name.name);
	}

	return err;
}

static struct dentry *ovl_lookup(struct inode *dir, struct dentry *dentry,
				   struct nameidata *nd)
{
	struct ovl_entry *oe;
	struct dentry *upperdir;
	struct dentry *lowerdir;
	struct dentry *upperdentry = NULL;
	struct dentry *lowerdentry = NULL;
	struct inode *inode = NULL;
	int err;

	err = -ENOMEM;
	oe = ovl_alloc_entry();
	if (!oe)
		goto out;

	upperdir = ovl_dentry_upper(dentry->d_parent);
	lowerdir = ovl_dentry_lower(dentry->d_parent);

	if (upperdir) {
		upperdentry = ovl_lookup_real(upperdir, &dentry->d_name);
		err = PTR_ERR(upperdentry);
		if (IS_ERR(upperdentry))
			goto out_put_dir;

		if (lowerdir && upperdentry &&
		    (S_ISLNK(upperdentry->d_inode->i_mode) ||
		     S_ISDIR(upperdentry->d_inode->i_mode))) {
			const struct cred *old_cred;
			struct cred *override_cred;

			err = -ENOMEM;
			override_cred = prepare_creds();
			if (!override_cred)
				goto out_dput_upper;

			/* CAP_SYS_ADMIN needed for getxattr */
			cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
			old_cred = override_creds(override_cred);

			if (ovl_is_opaquedir(upperdentry)) {
				oe->opaque = true;
			} else if (ovl_is_whiteout(upperdentry)) {
				dput(upperdentry);
				upperdentry = NULL;
				oe->opaque = true;
			}
			revert_creds(old_cred);
			put_cred(override_cred);
		}
	}
	if (lowerdir && !oe->opaque) {
		lowerdentry = ovl_lookup_real(lowerdir, &dentry->d_name);
		err = PTR_ERR(lowerdentry);
		if (IS_ERR(lowerdentry))
			goto out_dput_upper;
	}

	if (lowerdentry && upperdentry &&
	    (!S_ISDIR(upperdentry->d_inode->i_mode) ||
	     !S_ISDIR(lowerdentry->d_inode->i_mode))) {
		dput(lowerdentry);
		lowerdentry = NULL;
		oe->opaque = true;
	}

	if (lowerdentry || upperdentry) {
		struct dentry *realdentry;

		realdentry = upperdentry ? upperdentry : lowerdentry;
		err = -ENOMEM;
		inode = ovl_new_inode(dir->i_sb, realdentry->d_inode->i_mode, oe);
		if (!inode)
			goto out_dput;
	}

	if (upperdentry)
		oe->__upperdentry = upperdentry;

	if (lowerdentry)
		oe->lowerdentry = lowerdentry;

	dentry->d_fsdata = oe;
	dentry->d_op = &ovl_dentry_operations;
	d_add(dentry, inode);

	return NULL;

out_dput:
	dput(lowerdentry);
out_dput_upper:
	dput(upperdentry);
out_put_dir:
	kfree(oe);
out:
	return ERR_PTR(err);
}

static int ovl_copy_up_xattr(struct dentry *old, struct dentry *new)
{
	ssize_t list_size, size;
	char *buf, *name, *value;
	int error;

	if (!old->d_inode->i_op->getxattr ||
	    !new->d_inode->i_op->getxattr)
		return 0;

	list_size = vfs_listxattr(old, NULL, 0);
	if (list_size <= 0) {
		if (list_size == -EOPNOTSUPP)
			return 0;
		return list_size;
	}

	buf = kzalloc(list_size, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	error = -ENOMEM;
	value = kmalloc(XATTR_SIZE_MAX, GFP_KERNEL);
	if (!value)
		goto out;

	list_size = vfs_listxattr(old, buf, list_size);
	if (list_size <= 0) {
		error = list_size;
		goto out_free_value;
	}

	for (name = buf; name < (buf + list_size); name += strlen(name) + 1) {
		size = vfs_getxattr(old, name, value, XATTR_SIZE_MAX);
		if (size <= 0) {
			error = size;
			goto out_free_value;
		}
		error = vfs_setxattr(new, name, value, size, 0);
		if (error)
			goto out_free_value;
	}

out_free_value:
	kfree(value);
out:
	kfree(buf);
	return error;
}

static int ovl_copy_up_data(struct path *old, struct path *new, loff_t len)
{
	struct file *old_file;
	struct file *new_file;
	int error = 0;

	if (len == 0)
		return 0;

	old_file = vfs_open(old, O_RDONLY, current_cred());
	if (IS_ERR(old_file))
		return PTR_ERR(old_file);

	new_file = vfs_open(new, O_WRONLY, current_cred());
	if (IS_ERR(new_file)) {
		error = PTR_ERR(new_file);
		goto out_fput;
	}

	/* FIXME: copy up sparse files efficiently */
	while (len) {
		loff_t offset = new_file->f_pos;
		size_t this_len = OVL_COPY_UP_CHUNK_SIZE;
		long bytes;

		if (len < this_len)
			this_len = len;

		if (signal_pending_state(TASK_KILLABLE, current)) {
			error = -EINTR;
			break;
		}

		bytes = do_splice_direct(old_file, &offset, new_file, this_len,
				 SPLICE_F_MOVE);
		if (bytes <= 0) {
			error = bytes;
			break;
		}

		len -= bytes;
	}

	fput(new_file);
out_fput:
	fput(old_file);
	return error;
}

static struct dentry *ovl_lookup_create(struct dentry *upperdir,
					struct dentry *template)
{
	int err;
	struct dentry *newdentry;
	struct qstr *name = &template->d_name;

	newdentry = lookup_one_len(name->name, upperdir, name->len);
	if (IS_ERR(newdentry))
		return newdentry;

	if (newdentry->d_inode) {
		const struct cred *old_cred;
		struct cred *override_cred;

		/* No need to check whiteout if lower parent is non-existent */
		err = -EEXIST;
		if (!ovl_dentry_lower(template->d_parent))
			goto out_dput;

		if (!S_ISLNK(newdentry->d_inode->i_mode))
			goto out_dput;

		err = -ENOMEM;
		override_cred = prepare_creds();
		if (!override_cred)
			goto out_dput;

		/*
		 * CAP_SYS_ADMIN for getxattr
		 * CAP_FOWNER for unlink in sticky directory
		 */
		cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
		cap_raise(override_cred->cap_effective, CAP_FOWNER);
		old_cred = override_creds(override_cred);

		err = -EEXIST;
		if (ovl_is_whiteout(newdentry))
			err = vfs_unlink(upperdir->d_inode, newdentry);

		revert_creds(old_cred);
		put_cred(override_cred);
		if (err)
			goto out_dput;

		dput(newdentry);
		newdentry = lookup_one_len(name->name, upperdir, name->len);
		if (IS_ERR(newdentry)) {
			ovl_whiteout(upperdir, template);
			return newdentry;
		}

		/*
		 * Whiteout just been successfully removed, parent
		 * i_mutex is still held, there's no way the lookup
		 * could return positive.
		 */
		WARN_ON(newdentry->d_inode);
	}

	return newdentry;

out_dput:
	dput(newdentry);
	return ERR_PTR(err);
}

static struct dentry *ovl_upper_create(struct dentry *upperdir,
				       struct dentry *dentry,
				       struct kstat *stat, const char *link)
{
	int err;
	struct dentry *newdentry;
	struct inode *dir = upperdir->d_inode;

	newdentry = ovl_lookup_create(upperdir, dentry);
	if (IS_ERR(newdentry))
		goto out;

	switch (stat->mode & S_IFMT) {
	case S_IFREG:
		err = vfs_create(dir, newdentry, stat->mode, NULL);
		break;

	case S_IFDIR:
		err = vfs_mkdir(dir, newdentry, stat->mode);
		break;

	case S_IFCHR:
	case S_IFBLK:
	case S_IFIFO:
	case S_IFSOCK:
		err = vfs_mknod(dir, newdentry, stat->mode, stat->rdev);
		break;

	case S_IFLNK:
		err = vfs_symlink(dir, newdentry, link);
		break;

	default:
		err = -EPERM;
	}
	if (err) {
		if (ovl_dentry_is_opaque(dentry))
			ovl_whiteout(upperdir, dentry);
		dput(newdentry);
		newdentry = ERR_PTR(err);
	}

out:
	return newdentry;

}

static char *ovl_read_symlink(struct dentry *realdentry)
{
	int res;
	char *buf;
	struct inode *inode = realdentry->d_inode;
	mm_segment_t old_fs;

	res = -EINVAL;
	if (!inode->i_op->readlink)
		goto err;

	res = -ENOMEM;
	buf = (char *) __get_free_page(GFP_KERNEL);
	if (!buf)
		goto err;

	old_fs = get_fs();
	set_fs(get_ds());
	/* The cast to a user pointer is valid due to the set_fs() */
	res = inode->i_op->readlink(realdentry,
				    (char __user *)buf, PAGE_SIZE - 1);
	set_fs(old_fs);
	if (res < 0) {
		free_page((unsigned long) buf);
		goto err;
	}
	buf[res] = '\0';

	return buf;

err:
	return ERR_PTR(res);
}

static int ovl_set_timestamps(struct dentry *upperdentry, struct kstat *stat)
{
	struct iattr attr = {
		.ia_valid = ATTR_ATIME | ATTR_MTIME | ATTR_ATIME_SET | ATTR_MTIME_SET,
		.ia_atime = stat->atime,
		.ia_mtime = stat->mtime,
	};

	return notify_change(upperdentry, &attr);
}

static int ovl_set_mode(struct dentry *upperdentry, umode_t mode)
{
	struct iattr attr = {
		.ia_valid = ATTR_MODE,
		.ia_mode = mode,
	};

	return notify_change(upperdentry, &attr);
}

static int ovl_set_opaque(struct dentry *upperdentry)
{
	int err;
	const struct cred *old_cred;
	struct cred *override_cred;

	override_cred = prepare_creds();
	if (!override_cred)
		return -ENOMEM;

	/* CAP_SYS_ADMIN for setxattr of "trusted" namespace */
	cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
	old_cred = override_creds(override_cred);
	err = vfs_setxattr(upperdentry, ovl_opaque_xattr, "y", 1, 0);
	revert_creds(old_cred);
	put_cred(override_cred);

	return err;
}

static int ovl_remove_opaque(struct dentry *upperdentry)
{
	int err;
	const struct cred *old_cred;
	struct cred *override_cred;

	override_cred = prepare_creds();
	if (!override_cred)
		return -ENOMEM;

	/* CAP_SYS_ADMIN for removexattr of "trusted" namespace */
	cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
	old_cred = override_creds(override_cred);
	err = vfs_removexattr(upperdentry, ovl_opaque_xattr);
	revert_creds(old_cred);
	put_cred(override_cred);

	return err;
}

static int ovl_copy_up_locked(struct dentry *upperdir, struct dentry *dentry,
			      struct path *lowerpath, struct kstat *stat,
			      const char *link)
{
	int err;
	struct path newpath;
	umode_t mode = stat->mode;
	struct ovl_fs *ofs = dentry->d_sb->s_fs_info;

	/* Can't properly set mode on creation because of the umask */
	stat->mode &= S_IFMT;

	newpath.mnt = ofs->upper_mnt;
	newpath.dentry = ovl_upper_create(upperdir, dentry, stat, link);
	if (IS_ERR(newpath.dentry)) {
		err = PTR_ERR(newpath.dentry);

		/* Already copied up? */
		if (err == -EEXIST && ovl_path_type(dentry) != OVL_PATH_LOWER)
			return 0;

		return err;
	}

	if (S_ISREG(stat->mode)) {
		err = ovl_copy_up_data(lowerpath, &newpath, stat->size);
		if (err)
			goto err_remove;
	}

	err = ovl_copy_up_xattr(lowerpath->dentry, newpath.dentry);
	if (err)
		goto err_remove;

	mutex_lock(&newpath.dentry->d_inode->i_mutex);
	if (!S_ISLNK(stat->mode))
		err = ovl_set_mode(newpath.dentry, mode);
	if (!err)
		err = ovl_set_timestamps(newpath.dentry, stat);
	mutex_unlock(&newpath.dentry->d_inode->i_mutex);
	if (err)
		goto err_remove;

	ovl_dentry_update(dentry, newpath.dentry);

	/*
	 * Easiest way to get rid of the lower dentry reference is to
	 * drop this dentry.  This is neither needed nor possible for
	 * directories.
	 */
	if (!S_ISDIR(stat->mode))
		d_drop(dentry);

	return 0;

err_remove:
	if (S_ISDIR(stat->mode))
		vfs_rmdir(upperdir->d_inode, newpath.dentry);
	else
		vfs_unlink(upperdir->d_inode, newpath.dentry);

	dput(newpath.dentry);

	return err;
}

static int ovl_copy_up_one(struct dentry *parent, struct dentry *dentry,
			   struct path *lowerpath, struct kstat *stat)
{
	int err;
	struct kstat pstat;
	struct path parentpath;
	struct dentry *upperdir;
	const struct cred *old_cred;
	struct cred *override_cred;
	char *link = NULL;

	ovl_path_upper(parent, &parentpath);
	upperdir = parentpath.dentry;

	err = vfs_getattr(parentpath.mnt, parentpath.dentry, &pstat);
	if (err)
		return err;

	if (S_ISLNK(stat->mode)) {
		link = ovl_read_symlink(lowerpath->dentry);
		if (IS_ERR(link))
			return PTR_ERR(link);
	}

	err = -ENOMEM;
	override_cred = prepare_creds();
	if (!override_cred)
		goto out_free_link;

	override_cred->fsuid = stat->uid;
	override_cred->fsgid = stat->gid;
	/*
	 * CAP_SYS_ADMIN for copying up extended attributes
	 * CAP_DAC_OVERRIDE for create
	 * CAP_FOWNER for chmod, timestamp update
	 * CAP_FSETID for chmod
	 * CAP_MKNOD for mknod
	 */
	cap_raise(override_cred->cap_effective, CAP_SYS_ADMIN);
	cap_raise(override_cred->cap_effective, CAP_DAC_OVERRIDE);
	cap_raise(override_cred->cap_effective, CAP_FOWNER);
	cap_raise(override_cred->cap_effective, CAP_FSETID);
	cap_raise(override_cred->cap_effective, CAP_MKNOD);
	old_cred = override_creds(override_cred);

	mutex_lock_nested(&upperdir->d_inode->i_mutex, I_MUTEX_PARENT);
	/*
	 * Using upper filesystem locking to protect against copy up
	 * racing with rename (rename means the copy up was already
	 * successful).
	 */
	if (dentry->d_parent != parent) {
		WARN_ON((ovl_path_type(dentry) == OVL_PATH_LOWER));
		err = 0;
	} else {
		err = ovl_copy_up_locked(upperdir, dentry, lowerpath,
					 stat, link);
		if (!err) {
			/* Restore timestamps on parent (best effort) */
			ovl_set_timestamps(upperdir, &pstat);
		}
	}

	mutex_unlock(&upperdir->d_inode->i_mutex);

	revert_creds(old_cred);
	put_cred(override_cred);

out_free_link:
	if (link)
		free_page((unsigned long) link);

	return err;
}

static int ovl_copy_up(struct dentry *dentry)
{
	int err;

	err = 0;
	while (!err) {
		struct dentry *next;
		struct dentry *parent;
		struct path lowerpath;
		struct kstat stat;
		enum ovl_path_type type = ovl_path_type(dentry);

		if (type != OVL_PATH_LOWER)
			break;

		next = dget(dentry);
		/* find the topmost dentry not yet copied up */
		for (;;) {
			parent = dget_parent(next);

			type = ovl_path_type(parent);
			if (type != OVL_PATH_LOWER)
				break;

			dput(next);
			next = parent;
		}

		ovl_path_lower(next, &lowerpath);
		err = vfs_getattr(lowerpath.mnt, lowerpath.dentry, &stat);
		if (!err)
			err = ovl_copy_up_one(parent, next, &lowerpath, &stat);

		dput(parent);
		dput(next);
	}

	return err;
}

/* Optimize by not copying up the file first and truncating later */
static int ovl_copy_up_truncate(struct dentry *dentry, loff_t size)
{
	int err;
	struct kstat stat;
	struct path lowerpath;
	struct dentry *parent = dget_parent(dentry);

	err = ovl_copy_up(parent);
	if (err)
		goto out_dput_parent;

	ovl_path_lower(dentry, &lowerpath);
	err = vfs_getattr(lowerpath.mnt, lowerpath.dentry, &stat);
	if (err)
		goto out_dput_parent;

	if (size < stat.size)
		stat.size = size;

	err = ovl_copy_up_one(parent, dentry, &lowerpath, &stat);

out_dput_parent:
	dput(parent);
	return err;
}

static int ovl_setattr(struct dentry *dentry, struct iattr *attr)
{
	struct dentry *upperdentry;
	int err;

	if ((attr->ia_valid & ATTR_SIZE) && !ovl_dentry_upper(dentry))
		err = ovl_copy_up_truncate(dentry, attr->ia_size);
	else
		err = ovl_copy_up(dentry);
	if (err)
		return err;

	upperdentry = ovl_dentry_upper(dentry);

	if (attr->ia_valid & (ATTR_KILL_SUID|ATTR_KILL_SGID))
		attr->ia_valid &= ~ATTR_MODE;

	mutex_lock(&upperdentry->d_inode->i_mutex);
	err = notify_change(upperdentry, attr);
	mutex_unlock(&upperdentry->d_inode->i_mutex);

	return err;
}

static int ovl_getattr(struct vfsmount *mnt, struct dentry *dentry,
			 struct kstat *stat)
{
	struct path realpath;

	ovl_path_real(dentry, &realpath);
	return vfs_getattr(realpath.mnt, realpath.dentry, stat);
}

static int ovl_dir_getattr(struct vfsmount *mnt, struct dentry *dentry,
			 struct kstat *stat)
{
	int err;
	enum ovl_path_type type;
	struct path realpath;

	type = ovl_path_real(dentry, &realpath);
	err = vfs_getattr(realpath.mnt, realpath.dentry, stat);
	if (err)
		return err;

	stat->dev = dentry->d_sb->s_dev;
	stat->ino = dentry->d_inode->i_ino;

	/*
	 * It's probably not worth it to count subdirs to get the
	 * correct link count.  nlink=1 seems to pacify 'find' and
	 * other utilities.
	 */
	if (type == OVL_PATH_MERGE)
		stat->nlink = 1;

	return 0;
}

static int ovl_permission(struct inode *inode, int mask, unsigned int flags)
{
	struct ovl_entry *oe;
	struct dentry *alias = NULL;
	struct inode *realinode;
	struct dentry *realdentry;
	bool is_upper;
	int err;

	if (S_ISDIR(inode->i_mode)) {
		oe = inode->i_private;
	} else if (flags & IPERM_FLAG_RCU) {
		return -ECHILD;
	} else {
		/*
		 * For non-directories find an alias and get the info
		 * from there.
		 */
		spin_lock(&inode->i_lock);
		if (WARN_ON(list_empty(&inode->i_dentry))) {
			spin_unlock(&inode->i_lock);
			return -ENOENT;
		}
		alias = list_entry(inode->i_dentry.next, struct dentry, d_alias);
		dget(alias);
		spin_unlock(&inode->i_lock);
		oe = alias->d_fsdata;
	}

	realdentry = ovl_upperdentry_dereference(oe);
	is_upper = true;
	if (!realdentry) {
		realdentry = oe->lowerdentry;
		is_upper = false;
	}

	/* Careful in RCU walk mode */
	realinode = ACCESS_ONCE(realdentry->d_inode);
	if (!realinode) {
		WARN_ON(!(flags & IPERM_FLAG_RCU));
		return -ENOENT;
	}

	if (mask & MAY_WRITE) {
		umode_t mode = realinode->i_mode;

		/*
		 * Writes will always be redirected to upper layer, so
		 * ignore lower layer being read-only.
		 */
		err = -EROFS;
		if (is_upper && IS_RDONLY(realinode) &&
		    (S_ISREG(mode) || S_ISDIR(mode) || S_ISLNK(mode)))
			goto out_dput;

		/*
		 * Nobody gets write access to an immutable file.
		 */
		err = -EACCES;
		if (IS_IMMUTABLE(realinode))
			goto out_dput;
	}

	if (realinode->i_op->permission)
		err = realinode->i_op->permission(realinode, mask, flags);
	else
		err = generic_permission(realinode, mask, flags,
					 realinode->i_op->check_acl);
out_dput:
	dput(alias);
	return err;
}

static int ovl_create_object(struct dentry *dentry, int mode, dev_t rdev,
			     const char *link)
{
	int err;
	struct dentry *newdentry;
	struct dentry *upperdir;
	struct inode *inode;
	struct kstat stat = {
		.mode = mode,
		.rdev = rdev,
	};

	err = -ENOMEM;
	inode = ovl_new_inode(dentry->d_sb, mode, dentry->d_fsdata);
	if (!inode)
		goto out;

	err = ovl_copy_up(dentry->d_parent);
	if (err)
		goto out_iput;

	upperdir = ovl_dentry_upper(dentry->d_parent);
	mutex_lock_nested(&upperdir->d_inode->i_mutex, I_MUTEX_PARENT);

	newdentry = ovl_upper_create(upperdir, dentry, &stat, link);
	err = PTR_ERR(newdentry);
	if (IS_ERR(newdentry))
		goto out_unlock;

	ovl_dentry_version_inc(dentry->d_parent);
	if (ovl_dentry_is_opaque(dentry) && S_ISDIR(mode)) {
		err = ovl_set_opaque(newdentry);
		if (err) {
			vfs_rmdir(upperdir->d_inode, newdentry);
			ovl_whiteout(upperdir, dentry);
			goto out_dput;
		}
	}
	ovl_dentry_update(dentry, newdentry);
	d_instantiate(dentry, inode);
	inode = NULL;
	newdentry = NULL;
	err = 0;

out_dput:
	dput(newdentry);
out_unlock:
	mutex_unlock(&upperdir->d_inode->i_mutex);
out_iput:
	iput(inode);
out:
	return err;
}

static int ovl_create(struct inode *dir, struct dentry *dentry, int mode,
			struct nameidata *nd)
{
	return ovl_create_object(dentry, (mode & 07777) | S_IFREG, 0, NULL);
}

static int ovl_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
	return ovl_create_object(dentry, (mode & 07777) | S_IFDIR, 0, NULL);
}

static int ovl_mknod(struct inode *dir, struct dentry *dentry, int mode,
		       dev_t rdev)
{
	return ovl_create_object(dentry, mode, rdev, NULL);
}

static int ovl_symlink(struct inode *dir, struct dentry *dentry,
			 const char *link)
{
	return ovl_create_object(dentry, S_IFLNK, 0, link);
}

struct ovl_link_data {
	struct dentry *realdentry;
	void *cookie;
};

static void *ovl_follow_link(struct dentry *dentry, struct nameidata *nd)
{
	void *ret;
	struct dentry *realdentry;
	struct inode *realinode;

	realdentry = ovl_dentry_real(dentry);
	realinode = realdentry->d_inode;

	if (WARN_ON(!realinode->i_op->follow_link))
		return ERR_PTR(-EPERM);

	ret = realinode->i_op->follow_link(realdentry, nd);
	if (IS_ERR(ret))
		return ret;

	if (realinode->i_op->put_link) {
		struct ovl_link_data *data;

		data = kmalloc(sizeof(struct ovl_link_data), GFP_KERNEL);
		if (!data) {
			realinode->i_op->put_link(realdentry, nd, ret);
			return ERR_PTR(-ENOMEM);
		}
		data->realdentry = realdentry;
		data->cookie = ret;

		return data;
	} else {
		return NULL;
	}
}

static void ovl_put_link(struct dentry *dentry, struct nameidata *nd, void *c)
{
	struct inode *realinode;
	struct ovl_link_data *data = c;

	if (!data)
		return;

	realinode = data->realdentry->d_inode;
	realinode->i_op->put_link(data->realdentry, nd, data->cookie);
	kfree(data);
}

static int ovl_readlink(struct dentry *dentry, char __user *buf, int bufsiz)
{
	struct path realpath;
	struct inode *realinode;

	ovl_path_real(dentry, &realpath);
	realinode = realpath.dentry->d_inode;

	if (!realinode->i_op->readlink)
		return -EINVAL;

	touch_atime(realpath.mnt, realpath.dentry);

	return realinode->i_op->readlink(realpath.dentry, buf, bufsiz);
}

static int ovl_do_remove(struct dentry *dentry, bool is_dir)
{
	int err;
	enum ovl_path_type type;
	struct path realpath;
	struct dentry *upperdir;

	err = ovl_copy_up(dentry->d_parent);
	if (err)
		return err;

	upperdir = ovl_dentry_upper(dentry->d_parent);
	mutex_lock_nested(&upperdir->d_inode->i_mutex, I_MUTEX_PARENT);
	type = ovl_path_real(dentry, &realpath);
	if (type != OVL_PATH_LOWER) {
		err = -ESTALE;
		if (realpath.dentry->d_parent != upperdir)
			goto out_d_drop;

		if (is_dir)
			err = vfs_rmdir(upperdir->d_inode, realpath.dentry);
		else
			err = vfs_unlink(upperdir->d_inode, realpath.dentry);
		if (err)
			goto out_d_drop;

		ovl_dentry_version_inc(dentry->d_parent);
	}

	if (type != OVL_PATH_UPPER || ovl_dentry_is_opaque(dentry))
		err = ovl_whiteout(upperdir, dentry);

	/*
	 * Keeping this dentry hashed would mean having to release
	 * upperpath/lowerpath, which could only be done if we are the
	 * sole user of this dentry.  Too tricky...  Just unhash for
	 * now.
	 */
out_d_drop:
	d_drop(dentry);
	mutex_unlock(&upperdir->d_inode->i_mutex);

	return err;
}

static int ovl_unlink(struct inode *dir, struct dentry *dentry)
{
	return ovl_do_remove(dentry, false);
}

static int ovl_check_empty_dir(struct dentry *dentry, struct list_head *list)
{
	int err;
	struct path lowerpath;
	struct path upperpath;
	struct ovl_cache_entry *p;
	struct ovl_readdir_data rdd = { .list = list };

	ovl_path_upper(dentry, &upperpath);
	ovl_path_lower(dentry, &lowerpath);

	err = ovl_dir_read_merged(&upperpath, &lowerpath, &rdd);
	if (err)
		return err;

	err = 0;

	list_for_each_entry(p, list, l_node) {
		if (p->is_whiteout)
			continue;

		if (p->name[0] == '.') {
			if (p->len == 1)
				continue;
			if (p->len == 2 && p->name[1] == '.')
				continue;
		}
		err = -ENOTEMPTY;
		break;
	}

	return err;
}

static int ovl_remove_whiteouts(struct dentry *dir, struct list_head *list)
{
	struct path upperpath;
	struct dentry *upperdir;
	struct ovl_cache_entry *p;
	const struct cred *old_cred;
	struct cred *override_cred;
	int ret = 0;

	ovl_path_upper(dir, &upperpath);
	upperdir = upperpath.dentry;

	override_cred = prepare_creds();
	if (!override_cred)
		return -ENOMEM;

	/*
	 * CAP_DAC_OVERRIDE for lookup and unlink
	 */
	cap_raise(override_cred->cap_effective, CAP_DAC_OVERRIDE);
	old_cred = override_creds(override_cred);

	mutex_lock(&upperdir->d_inode->i_mutex);
	list_for_each_entry(p, list, l_node) {
		if (p->is_whiteout) {
			struct dentry *dentry;

			dentry = lookup_one_len(p->name, upperdir, p->len);
			if (IS_ERR(dentry)) {
				ret = PTR_ERR(dentry);
				break;
			}
			ret = vfs_unlink(upperdir->d_inode, dentry);
			dput(dentry);
			if (ret)
				break;
		}
	}
	mutex_unlock(&upperdir->d_inode->i_mutex);

	revert_creds(old_cred);
	put_cred(override_cred);

	return ret;
}

static int ovl_check_empty_and_clear(struct dentry *dentry,
				     enum ovl_path_type type)
{
	int err;
	LIST_HEAD(list);

	err = ovl_check_empty_dir(dentry, &list);
	if (!err && type == OVL_PATH_MERGE)
		err = ovl_remove_whiteouts(dentry, &list);

	ovl_cache_free(&list);

	return err;
}

static int ovl_rmdir(struct inode *dir, struct dentry *dentry)
{
	int err;
	enum ovl_path_type type;

	type = ovl_path_type(dentry);
	if (type != OVL_PATH_UPPER) {
		err = ovl_check_empty_and_clear(dentry, type);
		if (err)
			return err;
	}

	return ovl_do_remove(dentry, true);
}

static int ovl_link(struct dentry *old, struct inode *newdir,
		    struct dentry *new)
{
	int err;
	struct dentry *olddentry;
	struct dentry *newdentry;
	struct dentry *upperdir;

	err = ovl_copy_up(old);
	if (err)
		goto out;

	err = ovl_copy_up(new->d_parent);
	if (err)
		goto out;

	upperdir = ovl_dentry_upper(new->d_parent);
	mutex_lock_nested(&upperdir->d_inode->i_mutex, I_MUTEX_PARENT);
	newdentry = ovl_lookup_create(upperdir, new);
	err = PTR_ERR(newdentry);
	if (IS_ERR(newdentry))
		goto out_unlock;

	olddentry = ovl_dentry_upper(old);
	err = vfs_link(olddentry, upperdir->d_inode, newdentry);
	if (!err) {
		ovl_dentry_version_inc(new->d_parent);
		ovl_dentry_update(new, newdentry);

		ihold(old->d_inode);
		d_instantiate(new, old->d_inode);
	} else {
		if (ovl_dentry_is_opaque(new))
			ovl_whiteout(upperdir, new);
		dput(newdentry);
	}
out_unlock:
	mutex_unlock(&upperdir->d_inode->i_mutex);
out:
	return err;

}

static int ovl_rename(struct inode *olddir, struct dentry *old,
			struct inode *newdir, struct dentry *new)
{
	int err;
	enum ovl_path_type old_type;
	struct dentry *old_upperdir;
	struct dentry *new_upperdir;
	struct dentry *olddentry;
	struct dentry *newdentry;
	struct dentry *trap;
	bool old_opaque;
	bool new_opaque;
	bool is_dir = S_ISDIR(old->d_inode->i_mode);

	/* Don't copy up directory trees */
	old_type = ovl_path_type(old);
	if (old_type != OVL_PATH_UPPER && is_dir)
		return -EXDEV;

	if (new->d_inode) {
		enum ovl_path_type new_type;

		new_type = ovl_path_type(new);

		if (new_type == OVL_PATH_LOWER && old_type == OVL_PATH_LOWER) {
			if (ovl_dentry_lower(old)->d_inode ==
			    ovl_dentry_lower(new)->d_inode)
				return 0;
		}
		if (new_type != OVL_PATH_LOWER && old_type != OVL_PATH_LOWER) {
			if (ovl_dentry_upper(old)->d_inode ==
			    ovl_dentry_upper(new)->d_inode)
				return 0;
		}

		if (new_type != OVL_PATH_UPPER &&
		    S_ISDIR(new->d_inode->i_mode)) {
			err = ovl_check_empty_and_clear(new, new_type);
			if (err)
				return err;
		}
	}

	err = ovl_copy_up(old);
	if (err)
		return err;

	err = ovl_copy_up(new->d_parent);
	if (err)
		return err;

	old_upperdir = ovl_dentry_upper(old->d_parent);
	new_upperdir = ovl_dentry_upper(new->d_parent);

	trap = lock_rename(new_upperdir, old_upperdir);

	olddentry = ovl_dentry_upper(old);
	newdentry = ovl_dentry_upper(new);
	if (newdentry) {
		dget(newdentry);
	} else {
		newdentry = ovl_lookup_create(new_upperdir, new);
		err = PTR_ERR(newdentry);
		if (IS_ERR(newdentry))
			goto out_unlock;
	}

	err = -ESTALE;
	if (olddentry->d_parent != old_upperdir)
		goto out_dput;
	if (newdentry->d_parent != new_upperdir)
		goto out_dput;
	if (olddentry == trap)
		goto out_dput;
	if (newdentry == trap)
		goto out_dput;

	old_opaque = ovl_dentry_is_opaque(old);
	new_opaque = ovl_dentry_is_opaque(new) ||
		ovl_path_type(new) != OVL_PATH_UPPER;

	if (is_dir && !old_opaque && new_opaque) {
		err = ovl_set_opaque(olddentry);
		if (err)
			goto out_dput;
	}

	err = vfs_rename(old_upperdir->d_inode, olddentry,
			 new_upperdir->d_inode, newdentry);

	if (err) {
		if (ovl_dentry_is_opaque(new))
			ovl_whiteout(new_upperdir, new);
		if (is_dir && !old_opaque && new_opaque)
			ovl_remove_opaque(olddentry);
		goto out_dput;
	}

	if (old_type != OVL_PATH_UPPER || old_opaque)
		err = ovl_whiteout(old_upperdir, old);
	if (is_dir && old_opaque && !new_opaque)
		ovl_remove_opaque(olddentry);

	if (old_opaque != new_opaque)
		ovl_dentry_set_opaque(old, new_opaque);

	ovl_dentry_version_inc(old->d_parent);
	ovl_dentry_version_inc(new->d_parent);

out_dput:
	dput(newdentry);
out_unlock:
	unlock_rename(new_upperdir, old_upperdir);
	return err;
}

static bool ovl_is_private_xattr(const char *name)
{
	return strncmp(name, "trusted.overlay.", 14) == 0;
}

static int ovl_setxattr(struct dentry *dentry, const char *name,
			  const void *value, size_t size, int flags)
{
	int err;
	struct dentry *upperdentry;

	if (ovl_is_private_xattr(name))
		return -EPERM;

	err = ovl_copy_up(dentry);
	if (err)
		return err;

	upperdentry = ovl_dentry_upper(dentry);
	return  vfs_setxattr(upperdentry, name, value, size, flags);
}

static ssize_t ovl_getxattr(struct dentry *dentry, const char *name,
			      void *value, size_t size)
{
	if (ovl_path_type(dentry->d_parent) == OVL_PATH_MERGE &&
	    ovl_is_private_xattr(name))
		return -ENODATA;

	return vfs_getxattr(ovl_dentry_real(dentry), name, value, size);
}

static ssize_t ovl_listxattr(struct dentry *dentry, char *list, size_t size)
{
	ssize_t res;
	int off;

	res = vfs_listxattr(ovl_dentry_real(dentry), list, size);
	if (res <= 0 || size == 0)
		return res;

	if (ovl_path_type(dentry->d_parent) != OVL_PATH_MERGE)
		return res;

	/* filter out private xattrs */
	for (off = 0; off < res;) {
		char *s = list + off;
		size_t slen = strlen(s) + 1;

		BUG_ON(off + slen > res);

		if (ovl_is_private_xattr(s)) {
			res -= slen;
			memmove(s, s + slen, res - off);
		} else {
			off += slen;
		}
	}

	return res;
}

static int ovl_removexattr(struct dentry *dentry, const char *name)
{
	int err;
	struct path realpath;
	enum ovl_path_type type;

	if (ovl_path_type(dentry->d_parent) == OVL_PATH_MERGE &&
	    ovl_is_private_xattr(name))
		return -ENODATA;

	type = ovl_path_real(dentry, &realpath);
	if (type == OVL_PATH_LOWER) {
		err = vfs_getxattr(realpath.dentry, name, NULL, 0);
		if (err < 0)
			return err;

		err = ovl_copy_up(dentry);
		if (err)
			return err;

		ovl_path_upper(dentry, &realpath);
	}

	return vfs_removexattr(realpath.dentry, name);
}

static bool ovl_open_need_copy_up(int flags, enum ovl_path_type type,
				  struct dentry *realdentry)
{
	if (type != OVL_PATH_LOWER)
		return false;

	if (special_file(realdentry->d_inode->i_mode))
		return false;

	if (!(OPEN_FMODE(flags) & FMODE_WRITE) && !(flags & O_TRUNC))
		return false;

	return true;
}

static struct file *ovl_open(struct dentry *dentry, int flags,
			     const struct cred *cred)
{
	int err;
	struct path realpath;
	enum ovl_path_type type;

	type = ovl_path_real(dentry, &realpath);
	if (ovl_open_need_copy_up(flags, type, realpath.dentry)) {
		if (flags & O_TRUNC)
			err = ovl_copy_up_truncate(dentry, 0);
		else
			err = ovl_copy_up(dentry);
		if (err)
			return ERR_PTR(err);

		ovl_path_upper(dentry, &realpath);
	}

	return vfs_open(&realpath, flags, cred);
}

static const struct inode_operations ovl_dir_inode_operations = {
	.lookup		= ovl_lookup,
	.mkdir		= ovl_mkdir,
	.symlink	= ovl_symlink,
	.unlink		= ovl_unlink,
	.rmdir		= ovl_rmdir,
	.rename		= ovl_rename,
	.link		= ovl_link,
	.setattr	= ovl_setattr,
	.create		= ovl_create,
	.mknod		= ovl_mknod,
	.permission	= ovl_permission,
	.getattr	= ovl_dir_getattr,
	.setxattr	= ovl_setxattr,
	.getxattr	= ovl_getxattr,
	.listxattr	= ovl_listxattr,
	.removexattr	= ovl_removexattr,
};

static const struct inode_operations ovl_file_inode_operations = {
	.setattr	= ovl_setattr,
	.permission	= ovl_permission,
	.getattr	= ovl_getattr,
	.setxattr	= ovl_setxattr,
	.getxattr	= ovl_getxattr,
	.listxattr	= ovl_listxattr,
	.removexattr	= ovl_removexattr,
	.open		= ovl_open,
};

static const struct inode_operations ovl_symlink_inode_operations = {
	.setattr	= ovl_setattr,
	.follow_link	= ovl_follow_link,
	.put_link	= ovl_put_link,
	.readlink	= ovl_readlink,
	.getattr	= ovl_getattr,
	.setxattr	= ovl_setxattr,
	.getxattr	= ovl_getxattr,
	.listxattr	= ovl_listxattr,
	.removexattr	= ovl_removexattr,
};

static struct inode *ovl_new_inode(struct super_block *sb, umode_t mode,
				   struct ovl_entry *oe)
{
	struct inode *inode;

	inode = new_inode(sb);
	if (!inode)
		return NULL;

	mode &= S_IFMT;

	inode->i_ino = get_next_ino();
	inode->i_mode = mode;
	inode->i_flags |= S_NOATIME | S_NOCMTIME;

	switch (mode) {
	case S_IFDIR:
		inode->i_private = oe;
		inode->i_op = &ovl_dir_inode_operations;
		inode->i_fop = &ovl_dir_operations;
		break;

	case S_IFLNK:
		inode->i_op = &ovl_symlink_inode_operations;
		break;

	case S_IFREG:
	case S_IFSOCK:
	case S_IFBLK:
	case S_IFCHR:
	case S_IFIFO:
		inode->i_op = &ovl_file_inode_operations;
		break;

	default:
		WARN(1, "illegal file type: %i\n", mode);
		inode = NULL;
	}

	return inode;

}

static void ovl_put_super(struct super_block *sb)
{
	struct ovl_fs *ufs = sb->s_fs_info;

	if (!(sb->s_flags & MS_RDONLY))
		mnt_drop_write(ufs->upper_mnt);

	mntput(ufs->upper_mnt);
	mntput(ufs->lower_mnt);

	kfree(ufs);
}

static int ovl_remount_fs(struct super_block *sb, int *flagsp, char *data)
{
	int flags = *flagsp;
	struct ovl_fs *ufs = sb->s_fs_info;

	/* When remounting rw or ro, we need to adjust the write access to the
	 * upper fs.
	 */
	if (((flags ^ sb->s_flags) & MS_RDONLY) == 0)
		/* No change to readonly status */
		return 0;

	if (flags & MS_RDONLY) {
		mnt_drop_write(ufs->upper_mnt);
		return 0;
	} else
		return mnt_want_write(ufs->upper_mnt);
}

/**
 * ovl_statfs
 * @sb: The overlayfs super block
 * @buf: The struct kstatfs to fill in with stats
 *
 * Get the filesystem statistics.  As writes always target the upper layer
 * filesystem pass the statfs to the same filesystem.
 */
static int ovl_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct dentry *root_dentry = dentry->d_sb->s_root;
	struct path path;
	ovl_path_upper(root_dentry, &path);

	if (!path.dentry->d_sb->s_op->statfs)
		return -ENOSYS;
	return path.dentry->d_sb->s_op->statfs(path.dentry, buf);
}

static const struct super_operations ovl_super_operations = {
	.put_super	= ovl_put_super,
	.remount_fs	= ovl_remount_fs,
	.statfs		= ovl_statfs,
};

struct ovl_config {
	char *lowerdir;
	char *upperdir;
};

enum {
	Opt_lowerdir,
	Opt_upperdir,
	Opt_err,
};

static const match_table_t ovl_tokens = {
	{Opt_lowerdir,			"lowerdir=%s"},
	{Opt_upperdir,			"upperdir=%s"},
	{Opt_err,			NULL}
};

static int ovl_parse_opt(char *opt, struct ovl_config *config)
{
	char *p;

	config->upperdir = NULL;
	config->lowerdir = NULL;

	while ((p = strsep(&opt, ",")) != NULL) {
		int token;
		substring_t args[MAX_OPT_ARGS];

		if (!*p)
			continue;

		token = match_token(p, ovl_tokens, args);
		switch (token) {
		case Opt_upperdir:
			kfree(config->upperdir);
			config->upperdir = match_strdup(&args[0]);
			if (!config->upperdir)
				return -ENOMEM;
			break;

		case Opt_lowerdir:
			kfree(config->lowerdir);
			config->lowerdir = match_strdup(&args[0]);
			if (!config->lowerdir)
				return -ENOMEM;
			break;

		default:
			return -EINVAL;
		}
	}
	return 0;
}

static int ovl_fill_super(struct super_block *sb, void *data, int silent)
{
	struct path lowerpath;
	struct path upperpath;
	struct inode *root_inode;
	struct dentry *root_dentry;
	struct ovl_entry *oe;
	struct ovl_fs *ufs;
	struct ovl_config config;
	int err;

	err = ovl_parse_opt((char *) data, &config);
	if (err)
		goto out;

	err = -EINVAL;
	if (!config.upperdir || !config.lowerdir) {
		printk(KERN_ERR "overlayfs: missing upperdir or lowerdir\n");
		goto out_free_config;
	}

	err = -ENOMEM;
	ufs = kmalloc(sizeof(struct ovl_fs), GFP_KERNEL);
	if (!ufs)
		goto out_free_config;

	oe = ovl_alloc_entry();
	if (oe == NULL)
		goto out_free_ufs;

	root_inode = ovl_new_inode(sb, S_IFDIR, oe);
	if (!root_inode)
		goto out_free_oe;

	err = kern_path(config.upperdir, LOOKUP_FOLLOW, &upperpath);
	if (err)
		goto out_put_root;

	err = kern_path(config.lowerdir, LOOKUP_FOLLOW, &lowerpath);
	if (err)
		goto out_put_upperpath;

	err = -ENOTDIR;
	if (!S_ISDIR(upperpath.dentry->d_inode->i_mode) ||
	    !S_ISDIR(lowerpath.dentry->d_inode->i_mode))
		goto out_put_lowerpath;

	ufs->upper_mnt = clone_private_mount(&upperpath);
	err = PTR_ERR(ufs->upper_mnt);
	if (IS_ERR(ufs->upper_mnt)) {
		printk(KERN_ERR "overlayfs: failed to clone upperpath\n");
		goto out_put_lowerpath;
	}

	ufs->lower_mnt = clone_private_mount(&lowerpath);
	err = PTR_ERR(ufs->lower_mnt);
	if (IS_ERR(ufs->lower_mnt)) {
		printk(KERN_ERR "overlayfs: failed to clone lowerpath\n");
		goto out_put_upper_mnt;
	}

	if (!(sb->s_flags & MS_RDONLY)) {
		err = mnt_want_write(ufs->upper_mnt);
		if (err)
			goto out_put_lower_mnt;
	}

	err = -ENOMEM;
	root_dentry = d_alloc_root(root_inode);
	if (!root_dentry)
		goto out_drop_write;

	mntput(upperpath.mnt);
	mntput(lowerpath.mnt);

	oe->__upperdentry = upperpath.dentry;
	oe->lowerdentry = lowerpath.dentry;

	root_dentry->d_fsdata = oe;
	root_dentry->d_op = &ovl_dentry_operations;

	sb->s_op = &ovl_super_operations;
	sb->s_root = root_dentry;
	sb->s_fs_info = ufs;

	return 0;

out_drop_write:
	if (!(sb->s_flags & MS_RDONLY))
		mnt_drop_write(ufs->upper_mnt);
out_put_lower_mnt:
	mntput(ufs->lower_mnt);
out_put_upper_mnt:
	mntput(ufs->upper_mnt);
out_put_lowerpath:
	path_put(&lowerpath);
out_put_upperpath:
	path_put(&upperpath);
out_put_root:
	iput(root_inode);
out_free_oe:
	kfree(oe);
out_free_ufs:
	kfree(ufs);
out_free_config:
	kfree(config.lowerdir);
	kfree(config.upperdir);
out:
	return err;
}

static struct dentry *ovl_mount(struct file_system_type *fs_type, int flags,
				const char *dev_name, void *raw_data)
{
	return mount_nodev(fs_type, flags, raw_data, ovl_fill_super);
}

static struct file_system_type ovl_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "overlayfs",
	.mount		= ovl_mount,
	.kill_sb	= kill_anon_super,
};

static int __init ovl_init(void)
{
	return register_filesystem(&ovl_fs_type);
}

static void __exit ovl_exit(void)
{
	unregister_filesystem(&ovl_fs_type);
}

module_init(ovl_init);
module_exit(ovl_exit);
