/*
 * GPIO char driver
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: gpio.c,v 1.5 2008/04/03 03:49:45 Exp $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/uaccess.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include "ext_io.h"

static si_t *gpio_sih;
static int gpio_major;
static struct {
	char *name;
} gpio_file[] = {
	{ "in"},
	{ "out"},
	{ "outen"},
	{ "control"}
};

static int gpio_init_flag = 0;
static int __init gpio_init(void);

#define WNR3500V2_WPS_LED_GPIO			1
#define WNR3500V2_CONNECTED_LED_GPIO	2
#define WNR3500V2_PWR_LED_GPIO			3  //pwr led green
#define WNR3500V2_DIAG_LED_GPIO			7  //pwr led amber
#define WNR3500V2_USB_PSU_GPIO			12

static int
gpio_open(struct inode *inode, struct file * file)
{
	if (MINOR(inode->i_rdev) > ARRAYSIZE(gpio_file))
		return -ENODEV;

	MOD_INC_USE_COUNT;
	return 0;
}

static int
gpio_release(struct inode *inode, struct file * file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t
gpio_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	u32 val;
	switch (MINOR(file->f_dentry->d_inode->i_rdev)) {
	case 0:
		val = si_gpioin(gpio_sih);
		break;
	case 1:
		val = si_gpioout(gpio_sih, 0, 0, GPIO_APP_PRIORITY);
		break;
	case 2:
		val = si_gpioouten(gpio_sih, 0, 0, GPIO_APP_PRIORITY);
		break;
	case 3:
		val = si_gpiocontrol(gpio_sih, 0, 0, GPIO_APP_PRIORITY);
		break;
	default:
		return -ENODEV;
	}

	if (put_user(val, (u32 *) buf))
		return -EFAULT;

	return sizeof(val);
}

int gpio_kernel_api(unsigned int cmd, unsigned int mask, unsigned int val)
{

	if (gpio_init_flag != 1) {
		if (gpio_init() != 0)
			return -EFAULT;
	}
	
	switch (cmd) {
		case 0:
			si_gpioout(gpio_sih, mask, val, GPIO_HI_PRIORITY);
			break;
		case 1:
			si_gpioouten(gpio_sih, mask, val, GPIO_HI_PRIORITY);
			break;
		case 3:
			si_gpioreserve(gpio_sih, mask, GPIO_HI_PRIORITY);
			break;
		default:
			printk("Unknown gpio_kenerl_api command\n");
			break;
	}

	return 0;
}
EXPORT_SYMBOL (gpio_kernel_api);

static ssize_t
gpio_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	u32 val;

	if (get_user(val, (u32 *) buf))
		return -EFAULT;

	switch (MINOR(file->f_dentry->d_inode->i_rdev)) {
	case 0:
		return -EACCES;
	case 1:
		si_gpioout(gpio_sih, ~0, val, GPIO_APP_PRIORITY);
		break;
	case 2:
		si_gpioouten(gpio_sih, ~0, val, GPIO_APP_PRIORITY);
		break;
	case 3:
		si_gpiocontrol(gpio_sih, ~0, val, GPIO_APP_PRIORITY);
		break;
	default:
		return -ENODEV;
	}

	return sizeof(val);
}

static struct file_operations gpio_fops = {
	owner:		THIS_MODULE,
	open:		gpio_open,
	release:	gpio_release,
	read:		gpio_read,
	write:		gpio_write,
};

extern int iswrt350n;
extern int iswrt300n11;
extern int iswnr3500v2;
extern int iswrt320n;
extern int iswrt160nv3;
static struct class *gpio_class = NULL;

static int __init
gpio_init(void)
{
	int i;

	if (!(gpio_sih = si_kattach(SI_OSH)))
		return -ENODEV;

	si_gpiosetcore(gpio_sih);

	if ((gpio_major = register_chrdev(127, "gpio", &gpio_fops)) < 0)
	{
		return gpio_major;
	}

//	devfs_mk_cdev(MKDEV(gpio_major, 0), S_IFCHR | S_IRUGO | S_IWUGO, "gpio");

//	devfs_mk_dir("gpio");
	gpio_class = class_create(THIS_MODULE, "gpio");

	for (i = 0; i < ARRAYSIZE(gpio_file); i++) {
//		register_chrdev(MKDEV(127, i), gpio_file[i].name, &gpio_fops);
		class_device_create(gpio_class, NULL, MKDEV(127, i), NULL, gpio_file[i].name);
//		printk("gpio dev %d created\n",dev);
//		devfs_mk_cdev(MKDEV(127, i), S_IFCHR | S_IRUGO | S_IWUGO, gpio_file[i].name);
		
	}
gpio_init_flag=1;

if (iswrt350n)
{
	printk(KERN_EMERG "WRT350N GPIO Init\n");
	/* For WRT350N USB LED control */
	si_gpioreserve(gpio_sih, 0x400, GPIO_HI_PRIORITY);
	si_gpioouten(gpio_sih, 0x400, 0x400, GPIO_HI_PRIORITY);
	si_gpioreserve(gpio_sih, 0x800, GPIO_HI_PRIORITY);
	si_gpioouten(gpio_sih, 0x800, 0x800, GPIO_HI_PRIORITY);

	//if (nvram_match("disabled_5397", "1")) {
