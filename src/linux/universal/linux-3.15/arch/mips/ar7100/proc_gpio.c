/*
 * proc_gpio: AR5315 GPIO pins in /proc/gpio/
 * by olg 
 * modification for AR7100 support by Sebastian Gottschall <s.gottschall@newmedia-net.de>
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
void ar7100_set_gpio(int gpio, int val)
{
	u32 reg = ar7100_reg_rd(AR7100_GPIO_OE);
	reg |= 1 << gpio;
	ar7100_reg_wr(AR7100_GPIO_OE, reg);
	(void)ar7100_reg_rd(AR7100_GPIO_OE);	/* flush write to hardware */
	reg = ar7100_reg_rd(AR7100_GPIO_OUT);
	if (val)
		reg |= 1 << gpio;
	else
		reg &= ~(1 << gpio);
	ar7100_reg_wr(AR7100_GPIO_OUT, reg);
}

int ar7100_get_gpio(int gpio)
{
	u32 reg = ar7100_reg_rd(AR7100_GPIO_OE);
	reg &= ~(1 << gpio);
	ar7100_reg_wr(AR7100_GPIO_OE, reg);
	reg = ar7100_reg_rd(AR7100_GPIO_IN);
	if (reg & (1 << gpio))
		return 1;
	else
		return 0;
}

void ar7100_gpio_config_output(int gpio)
{
	ar7100_reg_rmw_set(AR7100_GPIO_OE, (1 << gpio));
	(void)ar7100_reg_rd(AR7100_GPIO_OE);	/* flush write to hardware */
}

void ar7100_gpio_config_input(int gpio)
{
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, (1 << gpio));
	(void)ar7100_reg_rd(AR7100_GPIO_OE);	/* flush write to hardware */
}

void ar7100_gpio_out_val(int gpio, int val)
{
	if (val & 0x1) {
		ar7100_reg_rmw_set(AR7100_GPIO_OUT, (1 << gpio));
	} else {
		ar7100_reg_rmw_clear(AR7100_GPIO_OUT, (1 << gpio));
	}
	(void)ar7100_reg_rd(AR7100_GPIO_OUT);	/* flush write to hardware */
}

int ar7100_gpio_in_val(int gpio)
{
	return ((1 << gpio) & (ar7100_reg_rd(AR7100_GPIO_IN)));
}

EXPORT_SYMBOL(ar7100_set_gpio);
EXPORT_SYMBOL(ar7100_get_gpio);
EXPORT_SYMBOL(ar7100_gpio_config_output);
EXPORT_SYMBOL(ar7100_gpio_config_input);
EXPORT_SYMBOL(ar7100_gpio_in_val);
EXPORT_SYMBOL(ar7100_gpio_out_val);

typedef u32 gpio_words;
#define	GPIO_IGN_MASK		(GPIO_PIN(0) | GPIO_PIN(9) | GPIO_PIN(10))	//IGNORE  GPIO[0]:SI_CS1, GPIO[9-10]:UART

#define	GPIO_PIN_MAX		12
#define	GPIO_PIN(n)			(1 << (n))
#define	GPIO_PIN_MASK(_VAL)	(((gpio_words)(_VAL)) & (((((gpio_words)1)<<GPIO_PIN_MAX)-1) & (~(gpio_words)GPIO_IGN_MASK)) )

#define	GPIO_WL0_MAX		10
#define	GPIO_WL0(n)			(1 << ((n)+GPIO_PIN_MAX))
#define	GPIO_WL0_ADDR		KSEG1ADDR(AR71XX_PCI_MEM_BASE + 0x4048)	//AR9220 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0000000 + OFFSET [0x4048]
#define	GPIOOUT_WL0_ADDR		KSEG1ADDR(AR71XX_PCI_MEM_BASE + 0x404c)	//AR9220 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0000000 + OFFSET [0x4048]
#define	GPIO_WL0_MASK(_VAL)	(((gpio_words)(_VAL)) & (((gpio_words)1<<GPIO_WL0_MAX)-1))
#define	GPIO_WL0_TO(_VAL)	GPIO_WL0_MASK(((gpio_words)(_VAL))>>(GPIO_PIN_MAX))	//the value to AR9220 register
#define	GPIO_WL0_FROM(_VAL)	(GPIO_WL0_MASK(_VAL)<<(GPIO_PIN_MAX))	//the value from AR9220 register

