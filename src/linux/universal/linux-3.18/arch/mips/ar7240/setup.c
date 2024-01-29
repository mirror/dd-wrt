/*
 * setup.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <gottschall@dd-wrt.com>
 * Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2007 Atheros
 * mainly based on Atheros LSDK Code, some code taken from OpenWrt and ATH79 tree
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
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
#include <asm/mach-ar71xx/ar71xx.h>

#include "ar7240.h"

#ifdef CONFIG_WASP_SUPPORT
#include <ar934x.h>
#endif

#ifdef CONFIG_AR7240_EMULATION
#define         AG7240_CONSOLE_BAUD (9600)
#else
#define         AG7240_CONSOLE_BAUD (115200)
#endif

uint32_t ar7240_cpu_freq = 0, ar7240_ahb_freq, ar7240_ddr_freq;

u32 ar71xx_ahb_freq;
EXPORT_SYMBOL_GPL(ar71xx_ahb_freq);

#ifdef CONFIG_WASP_SUPPORT
uint32_t ath_ref_clk_freq;
#endif

static int __init ar7240_init_ioc(void);
void serial_print(char *fmt, ...);
void writeserial(char *str, int count);
#ifdef CONFIG_WASP_SUPPORT
static void wasp_sys_frequency(void);
#else
static void ar7240_sys_frequency(void);
#endif
#ifdef CONFIG_MACH_HORNET
static void hornet_sys_frequency(void);
void UartHornetInit(void);
void UartHornetPut(u8 byte);

#define Uart16550Init          UartHornetInit
#define Uart16550Put           UartHornetPut
#else
void Uart16550Init(void);
u8 Uart16550GetPoll(void);
#endif
/* 
 * Export AHB freq value to be used by Ethernet MDIO.
 */
EXPORT_SYMBOL(ar7240_ahb_freq);

void ar7240_restart(char *command)
{
	for (;;) {
		if (is_ar934x_10()) {
			/*
			 * WAR for full chip reset spi vs. boot-rom selection
			 * bug in wasp 1.0
			 */
			ar7240_reg_wr(AR7240_GPIO_OE, ar7240_reg_rd(AR7240_GPIO_OE) & (~(1 << 17)));
		} else {
			ar7240_reg_wr(AR7240_RESET, AR7240_RESET_FULL_CHIP);
		}
	}
}

void ar7240_halt(void)
{
	printk(KERN_NOTICE "\n** You can safely turn off the power\n");
	while (1) ;
}

void ar7240_power_off(void)
{
	ar7240_halt();
}

