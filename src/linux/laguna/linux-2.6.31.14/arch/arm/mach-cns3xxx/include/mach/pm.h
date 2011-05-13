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

#ifndef _CNS3XXX_PM_H_
#define  _CNS3XXX_PM_H_
#include <mach/board.h>
#define PMU_REG_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_PM_BASE_VIRT+offset)))

#define PM_CLK_GATE_REG				PMU_REG_VALUE(0x000)
#define PM_SOFT_RST_REG				PMU_REG_VALUE(0x004)
#define PM_HS_CFG_REG				PMU_REG_VALUE(0x008)
#define PM_CACTIVE_STA_REG			PMU_REG_VALUE(0x00C)
#define PM_PWR_STA_REG				PMU_REG_VALUE(0x010)
#define PM_CLK_CTRL_REG				PMU_REG_VALUE(0x014)
#define PM_PLL_LCD_I2S_CTRL_REG		PMU_REG_VALUE(0x018)
#define PM_PLL_HM_PD_CTRL_REG		PMU_REG_VALUE(0x01C)
#define PM_REGULAT_CTRL_REG			PMU_REG_VALUE(0x020)
#define PM_WDT_CTRL_REG				PMU_REG_VALUE(0x024)
#define PM_WU_CTRL0_REG				PMU_REG_VALUE(0x028)
#define PM_WU_CTRL1_REG				PMU_REG_VALUE(0x02C)
#define PM_CSR_REG					PMU_REG_VALUE(0x030)

/* PM_CLK_GATE_REG */
#define PM_CLK_GATE_REG_OFFSET_SDIO				(25)
#define PM_CLK_GATE_REG_OFFSET_GPU				(24)
#define PM_CLK_GATE_REG_OFFSET_CIM				(23)
#define PM_CLK_GATE_REG_OFFSET_LCDC				(22)
#define PM_CLK_GATE_REG_OFFSET_I2S				(21)
#define PM_CLK_GATE_REG_OFFSET_RAID				(20)
#define PM_CLK_GATE_REG_OFFSET_SATA				(19)
#define PM_CLK_GATE_REG_OFFSET_PCIE0			(17)
#define PM_CLK_GATE_REG_OFFSET_PCIE1			(18)
#define PM_CLK_GATE_REG_OFFSET_USB_HOST			(16)
#define PM_CLK_GATE_REG_OFFSET_USB_OTG			(15)
#define PM_CLK_GATE_REG_OFFSET_TIMER			(14)
#define PM_CLK_GATE_REG_OFFSET_CRYPTO			(13)
#define PM_CLK_GATE_REG_OFFSET_HCIE				(12)
#define PM_CLK_GATE_REG_OFFSET_SWITCH			(11)
#define PM_CLK_GATE_REG_OFFSET_GPIO				(10)
#define PM_CLK_GATE_REG_OFFSET_UART3			(9)
#define PM_CLK_GATE_REG_OFFSET_UART2			(8)
#define PM_CLK_GATE_REG_OFFSET_UART1			(7)
#define PM_CLK_GATE_REG_OFFSET_RTC				(5)
#define PM_CLK_GATE_REG_OFFSET_GDMA				(4)
#define PM_CLK_GATE_REG_OFFSET_SPI_PCM_I2C		(3)
#define PM_CLK_GATE_REG_OFFSET_SMC_NFI			(1)
#define PM_CLK_GATE_REG_MASK				(0x03FFFFBA)

