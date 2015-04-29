/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *  Copyright (C) 2007 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *  some portions:
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/pci.h>

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/serial.h>
#include <asm/traps.h>
#include <linux/serial_core.h>
#include <asm/bootinfo.h>

#include "ar7100.h"

#ifdef CONFIG_AR9100
#include "ar9100.h"
#endif

#ifdef CONFIG_AR7100_EMULATION
#define         AG7100_CONSOLE_BAUD (9600)
#else
#define         AG7100_CONSOLE_BAUD (115200)
#endif

#define AR71XX_MEM_SIZE_MIN	0x0200000
#define AR71XX_MEM_SIZE_MAX	0x8000000

#define AR71XX_BASE_FREQ	40000000
#define AR91XX_BASE_FREQ	5000000

u32 ar71xx_cpu_freq;
EXPORT_SYMBOL_GPL(ar71xx_cpu_freq);

u32 ar71xx_ahb_freq;
EXPORT_SYMBOL_GPL(ar71xx_ahb_freq);

u32 ar71xx_ddr_freq;
EXPORT_SYMBOL_GPL(ar71xx_ddr_freq);

static int __init ar7100_init_ioc(void);
void Uart16550Init(void);

void ar7100_restart(char *command)
{
	for (;;) {
		ar7100_reg_wr(AR7100_RESET, AR7100_RESET_FULL_CHIP);
	}
}

void ar7100_halt(void)
{
	printk(KERN_NOTICE "\n** You can safely turn off the power\n");
	while (1) ;
}

void ar7100_power_off(void)
{
	ar7100_halt();
}

#define sysRegRead(phys)	\
	(*(volatile u32 *)KSEG1ADDR(phys))

int is_ar9000 = 0;
EXPORT_SYMBOL(is_ar9000);
const char
*get_system_type(void)
{
	char *chip;
	u32 id;
	u32 rev;
	static char str[64];
	is_ar9000 = 0;
	id = ar71xx_reset_rr(RESET_REG_REV_ID) & REV_ID_MASK;
	rev = (id >> REV_ID_REVISION_SHIFT) & REV_ID_REVISION_MASK;
	switch (id & REV_ID_CHIP_MASK) {
	case REV_ID_CHIP_AR7130:
		chip = "7130";
		break;
	case REV_ID_CHIP_AR7141:
		chip = "7141";
		break;
	case REV_ID_CHIP_AR7161:
		chip = "7161";
		break;
	case REV_ID_CHIP_AR9130:
		chip = "9130";
		is_ar9000 = 1;
		break;
	case REV_ID_CHIP_AR9132:
		chip = "9132";
		is_ar9000 = 1;
		break;
	default:
		chip = "71xx";
	}
	sprintf(str, "Atheros AR%s rev %u (0x%02x)", chip, rev, id);
	return str;
}

#if defined(CONFIG_AR9100)
int valid_wmac_num(u_int16_t wmac_num)
{
	return (wmac_num == 0);
}

/*
 * HOWL has only one wmac device, hence the following routines
 * ignore the wmac_num parameter
 */
int get_wmac_irq(u_int16_t wmac_num)
{
	return AR7100_CPU_IRQ_WMAC;
}

unsigned long get_wmac_base(u_int16_t wmac_num)
{
	return KSEG1ADDR(AR9100_WMAC_BASE);
}

unsigned long get_wmac_mem_len(u_int16_t wmac_num)
{
	return AR9100_WMAC_LEN;
}

void enable_wmac_led()
{
	ar7100_reg_rmw_set(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_WMAC_LED);
}

EXPORT_SYMBOL(enable_wmac_led);

extern void ar7100_set_gpio(int gpio, int val);

void disable_wmac_led()
{
	ar7100_reg_rmw_clear(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_WMAC_LED);

	ar7100_set_gpio(6, 1);
}

EXPORT_SYMBOL(disable_wmac_led);

EXPORT_SYMBOL(valid_wmac_num);
EXPORT_SYMBOL(get_wmac_irq);
EXPORT_SYMBOL(get_wmac_base);
EXPORT_SYMBOL(get_wmac_mem_len);
#endif