const char
*get_system_type(void)
{

	char *chip;
	u32 id;
	u32 qca = 0;
	u32 tp = 0;
	u32 rev = 0;
	u32 ver = 1;
	static char str[64];
	id = ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK;

	switch (id) {
	case AR7240_REV_1_0:
		chip = "7240";
		break;
	case AR7240_REV_1_1:
		chip = "7240";
		rev = 1;
		break;
	case AR7240_REV_1_2:
		chip = "7240";
		rev = 2;
		break;
	case AR7241_REV_1_0:
		chip = "7241";
		break;
	case AR7242_REV_1_0:
		chip = "7242";
		break;
	case AR7241_REV_1_1:
		chip = "7241";
		rev = 1;
		break;
	case AR7242_REV_1_1:
		chip = "7242";
		rev = 1;
		break;
	case AR9330_REV_1_0:
		chip = "9330";
		break;
	case AR9330_REV_1_1:
		chip = "9330";
		rev = 1;
		break;
	case AR9330_REV_1_2:
		chip = "9330";
		rev = 2;
		break;
	case AR9331_REV_1_0:
		chip = "9331";
		break;
	case AR9331_REV_1_1:
		chip = "9331";
		rev = 1;
		break;
	case AR9331_REV_1_2:
		chip = "9331";
		rev = 2;
		break;
	case AR9344_REV_1_0:
		chip = "9344";
		break;
	case AR9344_REV_1_1:
		chip = "9344";
		rev = 1;
		break;
	case AR9344_REV_1_2:
		chip = "9344";
		rev = 2;
		break;
	case AR9344_REV_1_3:
		chip = "9344";
		rev = 3;
		break;
	case AR9342_REV_1_0:
		chip = "9342";
		break;
	case AR9342_REV_1_1:
		chip = "9342";
		rev = 1;
		break;
	case AR9342_REV_1_2:
		chip = "9342";
		rev = 2;
		break;
	case AR9342_REV_1_3:
		chip = "9342";
		rev = 3;
		break;
	case AR9341_REV_1_0:
		chip = "9341";
		break;
	case AR9341_REV_1_1:
		chip = "9341";
		rev = 1;
		break;
	case AR9341_REV_1_2:
		chip = "9341";
		rev = 2;
		break;
	case AR9341_REV_1_3:
		chip = "9341";
		rev = 3;
		break;
	case QCA9533_REV_1_0:
		chip = "9533";
		qca = 1;
		break;
	case QCA9533_V2:
		chip = "9533";
		ver = 2;
		qca = 1;
		break;
	case QCA9533_REV_1_1:
		chip = "9533";
		rev = 1;
		qca = 1;
		break;
	case QCA9533_REV_1_2:
		chip = "9533";
		rev = 2;
		qca = 1;
		break;
	case QCA9533_REV_1_3:
		chip = "9533";
		rev = 3;
		qca = 1;
		break;

	case QCA9556_REV_1_0:
		chip = "9556";
		qca = 1;
		break;
	case QCA9556_REV_1_1:
		chip = "9556";
		rev = 1;
		qca = 1;
		break;
	case QCA9556_REV_1_2:
		chip = "9556";
		rev = 2;
		qca = 1;
		break;
	case QCA9556_REV_1_3:
		chip = "9556";
		rev = 3;
		qca = 1;
		break;
	case QCA9558_REV_1_0:
		chip = "9558";
		qca = 1;
		break;
	case QCA9558_REV_1_1:
		chip = "9558";
		rev = 1;
		qca = 1;
		break;
	case QCA9558_REV_1_2:
		chip = "9558";
		rev = 2;
		qca = 1;
		break;
	case QCA9558_REV_1_3:
		chip = "9558";
		rev = 3;
		qca = 1;
		break;

	case QCA9563_REV_1_0:
		chip = "9563";
		qca = 1;
		break;
	case QCA9563_REV_1_1:
		chip = "9563";
		rev = 1;
		qca = 1;
		break;
	case QCA9563_REV_1_2:
		chip = "9563";
		rev = 2;
		qca = 1;
		break;
	case QCA9563_REV_1_3:
		chip = "9563";
		rev = 3;
		qca = 1;
		break;
	case TP9343_REV_1_0:
		chip = "9343";
		tp = 1;
		break;
	case TP9343_REV_1_1:
		chip = "9343";
		rev = 1;
		tp = 1;
		break;
	case TP9343_REV_1_2:
		chip = "9343";
		rev = 2;
		tp = 1;
		break;
	case TP9343_REV_1_3:
		chip = "9343";
		rev = 3;
		tp = 1;
		break;
	default:
		chip = "724x";
	}
	sprintf(str, "%s %s%s ver %u rev 1.%u (0x%04x)",qca ? "Qualcomm Atheros" : "Atheros", tp ? "TP" : qca ? "QCA" : "AR",chip, ver, rev, id);
	return str;
}

EXPORT_SYMBOL(get_system_type);

#if defined(CONFIG_WASP_SUPPORT) || defined(CONFIG_MACH_HORNET)
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
	return ATH_CPU_IRQ_WLAN;
}

unsigned long get_wmac_base(u_int16_t wmac_num)
{
	return KSEG1ADDR(ATH_WMAC_BASE);
}

unsigned long get_wmac_mem_len(u_int16_t wmac_num)
{
	return ATH_WMAC_LEN;
}

EXPORT_SYMBOL(valid_wmac_num);
EXPORT_SYMBOL(get_wmac_irq);
EXPORT_SYMBOL(get_wmac_base);
EXPORT_SYMBOL(get_wmac_mem_len);
#endif

/*
 * The bootloader musta set cpu_pll_config.
 * We extract the pll divider, multiply it by the base freq 40.
 * The cpu and ahb are divided off of that.
 */
//#define FB50 1

#if defined (CONFIG_WASP_SUPPORT)

#include "hack.h"