/* PM_SOFT_RST_REG */
#define PM_SOFT_RST_REG_OFFST_WARM_RST_FLAG		(31) 
#define PM_SOFT_RST_REG_OFFST_CPU1				(29) 
#define PM_SOFT_RST_REG_OFFST_CPU0				(28)
#define PM_SOFT_RST_REG_OFFST_SDIO				(25)
#define PM_SOFT_RST_REG_OFFST_GPU				(24)
#define PM_SOFT_RST_REG_OFFST_CIM				(23)
#define PM_SOFT_RST_REG_OFFST_LCDC				(22)
#define PM_SOFT_RST_REG_OFFST_I2S				(21)
#define PM_SOFT_RST_REG_OFFST_RAID				(20)
#define PM_SOFT_RST_REG_OFFST_SATA				(19)
#define PM_SOFT_RST_REG_OFFST_PCIE1				(18)
#define PM_SOFT_RST_REG_OFFST_PCIE0				(17)
#define PM_SOFT_RST_REG_OFFST_USB_HOST			(16)
#define PM_SOFT_RST_REG_OFFST_USB_OTG			(15)
#define PM_SOFT_RST_REG_OFFST_TIMER				(14)
#define PM_SOFT_RST_REG_OFFST_CRYPTO			(13)
#define PM_SOFT_RST_REG_OFFST_HCIE				(12)
#define PM_SOFT_RST_REG_OFFST_SWITCH			(11)
#define PM_SOFT_RST_REG_OFFST_GPIO				(10)
#define PM_SOFT_RST_REG_OFFST_UART3				(9)
#define PM_SOFT_RST_REG_OFFST_UART2				(8)
#define PM_SOFT_RST_REG_OFFST_UART1				(7)
#define PM_SOFT_RST_REG_OFFST_RTC				(5)
#define PM_SOFT_RST_REG_OFFST_GDMA				(4)
#define PM_SOFT_RST_REG_OFFST_SPI_PCM_I2C		(3)
#define PM_SOFT_RST_REG_OFFST_DMC				(2)
#define PM_SOFT_RST_REG_OFFST_SMC_NFI			(1)
#define PM_SOFT_RST_REG_OFFST_GLOBAL			(0)
#define PM_SOFT_RST_REG_MASK				(0xF3FFFFBF)

/* PMHS_CFG_REG */
#define PM_HS_CFG_REG_OFFSET_SDIO				(25)
#define PM_HS_CFG_REG_OFFSET_GPU				(24)
#define PM_HS_CFG_REG_OFFSET_CIM				(23)
#define PM_HS_CFG_REG_OFFSET_LCDC				(22)
#define PM_HS_CFG_REG_OFFSET_I2S				(21)
#define PM_HS_CFG_REG_OFFSET_RAID				(20)
#define PM_HS_CFG_REG_OFFSET_SATA				(19)
#define PM_HS_CFG_REG_OFFSET_PCIE1				(18)
#define PM_HS_CFG_REG_OFFSET_PCIE0				(17)
#define PM_HS_CFG_REG_OFFSET_USB_HOST			(16)
#define PM_HS_CFG_REG_OFFSET_USB_OTG			(15)
#define PM_HS_CFG_REG_OFFSET_TIMER				(14)
#define PM_HS_CFG_REG_OFFSET_CRYPTO				(13)
#define PM_HS_CFG_REG_OFFSET_HCIE				(12)
#define PM_HS_CFG_REG_OFFSET_SWITCH				(11)
#define PM_HS_CFG_REG_OFFSET_GPIO				(10)
#define PM_HS_CFG_REG_OFFSET_UART3				(9)
#define PM_HS_CFG_REG_OFFSET_UART2				(8)
#define PM_HS_CFG_REG_OFFSET_UART1				(7)
#define PM_HS_CFG_REG_OFFSET_RTC				(5)
#define PM_HS_CFG_REG_OFFSET_GDMA				(4)
#define PM_HS_CFG_REG_OFFSET_SPI_PCM_I2S		(3)
#define PM_HS_CFG_REG_OFFSET_DMC				(2)
#define PM_HS_CFG_REG_OFFSET_SMC_NFI			(1)
#define PM_HS_CFG_REG_MASK					(0x03FFFFBE)
#define PM_HS_CFG_REG_MASK_SUPPORT			(0x01100806)

