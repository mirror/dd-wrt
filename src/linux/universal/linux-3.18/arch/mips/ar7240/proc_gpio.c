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

#include "ar7240.h"

#define PROCFS_MAX_SIZE 64
extern const char *get_arch_type(void);
struct proc_dir_entry *proc_gpio, *gpio_dir;

//Masks for data exchange through "void *data" pointer
#define GPIO_IN (1<<5)
#define GPIO_OUT (1<<6)
#define GPIO_DIR (1<<7)
#define PIN_MASK 0x1f
#define GPIO_CR_M(x)                (1 << (x))	/* mask for i/o */

static void cleanup_proc(void);

//The buffer used to store the data returned by the proc file
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static ssize_t gpio_proc_read(struct file *file, char __user * buffer, size_t size, loff_t * ppos)
{
	unsigned int *data = PDE_DATA(file_inode(file));

	u32 reg = 0;
	char buf[2];
	if ((unsigned int)data & GPIO_IN)
		reg = ar7240_reg_rd(AR7240_GPIO_IN);
	if ((unsigned int)data & GPIO_OUT)
		reg = ar7240_reg_rd(AR7240_GPIO_OUT);
	if ((unsigned int)data & GPIO_DIR) {
		reg = ar7240_reg_rd(AR7240_GPIO_OE);
#ifdef CONFIG_WASP_SUPPORT
		if (GPIO_CR_M(((unsigned int)data) & PIN_MASK) & reg)
			buf[0] = '0';
		else
			buf[0] = '1';
	} else {
		if (GPIO_CR_M(((unsigned int)data) & PIN_MASK) & reg)
			buf[0] = '1';
		else
			buf[0] = '0';

	}
#else
	}
	if (GPIO_CR_M(((unsigned int)data) & PIN_MASK) & reg)
		buf[0] = '1';
	else
		buf[0] = '0';
#endif
	buf[1] = 0;
	return simple_read_from_buffer(buffer, size, ppos, buf, sizeof(buf));
}

static ssize_t gpio_proc_info_read(struct file *file, char __user * buffer, size_t size, loff_t * ppos)
{
	char buf[512];
	sprintf(buf,	"GPIO_IN   %#08X \n"
		        "GPIO_OUT  %#08X \n"
			"GPIO_SET  %#08X \n" 
			"GPIO_CLEAR  %#08X \n" 
			"GPIO_FUNC  %#08X \n" 
			"GPIO_INT_ENABLE  %#08X \n"
			"GPIO_DIR  %#08X \n", 
			ar7240_reg_rd(AR7240_GPIO_IN), 
			ar7240_reg_rd(AR7240_GPIO_OUT), 
			ar7240_reg_rd(AR7240_GPIO_SET), 
			ar7240_reg_rd(AR7240_GPIO_CLEAR), 
			ar7240_reg_rd(AR7240_GPIO_FUNCTIONS), 
			ar7240_reg_rd(AR7240_GPIO_INT_ENABLE), 
			ar7240_reg_rd(AR7240_GPIO_OE));
	return simple_read_from_buffer(buffer, size, ppos, buf, strlen(buf));

}