static void __init qca956x_clocks_init(void)
{
	unsigned long ref_rate;
	unsigned long cpu_rate;
	unsigned long ddr_rate;
	unsigned long ahb_rate;
	u32 pll, out_div, ref_div, nint, hfrac, lfrac, clk_ctrl, postdiv;
	u32 cpu_pll, ddr_pll;
	u32 bootstrap;

	bootstrap = ar7240_reg_rd(ATH_RESET_BASE + QCA956X_RESET_REG_BOOTSTRAP);
	if (bootstrap &	QCA956X_BOOTSTRAP_REF_CLK_40)
		ref_rate = 40 * 1000 * 1000;
	else
		ref_rate = 25 * 1000 * 1000;

	pll = ar7240_reg_rd(ATH_PLL_BASE + QCA956X_PLL_CPU_CONFIG_REG);
	out_div = (pll >> QCA956X_PLL_CPU_CONFIG_OUTDIV_SHIFT) &
		  QCA956X_PLL_CPU_CONFIG_OUTDIV_MASK;
	ref_div = (pll >> QCA956X_PLL_CPU_CONFIG_REFDIV_SHIFT) &
		  QCA956X_PLL_CPU_CONFIG_REFDIV_MASK;

	pll = ar7240_reg_rd(ATH_PLL_BASE + QCA956X_PLL_CPU_CONFIG1_REG);
	nint = (pll >> QCA956X_PLL_CPU_CONFIG1_NINT_SHIFT) &
	       QCA956X_PLL_CPU_CONFIG1_NINT_MASK;
	hfrac = (pll >> QCA956X_PLL_CPU_CONFIG1_NFRAC_H_SHIFT) &
	       QCA956X_PLL_CPU_CONFIG1_NFRAC_H_MASK;
	lfrac = (pll >> QCA956X_PLL_CPU_CONFIG1_NFRAC_L_SHIFT) &
	       QCA956X_PLL_CPU_CONFIG1_NFRAC_L_MASK;

	cpu_pll = nint * ref_rate / ref_div;
	cpu_pll += (lfrac * ref_rate) / ((ref_div * 25) << 13);
	cpu_pll += (hfrac >> 13) * ref_rate / ref_div;
	cpu_pll /= (1 << out_div);

	pll = ar7240_reg_rd(ATH_PLL_BASE + QCA956X_PLL_DDR_CONFIG_REG);
	out_div = (pll >> QCA956X_PLL_DDR_CONFIG_OUTDIV_SHIFT) &
		  QCA956X_PLL_DDR_CONFIG_OUTDIV_MASK;
	ref_div = (pll >> QCA956X_PLL_DDR_CONFIG_REFDIV_SHIFT) &
		  QCA956X_PLL_DDR_CONFIG_REFDIV_MASK;
	pll = ar7240_reg_rd(ATH_PLL_BASE + QCA956X_PLL_DDR_CONFIG1_REG);
	nint = (pll >> QCA956X_PLL_DDR_CONFIG1_NINT_SHIFT) &
	       QCA956X_PLL_DDR_CONFIG1_NINT_MASK;
	hfrac = (pll >> QCA956X_PLL_DDR_CONFIG1_NFRAC_H_SHIFT) &
	       QCA956X_PLL_DDR_CONFIG1_NFRAC_H_MASK;
	lfrac = (pll >> QCA956X_PLL_DDR_CONFIG1_NFRAC_L_SHIFT) &
	       QCA956X_PLL_DDR_CONFIG1_NFRAC_L_MASK;

	ddr_pll = nint * ref_rate / ref_div;
	ddr_pll += (lfrac * ref_rate) / ((ref_div * 25) << 13);
	ddr_pll += (hfrac >> 13) * ref_rate / ref_div;
	ddr_pll /= (1 << out_div);

	clk_ctrl = ar7240_reg_rd(ATH_PLL_BASE + QCA956X_PLL_CLK_CTRL_REG);

	postdiv = (clk_ctrl >> QCA956X_PLL_CLK_CTRL_CPU_POST_DIV_SHIFT) &
		  QCA956X_PLL_CLK_CTRL_CPU_POST_DIV_MASK;

	if (clk_ctrl & QCA956X_PLL_CLK_CTRL_CPU_PLL_BYPASS)
		cpu_rate = ref_rate;
	else if (clk_ctrl & QCA956X_PLL_CLK_CTRL_CPU_DDRCLK_FROM_CPUPLL)
		cpu_rate = ddr_pll / (postdiv + 1);
	else
		cpu_rate = cpu_pll / (postdiv + 1);

	postdiv = (clk_ctrl >> QCA956X_PLL_CLK_CTRL_DDR_POST_DIV_SHIFT) &
		  QCA956X_PLL_CLK_CTRL_DDR_POST_DIV_MASK;

	if (clk_ctrl & QCA956X_PLL_CLK_CTRL_DDR_PLL_BYPASS)
		ddr_rate = ref_rate;
	else if (clk_ctrl & QCA956X_PLL_CLK_CTRL_CPU_DDRCLK_FROM_DDRPLL)
		ddr_rate = cpu_pll / (postdiv + 1);
	else
		ddr_rate = ddr_pll / (postdiv + 1);

	postdiv = (clk_ctrl >> QCA956X_PLL_CLK_CTRL_AHB_POST_DIV_SHIFT) &
		  QCA956X_PLL_CLK_CTRL_AHB_POST_DIV_MASK;

	if (clk_ctrl & QCA956X_PLL_CLK_CTRL_AHB_PLL_BYPASS)
		ahb_rate = ref_rate;
	else if (clk_ctrl & QCA956X_PLL_CLK_CTRL_AHBCLK_FROM_DDRPLL)
		ahb_rate = ddr_pll / (postdiv + 1);
	else
		ahb_rate = cpu_pll / (postdiv + 1);


	ath_ref_clk_freq = ref_rate;
	ar71xx_ref_freq = ref_rate;
	ar7240_cpu_freq = cpu_rate;
	ar7240_ddr_freq = ddr_rate;
	ar7240_ahb_freq = ahb_rate;
	printk(KERN_INFO "ref %d, cpu %d, ddr %d, ahb %d\n",ref_rate,ar7240_cpu_freq,ar7240_ddr_freq,ar7240_ahb_freq);
}



