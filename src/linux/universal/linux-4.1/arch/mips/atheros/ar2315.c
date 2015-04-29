/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * Platform devices for Atheros SoCs
 */

#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/leds.h>
#include <asm/bootinfo.h>
#include <asm/reboot.h>
#include <asm/time.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <ar231x_platform.h>
#include <ar2315_regs.h>
#include <ar231x.h>
#include "devices.h"
#include "ar2315.h"

static u32 gpiointmask = 0, gpiointval = 0;

static inline void ar2315_gpio_irq(void)
{
	u32 pend;
	int bit = -1;

	/* only do one gpio interrupt at a time */
	pend = (ar231x_read_reg(AR2315_GPIO_DI) ^ gpiointval) & gpiointmask;

	if (pend) {
		bit = fls(pend) - 1;
		pend &= ~(1 << bit);
		gpiointval ^= (1 << bit);
	}

	if (!pend)
		ar231x_write_reg(AR2315_ISR, AR2315_ISR_GPIO);

	/* Enable interrupt with edge detection */
	if ((ar231x_read_reg(AR2315_GPIO_CR) & AR2315_GPIO_CR_M(bit)) != AR2315_GPIO_CR_I(bit))
		return;

	if (bit >= 0)
		do_IRQ(AR531X_GPIO_IRQ_BASE + bit);
}

#ifdef CONFIG_ATHEROS_AR2315_PCI
static inline void pci_abort_irq(void)
{
	ar231x_write_reg(AR2315_PCI_INT_STATUS, AR2315_PCI_ABORT_INT);
}

static inline void pci_ack_irq(void)
{
	ar231x_write_reg(AR2315_PCI_INT_STATUS, AR2315_PCI_EXT_INT);
}

void ar2315_pci_irq(int irq)
{
	if (ar231x_read_reg(AR2315_PCI_INT_STATUS) == AR2315_PCI_ABORT_INT)
		pci_abort_irq();
	else {
		do_IRQ(irq);
		pci_ack_irq();
	}
}
#endif /* CONFIG_ATHEROS_AR2315_PCI */

/*
 * Called when an interrupt is received, this function
 * determines exactly which interrupt it was, and it
 * invokes the appropriate handler.
 *
 * Implicitly, we also define interrupt priority by
 * choosing which to dispatch first.
 */
static asmlinkage void
ar2315_irq_dispatch(void)
{
	int pending = read_c0_status() & read_c0_cause();

	if (pending & CAUSEF_IP3)
		do_IRQ(AR2315_IRQ_WLAN0_INTRS);
	else if (pending & CAUSEF_IP4)
		do_IRQ(AR2315_IRQ_ENET0_INTRS);
#ifdef CONFIG_ATHEROS_AR2315_PCI
	else if (pending & CAUSEF_IP5)
		ar2315_pci_irq(AR2315_IRQ_LCBUS_PCI);
#endif
	else if (pending & CAUSEF_IP2) {
		unsigned int misc_intr = ar231x_read_reg(AR2315_ISR) & ar231x_read_reg(AR2315_IMR);

		if (misc_intr & AR2315_ISR_SPI)
			do_IRQ(AR531X_MISC_IRQ_SPI);
		else if (misc_intr & AR2315_ISR_TIMER)
			do_IRQ(AR531X_MISC_IRQ_TIMER);
		else if (misc_intr & AR2315_ISR_AHB)
			do_IRQ(AR531X_MISC_IRQ_AHB_PROC);
		else if (misc_intr & AR2315_ISR_GPIO)
			ar2315_gpio_irq();
		else if (misc_intr & AR2315_ISR_UART0)
			do_IRQ(AR531X_MISC_IRQ_UART0);
		else if (misc_intr & AR2315_ISR_WD)
			do_IRQ(AR531X_MISC_IRQ_WATCHDOG);
		else
			do_IRQ(AR531X_MISC_IRQ_NONE);
	} else if (pending & CAUSEF_IP7)
		do_IRQ(AR531X_IRQ_CPU_CLOCK);
}

static void ar2315_set_gpiointmask(int gpio, int level)
{
	u32 reg;

	reg = ar231x_read_reg(AR2315_GPIO_INT);
	reg &= ~(AR2315_GPIO_INT_M | AR2315_GPIO_INT_LVL_M);
	reg |= gpio | AR2315_GPIO_INT_LVL(level);
	ar231x_write_reg(AR2315_GPIO_INT, reg);
}

