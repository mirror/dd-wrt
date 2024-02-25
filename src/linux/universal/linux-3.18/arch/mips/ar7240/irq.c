/*
 *  Atheros AR71xx SoC specific interrupt handling
 *
 *  Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros 2.6.15 BSP
 *  Parts of this file are based on Atheros 2.6.31 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>

#include <asm/mach-ar71xx/ar71xx.h>

static void ar71xx_gpio_irq_dispatch(void)
{
	void __iomem *base = ar71xx_gpio_base;
	u32 pending;

	pending = __raw_readl(base + AR71XX_GPIO_REG_INT_PENDING) & __raw_readl(base + AR71XX_GPIO_REG_INT_ENABLE);

	if (pending)
		do_IRQ(AR71XX_GPIO_IRQ_BASE + fls(pending) - 1);
	else
		spurious_interrupt();
}

static void ar71xx_gpio_irq_unmask(struct irq_data *d)
{
	unsigned int irq = d->irq - AR71XX_GPIO_IRQ_BASE;
	void __iomem *base = ar71xx_gpio_base;
	u32 t;

	t = __raw_readl(base + AR71XX_GPIO_REG_INT_ENABLE);
	__raw_writel(t | (1 << irq), base + AR71XX_GPIO_REG_INT_ENABLE);

	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_INT_ENABLE);
}

static void ar71xx_gpio_irq_mask(struct irq_data *d)
{
	unsigned int irq = d->irq - AR71XX_GPIO_IRQ_BASE;
	void __iomem *base = ar71xx_gpio_base;
	u32 t;

	t = __raw_readl(base + AR71XX_GPIO_REG_INT_ENABLE);
	__raw_writel(t & ~(1 << irq), base + AR71XX_GPIO_REG_INT_ENABLE);

	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_INT_ENABLE);
}

static struct irq_chip ar71xx_gpio_irq_chip = {
	.name = "AR71XX GPIO",
	.irq_unmask = ar71xx_gpio_irq_unmask,
	.irq_mask = ar71xx_gpio_irq_mask,
	.irq_mask_ack = ar71xx_gpio_irq_mask,
};

static struct irqaction ar71xx_gpio_irqaction = {
	.handler = no_action,
	.name = "cascade [AR71XX GPIO]",
};

#define GPIO_INT_ALL	0xffff

static void __init ar71xx_gpio_irq_init(void)
{
	void __iomem *base = ar71xx_gpio_base;
	int i;

	__raw_writel(0, base + AR71XX_GPIO_REG_INT_ENABLE);
	__raw_writel(0, base + AR71XX_GPIO_REG_INT_PENDING);

	/* setup type of all GPIO interrupts to level sensitive */
	__raw_writel(GPIO_INT_ALL, base + AR71XX_GPIO_REG_INT_TYPE);

	/* setup polarity of all GPIO interrupts to active high */
	__raw_writel(GPIO_INT_ALL, base + AR71XX_GPIO_REG_INT_POLARITY);

	for (i = AR71XX_GPIO_IRQ_BASE; i < AR71XX_GPIO_IRQ_BASE + AR71XX_GPIO_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ar71xx_gpio_irq_chip, handle_level_irq);

	setup_irq(AR71XX_MISC_IRQ_GPIO, &ar71xx_gpio_irqaction);
}

