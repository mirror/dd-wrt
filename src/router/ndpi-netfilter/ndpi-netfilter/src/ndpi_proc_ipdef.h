#ifndef NDPI_IPDEF_H
#define NDPI_IPDEF_H
NDPI_STATIC int n_ipdef_proc_open(struct inode *inode, struct file *file);

NDPI_STATIC int n_ipdef_proc_close(struct inode *inode, struct file *file);
NDPI_STATIC int n_ip6def_proc_close(struct inode *inode, struct file *file);

NDPI_STATIC ssize_t n_ipdef_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);
NDPI_STATIC ssize_t n_ip6def_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);

NDPI_STATIC ssize_t n_ipdef_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);
NDPI_STATIC ssize_t n_ip6def_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);
#endif