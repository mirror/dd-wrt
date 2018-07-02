/*
 * vim: tabstop=8 : noexpandtab
 */
#ifndef _934x_H
#define _934x_H

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
#define ATH_OTP_BASE			ATH_APB_BASE+0x00130000

/*
 * DDR block
 */
#define ATH_DDR_CONFIG			ATH_DDR_CTL_BASE+0
#define ATH_DDR_CONFIG2			ATH_DDR_CTL_BASE+4
#define ATH_DDR_MODE			ATH_DDR_CTL_BASE+0x08
#define ATH_DDR_EXT_MODE		ATH_DDR_CTL_BASE+0x0c
#define ATH_DDR_CONTROL			ATH_DDR_CTL_BASE+0x10
#define ATH_DDR_REFRESH			ATH_DDR_CTL_BASE+0x14
#define ATH_DDR_RD_DATA_THIS_CYCLE	ATH_DDR_CTL_BASE+0x18
#define ATH_DDR_TAP_CONTROL0		ATH_DDR_CTL_BASE+0x1c
#define ATH_DDR_TAP_CONTROL1		ATH_DDR_CTL_BASE+0x20
#define ATH_DDR_TAP_CONTROL2		ATH_DDR_CTL_BASE+0x24
#define ATH_DDR_TAP_CONTROL3		ATH_DDR_CTL_BASE+0x28

/*
 * DDR Config values
 */
#define ATH_DDR_CONFIG_16BIT		(1 << 31)
#define ATH_DDR_CONFIG_PAGE_OPEN	(1 << 30)
#define ATH_DDR_CONFIG_CAS_LAT_SHIFT	27
#define ATH_DDR_CONFIG_TMRD_SHIFT	23
#define ATH_DDR_CONFIG_TRFC_SHIFT	17
#define ATH_DDR_CONFIG_TRRD_SHIFT	13
#define ATH_DDR_CONFIG_TRP_SHIFT	9
#define ATH_DDR_CONFIG_TRCD_SHIFT	5
#define ATH_DDR_CONFIG_TRAS_SHIFT	0

#define ATH_DDR_CONFIG2_BL2		(2 << 0)
#define ATH_DDR_CONFIG2_BL4		(4 << 0)
#define ATH_DDR_CONFIG2_BL8		(8 << 0)

#define ATH_DDR_CONFIG2_BT_IL		(1 << 4)
#define ATH_DDR_CONFIG2_CNTL_OE_EN	(1 << 5)
#define ATH_DDR_CONFIG2_PHASE_SEL	(1 << 6)
#define ATH_DDR_CONFIG2_DRAM_CKE	(1 << 7)
#define ATH_DDR_CONFIG2_TWR_SHIFT	8
#define ATH_DDR_CONFIG2_TRTW_SHIFT	12
#define ATH_DDR_CONFIG2_TRTP_SHIFT	17
#define ATH_DDR_CONFIG2_TWTR_SHIFT	21
#define ATH_DDR_CONFIG2_HALF_WIDTH_L	(1 << 31)

#define ATH_DDR_TAP_DEFAULT		0x18

/*
 * DDR block, gmac flushing
 */
#define ATH_DDR_GE0_FLUSH		ATH_DDR_CTL_BASE+0x9c
#define ATH_DDR_GE1_FLUSH		ATH_DDR_CTL_BASE+0xa0
#define ATH_DDR_USB_FLUSH		ATH_DDR_CTL_BASE+0xa4
#define ATH_DDR_PCIE_FLUSH		ATH_DDR_CTL_BASE+0xa8

#define ATH_EEPROM_GE0_MAC_ADDR		0xbfff1000
#define ATH_EEPROM_GE1_MAC_ADDR		0xbfff1006

/*
 * PLL block/CPU
 */

#define ATH_PLL_CONFIG			ATH_PLL_BASE+0x0
#define ATH_DDR_CLK_CTRL		ATH_PLL_BASE+0x8


#define PLL_DIV_SHIFT			0
#define PLL_DIV_MASK			0x3ff
#define REF_DIV_SHIFT			10
#define REF_DIV_MASK			0xf
#define AHB_DIV_SHIFT			19
#define AHB_DIV_MASK			0x1
#define DDR_DIV_SHIFT			22
#define DDR_DIV_MASK			0x1
#define ATH_DDR_PLL_CONFIG		ATH_PLL_BASE+0x4
#define ATH_ETH_XMII_CONFIG		ATH_PLL_BASE+0x2c
#define ATH_AUDIO_PLL_CONFIG		ATH_PLL_BASE+0x30
#define ATH_FRAC_FREQ_CONFIG		ATH_PLL_BASE+0x34

#define ATH_ETH_INT0_CLK		ATH_PLL_BASE+0x14
#define ATH_ETH_INT1_CLK		ATH_PLL_BASE+0x18


/*
 * USB block
 */
#define ATH_USB_FLADJ_VAL		ATH_USB_CONFIG_BASE
#define ATH_USB_CONFIG			ATH_USB_CONFIG_BASE+0x4
#define ATH_USB_WINDOW			0x1000000
#define ATH_USB_MODE			ATH_USB_EHCI_BASE+0x1a8

/*
 * PCI block
 */
#define ATH_PCI_WINDOW			0x8000000 /* 128MB */
#define ATH_PCI_WINDOW0_OFFSET		ATH_DDR_CTL_BASE+0x7c
#define ATH_PCI_WINDOW1_OFFSET		ATH_DDR_CTL_BASE+0x80
#define ATH_PCI_WINDOW2_OFFSET		ATH_DDR_CTL_BASE+0x84
#define ATH_PCI_WINDOW3_OFFSET		ATH_DDR_CTL_BASE+0x88
#define ATH_PCI_WINDOW4_OFFSET		ATH_DDR_CTL_BASE+0x8c
#define ATH_PCI_WINDOW5_OFFSET		ATH_DDR_CTL_BASE+0x90
#define ATH_PCI_WINDOW6_OFFSET		ATH_DDR_CTL_BASE+0x94
#define ATH_PCI_WINDOW7_OFFSET		ATH_DDR_CTL_BASE+0x98

#define ATH_PCI_WINDOW0_VAL		0x10000000
#define ATH_PCI_WINDOW1_VAL		0x11000000
#define ATH_PCI_WINDOW2_VAL		0x12000000
#define ATH_PCI_WINDOW3_VAL		0x13000000
#define ATH_PCI_WINDOW4_VAL		0x14000000
#define ATH_PCI_WINDOW5_VAL		0x15000000
#define ATH_PCI_WINDOW6_VAL		0x16000000
#define ATH_PCI_WINDOW7_VAL		0x07000000

#define ath_write_pci_window(_no)	\
	ath_reg_wr(ATH_PCI_WINDOW##_no##_OFFSET, ATH_PCI_WINDOW##_no##_VAL);

/*
 * CRP. To access the host controller config and status registers
 */
#define ATH_PCI_CRP			0x180c0000
#define ATH_PCI_DEV_CFGBASE		0x14000000
#define ATH_PCI_CRP_AD_CBE		ATH_PCI_CRP
#define ATH_PCI_CRP_WRDATA		ATH_PCI_CRP+0x4
#define ATH_PCI_CRP_RDDATA		ATH_PCI_CRP+0x8
#define ATH_PCI_ERROR			ATH_PCI_CRP+0x1c
#define ATH_PCI_ERROR_ADDRESS		ATH_PCI_CRP+0x20
#define ATH_PCI_AHB_ERROR		ATH_PCI_CRP+0x24
#define ATH_PCI_AHB_ERROR_ADDRESS	ATH_PCI_CRP+0x28

#define ATH_CRP_CMD_WRITE		0x00010000
#define ATH_CRP_CMD_READ		0x00000000

/*
 * PCI CFG. To generate config cycles
 */
#define ATH_PCI_CFG_AD			ATH_PCI_CRP+0xc
#define ATH_PCI_CFG_CBE			ATH_PCI_CRP+0x10
#define ATH_PCI_CFG_WRDATA		ATH_PCI_CRP+0x14
#define ATH_PCI_CFG_RDDATA		ATH_PCI_CRP+0x18
#define ATH_CFG_CMD_READ		0x0000000a
#define ATH_CFG_CMD_WRITE		0x0000000b

#define ATH_PCI_IDSEL_ADLINE_START	17


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
 * IRQ Map.
 * There are 4 conceptual ICs in the system. We generally give a block of 16
 * irqs to each IC.
 *	CPU :    0 - 0xf
 *	MISC: 0x10 - 0x1f
 *	GPIO: 0x20 - 0x3f
 *	PCI : 0x40 - 0x4f
 *
 */
#define ATH_CPU_IRQ_BASE		0x00
#define ATH_MISC_IRQ_BASE		0x10
#define ATH_GPIO_IRQ_BASE		0x20

/*
 * Interrupts connected to the Misc->GPIO line
 */
#define ATH_GPIO_IRQn(_gpio)		ATH_GPIO_IRQ_BASE+(_gpio)
#define ATH_GPIO_IRQ_COUNT		32

#define ATH_PCI_IRQ_BASE		(ATH_GPIO_IRQ_BASE + ATH_GPIO_IRQ_COUNT)

//#define MIPS_CPU_IRQ_BASE		ATH_CPU_IRQ_BASE

/*
 * The IPs. Connected to CPU (hardware IP's; the first two are software)
 */
#define ATH_CPU_IRQ_WLAN		ATH_CPU_IRQ_BASE+2
#define ATH_CPU_IRQ_PCI			ATH_CPU_IRQ_BASE+8
#define ATH_CPU_IRQ_USB			ATH_CPU_IRQ_BASE+3
#define ATH_CPU_IRQ_GE0			ATH_CPU_IRQ_BASE+4
#define ATH_CPU_IRQ_GE1			ATH_CPU_IRQ_BASE+5
#define ATH_CPU_IRQ_MISC		ATH_CPU_IRQ_BASE+6
#define ATH_CPU_IRQ_TIMER		ATH_CPU_IRQ_BASE+7
#define ATH_CPU_IRQ_PCI_EP		ATH_CPU_IRQ_BASE+3

/*
 * Interrupts connected to the CPU->Misc line.
 */
#define ATH_MISC_IRQ_TIMER		ATH_MISC_IRQ_BASE+0
#define ATH_MISC_IRQ_ERROR		ATH_MISC_IRQ_BASE+1
#define ATH_MISC_IRQ_GPIO		ATH_MISC_IRQ_BASE+2
#define ATH_MISC_IRQ_UART		ATH_MISC_IRQ_BASE+3
#define ATH_MISC_IRQ_WATCHDOG		ATH_MISC_IRQ_BASE+4
#define ATH_MISC_IRQ_PERF_COUNTER	ATH_MISC_IRQ_BASE+5
#ifdef CONFIG_ATH_HS_UART
#define ATH_MISC_IRQ_HS_UART		ATH_MISC_IRQ_BASE+6
#else
#define ATH_MISC_IRQ_USB_OHCI		ATH_MISC_IRQ_BASE+6
#endif
#define ATH_MISC_IRQ_DMA		ATH_MISC_IRQ_BASE+7
#define ATH_MISC_IRQ_ENET_LINK		ATH_MISC_IRQ_BASE+12
#define ATH_MISC_IRQ_NAT_AGER		ATH_MISC_IRQ_BASE+13

#define ATH_MISC_IRQ_COUNT		14

#define MIMR_TIMER			0x01
#define MIMR_ERROR			0x02
#define MIMR_GPIO			0x04
#define MIMR_UART			0x08
#define MIMR_WATCHDOG			0x10
#define MIMR_PERF_COUNTER		0x20
#ifdef CONFIG_ATH_HS_UART
#define MIMR_HS_UART			0x40
#else
#define MIMR_OHCI_USB			0x40
#endif
#define MIMR_DMA			0x80
#define MIMR_ENET_LINK			0x1000
#define MIMR_NAT_AGER			0x2000

#define MISR_TIMER			MIMR_TIMER
#define MISR_ERROR			MIMR_ERROR
#define MISR_GPIO			MIMR_GPIO
#define MISR_UART			MIMR_UART
#define MISR_WATCHDOG			MIMR_WATCHDOG
#define MISR_PERF_COUNTER		MIMR_PERF_COUNTER
#define MISR_OHCI_USB			MIMR_OHCI_USB
#define MISR_DMA			MIMR_DMA

/* Interrupts connected to CPU->PCI */
#ifdef CONFIG_PERICOM
#	define ATH_PRI_BUS_NO		0u
#	define ATH_PORT0_BUS_NO		1u
#	define ATH_PORT1_BUS_NO		2u
#	define ATH_PCI_IRQ_DEV0		(ATH_PCI_IRQ_BASE + 0)
#	define ATH_PCI_IRQ_DEV1		(ATH_PCI_IRQ_BASE + 1)
#	define ATH_PCI_IRQ_COUNT	2
#else
#	define ATH_PCI_IRQ_DEV0		(ATH_PCI_IRQ_BASE + 0)
#	define ATH_PCI_IRQ_COUNT	1
#endif /* CONFIG_PERICOM */

/*
 * PCI interrupt mask and status
 */
#define PIMR_DEV0			0x01
#define PIMR_DEV1			0x02
#define PIMR_DEV2			0x04
#define PIMR_CORE			0x10

#define PISR_DEV0			PIMR_DEV0
#define PISR_DEV1			PIMR_DEV1
#define PISR_DEV2			PIMR_DEV2
#define PISR_CORE			PIMR_CORE

void ar7240_pci_irq_init(int irq_base); /* ??? */
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

#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_MSB                        31
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_LSB                        24
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_MASK                       0xff000000
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_GET(x)                     (((x) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_MASK) >> GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_LSB)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_SET(x)                     (((x) << GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_LSB) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_MASK)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_11_RESET                      0x3 // 3
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_MSB                        23
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_LSB                        16
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_MASK                       0x00ff0000
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_GET(x)                     (((x) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_MASK) >> GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_LSB)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_SET(x)                     (((x) << GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_LSB) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_MASK)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_RESET                      0x18 // 24
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_MSB                         15
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_LSB                         8
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_MASK                        0x0000ff00
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_GET(x)                      (((x) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_MASK) >> GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_LSB)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_SET(x)                      (((x) << GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_LSB) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_MASK)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_9_RESET                       0x0 // 0
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_MSB                         7
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_LSB                         0
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_MASK                        0x000000ff
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_GET(x)                      (((x) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_MASK) >> GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_LSB)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_SET(x)                      (((x) << GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_LSB) & GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_MASK)
#define GPIO_OUT_FUNCTION2_ENABLE_GPIO_8_RESET                       0x0 // 0
#define GPIO_OUT_FUNCTION2_ADDRESS                                   0x18040034
#define GPIO_OUT_FUNCTION2_OFFSET                                    0x0034
// SW modifiable bits
#define GPIO_OUT_FUNCTION2_SW_MASK                                   0xffffffff
// bits defined at reset
#define GPIO_OUT_FUNCTION2_RSTMASK                                   0xffffffff
// reset value (ignore bits undefined at reset)
#define GPIO_OUT_FUNCTION2_RESET                                     0x03180000

#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_MSB                        31
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_LSB                        24
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_MASK                       0xff000000
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_GET(x)                     (((x) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_MASK) >> GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_LSB)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_SET(x)                     (((x) << GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_LSB) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_MASK)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_15_RESET                      0x0 // 0
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_MSB                        23
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_LSB                        16
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_MASK                       0x00ff0000
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_GET(x)                     (((x) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_MASK) >> GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_LSB)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_SET(x)                     (((x) << GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_LSB) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_MASK)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_14_RESET                      0x0 // 0
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_MSB                        15
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_LSB                        8
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_MASK                       0x0000ff00
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_GET(x)                     (((x) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_MASK) >> GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_LSB)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_SET(x)                     (((x) << GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_LSB) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_MASK)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_RESET                      0x0 // 0
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_MSB                        7
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_LSB                        0
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_MASK                       0x000000ff
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_GET(x)                     (((x) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_MASK) >> GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_LSB)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_SET(x)                     (((x) << GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_LSB) & GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_MASK)
#define GPIO_OUT_FUNCTION3_ENABLE_GPIO_12_RESET                      0x0 // 0
#define GPIO_OUT_FUNCTION3_ADDRESS                                   0x18040038
#define GPIO_OUT_FUNCTION3_OFFSET                                    0x0038
// SW modifiable bits
#define GPIO_OUT_FUNCTION3_SW_MASK                                   0xffffffff
// bits defined at reset
#define GPIO_OUT_FUNCTION3_RSTMASK                                   0xffffffff
// reset value (ignore bits undefined at reset)
#define GPIO_OUT_FUNCTION3_RESET                                     0x00000000



/* SPI, SLIC and GPIO are all multiplexed on gpio pins */
#define ATH_SPI_FS		(ATH_SPI_BASE+0x00)
#define ATH_SPI_READ		(ATH_SPI_BASE+0x00)
#define ATH_SPI_CLOCK		(ATH_SPI_BASE+0x04)
#define ATH_SPI_WRITE		(ATH_SPI_BASE+0x08)
#define ATH_SPI_RD_STATUS	(ATH_SPI_BASE+0x0c)
#define ATH_SPI_SHIFT_DO	(ATH_SPI_BASE+0x10)
#define ATH_SPI_SHIFT_CNT	(ATH_SPI_BASE+0x14)
#define ATH_SPI_SHIFT_DI	(ATH_SPI_BASE+0x18)
#define ATH_SPI_D0_HIGH		(1<<0)	/* Pin spi_do */
#define ATH_SPI_CLK_HIGH	(1<<8)	/* Pin spi_clk */

#define ATH_SPI_CS_ENABLE_0	(6<<16)	/* Pin gpio/cs0 (active low) */
#define ATH_SPI_CS_ENABLE_1	(5<<16)	/* Pin gpio/cs1 (active low) */
#define ATH_SPI_CS_ENABLE_2	(3<<16)	/* Pin gpio/cs2 (active low) */
#define ATH_SPI_CS_DIS		0x70000

/*
 * High Speed Uart
 */
#define ATH_HS_UART_CS			0x18500004
#define ATH_HS_UART_CLK			0x18500008
#define ATH_HS_UART_INT_STATUS		0x1850000c
#define ATH_HS_UART_INT_EN		0x18500010

#define ATH_HS_UART_BAUD		115200
/*
 * SOC
 */
#define ATH_SPI_CMD_WRITE_SR		0x01
#define ATH_SPI_CMD_WREN		0x06
#define ATH_SPI_CMD_RD_STATUS		0x05
#define ATH_SPI_CMD_FAST_READ		0x0b
#define ATH_SPI_CMD_PAGE_PROG		0x02
#define ATH_SPI_CMD_SECTOR_ERASE	0xd8

/*
 * Reset block
 */
#define ATH_GENERAL_TMR			ATH_RESET_BASE+0
#define ATH_GENERAL_TMR_RELOAD		ATH_RESET_BASE+4
#define ATH_WATCHDOG_TMR_CONTROL	ATH_RESET_BASE+8
#define ATH_WATCHDOG_TMR		ATH_RESET_BASE+0xc
#define ATH_MISC_INT_STATUS		ATH_RESET_BASE+0x10
#define ATH_MISC_INT_MASK		ATH_RESET_BASE+0x14

#define ATH_PCI_INT_STATUS		ATH_PCI_CTLR_BASE+0x4c
#define ATH_PCI_INT_MASK		ATH_PCI_CTLR_BASE+0x50
#define ATH_PCI_INT_A_L			(1 << 14) /* INTA Level Trigger */
#define ATH_PCI_INT_B_L			(1 << 15) /* INTB Level Trigger */
#define ATH_PCI_INT_C_L			(1 << 16) /* INTC Level Trigger */
#define ATH_GLOBAL_INT_STATUS		ATH_RESET_BASE+0x18
#define ATH_RESET			ATH_RESET_BASE+0x1c
#define ATH_OBSERVATION_ENABLE		ATH_RESET_BASE+0x28

#define ATH_BOOTSTRAP_REG		(ATH_RESET_BASE + 0xb0)
#define ATH_REF_CLK_40			(1 << 4) /* 0 - 25MHz	1 - 40 MHz */
#define ATH_DDR_WIDTH_32		(1 << 3)

#define ATH_PCIE_WMAC_INT_STATUS	ATH_RESET_BASE+0xac
#	define WMAC_MISC_INT	(1 << 0)	/* Indicates there is a WMAC Intr */
#	define WMAC_TX_INT	(1 << 1)	/* Reason of interrupt */
#	define WMAC_RXLP_INT	(1 << 2)
#	define WMAC_RXHP_INT	(1 << 3)
#	define PCIE_RC_INT	(1 << 4)
#	define PCIE_RC_INT0	(1 << 5)
#	define PCIE_RC_INT1	(1 << 6)
#	define PCIE_RC_INT2	(1 << 7)
#	define PCIE_RC_INT3	(1 << 8)

#	define PCI_WMAC_INTR	(PCIE_RC_INT | PCIE_RC_INT0 | PCIE_RC_INT1 | \
				 PCIE_RC_INT2 | PCIE_RC_INT3)


#define ATH_WD_ACT_MASK			3u
#define ATH_WD_ACT_NONE			0u /* No Action */
#define ATH_WD_ACT_GP_INTR		1u /* General purpose intr */
#define ATH_WD_ACT_NMI			2u /* NMI */
#define ATH_WD_ACT_RESET		3u /* Full Chip Reset */

#define ATH_WD_LAST_SHIFT		31
#define ATH_WD_LAST_MASK		((uint32_t)(1 << ATH_WD_LAST_SHIFT))



/*
 * Performace counters
 */
#define ATH_PERF0_COUNTER		ATH_GE0_BASE+0xa0
#define ATH_PERF1_COUNTER		ATH_GE1_BASE+0xa0

/*
 * SLIC/STEREO DMA Size Configurations
 */
#define ATH_DMA_BUF_SIZE_4X2		0x00
#define ATH_DMA_BUF_SIZE_8X2		0x01
#define ATH_DMA_BUF_SIZE_16X2		0x02
#define ATH_DMA_BUF_SIZE_32X2		0x03
#define ATH_DMA_BUF_SIZE_64X2		0x04
#define ATH_DMA_BUF_SIZE_128X2		0x05
#define ATH_DMA_BUF_SIZE_256X2		0x06
#define ATH_DMA_BUF_SIZE_512X2		0x07

/*
 * SLIC/STEREO DMA Assignments
 */
#define ATH_DMA_CHAN_SLIC0_RX		0
#define ATH_DMA_CHAN_SLIC1_RX		1
#define ATH_DMA_CHAN_STEREO_RX		2
#define ATH_DMA_CHAN_SLIC0_TX		3
#define ATH_DMA_CHAN_SLIC1_TX		4
#define ATH_DMA_CHAN_STEREO_TX		5

/*
 * MBOX register definitions
 */
