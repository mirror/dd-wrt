/*
 * proc_gpio: AR5315 GPIO pins in /proc/gpio/
 * by olg 
 * modification for AR7240 support by Sebastian Gottschall <s.gottschall@newmedia-net.de>
 * GPL'ed
 * some code stolen from Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for copy_from_user */

#include "ar7100.h"

extern void set_wl0_gpio(int gpio, int val);

extern void set_wl1_gpio(int gpio, int val);

extern int get_wl0_gpio(int gpio);

extern int get_wl1_gpio(int gpio);

#define PROCFS_MAX_SIZE 64
extern const char *get_arch_type(void);
static struct proc_dir_entry *proc_gpio, *gpio_dir;

//Masks for data exchange through "void *data" pointer

static void cleanup_proc(void);

//The buffer used to store the data returned by the proc file
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static ssize_t gpio_proc_read(struct file *file, char __user * buffer, size_t size, loff_t * ppos)
{
	u32 reg = 0;
	unsigned int *data = PDE_DATA(file_inode(file));
	char buf[2];

	if (((unsigned int)data) >= 16)
		reg = get_wl1_gpio((unsigned int)data - 16);
	else
		reg = get_wl0_gpio((unsigned int)data);

	if (reg)
		buf[0] = '1';
	else
		buf[0] = '0';
	buf[1] = 0;

	return simple_read_from_buffer(buffer, size, ppos, buf, sizeof(buf));
}

static ssize_t gpio_proc_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	u32 reg = 0;
	unsigned int *data = PDE_DATA(file_inode(file));

	/* get buffer size */
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	/* write data to the buffer */
	if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
		return -EFAULT;
	}

	procfs_buffer[procfs_buffer_size] = 0;

	if (procfs_buffer[0] == '0' || procfs_buffer[0] == 'i')
		reg = 0;
	if (procfs_buffer[0] == '1' || procfs_buffer[0] == 'o')
		reg = 1;
	if (((unsigned int)data) >= 16)
		set_wl1_gpio(((unsigned int)data) - 16, reg);
	else
		set_wl0_gpio(((unsigned int)data), reg);
	return procfs_buffer_size;
}

static const struct file_operations fops_data = {
	.read = gpio_proc_read,
	.write = gpio_proc_write,
	.llseek = default_llseek,
};

static __init int register_proc(void)
{
	unsigned char i, flag = 0;
	char proc_name[64];
	int gpiocount = 32;

	/* create directory gpio */
	gpio_dir = proc_mkdir("wl0gpio", NULL);
	if (gpio_dir == NULL)
		goto fault;

	for (i = 0; i < gpiocount; i++)	//create for every GPIO "x_in"," x_out" and "x_dir"
	{
		sprintf(proc_name, "%i_out", i);
		proc_gpio = proc_create_data(proc_name, S_IRUGO, gpio_dir, &fops_data, i);
	}

	printk(KERN_NOTICE "wl0gpio_proc: module loaded and /proc/wl0gpio/ created\n");
	return 0;

fault:
	cleanup_proc();
	return -EFAULT;
}

static void cleanup_proc(void)
{
	unsigned char i;
	char proc_name[64];
	int gpiocount = 32;

	for (i = 0; i < gpiocount; i++) {
		sprintf(proc_name, "%i_out", i);
		remove_proc_entry(proc_name, gpio_dir);
	}
	remove_proc_entry("wl0gpio", NULL);
	printk(KERN_INFO "wl0gpio_proc: unloaded and /proc/wl0gpio/ removed\n");

}

module_init(register_proc);
module_exit(cleanup_proc);

MODULE_AUTHOR("Sebastian Gottschall");
MODULE_DESCRIPTION("AR7240 GPIO pins in /proc/wl0gpio/");
MODULE_LICENSE("GPL");
