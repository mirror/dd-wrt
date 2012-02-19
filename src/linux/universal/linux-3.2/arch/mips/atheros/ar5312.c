/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006-2009 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * Platform devices for Atheros SoCs
 */

#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/mtd/physmap.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/leds.h>
#include <asm/bootinfo.h>
#include <asm/reboot.h>
#include <asm/time.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <gpio.h>

#include <ar231x_platform.h>
#include <ar5312_regs.h>
#include <ar231x.h>
#include "devices.h"
#include "ar5312.h"

static void
ar5312_misc_irq_dispatch(void)
{
	unsigned int ar231x_misc_intrs = ar231x_read_reg(AR531X_ISR) & ar231x_read_reg(AR531X_IMR);

	if (ar231x_misc_intrs & AR531X_ISR_TIMER) {
		do_IRQ(AR531X_MISC_IRQ_TIMER);
		(void)ar231x_read_reg(AR531X_TIMER);
	} else if (ar231x_misc_intrs & AR531X_ISR_AHBPROC)
		do_IRQ(AR531X_MISC_IRQ_AHB_PROC);
	else if ((ar231x_misc_intrs & AR531X_ISR_UART0))
		do_IRQ(AR531X_MISC_IRQ_UART0);
	else if (ar231x_misc_intrs & AR531X_ISR_WD)
		do_IRQ(AR531X_MISC_IRQ_WATCHDOG);
	else
		do_IRQ(AR531X_MISC_IRQ_NONE);
}

static asmlinkage void
ar5312_irq_dispatch(void)
{
	int pending = read_c0_status() & read_c0_cause();

	if (pending & CAUSEF_IP2)
		do_IRQ(AR5312_IRQ_WLAN0_INTRS);
	else if (pending & CAUSEF_IP3)
		do_IRQ(AR5312_IRQ_ENET0_INTRS);
	else if (pending & CAUSEF_IP4)
		do_IRQ(AR5312_IRQ_ENET1_INTRS);
	else if (pending & CAUSEF_IP5)
		do_IRQ(AR5312_IRQ_WLAN1_INTRS);
	else if (pending & CAUSEF_IP6)
		ar5312_misc_irq_dispatch();
	else if (pending & CAUSEF_IP7)
		do_IRQ(AR531X_IRQ_CPU_CLOCK);
}


/* Enable the specified AR531X_MISC_IRQ interrupt */
static void
ar5312_misc_intr_enable(struct irq_data *irq)
{
	unsigned int imr;

	imr = ar231x_read_reg(AR531X_IMR);
	imr |= (1 << (irq->irq - AR531X_MISC_IRQ_BASE - 1));
	ar231x_write_reg(AR531X_IMR, imr);
}

/* Disable the specified AR531X_MISC_IRQ interrupt */
static void
ar5312_misc_intr_disable(struct irq_data *irq)
{
	unsigned int imr;

	imr = ar231x_read_reg(AR531X_IMR);
	imr &= ~(1 << (irq->irq - AR531X_MISC_IRQ_BASE - 1));
	ar231x_write_reg(AR531X_IMR, imr);
	ar231x_read_reg(AR531X_IMR); /* flush write buffer */
}

static void
ar5312_misc_intr_end(struct irq_data *irq)
{
	if (!(irq_desc[irq->irq].status_use_accessors & (IRQF_DISABLED)))
		ar5312_misc_intr_enable(irq);
}

static struct irq_chip ar5312_misc_intr_controller = {
	.name     = "AR5312-MISC",
	.irq_disable  = ar5312_misc_intr_disable,
	.irq_ack      = ar5312_misc_intr_disable,
	.irq_mask_ack = ar5312_misc_intr_disable,
	.irq_mask     = ar5312_misc_intr_disable,
	.irq_unmask   = ar5312_misc_intr_enable,
	.irq_shutdown = ar5312_misc_intr_end,
};


static irqreturn_t ar5312_ahb_proc_handler(int cpl, void *dev_id)
{
	u32 proc1 = ar231x_read_reg(AR531X_PROC1);
	u32 procAddr = ar231x_read_reg(AR531X_PROCADDR); /* clears error state */
	u32 dma1 = ar231x_read_reg(AR531X_DMA1);
	u32 dmaAddr = ar231x_read_reg(AR531X_DMAADDR);   /* clears error state */

	printk("AHB interrupt: PROCADDR=0x%8.8x  PROC1=0x%8.8x  DMAADDR=0x%8.8x  DMA1=0x%8.8x\n",
			procAddr, proc1, dmaAddr, dma1);

	machine_restart("AHB error"); /* Catastrophic failure */
	return IRQ_HANDLED;
}


static struct irqaction ar5312_ahb_proc_interrupt  = {
	.handler = ar5312_ahb_proc_handler,
	.flags   = IRQF_DISABLED,
	.name    = "ar5312_ahb_proc_interrupt",
};