#define ATH_MBOX_FIFO				(ATH_DMA_BASE+0x00)
#define ATH_MBOX_FIFO_STATUS			(ATH_DMA_BASE+0x08)
#define ATH_MBOX_SLIC_FIFO_STATUS		(ATH_DMA_BASE+0x0c)
#define ATH_MBOX_DMA_POLICY			(ATH_DMA_BASE+0x10)
#define ATH_MBOX_SLIC_DMA_POLICY		(ATH_DMA_BASE+0x14)
#define ATH_MBOX_DMA_RX_DESCRIPTOR_BASE0	(ATH_DMA_BASE+0x18)
#define ATH_MBOX_DMA_RX_CONTROL0		(ATH_DMA_BASE+0x1c)
#define ATH_MBOX_DMA_TX_DESCRIPTOR_BASE0	(ATH_DMA_BASE+0x20)
#define ATH_MBOX_DMA_TX_CONTROL0		(ATH_DMA_BASE+0x24)
#define ATH_MBOX_DMA_RX_DESCRIPTOR_BASE1	(ATH_DMA_BASE+0x28)
#define ATH_MBOX_DMA_RX_CONTROL1		(ATH_DMA_BASE+0x2c)
#define ATH_MBOX_DMA_TX_DESCRIPTOR_BASE1	(ATH_DMA_BASE+0x30)
#define ATH_MBOX_DMA_TX_CONTROL1		(ATH_DMA_BASE+0x34)
#define ATH_MBOX_FRAME				(ATH_DMA_BASE+0x34)
#define ATH_MBOX_SLIC_FRAME			(ATH_DMA_BASE+0x3c)
#define ATH_MBOX_FIFO_TIMEOUT			(ATH_DMA_BASE+0x40)
#define ATH_MBOX_INT_STATUS			(ATH_DMA_BASE+0x44)
#define ATH_MBOX_SLIC_INT_STATUS		(ATH_DMA_BASE+0x48)
#define ATH_MBOX_INT_ENABLE			(ATH_DMA_BASE+0x4c)
#define ATH_MBOX_SLIC_INT_ENABLE		(ATH_DMA_BASE+0x50)
#define ATH_MBOX_FIFO_RESET			(ATH_DMA_BASE+0x58)
#define ATH_MBOX_SLIC_FIFO_RESET		(ATH_DMA_BASE+0x5c)

#define ATH_MBOX_DMA_POLICY_RX_QUANTUM		(1<< 1)
#define ATH_MBOX_DMA_POLICY_TX_QUANTUM		(1<< 3)
#define ATH_MBOX_DMA_POLICY_TX_FIFO_THRESH(x)	((0xff&x)<< 4)

/*
 * MBOX Enables
 */
#define ATH_MBOX_DMA_PAUSE                      0x1
#define ATH_MBOX_DMA_START                      0x2
#define ATH_MBOX_DMA_RESUME                     0x4

#define ATH_MBOX_DMA_POLICY_RX_QUANTUM          (1<< 1)
#define ATH_MBOX_DMA_POLICY_TX_QUANTUM          (1<< 3)
#define ATH_MBOX_DMA_POLICY_TX_FIFO_THRESH(x)   ((0xff&x)<< 4)

#define ATH_MBOX_RX_DMA_COMPLETE                (1 << 10)
#define ATH_MBOX_TX_DMA_EOM_COMPLETE            (1 << 8)
#define ATH_MBOX_TX_DMA_COMPLETE                (1 << 6)
#define ATH_MBOX_TX_OVERFLOW                    (1 << 5)
#define ATH_MBOX_RX_UNDERFLOW                   (1 << 4)
#define ATH_MBOX_TX_NOT_EMPTY                   (1 << 2)
#define ATH_MBOX_RX_NOT_FULL                    (1 << 0)


/*
 * SLIC register definitions
 */
#define ATH_SLIC_SLOT				(ATH_SLIC_BASE+0x00)
#define ATH_SLIC_CLOCK_CTRL			(ATH_SLIC_BASE+0x04)
#define ATH_SLIC_CTRL				(ATH_SLIC_BASE+0x08)
#define ATH_SLIC_TX_SLOTS1			(ATH_SLIC_BASE+0x0c)
#define ATH_SLIC_TX_SLOTS2			(ATH_SLIC_BASE+0x10)
#define ATH_SLIC_RX_SLOTS1			(ATH_SLIC_BASE+0x14)
#define ATH_SLIC_RX_SLOTS2			(ATH_SLIC_BASE+0x18)
#define ATH_SLIC_TIMING_CTRL			(ATH_SLIC_BASE+0x1c)
#define ATH_SLIC_INTR				(ATH_SLIC_BASE+0x20)
#define ATH_SLIC_SWAP				(ATH_SLIC_BASE+0x24)

/*
 * SLIC Control bits
 */
#define ATH_SLIC_CNTRL_ENABLE			(1<<0)
#define ATH_SLIC_CNTRL_SLOT0_ENABLE		(1<<1)
#define ATH_SLIC_CNTRL_SLOT1_ENABLE		(1<<2)
#define ATH_SLIC_CNTRL_IRQ_ENABLE		(1<<3)

/*
 * STEREO register definitions
 */
#define ATH_STEREO_CONFIG			(ATH_STEREO_BASE+0x00)
#define ATH_STEREO_VOLUME			(ATH_STEREO_BASE+0x04)
#define ATH_STEREO_MCLK				(ATH_STEREO_BASE+0x08)

/*
 * Stereo Configuration Bits
 */
#define ATH_STEREO_CONFIG_SPDIF_ENABLE		(1<<23)
#define ATH_STEREO_CONFIG_ENABLE		(1<<21)
#define ATH_STEREO_CONFIG_RESET			(1<<19)
#define ATH_STEREO_CONFIG_DELAY			(1<<18)
#define ATH_STEREO_CONFIG_PCM_SWAP		(1<<17)
#define ATH_STEREO_CONFIG_MIC_WORD_SIZE		(1<<16)
#define ATH_STEREO_CONFIG_MODE(x)		((3&x)<<14)
#define ATH_STEREO_MODE_STEREO			0
#define ATH_STEREO_MODE_LEFT			1
#define ATH_STEREO_MODE_RIGHT			2
#define ATH_STEREO_CONFIG_DATA_WORD_SIZE(x)	((3&x)<<12)
#define ATH_STEREO_CONFIG_I2S_32B_WORD		(1<<11)
#define ATH_STEREO_CONFIG_I2S_MCLK_SEL		(1<<10)
#define ATH_STEREO_CONFIG_SAMPLE_CNT_CLEAR_TYPE	(1<<9)
#define ATH_STEREO_CONFIG_MASTER		(1<<8)
#define ATH_STEREO_CONFIG_PSEDGE(x)		(0xff&x)

/*
 * Word sizes to use with common configurations:
 */
#define ATH_STEREO_WS_8B		0
#define ATH_STEREO_WS_16B		1
#define ATH_STEREO_WS_24B		2
#define ATH_STEREO_WS_32B		3

/*
 * Slic Configuration Bits
 */
#define ATH_SLIC_SLOT_SEL(x)				(0x7f&x)
#define ATH_SLIC_CLOCK_CTRL_DIV(x)			(0x3f&x)
#define ATH_SLIC_CTRL_CLK_EN				(1<<3)
#define ATH_SLIC_CTRL_MASTER				(1<<2)
#define ATH_SLIC_CTRL_EN				(1<<1)
#define ATH_SLIC_TX_SLOTS1_EN(x)			(x)
#define ATH_SLIC_TX_SLOTS2_EN(x)			(x)
#define ATH_SLIC_RX_SLOTS1_EN(x)			(x)
#define ATH_SLIC_RX_SLOTS2_EN(x)			(x)
#define ATH_SLIC_TIMING_CTRL_RXDATA_SAMPLE_POS_EXTEND	(1<<11)
#define ATH_SLIC_TIMING_CTRL_DATAOEN_ALWAYS		(1<<9)
#define ATH_SLIC_TIMING_CTRL_RXDATA_SAMPLE_POS(x)	((0x3&x)<<7)
#define ATH_SLIC_TIMING_CTRL_TXDATA_FS_SYNC(x)		((0x3&x)<<5)
#define ATH_SLIC_TIMING_CTRL_LONG_FSCLKS(x)		((0x7&x)<<2)
#define ATH_SLIC_TIMING_CTRL_FS_POS			(1<<1)
#define ATH_SLIC_TIMING_CTRL_LONG_FS			(1<<0)
#define ATH_SLIC_INTR_MASK(x)				(0x1f&x)
#define ATH_SLIC_SWAP_RX_DATA				(1<<1)
#define ATH_SLIC_SWAP_TX_DATA				(1<<0)

#define ATH_SLIC_TIMING_CTRL_RXDATA_SAMPLE_POS_2_NGEDGE	2
#define ATH_SLIC_TIMING_CTRL_RXDATA_SAMPLE_POS_1_NGEDGE	1
#define ATH_SLIC_TIMING_CTRL_TXDATA_FS_SYNC_NXT_PSEDGE	2
#define ATH_SLIC_TIMING_CTRL_TXDATA_FS_SYNC_NXT_NGEDGE	3
#define ATH_SLIC_TIMING_CTRL_LONG_FSCLKS_1BIT		0
#define ATH_SLIC_TIMING_CTRL_LONG_FSCLKS_8BIT		7
#define ATH_SLIC_INTR_STATUS_NO_INTR			0
#define ATH_SLIC_INTR_STATUS_UNEXP_FRAME		1
#define ATH_SLIC_INTR_MASK_RESET			0x1f
#define ATH_SLIC_INTR_MASK_0				1
#define ATH_SLIC_INTR_MASK_1				2
#define ATH_SLIC_INTR_MASK_2				4
#define ATH_SLIC_INTR_MASK_3				8
#define ATH_SLIC_INTR_MASK_4				16

/*
 * Common configurations for stereo block
 */
#define ATH_STEREO_CFG_MASTER_STEREO_FS32_48KHZ(ws) ( \
	ATH_STEREO_CONFIG_DELAY | \
	ATH_STEREO_CONFIG_RESET | \
	ATH_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
	ATH_STEREO_CONFIG_MODE(ATH_STEREO_MODE_LEFT) | \
	ATH_STEREO_CONFIG_MASTER | \
	ATH_STEREO_CONFIG_PSEDGE(26))

#define ATH_STEREO_CFG_MASTER_STEREO_FS64_48KHZ(ws) ( \
	ATH_STEREO_CONFIG_DELAY | \
	ATH_STEREO_CONFIG_RESET | \
	ATH_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
	ATH_STEREO_CONFIG_MODE(ATH_STEREO_MODE_STEREO) | \
	ATH_STEREO_CONFIG_I2S_32B_WORD | \
	ATH_STEREO_CONFIG_MASTER | \
	ATH_STEREO_CONFIG_PSEDGE(13))

#define ATH_STEREO_CFG_SLAVE_STEREO_FS32_48KHZ(ws) ( \
	ATH_STEREO_CONFIG_RESET | \
	ATH_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
	ATH_STEREO_CONFIG_MODE(ATH_STEREO_MODE_STEREO) | \
	ATH_STEREO_CONFIG_PSEDGE(26))

#define ATH_STEREO_CFG_SLAVE_STEREO_FS64_48KHZ(ws) ( \
	ATH_STEREO_CONFIG_RESET | \
	ATH_STEREO_CONFIG_I2S_32B_WORD | \
	ATH_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
	ATH_STEREO_CONFIG_MODE(ATH_STEREO_MODE_STEREO) | \
	ATH_STEREO_CONFIG_PSEDGE(13))

/*
 * PERF CTL bits
 */
#define PERF_CTL_PCI_AHB_0		( 0)
#define PERF_CTL_PCI_AHB_1		( 1)
#define PERF_CTL_USB_0			( 2)
#define PERF_CTL_USB_1			( 3)
#define PERF_CTL_GE0_PKT_CNT		( 4)
#define PERF_CTL_GEO_AHB_1		( 5)
#define PERF_CTL_GE1_PKT_CNT		( 6)
#define PERF_CTL_GE1_AHB_1		( 7)
#define PERF_CTL_PCI_DEV_0_BUSY		( 8)
#define PERF_CTL_PCI_DEV_1_BUSY		( 9)
#define PERF_CTL_PCI_DEV_2_BUSY		(10)
#define PERF_CTL_PCI_HOST_BUSY		(11)
#define PERF_CTL_PCI_DEV_0_ARB		(12)
#define PERF_CTL_PCI_DEV_1_ARB		(13)
#define PERF_CTL_PCI_DEV_2_ARB		(14)
#define PERF_CTL_PCI_HOST_ARB		(15)
#define PERF_CTL_PCI_DEV_0_ACTIVE	(16)
#define PERF_CTL_PCI_DEV_1_ACTIVE	(17)
#define PERF_CTL_PCI_DEV_2_ACTIVE	(18)
#define PERF_CTL_HOST_ACTIVE		(19)

#define ath_perf0_ctl(_val) ath_reg_wr(ATH_PERF_CTL, (_val))
#define ath_perf1_ctl(_val) ath_reg_rmw_set(ATH_PERF_CTL, ((_val) << 8))

#define ATH_PLL_USE_REV_ID		0
#define ATH_PLL_200_200_100		1
#define ATH_PLL_300_300_150		2
#define ATH_PLL_333_333_166		3
#define ATH_PLL_266_266_133		4
#define ATH_PLL_266_266_66		5
#define ATH_PLL_400_400_200		6
#define ATH_PLL_600_400_150		7


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
#define ATH_RESET_USB_PHY_PLL_PWD_EXT	(1 << 15)
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

