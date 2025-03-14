/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (c) 2021 Dávid Virág
 *
 * Device Tree binding constants for Exynos7885 clock controller.
 */

#ifndef _DT_BINDINGS_CLOCK_EXYNOS_7885_H
#define _DT_BINDINGS_CLOCK_EXYNOS_7885_H

/* CMU_TOP */
#define CLK_FOUT_SHARED0_PLL		1
#define CLK_FOUT_SHARED1_PLL		2
#define CLK_DOUT_SHARED0_DIV2		3
#define CLK_DOUT_SHARED0_DIV3		4
#define CLK_DOUT_SHARED0_DIV4		5
#define CLK_DOUT_SHARED0_DIV5		6
#define CLK_DOUT_SHARED1_DIV2		7
#define CLK_DOUT_SHARED1_DIV3		8
#define CLK_DOUT_SHARED1_DIV4		9
#define CLK_MOUT_CORE_BUS		10
#define CLK_MOUT_CORE_CCI		11
#define CLK_MOUT_CORE_G3D		12
#define CLK_DOUT_CORE_BUS		13
#define CLK_DOUT_CORE_CCI		14
#define CLK_DOUT_CORE_G3D		15
#define CLK_GOUT_CORE_BUS		16
#define CLK_GOUT_CORE_CCI		17
#define CLK_GOUT_CORE_G3D		18
#define CLK_MOUT_PERI_BUS		19
#define CLK_MOUT_PERI_SPI0		20
#define CLK_MOUT_PERI_SPI1		21
#define CLK_MOUT_PERI_UART0		22
#define CLK_MOUT_PERI_UART1		23
#define CLK_MOUT_PERI_UART2		24
#define CLK_MOUT_PERI_USI0		25
#define CLK_MOUT_PERI_USI1		26
#define CLK_MOUT_PERI_USI2		27
#define CLK_DOUT_PERI_BUS		28
#define CLK_DOUT_PERI_SPI0		29
#define CLK_DOUT_PERI_SPI1		30
#define CLK_DOUT_PERI_UART0		31
#define CLK_DOUT_PERI_UART1		32
#define CLK_DOUT_PERI_UART2		33
#define CLK_DOUT_PERI_USI0		34
#define CLK_DOUT_PERI_USI1		35
#define CLK_DOUT_PERI_USI2		36
#define CLK_GOUT_PERI_BUS		37
#define CLK_GOUT_PERI_SPI0		38
#define CLK_GOUT_PERI_SPI1		39
#define CLK_GOUT_PERI_UART0		40
#define CLK_GOUT_PERI_UART1		41
#define CLK_GOUT_PERI_UART2		42
#define CLK_GOUT_PERI_USI0		43
#define CLK_GOUT_PERI_USI1		44
#define CLK_GOUT_PERI_USI2		45
#define CLK_MOUT_FSYS_BUS		46
#define CLK_MOUT_FSYS_MMC_CARD		47
#define CLK_MOUT_FSYS_MMC_EMBD		48
#define CLK_MOUT_FSYS_MMC_SDIO		49
#define CLK_MOUT_FSYS_USB30DRD		50
#define CLK_DOUT_FSYS_BUS		51
#define CLK_DOUT_FSYS_MMC_CARD		52
#define CLK_DOUT_FSYS_MMC_EMBD		53
#define CLK_DOUT_FSYS_MMC_SDIO		54
#define CLK_DOUT_FSYS_USB30DRD		55
#define CLK_GOUT_FSYS_BUS		56
#define CLK_GOUT_FSYS_MMC_CARD		57
#define CLK_GOUT_FSYS_MMC_EMBD		58
#define CLK_GOUT_FSYS_MMC_SDIO		59
#define CLK_GOUT_FSYS_USB30DRD		60
#define TOP_NR_CLK			61

