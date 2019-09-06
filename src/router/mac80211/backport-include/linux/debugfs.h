#ifndef __BACKPORT_DEBUGFS_H_
#define __BACKPORT_DEBUGFS_H_
#include_next <linux/debugfs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <generated/utsrelease.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
#define debugfs_real_fops(file) (file)->f_op
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
#define debugfs_create_devm_seqfile LINUX_BACKPORT(debugfs_create_devm_seqfile)
#if defined(CONFIG_DEBUG_FS)
struct dentry *debugfs_create_devm_seqfile(struct device *dev, const char *name,
					   struct dentry *parent,
					   int (*read_fn)(struct seq_file *s,
							  void *data));
#else
static inline struct dentry *debugfs_create_devm_seqfile(struct device *dev,
							 const char *name,
							 struct dentry *parent,
					   int (*read_fn)(struct seq_file *s,
							  void *data))
{
	return ERR_PTR(-ENODEV);
}
#endif /* CONFIG_DEBUG_FS */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
#define debugfs_create_bool LINUX_BACKPORT(debugfs_create_bool)
#ifdef CONFIG_DEBUG_FS
struct dentry *debugfs_create_bool(const char *name, umode_t mode,
				   struct dentry *parent, bool *value);
#else
static inline struct dentry *
debugfs_create_bool(const char *name, umode_t mode,
		    struct dentry *parent, bool *value)
{
	return ERR_PTR(-ENODEV);
}
#endif
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0) */

#endif /* __BACKPORT_DEBUGFS_H_ */
