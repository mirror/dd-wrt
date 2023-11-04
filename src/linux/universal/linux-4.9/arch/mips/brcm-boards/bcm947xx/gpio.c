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
#include <linux/uaccess.h>

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
	switch (MINOR(file_inode(file)->i_rdev)) {
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
		case 2:
			return si_gpioin(gpio_sih);
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
	switch (MINOR(file_inode(file)->i_rdev)) {
	case 0:
		return -EACCES;
	case 1:
		si_gpioout(gpio_sih, ~0, val, GPIO_HI_PRIORITY);
		break;
	case 2:
		si_gpioouten(gpio_sih, ~0, val, GPIO_HI_PRIORITY);
		break;
	case 3:
		si_gpiocontrol(gpio_sih, ~0, val, GPIO_HI_PRIORITY);
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
int isbuffalo=0;
int isbuffalowxr = 0;
int isd1800h=0;
int isac66=0;
int isac68=0;
int isdefault=1;
EXPORT_SYMBOL(isd1800h);
EXPORT_SYMBOL(isac66);

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

gpio_init_flag=1;
int gpios = 0;

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

uint boardnum = bcm_strtoul( nvram_safe_get( "boardnum" ), NULL, 0 );

gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<14 || 1<<15;

if ((boardnum == 1 || boardnum == 3500)
	    && nvram_match("boardtype", "0x04CF")
	    && (nvram_match("boardrev", "0x1213") || nvram_match("boardrev", "02")))
{
		printk(KERN_EMERG "WNR3500V2 GPIO Init\n");
		gpios = 1 << 1 | 1 << 2 | 1 << 3 | 1 << 7;
}


if ((boardnum == 0) && nvram_match("boardtype", "0xf52e") && (nvram_match("boardrev", "0x1204")))
{
		printk(KERN_EMERG "Buffalo D1800H init\n");
		gpios = 0;
		isd1800h = 1;
		isdefault = 0;
}

if (nvram_match("productid", "RT-AC66U")) {
		printk(KERN_EMERG "Asus-RT-AC66U init\n");
		gpios = 0;
		isac66 = 1;
		isdefault = 0;
}
if (nvram_match("boardtype", "0xF5B2") && nvram_match("boardrev", "0x1100") && nvram_match("pci/2/1/sb20in80and160hr5ghpo", "0"))
{
		printk(KERN_EMERG "Asus-RT-AC66U init\n");
		gpios = 0;
		isac66 = 1;
		isdefault = 0;
}

if (nvram_match("productid", "RT-N66U")) {
		printk(KERN_EMERG "Asus-RT-AC66U init\n");
		gpios = 0;
		isac66 = 1;
		isdefault = 0;
}
if (nvram_match("boardtype", "0xF5B2") && nvram_match("boardrev", "0x1100") && !nvram_match("pci/2/1/sb20in80and160hr5ghpo", "0"))
{
		printk(KERN_EMERG "Asus-RT-N66U init\n");
		gpios = 0;
		isac66 = 1;
		isdefault = 0;
}

if ((boardnum == 42 || boardnum == 66)
		&& nvram_match("boardtype", "0x04EF")
		&& (nvram_match("boardrev", "0x1304") || nvram_match("boardrev", "0x1305") || nvram_match("boardrev", "0x1307")))
{
		printk(KERN_EMERG "WRT320N/E2000 GPIO Init\n");
		gpios = 1 << 2 | 1 << 3 | 1 << 4;
}

if (boardnum == 42 && ((nvram_match("boot_hw_model", "WRT160N") && nvram_match("boot_hw_ver", "3.0")) 
		|| (nvram_match("boot_hw_model", "M10") && nvram_match("boot_hw_ver", "1.0")) 
		|| (nvram_match("boot_hw_model", "E100") && nvram_match("boot_hw_ver", "1.0")) ) )
{
		printk(KERN_EMERG "WRT160Nv3/M10/E1000 GPIO Init\n");
		gpios = 1 << 1 | 1 << 2 | 1 << 4;
}

if (boardnum == 42 && ((nvram_match("boot_hw_model", "WRT310N") && nvram_match("boot_hw_ver", "2.0"))
		|| (nvram_match("boot_hw_model", "M20") && nvram_match("boot_hw_ver", "1.0")) ) )
{
		printk(KERN_EMERG "WRT310Nv2/M20 GPIO Init\n");
		gpios = 1 << 1 | 1 << 2 | 1 << 4;
}

if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x11")
		&& nvram_match("boardtype", "0x048e")
		&& (nvram_match("melco_id", "32093") || nvram_match("melco_id", "32064")))
{
		printk(KERN_EMERG "WHR-G125 / WHR-HP-G125 GPIO Init\n");
		gpios = 1 << 1 | 1 << 6 | 1 << 7;
}

if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x13")
	    && nvram_match("boardtype", "0x467"))
{
		printk(KERN_EMERG "WHR-G54S / WHR-HP-G54 GPIO Init\n");
		gpios = 1 << 1 | 1 << 6 | 1 << 7;
}

if (nvram_match("boardtype", "0x04cf") && (nvram_match("boot_hw_model", "WRT610N")
	|| nvram_match("boot_hw_model", "E300")))
{
		printk(KERN_EMERG "WRT610Nv2/E3000 GPIO Init\n");
		gpios = 1 << 0 | 1 << 3 | 1 << 5 | 1 << 7;
}

