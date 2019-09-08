#ifndef NDPI_PROC_INFO_H
#define NDPI_PROC_INFO_H

static ssize_t _ninfo_proc_read(struct ndpi_net *n, char __user *buf,
                              size_t count, loff_t *ppos,int family);

static ssize_t ninfo_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);

#ifdef NDPI_DETECTION_SUPPORT_IPV6
static ssize_t ninfo6_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);
#endif

static ssize_t ninfo_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);

#ifdef BT_ANNOUNCE
static ssize_t nann_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);
#endif

static ssize_t nproto_proc_read(struct file *file, char __user *buf,
                     size_t count, loff_t *ppos);

static int nproto_proc_close(struct inode *inode, struct file *file);

static ssize_t nproto_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);

#endif