static void ar71xx_misc_irq_dispatch(void)
{
	u32 pending;

	pending = ar71xx_reset_rr(AR71XX_RESET_REG_MISC_INT_STATUS)
	    & ar71xx_reset_rr(AR71XX_RESET_REG_MISC_INT_ENABLE);

	if (pending & MISC_INT_UART)
		do_IRQ(AR71XX_MISC_IRQ_UART);

	else if (pending & MISC_INT_DMA)
		do_IRQ(AR71XX_MISC_IRQ_DMA);

	else if (pending & MISC_INT_PERFC)
		do_IRQ(AR71XX_MISC_IRQ_PERFC);

	else if (pending & MISC_INT_TIMER)
		do_IRQ(AR71XX_MISC_IRQ_TIMER);

	else if (pending & MISC_INT_OHCI)
		do_IRQ(AR71XX_MISC_IRQ_OHCI);

	else if (pending & MISC_INT_ERROR)
		do_IRQ(AR71XX_MISC_IRQ_ERROR);

	else if (pending & MISC_INT_GPIO)
		ar71xx_gpio_irq_dispatch();

	else if (pending & MISC_INT_WDOG)
		do_IRQ(AR71XX_MISC_IRQ_WDOG);

	else if (pending & MISC_INT_TIMER2)
		do_IRQ(AR71XX_MISC_IRQ_TIMER2);

	else if (pending & MISC_INT_TIMER3)
		do_IRQ(AR71XX_MISC_IRQ_TIMER3);

	else if (pending & MISC_INT_TIMER4)
		do_IRQ(AR71XX_MISC_IRQ_TIMER4);

	else if (pending & MISC_INT_DDR_PERF)
		do_IRQ(AR71XX_MISC_IRQ_DDR_PERF);

	else if (pending & MISC_INT_ENET_LINK)
		do_IRQ(AR71XX_MISC_IRQ_ENET_LINK);

	else
		spurious_interrupt();
}

static void ar71xx_misc_irq_unmask(struct irq_data *d)
{
	unsigned int irq = d->irq - AR71XX_MISC_IRQ_BASE;
	void __iomem *base = ar71xx_reset_base;
	u32 t;

	t = __raw_readl(base + AR71XX_RESET_REG_MISC_INT_ENABLE);
	__raw_writel(t | (1 << irq), base + AR71XX_RESET_REG_MISC_INT_ENABLE);

	/* flush write */
	(void)__raw_readl(base + AR71XX_RESET_REG_MISC_INT_ENABLE);
}

static void ar71xx_misc_irq_mask(struct irq_data *d)
{
	unsigned int irq = d->irq - AR71XX_MISC_IRQ_BASE;
	void __iomem *base = ar71xx_reset_base;
	u32 t;

	t = __raw_readl(base + AR71XX_RESET_REG_MISC_INT_ENABLE);
	__raw_writel(t & ~(1 << irq), base + AR71XX_RESET_REG_MISC_INT_ENABLE);

	/* flush write */
	(void)__raw_readl(base + AR71XX_RESET_REG_MISC_INT_ENABLE);
}

static void ar724x_misc_irq_ack(struct irq_data *d)
{
	unsigned int irq = d->irq - AR71XX_MISC_IRQ_BASE;
	void __iomem *base = ar71xx_reset_base;
	u32 t;

	t = __raw_readl(base + AR71XX_RESET_REG_MISC_INT_STATUS);
	__raw_writel(t & ~(1 << irq), base + AR71XX_RESET_REG_MISC_INT_STATUS);

	/* flush write */
	(void)__raw_readl(base + AR71XX_RESET_REG_MISC_INT_STATUS);
}

static struct irq_chip ar71xx_misc_irq_chip = {
	.name = "AR71XX MISC",
	.irq_unmask = ar71xx_misc_irq_unmask,
	.irq_mask = ar71xx_misc_irq_mask,
};

static struct irqaction ar71xx_misc_irqaction = {
	.handler = no_action,
	.name = "cascade [AR71XX MISC]",
};

static void __init ar71xx_misc_irq_init(void)
{
	void __iomem *base = ar71xx_reset_base;
	int i;

	__raw_writel(0, base + AR71XX_RESET_REG_MISC_INT_ENABLE);
	__raw_writel(0, base + AR71XX_RESET_REG_MISC_INT_STATUS);

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7240:
	case AR71XX_SOC_AR7241:
	case AR71XX_SOC_AR7242:
	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
	case AR71XX_SOC_QCA9533:
	case AR71XX_SOC_QCA9556:
	case AR71XX_SOC_QCA9558:
	case AR71XX_SOC_QCA9563:
	case AR71XX_SOC_TP9343:
	case AR71XX_SOC_QCN550X:
		ar71xx_misc_irq_chip.irq_ack = ar724x_misc_irq_ack;
		break;
	default:
		ar71xx_misc_irq_chip.irq_mask_ack = ar71xx_misc_irq_mask;
		break;
	}

	for (i = AR71XX_MISC_IRQ_BASE; i < AR71XX_MISC_IRQ_BASE + AR71XX_MISC_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ar71xx_misc_irq_chip, handle_level_irq);

	setup_irq(AR71XX_CPU_IRQ_MISC, &ar71xx_misc_irqaction);
}

