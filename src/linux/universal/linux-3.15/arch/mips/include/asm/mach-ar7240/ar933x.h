/*
 * Copyright (c) 2008-2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _AR933X_H
#define _AR933X_H

#include <asm/addrspace.h>

#ifdef CONFIG_HORNET_EMULATION
#ifdef is_emu
#undef is_emu
#define is_emu()	 (1)
#endif
#endif
/*
 * Address map
 */
#define ATH_PCI_MEM_BASE		0x10000000	/* 128M */
#define ATH_APB_BASE			0x18000000	/* 384M */
#define ATH_GE0_BASE			0x19000000	/* 16M */
#define ATH_GE1_BASE			0x1a000000	/* 16M */
#define ATH_USB_OHCI_BASE		0x1b000000
#define ATH_USB_EHCI_BASE		0x1b000000
#define ATH_SPI_BASE			0x1f000000

/*
 * Added the PCI LCL RESET register from u-boot
 * ath_soc.h so that we can query the PCI LCL RESET
 * register for the presence of WLAN H/W.
 */
#define ATH_PCI_LCL_BASE		(ATH_APB_BASE+0x000f0000)
#define ATH_PCI_LCL_APP			(ATH_PCI_LCL_BASE+0x00)
#define ATH_PCI_LCL_RESET		(ATH_PCI_LCL_BASE+0x18)

/*
 * APB block
 */
#define ATH_DDR_CTL_BASE		ATH_APB_BASE+0x00000000
#define ATH_CPU_BASE			ATH_APB_BASE+0x00010000
#define ATH_UART_BASE			ATH_APB_BASE+0x00020000
#define ATH_USB_CONFIG_BASE		ATH_APB_BASE+0x00030000
#define ATH_GPIO_BASE			ATH_APB_BASE+0x00040000
#define ATH_PLL_BASE			ATH_APB_BASE+0x00050000
#define ATH_RESET_BASE			ATH_APB_BASE+0x00060000
#define ATH_DMA_BASE			ATH_APB_BASE+0x000A0000
#define ATH_SLIC_BASE			ATH_APB_BASE+0x000A9000
#define ATH_STEREO_BASE			ATH_APB_BASE+0x000B0000
#define ATH_PCI_CTLR_BASE		ATH_APB_BASE+0x000F0000


/*
 * PLL block/CPU
 */

/*
 * USB block
 */
#define ATH_USB_FLADJ_VAL		ATH_USB_CONFIG_BASE
#define ATH_USB_CONFIG			ATH_USB_CONFIG_BASE+0x4
#define ATH_USB_WINDOW			0x1000000
#define ATH_USB_MODE			ATH_USB_EHCI_BASE+0x1a8

/*
 * gpio configs
 */
#define ATH_GPIO_OE			ATH_GPIO_BASE+0x0
#define ATH_GPIO_IN			ATH_GPIO_BASE+0x4
#define ATH_GPIO_OUT			ATH_GPIO_BASE+0x8
#define ATH_GPIO_SET			ATH_GPIO_BASE+0xc
#define ATH_GPIO_CLEAR			ATH_GPIO_BASE+0x10
#define ATH_GPIO_INT_ENABLE		ATH_GPIO_BASE+0x14
#define ATH_GPIO_INT_TYPE		ATH_GPIO_BASE+0x18
#define ATH_GPIO_INT_POLARITY		ATH_GPIO_BASE+0x1c
#define ATH_GPIO_INT_PENDING		ATH_GPIO_BASE+0x20
#define ATH_GPIO_INT_MASK		ATH_GPIO_BASE+0x24
#define ATH_GPIO_IN_ETH_SWITCH_LED	ATH_GPIO_BASE+0x28
#define ATH_GPIO_OUT_FUNCTION0		ATH_GPIO_BASE+0x2c
#define ATH_GPIO_OUT_FUNCTION1		ATH_GPIO_BASE+0x30
#define ATH_GPIO_OUT_FUNCTION2		ATH_GPIO_BASE+0x34
#define ATH_GPIO_OUT_FUNCTION3		ATH_GPIO_BASE+0x38
#define ATH_GPIO_OUT_FUNCTION4		ATH_GPIO_BASE+0x3c
#define ATH_GPIO_OUT_FUNCTION5		ATH_GPIO_BASE+0x40
#define ATH_GPIO_IN_ENABLE0		ATH_GPIO_BASE+0x44
#define ATH_GPIO_IN_ENABLE1		ATH_GPIO_BASE+0x48
#define ATH_GPIO_IN_ENABLE2		ATH_GPIO_BASE+0x4c
#define ATH_GPIO_IN_ENABLE3		ATH_GPIO_BASE+0x50
#define ATH_GPIO_IN_ENABLE4		ATH_GPIO_BASE+0x54
#define ATH_GPIO_IN_ENABLE5		ATH_GPIO_BASE+0x58
#define ATH_GPIO_IN_ENABLE6		ATH_GPIO_BASE+0x5c
#define ATH_GPIO_IN_ENABLE7		ATH_GPIO_BASE+0x60
#define ATH_GPIO_IN_ENABLE8		ATH_GPIO_BASE+0x64
#define ATH_GPIO_IN_ENABLE9		ATH_GPIO_BASE+0x68
#define ATH_GPIO_FUNCTIONS		ATH_GPIO_BASE+0x6c

/*
 * GPIO Function Enables
 */
#define ATH_GPIO_FUNCTION_STEREO_EN	(1<<17)
#define ATH_GPIO_FUNCTION_SLIC_EN	(1<<16)