static ssize_t gpio_proc_write(struct file *file, const char __user * buffer, size_t count, loff_t * ppos)
{
	u32 reg = 0;
	unsigned int *data = PDE_DATA(file_inode(file));

	if (!count)
	    return 0;

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

	if ((unsigned int)data & GPIO_IN)
		reg = ar7240_reg_rd(AR7240_GPIO_IN);
	if ((unsigned int)data & GPIO_OUT)
		reg = ar7240_reg_rd(AR7240_GPIO_OUT);
	if ((unsigned int)data & GPIO_DIR) {
		reg = ar7240_reg_rd(AR7240_GPIO_OE);
#ifdef CONFIG_WASP_SUPPORT
		if (procfs_buffer[0] == '0' || procfs_buffer[0] == 'i')
			reg = reg | GPIO_CR_M(((unsigned int)data) & PIN_MASK);
		if (procfs_buffer[0] == '1' || procfs_buffer[0] == 'o')
			reg = reg & ~(GPIO_CR_M(((unsigned int)data) & PIN_MASK));
	} else {
		if (procfs_buffer[0] == '0' || procfs_buffer[0] == 'i')
			reg = reg & ~(GPIO_CR_M(((unsigned int)data) & PIN_MASK));
		if (procfs_buffer[0] == '1' || procfs_buffer[0] == 'o')
			reg = reg | GPIO_CR_M(((unsigned int)data) & PIN_MASK);
	}
#else
	}
	if (procfs_buffer[0] == '0' || procfs_buffer[0] == 'i')
		reg = reg & ~(GPIO_CR_M(((unsigned int)data) & PIN_MASK));
	if (procfs_buffer[0] == '1' || procfs_buffer[0] == 'o')
		reg = reg | GPIO_CR_M(((unsigned int)data) & PIN_MASK);
#endif

	if ((unsigned int)data & GPIO_IN) {
		ar7240_reg_wr(AR7240_GPIO_IN, reg);
	}
	if ((unsigned int)data & GPIO_OUT) {
		ar7240_reg_wr(AR7240_GPIO_OUT, reg);
	}
	if ((unsigned int)data & GPIO_DIR) {
		ar7240_reg_wr(AR7240_GPIO_OE, reg);
	}

	return procfs_buffer_size;
}

void ar7100_set_gpio(int gpio, int val)
{
	u32 reg = ar7240_reg_rd(AR7240_GPIO_OE);
	reg |= 1 << gpio;
	ar7240_reg_wr(AR7240_GPIO_OE, reg);
	(void)ar7240_reg_rd(AR7240_GPIO_OE);	/* flush write to hardware */
	reg = ar7240_reg_rd(AR7240_GPIO_OUT);
	if (val)
		reg |= 1 << gpio;
	else
		reg &= ~(1 << gpio);
	ar7240_reg_wr(AR7240_GPIO_OUT, reg);
}

int ar7100_get_gpio(int gpio)
{
	u32 reg = ar7240_reg_rd(AR7240_GPIO_OE);
	reg &= ~(1 << gpio);
	ar7240_reg_wr(AR7240_GPIO_OE, reg);
	reg = ar7240_reg_rd(AR7240_GPIO_IN);
	if (reg & (1 << gpio))
		return 1;
	else
		return 0;
}

EXPORT_SYMBOL(ar7100_set_gpio);
EXPORT_SYMBOL(ar7100_get_gpio);

typedef u32 gpio_words;

#define	GPIO_WL0_ADDR		KSEG1ADDR(AR7240_PCI_MEM_BASE + 0x4048)	//AR9220 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0000000 + OFFSET [0x4048]
#define	GPIOOUT_WL0_ADDR		KSEG1ADDR(AR7240_PCI_MEM_BASE + 0x404c)	//AR9223 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0010000 + OFFSET [0x4048]

#define	GPIOOUT_WMAC_ADDR		KSEG1ADDR(0x18100000 + 0x4030)	//AR9223 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0010000 + OFFSET [0x4048]
#define	GPIO_WMAC_ADDR		KSEG1ADDR(0x18100000 + 0x4028)	//AR9220 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0000000 + OFFSET [0x4048]
#define	GPIOIN_WMAC_ADDR		KSEG1ADDR(0x18100000 + 0x402c)	//AR9220 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0000000 + OFFSET [0x4048]

#define AR_GPIO_OE_OUT                           0x404c	// GPIO output register
#define AR_GPIO_OE_OUT_DRV                       0x3	// 2 bit field mask, shifted by 2*bitpos
#define AR_GPIO_OE_OUT_DRV_NO                    0x0	// tristate
#define AR_GPIO_OE_OUT_DRV_LOW                   0x1	// drive if low
#define AR_GPIO_OE_OUT_DRV_HI                    0x2	// drive if high
#define AR_GPIO_OE_OUT_DRV_ALL                   0x3	// drive always