static void ar934x_ip2_irq_dispatch(unsigned int irq, struct irq_desc *desc)
{
	u32 status;

	disable_irq_nosync(irq);

	status = ar71xx_reset_rr(AR934X_RESET_REG_PCIE_WMAC_INT_STATUS);
	status &= AR934X_PCIE_WMAC_INT_PCIE_ALL | AR934X_PCIE_WMAC_INT_WMAC_ALL;

	if (status == 0) {
		spurious_interrupt();
		goto enable;
	}

	if (status & AR934X_PCIE_WMAC_INT_PCIE_ALL) {
		ar71xx_ddr_flush(AR934X_DDR_REG_FLUSH_PCIE);
		generic_handle_irq(AR934X_IP2_IRQ_PCIE);
	}
	
	if (status & AR934X_PCIE_WMAC_INT_WMAC_ALL) {
		ar71xx_ddr_flush(AR934X_DDR_REG_FLUSH_WMAC);
		generic_handle_irq(AR934X_IP2_IRQ_WMAC);
	}
	
enable:
	enable_irq(irq);


}


static void qca955x_ip2_irq_dispatch(unsigned int irq, struct irq_desc *desc)
{
	u32 status;

	disable_irq_nosync(irq);

	status = ar71xx_reset_rr(QCA955X_RESET_REG_EXT_INT_STATUS);
	status &= QCA955X_EXT_INT_PCIE_RC1_ALL | QCA955X_EXT_INT_WMAC_ALL;

	if (status == 0) {
		spurious_interrupt();
		goto enable;
	}

	if (status & QCA955X_EXT_INT_PCIE_RC1_ALL) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP2_IRQ(0));
	}

	if (status & QCA955X_EXT_INT_WMAC_ALL) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP2_IRQ(1));
	}
enable:
	enable_irq(irq);
}

static void qca955x_ip3_irq_dispatch(unsigned int irq, struct irq_desc *desc)
{
	u32 status;

	disable_irq_nosync(irq);

	status = ar71xx_reset_rr(QCA955X_RESET_REG_EXT_INT_STATUS);
	status &= QCA955X_EXT_INT_PCIE_RC2_ALL |
		  QCA955X_EXT_INT_USB1 |
		  QCA955X_EXT_INT_USB2;

	if (status == 0) {
		spurious_interrupt();
		goto enable;
	}

	if (status & QCA955X_EXT_INT_USB1) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP3_IRQ(0));
	}

	if (status & QCA955X_EXT_INT_USB2) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP3_IRQ(1));
	}

	if (status & QCA955X_EXT_INT_PCIE_RC2_ALL) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP3_IRQ(2));
	}
enable:
	enable_irq(irq);

}

static struct irq_chip ip2_chip;
static struct irq_chip ip3_chip;

static void ar934x_ip2_irq_init(void)
{
	int i;
	for (i = AR934X_IP2_IRQ_BASE; i < AR934X_IP2_IRQ_BASE + AR934X_IP2_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ip2_chip, handle_level_irq);

	irq_set_chained_handler(AR71XX_CPU_IRQ_IP2, ar934x_ip2_irq_dispatch);
}


static void qca955x_enable_timer_cb(void) {
	u32 misc;

	misc = ar71xx_reset_rr(AR71XX_RESET_REG_MISC_INT_ENABLE);
	misc |= MISC_INT_MIPS_SI_TIMERINT_MASK;
	ar71xx_reset_wr(AR71XX_RESET_REG_MISC_INT_ENABLE, misc);
}