/* PM_CACTIVE_STA_REG */
#define PM_CACTIVE_STA_REG_OFFSET_SDIO				(25)
#define PM_CACTIVE_STA_REG_OFFSET_GPU				(24)
#define PM_CACTIVE_STA_REG_OFFSET_CIM				(23)
#define PM_CACTIVE_STA_REG_OFFSET_LCDC				(22)
#define PM_CACTIVE_STA_REG_OFFSET_I2S				(21)
#define PM_CACTIVE_STA_REG_OFFSET_RAID				(20)
#define PM_CACTIVE_STA_REG_OFFSET_SATA				(19)
#define PM_CACTIVE_STA_REG_OFFSET_PCIE1				(18)
#define PM_CACTIVE_STA_REG_OFFSET_PCIE0				(17)
#define PM_CACTIVE_STA_REG_OFFSET_USB_HOST			(16)
#define PM_CACTIVE_STA_REG_OFFSET_USB_OTG			(15)
#define PM_CACTIVE_STA_REG_OFFSET_TIMER				(14)
#define PM_CACTIVE_STA_REG_OFFSET_CRYPTO			(13)
#define PM_CACTIVE_STA_REG_OFFSET_HCIE				(12)
#define PM_CACTIVE_STA_REG_OFFSET_SWITCH			(11)
#define PM_CACTIVE_STA_REG_OFFSET_GPIO				(10)
#define PM_CACTIVE_STA_REG_OFFSET_UART3				(9)
#define PM_CACTIVE_STA_REG_OFFSET_UART2				(8)
#define PM_CACTIVE_STA_REG_OFFSET_UART1				(7)
#define PM_CACTIVE_STA_REG_OFFSET_RTC				(5)
#define PM_CACTIVE_STA_REG_OFFSET_GDMA				(4)
#define PM_CACTIVE_STA_REG_OFFSET_SPI_PCM_I2S		(3)
#define PM_CACTIVE_STA_REG_OFFSET_DMC				(2)
#define PM_CACTIVE_STA_REG_OFFSET_SMC_NFI			(1)
#define PM_CACTIVE_STA_REG_MASK					(0x03FFFFBE)

/* PM_PWR_STA_REG */
#define PM_PWR_STA_REG_REG_OFFSET_SDIO				(25)
#define PM_PWR_STA_REG_REG_OFFSET_GPU				(24)
#define PM_PWR_STA_REG_REG_OFFSET_CIM				(23)
#define PM_PWR_STA_REG_REG_OFFSET_LCDC				(22)
#define PM_PWR_STA_REG_REG_OFFSET_I2S				(21)
#define PM_PWR_STA_REG_REG_OFFSET_RAID				(20)
#define PM_PWR_STA_REG_REG_OFFSET_SATA				(19)
#define PM_PWR_STA_REG_REG_OFFSET_PCIE1				(18)
#define PM_PWR_STA_REG_REG_OFFSET_PCIE0				(17)
#define PM_PWR_STA_REG_REG_OFFSET_USB_HOST			(16)
#define PM_PWR_STA_REG_REG_OFFSET_USB_OTG			(15)
#define PM_PWR_STA_REG_REG_OFFSET_TIMER				(14)
#define PM_PWR_STA_REG_REG_OFFSET_CRYPTO			(13)
#define PM_PWR_STA_REG_REG_OFFSET_HCIE				(12)
#define PM_PWR_STA_REG_REG_OFFSET_SWITCH			(11)
#define PM_PWR_STA_REG_REG_OFFSET_GPIO				(10)
#define PM_PWR_STA_REG_REG_OFFSET_UART3				(9)
#define PM_PWR_STA_REG_REG_OFFSET_UART2				(8)
#define PM_PWR_STA_REG_REG_OFFSET_UART1				(7)
#define PM_PWR_STA_REG_REG_OFFSET_RTC				(5)
#define PM_PWR_STA_REG_REG_OFFSET_GDMA				(4)
#define PM_PWR_STA_REG_REG_OFFSET_SPI_PCM_I2S		(3)
#define PM_PWR_STA_REG_REG_OFFSET_DMC				(2)
#define PM_PWR_STA_REG_REG_OFFSET_SMC_NFI			(1)
#define PM_PWR_STA_REG_REG_MASK					(0x03FFFFBE)

