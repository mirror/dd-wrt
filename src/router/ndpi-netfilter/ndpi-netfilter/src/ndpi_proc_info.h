#ifndef NDPI_PROC_INFO_H
#define NDPI_PROC_INFO_H

NDPI_STATIC ssize_t _ninfo_proc_read(struct ndpi_net *n, char __user *buf,
                              size_t count, loff_t *ppos,int family);

NDPI_STATIC ssize_t ninfo_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);

#ifdef NDPI_DETECTION_SUPPORT_IPV6
NDPI_STATIC ssize_t ninfo6_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);
#endif

NDPI_STATIC ssize_t ninfo_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);

#ifdef BT_ANNOUNCE
NDPI_STATIC ssize_t nann_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos);
#endif

NDPI_STATIC ssize_t nproto_proc_read(struct file *file, char __user *buf,
                     size_t count, loff_t *ppos);

NDPI_STATIC int nproto_proc_close(struct inode *inode, struct file *file);

NDPI_STATIC ssize_t nproto_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);

NDPI_STATIC ssize_t ndebug_proc_read(struct file *file, char __user *buf,
                     size_t count, loff_t *ppos);

NDPI_STATIC int ndebug_proc_close(struct inode *inode, struct file *file);

NDPI_STATIC ssize_t ndebug_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);

NDPI_STATIC int risk_names(struct ndpi_net *n, char *lbuf,size_t count);

NDPI_STATIC int nrisk_proc_open(struct inode *inode, struct file *file);

NDPI_STATIC ssize_t nrisk_proc_read(struct file *file, char __user *buf,
                     size_t count, loff_t *ppos);

NDPI_STATIC ssize_t nrisk_proc_write(struct file *file, const char __user *buffer,
		                     size_t length, loff_t *loff);

NDPI_STATIC int nrisk_proc_close(struct inode *inode, struct file *file);
NDPI_STATIC int ncfg_proc_open(struct inode *inode, struct file *file);
NDPI_STATIC ssize_t ncfg_proc_read(struct file *file, char __user *buf,
                     size_t count, loff_t *ppos);
NDPI_STATIC ssize_t ncfg_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff);
NDPI_STATIC int ncfg_proc_close(struct inode *inode, struct file *file);
#endif