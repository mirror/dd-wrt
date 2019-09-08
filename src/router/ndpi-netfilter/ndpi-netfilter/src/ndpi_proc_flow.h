#ifndef NDPI_PROC_FLOW_H
#define NDPI_PROC_FLOW_H
static void nflow_proc_read_start(struct ndpi_net *n);

static int nflow_proc_open(struct inode *inode, struct file *file); 

static int nflow_proc_close(struct inode *inode, struct file *file);

static ssize_t ndpi_dump_acct_info(struct ndpi_net *n,char *buf, size_t buflen,
			    struct nf_ct_ext_ndpi *ct);

static size_t ndpi_dump_lost_rec(char *buf,size_t bufsize,
          uint32_t cpi, uint32_t cpo, uint64_t cbi, uint64_t cbo);

static size_t ndpi_dump_start_rec(char *buf,size_t bufsize, uint32_t tm);

static ssize_t ndpi_dump_acct_info_bin(struct ndpi_net *n, int v6,
	char *buf, size_t buflen, struct nf_ct_ext_ndpi *ct);

static ssize_t nflow_proc_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos);
static ssize_t nflow_proc_write(struct file *file, const char __user *buffer,
			 size_t length, loff_t *loff);

static loff_t nflow_proc_llseek(struct file *file, loff_t offset, int whence);
#endif