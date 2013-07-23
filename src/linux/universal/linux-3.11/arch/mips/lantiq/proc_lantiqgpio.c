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

#define LQ_GPIO0_BASE_ADDR	0x1E100B10
#define LQ_GPIO1_BASE_ADDR	0x1E100B40
#define LQ_GPIO2_BASE_ADDR	0x1E100B70
#define LQ_GPIO3_BASE_ADDR	0x1E100Ba0
#define LQ_GPIO_SIZE		0x30
#define PINS_PER_PORT		16

#define LQ_GPIO_OUT			0x00
#define LQ_GPIO_IN			0x04
#define LQ_GPIO_DIR			0x08
#define LQ_GPIO_ALTSEL0		0x0C
#define LQ_GPIO_ALTSEL1		0x10
#define LQ_GPIO_OD			0x14

#define lq_gpio_getbit(m, r, p)		!!(ltq_r32(m + r) & (1 << p))
#define lq_gpio_setbit(m, r, p)		ltq_w32_mask(0, (1 << p), m + r)
#define lq_gpio_clearbit(m, r, p)	ltq_w32_mask((1 << p), 0, m + r)

#ifdef CONFIG_AR9
#define MAXGPIO 56
#else
#define MAXGPIO 32
#endif

#define GPIO_IN (1<<6)
#define GPIO_OUT (1<<7)
#define GPIO_DIR (1<<8)
#define PIN_MASK 0x3f

extern void ltq_stp_set(struct gpio_chip *chip, unsigned offset, int value);

static void set_gpio_out(int pin, int val)
{
	void __iomem *membase = (void *)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);

	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}

	if (val)
		lq_gpio_setbit(membase, LQ_GPIO_OUT, pin);
	else
		lq_gpio_clearbit(membase, LQ_GPIO_OUT, pin);
}

static void set_gpio_in(int pin, int val)
{
	void __iomem *membase = (void *)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}

	if (val)
		lq_gpio_setbit(membase, LQ_GPIO_IN, pin);
	else
		lq_gpio_clearbit(membase, LQ_GPIO_IN, pin);
}

static int get_gpio_in(int pin)
{
	void __iomem *membase = (void *)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	return lq_gpio_getbit(membase, LQ_GPIO_IN, pin);
}

static int get_gpio_out(int pin)
{
	void __iomem *membase = (void *)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	return lq_gpio_getbit(membase, LQ_GPIO_OUT, pin);
}

static void set_dir(int pin, int dir)
{
	void __iomem *membase = (void *)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (dir) {
		lq_gpio_setbit(membase, LQ_GPIO_OD, pin);
		lq_gpio_setbit(membase, LQ_GPIO_DIR, pin);
	} else {
		lq_gpio_clearbit(membase, LQ_GPIO_OD, pin);
		lq_gpio_clearbit(membase, LQ_GPIO_DIR, pin);
	}

}


void set_gpio(int pin, int val)
{
	if (pin >= 200) {
		ltq_stp_set(NULL, pin - 200, val);
		return;
	}
	set_dir(pin, 1);
	set_gpio_out(pin, val);
}

EXPORT_SYMBOL(set_gpio);

int usb_led_pin = -1;

void ap_usb_led_on(void)
{
	if (usb_led_pin >= 0)
		set_gpio(usb_led_pin&0xff, usb_led_pin & 0xf00 ? 0 : 1);
}

EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb_led_off(void)
{
	if (usb_led_pin >= 0)
		set_gpio(usb_led_pin&0xff, usb_led_pin & 0xf00 ? 1 : 0);
}

EXPORT_SYMBOL(ap_usb_led_off);

static int get_dir(int pin)
{
	void __iomem *membase = (void *)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	int val = 0;
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if (pin >= PINS_PER_PORT) {
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;

	}

	val |= lq_gpio_getbit(membase, LQ_GPIO_IN, pin);
	val |= lq_gpio_getbit(membase, LQ_GPIO_OD, pin);
	return val;

}

#define PROCFS_MAX_SIZE 64
extern const char *get_arch_type(void);
static struct proc_dir_entry *proc_gpio, *gpio_dir;

//Masks for data exchange through "void *data" pointer

static void cleanup_proc(void);

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

static __init int register_proc(void)
{
	unsigned char i;
	unsigned int flag = 0;
	char proc_name[64];
	int gpiocount = MAXGPIO;

	/* create directory gpio */
	gpio_dir = proc_mkdir("gpio", NULL);
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
	cleanup_proc();
	return -EFAULT;
}

static void cleanup_proc(void)
{
	unsigned char i;
	char proc_name[64];
	int gpiocount = MAXGPIO;

	for (i = 0; i < gpiocount; i++) {
		sprintf(proc_name, "%i_out", i);
		remove_proc_entry(proc_name, gpio_dir);
	}
	remove_proc_entry("gpio", NULL);
	printk(KERN_INFO "gpio_proc: unloaded and /proc/gpio/ removed\n");

}

module_init(register_proc);
module_exit(cleanup_proc);

MODULE_AUTHOR("Sebastian Gottschall");
MODULE_DESCRIPTION("Danube GPIO pins in /proc/gpio/");
MODULE_LICENSE("GPL");