#define ATH_GPIO_FUNCTION_OVERCURRENT_EN		(1<< 4)
#define ATH_GPIO_FUNCTION_USB_CLK_CORE_EN		(1<< 0)
#define ATH_GPIO_FUNCTION_WMAC_LED			(1<<22)
#define ATH_GPIO_FUNCTION_STEREO_EN			(1<<17)
#define ATH_GPIO_FUNCTION_SLIC_EN			(1<<16)
#define ATH_GPIO_FUNCTION_SPDIF2TCK_EN			(1<<31)
#define ATH_GPIO_FUNCTION_SPDIF_EN			(1<<30)
#define ATH_GPIO_FUNCTION_I2S_GPIO_18_22_EN		(1<<29)
#define ATH_GPIO_FUNCTION_I2S_REFCLKEN			(1<<28)
#define ATH_GPIO_FUNCTION_I2S_MCKEN			(1<<27)
#define ATH_GPIO_FUNCTION_I2S0_EN			(1<<26)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED_DUPL_EN	(1<<25)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED_COLL		(1<<24)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED_ACTV		(1<<23)
#define ATH_GPIO_FUNCTION_PLL_SHIFT_EN			(1<<22)
#define ATH_GPIO_FUNCTION_EXT_MDIO_SEL			(1<<21)
#define ATH_GPIO_FUNCTION_CLK_OBS6_ENABLE		(1<<20)
#define ATH_GPIO_FUNCTION_CLK_OBS0_ENABLE		(1<<19)
#define ATH_GPIO_FUNCTION_SPI_EN			(1<<18)
#define ATH_GPIO_FUNCTION_DDR_DQOE_EN			(1<<17)
#define ATH_GPIO_FUNCTION_PCIEPHY_TST_EN		(1<<16)
#define ATH_GPIO_FUNCTION_S26_UART_DISABLE		(1<<15)
#define ATH_GPIO_FUNCTION_SPI_CS_1_EN			(1<<14)
#define ATH_GPIO_FUNCTION_SPI_CS_0_EN			(1<<13)
#define ATH_GPIO_FUNCTION_CLK_OBS5_ENABLE		(1<<12)
#define ATH_GPIO_FUNCTION_CLK_OBS4_ENABLE		(1<<11)
#define ATH_GPIO_FUNCTION_CLK_OBS3_ENABLE		(1<<10)
#define ATH_GPIO_FUNCTION_CLK_OBS2_ENABLE		(1<< 9)
#define ATH_GPIO_FUNCTION_CLK_OBS1_ENABLE		(1<< 8)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED4_EN		(1<< 7)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED3_EN		(1<< 6)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED2_EN		(1<< 5)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED1_EN		(1<< 4)
#define ATH_GPIO_FUNCTION_ETH_SWITCH_LED0_EN		(1<< 3)
#define ATH_GPIO_FUNCTION_UART_RTS_CTS_EN		(1<< 2)
#define ATH_GPIO_FUNCTION_UART_EN			(1<< 1)
#define ATH_GPIO_FUNCTION_2_EN_I2WS_ON_0		(1<< 4)
#define ATH_GPIO_FUNCTION_2_EN_I2SCK_ON_1		(1<< 3)
#define ATH_GPIO_FUNCTION_2_I2S_ON_LED			(1<< 1)
#define ATH_GPIO_FUNCTION_SRIF_ENABLE			(1<< 0)
#define ATH_GPIO_FUNCTION_JTAG_DISABLE			(1<< 1)
#define ATH_GPIO_FUNCTION_USB_LED			(1<< 4)
#define ATH_GPIO_FUNCTION_JTAG_DISABLE			(1<< 1)

#define ATH_GPIO_OE_EN(x)				(x)
#define ATH_GPIO_IN_ENABLE4_SLIC_PCM_FS_IN(x)		((0xff&x)<< 8)
#define ATH_GPIO_IN_ENABLE4_SLIC_DATA_IN(x)		(0xff&x)
#define ATH_GPIO_OUT_FUNCTION3_ENABLE_GPIO_15(x)	((0xff&x)<<24)
#define ATH_GPIO_OUT_FUNCTION3_ENABLE_GPIO_14(x)	((0xff&x)<<16)
#define ATH_GPIO_OUT_FUNCTION3_ENABLE_GPIO_13(x)	((0xff&x)<< 8)
#define ATH_GPIO_OUT_FUNCTION3_ENABLE_GPIO_12(x)	(0xff&x)
#define ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_11(x)	((0xff&x)<<24)
#define ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_10(x)	((0xff&x)<<16)
#define ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_9(x)		((0xff&x)<< 8)
#define ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_8(x)		(0xff&x)
#define ATH_GPIO_OUT_FUNCTION1_ENABLE_GPIO_7(x)		((0xff&x)<<24)
#define ATH_GPIO_OUT_FUNCTION1_ENABLE_GPIO_6(x)		((0xff&x)<<16)
#define ATH_GPIO_OUT_FUNCTION1_ENABLE_GPIO_5(x)		((0xff&x)<< 8)
#define ATH_GPIO_OUT_FUNCTION1_ENABLE_GPIO_4(x)		(0xff&x)
#define ATH_GPIO_OUT_FUNCTION0_ENABLE_GPIO_3(x)		((0xff&x)<<24)
#define ATH_GPIO_OUT_FUNCTION0_ENABLE_GPIO_2(x)		((0xff&x)<<16)
#define ATH_GPIO_IN_ENABLE1_I2SEXT_MCLK(x)		((0xff&x)<<24)
#define ATH_GPIO_IN_ENABLE0_UART_SIN(x)			((0xff&x)<< 8)
#define ATH_GPIO_IN_ENABLE0_SPI_DATA_IN(x)		(0xff&x)




/*
 * PLL
 */
#define AR7240_CPU_PLL_CONFIG           AR7240_PLL_BASE
#define AR7240_USB_PLL_CONFIG           AR7240_PLL_BASE+0x4
#define AR7240_PCIE_PLL_CONFIG          AR7240_PLL_BASE+0x10
#define AR7240_CPU_CLOCK_CONTROL        AR7240_PLL_BASE+8