#define RST_RESET_HOST_RESET_MSB                                     31
#define RST_RESET_HOST_RESET_LSB                                     31
#define RST_RESET_HOST_RESET_MASK                                    0x80000000
#define RST_RESET_HOST_RESET_GET(x)                                  (((x) & RST_RESET_HOST_RESET_MASK) >> RST_RESET_HOST_RESET_LSB)
#define RST_RESET_HOST_RESET_SET(x)                                  (((x) << RST_RESET_HOST_RESET_LSB) & RST_RESET_HOST_RESET_MASK)
#define RST_RESET_HOST_RESET_RESET                                   0x0 // 0
#define RST_RESET_SLIC_RESET_MSB                                     30
#define RST_RESET_SLIC_RESET_LSB                                     30
#define RST_RESET_SLIC_RESET_MASK                                    0x40000000
#define RST_RESET_SLIC_RESET_GET(x)                                  (((x) & RST_RESET_SLIC_RESET_MASK) >> RST_RESET_SLIC_RESET_LSB)
#define RST_RESET_SLIC_RESET_SET(x)                                  (((x) << RST_RESET_SLIC_RESET_LSB) & RST_RESET_SLIC_RESET_MASK)
#define RST_RESET_SLIC_RESET_RESET                                   0x0 // 0
#define RST_RESET_HDMA_RESET_MSB                                     29
#define RST_RESET_HDMA_RESET_LSB                                     29
#define RST_RESET_HDMA_RESET_MASK                                    0x20000000
#define RST_RESET_HDMA_RESET_GET(x)                                  (((x) & RST_RESET_HDMA_RESET_MASK) >> RST_RESET_HDMA_RESET_LSB)
#define RST_RESET_HDMA_RESET_SET(x)                                  (((x) << RST_RESET_HDMA_RESET_LSB) & RST_RESET_HDMA_RESET_MASK)
#define RST_RESET_HDMA_RESET_RESET                                   0x1 // 1
#define RST_RESET_EXTERNAL_RESET_MSB                                 28
#define RST_RESET_EXTERNAL_RESET_LSB                                 28
#define RST_RESET_EXTERNAL_RESET_MASK                                0x10000000
#define RST_RESET_EXTERNAL_RESET_GET(x)                              (((x) & RST_RESET_EXTERNAL_RESET_MASK) >> RST_RESET_EXTERNAL_RESET_LSB)
#define RST_RESET_EXTERNAL_RESET_SET(x)                              (((x) << RST_RESET_EXTERNAL_RESET_LSB) & RST_RESET_EXTERNAL_RESET_MASK)
#define RST_RESET_EXTERNAL_RESET_RESET                               0x0 // 0
#define RST_RESET_RTC_RESET_MSB                                      27
#define RST_RESET_RTC_RESET_LSB                                      27
#define RST_RESET_RTC_RESET_MASK                                     0x08000000
#define RST_RESET_RTC_RESET_GET(x)                                   (((x) & RST_RESET_RTC_RESET_MASK) >> RST_RESET_RTC_RESET_LSB)
#define RST_RESET_RTC_RESET_SET(x)                                   (((x) << RST_RESET_RTC_RESET_LSB) & RST_RESET_RTC_RESET_MASK)
#define RST_RESET_RTC_RESET_RESET                                    0x1 // 1
#define RST_RESET_PCIEEP_RST_INT_MSB                                 26
#define RST_RESET_PCIEEP_RST_INT_LSB                                 26
#define RST_RESET_PCIEEP_RST_INT_MASK                                0x04000000
#define RST_RESET_PCIEEP_RST_INT_GET(x)                              (((x) & RST_RESET_PCIEEP_RST_INT_MASK) >> RST_RESET_PCIEEP_RST_INT_LSB)
#define RST_RESET_PCIEEP_RST_INT_SET(x)                              (((x) << RST_RESET_PCIEEP_RST_INT_LSB) & RST_RESET_PCIEEP_RST_INT_MASK)
#define RST_RESET_PCIEEP_RST_INT_RESET                               0x0 // 0
#define RST_RESET_CHKSUM_ACC_RESET_MSB                               25
#define RST_RESET_CHKSUM_ACC_RESET_LSB                               25
#define RST_RESET_CHKSUM_ACC_RESET_MASK                              0x02000000
#define RST_RESET_CHKSUM_ACC_RESET_GET(x)                            (((x) & RST_RESET_CHKSUM_ACC_RESET_MASK) >> RST_RESET_CHKSUM_ACC_RESET_LSB)
#define RST_RESET_CHKSUM_ACC_RESET_SET(x)                            (((x) << RST_RESET_CHKSUM_ACC_RESET_LSB) & RST_RESET_CHKSUM_ACC_RESET_MASK)
#define RST_RESET_CHKSUM_ACC_RESET_RESET                             0x0 // 0
#define RST_RESET_FULL_CHIP_RESET_MSB                                24
#define RST_RESET_FULL_CHIP_RESET_LSB                                24
#define RST_RESET_FULL_CHIP_RESET_MASK                               0x01000000
#define RST_RESET_FULL_CHIP_RESET_GET(x)                             (((x) & RST_RESET_FULL_CHIP_RESET_MASK) >> RST_RESET_FULL_CHIP_RESET_LSB)
#define RST_RESET_FULL_CHIP_RESET_SET(x)                             (((x) << RST_RESET_FULL_CHIP_RESET_LSB) & RST_RESET_FULL_CHIP_RESET_MASK)
#define RST_RESET_FULL_CHIP_RESET_RESET                              0x0 // 0
#define RST_RESET_GE1_MDIO_RESET_MSB                                 23
#define RST_RESET_GE1_MDIO_RESET_LSB                                 23
#define RST_RESET_GE1_MDIO_RESET_MASK                                0x00800000
#define RST_RESET_GE1_MDIO_RESET_GET(x)                              (((x) & RST_RESET_GE1_MDIO_RESET_MASK) >> RST_RESET_GE1_MDIO_RESET_LSB)
#define RST_RESET_GE1_MDIO_RESET_SET(x)                              (((x) << RST_RESET_GE1_MDIO_RESET_LSB) & RST_RESET_GE1_MDIO_RESET_MASK)
#define RST_RESET_GE1_MDIO_RESET_RESET                               0x1 // 1
#define RST_RESET_GE0_MDIO_RESET_MSB                                 22
#define RST_RESET_GE0_MDIO_RESET_LSB                                 22
#define RST_RESET_GE0_MDIO_RESET_MASK                                0x00400000
#define RST_RESET_GE0_MDIO_RESET_GET(x)                              (((x) & RST_RESET_GE0_MDIO_RESET_MASK) >> RST_RESET_GE0_MDIO_RESET_LSB)
#define RST_RESET_GE0_MDIO_RESET_SET(x)                              (((x) << RST_RESET_GE0_MDIO_RESET_LSB) & RST_RESET_GE0_MDIO_RESET_MASK)
#define RST_RESET_GE0_MDIO_RESET_RESET                               0x1 // 1
#define RST_RESET_CPU_NMI_MSB                                        21
#define RST_RESET_CPU_NMI_LSB                                        21
#define RST_RESET_CPU_NMI_MASK                                       0x00200000
#define RST_RESET_CPU_NMI_GET(x)                                     (((x) & RST_RESET_CPU_NMI_MASK) >> RST_RESET_CPU_NMI_LSB)
#define RST_RESET_CPU_NMI_SET(x)                                     (((x) << RST_RESET_CPU_NMI_LSB) & RST_RESET_CPU_NMI_MASK)
#define RST_RESET_CPU_NMI_RESET                                      0x0 // 0
#define RST_RESET_CPU_COLD_RESET_MSB                                 20
#define RST_RESET_CPU_COLD_RESET_LSB                                 20
#define RST_RESET_CPU_COLD_RESET_MASK                                0x00100000
#define RST_RESET_CPU_COLD_RESET_GET(x)                              (((x) & RST_RESET_CPU_COLD_RESET_MASK) >> RST_RESET_CPU_COLD_RESET_LSB)
#define RST_RESET_CPU_COLD_RESET_SET(x)                              (((x) << RST_RESET_CPU_COLD_RESET_LSB) & RST_RESET_CPU_COLD_RESET_MASK)
#define RST_RESET_CPU_COLD_RESET_RESET                               0x0 // 0
#define RST_RESET_HOST_RESET_INT_MSB                                 19
#define RST_RESET_HOST_RESET_INT_LSB                                 19
#define RST_RESET_HOST_RESET_INT_MASK                                0x00080000
#define RST_RESET_HOST_RESET_INT_GET(x)                              (((x) & RST_RESET_HOST_RESET_INT_MASK) >> RST_RESET_HOST_RESET_INT_LSB)
#define RST_RESET_HOST_RESET_INT_SET(x)                              (((x) << RST_RESET_HOST_RESET_INT_LSB) & RST_RESET_HOST_RESET_INT_MASK)
#define RST_RESET_HOST_RESET_INT_RESET                               0x0 // 0
#define RST_RESET_PCIEEP_RESET_MSB                                   18
#define RST_RESET_PCIEEP_RESET_LSB                                   18
#define RST_RESET_PCIEEP_RESET_MASK                                  0x00040000
#define RST_RESET_PCIEEP_RESET_GET(x)                                (((x) & RST_RESET_PCIEEP_RESET_MASK) >> RST_RESET_PCIEEP_RESET_LSB)
#define RST_RESET_PCIEEP_RESET_SET(x)                                (((x) << RST_RESET_PCIEEP_RESET_LSB) & RST_RESET_PCIEEP_RESET_MASK)
#define RST_RESET_PCIEEP_RESET_RESET                                 0x0 // 0
#define RST_RESET_UART1_RESET_MSB                                    17
#define RST_RESET_UART1_RESET_LSB                                    17
#define RST_RESET_UART1_RESET_MASK                                   0x00020000
#define RST_RESET_UART1_RESET_GET(x)                                 (((x) & RST_RESET_UART1_RESET_MASK) >> RST_RESET_UART1_RESET_LSB)
#define RST_RESET_UART1_RESET_SET(x)                                 (((x) << RST_RESET_UART1_RESET_LSB) & RST_RESET_UART1_RESET_MASK)
#define RST_RESET_UART1_RESET_RESET                                  0x0 // 0
#define RST_RESET_DDR_RESET_MSB                                      16
#define RST_RESET_DDR_RESET_LSB                                      16
#define RST_RESET_DDR_RESET_MASK                                     0x00010000
#define RST_RESET_DDR_RESET_GET(x)                                   (((x) & RST_RESET_DDR_RESET_MASK) >> RST_RESET_DDR_RESET_LSB)
#define RST_RESET_DDR_RESET_SET(x)                                   (((x) << RST_RESET_DDR_RESET_LSB) & RST_RESET_DDR_RESET_MASK)
#define RST_RESET_DDR_RESET_RESET                                    0x0 // 0
#define RST_RESET_USB_PHY_PLL_PWD_EXT_MSB                            15
#define RST_RESET_USB_PHY_PLL_PWD_EXT_LSB                            15
#define RST_RESET_USB_PHY_PLL_PWD_EXT_MASK                           0x00008000
#define RST_RESET_USB_PHY_PLL_PWD_EXT_GET(x)                         (((x) & RST_RESET_USB_PHY_PLL_PWD_EXT_MASK) >> RST_RESET_USB_PHY_PLL_PWD_EXT_LSB)
#define RST_RESET_USB_PHY_PLL_PWD_EXT_SET(x)                         (((x) << RST_RESET_USB_PHY_PLL_PWD_EXT_LSB) & RST_RESET_USB_PHY_PLL_PWD_EXT_MASK)
#define RST_RESET_USB_PHY_PLL_PWD_EXT_RESET                          0x0 // 0
#define RST_RESET_NANDF_RESET_MSB                                    14
#define RST_RESET_NANDF_RESET_LSB                                    14
#define RST_RESET_NANDF_RESET_MASK                                   0x00004000
#define RST_RESET_NANDF_RESET_GET(x)                                 (((x) & RST_RESET_NANDF_RESET_MASK) >> RST_RESET_NANDF_RESET_LSB)
#define RST_RESET_NANDF_RESET_SET(x)                                 (((x) << RST_RESET_NANDF_RESET_LSB) & RST_RESET_NANDF_RESET_MASK)
#define RST_RESET_NANDF_RESET_RESET                                  0x1 // 1
#define RST_RESET_GE1_MAC_RESET_MSB                                  13
#define RST_RESET_GE1_MAC_RESET_LSB                                  13
#define RST_RESET_GE1_MAC_RESET_MASK                                 0x00002000
#define RST_RESET_GE1_MAC_RESET_GET(x)                               (((x) & RST_RESET_GE1_MAC_RESET_MASK) >> RST_RESET_GE1_MAC_RESET_LSB)
#define RST_RESET_GE1_MAC_RESET_SET(x)                               (((x) << RST_RESET_GE1_MAC_RESET_LSB) & RST_RESET_GE1_MAC_RESET_MASK)
#define RST_RESET_GE1_MAC_RESET_RESET                                0x1 // 1
#define RST_RESET_ETH_SWITCH_ARESET_MSB                              12
#define RST_RESET_ETH_SWITCH_ARESET_LSB                              12
#define RST_RESET_ETH_SWITCH_ARESET_MASK                             0x00001000
#define RST_RESET_ETH_SWITCH_ARESET_GET(x)                           (((x) & RST_RESET_ETH_SWITCH_ARESET_MASK) >> RST_RESET_ETH_SWITCH_ARESET_LSB)
#define RST_RESET_ETH_SWITCH_ARESET_SET(x)                           (((x) << RST_RESET_ETH_SWITCH_ARESET_LSB) & RST_RESET_ETH_SWITCH_ARESET_MASK)
#define RST_RESET_ETH_SWITCH_ARESET_RESET                            0x1 // 1
#define RST_RESET_USB_PHY_ARESET_MSB                                 11
#define RST_RESET_USB_PHY_ARESET_LSB                                 11
#define RST_RESET_USB_PHY_ARESET_MASK                                0x00000800
#define RST_RESET_USB_PHY_ARESET_GET(x)                              (((x) & RST_RESET_USB_PHY_ARESET_MASK) >> RST_RESET_USB_PHY_ARESET_LSB)
#define RST_RESET_USB_PHY_ARESET_SET(x)                              (((x) << RST_RESET_USB_PHY_ARESET_LSB) & RST_RESET_USB_PHY_ARESET_MASK)
#define RST_RESET_USB_PHY_ARESET_RESET                               0x1 // 1
#define RST_RESET_HOST_DMA_INT_MSB                                   10
#define RST_RESET_HOST_DMA_INT_LSB                                   10
#define RST_RESET_HOST_DMA_INT_MASK                                  0x00000400
#define RST_RESET_HOST_DMA_INT_GET(x)                                (((x) & RST_RESET_HOST_DMA_INT_MASK) >> RST_RESET_HOST_DMA_INT_LSB)
#define RST_RESET_HOST_DMA_INT_SET(x)                                (((x) << RST_RESET_HOST_DMA_INT_LSB) & RST_RESET_HOST_DMA_INT_MASK)
#define RST_RESET_HOST_DMA_INT_RESET                                 0x0 // 0
#define RST_RESET_GE0_MAC_RESET_MSB                                  9
#define RST_RESET_GE0_MAC_RESET_LSB                                  9
#define RST_RESET_GE0_MAC_RESET_MASK                                 0x00000200
#define RST_RESET_GE0_MAC_RESET_GET(x)                               (((x) & RST_RESET_GE0_MAC_RESET_MASK) >> RST_RESET_GE0_MAC_RESET_LSB)
#define RST_RESET_GE0_MAC_RESET_SET(x)                               (((x) << RST_RESET_GE0_MAC_RESET_LSB) & RST_RESET_GE0_MAC_RESET_MASK)
#define RST_RESET_GE0_MAC_RESET_RESET                                0x1 // 1
#define RST_RESET_ETH_SWITCH_RESET_MSB                               8
#define RST_RESET_ETH_SWITCH_RESET_LSB                               8
#define RST_RESET_ETH_SWITCH_RESET_MASK                              0x00000100
#define RST_RESET_ETH_SWITCH_RESET_GET(x)                            (((x) & RST_RESET_ETH_SWITCH_RESET_MASK) >> RST_RESET_ETH_SWITCH_RESET_LSB)
#define RST_RESET_ETH_SWITCH_RESET_SET(x)                            (((x) << RST_RESET_ETH_SWITCH_RESET_LSB) & RST_RESET_ETH_SWITCH_RESET_MASK)
#define RST_RESET_ETH_SWITCH_RESET_RESET                             0x1 // 1
#define RST_RESET_PCIE_PHY_RESET_MSB                                 7
#define RST_RESET_PCIE_PHY_RESET_LSB                                 7
#define RST_RESET_PCIE_PHY_RESET_MASK                                0x00000080
#define RST_RESET_PCIE_PHY_RESET_GET(x)                              (((x) & RST_RESET_PCIE_PHY_RESET_MASK) >> RST_RESET_PCIE_PHY_RESET_LSB)
#define RST_RESET_PCIE_PHY_RESET_SET(x)                              (((x) << RST_RESET_PCIE_PHY_RESET_LSB) & RST_RESET_PCIE_PHY_RESET_MASK)
#define RST_RESET_PCIE_PHY_RESET_RESET                               0x1 // 1
#define RST_RESET_PCIE_RESET_MSB                                     6
#define RST_RESET_PCIE_RESET_LSB                                     6
#define RST_RESET_PCIE_RESET_MASK                                    0x00000040
#define RST_RESET_PCIE_RESET_GET(x)                                  (((x) & RST_RESET_PCIE_RESET_MASK) >> RST_RESET_PCIE_RESET_LSB)
#define RST_RESET_PCIE_RESET_SET(x)                                  (((x) << RST_RESET_PCIE_RESET_LSB) & RST_RESET_PCIE_RESET_MASK)
#define RST_RESET_PCIE_RESET_RESET                                   0x1 // 1
#define RST_RESET_USB_HOST_RESET_MSB                                 5
#define RST_RESET_USB_HOST_RESET_LSB                                 5
#define RST_RESET_USB_HOST_RESET_MASK                                0x00000020
#define RST_RESET_USB_HOST_RESET_GET(x)                              (((x) & RST_RESET_USB_HOST_RESET_MASK) >> RST_RESET_USB_HOST_RESET_LSB)
#define RST_RESET_USB_HOST_RESET_SET(x)                              (((x) << RST_RESET_USB_HOST_RESET_LSB) & RST_RESET_USB_HOST_RESET_MASK)
#define RST_RESET_USB_HOST_RESET_RESET                               0x1 // 1
#define RST_RESET_USB_PHY_RESET_MSB                                  4
#define RST_RESET_USB_PHY_RESET_LSB                                  4
#define RST_RESET_USB_PHY_RESET_MASK                                 0x00000010
#define RST_RESET_USB_PHY_RESET_GET(x)                               (((x) & RST_RESET_USB_PHY_RESET_MASK) >> RST_RESET_USB_PHY_RESET_LSB)
#define RST_RESET_USB_PHY_RESET_SET(x)                               (((x) << RST_RESET_USB_PHY_RESET_LSB) & RST_RESET_USB_PHY_RESET_MASK)
#define RST_RESET_USB_PHY_RESET_RESET                                0x1 // 1
#define RST_RESET_USB_PHY_SUSPEND_OVERRIDE_MSB                       3
#define RST_RESET_USB_PHY_SUSPEND_OVERRIDE_LSB                       3
#define RST_RESET_USB_PHY_SUSPEND_OVERRIDE_MASK                      0x00000008
#define RST_RESET_USB_PHY_SUSPEND_OVERRIDE_GET(x)                    (((x) & RST_RESET_USB_PHY_SUSPEND_OVERRIDE_MASK) >> RST_RESET_USB_PHY_SUSPEND_OVERRIDE_LSB)
#define RST_RESET_USB_PHY_SUSPEND_OVERRIDE_SET(x)                    (((x) << RST_RESET_USB_PHY_SUSPEND_OVERRIDE_LSB) & RST_RESET_USB_PHY_SUSPEND_OVERRIDE_MASK)
#define RST_RESET_USB_PHY_SUSPEND_OVERRIDE_RESET                     0x0 // 0
#define RST_RESET_LUT_RESET_MSB                                      2
#define RST_RESET_LUT_RESET_LSB                                      2
#define RST_RESET_LUT_RESET_MASK                                     0x00000004
#define RST_RESET_LUT_RESET_GET(x)                                   (((x) & RST_RESET_LUT_RESET_MASK) >> RST_RESET_LUT_RESET_LSB)
#define RST_RESET_LUT_RESET_SET(x)                                   (((x) << RST_RESET_LUT_RESET_LSB) & RST_RESET_LUT_RESET_MASK)
#define RST_RESET_LUT_RESET_RESET                                    0x0 // 0
#define RST_RESET_MBOX_RESET_MSB                                     1
#define RST_RESET_MBOX_RESET_LSB                                     1
#define RST_RESET_MBOX_RESET_MASK                                    0x00000002
#define RST_RESET_MBOX_RESET_GET(x)                                  (((x) & RST_RESET_MBOX_RESET_MASK) >> RST_RESET_MBOX_RESET_LSB)
#define RST_RESET_MBOX_RESET_SET(x)                                  (((x) << RST_RESET_MBOX_RESET_LSB) & RST_RESET_MBOX_RESET_MASK)
#define RST_RESET_MBOX_RESET_RESET                                   0x0 // 0
#define RST_RESET_I2S_RESET_MSB                                      0
#define RST_RESET_I2S_RESET_LSB                                      0
#define RST_RESET_I2S_RESET_MASK                                     0x00000001
#define RST_RESET_I2S_RESET_GET(x)                                   (((x) & RST_RESET_I2S_RESET_MASK) >> RST_RESET_I2S_RESET_LSB)
#define RST_RESET_I2S_RESET_SET(x)                                   (((x) << RST_RESET_I2S_RESET_LSB) & RST_RESET_I2S_RESET_MASK)
#define RST_RESET_I2S_RESET_RESET                                    0x0 // 0
#define RST_RESET_ADDRESS                                            0x1806001c
#define RST_RESET_OFFSET                                             0x001c
// SW modifiable bits
#define RST_RESET_SW_MASK                                            0xffffffff
// bits defined at reset
#define RST_RESET_RSTMASK                                            0xffffffff
// reset value (ignore bits undefined at reset)
#define RST_RESET_RESET                                              0x28c07bf0
#define ATH_REV_ID			(ATH_RESET_BASE + 0x90)
#define ATH_REV_ID_MASK_MAJOR		0xfff0u
#define ATH_REV_ID_MASK_MINOR		0xfu
#define ATH_REV_ID_MASK			\
	(ATH_REV_ID_MASK_MAJOR | ATH_REV_ID_MASK_MINOR)

#define ATH_AR9344_1_x			0x2120
#define ATH_AR9342_1_x			0x1120
#define ATH_AR9341_1_x			0x0120

#define ATH_AR9344_1_0			0x2120
#define ATH_AR9342_1_0			0x1120
#define ATH_AR9341_1_0			0x0120

#define ATH_AR9344_1_1			0x2121
#define ATH_AR9342_1_1			0x1121
#define ATH_AR9341_1_1			0x0121

#undef is_ar934x
#undef is_wasp

#define is_ar934x()	(1)
#define is_wasp()	(1)

#define is_ar9344()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK_MAJOR) == ATH_AR9344_1_x)
#define is_ar9342()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK_MAJOR) == ATH_AR9342_1_x)
#define is_ar9341()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK_MAJOR) == ATH_AR9341_1_x)

#define is_ar9344_10()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK) == ATH_AR9344_1_0)
#define is_ar9342_10()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK) == ATH_AR9342_1_0)
#define is_ar9341_10()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK) == ATH_AR9341_1_0)

#define is_ar9344_11()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK) == ATH_AR9344_1_1)
#define is_ar9342_11()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK) == ATH_AR9342_1_1)
#define is_ar9341_11()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK) == ATH_AR9341_1_1)

#define is_ar934x_10()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK_MINOR) == 0)
#define is_ar934x_11()	((ath_reg_rd(ATH_REV_ID) & ATH_REV_ID_MASK_MINOR) == 1)

#define ATH_WMAC_BASE		ATH_APB_BASE + 0x100000
#define ATH_WMAC_LEN		0x1ffff /* XXX:Check this */

/*
 * Mii block
 */
#define ATH_MII0_CTRL		0x18070000
#define ATH_MII1_CTRL		0x18070004

#define ath_flush_ge(_unit) /* Not required for WASP */

#define ath_flush_pcie() do { \
	ath_reg_wr(ATH_DDR_PCIE_FLUSH, 1); \
	while((ath_reg_rd(ATH_DDR_PCIE_FLUSH) & 0x1)); \
	ath_reg_wr(ATH_DDR_PCIE_FLUSH, 1); \
	while((ath_reg_rd(ATH_DDR_PCIE_FLUSH) & 0x1)); \
} while(0)

#define ath_flush_USB() do { \
	ath_reg_wr(ATH_DDR_USB_FLUSH, 1); \
	while((ath_reg_rd(ATH_DDR_USB_FLUSH) & 0x1)); \
	ath_reg_wr(ATH_DDR_USB_FLUSH, 1); \
	while((ath_reg_rd(ATH_DDR_USB_FLUSH) & 0x1)); \
} while(0)

#undef ath_arch_init_irq
#define ath_arch_init_irq() do {	\
	set_irq_chip_and_handler(	\
		ATH_CPU_IRQ_WLAN,	\
		&dummy_irq_chip,	\
		handle_percpu_irq);	\
} while (0)

// 32'h0000 (CPU_PLL_CONFIG)
#define CPU_PLL_CONFIG_UPDATING_MSB                                  31
#define CPU_PLL_CONFIG_UPDATING_LSB                                  31
#define CPU_PLL_CONFIG_UPDATING_MASK                                 0x80000000
#define CPU_PLL_CONFIG_UPDATING_GET(x)                               (((x) & CPU_PLL_CONFIG_UPDATING_MASK) >> CPU_PLL_CONFIG_UPDATING_LSB)
#define CPU_PLL_CONFIG_UPDATING_SET(x)                               (((x) << CPU_PLL_CONFIG_UPDATING_LSB) & CPU_PLL_CONFIG_UPDATING_MASK)
#define CPU_PLL_CONFIG_UPDATING_RESET                                1
#define CPU_PLL_CONFIG_PLLPWD_MSB                                    30
#define CPU_PLL_CONFIG_PLLPWD_LSB                                    30
#define CPU_PLL_CONFIG_PLLPWD_MASK                                   0x40000000
#define CPU_PLL_CONFIG_PLLPWD_GET(x)                                 (((x) & CPU_PLL_CONFIG_PLLPWD_MASK) >> CPU_PLL_CONFIG_PLLPWD_LSB)
#define CPU_PLL_CONFIG_PLLPWD_SET(x)                                 (((x) << CPU_PLL_CONFIG_PLLPWD_LSB) & CPU_PLL_CONFIG_PLLPWD_MASK)
#define CPU_PLL_CONFIG_PLLPWD_RESET                                  1
#define CPU_PLL_CONFIG_SPARE_MSB                                     29
#define CPU_PLL_CONFIG_SPARE_LSB                                     22
#define CPU_PLL_CONFIG_SPARE_MASK                                    0x3fc00000
#define CPU_PLL_CONFIG_SPARE_GET(x)                                  (((x) & CPU_PLL_CONFIG_SPARE_MASK) >> CPU_PLL_CONFIG_SPARE_LSB)
#define CPU_PLL_CONFIG_SPARE_SET(x)                                  (((x) << CPU_PLL_CONFIG_SPARE_LSB) & CPU_PLL_CONFIG_SPARE_MASK)
#define CPU_PLL_CONFIG_SPARE_RESET                                   0
#define CPU_PLL_CONFIG_OUTDIV_MSB                                    21
#define CPU_PLL_CONFIG_OUTDIV_LSB                                    19
#define CPU_PLL_CONFIG_OUTDIV_MASK                                   0x00380000
#define CPU_PLL_CONFIG_OUTDIV_GET(x)                                 (((x) & CPU_PLL_CONFIG_OUTDIV_MASK) >> CPU_PLL_CONFIG_OUTDIV_LSB)
#define CPU_PLL_CONFIG_OUTDIV_SET(x)                                 (((x) << CPU_PLL_CONFIG_OUTDIV_LSB) & CPU_PLL_CONFIG_OUTDIV_MASK)
#define CPU_PLL_CONFIG_OUTDIV_RESET                                  0
#define CPU_PLL_CONFIG_RANGE_MSB                                     18
#define CPU_PLL_CONFIG_RANGE_LSB                                     17
#define CPU_PLL_CONFIG_RANGE_MASK                                    0x00060000
#define CPU_PLL_CONFIG_RANGE_GET(x)                                  (((x) & CPU_PLL_CONFIG_RANGE_MASK) >> CPU_PLL_CONFIG_RANGE_LSB)
#define CPU_PLL_CONFIG_RANGE_SET(x)                                  (((x) << CPU_PLL_CONFIG_RANGE_LSB) & CPU_PLL_CONFIG_RANGE_MASK)
#define CPU_PLL_CONFIG_RANGE_RESET                                   3
#define CPU_PLL_CONFIG_REFDIV_MSB                                    16
#define CPU_PLL_CONFIG_REFDIV_LSB                                    12
#define CPU_PLL_CONFIG_REFDIV_MASK                                   0x0001f000
#define CPU_PLL_CONFIG_REFDIV_GET(x)                                 (((x) & CPU_PLL_CONFIG_REFDIV_MASK) >> CPU_PLL_CONFIG_REFDIV_LSB)
#define CPU_PLL_CONFIG_REFDIV_SET(x)                                 (((x) << CPU_PLL_CONFIG_REFDIV_LSB) & CPU_PLL_CONFIG_REFDIV_MASK)
#define CPU_PLL_CONFIG_REFDIV_RESET                                  2
#define CPU_PLL_CONFIG_NINT_MSB                                      11
#define CPU_PLL_CONFIG_NINT_LSB                                      6
#define CPU_PLL_CONFIG_NINT_MASK                                     0x00000fc0
#define CPU_PLL_CONFIG_NINT_GET(x)                                   (((x) & CPU_PLL_CONFIG_NINT_MASK) >> CPU_PLL_CONFIG_NINT_LSB)
#define CPU_PLL_CONFIG_NINT_SET(x)                                   (((x) << CPU_PLL_CONFIG_NINT_LSB) & CPU_PLL_CONFIG_NINT_MASK)
#define CPU_PLL_CONFIG_NINT_RESET                                    20
#define CPU_PLL_CONFIG_NFRAC_MSB                                     5
#define CPU_PLL_CONFIG_NFRAC_LSB                                     0
#define CPU_PLL_CONFIG_NFRAC_MASK                                    0x0000003f
#define CPU_PLL_CONFIG_NFRAC_GET(x)                                  (((x) & CPU_PLL_CONFIG_NFRAC_MASK) >> CPU_PLL_CONFIG_NFRAC_LSB)
#define CPU_PLL_CONFIG_NFRAC_SET(x)                                  (((x) << CPU_PLL_CONFIG_NFRAC_LSB) & CPU_PLL_CONFIG_NFRAC_MASK)
#define CPU_PLL_CONFIG_NFRAC_RESET                                   16
#define CPU_PLL_CONFIG_ADDRESS                                       0x0000
#define CPU_PLL_CONFIG_OFFSET                                        0x0000
// SW modifiable bits
#define CPU_PLL_CONFIG_SW_MASK                                       0xffffffff
// bits defined at reset
#define CPU_PLL_CONFIG_RSTMASK                                       0xffffffff
// reset value (ignore bits undefined at reset)
#define CPU_PLL_CONFIG_RESET                                         0xc0062510