static void qca955x_irq_init(void)
{
	int i;

	for (i = AR934X_IP2_IRQ_BASE;
	     i < AR934X_IP2_IRQ_BASE + AR934X_IP2_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ip2_chip,
					 handle_level_irq);

	irq_set_chained_handler(AR71XX_CPU_IRQ_IP2, qca955x_ip2_irq_dispatch);

	for (i = AR934X_IP3_IRQ_BASE;
	     i < AR934X_IP3_IRQ_BASE + AR934X_IP3_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ip3_chip,
					 handle_level_irq);
	irq_set_chained_handler(AR71XX_CPU_IRQ_IP3, qca955x_ip3_irq_dispatch);

	if (ar71xx_soc == AR71XX_SOC_QCA9556) {
		/* QCA956x timer init workaround has to be applied right before setting
		 * up the clock. Else, there will be no jiffies */
		late_time_init = &qca955x_enable_timer_cb;
	}
	

}


static void qca956x_ip2_irq_dispatch(unsigned int irq, struct irq_desc *desc)
{
	u32 status;

	disable_irq_nosync(irq);

	status = ar71xx_reset_rr(QCA956X_RESET_REG_EXT_INT_STATUS);
	status &= QCA956X_EXT_INT_PCIE_RC1_ALL | QCA956X_EXT_INT_WMAC_ALL;

	if (status == 0) {
		spurious_interrupt();
		goto enable;
	}

	if (status & QCA956X_EXT_INT_PCIE_RC1_ALL) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP2_IRQ(0));
	}

	if (status & QCA956X_EXT_INT_WMAC_ALL) {
		/* TODO: flsuh DDR? */
		generic_handle_irq(AR934X_IP2_IRQ(1));
	}
enable:
	enable_irq(irq);
}

static void qca956x_ip3_irq_dispatch(unsigned int irq, struct irq_desc *desc)
{
	u32 status;

	disable_irq_nosync(irq);

	status = ar71xx_reset_rr(QCA956X_RESET_REG_EXT_INT_STATUS);
	status &= QCA956X_EXT_INT_PCIE_RC2_ALL |
		  QCA956X_EXT_INT_USB1 | QCA956X_EXT_INT_USB2;

	if (status == 0) {
		spurious_interrupt();
		goto enable;
	}

	if (status & QCA956X_EXT_INT_USB1) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP3_IRQ(0));
	}

	if (status & QCA956X_EXT_INT_USB2) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP3_IRQ(1));
	}

	if (status & QCA956X_EXT_INT_PCIE_RC2_ALL) {
		/* TODO: flush DDR? */
		generic_handle_irq(AR934X_IP3_IRQ(2));
	}
enable:
	enable_irq(irq);
}

static void qca956x_enable_timer_cb(void) {
	u32 misc;

	misc = ar71xx_reset_rr(AR71XX_RESET_REG_MISC_INT_ENABLE);
	misc |= MISC_INT_MIPS_SI_TIMERINT_MASK;
	ar71xx_reset_wr(AR71XX_RESET_REG_MISC_INT_ENABLE, misc);
}

static void qca956x_irq_init(void)
{
	int i;

	for (i = AR934X_IP2_IRQ_BASE;
	     i < AR934X_IP2_IRQ_BASE + AR934X_IP2_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ip2_chip,
					 handle_level_irq);

	irq_set_chained_handler(AR71XX_CPU_IRQ_IP2, qca956x_ip2_irq_dispatch);

	for (i = AR934X_IP3_IRQ_BASE;
	     i < AR934X_IP3_IRQ_BASE + AR934X_IP3_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ip3_chip,
					 handle_level_irq);

	irq_set_chained_handler(AR71XX_CPU_IRQ_IP3, qca956x_ip3_irq_dispatch);

	/* QCA956x timer init workaround has to be applied right before setting
	 * up the clock. Else, there will be no jiffies */
	late_time_init = &qca956x_enable_timer_cb;
}