EXPORT_SYMBOL(get_system_type);

static void __init ar71xx_detect_mem_size(void)
{
	volatile u8 *p;
	u8 memsave;
	u32 size;

	p = (volatile u8 *)KSEG1ADDR(0);
	memsave = *p;
	for (size = AR71XX_MEM_SIZE_MIN; size <= (AR71XX_MEM_SIZE_MAX >> 1); size <<= 1) {
		volatile u8 *r;

		r = (p + size);
		*p = 0x55;
		if (*r == 0x55) {
			/* Mirrored data found, try another pattern */
			*p = 0xAA;
			if (*r == 0xAA) {
				/* Mirrored data found again, stop detection */
				break;
			}
		}
	}
	*p = memsave;

	add_memory_region(0, size, BOOT_MEM_RAM);
}

/*
 * The bootloader musta set cpu_pll_config.
 * We extract the pll divider, multiply it by the base freq 40.
 * The cpu and ahb are divided off of that.
 */
static void __init ar91xx_detect_sys_frequency(void)
{
	u32 pll;
	u32 freq;
	u32 div;

	pll = ar71xx_pll_rr(PLL_REG_CPU_PLL_CFG);

	div = ((pll >> AR91XX_PLL_DIV_SHIFT) & AR91XX_PLL_DIV_MASK);
	freq = div * AR91XX_BASE_FREQ;

	ar71xx_cpu_freq = freq;

	div = ((pll >> AR91XX_DDR_DIV_SHIFT) & AR91XX_DDR_DIV_MASK) + 1;
	ar71xx_ddr_freq = freq / div;

	div = (((pll >> AR91XX_AHB_DIV_SHIFT) & AR91XX_AHB_DIV_MASK) + 1) * 2;
	ar71xx_ahb_freq = ar71xx_cpu_freq / div;
}

static void __init ar71xx_detect_sys_frequency(void)
{
	u32 pll;
	u32 freq;
	u32 div;

	if ((ar71xx_reset_rr(RESET_REG_REV_ID) & REV_ID_MASK) >= REV_ID_CHIP_AR9130) {
		return ar91xx_detect_sys_frequency();
	}

	pll = ar71xx_pll_rr(PLL_REG_CPU_PLL_CFG);

	div = ((pll >> AR71XX_PLL_DIV_SHIFT) & AR71XX_PLL_DIV_MASK) + 1;
	freq = div * AR71XX_BASE_FREQ;

	div = ((pll >> AR71XX_CPU_DIV_SHIFT) & AR71XX_CPU_DIV_MASK) + 1;
	ar71xx_cpu_freq = freq / div;

	div = ((pll >> AR71XX_DDR_DIV_SHIFT) & AR71XX_DDR_DIV_MASK) + 1;
	ar71xx_ddr_freq = freq / div;

	div = (((pll >> AR71XX_AHB_DIV_SHIFT) & AR71XX_AHB_DIV_MASK) + 1) * 2;
	ar71xx_ahb_freq = ar71xx_cpu_freq / div;
}

#define AR71XX_UART_FLAGS (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST)
extern int early_serial_setup(struct uart_port *port);

void __init ar7100_serial_setup(void)
{
	struct uart_port p;

	memset(&p, 0, sizeof(p));

	p.flags = AR71XX_UART_FLAGS;
	p.iotype = UPIO_MEM32;
	p.uartclk = ar71xx_ahb_freq;
	p.irq = AR7100_MISC_IRQ_UART;
	p.regshift = 2;
	p.mapbase = (u32)KSEG1ADDR(AR7100_UART_BASE);
	p.membase = (void __iomem *)p.mapbase;

	if (early_serial_setup(&p) != 0)
		printk(KERN_ERR "early_serial_setup failed\n");

}