// 32'h0004 (DDR_PLL_CONFIG)
#define DDR_PLL_CONFIG_UPDATING_MSB                                  31
#define DDR_PLL_CONFIG_UPDATING_LSB                                  31
#define DDR_PLL_CONFIG_UPDATING_MASK                                 0x80000000
#define DDR_PLL_CONFIG_UPDATING_GET(x)                               (((x) & DDR_PLL_CONFIG_UPDATING_MASK) >> DDR_PLL_CONFIG_UPDATING_LSB)
#define DDR_PLL_CONFIG_UPDATING_SET(x)                               (((x) << DDR_PLL_CONFIG_UPDATING_LSB) & DDR_PLL_CONFIG_UPDATING_MASK)
#define DDR_PLL_CONFIG_UPDATING_RESET                                1
#define DDR_PLL_CONFIG_PLLPWD_MSB                                    30
#define DDR_PLL_CONFIG_PLLPWD_LSB                                    30
#define DDR_PLL_CONFIG_PLLPWD_MASK                                   0x40000000
#define DDR_PLL_CONFIG_PLLPWD_GET(x)                                 (((x) & DDR_PLL_CONFIG_PLLPWD_MASK) >> DDR_PLL_CONFIG_PLLPWD_LSB)
#define DDR_PLL_CONFIG_PLLPWD_SET(x)                                 (((x) << DDR_PLL_CONFIG_PLLPWD_LSB) & DDR_PLL_CONFIG_PLLPWD_MASK)
#define DDR_PLL_CONFIG_PLLPWD_RESET                                  1
#define DDR_PLL_CONFIG_SPARE_MSB                                     29
#define DDR_PLL_CONFIG_SPARE_LSB                                     26
#define DDR_PLL_CONFIG_SPARE_MASK                                    0x3c000000
#define DDR_PLL_CONFIG_SPARE_GET(x)                                  (((x) & DDR_PLL_CONFIG_SPARE_MASK) >> DDR_PLL_CONFIG_SPARE_LSB)
#define DDR_PLL_CONFIG_SPARE_SET(x)                                  (((x) << DDR_PLL_CONFIG_SPARE_LSB) & DDR_PLL_CONFIG_SPARE_MASK)
#define DDR_PLL_CONFIG_SPARE_RESET                                   0
#define DDR_PLL_CONFIG_OUTDIV_MSB                                    25
#define DDR_PLL_CONFIG_OUTDIV_LSB                                    23
#define DDR_PLL_CONFIG_OUTDIV_MASK                                   0x03800000
#define DDR_PLL_CONFIG_OUTDIV_GET(x)                                 (((x) & DDR_PLL_CONFIG_OUTDIV_MASK) >> DDR_PLL_CONFIG_OUTDIV_LSB)
#define DDR_PLL_CONFIG_OUTDIV_SET(x)                                 (((x) << DDR_PLL_CONFIG_OUTDIV_LSB) & DDR_PLL_CONFIG_OUTDIV_MASK)
#define DDR_PLL_CONFIG_OUTDIV_RESET                                  0
#define DDR_PLL_CONFIG_RANGE_MSB                                     22
#define DDR_PLL_CONFIG_RANGE_LSB                                     21
#define DDR_PLL_CONFIG_RANGE_MASK                                    0x00600000
#define DDR_PLL_CONFIG_RANGE_GET(x)                                  (((x) & DDR_PLL_CONFIG_RANGE_MASK) >> DDR_PLL_CONFIG_RANGE_LSB)
#define DDR_PLL_CONFIG_RANGE_SET(x)                                  (((x) << DDR_PLL_CONFIG_RANGE_LSB) & DDR_PLL_CONFIG_RANGE_MASK)
#define DDR_PLL_CONFIG_RANGE_RESET                                   3
#define DDR_PLL_CONFIG_REFDIV_MSB                                    20
#define DDR_PLL_CONFIG_REFDIV_LSB                                    16
#define DDR_PLL_CONFIG_REFDIV_MASK                                   0x001f0000
#define DDR_PLL_CONFIG_REFDIV_GET(x)                                 (((x) & DDR_PLL_CONFIG_REFDIV_MASK) >> DDR_PLL_CONFIG_REFDIV_LSB)
#define DDR_PLL_CONFIG_REFDIV_SET(x)                                 (((x) << DDR_PLL_CONFIG_REFDIV_LSB) & DDR_PLL_CONFIG_REFDIV_MASK)
#define DDR_PLL_CONFIG_REFDIV_RESET                                  2
#define DDR_PLL_CONFIG_NINT_MSB                                      15
#define DDR_PLL_CONFIG_NINT_LSB                                      10
#define DDR_PLL_CONFIG_NINT_MASK                                     0x0000fc00
#define DDR_PLL_CONFIG_NINT_GET(x)                                   (((x) & DDR_PLL_CONFIG_NINT_MASK) >> DDR_PLL_CONFIG_NINT_LSB)
#define DDR_PLL_CONFIG_NINT_SET(x)                                   (((x) << DDR_PLL_CONFIG_NINT_LSB) & DDR_PLL_CONFIG_NINT_MASK)
#define DDR_PLL_CONFIG_NINT_RESET                                    20
#define DDR_PLL_CONFIG_NFRAC_MSB                                     9
#define DDR_PLL_CONFIG_NFRAC_LSB                                     0
#define DDR_PLL_CONFIG_NFRAC_MASK                                    0x000003ff
#define DDR_PLL_CONFIG_NFRAC_GET(x)                                  (((x) & DDR_PLL_CONFIG_NFRAC_MASK) >> DDR_PLL_CONFIG_NFRAC_LSB)
#define DDR_PLL_CONFIG_NFRAC_SET(x)                                  (((x) << DDR_PLL_CONFIG_NFRAC_LSB) & DDR_PLL_CONFIG_NFRAC_MASK)
#define DDR_PLL_CONFIG_NFRAC_RESET                                   512
#define DDR_PLL_CONFIG_ADDRESS                                       0x0004
#define DDR_PLL_CONFIG_OFFSET                                        0x0004
// SW modifiable bits
#define DDR_PLL_CONFIG_SW_MASK                                       0xffffffff
// bits defined at reset
#define DDR_PLL_CONFIG_RSTMASK                                       0xffffffff
// reset value (ignore bits undefined at reset)
#define DDR_PLL_CONFIG_RESET                                         0xc0625200

// 32'h0008 (CPU_DDR_CLOCK_CONTROL)
#define CPU_DDR_CLOCK_CONTROL_SPARE_MSB                              31
#define CPU_DDR_CLOCK_CONTROL_SPARE_LSB                              25
#define CPU_DDR_CLOCK_CONTROL_SPARE_MASK                             0xfe000000
#define CPU_DDR_CLOCK_CONTROL_SPARE_GET(x)                           (((x) & CPU_DDR_CLOCK_CONTROL_SPARE_MASK) >> CPU_DDR_CLOCK_CONTROL_SPARE_LSB)
#define CPU_DDR_CLOCK_CONTROL_SPARE_SET(x)                           (((x) << CPU_DDR_CLOCK_CONTROL_SPARE_LSB) & CPU_DDR_CLOCK_CONTROL_SPARE_MASK)
#define CPU_DDR_CLOCK_CONTROL_SPARE_RESET                            0
#define CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_MSB                 24
#define CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_LSB                 24
#define CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_MASK                0x01000000
#define CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_GET(x)              (((x) & CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_MASK) >> CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_LSB)
#define CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_SET(x)              (((x) << CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_LSB) & CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_MASK)
#define CPU_DDR_CLOCK_CONTROL_AHBCLK_FROM_DDRPLL_RESET               1
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_MSB            23
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_LSB            23
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_MASK           0x00800000
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_GET(x)         (((x) & CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_MASK) >> CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_LSB)
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_SET(x)         (((x) << CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_LSB) & CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_MASK)
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_DEASSRT_RESET          0
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_MSB               22
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_LSB               22
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_MASK              0x00400000
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_GET(x)            (((x) & CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_MASK) >> CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_LSB)
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_SET(x)            (((x) << CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_LSB) & CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_MASK)
#define CPU_DDR_CLOCK_CONTROL_CPU_RESET_EN_BP_ASRT_RESET             0
#define CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_MSB                 21
#define CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_LSB                 21
#define CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_MASK                0x00200000
#define CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_GET(x)              (((x) & CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_MASK) >> CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_LSB)
#define CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_SET(x)              (((x) << CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_LSB) & CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_MASK)
#define CPU_DDR_CLOCK_CONTROL_DDRCLK_FROM_DDRPLL_RESET               1
#define CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_MSB                 20
#define CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_LSB                 20
#define CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_MASK                0x00100000
#define CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_GET(x)              (((x) & CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_MASK) >> CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_LSB)
#define CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_SET(x)              (((x) << CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_LSB) & CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_MASK)
#define CPU_DDR_CLOCK_CONTROL_CPUCLK_FROM_CPUPLL_RESET               1
#define CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_MSB                       19
#define CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_LSB                       15
#define CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_MASK                      0x000f8000
#define CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_GET(x)                    (((x) & CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_MASK) >> CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_LSB)
#define CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_SET(x)                    (((x) << CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_LSB) & CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_MASK)
#define CPU_DDR_CLOCK_CONTROL_AHB_POST_DIV_RESET                     0
#define CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_MSB                       14
#define CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_LSB                       10
#define CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_MASK                      0x00007c00
#define CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_GET(x)                    (((x) & CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_MASK) >> CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_LSB)
#define CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_SET(x)                    (((x) << CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_LSB) & CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_MASK)
#define CPU_DDR_CLOCK_CONTROL_DDR_POST_DIV_RESET                     0
#define CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_MSB                       9
#define CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_LSB                       5
#define CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_MASK                      0x000003e0
#define CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_GET(x)                    (((x) & CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_MASK) >> CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_LSB)
#define CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_SET(x)                    (((x) << CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_LSB) & CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_MASK)
#define CPU_DDR_CLOCK_CONTROL_CPU_POST_DIV_RESET                     0
#define CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_MSB                     4
#define CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_LSB                     4
#define CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_MASK                    0x00000010
#define CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_GET(x)                  (((x) & CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_MASK) >> CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_LSB)
#define CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_SET(x)                  (((x) << CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_LSB) & CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_MASK)
#define CPU_DDR_CLOCK_CONTROL_AHB_PLL_BYPASS_RESET                   1
#define CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_MSB                     3
#define CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_LSB                     3
#define CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_MASK                    0x00000008
#define CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_GET(x)                  (((x) & CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_MASK) >> CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_LSB)
#define CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_SET(x)                  (((x) << CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_LSB) & CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_MASK)
#define CPU_DDR_CLOCK_CONTROL_DDR_PLL_BYPASS_RESET                   1
#define CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_MSB                     2
#define CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_LSB                     2
#define CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_MASK                    0x00000004
#define CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_GET(x)                  (((x) & CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_MASK) >> CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_LSB)
#define CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_SET(x)                  (((x) << CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_LSB) & CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_MASK)
#define CPU_DDR_CLOCK_CONTROL_CPU_PLL_BYPASS_RESET                   1
#define CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_MSB                       1
#define CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_LSB                       1
#define CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_MASK                      0x00000002
#define CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_GET(x)                    (((x) & CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_MASK) >> CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_LSB)
#define CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_SET(x)                    (((x) << CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_LSB) & CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_MASK)
#define CPU_DDR_CLOCK_CONTROL_RESET_SWITCH_RESET                     0
#define CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_MSB                       0
#define CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_LSB                       0
#define CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_MASK                      0x00000001
#define CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_GET(x)                    (((x) & CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_MASK) >> CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_LSB)
#define CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_SET(x)                    (((x) << CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_LSB) & CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_MASK)
#define CPU_DDR_CLOCK_CONTROL_CLOCK_SWITCH_RESET                     0
#define CPU_DDR_CLOCK_CONTROL_ADDRESS                                0x0008
#define CPU_DDR_CLOCK_CONTROL_OFFSET                                 0x0008
// SW modifiable bits
#define CPU_DDR_CLOCK_CONTROL_SW_MASK                                0xffffffff
// bits defined at reset
#define CPU_DDR_CLOCK_CONTROL_RSTMASK                                0xffffffff
// reset value (ignore bits undefined at reset)
#define CPU_DDR_CLOCK_CONTROL_RESET                                  0x0130001c

// 32'h000c (CPU_SYNC)
#define CPU_SYNC_LENGTH_MSB                                          19
#define CPU_SYNC_LENGTH_LSB                                          16
#define CPU_SYNC_LENGTH_MASK                                         0x000f0000
#define CPU_SYNC_LENGTH_GET(x)                                       (((x) & CPU_SYNC_LENGTH_MASK) >> CPU_SYNC_LENGTH_LSB)
#define CPU_SYNC_LENGTH_SET(x)                                       (((x) << CPU_SYNC_LENGTH_LSB) & CPU_SYNC_LENGTH_MASK)
#define CPU_SYNC_LENGTH_RESET                                        0
#define CPU_SYNC_PATTERN_MSB                                         15
#define CPU_SYNC_PATTERN_LSB                                         0
#define CPU_SYNC_PATTERN_MASK                                        0x0000ffff
#define CPU_SYNC_PATTERN_GET(x)                                      (((x) & CPU_SYNC_PATTERN_MASK) >> CPU_SYNC_PATTERN_LSB)
#define CPU_SYNC_PATTERN_SET(x)                                      (((x) << CPU_SYNC_PATTERN_LSB) & CPU_SYNC_PATTERN_MASK)
#define CPU_SYNC_PATTERN_RESET                                       65535
#define CPU_SYNC_ADDRESS                                             0x000c
#define CPU_SYNC_OFFSET                                              0x000c
// SW modifiable bits
#define CPU_SYNC_SW_MASK                                             0x000fffff
// bits defined at reset
#define CPU_SYNC_RSTMASK                                             0xffffffff
// reset value (ignore bits undefined at reset)
#define CPU_SYNC_RESET                                               0x0000ffff

// 32'h0010 (PCIE_PLL_CONFIG)
#define PCIE_PLL_CONFIG_UPDATING_MSB                                 31
#define PCIE_PLL_CONFIG_UPDATING_LSB                                 31
#define PCIE_PLL_CONFIG_UPDATING_MASK                                0x80000000
#define PCIE_PLL_CONFIG_UPDATING_GET(x)                              (((x) & PCIE_PLL_CONFIG_UPDATING_MASK) >> PCIE_PLL_CONFIG_UPDATING_LSB)
#define PCIE_PLL_CONFIG_UPDATING_SET(x)                              (((x) << PCIE_PLL_CONFIG_UPDATING_LSB) & PCIE_PLL_CONFIG_UPDATING_MASK)
#define PCIE_PLL_CONFIG_UPDATING_RESET                               0
#define PCIE_PLL_CONFIG_PLLPWD_MSB                                   30
#define PCIE_PLL_CONFIG_PLLPWD_LSB                                   30
#define PCIE_PLL_CONFIG_PLLPWD_MASK                                  0x40000000
#define PCIE_PLL_CONFIG_PLLPWD_GET(x)                                (((x) & PCIE_PLL_CONFIG_PLLPWD_MASK) >> PCIE_PLL_CONFIG_PLLPWD_LSB)
#define PCIE_PLL_CONFIG_PLLPWD_SET(x)                                (((x) << PCIE_PLL_CONFIG_PLLPWD_LSB) & PCIE_PLL_CONFIG_PLLPWD_MASK)
#define PCIE_PLL_CONFIG_PLLPWD_RESET                                 1
#define PCIE_PLL_CONFIG_BYPASS_MSB                                   16
#define PCIE_PLL_CONFIG_BYPASS_LSB                                   16
#define PCIE_PLL_CONFIG_BYPASS_MASK                                  0x00010000
#define PCIE_PLL_CONFIG_BYPASS_GET(x)                                (((x) & PCIE_PLL_CONFIG_BYPASS_MASK) >> PCIE_PLL_CONFIG_BYPASS_LSB)
#define PCIE_PLL_CONFIG_BYPASS_SET(x)                                (((x) << PCIE_PLL_CONFIG_BYPASS_LSB) & PCIE_PLL_CONFIG_BYPASS_MASK)
#define PCIE_PLL_CONFIG_BYPASS_RESET                                 1
#define PCIE_PLL_CONFIG_REFDIV_MSB                                   14
#define PCIE_PLL_CONFIG_REFDIV_LSB                                   10
#define PCIE_PLL_CONFIG_REFDIV_MASK                                  0x00007c00
#define PCIE_PLL_CONFIG_REFDIV_GET(x)                                (((x) & PCIE_PLL_CONFIG_REFDIV_MASK) >> PCIE_PLL_CONFIG_REFDIV_LSB)
#define PCIE_PLL_CONFIG_REFDIV_SET(x)                                (((x) << PCIE_PLL_CONFIG_REFDIV_LSB) & PCIE_PLL_CONFIG_REFDIV_MASK)
#define PCIE_PLL_CONFIG_REFDIV_RESET                                 1
#define PCIE_PLL_CONFIG_ADDRESS                                      0x0010
#define PCIE_PLL_CONFIG_OFFSET                                       0x0010
// SW modifiable bits
#define PCIE_PLL_CONFIG_SW_MASK                                      0xc0017c00
// bits defined at reset
#define PCIE_PLL_CONFIG_RSTMASK                                      0xffffffff
// reset value (ignore bits undefined at reset)
#define PCIE_PLL_CONFIG_RESET                                        0x40010400

// 32'h0014 (PCIE_PLL_DITHER_DIV_MAX)
#define PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_MSB                        31
#define PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_LSB                        31
#define PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_MASK                       0x80000000
#define PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_GET(x)                     (((x) & PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_MASK) >> PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_LSB)
#define PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_SET(x)                     (((x) << PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_LSB) & PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_MASK)
#define PCIE_PLL_DITHER_DIV_MAX_EN_DITHER_RESET                      1
#define PCIE_PLL_DITHER_DIV_MAX_USE_MAX_MSB                          30
#define PCIE_PLL_DITHER_DIV_MAX_USE_MAX_LSB                          30
#define PCIE_PLL_DITHER_DIV_MAX_USE_MAX_MASK                         0x40000000
#define PCIE_PLL_DITHER_DIV_MAX_USE_MAX_GET(x)                       (((x) & PCIE_PLL_DITHER_DIV_MAX_USE_MAX_MASK) >> PCIE_PLL_DITHER_DIV_MAX_USE_MAX_LSB)
#define PCIE_PLL_DITHER_DIV_MAX_USE_MAX_SET(x)                       (((x) << PCIE_PLL_DITHER_DIV_MAX_USE_MAX_LSB) & PCIE_PLL_DITHER_DIV_MAX_USE_MAX_MASK)
#define PCIE_PLL_DITHER_DIV_MAX_USE_MAX_RESET                        1
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_MSB                      20
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_LSB                      15
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_MASK                     0x001f8000
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_GET(x)                   (((x) & PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_MASK) >> PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_LSB)
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_SET(x)                   (((x) << PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_LSB) & PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_MASK)
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_INT_RESET                    19
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_MSB                     14
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_LSB                     1
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_MASK                    0x00007ffe
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_GET(x)                  (((x) & PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_MASK) >> PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_LSB)
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_SET(x)                  (((x) << PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_LSB) & PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_MASK)
#define PCIE_PLL_DITHER_DIV_MAX_DIV_MAX_FRAC_RESET                   16383
#define PCIE_PLL_DITHER_DIV_MAX_ADDRESS                              0x0014
#define PCIE_PLL_DITHER_DIV_MAX_OFFSET                               0x0014
// SW modifiable bits
#define PCIE_PLL_DITHER_DIV_MAX_SW_MASK                              0xc01ffffe
// bits defined at reset
#define PCIE_PLL_DITHER_DIV_MAX_RSTMASK                              0xffffffff
// reset value (ignore bits undefined at reset)
#define PCIE_PLL_DITHER_DIV_MAX_RESET                                0xc009fffe

// 32'h0018 (PCIE_PLL_DITHER_DIV_MIN)
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_MSB                      20
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_LSB                      15
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_MASK                     0x001f8000
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_GET(x)                   (((x) & PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_MASK) >> PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_LSB)
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_SET(x)                   (((x) << PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_LSB) & PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_MASK)
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_INT_RESET                    19
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_MSB                     14
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_LSB                     1
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_MASK                    0x00007ffe
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_GET(x)                  (((x) & PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_MASK) >> PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_LSB)
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_SET(x)                  (((x) << PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_LSB) & PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_MASK)
#define PCIE_PLL_DITHER_DIV_MIN_DIV_MIN_FRAC_RESET                   14749
#define PCIE_PLL_DITHER_DIV_MIN_ADDRESS                              0x0018
#define PCIE_PLL_DITHER_DIV_MIN_OFFSET                               0x0018
// SW modifiable bits
#define PCIE_PLL_DITHER_DIV_MIN_SW_MASK                              0x001ffffe
// bits defined at reset
#define PCIE_PLL_DITHER_DIV_MIN_RSTMASK                              0xffffffff
// reset value (ignore bits undefined at reset)
#define PCIE_PLL_DITHER_DIV_MIN_RESET                                0x0009f33a

// 32'h001c (PCIE_PLL_DITHER_STEP)
#define PCIE_PLL_DITHER_STEP_UPDATE_CNT_MSB                          31
#define PCIE_PLL_DITHER_STEP_UPDATE_CNT_LSB                          28
#define PCIE_PLL_DITHER_STEP_UPDATE_CNT_MASK                         0xf0000000
#define PCIE_PLL_DITHER_STEP_UPDATE_CNT_GET(x)                       (((x) & PCIE_PLL_DITHER_STEP_UPDATE_CNT_MASK) >> PCIE_PLL_DITHER_STEP_UPDATE_CNT_LSB)
#define PCIE_PLL_DITHER_STEP_UPDATE_CNT_SET(x)                       (((x) << PCIE_PLL_DITHER_STEP_UPDATE_CNT_LSB) & PCIE_PLL_DITHER_STEP_UPDATE_CNT_MASK)
#define PCIE_PLL_DITHER_STEP_UPDATE_CNT_RESET                        0
#define PCIE_PLL_DITHER_STEP_STEP_INT_MSB                            24
#define PCIE_PLL_DITHER_STEP_STEP_INT_LSB                            15
#define PCIE_PLL_DITHER_STEP_STEP_INT_MASK                           0x01ff8000
#define PCIE_PLL_DITHER_STEP_STEP_INT_GET(x)                         (((x) & PCIE_PLL_DITHER_STEP_STEP_INT_MASK) >> PCIE_PLL_DITHER_STEP_STEP_INT_LSB)
#define PCIE_PLL_DITHER_STEP_STEP_INT_SET(x)                         (((x) << PCIE_PLL_DITHER_STEP_STEP_INT_LSB) & PCIE_PLL_DITHER_STEP_STEP_INT_MASK)
#define PCIE_PLL_DITHER_STEP_STEP_INT_RESET                          0
#define PCIE_PLL_DITHER_STEP_STEP_FRAC_MSB                           14
#define PCIE_PLL_DITHER_STEP_STEP_FRAC_LSB                           1
#define PCIE_PLL_DITHER_STEP_STEP_FRAC_MASK                          0x00007ffe
#define PCIE_PLL_DITHER_STEP_STEP_FRAC_GET(x)                        (((x) & PCIE_PLL_DITHER_STEP_STEP_FRAC_MASK) >> PCIE_PLL_DITHER_STEP_STEP_FRAC_LSB)
#define PCIE_PLL_DITHER_STEP_STEP_FRAC_SET(x)                        (((x) << PCIE_PLL_DITHER_STEP_STEP_FRAC_LSB) & PCIE_PLL_DITHER_STEP_STEP_FRAC_MASK)
#define PCIE_PLL_DITHER_STEP_STEP_FRAC_RESET                         10
#define PCIE_PLL_DITHER_STEP_ADDRESS                                 0x001c
#define PCIE_PLL_DITHER_STEP_OFFSET                                  0x001c
// SW modifiable bits
#define PCIE_PLL_DITHER_STEP_SW_MASK                                 0xf1fffffe
// bits defined at reset
#define PCIE_PLL_DITHER_STEP_RSTMASK                                 0xffffffff
// reset value (ignore bits undefined at reset)
#define PCIE_PLL_DITHER_STEP_RESET                                   0x00000014