static void qca953x_ip2_irq_dispatch(unsigned int irq,struct irq_desc *desc)
{
	u32 status;

	status = ar71xx_reset_rr(QCA953X_RESET_REG_PCIE_WMAC_INT_STATUS);
	status &= QCA953X_PCIE_WMAC_INT_PCIE_ALL | QCA953X_PCIE_WMAC_INT_WMAC_ALL;

	if (status == 0) {
		spurious_interrupt();
		goto enable;
	}

	if (status & QCA953X_PCIE_WMAC_INT_PCIE_ALL) {
		ar71xx_ddr_flush(QCA953X_DDR_REG_FLUSH_PCIE);
		generic_handle_irq(AR934X_IP2_IRQ(0));
	} else if (status & QCA953X_PCIE_WMAC_INT_WMAC_ALL) {
		ar71xx_ddr_flush(QCA953X_DDR_REG_FLUSH_WMAC);
		generic_handle_irq(AR934X_IP2_IRQ(1));
	} else {
		spurious_interrupt();
	}

enable:
	enable_irq(irq);
}

static void qca953x_irq_init(void)
{
	int i;

	for (i = AR934X_IP2_IRQ_BASE;
	     i < AR934X_IP2_IRQ_BASE + AR934X_IP2_IRQ_COUNT; i++)
		irq_set_chip_and_handler(i, &ip2_chip, handle_level_irq);

	irq_set_chained_handler(AR71XX_CPU_IRQ_IP2, qca953x_ip2_irq_dispatch);
}



/*
 * The IP2/IP3 lines are tied to a PCI/WMAC/USB device. Drivers for
 * these devices typically allocate coherent DMA memory, however the
 * DMA controller may still have some unsynchronized data in the FIFO.
 * Issue a flush in the handlers to ensure that the driver sees
 * the update.
 */