#define AR7240_USB_PLL_GE0_OFFSET       AR7240_PLL_BASE+0x10
#define AR7240_USB_PLL_GE1_OFFSET       AR7240_PLL_BASE+0x14
#define AR7242_ETH_XMII_CONFIG          AR7240_PLL_BASE+0x2c

/* Hornet's CPU PLL Configuration Register */
#define HORNET_PLL_CONFIG_NINT_SHIFT            10
#define HORNET_PLL_CONFIG_NINT_MASK             (0x3f << HORNET_PLL_CONFIG_NINT_SHIFT)
#define HORNET_PLL_CONFIG_REFDIV_SHIFT          16
#define HORNET_PLL_CONFIG_REFDIV_MASK           (0x1f << HORNET_PLL_CONFIG_REFDIV_SHIFT)
#define HORNET_PLL_CONFIG_OUTDIV_SHIFT          23
#define HORNET_PLL_CONFIG_OUTDIV_MASK           (0x7 << HORNET_PLL_CONFIG_OUTDIV_SHIFT)
#define HORNET_PLL_CONFIG_PLLPWD_SHIFT          30
#define HORNET_PLL_CONFIG_PLLPWD_MASK           (0x1 << HORNET_PLL_CONFIG_PLLPWD_SHIFT)
#define HORNET_PLL_CONFIG_UPDATING_SHIFT        31
#define HORNET_PLL_CONFIG_UPDATING_MASK         (0x1 << HORNET_PLL_CONFIG_UPDATING_SHIFT)
/* Hornet's CPU PLL Configuration 2 Register */
#define HORNET_PLL_CONFIG2_SETTLE_TIME_SHIFT    0
#define HORNET_PLL_CONFIG2_SETTLE_TIME_MASK     (0xfff << HORNET_PLL_CONFIG2_SETTLE_TIME_SHIFT)
/* Hornet's CPU Clock Control Register */
#define HORNET_CLOCK_CONTROL_BYPASS_SHIFT       2
#define HORNET_CLOCK_CONTROL_BYPASS_MASK        (0x1 << HORNET_CLOCK_CONTROL_BYPASS_SHIFT)
#define HORNET_CLOCK_CONTROL_CPU_POST_DIV_SHIFT 5
#define HORNET_CLOCK_CONTROL_CPU_POST_DIV_MASK  (0x3 << HORNET_CLOCK_CONTROL_CPU_POST_DIV_SHIFT)
#define HORNET_CLOCK_CONTROL_DDR_POST_DIV_SFIFT 10
#define HORNET_CLOCK_CONTROL_DDR_POST_DIV_MASK  (0x3 << HORNET_CLOCK_CONTROL_DDR_POST_DIV_SFIFT)
#define HORNET_CLOCK_CONTROL_AHB_POST_DIV_SFIFT 15
#define HORNET_CLOCK_CONTROL_AHB_POST_DIV_MASK  (0x3 << HORNET_CLOCK_CONTROL_AHB_POST_DIV_SFIFT)

#define CLOCK_CONTROL_CLOCK_SWITCH_SHIFT  0
#define CLOCK_CONTROL_CLOCK_SWITCH_MASK  (1 << CLOCK_CONTROL_CLOCK_SWITCH_SHIFT)
#define CLOCK_CONTROL_RST_SWITCH_SHIFT    1
#define CLOCK_CONTROL_RST_SWITCH_MASK    (1 << CLOCK_CONTROL_RST_SWITCH_SHIFT)

/*
 * Reset block
 */
#define HORNET_BOOTSTRAP_STATUS             AR7240_RESET_BASE+0xac /* Hornet's bootstrap register */
#define HORNET_BOOTSTRAP_SEL_25M_40M_MASK   0x00000001 /* Hornet's bootstrap register */
#define HORNET_BOOTSTRAP_FW_CONTROL_MASK    0x00000700 /* Hornet's bootstrap register */
#define HORNET_BOOTSTRAP_MEM_TYPE_MASK      0x00003000 /* Hornet's bootstrap register */


/*
 * Uart block
 */
/* Hornet uses high-speed UART controller */
// 0x0000 (UARTDATA)
#define UARTDATA_UARTTXCSR_MSB                                       9
#define UARTDATA_UARTTXCSR_LSB                                       9
#define UARTDATA_UARTTXCSR_MASK                                      0x00000200
#define UARTDATA_UARTTXCSR_GET(x)                                    (((x) & UARTDATA_UARTTXCSR_MASK) >> UARTDATA_UARTTXCSR_LSB)
#define UARTDATA_UARTTXCSR_SET(x)                                    (((0 | (x)) << UARTDATA_UARTTXCSR_LSB) & UARTDATA_UARTTXCSR_MASK)
#define UARTDATA_UARTTXCSR_RESET                                     0
#define UARTDATA_UARTRXCSR_MSB                                       8
#define UARTDATA_UARTRXCSR_LSB                                       8
#define UARTDATA_UARTRXCSR_MASK                                      0x00000100
#define UARTDATA_UARTRXCSR_GET(x)                                    (((x) & UARTDATA_UARTRXCSR_MASK) >> UARTDATA_UARTRXCSR_LSB)
#define UARTDATA_UARTRXCSR_SET(x)                                    (((0 | (x)) << UARTDATA_UARTRXCSR_LSB) & UARTDATA_UARTRXCSR_MASK)
#define UARTDATA_UARTRXCSR_RESET                                     0
#define UARTDATA_UARTTXRXDATA_MSB                                    7
#define UARTDATA_UARTTXRXDATA_LSB                                    0
#define UARTDATA_UARTTXRXDATA_MASK                                   0x000000ff
#define UARTDATA_UARTTXRXDATA_GET(x)                                 (((x) & UARTDATA_UARTTXRXDATA_MASK) >> UARTDATA_UARTTXRXDATA_LSB)
#define UARTDATA_UARTTXRXDATA_SET(x)                                 (((0 | (x)) << UARTDATA_UARTTXRXDATA_LSB) & UARTDATA_UARTTXRXDATA_MASK)
#define UARTDATA_UARTTXRXDATA_RESET                                  0
#define UARTDATA_ADDRESS                                             0x0000
#define UARTDATA_HW_MASK                                             0x000003ff
#define UARTDATA_SW_MASK                                             0x000003ff
#define UARTDATA_RSTMASK                                             0x000003ff
#define UARTDATA_RESET                                               0x00000000