static void ar2315_gpio_intr_enable(struct irq_data *irq)
{
	unsigned int gpio = irq->irq - AR531X_GPIO_IRQ_BASE;

	/* Enable interrupt with edge detection */
	if ((ar231x_read_reg(AR2315_GPIO_CR) & AR2315_GPIO_CR_M(gpio)) != AR2315_GPIO_CR_I(gpio))
		return;

	gpiointmask |= (1 << gpio);
	ar2315_set_gpiointmask(gpio, 3);
}

static unsigned int ar2315_gpio_intr_startup(struct irq_data *irq)
{
	unsigned int gpio = irq->irq - AR531X_GPIO_IRQ_BASE;

	/* reconfigure GPIO line as input */
	ar231x_mask_reg(AR2315_GPIO_CR, AR2315_GPIO_CR_M(gpio), AR2315_GPIO_CR_I(gpio));
	ar2315_gpio_intr_enable(irq);
	return 0;
}

static void ar2315_gpio_intr_disable(struct irq_data *irq)
{
	unsigned int gpio = irq->irq - AR531X_GPIO_IRQ_BASE;

	/* Disable interrupt */
	gpiointmask &= ~(1 << gpio);
	ar2315_set_gpiointmask(gpio, 0);
}

static void
ar2315_gpio_intr_end(struct irq_data *irq)
{
	if (!(irq_desc[irq->irq].status_use_accessors & (IRQF_DISABLED)))
		ar2315_gpio_intr_enable(irq);
}

static struct irq_chip ar2315_gpio_intr_controller = {
	.name	= "AR2315-GPIO",
	.irq_startup  = ar2315_gpio_intr_startup,
	.irq_ack      = ar2315_gpio_intr_disable,
	.irq_mask_ack = ar2315_gpio_intr_disable,
	.irq_mask     = ar2315_gpio_intr_disable,
	.irq_unmask   = ar2315_gpio_intr_enable,
	.irq_shutdown      = ar2315_gpio_intr_end,
};

static void
ar2315_misc_intr_enable(struct irq_data *irq)
{
	unsigned int imr;

	imr = ar231x_read_reg(AR2315_IMR);
	switch(irq->irq) {
	case AR531X_MISC_IRQ_SPI:
		 imr |= AR2315_ISR_SPI;
		 break;
	case AR531X_MISC_IRQ_TIMER:
	     imr |= AR2315_ISR_TIMER;
	     break;
	case AR531X_MISC_IRQ_AHB_PROC:
	     imr |= AR2315_ISR_AHB;
	     break;
	case AR531X_MISC_IRQ_GPIO:
	     imr |= AR2315_ISR_GPIO;
	     break;
	case AR531X_MISC_IRQ_UART0:
	     imr |= AR2315_ISR_UART0;
	     break;
	case AR531X_MISC_IRQ_WATCHDOG:
	     imr |= AR2315_ISR_WD;
	     break;
	default:
		break;
	}
	ar231x_write_reg(AR2315_IMR, imr);
}

static void
ar2315_misc_intr_disable(struct irq_data *irq)
{
	unsigned int imr;

	imr = ar231x_read_reg(AR2315_IMR);
	switch(irq->irq) {
	case AR531X_MISC_IRQ_SPI:
		 imr &= ~AR2315_ISR_SPI;
		 break;
	case AR531X_MISC_IRQ_TIMER:
	     imr &= ~AR2315_ISR_TIMER;
	     break;
	case AR531X_MISC_IRQ_AHB_PROC:
	     imr &= ~AR2315_ISR_AHB;
	     break;
	case AR531X_MISC_IRQ_GPIO:
	     imr &= ~AR2315_ISR_GPIO;
	     break;
	case AR531X_MISC_IRQ_UART0:
	     imr &= ~AR2315_ISR_UART0;
	     break;
	case AR531X_MISC_IRQ_WATCHDOG:
	     imr &= ~AR2315_ISR_WD;
	     break;
	default:
		break;
	}
	ar231x_write_reg(AR2315_IMR, imr);
}

static void
ar2315_misc_intr_end(struct irq_data *irq)
{
	if (!(irq_desc[irq->irq].status_use_accessors & (IRQF_DISABLED)))
		ar2315_misc_intr_enable(irq);
}


static struct irq_chip ar2315_misc_intr_controller = {
	.name	= "AR2315-MISC",
	.irq_ack      = ar2315_misc_intr_disable,
	.irq_mask_ack = ar2315_misc_intr_disable,
	.irq_mask     = ar2315_misc_intr_disable,
	.irq_unmask   = ar2315_misc_intr_enable,
	.irq_shutdown      = ar2315_misc_intr_end,
};

