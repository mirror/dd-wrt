/******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, Version 2, as
 *  published by the Free Software Foundation.
 *
 *  This file is distributed in the hope that it will be useful,
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 *  NONINFRINGEMENT.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this file; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or
 *  visit http://www.gnu.org/licenses/.
 *
 *  This file may also be available under a different license from Cavium.
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/
#include <linux/pm.h>
#include <linux/interrupt.h>
#include <mach/pm.h>
#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/proc_fs.h> 
#include <linux/delay.h>
#include <mach/misc.h>

/*
 * cns3xxx_pwr_clk_en - clock enable 
 * @block: bitmap for peripheral
 */
void cns3xxx_pwr_clk_en(unsigned int block)
{
	PM_CLK_GATE_REG |= (block&PM_CLK_GATE_REG_MASK);
}

void cns3xxx_pwr_clk_disable(unsigned int block)
{
	PM_CLK_GATE_REG &= ~(block&PM_CLK_GATE_REG_MASK);
}

/*
 * cns3xxx_pwr_soft_rst - software reset
 * @block: bitmap for peripheral
 */
void cns3xxx_pwr_soft_rst_force(unsigned int block)
{
	/* bit 0, 28, 29 => program low to reset, 
	 * the other else program low and then high
	 */
	if (block & 0x30000001) {
		PM_SOFT_RST_REG &= ~(block&PM_SOFT_RST_REG_MASK);
	} else {
		PM_SOFT_RST_REG &= ~(block&PM_SOFT_RST_REG_MASK);
		PM_SOFT_RST_REG |= (block&PM_SOFT_RST_REG_MASK);
	}
}

void cns3xxx_pwr_soft_rst(unsigned int block)
{
	static unsigned int soft_reset = 0;

	if(soft_reset & block) {
		//Because SPI/I2C/GPIO use the same block, just only reset once...
		return;
	}
	else {
		soft_reset |= block;
	}
	cns3xxx_pwr_soft_rst_force(block);
}	
	
/*
 * void cns3xxx_pwr_lp_hs - lower power handshake
 * @dev: bitmap for device 
 * 	
 */
void cns3xxx_lp_hs(unsigned int dev)
{
	
	if (PM_HS_CFG_REG_MASK_SUPPORT & dev) {
		PM_HS_CFG_REG |= dev;

		/* TODO: disable clock */
	}
}

/*
 * cns3xxx_pwr_mode - change CPU power mode
 * @pwr_mode: CPU power mode
 * CNS3XXX_PWR_CPU_MODE_DFS, CNS3XXX_PWR_CPU_MODE_IDLE
 * CNS3XXX_PWR_CPU_MODE_HALT, CNS3XXX_PWR_CPU_MODE_DOZE
 * CNS3XXX_PWR_CPU_MODE_SLEEP, CNS3XXX_PWR_CPU_MODE_HIBERNATE
 */
static void cns3xxx_pwr_mode(unsigned int pwr_mode)
{
	if (CNS3XXX_PWR_CPU_MODE_HIBERNATE < pwr_mode) {
		return;
	}

	PM_CLK_CTRL_REG &= 
			~(0x7<<PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE);
	PM_CLK_CTRL_REG |= 
			((pwr_mode&0x7)<<PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE);
};

/* cns3xxx_pwr_power_up - 
 * cns3xxx_pwr_power_down - 
 * @dev_num: bitmap for functional block
 * 	CNS3XXX_PWR_PLL_PCIE_PHY1, CNS3XXX_PWR_PLL_PCIE_PHY0
 *	CNS3XXX_PWR_PLL_SATA_PHY1, CNS3XXX_PWR_PLL_SATA_PHY0
 * 	CNS3XXX_PWR_PLL_USB_PHY1, CNS3XXX_PWR_PLL_USB_PHY0
 * 	CNS3XXX_PWR_PLL_I2SCD, CNS3XXX_PWR_PLL_I2S
 * 	CNS3XXX_PWR_PLL_LCD, CNS3XXX_PWR_PLL_USB
 * 	CNS3XXX_PWR_PLL_RGMII, CNS3XXX_PWR_PLL_ALL
 */
void cns3xxx_pwr_power_up(unsigned int dev_num) 
{
	PM_PLL_HM_PD_CTRL_REG &= ~(dev_num & CNS3XXX_PWR_PLL_ALL);

	/* TODO: wait for 300us for the PLL output clock locked */
};