static void wasp_sys_frequency(void)
{
	uint32_t pll, out_div, ref_div, nint, frac, clk_ctrl;
	uint32_t ref;

	if (ar7240_cpu_freq)
		return;
	if (is_qca956x()) {
		qca956x_clocks_init();
		return;
	}
	if ((ar7240_reg_rd(ATH_BOOTSTRAP_REG) & ATH_REF_CLK_40)) {
		ref = (40 * 1000000);
	} else {
		ref = (25 * 1000000);
	}

	ath_ref_clk_freq = ref;
	ar71xx_ref_freq = ref;

	clk_ctrl = ar7240_reg_rd(ATH_DDR_CLK_CTRL);

	pll = ar7240_reg_rd(CPU_DPLL2_ADDRESS);

	if (!is_qca955x() && !is_qca953x() && CPU_DPLL2_LOCAL_PLL_GET(pll)) {
		out_div = CPU_DPLL2_OUTDIV_GET(pll);

		pll = ar7240_reg_rd(CPU_DPLL_ADDRESS);

		nint = CPU_DPLL_NINT_GET(pll);

		frac = CPU_DPLL_NFRAC_GET(pll);

		ref_div = CPU_DPLL_REFDIV_GET(pll);
		pll = ref >> 18;
		frac = frac * pll / ref_div;
		printk("cpu srif ");
	} else {
		pll = ar7240_reg_rd(ATH_PLL_CONFIG);

		out_div = CPU_PLL_CONFIG_OUTDIV_GET(pll);
		ref_div = CPU_PLL_CONFIG_REFDIV_GET(pll);
		nint = CPU_PLL_CONFIG_NINT_GET(pll);
		frac = CPU_PLL_CONFIG_NFRAC_GET(pll);
		pll = ref >> 6;
		frac = frac * pll / ref_div;
		printk("cpu apb ");
	}
	ar7240_cpu_freq = (((nint * (ref / ref_div)) + frac) >> out_div) / (CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_GET(clk_ctrl) + 1);

	pll = ar7240_reg_rd(DDR_DPLL2_ADDRESS);
	if (!is_qca955x() && !is_qca953x() && DDR_DPLL2_LOCAL_PLL_GET(pll)) {
		out_div = DDR_DPLL2_OUTDIV_GET(pll);

		pll = ar7240_reg_rd(DDR_DPLL_ADDRESS);
		nint = DDR_DPLL_NINT_GET(pll);
		frac = DDR_DPLL_NFRAC_GET(pll);
		ref_div = DDR_DPLL_REFDIV_GET(pll);
		pll = ref >> 18;
		frac = frac * pll / ref_div;
		printk("ddr srif ");
	} else {
		pll = ar7240_reg_rd(ATH_DDR_PLL_CONFIG);
		out_div = DDR_PLL_CONFIG_OUTDIV_GET(pll);
		ref_div = DDR_PLL_CONFIG_REFDIV_GET(pll);
		nint = DDR_PLL_CONFIG_NINT_GET(pll);
		frac = DDR_PLL_CONFIG_NFRAC_GET(pll);
		pll = ref >> 10;
		frac = frac * pll / ref_div;
		printk("ddr apb ");
	}
	ar7240_ddr_freq = (((nint * (ref / ref_div)) + frac) >> out_div) / (CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_GET(clk_ctrl) + 1);

	if (CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_GET(clk_ctrl)) {
		ar7240_ahb_freq = ar7240_ddr_freq / (CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_GET(clk_ctrl) + 1);
	} else {
		ar7240_ahb_freq = ar7240_cpu_freq / (CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_GET(clk_ctrl) + 1);
	}

	printk("cpu %u ddr %u ahb %u\n", ar7240_cpu_freq / 1000000, ar7240_ddr_freq / 1000000, ar7240_ahb_freq / 1000000);

}
#else

static void ar7240_sys_frequency(void)
{
#ifdef CONFIG_MACH_HORNET
	hornet_sys_frequency();
#else				/* CONFIG_MACH_HORNET */
#ifdef CONFIG_AR7240_EMULATION
#ifdef FB50
	ar7240_cpu_freq = 66000000;
	ar7240_ddr_freq = 66000000;
	ar7240_ahb_freq = 33000000;
#else
#if 1
	ar7240_cpu_freq = 300000000;
	ar7240_ddr_freq = 300000000;
	ar7240_ahb_freq = 150000000;
#else
	ar7240_cpu_freq = 62500000;
	ar7240_ddr_freq = 62500000;
	ar7240_ahb_freq = 31250000;
#endif

#endif
	return;
#else
	uint32_t pll, pll_div, ahb_div, ddr_div, freq, ref_div;

	if (ar7240_cpu_freq)
		return;

	pll = ar7240_reg_rd(AR7240_PLL_CONFIG);

	pll_div = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK);
	ref_div = ((pll >> REF_DIV_SHIFT) & REF_DIV_MASK) * 2;
	ddr_div = ((pll >> DDR_DIV_SHIFT) & DDR_DIV_MASK) + 1;
	ahb_div = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1) * 2;
	
	freq = (pll_div * 40000000) / ref_div;

	ar7240_cpu_freq = freq;
	ar7240_ddr_freq = freq / ddr_div;
	ar7240_ahb_freq = ar7240_cpu_freq / ahb_div;