#define	GPIO_WL1_MAX		10
#define	GPIO_WL1(n)			(1 << ((n)+GPIO_PIN_MAX+GPIO_WL0_MAX))
#define	GPIO_WL1_ADDR		KSEG1ADDR(AR71XX_PCI_MEM_BASE + 0x00010000 + 0x4048)	//AR9223 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0010000 + OFFSET [0x4048]
#define	GPIOOUT_WL1_ADDR		KSEG1ADDR(AR71XX_PCI_MEM_BASE + 0x00010000 + 0x404c)	//AR9223 GPIO IN/OUT REGISTER   --> PCI MAP 0xB0010000 + OFFSET [0x4048]
#define	GPIO_WL1_MASK(_VAL)	(((gpio_words)(_VAL)) & (((gpio_words)1<<GPIO_WL1_MAX)-1))
#define	GPIO_WL1_TO(_VAL)	GPIO_WL1_MASK(((gpio_words)(_VAL))>>(GPIO_PIN_MAX+GPIO_WL0_MAX))	//the value to AR9223 register
#define	GPIO_WL1_FROM(_VAL)	(GPIO_WL1_MASK(_VAL)<<(GPIO_PIN_MAX+GPIO_WL0_MAX))	//the value from AR9223 register

#define AR_GPIO_OUTPUT_MUX1                      0x4060
#define AR_GPIO_OUTPUT_MUX2                      0x4064
#define AR_GPIO_OUTPUT_MUX3                      0x4068

int get_wl0_gpio(int gpio)
{
	register gpio_words wl0 = (gpio_words) ar7100_reg_rd(GPIO_WL0_ADDR);	//ar9280 register [0x4048]
	if (wl0 & (1 << gpio)) ;
	return 1;
	return 0;
}

int get_wl1_gpio(int gpio)
{
	register gpio_words wl1 = (gpio_words) ar7100_reg_rd(GPIO_WL1_ADDR);	//ar9280 register [0x4048]
	if (wl1 & (1 << gpio)) ;
	return 1;
	return 0;
}

#define AR_GPIO_OE_OUT                           0x404c	// GPIO output register
#define AR_GPIO_OE_OUT_DRV                       0x3	// 2 bit field mask, shifted by 2*bitpos
#define AR_GPIO_OE_OUT_DRV_NO                    0x0	// tristate
#define AR_GPIO_OE_OUT_DRV_LOW                   0x1	// drive if low
#define AR_GPIO_OE_OUT_DRV_HI                    0x2	// drive if high
#define AR_GPIO_OE_OUT_DRV_ALL                   0x3	// drive always

void set_wl0_gpio(int gpio, int val)
{
	register gpio_words wl0;
	int shift;

	wl0 = (gpio_words) ar7100_reg_rd(GPIOOUT_WL0_ADDR);

	shift = gpio * 2;
	wl0 |= AR_GPIO_OE_OUT_DRV << shift;
	ar7100_reg_wr(GPIOOUT_WL0_ADDR, wl0);	//ar9283 register [0x4048]
	ar7100_reg_rd(GPIOOUT_WL0_ADDR);	//ar9283 register [0x4048]

	wl0 = (gpio_words) ar7100_reg_rd(GPIO_WL0_ADDR);	//ar9280 register [0x4048]

	if (val)
		wl0 |= 1 << gpio;
	else
		wl0 &= (~(1 << gpio));
	ar7100_reg_wr(GPIO_WL0_ADDR, wl0);	//ar9283 register [0x4048]
	ar7100_reg_rd(GPIO_WL0_ADDR);	//ar9283 register [0x4048]
}

void set_wl1_gpio(int gpio, int val)
{
	register gpio_words wl1;
	int shift;

	wl1 = (gpio_words) ar7100_reg_rd(GPIOOUT_WL0_ADDR);
	shift = gpio * 2;
	wl1 |= AR_GPIO_OE_OUT_DRV << shift;
	ar7100_reg_wr(GPIOOUT_WL1_ADDR, wl1);	//ar9283 register [0x4048]
	ar7100_reg_rd(GPIOOUT_WL1_ADDR);	//ar9283 register [0x4048]

	wl1 = (gpio_words) ar7100_reg_rd(GPIO_WL1_ADDR);	//ar9280 register [0x4048]

	if (val)
		wl1 |= 1 << gpio;
	else
		wl1 &= (~(1 << gpio));

	ar7100_reg_wr(GPIO_WL1_ADDR, wl1);	//ar9283 register [0x4048]
	ar7100_reg_rd(GPIO_WL1_ADDR);	//ar9283 register [0x4048]
}