// 0x0004 (UARTCS)
#define UARTCS_UARTRXBUSY_MSB                                        15
#define UARTCS_UARTRXBUSY_LSB                                        15
#define UARTCS_UARTRXBUSY_MASK                                       0x00008000
#define UARTCS_UARTRXBUSY_GET(x)                                     (((x) & UARTCS_UARTRXBUSY_MASK) >> UARTCS_UARTRXBUSY_LSB)
#define UARTCS_UARTRXBUSY_SET(x)                                     (((0 | (x)) << UARTCS_UARTRXBUSY_LSB) & UARTCS_UARTRXBUSY_MASK)
#define UARTCS_UARTRXBUSY_RESET                                      0
#define UARTCS_UARTTXBUSY_MSB                                        14
#define UARTCS_UARTTXBUSY_LSB                                        14
#define UARTCS_UARTTXBUSY_MASK                                       0x00004000
#define UARTCS_UARTTXBUSY_GET(x)                                     (((x) & UARTCS_UARTTXBUSY_MASK) >> UARTCS_UARTTXBUSY_LSB)
#define UARTCS_UARTTXBUSY_SET(x)                                     (((0 | (x)) << UARTCS_UARTTXBUSY_LSB) & UARTCS_UARTTXBUSY_MASK)
#define UARTCS_UARTTXBUSY_RESET                                      0
#define UARTCS_UARTHOSTINTEN_MSB                                     13
#define UARTCS_UARTHOSTINTEN_LSB                                     13
#define UARTCS_UARTHOSTINTEN_MASK                                    0x00002000
#define UARTCS_UARTHOSTINTEN_GET(x)                                  (((x) & UARTCS_UARTHOSTINTEN_MASK) >> UARTCS_UARTHOSTINTEN_LSB)
#define UARTCS_UARTHOSTINTEN_SET(x)                                  (((0 | (x)) << UARTCS_UARTHOSTINTEN_LSB) & UARTCS_UARTHOSTINTEN_MASK)
#define UARTCS_UARTHOSTINTEN_RESET                                   0
#define UARTCS_UARTHOSTINT_MSB                                       12
#define UARTCS_UARTHOSTINT_LSB                                       12
#define UARTCS_UARTHOSTINT_MASK                                      0x00001000
#define UARTCS_UARTHOSTINT_GET(x)                                    (((x) & UARTCS_UARTHOSTINT_MASK) >> UARTCS_UARTHOSTINT_LSB)
#define UARTCS_UARTHOSTINT_SET(x)                                    (((0 | (x)) << UARTCS_UARTHOSTINT_LSB) & UARTCS_UARTHOSTINT_MASK)
#define UARTCS_UARTHOSTINT_RESET                                     0
#define UARTCS_UARTTXBREAK_MSB                                       11
#define UARTCS_UARTTXBREAK_LSB                                       11
#define UARTCS_UARTTXBREAK_MASK                                      0x00000800
#define UARTCS_UARTTXBREAK_GET(x)                                    (((x) & UARTCS_UARTTXBREAK_MASK) >> UARTCS_UARTTXBREAK_LSB)
#define UARTCS_UARTTXBREAK_SET(x)                                    (((0 | (x)) << UARTCS_UARTTXBREAK_LSB) & UARTCS_UARTTXBREAK_MASK)
#define UARTCS_UARTTXBREAK_RESET                                     0
#define UARTCS_UARTRXBREAK_MSB                                       10
#define UARTCS_UARTRXBREAK_LSB                                       10
#define UARTCS_UARTRXBREAK_MASK                                      0x00000400
#define UARTCS_UARTRXBREAK_GET(x)                                    (((x) & UARTCS_UARTRXBREAK_MASK) >> UARTCS_UARTRXBREAK_LSB)
#define UARTCS_UARTRXBREAK_SET(x)                                    (((0 | (x)) << UARTCS_UARTRXBREAK_LSB) & UARTCS_UARTRXBREAK_MASK)
#define UARTCS_UARTRXBREAK_RESET                                     0
#define UARTCS_UARTSERIATXREADY_MSB                                  9
#define UARTCS_UARTSERIATXREADY_LSB                                  9
#define UARTCS_UARTSERIATXREADY_MASK                                 0x00000200
#define UARTCS_UARTSERIATXREADY_GET(x)                               (((x) & UARTCS_UARTSERIATXREADY_MASK) >> UARTCS_UARTSERIATXREADY_LSB)
#define UARTCS_UARTSERIATXREADY_SET(x)                               (((0 | (x)) << UARTCS_UARTSERIATXREADY_LSB) & UARTCS_UARTSERIATXREADY_MASK)
#define UARTCS_UARTSERIATXREADY_RESET                                0
#define UARTCS_UARTTXREADYORIDE_MSB                                  8
#define UARTCS_UARTTXREADYORIDE_LSB                                  8
#define UARTCS_UARTTXREADYORIDE_MASK                                 0x00000100
#define UARTCS_UARTTXREADYORIDE_GET(x)                               (((x) & UARTCS_UARTTXREADYORIDE_MASK) >> UARTCS_UARTTXREADYORIDE_LSB)
#define UARTCS_UARTTXREADYORIDE_SET(x)                               (((0 | (x)) << UARTCS_UARTTXREADYORIDE_LSB) & UARTCS_UARTTXREADYORIDE_MASK)
#define UARTCS_UARTTXREADYORIDE_RESET                                0
#define UARTCS_UARTRXREADYORIDE_MSB                                  7
#define UARTCS_UARTRXREADYORIDE_LSB                                  7
#define UARTCS_UARTRXREADYORIDE_MASK                                 0x00000080
#define UARTCS_UARTRXREADYORIDE_GET(x)                               (((x) & UARTCS_UARTRXREADYORIDE_MASK) >> UARTCS_UARTRXREADYORIDE_LSB)
#define UARTCS_UARTRXREADYORIDE_SET(x)                               (((0 | (x)) << UARTCS_UARTRXREADYORIDE_LSB) & UARTCS_UARTRXREADYORIDE_MASK)
#define UARTCS_UARTRXREADYORIDE_RESET                                0
#define UARTCS_UARTDMAEN_MSB                                         6
#define UARTCS_UARTDMAEN_LSB                                         6
#define UARTCS_UARTDMAEN_MASK                                        0x00000040
#define UARTCS_UARTDMAEN_GET(x)                                      (((x) & UARTCS_UARTDMAEN_MASK) >> UARTCS_UARTDMAEN_LSB)
#define UARTCS_UARTDMAEN_SET(x)                                      (((0 | (x)) << UARTCS_UARTDMAEN_LSB) & UARTCS_UARTDMAEN_MASK)
#define UARTCS_UARTDMAEN_RESET                                       0
#define UARTCS_UARTFLOWCONTROLMODE_MSB                               5
#define UARTCS_UARTFLOWCONTROLMODE_LSB                               4
#define UARTCS_UARTFLOWCONTROLMODE_MASK                              0x00000030
#define UARTCS_UARTFLOWCONTROLMODE_GET(x)                            (((x) & UARTCS_UARTFLOWCONTROLMODE_MASK) >> UARTCS_UARTFLOWCONTROLMODE_LSB)
#define UARTCS_UARTFLOWCONTROLMODE_SET(x)                            (((0 | (x)) << UARTCS_UARTFLOWCONTROLMODE_LSB) & UARTCS_UARTFLOWCONTROLMODE_MASK)
#define UARTCS_UARTFLOWCONTROLMODE_RESET                             0
#define UARTCS_UARTINTERFACEMODE_MSB                                 3
#define UARTCS_UARTINTERFACEMODE_LSB                                 2
#define UARTCS_UARTINTERFACEMODE_MASK                                0x0000000c
#define UARTCS_UARTINTERFACEMODE_GET(x)                              (((x) & UARTCS_UARTINTERFACEMODE_MASK) >> UARTCS_UARTINTERFACEMODE_LSB)
#define UARTCS_UARTINTERFACEMODE_SET(x)                              (((0 | (x)) << UARTCS_UARTINTERFACEMODE_LSB) & UARTCS_UARTINTERFACEMODE_MASK)
#define UARTCS_UARTINTERFACEMODE_RESET                               0
#define UARTCS_UARTPARITYMODE_MSB                                    1
#define UARTCS_UARTPARITYMODE_LSB                                    0
#define UARTCS_UARTPARITYMODE_MASK                                   0x00000003
#define UARTCS_UARTPARITYMODE_GET(x)                                 (((x) & UARTCS_UARTPARITYMODE_MASK) >> UARTCS_UARTPARITYMODE_LSB)
#define UARTCS_UARTPARITYMODE_SET(x)                                 (((0 | (x)) << UARTCS_UARTPARITYMODE_LSB) & UARTCS_UARTPARITYMODE_MASK)
#define UARTCS_UARTPARITYMODE_RESET                                  0
#define UARTCS_ADDRESS                                               0x0004
#define UARTCS_HW_MASK                                               0x0000ffff
#define UARTCS_SW_MASK                                               0x0000ffff
#define UARTCS_RSTMASK                                               0x000029ff
#define UARTCS_RESET                                                 0x00000000

