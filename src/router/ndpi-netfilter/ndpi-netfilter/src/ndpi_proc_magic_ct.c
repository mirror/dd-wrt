#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/netfilter.h>

#include "ndpi_main.h"

#include "ndpi_main_common.h"
#include "ndpi_strcol.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_magic_ct.h"

int nmagic_ct_proc_open(struct inode *inode, struct file *file) {
	return 0;
}

int nmagic_ct_proc_close(struct inode *inode, struct file *file) {
	return 0;
}

ssize_t nmagic_ct_proc_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos) {
	struct ndpi_net *n = pde_data(file_inode(file));

	char lbuf[128];
	int len = 0;

	if (*ppos > 0) {
		return 0;
	}
	if (*ppos < 0) {
		return -EINVAL;
	}

	len += scnprintf(lbuf, sizeof(lbuf), "%hu\n", n->magic_ct);
	if (len > count) {
		return -EINVAL;
	}

	if (copy_to_user(buf, lbuf, len)) {
		return -EFAULT;
	}

	*ppos = len;
	return len;
}

ssize_t nmagic_ct_proc_write(struct file *file, const char __user *buffer,
			 size_t length, loff_t *loff) {
	struct ndpi_net *n = pde_data(file_inode(file));

	char lbuf[128];
	unsigned int new_magic_ct;

	if (*loff != 0 || length >= sizeof(lbuf)) {
		return -EINVAL;
	}

	if (copy_from_user(lbuf, buffer, length)) {
		return -EFAULT;
	}

	lbuf[length] = '\0';
	if (kstrtouint(lbuf, 10, &new_magic_ct) != 0 || new_magic_ct == 0 || new_magic_ct > USHRT_MAX) {
		return -EINVAL;
	}
	if(atomic64_read(&n->protocols_cnt[0]) == 0) {
		n->magic_ct = new_magic_ct;
		return length;
	}
	return -EINVAL;
}
