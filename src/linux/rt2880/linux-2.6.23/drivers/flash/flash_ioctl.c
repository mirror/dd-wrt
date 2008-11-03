#include <linux/init.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/fs.h>       
#include <linux/errno.h>    
#include <linux/types.h>    
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    
#include <asm/system.h>     
#include <linux/wireless.h>
#include <asm/uaccess.h>

#include "flash.h"
#include "flash_ioctl.h"

#undef CONFIG_DEVFS_FS
#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static	devfs_handle_t devfs_handle;
#endif

int	flash_major =  200;

extern u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
extern u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

int flash_ioctl (struct inode *inode, struct file *filp,
                     unsigned int cmd, unsigned long arg)
{
    struct flash_opt *opt=(struct flash_opt *)arg;
    unsigned char *buf;
    unsigned int tmp;
    unsigned int start_sect=0,end_sect=0;

    buf=kmalloc(FLASH_MAX_RW_SIZE, GFP_KERNEL);

    switch(cmd) 
    {
    case FLASH_IOCTL_READ:
	if(FlashRead( (unsigned int *)buf, opt->src, opt->bytes) < 0) {
	    opt->result=OUT_OF_SCOPE;
	}
	copy_to_user((char *)opt->dest,buf, opt->bytes);
	break;
    case FLASH_IOCTL_WRITE:
	copy_from_user( buf, (char *)opt->src, opt->bytes);
	if(FlashWrite((unsigned short *)buf, (unsigned short *)opt->dest, opt->bytes)<0){
	    opt->result = OUT_OF_SCOPE;
	}
	break;
    case FLASH_IOCTL_ERASE:
	if(FlashGetSector(opt->start_addr, &start_sect) && FlashGetSector(opt->end_addr, &end_sect)){
		printk("Erase Sector From %d To %d \n",start_sect, end_sect);
		if(FlashErase(start_sect, end_sect)<0) {
			opt->result = OUT_OF_SCOPE;
		}
	}
	break;
    default:
	break;
    }

    kfree(buf);
    return 0;
}

struct file_operations flash_fops = {
    ioctl:      flash_ioctl,
};


static int flash_init(void)
{

#ifdef  CONFIG_DEVFS_FS
    if(devfs_register_chrdev(flash_major, FLASH_DEVNAME , &flash_fops)) {
	printk(KERN_WARNING " flash: can't create device node - %s\n",FLASH_DEVNAME);
	return -EIO;
    }

    devfs_handle = devfs_register(NULL, FLASH_DEVNAME, DEVFS_FL_DEFAULT, flash_major, 0, 
	    S_IFCHR | S_IRUGO | S_IWUGO, &flash_fops, NULL);
#else
    int result=0;
    result = register_chrdev(flash_major, FLASH_DEVNAME, &flash_fops);
    if (result < 0) {
	printk(KERN_WARNING "flash: can't get major %d\n",flash_major);
        return result;
    }

    if (flash_major == 0) {
	flash_major = result; /* dynamic */
    }
#endif

    return 0;
}



static void flash_exit(void)
{
    printk("flash_exit\n");

#ifdef  CONFIG_DEVFS_FS
    devfs_unregister_chrdev(flash_major, FLASH_DEVNAME);
    devfs_unregister(devfs_handle);
#else
    unregister_chrdev(flash_major, FLASH_DEVNAME);
#endif

}

module_init(flash_init);
module_exit(flash_exit);
MODULE_LICENSE("GPL");