unsigned int __cpuinit get_c0_compare_irq(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

unsigned int __cpuinit get_c0_compare_int(void)
{
	//printk("%s: returning timer irq : %d\n",__func__, AR7240_CPU_IRQ_TIMER);
	return AR7100_CPU_IRQ_TIMER;
}

void __init plat_time_init(void)
{
	mips_hpt_frequency = ar71xx_cpu_freq / 2;
	printk("%s: plat time init done\n", __func__);
}

#if 1
int ar7100_be_handler(struct pt_regs *regs, int is_fixup)
{
#ifndef CONFIG_AR9100
	int error = 0, status, trouble = 0;
#if 0
	if (!is_fixup && (regs->cp0_cause & 4)) {
		/* Data bus error - print PA */
		printk("DBE physical address: %010Lx\n", __read_64bit_c0_register($26, 1));
	}
#endif
	error = ar7100_reg_rd(AR7100_PCI_ERROR) & 3;

	if (error) {
		printk("PCI error %d at PCI addr 0x%x\n", error, ar7100_reg_rd(AR7100_PCI_ERROR_ADDRESS));
		ar7100_reg_wr(AR7100_PCI_ERROR, error);
		ar7100_local_read_config(PCI_STATUS, 2, &status);
		printk("PCI status: %#x\n", status);
		trouble = 1;
	}

	error = 0;
	error = ar7100_reg_rd(AR7100_PCI_AHB_ERROR) & 1;

	if (error) {
		printk("AHB error at AHB address 0x%x\n", ar7100_reg_rd(AR7100_PCI_AHB_ERROR_ADDRESS));
		ar7100_reg_wr(AR7100_PCI_AHB_ERROR, error);
		ar7100_local_read_config(PCI_STATUS, 2, &status);
		printk("PCI status: %#x\n", status);
		trouble = 1;
	}
#endif

	printk("ar7100 data bus error: cause %#x\n", read_c0_cause());
	return (is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL);
}
#endif

void ar7100_gpio_config_output(int gpio);

void __init plat_mem_setup(void)
{
#if 1
	board_be_handler = ar7100_be_handler;
#endif
//      board_time_init = ar7100_timer_init;
	_machine_restart = ar7100_restart;
	_machine_halt = ar7100_halt;
	pm_power_off = ar7100_power_off;

	ar71xx_ddr_base = ioremap_nocache(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE);

	ar71xx_pll_base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	ar71xx_reset_base = ioremap_nocache(AR71XX_RESET_BASE, AR71XX_RESET_SIZE);

	ar71xx_gpio_base = ioremap_nocache(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);

	ar71xx_usb_ctrl_base = ioremap_nocache(AR71XX_USB_CTRL_BASE, AR71XX_USB_CTRL_SIZE);

	ar71xx_detect_mem_size();

	/* 
	 ** early_serial_setup seems to conflict with serial8250_register_port() 
	 ** In order for console to work, we need to call register_console().
	 ** We can call serial8250_register_port() directly or use
	 ** platform_add_devices() function which eventually calls the 
	 ** register_console(). AP71 takes this approach too. Only drawback
	 ** is if system screws up before we register console, we won't see
	 ** any msgs on the console.  System being stable now this should be
	 ** a special case anyways. Just initialize Uart here.
	 */

	Uart16550Init();

#if 0
	ar7100_serial_setup();
#endif
	printk(KERN_INFO "booting platform %s\n", get_system_type());
#ifdef CONFIG_AR9100
	ar7100_set_gpio(6, 1);
	ar7100_reg_wr(AR9100_OBS_GPIO_1, 0x16);
	ar7100_reg_wr(AR9100_OBS_OE, 0x40);
#endif
}

/*
 * -------------------------------------------------
 * Early printk hack
 */
/* === CONFIG === */

#define		REG_OFFSET		4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define         UART16550_READ(y)   ar7100_reg_rd((AR7100_UART_BASE+y))
#define         UART16550_WRITE(x, z)  ar7100_reg_wr((AR7100_UART_BASE+x), z)

static int serial_inited = 0;

#define         MY_WRITE(y, z)  ((*((volatile u32*)(y))) = z)

void Uart16550Init()
{
	int freq, div;

	ar71xx_detect_sys_frequency();
	freq = ar71xx_ahb_freq;

#if 0
	MY_WRITE(0xb8040000, 0xcff);
	MY_WRITE(0xb8040008, 0x3b);

	MY_WRITE(0xb8040028, 0x100);

	MY_WRITE(0xb8040008, 0x2f);
#endif

	div = freq / (AG7100_CONSOLE_BAUD * 16);

	/* set DIAB bit */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x80);

	/* set divisor */
	UART16550_WRITE(OFS_DIVISOR_LSB, (div & 0xff));
	UART16550_WRITE(OFS_DIVISOR_MSB, (div >> 8) & 0xff);

	/*UART16550_WRITE(OFS_DIVISOR_LSB, 0x61);
	   UART16550_WRITE(OFS_DIVISOR_MSB, 0x03); */

	/* clear DIAB bit */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x00);

	/* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

	UART16550_WRITE(OFS_INTR_ENABLE, 0);
}

u8 Uart16550GetPoll(void)
{
	while ((UART16550_READ(OFS_LINE_STATUS) & 0x1) == 0) ;
	return UART16550_READ(OFS_RCV_BUFFER);
}

void Uart16550Put(u8 byte)
{
	if (!serial_inited) {
		serial_inited = 1;
		Uart16550Init();
	}
	while (((UART16550_READ(OFS_LINE_STATUS)) & 0x20) == 0x0) ;
	UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

unsigned int ar7100_serial_in(int offset)
{
	return UART16550_READ(offset);
}

void ar7100_serial_out(int offset, int value)
{
	UART16550_WRITE(offset, (u8)value);
}

#include <asm/uaccess.h>
#define M_PERFCTL_EVENT(event)          ((event) << 5)
unsigned int clocks_at_start;

void start_cntrs(unsigned int event0, unsigned int event1)
{
	write_c0_perfcntr0(0x00000000);
	write_c0_perfcntr1(0x00000000);
	/*
	 * go...
	 */
	write_c0_perfctrl0(0x80000000 | M_PERFCTL_EVENT(event0) | 0xf);
	write_c0_perfctrl1(0x00000000 | M_PERFCTL_EVENT(event1) | 0xf);
}