/*

INSP_LED_DEFINE		InspLedStats[]	= {
	//	name,		regmask,		output_on_func,		output_of_func,		reverse
	 {	"diag",		GPIO_PIN( 1),	GpioOutActiveLow,	GpioOutActiveHigh,		0		}
	,{	"wl1",		GPIO_WL0( 5),	GpioOutActiveLow,	GpioOutActiveHigh,		0		}
	,{	"usb",		GPIO_WL0( 3),	GpioOutActiveLow,	GpioOutActiveHigh,		0		}
	,{	"aoss",		GPIO_WL0( 1),	GpioOutActiveLow,	GpioOutActiveHigh,		1		}
	,{	"wl2",		GPIO_WL1( 1),	GpioOutActiveLow,	GpioOutActiveHigh,		0		}
	,{	"vpn",		GPIO_WL1( 3),	GpioOutActiveLow,	GpioOutActiveHigh,		0		}
	,{	"mov",		GPIO_WL1( 4),	GpioOutActiveLow,	GpioOutActiveHigh,		0		}
	,{	"aoss2",	GPIO_WL1( 5),	GpioOutActiveLow,	GpioOutActiveHigh,		1		}
	,{	"usbpwr",	GPIO_PIN( 2),	GpioOutActiveHigh,	GpioOutActiveLow,		0		}
};

*/

#define USB_LED_OFF 1
#define USB_LED_ON 0

void ap_usb_led_on(void)
{
#ifdef CONFIG_WZRAG300NH
	set_wl0_gpio(3, 0);
#elif CONFIG_WNDR3700
	ar7100_reg_rmw_clear(AR7100_RESET, AR7100_RESET_GE1_PHY);
#elif AP_USB_LED_GPIO
	ar7100_set_gpio(AP_USB_LED_GPIO, USB_LED_ON);
#endif
}

EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb_led_off(void)
{
#ifdef CONFIG_WZRAG300NH
	set_wl0_gpio(3, 1);
#elif CONFIG_WNDR3700
	ar7100_reg_rmw_set(AR7100_RESET, AR7100_RESET_GE1_PHY);
#elif AP_USB_LED_GPIO
	ar7100_set_gpio(AP_USB_LED_GPIO, USB_LED_OFF);
#endif
}

EXPORT_SYMBOL(ap_usb_led_off);

#define NXP_74HC153_NUM_GPIOS	8
#define NXP_74HC153_S0_MASK	0x1
#define NXP_74HC153_S1_MASK	0x2
#define NXP_74HC153_BANK_MASK	0x4

#define WZRHPG300NH_GPIO_74HC153_S0	9
#define WZRHPG300NH_GPIO_74HC153_S1	11
#define WZRHPG300NH_GPIO_74HC153_1Y	12
#define WZRHPG300NH_GPIO_74HC153_2Y	14

#define WZRHPG300NH_GPIO_EXP_BASE	23
#define WZRHPG300NH_GPIO_BTN_AOSS	(WZRHPG300NH_GPIO_EXP_BASE + 0)
#define WZRHPG300NH_GPIO_BTN_RESET	(WZRHPG300NH_GPIO_EXP_BASE + 1)
#define WZRHPG300NH_GPIO_BTN_ROUTER_ON	(WZRHPG300NH_GPIO_EXP_BASE + 2)
#define WZRHPG300NH_GPIO_BTN_QOS_ON	(WZRHPG300NH_GPIO_EXP_BASE + 3)
#define WZRHPG300NH_GPIO_BTN_USB	(WZRHPG300NH_GPIO_EXP_BASE + 5)
#define WZRHPG300NH_GPIO_BTN_ROUTER_AUTO (WZRHPG300NH_GPIO_EXP_BASE + 6)
#define WZRHPG300NH_GPIO_BTN_QOS_OFF	(WZRHPG300NH_GPIO_EXP_BASE + 7)