static void ar71xx_ip2_handler(void)
{
	ar71xx_ddr_flush(AR71XX_DDR_REG_FLUSH_PCI);
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void ar724x_ip2_handler(void)
{
	ar71xx_ddr_flush(AR724X_DDR_REG_FLUSH_PCIE);
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void ar913x_ip2_handler(void)
{
	ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_WMAC);
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void ar933x_ip2_handler(void)
{
	ar71xx_ddr_flush(AR933X_DDR_REG_FLUSH_WMAC);
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void ar934x_ip2_handler(void)
{
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void qca9533_ip2_handler(void)
{
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void ar71xx_ip3_handler(void)
{
	ar71xx_ddr_flush(AR71XX_DDR_REG_FLUSH_USB);
	do_IRQ(AR71XX_CPU_IRQ_USB);
}

static void ar724x_ip3_handler(void)
{
	ar71xx_ddr_flush(AR724X_DDR_REG_FLUSH_USB);
	do_IRQ(AR71XX_CPU_IRQ_USB);
}

static void ar913x_ip3_handler(void)
{
	ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_USB);
	do_IRQ(AR71XX_CPU_IRQ_USB);
}

static void ar933x_ip3_handler(void)
{
	ar71xx_ddr_flush(AR933X_DDR_REG_FLUSH_USB);
	do_IRQ(AR71XX_CPU_IRQ_USB);
}

static void ar934x_ip3_handler(void)
{
	ar71xx_ddr_flush(AR934X_DDR_REG_FLUSH_USB);
	do_IRQ(AR71XX_CPU_IRQ_USB);
}

static void qca9533_ip3_handler(void)
{
	ar71xx_ddr_flush(QCA953X_DDR_REG_FLUSH_USB);
	do_IRQ(AR71XX_CPU_IRQ_USB);
}


static void ar71xx_default_ip2_handler(void)
{
	do_IRQ(AR71XX_CPU_IRQ_IP2);
}

static void ar71xx_default_ip3_handler(void)
{
	do_IRQ(AR71XX_CPU_IRQ_IP3);
}

static void (*ip2_handler) (void);
static void (*ip3_handler) (void);

asmlinkage void plat_irq_dispatch(void)
{
	unsigned long pending;

	pending = read_c0_status() & read_c0_cause() & ST0_IM;

	if (pending & STATUSF_IP7)
		do_IRQ(AR71XX_CPU_IRQ_TIMER);

	else if (pending & STATUSF_IP2)
		ip2_handler();

	else if (pending & STATUSF_IP4)
		do_IRQ(AR71XX_CPU_IRQ_GE0);

	else if (pending & STATUSF_IP5)
		do_IRQ(AR71XX_CPU_IRQ_GE1);

	else if (pending & STATUSF_IP3)
		ip3_handler();

	else if (pending & STATUSF_IP6)
		ar71xx_misc_irq_dispatch();

	else
		spurious_interrupt();
}

static void ath79_ip2_disable(struct irq_data *data)
{
	disable_irq(AR71XX_CPU_IRQ_IP2);
}

static void ath79_ip2_enable(struct irq_data *data)
{
	enable_irq(AR71XX_CPU_IRQ_IP2);
}

static void ath79_ip3_disable(struct irq_data *data)
{
	disable_irq(AR71XX_CPU_IRQ_IP3);
}

static void ath79_ip3_enable(struct irq_data *data)
{
	enable_irq(AR71XX_CPU_IRQ_IP3);
}

void __init arch_init_irq(void)
{
	ip2_chip = dummy_irq_chip;
	ip3_chip = dummy_irq_chip;
	ip2_chip.irq_disable = ath79_ip2_disable;
	ip2_chip.irq_enable = ath79_ip2_enable;
	ip3_chip.irq_disable = ath79_ip3_disable;
	ip3_chip.irq_enable = ath79_ip3_enable;

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7130:
	case AR71XX_SOC_AR7141:
	case AR71XX_SOC_AR7161:
		ip2_handler = ar71xx_ip2_handler;
		ip3_handler = ar71xx_ip3_handler;
		break;

	case AR71XX_SOC_AR7240:
	case AR71XX_SOC_AR7241:
	case AR71XX_SOC_AR7242:
		ip2_handler = ar724x_ip2_handler;
		ip3_handler = ar724x_ip3_handler;
		break;

	case AR71XX_SOC_AR9130:
	case AR71XX_SOC_AR9132:
		ip2_handler = ar913x_ip2_handler;
		ip3_handler = ar913x_ip3_handler;
		break;

	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		ip2_handler = ar933x_ip2_handler;
		ip3_handler = ar933x_ip3_handler;
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		ip2_handler = ar934x_ip2_handler;
		ip3_handler = ar934x_ip3_handler;
		break;
	case AR71XX_SOC_QCA9533:
		ip2_handler = qca9533_ip2_handler;
		ip3_handler = qca9533_ip3_handler;
		break;
	case AR71XX_SOC_QCA9556:
	case AR71XX_SOC_QCA9558:
	case AR71XX_SOC_QCA9563:
	case AR71XX_SOC_QCN550X:
	case AR71XX_SOC_TP9343:
		ip2_handler = ar71xx_default_ip2_handler;
		ip3_handler = ar71xx_default_ip3_handler;
		break;

	default:
		BUG();
	}

	mips_cpu_irq_init();

	ar71xx_misc_irq_init();

	if (ar71xx_soc == AR71XX_SOC_AR9341 || ar71xx_soc == AR71XX_SOC_AR9342 || ar71xx_soc == AR71XX_SOC_AR9344)
		ar934x_ip2_irq_init();
	else if (ar71xx_soc == AR71XX_SOC_QCA9533)
		qca953x_irq_init();
	else if (ar71xx_soc == AR71XX_SOC_QCA9556 || ar71xx_soc == AR71XX_SOC_QCA9558)
		qca955x_irq_init();
	else if (ar71xx_soc == AR71XX_SOC_QCA9563 || ar71xx_soc == AR71XX_SOC_TP9343 || ar71xx_soc == AR71XX_SOC_QCN550X)
		qca956x_irq_init();

	cp0_perfcount_irq = AR71XX_MISC_IRQ_PERFC;

	ar71xx_gpio_irq_init();
}
