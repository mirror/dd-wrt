
static int n_hostdef_proc_open(struct inode *inode, struct file *file);
static ssize_t n_hostdef_proc_read(struct file *file, char __user *buf,
				size_t count, loff_t *ppos);

static int n_hostdef_proc_close(struct inode *inode, struct file *file);

static ssize_t n_hostdef_proc_write(struct file *file, const char __user *buffer,
				size_t length, loff_t *loff);