void cns3xxx_pwr_power_down(unsigned int dev_num)
{
	/* write '1' to power down */
	PM_PLL_HM_PD_CTRL_REG |= (dev_num & CNS3XXX_PWR_PLL_ALL);
};

#if 0
/* cns3xxx_pwr_change_pll_ddr - change DDR2 frequency
 * @ddr_sel: DDR2 clock select
 * 	CNS3XXX_PWR_PLL_DDR2_200MHZ
 * 	CNS3XXX_PWR_PLL_DDR2_266MHZ
 * 	CNS3XXX_PWR_PLL_DDR2_333MHZ
 * 	CNS3XXX_PWR_PLL_DDR2_400MHZ
 */
void cns3xxx_pwr_change_pll_ddr(unsigned int ddr_sel)
{
	if (CNS3XXX_PWR_PLL_DDR2_400MHZ < ddr_sel) {
		return;
	}
	
	PM_CLK_CTRL_REG &= ~(0x3 << PM_CLK_CTRL_REG_OFFSET_PLL_DDR2_SEL);
	PM_CLK_CTRL_REG |= (ddr_sel << PM_CLK_CTRL_REG_OFFSET_PLL_DDR2_SEL);
}
#endif

#define GIC_REG_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT+offset)))


/* Change CPU frequency and divider */
/*
 * cns3xxx_pwr_change_pll_cpu - change PLL CPU frequency
 * @cpu_sel: PLL CPU frequency
 * @div_sel: divider
 *
 * This feature requires that 2nd core is in WFI mode and L2 cache is disabled
 * Before calling this function, please make sure that L2 cache is not in use
 *  
 */
void cns3xxx_pwr_change_cpu_clock(unsigned int cpu_sel, unsigned int div_sel)
{
	/* 1. Set PLL_CPU_SEL
	 * 2. Set in DFS mode
	 * 3. disable all interrupt except interrupt ID-32 (clkscale_intr)
	 * 4. Let CPU enter into WFI state
	 * 5. Wait PMU to change PLL_CPU and divider and wake up CPU 
 	 */	
	int old_cpu, old_div;


	/* sanity check */
	if ((CNS3XXX_PWR_PLL_CPU_700MHZ < cpu_sel) 
			|| (CNS3XXX_PWR_CPU_CLK_DIV_BY4 < div_sel)) {
		return;
	}

	old_cpu = (PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) &0xf;
	old_div = (PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3;

	if ((cpu_sel == old_cpu) 
			&& (div_sel == old_div)) {
		return;
	}
		
	/* 1. Set PLL_CPU_SEL */
	PM_PLL_CPU_SEL(cpu_sel);
	PM_CPU_CLK_DIV(div_sel);

	/* 2. Set in DFS mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);

	/* 3. disable all interrupt except interrupt ID-32 (clkscale_intr) */ 
	/* disable all interrupt */
	GIC_REG_VALUE(0x184) = 0xffffffff;
	GIC_REG_VALUE(0x188) = 0xffffffff;
	/* enable interrupt id 32*/
	GIC_REG_VALUE(0x104) = 0x00000001;
	GIC_REG_VALUE(0x108) = 0x80000000;

	/* 4. Let CPU enter into WFI state */	
	asm volatile(
			"mov r0, #0\n"
			"mcr p15, 0, r0, c7, c0, 4\n"
			);
	

#if 0 
	{
		int i;
		for (i=IRQ_CNS3XXX_PMU+1; i<IRQ_CNS3XXX_EXTERNAL_PIN0; i++) {
			enable_irq(i);	
		}
	}
#else
	GIC_REG_VALUE(0x104) = 0xffffffff;
	GIC_REG_VALUE(0x108) = 0xffffffff;
#endif

	{
	/* for timer, because CPU clock is changed */
		int pclk = (cns3xxx_cpu_clock() >> 3);
		*(volatile unsigned int *) (CNS3XXX_TIMER1_2_3_BASE_VIRT + TIMER1_AUTO_RELOAD_OFFSET)
				= pclk/15*0x25000;
	}

}


/*
 * clock_out_sel - select clock source to ClkOut pin
 * This function just select pll_cpu to ClkOut pin,
 * we can measure the ClkOut frequency to make sure whether pll_cpu is change
 *
 */
void clock_out_sel(void) 
{

	int temp = PM_CLK_CTRL_REG;
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 26); /* Set GPIOB26 to ClkOut*/
	/* debug purpose, use ext intr 1 and 2 to generate interrupt */
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 27); /* Set GPIOB27 to external interrupt 2*/
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 28); /* Set GPIOB28 to external interrupt 1*/
	/* select ClkOut source as pll_cpu_clk and ClkOut divider is by 16 */
	temp &=~(0x3 << 20);
	temp &=~(0xf << 16);
	temp |= (0x3 << 20); 
	temp |= (0x1 << 16);
	PM_CLK_CTRL_REG = temp;
}

