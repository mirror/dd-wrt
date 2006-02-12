/*
 * Linux Broadcom BCM47xx GPIO char driver
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$  
 * */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <sbutils.h>
#include <bcmdevs.h>

#include <linux_gpio.h>

/* handle to the sb */
static sb_t *gpio_sbh; 

/* major number assigned to the device and device handles */
static int gpio_major;
devfs_handle_t gpiodev_handle;

static int
gpio_open(struct inode *inode, struct file * file)
{
	MOD_INC_USE_COUNT;
	return 0;
}

static int
gpio_release(struct inode *inode, struct file * file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

static int
gpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct gpio_ioctl gpioioc;

	if (copy_from_user(&gpioioc, (struct gpio_ioctl *)arg, sizeof(struct gpio_ioctl)))
		return -EFAULT;

	switch(cmd) {
		case GPIO_IOC_RESERVE:
			gpioioc.val = sb_gpioreserve(gpio_sbh, gpioioc.mask, GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_RELEASE:
			/* 
 			* releasing the gpio doesn't change the current 
			* value on the GPIO last write value 
 			* persists till some one overwrites it
			*/
			gpioioc.val = sb_gpiorelease(gpio_sbh, gpioioc.mask, GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_OUT:
			gpioioc.val = sb_gpioout(gpio_sbh, gpioioc.mask, gpioioc.val,GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_OUTEN:
			gpioioc.val = sb_gpioouten(gpio_sbh, gpioioc.mask, gpioioc.val, GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_IN:
			gpioioc.val = sb_gpioin(gpio_sbh);
			break;
		default:
			break;
	}
	if (copy_to_user((struct gpio_ioctl *)arg, &gpioioc , sizeof(struct gpio_ioctl)))
		return -EFAULT;

	return 0;

}
static struct file_operations gpio_fops = {
	owner:		THIS_MODULE,
	open:		gpio_open,
	release:	gpio_release,
	ioctl:		gpio_ioctl,
};

static int __init
gpio_init(void)
{
	if (!(gpio_sbh = sb_kattach()))
		return -ENODEV;

	if ((gpio_major = devfs_register_chrdev(0, "gpio", &gpio_fops)) < 0)
		return gpio_major;

	gpiodev_handle = devfs_register(NULL, "gpio", DEVFS_FL_DEFAULT, 
			gpio_major, 0, S_IFCHR | S_IRUGO | S_IWUGO, 
			&gpio_fops, NULL);

	return 0;
}

static void __exit
gpio_exit(void)
{
	if (gpiodev_handle != NULL)
		devfs_unregister(gpiodev_handle);
	gpiodev_handle = NULL;
	devfs_unregister_chrdev(gpio_major, "gpio");
	sb_detach(gpio_sbh);
}

module_init(gpio_init);
module_exit(gpio_exit);
