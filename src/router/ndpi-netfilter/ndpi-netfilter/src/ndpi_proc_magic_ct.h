int nmagic_ct_proc_open(struct inode *inode, struct file *file); 

int nmagic_ct_proc_close(struct inode *inode, struct file *file);

ssize_t nmagic_ct_proc_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos);
ssize_t nmagic_ct_proc_write(struct file *file, const char __user *buffer,
			 size_t length, loff_t *loff);