if (boardnum == 42 && nvram_match("boardrev", "0x10")
	    && (nvram_match("boardtype", "0x0467")
	    	|| nvram_match("boardtype", "0x0708")
	    	|| nvram_match("boardtype", "0x0101")))
{
		printk(KERN_EMERG "WRT54G/GS/GL/TM GPIO Init\n");
		gpios = 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 7;
}

if (boardnum == 45 && nvram_match("boardrev", "0x1402")
		&& nvram_match("boardtype", "0x04EC"))
{
		printk(KERN_EMERG "RT-N10 GPIO Init\n");
		gpios = 1 << 1;
}

if (boardnum == 45 && nvram_match("boardrev", "0x1102")
		&& nvram_match("boardtype", "0x0550"))
{
		printk(KERN_EMERG "RT-N10U GPIO Init\n");
		gpios = 1 << 5;
}

if (boardnum == 45 && nvram_match("boardrev", "0x1153")
		&& nvram_match("boardtype", "0x058e"))
{
		printk(KERN_EMERG "RT-N10+ D1 GPIO INIT\n");
		gpios = 1 << 5;
}

if (boardnum == 1 && nvram_match("boardtype", "0xE4CD")
		&& nvram_match("boardrev", "0x1700"))
{
		printk(KERN_EMERG "WNR2000v2 GPIO Init\n");
		gpios = 1 << 2 | 1 << 6 | 1 << 7 | 1 << 8;
}

if (boardnum == 45 && nvram_match("boardrev", "0x1201")
	    && nvram_match("boardtype", "0x04CD"))
{
		printk(KERN_EMERG "RT-N12 GPIO Init\n");
		gpios = 1 << 0 | 1 << 2;
}

if (boardnum == 45 && nvram_match("boardrev", "0x1218")
		&& nvram_match("boardtype", "0x04cf"))
{
		printk(KERN_EMERG "RT-N16 GPIO Init\n");
		gpios = 1 << 1;
}

if (boardnum == 1 && nvram_match("boardrev", "0x23")
		&& nvram_match("boardtype", "0x0472"))
{
		if (nvram_match("cardbus", "1")) {
		printk(KERN_EMERG "WNR324v2 GPIO Init\n");
		gpios = 1 << 2 | 1 << 3 | 1 << 7;
		} else {
		printk(KERN_EMERG "WNDR3300 GPIO Init\n");
		gpios = 1 << 5 | 1 << 7;
		}
}

if (nvram_match("boardnum", "00") && nvram_match("boardtype", "0x0101")
		&& nvram_match("boardrev", "0x10"))
{
		printk(KERN_EMERG "WBR2-G54(S) GPIO Init\n");
		gpios = 1 << 1 | 1 << 6;
}

if (nvram_match("boardtype", "0xd4cf")
		&& nvram_match("boardrev", "0x1204"))
{
		printk(KERN_EMERG "F7D4301v1 GPIO Init\n");
		gpios = 1 << 10 | 1 << 11 | 1 << 13;
}

if (nvram_match("boardtype", "0xa4cf") 
		&& (nvram_match("boardrev", "0x1100") || nvram_match("boardrev", "0x1102")))
{
		printk(KERN_EMERG "F7D3301v1/3302v1/4302v1  - F5D8235v3 GPIO Init\n");
		gpios = 1 << 10 | 1 << 11 | 1 << 13;
}

if (nvram_match("boot_hw_model", "E1000")
		&& (nvram_match("boot_hw_ver", "2.0") || nvram_match("boot_hw_ver", "2.1")))
{
		printk(KERN_EMERG "E1000v2/v21 GPIO Init\n");
		gpios = 1 << 6 | 1 << 7 | 1 << 8;
}

if (nvram_match("boot_hw_model", "E4200")
		&& nvram_match("boot_hw_ver", "1.0"))
{
		printk(KERN_EMERG "E4200 GPIO Init\n");
		gpios = 1 << 3 | 1 << 5;
}

if (nvram_match("boardnum", "01") && nvram_match("boardtype", "0xb4cf")
	    && nvram_match("boardrev", "0x1100"))
{
		printk(KERN_EMERG "WNDR3400 GPIO Init\n");
		gpios = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 7;
}

if (nvram_match("boardnum", "01") && nvram_match("boardtype", "0xF52C")
	    && nvram_match("boardrev", "0x1101"))
{
		printk(KERN_EMERG "WNDR4000 GPIO Init\n");
		gpios = 1 << 0 | 1 << 6 | 1 << 7;
}
if (nvram_match("boardnum", "4536") && nvram_match("boardtype", "0xf52e")
	    && nvram_match("boardrev", "0x1102")) {
		
	if( nvram_match("pci/1/1/pa2gw1a0", "0x1DFC") ){
	        printk(KERN_EMERG "WNDR4500 GPIO Init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<14 | 1<<15;
	}else if ( nvram_match("pci/1/1/pa2gw1a0", "0x1791") ){
		printk(KERN_EMERG "WNDR4500V2 GPIO Init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<14 | 1<<15;
	}else{
		printk(KERN_EMERG "R6300 GPIO Init\n");
                gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<15;
	}
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
	
	for (i = 0; i < 16; i++)
	{
		if (gpios&1<<i) {
			si_gpioreserve(gpio_sih, 1 << i, GPIO_APP_PRIORITY);
		}
	}

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
