/*
 *
 * BRIEF MODULE DESCRIPTION
 *	Alchemy Db1x00 board setup.
 *
 * Copyright 2000 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *         	ppopov@mvista.com or source@mvista.com
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
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/mc146818rtc.h>
#include <linux/delay.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/keyboard.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>
#include <asm/pgtable.h>
#include <asm/au1000.h>
#include <asm/db1x00.h>

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX) && defined(CONFIG_MIPS_DB1550)
#include <asm/au1xxx_dbdma.h>
extern struct ide_ops *ide_ops;
extern struct ide_ops au1xxx_ide_ops;
extern u32 au1xxx_ide_virtbase;
extern u64 au1xxx_ide_physbase;
extern int au1xxx_ide_irq;

/* Ddma */
chan_tab_t *ide_read_ch, *ide_write_ch;
u32 au1xxx_ide_ddma_enable = 0, switch4ddma = 1; // PIO+ddma

dbdev_tab_t new_dbdev_tab_element = { DSCR_CMD0_THROTTLE, DEV_FLAGS_ANYUSE, 0, 0, 0x00000000, 0, 0 };
#endif /* end CONFIG_BLK_DEV_IDE_AU1XXX */

extern struct rtc_ops no_rtc_ops;

void board_reset (void)
{
	/* Hit BCSR.SYSTEM_CONTROL[SW_RST] */
	au_writel(0x00000000, 0xAE00001C);
}

void board_power_off (void)
{
#ifdef CONFIG_MIPS_MIRAGE
	au_writel((1 << 26) | (1 << 10), GPIO2_OUTPUT);
#endif
}

void __init board_setup(void)
{
	u32 pin_func;

	rtc_ops = &no_rtc_ops;

	/* not valid for 1550 */
#ifdef CONFIG_AU1X00_USB_DEVICE
	// 2nd USB port is USB device
	pin_func = au_readl(SYS_PINFUNC) & (u32)(~0x8000);
	au_writel(pin_func, SYS_PINFUNC);
#endif

#if defined(CONFIG_IRDA) && (defined(CONFIG_SOC_AU1000) || defined(CONFIG_SOC_AU1100))
	/* set IRFIRSEL instead of GPIO15 */
	pin_func = au_readl(SYS_PINFUNC) | (u32)((1<<8));
	au_writel(pin_func, SYS_PINFUNC);
	/* power off until the driver is in use */
	bcsr->resets &= ~BCSR_RESETS_IRDA_MODE_MASK;
	bcsr->resets |= BCSR_RESETS_IRDA_MODE_OFF;
	au_sync();
#endif
	au_writel(0, 0xAE000010); /* turn off pcmcia power */

#ifdef CONFIG_MIPS_MIRAGE
	/* enable GPIO[31:0] inputs */
	au_writel(0, SYS_PININPUTEN);

	/* GPIO[20] is output, tristate the other input primary GPIO's */
	au_writel((u32)(~(1<<20)), SYS_TRIOUTCLR);

	/* set GPIO[210:208] instead of SSI_0 */
	pin_func = au_readl(SYS_PINFUNC) | (u32)(1);

	/* set GPIO[215:211] for LED's */
	pin_func |= (u32)((5<<2));

	/* set GPIO[214:213] for more LED's */
	pin_func |= (u32)((5<<12));

	/* set GPIO[207:200] instead of PCMCIA/LCD */
	pin_func |= (u32)((3<<17));
	au_writel(pin_func, SYS_PINFUNC);

	/* Enable speaker amplifier.  This should
	 * be part of the audio driver.
	 */
	au_writel(au_readl(GPIO2_DIR) | 0x200, GPIO2_DIR);
	au_writel(0x02000200, GPIO2_OUTPUT);
#endif

#if defined(CONFIG_AU1XXX_SMC91111)
#define CPLD_CONTROL (0xAF00000C)
	{
	extern uint32_t au1xxx_smc91111_base;
	extern unsigned int au1xxx_smc91111_irq;
	extern int au1xxx_smc91111_nowait;

	au1xxx_smc91111_base = 0xAC000300;
	au1xxx_smc91111_irq = AU1000_GPIO_8;
	au1xxx_smc91111_nowait = 1;

	/* set up the Static Bus timing - only 396Mhz */
	bcsr->resets |= 0x7;
	au_writel(0x00010003, MEM_STCFG0);
	au_writel(0x000c00c0, MEM_STCFG2);
	au_writel(0x85E1900D, MEM_STTIME2);
	}
#endif /* end CONFIG_SMC91111 */
	au_sync();

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX) && defined(CONFIG_MIPS_DB1550)
	/*
	 * Iniz IDE parameters
	 */
	ide_ops = &au1xxx_ide_ops;
	au1xxx_ide_irq = DAUGHTER_CARD_IRQ;
	au1xxx_ide_physbase = AU1XXX_ATA_PHYS_ADDR;
	au1xxx_ide_virtbase = KSEG1ADDR(AU1XXX_ATA_PHYS_ADDR);

	/*
	 * change PIO or PIO+Ddma
	 * check the GPIO-6 pin condition. db1550:s6_dot
	 */
	switch4ddma = (au_readl(SYS_PINSTATERD) & (1 << 6)) ? 1 : 0;
#endif

#ifdef CONFIG_MIPS_DB1000
    printk("AMD Alchemy Au1000/Db1000 Board\n");
#endif
#ifdef CONFIG_MIPS_DB1500
    printk("AMD Alchemy Au1500/Db1500 Board\n");
#endif
#ifdef CONFIG_MIPS_DB1100
    printk("AMD Alchemy Au1100/Db1100 Board\n");
#endif
#ifdef CONFIG_MIPS_BOSPORUS
    printk("AMD Alchemy Bosporus Board\n");
#endif
#ifdef CONFIG_MIPS_MIRAGE
    printk("AMD Alchemy Mirage Board\n");
#endif
#ifdef CONFIG_MIPS_DB1550
    printk("AMD Alchemy Au1550/Db1550 Board\n");
#endif
}