static irqreturn_t ar2315_ahb_proc_handler(int cpl, void *dev_id)
{
    ar231x_write_reg(AR2315_AHB_ERR0, AHB_ERROR_DET);
    ar231x_read_reg(AR2315_AHB_ERR1);

    printk(KERN_ERR "AHB fatal error\n");
    machine_restart("AHB error"); /* Catastrophic failure */

    return IRQ_HANDLED;
}

static struct irqaction ar2315_ahb_proc_interrupt  = {
	.handler	= ar2315_ahb_proc_handler,
	.flags		= IRQF_DISABLED,
	.name		= "ar2315_ahb_proc_interrupt",
};

static struct irqaction cascade  = {
	.handler	= no_action,
	.flags		= IRQF_DISABLED,
	.name		= "cascade",
};

void
ar2315_irq_init(void)
{
	int i;

	if (!is_2315())
		return;

	ar231x_irq_dispatch = ar2315_irq_dispatch;
	gpiointval = ar231x_read_reg(AR2315_GPIO_DI);
	for (i = 0; i < AR531X_MISC_IRQ_COUNT; i++) {
		int irq = AR531X_MISC_IRQ_BASE + i;
		irq_set_chip_and_handler(irq, &ar2315_misc_intr_controller,
			handle_level_irq);
	}
	for (i = 0; i < AR531X_GPIO_IRQ_COUNT; i++) {
		int irq = AR531X_GPIO_IRQ_BASE + i;
		irq_set_chip_and_handler(irq, &ar2315_gpio_intr_controller,
			handle_level_irq);
	}
	setup_irq(AR531X_MISC_IRQ_GPIO, &cascade);
	setup_irq(AR531X_MISC_IRQ_AHB_PROC, &ar2315_ahb_proc_interrupt);
	setup_irq(AR2315_IRQ_MISC_INTRS, &cascade);
}

const struct ar231x_gpiodev ar2315_gpiodev;

static u32
ar2315_gpio_get_output(void)
{
	u32 reg;
	reg = ar231x_read_reg(AR2315_GPIO_CR);
	reg &= ar2315_gpiodev.valid_mask;
	return reg;
}

static u32
ar2315_gpio_set_output(u32 mask, u32 val)
{
	u32 reg;

	reg = ar231x_read_reg(AR2315_GPIO_CR);
	reg &= ~mask;
	reg |= val;
	ar231x_write_reg(AR2315_GPIO_CR, reg);
	return reg;
}

static u32
ar2315_gpio_get(void)
{
	u32 reg;
	reg = ar231x_read_reg(AR2315_GPIO_DI);
	reg &= ar2315_gpiodev.valid_mask;
	return reg;
}

static u32
ar2315_gpio_set(u32 mask, u32 value)
{
	u32 reg;
	reg = ar231x_read_reg(AR2315_GPIO_DO);
	reg &= ~mask;
	reg |= value;
	ar231x_write_reg(AR2315_GPIO_DO, reg);
	return reg;
}

const struct ar231x_gpiodev ar2315_gpiodev = {
	.valid_mask = (1 << 22) - 1,
	.get_output = ar2315_gpio_get_output,
	.set_output = ar2315_gpio_set_output,
	.get = ar2315_gpio_get,
	.set = ar2315_gpio_set,
};

static struct ar231x_eth ar2315_eth_data = {
	.reset_base = AR2315_RESET,
	.reset_mac = AR2315_RESET_ENET0,
	.reset_phy = AR2315_RESET_EPHY0,
	.phy_base = KSEG1ADDR(AR2315_ENET0),
	.config = &ar231x_board,
};

static struct resource ar2315_spiflash_res[] = {
	{
		.name = "flash_base",
		.flags = IORESOURCE_MEM,
		.start = KSEG1ADDR(AR2315_SPI_READ),
		.end = KSEG1ADDR(AR2315_SPI_READ) + 0x1000000 - 1,
	},
	{
		.name = "flash_regs",
		.flags = IORESOURCE_MEM,
		.start = 0x11300000,
		.end = 0x11300012,
	},
};

static struct platform_device ar2315_spiflash = {
	.id = 0,
	.name = "spiflash",
	.resource = ar2315_spiflash_res,
	.num_resources = ARRAY_SIZE(ar2315_spiflash_res)
};

static struct platform_device ar2315_wdt = {
	.id = 0,
	.name = "ar2315_wdt",
};

#define SPI_FLASH_CTL      0x00
#define SPI_FLASH_OPCODE   0x04
#define SPI_FLASH_DATA     0x08

static inline u32
spiflash_read_reg(int reg)
{
	return ar231x_read_reg(AR2315_SPI + reg);
}

