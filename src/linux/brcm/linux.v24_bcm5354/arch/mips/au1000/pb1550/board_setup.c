/*
 *
 * BRIEF MODULE DESCRIPTION
 *	Alchemy Pb1550 board setup.
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
#include <asm/pb1550.h>

extern struct rtc_ops no_rtc_ops;

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX)
extern struct ide_ops *ide_ops;
extern struct ide_ops au1xxx_ide_ops;
extern u32 au1xxx_ide_virtbase;
extern u64 au1xxx_ide_physbase;
extern unsigned int au1xxx_ide_irq;

u32 au1xxx_ide_ddma_enable = 0, switch4ddma = 1; // PIO+ddma
#endif /* end CONFIG_BLK_DEV_IDE_AU1XXX */

void board_reset (void)
{
    /* Hit BCSR.SYSTEM_CONTROL[SW_RST] */
	au_writew(au_readw(0xAF00001C) & ~(1<<15), 0xAF00001C);
}

void board_power_off (void)
{
	/* power off system */
	printk("\n** Powering off Pb1550\n");
	au_writew(au_readw(0xAF00001C) | (3<<14), 0xAF00001C); 
	au_sync();
	while(1); /* should not get here */
}

void __init board_setup(void)
{
	u32 pin_func;
	rtc_ops = &no_rtc_ops;

	/* Enable PSC1 SYNC for AC97.  Normaly done in audio driver,
	 * but it is board specific code, so put it here.
	 */
	pin_func = au_readl(SYS_PINFUNC);
	au_sync();
	pin_func |= SYS_PF_MUST_BE_SET | SYS_PF_PSC1_S1;
	au_writel(pin_func, SYS_PINFUNC);

	/* Do some more for PSC3 I2S audio.
	*/
	pin_func = au_readl(SYS_PINFUNC);
	au_sync();
	pin_func &= ~SYS_PF_PSC3_MASK;
	pin_func |= SYS_PF_PSC3_I2S | SYS_PF_EX0;
	au_writel(pin_func, SYS_PINFUNC);

	au_writel(0, (u32)bcsr|0x10); /* turn off pcmcia power */
	au_sync();

#if defined(CONFIG_AU1XXX_SMC91111)
#if defined(CONFIG_BLK_DEV_IDE_AU1XXX)
#error "Resource conflict occured. Disable either Ethernet or IDE daughter card."
#else
#define CPLD_CONTROL (0xAF00000C)
	{
	/* set up the Static Bus timing */
	/* only 396Mhz */
	/* reset the DC */
	au_writew(au_readw(CPLD_CONTROL) | 0x0f, CPLD_CONTROL);
	au_writel(0x00010003, MEM_STCFG0);
	au_writel(0x000c00c0, MEM_STCFG2);
	au_writel(0x85E1900D, MEM_STTIME2);
	}
#endif
#endif /* end CONFIG_SMC91111 */

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX)
	/*
	 * Iniz IDE parameters
	 */
	ide_ops = &au1xxx_ide_ops;
	au1xxx_ide_irq = DAUGHTER_CARD_IRQ;;
	au1xxx_ide_physbase = AU1XXX_ATA_PHYS_ADDR;
	au1xxx_ide_virtbase = KSEG1ADDR(AU1XXX_ATA_PHYS_ADDR);
	/*
	 * change PIO or PIO+Ddma
	 * check the GPIO-6 pin condition. pb1550:s15_dot
	 */
	switch4ddma = (au_readl(SYS_PINSTATERD) & (1 << 6)) ? 1 : 0;
#endif
	printk("AMD Alchemy Pb1550 Board\n");
}