// 32'h0020 (LDO_POWER_CONTROL)
#define LDO_POWER_CONTROL_PKG_SEL_MSB                                5
#define LDO_POWER_CONTROL_PKG_SEL_LSB                                5
#define LDO_POWER_CONTROL_PKG_SEL_MASK                               0x00000020
#define LDO_POWER_CONTROL_PKG_SEL_GET(x)                             (((x) & LDO_POWER_CONTROL_PKG_SEL_MASK) >> LDO_POWER_CONTROL_PKG_SEL_LSB)
#define LDO_POWER_CONTROL_PKG_SEL_SET(x)                             (((x) << LDO_POWER_CONTROL_PKG_SEL_LSB) & LDO_POWER_CONTROL_PKG_SEL_MASK)
#define LDO_POWER_CONTROL_PKG_SEL_RESET                              0
#define LDO_POWER_CONTROL_PWDLDO_CPU_MSB                             4
#define LDO_POWER_CONTROL_PWDLDO_CPU_LSB                             4
#define LDO_POWER_CONTROL_PWDLDO_CPU_MASK                            0x00000010
#define LDO_POWER_CONTROL_PWDLDO_CPU_GET(x)                          (((x) & LDO_POWER_CONTROL_PWDLDO_CPU_MASK) >> LDO_POWER_CONTROL_PWDLDO_CPU_LSB)
#define LDO_POWER_CONTROL_PWDLDO_CPU_SET(x)                          (((x) << LDO_POWER_CONTROL_PWDLDO_CPU_LSB) & LDO_POWER_CONTROL_PWDLDO_CPU_MASK)
#define LDO_POWER_CONTROL_PWDLDO_CPU_RESET                           0
#define LDO_POWER_CONTROL_PWDLDO_DDR_MSB                             3
#define LDO_POWER_CONTROL_PWDLDO_DDR_LSB                             3
#define LDO_POWER_CONTROL_PWDLDO_DDR_MASK                            0x00000008
#define LDO_POWER_CONTROL_PWDLDO_DDR_GET(x)                          (((x) & LDO_POWER_CONTROL_PWDLDO_DDR_MASK) >> LDO_POWER_CONTROL_PWDLDO_DDR_LSB)
#define LDO_POWER_CONTROL_PWDLDO_DDR_SET(x)                          (((x) << LDO_POWER_CONTROL_PWDLDO_DDR_LSB) & LDO_POWER_CONTROL_PWDLDO_DDR_MASK)
#define LDO_POWER_CONTROL_PWDLDO_DDR_RESET                           0
#define LDO_POWER_CONTROL_CPU_REFSEL_MSB                             2
#define LDO_POWER_CONTROL_CPU_REFSEL_LSB                             1
#define LDO_POWER_CONTROL_CPU_REFSEL_MASK                            0x00000006
#define LDO_POWER_CONTROL_CPU_REFSEL_GET(x)                          (((x) & LDO_POWER_CONTROL_CPU_REFSEL_MASK) >> LDO_POWER_CONTROL_CPU_REFSEL_LSB)
#define LDO_POWER_CONTROL_CPU_REFSEL_SET(x)                          (((x) << LDO_POWER_CONTROL_CPU_REFSEL_LSB) & LDO_POWER_CONTROL_CPU_REFSEL_MASK)
#define LDO_POWER_CONTROL_CPU_REFSEL_RESET                           3
#define LDO_POWER_CONTROL_SELECT_DDR1_MSB                            0
#define LDO_POWER_CONTROL_SELECT_DDR1_LSB                            0
#define LDO_POWER_CONTROL_SELECT_DDR1_MASK                           0x00000001
#define LDO_POWER_CONTROL_SELECT_DDR1_GET(x)                         (((x) & LDO_POWER_CONTROL_SELECT_DDR1_MASK) >> LDO_POWER_CONTROL_SELECT_DDR1_LSB)
#define LDO_POWER_CONTROL_SELECT_DDR1_SET(x)                         (((x) << LDO_POWER_CONTROL_SELECT_DDR1_LSB) & LDO_POWER_CONTROL_SELECT_DDR1_MASK)
#define LDO_POWER_CONTROL_SELECT_DDR1_RESET                          0
#define LDO_POWER_CONTROL_ADDRESS                                    0x0020
#define LDO_POWER_CONTROL_OFFSET                                     0x0020
// SW modifiable bits
#define LDO_POWER_CONTROL_SW_MASK                                    0x0000003f
// bits defined at reset
#define LDO_POWER_CONTROL_RSTMASK                                    0xffffffff
// reset value (ignore bits undefined at reset)
#define LDO_POWER_CONTROL_RESET                                      0x00000006

// 32'h0024 (SWITCH_CLOCK_SPARE)
#define SWITCH_CLOCK_SPARE_SPARE_MSB                                 31
#define SWITCH_CLOCK_SPARE_SPARE_LSB                                 12
#define SWITCH_CLOCK_SPARE_SPARE_MASK                                0xfffff000
#define SWITCH_CLOCK_SPARE_SPARE_GET(x)                              (((x) & SWITCH_CLOCK_SPARE_SPARE_MASK) >> SWITCH_CLOCK_SPARE_SPARE_LSB)
#define SWITCH_CLOCK_SPARE_SPARE_SET(x)                              (((x) << SWITCH_CLOCK_SPARE_SPARE_LSB) & SWITCH_CLOCK_SPARE_SPARE_MASK)
#define SWITCH_CLOCK_SPARE_SPARE_RESET                               0
#define SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_MSB                   11
#define SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_LSB                   8
#define SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_MASK                  0x00000f00
#define SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_GET(x)                (((x) & SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_MASK) >> SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_LSB)
#define SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_SET(x)                (((x) << SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_LSB) & SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_MASK)
#define SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_RESET                 5
#define SWITCH_CLOCK_SPARE_UART1_CLK_SEL_MSB                         7
#define SWITCH_CLOCK_SPARE_UART1_CLK_SEL_LSB                         7
#define SWITCH_CLOCK_SPARE_UART1_CLK_SEL_MASK                        0x00000080
#define SWITCH_CLOCK_SPARE_UART1_CLK_SEL_GET(x)                      (((x) & SWITCH_CLOCK_SPARE_UART1_CLK_SEL_MASK) >> SWITCH_CLOCK_SPARE_UART1_CLK_SEL_LSB)
#define SWITCH_CLOCK_SPARE_UART1_CLK_SEL_SET(x)                      (((x) << SWITCH_CLOCK_SPARE_UART1_CLK_SEL_LSB) & SWITCH_CLOCK_SPARE_UART1_CLK_SEL_MASK)
#define SWITCH_CLOCK_SPARE_UART1_CLK_SEL_RESET                       0
#define SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_MSB                          6
#define SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_LSB                          6
#define SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_MASK                         0x00000040
#define SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_GET(x)                       (((x) & SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_MASK) >> SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_LSB)
#define SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_SET(x)                       (((x) << SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_LSB) & SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_MASK)
#define SWITCH_CLOCK_SPARE_MDIO_CLK_SEL_RESET                        0
#define SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_MSB                       5
#define SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_LSB                       5
#define SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_MASK                      0x00000020
#define SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_GET(x)                    (((x) & SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_MASK) >> SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_LSB)
#define SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_SET(x)                    (((x) << SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_LSB) & SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_MASK)
#define SWITCH_CLOCK_SPARE_OEN_CLK125M_PLL_RESET                     1
#define SWITCH_CLOCK_SPARE_EN_PLL_TOP_MSB                            4
#define SWITCH_CLOCK_SPARE_EN_PLL_TOP_LSB                            4
#define SWITCH_CLOCK_SPARE_EN_PLL_TOP_MASK                           0x00000010
#define SWITCH_CLOCK_SPARE_EN_PLL_TOP_GET(x)                         (((x) & SWITCH_CLOCK_SPARE_EN_PLL_TOP_MASK) >> SWITCH_CLOCK_SPARE_EN_PLL_TOP_LSB)
#define SWITCH_CLOCK_SPARE_EN_PLL_TOP_SET(x)                         (((x) << SWITCH_CLOCK_SPARE_EN_PLL_TOP_LSB) & SWITCH_CLOCK_SPARE_EN_PLL_TOP_MASK)
#define SWITCH_CLOCK_SPARE_EN_PLL_TOP_RESET                          1
#define SWITCH_CLOCK_SPARE_EEE_ENABLE_MSB                            3
#define SWITCH_CLOCK_SPARE_EEE_ENABLE_LSB                            3
#define SWITCH_CLOCK_SPARE_EEE_ENABLE_MASK                           0x00000008
#define SWITCH_CLOCK_SPARE_EEE_ENABLE_GET(x)                         (((x) & SWITCH_CLOCK_SPARE_EEE_ENABLE_MASK) >> SWITCH_CLOCK_SPARE_EEE_ENABLE_LSB)
#define SWITCH_CLOCK_SPARE_EEE_ENABLE_SET(x)                         (((x) << SWITCH_CLOCK_SPARE_EEE_ENABLE_LSB) & SWITCH_CLOCK_SPARE_EEE_ENABLE_MASK)
#define SWITCH_CLOCK_SPARE_EEE_ENABLE_RESET                          0
#define SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_MSB             2
#define SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_LSB             2
#define SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_MASK            0x00000004
#define SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_GET(x)          (((x) & SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_MASK) >> SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_LSB)
#define SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_SET(x)          (((x) << SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_LSB) & SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_MASK)
#define SWITCH_CLOCK_SPARE_SWITCHCLK_FROM_PYTHON_OFF_RESET           0
#define SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_MSB                  1
#define SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_LSB                  1
#define SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_MASK                 0x00000002
#define SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_GET(x)               (((x) & SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_MASK) >> SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_LSB)
#define SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_SET(x)               (((x) << SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_LSB) & SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_MASK)
#define SWITCH_CLOCK_SPARE_SWITCH_FUNC_TST_MODE_RESET                0
#define SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_MSB                         0
#define SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_LSB                         0
#define SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_MASK                        0x00000001
#define SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_GET(x)                      (((x) & SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_MASK) >> SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_LSB)
#define SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_SET(x)                      (((x) << SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_LSB) & SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_MASK)
#define SWITCH_CLOCK_SPARE_SWITCHCLK_SEL_RESET                       1
#define SWITCH_CLOCK_SPARE_ADDRESS                                   0x18050024
#define SWITCH_CLOCK_SPARE_OFFSET                                    0x0024
// SW modifiable bits
#define SWITCH_CLOCK_SPARE_SW_MASK                                   0xffffffff
// bits defined at reset
#define SWITCH_CLOCK_SPARE_RSTMASK                                   0xffffffff
// reset value (ignore bits undefined at reset)
#define SWITCH_CLOCK_SPARE_RESET                                     0x00000531

// 32'h0028 (CURRENT_PCIE_PLL_DITHER)
#define CURRENT_PCIE_PLL_DITHER_INT_MSB                              20
#define CURRENT_PCIE_PLL_DITHER_INT_LSB                              15
#define CURRENT_PCIE_PLL_DITHER_INT_MASK                             0x001f8000
#define CURRENT_PCIE_PLL_DITHER_INT_GET(x)                           (((x) & CURRENT_PCIE_PLL_DITHER_INT_MASK) >> CURRENT_PCIE_PLL_DITHER_INT_LSB)
#define CURRENT_PCIE_PLL_DITHER_INT_SET(x)                           (((x) << CURRENT_PCIE_PLL_DITHER_INT_LSB) & CURRENT_PCIE_PLL_DITHER_INT_MASK)
#define CURRENT_PCIE_PLL_DITHER_INT_RESET                            1
#define CURRENT_PCIE_PLL_DITHER_FRAC_MSB                             13
#define CURRENT_PCIE_PLL_DITHER_FRAC_LSB                             0
#define CURRENT_PCIE_PLL_DITHER_FRAC_MASK                            0x00003fff
#define CURRENT_PCIE_PLL_DITHER_FRAC_GET(x)                          (((x) & CURRENT_PCIE_PLL_DITHER_FRAC_MASK) >> CURRENT_PCIE_PLL_DITHER_FRAC_LSB)
#define CURRENT_PCIE_PLL_DITHER_FRAC_SET(x)                          (((x) << CURRENT_PCIE_PLL_DITHER_FRAC_LSB) & CURRENT_PCIE_PLL_DITHER_FRAC_MASK)
#define CURRENT_PCIE_PLL_DITHER_FRAC_RESET                           0
#define CURRENT_PCIE_PLL_DITHER_ADDRESS                              0x0028
#define CURRENT_PCIE_PLL_DITHER_OFFSET                               0x0028
// SW modifiable bits
#define CURRENT_PCIE_PLL_DITHER_SW_MASK                              0x001fbfff
// bits defined at reset
#define CURRENT_PCIE_PLL_DITHER_RSTMASK                              0xffffffff
// reset value (ignore bits undefined at reset)
#define CURRENT_PCIE_PLL_DITHER_RESET                                0x00008000

// 32'h002c (ETH_XMII)
#define ETH_XMII_TX_INVERT_MSB                                       31
#define ETH_XMII_TX_INVERT_LSB                                       31
#define ETH_XMII_TX_INVERT_MASK                                      0x80000000
#define ETH_XMII_TX_INVERT_GET(x)                                    (((x) & ETH_XMII_TX_INVERT_MASK) >> ETH_XMII_TX_INVERT_LSB)
#define ETH_XMII_TX_INVERT_SET(x)                                    (((x) << ETH_XMII_TX_INVERT_LSB) & ETH_XMII_TX_INVERT_MASK)
#define ETH_XMII_TX_INVERT_RESET                                     0
#define ETH_XMII_GIGE_QUAD_MSB                                       30
#define ETH_XMII_GIGE_QUAD_LSB                                       30
#define ETH_XMII_GIGE_QUAD_MASK                                      0x40000000
#define ETH_XMII_GIGE_QUAD_GET(x)                                    (((x) & ETH_XMII_GIGE_QUAD_MASK) >> ETH_XMII_GIGE_QUAD_LSB)
#define ETH_XMII_GIGE_QUAD_SET(x)                                    (((x) << ETH_XMII_GIGE_QUAD_LSB) & ETH_XMII_GIGE_QUAD_MASK)
#define ETH_XMII_GIGE_QUAD_RESET                                     0
#define ETH_XMII_RX_DELAY_MSB                                        29
#define ETH_XMII_RX_DELAY_LSB                                        28
#define ETH_XMII_RX_DELAY_MASK                                       0x30000000
#define ETH_XMII_RX_DELAY_GET(x)                                     (((x) & ETH_XMII_RX_DELAY_MASK) >> ETH_XMII_RX_DELAY_LSB)
#define ETH_XMII_RX_DELAY_SET(x)                                     (((x) << ETH_XMII_RX_DELAY_LSB) & ETH_XMII_RX_DELAY_MASK)
#define ETH_XMII_RX_DELAY_RESET                                      0
#define ETH_XMII_TX_DELAY_MSB                                        27
#define ETH_XMII_TX_DELAY_LSB                                        26
#define ETH_XMII_TX_DELAY_MASK                                       0x0c000000
#define ETH_XMII_TX_DELAY_GET(x)                                     (((x) & ETH_XMII_TX_DELAY_MASK) >> ETH_XMII_TX_DELAY_LSB)
#define ETH_XMII_TX_DELAY_SET(x)                                     (((x) << ETH_XMII_TX_DELAY_LSB) & ETH_XMII_TX_DELAY_MASK)
#define ETH_XMII_TX_DELAY_RESET                                      0
#define ETH_XMII_GIGE_MSB                                            25
#define ETH_XMII_GIGE_LSB                                            25
#define ETH_XMII_GIGE_MASK                                           0x02000000
#define ETH_XMII_GIGE_GET(x)                                         (((x) & ETH_XMII_GIGE_MASK) >> ETH_XMII_GIGE_LSB)
#define ETH_XMII_GIGE_SET(x)                                         (((x) << ETH_XMII_GIGE_LSB) & ETH_XMII_GIGE_MASK)
#define ETH_XMII_GIGE_RESET                                          0
#define ETH_XMII_OFFSET_PHASE_MSB                                    24
#define ETH_XMII_OFFSET_PHASE_LSB                                    24
#define ETH_XMII_OFFSET_PHASE_MASK                                   0x01000000
#define ETH_XMII_OFFSET_PHASE_GET(x)                                 (((x) & ETH_XMII_OFFSET_PHASE_MASK) >> ETH_XMII_OFFSET_PHASE_LSB)
#define ETH_XMII_OFFSET_PHASE_SET(x)                                 (((x) << ETH_XMII_OFFSET_PHASE_LSB) & ETH_XMII_OFFSET_PHASE_MASK)
#define ETH_XMII_OFFSET_PHASE_RESET                                  0
#define ETH_XMII_OFFSET_COUNT_MSB                                    23
#define ETH_XMII_OFFSET_COUNT_LSB                                    16
#define ETH_XMII_OFFSET_COUNT_MASK                                   0x00ff0000
#define ETH_XMII_OFFSET_COUNT_GET(x)                                 (((x) & ETH_XMII_OFFSET_COUNT_MASK) >> ETH_XMII_OFFSET_COUNT_LSB)
#define ETH_XMII_OFFSET_COUNT_SET(x)                                 (((x) << ETH_XMII_OFFSET_COUNT_LSB) & ETH_XMII_OFFSET_COUNT_MASK)
#define ETH_XMII_OFFSET_COUNT_RESET                                  0
#define ETH_XMII_PHASE1_COUNT_MSB                                    15
#define ETH_XMII_PHASE1_COUNT_LSB                                    8
#define ETH_XMII_PHASE1_COUNT_MASK                                   0x0000ff00
#define ETH_XMII_PHASE1_COUNT_GET(x)                                 (((x) & ETH_XMII_PHASE1_COUNT_MASK) >> ETH_XMII_PHASE1_COUNT_LSB)
#define ETH_XMII_PHASE1_COUNT_SET(x)                                 (((x) << ETH_XMII_PHASE1_COUNT_LSB) & ETH_XMII_PHASE1_COUNT_MASK)
#define ETH_XMII_PHASE1_COUNT_RESET                                  1
#define ETH_XMII_PHASE0_COUNT_MSB                                    7
#define ETH_XMII_PHASE0_COUNT_LSB                                    0
#define ETH_XMII_PHASE0_COUNT_MASK                                   0x000000ff
#define ETH_XMII_PHASE0_COUNT_GET(x)                                 (((x) & ETH_XMII_PHASE0_COUNT_MASK) >> ETH_XMII_PHASE0_COUNT_LSB)
#define ETH_XMII_PHASE0_COUNT_SET(x)                                 (((x) << ETH_XMII_PHASE0_COUNT_LSB) & ETH_XMII_PHASE0_COUNT_MASK)
#define ETH_XMII_PHASE0_COUNT_RESET                                  1
#define ETH_XMII_ADDRESS                                             0x002c
#define ETH_XMII_OFFSET                                              0x002c
// SW modifiable bits
#define ETH_XMII_SW_MASK                                             0xffffffff
// bits defined at reset
#define ETH_XMII_RSTMASK                                             0xffffffff
// reset value (ignore bits undefined at reset)
#define ETH_XMII_RESET                                               0x00000101

// 32'h0030 (AUDIO_PLL_CONFIG)
#define AUDIO_PLL_CONFIG_UPDATING_MSB                                31
#define AUDIO_PLL_CONFIG_UPDATING_LSB                                31
#define AUDIO_PLL_CONFIG_UPDATING_MASK                               0x80000000
#define AUDIO_PLL_CONFIG_UPDATING_GET(x)                             (((x) & AUDIO_PLL_CONFIG_UPDATING_MASK) >> AUDIO_PLL_CONFIG_UPDATING_LSB)
#define AUDIO_PLL_CONFIG_UPDATING_SET(x)                             (((x) << AUDIO_PLL_CONFIG_UPDATING_LSB) & AUDIO_PLL_CONFIG_UPDATING_MASK)
#define AUDIO_PLL_CONFIG_UPDATING_RESET                              1
#define AUDIO_PLL_CONFIG_EXT_DIV_MSB                                 14
#define AUDIO_PLL_CONFIG_EXT_DIV_LSB                                 12
#define AUDIO_PLL_CONFIG_EXT_DIV_MASK                                0x00007000
#define AUDIO_PLL_CONFIG_EXT_DIV_GET(x)                              (((x) & AUDIO_PLL_CONFIG_EXT_DIV_MASK) >> AUDIO_PLL_CONFIG_EXT_DIV_LSB)
#define AUDIO_PLL_CONFIG_EXT_DIV_SET(x)                              (((x) << AUDIO_PLL_CONFIG_EXT_DIV_LSB) & AUDIO_PLL_CONFIG_EXT_DIV_MASK)
#define AUDIO_PLL_CONFIG_EXT_DIV_RESET                               1
#define AUDIO_PLL_CONFIG_POSTPLLDIV_MSB                              9
#define AUDIO_PLL_CONFIG_POSTPLLDIV_LSB                              7
#define AUDIO_PLL_CONFIG_POSTPLLDIV_MASK                             0x00000380
#define AUDIO_PLL_CONFIG_POSTPLLDIV_GET(x)                           (((x) & AUDIO_PLL_CONFIG_POSTPLLDIV_MASK) >> AUDIO_PLL_CONFIG_POSTPLLDIV_LSB)
#define AUDIO_PLL_CONFIG_POSTPLLDIV_SET(x)                           (((x) << AUDIO_PLL_CONFIG_POSTPLLDIV_LSB) & AUDIO_PLL_CONFIG_POSTPLLDIV_MASK)
#define AUDIO_PLL_CONFIG_POSTPLLDIV_RESET                            1
#define AUDIO_PLL_CONFIG_PLLPWD_MSB                                  5
#define AUDIO_PLL_CONFIG_PLLPWD_LSB                                  5
#define AUDIO_PLL_CONFIG_PLLPWD_MASK                                 0x00000020
#define AUDIO_PLL_CONFIG_PLLPWD_GET(x)                               (((x) & AUDIO_PLL_CONFIG_PLLPWD_MASK) >> AUDIO_PLL_CONFIG_PLLPWD_LSB)
#define AUDIO_PLL_CONFIG_PLLPWD_SET(x)                               (((x) << AUDIO_PLL_CONFIG_PLLPWD_LSB) & AUDIO_PLL_CONFIG_PLLPWD_MASK)
#define AUDIO_PLL_CONFIG_PLLPWD_RESET                                1
#define AUDIO_PLL_CONFIG_BYPASS_MSB                                  4
#define AUDIO_PLL_CONFIG_BYPASS_LSB                                  4
#define AUDIO_PLL_CONFIG_BYPASS_MASK                                 0x00000010
#define AUDIO_PLL_CONFIG_BYPASS_GET(x)                               (((x) & AUDIO_PLL_CONFIG_BYPASS_MASK) >> AUDIO_PLL_CONFIG_BYPASS_LSB)
#define AUDIO_PLL_CONFIG_BYPASS_SET(x)                               (((x) << AUDIO_PLL_CONFIG_BYPASS_LSB) & AUDIO_PLL_CONFIG_BYPASS_MASK)
#define AUDIO_PLL_CONFIG_BYPASS_RESET                                1
#define AUDIO_PLL_CONFIG_REFDIV_MSB                                  3
#define AUDIO_PLL_CONFIG_REFDIV_LSB                                  0
#define AUDIO_PLL_CONFIG_REFDIV_MASK                                 0x0000000f
#define AUDIO_PLL_CONFIG_REFDIV_GET(x)                               (((x) & AUDIO_PLL_CONFIG_REFDIV_MASK) >> AUDIO_PLL_CONFIG_REFDIV_LSB)
#define AUDIO_PLL_CONFIG_REFDIV_SET(x)                               (((x) << AUDIO_PLL_CONFIG_REFDIV_LSB) & AUDIO_PLL_CONFIG_REFDIV_MASK)
#define AUDIO_PLL_CONFIG_REFDIV_RESET                                3
#define AUDIO_PLL_CONFIG_ADDRESS                                     0x0030
#define AUDIO_PLL_CONFIG_OFFSET                                      0x0030
// SW modifiable bits
#define AUDIO_PLL_CONFIG_SW_MASK                                     0x800073bf
// bits defined at reset
#define AUDIO_PLL_CONFIG_RSTMASK                                     0xffffffff
// reset value (ignore bits undefined at reset)
#define AUDIO_PLL_CONFIG_RESET                                       0x800010b3