static inline void
spiflash_write_reg(int reg, u32 data)
{
	ar231x_write_reg(AR2315_SPI + reg, data);
}

static u32
spiflash_wait_status(void)
{
	u32 reg;

	do {
		reg = spiflash_read_reg(SPI_FLASH_CTL);
	} while (reg & SPI_CTL_BUSY);

	return reg;
}

static u8
spiflash_probe(void)
{
	u32 reg;

	reg = spiflash_wait_status();
	reg &= ~SPI_CTL_TX_RX_CNT_MASK;
	reg |= (1 << 4) | 4 | SPI_CTL_START;

	spiflash_write_reg(SPI_FLASH_OPCODE, 0xab);
	spiflash_write_reg(SPI_FLASH_CTL, reg);

	reg = spiflash_wait_status();
	reg = spiflash_read_reg(SPI_FLASH_DATA);
	reg &= 0xff;

	return (u8) reg;
}


#define STM_8MBIT_SIGNATURE     0x13
#define STM_16MBIT_SIGNATURE    0x14
#define STM_32MBIT_SIGNATURE    0x15
#define STM_64MBIT_SIGNATURE    0x16
#define STM_128MBIT_SIGNATURE   0x17

static u8 __init *
ar2315_flash_limit(void)
{
	u32 flash_size = 0;

	/* probe the flash chip size */
	switch(spiflash_probe()) {
		case STM_8MBIT_SIGNATURE:
			flash_size = 0x00100000;
			break;
		case STM_16MBIT_SIGNATURE:
			flash_size = 0x00200000;
			break;
		case STM_32MBIT_SIGNATURE:
			flash_size = 0x00400000;
			break;
		case STM_64MBIT_SIGNATURE:
			flash_size = 0x00800000;
			break;
		case STM_128MBIT_SIGNATURE:
			flash_size = 0x01000000;
			break;
	}

	ar2315_spiflash_res[0].end = ar2315_spiflash_res[0].start +
		flash_size - 1;
	return (u8 *) ar2315_spiflash_res[0].end + 1;
}

#ifdef CONFIG_LEDS_GPIO
static struct gpio_led ar2315_leds[6];
static struct gpio_led_platform_data ar2315_led_data = {
	.leds = (void *) ar2315_leds,
};

static struct platform_device ar2315_gpio_leds = {
	.name = "leds-gpio",
	.id = -1,
	.dev = {
		.platform_data = (void *) &ar2315_led_data,
	}
};

static void __init
ar2315_init_gpio(void)
{
	static char led_names[6][6];
	int i, led = 0;

	ar2315_led_data.num_leds = 0;
	for(i = 1; i < 8; i++)
	{
		if((i == AR2315_RESET_GPIO) ||
		   (i == ar231x_board.config->resetConfigGpio))
			continue;

		if(i == ar231x_board.config->sysLedGpio)
			strcpy(led_names[led], "wlan");
		else
			sprintf(led_names[led], "gpio%d", i);

		ar2315_leds[led].name = led_names[led];
		ar2315_leds[led].gpio = i;
		ar2315_leds[led].active_low = 0;
		led++;
	}
	ar2315_led_data.num_leds = led;
	platform_device_register(&ar2315_gpio_leds);
}
#else
static inline void ar2315_init_gpio(void)
{
}
#endif

int __init
ar2315_init_devices(void)
{
	if (!is_2315())
		return 0;

	/* Find board configuration */
	ar231x_find_config(ar2315_flash_limit());
	ar2315_eth_data.macaddr = ar231x_board.config->enet0_mac;

	ar2315_init_gpio();
	platform_device_register(&ar2315_wdt);
	platform_device_register(&ar2315_spiflash);
	ar231x_add_ethernet(0, KSEG1ADDR(AR2315_ENET0), AR2315_IRQ_ENET0_INTRS,
		&ar2315_eth_data);
	ar231x_add_wmac(0, AR2315_WLAN0, AR2315_IRQ_WLAN0_INTRS);

	return 0;
}

static void
ar2315_restart(char *command)
{
	void (*mips_reset_vec)(void) = (void *) 0xbfc00000;

	local_irq_disable();

	/* try reset the system via reset control */
	ar231x_write_reg(AR2315_COLD_RESET,AR2317_RESET_SYSTEM);

	/* Cold reset does not work on the AR2315/6, use the GPIO reset bits a workaround.
	 * give it some time to attempt a gpio based hardware reset
	 * (atheros reference design workaround) */
	gpio_direction_output(AR2315_RESET_GPIO, 0);
	mdelay(100);

	/* now do GPIO 0 reset, known to be used on Seano devices */
	gpio_direction_output(0, 0);
	mdelay(100);


	/* Some boards (e.g. Senao EOC-2610) don't implement the reset logic
	 * workaround. Attempt to jump to the mips reset location -
	 * the boot loader itself might be able to recover the system */
	mips_reset_vec();
}


