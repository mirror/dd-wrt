/*
 *
 * BRIEF MODULE DESCRIPTION
 *	Definitions for IDT RC32355 CPU.
 *
 * Copyright 2002 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *         	stevel@mvista.com or source@mvista.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 ****************************************************************************
 * P. Sadik Oct 07,2003
 *
 * Started revision history
 * Modified to make IDT_BUS_FREQ a kernel configuration parameter.
 ****************************************************************************
 * P. Sadik   Oct 10, 2003
 *
 * Removed IDT_BUS_FREQ, since this parameter is no longer required. Instead
 * idt_cpu_freq is used everywhere
 ****************************************************************************
 * P. Sadik   Oct 20, 2003
 *
 * Removed RC32438_BASE_BAUD.
 ****************************************************************************
 */
 
#ifndef _RC32438_H_
#define _RC32438_H_
#include <linux/config.h>
#include <linux/delay.h>
#include <asm/io.h>
#define RC32438_REG_BASE   0x18000000

#define interrupt ((volatile INT_t ) INT0_VirtualAddress)
#define timer     ((volatile TIM_t)  TIM0_VirtualAddress)
#define gpio	  ((volatile GPIO_t) GPIO0_VirtualAddress)

#define IDT_CLOCK_MULT 2
#define MIPS_CPU_TIMER_IRQ 7
/* Interrupt Controller */
#define IC_GROUP0_PEND     (RC32438_REG_BASE + 0x38000)
#define IC_GROUP0_MASK     (RC32438_REG_BASE + 0x38008)
#define IC_GROUP_OFFSET    0x0C

#define NUM_INTR_GROUPS    5
/* 16550 UARTs */

#define GROUP0_IRQ_BASE 8		/* GRP2 IRQ numbers start here */
#define GROUP1_IRQ_BASE (GROUP0_IRQ_BASE + 32) /* GRP3 IRQ numbers start here */
#define GROUP2_IRQ_BASE (GROUP1_IRQ_BASE + 32) /* GRP4 IRQ numbers start here */
#define GROUP3_IRQ_BASE (GROUP2_IRQ_BASE + 32)	/* GRP5 IRQ numbers start here */
#define GROUP4_IRQ_BASE (GROUP3_IRQ_BASE + 32)

#ifdef __MIPSEB__
#define RC32438_UART0_BASE (RC32438_REG_BASE + 0x50003)
#define RC32438_UART1_BASE (RC32438_REG_BASE + 0x50023)
#else
#define RC32438_UART0_BASE (RC32438_REG_BASE + 0x50000)
#define RC32438_UART1_BASE (RC32438_REG_BASE + 0x50020)
#endif

#define RC32438_UART0_IRQ  GROUP3_IRQ_BASE + 0
#define RC32438_UART1_IRQ  GROUP3_IRQ_BASE + 3

#define local_readl(addr) __raw_readl(addr)
#define local_writel(l,addr) __raw_writel(l,addr)

/* cpu pipeline flush */
static inline void rc32438_sync(void)
{
        __asm__ volatile ("sync");
}

static inline void rc32438_sync_udelay(int us)
{
        __asm__ volatile ("sync");
        udelay(us);
}

static inline void rc32438_sync_delay(int ms)
{
        __asm__ volatile ("sync");
        mdelay(ms);
}
static inline u8 rc32438_readb(unsigned long pa)
{
	return *((volatile u8 *)KSEG1ADDR(pa));
}
static inline u16 rc32438_readw(unsigned long pa)
{
	return *((volatile u16 *)KSEG1ADDR(pa));
}
static inline u32 rc32438_readl(unsigned long pa)
{
	return *((volatile u32 *)KSEG1ADDR(pa));
}
static inline void rc32438_writeb(u8 val, unsigned long pa)
{
	*((volatile u8 *)KSEG1ADDR(pa)) = val;
}
static inline void rc32438_writew(u16 val, unsigned long pa)
{
	*((volatile u16 *)KSEG1ADDR(pa)) = val;
}
static inline void rc32438_writel(u32 val, unsigned long pa)
{
	*((volatile u32 *)KSEG1ADDR(pa)) = val;
}

/*
 * C access to CLZ and CLO instructions
 * (count leading zeroes/ones).
 */
static inline int rc32438_clz(unsigned long val)
{
	int ret;
        __asm__ volatile (
		".set\tnoreorder\n\t"
		".set\tnoat\n\t"
		".set\tmips32\n\t"
		"clz\t%0,%1\n\t"
                ".set\tmips0\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                : "=r" (ret)
		: "r" (val));

	return ret;
}
static inline int rc32438_clo(unsigned long val)
{
	int ret;
        __asm__ volatile (
		".set\tnoreorder\n\t"
		".set\tnoat\n\t"
		".set\tmips32\n\t"
		"clo\t%0,%1\n\t"
                ".set\tmips0\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                : "=r" (ret)
		: "r" (val));

	return ret;
}
#endif /* _RC32438_H_ */