// 32'h0034 (AUDIO_PLL_MODULATION)
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC_MSB                        28
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC_LSB                        11
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC_MASK                       0x1ffff800
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC_GET(x)                     (((x) & AUDIO_PLL_MODULATION_TGT_DIV_FRAC_MASK) >> AUDIO_PLL_MODULATION_TGT_DIV_FRAC_LSB)
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC_SET(x)                     (((x) << AUDIO_PLL_MODULATION_TGT_DIV_FRAC_LSB) & AUDIO_PLL_MODULATION_TGT_DIV_FRAC_MASK)
#define AUDIO_PLL_MODULATION_TGT_DIV_FRAC_RESET                      84222
#define AUDIO_PLL_MODULATION_TGT_DIV_INT_MSB                         6
#define AUDIO_PLL_MODULATION_TGT_DIV_INT_LSB                         1
#define AUDIO_PLL_MODULATION_TGT_DIV_INT_MASK                        0x0000007e
#define AUDIO_PLL_MODULATION_TGT_DIV_INT_GET(x)                      (((x) & AUDIO_PLL_MODULATION_TGT_DIV_INT_MASK) >> AUDIO_PLL_MODULATION_TGT_DIV_INT_LSB)
#define AUDIO_PLL_MODULATION_TGT_DIV_INT_SET(x)                      (((x) << AUDIO_PLL_MODULATION_TGT_DIV_INT_LSB) & AUDIO_PLL_MODULATION_TGT_DIV_INT_MASK)
#define AUDIO_PLL_MODULATION_TGT_DIV_INT_RESET                       20
#define AUDIO_PLL_MODULATION_START_MSB                               0
#define AUDIO_PLL_MODULATION_START_LSB                               0
#define AUDIO_PLL_MODULATION_START_MASK                              0x00000001
#define AUDIO_PLL_MODULATION_START_GET(x)                            (((x) & AUDIO_PLL_MODULATION_START_MASK) >> AUDIO_PLL_MODULATION_START_LSB)
#define AUDIO_PLL_MODULATION_START_SET(x)                            (((x) << AUDIO_PLL_MODULATION_START_LSB) & AUDIO_PLL_MODULATION_START_MASK)
#define AUDIO_PLL_MODULATION_START_RESET                             0
#define AUDIO_PLL_MODULATION_ADDRESS                                 0x0034
#define AUDIO_PLL_MODULATION_OFFSET                                  0x0034
// SW modifiable bits
#define AUDIO_PLL_MODULATION_SW_MASK                                 0x1ffff87f
// bits defined at reset
#define AUDIO_PLL_MODULATION_RSTMASK                                 0xffffffff
// reset value (ignore bits undefined at reset)
#define AUDIO_PLL_MODULATION_RESET                                   0x0a47f028

// 32'h0038 (AUDIO_PLL_MOD_STEP)
#define AUDIO_PLL_MOD_STEP_FRAC_MSB                                  31
#define AUDIO_PLL_MOD_STEP_FRAC_LSB                                  14
#define AUDIO_PLL_MOD_STEP_FRAC_MASK                                 0xffffc000
#define AUDIO_PLL_MOD_STEP_FRAC_GET(x)                               (((x) & AUDIO_PLL_MOD_STEP_FRAC_MASK) >> AUDIO_PLL_MOD_STEP_FRAC_LSB)
#define AUDIO_PLL_MOD_STEP_FRAC_SET(x)                               (((x) << AUDIO_PLL_MOD_STEP_FRAC_LSB) & AUDIO_PLL_MOD_STEP_FRAC_MASK)
#define AUDIO_PLL_MOD_STEP_FRAC_RESET                                1
#define AUDIO_PLL_MOD_STEP_INT_MSB                                   13
#define AUDIO_PLL_MOD_STEP_INT_LSB                                   4
#define AUDIO_PLL_MOD_STEP_INT_MASK                                  0x00003ff0
#define AUDIO_PLL_MOD_STEP_INT_GET(x)                                (((x) & AUDIO_PLL_MOD_STEP_INT_MASK) >> AUDIO_PLL_MOD_STEP_INT_LSB)
#define AUDIO_PLL_MOD_STEP_INT_SET(x)                                (((x) << AUDIO_PLL_MOD_STEP_INT_LSB) & AUDIO_PLL_MOD_STEP_INT_MASK)
#define AUDIO_PLL_MOD_STEP_INT_RESET                                 0
#define AUDIO_PLL_MOD_STEP_UPDATE_CNT_MSB                            3
#define AUDIO_PLL_MOD_STEP_UPDATE_CNT_LSB                            0
#define AUDIO_PLL_MOD_STEP_UPDATE_CNT_MASK                           0x0000000f
#define AUDIO_PLL_MOD_STEP_UPDATE_CNT_GET(x)                         (((x) & AUDIO_PLL_MOD_STEP_UPDATE_CNT_MASK) >> AUDIO_PLL_MOD_STEP_UPDATE_CNT_LSB)
#define AUDIO_PLL_MOD_STEP_UPDATE_CNT_SET(x)                         (((x) << AUDIO_PLL_MOD_STEP_UPDATE_CNT_LSB) & AUDIO_PLL_MOD_STEP_UPDATE_CNT_MASK)
#define AUDIO_PLL_MOD_STEP_UPDATE_CNT_RESET                          0
#define AUDIO_PLL_MOD_STEP_ADDRESS                                   0x0038
#define AUDIO_PLL_MOD_STEP_OFFSET                                    0x0038
// SW modifiable bits
#define AUDIO_PLL_MOD_STEP_SW_MASK                                   0xffffffff
// bits defined at reset
#define AUDIO_PLL_MOD_STEP_RSTMASK                                   0xffffffff
// reset value (ignore bits undefined at reset)
#define AUDIO_PLL_MOD_STEP_RESET                                     0x00004000

// 32'h003c (CURRENT_AUDIO_PLL_MODULATION)
#define CURRENT_AUDIO_PLL_MODULATION_FRAC_MSB                        27
#define CURRENT_AUDIO_PLL_MODULATION_FRAC_LSB                        10
#define CURRENT_AUDIO_PLL_MODULATION_FRAC_MASK                       0x0ffffc00
#define CURRENT_AUDIO_PLL_MODULATION_FRAC_GET(x)                     (((x) & CURRENT_AUDIO_PLL_MODULATION_FRAC_MASK) >> CURRENT_AUDIO_PLL_MODULATION_FRAC_LSB)
#define CURRENT_AUDIO_PLL_MODULATION_FRAC_SET(x)                     (((x) << CURRENT_AUDIO_PLL_MODULATION_FRAC_LSB) & CURRENT_AUDIO_PLL_MODULATION_FRAC_MASK)
#define CURRENT_AUDIO_PLL_MODULATION_FRAC_RESET                      1
#define CURRENT_AUDIO_PLL_MODULATION_INT_MSB                         6
#define CURRENT_AUDIO_PLL_MODULATION_INT_LSB                         1
#define CURRENT_AUDIO_PLL_MODULATION_INT_MASK                        0x0000007e
#define CURRENT_AUDIO_PLL_MODULATION_INT_GET(x)                      (((x) & CURRENT_AUDIO_PLL_MODULATION_INT_MASK) >> CURRENT_AUDIO_PLL_MODULATION_INT_LSB)
#define CURRENT_AUDIO_PLL_MODULATION_INT_SET(x)                      (((x) << CURRENT_AUDIO_PLL_MODULATION_INT_LSB) & CURRENT_AUDIO_PLL_MODULATION_INT_MASK)
#define CURRENT_AUDIO_PLL_MODULATION_INT_RESET                       0
#define CURRENT_AUDIO_PLL_MODULATION_ADDRESS                         0x003c
#define CURRENT_AUDIO_PLL_MODULATION_OFFSET                          0x003c
// SW modifiable bits
#define CURRENT_AUDIO_PLL_MODULATION_SW_MASK                         0x0ffffc7e
// bits defined at reset
#define CURRENT_AUDIO_PLL_MODULATION_RSTMASK                         0xffffffff
// reset value (ignore bits undefined at reset)
#define CURRENT_AUDIO_PLL_MODULATION_RESET                           0x00000400

// 32'h0040 (BB_PLL_CONFIG)
#define BB_PLL_CONFIG_UPDATING_MSB                                   31
#define BB_PLL_CONFIG_UPDATING_LSB                                   31
#define BB_PLL_CONFIG_UPDATING_MASK                                  0x80000000
#define BB_PLL_CONFIG_UPDATING_GET(x)                                (((x) & BB_PLL_CONFIG_UPDATING_MASK) >> BB_PLL_CONFIG_UPDATING_LSB)
#define BB_PLL_CONFIG_UPDATING_SET(x)                                (((x) << BB_PLL_CONFIG_UPDATING_LSB) & BB_PLL_CONFIG_UPDATING_MASK)
#define BB_PLL_CONFIG_UPDATING_RESET                                 1
#define BB_PLL_CONFIG_PLLPWD_MSB                                     30
#define BB_PLL_CONFIG_PLLPWD_LSB                                     30
#define BB_PLL_CONFIG_PLLPWD_MASK                                    0x40000000
#define BB_PLL_CONFIG_PLLPWD_GET(x)                                  (((x) & BB_PLL_CONFIG_PLLPWD_MASK) >> BB_PLL_CONFIG_PLLPWD_LSB)
#define BB_PLL_CONFIG_PLLPWD_SET(x)                                  (((x) << BB_PLL_CONFIG_PLLPWD_LSB) & BB_PLL_CONFIG_PLLPWD_MASK)
#define BB_PLL_CONFIG_PLLPWD_RESET                                   1
#define BB_PLL_CONFIG_SPARE_MSB                                      29
#define BB_PLL_CONFIG_SPARE_LSB                                      29
#define BB_PLL_CONFIG_SPARE_MASK                                     0x20000000
#define BB_PLL_CONFIG_SPARE_GET(x)                                   (((x) & BB_PLL_CONFIG_SPARE_MASK) >> BB_PLL_CONFIG_SPARE_LSB)
#define BB_PLL_CONFIG_SPARE_SET(x)                                   (((x) << BB_PLL_CONFIG_SPARE_LSB) & BB_PLL_CONFIG_SPARE_MASK)
#define BB_PLL_CONFIG_SPARE_RESET                                    0
#define BB_PLL_CONFIG_REFDIV_MSB                                     28
#define BB_PLL_CONFIG_REFDIV_LSB                                     24
#define BB_PLL_CONFIG_REFDIV_MASK                                    0x1f000000
#define BB_PLL_CONFIG_REFDIV_GET(x)                                  (((x) & BB_PLL_CONFIG_REFDIV_MASK) >> BB_PLL_CONFIG_REFDIV_LSB)
#define BB_PLL_CONFIG_REFDIV_SET(x)                                  (((x) << BB_PLL_CONFIG_REFDIV_LSB) & BB_PLL_CONFIG_REFDIV_MASK)
#define BB_PLL_CONFIG_REFDIV_RESET                                   1
#define BB_PLL_CONFIG_NINT_MSB                                       21
#define BB_PLL_CONFIG_NINT_LSB                                       16
#define BB_PLL_CONFIG_NINT_MASK                                      0x003f0000
#define BB_PLL_CONFIG_NINT_GET(x)                                    (((x) & BB_PLL_CONFIG_NINT_MASK) >> BB_PLL_CONFIG_NINT_LSB)
#define BB_PLL_CONFIG_NINT_SET(x)                                    (((x) << BB_PLL_CONFIG_NINT_LSB) & BB_PLL_CONFIG_NINT_MASK)
#define BB_PLL_CONFIG_NINT_RESET                                     2
#define BB_PLL_CONFIG_NFRAC_MSB                                      13
#define BB_PLL_CONFIG_NFRAC_LSB                                      0
#define BB_PLL_CONFIG_NFRAC_MASK                                     0x00003fff
#define BB_PLL_CONFIG_NFRAC_GET(x)                                   (((x) & BB_PLL_CONFIG_NFRAC_MASK) >> BB_PLL_CONFIG_NFRAC_LSB)
#define BB_PLL_CONFIG_NFRAC_SET(x)                                   (((x) << BB_PLL_CONFIG_NFRAC_LSB) & BB_PLL_CONFIG_NFRAC_MASK)
#define BB_PLL_CONFIG_NFRAC_RESET                                    3276
#define BB_PLL_CONFIG_ADDRESS                                        0x0040
#define BB_PLL_CONFIG_OFFSET                                         0x0040
// SW modifiable bits
#define BB_PLL_CONFIG_SW_MASK                                        0xff3f3fff
// bits defined at reset
#define BB_PLL_CONFIG_RSTMASK                                        0xffffffff
// reset value (ignore bits undefined at reset)
#define BB_PLL_CONFIG_RESET                                          0xc1020ccc

// 32'h0044 (DDR_PLL_DITHER)
#define DDR_PLL_DITHER_DITHER_EN_MSB                                 31
#define DDR_PLL_DITHER_DITHER_EN_LSB                                 31
#define DDR_PLL_DITHER_DITHER_EN_MASK                                0x80000000
#define DDR_PLL_DITHER_DITHER_EN_GET(x)                              (((x) & DDR_PLL_DITHER_DITHER_EN_MASK) >> DDR_PLL_DITHER_DITHER_EN_LSB)
#define DDR_PLL_DITHER_DITHER_EN_SET(x)                              (((x) << DDR_PLL_DITHER_DITHER_EN_LSB) & DDR_PLL_DITHER_DITHER_EN_MASK)
#define DDR_PLL_DITHER_DITHER_EN_RESET                               0
#define DDR_PLL_DITHER_UPDATE_COUNT_MSB                              30
#define DDR_PLL_DITHER_UPDATE_COUNT_LSB                              27
#define DDR_PLL_DITHER_UPDATE_COUNT_MASK                             0x78000000
#define DDR_PLL_DITHER_UPDATE_COUNT_GET(x)                           (((x) & DDR_PLL_DITHER_UPDATE_COUNT_MASK) >> DDR_PLL_DITHER_UPDATE_COUNT_LSB)
#define DDR_PLL_DITHER_UPDATE_COUNT_SET(x)                           (((x) << DDR_PLL_DITHER_UPDATE_COUNT_LSB) & DDR_PLL_DITHER_UPDATE_COUNT_MASK)
#define DDR_PLL_DITHER_UPDATE_COUNT_RESET                            15
#define DDR_PLL_DITHER_NFRAC_STEP_MSB                                26
#define DDR_PLL_DITHER_NFRAC_STEP_LSB                                20
#define DDR_PLL_DITHER_NFRAC_STEP_MASK                               0x07f00000
#define DDR_PLL_DITHER_NFRAC_STEP_GET(x)                             (((x) & DDR_PLL_DITHER_NFRAC_STEP_MASK) >> DDR_PLL_DITHER_NFRAC_STEP_LSB)
#define DDR_PLL_DITHER_NFRAC_STEP_SET(x)                             (((x) << DDR_PLL_DITHER_NFRAC_STEP_LSB) & DDR_PLL_DITHER_NFRAC_STEP_MASK)
#define DDR_PLL_DITHER_NFRAC_STEP_RESET                              1
#define DDR_PLL_DITHER_NFRAC_MIN_MSB                                 19
#define DDR_PLL_DITHER_NFRAC_MIN_LSB                                 10
#define DDR_PLL_DITHER_NFRAC_MIN_MASK                                0x000ffc00
#define DDR_PLL_DITHER_NFRAC_MIN_GET(x)                              (((x) & DDR_PLL_DITHER_NFRAC_MIN_MASK) >> DDR_PLL_DITHER_NFRAC_MIN_LSB)
#define DDR_PLL_DITHER_NFRAC_MIN_SET(x)                              (((x) << DDR_PLL_DITHER_NFRAC_MIN_LSB) & DDR_PLL_DITHER_NFRAC_MIN_MASK)
#define DDR_PLL_DITHER_NFRAC_MIN_RESET                               25
#define DDR_PLL_DITHER_NFRAC_MAX_MSB                                 9
#define DDR_PLL_DITHER_NFRAC_MAX_LSB                                 0
#define DDR_PLL_DITHER_NFRAC_MAX_MASK                                0x000003ff
#define DDR_PLL_DITHER_NFRAC_MAX_GET(x)                              (((x) & DDR_PLL_DITHER_NFRAC_MAX_MASK) >> DDR_PLL_DITHER_NFRAC_MAX_LSB)
#define DDR_PLL_DITHER_NFRAC_MAX_SET(x)                              (((x) << DDR_PLL_DITHER_NFRAC_MAX_LSB) & DDR_PLL_DITHER_NFRAC_MAX_MASK)
#define DDR_PLL_DITHER_NFRAC_MAX_RESET                               1000
#define DDR_PLL_DITHER_ADDRESS                                       0x0044
#define DDR_PLL_DITHER_OFFSET                                        0x0044
// SW modifiable bits
#define DDR_PLL_DITHER_SW_MASK                                       0xffffffff
// bits defined at reset
#define DDR_PLL_DITHER_RSTMASK                                       0xffffffff
// reset value (ignore bits undefined at reset)
#define DDR_PLL_DITHER_RESET                                         0x781067e8

// 32'h0048 (CPU_PLL_DITHER)
#define CPU_PLL_DITHER_DITHER_EN_MSB                                 31
#define CPU_PLL_DITHER_DITHER_EN_LSB                                 31
#define CPU_PLL_DITHER_DITHER_EN_MASK                                0x80000000
#define CPU_PLL_DITHER_DITHER_EN_GET(x)                              (((x) & CPU_PLL_DITHER_DITHER_EN_MASK) >> CPU_PLL_DITHER_DITHER_EN_LSB)
#define CPU_PLL_DITHER_DITHER_EN_SET(x)                              (((x) << CPU_PLL_DITHER_DITHER_EN_LSB) & CPU_PLL_DITHER_DITHER_EN_MASK)
#define CPU_PLL_DITHER_DITHER_EN_RESET                               0
#define CPU_PLL_DITHER_UPDATE_COUNT_MSB                              23
#define CPU_PLL_DITHER_UPDATE_COUNT_LSB                              18
#define CPU_PLL_DITHER_UPDATE_COUNT_MASK                             0x00fc0000
#define CPU_PLL_DITHER_UPDATE_COUNT_GET(x)                           (((x) & CPU_PLL_DITHER_UPDATE_COUNT_MASK) >> CPU_PLL_DITHER_UPDATE_COUNT_LSB)
#define CPU_PLL_DITHER_UPDATE_COUNT_SET(x)                           (((x) << CPU_PLL_DITHER_UPDATE_COUNT_LSB) & CPU_PLL_DITHER_UPDATE_COUNT_MASK)
#define CPU_PLL_DITHER_UPDATE_COUNT_RESET                            20
#define CPU_PLL_DITHER_NFRAC_STEP_MSB                                17
#define CPU_PLL_DITHER_NFRAC_STEP_LSB                                12
#define CPU_PLL_DITHER_NFRAC_STEP_MASK                               0x0003f000
#define CPU_PLL_DITHER_NFRAC_STEP_GET(x)                             (((x) & CPU_PLL_DITHER_NFRAC_STEP_MASK) >> CPU_PLL_DITHER_NFRAC_STEP_LSB)
#define CPU_PLL_DITHER_NFRAC_STEP_SET(x)                             (((x) << CPU_PLL_DITHER_NFRAC_STEP_LSB) & CPU_PLL_DITHER_NFRAC_STEP_MASK)
#define CPU_PLL_DITHER_NFRAC_STEP_RESET                              1
#define CPU_PLL_DITHER_NFRAC_MIN_MSB                                 11
#define CPU_PLL_DITHER_NFRAC_MIN_LSB                                 6
#define CPU_PLL_DITHER_NFRAC_MIN_MASK                                0x00000fc0
#define CPU_PLL_DITHER_NFRAC_MIN_GET(x)                              (((x) & CPU_PLL_DITHER_NFRAC_MIN_MASK) >> CPU_PLL_DITHER_NFRAC_MIN_LSB)
#define CPU_PLL_DITHER_NFRAC_MIN_SET(x)                              (((x) << CPU_PLL_DITHER_NFRAC_MIN_LSB) & CPU_PLL_DITHER_NFRAC_MIN_MASK)
#define CPU_PLL_DITHER_NFRAC_MIN_RESET                               3
#define CPU_PLL_DITHER_NFRAC_MAX_MSB                                 5
#define CPU_PLL_DITHER_NFRAC_MAX_LSB                                 0
#define CPU_PLL_DITHER_NFRAC_MAX_MASK                                0x0000003f
#define CPU_PLL_DITHER_NFRAC_MAX_GET(x)                              (((x) & CPU_PLL_DITHER_NFRAC_MAX_MASK) >> CPU_PLL_DITHER_NFRAC_MAX_LSB)
#define CPU_PLL_DITHER_NFRAC_MAX_SET(x)                              (((x) << CPU_PLL_DITHER_NFRAC_MAX_LSB) & CPU_PLL_DITHER_NFRAC_MAX_MASK)
#define CPU_PLL_DITHER_NFRAC_MAX_RESET                               60
#define CPU_PLL_DITHER_ADDRESS                                       0x0048
#define CPU_PLL_DITHER_OFFSET                                        0x0048
// SW modifiable bits
#define CPU_PLL_DITHER_SW_MASK                                       0x80ffffff
// bits defined at reset
#define CPU_PLL_DITHER_RSTMASK                                       0xffffffff
// reset value (ignore bits undefined at reset)
#define CPU_PLL_DITHER_RESET                                         0x005010fc