/*
 * This table is indexed by bits 5..4 of the CLOCKCTL1 register
 * to determine the predevisor value.
 */
static int __initdata CLOCKCTL1_PREDIVIDE_TABLE[4] = { 1, 2, 4, 5 };
static int __initdata PLLC_DIVIDE_TABLE[5] = { 2, 3, 4, 6, 3 };

static unsigned int __init
ar2315_sys_clk(unsigned int clockCtl)
{
    unsigned int pllcCtrl,cpuDiv;
    unsigned int pllcOut,refdiv,fdiv,divby2;
	unsigned int clkDiv;

    pllcCtrl = ar231x_read_reg(AR2315_PLLC_CTL);
    refdiv = (pllcCtrl & PLLC_REF_DIV_M) >> PLLC_REF_DIV_S;
    refdiv = CLOCKCTL1_PREDIVIDE_TABLE[refdiv];
    fdiv = (pllcCtrl & PLLC_FDBACK_DIV_M) >> PLLC_FDBACK_DIV_S;
    divby2 = (pllcCtrl & PLLC_ADD_FDBACK_DIV_M) >> PLLC_ADD_FDBACK_DIV_S;
    divby2 += 1;
    pllcOut = (40000000/refdiv)*(2*divby2)*fdiv;


    /* clkm input selected */
	switch(clockCtl & CPUCLK_CLK_SEL_M) {
		case 0:
		case 1:
			clkDiv = PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKM_DIV_M) >> PLLC_CLKM_DIV_S];
			break;
		case 2:
			clkDiv = PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKC_DIV_M) >> PLLC_CLKC_DIV_S];
			break;
		default:
			pllcOut = 40000000;
			clkDiv = 1;
			break;
	}
	cpuDiv = (clockCtl & CPUCLK_CLK_DIV_M) >> CPUCLK_CLK_DIV_S;
	cpuDiv = cpuDiv * 2 ?: 1;
	return (pllcOut/(clkDiv * cpuDiv));
}

unsigned int
ar2315_cpu_frequency(void)
{
    return ar2315_sys_clk(ar231x_read_reg(AR2315_CPUCLK));
}

static inline unsigned int
ar2315_apb_frequency(void)
{
    return ar2315_sys_clk(ar231x_read_reg(AR2315_AMBACLK));
}
extern unsigned int ath_cpufreq;
void __init
ar2315_time_init(void)
{
	if (!is_2315())
		return;
	ath_cpufreq = ar2315_cpu_frequency();
	mips_hpt_frequency = ath_cpufreq / 2;
}

void __init
ar2315_prom_init(void)
{
	u32 memsize, memcfg, devid;

	if (!is_2315())
		return;

	memcfg = ar231x_read_reg(AR2315_MEM_CFG);
	memsize   = 1 + ((memcfg & SDRAM_DATA_WIDTH_M) >> SDRAM_DATA_WIDTH_S);
	memsize <<= 1 + ((memcfg & SDRAM_COL_WIDTH_M) >> SDRAM_COL_WIDTH_S);
	memsize <<= 1 + ((memcfg & SDRAM_ROW_WIDTH_M) >> SDRAM_ROW_WIDTH_S);
	memsize <<= 3;
	add_memory_region(0, memsize, BOOT_MEM_RAM);

	/* Detect the hardware based on the device ID */
	devid = ar231x_read_reg(AR2315_SREV) & AR2315_REV_CHIP;
	switch(devid) {
		case 0x90:
		case 0x91:
			ar231x_devtype = DEV_TYPE_AR2317;
			break;
		default:
			ar231x_devtype = DEV_TYPE_AR2315;
			break;
	}
	ar231x_gpiodev = &ar2315_gpiodev;
	ar231x_board.devid = devid;
}

void __init
ar2315_plat_setup(void)
{
	u32 config;

	if (!is_2315())
		return;

	/* Clear any lingering AHB errors */
	config = read_c0_config();
	write_c0_config(config & ~0x3);
	ar231x_write_reg(AR2315_AHB_ERR0,AHB_ERROR_DET);
	ar231x_read_reg(AR2315_AHB_ERR1);
	ar231x_write_reg(AR2315_WDC, AR2315_WDC_IGNORE_EXPIRATION);

	_machine_restart = ar2315_restart;
	ar231x_serial_setup(KSEG1ADDR(AR2315_UART0), ar2315_apb_frequency());
}
