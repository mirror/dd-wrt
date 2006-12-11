/*
 * Linux Broadcom BCM47xx GPIO char driver
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: linux_gpio.c,v 1.3.4.1 2006/10/18 07:00:04 michael Exp $
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <sbutils.h>
#include <bcmdevs.h>

#include <linux_gpio.h>

/* handle to the sb */
static sb_t *gpio_sbh = NULL;

/* major number assigned to the device and device handles */
static int gpio_major;
devfs_handle_t gpiodev_handle;

static int gpio_init_flag = 0;
static int __init gpio_init(void);

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

int gpio_kernel_api(unsigned int cmd, unsigned int mask, unsigned int val)
{

	if (gpio_init_flag != 1) {
		if (gpio_init() != 0)
			return -EFAULT;
	}
	
	switch (cmd) {
		case 0:
			sb_gpioout(gpio_sbh, mask, val, GPIO_HI_PRIORITY);
			break;
		case 1:
			sb_gpioouten(gpio_sbh, mask, val, GPIO_HI_PRIORITY);
			break;
		case 3:
			sb_gpioreserve(gpio_sbh, mask, GPIO_HI_PRIORITY);
			break;
		default:
			printk("Unknown gpio_kenerl_api command\n");
			break;
	}

	return 0;
}
//EXPORT_SYMBOL (gpio_kernel_api);

static int
gpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct gpio_ioctl gpioioc;

	if (copy_from_user(&gpioioc, (struct gpio_ioctl *)arg, sizeof(struct gpio_ioctl)))
		return -EFAULT;


	switch (cmd) {
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
			gpioioc.val = sb_gpioout(gpio_sbh, gpioioc.mask, gpioioc.val,
			                         GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_OUTEN:
			gpioioc.val = sb_gpioouten(gpio_sbh, gpioioc.mask, gpioioc.val,
			                           GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_IN:
			gpioioc.val = sb_gpioin(gpio_sbh);
			break;
		default:
			break;
	}
	if (copy_to_user((struct gpio_ioctl *)arg, &gpioioc, sizeof(struct gpio_ioctl)))
		return -EFAULT;

	return 0;

}
static struct file_operations gpio_fops = {
	owner:		THIS_MODULE,
	open:		gpio_open,
	release:	gpio_release,
	ioctl:		gpio_ioctl,
	};

extern int iswrt350n;

static int __init
gpio_init(void)
{
	if (!(gpio_sbh = sb_kattach(SB_OSH)))
		return -ENODEV;

	if ((gpio_major = devfs_register_chrdev(0, "gpio", &gpio_fops)) < 0)
		return gpio_major;

	gpiodev_handle = devfs_register(NULL, "gpio", DEVFS_FL_DEFAULT,
	                                gpio_major, 0, S_IFCHR | S_IRUGO | S_IWUGO,
	                                &gpio_fops, NULL);
	printk(KERN_EMERG "WRT350N GPIO Init\n");

	gpio_init_flag = 1;

// BrainSlayer: WRT350N detection

/*char *boardnum = nvram_get("boardnum");
char *boardtype = nvram_get("boardtype");
char *cardbus = nvram_get("boardtype");
//char *boardflags = nvram_get("boardflags");
if (boardnum==NULL || strcmp(boardnum,"42"))return 0;
if (boardtype==NULL || strcmp(boardtype,"0x478"))return 0;
if (cardbus==NULL || strcmp(cardbus,"1"))return 0;
*/

if (iswrt350n)
{
	/* For WRT350N USB LED control */
	sb_gpioreserve(gpio_sbh, 0x400, GPIO_HI_PRIORITY);
	sb_gpioouten(gpio_sbh, 0x400, 0x400, GPIO_HI_PRIORITY);
	sb_gpioreserve(gpio_sbh, 0x800, GPIO_HI_PRIORITY);
	sb_gpioouten(gpio_sbh, 0x800, 0x800, GPIO_HI_PRIORITY);

	//if (nvram_match("disabled_5397", "1")) {
		printk("5397 switch GPIO-Reset \n");
		sb_gpioreserve(gpio_sbh, 0x4, GPIO_HI_PRIORITY);
		sb_gpioouten(gpio_sbh, 0x4, 0x4, GPIO_HI_PRIORITY);
		sb_gpioout(gpio_sbh, 0x4, 0x4, GPIO_HI_PRIORITY);
	//}
}
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
	gpio_init_flag = 0;
}

module_init(gpio_init);
module_exit(gpio_exit);