// 0x0008 (UARTCLOCK)
#define UARTCLOCK_UARTCLOCKSCALE_MSB                                 23
#define UARTCLOCK_UARTCLOCKSCALE_LSB                                 16
#define UARTCLOCK_UARTCLOCKSCALE_MASK                                0x00ff0000
#define UARTCLOCK_UARTCLOCKSCALE_GET(x)                              (((x) & UARTCLOCK_UARTCLOCKSCALE_MASK) >> UARTCLOCK_UARTCLOCKSCALE_LSB)
#define UARTCLOCK_UARTCLOCKSCALE_SET(x)                              (((0 | (x)) << UARTCLOCK_UARTCLOCKSCALE_LSB) & UARTCLOCK_UARTCLOCKSCALE_MASK)
#define UARTCLOCK_UARTCLOCKSCALE_RESET                               0
#define UARTCLOCK_UARTCLOCKSTEP_MSB                                  15
#define UARTCLOCK_UARTCLOCKSTEP_LSB                                  0
#define UARTCLOCK_UARTCLOCKSTEP_MASK                                 0x0000ffff
#define UARTCLOCK_UARTCLOCKSTEP_GET(x)                               (((x) & UARTCLOCK_UARTCLOCKSTEP_MASK) >> UARTCLOCK_UARTCLOCKSTEP_LSB)
#define UARTCLOCK_UARTCLOCKSTEP_SET(x)                               (((0 | (x)) << UARTCLOCK_UARTCLOCKSTEP_LSB) & UARTCLOCK_UARTCLOCKSTEP_MASK)
#define UARTCLOCK_UARTCLOCKSTEP_RESET                                0
#define UARTCLOCK_ADDRESS                                            0x0008
#define UARTCLOCK_HW_MASK                                            0x00ffffff
#define UARTCLOCK_SW_MASK                                            0x00ffffff
#define UARTCLOCK_RSTMASK                                            0x00ffffff
#define UARTCLOCK_RESET                                              0x00000000

