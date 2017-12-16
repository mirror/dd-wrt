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
	{
	"in"}, {
	"out"}, {
	"outen"}, {
	"control"}, {
	"hc595"}
};

static int gpio_init_flag = 0;
static int __init gpio_init(void);
void set_hc595(uint32 pin, uint32 value);
void set_hc595_reset(void);
void set_hc595_core(si_t *sih);


static int gpio_open(struct inode *inode, struct file *file)
{
	if (MINOR(inode->i_rdev) > ARRAYSIZE(gpio_file))
		return -ENODEV;

	MOD_INC_USE_COUNT;
	return 0;
}

static int gpio_release(struct inode *inode, struct file *file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t gpio_read(struct file *file, char *buf, size_t count, loff_t * ppos)
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

	if (put_user(val, (u32 *)buf))
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

EXPORT_SYMBOL(gpio_kernel_api);

static ssize_t gpio_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{
	u32 val;

	if (get_user(val, (u32 *)buf))
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
	case 4:
		set_hc595(val >> 4, val & 0xf);
		break;
	default:
		return -ENODEV;
	}

	return sizeof(val);
}

static struct file_operations gpio_fops = {
      owner:THIS_MODULE,
      open:gpio_open,
      release:gpio_release,
      read:gpio_read,
      write:gpio_write,
};

extern int iswrt350n;
extern int iswrt300n11;
int isac66 = 0;
int isac68 = 0;
int isbuffalo = 0;
int isbuffalowxr = 0;
int isdefault = 0;

static int __init gpio_init(void)
{
	int i;
	int gpios = 0;
	printk(KERN_INFO "init gpio code\n");
	if (!(gpio_sih = si_kattach(SI_OSH)))
		return -ENODEV;

	si_gpiosetcore(gpio_sih);
	set_hc595_core(gpio_sih);
	if ((gpio_major = register_chrdev(127, "gpio", &gpio_fops)) < 0) {
		return gpio_major;
	}

	uint boardnum = bcm_strtoul(nvram_safe_get("boardnum"), NULL, 0);

	if (boardnum == 00 && nvram_match("boardtype", "0xF646")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("melco_id", "RD_BB12068")) {
		printk(KERN_EMERG "Buffalo WZR-1750DHP\n");
		isbuffalo = 1;
		set_hc595_reset();
		gpio_init_flag = 1;
		return 0;

	}

	if (boardnum == 00 && nvram_match("boardtype", "0x0665")
	    && nvram_match("boardrev", "0x1103")
	    && nvram_match("melco_id", "RD_BB13049")) {
		printk(KERN_EMERG "Buffalo WXR-1900DHP\n");
		isbuffalowxr = 1;
		gpio_init_flag = 1;
		return 0;

	}

	if ((!strncmp(nvram_safe_get("boardnum"),"2013",4) || !strncmp(nvram_safe_get("boardnum"),"2014",4)) && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("0:rxchain", "7")) {
		printk(KERN_EMERG "Buffalo WZR-900DHP\n");
		isbuffalo = 1;
		set_hc595_reset();
		gpio_init_flag = 1;
		return 0;
	}

	if ((!strncmp(nvram_safe_get("boardnum"),"2013",4) || !strncmp(nvram_safe_get("boardnum"),"2014",4)) && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("0:rxchain", "3")) {
		printk(KERN_EMERG "Buffalo WZR-600DHP2\n");
		isbuffalo = 1;
		set_hc595_reset();
		gpio_init_flag = 1;
		return 0;
	}

	if ((boardnum == 0) && nvram_match("boardtype", "0x0646") && (nvram_match("boardrev", "0x1100"))) {
		printk(KERN_EMERG "Asus-RT-AC56U init\n");
		isac66 = 1;
	}

	if (nvram_match("model","RT-AC68U")) {
		printk(KERN_EMERG "Asus-RT-AC68U init\n");
		isac68 = 1;
	} else if ((boardnum != 24) && nvram_match("boardtype", "0x0646") && (nvram_match("boardrev", "0x1100"))) {
		printk(KERN_EMERG "Asus-RT-AC68U init\n");
		isac68 = 1;
	}

	if ((boardnum == 24) && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110") && !nvram_match("gpio6", "wps_led")) {
		printk(KERN_EMERG "DLink DIR-868 init\n");
		isac66 = 1;
	}

	if ((boardnum == 679) && nvram_match("boardtype", "0x0646") && (nvram_match("boardrev", "0x1110"))) {
		printk(KERN_EMERG "Netgear AC1450/R6250/R6300v2/EX6200 init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<15;
	}
	
	if ((boardnum == 32) && nvram_match("boardtype", "0x0646") && (nvram_match("boardrev", "0x1601")&& !nvram_match("et2phyaddr", "30"))) {
		printk(KERN_EMERG "Netgear R6400 init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<15;
	}

	if ((boardnum == 32) && nvram_match("boardtype", "0x0665") && (nvram_match("boardrev", "0x1301"))) {
		printk(KERN_EMERG "Netgear R7000 init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<15;
	}
	
	if ((boardnum == 32) && nvram_match("boardtype", "0x0665") && (nvram_match("boardrev", "0x1101"))) {
		printk(KERN_EMERG "Netgear R8000 init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<15;
	}
	
	if ((boardnum == 32) && nvram_match("boardtype", "0x072F") && (nvram_match("boardrev", "0x1101"))) {
		printk(KERN_EMERG "Netgear R8500 init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<14 | 1<<15 | 1<<16 | 1<<17 | 1<<18 | 1<<19 | 1<<20 | 1<<21 | 1<<22 | 1<<23  | 1<<24 ;
	}
	
	if ((boardnum == 32) && nvram_match("boardtype", "0x0646") && (nvram_match("boardrev", "0x1601") && nvram_match("et2phyaddr", "30"))) {
		printk(KERN_EMERG "Netgear R7000P init\n");
		gpios = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<14 | 1<<15 | 1<<16 | 1<<17 | 1<<18 | 1<<19 | 1<<20 | 1<<21 | 1<<22 | 1<<23  | 1<<24 ;
	}
	
	if (nvram_match("model","RT-AC1200G+")) {
		printk(KERN_EMERG " Init Asus RT-AC1200G+\n");
		isdefault = 1;
	}	
	

	for (i = 0; i < 32; i++) {
		if (gpios & (1<<i)) {
			si_gpioreserve(gpio_sih, 1 << i, GPIO_APP_PRIORITY);
		}
	}

	gpio_init_flag = 1;
	return 0;
}

static void __exit gpio_exit(void)
{
	int i;

	si_detach(gpio_sih);
}

module_init(gpio_init);
module_exit(gpio_exit);