int is_ar9300 = 0;

#ifdef CONFIG_WASP_SUPPORT
void set_wmac_gpio(int gpio, int val)
{
	register gpio_words wl0;
	int shift;
	int addr;

	unsigned int output = GPIOOUT_WMAC_ADDR;

	wl0 = (gpio_words) ar7240_reg_rd(output);
	shift = gpio * 2;
	wl0 |= AR_GPIO_OE_OUT_DRV << shift;
	ar7240_reg_wr(output, wl0);	//ar9283 register [0x4048]
	ar7240_reg_rd(output);	//ar9283 register [0x4048]

	wl0 = (gpio_words) ar7240_reg_rd(GPIO_WMAC_ADDR);	//ar9280 register [0x4048]
	if (val)
		wl0 |= 1 << gpio;
	else
		wl0 &= (~(1 << gpio));

	ar7240_reg_wr(GPIO_WMAC_ADDR, wl0);	//ar9283 register [0x4048]
	ar7240_reg_rd(GPIO_WMAC_ADDR);	//ar9283 register [0x4048]

}
#endif
#ifndef CONFIG_MACH_HORNET
extern int ath_nopcie;
#endif
void set_wl0_gpio(int gpio, int val)
{
	register gpio_words wl0;
	int shift;
	int addr;
#ifndef CONFIG_MACH_HORNET
	if (ath_nopcie)
		return;
#endif
	unsigned int output = GPIOOUT_WL0_ADDR;
	if (is_ar9300)
		output = KSEG1ADDR(AR7240_PCI_MEM_BASE + 0x4050);

	wl0 = (gpio_words) ar7240_reg_rd(output);
	shift = gpio * 2;
	wl0 |= AR_GPIO_OE_OUT_DRV << shift;
	ar7240_reg_wr(output, wl0);	//ar9283 register [0x4048]
	ar7240_reg_rd(output);	//ar9283 register [0x4048]

	wl0 = (gpio_words) ar7240_reg_rd(GPIO_WL0_ADDR);	//ar9280 register [0x4048]
	if (val)
		wl0 |= 1 << gpio;
	else
		wl0 &= (~(1 << gpio));

	ar7240_reg_wr(GPIO_WL0_ADDR, wl0);	//ar9283 register [0x4048]
	ar7240_reg_rd(GPIO_WL0_ADDR);	//ar9283 register [0x4048]
}

#define MS(_v, _f)  (((_v) & _f) >> _f##_S)

#define AR9287_GPIO_IN_VAL                           0x001FF800
#define AR9287_GPIO_IN_VAL_S                         11
#define AR9287_GPIO_OE_OUT                           0x404c	// GPIO output enable register
#define AR9287_GPIO_OE_OUT_DRV                       0x3	// 2 bit field mask, shifted by 2*bitpos
#define AR9287_GPIO_OE_OUT_DRV_NO                    0x0	// tristate
#define AR9287_GPIO_OE_OUT_DRV_LOW                   0x1	// drive if low
#define AR9287_GPIO_OE_OUT_DRV_HI                    0x2	// drive if high
#define AR9287_GPIO_OE_OUT_DRV_ALL                   0x3	// drive always
#define ar7240_reg_rmw(_off, _set, _clr) do { \
            ar7240_reg_rmw_clear(((_off)), (_clr)); \
            ar7240_reg_rmw_set(((_off)), (_set)); \
	} while(0)

#define AR9300_GPIO_IN_VAL                       0x0001FFFF
#define AR9300_GPIO_IN_VAL_S                     0