/* PM_CLK_CTRL_REG */
#define PM_CLK_CTRL_REG_OFFSET_I2S_MCLK			(31)
#define PM_CLK_CTRL_REG_OFFSET_DDR2_CHG_EN		(30)
#define PM_CLK_CTRL_REG_OFFSET_PCIE_REF1_EN		(29)
#define PM_CLK_CTRL_REG_OFFSET_PCIE_REF0_EN		(28)
#define PM_CLK_CTRL_REG_OFFSET_TIMER_SIM_MODE	(27)
#define PM_CLK_CTRL_REG_OFFSET_I2SCLK_DIV		(24)
#define PM_CLK_CTRL_REG_OFFSET_I2SCLK_SEL		(22)
#define PM_CLK_CTRL_REG_OFFSET_CLKOUT_DIV		(20)
#define PM_CLK_CTRL_REG_OFFSET_CLKOUT_SEL		(16)
#define PM_CLK_CTRL_REG_OFFSET_MDC_DIV			(14)
#define PM_CLK_CTRL_REG_OFFSET_CRYPTO_CLK_SEL	(12)
#define PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE		(9)
#define PM_CLK_CTRL_REG_OFFSET_PLL_DDR2_SEL		(7)
#define PM_CLK_CTRL_REG_OFFSET_DIV_IMMEDIATE	(6)
#define PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV		(4)
#define PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL		(0)

#define PM_CPU_CLK_DIV(DIV) { \
	PM_CLK_CTRL_REG &= ~((0x3) << PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV); \
	PM_CLK_CTRL_REG |= (((DIV)&0x3) << PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV); \
}

#define PM_PLL_CPU_SEL(CPU) { \
	PM_CLK_CTRL_REG &= ~((0xF) << PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL); \
	PM_CLK_CTRL_REG |= (((CPU)&0xF) << PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL); \
}
	
/* PM_PLL_LCD_I2S_CTRL_REG */
#define PM_PLL_LCD_I2S_CTRL_REG_OFFSET_MCLK_SMC_DIV	(22)
#define PM_PLL_LCD_I2S_CTRL_REG_OFFSET_R_SEL		(17)
#define PM_PLL_LCD_I2S_CTRL_REG_OFFSET_PLL_LCD_P	(11)
#define PM_PLL_LCD_I2S_CTRL_REG_OFFSET_PLL_LCD_M	(3)
#define PM_PLL_LCD_I2S_CTRL_REG_OFFSET_PLL_LCD_S	(0)

/* PM_PLL_HM_PD_CTRL_REG */
/*
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PCIE_PHY1		(13)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PCIE_PHY0		(12)
*/
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_SATA_PHY1		(11)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_SATA_PHY0		(10)
/*
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_USB_PHY1		(9)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_USB_PHY0		(8)
*/
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_I2SCD		(6)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_I2S		(5)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_LCD		(4)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB		(3)
#define PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_RGMII		(2)
#define PM_PLL_HM_PD_CTRL_REG_MASK				(0x00000C7C)

/* PM_REGULAT_CTRL_REG */

/* PM_WDT_CTRL_REG */
#define PM_WDT_CTRL_REG_OFFSET_RESET_CPU_ONLY		(0)

/* PM_WU_CTRL0_REG */

/* PM_WU_CTRL1_REG */

/* PM_CSR_REG - Clock Scaling Register*/
#define PM_CSR_REG_OFFSET_CSR_EN		(30)
#define PM_CSR_REG_OFFSET_CSR_NUM		(0)


#define CNS3XXX_PWR_CLK_EN(BLOCK) (0x1<<PM_CLK_GATE_REG_OFFSET_##BLOCK)

/* Software reset*/
#define CNS3XXX_PWR_SOFTWARE_RST(BLOCK) (0x1<<PM_SOFT_RST_REG_OFFST_##BLOCK)



/* CNS3XXX support several power saving mode as following,
 * DFS, IDLE, HALT, DOZE, SLEEP, Hibernate
 */
#define CNS3XXX_PWR_CPU_MODE_DFS		(0)
#define CNS3XXX_PWR_CPU_MODE_IDLE		(1)
#define CNS3XXX_PWR_CPU_MODE_HALT		(2)
#define CNS3XXX_PWR_CPU_MODE_DOZE		(3)
#define CNS3XXX_PWR_CPU_MODE_SLEEP		(4)
#define CNS3XXX_PWR_CPU_MODE_HIBERNATE	(5)