//		printk("5397 switch GPIO-Reset \n");
	//}
		
	USB_SET_LED(USB_DISCONNECT); //2005-02-24 by kanki for USB LED

}
if (iswrt350n)
{
		
		si_gpioreserve(gpio_sih, 0x4, GPIO_HI_PRIORITY);
		si_gpioouten(gpio_sih, 0x4, 0x4, GPIO_HI_PRIORITY);
		si_gpioout(gpio_sih, 0x4, 0x4, GPIO_HI_PRIORITY);
}
if (iswnr3500v2)
{
		printk(KERN_EMERG "WNR3500V2 GPIO Init\n");
		si_gpioreserve(gpio_sih, 1 << WNR3500V2_WPS_LED_GPIO, GPIO_APP_PRIORITY);
		si_gpioreserve(gpio_sih, 1 << WNR3500V2_CONNECTED_LED_GPIO, GPIO_APP_PRIORITY);
		si_gpioreserve(gpio_sih, 1 << WNR3500V2_PWR_LED_GPIO, GPIO_APP_PRIORITY);
		si_gpioreserve(gpio_sih, 1 << WNR3500V2_DIAG_LED_GPIO, GPIO_APP_PRIORITY);
//		si_gpioreserve(gpio_sih, 1 << WNR3500V2_USB_PSU_GPIO, GPIO_HI_PRIORITY);
//		si_gpioouten(gpio_sih, 1 << WNR3500V2_USB_PSU_GPIO, 1 << WNR3500V2_USB_PSU_GPIO, GPIO_HI_PRIORITY);
//		si_gpioout(gpio_sih, 1 << WNR3500V2_USB_PSU_GPIO, 1 << WNR3500V2_USB_PSU_GPIO, GPIO_HI_PRIORITY);
	
}
if (iswrt320n)
{
		printk(KERN_EMERG "WRT320N GPIO Init\n");
		si_gpioreserve(gpio_sih, 1 << 2, GPIO_APP_PRIORITY); //diag led
		si_gpioreserve(gpio_sih, 1 << 3, GPIO_APP_PRIORITY); //wps_led
		si_gpioreserve(gpio_sih, 1 << 4, GPIO_APP_PRIORITY); //wps_status_led
}

if (iswrt160nv3)
{
		printk(KERN_EMERG "WRT160Nv3 GPIO Init\n");
		si_gpioreserve(gpio_sih, 1 << 1, GPIO_APP_PRIORITY); //pwr led		
		si_gpioreserve(gpio_sih, 1 << 2, GPIO_APP_PRIORITY); //ses_orange
		si_gpioreserve(gpio_sih, 1 << 4, GPIO_APP_PRIORITY); //ses_white
}

if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x11")
	    && nvram_match("boardtype", "0x048e") && nvram_match("melco_id", "32093"))
{
		printk(KERN_EMERG "WHR-G125 GPIO Init\n");
		si_gpioreserve(gpio_sih, 1 << 1, GPIO_APP_PRIORITY);	
		si_gpioreserve(gpio_sih, 1 << 6, GPIO_APP_PRIORITY);
		si_gpioreserve(gpio_sih, 1 << 7, GPIO_APP_PRIORITY);
}

if ((nvram_match("boardtype", "0x04cf") && nvram_match("boot_hw_model", "WRT610N"))
{
		printk(KERN_EMERG "WRT610Nv2 GPIO Init\n");
		si_gpioreserve(gpio_sih, 1 << 0, GPIO_APP_PRIORITY);	
		si_gpioreserve(gpio_sih, 1 << 3, GPIO_APP_PRIORITY);
		si_gpioreserve(gpio_sih, 1 << 5, GPIO_APP_PRIORITY);
		si_gpioreserve(gpio_sih, 1 << 7, GPIO_APP_PRIORITY);
}
/*if (iswrt300n11)
{
	printk(KERN_EMERG "WRT300N v1.1 GPIO Init\n");
		int reset = 1 << 8;
		sb_gpioout(gpio_sbh, reset, 0, GPIO_DRV_PRIORITY);
		sb_gpioouten(gpio_sbh, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(50);
		sb_gpioout(gpio_sbh, reset, reset, GPIO_DRV_PRIORITY);
		bcm_mdelay(20);	
}*/

	return 0;
}

static void __exit
gpio_exit(void)
{
	int i;

//	for (i = 0; i < ARRAYSIZE(gpio_file); i++)
//		devfs_unregister(gpio_file[i].handle);
//	devfs_unregister(gpio_dir);
//	devfs_unregister_chrdev(gpio_major, "gpio");
	si_detach(gpio_sih);
}

module_init(gpio_init);
module_exit(gpio_exit);
