/*
 * ixp425_gpio.c : Intel IXP425 GPIO driver Linux
 *                 Copyright (C) 2004, Tim Harvey <tim_harvey@yahoo.com>
 *                 Hautespot Networks, www.hautespot.net
 *
 * Description:
 *
 *    This driver allows access to the IXP425 Digital IO
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * TODO: 
 *   - extend to allow interrupts
 *
 */

#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>

#include <asm/uaccess.h>	/* copy_to_user, copy_from_user */
//#include <asm-arm/arch-ixp4xx/gpio.h>

//#include <linux/ixp425-gpio.h>

#include <asm-arm/hardware.h>
#include <asm-arm/arch-ixp4xx/ixp4xx-regs.h>

struct gpio_bit {
  unsigned char bit;
  unsigned char state;
};

#define DEVICE_NAME "gpio"
#define GPIO_MAJOR 127
//#define DEBUG

/*
 * GPIO interface
 */

/* /dev/gpio */
static int gpio_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg);

/* /proc/driver/gpio */
static int gpio_read_proc(char *page, char **start, off_t off,
                         int count, int *eof, void *data);

static unsigned char gpio_status;        /* bitmapped status byte.       */

#define ERROR   (-1)
#define OK                      0
#define ixp425GPIOLineConfig    gpio_line_config
#define ixp425GPIOLineGet       gpio_line_get
#define ixp425GPIOLineSet       gpio_line_set
#define sysMicroDelay(x)        udelay(x)
#define IXP425_GPIO_SIG         int

#define GPIO_IS_OPEN             0x01    /* means /dev/gpio is in use     */

/*
 *      Now all the various file operations that we export.
 */
static int gpio_ioctl(struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg)
{
	struct gpio_bit bit;
	IXP425_GPIO_SIG val;

	if (copy_from_user(&bit, (struct gpio_bit *)arg, 
				sizeof(bit)))
		return -EFAULT;
#ifdef DEBUG
printk("ixp425_gpio: ioctl cmd 0x%02x, bit %d, state %d\n", cmd, bit.bit, bit.state);
#endif

        switch (cmd) {

        case GPIO_GET_BIT:
		ixp425GPIOLineGet(bit.bit, &val);
		bit.state = val;
#ifdef DEBUG
             	printk("ixp425_gpio: Read _bit 0x%02x %s\n", bit.bit, (bit.state==0)?"LOW":"HIGH");
#endif
		return copy_to_user((void *)arg, &bit, sizeof(bit)) ? -EFAULT : 0;
        case GPIO_SET_BIT:
#ifdef DEBUG
             	printk("ixp425_gpio: Write _bit 0x%02x %s\n", bit.bit, (bit.state==0)?"LOW":"HIGH");
#endif
		val = bit.state;
		ixp425GPIOLineSet(bit.bit, val);
		return OK;
	case GPIO_GET_CONFIG:
          	ixp425GPIOLineConfig(bit.bit, bit.state);
#ifdef DEBUG
             	printk("ixp425_gpio: Read config _bit 0x%02x %s\n", bit.bit, (bit.state==IXP425_GPIO_IN)?"IN":"OUT");
#endif
		return copy_to_user((void *)arg, &bit, sizeof(bit)) ? -EFAULT : 0;
	case GPIO_SET_CONFIG:
		val = bit.state;
#ifdef DEBUG
             	printk("ixp425_gpio: Write config _bit 0x%02x %s\n", bit.bit, (bit.state==IXP425_GPIO_IN)?"IN":"OUT");
#endif
          	ixp425GPIOLineConfig(bit.bit, bit.state);
		return OK;
	}
	return -EINVAL;
}


static int gpio_open(struct inode *inode, struct file *file)
{
        if (gpio_status & GPIO_IS_OPEN)
                return -EBUSY; 

        gpio_status |= GPIO_IS_OPEN;

        return 0;
}


static int gpio_release(struct inode *inode, struct file *file)
{
        /*
         * Turn off all interrupts once the device is no longer
         * in use and clear the data.
         */

        gpio_status &= ~GPIO_IS_OPEN;

        return 0;
}


/*
 *      The various file operations we support.
 */

static struct file_operations gpio_fops = {
        .owner          = THIS_MODULE,
        .ioctl          = gpio_ioctl,
        .open           = gpio_open,
        .release        = gpio_release,
};

static struct miscdevice gpio_dev =
{
        .minor          = 0,
        .name           = "gpio",
        .fops           = &gpio_fops,
};




#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *dir;

/*
 *      Info exported via "/proc/driver/gpio".
 */
static int gpio_get_status(char *buf)
{
        char *p;

        p = buf;

        /*
         */
	u8 val = 0;
	int i;
	int bit;
	for (i = 0; i < 16; i++) {
		gpio_line_get(i, &bit);
		if (bit)
			val |= (1 << i);
	}
        p += sprintf(p, "gpio_config\t: 0x%04x\n", val);

        return p - buf;
}


/* /proc/driver/gpio read op 
 */
static int gpio_read_proc(char *page, char **start, off_t off,
                             int count, int *eof, void *data)
{
        int len = gpio_get_status (page);
        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len>count) len = count;
        if (len<0) len = 0;
        return len;
}
#endif /* CONFIG_PROC_FS */


static int __init gpio_init_module(void)
{
        int retval;

        /* register /dev/gpio file ops */
	retval = register_chrdev(GPIO_MAJOR, DEVICE_NAME, &gpio_fops);
        if(retval < 0)
                return retval;

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *res;
	dir = proc_mkdir("driver/gpio", NULL);
	if (!dir) {
		misc_deregister(&gpio_dev);
		return -ENOMEM;
	}
        /* register /proc/driver/gpio */
	res = create_proc_entry("info", 644, dir);
	if (res) {
		res->read_proc= gpio_read_proc;
	} else {
		misc_deregister(&gpio_dev);
		return -ENOMEM;
	}
#endif

	printk("ixp425_gpio: IXP425 GPIO driver loaded\n");

	return 0; 
}

static void __exit gpio_cleanup_module(void)
{
	remove_proc_entry ("info", dir);
        misc_deregister(&gpio_dev);

	printk("ixp425_gpio: IXP425 GPIO driver unloaded\n");
}

module_init(gpio_init_module);
module_exit(gpio_cleanup_module);

MODULE_AUTHOR("Tim Harvey <tim_harvey@yahoo.com>");
MODULE_LICENSE("GPL");