void stop_cntrs(void)
{
	write_c0_perfctrl0(0);
	write_c0_perfctrl1(0);
}

void read_cntrs(unsigned int *c0, unsigned int *c1)
{
	*c0 = read_c0_perfcntr0();
	*c1 = read_c0_perfcntr1();
}

static int ar7100_ioc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t ar7100_ioc_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	unsigned int c0, c1, ticks = (read_c0_count() - clocks_at_start);
	char str[256];
	unsigned int secs = ticks / mips_hpt_frequency;

	read_cntrs(&c0, &c1);
	stop_cntrs();
	sprintf(str, "%d secs (%#x) event0:%#x event1:%#x", secs, ticks, c0, c1);
	copy_to_user(buf, str, strlen(str));

	return (strlen(str));
}

#if 0
static void ar7100_dcache_test(void)
{
	int i, j;
	unsigned char p;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < (10 * 1024); j++) {
			p = *((unsigned char *)0x81000000 + j);
		}
	}
}
#endif

static ssize_t ar7100_ioc_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{
	int event0, event1;

	sscanf(buf, "%d:%d", &event0, &event1);
	printk("\nevent0 %d event1 %d\n", event0, event1);

	clocks_at_start = read_c0_count();
	start_cntrs(event0, event1);

	return (count);
}

struct file_operations ar7100_ioc_fops = {
	.open = ar7100_ioc_open,
	.read = ar7100_ioc_read,
	.write = ar7100_ioc_write,
};

/*
 * General purpose ioctl i/f
 */
int __init ar7100_init_ioc(void)
{
	static int _mymajor;

	_mymajor = register_chrdev(77, "AR7100_GPIOC", &ar7100_ioc_fops);

	if (_mymajor < 0) {
		printk("Failed to register GPIOC\n");
		return _mymajor;
	}

	printk("AR7100 GPIOC major %d\n", _mymajor);
	return 0;
}

unsigned int getCPUClock(void)
{
	return ar71xx_cpu_freq / 1000000;
}

device_initcall(ar7100_init_ioc);