// 0x000c (UARTINT)
#define UARTINT_UARTTXEMPTYINT_MSB                                   9
#define UARTINT_UARTTXEMPTYINT_LSB                                   9
#define UARTINT_UARTTXEMPTYINT_MASK                                  0x00000200
#define UARTINT_UARTTXEMPTYINT_GET(x)                                (((x) & UARTINT_UARTTXEMPTYINT_MASK) >> UARTINT_UARTTXEMPTYINT_LSB)
#define UARTINT_UARTTXEMPTYINT_SET(x)                                (((0 | (x)) << UARTINT_UARTTXEMPTYINT_LSB) & UARTINT_UARTTXEMPTYINT_MASK)
#define UARTINT_UARTTXEMPTYINT_RESET                                 0
#define UARTINT_UARTRXFULLINT_MSB                                    8
#define UARTINT_UARTRXFULLINT_LSB                                    8
#define UARTINT_UARTRXFULLINT_MASK                                   0x00000100
#define UARTINT_UARTRXFULLINT_GET(x)                                 (((x) & UARTINT_UARTRXFULLINT_MASK) >> UARTINT_UARTRXFULLINT_LSB)
#define UARTINT_UARTRXFULLINT_SET(x)                                 (((0 | (x)) << UARTINT_UARTRXFULLINT_LSB) & UARTINT_UARTRXFULLINT_MASK)
#define UARTINT_UARTRXFULLINT_RESET                                  0
#define UARTINT_UARTRXBREAKOFFINT_MSB                                7
#define UARTINT_UARTRXBREAKOFFINT_LSB                                7
#define UARTINT_UARTRXBREAKOFFINT_MASK                               0x00000080
#define UARTINT_UARTRXBREAKOFFINT_GET(x)                             (((x) & UARTINT_UARTRXBREAKOFFINT_MASK) >> UARTINT_UARTRXBREAKOFFINT_LSB)
#define UARTINT_UARTRXBREAKOFFINT_SET(x)                             (((0 | (x)) << UARTINT_UARTRXBREAKOFFINT_LSB) & UARTINT_UARTRXBREAKOFFINT_MASK)
#define UARTINT_UARTRXBREAKOFFINT_RESET                              0
#define UARTINT_UARTRXBREAKONINT_MSB                                 6
#define UARTINT_UARTRXBREAKONINT_LSB                                 6
#define UARTINT_UARTRXBREAKONINT_MASK                                0x00000040
#define UARTINT_UARTRXBREAKONINT_GET(x)                              (((x) & UARTINT_UARTRXBREAKONINT_MASK) >> UARTINT_UARTRXBREAKONINT_LSB)
#define UARTINT_UARTRXBREAKONINT_SET(x)                              (((0 | (x)) << UARTINT_UARTRXBREAKONINT_LSB) & UARTINT_UARTRXBREAKONINT_MASK)
#define UARTINT_UARTRXBREAKONINT_RESET                               0
#define UARTINT_UARTRXPARITYERRINT_MSB                               5
#define UARTINT_UARTRXPARITYERRINT_LSB                               5
#define UARTINT_UARTRXPARITYERRINT_MASK                              0x00000020
#define UARTINT_UARTRXPARITYERRINT_GET(x)                            (((x) & UARTINT_UARTRXPARITYERRINT_MASK) >> UARTINT_UARTRXPARITYERRINT_LSB)
#define UARTINT_UARTRXPARITYERRINT_SET(x)                            (((0 | (x)) << UARTINT_UARTRXPARITYERRINT_LSB) & UARTINT_UARTRXPARITYERRINT_MASK)
#define UARTINT_UARTRXPARITYERRINT_RESET                             0
#define UARTINT_UARTTXOFLOWERRINT_MSB                                4
#define UARTINT_UARTTXOFLOWERRINT_LSB                                4
#define UARTINT_UARTTXOFLOWERRINT_MASK                               0x00000010
#define UARTINT_UARTTXOFLOWERRINT_GET(x)                             (((x) & UARTINT_UARTTXOFLOWERRINT_MASK) >> UARTINT_UARTTXOFLOWERRINT_LSB)
#define UARTINT_UARTTXOFLOWERRINT_SET(x)                             (((0 | (x)) << UARTINT_UARTTXOFLOWERRINT_LSB) & UARTINT_UARTTXOFLOWERRINT_MASK)
#define UARTINT_UARTTXOFLOWERRINT_RESET                              0
#define UARTINT_UARTRXOFLOWERRINT_MSB                                3
#define UARTINT_UARTRXOFLOWERRINT_LSB                                3
#define UARTINT_UARTRXOFLOWERRINT_MASK                               0x00000008
#define UARTINT_UARTRXOFLOWERRINT_GET(x)                             (((x) & UARTINT_UARTRXOFLOWERRINT_MASK) >> UARTINT_UARTRXOFLOWERRINT_LSB)
#define UARTINT_UARTRXOFLOWERRINT_SET(x)                             (((0 | (x)) << UARTINT_UARTRXOFLOWERRINT_LSB) & UARTINT_UARTRXOFLOWERRINT_MASK)
#define UARTINT_UARTRXOFLOWERRINT_RESET                              0
#define UARTINT_UARTRXFRAMINGERRINT_MSB                              2
#define UARTINT_UARTRXFRAMINGERRINT_LSB                              2
#define UARTINT_UARTRXFRAMINGERRINT_MASK                             0x00000004
#define UARTINT_UARTRXFRAMINGERRINT_GET(x)                           (((x) & UARTINT_UARTRXFRAMINGERRINT_MASK) >> UARTINT_UARTRXFRAMINGERRINT_LSB)
#define UARTINT_UARTRXFRAMINGERRINT_SET(x)                           (((0 | (x)) << UARTINT_UARTRXFRAMINGERRINT_LSB) & UARTINT_UARTRXFRAMINGERRINT_MASK)
#define UARTINT_UARTRXFRAMINGERRINT_RESET                            0
#define UARTINT_UARTTXREADYINT_MSB                                   1
#define UARTINT_UARTTXREADYINT_LSB                                   1
#define UARTINT_UARTTXREADYINT_MASK                                  0x00000002
#define UARTINT_UARTTXREADYINT_GET(x)                                (((x) & UARTINT_UARTTXREADYINT_MASK) >> UARTINT_UARTTXREADYINT_LSB)
#define UARTINT_UARTTXREADYINT_SET(x)                                (((0 | (x)) << UARTINT_UARTTXREADYINT_LSB) & UARTINT_UARTTXREADYINT_MASK)
#define UARTINT_UARTTXREADYINT_RESET                                 0
#define UARTINT_UARTRXVALIDINT_MSB                                   0
#define UARTINT_UARTRXVALIDINT_LSB                                   0
#define UARTINT_UARTRXVALIDINT_MASK                                  0x00000001
#define UARTINT_UARTRXVALIDINT_GET(x)                                (((x) & UARTINT_UARTRXVALIDINT_MASK) >> UARTINT_UARTRXVALIDINT_LSB)
#define UARTINT_UARTRXVALIDINT_SET(x)                                (((0 | (x)) << UARTINT_UARTRXVALIDINT_LSB) & UARTINT_UARTRXVALIDINT_MASK)
#define UARTINT_UARTRXVALIDINT_RESET                                 0
#define UARTINT_ADDRESS                                              0x000c
#define UARTINT_HW_MASK                                              0x000003ff
#define UARTINT_SW_MASK                                              0x000003ff
#define UARTINT_RSTMASK                                              0x000003ff
#define UARTINT_RESET                                                0x00000000