// 32'h181161c0 (DPLL)
#define CPU_DPLL_REFDIV_MSB                                              31
#define CPU_DPLL_REFDIV_LSB                                              27
#define CPU_DPLL_REFDIV_MASK                                             0xf8000000
#define CPU_DPLL_REFDIV_GET(x)                                           (((x) & CPU_DPLL_REFDIV_MASK) >> CPU_DPLL_REFDIV_LSB)
#define CPU_DPLL_REFDIV_SET(x)                                           (((x) << CPU_DPLL_REFDIV_LSB) & CPU_DPLL_REFDIV_MASK)
#define CPU_DPLL_REFDIV_RESET                                            0x1 // 1
#define CPU_DPLL_NINT_MSB                                                26
#define CPU_DPLL_NINT_LSB                                                18
#define CPU_DPLL_NINT_MASK                                               0x07fc0000
#define CPU_DPLL_NINT_GET(x)                                             (((x) & CPU_DPLL_NINT_MASK) >> CPU_DPLL_NINT_LSB)
#define CPU_DPLL_NINT_SET(x)                                             (((x) << CPU_DPLL_NINT_LSB) & CPU_DPLL_NINT_MASK)
#define CPU_DPLL_NINT_RESET                                              0x10 // 16
#define CPU_DPLL_NFRAC_MSB                                               17
#define CPU_DPLL_NFRAC_LSB                                               0
#define CPU_DPLL_NFRAC_MASK                                              0x0003ffff
#define CPU_DPLL_NFRAC_GET(x)                                            (((x) & CPU_DPLL_NFRAC_MASK) >> CPU_DPLL_NFRAC_LSB)
#define CPU_DPLL_NFRAC_SET(x)                                            (((x) << CPU_DPLL_NFRAC_LSB) & CPU_DPLL_NFRAC_MASK)
#define CPU_DPLL_NFRAC_RESET                                             0x0 // 0
#define CPU_DPLL_ADDRESS                                                 0x181161c0

#define CPU_DPLL2_RANGE_MSB                                              31
#define CPU_DPLL2_RANGE_LSB                                              31
#define CPU_DPLL2_RANGE_MASK                                             0x80000000
#define CPU_DPLL2_RANGE_GET(x)                                           (((x) & CPU_DPLL2_RANGE_MASK) >> CPU_DPLL2_RANGE_LSB)
#define CPU_DPLL2_RANGE_SET(x)                                           (((x) << CPU_DPLL2_RANGE_LSB) & CPU_DPLL2_RANGE_MASK)
#define CPU_DPLL2_RANGE_RESET                                            0x0 // 0
#define CPU_DPLL2_LOCAL_PLL_MSB                                          30
#define CPU_DPLL2_LOCAL_PLL_LSB                                          30
#define CPU_DPLL2_LOCAL_PLL_MASK                                         0x40000000
#define CPU_DPLL2_LOCAL_PLL_GET(x)                                       (((x) & CPU_DPLL2_LOCAL_PLL_MASK) >> CPU_DPLL2_LOCAL_PLL_LSB)
#define CPU_DPLL2_LOCAL_PLL_SET(x)                                       (((x) << CPU_DPLL2_LOCAL_PLL_LSB) & CPU_DPLL2_LOCAL_PLL_MASK)
#define CPU_DPLL2_LOCAL_PLL_RESET                                        0x0 // 0
#define CPU_DPLL2_KI_MSB                                                 29
#define CPU_DPLL2_KI_LSB                                                 26
#define CPU_DPLL2_KI_MASK                                                0x3c000000
#define CPU_DPLL2_KI_GET(x)                                              (((x) & CPU_DPLL2_KI_MASK) >> CPU_DPLL2_KI_LSB)
#define CPU_DPLL2_KI_SET(x)                                              (((x) << CPU_DPLL2_KI_LSB) & CPU_DPLL2_KI_MASK)
#define CPU_DPLL2_KI_RESET                                               0x6 // 6
#define CPU_DPLL2_KD_MSB                                                 25
#define CPU_DPLL2_KD_LSB                                                 19
#define CPU_DPLL2_KD_MASK                                                0x03f80000
#define CPU_DPLL2_KD_GET(x)                                              (((x) & CPU_DPLL2_KD_MASK) >> CPU_DPLL2_KD_LSB)
#define CPU_DPLL2_KD_SET(x)                                              (((x) << CPU_DPLL2_KD_LSB) & CPU_DPLL2_KD_MASK)
#define CPU_DPLL2_KD_RESET                                               0x7f // 127
#define CPU_DPLL2_EN_NEGTRIG_MSB                                         18
#define CPU_DPLL2_EN_NEGTRIG_LSB                                         18
#define CPU_DPLL2_EN_NEGTRIG_MASK                                        0x00040000
#define CPU_DPLL2_EN_NEGTRIG_GET(x)                                      (((x) & CPU_DPLL2_EN_NEGTRIG_MASK) >> CPU_DPLL2_EN_NEGTRIG_LSB)
#define CPU_DPLL2_EN_NEGTRIG_SET(x)                                      (((x) << CPU_DPLL2_EN_NEGTRIG_LSB) & CPU_DPLL2_EN_NEGTRIG_MASK)
#define CPU_DPLL2_EN_NEGTRIG_RESET                                       0x0 // 0
#define CPU_DPLL2_SEL_1SDM_MSB                                           17
#define CPU_DPLL2_SEL_1SDM_LSB                                           17
#define CPU_DPLL2_SEL_1SDM_MASK                                          0x00020000
#define CPU_DPLL2_SEL_1SDM_GET(x)                                        (((x) & CPU_DPLL2_SEL_1SDM_MASK) >> CPU_DPLL2_SEL_1SDM_LSB)
#define CPU_DPLL2_SEL_1SDM_SET(x)                                        (((x) << CPU_DPLL2_SEL_1SDM_LSB) & CPU_DPLL2_SEL_1SDM_MASK)
#define CPU_DPLL2_SEL_1SDM_RESET                                         0x0 // 0
#define CPU_DPLL2_PLL_PWD_MSB                                            16
#define CPU_DPLL2_PLL_PWD_LSB                                            16
#define CPU_DPLL2_PLL_PWD_MASK                                           0x00010000
#define CPU_DPLL2_PLL_PWD_GET(x)                                         (((x) & CPU_DPLL2_PLL_PWD_MASK) >> CPU_DPLL2_PLL_PWD_LSB)
#define CPU_DPLL2_PLL_PWD_SET(x)                                         (((x) << CPU_DPLL2_PLL_PWD_LSB) & CPU_DPLL2_PLL_PWD_MASK)
#define CPU_DPLL2_PLL_PWD_RESET                                          0x1 // 1
#define CPU_DPLL2_OUTDIV_MSB                                             15
#define CPU_DPLL2_OUTDIV_LSB                                             13
#define CPU_DPLL2_OUTDIV_MASK                                            0x0000e000
#define CPU_DPLL2_OUTDIV_GET(x)                                          (((x) & CPU_DPLL2_OUTDIV_MASK) >> CPU_DPLL2_OUTDIV_LSB)
#define CPU_DPLL2_OUTDIV_SET(x)                                          (((x) << CPU_DPLL2_OUTDIV_LSB) & CPU_DPLL2_OUTDIV_MASK)
#define CPU_DPLL2_OUTDIV_RESET                                           0x0 // 0
#define CPU_DPLL2_DELTA_MSB                                              12
#define CPU_DPLL2_DELTA_LSB                                              7
#define CPU_DPLL2_DELTA_MASK                                             0x00001f80
#define CPU_DPLL2_DELTA_GET(x)                                           (((x) & CPU_DPLL2_DELTA_MASK) >> CPU_DPLL2_DELTA_LSB)
#define CPU_DPLL2_DELTA_SET(x)                                           (((x) << CPU_DPLL2_DELTA_LSB) & CPU_DPLL2_DELTA_MASK)
#define CPU_DPLL2_DELTA_RESET                                            0x1e // 30
#define CPU_DPLL2_SPARE_MSB                                              6
#define CPU_DPLL2_SPARE_LSB                                              0
#define CPU_DPLL2_SPARE_MASK                                             0x0000007f
#define CPU_DPLL2_SPARE_GET(x)                                           (((x) & CPU_DPLL2_SPARE_MASK) >> CPU_DPLL2_SPARE_LSB)
#define CPU_DPLL2_SPARE_SET(x)                                           (((x) << CPU_DPLL2_SPARE_LSB) & CPU_DPLL2_SPARE_MASK)
#define CPU_DPLL2_SPARE_RESET                                            0x0 // 0
#define CPU_DPLL2_ADDRESS                                                0x181161c4

// 32'h18116240 (DDR_DPLL)
#define DDR_DPLL_REFDIV_MSB                                              31
#define DDR_DPLL_REFDIV_LSB                                              27
#define DDR_DPLL_REFDIV_MASK                                             0xf8000000
#define DDR_DPLL_REFDIV_GET(x)                                           (((x) & DDR_DPLL_REFDIV_MASK) >> DDR_DPLL_REFDIV_LSB)
#define DDR_DPLL_REFDIV_SET(x)                                           (((x) << DDR_DPLL_REFDIV_LSB) & DDR_DPLL_REFDIV_MASK)
#define DDR_DPLL_REFDIV_RESET                                            0x1 // 1
#define DDR_DPLL_NINT_MSB                                                26
#define DDR_DPLL_NINT_LSB                                                18
#define DDR_DPLL_NINT_MASK                                               0x07fc0000
#define DDR_DPLL_NINT_GET(x)                                             (((x) & DDR_DPLL_NINT_MASK) >> DDR_DPLL_NINT_LSB)
#define DDR_DPLL_NINT_SET(x)                                             (((x) << DDR_DPLL_NINT_LSB) & DDR_DPLL_NINT_MASK)
#define DDR_DPLL_NINT_RESET                                              0x10 // 16
#define DDR_DPLL_NFRAC_MSB                                               17
#define DDR_DPLL_NFRAC_LSB                                               0
#define DDR_DPLL_NFRAC_MASK                                              0x0003ffff
#define DDR_DPLL_NFRAC_GET(x)                                            (((x) & DDR_DPLL_NFRAC_MASK) >> DDR_DPLL_NFRAC_LSB)
#define DDR_DPLL_NFRAC_SET(x)                                            (((x) << DDR_DPLL_NFRAC_LSB) & DDR_DPLL_NFRAC_MASK)
#define DDR_DPLL_NFRAC_RESET                                             0x0 // 0
#define DDR_DPLL_ADDRESS                                                 0x18116240

// 32'h18116244 (DDR_DPLL2)
#define DDR_DPLL2_RANGE_MSB                                              31
#define DDR_DPLL2_RANGE_LSB                                              31
#define DDR_DPLL2_RANGE_MASK                                             0x80000000
#define DDR_DPLL2_RANGE_GET(x)                                           (((x) & DDR_DPLL2_RANGE_MASK) >> DDR_DPLL2_RANGE_LSB)
#define DDR_DPLL2_RANGE_SET(x)                                           (((x) << DDR_DPLL2_RANGE_LSB) & DDR_DPLL2_RANGE_MASK)
#define DDR_DPLL2_RANGE_RESET                                            0x0 // 0
#define DDR_DPLL2_LOCAL_PLL_MSB                                          30
#define DDR_DPLL2_LOCAL_PLL_LSB                                          30
#define DDR_DPLL2_LOCAL_PLL_MASK                                         0x40000000
#define DDR_DPLL2_LOCAL_PLL_GET(x)                                       (((x) & DDR_DPLL2_LOCAL_PLL_MASK) >> DDR_DPLL2_LOCAL_PLL_LSB)
#define DDR_DPLL2_LOCAL_PLL_SET(x)                                       (((x) << DDR_DPLL2_LOCAL_PLL_LSB) & DDR_DPLL2_LOCAL_PLL_MASK)
#define DDR_DPLL2_LOCAL_PLL_RESET                                        0x0 // 0
#define DDR_DPLL2_KI_MSB                                                 29
#define DDR_DPLL2_KI_LSB                                                 26
#define DDR_DPLL2_KI_MASK                                                0x3c000000
#define DDR_DPLL2_KI_GET(x)                                              (((x) & DDR_DPLL2_KI_MASK) >> DDR_DPLL2_KI_LSB)
#define DDR_DPLL2_KI_SET(x)                                              (((x) << DDR_DPLL2_KI_LSB) & DDR_DPLL2_KI_MASK)
#define DDR_DPLL2_KI_RESET                                               0x6 // 6
#define DDR_DPLL2_KD_MSB                                                 25
#define DDR_DPLL2_KD_LSB                                                 19
#define DDR_DPLL2_KD_MASK                                                0x03f80000
#define DDR_DPLL2_KD_GET(x)                                              (((x) & DDR_DPLL2_KD_MASK) >> DDR_DPLL2_KD_LSB)
#define DDR_DPLL2_KD_SET(x)                                              (((x) << DDR_DPLL2_KD_LSB) & DDR_DPLL2_KD_MASK)
#define DDR_DPLL2_KD_RESET                                               0x7f // 127
#define DDR_DPLL2_EN_NEGTRIG_MSB                                         18
#define DDR_DPLL2_EN_NEGTRIG_LSB                                         18
#define DDR_DPLL2_EN_NEGTRIG_MASK                                        0x00040000
#define DDR_DPLL2_EN_NEGTRIG_GET(x)                                      (((x) & DDR_DPLL2_EN_NEGTRIG_MASK) >> DDR_DPLL2_EN_NEGTRIG_LSB)
#define DDR_DPLL2_EN_NEGTRIG_SET(x)                                      (((x) << DDR_DPLL2_EN_NEGTRIG_LSB) & DDR_DPLL2_EN_NEGTRIG_MASK)
#define DDR_DPLL2_EN_NEGTRIG_RESET                                       0x0 // 0
#define DDR_DPLL2_SEL_1SDM_MSB                                           17
#define DDR_DPLL2_SEL_1SDM_LSB                                           17
#define DDR_DPLL2_SEL_1SDM_MASK                                          0x00020000
#define DDR_DPLL2_SEL_1SDM_GET(x)                                        (((x) & DDR_DPLL2_SEL_1SDM_MASK) >> DDR_DPLL2_SEL_1SDM_LSB)
#define DDR_DPLL2_SEL_1SDM_SET(x)                                        (((x) << DDR_DPLL2_SEL_1SDM_LSB) & DDR_DPLL2_SEL_1SDM_MASK)
#define DDR_DPLL2_SEL_1SDM_RESET                                         0x0 // 0
#define DDR_DPLL2_PLL_PWD_MSB                                            16
#define DDR_DPLL2_PLL_PWD_LSB                                            16
#define DDR_DPLL2_PLL_PWD_MASK                                           0x00010000
#define DDR_DPLL2_PLL_PWD_GET(x)                                         (((x) & DDR_DPLL2_PLL_PWD_MASK) >> DDR_DPLL2_PLL_PWD_LSB)
#define DDR_DPLL2_PLL_PWD_SET(x)                                         (((x) << DDR_DPLL2_PLL_PWD_LSB) & DDR_DPLL2_PLL_PWD_MASK)
#define DDR_DPLL2_PLL_PWD_RESET                                          0x1 // 1
#define DDR_DPLL2_OUTDIV_MSB                                             15
#define DDR_DPLL2_OUTDIV_LSB                                             13
#define DDR_DPLL2_OUTDIV_MASK                                            0x0000e000
#define DDR_DPLL2_OUTDIV_GET(x)                                          (((x) & DDR_DPLL2_OUTDIV_MASK) >> DDR_DPLL2_OUTDIV_LSB)
#define DDR_DPLL2_OUTDIV_SET(x)                                          (((x) << DDR_DPLL2_OUTDIV_LSB) & DDR_DPLL2_OUTDIV_MASK)
#define DDR_DPLL2_OUTDIV_RESET                                           0x0 // 0
#define DDR_DPLL2_DELTA_MSB                                              12
#define DDR_DPLL2_DELTA_LSB                                              7
#define DDR_DPLL2_DELTA_MASK                                             0x00001f80
#define DDR_DPLL2_DELTA_GET(x)                                           (((x) & DDR_DPLL2_DELTA_MASK) >> DDR_DPLL2_DELTA_LSB)
#define DDR_DPLL2_DELTA_SET(x)                                           (((x) << DDR_DPLL2_DELTA_LSB) & DDR_DPLL2_DELTA_MASK)
#define DDR_DPLL2_DELTA_RESET                                            0x1e // 30
#define DDR_DPLL2_SPARE_MSB                                              6
#define DDR_DPLL2_SPARE_LSB                                              0
#define DDR_DPLL2_SPARE_MASK                                             0x0000007f
#define DDR_DPLL2_SPARE_GET(x)                                           (((x) & DDR_DPLL2_SPARE_MASK) >> DDR_DPLL2_SPARE_LSB)
#define DDR_DPLL2_SPARE_SET(x)                                           (((x) << DDR_DPLL2_SPARE_LSB) & DDR_DPLL2_SPARE_MASK)
#define DDR_DPLL2_SPARE_RESET                                            0x0 // 0
#define DDR_DPLL2_ADDRESS                                                0x18116244


#ifdef CONFIG_ATHRS_HW_NAT

/*
 * Ingress / Egress LUT Registers
 */
#define ATHR_EGRESS_LUT_REG       (ATH_APB_BASE + 0x80000)
#define ATHR_INGRESS_LUT_REG      (ATH_APB_BASE + 0x81000)

/*
 * Fragment LUT Registers
 */
#define ATHR_EGRESS_LUT_FRAG_REG  (ATH_APB_BASE + 0x82000)
#define ATHR_INGRESS_LUT_FRAG_REG (ATH_APB_BASE + 0x83000)

/*
 * Global IP Address Registers for wan interface
 */
#define ATHR_WAN_IP_ADDR_REG	  (ATH_GE0_BASE + 0x210)

/*
 * Nat status Registers
 */
#define ETH_EG_NAT_STATUS          (ATH_GE0_BASE + 0x228)
#define ETH_IG_NAT_STATUS          (ATH_GE0_BASE + 0x230)

/*
 * Ager Registers
 */
#define ETH_EG_AGER_FIFO_REG       (ATH_APB_BASE + 0x80020)
#define ETH_IG_AGER_FIFO_REG       (ATH_APB_BASE + 0x81030)

/*
 * Nat counter Registers
 */
#define ETH_EG_NAT_CNTR          (ATH_GE0_BASE + 0x22c)
#define ETH_IG_NAT_CNTR          (ATH_GE0_BASE + 0x234)

/*
 * Nat debug Registers
 */
#define ETH_EG_NAT_DBG           (ATH_GE0_BASE + 0x310)
#define ETH_IG_NAT_DBG           (ATH_GE0_BASE + 0x2f8)

/*
 * MAC Addr Registers
 */

#define ETH_LCL_MAC_ADDR_DW0       (ATH_GE0_BASE + 0x200)
#define ETH_LCL_MAC_ADDR_DW1       (ATH_GE0_BASE + 0x204)
#define ETH_DST_MAC_ADDR_DW0       (ATH_GE0_BASE + 0x208)
#define ETH_DST_MAC_ADDR_DW1       (ATH_GE0_BASE + 0x20c)

/*
 * Ager INTR Registers
 */
#define ATHR_LUTS_AGER_INTR        (ATH_APB_BASE + 0x70004)
#define ATHR_LUTS_AGER_INTR_MASK   (ATH_APB_BASE + 0x70008)

#endif /* #ifdef CONFIG_ATHRS_HW_NAT */

#ifdef CONFIG_ATHRS_HW_ACL

/********************************************************************
 * EG ACL Registers
 *********************************************************************/
#define ATHR_EG_ACL_STATUS              (ATH_GE0_BASE + 0x238)
#define ATHR_EG_ACL_CMD0_AND_ACTION     (ATH_GE0_BASE + 0x240)
#define ATHR_EG_ACL_CMD1234             (ATH_GE0_BASE + 0x244)
#define ATHR_EG_ACL_OPERAND0            (ATH_GE0_BASE + 0x248)
#define ATHR_EG_ACL_OPERAND1            (ATH_GE0_BASE + 0x24c)
#define ATHR_EG_ACL_MEM_CTRL            (ATH_GE0_BASE + 0x250)
#define ATHR_EG_ACL_RULE_TABLE_FIELDS   (ATH_GE0_BASE + 0x378)
#define ATHR_EG_ACL_RULE_TABLE0_LOWER   (ATH_GE0_BASE + 0x354)
#define ATHR_EG_ACL_RULE_TABLE0_UPPER   (ATH_GE0_BASE + 0x358)
#define ATHR_EG_ACL_RULE_TABLE1_LOWER   (ATH_GE0_BASE + 0x35c)
#define ATHR_EG_ACL_RULE_TABLE1_UPPER   (ATH_GE0_BASE + 0x360)
#define ATHR_EG_ACL_RULE_TABLE2_LOWER   (ATH_GE0_BASE + 0x364)
#define ATHR_EG_ACL_RULE_TABLE2_UPPER   (ATH_GE0_BASE + 0x368)
#define ATHR_EG_ACL_RULE_TABLE3_LOWER   (ATH_GE0_BASE + 0x36c)
#define ATHR_EG_ACL_RULE_TABLE3_UPPER   (ATH_GE0_BASE + 0x370)

/********************************************************************
 * IG ACL Registers
 ********************************************************************/
#define ATHR_IG_ACL_STATUS              (ATH_GE0_BASE + 0x23C)
#define ATHR_IG_ACL_CMD0_AND_ACTION     (ATH_GE0_BASE + 0x254)
#define ATHR_IG_ACL_CMD1234             (ATH_GE0_BASE + 0x258)
#define ATHR_IG_ACL_OPERAND0            (ATH_GE0_BASE + 0x25C)
#define ATHR_IG_ACL_OPERAND1            (ATH_GE0_BASE + 0x260)
#define ATHR_IG_ACL_MEM_CTRL            (ATH_GE0_BASE + 0x264)
#define ATHR_IG_ACL_RULE_TABLE_FIELDS   (ATH_GE0_BASE + 0x374)
#define ATHR_IG_ACL_RULE_TABLE0_LOWER   (ATH_GE0_BASE + 0x334)
#define ATHR_IG_ACL_RULE_TABLE0_UPPER   (ATH_GE0_BASE + 0x338)
#define ATHR_IG_ACL_RULE_TABLE1_LOWER   (ATH_GE0_BASE + 0x33c)
#define ATHR_IG_ACL_RULE_TABLE1_UPPER   (ATH_GE0_BASE + 0x340)
#define ATHR_IG_ACL_RULE_TABLE2_LOWER   (ATH_GE0_BASE + 0x344)
#define ATHR_IG_ACL_RULE_TABLE2_UPPER   (ATH_GE0_BASE + 0x348)
#define ATHR_IG_ACL_RULE_TABLE3_LOWER   (ATH_GE0_BASE + 0x34c)
#define ATHR_IG_ACL_RULE_TABLE3_UPPER   (ATH_GE0_BASE + 0x350)

#endif /* #ifdef CONFIG_ATHRS_HW_ACL */