/* CMU_CORE */
#define CLK_MOUT_CORE_BUS_USER			1
#define CLK_MOUT_CORE_CCI_USER			2
#define CLK_MOUT_CORE_G3D_USER			3
#define CLK_MOUT_CORE_GIC			4
#define CLK_DOUT_CORE_BUSP			5
#define CLK_GOUT_CCI_ACLK			6
#define CLK_GOUT_GIC400_CLK			7
#define CLK_GOUT_TREX_D_CORE_ACLK		8
#define CLK_GOUT_TREX_D_CORE_GCLK		9
#define CLK_GOUT_TREX_D_CORE_PCLK		10
#define CLK_GOUT_TREX_P_CORE_ACLK_P_CORE	11
#define CLK_GOUT_TREX_P_CORE_CCLK_P_CORE	12
#define CLK_GOUT_TREX_P_CORE_PCLK		13
#define CLK_GOUT_TREX_P_CORE_PCLK_P_CORE	14
#define CORE_NR_CLK				15

/* CMU_PERI */
#define CLK_MOUT_PERI_BUS_USER		1
#define CLK_MOUT_PERI_SPI0_USER		2
#define CLK_MOUT_PERI_SPI1_USER		3
#define CLK_MOUT_PERI_UART0_USER	4
#define CLK_MOUT_PERI_UART1_USER	5
#define CLK_MOUT_PERI_UART2_USER	6
#define CLK_MOUT_PERI_USI0_USER		7
#define CLK_MOUT_PERI_USI1_USER		8
#define CLK_MOUT_PERI_USI2_USER		9
#define CLK_GOUT_GPIO_TOP_PCLK		10
#define CLK_GOUT_HSI2C0_PCLK		11
#define CLK_GOUT_HSI2C1_PCLK		12
#define CLK_GOUT_HSI2C2_PCLK		13
#define CLK_GOUT_HSI2C3_PCLK		14
#define CLK_GOUT_I2C0_PCLK		15
#define CLK_GOUT_I2C1_PCLK		16
#define CLK_GOUT_I2C2_PCLK		17
#define CLK_GOUT_I2C3_PCLK		18
#define CLK_GOUT_I2C4_PCLK		19
#define CLK_GOUT_I2C5_PCLK		20
#define CLK_GOUT_I2C6_PCLK		21
#define CLK_GOUT_I2C7_PCLK		22
#define CLK_GOUT_PWM_MOTOR_PCLK		23
#define CLK_GOUT_SPI0_PCLK		24
#define CLK_GOUT_SPI0_EXT_CLK		25
#define CLK_GOUT_SPI1_PCLK		26
#define CLK_GOUT_SPI1_EXT_CLK		27
#define CLK_GOUT_UART0_EXT_UCLK		28
#define CLK_GOUT_UART0_PCLK		29
#define CLK_GOUT_UART1_EXT_UCLK		30
#define CLK_GOUT_UART1_PCLK		31
#define CLK_GOUT_UART2_EXT_UCLK		32
#define CLK_GOUT_UART2_PCLK		33
#define CLK_GOUT_USI0_PCLK		34
#define CLK_GOUT_USI0_SCLK		35
#define CLK_GOUT_USI1_PCLK		36
#define CLK_GOUT_USI1_SCLK		37
#define CLK_GOUT_USI2_PCLK		38
#define CLK_GOUT_USI2_SCLK		39
#define CLK_GOUT_MCT_PCLK		40
#define CLK_GOUT_SYSREG_PERI_PCLK	41
#define CLK_GOUT_WDT0_PCLK		42
#define CLK_GOUT_WDT1_PCLK		43
#define PERI_NR_CLK			44

/* CMU_FSYS */
#define CLK_MOUT_FSYS_BUS_USER		1
#define CLK_MOUT_FSYS_MMC_CARD_USER	2
#define CLK_MOUT_FSYS_MMC_EMBD_USER	3
#define CLK_MOUT_FSYS_MMC_SDIO_USER	4
#define CLK_GOUT_MMC_CARD_ACLK		5
#define CLK_GOUT_MMC_CARD_SDCLKIN	6
#define CLK_GOUT_MMC_EMBD_ACLK		7
#define CLK_GOUT_MMC_EMBD_SDCLKIN	8
#define CLK_GOUT_MMC_SDIO_ACLK		9
#define CLK_GOUT_MMC_SDIO_SDCLKIN	10
#define CLK_MOUT_FSYS_USB30DRD_USER	11
#define FSYS_NR_CLK			12

#endif /* _DT_BINDINGS_CLOCK_EXYNOS_7885_H */