// 0x0010 (UARTINTEN)
#define UARTINTEN_UARTTXEMPTYINTEN_MSB                               9
#define UARTINTEN_UARTTXEMPTYINTEN_LSB                               9
#define UARTINTEN_UARTTXEMPTYINTEN_MASK                              0x00000200
#define UARTINTEN_UARTTXEMPTYINTEN_GET(x)                            (((x) & UARTINTEN_UARTTXEMPTYINTEN_MASK) >> UARTINTEN_UARTTXEMPTYINTEN_LSB)
#define UARTINTEN_UARTTXEMPTYINTEN_SET(x)                            (((0 | (x)) << UARTINTEN_UARTTXEMPTYINTEN_LSB) & UARTINTEN_UARTTXEMPTYINTEN_MASK)
#define UARTINTEN_UARTTXEMPTYINTEN_RESET                             0
#define UARTINTEN_UARTRXFULLINTEN_MSB                                8
#define UARTINTEN_UARTRXFULLINTEN_LSB                                8
#define UARTINTEN_UARTRXFULLINTEN_MASK                               0x00000100
#define UARTINTEN_UARTRXFULLINTEN_GET(x)                             (((x) & UARTINTEN_UARTRXFULLINTEN_MASK) >> UARTINTEN_UARTRXFULLINTEN_LSB)
#define UARTINTEN_UARTRXFULLINTEN_SET(x)                             (((0 | (x)) << UARTINTEN_UARTRXFULLINTEN_LSB) & UARTINTEN_UARTRXFULLINTEN_MASK)
#define UARTINTEN_UARTRXFULLINTEN_RESET                              0
#define UARTINTEN_UARTRXBREAKOFFINTEN_MSB                            7
#define UARTINTEN_UARTRXBREAKOFFINTEN_LSB                            7
#define UARTINTEN_UARTRXBREAKOFFINTEN_MASK                           0x00000080
#define UARTINTEN_UARTRXBREAKOFFINTEN_GET(x)                         (((x) & UARTINTEN_UARTRXBREAKOFFINTEN_MASK) >> UARTINTEN_UARTRXBREAKOFFINTEN_LSB)
#define UARTINTEN_UARTRXBREAKOFFINTEN_SET(x)                         (((0 | (x)) << UARTINTEN_UARTRXBREAKOFFINTEN_LSB) & UARTINTEN_UARTRXBREAKOFFINTEN_MASK)
#define UARTINTEN_UARTRXBREAKOFFINTEN_RESET                          0
#define UARTINTEN_UARTRXBREAKONINTEN_MSB                             6
#define UARTINTEN_UARTRXBREAKONINTEN_LSB                             6
#define UARTINTEN_UARTRXBREAKONINTEN_MASK                            0x00000040
#define UARTINTEN_UARTRXBREAKONINTEN_GET(x)                          (((x) & UARTINTEN_UARTRXBREAKONINTEN_MASK) >> UARTINTEN_UARTRXBREAKONINTEN_LSB)
#define UARTINTEN_UARTRXBREAKONINTEN_SET(x)                          (((0 | (x)) << UARTINTEN_UARTRXBREAKONINTEN_LSB) & UARTINTEN_UARTRXBREAKONINTEN_MASK)
#define UARTINTEN_UARTRXBREAKONINTEN_RESET                           0
#define UARTINTEN_UARTRXPARITYERRINTEN_MSB                           5
#define UARTINTEN_UARTRXPARITYERRINTEN_LSB                           5
#define UARTINTEN_UARTRXPARITYERRINTEN_MASK                          0x00000020
#define UARTINTEN_UARTRXPARITYERRINTEN_GET(x)                        (((x) & UARTINTEN_UARTRXPARITYERRINTEN_MASK) >> UARTINTEN_UARTRXPARITYERRINTEN_LSB)
#define UARTINTEN_UARTRXPARITYERRINTEN_SET(x)                        (((0 | (x)) << UARTINTEN_UARTRXPARITYERRINTEN_LSB) & UARTINTEN_UARTRXPARITYERRINTEN_MASK)
#define UARTINTEN_UARTRXPARITYERRINTEN_RESET                         0
#define UARTINTEN_UARTTXOFLOWERRINTEN_MSB                            4
#define UARTINTEN_UARTTXOFLOWERRINTEN_LSB                            4
#define UARTINTEN_UARTTXOFLOWERRINTEN_MASK                           0x00000010
#define UARTINTEN_UARTTXOFLOWERRINTEN_GET(x)                         (((x) & UARTINTEN_UARTTXOFLOWERRINTEN_MASK) >> UARTINTEN_UARTTXOFLOWERRINTEN_LSB)
#define UARTINTEN_UARTTXOFLOWERRINTEN_SET(x)                         (((0 | (x)) << UARTINTEN_UARTTXOFLOWERRINTEN_LSB) & UARTINTEN_UARTTXOFLOWERRINTEN_MASK)
#define UARTINTEN_UARTTXOFLOWERRINTEN_RESET                          0
#define UARTINTEN_UARTRXOFLOWERRINTEN_MSB                            3
#define UARTINTEN_UARTRXOFLOWERRINTEN_LSB                            3
#define UARTINTEN_UARTRXOFLOWERRINTEN_MASK                           0x00000008
#define UARTINTEN_UARTRXOFLOWERRINTEN_GET(x)                         (((x) & UARTINTEN_UARTRXOFLOWERRINTEN_MASK) >> UARTINTEN_UARTRXOFLOWERRINTEN_LSB)
#define UARTINTEN_UARTRXOFLOWERRINTEN_SET(x)                         (((0 | (x)) << UARTINTEN_UARTRXOFLOWERRINTEN_LSB) & UARTINTEN_UARTRXOFLOWERRINTEN_MASK)
#define UARTINTEN_UARTRXOFLOWERRINTEN_RESET                          0
#define UARTINTEN_UARTRXFRAMINGERRINTEN_MSB                          2
#define UARTINTEN_UARTRXFRAMINGERRINTEN_LSB                          2
#define UARTINTEN_UARTRXFRAMINGERRINTEN_MASK                         0x00000004
#define UARTINTEN_UARTRXFRAMINGERRINTEN_GET(x)                       (((x) & UARTINTEN_UARTRXFRAMINGERRINTEN_MASK) >> UARTINTEN_UARTRXFRAMINGERRINTEN_LSB)
#define UARTINTEN_UARTRXFRAMINGERRINTEN_SET(x)                       (((0 | (x)) << UARTINTEN_UARTRXFRAMINGERRINTEN_LSB) & UARTINTEN_UARTRXFRAMINGERRINTEN_MASK)
#define UARTINTEN_UARTRXFRAMINGERRINTEN_RESET                        0
#define UARTINTEN_UARTTXREADYINTEN_MSB                               1
#define UARTINTEN_UARTTXREADYINTEN_LSB                               1
#define UARTINTEN_UARTTXREADYINTEN_MASK                              0x00000002
#define UARTINTEN_UARTTXREADYINTEN_GET(x)                            (((x) & UARTINTEN_UARTTXREADYINTEN_MASK) >> UARTINTEN_UARTTXREADYINTEN_LSB)
#define UARTINTEN_UARTTXREADYINTEN_SET(x)                            (((0 | (x)) << UARTINTEN_UARTTXREADYINTEN_LSB) & UARTINTEN_UARTTXREADYINTEN_MASK)
#define UARTINTEN_UARTTXREADYINTEN_RESET                             0
#define UARTINTEN_UARTRXVALIDINTEN_MSB                               0
#define UARTINTEN_UARTRXVALIDINTEN_LSB                               0
#define UARTINTEN_UARTRXVALIDINTEN_MASK                              0x00000001
#define UARTINTEN_UARTRXVALIDINTEN_GET(x)                            (((x) & UARTINTEN_UARTRXVALIDINTEN_MASK) >> UARTINTEN_UARTRXVALIDINTEN_LSB)
#define UARTINTEN_UARTRXVALIDINTEN_SET(x)                            (((0 | (x)) << UARTINTEN_UARTRXVALIDINTEN_LSB) & UARTINTEN_UARTRXVALIDINTEN_MASK)
#define UARTINTEN_UARTRXVALIDINTEN_RESET                             0
#define UARTINTEN_ADDRESS                                            0x0010
#define UARTINTEN_HW_MASK                                            0x000003ff
#define UARTINTEN_SW_MASK                                            0x000003ff
#define UARTINTEN_RSTMASK                                            0x000003ff
#define UARTINTEN_RESET                                              0x00000000