static struct irqaction cascade  = {
	.handler = no_action,
	.flags   = IRQF_DISABLED,
	.name    = "cascade",
};

void __init ar5312_irq_init(void)
{
	int i;

	if (!is_5312())
		return;

	ar231x_irq_dispatch = ar5312_irq_dispatch;
	for (i = 0; i < AR531X_MISC_IRQ_COUNT; i++) {
		int irq = AR531X_MISC_IRQ_BASE + i;
		irq_set_chip_and_handler(irq, &ar5312_misc_intr_controller,
			handle_level_irq);
	}
	setup_irq(AR531X_MISC_IRQ_AHB_PROC, &ar5312_ahb_proc_interrupt);
	setup_irq(AR5312_IRQ_MISC_INTRS, &cascade);
}

const struct ar231x_gpiodev ar5312_gpiodev;

static u32
ar5312_gpio_get_output(void)
{
	u32 reg;
	reg = ~(ar231x_read_reg(AR531X_GPIO_CR));
	reg &= ar5312_gpiodev.valid_mask;
	return reg;
}

static u32
ar5312_gpio_set_output(u32 mask, u32 val)
{
	u32 reg;

	reg = ar231x_read_reg(AR531X_GPIO_CR);
	reg |= mask;
	reg &= ~val;
	ar231x_write_reg(AR531X_GPIO_CR, reg);
	return reg;
}

static u32
ar5312_gpio_get(void)
{
	u32 reg;
	reg = ar231x_read_reg(AR531X_GPIO_DI);
	reg &= ar5312_gpiodev.valid_mask;
	return reg;
}

static u32
ar5312_gpio_set(u32 mask, u32 value)
{
	u32 reg;
	reg = ar231x_read_reg(AR531X_GPIO_DO);
	reg &= ~mask;
	reg |= value;
	ar231x_write_reg(AR531X_GPIO_DO, reg);
	return reg;
}

const struct ar231x_gpiodev ar5312_gpiodev = {
	.valid_mask = (1 << 8) - 1,
	.get_output = ar5312_gpio_get_output,
	.set_output = ar5312_gpio_set_output,
	.get = ar5312_gpio_get,
	.set = ar5312_gpio_set,
};

static struct physmap_flash_data ar5312_flash_data = {
	.width = 2,
};

static struct resource ar5312_flash_resource = {
	.start = AR531X_FLASH,
	.end = AR531X_FLASH + 0x800000 - 1,
	.flags = IORESOURCE_MEM,
};

static struct ar231x_eth ar5312_eth0_data = {
	.reset_base = AR531X_RESET,
	.reset_mac = AR531X_RESET_ENET0,
	.reset_phy = AR531X_RESET_EPHY0,
	.phy_base = KSEG1ADDR(AR531X_ENET0),
	.config = &ar231x_board,
};

static struct ar231x_eth ar5312_eth1_data = {
	.reset_base = AR531X_RESET,
	.reset_mac = AR531X_RESET_ENET1,
	.reset_phy = AR531X_RESET_EPHY1,
	.phy_base = KSEG1ADDR(AR531X_ENET1),
	.config = &ar231x_board,
};

static struct platform_device ar5312_physmap_flash = {
	.name = "physmap-flash",
	.id = 0,
	.dev.platform_data = &ar5312_flash_data,
	.resource = &ar5312_flash_resource,
	.num_resources = 1,
};

#ifdef CONFIG_LEDS_GPIO
static struct gpio_led ar5312_leds[] = {
	{ .name = "wlan", .gpio = 0, .active_low = 1, },
};

static const struct gpio_led_platform_data ar5312_led_data = {
	.num_leds = ARRAY_SIZE(ar5312_leds),
	.leds = (void *) ar5312_leds,
};

static struct platform_device ar5312_gpio_leds = {
	.name = "leds-gpio",
	.id = -1,
	.dev.platform_data = (void *) &ar5312_led_data,
};
#endif

/*
 * NB: This mapping size is larger than the actual flash size,
 * but this shouldn't be a problem here, because the flash
 * will simply be mapped multiple times.
 */
static char __init *ar5312_flash_limit(void)
{
	u32 ctl;
	/*
	 * Configure flash bank 0.
	 * Assume 8M window size. Flash will be aliased if it's smaller
	 */
	ctl = FLASHCTL_E |
		FLASHCTL_AC_8M |
		FLASHCTL_RBLE |
		(0x01 << FLASHCTL_IDCY_S) |
		(0x07 << FLASHCTL_WST1_S) |
		(0x07 << FLASHCTL_WST2_S) |
		(ar231x_read_reg(AR531X_FLASHCTL0) & FLASHCTL_MW);

	ar231x_write_reg(AR531X_FLASHCTL0, ctl);

	/* Disable other flash banks */
	ar231x_write_reg(AR531X_FLASHCTL1,
		ar231x_read_reg(AR531X_FLASHCTL1) & ~(FLASHCTL_E | FLASHCTL_AC));

	ar231x_write_reg(AR531X_FLASHCTL2,
		ar231x_read_reg(AR531X_FLASHCTL2) & ~(FLASHCTL_E | FLASHCTL_AC));

	return (char *) KSEG1ADDR(AR531X_FLASH + 0x800000);
}