#endif
#endif
}
#endif
void __init plat_time_init(void)
{
	/* 
	 * to generate the first CPU timer interrupt
	 */
	write_c0_count(0);
	write_c0_compare(0xffff);

	mips_hpt_frequency = ar7240_cpu_freq / 2;
}

#define compare_change_hazard() \
	do { \
		irq_disable_hazard(); \
		irq_disable_hazard(); \
		irq_disable_hazard(); \
		irq_disable_hazard(); \
	} while (0)

#if 0
int ar7240_be_handler(struct pt_regs *regs, int is_fixup)
{
#if 0
	if (!is_fixup && (regs->cp0_cause & 4)) {
		/* Data bus error - print PA */
		printk("DBE physical address: %010Lx\n", __read_64bit_c0_register($26, 1));
	}
#endif
#ifdef CONFIG_PCI
	int error = 0, status, trouble = 0;
	error = ar7240_reg_rd(AR7240_PCI_ERROR) & 3;

	if (error) {
		printk("PCI error %d at PCI addr 0x%x\n", error, ar7240_reg_rd(AR7240_PCI_ERROR_ADDRESS));
		ar7240_reg_wr(AR7240_PCI_ERROR, error);
		ar7240_local_read_config(PCI_STATUS, 2, &status);
		printk("PCI status: %#x\n", status);
		trouble = 1;
	}

	error = 0;
	error = ar7240_reg_rd(AR7240_PCI_AHB_ERROR) & 1;

	if (error) {
		printk("AHB error at AHB address 0x%x\n", ar7240_reg_rd(AR7240_PCI_AHB_ERROR_ADDRESS));
		ar7240_reg_wr(AR7240_PCI_AHB_ERROR, error);
		ar7240_local_read_config(PCI_STATUS, 2, &status);
		printk("PCI status: %#x\n", status);
		trouble = 1;
	}
#endif

	printk("ar7240 data bus error: cause %#x\n", read_c0_cause());
	return (is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL);
}
#endif
void disable_early_printk(void)
{
}

static int ramsize;

#define AR71XX_MEM_SIZE_MIN	(2 * 1024 * 1024)
#define AR71XX_MEM_SIZE_MAX	(256 * 1024 * 1024)

static void __init ar71xx_detect_mem_size(void)
{
	unsigned long size;

	for (size = AR71XX_MEM_SIZE_MIN; size < AR71XX_MEM_SIZE_MAX; size <<= 1) {
		if (!memcmp(ar71xx_detect_mem_size, ar71xx_detect_mem_size + size, 1024))
			break;
	}
	ramsize = size;

	add_memory_region(0, size, BOOT_MEM_RAM);
}

