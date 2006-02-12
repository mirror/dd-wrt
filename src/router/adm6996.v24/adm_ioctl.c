#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

#include "adm6996.h"

adm_info_t* ainfo;

static int adm_ioctl(struct inode *inode, struct file *file,
		                 unsigned int cmd, unsigned long arg);

static int adm_ioctl(struct inode *inode, struct file *file, 
		                 unsigned int cmd, unsigned long arg)
{
	// struct adm_ioc aioc;
	uint32 val;
			struct adm_port_stat aps[6];
	
	switch (cmd) {
		case ADM_GET_CHIPID:
			adm_rreg (ainfo, 1, 0, &val);
			goto return_val;

		case ADM_GET_PORTS_NUM:
			val = 6;
			goto return_val;

		case ADM_GET_PORTS:
		{
			adm_get_ports_stats (6, aps);
			return copy_to_user((void *)arg, &aps, 50) ? -EFAULT : 0;
		}
 
		default:
		  return -EINVAL;
	}

return_val:
	return copy_to_user((void *)arg, &val, sizeof val) ? -EFAULT : 0;

}