int __init ar5312_init_devices(void)
{
	struct ar231x_boarddata *config;
	u32 fctl = 0;
	u8 *c;

	if (!is_5312())
		return 0;

	/* Locate board/radio config data */
	ar231x_find_config(ar5312_flash_limit());
	config = ar231x_board.config;

	/* AR2313 has CPU minor rev. 10 */
	if ((current_cpu_data.processor_id & 0xff) == 0x0a)
		ar231x_devtype = DEV_TYPE_AR2313;

	/* AR2312 shares the same Silicon ID as AR5312 */
	else if (config->flags & BD_ISCASPER)
		ar231x_devtype = DEV_TYPE_AR2312;

	/* Everything else is probably AR5312 or compatible */
	else
		ar231x_devtype = DEV_TYPE_AR5312;

	/* fixup flash width */
	fctl = ar231x_read_reg(AR531X_FLASHCTL) & FLASHCTL_MW;
	switch (fctl) {
	case FLASHCTL_MWx16:
		ar5312_flash_data.width = 2;
		break;
	case FLASHCTL_MWx8:
	default:
		ar5312_flash_data.width = 1;
		break;
	}

	platform_device_register(&ar5312_physmap_flash);

#ifdef CONFIG_LEDS_GPIO
	ar5312_leds[0].gpio = config->sysLedGpio;
	platform_device_register(&ar5312_gpio_leds);
#endif

	/* Fix up MAC addresses if necessary */
	if (!memcmp(config->enet0_mac, "\xff\xff\xff\xff\xff\xff", 6))
		memcpy(config->enet0_mac, config->enet1_mac, 6);

	/* If ENET0 and ENET1 have the same mac address,
	 * increment the one from ENET1 */
	if (memcmp(config->enet0_mac, config->enet1_mac, 6) == 0) {
		c = config->enet1_mac + 5;
		while ((c >= config->enet1_mac) && !(++(*c)))
			c--;
	}

	switch(ar231x_devtype) {
	case DEV_TYPE_AR5312:
		ar5312_eth0_data.macaddr = config->enet0_mac;
		ar231x_add_ethernet(0, KSEG1ADDR(AR531X_ENET0),
			AR5312_IRQ_ENET0_INTRS, &ar5312_eth0_data);

		ar5312_eth1_data.macaddr = config->enet1_mac;
		ar231x_add_ethernet(1, KSEG1ADDR(AR531X_ENET1),
			AR5312_IRQ_ENET1_INTRS, &ar5312_eth1_data);

		if (!ar231x_board.radio)
			return 0;

		if (!(config->flags & BD_WLAN0))
			break;

		ar231x_add_wmac(0, AR531X_WLAN0, AR5312_IRQ_WLAN0_INTRS);
		break;
	/*
	 * AR2312/3 ethernet uses the PHY of ENET0, but the MAC
	 * of ENET1. Atheros calls it 'twisted' for a reason :)
	 */
	case DEV_TYPE_AR2312:
	case DEV_TYPE_AR2313:
		ar5312_eth1_data.phy_base = ar5312_eth0_data.phy_base;
		ar5312_eth1_data.reset_phy = ar5312_eth0_data.reset_phy;
		ar5312_eth1_data.macaddr = config->enet0_mac;
		ar231x_add_ethernet(0, KSEG1ADDR(AR531X_ENET1),
			AR5312_IRQ_ENET1_INTRS, &ar5312_eth1_data);

		if (!ar231x_board.radio)
			return 0;
		break;
	default:
		break;
	}

	if (config->flags & BD_WLAN1)
		ar231x_add_wmac(1, AR531X_WLAN1, AR5312_IRQ_WLAN1_INTRS);

	return 0;
}


static void ar5312_restart(char *command)
{
	/* reset the system */
	local_irq_disable();
	while(1) {
		ar231x_write_reg(AR531X_RESET, AR531X_RESET_SYSTEM);
	}
}


/*
 * This table is indexed by bits 5..4 of the CLOCKCTL1 register
 * to determine the predevisor value.
 */
static int __initdata CLOCKCTL1_PREDIVIDE_TABLE[4] = { 1, 2, 4, 5 };