#define uart_reg_read(x)        ar7240_reg_rd( (AR7240_UART_BASE+x) )
#define uart_reg_write(x, y)    ar7240_reg_wr( (AR7240_UART_BASE+x), y)
/*
 * ATH_RESET bit defines
 */
#define ATH_RESET_SLIC			(1 << 30)
#define ATH_RESET_EXTERNAL		(1 << 28)
#define ATH_RESET_FULL_CHIP		(1 << 24)
#define ATH_RESET_GE0_MDIO		(1 << 22)
#define ATH_RESET_CPU_NMI		(1 << 21)
#define ATH_RESET_CPU_COLD_RESET_MASK	(1 << 20)
#define ATH_RESET_DMA			(1 << 19)
#define ATH_RESET_STEREO		(1 << 17)
#define ATH_RESET_DDR			(1 << 16)
#define ATH_RESET_GE1_MAC		(1 << 13)
#define ATH_RESET_GE1_PHY		(1 << 12)
#define ATH_RESET_USB_PHY_ANALOG	(1 << 11)
#define ATH_RESET_PCIE_PHY_SHIFT	(1 << 10)
#define ATH_RESET_GE0_MAC		(1 << 9)
#define ATH_RESET_GE0_PHY		(1 << 8)
#define ATH_RESET_USBSUS_OVRIDE		(1 << 3)
#define ATH_RESET_USB_OHCI_DLL		(1 << 3)
#define ATH_RESET_USB_HOST		(1 << 5)
#define ATH_RESET_USB_PHY		(1 << 4)
#define ATH_RESET_PCI_BUS		(1 << 1)
#define ATH_RESET_PCI_CORE		(1 << 0)
#define ATH_RESET_I2S			(1 << 0)


#endif