/* Enable functional block */
#if 0
#define CNS3XXX_PWR_PLL_PCIE_PHY1	(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PCIE_PHY1)
#define CNS3XXX_PWR_PLL_PCIE_PHY0	(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PCIE_PHY0)
#define CNS3XXX_PWR_PLL_SATA_PHY1	(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_SATA_PHY1)
#define CNS3XXX_PWR_PLL_SATA_PHY0	(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_USB_PHY0)
#define CNS3XXX_PWR_PLL_USB_PHY1	(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_USB_PHY1)
#define CNS3XXX_PWR_PLL_USB_PHY0	(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_USB_PHY0)
#define CNS3XXX_PWR_PLL_I2SCD		(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_I2SCD)
#define CNS3XXX_PWR_PLL_I2S			(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_I2S)
#define CNS3XXX_PWR_PLL_LCD			(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_LCD)
#define CNS3XXX_PWR_PLL_USB			(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB)
#define CNS3XXX_PWR_PLL_RGMII		(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_RGMII)
#else
#define CNS3XXX_PWR_PLL(BLOCK)	(0x1<<PM_PLL_HM_PD_CTRL_REG_OFFSET_##BLOCK)
#endif
#define CNS3XXX_PWR_PLL_ALL			PM_PLL_HM_PD_CTRL_REG_MASK

void cns3xxx_pwr_power_up(unsigned int dev_num);
void cns3xxx_pwr_power_down(unsigned int dev_num);


/* Change CPU frequency and divider */
#define CNS3XXX_PWR_PLL_CPU_300MHZ		(0)
#define CNS3XXX_PWR_PLL_CPU_333MHZ		(1)
#define CNS3XXX_PWR_PLL_CPU_366MHZ		(2)
#define CNS3XXX_PWR_PLL_CPU_400MHZ		(3)
#define CNS3XXX_PWR_PLL_CPU_433MHZ		(4)
#define CNS3XXX_PWR_PLL_CPU_466MHZ		(5)
#define CNS3XXX_PWR_PLL_CPU_500MHZ		(6)
#define CNS3XXX_PWR_PLL_CPU_533MHZ		(7)
#define CNS3XXX_PWR_PLL_CPU_566MHZ		(8)
#define CNS3XXX_PWR_PLL_CPU_600MHZ		(9)
#define CNS3XXX_PWR_PLL_CPU_633MHZ		(10)
#define CNS3XXX_PWR_PLL_CPU_666MHZ		(11)
#define CNS3XXX_PWR_PLL_CPU_700MHZ		(12)

#define CNS3XXX_PWR_CPU_CLK_DIV_BY1		(0) 
#define CNS3XXX_PWR_CPU_CLK_DIV_BY2		(1)
#define CNS3XXX_PWR_CPU_CLK_DIV_BY4	 	(2)


void cns3xxx_pwr_change_pll_cpu(unsigned int cpu_sel);



/* Change DDR2 frequency */
#define CNS3XXX_PWR_PLL_DDR2_200MHZ		(0)
#define CNS3XXX_PWR_PLL_DDR2_266MHZ		(1)
#define CNS3XXX_PWR_PLL_DDR2_333MHZ		(2)
#define CNS3XXX_PWR_PLL_DDR2_400MHZ		(3)

/* Clock enable*/
void cns3xxx_pwr_clk_en(unsigned int block);
/* Clock disable*/
void cns3xxx_pwr_clk_disable(unsigned int block);
/* Software reset*/
void cns3xxx_pwr_soft_rst(unsigned int block);
void cns3xxx_pwr_soft_rst_force(unsigned int block);
/* PLL/Hard macro */
void cns3xxx_pwr_power_up(unsigned int dev_num);
void cns3xxx_pwr_power_down(unsigned int dev_num);
/* Change CPU clock */
void cns3xxx_pwr_change_cpu_clock(unsigned int cpu_sel, unsigned int div_sel);
/* System enter into sleep mode */
void cns3xxx_pwr_sleep(void);

int cns3xxx_cpu_clock(void);
#endif
