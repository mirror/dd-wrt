/*
 * proc_gpio: AR5315 GPIO pins in /proc/gpio/
 * by olg 
 * modification for Danube support by Sebastian Gottschall <s.gottschall@newmedia-net.de>
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
#include <linux/gpio.h>
#include <asm/uaccess.h>	/* for copy_from_user */
#include <lantiq.h>

#define MAXGPIO 16

#define GPIO_IN (1<<6)
#define GPIO_OUT (1<<7)
#define GPIO_DIR (1<<8)
#define PIN_MASK 0x3f

extern void ltq_ebu_set(struct gpio_chip *chip, unsigned offset, int value);

static void set_gpio_out(int pin, int val)
{
	ltq_ebu_set(NULL, pin, val);
}

static void set_gpio_in(int pin, int val)
{
	ltq_ebu_set(NULL, pin, val);
}

static int get_gpio_in(int pin)
{
	return 0;
}

static int get_gpio_out(int pin)
{
	return 0;
}

static void set_dir(int pin, int dir)
{

}

static int get_dir(int pin)
{
	return 0;

}

#define PROCFS_MAX_SIZE 64
extern const char *get_arch_type(void);
static struct proc_dir_entry *proc_gpio, *gpio_dir;

//Masks for data exchange through "void *data" pointer


//The buffer used to store the data returned by the proc file
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static int
gpio_proc_read(char *buf, char **start, off_t offset,
	       int len, int *eof, void *data)
{
	u32 reg = 0;
	if ((unsigned int)data & GPIO_IN) {
		reg = get_gpio_in(((unsigned int)data) & PIN_MASK);
	}
	if ((unsigned int)data & GPIO_OUT) {
		reg = get_gpio_out(((unsigned int)data) & PIN_MASK);
	}
	if ((unsigned int)data & GPIO_DIR) {
		reg = get_dir(((unsigned int)data) & PIN_MASK);
	}

	if (reg)
		buf[0] = '1';
	else
		buf[0] = '0';
	buf[1] = 0;

	*eof = 1;

	return (2);

}

static int
gpio_proc_write(struct file *file, const char *buffer, unsigned long count,
		void *data)
{
	u32 reg = 0;

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

	if ((unsigned int)data & GPIO_IN) {
		set_gpio_in(((unsigned int)data) & PIN_MASK, reg);
	}
	if ((unsigned int)data & GPIO_OUT) {
		set_gpio_out(((unsigned int)data) & PIN_MASK, reg);
	}
	if ((unsigned int)data & GPIO_DIR) {
		set_dir(((unsigned int)data) & PIN_MASK, reg);
	}

	return procfs_buffer_size;
}

__init int register_ebu_proc(void)
{
	unsigned char i;
	unsigned int flag = 0;
	char proc_name[64];
	int gpiocount = MAXGPIO;

	/* create directory gpio */
	gpio_dir = proc_mkdir("gpioebu", NULL);
	if (gpio_dir == NULL)
		goto fault;

	for (i = 0; i < gpiocount * 3; i++)	//create for every GPIO "x_in"," x_out" and "x_dir"
	{
		if (i / gpiocount == 0) {
			flag = GPIO_IN;
			sprintf(proc_name, "%i_in", i);
		}
		if (i / gpiocount == 1) {
			flag = GPIO_OUT;
			sprintf(proc_name, "%i_out", i % gpiocount);
		}
		if (i / gpiocount == 2) {
			flag = GPIO_DIR;
			sprintf(proc_name, "%i_dir", i % gpiocount);
		}

		proc_gpio = create_proc_entry(proc_name, S_IRUGO, gpio_dir);
		if (proc_gpio) {
			proc_gpio->read_proc = gpio_proc_read;
			proc_gpio->write_proc = gpio_proc_write;
			//proc_gpio->owner = THIS_MODULE;
			proc_gpio->data = (void *)((i % gpiocount) | flag);
		} else
			goto fault;

	}

	printk(KERN_NOTICE
	       "gpio_proc: module loaded and /proc/gpio/ created\n");
	return 0;

fault:
	return -EFAULT;
}