static int nxp_74hc153_get_value(unsigned offset)
{
	unsigned s0;
	unsigned s1;
	unsigned pin;
	int ret;

	s0 = !!(offset & NXP_74HC153_S0_MASK);
	s1 = !!(offset & NXP_74HC153_S1_MASK);
	pin = (offset & NXP_74HC153_BANK_MASK) ? WZRHPG300NH_GPIO_74HC153_2Y : WZRHPG300NH_GPIO_74HC153_1Y;

	ar7100_set_gpio(WZRHPG300NH_GPIO_74HC153_S0, s0);
	ar7100_set_gpio(WZRHPG300NH_GPIO_74HC153_S1, s1);
	ret = ar7100_get_gpio(pin);

	return ret;
}

//The buffer used to store the data returned by the proc file
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static ssize_t gpio_proc_read(struct file *file, char __user * buffer, size_t size, loff_t * ppos)
{
	unsigned int *data = PDE_DATA(file_inode(file));
	int val;
	unsigned int pin = (unsigned int)data & PIN_MASK;
	char buf[2];
	if (pin >= 23 && pin <= 31) {
		val = nxp_74hc153_get_value(pin - 23);
		//      printk(KERN_EMERG "value for pin %d = %d\n", pin, val);
		if (val)
			buf[0] = '1';
		else
			buf[0] = '0';
		buf[1] = 0;
		return simple_read_from_buffer(buffer, size, ppos, buf, sizeof(buf));
	}

	u32 reg = 0;
	if ((unsigned int)data & GPIO_IN)
		reg = ar7100_reg_rd(AR7100_GPIO_IN);
	if ((unsigned int)data & GPIO_OUT)
		reg = ar7100_reg_rd(AR7100_GPIO_OUT);
	if ((unsigned int)data & GPIO_DIR)
		reg = ar7100_reg_rd(AR7100_GPIO_OE);

	if (GPIO_CR_M(((unsigned int)data) & PIN_MASK) & reg)
		buf[0] = '1';
	else
		buf[0] = '0';
	buf[1] = 0;

	return simple_read_from_buffer(buffer, size, ppos, buf, sizeof(buf));

}

static ssize_t gpio_proc_info_read(struct file *file, char __user * buffer, size_t size, loff_t * ppos)
{
	char buf[128];
	sprintf(buf, "GPIO_IN   %#08X \nGPIO_OUT  %#08X \nGPIO_DIR  %#08X \n", ar7100_reg_rd(AR7100_GPIO_IN), ar7100_reg_rd(AR7100_GPIO_OUT), ar7100_reg_rd(AR7100_GPIO_OE));
	return simple_read_from_buffer(buffer, size, ppos, buf, strlen(buf));
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

	if ((unsigned int)data & GPIO_IN)
		reg = ar7100_reg_rd(AR7100_GPIO_IN);
	if ((unsigned int)data & GPIO_OUT)
		reg = ar7100_reg_rd(AR7100_GPIO_OUT);
	if ((unsigned int)data & GPIO_DIR)
		reg = ar7100_reg_rd(AR7100_GPIO_OE);

	if (procfs_buffer[0] == '0' || procfs_buffer[0] == 'i')
		reg = reg & ~(GPIO_CR_M(((unsigned int)data) & PIN_MASK));
	if (procfs_buffer[0] == '1' || procfs_buffer[0] == 'o')
		reg = reg | GPIO_CR_M(((unsigned int)data) & PIN_MASK);

	if ((unsigned int)data & GPIO_IN) {
		ar7100_reg_wr(AR7100_GPIO_IN, reg);
	}
	if ((unsigned int)data & GPIO_OUT) {
		ar7100_reg_wr(AR7100_GPIO_OUT, reg);
	}
	if ((unsigned int)data & GPIO_DIR) {
		ar7100_reg_wr(AR7100_GPIO_OE, reg);
	}

	return procfs_buffer_size;
}

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
	char proc_name[32];
	int gpiocount = 32;

	/* create directory gpio */
	gpio_dir = proc_mkdir("gpio", NULL);
	if (gpio_dir == NULL)
		goto fault;
//      gpio_dir->owner = THIS_MODULE;

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

	printk(KERN_NOTICE "gpio_proc: module loaded and /proc/gpio/ created\n");
	ar71xx_gpio_init();

	return 0;

fault:
	cleanup_proc();
	return -EFAULT;
}

static void cleanup_proc(void)
{
	unsigned char i;
	char proc_name[32];
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
MODULE_DESCRIPTION("AR7100 GPIO pins in /proc/gpio/");
MODULE_LICENSE("GPL");