void cns3xxx_wfi(void)
{
	mb();
	asm volatile(
			"mov r0, #0\n"
			"mcr p15, 0, r0, c7, c10, 4\n"
			"mcr p15, 0, r0, c7, c0, 4\n"
			);
}

/*
 * cns3xxx_pwr_sleep - 
 */
void cns3xxx_pwr_sleep(void)
{
	/* 1. Set in sleep mode
	 * 2. disable all functional block
	 * 3. make sure that all function block are in power off state
	 * 4. power down all PLL 
	 * 5. Let CPU enter into WFI state
	 * 6. Wait PMU to change PLL_CPU and divider and wake up CPU 
 	 */	
	int i, j, count = 0;
	/* 1. Set in SLEEP mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_SLEEP);

	/* 2. disable all functional block */
	i = PM_CLK_GATE_REG;
	PM_CLK_GATE_REG = 0x0;

	/* 3. make sure that all function block are in power off state */
	while (0x4 != PM_PWR_STA_REG) {
		count++;
		if (1000 == count) {
			count = PM_PWR_STA_REG;
			break;
		}
	};

	/* 4. power down all PLL */
	j = PM_PLL_HM_PD_CTRL_REG;
	PM_PLL_HM_PD_CTRL_REG = 0x00003FFC;

#if	0
    /* set DMC to low power hand shake */
    PM_HS_CFG_REG |= (0x1 << 2);
    /* disable DMC */
    PM_CLK_GATE_REG &= ~(0x1<<2);
#endif

	/* set wake up interrupt source, use ext_intr1 to wake up*/
	PM_WU_CTRL0_REG = 0x0; PM_WU_CTRL1_REG = 0x40000000;
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 27);

	/* 5. Let CPU enter into WFI state */
	GIC_REG_VALUE(0x104) = 0x1; /* enable clock scaling interrupt */
	printk("<0>enter WFI\n");
	cns3xxx_wfi();
	PM_CLK_GATE_REG = i;
	PM_PLL_HM_PD_CTRL_REG = j;
	printk("<0>leave WFI\n");
	GIC_REG_VALUE(0x104) = 0xffffffff;
	GIC_REG_VALUE(0x108) = 0xffffffff;
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);
}

/*
 * cns3xxx_pwr_sleep_test - enter into sleep and won't be wake up
 */
void cns3xxx_pwr_sleep_test(void)
{
	int i, j, count = 0;
	/* 1. Set in SLEEP mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_SLEEP);

	/* 2. disable all functional block */
	i = PM_CLK_GATE_REG;
	PM_CLK_GATE_REG = 0x0;

	/* 3. make sure that all function block are in power off state */
	while (0x4 != PM_PWR_STA_REG) {
		count++;
		if (1000 == count) {
			count = PM_PWR_STA_REG;
			break;
		}
	};
	/* 4. power down all PLL */
	j = PM_PLL_HM_PD_CTRL_REG;
	PM_PLL_HM_PD_CTRL_REG = 0x00003FFC;

	/* set wake up interrupt source, do nothing */
	PM_WU_CTRL0_REG = 0x0; PM_WU_CTRL1_REG = 0x00000000;

	/* 5. Let CPU enter into WFI state */
	GIC_REG_VALUE(0x104) = 0x1; /* enable clock scaling interrupt */
	printk("<0>enter WFI\n");
	cns3xxx_wfi();
	PM_CLK_GATE_REG = i;
	PM_PLL_HM_PD_CTRL_REG = j;
	printk("<0>leave WFI, count 0x%.8x\n", count);
	GIC_REG_VALUE(0x104) = 0xffffffff;
	GIC_REG_VALUE(0x108) = 0xffffffff;
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);
}