unsigned int __cpuinit get_c0_compare_irq(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

unsigned int __cpuinit get_c0_compare_int(void)
{
	//printk("%s: returning timer irq : %d\n",__func__, AR7240_CPU_IRQ_TIMER);
	return AR7240_CPU_IRQ_TIMER;
}

#define AR71XX_REV_ID_REVISION_MASK	0x3
#define AR71XX_REV_ID_REVISION_SHIFT	2
#define AR933X_RESET_REG_BOOTSTRAP		0xac
#define AR933X_RESET_REG_BOOTSTRAP		0xac
#define AR933X_BOOTSTRAP_MDIO_GPIO_EN		BIT(18)
#define AR933X_BOOTSTRAP_EEPBUSY		BIT(4)
#define AR933X_BOOTSTRAP_REF_CLK_40		BIT(0)

u32 ar71xx_soc_rev;
EXPORT_SYMBOL_GPL(ar71xx_soc_rev);

u32 ar71xx_ref_freq;
EXPORT_SYMBOL_GPL(ar71xx_ref_freq);

int is_ar9000;
EXPORT_SYMBOL(is_ar9000);
void __init plat_mem_setup(void)
{

	u32 id;
	u32 t;
	u32 rev = 0;
	set_io_port_base(KSEG1);
	Uart16550Init();
	id = ar7240_reg_rd(AR7240_REV_ID);

	if (is_ar7240()) {
		serial_print("AR7240\n");
		ar71xx_soc = AR71XX_SOC_AR7240;
		ar71xx_soc_rev = id & AR724X_REV_ID_REVISION_MASK;
	} else if (is_ar7241()) {
		serial_print("AR7241\n");
		ar71xx_soc = AR71XX_SOC_AR7241;
		ar71xx_soc_rev = id & AR724X_REV_ID_REVISION_MASK;
	} else if (is_ar7242()) {
		serial_print("AR7242\n");
		ar71xx_soc = AR71XX_SOC_AR7242;
		ar71xx_soc_rev = id & AR724X_REV_ID_REVISION_MASK;
	} else if (is_ar9330()) {
		serial_print("AR9330\n");
		ar71xx_soc = AR71XX_SOC_AR9330;
		ar71xx_soc_rev = id & AR933X_REV_ID_REVISION_MASK;
	} else if (is_ar9331()) {
		serial_print("AR9331\n");
		ar71xx_soc = AR71XX_SOC_AR9331;
		ar71xx_soc_rev = id & AR933X_REV_ID_REVISION_MASK;
	} else if (is_ar9341()) {
		serial_print("AR9341\n");
		ar71xx_soc = AR71XX_SOC_AR9341;
		ar71xx_soc_rev = id & AR934X_REV_ID_REVISION_MASK;
	} else if (is_ar9342()) {
		serial_print("AR9342\n");
		ar71xx_soc = AR71XX_SOC_AR9342;
		ar71xx_soc_rev = id & AR934X_REV_ID_REVISION_MASK;
	} else if (is_ar9344()) {
		serial_print("AR9344\n");
		ar71xx_soc = AR71XX_SOC_AR9344;
		ar71xx_soc_rev = id & AR934X_REV_ID_REVISION_MASK;
	} else if (is_qca9533_v2()) {
		serial_print("QCA9533 V2\n");
		ar71xx_soc = AR71XX_SOC_QCA9533;
		ar71xx_soc_rev = 2;
	} else if (is_qca9533()) {
		serial_print("QCA9533\n");
		ar71xx_soc = AR71XX_SOC_QCA9533;
		ar71xx_soc_rev = id & QCA953X_REV_ID_REVISION_MASK;
	} else if (is_qca9556()) {
		serial_print("QCA9556\n");
		ar71xx_soc = AR71XX_SOC_QCA9556;
		ar71xx_soc_rev = id & QCA955X_REV_ID_REVISION_MASK;
	} else if (is_qca9558()) {
		serial_print("QCA9558\n");
		ar71xx_soc = AR71XX_SOC_QCA9558;
		ar71xx_soc_rev = id & QCA955X_REV_ID_REVISION_MASK;
	} else if (is_qca9563()) {
		serial_print("QCA9563\n");
		ar71xx_soc = AR71XX_SOC_QCA9563;
		ar71xx_soc_rev = id & QCA956X_REV_ID_REVISION_MASK;
	} else if (is_tp9343()) {
		serial_print("TP9343\n");
		ar71xx_soc = AR71XX_SOC_TP9343;
		ar71xx_soc_rev = id & QCA956X_REV_ID_REVISION_MASK;
	} else {

	}

	ar71xx_ddr_base = ioremap_nocache(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE);

	ar71xx_pll_base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	ar71xx_reset_base = ioremap_nocache(AR71XX_RESET_BASE, AR71XX_RESET_SIZE);

	ar71xx_gpio_base = ioremap_nocache(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);

	ar71xx_usb_ctrl_base = ioremap_nocache(AR71XX_USB_CTRL_BASE, AR71XX_USB_CTRL_SIZE);

#if 0
	board_be_handler = ar7240_be_handler;
#endif
//    board_timer_setup   =  ar7240_timer_setup;
	_machine_restart = ar7240_restart;
	_machine_halt = ar7240_halt;
	pm_power_off = ar7240_power_off;

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
	serial_print("detect mem size\n");
	ar71xx_detect_mem_size();
	serial_print("Uart Init\n");

	ar71xx_ahb_freq = ar7240_ahb_freq;

#ifdef CONFIG_MACH_HORNET
	serial_print("Booting (Hornet)...\n");
	/* clear wmac reset */
	ar7240_reg_wr(AR7240_RESET, (ar7240_reg_rd(AR7240_RESET) & (~AR7240_RESET_WMAC)));
#elif CONFIG_WASP_SUPPORT
	serial_print("Booting WASP !!! -:) ...\n");
#else
	serial_print("Booting AR7240(Python)...\n");
#endif
	is_ar9000 = 1;
	printk(KERN_INFO "sys id = %X %s\n", id, get_system_type());
//#if 0
//    serial_setup();
//#endif
}

/*
 * -------------------------------------------------
 * Early printk hack
 */
/* === CONFIG === */

#define		REG_OFFSET		4

static int serial_inited = 0;

#define         MY_WRITE(y, z)  ((*((volatile u32*)(y))) = z)

#ifndef CONFIG_MACH_HORNET

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

#define         UART16550_READ(y)   ar7240_reg_rd((AR7240_UART_BASE+y))
#define         UART16550_WRITE(x, z)  ar7240_reg_wr((AR7240_UART_BASE+x), z)

void Uart16550Init()
{
	int freq, div;

#ifdef CONFIG_WASP_SUPPORT
	wasp_sys_frequency();
	freq = ath_ref_clk_freq;
#else
	ar7240_sys_frequency();
	freq = ar7240_ahb_freq;

#if 0				// CONFIG_DIR615E
	MY_WRITE(0xb8040000, 0xcff);

	MY_WRITE(0xb8040008, 0x3b);

	/* Enable UART , SPI and Disable S26 UART */
	MY_WRITE(0xb8040028, (ar7240_reg_rd(0xb8040028) | 0x48002));

	MY_WRITE(0xb8040008, 0x2f);
#endif
#endif

	div = freq / (AG7240_CONSOLE_BAUD * 16);

//    div = 0xCB;
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

u8 Uart16550GetPoll()
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
#endif
extern int vsprintf(char *buf, const char *fmt, va_list args);
static char sprint_buf[1024];

void serial_print(char *fmt, ...)
{
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsprintf(sprint_buf, fmt, args);
	va_end(args);
	writeserial(sprint_buf, n);
}

void writeserial(char *str, int count)
{
	int i;
	for (i = 0; i <= count; i++)
		Uart16550Put(str[i]);

	Uart16550Put('\r');
	memset(str, '\0', 1024);
	return;
}

unsigned int getCPUClock(void)
{
	return ar7240_cpu_freq / 1000000;
}

#ifdef CONFIG_MACH_HORNET

void UartHornetInit(void)
{
	unsigned int rdata;
	unsigned int baudRateDivisor, clock_step;
	unsigned int fcEnable = 0;

	ar7240_sys_frequency();

#if 0
	MY_WRITE(0xb8040000, 0xcff);
	MY_WRITE(0xb8040008, 0x3b);
	/* Enable UART , SPI and Disable S26 UART */
	MY_WRITE(0xb8040028, (ar7240_reg_rd(0xb8040028) | 0x48002));

	MY_WRITE(0xb8040008, 0x2f);
#endif
#ifdef CONFIG_HORNET_EMULATION
	baudRateDivisor = (ar7240_ahb_freq / (16 * AG7240_CONSOLE_BAUD)) - 1;	// 24 MHz clock is taken as UART clock 
#else

	rdata = ar7240_reg_rd(HORNET_BOOTSTRAP_STATUS);
	rdata &= HORNET_BOOTSTRAP_SEL_25M_40M_MASK;

	if (rdata)
		baudRateDivisor = (40000000 / (16 * AG7240_CONSOLE_BAUD)) - 1;	// 40 MHz clock is taken as UART clock        
	else
		baudRateDivisor = (25000000 / (16 * AG7240_CONSOLE_BAUD)) - 1;	// 25 MHz clock is taken as UART clock        
#endif

	clock_step = 8192;

	rdata = UARTCLOCK_UARTCLOCKSCALE_SET(baudRateDivisor) | UARTCLOCK_UARTCLOCKSTEP_SET(clock_step);
	uart_reg_write(UARTCLOCK_ADDRESS, rdata);

	/* Config Uart Controller */
#if 1				/* No interrupt */
	rdata = UARTCS_UARTDMAEN_SET(0) | UARTCS_UARTHOSTINTEN_SET(0) | UARTCS_UARTHOSTINT_SET(0)
	    | UARTCS_UARTSERIATXREADY_SET(0) | UARTCS_UARTTXREADYORIDE_SET(~fcEnable)
	    | UARTCS_UARTRXREADYORIDE_SET(~fcEnable) | UARTCS_UARTHOSTINTEN_SET(0);
#else
	rdata = UARTCS_UARTDMAEN_SET(0) | UARTCS_UARTHOSTINTEN_SET(0) | UARTCS_UARTHOSTINT_SET(0)
	    | UARTCS_UARTSERIATXREADY_SET(0) | UARTCS_UARTTXREADYORIDE_SET(~fcEnable)
	    | UARTCS_UARTRXREADYORIDE_SET(~fcEnable) | UARTCS_UARTHOSTINTEN_SET(1);
#endif

	/* is_dte == 1 */
	rdata = rdata | UARTCS_UARTINTERFACEMODE_SET(2);

	if (fcEnable) {
		rdata = rdata | UARTCS_UARTFLOWCONTROLMODE_SET(2);
	}

	/* invert_fc ==0 (Inverted Flow Control) */
	//rdata = rdata | UARTCS_UARTFLOWCONTROLMODE_SET(3);

	/* parityEnable == 0 */
	//rdata = rdata | UARTCS_UARTPARITYMODE_SET(2); -->Parity Odd  
	//rdata = rdata | UARTCS_UARTPARITYMODE_SET(3); -->Parity Even
	uart_reg_write(UARTCS_ADDRESS, rdata);

	serial_inited = 1;
}

u8 UartHornetGetPoll(void)
{
	u8 ret_val;
	unsigned int rdata;

	do {
		rdata = uart_reg_read(UARTDATA_ADDRESS);
	} while (!UARTDATA_UARTRXCSR_GET(rdata));

	ret_val = (u8)UARTDATA_UARTTXRXDATA_GET(rdata);
	rdata = UARTDATA_UARTRXCSR_SET(1);
	uart_reg_write(UARTDATA_ADDRESS, rdata);

	return ret_val;
}

void UartHornetPut(u8 byte)
{
	unsigned int rdata;

	if (!serial_inited) {
		serial_inited = 1;
		UartHornetInit();
	}

	do {
		rdata = uart_reg_read(UARTDATA_ADDRESS);
	} while (UARTDATA_UARTTXCSR_GET(rdata) == 0);

	rdata = UARTDATA_UARTTXRXDATA_SET((unsigned int)byte);
	rdata |= UARTDATA_UARTTXCSR_SET(1);

	uart_reg_write(UARTDATA_ADDRESS, rdata);
}

static void hornet_sys_frequency(void)
{
#ifdef CONFIG_HORNET_EMULATION
#ifdef CONFIG_HORNET_EMULATION_WLAN_HARDI	/* FPGA WLAN emulation */
	ar7240_cpu_freq = 48000000;
	ar7240_ddr_freq = 48000000;
	ar7240_ahb_freq = 24000000;
#else
	ar7240_cpu_freq = 80000000;
	ar7240_ddr_freq = 80000000;
	ar7240_ahb_freq = 40000000;
#endif
#else
	/* Hornet's PLL is completely different from Python's */
	u32 ref_clock_rate, pll_freq;
	u32 pllreg, clockreg;
	u32 nint, refdiv, outdiv;
	u32 cpu_div, ahb_div, ddr_div;

	if (ar7240_reg_rd(HORNET_BOOTSTRAP_STATUS) & HORNET_BOOTSTRAP_SEL_25M_40M_MASK)
		ref_clock_rate = 40 * 1000000;
	else
		ref_clock_rate = 25 * 1000000;

	ar71xx_ref_freq = ref_clock_rate;

	pllreg = ar7240_reg_rd(AR7240_CPU_PLL_CONFIG);
	clockreg = ar7240_reg_rd(AR7240_CPU_CLOCK_CONTROL);

	if (clockreg & HORNET_CLOCK_CONTROL_BYPASS_MASK) {
		/* Bypass PLL */
		pll_freq = ref_clock_rate;
		cpu_div = ahb_div = ddr_div = 1;
	} else {
		nint = (pllreg & HORNET_PLL_CONFIG_NINT_MASK) >> HORNET_PLL_CONFIG_NINT_SHIFT;
		refdiv = (pllreg & HORNET_PLL_CONFIG_REFDIV_MASK) >> HORNET_PLL_CONFIG_REFDIV_SHIFT;
		outdiv = (pllreg & HORNET_PLL_CONFIG_OUTDIV_MASK) >> HORNET_PLL_CONFIG_OUTDIV_SHIFT;

		pll_freq = (ref_clock_rate / refdiv) * nint;

		if (outdiv == 1)
			pll_freq /= 2;
		else if (outdiv == 2)
			pll_freq /= 4;
		else if (outdiv == 3)
			pll_freq /= 8;
		else if (outdiv == 4)
			pll_freq /= 16;
		else if (outdiv == 5)
			pll_freq /= 32;
		else if (outdiv == 6)
			pll_freq /= 64;
		else if (outdiv == 7)
			pll_freq /= 128;
		else		/* outdiv == 0 --> illegal value */
			pll_freq /= 2;

		cpu_div = (clockreg & HORNET_CLOCK_CONTROL_CPU_POST_DIV_MASK) >> HORNET_CLOCK_CONTROL_CPU_POST_DIV_SHIFT;
		ddr_div = (clockreg & HORNET_CLOCK_CONTROL_DDR_POST_DIV_MASK) >> HORNET_CLOCK_CONTROL_DDR_POST_DIV_SFIFT;
		ahb_div = (clockreg & HORNET_CLOCK_CONTROL_AHB_POST_DIV_MASK) >> HORNET_CLOCK_CONTROL_AHB_POST_DIV_SFIFT;

		/*
		 * b00 : div by 1, b01 : div by 2, b10 : div by 3, b11 : div by 4
		 */
		cpu_div++;
		ddr_div++;
		ahb_div++;
	}

	ar7240_cpu_freq = pll_freq / cpu_div;
	ar7240_ddr_freq = pll_freq / ddr_div;
	ar7240_ahb_freq = pll_freq / ahb_div;
#endif
}
#endif
