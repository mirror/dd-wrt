#ifndef NDPI_PROC_FLOW_H
#define NDPI_PROC_FLOW_H

NDPI_STATIC void nflow_proc_read_start(struct ndpi_net *n);

NDPI_STATIC int nflow_proc_open(struct inode *inode, struct file *file); 

NDPI_STATIC int nflow_proc_close(struct inode *inode, struct file *file);

NDPI_STATIC size_t ndpi_dump_opt(char *buf, size_t bufsize,struct nf_ct_ext_ndpi *ct);
NDPI_STATIC ssize_t ndpi_dump_acct_info(struct ndpi_net *n, struct nf_ct_ext_ndpi *ct);

NDPI_STATIC size_t ndpi_dump_lost_rec(char *buf,size_t bufsize,
          uint32_t cpi, uint32_t cpo, uint64_t cbi, uint64_t cbo);

NDPI_STATIC size_t ndpi_dump_start_rec(char *buf,size_t bufsize, time64_t tm);

NDPI_STATIC ssize_t ndpi_dump_acct_info_bin(struct ndpi_net *n, int v6,
	char *buf, size_t buflen, struct nf_ct_ext_ndpi *ct);

NDPI_STATIC ssize_t nflow_proc_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos);
NDPI_STATIC ssize_t nflow_proc_write(struct file *file, const char __user *buffer,
			 size_t length, loff_t *loff);

NDPI_STATIC loff_t nflow_proc_llseek(struct file *file, loff_t offset, int whence);
#endif