unsigned int __init
ar5312_cpu_frequency(void)
{
	unsigned int result;
	unsigned int predivide_mask, predivide_shift;
	unsigned int multiplier_mask, multiplier_shift;
	unsigned int clockCtl1, preDivideSelect, preDivisor, multiplier;
	unsigned int doubler_mask;
	u16 devid;

	/* Trust the bootrom's idea of cpu frequency. */
	if ((result = ar231x_read_reg(AR5312_SCRATCH)))
		return result;

	devid = ar231x_read_reg(AR531X_REV);
	devid &= AR531X_REV_MAJ;
	devid >>= AR531X_REV_MAJ_S;
	if (devid == AR531X_REV_MAJ_AR2313) {
		predivide_mask = AR2313_CLOCKCTL1_PREDIVIDE_MASK;
		predivide_shift = AR2313_CLOCKCTL1_PREDIVIDE_SHIFT;
		multiplier_mask = AR2313_CLOCKCTL1_MULTIPLIER_MASK;
		multiplier_shift = AR2313_CLOCKCTL1_MULTIPLIER_SHIFT;
		doubler_mask = AR2313_CLOCKCTL1_DOUBLER_MASK;
	} else { /* AR5312 and AR2312 */
		predivide_mask = AR5312_CLOCKCTL1_PREDIVIDE_MASK;
		predivide_shift = AR5312_CLOCKCTL1_PREDIVIDE_SHIFT;
		multiplier_mask = AR5312_CLOCKCTL1_MULTIPLIER_MASK;
		multiplier_shift = AR5312_CLOCKCTL1_MULTIPLIER_SHIFT;
		doubler_mask = AR5312_CLOCKCTL1_DOUBLER_MASK;
	}

	/*
	 * Clocking is derived from a fixed 40MHz input clock.
	 *
	 *  cpuFreq = InputClock * MULT (where MULT is PLL multiplier)
	 *  sysFreq = cpuFreq / 4	   (used for APB clock, serial,
	 *							   flash, Timer, Watchdog Timer)
	 *
	 *  cntFreq = cpuFreq / 2	   (use for CPU count/compare)
	 *
	 * So, for example, with a PLL multiplier of 5, we have
	 *
	 *  cpuFreq = 200MHz
	 *  sysFreq = 50MHz
	 *  cntFreq = 100MHz
	 *
	 * We compute the CPU frequency, based on PLL settings.
	 */

	clockCtl1 = ar231x_read_reg(AR5312_CLOCKCTL1);
	preDivideSelect = (clockCtl1 & predivide_mask) >> predivide_shift;
	preDivisor = CLOCKCTL1_PREDIVIDE_TABLE[preDivideSelect];
	multiplier = (clockCtl1 & multiplier_mask) >> multiplier_shift;

	if (clockCtl1 & doubler_mask) {
		multiplier = multiplier << 1;
	}
	return (40000000 / preDivisor) * multiplier;
}

static inline int
ar5312_sys_frequency(void)
{
	return ar5312_cpu_frequency() / 4;
}

void __init
ar5312_time_init(void)
{
	if (!is_5312())
		return;

	mips_hpt_frequency = ar5312_cpu_frequency() / 2;
}


void __init
ar5312_prom_init(void)
{
	u32 memsize, memcfg, bank0AC, bank1AC;
	u32 devid;

	if (!is_5312())
		return;

	/* Detect memory size */
	memcfg = ar231x_read_reg(AR531X_MEM_CFG1);
	bank0AC = (memcfg & MEM_CFG1_AC0) >> MEM_CFG1_AC0_S;
	bank1AC = (memcfg & MEM_CFG1_AC1) >> MEM_CFG1_AC1_S;
	memsize = (bank0AC ? (1 << (bank0AC+1)) : 0)
	        + (bank1AC ? (1 << (bank1AC+1)) : 0);
	memsize <<= 20;
	add_memory_region(0, memsize, BOOT_MEM_RAM);

	devid = ar231x_read_reg(AR531X_REV);
	devid >>= AR531X_REV_WMAC_MIN_S;
	devid &= AR531X_REV_CHIP;
	ar231x_board.devid = (u16) devid;
	ar231x_gpiodev = &ar5312_gpiodev;
}
extern unsigned int ath_cpufreq;
void __init
ar5312_plat_setup(void)
{
	if (!is_5312())
		return;

	/* Clear any lingering AHB errors */
	ar231x_read_reg(AR531X_PROCADDR);
	ar231x_read_reg(AR531X_DMAADDR);
	ar231x_write_reg(AR531X_WD_CTRL, AR531X_WD_CTRL_IGNORE_EXPIRATION);
	ath_cpufreq = ar5312_sys_frequency();
	_machine_restart = ar5312_restart;
	ar231x_serial_setup(KSEG1ADDR(AR531X_UART0), ath_cpufreq);
	ath_cpufreq *=4;
}