/*
 * cns3xxx_pwr_doze - 
 */
void cns3xxx_pwr_doze(void)
{
	/* 1. Set in doze mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DOZE);


	/* set wake up interrupt source*/
	PM_WU_CTRL0_REG = 0x0; PM_WU_CTRL1_REG = 0x40000000;
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 27);

	/* 5. Let CPU enter into WFI state */
	GIC_REG_VALUE(0x104) = 0x1; /* enable clock scaling interrupt */
	printk("<0>enter WFI\n");
	cns3xxx_wfi();
	printk("<0>leave WFI\n");
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);
}

/*
 * cns3xxx_pwr_idle -
 * IDLE mode just turn off CPU clock. 
 * L2 cache, peripheral, PLL, external DRAM and chip power are still on 
 */
void cns3xxx_pwr_idle(void)
{
	/* 1. Set in IDLE mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_IDLE);

#if 1
	/* disable all interrupt except interrupt ID-32 (clkscale_intr) 
 	 * 
 	 * CPU can be wake up by any interrupt here, 
 	 * we disable all interrupt is just for testing 
	 */

	/* disable all interrupt */
	GIC_REG_VALUE(0x184) = 0xffffffff; GIC_REG_VALUE(0x188) = 0xffffffff;
	/* enable interrupt id 32*/
	GIC_REG_VALUE(0x104) = 0x00000001; GIC_REG_VALUE(0x108) = 0x00000000;
#endif

	/* set wake up interrupt source*/
	PM_WU_CTRL0_REG = 0x0; PM_WU_CTRL1_REG = 0x40000000;
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 27);

	/* 5. Let CPU enter into WFI state */
	printk("<0>enter WFI\n");
	cns3xxx_wfi();
	printk("<0>leave WFI\n");
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);
	GIC_REG_VALUE(0x104) = 0xffffffff;
	GIC_REG_VALUE(0x108) = 0xffffffff;
}

/*
 * cns3xxx_pwr_halt - 
 * HALT mode just turn off CPU and L2 cache clock. 
 * peripheral, PLL, external DRAM and chip power are still on 
 */

void cns3xxx_pwr_halt(void)
{
	/* 1. Set in HALT mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_HALT);

 	/* 
 	 * CPU can be wake up by any interrupt here, 
 	 * for test, we disable all interrupt except ID-32
	 */
	/* disable all interrupt */
	GIC_REG_VALUE(0x184) = 0xffffffff; GIC_REG_VALUE(0x188) = 0xffffffff;
	/* enable interrupt id 32*/
	GIC_REG_VALUE(0x104) = 0x00000001; GIC_REG_VALUE(0x108) = 0x00000000;

	/* set wake up interrupt source to trigger clock scaling interrupt */
	PM_WU_CTRL0_REG = 0x0; PM_WU_CTRL1_REG = 0x40000000;
	//MISC_GPIOB_PIN_ENABLE_REG |= (0x1 << 27);

	/* 5. Let CPU enter into WFI state */
	cns3xxx_wfi();
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);
	GIC_REG_VALUE(0x104) = 0xffffffff;
	GIC_REG_VALUE(0x108) = 0xffffffff;
}

/*
 * cns3xxx_cpu_clock - return CPU/L2 clock
 *  aclk: cpu clock/2
 *  hclk: cpu clock/4
 *  pclk: cpu clock/8
 */
int cns3xxx_cpu_clock(void)
{
#define CPU_BASE 300
	int cpu, cpu_sel, div_sel;
	
	cpu_sel = (PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf;
	div_sel = (PM_CLK_CTRL_REG >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3;

	cpu = (CPU_BASE + ((cpu_sel/3) * 100) + ((cpu_sel %3) *33)) >> div_sel;
	return cpu;
}

static int __init cns3xxx_pmu_init(void)
{
	return 0;
}


EXPORT_SYMBOL(cns3xxx_pwr_power_up);
EXPORT_SYMBOL(cns3xxx_pwr_clk_en);
EXPORT_SYMBOL(cns3xxx_pwr_soft_rst);
EXPORT_SYMBOL(cns3xxx_pwr_soft_rst_force);
EXPORT_SYMBOL(cns3xxx_cpu_clock);

module_init(cns3xxx_pmu_init);
