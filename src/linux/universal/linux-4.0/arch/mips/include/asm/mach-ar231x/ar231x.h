#ifndef __AR531X_H
#define __AR531X_H

#define AR531X_MISC_IRQ_BASE		0x20
#define AR531X_GPIO_IRQ_BASE		0x30

/* Software's idea of interrupts handled by "CPU Interrupt Controller" */
#define AR531X_IRQ_NONE		MIPS_CPU_IRQ_BASE+0
#define AR531X_IRQ_CPU_CLOCK	MIPS_CPU_IRQ_BASE+7 /* C0_CAUSE: 0x8000 */

/* Miscellaneous interrupts, which share IP6 */
#define AR531X_MISC_IRQ_NONE		AR531X_MISC_IRQ_BASE+0
#define AR531X_MISC_IRQ_TIMER		AR531X_MISC_IRQ_BASE+1
#define AR531X_MISC_IRQ_AHB_PROC	AR531X_MISC_IRQ_BASE+2
#define AR531X_MISC_IRQ_AHB_DMA		AR531X_MISC_IRQ_BASE+3
#define AR531X_MISC_IRQ_GPIO		AR531X_MISC_IRQ_BASE+4
#define AR531X_MISC_IRQ_UART0		AR531X_MISC_IRQ_BASE+5
#define AR531X_MISC_IRQ_UART0_DMA	AR531X_MISC_IRQ_BASE+6
#define AR531X_MISC_IRQ_WATCHDOG	AR531X_MISC_IRQ_BASE+7
#define AR531X_MISC_IRQ_LOCAL		AR531X_MISC_IRQ_BASE+8
#define AR531X_MISC_IRQ_SPI 		AR531X_MISC_IRQ_BASE+9
#define AR531X_MISC_IRQ_COUNT		10

/* GPIO Interrupts [0..7], share AR531X_MISC_IRQ_GPIO */
#define AR531X_GPIO_IRQ_NONE            AR531X_GPIO_IRQ_BASE+0
#define AR531X_GPIO_IRQ(n)              AR531X_GPIO_IRQ_BASE+n
#define AR531X_GPIO_IRQ_COUNT           22

static inline u32
ar231x_read_reg(u32 reg)
{
	return __raw_readl((u32 *) KSEG1ADDR(reg));
}

static inline void
ar231x_write_reg(u32 reg, u32 val)
{
	__raw_writel(val, (u32 *) KSEG1ADDR(reg));
}

static inline u32
ar231x_mask_reg(u32 reg, u32 mask, u32 val)
{
	u32 ret;

	ret = ar231x_read_reg(reg);
	ret &= ~mask;
	ret |= val;
	ar231x_write_reg(reg, ret);

	return ret;
}

#endif