/*
 * From
 * //depot/chips/wasp/1.0/rtl/otp_intf/blueprint/efuse_reg.h
 */

// 32'h0000 (OTP_MEM_0)
#define OTP_MEM_0_OTP_MEM_MSB				31
#define OTP_MEM_0_OTP_MEM_LSB				0
#define OTP_MEM_0_OTP_MEM_MASK				0xffffffff
#define OTP_MEM_0_OTP_MEM_GET(x)			(((x) & OTP_MEM_0_OTP_MEM_MASK) >> OTP_MEM_0_OTP_MEM_LSB)
#define OTP_MEM_0_OTP_MEM_SET(x)			(((x) << OTP_MEM_0_OTP_MEM_LSB) & OTP_MEM_0_OTP_MEM_MASK)
// #define OTP_MEM_0_OTP_MEM_RESET				32'd0
#define OTP_MEM_0_ADDRESS				0x0000
#define OTP_MEM_0_OFFSET				0x0000
#define OTP_MEM_ADDRESS					OTP_MEM_0_ADDRESS
#define OTP_MEM_OFFSET					OTP_MEM_0_OFFSET
// SW modifiable bits
#define OTP_MEM_0_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_MEM_0_RSTMASK				0x00000000
// reset value (ignore bits undefined at reset)
#define OTP_MEM_0_RESET					0x00000000

// 32'h1000 (OTP_INTF0)
#define OTP_INTF0_EFUSE_WR_ENABLE_REG_V_MSB		31
#define OTP_INTF0_EFUSE_WR_ENABLE_REG_V_LSB		0
#define OTP_INTF0_EFUSE_WR_ENABLE_REG_V_MASK		0xffffffff
#define OTP_INTF0_EFUSE_WR_ENABLE_REG_V_GET(x)		(((x) & OTP_INTF0_EFUSE_WR_ENABLE_REG_V_MASK) >> OTP_INTF0_EFUSE_WR_ENABLE_REG_V_LSB)
#define OTP_INTF0_EFUSE_WR_ENABLE_REG_V_SET(x)		(((x) << OTP_INTF0_EFUSE_WR_ENABLE_REG_V_LSB) & OTP_INTF0_EFUSE_WR_ENABLE_REG_V_MASK)
// #define OTP_INTF0_EFUSE_WR_ENABLE_REG_V_RESET		32'd0
#define OTP_INTF0_ADDRESS				0x1000
#define OTP_INTF0_OFFSET				0x1000
// SW modifiable bits
#define OTP_INTF0_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_INTF0_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF0_RESET					0x00000000

// 32'h1004 (OTP_INTF1)
#define OTP_INTF1_BITMASK_WR_REG_V_MSB			31
#define OTP_INTF1_BITMASK_WR_REG_V_LSB			0
#define OTP_INTF1_BITMASK_WR_REG_V_MASK			0xffffffff
#define OTP_INTF1_BITMASK_WR_REG_V_GET(x)		(((x) & OTP_INTF1_BITMASK_WR_REG_V_MASK) >> OTP_INTF1_BITMASK_WR_REG_V_LSB)
#define OTP_INTF1_BITMASK_WR_REG_V_SET(x)		(((x) << OTP_INTF1_BITMASK_WR_REG_V_LSB) & OTP_INTF1_BITMASK_WR_REG_V_MASK)
// #define OTP_INTF1_BITMASK_WR_REG_V_RESET		32'd0
#define OTP_INTF1_ADDRESS				0x1004
#define OTP_INTF1_OFFSET				0x1004
// SW modifiable bits
#define OTP_INTF1_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_INTF1_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF1_RESET					0x00000000

// 32'h1008 (OTP_INTF2)
#define OTP_INTF2_PG_STROBE_PW_REG_V_MSB		31
#define OTP_INTF2_PG_STROBE_PW_REG_V_LSB		0
#define OTP_INTF2_PG_STROBE_PW_REG_V_MASK		0xffffffff
#define OTP_INTF2_PG_STROBE_PW_REG_V_GET(x)		(((x) & OTP_INTF2_PG_STROBE_PW_REG_V_MASK) >> OTP_INTF2_PG_STROBE_PW_REG_V_LSB)
#define OTP_INTF2_PG_STROBE_PW_REG_V_SET(x)		(((x) << OTP_INTF2_PG_STROBE_PW_REG_V_LSB) & OTP_INTF2_PG_STROBE_PW_REG_V_MASK)
// #define OTP_INTF2_PG_STROBE_PW_REG_V_RESET		32'dPG_STROBE_PW_RESET_IN
#define OTP_INTF2_ADDRESS				0x1008
#define OTP_INTF2_OFFSET				0x1008
// SW modifiable bits
#define OTP_INTF2_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_INTF2_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF2_RESET					0x00000000

// 32'h100c (OTP_INTF3)
#define OTP_INTF3_RD_STROBE_PW_REG_V_MSB		31
#define OTP_INTF3_RD_STROBE_PW_REG_V_LSB		0
#define OTP_INTF3_RD_STROBE_PW_REG_V_MASK		0xffffffff
#define OTP_INTF3_RD_STROBE_PW_REG_V_GET(x)		(((x) & OTP_INTF3_RD_STROBE_PW_REG_V_MASK) >> OTP_INTF3_RD_STROBE_PW_REG_V_LSB)
#define OTP_INTF3_RD_STROBE_PW_REG_V_SET(x)		(((x) << OTP_INTF3_RD_STROBE_PW_REG_V_LSB) & OTP_INTF3_RD_STROBE_PW_REG_V_MASK)
// #define OTP_INTF3_RD_STROBE_PW_REG_V_RESET		32'dRD_STROBE_PW_RESET_IN
#define OTP_INTF3_ADDRESS				0x100c
#define OTP_INTF3_OFFSET				0x100c
// SW modifiable bits
#define OTP_INTF3_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_INTF3_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF3_RESET					0x00000000

// 32'h1010 (OTP_INTF4)
#define OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_MSB		31
#define OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_LSB		0
#define OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_MASK		0xffffffff
#define OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_GET(x)		(((x) & OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_MASK) >> OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_LSB)
#define OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_SET(x)		(((x) << OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_LSB) & OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_MASK)
// #define OTP_INTF4_VDDQ_SETTLE_TIME_REG_V_RESET		32'dVDDQ_SETTLE_TIME_RESET_IN
#define OTP_INTF4_ADDRESS				0x1010
#define OTP_INTF4_OFFSET				0x1010
// SW modifiable bits
#define OTP_INTF4_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_INTF4_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF4_RESET					0x00000000

// 32'h1014 (OTP_INTF5)
#define OTP_INTF5_EFUSE_INT_ENABLE_REG_V_MSB		0
#define OTP_INTF5_EFUSE_INT_ENABLE_REG_V_LSB		0
#define OTP_INTF5_EFUSE_INT_ENABLE_REG_V_MASK		0x00000001
#define OTP_INTF5_EFUSE_INT_ENABLE_REG_V_GET(x)		(((x) & OTP_INTF5_EFUSE_INT_ENABLE_REG_V_MASK) >> OTP_INTF5_EFUSE_INT_ENABLE_REG_V_LSB)
#define OTP_INTF5_EFUSE_INT_ENABLE_REG_V_SET(x)		(((x) << OTP_INTF5_EFUSE_INT_ENABLE_REG_V_LSB) & OTP_INTF5_EFUSE_INT_ENABLE_REG_V_MASK)
// #define OTP_INTF5_EFUSE_INT_ENABLE_REG_V_RESET		1'd0
#define OTP_INTF5_ADDRESS				0x1014
#define OTP_INTF5_OFFSET				0x1014
// SW modifiable bits
#define OTP_INTF5_SW_MASK				0x00000001
// bits defined at reset
#define OTP_INTF5_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF5_RESET					0x00000000

// 32'h1018 (OTP_STATUS0)
#define OTP_STATUS0_EFUSE_READ_DATA_VALID_MSB		2
#define OTP_STATUS0_EFUSE_READ_DATA_VALID_LSB		2
#define OTP_STATUS0_EFUSE_READ_DATA_VALID_MASK		0x00000004
#define OTP_STATUS0_EFUSE_READ_DATA_VALID_GET(x)	(((x) & OTP_STATUS0_EFUSE_READ_DATA_VALID_MASK) >> OTP_STATUS0_EFUSE_READ_DATA_VALID_LSB)
#define OTP_STATUS0_EFUSE_READ_DATA_VALID_SET(x)	(((x) << OTP_STATUS0_EFUSE_READ_DATA_VALID_LSB) & OTP_STATUS0_EFUSE_READ_DATA_VALID_MASK)
// #define OTP_STATUS0_EFUSE_READ_DATA_VALID_RESET		1'd0
#define OTP_STATUS0_EFUSE_ACCESS_BUSY_MSB		1
#define OTP_STATUS0_EFUSE_ACCESS_BUSY_LSB		1
#define OTP_STATUS0_EFUSE_ACCESS_BUSY_MASK		0x00000002
#define OTP_STATUS0_EFUSE_ACCESS_BUSY_GET(x)		(((x) & OTP_STATUS0_EFUSE_ACCESS_BUSY_MASK) >> OTP_STATUS0_EFUSE_ACCESS_BUSY_LSB)
#define OTP_STATUS0_EFUSE_ACCESS_BUSY_SET(x)		(((x) << OTP_STATUS0_EFUSE_ACCESS_BUSY_LSB) & OTP_STATUS0_EFUSE_ACCESS_BUSY_MASK)
// #define OTP_STATUS0_EFUSE_ACCESS_BUSY_RESET		1'd0
#define OTP_STATUS0_OTP_SM_BUSY_MSB			0
#define OTP_STATUS0_OTP_SM_BUSY_LSB			0
#define OTP_STATUS0_OTP_SM_BUSY_MASK			0x00000001
#define OTP_STATUS0_OTP_SM_BUSY_GET(x)			(((x) & OTP_STATUS0_OTP_SM_BUSY_MASK) >> OTP_STATUS0_OTP_SM_BUSY_LSB)
#define OTP_STATUS0_OTP_SM_BUSY_SET(x)			(((x) << OTP_STATUS0_OTP_SM_BUSY_LSB) & OTP_STATUS0_OTP_SM_BUSY_MASK)
// #define OTP_STATUS0_OTP_SM_BUSY_RESET			1'd0
#define OTP_STATUS0_ADDRESS				0x1018
#define OTP_STATUS0_OFFSET				0x1018
// SW modifiable bits
#define OTP_STATUS0_SW_MASK				0x00000007
// bits defined at reset
#define OTP_STATUS0_RSTMASK				0xfffffff8
// reset value (ignore bits undefined at reset)
#define OTP_STATUS0_RESET				0x00000000

// 32'h101c (OTP_STATUS1)
#define OTP_STATUS1_EFUSE_READ_DATA_MSB			31
#define OTP_STATUS1_EFUSE_READ_DATA_LSB			0
#define OTP_STATUS1_EFUSE_READ_DATA_MASK		0xffffffff
#define OTP_STATUS1_EFUSE_READ_DATA_GET(x)		(((x) & OTP_STATUS1_EFUSE_READ_DATA_MASK) >> OTP_STATUS1_EFUSE_READ_DATA_LSB)
#define OTP_STATUS1_EFUSE_READ_DATA_SET(x)		(((x) << OTP_STATUS1_EFUSE_READ_DATA_LSB) & OTP_STATUS1_EFUSE_READ_DATA_MASK)
// #define OTP_STATUS1_EFUSE_READ_DATA_RESET		32'd0
#define OTP_STATUS1_ADDRESS				0x101c
#define OTP_STATUS1_OFFSET				0x101c
// SW modifiable bits
#define OTP_STATUS1_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_STATUS1_RSTMASK				0x00000000
// reset value (ignore bits undefined at reset)
#define OTP_STATUS1_RESET				0x00000000

// 32'h1020 (OTP_INTF6)
#define OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_MSB		31
#define OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_LSB		0
#define OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_MASK	0xffffffff
#define OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_GET(x)	(((x) & OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_MASK) >> OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_LSB)
#define OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_SET(x)	(((x) << OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_LSB) & OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_MASK)
// #define OTP_INTF6_BACK_TO_BACK_ACCESS_DELAY_RESET	32'dBACK_TO_BACK_ACCESS_DELAY_RESET_IN
#define OTP_INTF6_ADDRESS				0x1020
#define OTP_INTF6_OFFSET				0x1020
// SW modifiable bits
#define OTP_INTF6_SW_MASK				0xffffffff
// bits defined at reset
#define OTP_INTF6_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_INTF6_RESET					0x00000000

// 32'h1024 (OTP_LDO_CONTROL)
#define OTP_LDO_CONTROL_ENABLE_MSB			0
#define OTP_LDO_CONTROL_ENABLE_LSB			0
#define OTP_LDO_CONTROL_ENABLE_MASK			0x00000001
#define OTP_LDO_CONTROL_ENABLE_GET(x)			(((x) & OTP_LDO_CONTROL_ENABLE_MASK) >> OTP_LDO_CONTROL_ENABLE_LSB)
#define OTP_LDO_CONTROL_ENABLE_SET(x)			(((x) << OTP_LDO_CONTROL_ENABLE_LSB) & OTP_LDO_CONTROL_ENABLE_MASK)
// #define OTP_LDO_CONTROL_ENABLE_RESET			1'd0
#define OTP_LDO_CONTROL_ADDRESS				0x1024
#define OTP_LDO_CONTROL_OFFSET				0x1024
// SW modifiable bits
#define OTP_LDO_CONTROL_SW_MASK				0x00000001
// bits defined at reset
#define OTP_LDO_CONTROL_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_LDO_CONTROL_RESET				0x00000000

// 32'h1028 (OTP_LDO_POWER_GOOD)
#define OTP_LDO_POWER_GOOD_DELAY_MSB			11
#define OTP_LDO_POWER_GOOD_DELAY_LSB			0
#define OTP_LDO_POWER_GOOD_DELAY_MASK			0x00000fff
#define OTP_LDO_POWER_GOOD_DELAY_GET(x)			(((x) & OTP_LDO_POWER_GOOD_DELAY_MASK) >> OTP_LDO_POWER_GOOD_DELAY_LSB)
#define OTP_LDO_POWER_GOOD_DELAY_SET(x)			(((x) << OTP_LDO_POWER_GOOD_DELAY_LSB) & OTP_LDO_POWER_GOOD_DELAY_MASK)
// #define OTP_LDO_POWER_GOOD_DELAY_RESET			12'dOTP_LDO_POWER_GOOD_RESET_IN
#define OTP_LDO_POWER_GOOD_ADDRESS			0x1028
#define OTP_LDO_POWER_GOOD_OFFSET			0x1028
// SW modifiable bits
#define OTP_LDO_POWER_GOOD_SW_MASK			0x00000fff
// bits defined at reset
#define OTP_LDO_POWER_GOOD_RSTMASK			0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_LDO_POWER_GOOD_RESET			0x00000000

// 32'h102c (OTP_LDO_STATUS)
#define OTP_LDO_STATUS_POWER_ON_MSB			0
#define OTP_LDO_STATUS_POWER_ON_LSB			0
#define OTP_LDO_STATUS_POWER_ON_MASK			0x00000001
#define OTP_LDO_STATUS_POWER_ON_GET(x)			(((x) & OTP_LDO_STATUS_POWER_ON_MASK) >> OTP_LDO_STATUS_POWER_ON_LSB)
#define OTP_LDO_STATUS_POWER_ON_SET(x)			(((x) << OTP_LDO_STATUS_POWER_ON_LSB) & OTP_LDO_STATUS_POWER_ON_MASK)
// #define OTP_LDO_STATUS_POWER_ON_RESET			1'd0
#define OTP_LDO_STATUS_ADDRESS				0x102c
#define OTP_LDO_STATUS_OFFSET				0x102c
// SW modifiable bits
#define OTP_LDO_STATUS_SW_MASK				0x00000001
// bits defined at reset
#define OTP_LDO_STATUS_RSTMASK				0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_LDO_STATUS_RESET				0x00000000

// 32'h1030 (OTP_VDDQ_HOLD_TIME)
#define OTP_VDDQ_HOLD_TIME_DELAY_MSB			31
#define OTP_VDDQ_HOLD_TIME_DELAY_LSB			0
#define OTP_VDDQ_HOLD_TIME_DELAY_MASK			0xffffffff
#define OTP_VDDQ_HOLD_TIME_DELAY_GET(x)			(((x) & OTP_VDDQ_HOLD_TIME_DELAY_MASK) >> OTP_VDDQ_HOLD_TIME_DELAY_LSB)
#define OTP_VDDQ_HOLD_TIME_DELAY_SET(x)			(((x) << OTP_VDDQ_HOLD_TIME_DELAY_LSB) & OTP_VDDQ_HOLD_TIME_DELAY_MASK)
// #define OTP_VDDQ_HOLD_TIME_DELAY_RESET			32'dOTP_VDDQ_HOLD_TIME_RESET_IN
#define OTP_VDDQ_HOLD_TIME_ADDRESS			0x1030
#define OTP_VDDQ_HOLD_TIME_OFFSET			0x1030
// SW modifiable bits
#define OTP_VDDQ_HOLD_TIME_SW_MASK			0xffffffff
// bits defined at reset
#define OTP_VDDQ_HOLD_TIME_RSTMASK			0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_VDDQ_HOLD_TIME_RESET			0x00000000

// 32'h1034 (OTP_PGENB_SETUP_HOLD_TIME)
#define OTP_PGENB_SETUP_HOLD_TIME_DELAY_MSB		31
#define OTP_PGENB_SETUP_HOLD_TIME_DELAY_LSB		0
#define OTP_PGENB_SETUP_HOLD_TIME_DELAY_MASK		0xffffffff
#define OTP_PGENB_SETUP_HOLD_TIME_DELAY_GET(x)		(((x) & OTP_PGENB_SETUP_HOLD_TIME_DELAY_MASK) >> OTP_PGENB_SETUP_HOLD_TIME_DELAY_LSB)
#define OTP_PGENB_SETUP_HOLD_TIME_DELAY_SET(x)		(((x) << OTP_PGENB_SETUP_HOLD_TIME_DELAY_LSB) & OTP_PGENB_SETUP_HOLD_TIME_DELAY_MASK)
// #define OTP_PGENB_SETUP_HOLD_TIME_DELAY_RESET		32'dOTP_PGENB_SETUP_HOLD_TIME_RESET_IN
#define OTP_PGENB_SETUP_HOLD_TIME_ADDRESS		0x1034
#define OTP_PGENB_SETUP_HOLD_TIME_OFFSET		0x1034
// SW modifiable bits
#define OTP_PGENB_SETUP_HOLD_TIME_SW_MASK		0xffffffff
// bits defined at reset
#define OTP_PGENB_SETUP_HOLD_TIME_RSTMASK		0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_PGENB_SETUP_HOLD_TIME_RESET			0x00000000

// 32'h1038 (OTP_STROBE_PULSE_INTERVAL)
#define OTP_STROBE_PULSE_INTERVAL_DELAY_MSB		31
#define OTP_STROBE_PULSE_INTERVAL_DELAY_LSB		0
#define OTP_STROBE_PULSE_INTERVAL_DELAY_MASK		0xffffffff
#define OTP_STROBE_PULSE_INTERVAL_DELAY_GET(x)		(((x) & OTP_STROBE_PULSE_INTERVAL_DELAY_MASK) >> OTP_STROBE_PULSE_INTERVAL_DELAY_LSB)
#define OTP_STROBE_PULSE_INTERVAL_DELAY_SET(x)		(((x) << OTP_STROBE_PULSE_INTERVAL_DELAY_LSB) & OTP_STROBE_PULSE_INTERVAL_DELAY_MASK)
// #define OTP_STROBE_PULSE_INTERVAL_DELAY_RESET		32'dOTP_STROBE_PULSE_INTERVAL_RESET_IN
#define OTP_STROBE_PULSE_INTERVAL_ADDRESS		0x1038
#define OTP_STROBE_PULSE_INTERVAL_OFFSET		0x1038
// SW modifiable bits
#define OTP_STROBE_PULSE_INTERVAL_SW_MASK		0xffffffff
// bits defined at reset
#define OTP_STROBE_PULSE_INTERVAL_RSTMASK		0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_STROBE_PULSE_INTERVAL_RESET			0x00000000

// 32'h103c (OTP_CSB_ADDR_LOAD_SETUP_HOLD)
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_MSB		31
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_LSB		0
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_MASK		0xffffffff
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_GET(x)	(((x) & OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_MASK) >> OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_LSB)
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_SET(x)	(((x) << OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_LSB) & OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_MASK)
// #define OTP_CSB_ADDR_LOAD_SETUP_HOLD_DELAY_RESET	32'dOTP_CSB_ADDR_LOAD_SETUP_HOLD_RESET_IN
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_ADDRESS		0x103c
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_OFFSET		0x103c
// SW modifiable bits
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_SW_MASK		0xffffffff
// bits defined at reset
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_RSTMASK		0xffffffff
// reset value (ignore bits undefined at reset)
#define OTP_CSB_ADDR_LOAD_SETUP_HOLD_RESET		0x00000000

#define	ATH_OTP_SIZE					(1 << 10)
#define ATH_OTP_INTF0_ADDRESS				(ATH_OTP_BASE + OTP_INTF0_ADDRESS)
#define ATH_OTP_INTF2_ADDRESS				(ATH_OTP_BASE + OTP_INTF2_ADDRESS)
#define ATH_OTP_INTF3_ADDRESS				(ATH_OTP_BASE + OTP_INTF3_ADDRESS)
#define ATH_OTP_PGENB_SETUP_HOLD_TIME_ADDRESS		(ATH_OTP_BASE + OTP_PGENB_SETUP_HOLD_TIME_ADDRESS)
#define ATH_OTP_MEM_ADDRESS				(ATH_OTP_BASE + OTP_MEM_0_ADDRESS)
#define ATH_OTP_STATUS0_ADDRESS				(ATH_OTP_BASE + OTP_STATUS0_ADDRESS)
#define ATH_OTP_STATUS1_ADDRESS				(ATH_OTP_BASE + OTP_STATUS1_ADDRESS)
#define ATH_OTP_LDO_CONTROL				(ATH_OTP_BASE + OTP_LDO_CONTROL_ADDRESS)
#define ATH_OTP_LDO_STATUS				(ATH_OTP_BASE + OTP_LDO_STATUS_ADDRESS)

#define ATH_PCIE_RC_ASPM_SUPPORT			0x180c007c
#define ATH_PCIE_RC_SUPP_L0				(1 << 10)
#define ATH_PCIE_RC_SUPP_L1				(1 << 11)

#define ATH_PCIE_RC_ASPM_ENABLE				0x180c0080
#define ATH_PCIE_RC_EN_L0				(1 << 0)
#define ATH_PCIE_RC_EN_L1				(1 << 1)

#define ATH_PCIE_EP_ASPM_ENABLE				0x14000080
#define ATH_PCIE_EP_EN_L0				(1 << 0)
#define ATH_PCIE_EP_EN_L1				(1 << 1)
#endif /* _934x_H */