int get_wl0_gpio(int gpio)
{
	u32 gpio_shift;
	gpio_shift = 2 * gpio;

	unsigned int output = GPIOOUT_WL0_ADDR;
	if (is_ar9300)
		output = KSEG1ADDR(AR7240_PCI_MEM_BASE + 0x4050);

	ar7240_reg_rmw(output, (AR9287_GPIO_OE_OUT_DRV_NO << gpio_shift), (AR9287_GPIO_OE_OUT_DRV << gpio_shift));

	if (is_ar9300)
		return (MS(ar7240_reg_rd(GPIOOUT_WL0_ADDR), AR9300_GPIO_IN_VAL) & (1 << gpio)) != 0;
	else
		return (MS(ar7240_reg_rd(GPIO_WL0_ADDR), AR9287_GPIO_IN_VAL) & (1 << gpio)) != 0;

}

#ifdef CONFIG_WASP_SUPPORT
int get_wmac_gpio(int gpio)
{
	u32 gpio_shift;
	gpio_shift = 2 * gpio;
	ar7240_reg_rmw(GPIOOUT_WMAC_ADDR, (AR9287_GPIO_OE_OUT_DRV_NO << gpio_shift), (AR9287_GPIO_OE_OUT_DRV << gpio_shift));

	return (MS(ar7240_reg_rd(GPIOIN_WMAC_ADDR), AR9300_GPIO_IN_VAL) & (1 << gpio)) != 0;
}
#endif

#define USB_LED_OFF 1
#define USB_LED_ON 0

void ap_usb_led_on(void)
{
#ifdef CONFIG_WZRG300NH2
	printk(KERN_INFO "switch USB LED On\n");
	set_wl0_gpio(4, 0);
#else
#ifdef AP_USB_LED_GPIO
	printk(KERN_INFO "switch USB LED On\n");
	ar7100_set_gpio(AP_USB_LED_GPIO, USB_LED_ON);
#endif
#endif
}

EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb_led_off(void)
{
#ifdef CONFIG_WZRG300NH2
	printk(KERN_INFO "switch USB LED Off\n");
	set_wl0_gpio(4, 1);
#else
#ifdef AP_USB_LED_GPIO
	printk(KERN_INFO "switch USB LED Off\n");
	ar7100_set_gpio(AP_USB_LED_GPIO, USB_LED_OFF);
#endif
#endif
}

EXPORT_SYMBOL(ap_usb_led_off);

void __init ar71xx_gpio_init(void);

static const struct file_operations fops_data = {
	.read = gpio_proc_read,
	.write = gpio_proc_write,
	.llseek = default_llseek,
};

static const struct file_operations fops_info = {
	.read = gpio_proc_info_read,
	.llseek = default_llseek,
};

static __init int register_proc(void)
{
	unsigned char i, flag = 0;

	char proc_name[64];
	int gpiocount = 32;

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

		proc_gpio = proc_create_data(proc_name, S_IRUGO, gpio_dir, &fops_data, ((i % gpiocount) | flag));

	}

	proc_gpio = proc_create("info", S_IRUGO, gpio_dir, &fops_info);

	printk(KERN_NOTICE "gpio_proc: module loaded and /proc/gpio/ created (cool)\n");
	ar71xx_gpio_init();
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
		sprintf(proc_name, "%i_in", i);
		remove_proc_entry(proc_name, gpio_dir);
		sprintf(proc_name, "%i_out", i);
		remove_proc_entry(proc_name, gpio_dir);
		sprintf(proc_name, "%i_dir", i);
		remove_proc_entry(proc_name, gpio_dir);
	}
	remove_proc_entry("info", gpio_dir);
	remove_proc_entry("gpio", NULL);
	printk(KERN_INFO "gpio_proc: unloaded and /proc/gpio/ removed\n");

}

module_init(register_proc);
module_exit(cleanup_proc);

MODULE_AUTHOR("Sebastian Gottschall");
MODULE_DESCRIPTION("AR7240 GPIO pins in /proc/gpio/");
MODULE_LICENSE("GPL");
