/*
 *
 * BRIEF MODULE DESCRIPTION
 *	Alchemy Pb1200 board setup.
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
#include <linux/ide.h>

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX)
#include <linux/ide.h>
#endif

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/keyboard.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>
#include <asm/pgtable.h>
#include <asm/au1000.h>
#include <asm/ficmmp.h>
#include <asm/au1xxx_dbdma.h>
#include <asm/au1xxx_gpio.h>

extern struct rtc_ops no_rtc_ops;

/* value currently in the board configuration register */
u16 ficmmp_config = 0;

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX)
extern struct ide_ops *ide_ops;
extern struct ide_ops au1xxx_ide_ops;
extern u32 au1xxx_ide_virtbase;
extern u64 au1xxx_ide_physbase;
extern int au1xxx_ide_irq;

u32 led_base_addr;
/* Ddma */
chan_tab_t *ide_read_ch, *ide_write_ch;
u32 au1xxx_ide_ddma_enable = 0, switch4ddma = 1; // PIO+ddma

dbdev_tab_t new_dbdev_tab_element = { DSCR_CMD0_THROTTLE, DEV_FLAGS_ANYUSE, 0, 0, 0x00000000, 0, 0 };
#endif /* end CONFIG_BLK_DEV_IDE_AU1XXX */

void board_reset (void)
{
	au_writel(0, 0xAD80001C);
}

void board_power_off (void)
{
}

void __init board_setup(void)
{
	char *argptr = NULL;
	u32 pin_func;
	rtc_ops = &no_rtc_ops;

	ficmmp_config_init();	//Initialize FIC control register
	
#if 0
	/* Enable PSC1 SYNC for AC97.  Normaly done in audio driver,
	 * but it is board specific code, so put it here.
	 */
	pin_func = au_readl(SYS_PINFUNC);
	au_sync();
	pin_func |= SYS_PF_MUST_BE_SET | SYS_PF_PSC1_S1;
	au_writel(pin_func, SYS_PINFUNC);

	au_writel(0, (u32)bcsr|0x10); /* turn off pcmcia power */
	au_sync();
#endif

#if defined( CONFIG_I2C_ALGO_AU1550 )
	{
	u32 freq0, clksrc;

	/* Select SMBUS in CPLD */
	/* bcsr->resets &= ~(BCSR_RESETS_PCS0MUX); */

	pin_func = au_readl(SYS_PINFUNC);
	au_sync();
	pin_func &= ~(3<<17 | 1<<4);
	/* Set GPIOs correctly */
	pin_func |= 2<<17;
	au_writel(pin_func, SYS_PINFUNC);
	au_sync();

	/* The i2c driver depends on 50Mhz clock */
	freq0 = au_readl(SYS_FREQCTRL0);
	au_sync();
	freq0 &= ~(SYS_FC_FRDIV1_MASK | SYS_FC_FS1 | SYS_FC_FE1);
	freq0 |= (3<<SYS_FC_FRDIV1_BIT);
	/* 396Mhz / (3+1)*2 == 49.5Mhz */
	au_writel(freq0, SYS_FREQCTRL0);
	au_sync();
	freq0 |= SYS_FC_FE1;
	au_writel(freq0, SYS_FREQCTRL0);
	au_sync();

	clksrc = au_readl(SYS_CLKSRC);
	au_sync();
	clksrc &= ~0x01f00000;
	/* bit 22 is EXTCLK0 for PSC0 */
	clksrc |= (0x3 << 22);
	au_writel(clksrc, SYS_CLKSRC);
	au_sync();
	}
#endif

#ifdef CONFIG_FB_AU1200
	argptr = prom_getcmdline();
	strcat(argptr, " video=au1200fb:");
#endif

#if defined(CONFIG_BLK_DEV_IDE_AU1XXX)
	/*
	 * Iniz IDE parameters
	 */
	ide_ops = &au1xxx_ide_ops;
	au1xxx_ide_irq = FICMMP_IDE_INT;
	au1xxx_ide_physbase = AU1XXX_ATA_PHYS_ADDR;
	au1xxx_ide_virtbase = KSEG1ADDR(AU1XXX_ATA_PHYS_ADDR);
	switch4ddma = 0;
	/*
	ide_ops = &au1xxx_ide_ops;
	au1xxx_ide_irq = FICMMP_IDE_INT;
	au1xxx_ide_base = KSEG1ADDR(AU1XXX_ATA_BASE);
	*/
	au1xxx_gpio_write(9, 1);
	printk("B4001010: %X\n", *((u32*)0xB4001010));
	printk("B4001014: %X\n", *((u32*)0xB4001014));
	printk("B4001018: %X\n", *((u32*)0xB4001018));
	printk("B1900100: %X\n", *((u32*)0xB1900100));
	
#if 0
	ficmmp_config_clear(FICMMP_CONFIG_IDERST);
	mdelay(100);
	ficmmp_config_set(FICMMP_CONFIG_IDERST);
	mdelay(100);
#endif
	/*
	 * change PIO or PIO+Ddma
	 * check the GPIO-5 pin condition. pb1200:s18_dot
	 */
/*	switch4ddma = 0; //(au_readl(SYS_PINSTATERD) & (1 << 5)) ? 1 : 0; */
#endif

	/* The Pb1200 development board uses external MUX for PSC0 to
	support SMB/SPI. bcsr->resets bit 12: 0=SMB 1=SPI
	*/
#if defined(CONFIG_AU1550_PSC_SPI) && defined(CONFIG_I2C_ALGO_AU1550)
	#error I2C and SPI are mutually exclusive. Both are physically connected to PSC0.\
			Refer to Pb1200 documentation.
#elif defined( CONFIG_AU1550_PSC_SPI )
	//bcsr->resets |= BCSR_RESETS_PCS0MUX;
#elif defined( CONFIG_I2C_ALGO_AU1550 )
	//bcsr->resets &= (~BCSR_RESETS_PCS0MUX);
#endif
	au_sync();

	printk("FIC Multimedia Player Board\n");
	au1xxx_gpio_tristate(5);
	printk("B1900100: %X\n", *((volatile u32*)0xB1900100));
	printk("B190002C: %X\n", *((volatile u32*)0xB190002C));
}

int
board_au1200fb_panel (void)
{
	au1xxx_gpio_tristate(6);

	if (au1xxx_gpio_read(12) == 0)
		return 9; /* FS453_640x480 (Composite/S-Video) */
	else
		return 7; /* Sharp 320x240 TFT */
}

int
board_au1200fb_panel_init (void)
{
	/*Enable data buffers*/
	ficmmp_config_clear(FICMMP_CONFIG_LCMDATAOUT);
	/*Take LCD out of reset*/
	ficmmp_config_set(FICMMP_CONFIG_LCMPWREN | FICMMP_CONFIG_LCMEN);
	return 0;
}

int 
board_au1200fb_panel_shutdown (void)
{
	/*Disable data buffers*/
	ficmmp_config_set(FICMMP_CONFIG_LCMDATAOUT);
	/*Put LCD in reset, remove power*/
	ficmmp_config_clear(FICMMP_CONFIG_LCMEN | FICMMP_CONFIG_LCMPWREN);
	return 0;
}

